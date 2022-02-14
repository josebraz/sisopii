#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "constants.h"
#include "logs.h"

struct sockaddr_in servaddr;
int sockfd;

void *message_receiver(void *arg);

void send_message(char* message);

void start_client() {
    char *hello = "Hello from client";
   
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
   
    memset(&servaddr, 0, sizeof(servaddr));
       
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;
       
    pthread_t receiver_thread;
    pthread_create(&receiver_thread, NULL, message_receiver, NULL);
}

void *message_receiver(void *arg) {
    char buffer[PAYLOAD_MAX_SIZE];
    int n, len;

    while (1) {
        n = recvfrom(
            sockfd, 
            (char *) buffer, 
            PAYLOAD_MAX_SIZE, 
            MSG_WAITALL, 
            (struct sockaddr *) &servaddr,
            &len
        );
        buffer[n] = '\0';
        printf("Server : %s\n", buffer);
    }
   
    close(sockfd);
}

void send_message(char* message) {
    int message_size = strlen(message);

    if (message_size > PAYLOAD_MAX_SIZE) {
        log_error("A mensagem para o servidor é muito grande");
    } else {
        ssize_t size = sendto(
            sockfd, 
            (const char *) message, 
            message_size,
            MSG_CONFIRM, 
            (const struct sockaddr *) &servaddr, 
            sizeof(servaddr)
        );
        
        if (size == -1) {
            log_error("Mensagem não enviada");
        }
    }
}

int main() {
    start_client();

    char user_input[PAYLOAD_MAX_SIZE];

    while (strcmp(user_input, "") != 0) {
        printf("Digite um comando: ");
        scanf("%s", user_input);
        send_message(user_input);
    }

    return 0;
}
