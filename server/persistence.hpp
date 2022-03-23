#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "../types.hpp"

/**
 * Le os usuários do banco de dados da aplicação
 * 
 * @param users É a lista que será atribuída dos clientes lidos
 * @return int Quantidade de usuários lidos
 */
int read_users(user_p *users);

/**
 * Grava os usuários no banco, substituindo todos por esses novos
 * 
 * @param users Usuários que serão salvos
 * @param total Quantidade de elementos que a lista do users tem
 */
void write_users(const user_p users[], const int total);

/**
 * Limpa todos os usuários do banco de dados
 */
void clear_all_users();

#endif