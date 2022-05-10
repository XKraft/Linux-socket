#include"../head/chatroom.h"
#include <iostream>
#include <fstream>
//初始化结构体chatroom
void init_chatroom(chatroom* room)
{
    room->user_head = new user_node;
    room->user_head->name = "";
    room->user_head->userfd = 0;
    room->user_head->next_user = NULL;

    room->number = 0;
    
    room->file_head = new file_node;
    room->file_head->filename = "";
    room->file_head->len = 0;
    room->file_head->next_file = NULL;
}

// //查找用户
// user_node* find_user(chatroom* room, string name)
// {
//     user_node* head = room->user_head;
//     user_node* p = NULL;
//     while(head->next_user != NULL)
//     {
//         p = head->next_user;
//         if(p->name == name) break;
//         head = p;
//     }
//     return p;
// }

//增加用户的同时判断是否重复
bool add_user(chatroom* room, string name, int userfd)
{
    user_node* head = room->user_head;
    user_node* p = NULL;
    while(head->next_user != NULL)      //判断是否有重复用户名
    {
        p = head->next_user;
        if(p->name == name)
        {
            return false;
            break;
        }
        head = p;
    }
    //头部插入
    
    room->number += 1;

    user_node* new_user = new user_node;
    new_user->name = name;
    
    new_user->userfd = userfd;
    new_user->next_user = room->user_head->next_user;
    room->user_head->next_user = new_user;
    head = p = NULL;

    return true;
}

void SendMsgToAllClients(chatroom* room, uint8_t* buf, int len)
{
    user_node* head = room->user_head;
    while(head->next_user)
    {
        write(head->next_user->userfd, buf, len);
        head = head->next_user;
    }
}
//删除用户
void remove_user(chatroom* room, string name)
{
    user_node* head = room->user_head;
    user_node* p = NULL;
    while(head->next_user != NULL)
    {
        p = head->next_user;
        if(p->name == name) break;
        head = p;
    }
    head->next_user = p->next_user;
    room->number -= 1;
    cout << "已删除用户"<< name <<endl;
    delete p;
    head = p = NULL;
}

// //查找文件
// file_node* find_file(chatroom* room, string filename)
// {
//     file_node* head = room->file_head;
//     file_node* p = NULL;
//     while(head->next_file != NULL)
//     {
//         p = head->next_file;
//         if(p->filename == filename) break;
//         head = p;
//     }
//     return p;
// }

// //从本地打开文件并发送
// uint8_t* send_file(chatroom* room, string filename)
// {
//     string file_path = "./file_dir" + filename;
//     ifstream ifs;
//     ifs.open(file_path, ios::in);
//     if(!ifs.is_open())
//     {
//         cout<<"open file failed...";
//         return 0;
//     }
//     uint8_t* str = new uint8_t;
//     int num = 0;
//     char c;
//     while((c = ifs.get()) != EOF)
//     {
//         *(str+(num++)) = (uint8_t)c;
//     }
//     return str;
// }