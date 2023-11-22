#include "chatserver.h"
#include "chatservice.h"
#include "json.hpp"
#include <functional>
#include <iostream>
using namespace std;
using namespace placeholders;
using json = nlohmann::json;

//初始化聊天服务器对象
ChatServer::ChatServer(EventLoop* loop,
    const InetAddress& listenAddr,
    const string& nameArg):_server(loop,listenAddr,nameArg),_loop(loop)
    {
        //注册连接回调
        _server.setConnectionCallback(bind(&ChatServer::onConnection,this,_1));
        //注册通信回调
        _server.setMessageCallback(bind(&ChatServer::onMessage,this,_1,_2,_3));
        //设置线程数量
        _server.setThreadNum(4);

    }

//启动服务
void ChatServer::start()
{
    _server.start();
}

//上报链接相关信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    //客户端断开连接
    if(!conn->connected())
    {
        conn->shutdown();
    }
}
//上报读写事件相关信息的回调函数
void ChatServer::onMessage(const TcpConnectionPtr &conn,
                        Buffer *buffer,
                        Timestamp time)
{
    string buf = buffer -> retrieveAllAsString();//从缓冲区中取数据
    json js = json::parse(buf);//数据的反序列化
    //目的：完全解耦网络模块代码和业务模块代码
    //通过js["msgid"] 获取一个业务 Handler
    MsgHandler msgHandler = ChatService::instance()->getHandle(js["msgid"].get<int>());
    msgHandler(conn,js,time);
     
}
