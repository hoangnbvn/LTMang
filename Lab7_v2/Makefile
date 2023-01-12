CC=gcc
CFLAGS=-O3 -Wall
LIBS=-pthread

all: server.out client.out
server.out: server.o linklist.o
	$(CC) $(CFLAGS) $(LIBS) -o server server.o linklist.o
client.out: client.o 
	$(CC) $(CFLAGS) $(LIBS) -o client client.o 
server.o: server.c
	$(CC) $(CFLAGS) -c server.c
linklist.o: linklist.h linklist.c
	gcc linklist.h -c linklist.c
client.o: client.c
	$(CC) $(CFLAGS) -c client.c

clean:
	rm -f *.o client server linklist.h.gch