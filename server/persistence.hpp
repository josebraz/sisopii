#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "../types.hpp"

int read_users(user_p *users);

void write_users(const user_p users[], const int total);

void clear_all_users();

#endif