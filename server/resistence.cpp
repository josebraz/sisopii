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
pthread_cond_t receiver_cond, wait_cond, election_cond;
pthread_mutex_t mutex_response, election_mutex;

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

    ret = pthread_cond_init(&election_cond, NULL);
    if (ret != 0) {
        perror("cond init failed");
        exit(EXIT_FAILURE);
    }

    ret = pthread_mutex_init(&mutex_response, NULL);
    if (ret != 0) {
        perror("mutex init failed");
        exit(EXIT_FAILURE);
    }

    ret = pthread_mutex_init(&election_mutex, NULL);
    if (ret != 0) {
        perror("mutex init failed");
        exit(EXIT_FAILURE);
    }
}

void process_received_message(resistence_msg *resistence, int seqn) {
    pid_t my_pid = getpid();

    if (resistence->type == PACKET_CMD_POKE_COORD_T) {
        if (is_coordinator == 1) {
            // Se sou coordinator, respondo ao poke para saberem que estou vivo :)
            send_poke_response(resistence->sender, seqn);
        }
    } else if (resistence->type == PACKET_CMD_ELECTION_T) {
        if (resistence->sender < my_pid) {
            // se podemos ser eleitos, enviamos uma resposta e começamos
            // uma eleição se não nos candidatamos ainda
            send_election_response(resistence->sender, seqn);
            if (in_election == 0) {
                init_election();
            }
        } else {
            // não somos o maior id, só marcamos que estamos em uma eleição
            in_election = 1;
        }
    } else if (resistence->type == PACKET_CMD_COORD_AD_T) {
        pthread_mutex_lock(&election_mutex);
        coord_pid = resistence->sender;
        in_election = 0;
        is_coordinator = 0;
        pthread_cond_signal(&election_cond);
        pthread_mutex_unlock(&election_mutex);
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

        // printf("resistence receiver from %d\n", ntohs(cliaddr.sin_port));
        // print_packet(message);
        // print_resistence(resistence);

        // Processa a mensagem
        if (message->type == PACKET_RESISTENCE_MSG_RESULT_T) {
            signal_response(message, &mutex_response, 
                &receiver_cond, &wait_cond, &wait_seqn, &last_message_received);
        } else if (message->type == PACKET_RESISTENCE_MSG_T) {
            if (resistence->destination == my_pid || 
                resistence->destination == ANY_DEST) {
                process_received_message(resistence, message->seqn);
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
    printf("start init_election\n");
    // bloqueante até que tenhamos um eleito
    in_election = 1;
    resistence_msg *result = send_election();

    if (result == NULL) {
        // ninguém me respondeu, é porque eu sou o maior ID e devo ser o coordenador
        send_announcement();
        pthread_mutex_lock(&election_mutex);
        in_election = 0;
        is_coordinator = 1;
        coord_pid = getpid();
        pthread_mutex_unlock(&election_mutex);
    } else {
        // não sou o maior ID, devo espear até que elejam alguém 
        pthread_mutex_lock(&election_mutex);
        pthread_cond_wait(&election_cond, &election_mutex);
        pthread_mutex_unlock(&election_mutex);
    }
    printf("end init_election\n");
}

void find_coordinator() {
    resistence_msg *result = NULL;
    pthread_mutex_lock(&election_mutex);
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
    pthread_mutex_unlock(&election_mutex);
    printf("Coordinator fold %d, my_pid %d\n", coord_pid, getpid());
}

void start_coordinator_inspection() {
    resistence_msg *result = NULL;
    // Fica mandado poke para o grupo até se tornar o coordenador
    int consecutive_failures = 0;
    while (is_coordinator == 0) {
        sleep(3);
        result = poke_coordinator(coord_pid);
        if (result == NULL && in_election == 0) {
            // Aqui vamos roubar um pouco, toleramos até 2 falhas consecutivas
            // do coordenador, é um forma de mitigar o problema da não confiabilidade do IP
            if (consecutive_failures >= 2) {
                // o coordinator parou de responder e nos ainda 
                // não estamos em uma eleição, vamos começar uma
                init_election();
                consecutive_failures = 0;
            } else {
                consecutive_failures++;
            }
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

    find_coordinator();
    start_coordinator_inspection();
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

        // printf("send_broadcast_and_wait\n");
        // print_packet(response_packet);
        // print_resistence(resistence_rsp);

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

void send_poke_response(int destination, int seqn) {
    pid_t my_pid = getpid();

    resistence_msg response = {
        PACKET_CMD_POKE_COORD_T,
        (int) my_pid,
        destination
    };

    printf("Responding POKE coordinator\n");

    send_broadcast(&response, PACKET_RESISTENCE_MSG_RESULT_T, seqn);
}

resistence_msg *send_election() {
    pid_t my_pid = getpid();
    
    resistence_msg resistence = {
        PACKET_CMD_ELECTION_T,
        (int) my_pid,
        ANY_DEST
    };

    printf("send_election\n");

    return send_broadcast_and_wait(&resistence, PACKET_RESISTENCE_MSG_T, next_seq_n++);
}

void send_election_response(int destination, int seqn) {
    pid_t my_pid = getpid();

    resistence_msg response = {
        PACKET_CMD_ELECTION_ANWER_T,
        (int) my_pid,
        destination
    };

    printf("Responding ELECTION WITH ANWER\n");

    send_broadcast(&response, PACKET_RESISTENCE_MSG_RESULT_T, seqn);
}

void send_announcement() {
    pid_t my_pid = getpid();
    
    resistence_msg resistence = {
        PACKET_CMD_COORD_AD_T,
        (int) my_pid,
        ANY_DEST
    };

    printf("send_announcement\n");

    send_broadcast(&resistence, PACKET_RESISTENCE_MSG_T, next_seq_n++);
}

