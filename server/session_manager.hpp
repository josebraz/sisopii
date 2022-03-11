#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include <stdint.h>

#include "../types.hpp"

void init_session_manager();

user_p find_user(const char *username);

int login(const char *username, char *message);

int logout(const char *username, char *message);


#endif