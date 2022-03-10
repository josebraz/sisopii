
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <iostream>
#include <vector>
#include <string>


#include "../constants.h"
#include "../types.hpp"
#include "../server/persistence.hpp"
#include "../communication_utils.hpp"

#include "../server/server_comm_manager.hpp"
#include "../client/client_comm_manager.hpp"

void print_users(user_p *users, int total) {
    for (int i = 0; i < total; i++)
    {
        if (users[i]->follows->size() > 0)
        {
            cerr << "Username " << users[i]->username << " first follower " << users[i]->follows->front() << endl;
        }
        else
        {
            cerr << "Username " << users[i]->username << endl;
        }
    }
}

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

    user_p user2 = new user();
    user2->username = "user2";
    user2->follows = followers2;

    user_p user3 = new user();
    user3->username = "user3";
    user3->follows = followers3;

    *test_users = new user_p[total];

    (*test_users)[0] = user1;
    (*test_users)[1] = user2;
    (*test_users)[2] = user3;

    return total;
}

bool compare_user(user_p user1, user_p user2) {
    if (user1->username.compare(user2->username) != 0) {
        cerr << "Not same username " << user1->username << " != " << user2->username << endl;
        return false;
    }

    if (user1->follows->size() != user2->follows->size()) {
        cerr << "Not same follows size " << user1->follows->size() << " != " << user2->follows->size() << endl;
        return false;
    }

    for (int i = 0; i < user1->follows->size(); i++) {
        if (user1->follows->at(i).compare(user2->follows->at(i)) != 0) {
            cerr << "Not same follow " << user1->follows->at(i) << " != " << user2->follows->at(i) << endl;
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

    user_p *test_users_read;
    int read_total = read_users(&test_users_read);

    for (int i = 0; i < total; i++) {
        if (!compare_user(test_users_read[i], test_users[i])) {
            return false;
        }
    }

    free(test_users);
    free(test_users_read);

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
        cerr << "Not same type " << readed_message->type << " != " << original_message.type << endl;
        return false;
    }

    if (readed_message->seqn != original_message.seqn) {
        cerr << "Not same seqn " << readed_message->seqn << " != " << original_message.seqn << endl;
        return false;
    }

    if (readed_message->length != original_message.length) {
        cerr << "Not same length " << readed_message->length << " != " << original_message.length << endl;
        return false;
    }

    if (strcmp(readed_message->payload, original_message.payload) != 0) {
        cerr << "Not same payload " << readed_message->payload << " != " << original_message.payload << endl;
        return false;
    }

    free_packet(readed_message);
    free(buffer);

    cout << "OK" << endl;
    return true;
}

void server_client_test() {
    cout << "server_client_test... " << endl;

    char user[] = "jose"; 
    char server_addr[] = "localhost";

    pthread_t s_thread = start_server(SERVER_PORT);
    pthread_t c_thread = start_client(server_addr, SERVER_PORT);
    
    int login_result = send_echo_msg("TA AI?");

    printf("login_result %d\n", login_result);

    // pthread_join(s_thread, NULL);

    cout << "OK" << endl;
}

int main(int argc, char **argv) {
    persistence_test();
    marshalling_packet_test();
    server_client_test();

    return 0;
}