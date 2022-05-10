#ifndef _CHATROOM
#define _CHATROOM
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <unistd.h>
using namespace std;

typedef struct user_node
{
    string name;
    int userfd;
    user_node* next_user;
} user_node;

typedef struct file_node
{
    string filename;
    int len;
    file_node* next_file;
} file_node;

typedef struct chatroom
{
    user_node* user_head;
    int number;
    file_node* file_head;
} chatroom;

//初始化聊天室
extern void init_chatroom(chatroom* room);
//查找用户
// extern user_node* find_user(chatroom* room, string name);
// //增加用户，增加时需要判断用户名是否重复
extern bool add_user(chatroom* room, string name, int userfd);
//向所有人发送消息
extern void SendMsgToAllClients(chatroom* room, uint8_t* buf, int len);
//删除用户
extern void remove_user(chatroom* room, string name);
// //接收&保存文件->将字符数组写入文件
// extern void save_file(chatroom* room, uint8_t* str, int len, string filename);
// //发送文件->将服务端文件写入字符数组
// extern uint8_t* send_file(chatroom* room, string filename);
// //查找文件
// extern file_node* find_file(chatroom* room, string filename);
#endif