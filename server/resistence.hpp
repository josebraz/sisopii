#ifndef RESISTENCE_H
#define RESISTENCE_H

#include <netinet/in.h>

#include "../types.hpp"

#define ANY_DEST -1

void init_election();

void start_resistence_server();

int send_broadcast(resistence_msg *resistence, uint16_t type, uint16_t seqn);

resistence_msg *send_broadcast_and_wait(resistence_msg *resistence, uint16_t type, uint16_t seqn);

resistence_msg *poke_coordinator(int coord);

#endif