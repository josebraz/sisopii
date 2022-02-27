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
#include "persistence.hpp"
#include "communication_manager.hpp"

using namespace std;

int sockfd;

void start_server()
{
    struct sockaddr_in servaddr, cliaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(SERVER_PORT);

    if (bind(
            sockfd,
            (const struct sockaddr *)&servaddr,
            sizeof(servaddr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    pthread_t receiver_thread;
    pthread_create(&receiver_thread, NULL, message_receiver, NULL);
}

void *message_receiver(void *arg) {
    struct sockaddr_in cliaddr;
    char buffer[PAYLOAD_MAX_SIZE];
    int len, n;
    while (true)
    {
        n = recvfrom(
            sockfd,
            (char *) buffer,
            PAYLOAD_MAX_SIZE,
            MSG_WAITALL,
            (struct sockaddr *) &cliaddr,
            (socklen_t *) &len);
        buffer[n] = '\0';
        printf("Client : %s\n", buffer);
    }
}

void send_message(char *message, const struct sockaddr *cliaddr) {
    int message_size = strlen(message);

    if (message_size > PAYLOAD_MAX_SIZE)
    {
        log_error("A mensagem para o servidor é muito grande");
    }
    else
    {
        ssize_t size = sendto(
            sockfd,
            (const char *)message,
            message_size,
            MSG_CONFIRM,
            cliaddr,
            sizeof(cliaddr));

        if (size == -1)
        {
            log_error("Mensagem não enviada");
        }
    }
}
