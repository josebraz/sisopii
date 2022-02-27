
#ifndef COM_MANAGER_H
#define COM_MANAGER_H

void *message_receiver(void *arg);

void send_message(char *message);

void start_client();

#endif
