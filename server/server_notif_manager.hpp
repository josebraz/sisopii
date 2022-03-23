#ifndef SRV_NOTIF_MNG_H
#define SRV_NOTIF_MNG_H

#include "../types.hpp"

/**
 * Inicia o servidor de notificação
 * 
 * @param cb callback da função de enviar mensagem para o cliente
 * @return pthread_t nova thread do consumidor
 */
pthread_t start_server_notif_mng(send_notif_callback_t cb);

/**
 * Produz uma nova notificação com a mensagem e atualiza
 * os seguidores desse autor que precisam receber essa mensagem
 * 
 * @param author autor da mensagem
 * @param message conteúdo da mensagem
 */
void producer_new_notification(const user_p author, const char *message);

/**
 * Envia para todos os endereços do usuário user_follow a mensagem message
 * 
 * @param type Tipo da mensage
 * @param user_follow usuário que precisa receber essa mensagem
 * @param message 
 * @return true 
 * @return false 
 */
bool send_to_all_addresses(const uint16_t type, const user_p user_follow, notification *message);

#endif