
#ifndef COM_MANAGER_H
#define COM_MANAGER_H

#include <stdint.h>

void *message_receiver(void *arg);

void send_message(uint16_t type, char *message);

void send_login_msg(char *user);

void send_logout_msg(char *user);

void send_alive_msg(char *user);

void send_follow_msg(char *user);

void send_unfollow_msg(char *user);

void send_notify_msg(char *message);

void start_client(char *server_addr, int port, char *my_user);

#endif
