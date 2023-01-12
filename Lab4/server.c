//Message from client
// $gpass: get encryted pass
// $bye: signout 

//Message from server
// @wname: cannot find inputed username
// @cname: correct username
// @wpass: wrong pass
// @cpass: correcr pass
// @block: 3 times wrong pass -> blocked
// @err: error pass
// @nerr: not error pass
// @nready: not active or blocked account

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
#include "dllist.h"
#include "fields.h"

#define BUFF_SIZE 1024

typedef struct {
  char *name;
  char *pass;
  int status;
  int sign; // 1: sign in; 0: not sign in
} Account;

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

void Print(Dllist l)
{
    Dllist ptr; 
    FILE *f = fopen("account.txt","w+");
    dll_traverse(ptr,l) {
        Account *tmp = (Account*)jval_v(dll_val(ptr)); 
        fprintf(f,"%s %s %d\n",tmp->name,tmp->pass,tmp->status);
    }
    fclose(f);
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
    int check = 0;
    char n[1000], c[1000];
    char pass1[1000][1000], pass2[1000][1000]; // Save pass-change history from client1 client2
    int i1=0,i2=0; // Save off-set

    //Message
    char* wname = "@wname";
    char* cname = "@cname";
    char* wpass = "@wpass";
    char* cpass = "@cpass";
    char* block = "@block";
    char* nready = "@nready";
    char* err = "@err";
    char* nerr = "@nerr";

    //Read Port number
    if(argc != 2){
        printf("Input again.\n");
        exit(0);
    }
    PORT = atoi(argv[1]);

    IS is; Dllist l,ptr;
    int choice;
    Account *a; Account *tmp;
    
    is = new_inputstruct("account.txt");
    l = new_dllist();

 // Read from account.txt and add to link-list   
    while (get_line(is) >= 0) {
        if (is->NF > 1) {
            a = malloc(sizeof(Account));
            a->name = (char*)malloc(sizeof(char)*20);
            strcpy(a->name, is->fields[0]);
            a->pass = (char*)malloc(sizeof(char)*20);
            strcpy(a->pass, is->fields[1]);
            a->status = atoi(is->fields[2]);
            a->sign = 0; 
            dll_append(l,new_jval_v(a));
        }
    }
  
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
            }
            // When client1 != client and client2 = 0 -> save client's info to client2
            else if(ntohs(client1.sin_port)!=ntohs(client.sin_port) && ntohs(client2.sin_port)==0) {
                client2.sin_family = AF_INET;
				client2.sin_port = client.sin_port;
				client2.sin_addr.s_addr = INADDR_ANY ;
				printf("new client2: [%s:%d]\n", inet_ntoa(client2.sin_addr), ntohs(client2.sin_port));       
            }
        
            // Client's request processing
            // login request
            if(strcmp(buff,"$gpass")!=0) {
                //Check user-name and pass
                dll_traverse(ptr, l)
                {
                    tmp = (Account*)jval_v(dll_val(ptr));
                    // if can find username
                    if(strcmp(tmp->name,buff) == 0){
                        bytes_sent = sendto(server_sock, (char*)cname, strlen(cname), 0, (struct sockaddr*)&client, sin_size);
                        // if pass is incorrect
                        // Input password 3 times - check
                        check = 1;
                        int wrong = 0;
                        for(int i=0; i<3; i++)
                        {
                            // receive pass
                            bytes_received = recvfrom(server_sock, buff, BUFF_SIZE-1, 0, (struct sockaddr *) &client, &sin_size);
                            buff[bytes_received] = '\0';
                            printf("[%s:%d]: %s\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port), buff);
                            //correct pass
                            if(strcmp(tmp->pass,buff)==0) {
                                if(tmp->status==0){
                                    bytes_sent = sendto(server_sock, (char*)nready, strlen(nready), 0, (struct sockaddr*)&client, sin_size);
                                    break;
                                }
                                else {
                                    bytes_sent = sendto(server_sock, (char*)cpass, strlen(cpass), 0, (struct sockaddr*)&client, sin_size);
                                    tmp->sign = 1;
                                    //change pass
                                    bytes_sent = sendto(server_sock, (char*)nerr, strlen(nerr), 0, (struct sockaddr*)&client, sin_size);
                                    while (1)
                                    {
                                        bytes_received = recvfrom(server_sock, buff, BUFF_SIZE-1, 0, (struct sockaddr *) &client, &sin_size);
                                        buff[bytes_received] = '\0';
                                        printf("[%s:%d]: %s\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port), buff);
                                        if(strcmp(buff,"$bye")==0) {
                                            bytes_sent = sendto(server_sock, (char*)tmp->name, strlen(tmp->name), 0, (struct sockaddr*)&client, sin_size);
                                            tmp->sign = 0;
                                            break;
                                        }
                                        else {
                                            if(stringHandle(buff,n,c)<0) {
                                                bytes_sent = sendto(server_sock, (char*)err, strlen(err), 0, (struct sockaddr*)&client, sin_size);
                                                break;;
                                            }
                                            else {
                                                bytes_sent = sendto(server_sock, (char*)nerr, strlen(nerr), 0, (struct sockaddr*)&client, sin_size);
                                                strcpy(tmp->pass,buff); Print(l);
                                                if(ntohs(client1.sin_port)==ntohs(client.sin_port)) {
                                                    strcpy(pass1[i1++],buff);
                                                }
                                                if(ntohs(client2.sin_port)==ntohs(client.sin_port)) {
                                                    strcpy(pass2[i2++],buff);
                                                }
                                                continue;
                                            }
                                        }
                                    }
                                }
                                break;
                            }
                            else {
                                wrong++;
                                // Change status
                                if(wrong >= 3) {
                                    tmp->status = 0;
                                    //block mess
                                    bytes_sent = sendto(server_sock, (char*)block, strlen(block), 0, (struct sockaddr*)&client, sin_size);
                                }
                                else{
                                    bytes_sent = sendto(server_sock, (char*)wpass, strlen(wpass), 0, (struct sockaddr*)&client, sin_size);
                                }
                                continue;
                            }
                        }
                        break;
                    }
                }
                if(check == 0) {
                    bytes_sent = sendto(server_sock, (char*)wname, strlen(wname), 0, (struct sockaddr*)&client, sin_size);
                }
                Print(l);
                continue;
            } 
            // get encryted pass
            else {
                if(ntohs(client1.sin_port)==ntohs(client.sin_port)) {
                    if(stringHandle(pass2[i2-1],n,c)==0) {
                        bytes_sent = sendto(server_sock,(char*)n,strlen(n),0,(struct sockaddr*)&client1,sin_size);
                        if (bytes_sent < 0) perror("\nError: ");
                        bytes_sent = sendto(server_sock,(char*)c,strlen(c),0,(struct sockaddr*)&client1,sin_size);
                        if (bytes_sent < 0) perror("\nError: ");
				        continue;
                    }
                }
                if(ntohs(client2.sin_port)==ntohs(client.sin_port)) {
                    if(stringHandle(pass1[i1-1],n,c)==0) {
                        bytes_sent = sendto(server_sock,(char*)n,strlen(n),0,(struct sockaddr*)&client2,sin_size);
                        if (bytes_sent < 0) perror("\nError: ");
                        bytes_sent = sendto(server_sock,(char*)c,strlen(c),0,(struct sockaddr*)&client2,sin_size);
                        if (bytes_sent < 0) perror("\nError: ");
				        continue;
                    }
                }               
                continue;
            }
        }
        				
	}
        
	close(server_sock);
    free(a); free_dllist(l);  
    jettison_inputstruct(is);
	return 0;
}
