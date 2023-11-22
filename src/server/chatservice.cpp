#include "public.h"
#include "chatservice.h"
#include "usermodel.h"

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
    int id = js["id"];
    string pwd = js["password"];
    User user = _userModel.query(id);//查询id用户数据存入user对象
    if(user.getPwd() == pwd && user.getId() == id)//查询到了用户，并且密码正确
    {
        if(user.getState() == "online")
        {
            //该用户已经登录，不允许重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "该账号已经登录，请重新输入新账号";
            conn->send(response.dump());//发回客户端

        }
        else
        {
        //登录成功，更新用户状态信息 state offline->online
        user.setState("online");
        _userModel.updateState(user);

        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        response["name"] = user.getName();
        response["errmsg"] = "用户名登录成功";
        conn->send(response.dump());//发回客户端
        }
        
    }
    else if(user.getPwd() != pwd && user.getId() == id)
    {
        //用户存但密码错误
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "用户存但密码错误";
        conn->send(response.dump());//发回客户端
    }
    else if(user.getId() != id)
    {
        //用户不存在
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 3;
        response["errmsg"] = "用户不存在";
        conn->send(response.dump());//发回客户端
    }

}
//处理注册业务
void ChatService::regster(const TcpConnectionPtr &conn,json &js,Timestamp time)
{
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = _userModel.insert(user);
    if(state)
    {
        //注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());//发回客户端
    }
    else
    {
        //注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());//发回客户端
    }

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