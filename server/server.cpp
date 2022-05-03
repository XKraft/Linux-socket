#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include<stdio.h>
#include"../head/chatroom.h"
#include"../head/protocol.h"

#define SERVER_IP "10.0.0.2" //ip地址
#define SERVER_PORT 3000 //端口号

chatroom room;
void *client_proc(void* arg);

int main()
{
    int sockfd = -1, connfd = -1;
    sockaddr_in servaddr, cliaddr;
    socklen_t cliaddr_len;

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//设置ip地址，使用这个宏定义好像可以自动分配，如果到时候不行的话可以手动填一下（字符串）
    servaddr.sin_port = htons(SERVER_PORT);//设置端口号

    if(-1 == (sockfd = socket(AF_INET, SOCK_STREAM, 0)))
    {
        printf("error1:socket error!\n");
        return -1;
    }
    if(-1 == bind(sockfd, (sockaddr*)&servaddr, sizeof(servaddr)))
    {
        printf("error2:bind error!\n");
        return -1;
    }
    if(-1 == listen(sockfd, 100))
    {
        printf("error3:listen error!\n");
        return -1;
    }

    //初始化聊天室
    init_chatroom(&room);

    //主循环
    while(1)
    {
        cliaddr_len = sizeof(cliaddr);
        connfd = accept(sockfd, (sockaddr*)&cliaddr, &cliaddr_len);
        pthread_t tid;
        pthread_create(&tid, NULL, client_proc, (void*)&connfd);
        pthread_detach(tid);
    }
}

//线程函数
void *client_proc(void* arg)
{
    int connfd = *(int *)arg;
    uint8_t buf;
    Protocol_t msg;
    while(1)
    {
        read(sockfd, &buf, 1);
        if(pro_msg_parse(buf, &msg))
        {
            process_msg(&msg, sockfd);
            if(msg.username)
            {
                free(msg.username);
                msg.username = NULL;
            }
            if(msg.load)
            {
                free(msg.load);
                msg.load = NULL;
            }
        }
    }
}

void process_msg(Protocol_t* msg, int sockfd)
{
    switch (msg->id)
    {
    case PRO_ID_CONNECT:
        
        break;
    case PRO_ID_CHATTEXT:
        Pro_chattext_t chat_msg;
        pro_msg_chattext_decode(msg, &chat_msg);
        if(strcmp(chat_msg.username, username) != 0)
            printf("%s:%s\n", chat_msg.username, chat_msg.text);
        if(chat_msg.text)
        {
            free(chat_msg.text);
            chat_msg.text = NULL;
        }
        if(chat_msg.username)
        {
            free(chat_msg.username);
            chat_msg.username = NULL;
        }
        break;
    
    default:
        break;
    }
}