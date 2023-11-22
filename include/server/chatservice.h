#pragma once

#include<unordered_map>
#include<muduo/net/TcpConnection.h>
#include<functional>

#include"json.hpp"
using namespace std;
using namespace muduo::net;
using namespace muduo;
using json = nlohmann::json;

//表示处理消息的事件回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr&,json&,Timestamp)>;
//单例模式
 class ChatService
 {
public:
    //获取单例对象的接口函数
    static ChatService* instance();
    ChatService(const ChatService&) = delete;
    ChatService& operator=(const ChatService&) = delete;

    //处理登录业务
    void login(const TcpConnectionPtr &conn,json &js,Timestamp time);
    //处理注册业务
    void regster(const TcpConnectionPtr &conn,json &js,Timestamp time);
    //获取消息对应的处理器
    MsgHandler getHandle(int msgid);
private:
    ChatService();
    //存储消息id和对应的业务处理方法
    unordered_map<int,MsgHandler> _msgHandleMap;
 };