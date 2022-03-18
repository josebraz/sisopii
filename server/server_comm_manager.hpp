#ifndef SERVER_COM_MANAGER_H
#define SERVER_COM_MANAGER_H

#include <stdint.h>

#include "../types.hpp"

pthread_t start_server(int port);

bool server_send_message(uint16_t type, size_t payload_size, char *payload, const user_address *cliaddr, const uint16_t seqn);

bool server_send_notif(uint16_t type, notification *payload, const user_address *cliaddr, const uint16_t seqn);

void *server_message_receiver(void *arg);

#endif