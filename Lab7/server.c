#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <errno.h>
#include <arpa/inet.h>
#include "dllist.h"
#include "fields.h"

#define BACKLOG 20
#define BUFF_SIZE 1024
#define MAX 1000

char *chat;
char mess[MAX][BUFF_SIZE];
pid_t pid,ppid;	

typedef struct {
  char *name;
  char *pass;
  int status;
} Account;
Dllist l,ptr;
Account *cur;
int id=0;

void readAcc(Dllist l)
{
	IS is; Account *a; 
	is = new_inputstruct("taikhoan.txt");

 	// Read from account.txt and add to link-list   
    while (get_line(is) >= 0) {
        if (is->NF > 1) {
            a = malloc(sizeof(Account));
            a->name = (char*)malloc(sizeof(char)*20);
            strcpy(a->name, is->fields[0]);
            a->pass = (char*)malloc(sizeof(char)*20);
            strcpy(a->pass, is->fields[1]);
            a->status = atoi(is->fields[2]);
            dll_append(l,new_jval_v(a));
        }
    }
	free(a); 
	jettison_inputstruct(is);
}

void saveMess(char *string){ 
    FILE *pt = fopen("groupchat.txt", "a+");
    fprintf(pt, "%s\n", string);
    fclose(pt);
}

int checkLogin(char *username, char *password, Dllist l)
{ 
	Dllist ptr; int check = -2;
    dll_traverse(ptr,l) {
        Account *tmp = (Account*)jval_v(dll_val(ptr)); 
        if(strcmp(tmp->name, username) == 0){
            if(strcmp(tmp->pass, password) == 0){
				if (tmp->status == 0)
				{
					check = 0;
				}
				else {
					check = 1;
				}
            }
			else check = -1;
			break;
        }
    }
    return check;
}

