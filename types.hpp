#ifndef TYPES_H
#define TYPES_H

#include <vector>
#include <string>

#include "stdint.h"
#include "constants.h"

// Tipos de comandos que o usuário pode enviar
#define PACKET_CMD_LOGIN_T 0
#define PACKET_CMD_LOGOUT_T 1
#define PACKET_CMD_ALIVE_T 2
#define PACKET_CMD_FOLLOW_T 3
#define PACKET_CMD_UNFOLLOW_T 4
#define PACKET_CMD_NOTIFY_T 5
#define PACKET_CMD_ECHO_T 30

// Tipos de dados que o usuário pode receber com status OK
#define PACKET_DATA_NOTIFICATION_T 100
#define PACKET_DATA_LOGIN_OK_T 101
#define PACKET_DATA_LOGOUT_OK_T 102
#define PACKET_DATA_GENERAL_OK_T 103

// Tipos de dados que o usuário pode receber com status ERROR
#define PACKET_DATA_LOGIN_ERROR_T 200
#define PACKET_DATA_LOGOUT_ERROR_T 201
#define PACKET_DATA_GENERAL_ERROR_T 202

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
    const char *_string; // Mensagem
} notification;

typedef struct __user
{
    string username;
    vector<string>* follows;
    uint8_t sessions;
} user, *user_p;

#endif