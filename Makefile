all: server client

server: server.obj
	g++ -o build/server server.o logs.o persistence.o -lpthread
	
server.obj: server/server.cpp logs.c server/persistence.cpp
	g++ -c server/server.cpp logs.c server/persistence.cpp
	
client: client.obj
	gcc -o build/client client.o logs.o -lpthread
	
client.obj: client/client.c logs.c
	gcc -c client/client.c logs.c

tests: tests.obj
	g++ -o build/tests test.o logs.o persistence.o -lpthread

tests.obj:
	g++ -c tests/test.cpp logs.c server/persistence.cpp -DTEST 
	
clean: 
	rm -f server.o client.o logs.o test.o build/server build/client
	
