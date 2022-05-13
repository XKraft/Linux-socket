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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 这里实现了一个聊天室结构体，目的是记录加入聊天室的成员名单、聊天室中的文件
// chatroom这一结构体中定义一个用户链表，一个文件链表
// 特别注意的聊天室在本地存有一个文件目录日志文件filelist.txt，里面保存了上传到聊天室的文件袋文件名，文件链表在每次启动服务器时的
// 初始化中，会读取日志文件里的内容，并加载到链表中，以保证每次服务器关机后聊天室文件不会丢失
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//用户链表节点
typedef struct user_node
{
    string name;//用户名
    int userfd;//用户的读写操作符
    user_node* next_user;
} user_node;

//文件链表节点
typedef struct file_node
{
    string filename;//文件名
    file_node* next_file;
} file_node;

//聊天室结构体
typedef struct chatroom
{
    user_node* user_head;//用户链表
    int number;//当前用户人数
    file_node* file_head;//文件链表
} chatroom;

//函数作用:初始化聊天室，并将日志文件里的内容加载到文件链表中
//参数说明:param1 想要操作的聊天室的指针
extern void init_chatroom(chatroom* room);

//函数作用:判断用户名是否存在，如果不存在则加入到用户链表中，并返回true
//参数说明:param1 想要操作的聊天室的指针 param2　想要加入的用户的用户名　param3 想要加入的用户的读写操作符
extern bool add_user(chatroom* room, string name, int userfd);

//函数作用:向聊天室中的所有成员发送消息文本
//参数说明:param1 想要操作的聊天室的指针 param2　已经打包好的消息文本数据流　param3 数据流长度
extern void SendMsgToAllClients(chatroom* room, uint8_t* buf, int len);

//函数作用:从用户链表中删除某一用户
//参数说明:param1 想要操作的聊天室的指针 param2　想要删除的用户的用户名
extern void remove_user(chatroom* room, string name);

//函数作用:将用户上传的文件保存到文件夹中，文件名存入文件链表，并更新文件目录日志文件
//参数说明:param1 想要操作的聊天室的指针 param2　上传的文件袋文件名　param3 文件大小　param4 文件具体内容
extern void SaveFile(chatroom* room, string filename, int file_size, char* fileload);

//函数作用:从文件夹中读取文件，打包成数据流，返回文件大小
//参数说明:param1 想要操作的聊天室的指针 param2　用户想要下载的文件的文件名　param3 文件具体内容
extern int SendFile(chatroom* room, string filename, char*& file_buf);

//函数作用:向客户端发送聊天室的文件目录，以返回值发送
//参数说明:param1 想要操作的聊天室的指针
extern char* SendFileList(chatroom* room);
#endif