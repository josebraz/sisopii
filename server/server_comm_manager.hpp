#ifndef SERVER_COM_MANAGER_H
#define SERVER_COM_MANAGER_H

#include <stdint.h>

#include "../types.hpp"

/**
 * Inicia o servidor UDP e cria uma nova thread
 * para não bloquear a thread principal
 * 
 * @param port usada para receber os dados
 * @return pthread_t nova thread criada para escutar o socket
 */
pthread_t start_server(int port);

/**
 * Envia uma mensagem genérica para o cliaddr.
 * Envelopa o payload em um packet para ser enviado
 * 
 * @param type Tipo da mensagem, ver types.hpp
 * @param payload_size Tamanho do payload da mensagem
 * @param payload Array de bytes a serem enviados
 * @param cliaddr Endereço do cliente destinatário
 * @param seqn Número de sequência da mensagem
 * @return true Caso consiga enviar com sucesso
 * @return false Deu algum erro no envio
 */
bool server_send_message(uint16_t type, size_t payload_size, char *payload, const user_address *cliaddr, const uint16_t seqn);

/**
 * Envia uma mensagem de notificação para o cliaddr.
 * Envelopa o payload em um packet para ser enviado
 * 
 * @param type Tipo da mensagem, ver types.hpp
 * @param payload Array de bytes a serem enviados
 * @param cliaddr Endereço do cliente destinatário
 * @param seqn Número de sequência da mensagem
 * @return true Caso consiga enviar com sucesso
 * @return false Deu algum erro no envio
 */
bool server_send_notif(uint16_t type, notification *payload, const user_address *cliaddr, const uint16_t seqn);

#endif