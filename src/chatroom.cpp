#include"../head/chatroom.h"
//初始化结构体chatroom
void init_chatroom(chatroom* room)
{
    room->number = 0;
    room->send_flag = false;
    room->user_head = NULL;
    room->msg = NULL;
}