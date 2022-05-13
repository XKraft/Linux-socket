#ifndef PROTOCOL
#define PROTOCOL

#include<stdint.h>
#include<malloc.h>
#include<string.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 这个头文件里面是自己实现的一个简单的通信协议，服务器和客户端通过这个协议定义的消息包进行消息传输，从而实现相应的功能
// 通过这个协议，可以实现不定长消息的传输，不局限与规定的缓冲区的大小
// 消息包的结构如下
// | MSG_ID |   USERNAME_LEN   |    LEN    |   USERNAME   |  LOAD  |
// |消息类型 |消息发送者名字的长度|消息负载长度|消息发送者名字 |消息负载 |
// | 32位   |       32位       |    32位    |    不定长    | 不定长  |
// 目前此协议消息类型有6种，每种消息都配备打包消息序列pack和解码消息包decode函数
// 此协议目前的问题是大量用到了动态规划，然而却没有内置归还内存，需要使用者手动归还内存
/////////////////////////////////////////////////////////////////////////////////////////////////////////////


//定义消息id
#define PRO_ID_UNKNOWN UNKNOWN //无法解析的命令
#define PRO_ID_CONNECT CONNECT //客户端第一次连接服务器发送此消息，用于服务器确认用户名是否重复
#define PRO_ID_ANSWER ANSWER //服务器应答客户端连接请求
#define PRO_ID_CHATTEXT CHATTEXT //服务器与客户端直接相互发生聊天文本消息
#define PRO_ID_SENDFILE SENDFILE //服务器和客户端之间相互发送文件（图片、语音、文本等）
#define PRO_ID_DOWNLOADFILE DOWNLOADFILE //客户端向服务器发送下载文件请求
#define PRO_ID_COMMAND COMMAND //客户端向服务器发送的命令，如退出聊天室、查看服务器中的文件等等

#define PRO_MSG_ANSWER_lEN 21

enum MSG_ID {
    UNKNOWN,
    CONNECT,
    ANSWER,
    CHATTEXT,
    SENDFILE,
    DOWNLOADFILE,
    COMMAND
};

typedef struct Protocol_t
{
    MSG_ID id;//消息包的消息id
    int username_len;//用户名长度
    int len;//消息负载长度

    char* username;//发生该消息的用户的用户名
    uint8_t* load;//消息负载
} Protocol_t;

typedef struct Pro_connect_t
{
    char* username;//发生该消息的用户的用户名
} Pro_connect_t;

typedef struct Pro_answer_t
{
    bool answer;//服务器的应答结果，true表示接收请求，加入聊天室
} Pro_answer_t;

typedef struct Pro_chattext_t
{
    char* username;//聊天内容发送者用户名
    char* text;//聊天文本内容
} Pro_chattext_t;

typedef struct Pro_command_t
{
    char* username;//命令发送者用户名
    char* text;//命令内容
} Pro_command_t;

typedef struct Pro_sendfile_t
{
    char* filename; //文件发送者的用户名
    int file_size; //文件的大小
    char* fileload; //文件内容
} Pro_sendfile_t;

typedef struct Pro_downloadfile_t
{
    char* username; //请求下载文件的用户的用户名
    char* filename; //请求下载的文件名
} Pro_downloadfile_t;

// 函数

//函数功能：消息包解析函数，用于从数据流中解析出消息包，解析成功返回true，并将结果存入Protocol_t类型的结构体中
//参数说明：param1 数据流中的一字节 param2 Protocol_t类型的结构体指针
extern bool pro_msg_parse(uint8_t byte, Protocol_t* msg);

//函数功能：将消息包打包成发送用的数据流，打包结果存入缓冲区buf中，并返回数据流长度
//参数说明：param1 缓冲区 param2 需要打包的Protocol_t类型的结构体指针
extern int pro_msg_send_buf(uint8_t*& buf, Protocol_t* msg);

//函数功能：PRO_ID_CONNECT消息的打包函数，将需要发送的消息打包到Protocol_t类型的结构体中
//参数说明：param1 Protocol_t类型的结构体指针 param2 请求连接的用户的用户名
extern void pro_msg_connect_pack(Protocol_t* msg, char* username);

