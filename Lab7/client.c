#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#define BUFF_SIZE 1024

int client_sock;

void sig_child(int signo){
    pid_t pid;
    int stat;
    while((pid = waitpid(-1, &stat, WNOHANG)) > 0){
        printf("Process %d terminated\n", pid);
    }
}

void listenServer(){
	char buff[BUFF_SIZE]; 
	int bytes_sent, bytes_received;
    do{
            bzero(buff,BUFF_SIZE);
            bytes_received =  recv(client_sock, buff, BUFF_SIZE, 0); 
            buff[bytes_received] = '\0';
            if(strcmp(buff,"@bye")==0) break;
            puts(buff);
    }while(1);
}

void sendServer(){
    char mess[100];
    int bytes_sent, msg_len;    
    do{
        memset(mess,'\0',(strlen(mess)+1));
        fgets(mess, 100, stdin);		
        msg_len = strlen(mess);
        mess[strlen(mess)-1] = '\0';

	    bytes_sent = send(client_sock, mess, msg_len, 0);
	    if(bytes_sent < 0) perror("\nError: ");
    }while(strcmp(mess,"bye")!=0);
}
int main(int argc, char** argv){
	pid_t pid; int stat;
    char* SERVER_ADDR; int SERVER_PORT;
	char buff[BUFF_SIZE]; 
	struct sockaddr_in server_addr; /* server's address information */
	int msg_len, bytes_sent, bytes_received;

    char login[100]; char mess[100];

    // Read IP address and Port number
    if(argc < 5){
        printf("Server connection failed! Usage: %s [host_ip_address] [port_number] [username] [password]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    SERVER_ADDR = argv[1];
    SERVER_PORT = atoi(argv[2]);
    strcpy(login,argv[3]);
    strcat(login," ");
    strcat(login,argv[4]);
    //Step 1: Construct socket
	client_sock = socket(AF_INET,SOCK_STREAM,0);

	//Step 2: Specify server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
	
	//Step 3: Request to connect server
	if(connect(client_sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0){
		printf("\nError!Can not connect to sever! Client exit imediately! ");
		return 0;
	}
		
	//Step 4: Communicate with server	
    bytes_sent = send(client_sock,login,strlen(login),0);	
    if(bytes_sent < 0) perror("\nError: ");	

    bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
	if (bytes_received < 0)
			perror("\nError: ");
	else if (bytes_received == 0)
			printf("Connection closed.\n");	
    buff[bytes_received] = '\0';

    if(strcmp(buff,"@err")==0)
    {
        printf("Login false.\n");
    	close(client_sock);
	    return 0;
    }
    else{
        puts(buff); // put chat history
    }
    signal(SIGCHLD, sig_child);
    if((pid = fork()) == 0){ // Recveice fromm server
        do{
            bzero(buff,BUFF_SIZE);
            bytes_received =  recv(client_sock, buff, BUFF_SIZE, 0); 
            buff[bytes_received] = '\0';
            if(strcmp(buff,"@bye")==0) break;
            puts(buff);
        }while(1);
        printf("Client exit\n");
        close(client_sock);
        exit(0);
    }
    else { // Send to server
        do{
        memset(mess,'\0',(strlen(mess)+1));
        fgets(mess, 100, stdin);		
        msg_len = strlen(mess);
        mess[strlen(mess)-1] = '\0';

	    bytes_sent = send(client_sock, mess, msg_len, 0);
	    if(bytes_sent < 0) perror("\nError: ");
        }while(strcmp(mess,"bye")!=0);
    }
	//Step 5: Close socket
	close(client_sock);
	return 0;
}
