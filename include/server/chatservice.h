#pragma once

#include<unordered_map>
#include<muduo/net/TcpConnection.h>
#include<functional>
#include<mutex>

#include "groupmodel.h"
#include "friendmodel.h"
#include "CommonConnectionPool.h"
#include "json.hpp"
#include "usermodel.h"
#include "offlinemessagemodel.h"
#include  "redis.h"
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
    //添加好友业务
    void addFriend(const TcpConnectionPtr &conn,json &js,Timestamp time);
    //一对一聊天业务
    void oneChat(const TcpConnectionPtr &conn,json &js,Timestamp time);

    //创建群组业务
    void createGroup(const TcpConnectionPtr &conn,json &js,Timestamp time);

    //加入群组业务
    void addGroup(const TcpConnectionPtr &conn,json &js,Timestamp time);

    //群组聊天业务
    void groupChat(const TcpConnectionPtr &conn,json &js,Timestamp time);

    //处理注销业务
    void loginout(const TcpConnectionPtr &conn,json &js,Timestamp time);

    //redis消息队列中获取订阅消息
    void handleRedisSubscribeMessage(int channel,string message);

    

    //获取消息对应的处理器
    MsgHandler getHandle(int msgid);

    //服务器异常，业务重置方法
    void reset();

    //处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);
private:
    ChatService();
    //存储消息id和对应的业务处理方法
    unordered_map<int,MsgHandler> _msgHandleMap;

    //存储在线用户的通信连接
    unordered_map<int,TcpConnectionPtr> _userConectionMap;

    //定义互斥锁保证_userConectionMap的线程安全
    mutex _connMutex;

    //数据操作类对象
    UserModel _userModel;

    OfflineMsgModel _offlineMsgMode;

    //好友操作类对象
    FriendModel _friendModel;

    //群组操作类
    GroupModel _groupModel;

    //redis操作对象
    Redis _redis;

    
    
 };