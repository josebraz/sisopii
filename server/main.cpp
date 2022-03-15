#include "pthread.h"

#include "server_comm_manager.hpp"
#include "server_notif_manager.hpp"
#include "session_manager.hpp"
#include "../constants.h"

int main()
{
    printf("Iniciando o server...\n");
    init_session_manager();

    pthread_t notif_thread = start_server_notif_mng(&server_send_notif);
    pthread_t server_thread = start_server(SERVER_PORT);

    printf("Server iniciado com sucesso!\n");

    pthread_join(server_thread, NULL);
    pthread_join(notif_thread, NULL);

    finalize_session_manager();

    printf("Server finalizado\n");

    return 0;
}