//函数功能：PRO_ID_CONNECT消息的解码函数，将从数据流中解析出来的消息包解码成具体的消息内容，存入Pro_connect_t类型的结构体中
//参数说明：param1 Protocol_t类型的结构体指针 param2 Pro_connect_t类型的结构体指针
extern bool pro_msg_connect_decode(Protocol_t* msg, Pro_connect_t* _msg);

//函数功能：PRO_ID_ANSWER消息的打包函数，将需要发送的消息打包到Protocol_t类型的结构体中
//参数说明：param1 Protocol_t类型的结构体指针 param2 服务器名字，默认为server param3 服务器的回答结果，true表示接受加入聊天室
extern void pro_msg_answer_pack(Protocol_t* msg, char* username, bool answer);

//函数功能：PRO_ID_ANSWER消息的解码函数，将从数据流中解析出来的消息包解码成具体的消息内容，存入Pro_answer_t类型的结构体中
//参数说明：param1 Protocol_t类型的结构体指针 param2 Pro_answer_t类型的结构体指针
extern bool pro_msg_answer_decode(Protocol_t* msg, Pro_answer_t* _msg);

//函数功能：PRO_ID_CHATTEXT消息的打包函数，将需要发送的消息打包到Protocol_t类型的结构体中
//参数说明：param1 Protocol_t类型的结构体指针 param2 发送聊天文本的用户的用户名 param3 具体的聊天文本内容
extern void pro_msg_chattext_pack(Protocol_t* msg, char* username, char* text);

//函数功能：PRO_ID_CHATTEXT消息的解码函数，将从数据流中解析出来的消息包解码成具体的消息内容，存入Pro_chattext_t类型的结构体中
//参数说明：param1 Protocol_t类型的结构体指针 param2 Pro_chattext_t类型的结构体指针
extern bool pro_msg_chattext_decode(Protocol_t* msg, Pro_chattext_t* _msg);

//函数功能：PRO_ID_SENDFILE消息的打包函数，将需要发送的消息打包到Protocol_t类型的结构体中
//参数说明：param1 Protocol_t类型的结构体指针 param2 发送文件的用户的用户名 param3 文件内容 param4 文件大小
extern void pro_msg_sendfile_pack(Protocol_t* msg, char* filename, char* buf, int file_size);

//函数功能：PRO_ID_SENDFILE消息的解码函数，将从数据流中解析出来的消息包解码成具体的消息内容，存入Pro_sendfile_t类型的结构体中
//参数说明：param1 Protocol_t类型的结构体指针 param2 Pro_sendfile_t类型的结构体指针
extern bool pro_msg_sendfile_decode(Protocol_t* msg, Pro_sendfile_t* _msg);

//函数功能：PRO_ID_DOWNLOADFILE消息的打包函数，将需要发送的消息打包到Protocol_t类型的结构体中
//参数说明：param1 Protocol_t类型的结构体指针 param2 请求下载文件的用户的用户名 param3 文件名
extern void pro_msg_downloadfile_pack(Protocol_t* msg, char* username, char* filename);

//函数功能：PRO_ID_DOWNLOADFILE消息的解码函数，将从数据流中解析出来的消息包解码成具体的消息内容，存入Pro_downloadfile_t类型的结构体中
//参数说明：param1 Protocol_t类型的结构体指针 param2 Pro_downloadfile_t类型的结构体指针
extern bool pro_msg_downloadfile_decode(Protocol_t* msg, Pro_downloadfile_t* _msg);

//函数功能：PRO_ID_COMMAND消息的打包函数，将需要发送的消息打包到Protocol_t类型的结构体中
//参数说明：param1 Protocol_t类型的结构体指针 param2 发送命令的用户的用户名 param3 命令内容
extern void pro_msg_command_pack(Protocol_t* msg, char* username, char* text);

//函数功能：PRO_ID_COMMAND消息的解码函数，将从数据流中解析出来的消息包解码成具体的消息内容，存入Pro_command_t类型的结构体中
//参数说明：param1 Protocol_t类型的结构体指针 param2 Pro_command_t类型的结构体指针
extern bool pro_msg_command_decode(Protocol_t* msg, Pro_command_t* _msg);

#endif