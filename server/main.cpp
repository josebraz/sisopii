#include "pthread.h"

#include "server_comm_manager.hpp"
#include "server_notif_manager.hpp"
#include "session_manager.hpp"
#include "../constants.h"


/*
inicia um processo servidor
não recebe parametros alem do padrao que representa o caminho do executavel
*/
int main()
{
    printf("Iniciando o server...\n");

    // inicia uma secao lendo o arquivo binario onde estao guardados os historicos dos usuarios,
    // permitindo assim que o server tenha uma persistencia na interacao dos clientes
    init_session_manager();

    // alocamos threads para cuidar do funcionamento da permanencia do servidor online e para o gerenciamento de troca de notificacoes
    pthread_t notif_thread = start_server_notif_mng(&server_send_notif);
    pthread_t server_thread = start_server(SERVER_PORT);

    printf(GRN "Server iniciado com sucesso!" NC " \n");

    pthread_join(server_thread, NULL);
    pthread_join(notif_thread, NULL);

    finalize_session_manager();

    printf(GRN "Server finalizado" NC " \n");

    return 0;
}