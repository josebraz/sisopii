#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "presentation.hpp"
#include "../constants.h"
#include "../logs.hpp"
#include "communication_manager.hpp"

void start_presentation() {
    char user_input[PAYLOAD_MAX_SIZE];

    while (1) {
        printf("Digite um comando: ");
        scanf("%s", user_input);

        if (strlen(user_input) == 0) {
            continue;
        }

        if (strncmp("FOLLOW", user_input, 6) == 0) {
            send_follow_msg(user_input + 7);
        } else if (strncmp("SEND", user_input, 4) == 0) {
            send_notify_msg(user_input + 5);
        } else if (strncmp("EXIT", user_input, 4) == 0) {
            printf("BYE\n");
            break;
        } else {
            printf("Comando não reconhecido\n");
        }
    }
}