#ifndef _CHATROOM
#define _CHATROOM
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <iostream>
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
//增加用户，增加时需要判断用户名是否重复
extern bool add_user(chatroom* room, string name, int userfd);
//向所有人发送消息
extern void SendMsgToAllClients(chatroom* room, uint8_t* buf, int len);
//删除用户
extern void remove_user(chatroom* room, string name);
//接收&保存文件->将字符数组写入文件
extern void SaveFile(chatroom* room, string filename, int file_size, char* fileload);
//发送文件->将服务端文件写入字符数组
extern int SendFile(chatroom* room, string filename, char*& file_buf);

extern char* SendFileList(chatroom* room);
#endif