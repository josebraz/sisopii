#ifndef TYPES_H
#define TYPES_H

#include <vector>
#include <string>

#include "stdint.h"
#include "constants.h"

using namespace std;

typedef struct __packet
{
    uint16_t type;        //Tipo do pacote (p.ex. DATA | CMD)
    uint16_t seqn;        //Número de sequência
    uint16_t length;      //Comprimento do payload
    uint16_t timestamp;   // Timestamp do dado
    const char *_payload; //Dados da mensagem
} packet;

typedef struct __notification
{
    uint32_t id;         //Identificador da notificação (sugere-se um identificador único)
    uint32_t timestamp;  //Timestamp da notificação
    uint16_t length;     //Tamanho da mensagem
    uint16_t pending;    //Quantidade de leitores pendentes
    const char *_string; //Mensagem
} notification;

typedef struct __user
{
    string username;
    vector<string>* follows;
} user, *user_p;

#endif