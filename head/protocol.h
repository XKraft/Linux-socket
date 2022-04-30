#ifndef PROTOCOL
#define PROTOCOL

#include<stdint.h>

//定义消息id
#define PRO_ID_CONNECT 0 //第一次连接
#define PRO_ID_CHATTEXT 1 //聊天文本
#define PRO_ID_SENDFILE 2 //向服务器发送文件
#define PRO_ID_DOWNLOADFILE 3 //从服务器下载文件
#define PRO_ID_COMMAND 4 //命令

enum MSG_ID{
    CONNECT,
    CHATTEXT,
    SENDFILE,
    DOWNLOADFILE,
    COMMAND
};

typedef struct Protocol_t
{

    MSG_ID id;//消息包的消息id
    char* username;//发生该消息的用户的用户名
    int len;//消息负载长度
    uint8_t* load;//消息负载
} Protocol_t;

typedef struct Pro_connect_t
{
    char* username;//发生该消息的用户的用户名
} Pro_connect_t;

typedef struct Pro_chattext_t
{
    char* username;
    char* text;
};

extern bool pro_msg_parse(uint8_t byte, Protocol_t* msg);
extern int pro_msg_send_buf(uint8_t* buf, Protocol_t* msg);

extern bool pro_msg_connect_pack(char* username, MSG_ID id);
extern bool pro_msg_connect_decode(Protocol_t* msg, Pro_connect_t* _msg);

extern bool pro_msg_chattext_pack(char* username, MSG_ID id, char* text);
extern bool pro_msg_chattext_decode(Protocol_t* msg, Pro_chattext_t* _msg);

// extern bool pro_msg_sendfile_pack(char* username, MSG_ID id);
// extern bool pro_msg_sendfile_decode(Protocol_t* msg, Pro_connect_t* _msg);

// extern bool pro_msg_downloadfile_pack(char* username, MSG_ID id);
// extern bool pro_msg_downloadfile_decode(Protocol_t* msg, Pro_connect_t* _msg);

// extern bool pro_msg_command_pack(char* username, MSG_ID id);
// extern bool pro_msg_command_decode(Protocol_t* msg, Pro_connect_t* _msg);

#endif