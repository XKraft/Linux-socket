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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 本文件中实现了客户端的具体功能
// 客户端的工作流程大概是:程序启动后，先输入用户名，然后以TCP协议与服务器初步建立连接，发送一个CONNECT消息序列，服务器收
// 到后，发送ANSWER消息序列应答，如果用户名不是重复的，就会接受请求，服务器就会和客户端正式建立连接，此时会创建一个子线程，
// 用于读取服务器发送的数据流，并解析出来，然后做出相应的反应，而主线程则会让用户输入命令或者聊天内容，以"/"开头的为命令，
// 否则为聊天文本，两个线程并发进行，直到用户输入退出命令，则会告诉服务器将要退出服务器，连接关闭，程序终止。
// 已经实现的内容
// 1.发送聊天文本，显示其他用户的聊天内容
// 2.查看聊天室中的文件列表
// 3.上传文件到聊天室中，包括文本、图片、音频等等类型
// 4.从聊天室中下载文件
// 5.退出聊天室
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SERVER_IP "127.0.0.1" //服务器ip地址
#define SERVER_PORT 3000 //端口号
#define HELP_NUM 5 //help数组的元素个数

const char* help[] = {
    "帮助菜单:",
    "/exit 退出聊天室",
    "/ls file 查看聊天室文件列表",
    "/sendfile 上传文件到聊天室",
    "/downloadfile 从聊天室下载文件"
};//用于打印帮助菜单

//全局变量
char* username = NULL;

//函数声明
bool SendUsername(int sockfd, char* username, uint8_t* buf, int buflen, Protocol_t* msg);
void* ReadServer(void* arg);
void process_msg(Protocol_t* msg, int sockfd);
bool process_text(char* text, int sockfd, Protocol_t* msg, int buflen, uint8_t* buf);
void SendFile(char* file, int sockfd);
void DownloadFile(char* filename, int sockfd);
void RequestFileList(char* command, int sockfd);
void SaveFile(char* filename, int file_size, char* fileload);

