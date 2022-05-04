#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include"../head/myStr.h"
#include"../head/protocol.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 3000 //端口号

char* username = NULL;

bool SendUsername(int sockfd, char* username, uint8_t* buf, int buflen, Protocol_t* msg);
void *ReadServer(void* arg);
void process_msg(Protocol_t* msg, int sockfd);
void process_text(char* text, int sockfd, Protocol_t* msg, int buflen, uint8_t* buf);

int main()
{
    struct sockaddr_in servaddr;
    int sockfd;
    char* text = NULL;
    uint8_t* buf = NULL;
    int buflen = 0;
    Protocol_t msg;

    printf("请输入用户名:");
    username = Str_get();
    // while(strlen(username) > 10)
    // {
    //     printf("用户名输入错误,请不超过10个英文字符!请重新输入:");
    //     username = Str_get();
    // }
    // //printf("%ld\n", sizeof(*username) * strlen(username));

    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr);//设置服务器ip地址
    servaddr.sin_port = htons(SERVER_PORT);//设置端口号

    if(-1 == (sockfd = socket(AF_INET, SOCK_STREAM, 0)))
    {
        printf("error1:socket error!\n");
        return -1;
    }

    if(connect(sockfd, (sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("error2:unable to connect to server!\n");
        return -2;
    }
    
    if(!SendUsername(sockfd, username, buf, buflen, &msg))
        return -3;

    pthread_t tid;
    pthread_create(&tid, NULL, ReadServer, (void*)&sockfd);
    pthread_detach(tid);

    while(1)
    {
        text = Str_get();
        process_text(text, sockfd, &msg, buflen, buf);
        free(text);
        text = NULL;
    }

    return 0;
}

bool SendUsername(int sockfd, char* username, uint8_t* buf, int buflen, Protocol_t* msg)
{
    Pro_answer_t answer_msg;
    pro_msg_connect_pack(msg, username);
    buflen = pro_msg_send_buf(buf, msg);
    write(sockfd, buf, buflen);
    if(buf)
    {
        free(buf); buf = NULL;
    }
    if(msg->load)
    {
        free(msg->load); msg->load = NULL;
    }

    buf = (uint8_t*)malloc(sizeof(uint8_t) * PRO_MSG_ANSWER_lEN);
    buflen = read(sockfd, buf, PRO_MSG_ANSWER_lEN);

    for(int i = 0; i < buflen; ++i)
    {
        if(pro_msg_parse(buf[i], msg))
        {
            if(buf)
            {
                free(buf); buf = NULL;
            }
            if(pro_msg_answer_decode(msg, &answer_msg))
            {
                if(1 == answer_msg.answer)
                {
                    printf("登陆成功!已连接到服务器\n");
                    return true;
                }
                else
                {
                    printf("登陆失败!用户名已存在\n");
                    return false;
                }
            }
        }
    }
    if(buf)
    {
        free(buf); buf = NULL;
    }
    printf("登陆失败!\n");
    return false;
}

//线程函数
void *ReadServer(void* arg)
{
    int sockfd = *(int *)arg;
    uint8_t buf;
    Protocol_t msg;
    while(1)
    {
        read(sockfd, &buf, 1);
        if(pro_msg_parse(buf, &msg))
        {
            process_msg(&msg, sockfd);
        }
    }
}

void process_msg(Protocol_t* msg, int sockfd)
{
    Pro_chattext_t chat_msg;
    switch (msg->id)
    {
    case PRO_ID_CHATTEXT:
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

void process_text(char* text, int sockfd, Protocol_t* msg, int buflen, uint8_t* buf)
{
    if(text[0] != '/')
    {
        pro_msg_chattext_pack(msg, username, text);
        buflen = pro_msg_send_buf(buf, msg);
        write(sockfd, buf, buflen);
        if(msg->load)
        {
            free(msg->load);
            msg->load = NULL;
        }
        if(buf)
        {
            free(buf);
            buf = NULL;
        }
    }
}