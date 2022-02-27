
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <vector>
#include <string>

#include "../constants.h"
#include "../types.hpp"
#include "../server/persistence.hpp"

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

void persistence_test() {
    user_p *test_users;
    int total = create_test_users(&test_users);

    write_users((const user_p*) test_users, total);

    user_p *test_users_read;
    int read_total = read_users(&test_users_read);

    for (int i = 0; i < total; i++) {
        compare_user(test_users_read[i], test_users[i]);
    }
}

int main(int argc, char **argv) {
    persistence_test();

    return 0;
}