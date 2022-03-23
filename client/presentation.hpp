
#ifndef PRESENTATION_H
#define PRESENTATION_H

#include "../types.hpp"

/**
 * Inicia a interface com o usuário e fica esperando
 * as entradas do usuário
 * 
 * @param my_user meu usuário da sessão
 */
void start_presentation(char *my_user);

/**
 * Apresenta uma notificação recebida do server
 * 
 * @param notif notificação recebida
 */
void on_new_notification(notification *notif);

#endif