CC = gcc
<<<<<<< HEAD
<<<<<<< HEAD
objects = client server *.o
all: client server
     
client: client.o
	$(CC) -o client client.o $(LDFLAGS)

server: server.o
	$(CC) -o server server.o $(LDFLAGS)
=======
=======
>>>>>>> 4bd741a (Organização pro github)
CFLAGS = -Wall -g

COMMON_OBJS = obj/socket_handler.o obj/common_packets.o obj/basic_for_packets.o

all: server client

server: obj/server.o obj/server_tools.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o server obj/server.o obj/server_tools.o $(COMMON_OBJS)

client: obj/client.o obj/client_tools.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o client obj/client.o obj/client_tools.o $(COMMON_OBJS)

<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
obj/server.o: src/server.c
	$(CC) $(CFLAGS) -c src/server.c -o obj/server.o
>>>>>>> 63ffaa6 ([ADD] basico)

obj/client.o: src/client.c
	$(CC) $(CFLAGS) -c src/client.c -o obj/client.o

obj/socket_handler.o: src/socket_handler.c
	$(CC) $(CFLAGS) -c src/socket_handler.c -o obj/socket_handler.o
=======
obj/%.o: src/%.c
=======
=======
>>>>>>> 4bd741a (Organização pro github)
obj/server.o: src/server.c 
	$(CC) $(CFLAGS) -c $< -o $@ $(LFLAGS)

obj/client.o: src/client.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LFLAGS)

obj/%.o: src/%.c header/%.h
<<<<<<< HEAD
>>>>>>> 6c02a62 ([FIX] validação dos pacotes e makefile)
	$(CC) $(CFLAGS) -c $< -o $@ $(LFLAGS)
>>>>>>> fa95a3d ([ADD] CRC-8 e send_ack e nack (não funcionam ainda))
=======
	$(CC) $(CFLAGS) -c $< -o $@ $(LFLAGS)
>>>>>>> 4bd741a (Organização pro github)

clean:
	rm -f server client obj/*.o
