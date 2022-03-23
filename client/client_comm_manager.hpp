
#ifndef CLIENT_COM_MANAGER_H
#define CLIENT_COM_MANAGER_H

#include <stdint.h>

#include "../types.hpp"

/**
 * Envia uma mensagem genérica para o servidor.
 * É uma função bloqueante que espera a resposta do servidor
 * 
 * @param type Tipo da mensagem
 * @param message Conteúdo da mensagem
 * @return packet* Pacote recebido como resposta do servidor
 */
packet *client_send_message(uint16_t type, char *message);

/**
 * Envia um echo para o servidor repetir a mensagem
 * É útil para os testes automatizados
 * 
 * @param text Texto do echo
 * @return 1 se a resposta foi ok 
 * @return 0 se a resposta foi um error ou não teve resposta
 */
int send_echo_msg(char *text);

/**
 * Envia uma mensagem de login
 * 
 * @param text usuário que está fazendo o login
 * @param result resposta do servidor para a mensagem enviada
 * @return 1 se a resposta foi ok 
 * @return 0 se a resposta foi um error ou não teve resposta
 */
int send_login_msg(char *user, char *result);

/**
 * Envia uma mensagem de logout
 * 
 * @param text usuário que está fazendo o logout
 * @param result resposta do servidor para a mensagem enviada
 * @return 1 se a resposta foi ok 
 * @return 0 se a resposta foi um error ou não teve resposta
 */
int send_logout_msg(char *user, char *result);

/**
 * Envia uma mensagem para o usuário logado seguir o user
 * 
 * @param user usuário a ser seguido
 * @param result resposta do servidor para a mensagem enviada
 * @return 1 se a resposta foi ok 
 * @return 0 se a resposta foi um error ou não teve resposta
 */
int send_follow_msg(char *user, char *result);

/**
 * Envia uma mensagem para o usuário logado deixar de seguir o user
 * 
 * @param user usuário a ser deixado de seguir
 * @param result resposta do servidor para a mensagem enviada
 * @return 1 se a resposta foi ok 
 * @return 0 se a resposta foi um error ou não teve resposta
 */
int send_unfollow_msg(char *user, char *result);

/**
 * Envia uma nova publicação do usuário logado para todos os seus seguidores
 * 
 * @param message nova mensagem que o usuário mandou
 * @param result resposta do servidor para a mensagem enviada
 * @return 1 se a resposta foi ok 
 * @return 0 se a resposta foi um error ou não teve resposta
 */
int send_notify_msg(char *message, char *result);

/**
 * Começa a ouvir o cliente em busca de novas notificações
 * 
 * @param server_addr endereço do servidor
 * @param port porta do servidor
 * @return pthread_t nova thread que está escutando o socket
 */
pthread_t start_client(char *server_addr, int port);

#endif
