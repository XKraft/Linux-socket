#include"../head/chatroom.h"

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

//将文件保存到服务区的file文件夹，并更新文件目录和目录文件
void SaveFile(chatroom* room, string filename, int file_size, char* fileload)
{
    //将文件保存到本地file文件夹
    char* file = (char*)malloc(sizeof(char) * (strlen("./file/") + filename.length()));
    sprintf(file, "%s%s", "./file/", filename.c_str());
    FILE* fp = fopen(file, "wb"); 
    if(!fp)
    {
        printf("文件打开错误\n");
        exit(-1);
    }
    fwrite(fileload, sizeof(char), file_size, fp);
    fclose(fp);
    printf("文件%s已保存\n", filename);

    //更新聊天室文件目录
    file_node* head = room->file_head;
    while(head->next_file)
    {
        if(filename == head->next_file->filename)
            return;
        head = head->next_file;
    }
    file_node* temp = new file_node;
    temp->filename = filename;
    temp->next_file = room->file_head->next_file;
    room->file_head->next_file = temp;
    head = temp = NULL;
    
    //更新文件目录文件
    fp = fopen("filelist.txt", "w");
    if(!fp)
    {
        printf("文件列表打开错误\n");
        exit(-1);
    }
    head = room->file_head;
    while(head->next_file)
    {
        fprintf(fp, "%s\n", head->next_file->filename);
        head = head->next_file;
    }
    head = NULL;
    fclose(fp);
    printf("文件目录已更新\n");
}