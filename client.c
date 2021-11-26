#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
char name[10];
char password[10];
int sockfd;
typedef struct sockaddr s_addr;
void init(){
	sockfd = socket(PF_INET,SOCK_STREAM,0);
	struct sockaddr_in addr;
	addr.sin_family = PF_INET;
	addr.sin_port = htons(8080);
	addr.sin_addr.s_addr=htonl(INADDR_ANY);

	if (connect(sockfd,(s_addr*)&addr,sizeof(addr)) == -1){
		perror("Client connect fail");
		exit(-1);
	}
	printf("[sys] Client start Successfully !\n");
}
void start(){
    char buf[100];
    memset(buf,'\0',sizeof(buf));
    while(1){
        recv(sockfd,buf,sizeof(buf),0);
        if(strcmp(buf,"authenticate")==0){
            send(sockfd,name,strlen(name),0);
        }
    }
}
int main(){
    init();
    printf("[sys] username:");
    scanf("%s",name);
    printf("[sys] password:");
    scanf("%s",password);
    strcat(name,":");
    strcat(name,password);
    strcat(name,"#");
    start();
    return 0;
}