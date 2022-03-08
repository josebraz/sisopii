all: server client

server: server.obj
	g++ -o build/server main.o logs.o persistence.o communication_manager.o notification_manager.o session_manager.o communication_utils.o -lpthread
	
server.obj: server/main.cpp logs.cpp communication_utils.cpp server/persistence.cpp server/communication_manager.cpp server/notification_manager.cpp server/session_manager.cpp
	g++ -c server/main.cpp logs.cpp communication_utils.cpp server/persistence.cpp server/communication_manager.cpp server/notification_manager.cpp server/session_manager.cpp
	
client: client.obj
	g++ -o build/client main.o communication_manager.o notification_manager.o communication_utils.o presentation.o logs.o -lpthread
	
client.obj: client/main.cpp client/communication_manager.cpp client/presentation.cpp client/notification_manager.cpp communication_utils.cpp logs.cpp
	g++ -c client/main.cpp client/communication_manager.cpp client/presentation.cpp client/notification_manager.cpp communication_utils.cpp logs.cpp

tests: tests.obj
	g++ -o build/tests test.o logs.o persistence.o communication_utils.o -lpthread

tests.obj:
	g++ -c tests/test.cpp logs.cpp communication_utils.cpp server/persistence.cpp -DTEST 
	
clean: 
	rm -f server.o client.o logs.o test.o communication_utils.o persistence.o notification_manager.o presentation.o communication_manager.o build/* /tests/temp/*.bin
	
