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

#include "constants.h"
#include "types.hpp"
#include "persistence.hpp"

using namespace std;

void start_server()
{
    int sockfd;
    char buffer[PAYLOAD_MAX_SIZE];
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

    int len, n;
    while (true)
    {
        n = recvfrom(
            sockfd,
            (char *)buffer,
            PAYLOAD_MAX_SIZE,
            MSG_WAITALL,
            (struct sockaddr *)&cliaddr,
            (socklen_t *)&len);
        buffer[n] = '\0';
        printf("Client : %s\n", buffer);
    }
}

int main()
{

    start_server();

    return 0;
}
