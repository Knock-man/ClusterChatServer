#include<iostream>
#include "db.h"
#include "chatserver.h"

using namespace std;

int main(){
    EventLoop loop;
    InetAddress addr("127.0.0.1",8909);
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
{"msgid":5,"fromid":1,"fromname":"zhangsan","toid":4,"msg":"dsfds"}


*/