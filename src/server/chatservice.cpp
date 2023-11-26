#include "public.h"
#include "chatservice.h"
#include "usermodel.h"

#include <string>
#include <muduo/base/Logging.h>
#include <map>
#include <vector>
#include <iostream>


using namespace muduo;
using namespace std;


//注册消息以及对应的handler回调操作
ChatService::ChatService()
{
   _msgHandleMap[LOGIN_MSG] = bind(&ChatService::login,this,_1,_2,_3);
   _msgHandleMap[REG_MSG] = bind(&ChatService::regster,this,_1,_2,_3);
   _msgHandleMap[ADD_FRIEND_MSG] = bind(&ChatService::addFriend,this,_1,_2,_3);
   _msgHandleMap[ONE_CHAT_MSG] = bind(&ChatService::oneChat,this,_1,_2,_3);
   _msgHandleMap[CREATE_GROUP_MSG] = bind(&ChatService::createGroup,this,_1,_2,_3);
   _msgHandleMap[ADD_GROUP_MSG] = bind(&ChatService::addGroup,this,_1,_2,_3);
   _msgHandleMap[GROUP_CHAT_MSG] = bind(&ChatService::groupChat,this,_1,_2,_3);
   _msgHandleMap[LOGINOUT_MSG] = bind(&ChatService::loginout,this,_1,_2,_3);
    
   //连接redis服务器
   if(_redis.connect())
   {   
        //设置上报消息的回调
        _redis.init_notify_handler(bind(&ChatService::handleRedisSubscribeMessage,this,_1,_2));
   }

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
            {
                //登录成功记录用户连接信息
                lock_guard<mutex> lock(_connMutex);
                _userConectionMap.insert({id,conn});
            }   

            //id用户登录成功后，向redis订阅channel(id)
            _redis.subscribe(id);
        
        //登录成功，更新用户状态信息 state offline->online
        user.setState("online");
        _userModel.updateState(user);

        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        
        response["errno"] = 0;
        response["id"] = user.getId();
        response["name"] = user.getName();
        response["errmsg"] = "用户登录成功";

        //查询该用户是否有离线消息
        vector<string> vec = _offlineMsgMode.queryOffMsg(id);
        if( !vec.empty() )
        {
            response["offlinemsg"] = vec;
            
            //读取该用户的离线消息后，把该用户的所有离线消息删除掉
            _offlineMsgMode.removeOffMsg(id);
        }
        //查询该用户的好友信息并返回
        vector<User> userVec = _friendModel.query(id);
        if(!userVec.empty())//用户有好友
        {
            vector<string> vec;
            for(User &user : userVec)
            {
                json js;
                js["id"] = user.getId();
                js["name"] = user.getName();
                js["state"] = user.getState();
                vec.push_back(js.dump());
            }
            response["friends"] = vec;
        }
        //查询用户的群组信息
        vector<Group> groupuserVec = _groupModel.queryGroup(id);
        if(!groupuserVec.empty())
        {
            vector<string> groupV;
            for(Group &group : groupuserVec)
            {
                json grpjson;
                grpjson["id"] = group.getId();
                grpjson["groupname"] = group.getName();
                grpjson["groupdesc"] = group.getDesc();
                vector<string> userV;
                for(GroupUser &user :group.getUsers())
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    js["role"] = user.getRole();
                    userV.push_back(js.dump());
                }
                grpjson["users"] = userV;
                groupV.push_back(grpjson.dump());
            }
            response["groups"] = groupV;
        }
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

//添加好友业务 msgid id friendid 
void ChatService::addFriend(const TcpConnectionPtr &conn,json &js,Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    //存储好友消息
    _friendModel.insert(userid,friendid);

}
//过去消息对应的回调函数
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
//服务器异常，业务重置方法
void ChatService::reset()
{
    //把所online状态的用户，设置成offline
    _userModel.resetState();
    exit(0);
}
 //处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        
        //查找conn连接对应的用户id
        lock_guard<mutex> lock_(_connMutex);
        for(auto it = _userConectionMap.begin(); it != _userConectionMap.end(); it++)
        {
            if(it->second == conn)
            {
                user.setId(it->first);
                //从map表删除用户的连接信息
                _userConectionMap.erase(it);
                break;
            }
        }

    }

    //用户异常退出，，在redis中取消订阅通道
    _redis.unsubscribe(user.getId());

    //更新用户的状态信息
    if(user.getId() != -1)
    {
        user.setState("offline");
        _userModel.updateState(user);
    }
    
}


//一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr &conn,json &js,Timestamp time)
{
    //{"msgid": "fromid": "fromname":  "toid":  "msg": }
    int toid = js["toid"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConectionMap.find(toid);
        if(it != _userConectionMap.end())//本服务器在线
        {
            //toid在线，转发消息 服务器主动推送消息给toid用户
           it->second->send(js.dump());
           return;

        }
    }
    
    //查询toid是否在线，本地_userConectionMap没有，可能登录在其他服务器上在线
    User user = _userModel.query(toid);
    if(user.getState() == "online")//在线，在其他电脑登录
    {
        _redis.publish(toid,js.dump());//通过redis转发
        return;
    }
    
    // toid不在线，存储离线消息
    _offlineMsgMode.insertOffMsg(toid, js.dump());
}

//创建群组业务
void ChatService::createGroup(const TcpConnectionPtr &conn,json &js,Timestamp time)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    //存储新创建的群组消息
    Group group(-1,name,desc);
    if(_groupModel.createGroup(group))
    {
        //存储群组创建人信息
        _groupModel.addGroup(userid,group.getId(),"creator");
    }
    
}

//加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn,json &js,Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid,groupid,"normal");
}

//群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn,json &js,Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid,groupid);//返回该用户所在群的所有成员
    lock_guard<mutex> lock(_connMutex);
    for(auto id : useridVec)//遍历所有userid
    {    
        auto it = _userConectionMap.find(id);//查找userid对应的连接
        if(it != _userConectionMap.end())//该用户本地服务器在线,在map表有连接
        {
            //转发消息
            it->second->send(js.dump());
        }
        else//该用户本地服务器不在线，可能在其他服务器在线或者不在线
        {
            User user = _userModel.query(id);
            if(user.getState() == "online")//用户其它服务器在线
            {
                _redis.publish(id,js.dump());
            }
            else//不在线，存储离线消息
            {
                _offlineMsgMode.insertOffMsg(id,js.dump());
            }
            
        }
    }
}

//处理注销业务
void ChatService::loginout(const TcpConnectionPtr &conn,json &js,Timestamp time)
{
    int userid = js["id"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConectionMap.find(userid);
        if(it != _userConectionMap.end())
        {
            _userConectionMap.erase(it);
        }
    }

    //用户注销，相当于就是下线，在redis中取消订阅通道
    _redis.unsubscribe(userid);

    //更新用户的状态信息
    User user(userid,"","","offline");
    _userModel.updateState(user);
            
}

//redis消息队列中获取订阅消息
void ChatService::handleRedisSubscribeMessage(int userid,string msg)
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConectionMap.find(userid);//寻找userid对应的conn
    if(it != _userConectionMap.end())
    {
        it->second->send(msg);//发送
        return;
    }

    //redeis转发过程中用户下线，存入离线消息
    _offlineMsgMode.insertOffMsg(userid,msg);
}