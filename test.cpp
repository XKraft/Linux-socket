#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include<malloc.h>
#include <stdlib.h>

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

typedef struct Pro_chattext_t
{
    char* username;
    char* text;
};

bool parse(uint8_t byte, Protocol_t* pro);

char* Str_get()
{
    char c;
    char* str = NULL;
    int size = 0;
    while ((c = getchar()) != '\n')
    {
        if (size == 0)
        {
            str = (char*)malloc(sizeof(char) * (size + 1));
            if (NULL == str) exit(-1);
            str[size] = c;
        }
        else
        {
            str = (char*)realloc(str, size + 1);
            if (NULL == str) exit(-1);
            str[size] = c;
        }
        ++size;
    }
    str = (char*)realloc(str, size + 1);
    if (NULL == str) exit(-1);
    str[size] = '\0';
    return str;

}

int main()
{
    Protocol_t p;
    printf("请输入用户名：");
    char* username = Str_get();
    printf("请输入要发送的信息：");
    char* str = Str_get();

    //pack
    printf("正在pack成结构体!\n");
    //uint8_t head = 0xFE;
    p.id = CONNECT;
    p.username = username;
    p.username_len = strlen(p.username);
    p.load = (uint8_t*)str;
    p.len = strlen(str) * sizeof(char) / sizeof(uint8_t);
    //printf("%ld %ld %ld %ld %ld %ld %ld \n", sizeof(head), sizeof(p.id), sizeof(p.username), sizeof(p.username_len), sizeof(p.load), sizeof(p.len), sizeof(p));
    //printf("%s\n", (char*)p.load);

    //send_buf
    printf("正在把结构体打包成消息包系列\n");
    int len = 1 + sizeof(p.id) + sizeof(p.username_len) + sizeof(p.len) + strlen(p.username) * sizeof(char) / sizeof(uint8_t) + p.len + 1;
    //printf("%d\n", len);
    uint8_t* pack = (uint8_t*)malloc(sizeof(uint8_t) * len);
    pack[0] = 0xFE;
    int flag = 1;
    for (int i = 0; i < sizeof(p.id); ++i)
        pack[flag + i] = ((uint8_t*)(&p.id))[i];
    flag += sizeof(p.id);
    for (int i = 0; i < sizeof(p.username_len); ++i)
        pack[flag + i] = ((uint8_t*)(&p.username_len))[i];
    flag += sizeof(p.username_len);
    for (int i = 0; i < sizeof(p.len); ++i)
        pack[flag + i] = ((uint8_t*)(&p.len))[i];
    flag += sizeof(p.len);
    for (int i = 0; i < strlen(p.username) * sizeof(char) / sizeof(uint8_t); ++i)
        pack[flag + i] = ((uint8_t*)(p.username))[i];
    flag += strlen(p.username) * sizeof(char) / sizeof(uint8_t);
    for (int i = 0; i < p.len; ++i)
        pack[flag + i] = p.load[i];
    flag += p.len;
    pack[flag] = 0xFD;

    for (int i = 0; i < len; ++i)
    {
        printf("%x ", pack[i]);
    }
    printf("\n");

    //parse
    printf("正在解析消息序列\n");
    Protocol_t _p;
    for (int i = 0; i < len; ++i)
    {
        if (parse(pack[i], &_p))
        {
            if (_p.id == CONNECT)
            {
                printf("解析成功!\n");
                //decode
                printf("正在decode!\n");
                Pro_chattext_t chattext_msg;
                chattext_msg.username = _p.username;
                chattext_msg.username = (char*)realloc(chattext_msg.username, _p.username_len + 1);
                chattext_msg.username[_p.username_len] = '\0';
                chattext_msg.text = (char*)_p.load;
                chattext_msg.text = (char*)realloc(chattext_msg.text, _p.len * sizeof(uint8_t) / sizeof(char) + 1);
                chattext_msg.text[_p.len * sizeof(uint8_t) / sizeof(char)] = '\0';
                printf("%s:%s\n", chattext_msg.username, chattext_msg.text);
            }
        }
    }
    
    






    free(pack);

    return 0;
}

bool parse(uint8_t byte, Protocol_t* pro)
{
    static uint8_t* p = NULL;
    static int i = 0;
    static int j = 0;
    static Protocol_t _p;
    static long lenarray[5] = { sizeof(_p.id), sizeof(_p.username_len), sizeof(_p.len), 0, 0 };
    static bool flag = false;
    if (byte == 0xFE)
    {
        if (flag == false)
        {
            flag = true;
            i = 0;
            j = 0;
            p = (uint8_t*)malloc(sizeof(uint8_t) * lenarray[j]);
            return false;
        }
    }
    if (flag == true)
    {
        if (i < lenarray[j])
        {
            p[i++] = byte;
            return false;
        }
        else
        {
            switch (j)
            {
            case 0:
                _p.id = *((MSG_ID*)p);
                break;
            case 1:
                _p.username_len = *((int*)p); lenarray[3] = _p.username_len;
                break;
            case 2:
                _p.len = *((int*)p); lenarray[4] = _p.len;
            case 3:
                _p.username = (char*)p;
            case 4:
                _p.load = p;
            default:
                break;
            }
            ++j;
            i = 0;
            if (j < 5)
            {
                p = (uint8_t*)malloc(sizeof(uint8_t) * lenarray[j]);
                p[i++] = byte;
                return false;
            }
            else
            {
                flag = false;
                if (byte == 0xFD)
                {    
                    *pro = _p;
                    return true;
                }
                return false;
            }
        }
    }
}