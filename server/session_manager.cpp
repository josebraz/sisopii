#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "session_manager.hpp"
#include "persistence.hpp"
#include "../communication_utils.hpp"
#include "../types.hpp"

void init_session_manager() {
    server_users_size = read_users(server_users);
}

void finalize_session_manager() {
    for (int i = 0; i < server_users_size; i++) {
        free_user(server_users[i]);
    }
    server_users_size = 0;
}

user_p find_user(const char *username) {
    for (int i = 0; i < server_users_size; i++) {
        if (strcmp(server_users[i]->username.c_str(), username) == 0) {
            return server_users[i];
        }
    }
    return NULL;
}

int index_of_address(const user_p local_user, const user_address *address) {
    int fold_index = -1;
    for (int i = 0; i < local_user->addresses->size(); i++) {
        session_addr *current = local_user->addresses->at(i);
        if (current->client_address.sin_addr.s_addr == address->sin_addr.s_addr && 
                current->client_address.sin_port == address->sin_port) {
            fold_index = i;
            break;
        }
    }
    return fold_index;
}

int login(const char *username, const sockaddr_in *address, char *message, session_addr **session, user_p *user_logged) {
    user_p local_user = find_user(username);
    if (local_user == NULL) { 
        if (server_users_size < USER_SESSION_MAX_SIZE) {
            local_user = new user();
            local_user->username = strdup(username);
            local_user->follows = new vector<string>();
            local_user->addresses = new vector<session_addr*>();
            session_addr* new_session = new session_addr();
            memcpy(&(new_session->client_address), address, sizeof(sockaddr_in));
            new_session->seqn = (uint16_t) 0;
            new_session->server_sockfd = -1;

            local_user->addresses->push_back(new_session);
            local_user->pending_msg = new vector<uint32_t>();
            pthread_mutex_init(&(local_user->mutex_addr), NULL);
            pthread_mutex_init(&(local_user->mutex_follows), NULL);

            server_users[server_users_size++] = local_user;
            write_users(server_users, server_users_size);

            *session = new_session;
            *user_logged = local_user;
            strcpy(message, "Usuário criado com sucesso!");
            return PACKET_DATA_LOGIN_OK_T;
        } else {
            *session = NULL;
            *user_logged = NULL;
            strcpy(message, "Máximo número de usuários no servidor");
            return PACKET_DATA_LOGIN_ERROR_T;
        }
    } else {
        if (local_user->addresses->size() >= USER_MAX_SESSIONS) {
            *session = NULL;
            *user_logged = NULL;
            strcpy(message, "Máximo número de sessões do mesmo usuário");
            return PACKET_DATA_LOGIN_ERROR_T;
        } else {
            pthread_mutex_lock(&(local_user->mutex_addr));
            int fold_index = index_of_address(local_user, address);
            if (fold_index != -1) {
                pthread_mutex_unlock(&(local_user->mutex_addr));
                *session = NULL;
                *user_logged = NULL;
                strcpy(message, "Usuário já está logado nesse cliente");
                return PACKET_DATA_LOGIN_ERROR_T;
            } else {
                session_addr* new_session = new session_addr();
                memcpy(&(new_session->client_address), address, sizeof(sockaddr_in));
                new_session->seqn = (uint16_t) 0;
                new_session->server_sockfd = -1;
                local_user->addresses->push_back(new_session);
                pthread_mutex_unlock(&(local_user->mutex_addr));

                *session = new_session;
                *user_logged = local_user;
                strcpy(message, "Login OK!");
                return PACKET_DATA_LOGIN_OK_T;
            }
        }
    }
}

int logout(const user_p local_user, const user_address *address, char *message) {
    if (local_user == NULL) {
        strcpy(message, "Usuário não encontrado");
        return PACKET_DATA_LOGOUT_ERROR_T;
    } else {
        pthread_mutex_lock(&(local_user->mutex_addr));
        int fold_index = index_of_address(local_user, address);
        if (fold_index == -1) {
            pthread_mutex_unlock(&(local_user->mutex_addr));
            strcpy(message, "Usuário não encontrado");
            return PACKET_DATA_LOGOUT_ERROR_T;
        } else {
            session_addr *current = local_user->addresses->at(fold_index);
            local_user->addresses->erase(local_user->addresses->begin() + fold_index);
            pthread_mutex_unlock(&(local_user->mutex_addr));

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
            pthread_mutex_lock(&(user_followed->mutex_follows));
            for (int i = 0; i < user_followed->follows->size(); i++) {
                if (user_followed->follows->at(i).compare(my_username) == 0) {
                    pthread_mutex_unlock(&(user_followed->mutex_follows));

                    strcpy(message, "Você já segue esse usuário!");
                    return PACKET_DATA_FOLLOW_ERROR_T;
                }
            }
            user_followed->follows->push_back(my_username);
            pthread_mutex_unlock(&(user_followed->mutex_follows));

            write_users(server_users, server_users_size);
            strcpy(message, "Usuário seguido com sucesso!");
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
            pthread_mutex_lock(&(user_followed->mutex_follows));
            for (int i = 0; i < user_followed->follows->size(); i++) {
                if (user_followed->follows->at(i).compare(my_username) == 0) {
                    user_followed->follows->erase(user_followed->follows->begin() + i);
                    pthread_mutex_unlock(&(user_followed->mutex_follows));

                    write_users(server_users, server_users_size);
                    strcpy(message, "Usuário não será mais seguido por você!");
                    return PACKET_DATA_UNFOLLOW_OK_T;
                }
            }
            pthread_mutex_unlock(&(user_followed->mutex_follows));

            strcpy(message, "Você já não segue esse usuário");
            return PACKET_DATA_UNFOLLOW_ERROR_T;
        }
    }
}