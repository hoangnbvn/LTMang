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
#include "linklist.h"

#define LENGTH_NAME 31
#define LENGTH_MSG 101
#define LENGTH_SEND 201

typedef struct _User{
    int status; 
    char username[31];
    char password[50];
}User;
llist *acc_list; // List account read by taikhoan.txt
User *CurrentUser; 

typedef struct ClientNode {
    int data; // sockfd
    struct ClientNode* prev;
    struct ClientNode* link;
    char ip[16]; // ip address
    char name[31]; // username
    // char pass[50]; // password
    // int status;
} ClientList;

// Global variables
int server_sockfd = 0, client_sockfd = 0;
ClientList *root, *now;
ClientList *newNode(int sockfd, char* ip) {
    ClientList *np = (ClientList *)malloc( sizeof(ClientList) );
    np->data = sockfd;
//    np->status = 0;
    np->prev = NULL;
    np->link = NULL;
    strncpy(np->ip, ip, 16);
    strncpy(np->name, "NULL", 5);
    // strncpy(np->pass, "NULL", 5);
    return np;
}


void SaveFile(char *string){ // Save message into file groupchat.txt
    FILE *pt = fopen("groupchat.txt", "a+");
    fprintf(pt, "%s\n", string);
    fclose(pt);
}

char *getSaveFile(){ // Read message in file groupchat.txt
    FILE *pt = fopen("groupchat.txt", "r");
    char *str = calloc(1000, sizeof(char));
    bzero(str, 1000);
    int length = 0;
    char c = fgetc(pt);
    while(c != EOF){
        str[length++] = c; 
        c = fgetc(pt);
    }
    str[strlen(str)] = '\0';
    fclose(pt);
    return str;
}

void readUser(){ // Read account from file account.txt
    FILE *pt = fopen("taikhoan.txt", "r");
    if(pt == NULL){
        printf("File was not found\n");
        exit(1);
    }
    int i = 0;
    char c = fgetc(pt);
    User *data = calloc(20, sizeof(User));
    while (c != EOF){
        if(c != '\n' && c != '\t' && c != ' ' && c != EOF){
            fseek(pt, -1, SEEK_CUR);
            fscanf(pt, "%s", data[i].username);
            fscanf(pt, "%s", data[i].password);
            fscanf(pt, "%d", &data[i].status);
            push_llist(acc_list, &data[i]);// add account into linklist
            i++;
        }
        c = fgetc(pt);
    }
    fclose(pt);
}

void splitLogin(char* login, char* name, char* pass){
    int i=0; int j=0; int k=0;
    do{
        name[k++] = login[i++];
    }while(login[i]!=' ');
    name[k]='\0';
    i++;
    do{
        pass[j++] = login[i++]; 
    }while(i<strlen(login));
    pass[j]='\0';
}

void catch_ctrl_c_and_exit(int sig) {
    ClientList *tmp;
    while (root != NULL) {
        printf("\nClose socketfd: %d\n", root->data);
        close(root->data); // close all socket include server_sockfd
        tmp = root;
        root = root->link;
        free(tmp);
    }
    printf("Bye\n");
    exit(EXIT_SUCCESS);
}

void send_to_all_clients(ClientList *np, char tmp_buffer[]) {
    ClientList *tmp = root->link;
    while (tmp != NULL) {
        if (np->data != tmp->data) { // all clients except itself.
            printf("Send to sockfd %d: \"%s\" \n", tmp->data, tmp_buffer);
            send(tmp->data, tmp_buffer, LENGTH_SEND, 0);
        }
        tmp = tmp->link;
    }
}


