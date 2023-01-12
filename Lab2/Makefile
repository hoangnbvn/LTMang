CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 

all: resolver

resolver: resolver.o 
	${CC} -o resolver resolver.o 

resolver.o: resolver.c 
	${CC} ${CFLAGS} resolver.c

clean:
	rm -f *.o *~