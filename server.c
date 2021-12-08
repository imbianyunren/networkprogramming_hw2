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
char username[100][10];
char welcome[]="---------------------\n"
               "歡迎遊玩OX遊戲\n"
               "請輸入'list'選擇對戰對象\n"
               "輸入'exit'登出\n"
               "可以直接輸入非關鍵文字廣播\n"
               "---------------------\n";
void init(){
	if((sockfd = socket(PF_INET,SOCK_STREAM,0))==-1){
        perror("create socket Failed");
        exit(0);
    }
    struct sockaddr_in addr;
	addr.sin_family = PF_INET;
	addr.sin_port = htons(8080);
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
    if((bind(sockfd,(struct sockaddr*)&addr,sizeof(addr)))==-1){
        perror("server bind failed");
        exit(0);
    }
    if((listen(sockfd,100))==-1){
        perror("listen failed");
        exit(0);
    }
}
int authentication(char* buf){
    FILE *fp;
    char auth[100];
    fp=fopen("pwd","r");
    while(fscanf(fp,"%s",auth)!=EOF){
        if(strcmp(auth,buf)==0) return 1;
    }
    return 0;
}
void broadcast(char *ptr,int fd){
    printf("Someone broadcasting...\n");
    printf("%s",ptr);
    for(int i=0;i<100;i++){
        if(users[i]!=0&&users[i]!=fd){
            send(users[i],ptr,strlen(ptr),0);
        }
    }
}
void get_allusers(int fd){
    int another_user=0;
    char buf[100];
    char intro[]="\n---------------------\n"
                "目前在線名單:\n"
                "[id]   [username]\n"
                "---------------------\n\n";

    send(fd,intro,strlen(intro),0);
    for(int i=0;i<100;i++){
        if(users[i]!=0&&users[i]!=fd){
            another_user=1;
            sprintf(buf,"[%d]\t[%s]\n",users[i],username[users[i]]);
            send(fd,buf,strlen(buf),0);
            
        }
    }
    if(another_user==1){
        char tail[]="\ninput '$id' to send request\n"
                    "---------------------\n";
        send(fd,tail,strlen(tail),0);
    }else if(another_user==0){
        char no_people[]="there is no person online now\n"
                        "pls wait a moment..\n"
                        "---------------------\n";
        send(fd,no_people,strlen(no_people),0);
    }

}
void* service(void* p){
    int fd=*(int*) p;
    int ret=1;
    char* ptr;
    char buf[100];
    //login
    while(1){
        send(fd,"auth",strlen("auth"),0); //send auth to client
        recv(fd,buf,sizeof(buf),0); //recv username&&pwd from client
        printf("id:%d => %s\n",fd,buf);
        ret=authentication(buf);
        if(ret==1){
            char name[100];
            for(int i=0;i<strlen(buf);i++){
                if(buf[i]!=':'){
                    name[i]=buf[i];
                }else if(buf[i]==':'){
                    break;
                }
            }
            strcpy(username[fd],name);
            username[fd][strlen(username[fd])]='\0';
            printf("user: [%s] is login\n",username[fd]);
            send(fd,welcome,strlen(welcome),0);
            break;
        }
        else if(ret==0){
            printf("fail to login\n");
            char wrong[]="---------------------\n"
                        "wrong username or pwd\n"
                        "---------------------\n";
            send(fd,wrong,strlen(wrong),0);
            for(int i=0;i<100;i++){
                if(fd==users[i]){
                    users[i]=0;
                    break;
                }
            }
            pthread_exit(NULL); 
        }
    }
    while(1){
        memset(buf,'\0',sizeof(buf));
        if((recv(fd,buf,sizeof(buf),0))<=0){
            int i;
            for(i=0;i<100;i++){
                if(fd==users[i]){
                    users[i]=0;
                    break;
                }
            }
            char str[100];
            sprintf(str,"user: [%s] is logout\n",username[fd]);
            broadcast(str,fd);
            printf("user: [%s] is logout\n",username[fd]);
            pthread_exit(NULL); 
        }
        else if(strcmp(buf,"list\n")==0){
            get_allusers(fd);
        }
        else if(strstr(buf,"broadcast")!=NULL){
            char* ptr=&buf[10];
            char namee[100];
            sprintf(namee,"[broadcast from %s]: %s",username[fd],ptr);
            broadcast(namee,fd);
        }
        else if(strstr(buf,"$")!=NULL){
            int id=atoi(&buf[1]);
            int exist=0;
            for(int i=0;i<100;i++){
                if(id==users[i]){
                    exist=1;
                    break;
                }
            }
            if(exist==0){
                char not_exist[]="the id is disconnect or isn't exist";
                send(fd,not_exist,strlen(not_exist),0);
            }else if(exist==1){
                char defeat_req[100];
                sprintf(defeat_req,"[sys] [%d][%s]傳送了對戰請求,您想要跟他對戰嗎？(yes/no)\n",fd,username[fd]);
                send(id,defeat_req,strlen(defeat_req),0);
            }
        }
        else if(strstr(buf,"disagree ")!=NULL){
            int id=atoi(&buf[9]);
            char new_buf[100];
            sprintf(new_buf,"disagree %d %s",fd,username[fd]);
            send(id,new_buf,strlen(new_buf),0);
        }
        else if(strstr(buf,"AGREE ")!=NULL){
            int id=atoi(&buf[6]);
            char new_buf[100];
            sprintf(new_buf,"AGREE %d %s",fd,username[fd]);
            send(id,new_buf,strlen(new_buf),0);
        }
        else if(buf[0]=='#'){
            int pos=atoi(&buf[1]),defeat_fd;
            char *ptr=strstr(buf," ");
            ptr++;
            defeat_fd=atoi(ptr);
            send(defeat_fd,buf,sizeof(buf),0);
        }
    }

}
int main(){
    init();
    printf("server init successful!!\n");
    while(1){
        struct sockaddr_in serv_addr;
        socklen_t len=sizeof(serv_addr);
        int client=accept(sockfd,(struct sockaddr*)&serv_addr,&len);
        printf("New connect request from id:%d\n",client);
        int i;
        for(i=0;i<100;i++){
            if(users[i]==0){
                users[i]=client;
                pthread_t tid;
                // printf("%ld",tid);
                pthread_create(&tid,0,service,&client);
                break;
            }
        }
        if(i>=100){
            char str[]="Room is full,pls wait a moment";
            send(client,str,strlen(str),0);
            close(client);
        }
    }
}