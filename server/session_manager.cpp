#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "session_manager.hpp"
#include "persistence.hpp"
#include "../communication_utils.hpp"
#include "../types.hpp"

#define USER_SESSION_MAX_SIZE 1000

static int server_users_size = 0;
static user_p server_users[USER_SESSION_MAX_SIZE];

void init_session_manager() {
    server_users_size = read_users(server_users);
}

void finalize_session_manager() {
    for (int i = 0; i < server_users_size; i++) {
        free_user(server_users[i]);
    }
}

user_p find_user(const char *username) {
    for (int i = 0; i < server_users_size; i++) {
        if (strcmp(server_users[i]->username.c_str(), username) == 0) {
            return server_users[i];
        }
    }
    return NULL;
}

user_p find_user_by_address(const user_address *address) {
    int fold_index;
    for (int i = 0; i < server_users_size; i++) {
        fold_index = index_of_address(server_users[i], address);
        if (fold_index != -1) return server_users[i];
    }
    return NULL;
}

int index_of_address(const user_p local_user, const user_address *address) {
    int fold_index = -1;
    for (int i = 0; i < local_user->addresses->size(); i++) {
        user_address *current = local_user->addresses->at(i);
        if (current->sin_addr.s_addr == address->sin_addr.s_addr && 
                current->sin_port == address->sin_port) {
            fold_index = i;
            break;
        }
    }
    return fold_index;
}

int login(const char *username, const user_address *address, char *message) {
    user_p local_user = find_user(username);
    if (local_user == NULL) { 
        if (server_users_size < USER_SESSION_MAX_SIZE) {
            user_p local_user = new user();
            local_user->username = strdup(username);
            local_user->follows = new vector<string>();
            local_user->addresses = new vector<user_address*>();
            user_address* new_address = new user_address();
            memcpy(new_address, address, sizeof(user_address));
            local_user->addresses->push_back(new_address);
            local_user->pending_msg = new vector<uint32_t>();

            server_users[server_users_size++] = local_user;
            write_users(server_users, server_users_size);

            strcpy(message, "Usuário criado com sucesso!");
            return PACKET_DATA_LOGIN_OK_T;
        } else {
            strcpy(message, "Máximo número de usuários no servidor");
            return PACKET_DATA_LOGIN_ERROR_T;
        }
    } else {
        if (local_user->addresses->size() >= USER_MAX_SESSIONS) {
            strcpy(message, "Máximo número de sessões do mesmo usuário");
            return PACKET_DATA_LOGIN_ERROR_T;
        } else {
            int fold_index = index_of_address(local_user, address);
            if (fold_index != -1) {
                strcpy(message, "Usuário já está logado nesse cliente");
                return PACKET_DATA_LOGIN_ERROR_T;
            } else {
                user_address* new_address = new user_address();
                memcpy(new_address, address, sizeof(user_address));
                local_user->addresses->push_back(new_address);

                strcpy(message, "Login OK!");
                return PACKET_DATA_LOGIN_OK_T;
            }
        }
    }
}

int logout(const char *username, const user_address *address, char *message) {
    user_p local_user = find_user(username);
    if (local_user == NULL) {
        strcpy(message, "Usuário não encontrado");
        return PACKET_DATA_LOGOUT_ERROR_T;
    } else {
        int fold_index = index_of_address(local_user, address);
        if (fold_index == -1) {
            strcpy(message, "Usuário não encontrado");
            return PACKET_DATA_LOGOUT_ERROR_T;
        } else {
            user_address *current = local_user->addresses->at(fold_index);
            local_user->addresses->erase(local_user->addresses->begin() + fold_index);
            free(current);
            strcpy(message, "Usuário desconectado com sucesso");
            return PACKET_DATA_LOGOUT_OK_T;
        }
    }
}

int follow(const char *my_username, const char *followed, char *message) {
    if (my_username == NULL) {
        strcpy(message, "Sessão de usuário inválida");
        return PACKET_DATA_FOLLOW_ERROR_T;
    } else {
        user_p user_followed = find_user(followed);
        if (user_followed == NULL) {
            strcpy(message, "Usuário para seguir não encontrado");
            return PACKET_DATA_FOLLOW_ERROR_T;
        } else {
            for (int i = 0; i < user_followed->follows->size(); i++) {
                if (user_followed->follows->at(i).compare(my_username) == 0) {
                    strcpy(message, "Voçê já segue esse usuário!");
                    return PACKET_DATA_FOLLOW_ERROR_T;
                }
            }
            user_followed->follows->push_back(my_username);
            write_users(server_users, server_users_size);
            strcpy(message, "Usuário seguido com sucesso");
            return PACKET_DATA_FOLLOW_OK_T;
        }
    }
}

int unfollow(const char *my_username, const char *followed, char *message) {
    if (my_username == NULL) {
        strcpy(message, "Sessão de usuário inválida");
        return PACKET_DATA_UNFOLLOW_ERROR_T;
    } else {
        user_p user_followed = find_user(followed);
        if (user_followed == NULL) {
            strcpy(message, "Usuário para deixar de seguir não encontrado");
            return PACKET_DATA_UNFOLLOW_ERROR_T;
        } else {
            for (int i = 0; i < user_followed->follows->size(); i++) {
                if (user_followed->follows->at(i).compare(my_username) == 0) {
                    user_followed->follows->erase(user_followed->follows->begin() + i);
                    write_users(server_users, server_users_size);
                    strcpy(message, "Usuário não será mais seguido por você!");
                    return PACKET_DATA_UNFOLLOW_OK_T;
                }
            }
            strcpy(message, "Você já não segue esse usuário");
            return PACKET_DATA_UNFOLLOW_ERROR_T;
        }
    }
}