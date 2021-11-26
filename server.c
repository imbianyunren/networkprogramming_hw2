#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
int sockfd;
int users[100];
typedef struct socketaddr s_addr;
void init(){
	sockfd = socket(PF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
	addr.sin_family = PF_INET;
	addr.sin_port = htons(8080);
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
    bind(sockfd,(s_addr*)&addr,sizeof(addr));
    listen(sockfd,100);

}
int main(){
    init();
    printf("server init successful!!\n");
    while(1){
        struct sockaddr_in serv_addr;
        socklen_t len=sizeof(serv_addr);
        int client=accept(sockfd,(s_addr*)&serv_addr,&len);

    }
}