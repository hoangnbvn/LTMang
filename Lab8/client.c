#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define LENGTH_NAME 31
#define LENGTH_MSG 101
#define LENGTH_SEND 201
#define BUFF_SIZE 1024
// Global variables
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char nickname[LENGTH_NAME] = {};

void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}
void str_trim_lf (char* arr, int length) {
    int i;
    for (i = 0; i < length; i++) { // trim \n
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

int main(int argc, char** argv)
{
	char buff[BUFF_SIZE]; 
	int msg_len, bytes_sent, bytes_received;
    
    signal(SIGINT, catch_ctrl_c_and_exit);

    if(argc < 3){
        printf("Server connection failed! Usage: %s [host_ip_address] [port_number] [username] [password]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Naming
    printf("Please enter your name: ");
    if (fgets(nickname, LENGTH_NAME, stdin) != NULL) {
        str_trim_lf(nickname, LENGTH_NAME);
    }
    if (strlen(nickname) >= LENGTH_NAME-1) {
        printf("\nName must be  less than thirty characters.\n");
        exit(EXIT_FAILURE);
    }

    // Create socket
    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (sockfd == -1) {
        printf("Fail to create a socket.");
        exit(EXIT_FAILURE);
    }

    // Socket information
    struct sockaddr_in server_info, client_info;
    int s_addrlen = sizeof(server_info);
    int c_addrlen = sizeof(client_info);
    memset(&server_info, 0, s_addrlen);
    memset(&client_info, 0, c_addrlen);
    server_info.sin_family = PF_INET;
    server_info.sin_addr.s_addr = inet_addr(argv[1]);
    server_info.sin_port = htons(atoi(argv[2]));

    // Connect to Server
    int err = connect(sockfd, (struct sockaddr *)&server_info, s_addrlen);
    if (err == -1) {
        printf("Connection to Server error!\n");
        exit(EXIT_FAILURE);
    }
    
    // Names
    getsockname(sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);
    getpeername(sockfd, (struct sockaddr*) &server_info, (socklen_t*) &s_addrlen);
    printf("Connect to Server: %s:%d\n", inet_ntoa(server_info.sin_addr), ntohs(server_info.sin_port));
    printf("You are: %s:%d\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));

    send(sockfd, nickname, LENGTH_NAME, 0);

    while (1) {
        printf("Input message to server (input '@bye' to close client): ");
        memset(buff,'\0',(strlen(buff)+1));
        fgets(buff, BUFF_SIZE, stdin);		
        msg_len = strlen(buff);
        str_trim_lf(buff,msg_len);
        if(msg_len == 0)  break;

        bytes_sent = send(sockfd, buff, msg_len, 0);
        if(bytes_sent <= 0){
            printf("\nConnection closed!\n");
            break;
        }    
        if (strcmp(buff, "@bye") == 0) {
            break;
        }

        bytes_received = recv(sockfd, buff, BUFF_SIZE, 0);
        if (bytes_received < 0)
                perror("\nError: ");
        else if (bytes_received == 0)
                printf("Connection closed.\n");	
        buff[bytes_received] = '\0';
        printf("Reply from server: %s\n",buff);
        if(flag) {
            printf("Bye\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}