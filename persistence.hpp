#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "types.hpp"

int read_users(user *users[]);

void write_users(const user users[], const int total);

#endif