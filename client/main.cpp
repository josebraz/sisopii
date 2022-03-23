
#include <string>

#include "client_comm_manager.hpp"
#include "presentation.hpp"
#include "../constants.h"

/*
inicia um processo cliente
    recebe como argumento 3 parametros, alem do padrao que indica o caminho da aplicacao:
    1.  username    ->  fulano
    2.  server      ->  localhost
    3.  port        ->  8080
*/
int main(int argc, char **argv)
{
    // se não recebemos exatamente 4 argumentos (caminho + 3 parametros), jogamos um erro e terminamos o processo
    if (argc != 4) {
        printf(RED "Erro de sintaxe.\n");
        printf(NC "Use: path username server port\n");
        printf(NC "Ex.: " GRN "%s fulaninho localhost 8080" NC " \n", argv[0]);
        exit(1);
    }

    // caso tenhamos todos os dados necessarios, os atribuimos as variaveis pertinentes
    char *my_user = argv[1];        // username do cliente em questao
    char *server_addr = argv[2];    // server que o cliente está se conectando
    int port = atoi(argv[3]);       // porta usada para a comunicacao cliente <-> servidor


    start_client(server_addr, port);

    start_presentation(my_user);

    return 0;
}