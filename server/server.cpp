#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<iostream>
using namespace std;

#define SERVER_IP "10.0.0.2" //ip地址
#define SERVER_PORT 3000 //端口号

int main()
{
    int sockfd = -1;
    sockaddr_in servaddr;

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//设置ip地址，使用这个宏定义好像可以自动分配，如果到时候不行的话可以手动填一下（字符串）
    servaddr.sin_port = htons(SERVER_PORT);//设置端口号

    if(-1 == (sockfd = socket(AF_INET, SOCK_STREAM, 0)))
    {
        cout << "error1:socket error!" << endl;
        return -1;
    }
    if(-1 == bind(sockfd, (sockaddr*)&servaddr, sizeof(servaddr)))
    {
        cout << "error2:bind error!" << endl;
        return -1;
    }
    if(-1 == listen(sockfd, 5))
    {
        cout << "error3:listen error!" << endl;
        return -1;
    }

}