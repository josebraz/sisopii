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
#include <time.h>

#include "client_comm_manager.hpp"
#include "../constants.h"
#include "../logs.hpp"
#include "../types.hpp"
#include "../communication_utils.hpp"

struct sockaddr_in servaddr;
int client_sockfd;

uint16_t client_next_seq_n = 1, wait_seqn = 0;
packet *last_message_received = NULL;

timespec cond_timeout;
pthread_cond_t cond_wait_response;
pthread_mutex_t mutex_response;

void init_sync_comm() {
    int ret;
    ret = pthread_cond_init(&cond_wait_response, NULL);
    if (ret != 0) {
        perror("cond init failed");
        exit(EXIT_FAILURE);
    }

    ret = pthread_mutex_init(&mutex_response, NULL);
    if (ret != 0) {
        perror("mutex init failed");
        exit(EXIT_FAILURE);
    }
}

packet *wait_response(uint16_t seqn) {
    packet *response = NULL;
    pthread_mutex_lock(&mutex_response);
    cond_timeout.tv_sec = time(NULL) + 2;
    cond_timeout.tv_nsec = 0;
    wait_seqn = seqn;
    while (last_message_received == NULL || wait_seqn > last_message_received->seqn) {
        int ret = pthread_cond_timedwait(&cond_wait_response, &mutex_response, &cond_timeout);
        if (ret == 0) {
            response = last_message_received;
        } else {
            response = NULL;
            break;
        }
    }
    pthread_mutex_unlock(&mutex_response);
    return response;
}

void signal_response(packet *received_message) {
    pthread_mutex_lock(&mutex_response);
    if (last_message_received != NULL) {
        free_packet(last_message_received);
    }
    copy_packet(&last_message_received, received_message);
    if (last_message_received->seqn >= wait_seqn) {
        pthread_cond_signal(&cond_wait_response);
    }
    pthread_mutex_unlock(&mutex_response);
}

pthread_t start_client(char *server_addr, int port)
{
    struct hostent *server;

    server = gethostbyname(server_addr);
	if (server == NULL) 
    {
        perror("ERROR, no such host\n");
        exit(EXIT_FAILURE);
    }

    if ((client_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    init_sync_comm();
 
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(servaddr.sin_zero), 8);  

    pthread_t receiver_thread;
    pthread_create(&receiver_thread, NULL, client_message_receiver, NULL);
    return receiver_thread;
}

void *client_message_receiver(void *arg)
{
    char buffer[PAYLOAD_MAX_SIZE];
    packet *message;
    int n, len;

    while (true)
    {
        n = recvfrom(
            client_sockfd,
            (void *)buffer,
            PAYLOAD_MAX_SIZE,
            MSG_WAITALL,
            (struct sockaddr *)&servaddr,
            (socklen_t *) &len);

        if (n < 0) {
            perror("Client receiver error");
            continue;
        }
        
        unmarshalling_packet(&message, buffer);

        if (message == NULL) {
            perror("Client unmarshalling packet error");
            continue;
        }

        // printf("Client received: "); 
        // print_packet(message);

        signal_response(message);

        free_packet(message);
    }

    close(client_sockfd);
}

packet *client_send_message(uint16_t type, char *payload)
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
            client_next_seq_n++, 
            (uint16_t) payload_size, 
            (uint32_t) time(NULL),
            payload
        };

        // printf("Client send: "); 
        // print_packet(&message);

        size_t message_size = marshalling_packet(&message, &buffer);

        ssize_t size = sendto(
            client_sockfd,
            (const void *) buffer,
            message_size,
            0,
            (const struct sockaddr *)&servaddr,
            sizeof(struct sockaddr_in));
        
        free(buffer);

        if (size == -1)
        {
            log_error("Mensagem não enviada");
        } else {
            return wait_response(message.seqn);
        }
    }
    return NULL;
}

int send_echo_msg(char *text) {
    packet *message = client_send_message(PACKET_CMD_ECHO_T, text);
    if (message != NULL && message->type == PACKET_CMD_ECHO_T) {
        return 1;
    } else {
        return 0;
    }
}

int send_login_msg(char *username) {
    packet *message = client_send_message(PACKET_CMD_LOGIN_T, username);
    if (message != NULL && message->type == PACKET_DATA_LOGIN_OK_T) {
        return 1;
    } else {
        return 0;
    }
}

void send_logout_msg(char *user) {
    client_send_message(PACKET_CMD_LOGOUT_T, user);
}

void send_alive_msg(char *user) {
    client_send_message(PACKET_CMD_ALIVE_T, user);
}

void send_follow_msg(char *user) {
    client_send_message(PACKET_CMD_FOLLOW_T, user);
}

void send_unfollow_msg(char *user) {
    client_send_message(PACKET_CMD_UNFOLLOW_T, user);
}

void send_notify_msg(char *message) {
    client_send_message(PACKET_CMD_NOTIFY_T, message);
}
