#include "pthread.h"

#include "server_comm_manager.hpp"
#include "session_manager.hpp"
#include "../constants.h"

int main()
{
    init_session_manager();

    pthread_t server_thread = start_server(SERVER_PORT);

    pthread_join(server_thread, NULL);

    return 0;
}