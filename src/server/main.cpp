#include<iostream>
#include<signal.h>

#include "db.h"
#include "chatservice.h"
#include "chatserver.h"

using namespace std;
//处理服务器ctrl+c结束后，重置user的状态信息
void resetHandler(int signal)
{
    ChatService::instance()->reset();
    exit(0);
}
int main(int argc, char *argv[]){
    //解析通过命令行参数传递ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);
    signal(SIGINT,resetHandler);
    EventLoop loop;
    InetAddress addr(ip,port);
    ChatServer server(&loop,addr,"chatserver");
    server.start();
    loop.loop();
    
    return 0;
}

/*
登录
{"msgid":1,"id":2,"password":"1234"}


注册
{"msgid":3,"name":"lisi","password":"123456"}

加好友
{"msgid":6,"id":2,"friendid":4}

聊天：
{"msgid":6,"fromid":1,"fromname":"zhangsan","toid":4,"msg":"dsfds"}


*/