#ifndef SRV_NOTIF_MNG_H
#define SRV_NOTIF_MNG_H

#include "../types.hpp"

typedef bool (*send_notif_callback_t)(uint16_t type, notification *payload, const user_address *cliaddr);

pthread_t start_server_notif_mng(send_notif_callback_t cb);

void producer_new_notification(const user_p author, const char *message);

void *consumer_notification(void *arg);

#endif