CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 

all: main

main: main.o fields.o dllist.o jval.o
	${CC} -o main main.o fields.o dllist.o jval.o 

main.o: main.c 
	${CC} ${CFLAGS} main.c

dllist.o: dllist.c
	${CC} ${CFLAGS} dllist.c

fields.o: fields.c
	${CC} ${CFLAGS} fields.c

jval.o: jval.c
	${CC} ${CFLAGS} jval.c

clean:
	rm -f *.o *~