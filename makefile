CC = gcc
CFLAGS = -Wall -g

COMMON_OBJS = obj/socket_handler.o obj/common_packets.o obj/basic_for_packets.o

all: server client

server: obj/server.o obj/server_tools.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o server obj/server.o obj/server_tools.o $(COMMON_OBJS)

client: obj/client.o obj/client_tools.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o client obj/client.o obj/client_tools.o $(COMMON_OBJS)

obj/server.o: src/server.c 
	$(CC) $(CFLAGS) -c $< -o $@ $(LFLAGS)

obj/client.o: src/client.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LFLAGS)

obj/%.o: src/%.c header/%.h
	$(CC) $(CFLAGS) -c $< -o $@ $(LFLAGS)

clean:
	rm -f obj/*.o

purge: clean
	rm -f server client
