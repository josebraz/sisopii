#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include "communication_manager.hpp"
#include "../constants.h"
#include "../logs.hpp"
#include "../types.hpp"
#include "../communication_utils.hpp"

struct sockaddr_in servaddr;
int sockfd;
uint16_t next_seq_n = 0;

void start_client(char *server_addr, int port, char *my_user)
{
    struct hostent *server;

    server = gethostbyname(server_addr);
	if (server == NULL) 
    {
        perror("ERROR, no such host\n");
        exit(EXIT_FAILURE);
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr = *((struct in_addr *)server->h_addr);

    pthread_t receiver_thread;
    pthread_create(&receiver_thread, NULL, message_receiver, NULL);
}

void *message_receiver(void *arg)
{
    char buffer[PAYLOAD_MAX_SIZE];
    packet *message;
    int n, len;

    while (true)
    {
        n = recvfrom(
            sockfd,
            (void *)buffer,
            PAYLOAD_MAX_SIZE,
            MSG_WAITALL,
            (struct sockaddr *)&servaddr,
            (socklen_t *) &len);
        
        unmarshalling_packet(&message, buffer);
        
        printf("Server: "); 
        print_packet(message);

        free(message);
    }

    close(sockfd);
}

void send_message(uint16_t type, char *payload)
{
    size_t payload_size = strlen(payload);
    char *buffer;

    if (payload_size > PAYLOAD_MAX_SIZE)
    {
        log_error("A mensagem para o servidor é muito grande");
    }
    else
    {
        packet message = {
            type, 
            next_seq_n++, 
            payload_size, 
            time(NULL),
            payload
        };

        printf("Client: "); 
        print_packet(&message);

        size_t message_size = marshalling_packet(&message, &buffer);

        ssize_t size = sendto(
            sockfd,
            (const void *) &buffer,
            message_size,
            MSG_CONFIRM,
            (const struct sockaddr *)&servaddr,
            sizeof(servaddr));
        
        free(buffer);

        if (size == -1)
        {
            log_error("Mensagem não enviada");
        }
    }
}

void send_login_msg(char *user) {
    send_message(PACKET_CMD_LOGIN_T, user);
}

void send_logout_msg(char *user) {
    send_message(PACKET_CMD_LOGOUT_T, user);
}

void send_alive_msg(char *user) {
    send_message(PACKET_CMD_ALIVE_T, user);
}

void send_follow_msg(char *user) {
    send_message(PACKET_CMD_FOLLOW_T, user);
}

void send_unfollow_msg(char *user) {
    send_message(PACKET_CMD_UNFOLLOW_T, user);
}

void send_notify_msg(char *message) {
    send_message(PACKET_CMD_NOTIFY_T, message);
}
