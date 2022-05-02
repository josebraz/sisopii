#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "communication_utils.hpp"

size_t marshalling_packet(const packet *message, char **buffer) {
    size_t message_size = sizeof(packet) - sizeof(char *) + message->length + 1;
    size_t buffer_index = 0;
    *buffer = (char *) calloc(message_size, 1);
    
    memmove(*buffer, &(message->type), sizeof(uint16_t));
    buffer_index += sizeof(uint16_t);
    memmove(*buffer + buffer_index, &(message->seqn), sizeof(uint16_t));
    buffer_index += sizeof(uint16_t);
    memmove(*buffer + buffer_index, &(message->length), sizeof(uint16_t));
    buffer_index += sizeof(uint16_t);
    memmove(*buffer + buffer_index, &(message->timestamp), sizeof(uint32_t));
    buffer_index += sizeof(uint32_t);
    memmove(*buffer + buffer_index, message->payload, message->length + 1);

    return message_size;
}

size_t marshalling_notification(const notification *message, char **buffer) {
    size_t message_size = (sizeof(notification) - 2 * sizeof(char *)) + message->author_len + 1 + message->length + 1;
    size_t buffer_index = 0;
    *buffer = (char *) calloc(message_size, 1);

    memmove(*buffer, &(message->id), sizeof(uint32_t));
    buffer_index += sizeof(uint32_t);
    memmove(*buffer + buffer_index, &(message->timestamp), sizeof(uint32_t));
    buffer_index += sizeof(uint32_t);
    memmove(*buffer + buffer_index, &(message->length), sizeof(uint16_t));
    buffer_index += sizeof(uint16_t);
    memmove(*buffer + buffer_index, &(message->pending), sizeof(uint16_t));
    buffer_index += sizeof(uint16_t);
    memmove(*buffer + buffer_index, &(message->author_len), sizeof(uint16_t));
    buffer_index += sizeof(uint16_t);
    memmove(*buffer + buffer_index, message->author, message->author_len + 1);
    buffer_index += message->author_len + 1;
    memmove(*buffer + buffer_index, message->message, message->length + 1);
    buffer_index += message->length + 1;

    return message_size;
}

size_t marshalling_new_address(int address_length, const char *address, int new_port, char **buffer) {
    u_int16_t port16 = (uint16_t) new_port;
    u_int16_t len16 = (uint16_t) address_length;

    size_t message_size = sizeof(uint16_t) + sizeof(uint16_t) + address_length + 1;
    size_t buffer_index = 0;
    *buffer = (char *) calloc(message_size, 1);

    memmove(*buffer + buffer_index, &port16, sizeof(uint16_t));
    buffer_index += sizeof(uint16_t);

    memmove(*buffer + buffer_index, &len16, sizeof(uint16_t));
    buffer_index += sizeof(uint16_t);

    memmove(*buffer + buffer_index, address, address_length + 1);
    buffer_index += address_length + 1;

    return message_size;
}

size_t marshalling_resistence_msg(const resistence_msg *message, char **buffer) {
    size_t message_size = sizeof(resistence_msg);
    size_t buffer_index = 0;
    *buffer = (char *) calloc(message_size, 1);

    memmove(*buffer, &(message->type), sizeof(uint16_t));
    buffer_index += sizeof(uint16_t);
    memmove(*buffer + buffer_index, &(message->sender), sizeof(int));
    buffer_index += sizeof(int);
    memmove(*buffer + buffer_index, &(message->destination), sizeof(int));
    buffer_index += sizeof(int);

    return message_size;
}

void unmarshalling_packet(packet **message, const char *buffer) {
    *message = (packet *) calloc(sizeof(packet), 1);
    size_t buffer_index = 0;
    
    (*message)->type = *((uint16_t*)(buffer));
    buffer_index += sizeof(uint16_t);

    (*message)->seqn = *((uint16_t*)(buffer + buffer_index));
    buffer_index += sizeof(uint16_t);

    (*message)->length = *((uint16_t*)(buffer + buffer_index));
    buffer_index += sizeof(uint16_t);

    (*message)->timestamp = *((uint32_t*)(buffer + buffer_index));
    buffer_index += sizeof(uint32_t);

    (*message)->payload = (char *) calloc((*message)->length + 1, 1);
    memmove((*message)->payload, buffer + buffer_index, (*message)->length + 1);
}

void unmarshalling_notification(notification **message, const char *buffer) {
    *message = (notification *) calloc(sizeof(notification), 1);
    size_t buffer_index = 0;
    
    (*message)->id = *((uint32_t*)(buffer));
    buffer_index += sizeof(uint32_t);

    (*message)->timestamp = *((uint32_t*)(buffer + buffer_index));
    buffer_index += sizeof(uint32_t);

    (*message)->length = *((uint16_t*)(buffer + buffer_index));
    buffer_index += sizeof(uint16_t);

    (*message)->pending = *((uint16_t*)(buffer + buffer_index));
    buffer_index += sizeof(uint16_t);

    (*message)->author_len = *((uint16_t*)(buffer + buffer_index));
    buffer_index += sizeof(uint16_t);

    (*message)->author = (char *) calloc((*message)->author_len + 1, 1);
    memmove((*message)->author, buffer + buffer_index, (*message)->author_len + 1);
    buffer_index += (*message)->author_len + 1;

    (*message)->message = (char *) calloc((*message)->length + 1, 1);
    memmove((*message)->message, buffer + buffer_index, (*message)->length + 1);
    buffer_index += (*message)->length + 1;
}

