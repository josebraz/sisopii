#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "presentation.hpp"
#include "../constants.h"
#include "../logs.hpp"
#include "communication_manager.hpp"

void start_presentation() {
    char user_input[PAYLOAD_MAX_SIZE];

    while (strcmp(user_input, "") != 0)
    {
        printf("Digite um comando: ");
        scanf("%s", user_input);
        send_message(user_input);
    }
}