#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include <vector>
#include <iostream>

#include "server_notif_manager.hpp"
#include "server_comm_manager.hpp"
#include "session_manager.hpp"
#include "../communication_utils.hpp" 
#include "../types.hpp"

#define MAX_PENDING_MSG 100

using namespace std;

uint32_t new_notif_id = 0; // TODO: Carregar esse valor da persistencia
vector<notification*> pending_notifications; // TODO: Carregar esse valor da persistencia

pthread_cond_t cond_more, cond_less;
pthread_mutex_t mutex_notif;

#ifdef TEST
send_test_callback_t send_test_callback;
void register_callback(send_test_callback_t cb) {
    send_test_callback = cb;
}
#endif

pthread_t start_server_notif_mng() {
    int ret;
    if (pthread_cond_init(&cond_more, NULL) != 0 || 
            pthread_cond_init(&cond_less, NULL) != 0) {
        perror("cond init failed");
        exit(EXIT_FAILURE);
    }

    ret = pthread_mutex_init(&mutex_notif, NULL);
    if (ret != 0) {
        perror("mutex init failed");
        exit(EXIT_FAILURE);
    }

    pthread_t consumer_thread;
    pthread_create(&consumer_thread, NULL, consumer_notification, NULL);
    return consumer_thread;
}

void producer_new_notification(const user_p author, const char *message) {
    pthread_mutex_lock(&mutex_notif);
    while (pending_notifications.size() >= MAX_PENDING_MSG)
        pthread_cond_wait(&cond_less, &mutex_notif);

    uint32_t notif_id = new_notif_id++;
    notification *new_notif = new notification();
    new_notif->id = notif_id;
    new_notif->length = strlen(message);
    new_notif->timestamp = (uint32_t) time(NULL);
    new_notif->pending = author->follows->size();
    new_notif->author = strdup(author->username.c_str());
    new_notif->message = strdup(message);
    pending_notifications.push_back(new_notif);

    // Marca todos os usuários que seguem o autor para receberem essa notificação
    user_p user_follow;
    for (int i = 0; i < author->follows->size(); i++) {
        user_follow = find_user(author->follows->at(i).c_str());
        if (user_follow != NULL) {
            user_follow->pending_msg->push_back(notif_id);
        }
    }

    pthread_cond_signal(&cond_more);
    pthread_mutex_unlock(&mutex_notif);
}

bool send_to_all_addresses(const user_p user_follow, notification *message) {
    bool send_success = false;
    for (int i = 0; i < user_follow->addresses->size(); i++) {
        #ifdef TEST
        send_test_callback(PACKET_CMD_NOTIFY_T, message, user_follow->addresses->at(i));
        #else
        server_send_message(PACKET_CMD_NOTIFY_T, message->message, user_follow->addresses->at(i));
        #endif
        send_success = true;
    }
    return send_success;
}

void *consumer_notification(void *arg) {
    while (true) {
        pthread_mutex_lock(&mutex_notif);
        while(pending_notifications.size() <= 0)
            pthread_cond_wait(&cond_more, &mutex_notif);
        
        notification *current = pending_notifications.front();
        user_p author = find_user(current->author);

        for (int i = 0; current->pending != 0 && i < author->follows->size(); i++) {
            user_p user_follow = find_user(author->follows->at(i).c_str());
            
            if (user_follow->pending_msg->empty()) continue;

            // verifica se esse usuário está esperando esta mensagem como próxima mensagem
            bool pending_this = user_follow->pending_msg->front() == current->id;

            // caso esteja, enviamos para todos os endereços ativos no momento
            if (pending_this && !user_follow->addresses->empty()) {
                if (send_to_all_addresses(user_follow, current)) {
                    user_follow->pending_msg->erase(user_follow->pending_msg->begin());
                    current->pending -= 1;
                }
            }
        }

        // caso não haja mais nenhum usuário que precise receber essa
        // notificação, ela é eliminada das notificações pendentes
        if (current->pending == 0) {
            pending_notifications.erase(pending_notifications.begin());
            free_notification(current);
        }
        
        pthread_cond_signal(&cond_less);
        pthread_mutex_unlock(&mutex_notif);
    }
    
}