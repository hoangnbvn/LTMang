// Message $$: request that client send server to get data from server
// Message @@: request that server send client to annouce error
// Message !!: message that server send client to annouce that server has no input string 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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
        else return -1;
    }
    n[j] = '\0';
    c[k] = '\0';
    return 0;
}

int main(int argc, char** argv)
{ 
	int server_sock; /* file descriptors */
	int PORT;
    char buff[BUFF_SIZE]; 
	int bytes_sent, bytes_received;
	struct sockaddr_in server; /* server's address information */
	struct sockaddr_in client, client1, client2; /* clients's address information */
	int sin_size;
 
    char n[1000], c[1000];
    char msg1[1000][1000], msg2[1000][1000]; // Save msg from client1 client2
    int i1=0,i2=0; // Save msg off-set

    char* emty_msg = "!!";
    char* err_msg = "@@";
    //Read Port number
    if(argc != 2){
        printf("Input again.\n");
        exit(0);
    }
    PORT = atoi(argv[1]);

	//Step 1: Construct a UDP socket
	if ((server_sock=socket(AF_INET, SOCK_DGRAM, 0)) == -1 ){  /* calls socket() */
		perror("\nError: ");
		exit(0);
	}
	
	//Step 2: Bind address to socket
	server.sin_family = AF_INET;         
	server.sin_port = htons(PORT);   /* Remember htons() from "Conversions" section? =) */
	server.sin_addr.s_addr = INADDR_ANY;  /* INADDR_ANY puts your IP address automatically */   
	bzero(&(server.sin_zero),8); /* zero the rest of the structure */

	bzero(&client1,sizeof(client1)); 
	bzero(&client2,sizeof(client2));
  
	if(bind(server_sock,(struct sockaddr*)&server,sizeof(struct sockaddr))==-1){ /* calls bind() */
		perror("\nError: ");
		exit(0);
	}     
	
	//Step 3: Communicate with clients
	while(1){
		sin_size=sizeof(struct sockaddr_in);	
		bytes_received = recvfrom(server_sock, buff, BUFF_SIZE-1, 0, (struct sockaddr *) &client, &sin_size);
		buff[bytes_received] = '\0';
		printf("[%s:%d]: %s\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port), buff);

		if (bytes_received < 0)
			perror("\nError: ");
		else{

            // Save current client's info
            // When client1 = 0 -> save client's info to client1
            if(ntohs(client1.sin_port)==0) {
                client1.sin_family = AF_INET;
				client1.sin_port = client.sin_port;
				client1.sin_addr.s_addr = INADDR_ANY ;
				printf("new client1: [%s:%d]\n", inet_ntoa(client1.sin_addr), ntohs(client1.sin_port));
				if(strcmp(buff,"$$")!=0){ 
					strcpy(msg1[i1++],buff);
					continue;
				}
            }
            // When client1 != client and client2 = 0 -> save client's info to client2
            else if(ntohs(client1.sin_port)!=ntohs(client.sin_port) && ntohs(client2.sin_port)==0) {
                client2.sin_family = AF_INET;
				client2.sin_port = client.sin_port;
				client2.sin_addr.s_addr = INADDR_ANY ;
				printf("new client2: [%s:%d]\n", inet_ntoa(client2.sin_addr), ntohs(client2.sin_port));
				if(strcmp(buff,"$$")!=0){ 
					strcpy(msg2[i2++],buff); 
					continue;
				}               
            }
        
            // Client's request processing
            // When server received get-data request 
            if(strcmp(buff,"$$")==0) {

                if(ntohs(client1.sin_port)==ntohs(client.sin_port)) {
                    if(i2==0) {
                        bytes_sent = sendto(server_sock, (char*)emty_msg, strlen(emty_msg), 0, (struct sockaddr*)&client1, sin_size);
                        if (bytes_sent < 0) perror("\nError: ");
					    continue;
                    }
                    else {
                        if(stringHandle(msg2[i2-1],n,c)<0) {
                            bytes_sent = sendto(server_sock,(char*)err_msg,strlen(err_msg),0,(struct sockaddr*)&client1,sin_size);
                            if (bytes_sent < 0) perror("\nError: ");
					        continue;
                        }
                        else {
                            bytes_sent = sendto(server_sock,(char*)n,strlen(n),0,(struct sockaddr*)&client1,sin_size);
                            if (bytes_sent < 0) perror("\nError: ");
                            bytes_sent = sendto(server_sock,(char*)c,strlen(c),0,(struct sockaddr*)&client1,sin_size);
                            if (bytes_sent < 0) perror("\nError: ");
					        continue;
                        }
                    }
                }
                if(ntohs(client2.sin_port)==ntohs(client.sin_port)) {
                    if(i1==0) {
                        bytes_sent = sendto(server_sock,(char*)emty_msg,strlen(emty_msg),0,(struct sockaddr*)&client2,sin_size);
                        if (bytes_sent < 0) perror("\nError: ");
					    continue;
                    }
                    else {
                        if(stringHandle(msg1[i1-1],n,c)<0) {
                            bytes_sent = sendto(server_sock,(char*)err_msg,strlen(err_msg),0,(struct sockaddr*)&client2,sin_size);
                            if (bytes_sent < 0) perror("\nError: ");
					        continue;
                        }
                        else {
                            bytes_sent = sendto(server_sock,(char*)n,strlen(n),0,(struct sockaddr*)&client2,sin_size);
                            if (bytes_sent < 0) perror("\nError: ");
                            bytes_sent = sendto(server_sock,(char*)c,strlen(c),0,(struct sockaddr*)&client2,sin_size);
                            if (bytes_sent < 0) perror("\nError: ");
					        continue;
                        }
                    }
                }

            } 
            // When server received send-data request
            else {
                // save string to handle later
                if(ntohs(client1.sin_port)==ntohs(client.sin_port)) {
                    strcpy(msg1[i1++],buff);
                }
                if(ntohs(client2.sin_port)==ntohs(client.sin_port)) {
                    strcpy(msg2[i2++],buff);
                }
                continue;
            }
        }
        				
	}
        
	close(server_sock);
	return 0;
}
