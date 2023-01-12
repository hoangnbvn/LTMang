CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 

all: server client

server: server.o fields.o dllist.o jval.o 
	${CC} -o server server.o fields.o dllist.o jval.o 

server.o: server.c
	${CC} ${CFLAGS} server.c

# all: test

# test: test.o fields.o dllist.o jval.o 
# 	${CC} -o test test.o fields.o dllist.o jval.o

dllist.o: dllist.c
	${CC} ${CFLAGS} dllist.c

fields.o: fields.c
	${CC} ${CFLAGS} fields.c

jval.o: jval.c
	${CC} ${CFLAGS} jval.c

client: client.o 
	${CC} -o client client.o 

client.o: client.c
	${CC} ${CFLAGS} client.c

clean:
	rm -f *.o *~
	rm -f server
	rm -f client