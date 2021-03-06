#ifndef TYPES_H
#define TYPES_H

#include <vector>
#include <string>
#include <pthread.h>
#include <netinet/in.h>

#include "stdint.h"

// Tipos de comandos que o usuário/server pode enviar
// [0, 50) -> mensagem do user
// [50, 100) -> mensagem enviada pelo server
#define PACKET_CMD_LOGIN_T 0
#define PACKET_CMD_LOGOUT_T 1
#define PACKET_CMD_ALIVE_T 2
#define PACKET_CMD_FOLLOW_T 3
#define PACKET_CMD_UNFOLLOW_T 4
#define PACKET_CMD_NEW_NOTIFY_T 5 // quando o client envia uma nova notificação
#define PACKET_CMD_ECHO_T 6
#define PACKET_CMD_NOTIFY_T 50     // quando o server envia uma notificação
#define PACKET_CMD_NEW_SERVER_T 51 // quando o server informa uma nova instancia dele
#define PACKET_CMD_END_SERVER 99   // quando o servidor está prestes a morrer

// Tipos de dados que o usuário pode receber com status OK
#define PACKET_DATA_NOTIFICATION_T 100
#define PACKET_DATA_LOGIN_OK_T 101
#define PACKET_DATA_LOGOUT_OK_T 102
#define PACKET_DATA_GENERAL_OK_T 103
#define PACKET_DATA_FOLLOW_OK_T 104
#define PACKET_DATA_UNFOLLOW_OK_T 105
#define PACKET_DATA_ECHO_RESP_T 106

// Tipos de dados que o usuário pode receber com status ERROR
#define PACKET_DATA_LOGIN_ERROR_T 200
#define PACKET_DATA_LOGOUT_ERROR_T 201
#define PACKET_DATA_GENERAL_ERROR_T 202
#define PACKET_DATA_FOLLOW_ERROR_T 203
#define PACKET_DATA_UNFOLLOW_ERROR_T 204
#define PACKET_DATA_UNAUTHENTICATED_T 250

// Mensagens de request para a resistencia
#define PACKET_RESISTENCE_MSG_T 300
#define PACKET_CMD_POKE_COORD_T 301
#define PACKET_CMD_ELECTION_T 302
#define PACKET_CMD_ELECTION_ANWER_T 303
#define PACKET_CMD_COORD_AD_T 304

// Mensagens de result para a resistencia
#define PACKET_RESISTENCE_MSG_RESULT_T 350


#define USER_MAX_SESSIONS 2

using namespace std;

typedef struct __packet
{
    uint16_t type;        // Tipo do pacote
    uint16_t seqn;        // Número de sequência
    uint16_t length;      // Comprimento do payload
    uint32_t timestamp;   // Timestamp do dado
    char *payload;        // Dados da mensagem
} packet;

typedef struct __notification
{
    uint32_t id;         // Identificador da notificação
    uint32_t timestamp;  // Timestamp da notificação
    uint16_t length;     // Tamanho da mensagem
    uint16_t pending;    // Quantidade de leitores pendentes
    uint16_t author_len; // Tamanho do nome do autor
    char *author;        // Username author da mensagem
    char *message;       // Mensagem
} notification;

typedef struct __resistence_msg {
    uint16_t type;
    int sender;
    int destination;
} resistence_msg;

typedef sockaddr_in user_address;

typedef struct __session_addr {
    uint16_t seqn;
    user_address client_address; 
    int server_sockfd;
    pthread_t server_thread;
} session_addr;

typedef struct __user
{
    string username;
    vector<string>* follows;          // username de quem segue esse usuário
    // Começa dados efemeros (que não são persistidos)
    vector<uint32_t>* pending_msg;    // id das notificações que falta receber
    vector<session_addr*>* addresses; // endereços das sessões atuais
    pthread_mutex_t mutex_addr;       // mutex usado para alteração concorrente dos endereços
    pthread_mutex_t mutex_follows;    // mutex usado para alteração concorrente dos follows
} user, *user_p;

typedef bool (*send_notif_callback_t)(uint16_t type, notification *payload, const session_addr *address);

#endif