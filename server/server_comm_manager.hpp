#ifndef SERVER_COM_MANAGER_H
#define SERVER_COM_MANAGER_H

pthread_t start_server(int port);

void server_send_message(uint16_t type, char *payload, const struct sockaddr *cliaddr);

void *server_message_receiver(void *arg);

#endif