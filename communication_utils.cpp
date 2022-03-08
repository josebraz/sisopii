#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "communication_utils.hpp"

size_t marshalling_packet(const packet *message, char **buffer) {
    size_t message_size = sizeof(packet) - sizeof(char *) + message->length + 1;
    size_t buffer_index = 0;
    *buffer = (char *) calloc(message_size, 1);
    
    memmove(*buffer + buffer_index, &(message->type), sizeof(uint16_t));
    buffer_index += sizeof(uint16_t);
    memmove(*buffer + buffer_index, &(message->seqn), sizeof(uint16_t));
    buffer_index += sizeof(uint16_t);
    memmove(*buffer + buffer_index, &(message->length), sizeof(size_t));
    buffer_index += sizeof(size_t);
    memmove(*buffer + buffer_index, &(message->timestamp), sizeof(time_t));
    buffer_index += sizeof(time_t);
    memmove(*buffer + buffer_index, message->payload, message->length + 1);

    return message_size;
}

void unmarshalling_packet(packet **message, const char *buffer) {
    *message = (packet *) calloc(sizeof(packet), 1);
    size_t buffer_index = 0;
    
    (*message)->type = *((uint16_t*)(buffer + buffer_index));
    buffer_index += sizeof(uint16_t);

    (*message)->seqn = *((uint16_t*)(buffer + buffer_index));
    buffer_index += sizeof(uint16_t);

    (*message)->length = *((size_t*)(buffer + buffer_index));
    buffer_index += sizeof(size_t);

    (*message)->timestamp = *((time_t*)(buffer + buffer_index));
    buffer_index += sizeof(time_t);

    (*message)->payload = (char *) malloc((*message)->length + 1);
    memmove((*message)->payload, buffer + buffer_index, (*message)->length + 1);
}

void print_packet(const packet *message) {
    printf("packet { type=%d, seqn=%d, length=%ld, timestamp=%ld, payload=%s }\n",
        message->type, message->seqn, message->length, message->timestamp, message->payload);

}