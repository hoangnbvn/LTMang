//Message from client
// $gpass: get encryted pass
// $bye: signout 

//Message from server
// @wname: cannot find inputed username
// @wpass: wrong pass
// @err: error pass

#include <stdio.h>          /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFF_SIZE 1024

void Menu(){
    printf("\nUSER MANAGEMENT PROGRAM\n");
    printf("-----------------------------------\n");
    printf("1. Sign in\n");
    printf("2. Get encrypted pasword\n");
    printf("Input 'bye' to sign out\n");
}

int main(int argc, char** argv){
	int client_sock;
    char* SERV_IP; int SERV_PORT;
	char buff[BUFF_SIZE];
	struct sockaddr_in server_addr;
	int bytes_sent,bytes_received, sin_size;
    
    char* bye = "$bye";
    char* gpass = "$gpass";

    // Read IP address and Port number
    if(argc != 3) {
        printf("Input again.\n");

        exit(0);
    }
    SERV_IP = argv[1];
    SERV_PORT = atoi(argv[2]);

	//Step 1: Construct a UDP socket
	if ((client_sock=socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){  /* calls socket() */
		perror("\nError: ");
		exit(0);
	}

	//Step 2: Define the address of the server
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERV_PORT);
	server_addr.sin_addr.s_addr = inet_addr(SERV_IP);
	
	//Step 3: Communicate with server

    sin_size = sizeof(struct sockaddr);
    int choice;
    while (1)
    {
		Menu();
        scanf("%d",&choice);
        switch (choice)
        {
        case 1:
            printf("Input user name: ");
            memset(buff,'\0',strlen(buff)+1); fflush(stdin);
	        fgets(buff, BUFF_SIZE, stdin);
            buff[strlen(buff)-1] = '\0';
            bytes_sent = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr *) &server_addr, sin_size);
            if(bytes_sent < 0){
                perror("Error: ");
                close(client_sock);
                return 0;
            }
            //if can find username
            bytes_received = recvfrom(client_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *) &server_addr, &sin_size);
            buff[bytes_received] = '\0';
            if (strcmp(buff,"@cname")==0){
                for(int i=0; i<3; i++) {
                    printf("Input password: ");
                    memset(buff,'\0',strlen(buff)+1); 
                    fflush(stdin);
                    fgets(buff, BUFF_SIZE, stdin);
                    buff[strlen(buff)-1] = '\0';
                    bytes_sent = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr *) &server_addr, sin_size);
                    if(bytes_sent < 0){
                        perror("Error: ");
                        close(client_sock);
                        return 0;
                    }
                    bytes_received = recvfrom(client_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *) &server_addr, &sin_size);
                    buff[bytes_received] = '\0';
                    // not active or blocked account
                    if(strcmp(buff,"@nready")==0) {
                        printf("Account is not ready.\n");
                        break;
                    }
                    // correct pass
                    else if(strcmp(buff,"@cpass")==0) {
                        printf("OK.\n");
                        //change pass
                        while (1)
                        {
                            bytes_received = recvfrom(client_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *) &server_addr, &sin_size);
                            buff[bytes_received] = '\0';
                            if(strcmp(buff,"@err")==0) {
                                printf("Error.\n");
                                //close(client_sock);
                                break;
                            }
                            printf("Input new password: ");
                            memset(buff,'\0',strlen(buff)+1); fflush(stdin);
	                        fgets(buff, BUFF_SIZE, stdin);
                            buff[strlen(buff)-1] = '\0';
                            // check emty string
                            if(strlen(buff)==1 && buff[0]=='\n') {
                                printf("Close.\n");
                                close(client_sock);
                                exit(0);
                            }
                            // check bye string
                            if(strcmp(buff,"bye")==0) {
                                bytes_sent = sendto(client_sock, bye, strlen(bye), 0, (struct sockaddr *) &server_addr, sin_size);
                                bytes_received = recvfrom(client_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *) &server_addr, &sin_size);
                                buff[bytes_received] = '\0';
                                printf("Goodbye %s.\n",buff);
                                close(client_sock);
                                return 0;
                            }
                            bytes_sent = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr *) &server_addr, sin_size);
                            if(bytes_sent < 0){
                                perror("Error: ");
                                close(client_sock);
                                return 0;
                            }
                        }
                        break;
                    }
                    else if(strcmp(buff,"@wpass")==0) {
                        printf("Not OK.\n");
                        continue;
                    }
                    else if(strcmp(buff,"@block")==0) {
                        printf("Account is blocked.\n");
                        break;
                    }
                }
            }
            
            break;
        case 2: 
            //Send msg "$gpass" to server
            bytes_sent = sendto(client_sock, (char*)gpass, strlen(gpass), 0, (struct sockaddr*) &server_addr, sin_size);
            if(bytes_sent < 0){
                perror("Error: ");
                close(client_sock);
                return 0;
            }
            
            bytes_received = recvfrom(client_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *) &server_addr, &sin_size);
			if(bytes_received < 0){
				perror("Error: ");
				close(client_sock);
				return 0;
			}            
			buff[bytes_received] = '\0';
            // put num string
            printf("%s\n",buff);
			
            bytes_received = recvfrom(client_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *) &server_addr, &sin_size);
			if(bytes_received < 0){
				perror("Error: ");
				close(client_sock);
				return 0;
			}
			buff[bytes_received] = '\0';
            // put alpha string
			printf("%s\n",buff);          
            break;
        default:
            break;
        }
    }   	
	close(client_sock);
	return 0;
}