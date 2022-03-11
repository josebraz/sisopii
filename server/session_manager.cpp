#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "session_manager.hpp"
#include "persistence.hpp"
#include "../types.hpp"

#define USER_SESSION_MAX_SIZE 1000
#define USER_MAX_SESSIONS 2

static int server_users_size = 0;
static user_p server_users[USER_SESSION_MAX_SIZE];

void init_session_manager() {
    server_users_size = read_users(server_users);
}

user_p find_user(const char *username) {
    for (int i = 0; i < server_users_size; i++) {
        if (strcmp(server_users[i]->username.c_str(), username) == 0) {
            return server_users[i];
        }
    }
    return NULL;
}

int login(const char *username, char *message) {
    user_p local_user = find_user(username);
    if (local_user == NULL) { 
        if (server_users_size < USER_SESSION_MAX_SIZE) {
            user_p local_user = new user();
            local_user->username = strdup(username);
            local_user->follows = new vector<string>();
            local_user->sessions = 1;

            server_users[server_users_size++] = local_user;
            write_users(server_users, server_users_size);

            strcpy(message, "Usuário criado com sucesso!");
            return PACKET_DATA_LOGIN_OK_T;
        } else {
            strcpy(message, "Máximo número de usuários no servidor");
            return PACKET_DATA_LOGIN_ERROR_T;
        }
    } else {
        if (local_user->sessions >= USER_MAX_SESSIONS) {
            strcpy(message, "Máximo número de sessões do mesmo usuário");
            return PACKET_DATA_LOGIN_ERROR_T;
        } else {
            local_user->sessions += 1;
            strcpy(message, "Login OK!");
            return PACKET_DATA_LOGIN_OK_T;
        }
    }
}

int logout(const char *username, char *message) {
    user_p local_user = find_user(username);
    if (local_user == NULL) {
        strcpy(message, "Usuário não encontrado");
        return PACKET_DATA_LOGOUT_ERROR_T;
    } else {
        if (local_user->sessions == 0) {
            strcpy(message, "Usuário não encontrado");
            return PACKET_DATA_LOGOUT_ERROR_T;
        } else {
            local_user->sessions -= 1;
            strcpy(message, "Usuário desconectado com sucesso");
            return PACKET_DATA_LOGOUT_OK_T;
        }
    }
}