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

#include "resistence.hpp"
#include "../communication_utils.hpp"
#include "../types.hpp"
#include "../constants.h"

#define GROUP "239.255.255.250"

int in_election = 0;
int is_coordinator = 0;

int coord_pid = -1;
int broadcast_fd = -1;
uint16_t next_seq_n = 0, wait_seqn = 0;

packet *last_message_received = NULL;
pthread_t receiver_thread;
pthread_cond_t receiver_cond, wait_cond;
pthread_mutex_t mutex_response;

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

void *server_resistence_receiver(void *arg) {
    printf("Starting listener broadcast\n");
    pid_t my_pid = getpid();

    user_address cliaddr;
    char buffer[PAYLOAD_MAX_SIZE];
    packet *message = NULL;
    resistence_msg *resistence = NULL;
    socklen_t len = sizeof(user_address);
    
    while (1)
    {
        int n = recvfrom(
            broadcast_fd,
            (void *) buffer,
            PAYLOAD_MAX_SIZE,
            0,
            (struct sockaddr *) &cliaddr,
            &len
        );
        
        if (n < 0) {
            perror("Server receiver error");
            continue;
        }

        if (n != (sizeof(packet) + sizeof(resistence_msg) - sizeof(char *) + 1)) {
            // perror("Mensagem não é de resistencia");
            continue;
        }

        unmarshalling_packet(&message, buffer);

        if (message == NULL) {
            perror("Server unmarshalling packet error");
            continue;
        }

        unmarshalling_resistence_msg(&resistence, message->payload);

        if (resistence == NULL) {
            free_packet(message);
            message = NULL;
            perror("Server unmarshalling resistence error");
            continue;
        }

        // Chegou a mensagem que eu enviei por multicast pra mim mesmo, só ignorar
        if (resistence->sender == my_pid) {
            free_packet(message);
            message = NULL;
            free(resistence);
            resistence = NULL;
            continue;
        }

        // Processa a mensagem
        if (message->type == PACKET_RESISTENCE_MSG_RESULT_T) {
            printf("resistence receiver result from %d\n", ntohs(cliaddr.sin_port));
            print_packet(message);
            signal_response(message, &mutex_response, &receiver_cond, &wait_cond, &wait_seqn, &last_message_received);
        } else if (message->type == PACKET_RESISTENCE_MSG_T) {
            printf("resistence receiver from %d\n", ntohs(cliaddr.sin_port));
            print_packet(message);
            print_resistence(resistence);

            if (resistence->destination == my_pid || resistence->destination == ANY_DEST) {
                // Mensagem destinada para mim ou qualquer um
                if (resistence->type == PACKET_CMD_POKE_COORD_T && is_coordinator == 1) {
                    resistence_msg response = {
                        PACKET_CMD_POKE_COORD_T,
                        (int) my_pid,
                        resistence->sender
                    };
                    printf("Responding POKE coordinator\n");
                    print_resistence(&response);
                    send_broadcast(&response, PACKET_RESISTENCE_MSG_RESULT_T, message->seqn);
                }
            }            
        }

        if (resistence != NULL) {
            free(resistence);
            resistence = NULL;
        }

        if (message != NULL) {
            free_packet(message);
            message = NULL;
        }
    }

    if (resistence != NULL) {
        free(resistence);
        resistence = NULL;
    }

    if (message != NULL) {
        free_packet(message);
    }
}

void init_election() {
    printf("init_election\n");
    // bloqueante até que tenhamos um eleito
    sleep(5000); // TODO
}

