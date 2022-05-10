#include"../head/protocol.h"

int pro_msg_send_buf(uint8_t* &buf, Protocol_t* msg)
{
    int len = 1 + sizeof(msg->id) + sizeof(msg->username_len) + sizeof(msg->len) + strlen(msg->username) * sizeof(char) / sizeof(uint8_t) + msg->len + 1;

    uint8_t* pack = (uint8_t*)malloc(sizeof(uint8_t) * len);

    pack[0] = 0xFE;
    int flag = 1;

    for (int i = 0; i < sizeof(msg->id); ++i)
        pack[flag + i] = ((uint8_t*)(&msg->id))[i];
    flag += sizeof(msg->id);

    for (int i = 0; i < sizeof(msg->username_len); ++i)
        pack[flag + i] = ((uint8_t*)(&msg->username_len))[i];
    flag += sizeof(msg->username_len);

    for (int i = 0; i < sizeof(msg->len); ++i)
        pack[flag + i] = ((uint8_t*)(&msg->len))[i];
    flag += sizeof(msg->len);

    for (int i = 0; i < strlen(msg->username) * sizeof(char) / sizeof(uint8_t); ++i)
        pack[flag + i] = ((uint8_t*)(msg->username))[i];
    flag += strlen(msg->username) * sizeof(char) / sizeof(uint8_t);

    for (int i = 0; i < msg->len; ++i)
        pack[flag + i] = msg->load[i];
    flag += msg->len;

    pack[flag] = 0xFD;

    buf = pack;

    return len;
}

bool pro_msg_parse(uint8_t byte, Protocol_t* msg)
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
                break;
            case 3:
                _p.username = (char*)p;
                break;
            case 4:
                _p.load = p;
                break;
            default:
                break;
            }
            ++j;
            i = 0;
            if (j < 5 && lenarray[j] != 0)
            {
                if(j < 3)
                    if (p)
                    {
                        free(p); p = NULL;
                    }
                p = (uint8_t*)malloc(sizeof(uint8_t) * lenarray[j]);
                p[i++] = byte;
                return false;
            }
            else
            {
                flag = false;
                if (byte == 0xFD)
                {
                    *msg = _p;
                    return true;
                }
                return false;
            }
        }
    }
}

void pro_msg_connect_pack(Protocol_t* msg, char* username)
{
    msg->id = PRO_ID_CONNECT;
    msg->len = 0;
    msg->username_len = strlen(username);
    msg->username = username;
    msg->load = NULL;
}
bool pro_msg_connect_decode(Protocol_t* msg, Pro_connect_t* _msg)
{
    if(msg->id == PRO_ID_CONNECT)
    {
        _msg->username = msg->username;
        _msg->username = (char*)realloc(_msg->username, msg->username_len + 1);
        _msg->username[msg->username_len] = '\0';
        return true;
    }
    return false;
}

void pro_msg_answer_pack(Protocol_t* msg, char* username, bool answer)
{
    msg->id = PRO_ID_ANSWER;
    msg->username = username;
    msg->username_len = strlen(username);
    msg->len = 1;
    msg->load = (uint8_t*)malloc(sizeof(uint8_t));
    msg->load[0] = (uint8_t)answer;
}
bool pro_msg_answer_decode(Protocol_t* msg, Pro_answer_t* _msg)
{
    if(msg->id == PRO_ID_ANSWER)
    {
        _msg->answer = (bool)(*msg->load);
        if(msg->username)
        {
            free(msg->username); msg->username = NULL;
        }
        if(msg->load)
        {
            free(msg->load); msg->load =NULL;
        }
        return true;
    }
    return false;
}

void pro_msg_chattext_pack(Protocol_t* msg, char* username, char* text)
{
    msg->id = PRO_ID_CHATTEXT;
    msg->username = username;
    msg->username_len = strlen(username);
    msg->load = (uint8_t*)text;
    msg->len = strlen(text) * sizeof(char) / sizeof(uint8_t);
}
bool pro_msg_chattext_decode(Protocol_t* msg, Pro_chattext_t* _msg)
{
    if(msg->id == PRO_ID_CHATTEXT)
    {
        _msg->username = msg->username;
        _msg->username = (char*)realloc(_msg->username, msg->username_len + 1);
        _msg->username[msg->username_len] = '\0';
        _msg->text = (char*)msg->load;
        _msg->text = (char*)realloc(_msg->text, msg->len * sizeof(uint8_t) / sizeof(char) + 1);
        _msg->text[msg->len * sizeof(uint8_t) / sizeof(char)] = '\0';
        return true;
    }
    return false;
}

void pro_msg_command_pack(Protocol_t* msg, char* username, char* text)
{
    msg->id = PRO_ID_COMMAND;
    msg->username = username;
    msg->username_len = strlen(username);
    msg->load = (uint8_t*)text;
    msg->len = strlen(text) * sizeof(char) / sizeof(uint8_t);
}
bool pro_msg_command_decode(Protocol_t* msg, Pro_command_t* _msg)
{
    if(msg->id == PRO_ID_COMMAND)
    {
        _msg->username = msg->username;
        _msg->username = (char*)realloc(_msg->username, msg->username_len + 1);
        _msg->username[msg->username_len] = '\0';
        _msg->text = (char*)msg->load;
        _msg->text = (char*)realloc(_msg->text, msg->len * sizeof(uint8_t) / sizeof(char) + 1);
        _msg->text[msg->len * sizeof(uint8_t) / sizeof(char)] = '\0';
        return true;
    }
    return false;
}

void pro_msg_sendfile_pack(Protocol_t* msg, char* filename, char* buf, int file_size)
{
    msg->id = PRO_ID_SENDFILE;
    msg->username = filename;
    msg->username_len = strlen(filename);
    msg->load = (uint8_t*)buf;
    msg->len = file_size * sizeof(char) / sizeof(uint8_t);
}
bool pro_msg_sendfile_decode(Protocol_t* msg, Pro_sendfile_t* _msg)
{
    if (msg->id == PRO_ID_SENDFILE)
    {
        _msg->fileload = (char*)msg->load;
        _msg->file_size = msg->len * sizeof(uint8_t) / sizeof(char);
        _msg->filename = msg->username;
        _msg->filename = (char*)realloc(_msg->filename, msg->username_len + 1);
        _msg->filename[msg->username_len] = '\0';
        return true;
    }
    return 0;
}