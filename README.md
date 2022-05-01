# README

server里是服务端文件

client里是客户端文件

创建了head文件夹里面存放头文件

创建了src文件夹里面存放源文件

myStr里面的函数用于处理字符串，所有有关处理字符串的函数都可以放到里面

chatroom里面定义了结构体的大致样子，并配置了初始化函数

protocol里面定义了消息包结构体，以及相应的函数，但是只是声明，需要去实现它，每个函数的具体功能可以参考mavlink

Makefile已编写好，可以用命令make client_app和make server_app分别编译客户端和服务端

file_dir是服务端存放文件的文件夹

---

chatroom中保存文件的函数还未实现->不知道服务端收到的文件流的形式，之后确定了再补充