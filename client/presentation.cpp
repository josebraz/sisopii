#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "presentation.hpp"
#include "../constants.h"
#include "../logs.hpp"
#include "client_comm_manager.hpp"

void start_presentation(char *my_user) {
    size_t bufsize = PAYLOAD_MAX_SIZE;
    size_t characters;
    char *user_input = (char *)malloc(bufsize * sizeof(char));

    while (1) {
        printf("Digite um comando: ");

        characters = getline(&user_input, &bufsize, stdin);

        if (bufsize == 0) {
            continue;
        }

        user_input[characters-1] = '\0'; // elimina o \n

        if (strncmp("FOLLOW", user_input, 6) == 0) {
            send_follow_msg(user_input + 7);
        } else if (strncmp("SEND", user_input, 4) == 0) {
            send_notify_msg(user_input + 5);
        } else if (strncmp("EXIT", user_input, 4) == 0) {
            send_logout_msg(my_user);
            printf("BYE\n");
            break;
        } else {
            printf("Comando não reconhecido\n");
        }
        user_input[0] = '\0';
    }
}