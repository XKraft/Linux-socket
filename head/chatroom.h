#ifndef _CHATROOM
#define _CHATROOM
#include<stdio.h>

typedef struct user_node
{
    char* name;
    int port;
} user_node;


typedef struct chatroom
{
    user_node* user_head;
    int number;
    bool send_flag;
    char* msg;
} chatroom;

extern void init_chatroom(chatroom* room);

#endif