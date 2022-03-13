#ifndef COMM_UTILS_H
#define COMM_UTILS_H

#include "types.hpp"

size_t marshalling_packet(const packet *message, char **buffer);

void unmarshalling_packet(packet **message, const char *buffer);

void print_packet(const packet *message);

void print_notification(const notification *message);

void copy_packet(packet **dest, const packet *src);

void free_packet(packet *p);

void free_notification(notification *notif);

void free_user(user_p u);

#endif