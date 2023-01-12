#include <stdio.h>          /* These are the usual header files */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define BACKLOG 2   /* Number of allowed connections */
#define BUFF_SIZE 1024

int stringHandle(char* s, char* n, char* c)
{
    int j=0, k=0;
    for(int i=0; i<strlen(s); i++) {
        if(s[i]>='0' && s[i]<='9')
        {
            n[j] = s[i]; 
            j++;
        }
        else if((s[i]>='a' && s[i]<='z') || (s[i]>='A' && s[i]<='Z')) {
            c[k] = s[i];
            k++;
        }
        else if(s[i]== '\n') continue;
        else return -1;
    }
    n[j] = '\0';
    c[k] = '\0';
    return 0;
}

void fileWrite(char* filename, char* buff){
    FILE* f = fopen(filename,"w");
    int i=0;
    do {
        fprintf(f,"%c",buff[i]);
        i++;
    } while (i<strlen(buff));
    fclose(f);
}
int main(int argc, char** argv)
{
    int PORT;
	int listen_sock, conn_sock; /* file descriptors */
	char recv_data[BUFF_SIZE];
	int bytes_sent, bytes_received;
	struct sockaddr_in server; /* server's address information */
	struct sockaddr_in client; /* client's address information */
	int sin_size;
	
    char c[1000], n[1000];
    char filename[100]; 
    
    //Read Port number
    if(argc != 2){
        printf("Input again.\n");
        exit(0);
    }
    PORT = atoi(argv[1]);

	//Step 1: Construct a TCP socket to listen connection request
	if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){  /* calls socket() */
		perror("\nError: ");
		return 0;
	}
	
	//Step 2: Bind address to socket
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;         
	server.sin_port = htons(PORT);   /* Remember htons() from "Conversions" section? =) */
	server.sin_addr.s_addr = htonl(INADDR_ANY);  /* INADDR_ANY puts your IP address automatically */   
	if(bind(listen_sock, (struct sockaddr*)&server, sizeof(server))==-1){ /* calls bind() */
		perror("\nError: ");
		return 0;
	}     
	
	//Step 3: Listen request from client
	if(listen(listen_sock, BACKLOG) == -1){  /* calls listen() */
		perror("\nError: ");
		return 0;
	}
	
	//Step 4: Communicate with client
	while(1){
		//accept request
		sin_size = sizeof(struct sockaddr_in);
		if ((conn_sock = accept(listen_sock,( struct sockaddr *)&client, &sin_size)) == -1) 
			perror("\nError: ");
		printf("You got a connection from %s\n", inet_ntoa(client.sin_addr) ); /* prints client's IP */
		while (1) {
            bzero(recv_data,BUFF_SIZE);
            bytes_received = recv(conn_sock, recv_data, BUFF_SIZE-1, 0); //blocking
            if (bytes_received <= 0){
                printf("\nConnection closed");
                break;
            }
            else{
                recv_data[bytes_received] = '\0';
                printf("Receive: %s\n", recv_data);
            }

            if(strcmp(recv_data,"@1")==0)
            {
                bytes_sent = send(conn_sock, "$OK", 3, 0 );
                while(1){
                    //receives message from client
                    bzero(recv_data,BUFF_SIZE);
                    bytes_received = recv(conn_sock, recv_data, BUFF_SIZE-1, 0); //blocking
                    if (bytes_received <= 0){
                        printf("\nConnection closed");
                        break;
                    }
                    else{
                        recv_data[bytes_received] = '\0';
                        printf("Receive: %s\n", recv_data);
                    }
                    if(strcmp(recv_data,"@close")==0) {
                        bytes_sent = send(conn_sock, "$close", 3, 0 );
                        break;
                    }
                    if(stringHandle(recv_data,n,c)==0) {
                        bytes_sent = send(conn_sock, n, sizeof(n), 0 );
                        if (bytes_sent <= 0){
                            printf("\nConnection closed");
                            break;
                        }
                        bytes_received = recv(conn_sock, recv_data, BUFF_SIZE-1, 0); //blocking
                        if (bytes_received <= 0){
                            printf("\nConnection closed");
                            break;
                        }
                        recv_data[bytes_received] = '\0';
                        if(strcmp(recv_data,"@OK")==0) {
                            bytes_sent = send(conn_sock, c, sizeof(c), 0 );
                            if (bytes_sent <= 0){
                                printf("\nConnection closed");
                                break;
                            }
                        }
                    }
                    else {
                        bytes_sent = send(conn_sock, "$err", 4, 0);
                        if (bytes_sent <= 0){
                            printf("\nConnection closed");
                            break;
                        }
                    }
                }
            }
            else if(strcmp(recv_data,"@2")==0)
            {
                bytes_sent = send(conn_sock,"$OK",3,0);

                bytes_received = recv(conn_sock, recv_data, BUFF_SIZE-1, 0); //blocking
                if (bytes_received <= 0){
                    printf("\nConnection closed");
                    break;
                }
                recv_data[bytes_received] = '\0';
                printf("Receive: %s\n", recv_data);
                
                strcat(recv_data,"_recv");
                strcpy(filename,recv_data);
                
                bytes_sent = send(conn_sock,"$file",5,0);

                bytes_received = recv(conn_sock, recv_data, BUFF_SIZE-1, 0); //blocking
                if (bytes_received <= 0){
                    printf("\nConnection closed");
                    break;
                }
                recv_data[bytes_received-1] = '\0';

                fileWrite(filename,recv_data);
                
                bytes_sent = send(conn_sock, recv_data, sizeof(recv_data), 0 );
            }
            else break;
        }
	    close(conn_sock);	
    }
	close(listen_sock);
	return 0;
}
