
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include <iostream>
#include <vector>
#include <string>

#include "../constants.h"
#include "../types.hpp"
#include "../server/persistence.hpp"
#include "../communication_utils.hpp"

#include "../server/server_notif_manager.hpp"
#include "../server/server_comm_manager.hpp"
#include "../server/session_manager.hpp"
#include "../client/client_comm_manager.hpp"

user_address address1 = {
    AF_INET,
    4000,
    999930,
    {0,0,0,0,0,0,0,0}
};

user_address address2 = {
    AF_INET,
    30000,
    190930,
    {0,0,0,0,0,0,0,0}
};

user_address address3 = {
    AF_INET,
    2050,
    10930,
    {0,0,0,0,0,0,0,0}
};

user_address address4 = {
    AF_INET,
    2053,
    10400,
    {0,0,0,0,0,0,0,0}
};

int create_test_users(user_p *test_users[]) {
    int total = 3;

    vector<string> *followers1 = new vector<string>();
    followers1->push_back("user2");
    followers1->push_back("user3");

    vector<string> *followers2 = new vector<string>();
    followers2->push_back("user1");
    followers2->push_back("user3");

    vector<string> *followers3 = new vector<string>();
    followers3->push_back("user1");

    user_p user1 = new user();
    user1->username = "user1";
    user1->follows = followers1;
    user1->addresses = new vector<user_address*>();
    user1->addr_seqn = new vector<uint16_t>();
    user1->pending_msg = new vector<uint32_t>();
    pthread_mutex_init(&(user1->mutex_addr), NULL);
    pthread_mutex_init(&(user1->mutex_follows), NULL);

    user_p user2 = new user();
    user2->username = "user2";
    user2->follows = followers2;
    user2->addresses = new vector<user_address*>();
    user2->addr_seqn = new vector<uint16_t>();
    user2->pending_msg = new vector<uint32_t>();
    pthread_mutex_init(&(user2->mutex_addr), NULL);
    pthread_mutex_init(&(user2->mutex_follows), NULL);

    user_p user3 = new user();
    user3->username = "user3";
    user3->follows = followers3;
    user3->addresses = new vector<user_address*>();
    user3->addr_seqn = new vector<uint16_t>();
    user3->pending_msg = new vector<uint32_t>();
    pthread_mutex_init(&(user3->mutex_addr), NULL);
    pthread_mutex_init(&(user3->mutex_follows), NULL);

    *test_users = new user_p[total];

    (*test_users)[0] = user1;
    (*test_users)[1] = user2;
    (*test_users)[2] = user3;

    return total;
}

bool compare_user(user_p user1, user_p user2) {
    if (user1->username.compare(user2->username) != 0) {
        cerr << "ERRO" << endl << "Not same username " << user1->username << " != " << user2->username << endl;
        return false;
    }

    if (user1->follows->size() != user2->follows->size()) {
        cerr << "ERRO" << endl << "Not same follows size " << user1->follows->size() << " != " << user2->follows->size() << endl;
        return false;
    }

    for (int i = 0; i < user1->follows->size(); i++) {
        if (user1->follows->at(i).compare(user2->follows->at(i)) != 0) {
            cerr << "ERRO" << endl << "Not same follow " << user1->follows->at(i) << " != " << user2->follows->at(i) << endl;
            return false;
        }
    }

    return true;
} 

bool persistence_test() {
    cout << "persistence_test... ";

    user_p *test_users;
    int total = create_test_users(&test_users);

    write_users((const user_p*) test_users, total);

    user_p test_users_read[10];
    int read_total = read_users(test_users_read);

    for (int i = 0; i < total; i++) {
        if (!compare_user(test_users_read[i], test_users[i])) {
            return false;
        }
    }

    for (int i = 0; i < total; i++) {
        free_user(test_users[i]);
    }

    cout << "OK" << endl;
    return true;
}

