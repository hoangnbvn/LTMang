#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>

#define MAX 1000 // MAX OF CHARACTER OF MESSAGE

int sockfd; // connect socket
int interrup = 0;

struct sockaddr_in server_addr; // server infomation
fd_set checkfd;
char *StdinBuffer;
void sendInfo(){
    char *message = calloc(MAX, sizeof(char));
    bzero(message, MAX);
    strcpy(message, "Hello");
    send(sockfd, message, strlen(message), 0);
}

void *recevieInfo(){
    pthread_detach(pthread_self());
    while(1){
        while(interrup == 0){
            char *buffer = calloc(MAX, sizeof(char));
            int buffer_size;
            bzero(buffer, MAX);
            buffer_size = recv(sockfd, buffer, MAX, 0);
            buffer[buffer_size] = '\0';
            if(strcmp(buffer, "Exit") == 0 || buffer_size < 0){
                close(sockfd);
                return NULL;
            }
            sendInfo();
        }
    }
    return NULL;
}

void setupClient(const char *ip, int port){ // setup client
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){ // setup socket
        perror("Socket creation failed\n");
        exit(EXIT_FAILURE);
    }
    bzero(&server_addr, sizeof(server_addr));
    // bind server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);
    //connect to server
    if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0){
        perror("Connection with the server failed\n");
        exit(EXIT_FAILURE);
    }
}

void client_handle(){
    while(1){
        bzero(StdinBuffer, MAX);
        int buffer_size = 0;
        buffer_size = read(STDIN_FILENO, StdinBuffer, MAX);
        if(buffer_size == -1){
            perror("Error stdin\n");
            exit(EXIT_FAILURE);
        }else{
            StdinBuffer[buffer_size - 1] = '\0';
            if(strcmp(StdinBuffer, "stop") == 0){
                interrup = 1;
            }else if(strcmp(StdinBuffer, "continue") == 0){
                interrup = 0;
            }
        }
    }
}
int main(int argc, char const *argv[])  {
    if(argc < 3){
        printf("Server connection failed! Usage: %s [host_ip_address] [port_number]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[2]);
    setupClient(argv[1], port);
    pthread_t tid;
    pthread_create(&tid, NULL, recevieInfo, NULL);
    StdinBuffer = calloc(MAX, sizeof(char));
    printf("Enter 'stop' to stop send/recv, 'continue' is vice versa: \n");
    client_handle();

    return 0;
}