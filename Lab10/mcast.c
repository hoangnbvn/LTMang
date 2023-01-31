#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <time.h>
#include <pthread.h>
#include <sys/select.h>
#define MAX 1000
#define MAX_LIST 100
int sendfd, listenfd;

fd_set readfd;

struct sockaddr_in local_addr, group_addr;
struct ip_mreq group;
struct in_addr localInterface;

typedef struct _GroupList{
    struct sockaddr_in addr;
    int isMember;
    time_t timeout;
}GroupList;
GroupList *gl;
void *sendToGroup(){
    pthread_detach(pthread_self());
    char *buffer = "hello";
    unsigned int length = sizeof(group_addr);
    while(1){
        sendto(sendfd, buffer, strlen(buffer), 0, (struct sockaddr *)&group_addr, length); 
        sleep(1);
    }
    return NULL;
}

void *check_member(void * index){
    int i = (*(int *)index);
    time_t timeout;
    double elapsed;
    time(&gl[i].timeout);
    do{
        time(&timeout);
        elapsed = difftime(timeout, gl[i].timeout);
    }while(elapsed < 3);
    printf("Neigbor %s %d is timeout\n", inet_ntoa(gl[i].addr.sin_addr), gl[i].addr.sin_port);
    bzero(&gl[i].addr, sizeof(gl[i].addr));
    gl[i].isMember = -1;
    return NULL;
}

int check_new_member(struct sockaddr_in addr){
    int i;
    for(i = 0; i < MAX_LIST; i++){
        if(gl[i].addr.sin_addr.s_addr == addr.sin_addr.s_addr && gl[i].addr.sin_port == addr.sin_port){
            return i;
        }
    }
    return -1;
}

void printMember(){
    int i;
    printf("List of neighbors:\n");
    for(i = 0; i< MAX_LIST; i++){
        if(gl[i].isMember > 0){
            printf("%s %d\n", inet_ntoa(gl[i].addr.sin_addr), gl[i].addr.sin_port);
        }
    }
}

void command_handle(){
    char *buffer = calloc(MAX, sizeof(char));
    int buffer_size = read(STDIN_FILENO, buffer, MAX);
    buffer[buffer_size - 1] = '\0';
    if(strcmp(buffer, "quit") == 0){
        close(sendfd);
        close(listenfd);
        exit(EXIT_SUCCESS);
    }else if(strcmp(buffer, "print mtable") == 0){
        printMember();
    }else{
        printf("Wrong command\n");
    }
}

void *listenToGroup(){
    pthread_detach(pthread_self());
    char *buffer = calloc(MAX, sizeof(char));
    pthread_t tid[MAX_LIST];
    int buffer_size, i;
    unsigned int length = sizeof(local_addr);
    while(1){
        bzero(buffer, MAX);
        buffer_size = recvfrom(listenfd, buffer, MAX, 0, (struct sockaddr *)&local_addr, &length);
        buffer[buffer_size] = '\0';
        if(buffer_size > 0){
            //printf("%s %d %s\n", inet_ntoa(local_addr.sin_addr), local_addr.sin_port,buffer);
            i = check_new_member(local_addr);
            if(i < 0){
                for(i = 0; i < MAX_LIST; i++){
                    if(gl[i].isMember < 0){
                        gl[i].addr = local_addr;
                        gl[i].isMember = 1;
                        pthread_create(&tid[i], NULL, check_member, (void *)&i);
                        printf("New neibor %s %d joined\n", inet_ntoa(gl[i].addr.sin_addr), gl[i].addr.sin_port);
                        break;
                    }
                }
            }else{
                time(&gl[i].timeout);
            }
        }else{
            perror("Recvfrom failed\n");
            exit(0);
        }
    }
    return NULL;
}

void setupSocket(int port, const char *group_ip){
    if((sendfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1 || (listenfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }   

    bzero(&group_addr, sizeof(group_addr));
    group_addr.sin_family = AF_INET;
    group_addr.sin_port = htons(port);
    group_addr.sin_addr.s_addr = inet_addr(group_ip);

    bzero(&local_addr, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);
    local_addr.sin_addr.s_addr = INADDR_ANY;


    char loop = 0;
    if(setsockopt(sendfd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loop, sizeof(loop)) < 0){
        perror("Set option socket group failed");
        exit(EXIT_FAILURE);
    }

    localInterface.s_addr = inet_addr("127.0.0.1");

    if(setsockopt(sendfd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0){
        perror("Set local interface failed");
        exit(EXIT_FAILURE);
    }

    int reuse = 1;
    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0){
        perror("Set option socket reuse failed");
        exit(EXIT_FAILURE);
    }
    if(bind(listenfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) != 0){
        perror("Socket listen bind failed");
        exit(EXIT_FAILURE);
    }
    group.imr_multiaddr.s_addr = inet_addr(group_ip);
    group.imr_interface.s_addr = inet_addr("127.0.0.1");
    if(setsockopt(listenfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0){
        perror("Set group multicast failed");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char const *argv[]){
    pthread_t tid_send;
    pthread_t tid_recv;
    int i;
    gl = calloc(MAX_LIST, sizeof(GroupList));
    for(i = 0; i < MAX_LIST; i++){
        bzero(&gl[i].addr, sizeof(gl[i].addr));
        gl[i].isMember = -1;
    }
    if(argc < 3 ){
        printf("Server creation failed! Need port number! Usage: %s [port_number]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[2]);

    setupSocket(port, argv[1]);
    pthread_create(&tid_send, NULL, sendToGroup, NULL);
    pthread_create(&tid_recv, NULL, listenToGroup, NULL);

    FD_ZERO(&readfd);
    FD_SET(STDIN_FILENO, &readfd);
    int max_fd = STDIN_FILENO;
    int nEvent;
    while(1){
        nEvent = select(max_fd + 1, &readfd, NULL ,NULL, NULL);
        if(nEvent < 0){
            perror("Select error");
            exit(EXIT_FAILURE);
        }
        if(FD_ISSET(STDIN_FILENO, &readfd)){
            command_handle();
        }
    }
    

    close(listenfd);
    return 0;
}