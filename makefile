CC = gcc
<<<<<<< HEAD
objects = client server *.o
all: client server
     
client: client.o
	$(CC) -o client client.o $(LDFLAGS)

server: server.o
	$(CC) -o server server.o $(LDFLAGS)
=======
CFLAGS = -Wall -g

all: server client

server: obj/server.o obj/socket_handler.o
	$(CC) $(CFLAGS) -o server obj/server.o obj/socket_handler.o

client: obj/client.o obj/socket_handler.o
	$(CC) $(CFLAGS) -o client obj/client.o obj/socket_handler.o

obj/server.o: src/server.c
	$(CC) $(CFLAGS) -c src/server.c -o obj/server.o
>>>>>>> 63ffaa6 ([ADD] basico)

obj/client.o: src/client.c
	$(CC) $(CFLAGS) -c src/client.c -o obj/client.o

obj/socket_handler.o: src/socket_handler.c
	$(CC) $(CFLAGS) -c src/socket_handler.c -o obj/socket_handler.o

clean:
	rm -f server client obj/*.o
