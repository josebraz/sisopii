#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "presentation.hpp"
#include "../types.hpp"
#include "../constants.h"
#include "../logs.hpp"
#include "client_comm_manager.hpp"

#define NC "\e[0m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define CYN "\e[0;36m"
#define REDB "\e[41m"

char *my_user_g;

void presentation_sig_handler(sig_atomic_t sig) {
    char result[100];
    int ret = send_logout_msg(my_user_g, result);
    if (ret == 1) {
        printf("\nSessão encerrada, até a próxima " CYN ":)" NC " \n");
    } else {
        printf(RED "\nERRO:\n");
        printf(NC "Impossível encerrar a sessão: %s\n", result);
    }
    exit(1);
}

void start_presentation(char *my_user) {
    my_user_g = my_user;
    signal(SIGINT, presentation_sig_handler);

    int ret;
    size_t bufsize = PAYLOAD_MAX_SIZE;
    size_t characters;
    char *user_input = new char[bufsize];
    char result[PAYLOAD_MAX_SIZE];

    if (send_login_msg(my_user, result) == 0) {
        printf(RED "Erro no login: " NC "(%s)!\nFinalizando...", result);
        exit(1);
        return;
    }

    printf("Bem vindo, %s!\n", my_user);

    while (1) {
        characters = getline(&user_input, &bufsize, stdin);
        if (bufsize == 0) continue;

        user_input[characters-1] = '\0'; // elimina o \n

        if (strncmp("FOLLOW", user_input, 6) == 0) {
            ret = send_follow_msg(user_input + 7, result);
        } else if (strncmp("UNFOLLOW", user_input, 6) == 0) {
            ret = send_unfollow_msg(user_input + 9, result);
        } else if (strncmp("SEND", user_input, 4) == 0) {
            ret = send_notify_msg(user_input + 5, result);
        } else if (strncmp("EXIT", user_input, 4) == 0) {
            ret = send_logout_msg(my_user, result);
            printf("BYE\n");
            break;
        } else {
            printf(RED "ERRO:" NC "Comando não reconhecido" NC " \n");
            continue;
        }

        if (ret == 0) {
            printf(RED "ERRO: " NC "%s\n", result);
        } else {
            printf(GRN "OK" NC " \n");
        }

        user_input[0] = '\0';
    }

    free(user_input);
}

void on_new_notification(notification *notif) {
    time_t raw_time = (time_t) (notif->timestamp);
    struct tm *time_info = localtime(&raw_time);
    char time_buffer[80];
    strftime(time_buffer, 80, "%d/%m/%Y - %H:%M", time_info);

    printf(GRN "Nova mensagem!\n" CYN "(%s) %s: " NC "%s\n", time_buffer, notif->author, notif->message);
}