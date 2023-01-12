#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#define BUFF_SIZE 8192

void Menu(){
    printf("\nMENU\n");
    printf("-----------------------------------\n");
    printf("1. Send string\n");
    printf("2. Send file\n");
    printf("3. Exit\n");
    printf("-----------------------------------\n");
}

void readFile(char *file, char *buff){
    FILE* ptr;
    int i=0;

    ptr = fopen(file, "r");
 
    if (NULL == ptr) {
        printf("file can't be opened \n");
        return;
    }
 
    while (!feof(ptr)) {
        buff[i] = fgetc(ptr);
        i++;
    } 
    buff[i-1] = '\n';
    fclose(ptr);
}

int main(int argc, char** argv){
	int client_sock;
    char* SERVER_ADDR; int SERVER_PORT;
	char buff[BUFF_SIZE]; char filename[100]; char content[1000];
	struct sockaddr_in server_addr; /* server's address information */
	int msg_len, bytes_sent, bytes_received;
    // Read IP address and Port number
    if(argc != 3) {
        printf("Input again.\n");

        exit(0);
    }
    SERVER_ADDR = argv[1];
    SERVER_PORT = atoi(argv[2]);

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

    int choice;

	//Step 4: Communicate with server			
	while(1){
        bzero(buff,BUFF_SIZE); fflush(stdin);
        Menu();
        scanf("%d",&choice);
        fflush(stdin);
        switch (choice)
        {
        case 1:
            bytes_sent = send(client_sock, "@1",2, 0);
            if(bytes_sent <= 0){
                printf("\nConnection closed!\n");
                break;
            }
            bytes_received = recv(client_sock, buff, BUFF_SIZE-1, 0);
            if(bytes_received <= 0){
                printf("\nError!Cannot receive data from sever!\n");
                break;
            }
            if(strcmp(buff,"$OK")!=0) {
                break;
            }

            while (1)
            {
                //send message
                printf("Insert string to send: ");
                memset(buff,'\0',(strlen(buff)+1));
                fgets(buff, BUFF_SIZE, stdin);		
                msg_len = strlen(buff);
                if(msg_len == 0)  break;

                if(strlen(buff)==1 && buff[0]=='\n') {
                    printf("Close\n");
                    bytes_sent = send(client_sock, "@close", 6, 0);
                    bytes_received = recv(client_sock, buff, BUFF_SIZE-1, 0);
                    if(bytes_received <= 0){
                        printf("\nError!Cannot receive data from sever!\n");
                        break;
                    }
                    
                    break;
                }

                bytes_sent = send(client_sock, buff, msg_len, 0);
                if(bytes_sent <= 0){
                    printf("\nConnection closed!\n");
                    break;
                }
                bytes_received = recv(client_sock, buff, BUFF_SIZE-1, 0);
                if(bytes_received <= 0){
                    printf("\nError!Cannot receive data from sever!\n");
                    break;
                }
                if(strcmp(buff,"$err")==0) {
                    printf("Error!!!\n");
                }
                else {
                    buff[bytes_received] = '\0';
                    if(strlen(buff)>0) printf("Digit string: %s\n", buff);
                    bytes_sent = send(client_sock, "@OK", 3, 0);
                    if(bytes_sent <= 0){
                        printf("\nConnection closed!\n");
                        break;
                    }

                    bytes_received = recv(client_sock, buff, BUFF_SIZE-1, 0);
                    if(bytes_received <= 0){
                        printf("\nError!Cannot receive data from sever!\n");
                        break;
                    }
                    buff[bytes_received] = '\0';
                    if(strlen(buff)>0) printf("Character string: %s\n", buff);
                }
            }
            break;
        case 2:
            bytes_sent = send(client_sock, "@2",2, 0);
            if(bytes_sent <= 0){
                printf("\nConnection closed!\n");
                break;
            }
            bytes_received = recv(client_sock, buff, BUFF_SIZE-1, 0);
            if(bytes_received <= 0){
                printf("\nError!Cannot receive data from sever!\n");
                break;
            }
            //puts(buff);
            if(strcmp(buff,"$OK")!=0) {
                break;
            }
            printf("Insert filename to send: ");
            fflush(stdin);
			memset(filename,'\0',(strlen(filename)+1));
			fgets(filename, 100, stdin);
            msg_len = strlen(filename);
            filename[msg_len-1] = '\0';
            //strcpy(file,"a.txt");
            readFile(filename,content);
//            printf("\n%s",content);
            
            bytes_sent = send(client_sock, filename ,strlen(filename),0);

            bytes_received = recv(client_sock, buff, BUFF_SIZE-1, 0);
            if(bytes_received <= 0){
                printf("\nError!Cannot receive data from sever!\n");
                break;
            }
            if(strcmp(buff,"$file")!=0) {
                break;
            }

            bytes_sent = send(client_sock, content, strlen(content), 0);

            bytes_received = recv(client_sock, buff, BUFF_SIZE-1, 0);
            if(bytes_received <= 0){
                printf("\nError!Cannot receive data from sever!\n");
                break;
            }
            buff[bytes_received] = '\0';
		    printf("File's content:\n%s", buff);
            break;
        case 3:
            bytes_sent = send(client_sock,"@3",2,0);
            close(client_sock);
            exit(1);
        default: 
            printf("Input again (1 or 2 or 3).\n");
            break;
        }
	}
	
	//Step 4: Close socket
	close(client_sock);
	return 0;

}