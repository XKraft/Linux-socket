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
char servername[] = "server";

void *client_proc(void* arg);
void process_msg(Protocol_t* msg, int connfd);

int main()
{
    int sockfd = -1, connfd = -1;
    sockaddr_in servaddr, cliaddr;
    socklen_t cliaddr_len;
    char str[INET_ADDRSTRLEN];

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
    
    printf("等待用户连接中......\n");

    //主循环
    while(1)
    {
        cliaddr_len = sizeof(cliaddr);
        connfd = accept(sockfd, (sockaddr*)&cliaddr, &cliaddr_len);
        printf("%s:%d已连接到服务器\n", inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)), ntohs(cliaddr.sin_port));
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
        read(connfd, &buf, 1);
        if(pro_msg_parse(buf, &msg))
        {
            process_msg(&msg, connfd);
        }
    }
}

void process_msg(Protocol_t* msg, int connfd)
{
    uint8_t* buf = NULL;
    char* text = NULL;
    int buflen = 0;
    Pro_connect_t conn_msg;
    Pro_command_t comm_msg;
    Pro_sendfile_t sfile_msg;
    switch (msg->id)
    {
    case PRO_ID_CONNECT:
        pro_msg_connect_decode(msg, &conn_msg);
        if(add_user(&room, conn_msg.username, connfd))
        {
            pro_msg_answer_pack(msg, servername, 1);
            buflen = pro_msg_send_buf(buf, msg);
            write(connfd, buf, buflen);
            if(buf)
            {
                free(buf);
                buf = NULL;
            }

            text = (char*)malloc(sizeof(char) * (strlen(conn_msg.username) + strlen("加入了聊天室\n")));
            sprintf(text, "%s%s", conn_msg.username, "加入了聊天室");
            pro_msg_chattext_pack(msg, servername, text);
            buflen = pro_msg_send_buf(buf, msg);
            SendMsgToAllClients(&room, buf, buflen);

            if(buf)
            {
                free(buf);
                buf = NULL;
            }
            if(msg->load)
            {
                free(msg->load); msg->load = NULL;
            }
        }
        else
        {
            pro_msg_answer_pack(msg, servername, 0);
            buflen = pro_msg_send_buf(buf, msg);
            write(connfd, buf, buflen);
            if(buf)
            {
                free(buf);
                buf = NULL;
            }           

            text = "用户名已存在!\n";
            pro_msg_chattext_pack(msg, servername, text);
            buflen = pro_msg_send_buf(buf, msg);
            write(connfd, buf, buflen);
            if(buf)
            {
                free(buf);
                buf = NULL;
            }
            close(connfd);
        }
        
        
        break;
    case PRO_ID_CHATTEXT:
        buflen = pro_msg_send_buf(buf, msg);
        SendMsgToAllClients(&room, buf, buflen);
        printf("收到一条消息\n");
        if(buf)
        {
            free(buf);
            buf = NULL;
        }
        if(msg->username)
        {
            free(msg->username); msg->username = NULL;
        }
        if(msg->load)
        {
            free(msg->load); msg->load = NULL;
        }
        break;
    
    case PRO_ID_COMMAND:
        pro_msg_command_decode(msg, &comm_msg);
        if(strcmp(comm_msg.text, "/exit") == 0)
        {
            pro_msg_command_pack(msg, servername, "/exit_ok");
            buflen = pro_msg_send_buf(buf, msg);
            write(connfd, buf, buflen);
            remove_user(&room, comm_msg.username);
            if(buf)
            {
                free(buf); buf = NULL;
            }
            text = (char*)malloc(sizeof(char) * (strlen(comm_msg.username) + strlen("退出了聊天室\n")));
            sprintf(text, "%s%s", comm_msg.username, "退出了聊天室");
            pro_msg_chattext_pack(msg, servername, text);
            buflen = pro_msg_send_buf(buf, msg);
            SendMsgToAllClients(&room, buf, buflen);
            if(buf)
            {
                free(buf); buf = NULL;
            }
            if(msg->load)
            {
                free(msg->load); msg->load = NULL;
            }
            close(connfd);
            pthread_exit((void*)0);
        }
        break;

    case PRO_ID_SENDFILE:
        pro_msg_sendfile_decode(msg, &sfile_msg);
        SaveFile(&room, sfile_msg.filename, sfile_msg.file_size, sfile_msg.fileload);
        free(sfile_msg.filename); free(sfile_msg.fileload);
        break;
    default:
        break;
    }
}