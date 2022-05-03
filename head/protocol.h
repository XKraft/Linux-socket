#ifndef PROTOCOL
#define PROTOCOL

#include<stdint.h>
#include<malloc.h>
#include<string.h>

//定义消息id
#define PRO_ID_UNKNOWN UNKNOWN
#define PRO_ID_CONNECT CONNECT //第一次连接
#define PRO_ID_ANSWER ANSWER //应答连接请求
#define PRO_ID_CHATTEXT CHATTEXT //聊天文本
#define PRO_ID_SENDFILE SENDFILE //向服务器发送文件
#define PRO_ID_DOWNLOADFILE DOWNLOADFILE //从服务器下载文件
#define PRO_ID_COMMAND COMMAND //命令

#define PRO_MSG_ANSWER_lEN 21

enum MSG_ID{
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
    bool answer;//发生该消息的用户的用户名
} Pro_answer_t;

typedef struct Pro_chattext_t
{
    char* username;
    char* text;
} Pro_chattext_t;

extern bool pro_msg_parse(uint8_t byte, Protocol_t* msg);
extern int pro_msg_send_buf(uint8_t* &buf, Protocol_t* msg);

extern void pro_msg_connect_pack(Protocol_t* msg, char* username);
extern bool pro_msg_connect_decode(Protocol_t* msg, Pro_connect_t* _msg);

extern void pro_msg_answer_pack(Protocol_t* msg, char* username, bool answer);
extern bool pro_msg_answer_decode(Protocol_t* msg, Pro_answer_t* _msg);

extern void pro_msg_chattext_pack(Protocol_t* msg, char* username, char* text);
extern bool pro_msg_chattext_decode(Protocol_t* msg, Pro_chattext_t* _msg);

// extern bool pro_msg_sendfile_pack(Protocol_t* msg, char* username, MSG_ID id);
// extern bool pro_msg_sendfile_decode(Protocol_t* msg, Pro_connect_t* _msg);

// extern bool pro_msg_downloadfile_pack(Protocol_t* msg, char* username, MSG_ID id);
// extern bool pro_msg_downloadfile_decode(Protocol_t* msg, Pro_connect_t* _msg);

// extern bool pro_msg_command_pack(Protocol_t* msg, char* username, MSG_ID id);
// extern bool pro_msg_command_decode(Protocol_t* msg, Pro_connect_t* _msg);

#endif