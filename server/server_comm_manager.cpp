#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <iostream>
#include <vector>
#include <string>

#include "../constants.h"
#include "../types.hpp"
#include "../logs.hpp"
#include "../communication_utils.hpp"
#include "persistence.hpp"
#include "server_comm_manager.hpp"
#include "server_notif_manager.hpp"
#include "session_manager.hpp"

using namespace std;

int server_sockfd;
uint16_t server_next_seq_n = 1;

pthread_t start_server(int port)
{
    struct sockaddr_in servaddr;

    if ((server_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);
    bzero(&(servaddr.sin_zero), 8);  

    if (bind(
            server_sockfd,
            (const struct sockaddr *)&servaddr,
            sizeof(servaddr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    pthread_t receiver_thread;
    pthread_create(&receiver_thread, NULL, server_message_receiver, NULL);
    return receiver_thread;
}

void dispatch_message(packet *message, user_address *cliaddr) {
    static char result_message[RESULT_MESSAGE_MAX_SIZE];
    int result_code;

    user_p user_requester;
    switch (message->type) {
        case PACKET_CMD_ECHO_T:
            server_send_message(PACKET_DATA_ECHO_RESP_T, message->payload, cliaddr, message->seqn);
            break;
        case PACKET_CMD_LOGIN_T:
            result_code = login(message->payload, cliaddr, result_message);
            server_send_message(result_code, result_message, cliaddr, message->seqn);
            break;
        case PACKET_CMD_LOGOUT_T:
            result_code = logout(message->payload, cliaddr, result_message);
            server_send_message(result_code, result_message, cliaddr, message->seqn);
            break;
        case PACKET_CMD_ALIVE_T:
            // TODO: o alive é uma mensagem que o cliente manda para informar
            // o servidor que ainda será ativo
            break;
        case PACKET_CMD_FOLLOW_T:
            user_requester = find_user_by_address(cliaddr);
            if (user_requester == NULL) {
                server_send_message(PACKET_DATA_UNAUTHENTICATED_T, (char *) "Acesso negado", cliaddr, message->seqn);
            } else {
                result_code = follow(user_requester->username.c_str(), message->payload, result_message);
                server_send_message(result_code, result_message, cliaddr, message->seqn);
            }
            break;
        case PACKET_CMD_UNFOLLOW_T:
            user_requester = find_user_by_address(cliaddr);
            if (user_requester == NULL) {
                server_send_message(PACKET_DATA_UNAUTHENTICATED_T, (char *) "Acesso negado", cliaddr, message->seqn);
            } else {
                result_code = unfollow(user_requester->username.c_str(), message->payload, result_message);
                server_send_message(result_code, result_message, cliaddr, message->seqn);
            }
            break;
        case PACKET_CMD_NEW_NOTIFY_T:
            user_requester = find_user_by_address(cliaddr);
            if (user_requester == NULL) {
                server_send_message(PACKET_DATA_UNAUTHENTICATED_T, (char *) "Acesso negado", cliaddr, message->seqn);
            } else {
                producer_new_notification(user_requester, message->payload);
            }
            break;
        // TODO: implementar a lógica do request faltantes
    }
}

void *server_message_receiver(void *arg) {
    user_address cliaddr;
    char buffer[PAYLOAD_MAX_SIZE];
    char result_message[RESULT_MESSAGE_MAX_SIZE];
    int n;
    packet *message;

    socklen_t len = sizeof(user_address);

    while (true)
    {
        n = recvfrom(
            server_sockfd,
            (void *) buffer,
            PAYLOAD_MAX_SIZE,
            0,
            (struct sockaddr *) &cliaddr,
            &len);
        
        if (n < 0) {
            perror("Server receiver error");
            continue;
        }

        unmarshalling_packet(&message, buffer);

        if (message == NULL) {
            perror("Server unmarshalling packet error");
            continue;
        }

        printf("Server received: ");
        print_packet(message);

        dispatch_message(message, &cliaddr);

        free_packet(message);
    }
}

bool server_send_message(uint16_t type, char *payload, const user_address *cliaddr, const uint16_t seqn) {
    size_t payload_size = strlen(payload);
    char *buffer;

    if (payload_size > PAYLOAD_MAX_SIZE)
    {
        log_error("A mensagem para o servidor é muito grande");
        return false;
    }
    else
    {
        packet message = {
            type, 
            seqn, 
            (uint16_t) payload_size, 
            (uint32_t) time(NULL),
            payload
        };

        printf("Server send: "); 
        print_packet(&message);

        size_t message_size = marshalling_packet(&message, &buffer);

        ssize_t size = sendto(
            server_sockfd,
            (const void *) buffer,
            message_size,
            0,
            (const sockaddr*) cliaddr,
            sizeof(user_address));

        free(buffer);

        if (size == -1)
        {
            log_error("Mensagem não enviada");
            return false;
        }
    }
    return true;
}

bool server_send_notif(uint16_t type, notification *payload, const user_address *cliaddr) {
    return server_send_message(type, payload->message, cliaddr, server_next_seq_n++);
}
