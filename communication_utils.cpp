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
        printf("notification { id=%d, timestamp=%d, length=%d, pending=%d, author=\"%s\", message=\"%s\" }\n",
            message->id, message->timestamp, message->length, message->pending, message->author, message->message);
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