bool marshalling_packet_test() {
    cout << "marshalling_packet_test... ";

    packet *readed_message;
    char *buffer;
    char payload[] = "TESTEEE";

    packet original_message = {
        3U, 
        4U, 
        (uint16_t) strlen(payload), 
        (uint32_t) time(NULL),
        payload
    };

    size_t message_size = marshalling_packet(&original_message, &buffer);
    unmarshalling_packet(&readed_message, buffer);

    if (readed_message->type != original_message.type) {
        cerr << "ERRO" << endl << "Not same type " << readed_message->type << " != " << original_message.type << endl;
        return false;
    }

    if (readed_message->seqn != original_message.seqn) {
        cerr << "ERRO" << endl << "Not same seqn " << readed_message->seqn << " != " << original_message.seqn << endl;
        return false;
    }

    if (readed_message->length != original_message.length) {
        cerr << "ERRO" << endl << "Not same length " << readed_message->length << " != " << original_message.length << endl;
        return false;
    }

    if (strcmp(readed_message->payload, original_message.payload) != 0) {
        cerr << "ERRO" << endl << "Not same payload " << readed_message->payload << " != " << original_message.payload << endl;
        return false;
    }

    free_packet(readed_message);
    free(buffer);

    cout << "OK" << endl;
    return true;
}

bool marshalling_notification_test() {
    cout << "marshalling_notification_test... ";

    notification *readed_message;
    char *buffer;
    char payload[] = "TESTEEE";
    char author[] = "josebraz";

    notification original_message = {
        1, 
        (uint32_t) time(NULL), 
        (uint16_t) strlen(payload), 
        0,
        (uint16_t) strlen(author), 
        author,
        payload
    };

    size_t message_size = marshalling_notification(&original_message, &buffer);
    unmarshalling_notification(&readed_message, buffer);

    if (readed_message->id != original_message.id) {
        cerr << "ERRO" << endl << "Not same id " << readed_message->id << " != " << original_message.id << endl;
        return false;
    }

    if (readed_message->timestamp != original_message.timestamp) {
        cerr << "ERRO" << endl << "Not same timestamp " << readed_message->timestamp << " != " << original_message.timestamp << endl;
        return false;
    }

    if (readed_message->length != original_message.length) {
        cerr << "ERRO" << endl << "Not same length " << readed_message->length << " != " << original_message.length << endl;
        return false;
    }

    if (readed_message->author_len != original_message.author_len) {
        cerr << "ERRO" << endl << "Not same author_len " << readed_message->author_len << " != " << original_message.author_len << endl;
        return false;
    }

    if (strcmp(readed_message->author, original_message.author) != 0) {
        cerr << "ERRO" << endl << "Not same author " << readed_message->author << " != " << original_message.author << endl;
        return false;
    }

    if (strcmp(readed_message->message, original_message.message) != 0) {
        cerr << "ERRO" << endl << "Not same message " << readed_message->message << " != " << original_message.message << endl;
        return false;
    }

    free_notification(readed_message);
    free(buffer);

    cout << "OK" << endl;
    return true;
}

bool server_client_echo_test() {
    cout << "server_client_echo_test... ";

    char user[] = "jose"; 
    char server_addr[] = "localhost";
    char echo_message[] = "TA AI?";

    pthread_t s_thread = start_server(SERVER_PORT);
    pthread_t c_thread = start_client(server_addr, SERVER_PORT);
    
    int echo_result = send_echo_msg(echo_message);

    if (echo_result != 1) {
        cerr << "ERRO" << endl << "Echo result error" << endl;
        return false;
    }

    cout << "OK" << endl;
    return true;
}

