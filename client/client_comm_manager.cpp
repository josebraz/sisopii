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
#include "presentation.hpp"
#include "../constants.h"
#include "../logs.hpp"
#include "../types.hpp"
#include "../communication_utils.hpp"

struct sockaddr_in servaddr;
int client_sockfd;

uint16_t client_next_seq_n = 1, wait_seqn = NO_WAIT_SEQN;
packet *last_message_received = NULL;

pthread_t receiver_thread;
pthread_cond_t receiver_cond, wait_cond;
pthread_mutex_t mutex_response;

void *client_message_receiver(void *arg);

// inicia as variáveis de condição e os mutex
void init_sync_comm() {
    int ret;
    ret = pthread_cond_init(&receiver_cond, NULL);
    if (ret != 0) {
        perror("cond init failed");
        exit(EXIT_FAILURE);
    }

    ret = pthread_cond_init(&wait_cond, NULL);
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

pthread_t start_client(char *server_addr, int port)
{
    struct hostent *server = gethostbyname(server_addr);
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

    pthread_create(&receiver_thread, NULL, client_message_receiver, NULL);
    return receiver_thread;
}

void change_server_address(char *new_address, int new_port) {
    struct hostent *server = gethostbyname(new_address);
    if (server == NULL) 
    {
        perror("ERROR, no such host\n");
        exit(EXIT_FAILURE);
    }
    
    servaddr.sin_port = htons(new_port);
    // servaddr.sin_addr = *((struct in_addr *)server->h_addr);
}

void *client_message_receiver(void *arg)
{
    char buffer[PAYLOAD_MAX_SIZE];
    packet *message;
    notification* notif;
    int n, len;

    struct sockaddr_in clientaddr;
    if (getsockname(client_sockfd, (struct sockaddr *) &clientaddr, (socklen_t *) &len) == -1) {
        perror("getsockname");
        exit(EXIT_FAILURE);
    }

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

        if (is_response(message->type)) {
            signal_response(message, &mutex_response, &receiver_cond, &wait_cond, &wait_seqn, &last_message_received);
        } else { // é uma notificação enviada pelo server
            switch (message->type)
            {
            case PACKET_CMD_NOTIFY_T:
                unmarshalling_notification(&notif, message->payload);
                if (notif != NULL) {
                    on_new_notification(notif);
                }
                free_notification(notif);
                break;
            case PACKET_CMD_NEW_SERVER_T:
                char new_address[200];
                int new_port;
                
                unmarshalling_new_address(new_address, &new_port, message->payload);
                change_server_address(new_address, new_port);
                break;
            case PACKET_CMD_END_SERVER:
                printf("Servidor finalizado...\n");
                exit(1);
                break;
            default:
                break;
            }
        }

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

        size_t message_size = marshalling_packet(&message, &buffer);

        ssize_t size = sendto(
            client_sockfd,
            (const void *) buffer,
            message_size,
            0,
            (const struct sockaddr *)&servaddr,
            sizeof(struct sockaddr_in));

        if (size == -1)
        {
            log_error("Mensagem não enviada");
        } else {
            free(buffer);
            return wait_response(message.seqn, &mutex_response, &receiver_cond, &wait_cond, &wait_seqn, &last_message_received);
        }
    }
    return NULL;
}

int send_echo_msg(char *text) {
    packet *message = client_send_message(PACKET_CMD_ECHO_T, text);
    if (message != NULL && message->type == PACKET_DATA_ECHO_RESP_T) {
        return 1;
    } else {
        return 0;
    }
}

int send_login_msg(char *username, char *result) {
    packet *message = client_send_message(PACKET_CMD_LOGIN_T, username);
    if (message != NULL) {
        strcpy(result, message->payload);
    } else {
        strcpy(result, (char *) "Servidor indisponível");
    }
    if (message != NULL && message->type == PACKET_DATA_LOGIN_OK_T) {
        return 1;
    } else {
        return 0;
    }
}

int send_logout_msg(char *user, char *result) {
    packet *message = client_send_message(PACKET_CMD_LOGOUT_T, user);
    if (message != NULL) {
        strcpy(result, message->payload);
    } else {
        strcpy(result, (char *) "Servidor indisponível");
    }
    if (message != NULL && message->type == PACKET_DATA_LOGOUT_OK_T) {
        return 1;
    } else {
        return 0;
    }
}

int send_follow_msg(char *user, char *result) {
    packet *message = client_send_message(PACKET_CMD_FOLLOW_T, user);
    if (message != NULL) {
        strcpy(result, message->payload);
    } else {
        strcpy(result, (char *) "Servidor indisponível");
    }
    if (message != NULL && message->type == PACKET_DATA_FOLLOW_OK_T) {
        return 1;
    } else {
        return 0;
    }
}

int send_unfollow_msg(char *user, char *result) {
    packet *message = client_send_message(PACKET_CMD_UNFOLLOW_T, user);
    if (message != NULL) {
        strcpy(result, message->payload);
    } else {
        strcpy(result, (char *) "Servidor indisponível");
    }
    if (message != NULL && message->type == PACKET_DATA_UNFOLLOW_OK_T) {
        return 1;
    } else {
        return 0;
    }
}

int send_notify_msg(char *message, char *result) {
    packet *pack = client_send_message(PACKET_CMD_NEW_NOTIFY_T, message);
    if (pack != NULL) {
        strcpy(result, pack->payload);
    } else {
        strcpy(result, (char *) "Servidor indisponível");
    }
    if (pack != NULL && pack->type == PACKET_DATA_NOTIFICATION_T) {
        return 1;
    } else {
        return 0;
    }
}


