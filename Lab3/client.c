// Message $$: request that client send server to get data from server
// Message @@: request that server send client to annouce error
// Message !!: message that server send client to annouce that server has no input string 

#include <stdio.h>          /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFF_SIZE 1024

int main(int argc, char** argv){
	int client_sock;
    char* SERV_IP; int SERV_PORT;
	char buff[BUFF_SIZE];
	struct sockaddr_in server_addr;
	int bytes_sent,bytes_received, sin_size;
    char* get_msg = "$$";

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
    // Choose client's action
    // 1: Send data 
    // 2: Get data
    sin_size = sizeof(struct sockaddr);
    int choice;
    while (1)
    {
		printf("Input 1 or 2:\n");
		printf("1. Send data to server\n");
		printf("2. Get data from server\n");
		printf("Input an empty string to exit\n");

        scanf("%d",&choice);
        switch (choice)
        {
        case 1:
            printf("Insert string to send:");
            memset(buff,'\0',strlen(buff)+1); 
            fflush(stdin);
	        fgets(buff, BUFF_SIZE, stdin);
            //Check emty string
            if(strlen(buff)==1 && buff[0]=='\n') {
                printf("Close.\n");
                close(client_sock);
                exit(0);
            }
            buff[strlen(buff)-1] = '\0';
            bytes_sent = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr *) &server_addr, sin_size);
            if(bytes_sent < 0){
                perror("Error: ");
                close(client_sock);
                return 0;
            }
            break;
        case 2: 
            //Send msg "$$" to server
            bytes_sent = sendto(client_sock, (char*)get_msg, strlen(get_msg), 0, (struct sockaddr*) &server_addr, sin_size);
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
            // Process received msg
            // Error msg
            if(buff[0]=='@')printf("Error\n");
            // Emty msg
            else if(buff[0]=='!')printf("No input string.\n");
            // Normal msg
            else {
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
            }
            break;
        default:
            break;
        }
    }   	
	close(client_sock);
	return 0;
}