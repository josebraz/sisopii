
#ifndef CLIENT_COM_MANAGER_H
#define CLIENT_COM_MANAGER_H

#include <stdint.h>

#include "../types.hpp"

void *client_message_receiver(void *arg);

packet *client_send_message(uint16_t type, char *message);

int send_echo_msg(char *text);

int send_login_msg(char *user);

void send_logout_msg(char *user);

void send_alive_msg(char *user);

void send_follow_msg(char *user);

void send_unfollow_msg(char *user);

void send_notify_msg(char *message);

pthread_t start_client(char *server_addr, int port);

#endif
