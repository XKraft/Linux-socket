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

    FILE* fp = fopen("filelist.txt", "r");
    if(!fp)
    {
        printf("文件列表打开错误\n");
        exit(-1);
    }
    char* filename;
    fscanf(fp, "%s", filename);
    while(!feof(fp))
    {
        fscanf(fp, "%s", filename);
        file_node* temp = new file_node;
        temp->filename = filename;
        temp->next_file = room->file_head->next_file;
        room->file_head->next_file = temp;
    }
}

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

//查找文件目录里是否有这个文件，如果有则返回true
bool FindFile(chatroom* room, string filename)
{
    file_node* head = room->file_head;
    while(head->next_file)
    {
        if(filename == head->next_file->filename)
            return true;
        head = head->next_file;
    }
    return false;
}

//将文件保存到服务区的file文件夹，并更新文件目录和目录文件
void SaveFile(chatroom* room, string filename, int file_size, char* fileload)
{
    //将文件保存到本地file文件夹
    //char* file = (char*)malloc(sizeof(char) * (strlen("./file/") + filename.length()));
    //sprintf(file, "%s%s", "./file/", filename.c_str());
    string file = "./file/";
    file.append(filename);
    FILE* fp = fopen(file.c_str(), "wb"); 
    if(!fp)
    {
        printf("文件打开错误\n");
        exit(-1);
    }
    fwrite(fileload, sizeof(char), file_size, fp);
    fclose(fp);
    printf("文件%s已保存\n", filename.c_str());

    //更新聊天室文件目录
    if(FindFile(room, filename))
        return;
    file_node* temp = new file_node;
    temp->filename = filename;
    temp->next_file = room->file_head->next_file;
    room->file_head->next_file = temp;
    temp = NULL;
    
    //更新文件目录文件
    fp = fopen("filelist.txt", "w");
    if(!fp)
    {
        printf("文件列表打开错误\n");
        exit(-1);
    }
    file_node* head = room->file_head;
    while(head->next_file)
    {
        fprintf(fp, "%s\n", head->next_file->filename.c_str());
        head = head->next_file;
    }
    head = NULL;
    fclose(fp);
    printf("文件目录已更新\n");
}

//读取文件内容，将其存入file_buf中，返回文件大小file_size
int SendFile(chatroom* room, string filename, char*& file_buf)
{
    if(FindFile(room, filename))
    {
        string file = "./file/";
        file.append(filename);
        FILE* fp = fopen(file.c_str(), "rb");
        if (!fp)
        {
            printf("文件打开错误\n");
            return -1;
        }

        fseek(fp, 0, SEEK_END);
        int file_size = ftell(fp);
        file_buf = (char*)malloc(sizeof(char) * file_size);
        memset(file_buf, '\0', file_size * sizeof(char));
        fseek(fp, 0, SEEK_SET);
        fread(file_buf, sizeof(char), file_size, fp);
        fclose(fp);

        return file_size;   
    }
    else
        return -1;
}

char* SendFileList(chatroom* room)
{
    string filelist;
    file_node* head = room->file_head;
    while(head->next_file)
    {
        filelist += head->next_file->filename;
        filelist.append(" ");
        head = head->next_file;
    }
    char* _filelist = (char*)malloc(sizeof(char) * (filelist.length() + 1));
    strcpy(_filelist, filelist.c_str());
    return _filelist;
}