Account* findNode(char *username, char *password, Dllist l)
{ 
	Dllist ptr; Account* tmp;
    dll_traverse(ptr,l) {
        tmp = (Account*)jval_v(dll_val(ptr)); 
        if(strcmp(tmp->name, username) == 0 && strcmp(tmp->pass, password) == 0){
			break;
        }
    }
    return tmp;
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

char *getChat(){ // Read message in file groupchat.txt
    FILE *pt = fopen("groupchat.txt", "r");
    char *str = calloc(BUFF_SIZE, sizeof(char));
    bzero(str, BUFF_SIZE);
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

char * getLastMess(){ //get last message in file groupchat.txt
    FILE *pt = fopen("groupchat.txt", "r");
    char *str = calloc(1000, sizeof(char));
    char *buffer = calloc(1000, sizeof(char));
    bzero(str, 1000);
    bzero(buffer, 1000);
    do{
        bzero(str, 1000);
        strcpy(str, buffer);
        bzero(buffer, 1000);
        fgets(buffer, 1000, pt);
    }while(strlen(buffer) != 0);
    str[strlen(str) - 1] = '\0';
    fclose(pt);
    return str;
}

/* Handler process signal*/
void sig_chld(int signo);

/*
* Receive and echo message to client
* [IN] sockfd: socket descriptor that connects to client 	
*/
void echo(int sockfd);

int login(int sockfd){
	char buff[BUFF_SIZE];
	int bytes_sent, bytes_received;

	char name[100], pass[100];

	bytes_received = recv(sockfd, buff, BUFF_SIZE, 0); //blocking
	if (bytes_received < 0)
		perror("\nError: ");
	else if (bytes_received == 0)
		printf("Connection closed.");
	buff[bytes_received] = '\0';
	splitLogin(buff,name,pass);
	int check = checkLogin(name,pass,l);
	if(check == 1){ //correct username and password
		cur = findNode(name,pass,l);
		printf("User %s connfd %d\n", cur->name, sockfd);
		strcpy(mess[id], cur->name);
		strcat(mess[id], " joins chat room."); 
		id++;
		saveMess(mess[id-1]);
		chat = getChat(); chat[strlen(chat)] = '\0';

		bytes_sent = send(sockfd, (char*)chat, strlen(chat), 0);
		if(bytes_sent < 0)	perror("\nError: ");
	}
	else{
		bytes_sent = send(sockfd, "@err", 4, 0);
		if(bytes_sent < 0)	perror("\nError: ");
	}
	return check;
}
int main(int argc, char** argv){
	int sin_size;
    int PORT;
	int listen_sock, conn_sock, check; /* file descriptors */
	int bytes_sent, bytes_received;
	struct sockaddr_in server; /* server's address information */
	struct sockaddr_in client; /* client's address information */
  
    //Read Port number
    if(argc != 2){
        printf("Input again.\n");
        exit(0);
    }
    PORT = atoi(argv[1]);
	
	l = new_dllist();
 	// Read from account.txt and add to link-list   
	readAcc(l);

	if ((listen_sock=socket(AF_INET, SOCK_STREAM, 0)) == -1 ){  /* calls socket() */
		printf("socket() error\n");
		return 0;
	}
	
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;         
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);  /* INADDR_ANY puts your IP address automatically */   

	if(bind(listen_sock, (struct sockaddr*)&server, sizeof(server))==-1){ 
		perror("\nError: ");
		return 0;
	}     

	if(listen(listen_sock, BACKLOG) == -1){  
		perror("\nError: ");
		return 0;
	}
	
	/* Establish a signal handler to catch SIGCHLD */
	signal(SIGCHLD, sig_chld);

	while(1){
		sin_size=sizeof(struct sockaddr_in);
		if ((conn_sock = accept(listen_sock, (struct sockaddr *)&client, &sin_size))==-1){
			if (errno == EINTR)
				continue;
			else{
				perror("\nError: ");			
				return 0;
			}
		}
		/* fork() is called in child process */
		check = login(conn_sock);
		if(check == 1){
			if((pid = fork()) == 0){
				close(listen_sock);
				printf("You got a connection from %s\n", inet_ntoa(client.sin_addr)); /* prints client's IP */
				echo(conn_sock);					
				exit(0);
			}
		}
		close(conn_sock);
	}
	close(listen_sock);
	return 0;
}

void sig_chld(int signo){
	pid_t pid;
	int stat;
	
	/* Wait the child process terminate */
	while((pid = waitpid(-1, &stat, WNOHANG))>0)
		printf("\nChild %d terminated\n",pid);
}

void echo(int sockfd) {
	char buff[BUFF_SIZE];
	int bytes_sent, bytes_received;
    signal(SIGCHLD, sig_chld);

    if((ppid = fork()) == 0){
		FILE *pt;
		pt = fopen("groupchat.txt","r");
		fseek(pt,0,SEEK_END);
		int filesize = ftell(pt);
		int endfile;
		fclose(pt);
		while (1)
		{
			FILE *pt;
			pt = fopen("groupchat.txt","r");
			fseek(pt,0,SEEK_END);
			endfile =ftell(pt);
			if(endfile>filesize){
				// bytes_sent = send(sockfd, mess[id-1], strlen(mess[id-1]), 0);
				// filesize = endfile;
				char* chat = calloc(1000, sizeof(char));
				chat = getLastMess();
				bytes_sent = send(sockfd, chat, strlen(chat), 0);
				filesize = endfile;
				// bytes_received = recv(sockfd, buff, BUFF_SIZE, 0);
            	// buff[bytes_received] = '\0';
			}
			fclose(pt);
		}
		printf("User %s logout\n",cur->name);
		close(sockfd);
		exit(0);
    }
	else{
		do{
            bzero(buff, BUFF_SIZE);
            bytes_received = recv(sockfd, buff, BUFF_SIZE, 0);
            buff[bytes_received] = '\0';
			strcpy(mess[id], cur->name);
			strcat(mess[id], " : ");
			strcat(mess[id],buff);
			saveMess(mess[id]);
			id++;
			//bytes_sent = send(sockfd, mess[id-1], strlen(mess[id-1]), 0);
			if(strcmp(buff,"bye")==0){
				strcpy(mess[id], cur->name);
				strcat(mess[id], " left room chat.");
				saveMess(mess[id]);
				id++;
				bytes_sent = send(sockfd, "@bye", 4, 0);
				if(bytes_sent < 0)	perror("\nError: ");
				break;
			}
		} while(1);
		close(sockfd);
		exit(0);
	}
	close(sockfd);
	exit(0);
}