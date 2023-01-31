all: mcast client
mcast: mcast.c
	gcc -pthread -Wall -o mcast mcast.c
client: client.c
	gcc -pthread -Wall -o client client.c
clean:
	rm -f *.o *~ mcast client