void client_handler(void *p_client) {
    int leave_flag = 0;
    char login[100], mess[100];
    char *chat = calloc(1000, sizeof(char));; // chat history
    char recv_buffer[LENGTH_MSG] = {};
    char send_buffer[LENGTH_SEND] = {};
    ClientList *np = (ClientList *)p_client;

    int bytes;
    
    char n[100], p[100];
    bytes = recv(np->data, login, 100, 0);
    splitLogin(login,n,p);
    int check = -1;
    struct node *current;
    current = *acc_list;
    while(current != NULL){
        User *tmp = (User *)current->data;
        if(strcmp(tmp->username, n) == 0){
            if(strcmp(tmp->password, p) == 0 && tmp->status != 0){
                check = 1;
                break;
            }else{
                if(tmp->status == 0){
                    check = 0; break;
                }
                check = -1; break;
            }   
        }
        current = current->next;
    }

    if(check == 1){ //Login success
        strcpy(np->name,n);
        printf("%s(%s)(%d) join the chatroom.\n", np->name, np->ip, np->data);
        sprintf(send_buffer, "%s join the chatroom.", np->name, np->ip);
        strcpy(mess,send_buffer);
        SaveFile(mess);
        chat = getSaveFile(); chat[strlen(chat)] = '\0';
        bytes = send(np->data, chat, 1000, 0);
         // Conversation
        while (1) {
            if (leave_flag) {
                break;
            }
            int receive = recv(np->data, recv_buffer, LENGTH_MSG, 0);
            if (receive > 0) {
                if (strlen(recv_buffer) == 0) {
                    continue;
                }
                sprintf(send_buffer, "%s：%s", np->name, recv_buffer);
                strcpy(mess,send_buffer);
                SaveFile(mess);
            } else if (receive == 0 || strcmp(recv_buffer, "bye") == 0) { // chat "bye" to exit room
                printf("%s(%s)(%d) leave the chatroom.\n", np->name, np->ip, np->data);
                sprintf(send_buffer, "%s leave the chatroom.", np->name, np->ip);
                strcpy(mess,send_buffer);
                SaveFile(mess);
                leave_flag = 1;
            } else {
                printf("Fatal Error: -1\n");
                leave_flag = 1;
            }
            send_to_all_clients(np, send_buffer);
        }

    }
    else{ // Login false
        bytes = send(np->data, "@err", 4, 0);
    }
    // Remove Node
    close(np->data);
    if (np == now) { // remove an edge node
        now = np->prev;
        now->link = NULL;
    } else { // remove a middle node
        np->prev->link = np->link;
        np->link->prev = np->prev;
    }
    free(np);
}

int main(int argc, char **argv)
{
    signal(SIGINT, catch_ctrl_c_and_exit);
    
    int port = 5500; //default port = 5500
    if(argc == 2){
        port = atoi(argv[1]);
    }

    // Create socket
    server_sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (server_sockfd == -1) {
        printf("Fail to create a socket.");
        exit(EXIT_FAILURE);
    }

    // Socket information
    struct sockaddr_in server_info, client_info;
    int s_addrlen = sizeof(server_info);
    int c_addrlen = sizeof(client_info);
    memset(&server_info, 0, s_addrlen);
    memset(&client_info, 0, c_addrlen);
    server_info.sin_family = AF_INET;
    server_info.sin_addr.s_addr = INADDR_ANY;
    server_info.sin_port = htons(port);

    // Bind and Listen
    bind(server_sockfd, (struct sockaddr *)&server_info, s_addrlen);
    listen(server_sockfd, 5);

    // Print Server IP
    getsockname(server_sockfd, (struct sockaddr*) &server_info, (socklen_t*) &s_addrlen);
    printf("Start Server on: %s:%d\n", inet_ntoa(server_info.sin_addr), ntohs(server_info.sin_port));

    // Initial linked list for clients
    root = newNode(server_sockfd, inet_ntoa(server_info.sin_addr));
    now = root;
    acc_list = create_llist(NULL);
    readUser();
    while (1) {
        client_sockfd = accept(server_sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);

        // Print Client IP
        getpeername(client_sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);
        printf("Client %s:%d come in.\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));

        // Append linked list for clients
        ClientList *c = newNode(client_sockfd, inet_ntoa(client_info.sin_addr));
        c->prev = now;
        now->link = c;
        now = c;

        pthread_t id;
        if (pthread_create(&id, NULL, (void *)client_handler, (void *)now) != 0) {
            perror("Create pthread error!\n");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}