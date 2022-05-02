#ifndef COMM_UTILS_H
#define COMM_UTILS_H

#include "types.hpp"

#define NO_WAIT_SEQN 65535 // número de sequencia pra representar que nao estamos esperando nada

size_t marshalling_packet(const packet *message, char **buffer);

void unmarshalling_packet(packet **message, const char *buffer);

size_t marshalling_notification(const notification *message, char **buffer);

void unmarshalling_notification(notification **message, const char *buffer);

size_t marshalling_new_address(int address_length, const char *address, int new_port, char **buffer);

void unmarshalling_new_address(char *address, int *new_port, const char *buffer);

size_t marshalling_resistence_msg(const resistence_msg *message, char **buffer);

void unmarshalling_resistence_msg(resistence_msg **message, const char *buffer);

packet *wait_response(
    uint16_t seqn, 
    pthread_mutex_t *mutex, 
    pthread_cond_t *receiver_cond, 
    pthread_cond_t *wait_cond, 
    uint16_t *wait_seqn, 
    packet **last_message_received
);

void signal_response(
    packet *received_message, 
    pthread_mutex_t *mutex, 
    pthread_cond_t *receiver_cond, 
    pthread_cond_t *wait_cond, 
    uint16_t *wait_seqn, 
    packet **last_message_received
);

void print_resistence(const resistence_msg *message);

void print_packet(const packet *message);

void print_notification(const notification *message);

void copy_packet(packet **dest, const packet *src);

void free_packet(packet *p);

void free_notification(notification *notif);

void free_user(user_p u);

bool is_response(uint32_t type);

#endif