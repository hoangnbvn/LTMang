CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 

all: server client

server: server.o 
	${CC} -o server server.o

server.o: server.c
	${CC} ${CFLAGS} server.c

client: client.o 
	${CC} -o client client.o 

client.o: client.c
	${CC} ${CFLAGS} client.c

clean:
	rm -f *.o *~ clientlog* server client 