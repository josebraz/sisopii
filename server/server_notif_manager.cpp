#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>

#include <vector>
#include <iostream>
#include <string>

#include "server_notif_manager.hpp"
#include "session_manager.hpp"
#include "../communication_utils.hpp" 
#include "../types.hpp"

#define MAX_PENDING_MSG 1000

using namespace std;

uint32_t next_notif_id = 0;
vector<notification*> pending_notifications;

pthread_cond_t cond_more, cond_less;
pthread_mutex_t mutex_notif;
pthread_mutex_t mutex_send;

send_notif_callback_t send_notif_callback;

void *consumer_notification(void *arg);

void server_sig_handler(sig_atomic_t sig) {
    // manda uma mensagem para todos os clientes conectados deslogando eles
    for (int i = 0; i < server_users_size; i++) {
        // send_to_all_addresses(PACKET_CMD_END_SERVER, server_users[i], (char *)"Servidor finalizado");
    }

    printf("\nServidor encerrado\n");
    exit(1);
}

pthread_t start_server_notif_mng(send_notif_callback_t cb) {
    send_notif_callback = cb;
    signal(SIGINT, server_sig_handler);

    int ret;
    if (pthread_cond_init(&cond_more, NULL) != 0 || 
            pthread_cond_init(&cond_less, NULL) != 0) {
        perror("cond init failed");
        exit(EXIT_FAILURE);
    }

    ret = pthread_mutex_init(&mutex_notif, NULL);
    if (ret != 0) {
        perror("mutex notif init failed");
        exit(EXIT_FAILURE);
    }

    ret = pthread_mutex_init(&mutex_send, NULL);
    if (ret != 0) {
        perror("mutex send init failed");
        exit(EXIT_FAILURE);
    }

    pthread_t consumer_thread;
    pthread_create(&consumer_thread, NULL, consumer_notification, NULL);
    return consumer_thread;
}

void producer_new_notification(const user_p author, const char *message) {
    if (author->follows->empty()) return;

    pthread_mutex_lock(&mutex_notif);
    while (pending_notifications.size() >= MAX_PENDING_MSG)
        pthread_cond_wait(&cond_less, &mutex_notif);

    uint32_t notif_id = next_notif_id++;
    notification *new_notif = new notification();
    new_notif->id = notif_id;
    new_notif->length = strlen(message);
    new_notif->timestamp = (uint32_t) time(NULL);
    new_notif->pending = author->follows->size();
    new_notif->author_len = author->username.size();
    new_notif->author = strdup(author->username.c_str());
    new_notif->message = strdup(message);
    pending_notifications.push_back(new_notif);

    user_p user_follow;
    // Marca todos os usuários que seguem o autor para receberem essa notificação
    for (int i = 0; i < author->follows->size(); i++) {
        user_follow = find_user(author->follows->at(i).c_str());
        if (user_follow != NULL) {
            user_follow->pending_msg->push_back(notif_id);
        }
    }
    
    pthread_cond_signal(&cond_more);
    pthread_mutex_unlock(&mutex_notif);
}

bool send_to_all_addresses(const uint16_t type, const user_p user_follow, notification *message) {
    bool send_success = false;
    pthread_mutex_lock(&(user_follow->mutex_addr));
    for (int i = 0; i < user_follow->addresses->size(); i++) {
        session_addr *addr = user_follow->addresses->at(i);
        bool current_success = send_notif_callback(type, message, addr);
        if (current_success) {
            addr->seqn += 1;
        }
        send_success = current_success || send_success;
    }
    pthread_mutex_unlock(&(user_follow->mutex_addr));
    return send_success;
}

void *consumer_notification(void *arg) {
    int notif_index = 0;
    while (true) {
        pthread_mutex_lock(&mutex_notif);
        while(pending_notifications.empty())
            pthread_cond_wait(&cond_more, &mutex_notif);

        notification *current_notif = pending_notifications.at(notif_index);
        user_p author = find_user(current_notif->author);

        pthread_mutex_lock(&(author->mutex_follows));
        for (int i = 0; current_notif->pending != 0 && i < author->follows->size(); i++) {
            user_p user_follow = find_user(author->follows->at(i).c_str());
            
            if (user_follow->pending_msg->empty()) continue;

            // verifica se esse usuário está esperando esta mensagem como próxima mensagem
            bool pending_this = user_follow->pending_msg->front() == current_notif->id;

            // caso esteja, enviamos para todos os endereços ativos no momento
            if (pending_this) {
                if (send_to_all_addresses(PACKET_CMD_NOTIFY_T, user_follow, current_notif)) {
                    user_follow->pending_msg->erase(user_follow->pending_msg->begin());
                    current_notif->pending -= 1;
                }
            }
        }
        pthread_mutex_unlock(&(author->mutex_follows));

        // caso não haja mais nenhum usuário que precise receber essa
        // notificação, ela é eliminada das notificações pendentes
        if (current_notif->pending == 0) {
            pending_notifications.erase(pending_notifications.begin() + notif_index);
            free_notification(current_notif);
        } else {
            notif_index++;
        }

        if (pending_notifications.size() == 0) {
            notif_index = 0;
        } else {
            notif_index %= pending_notifications.size();
        }
        
        pthread_cond_signal(&cond_less);
        pthread_mutex_unlock(&mutex_notif);
    }
    
}