bool session_manager_test() {
    cout << "session_manager_test... ";

    char message[100];
    char user[] = "jose";
    int result_code;

    clear_all_users();
    init_session_manager();

    // Primeiro login, deve criar usuário
    result_code = login(user, &address1, message);
    if (result_code != PACKET_DATA_LOGIN_OK_T) {
        cerr << "ERRO" << endl << "Erro no primeiro login" << endl;
        return false;
    }

    // Repete o último login, com o mesmo endereço
    result_code = login(user, &address1, message);
    if (result_code != PACKET_DATA_LOGIN_ERROR_T) {
        cerr << "ERRO" << endl << "Mesmo login, no mesmo processo, deveria dar erro" << endl;
        return false;
    }

    // Segundo login, atinge o máximo de sessões
    result_code = login(user, &address2, message);
    if (result_code != PACKET_DATA_LOGIN_OK_T) {
        cerr << "ERRO" << endl << "Erro no segundo login" << endl;
        return false;
    }

    // Deve dar erro de máximas sessões
    result_code = login(user, &address3, message);
    if (result_code != PACKET_DATA_LOGIN_ERROR_T) {
        cerr << "ERRO" << endl << "Não respita máximo de sessões simultâneas" << endl;
        return false;
    }

    // Logout de uma das sessões ativas
    result_code = logout(user, &address1, message);
    if (result_code != PACKET_DATA_LOGOUT_OK_T) {
        cerr << "ERRO" << endl << "Erro no logout com duas sessões simultâneas" << endl;
        return false;
    }

    // Tenta dar logout de um endereço que não tem sessão
    result_code = logout(user, &address3, message);
    if (result_code != PACKET_DATA_LOGOUT_ERROR_T) {
        cerr << "ERRO" << endl << "Logout sem nenhum sessão funcionou (?)" << endl;
        return false;
    }

    // Depois de um logout, tenta logar mais uma vez
    result_code = login(user, &address1, message);
    if (result_code != PACKET_DATA_LOGIN_OK_T) {
        cerr << "ERRO" << endl << "Erro no segundo login após logout" << endl;
        return false;
    }

    // Logout da última sessão ativa
    result_code = logout(user, &address2, message);
    if (result_code != PACKET_DATA_LOGOUT_OK_T) {
        cerr << "ERRO" << endl << "Erro no logout da ultima sessão ativa" << endl;
        return false;
    }

    finalize_session_manager();

    cout << "OK" << endl;
    return true;
}

bool follow_unfollow_test() {
    cout << "follow_unfollow_test... ";

    char message[100];
    char user1[] = "jose";
    char user2[] = "gabriel";
    char user3[] = "matheus";
    int result_code;

    //////////////////////////////////
    // Setup
    clear_all_users();
    init_session_manager();

    login(user1, &address1, message);
    login(user2, &address2, message);
    login(user3, &address3, message);
    //////////////////////////////////

    // user1 segue o user2, tudo certo
    result_code = follow(user1, user2, message);
    if (result_code != PACKET_DATA_FOLLOW_OK_T) {
        cerr << "ERRO" << endl << "Erro ao user1 seguir o user2" << endl;
        return false;
    }

    // user1 segue o user2 de novo, deve dar erro
    result_code = follow(user1, user2, message);
    if (result_code != PACKET_DATA_FOLLOW_ERROR_T) {
        cerr << "ERRO" << endl << "user1 conseguiu seguir duas vezes o user2" << endl;
        return false;
    }

    // user1 segue o user3, tudo certo
    result_code = follow(user1, user3, message);
    if (result_code != PACKET_DATA_FOLLOW_OK_T) {
        cerr << "ERRO" << endl << "Erro ao user1 seguir o user3" << endl;
        return false;
    }

    // user2 da unfollow no user1 que ele não seguia, deve dar erro
    result_code = unfollow(user2, user1, message);
    if (result_code != PACKET_DATA_UNFOLLOW_ERROR_T) {
        cerr << "ERRO" << endl << "user2 conseguiu dar unfollow no user1 que não seguia" << endl;
        return false;
    }

    // user1 deixa de seguir o user3, tudo certo
    result_code = unfollow(user1, user3, message);
    if (result_code != PACKET_DATA_UNFOLLOW_OK_T) {
        cerr << "ERRO" << endl << "Erro ao user1 deixar de seguir o user3" << endl;
        return false;
    }

    finalize_session_manager();

    cout << "OK" << endl;
    return true;
}

static int step_callback = 0;

