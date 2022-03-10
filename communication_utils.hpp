#ifndef COMM_UTILS_H
#define COMM_UTILS_H

#include "types.hpp"

size_t marshalling_packet(const packet *message, char **buffer);

void unmarshalling_packet(packet **message, const char *buffer);

void print_packet(const packet *message);

void copy_packet(packet **dest, const packet *src);

void free_packet(packet *p);

#endif