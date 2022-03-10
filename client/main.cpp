
#include <string>

#include "client_comm_manager.hpp"
#include "presentation.hpp"

using namespace std;

int main(int argc, char **argv)
{
    if (argc != 4) {
        printf("Erro de sintaxe, use: %s username server port\n", argv[0]);
        exit(1);
    }

    char *my_user = argv[1];
    char *server_addr = argv[2];
    int port = atoi(argv[3]);

    start_client(server_addr, port);

    send_login_msg(my_user);

    start_presentation();

    return 0;
}