#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include<stdio.h>
#include"../head/chatroom.h"
#include"../head/protocol.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 本文件中实现了服务器的具体功能
// 服务器的工作流程大概是:程序启动后，会进行一些必要的TCP协议的绑定等操作,然后会等到客户端连接,当客户端连接到后,会马上开
// 辟一个子线程,单独用于客户端消息的处理,主线程则一值等待新的用户连接.在接收到客户端发送的用户名后,会判断用户名是否已经存
// 在,如果存在,则向服务端发送应答,不接受连接,关闭子线程,如果不存在,则继续执行.在子线程中,客户端向服务器发送的数据,并从中
// 解析出消息包,然后根据消息包中的内容做出相应的处理,直到收到客户端发送的请求退出聊天室命令,则关闭该线程,并删除该用户
// 已经实现的内容
// 1.接受到用户发送的聊天文本,并群发给所有在聊天室中的成员
// 2.向用户发送聊天室中文件列表
// 3.接收用户上传的文件,存入本地文件夹中,并更新文件目录日志文件
// 4.将文件列表存入一个文件目录日志文件中,每次启动自动读取,保证聊天室文件不会丢失
// 5.向用户发送文件
// 6.处理用户的退出请求
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SERVER_IP "192.168.43.208" //ip地址
#define SERVER_PORT 3000 //端口号

//全局变量
chatroom room;
char servername[] = "server";

//函数声明
void* client_proc(void* arg);
void process_msg(Protocol_t* msg, int connfd);
void SendMsgToOneClient(int connfd, char* text);
void SendFileToOneClient(int connfd, char* filename, int file_size, char* file_buf);

int main()
{
    int sockfd = -1, connfd = -1;
    sockaddr_in servaddr, cliaddr;
    socklen_t cliaddr_len;
    char str[INET_ADDRSTRLEN];

    //TCP协议
    servaddr.sin_family = AF_INET;
    //servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//设置ip地址，使用这个宏定义好像可以自动分配，如果到时候不行的话可以手动填一下（字符串）
    inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr.s_addr);
    servaddr.sin_port = htons(SERVER_PORT);//设置端口号

    if (-1 == (sockfd = socket(AF_INET, SOCK_STREAM, 0)))
    {
        printf("error1:socket error!\n");
        return -1;
    }
    if (-1 == bind(sockfd, (sockaddr*)&servaddr, sizeof(servaddr)))
    {
        printf("error2:bind error!\n");
        return -1;
    }
    if (-1 == listen(sockfd, 100))
    {
        printf("error3:listen error!\n");
        return -1;
    }

    //初始化聊天室
    init_chatroom(&room);

    printf("等待用户连接中......\n");

    //主循环
    while (1)
    {
        cliaddr_len = sizeof(cliaddr);
        connfd = accept(sockfd, (sockaddr*)&cliaddr, &cliaddr_len);
        printf("%s:%d已连接到服务器\n", inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)), ntohs(cliaddr.sin_port));
        //开辟子线程
        pthread_t tid;
        pthread_create(&tid, NULL, client_proc, (void*)&connfd);
        pthread_detach(tid);
    }
}

//线程函数,每个用户都对应一个子线程,用于处理客户端发送的数据
void* client_proc(void* arg)
{
    int connfd = *(int*)arg;
    uint8_t buf;
    Protocol_t msg;
    while (1)
    {
        read(connfd, &buf, 1);
        if (pro_msg_parse(buf, &msg))
        {
            process_msg(&msg, connfd);
        }
    }
}

