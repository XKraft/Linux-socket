#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include"../head/myStr.h"
#include"../head/protocol.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 3000 //端口号

Protocol_t msg;

char* Str_get();

int main()
{
    struct sockaddr_in servaddr;
    int sockfd;
    char* username = NULL;
    
    printf("请输入用户名(十个英文字符内):");
    username = Str_get();
    while(strlen(username) > 10)
    {
        printf("用户名输入错误,请不超过10个英文字符!请重新输入:");
        username = Str_get();
    }
    //printf("%ld\n", sizeof(*username) * strlen(username));

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
    else
    {
        int buflen = 0;
        pro_msg_connect_pack(username, PRO_ID_CONNECT);
        buflen = pro_msg_send_buf(buf, &msg);
        write(sockfd, buf, buflen);
        read(sockfd, )
    }

    return 0;
}