bool notification_test_callback(uint16_t type, notification *notif, const user_address *cliaddr, const uint16_t seqn) {
    if (step_callback == 0) {
        if (!(strcmp(notif->author, (char *) "gabriel") == 0 && notif->id == 1)) {
            cerr << "ERRO" << endl << "Notificação recebida errada no step " << step_callback << " seqn " << seqn << endl;
            exit(1);
        }
    } else if (step_callback == 1) {
        if (!(strcmp(notif->author, (char *) "gabriel") == 0 && notif->id == 2)) {
            cerr << "ERRO" << endl << "Notificação recebida errada no step " << step_callback << " seqn " << seqn << endl;
            exit(1);
        }
    } else if (step_callback == 2) {
        if (!(strcmp(notif->author, (char *) "matheus") == 0 && notif->id == 3)) {
            cerr << "ERRO" << endl << "Notificação recebida errada no step " << step_callback << " seqn " << seqn << endl;
            exit(1);
        }
    } else if (step_callback == 3) {
        if (!(strcmp(notif->author, (char *) "matheus") == 0 && notif->id == 3)) {
            cerr << "ERRO" << endl << "Notificação recebida errada no step " << step_callback << " seqn " << seqn << endl;
            exit(1);
        }
    } else if (step_callback == 4) {
        if (!(strcmp(notif->author, (char *) "jose") == 0 && notif->id == 0)) {
            cerr << "ERRO" << endl << "Notificação recebida errada no step " << step_callback << " seqn " << seqn << endl;
            exit(1);
        }
    } else if (step_callback == 5) {
         if (!(strcmp(notif->author, (char *) "jose") == 0 && notif->id == 4)) {
            cerr << "ERRO" << endl << "Notificação recebida errada no step " << step_callback << " seqn " << seqn << endl;
            exit(1);
        }
    } else if (step_callback == 6) {
         if (!(strcmp(notif->author, (char *) "jose") == 0 && notif->id == 5)) {
            cerr << "ERRO" << endl << "Notificação recebida errada no step " << step_callback << " seqn " << seqn << endl;
            exit(1);
        }
    } else if (step_callback == 7) {
         if (!(strcmp(notif->author, (char *) "jose") == 0 && notif->id == 5)) {
            cerr << "ERRO" << endl << "Notificação recebida errada no step " << step_callback << " seqn " << seqn << endl;
            exit(1);
        }
    }
    step_callback++;
    return true;
}

bool notification_test() {
    cout << "notification_test... ";

    char message[100];
    char user1[] = "jose";
    char user2[] = "gabriel";
    char user3[] = "matheus";
    int result_code;

    //////////////////////////////////
    // Setup
    clear_all_users();
    init_session_manager();
    pthread_t s = start_server_notif_mng(&notification_test_callback);

    login(user1, &address1, message);
    login(user2, &address2, message);
    login(user3, &address3, message);

    follow(user1, user2, message);
    follow(user1, user3, message);
    follow(user2, user3, message);
    follow(user3, user1, message);

    logout(user3, &address3, message);
    //////////////////////////////////
    
    producer_new_notification(find_user(user1), "Oi galeraaaa!");
    producer_new_notification(find_user(user2), "Oi galera!");
    producer_new_notification(find_user(user2), "Como voces estão?");
    producer_new_notification(find_user(user3), "E ai blz?");
    producer_new_notification(find_user(user1), "Tudo ótimo aqui!");

    // depois o user3 loga e precisa receber a primeira notificação
    usleep(500);
    login(user3, &address3, message);

    // aqui o user3 loga de outro lugar e não pode receber outra notificação
    usleep(500);
    login(user3, &address4, message);
    usleep(500);
    
    producer_new_notification(find_user(user1), "Bem vindo user3 (de novo?)!");

    // espera um tempinho para garantir que o serviço já enviou tudo
    usleep(500);

    if (step_callback != 8) {
        cerr << "ERRO" << endl << "Não recebemos todas as notificações " << step_callback << endl;
        exit(1);
    }

    finalize_session_manager();

    cout << "OK" << endl;
    return true;
}

int main(int argc, char **argv) {
    persistence_test();
    marshalling_packet_test();
    marshalling_notification_test();
    server_client_echo_test();
    session_manager_test();
    follow_unfollow_test();
    notification_test();

    return 0;
}