void start_resistence_server() {
    printf("start_resistence_server\n");
    init_sync_comm();
    
    if ((broadcast_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    u_int yes = 1;
    if (setsockopt(broadcast_fd, SOL_SOCKET, SO_REUSEADDR, (char*) &yes, sizeof(yes)) < 0){
       perror("Reusing ADDR failed");
       exit(EXIT_FAILURE);
    }

    sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERVER_RESISTENCE_PORT);

    if (bind(
            broadcast_fd,
            (const struct sockaddr *) &servaddr,
            sizeof(sockaddr_in)) < 0
    ) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // solicita para entrar no grupo multicast
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(GROUP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(broadcast_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &mreq, sizeof(mreq)) < 0){
        perror("join multicast group error");
        exit(EXIT_FAILURE);
    }

    pthread_t receiver_thread;
    pthread_create(&receiver_thread, NULL, server_resistence_receiver, NULL);

    resistence_msg *result = NULL;
    if (coord_pid == -1) {
        // não sabemos quem é o coordinator, então vamos mandar uma mensagem
        // broadcast pra perguntar aos outros quem é o coordinator
        // se não houver resposta, nós nos tornaremos o coordinator
        result = poke_coordinator(ANY_DEST);
        if (result == NULL) {
            printf("Não achamos ninguém na rede, nos marcamos como coordenador\n");
            coord_pid = getpid();
            is_coordinator = 1;
        } else {
            coord_pid = result->sender;
            free(result);
            result = NULL;
        }
    }

    printf("Coordinator fold %d, my_pid %d\n", coord_pid, getpid());

    // Fica mandado poke para o grupo até se tornar o coordenador
    int consecutive_failures = 0;
    while (is_coordinator == 0) {
        sleep(3);
        result = poke_coordinator(coord_pid);
        if (result == NULL && in_election == 0) {
            // Aqui vamos roubar um pouco, toleramos até 3 falhas consecutivas
            // do coordenador, é um forma de mitigar o problema da não confiabilidade do IP
            if (consecutive_failures > 3) {
                // o coordinator parou de responder e nos ainda 
                // não estamos em uma eleição, vamos começar uma
                init_election();
            }
            consecutive_failures++;
        } else {
            printf("COORD Respondeu, tudo certo!\n");
            consecutive_failures = 0;
        }

        if (result != NULL) {
            free(result);
            result = NULL;
        }
    }
}

int send_broadcast(resistence_msg *resistence, uint16_t type, uint16_t seqn) {
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(GROUP);
    servaddr.sin_port = htons(SERVER_RESISTENCE_PORT);

    char *buffer = NULL;
    char *payload = NULL;

    pid_t my_pid = getpid();

    size_t resistence_size = marshalling_resistence_msg(resistence, &payload);

    packet message = {
        type, 
        seqn, 
        (uint16_t) resistence_size, 
        (uint32_t) time(NULL),
        payload
    };

    size_t message_size = marshalling_packet(&message, &buffer);

    printf("sendto: %d\n", ntohs(servaddr.sin_port));
    ssize_t size = sendto(
        broadcast_fd,
        (const void *) buffer,
        message_size,
        0,
        (const struct sockaddr *)&servaddr,
        sizeof(struct sockaddr_in));

    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }
    if (payload != NULL) {
        free(payload);
        payload = NULL;
    }
    
    if (size == -1)
    {
        perror("Mensagem não enviada");
        return -1;
    }

    return seqn;
}

resistence_msg *send_broadcast_and_wait(resistence_msg *resistence, uint16_t type, uint16_t seqn) {
    send_broadcast(resistence, type, seqn);

    resistence_msg *resistence_rsp = NULL;
    packet *response_packet = NULL;

    do {
        if (resistence_rsp != NULL) {
            free(resistence_rsp);
            resistence_rsp = NULL;
        }
        response_packet = wait_response(
            seqn, 
            &mutex_response, 
            &receiver_cond, 
            &wait_cond,
            &wait_seqn, 
            &last_message_received
        );
        if (response_packet != NULL) {
            unmarshalling_resistence_msg(&resistence_rsp, response_packet->payload);
        } else {
            resistence_rsp = NULL;
        }

        printf("send_broadcast_and_wait\n");
        print_packet(response_packet);
        print_resistence(resistence_rsp);

        // ficamos presos no loop até recebermos uma mensagem destinada ao nosso PID ou a todos
        // ou der timeout esperando (response_packet == NULL)
    } while (response_packet != NULL &&                    // ainda não deu timeout
             resistence_rsp->sender == getpid() &&         // eu que enviei essa mensagem
             resistence_rsp->destination != ANY_DEST &&    // recebemos um broadcast
             resistence_rsp->destination != getpid());     // recebemos uma mensagem enviada diretamente pra nós

    return resistence_rsp;
}

resistence_msg *poke_coordinator(int coord) {
    pid_t my_pid = getpid();
    
    resistence_msg resistence = {
        PACKET_CMD_POKE_COORD_T,
        (int) my_pid,
        coord
    };

    printf("POKE coordinator\n");
    print_resistence(&resistence);

    return send_broadcast_and_wait(&resistence, PACKET_RESISTENCE_MSG_T, next_seq_n++);
}