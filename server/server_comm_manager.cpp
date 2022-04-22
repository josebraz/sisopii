#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include <iostream>
#include <vector>
#include <string>

#include "../constants.h"
#include "../types.hpp"
#include "../logs.hpp"
#include "../communication_utils.hpp"
#include "server_comm_manager.hpp"
#include "server_notif_manager.hpp"
#include "session_manager.hpp"

using namespace std;

typedef struct __receiver_args {
    session_addr *new_session;
    user_p user_logged;
} receiver_args;

void *server_session_receiver(void *arg);

void create_server(int port, struct sockaddr_in *servaddr, int *server_sockfd)
{
    if ((*server_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    servaddr->sin_family = AF_INET; // IPv4
    servaddr->sin_addr.s_addr = INADDR_ANY;
    servaddr->sin_port = htons(port);
    bzero(&(servaddr->sin_zero), 8);  

    if (bind(
            *server_sockfd,
            (const struct sockaddr *) servaddr,
            sizeof(sockaddr_in)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    socklen_t len = sizeof(sockaddr_in);
    if (getsockname(*server_sockfd, (struct sockaddr *) servaddr, &len) == -1) {
        perror("getsockname");
        exit(EXIT_FAILURE);
    }

    printf("create_server - port %d\n", ntohs(servaddr->sin_port));
}

void create_new_user_server(session_addr *new_session, user_p user_logged) {
    int server_sockfd;
    struct sockaddr_in servaddr;
    pthread_t receiver_thread;
    
    create_server(0, &servaddr, &server_sockfd);
    new_session->server_sockfd = server_sockfd;

    receiver_args *args = new receiver_args();
    args->new_session = new_session;
    args->user_logged = user_logged;

    pthread_create(&receiver_thread, NULL, server_session_receiver, args);
    new_session->server_thread = receiver_thread;

    // char array[] = "eth0";
    // struct ifreq ifr;
    // ifr.ifr_addr.sa_family = AF_INET;
    // strncpy(ifr.ifr_name , array , IFNAMSIZ - 1);
    // ioctl(server_sockfd, SIOCGIFADDR, &ifr);
    
    char *host_name = inet_ntoa(servaddr.sin_addr); // inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    int port = ntohs(servaddr.sin_port);
    char *buffer;
    int payload_size = marshalling_new_address(strlen(host_name), host_name, port, &buffer);

    printf("create_new_user_server host: %s port: %d\n", host_name, port);

    server_send_message(PACKET_CMD_NEW_SERVER_T, payload_size, buffer, new_session);
}

void dispatch_message(packet *message, const session_addr *address, const user_p user_requester) {
    printf("dispatch_message type: %d from: %d\n", message->type, ntohs(address->client_address.sin_port));

    static char result_message[RESULT_MESSAGE_MAX_SIZE];
    int result_code;
    switch (message->type) {
        case PACKET_CMD_ECHO_T:
            server_send_message(PACKET_DATA_ECHO_RESP_T, strlen(message->payload), message->payload, address);
            break;
        case PACKET_CMD_ALIVE_T:
            // TODO: o alive é uma mensagem que o cliente manda para informar
            // o servidor que ainda será ativo
            break;
        case PACKET_CMD_FOLLOW_T:
            if (user_requester == NULL) {
                strcpy(result_message, (char *) "Acesso negado");
                server_send_message(PACKET_DATA_UNAUTHENTICATED_T, strlen(result_message), result_message, address);
            } else {
                char *user_to_follow = message->payload;
                const char *requester_username = user_requester->username.c_str();
                result_code = follow(requester_username, user_to_follow, result_message);
                server_send_message(result_code, strlen(result_message), result_message, address);
                // if (result_code == PACKET_DATA_FOLLOW_OK_T) {
                //     char message[100] = "Novo Seguidor! ";
                //     send_to_all_addresses(PACKET_CMD_NOTIFY_T, find_user(user_to_follow), strcat(message, requester_username));
                // }
            }
            break;
        case PACKET_CMD_UNFOLLOW_T:
            if (user_requester == NULL) {
                strcpy(result_message, (char *) "Acesso negado");
                server_send_message(PACKET_DATA_UNAUTHENTICATED_T, strlen(result_message), result_message, address);
            } else {
                result_code = unfollow(user_requester->username.c_str(), message->payload, result_message);
                server_send_message(result_code, strlen(result_message), result_message, address);
            }
            break;
        case PACKET_CMD_NEW_NOTIFY_T:
            if (user_requester == NULL) {
                strcpy(result_message, (char *) "Acesso negado");
                server_send_message(PACKET_DATA_UNAUTHENTICATED_T, strlen(result_message), result_message, address);
            } else {
                producer_new_notification(user_requester, message->payload);
                strcpy(result_message, (char *) "Mensagem criada com sucesso!");
                server_send_message(PACKET_DATA_NOTIFICATION_T, strlen(result_message), result_message, address);
            }
            break;
    }
}

void start_server_entry_point(int port) {
    int server_sockfd;
    struct sockaddr_in servaddr;
    
    create_server(port, &servaddr, &server_sockfd);

    user_address cliaddr;
    char buffer[PAYLOAD_MAX_SIZE];
    char result_message[RESULT_MESSAGE_MAX_SIZE];
    int n;
    packet *message;

    socklen_t len = sizeof(user_address);

    printf(GRN "Server iniciado com sucesso!" NC " \n");

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

        session_addr address = {
            .seqn = message->seqn,
            .client_address = cliaddr, 
            .server_sockfd = server_sockfd,
            .server_thread = pthread_self()
        };

        if (message->type == PACKET_CMD_LOGIN_T) {
            session_addr *new_session = NULL;
            user_p user_logged = NULL;
            int result_code = login(message->payload, &cliaddr, result_message, &new_session, &user_logged);
            server_send_message(result_code, strlen(result_message), result_message, &address);
            if (result_code == PACKET_DATA_LOGIN_OK_T && new_session != NULL) {
                create_new_user_server(new_session, user_logged);
            }
        } else if (message->type == PACKET_CMD_ECHO_T) {
            server_send_message(PACKET_DATA_ECHO_RESP_T, strlen(message->payload), message->payload, &address);
        } else {
            perror("No socket padrão apenas mensagens de login e echo são permitidas");
        }

        free_packet(message);
    }
}

void *server_session_receiver(void *arg) {
    receiver_args *recv_args = (receiver_args *) arg;
    session_addr *session = recv_args->new_session;
    user_p user_logged = recv_args->user_logged;

    user_address cliaddr;
    char buffer[PAYLOAD_MAX_SIZE];
    char result_message[RESULT_MESSAGE_MAX_SIZE];
    packet *message = NULL;
    socklen_t len = sizeof(user_address);

    printf("Start session receiver to client port: %d\n", ntohs(session->client_address.sin_port));

    while (true)
    {
        int n = recvfrom(
            session->server_sockfd,
            (void *) buffer,
            PAYLOAD_MAX_SIZE,
            0,
            (struct sockaddr *) &cliaddr,
            &len);

        printf("session receiver from %d\n", ntohs(cliaddr.sin_port));
        
        if (n < 0) {
            perror("Server receiver error");
            continue;
        }

        if (cliaddr.sin_addr.s_addr != session->client_address.sin_addr.s_addr ||
            (cliaddr.sin_port != session->client_address.sin_port && session->client_address.sin_port != 0)) {
            perror("Mensagem recebida de outro cliente no socket dedicado");
            continue;
        }

        unmarshalling_packet(&message, buffer);

        if (message == NULL) {
            perror("Server unmarshalling packet error");
            continue;
        }

        session->seqn = message->seqn;

        if (message->type == PACKET_CMD_LOGOUT_T) {
            int result_code = logout(user_logged, &(session->client_address), result_message);
            server_send_message(result_code, strlen(result_message), result_message, session);
            if (result_code == PACKET_DATA_LOGOUT_OK_T) {
                // quando usuário der logout, precisamos matar esse socket
                break;
            }
        } else {
            dispatch_message(message, session, user_logged);
        }

        free_packet(message);
        message = NULL;
    }

    if (message != NULL) {
        free_packet(message);
    }

    printf("Closing session receiver on port: %d...\n", ntohs(session->client_address.sin_port));

    close(session->server_sockfd);
    free(session);
    free(recv_args);
    return NULL;
}

bool server_send_message(uint16_t type, size_t payload_size, char *payload, const session_addr *address) {
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
            address->seqn, 
            (uint16_t) payload_size, 
            (uint32_t) time(NULL),
            payload
        };

        printf("sendto: port %d ID %d\n", ntohs(address->client_address.sin_port), address->server_sockfd);

        size_t message_size = marshalling_packet(&message, &buffer);

        ssize_t size = sendto(
            address->server_sockfd,
            (const void *) buffer,
            message_size,
            0,
            (const sockaddr*) &(address->client_address),
            sizeof(user_address));

        free(buffer);

        if (size == -1)
        {
            perror("Mensagem não enviada");
            return false;
        }
    }
    return true;
}

bool server_send_notif(uint16_t type, notification *payload, const session_addr *address) {
    char *buffer;
    size_t message_size = marshalling_notification(payload, &buffer);

    bool sent = server_send_message(type, message_size, buffer, address);

    free(buffer);

    return sent;
}
