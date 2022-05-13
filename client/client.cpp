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

    printf("请输入用户名:");
    username = Str_get();

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
        if(process_text(text, sockfd, &msg, buflen, buf))
        {
            sleep(1);
            break;
        }
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
    Pro_command_t comm_msg;
    Pro_sendfile_t sfile_msg;
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
    case PRO_ID_COMMAND:
        pro_msg_command_decode(msg, &comm_msg);
        if(strcmp(comm_msg.text, "/exit_ok") == 0)
        {
            printf("已退出聊天室\n");
            close(sockfd);
            pthread_exit((void *)0);
        }
        break;

    case PRO_ID_SENDFILE:
        pro_msg_sendfile_decode(msg, &sfile_msg);
        SaveFile(sfile_msg.filename, sfile_msg.file_size, sfile_msg.fileload);
        free(sfile_msg.filename); free(sfile_msg.fileload);
        break;
    
    default:
        break;
    }
}

bool process_text(char* text, int sockfd, Protocol_t* msg, int buflen, uint8_t* buf)
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
        return false;
    }
    else
    {
        if(strcmp(text, "/exit") == 0)
        {
            pro_msg_command_pack(msg, username, text);
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
            return true;
        }
        if(strcmp(text, "/ls file") == 0)
        {
            RequestFileList(text, sockfd);
            free(text);
        }
        if(strcmp(text, "/sendfile") == 0)
        {
            printf("请输入发送文件所在路径:");
            char* file = Str_get();
            SendFile(file, sockfd);
            free(file); free(text);
        }
        if(strcmp(text, "/downloadfile") == 0)
        {
            printf("请输入想要下载的文件的文件名:");
            char* filename = Str_get();
            DownloadFile(filename, sockfd);
            free(filename); free(text);
        }
        return false;
    }
}

void SendFile(char* file, int sockfd)
{
    //从路径中获得文件名
    int filenamelen = 0;
    for(int i = strlen(file) - 1; i >=0; --i)
    {
        ++filenamelen;
        if(file[i] == '/')
            break;
    }
    char* filename = (char*)malloc(sizeof(char) * filenamelen);
    for(int i = 0, len = strlen(file); i < filenamelen; ++i)
        filename[i] = file[len - filenamelen + i];
    filename[filenamelen - 1] = '\0';
    printf("%s\n", filename);
    FILE* fp = fopen(file, "r");
    if(!fp)
    {
        printf("文件打开错误\n");
        return;
    }

    fseek(fp, 0, SEEK_END);
    int file_size = ftell(fp);
    char* file_buf = (char*)malloc(sizeof(char) * file_size);
    memset(file_buf, '\0', file_size * sizeof(char));
    fseek(fp, 0, SEEK_SET);
    fread(file_buf, sizeof(char), file_size, fp);

    Protocol_t msg;
    uint8_t* buf;
    int Buflen = 0;
    pro_msg_sendfile_pack(&msg, filename, file_buf, file_size);
    Buflen = pro_msg_send_buf(buf, &msg);
    write(sockfd, buf, Buflen);
    printf("已上传文件%s\n", filename);
    
    free(filename); free(file_buf); free(buf);
}

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

void SaveFile(char* filename, int file_size, char* fileload)
{
    FILE* fp = fopen(filename, "wb"); 
    if(!fp)
    {
        printf("文件打开错误\n");
        exit(-1);
    }
    fwrite(fileload, sizeof(char), file_size, fp);
    fclose(fp);
    printf("文件%s已保存\n", filename);
}