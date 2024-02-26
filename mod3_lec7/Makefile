CC=gcc

CFLAGS=-Wall -pedantic

all: server client_lec

server: tcp_server.c
	$(CC) $(CFLAGS) tcp_server.c -o server

client_lec: tcp_client.c
	$(CC) $(CFLAGS) tcp_client.c -o client

clean:
	rm server client
