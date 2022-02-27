#ifndef COM_MANAGER_H
#define COM_MANAGER_H

void start_server();

void send_message(char *message, const struct sockaddr *cliaddr);

void *message_receiver(void *arg);

#endif