//根据解析处理的消息id,做出对应的反应
void process_msg(Protocol_t* msg, int connfd)
{
    uint8_t* buf = NULL;
    char* text = NULL;
    int buflen = 0;
    char* file_buf = NULL;
    int file_size = 0;
    Pro_connect_t conn_msg;
    Pro_command_t comm_msg;
    Pro_sendfile_t sfile_msg;
    Pro_downloadfile_t dwfile_msg;
    switch (msg->id)
    {
    case PRO_ID_CONNECT://接收到客户端第一次连接发送的用户名
        pro_msg_connect_decode(msg, &conn_msg);
        if (add_user(&room, conn_msg.username, connfd))//判断用户名是否存在
        {
            pro_msg_answer_pack(msg, servername, 1);//表示接收连接请求
            buflen = pro_msg_send_buf(buf, msg);
            write(connfd, buf, buflen);
            if (buf)
            {
                free(buf);
                buf = NULL;
            }

            text = (char*)malloc(sizeof(char) * (strlen(conn_msg.username) + strlen("加入了聊天室\n")));
            sprintf(text, "%s%s", conn_msg.username, "加入了聊天室");
            pro_msg_chattext_pack(msg, servername, text);
            buflen = pro_msg_send_buf(buf, msg);
            SendMsgToAllClients(&room, buf, buflen);

            if (buf)
            {
                free(buf);
                buf = NULL;
            }
            if (msg->load)
            {
                free(msg->load); msg->load = NULL;
            }
        }
        else//表示用户名存在,不接受请求,关闭该线程
        {
            pro_msg_answer_pack(msg, servername, 0);
            buflen = pro_msg_send_buf(buf, msg);
            write(connfd, buf, buflen);
            if (buf)
            {
                free(buf);
                buf = NULL;
            }

            text = "用户名已存在!\n";
            pro_msg_chattext_pack(msg, servername, text);
            buflen = pro_msg_send_buf(buf, msg);
            write(connfd, buf, buflen);
            if (buf)
            {
                free(buf);
                buf = NULL;
            }
            close(connfd);
            pthread_exit((void*)0);
        }


        break;
    case PRO_ID_CHATTEXT://聊天文本,直接群发给所有在线用户
        buflen = pro_msg_send_buf(buf, msg);
        SendMsgToAllClients(&room, buf, buflen);
        printf("收到一条消息\n");
        if (buf)
        {
            free(buf);
            buf = NULL;
        }
        if (msg->username)
        {
            free(msg->username); msg->username = NULL;
        }
        if (msg->load)
        {
            free(msg->load); msg->load = NULL;
        }
        break;

    case PRO_ID_COMMAND://根据发送的命令做出不同反应
        pro_msg_command_decode(msg, &comm_msg);
        if (strcmp(comm_msg.text, "/exit") == 0)//表示用户退出
        {
            pro_msg_command_pack(msg, servername, "/exit_ok");//回应用户请求
            buflen = pro_msg_send_buf(buf, msg);
            write(connfd, buf, buflen);
            remove_user(&room, comm_msg.username);
            if (buf)
            {
                free(buf); buf = NULL;
            }
            text = (char*)malloc(sizeof(char) * (strlen(comm_msg.username) + strlen("退出了聊天室\n")));//告知其他用户退出信息
            sprintf(text, "%s%s", comm_msg.username, "退出了聊天室");
            pro_msg_chattext_pack(msg, servername, text);
            buflen = pro_msg_send_buf(buf, msg);
            SendMsgToAllClients(&room, buf, buflen);
            if (buf)
            {
                free(buf); buf = NULL;
            }
            if (msg->load)
            {
                free(msg->load); msg->load = NULL;
            }
            //关闭连接和线程
            close(connfd);
            pthread_exit((void*)0);
        }
        if (strcmp(comm_msg.text, "/ls file") == 0)//请求发送文件列表
        {
            printf("用户%s请求发送聊天室文件列表\n", comm_msg.username);
            char* filelist = SendFileList(&room);//将文件列表打包成一个字符串
            SendMsgToOneClient(connfd, filelist);//向用户发送消息
            printf("已向用户%s请求发送聊天室文件列表\n", comm_msg.username);
            free(comm_msg.username); free(comm_msg.text); free(filelist);
        }
        break;

    case PRO_ID_SENDFILE://用户上传文件
        pro_msg_sendfile_decode(msg, &sfile_msg);
        SaveFile(&room, sfile_msg.filename, sfile_msg.file_size, sfile_msg.fileload);//保存文件
        free(sfile_msg.filename); free(sfile_msg.fileload);
        break;

    case PRO_ID_DOWNLOADFILE://用户请求下载文件
        pro_msg_downloadfile_decode(msg, &dwfile_msg);
        printf("收到用户%s请求发送文件%s\n", dwfile_msg.username, dwfile_msg.filename);
        if ((file_size = SendFile(&room, dwfile_msg.filename, file_buf)) < 0)//判断文件是否存在
        {
            SendMsgToOneClient(connfd, "聊天室文件夹中没有该文件");
        }
        else
        {
            SendFileToOneClient(connfd, dwfile_msg.filename, file_size, file_buf);
            printf("已向用户%s发送文件%s\n", dwfile_msg.username, dwfile_msg.filename);
        }
        free(dwfile_msg.filename); free(dwfile_msg.username); free(file_buf);
        break;

    default:
        break;
    }
}

//向某一用户发送聊天文本信息
void SendMsgToOneClient(int connfd, char* text)
{
    Protocol_t msg;
    uint8_t* buf;
    int buflen = 0;
    pro_msg_chattext_pack(&msg, servername, text);
    buflen = pro_msg_send_buf(buf, &msg);
    write(connfd, buf, buflen);
    free(buf);
}

//向某一用户发送文件
void SendFileToOneClient(int connfd, char* filename, int file_size, char* file_buf)
{
    Protocol_t msg;
    uint8_t* buf;
    int buflen = 0;
    pro_msg_sendfile_pack(&msg, filename, file_buf, file_size);
    buflen = pro_msg_send_buf(buf, &msg);
    write(connfd, buf, buflen);
    free(buf);
}
