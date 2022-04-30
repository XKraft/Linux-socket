#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include"../head/myStr.h"

#define SERVER_PORT 3000 //端口号

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

    return 0;
}
