#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
char name[20];
char password[10];
int sockfd;
int defeat_fd;
char defeat_name[20];
char ox[9],turn1,turn2;
int game=0,myturn=0;

void init(){
	sockfd = socket(PF_INET,SOCK_STREAM,0);
	struct sockaddr_in addr;
	addr.sin_family = PF_INET;
	addr.sin_port = htons(8080);
	addr.sin_addr.s_addr=htonl(INADDR_ANY);

	if (connect(sockfd,(struct sockaddr*)&addr,sizeof(addr)) == -1){
		perror("Client connect fail");
		exit(-1);
	}
	printf("[sys] Client start Successfully !\n");
}
void print_ox(){
    // printf("It's your turn!\n");
    for(int i=0;i<9;i+=3){
        printf(" %c | %c | %c \n",ox[i],ox[i+1],ox[i+2]);
        if(i<6) printf("-----------\n");
    }
    // printf("pls input #(0-8)\n");
}
void first_print(){
    for(int i=0;i<9;i+=3){
        printf(" %d | %d | %d \n",i,i+1,i+2);
        if(i<6) printf("-----------\n");
    }
}
int win(char OorX){
    for(int i=0;i<9;i+=3)
        if(ox[i]==OorX&&ox[i+1]==OorX&&ox[i+2]==OorX){
            return 1;
        }
    for(int i=0;i<3;i++)
        if(ox[i]==OorX&&ox[i+3]==OorX&&ox[i+6]==OorX){
            return 1;
        }
    if(ox[0]==OorX&&ox[4]==OorX&&ox[8]==OorX) return 1;
    if(ox[2]==OorX&&ox[4]==OorX&&ox[6]==OorX) return 1;
    return 0;
}
int fair(){
    for(int i=0;i<9;i++){
        if(ox[i]==' ') return 0;
    }
    return 1;
}
void* recv_thread(void *p){
    char buf[1000];
    while(1){
        memset(buf,'\0',sizeof(buf));
        if((recv(sockfd,buf,sizeof(buf),0))<=0){
            pthread_exit(NULL); 
            return NULL;
        }
        else if(strstr(buf,"對戰請求")!=NULL){
            printf("%s",buf);
            memset(defeat_name,'\0',sizeof(defeat_name));
            char* ptr=strstr(buf,"]");
            defeat_fd=atoi(&buf[7]); //取得對方的id
            ptr+=3;
            while(*ptr!=']') ptr++;
            ptr+=2;
            int cnt=0;
            char ntr[10];
            while(*ptr!=']'){
                ntr[cnt++]=*ptr++;
            }
            ntr[cnt]='\0';
            strcpy(defeat_name,ntr); //取得對方的名字
        }
        else if(strstr(buf,"disagree")!=NULL){
            defeat_fd=atoi(&buf[9]);
            char *ptr=&buf[6];
            while(*ptr!=' ') ptr++;
            ptr++;
            strcpy(defeat_name,ptr);
            printf("[sys] [%s]拒絕了你，輸入list選擇對戰對象\n",defeat_name);
        }
        else if(strstr(buf,"AGREE ")!=NULL){
            defeat_fd=atoi(&buf[6]);
            char *ptr=&buf[6];
            while(*ptr!=' ') ptr++;
            ptr++;
            strcpy(defeat_name,ptr);
            printf("[sys] 與[%s]成功配對\n",defeat_name);
            printf("[sys] ---OOXX start game OOXX---\n");
            for(int i=0;i<9;i++) ox[i]=' ';
            game=1;
            turn1='X';turn2='O';
            first_print();
            printf("[sys] you are 'X'\n");
            printf("[sys] [%s] first,pls wait until your turn!\n",defeat_name);
        }
        else if(buf[0]=='#'){
            ox[atoi(&buf[1])]=turn2;
            if(win(turn2)){
                print_ox();
                printf("[sys] You lose, %s is win\n",defeat_name);
                game=0;
            }
            else if(fair()){
                print_ox();
                printf("[sys] 和[%s]平手!!\n",defeat_name);
                game=0;
            }
            else {
                myturn=1;
                // printf("\n-----------\n\n");
                printf("It's your turn!\n");
                print_ox();
                printf("pls input #(0-8)\n");
            }
        }
        else{
            printf("%s",buf);
        }
    }
}
void start(){
    char buf[1000];
    int login_times=0;
    char pwd[20];
    memset(buf,'\0',sizeof(buf));
    while(1){
        recv(sockfd,buf,sizeof(buf),0); //收到server
        if(strcmp(buf,"auth")==0){
            send(sockfd,name,strlen(name),0);
        }
        else if(strstr(buf,"歡迎")!=NULL){
            printf("%s",buf);
            break;
        }
        else if(strstr(buf,"wrong")!=NULL){
            printf("%s",buf);
            close(sockfd);
            return;
        }
    }
    pthread_t tid;
    pthread_create(&tid,0,recv_thread,NULL);
    
    while(1){
        memset(buf,'\0',sizeof(buf));
        fgets(buf,sizeof(buf),stdin);
        // printf("%s",buf);
        if(strcmp(buf,"list\n")==0){
            send(sockfd,buf,strlen(buf),0);
        }
        else if(buf[0]=='$'){
            send(sockfd,buf,strlen(buf),0);
            printf("[sys] 等待對方回應請稍後喔..\n");
        }
        else if(strcmp(buf,"exit\n")==0){
            memset(buf,'\0',sizeof(buf));
            sprintf(buf,"[sys] %s 已登出\n",name);
            send(sockfd,buf,strlen(buf),0);
            break;
        }
        else if(strcmp(buf,"no\n")==0){
            memset(buf,'\0',sizeof(buf));
            printf("[sys] 你已拒絕了%s \n",defeat_name);
            sprintf(buf,"disagree %d",defeat_fd);
            send(sockfd,buf,strlen(buf),0);
        }
        else if(strcmp(buf,"yes\n")==0){
            memset(buf,'\0',sizeof(buf));
            printf("[sys] 與[%s]成功配對！\n",defeat_name);
            sprintf(buf,"AGREE %d",defeat_fd);
            send(sockfd,buf,strlen(buf),0);
            printf("[sys] ---OOXX start game OOXX---\n");
            for(int i=0;i<9;i++) ox[i]=' ';
            game=1;
            myturn=1;
            turn1='O';turn2='X';
            first_print();
            printf("[sys] you are 'O'\n");
            printf("[sys] It's your turn!\n");
            printf("[sys] pls input #(0-8)\n");
            printf("[sys] EX: #4 is the center of chessboard\n");
            printf("=>");
        }
        else if(buf[0]=='#'){
            if(game==0) printf("[sys] Game is not start!!\n");
            else if(myturn==0) printf("[sys] it's not your turn!!\n");
            else{
                int pos=atoi(&buf[1]);
                if(ox[atoi(&buf[1])]!=' ' || pos>8 || pos<0){
                    printf("[sys] 你選的位子已經被選過了或是輸入範圍外的號碼(not 0-8)\n");
                    printf("please input #(0-8)\n");
                    continue;
                }
                else ox[atoi(&buf[1])]=turn1;
                // printf("-----------\n");
                if(win(turn1)){
                    print_ox();
                    printf("[sys]You win!!!\n");
                    sprintf(buf,"#%d %d",pos,defeat_fd);
                    send(sockfd,buf,strlen(buf),0);
                    game=0;
                }
                else if(fair()){
                    print_ox();
                    printf("[sys] 和[%s]平手!!\n",defeat_name);
                    sprintf(buf,"#%d %d",pos,defeat_fd);
                    send(sockfd,buf,strlen(buf),0);
                    game=0;
                }
                else {
                    print_ox();
                    printf("[sys] it's [%s] turn,pls wait until your turn!\n",defeat_name);
                    printf("-----------\n");
                    myturn=0;
                    memset(buf,'\0',sizeof(buf));
                    sprintf(buf,"#%d %d",pos,defeat_fd);
                    send(sockfd,buf,strlen(buf),0);
                }
            }
        }
        else{
            if(buf[0]=='\n'||buf[0]=='\0') continue;
            char msg[2000];
            printf("broadcast to others: %s",buf);
            sprintf(msg,"broadcast %s",buf);
            send(sockfd,msg,strlen(msg),0);
        }
    }
    printf("[sys] 你已登出\n");
    close(sockfd);
}
int main(){
    init();
    printf("[sys] username:");
    scanf("%s",name);
    printf("[sys] password:");
    scanf("%s",password);
    strcat(name,":");
    strcat(name,password);
    start();
    return 0;
}