#ifndef SRV_NOTIF_MNG_H
#define SRV_NOTIF_MNG_H

#include "../types.hpp"

#ifdef TEST
typedef void (*send_test_callback_t)(uint16_t type, notification *payload, const user_address *cliaddr);
void register_callback(send_test_callback_t cb);
#endif

pthread_t start_server_notif_mng();

void producer_new_notification(const user_p author, const char *message);

void *consumer_notification(void *arg);

#endif