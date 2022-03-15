#ifndef SRV_NOTIF_MNG_H
#define SRV_NOTIF_MNG_H

#include "../types.hpp"

pthread_t start_server_notif_mng(send_notif_callback_t cb);

void producer_new_notification(const user_p author, const char *message);

void *consumer_notification(void *arg);

bool send_to_all_addresses(const user_p user_follow, char *message);

#endif