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
    printf("packet { type=%d, seqn=%d, length=%d, timestamp=%d, payload=\"%s\" }\n",
        message->type, message->seqn, message->length, message->timestamp, message->payload);
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
    free(p->payload);
    free(p);
}