int main()
{
    struct sockaddr_in servaddr;
    int sockfd;
    char* text = NULL;
    uint8_t* buf = NULL;
    int buflen = 0;
    Protocol_t msg;

    //让用户输入用户名
    printf("请输入用户名:");
    username = Str_get();

    //通过TCP协议与服务器连接
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr);//设置服务器ip地址
    servaddr.sin_port = htons(SERVER_PORT);//设置端口号

    if (-1 == (sockfd = socket(AF_INET, SOCK_STREAM, 0)))
    {
        printf("error1:socket error!\n");
        return -1;
    }

    if (connect(sockfd, (sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("error2:unable to connect to server!\n");
        return -2;
    }

    //向服务器发送用户名,判断用户名是否已存在,如果存在则结束程序进行
    if (!SendUsername(sockfd, username, buf, buflen, &msg))
        return -3;

    //新建一个子线程,用于读取服务器发送的数据,并解析出来,作出反应
    pthread_t tid;
    pthread_create(&tid, NULL, ReadServer, (void*)&sockfd);
    pthread_detach(tid);

    //主循环,用于让用户输入命令,聊天文本等
    while (1)
    {
        text = Str_get();
        if (process_text(text, sockfd, &msg, buflen, buf))//如果返回true,则结束程序
        {
            sleep(1);
            break;
        }
    }

    return 0;
}

//第一次连接服务器时,向服务器发送用户名,以便服务器判断用户名是否已存在
bool SendUsername(int sockfd, char* username, uint8_t* buf, int buflen, Protocol_t* msg)
{
    //发送用户名
    Pro_answer_t answer_msg;
    pro_msg_connect_pack(msg, username);
    buflen = pro_msg_send_buf(buf, msg);
    write(sockfd, buf, buflen);
    if (buf)
    {
        free(buf); buf = NULL;
    }
    if (msg->load)
    {
        free(msg->load); msg->load = NULL;
    }

    //读取服务器应答结果
    buf = (uint8_t*)malloc(sizeof(uint8_t) * PRO_MSG_ANSWER_lEN);
    buflen = read(sockfd, buf, PRO_MSG_ANSWER_lEN);

    for (int i = 0; i < buflen; ++i)
    {
        //解析消息包序列
        if (pro_msg_parse(buf[i], msg))
        {
            if (buf)
            {
                free(buf); buf = NULL;
            }
            if (pro_msg_answer_decode(msg, &answer_msg))//解码内容
            {
                if (1 == answer_msg.answer)
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
    if (buf)
    {
        free(buf); buf = NULL;
    }
    printf("登陆失败!\n");
    return false;
}

//线程函数,用于不断读取服务器发送的数据,并做出反应
void* ReadServer(void* arg)
{
    int sockfd = *(int*)arg;
    uint8_t buf;
    Protocol_t msg;
    while (1)
    {
        read(sockfd, &buf, 1);
        if (pro_msg_parse(buf, &msg))
        {
            process_msg(&msg, sockfd);
        }
    }
}

//用于根据解析到的消息id,做出不同的反应
void process_msg(Protocol_t* msg, int sockfd)
{
    Pro_chattext_t chat_msg;
    Pro_command_t comm_msg;
    Pro_sendfile_t sfile_msg;
    switch (msg->id)
    {
    case PRO_ID_CHATTEXT://如果是聊天文本,则打印出来
        pro_msg_chattext_decode(msg, &chat_msg);
        if (strcmp(chat_msg.username, username) != 0)//确保自己发送的消息不会再次打印出来
            printf("%s:%s\n", chat_msg.username, chat_msg.text);
        if (chat_msg.text)
        {
            free(chat_msg.text);
            chat_msg.text = NULL;
        }
        if (chat_msg.username)
        {
            free(chat_msg.username);
            chat_msg.username = NULL;
        }
        break;

    case PRO_ID_COMMAND://如果是命令,判断是哪个命令
        pro_msg_command_decode(msg, &comm_msg);
        if (strcmp(comm_msg.text, "/exit_ok") == 0)//此命令表示服务器已受到退出聊天室请求,之后会关闭连接,关闭子进程
        {
            printf("已退出聊天室\n");
            close(sockfd);
            pthread_exit((void*)0);
        }
        break;

    case PRO_ID_SENDFILE://如果是服务器发送的文件,则将其保存到本地,保存位置为程序所在的根目录
        pro_msg_sendfile_decode(msg, &sfile_msg);
        SaveFile(sfile_msg.filename, sfile_msg.file_size, sfile_msg.fileload);
        free(sfile_msg.filename); free(sfile_msg.fileload);
        break;

    default:
        break;
    }
}

//此函数用于出来用户输入的内容,如果以"/"开头,表示是命令,否则为聊天文本
bool process_text(char* text, int sockfd, Protocol_t* msg, int buflen, uint8_t* buf)
{
    if (text[0] != '/')
    {
        //聊天文本之间发送给服务器
        pro_msg_chattext_pack(msg, username, text);
        buflen = pro_msg_send_buf(buf, msg);
        write(sockfd, buf, buflen);
        if (msg->load)
        {
            free(msg->load);
            msg->load = NULL;
        }
        if (buf)
        {
            free(buf);
            buf = NULL;
        }
        return false;
    }
    else
    {
        if (strcmp(text, "/exit") == 0)//退出聊天室命令,先向服务器发送退出命令,返回true让主循环结束
        {
            pro_msg_command_pack(msg, username, text);
            buflen = pro_msg_send_buf(buf, msg);
            write(sockfd, buf, buflen);
            if (buf)
            {
                free(buf); buf = NULL;
            }
            if (msg->load)
            {
                free(msg->load); msg->load = NULL;
            }
            return true;
        }
        else if (strcmp(text, "/ls file") == 0)//查看聊天室文件列表命令
        {
            RequestFileList(text, sockfd);
            free(text);
        }
        else if (strcmp(text, "/sendfile") == 0)//上传文件到聊天室命令,会要求用户输入文件所在路径
        {
            printf("请输入发送文件所在路径:");
            char* file = Str_get();
            SendFile(file, sockfd);
            free(file); free(text);
        }
        else if (strcmp(text, "/downloadfile") == 0)//请求下载文件命令,会要求用户输入文件名
        {
            printf("请输入想要下载的文件的文件名:");
            char* filename = Str_get();
            DownloadFile(filename, sockfd);
            free(filename); free(text);
        }
        else if (strcmp(text, "/help") == 0)//帮助命令,向用户输出帮助菜单
        {
            for (int i = 0; i < HELP_NUM; ++i)
            {
                printf("%s\n", help[i]);
            }
        }
        else
            printf("未知的命令，输入/help查看帮助\n");//提示用户命令无效,可以通过help查看帮助
        return false;
    }
}

//向服务器发送文件
void SendFile(char* file, int sockfd)
{
    //从路径中获得文件名
    int filenamelen = 0;
    for (int i = strlen(file) - 1; i >= 0; --i)
    {
        ++filenamelen;
        if (file[i] == '/')
            break;
    }
    char* filename = (char*)malloc(sizeof(char) * filenamelen);
    for (int i = 0, len = strlen(file); i < filenamelen; ++i)
        filename[i] = file[len - filenamelen + 1 + i];
    filename[filenamelen - 1] = '\0';

    //以二进制只读的方式打开想要发送的文件
    FILE* fp = fopen(file, "rb");
    if (!fp)
    {
        printf("文件打开错误\n");
        return;
    }

    //得到文件的大小和文件的具体内容
    fseek(fp, 0, SEEK_END);
    int file_size = ftell(fp);
    char* file_buf = (char*)malloc(sizeof(char) * file_size);
    memset(file_buf, '\0', file_size * sizeof(char));
    fseek(fp, 0, SEEK_SET);
    fread(file_buf, sizeof(char), file_size, fp);

    //通过自定义协议向服务器发送文件
    Protocol_t msg;
    uint8_t* buf;
    int Buflen = 0;
    pro_msg_sendfile_pack(&msg, filename, file_buf, file_size);
    Buflen = pro_msg_send_buf(buf, &msg);
    write(sockfd, buf, Buflen);
    printf("已上传文件%s\n", filename);
    fclose(fp);

    free(filename); free(file_buf); free(buf);
}

//通过自定义协议向服务器发送下载文件到本地的请求
void DownloadFile(char* filename, int sockfd)
{
    Protocol_t msg;
    uint8_t* buf;
    int Buflen = 0;
    pro_msg_downloadfile_pack(&msg, username, filename);
    Buflen = pro_msg_send_buf(buf, &msg);
    write(sockfd, buf, Buflen);
    free(buf);
}

//通过自定义协议请求服务器发送聊天室文件列表
void RequestFileList(char* command, int sockfd)
{
    Protocol_t msg;
    uint8_t* buf;
    int Buflen = 0;
    pro_msg_command_pack(&msg, username, command);
    Buflen = pro_msg_send_buf(buf, &msg);
    write(sockfd, buf, Buflen);
    free(buf);
}

//根据解析出来的文件名,文件大小,文件具体内容,将文件保存到本地
void SaveFile(char* filename, int file_size, char* fileload)
{
    FILE* fp = fopen(filename, "wb");
    if (!fp)
    {
        printf("文件打开错误\n");
        exit(-1);
    }
    fwrite(fileload, sizeof(char), file_size, fp);
    fclose(fp);
    printf("文件%s已保存\n", filename);
}