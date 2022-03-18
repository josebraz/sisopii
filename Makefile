all: server client

server: server.obj
	g++ -static -g -o build/server main.o logs.o persistence.o server_comm_manager.o server_notif_manager.o session_manager.o communication_utils.o -lpthread
	
server.obj: server/main.cpp logs.cpp communication_utils.cpp server/persistence.cpp server/server_comm_manager.cpp server/server_notif_manager.cpp server/session_manager.cpp
	g++ -static -g -c server/main.cpp logs.cpp communication_utils.cpp server/persistence.cpp server/server_comm_manager.cpp server/server_notif_manager.cpp server/session_manager.cpp
	
client: client.obj
	g++ -static -g -o build/client main.o client_comm_manager.o client_notif_manager.o communication_utils.o presentation.o logs.o -lpthread
	
client.obj: client/main.cpp client/client_comm_manager.cpp client/presentation.cpp client/client_notif_manager.cpp communication_utils.cpp logs.cpp
	g++ -static -g -c client/main.cpp client/client_comm_manager.cpp client/presentation.cpp client/client_notif_manager.cpp communication_utils.cpp logs.cpp

tests: tests/test.cpp logs.cpp communication_utils.cpp server/persistence.cpp server/server_comm_manager.cpp client/client_comm_manager.cpp server/session_manager.cpp server/server_notif_manager.cpp client/presentation.cpp
	g++ -static -g -o build/tests tests/test.cpp logs.cpp communication_utils.cpp server/persistence.cpp server/server_comm_manager.cpp client/client_comm_manager.cpp server/session_manager.cpp server/server_notif_manager.cpp client/presentation.cpp -DTEST -lpthread

clean: 
	rm -f server.o client.o logs.o test.o communication_utils.o persistence.o client_notif_manager.o server_notif_manager.o presentation.o server_comm_manager.o client_comm_manager.o build/* /tests/temp/*.bin
	
