#include<iostream>

#include "chatserver.h"

using namespace std;

int main(){
    EventLoop loop;
    InetAddress addr("127.0.0.1",8887);
    ChatServer server(&loop,addr,"chatserver");
    server.start();
    loop.loop();
    return 0;
}