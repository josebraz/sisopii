all: server client

server: server.obj
	gcc -o build/server server.o logs.o -lpthread
	
server.obj: server.c logs.c
	gcc -c server.c logs.c
	
client: client.obj
	gcc -o build/client client.o logs.o -lpthread
	
client.obj: client.c logs.c
	gcc -c client.c logs.c
	
clean: 
	rm server.o client.o logs.o build/server build/client
	
