all: server client

server: server.obj
	g++ -o build/server server.o logs.o persistence.o -lpthread
	
server.obj: server.cpp logs.c persistence.cpp
	g++ -c server.cpp logs.c persistence.cpp
	
client: client.obj
	gcc -o build/client client.o logs.o -lpthread
	
client.obj: client.c logs.c
	gcc -c client.c logs.c
	
clean: 
	rm server.o client.o logs.o build/server build/client
	