void unmarshalling_new_address(char *address, int *new_port, const char *buffer) {
    int buffer_index = 0; 
    u_int16_t address_length;

    *new_port = (int) *((uint16_t*)(buffer + buffer_index));
    buffer_index += sizeof(uint16_t);

    address_length = *((uint16_t*)(buffer + buffer_index));
    buffer_index += sizeof(uint16_t);

    memmove(address, buffer + buffer_index, address_length + 1);
    buffer_index += address_length + 1;
}

void unmarshalling_resistence_msg(resistence_msg **message, const char *buffer) {
    *message = (resistence_msg *) calloc(sizeof(resistence_msg), 1);
    int buffer_index = 0; 

    (*message)->type = *((uint16_t*)(buffer + buffer_index));
    buffer_index += sizeof(uint16_t);

    (*message)->sender = *((int *)(buffer + buffer_index));
    buffer_index += sizeof(int);

    (*message)->destination = *((int *)(buffer + buffer_index));
    buffer_index += sizeof(int);
}

packet *wait_response(
    uint16_t seqn, 
    pthread_mutex_t *mutex, 
    pthread_cond_t *receiver_cond, 
    pthread_cond_t *wait_cond, 
    uint16_t *wait_seqn, 
    packet **last_message_received
) {
    packet *response = NULL;
    pthread_mutex_lock(mutex);

    timespec cond_timeout;
    cond_timeout.tv_sec = time(NULL) + 2;
    cond_timeout.tv_nsec = 0;

    // espera se já tem gente esperando
    while (*wait_seqn != NO_WAIT_SEQN) {
        pthread_cond_wait(receiver_cond, mutex);
    }
    *wait_seqn = seqn;

    // informa que estamos escutando
    pthread_cond_signal(wait_cond);

    // Espera até que o servidor envie uma mensagem com o mesmo seqn da
    // mensagem que o cliente enviou, ou pode dar timeout depois de 2 segundos
    while (*last_message_received == NULL || *wait_seqn != (* last_message_received)->seqn) {
        int ret = pthread_cond_timedwait(receiver_cond, mutex, &cond_timeout);
        if (ret == 0) {
            response = *last_message_received;
        } else {
            response = NULL;
            break;
        }
    }
    *wait_seqn = NO_WAIT_SEQN;
    pthread_mutex_unlock(mutex);
    return response;
}

void signal_response(
    packet *received_message, 
    pthread_mutex_t *mutex, 
    pthread_cond_t *receiver_cond, 
    pthread_cond_t *wait_cond, 
    uint16_t *wait_seqn, 
    packet **last_message_received
) {
    pthread_mutex_lock(mutex);

    // espera por um tempo até alguém estar escutando o signal
    timespec cond_timeout;
    cond_timeout.tv_sec = time(NULL) + 1;
    cond_timeout.tv_nsec = 0;
    pthread_cond_timedwait(wait_cond, mutex, &cond_timeout);

    // Se já tinha uma mensagem recebida, precisamos liberar a memória
    if (*last_message_received != NULL) {
        free_packet(*last_message_received);
        *last_message_received = NULL;
    }
    copy_packet(last_message_received, received_message);

    // Avisa a variável de condição que estava esperando essa resposta
    // do servidor, liberando o bloqueio da função de sendo
    if ((*last_message_received)->seqn == *wait_seqn) {
        // printf("signal_response wait %d received %d\n", *wait_seqn, (*last_message_received)->seqn);
        pthread_cond_signal(receiver_cond);
    }

    pthread_mutex_unlock(mutex);
}

void print_resistence(const resistence_msg *message) {
    if (message == NULL) {
        printf("resistence_msg NULL\n");
    } else {
        printf("resistence_msg { type=%d, sender=%d, destination=%d }\n",
            message->type, message->sender, message->destination);
    }
}

void print_packet(const packet *message) {
    if (message == NULL) {
        printf("packet NULL\n");
    } else {
        printf("packet { type=%d, seqn=%d, length=%d, timestamp=%d, payload=\"%s\" }\n",
            message->type, message->seqn, message->length, message->timestamp, message->payload);
    }
}

void print_notification(const notification *message) {
    if (message == NULL) {
        printf("notification NULL\n");
    } else {
        printf("notification { id=%d, timestamp=%d, length=%d, pending=%d, author_len=%d, author=\"%s\", message=\"%s\" }\n",
            message->id, message->timestamp, message->length, message->pending, message->author_len, message->author, message->message);
    }
}

void copy_packet(packet **dest, const packet *src) {
    *dest = (packet *) malloc(sizeof(packet));
    (*dest)->payload = (char *) calloc(src->length + 1, 1);
    (*dest)->type = src->type;
    (*dest)->length = src->length;
    (*dest)->seqn = src->seqn;
    (*dest)->timestamp = src->timestamp;
    memmove((*dest)->payload, src->payload, src->length + 1);
}

void free_packet(packet *p) {
    if (p != NULL) {
        free(p->payload);
        free(p);
    }
}

void free_notification(notification *notif) {
    if (notif != NULL) {
        free(notif->author);
        free(notif->message);
        free(notif);
    }
}

void free_user(user_p user) {
    if (user != NULL) {
        free(user->follows);
        free(user->pending_msg);
        for (int i = 0; i < user->addresses->size(); i++) {
            free(user->addresses->at(i));
        }
        free(user->addresses);
        free(user);
    }
}

bool is_response(uint32_t type) {
    return type >= 100 && type <= 300;
}