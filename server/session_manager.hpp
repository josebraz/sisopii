#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#define USER_SESSION_MAX_SIZE 1000

#include <stdint.h>

#include "../types.hpp"

static int server_users_size = 0;
static user_p server_users[USER_SESSION_MAX_SIZE];

/**
 * Inicia o gerenciador de sessão
 */
void init_session_manager();

/**
 * Finaliza o gerenciador de sessão
 */
void finalize_session_manager();

/**
 * Encontra um usuário pelo username dele 
 * 
 * @param username 
 * @return user_p 
 */
user_p find_user(const char *username);

/**
 * Encontra o indice do endereço do usuário
 * 
 * @param local_user 
 * @param address 
 * @return int 
 */
int index_of_address(const user_p local_user, const user_address *address);

/**
 * Inicia uma sessão do usuário com o novo address
 * 
 * @param username do usuário que setá logando
 * @param address novo endereço da sessão
 * @param message mensagem a ser enviado para o usuário como resposta
 * @return int Tipo da mensagem que vai retornar como resposta
 */
int login(const char *username, const user_address *address, char *message, session_addr **session, user_p *user_logged);

/**
 * Finaliza a sessão do usuário no device address
 * 
 * @param username do usuário que setá deslogando
 * @param address endereço da sessão a ser eliminado
 * @param message mensagem a ser enviado para o usuário como resposta
 * @return int Tipo da mensagem que vai retornar como resposta
 */
int logout(const user_p local_user, const user_address *address, char *message);

/**
 * my_username começa a seguir o followed
 * 
 * @param my_username usuário logado
 * @param followed usuário que vai começar a seguir
 * @param message mensagem a ser enviado para o usuário como resposta
 * @return int Tipo da mensagem que vai retornar como resposta 
 */
int follow(const char *my_username, const char *followed, char *message);

/**
 * my_username deixa de seguir o followed
 * 
 * @param my_username usuário logado
 * @param followed usuário que vai deixar de seguir
 * @param message mensagem a ser enviado para o usuário como resposta
 * @return int Tipo da mensagem que vai retornar como resposta 
 */
int unfollow(const char *my_username, const char *followed, char *message);


#endif