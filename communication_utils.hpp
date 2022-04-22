#ifndef COMM_UTILS_H
#define COMM_UTILS_H

#include "types.hpp"

size_t marshalling_packet(const packet *message, char **buffer);

void unmarshalling_packet(packet **message, const char *buffer);

size_t marshalling_notification(const notification *message, char **buffer);

void unmarshalling_notification(notification **message, const char *buffer);

size_t marshalling_new_address(int address_length, const char *address, int new_port, char **buffer);

void unmarshalling_new_address(char *address, int *new_port, const char *buffer);

void print_packet(const packet *message);

void print_notification(const notification *message);

void copy_packet(packet **dest, const packet *src);

void free_packet(packet *p);

void free_notification(notification *notif);

void free_user(user_p u);

bool is_response(uint32_t type);

#endif