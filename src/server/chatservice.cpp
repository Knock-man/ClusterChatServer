#include "public.h"
#include "chatservice.h"
#include <string>
#include <muduo/base/Logging.h>
using namespace muduo;
using namespace std;

//注册消息以及对应的handler回调操作
ChatService::ChatService()
{
   _msgHandleMap[LOGIN_MSG] = bind(&ChatService::login,this,_1,_2,_3);
   _msgHandleMap[REG_MSG] = bind(&ChatService::regster,this,_1,_2,_3);
}

 //获取单例对象的接口函数
ChatService* ChatService::instance()
{
   static ChatService service;
   return &service;
}

//处理登录业务
void ChatService::login(const TcpConnectionPtr &conn,json &js,Timestamp time)
{
    LOG_INFO<<"do login service";
}
//处理注册业务
void ChatService::regster(const TcpConnectionPtr &conn,json &js,Timestamp time)
{
    LOG_INFO<<"do regster service";
}

 MsgHandler ChatService::getHandle(int msgid)
 {
    //记录错误日志，msgid没有对应的事件处理回调
    auto it = _msgHandleMap.find(msgid);
    if(it == _msgHandleMap.end())
    {
        //返回一个默认处理器，空操作
        return [=](const TcpConnectionPtr&,json&,Timestamp)
        {
            LOG_ERROR<< "msgid:"<<msgid<<"can not find handler!";
        };
    }
    else
    {
        return _msgHandleMap[msgid];
    }
    
 }