#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#define USER_SESSION_MAX_SIZE 1000

#include <stdint.h>

#include "../types.hpp"

static int server_users_size = 0;
static user_p server_users[USER_SESSION_MAX_SIZE];

void init_session_manager();

void finalize_session_manager();

user_p find_user(const char *username);

user_p find_user_by_address(const user_address *address);

int index_of_address(const user_p local_user, const user_address *address);

int login(const char *username, const user_address *address, char *message);

int logout(const char *username, const user_address *address, char *message);

int follow(const char *my_username, const char *followed, char *message);

int unfollow(const char *my_username, const char *followed, char *message);


#endif