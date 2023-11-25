#include "json.hpp"
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <unordered_map>
#include <iomanip>
#include <sstream>
#include <functional>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <atomic>

#include "group.h"
#include "user.h"
#include "public.h"

using namespace std;
using json = nlohmann::json;

//记录当前系统登录的用户信息
User g_currentUser;

//记录当前登录用户的好友列表信息
vector<User> g_currentUserFriendList;

//记录挡墙登录用户的群组列表信息
vector<Group> g_currentUserGroupList;

//显示当前登录成功用户的基本信息
void showCurrentUserData();

//接受线程
void readTaskHandler(int clientfd);

//获取系统时间(聊天信息需要添加时间信息)
string getCurrentTime();

//主菜单页面程序
bool ismainMenuRuning = false;

//主聊天页面程序
void mainMenu(int clientfd);


//聊天客户端程序实现，main线程用作发送线程，子线程用作接受线程
int main(int argc,char *argv[])
{
    if(argc < 3)
    {
        cerr<<"command invalid example: ./ChatClient 127.0.0.1 6000"<<endl;
        exit(-1);
    }
    //解析命令行参数传递的ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    //创建clientsocket
    int clientfd = socket(AF_INET,SOCK_STREAM,0);
    if(clientfd == -1)
    {
        cerr<<"socket create error"<<endl;
        exit(0);
    }    
    //绑定服务器地址
    sockaddr_in server;
    memset(&server,0,sizeof(server));
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_port = htons(port);
    server.sin_family = AF_INET;

    //client 和 server 进行连接
    if(connect(clientfd,(sockaddr*)&server,sizeof(sockaddr_in)) == -1)
    {
        cerr<< "connect server error" <<endl;
        close(clientfd);
        exit(-1);
    }

    //main线程用户接受用户输入，负责发送数据
    for(;;)
    {
        //显示首页面菜单 登录 注册 退出
        cout<<"{============首页==============}"<<endl;
        cout<<"            "<<"1.登录"<<endl;
        cout<<"            "<<"2.注册"<<endl;
        cout<<"            "<<"3.退出"<<endl;
        cout<<"{==============================}"<<endl;
        cout<<"          "<<"选择:";
        int choice = 0;
        cin>>choice;
        cin.get();//读取器缓冲区残留的回车 输入一个整数，再读取字符串，需要将回车字符读掉，否则字符串会将回车字符读掉

        switch(choice)
        {
            case 1:
            {
                //登录
                int id =0;
                char pwd[50] = {0};
                cout<<"        "<<"账号:";
                cin>>id;
                cin.get();
                cout<<"        "<<"密码:";
                cin.getline(pwd,50);

                json js;
                js["msgid"] = LOGIN_MSG;
                js["id"] = id;
                js["password"] = pwd;
                string request = js.dump();

                int len = send(clientfd,request.c_str(),strlen(request.c_str())+1,0);
                if(len == -1)
                {
                    cerr<<"send login msg error"<<request<<endl;
                }
                else
                {
                    char buffer[900024] = {0};
                    len = recv(clientfd,buffer,sizeof(buffer),0);
                    if(len == -1)
                    {
                        cerr<<"recv login response error"<<endl;
                    }
                    else
                    {  
                        json responsejs = json::parse(buffer);
                        if(responsejs["errno"].get<int>() != 0)//登录失败
                        {
                            cerr<<responsejs["errmsg"]<<endl;

                        }
                        else//登录成功
                        {
                            //记录当前用户的id和name
                            g_currentUser.setId(responsejs["id"].get<int>());
                            g_currentUser.setName(responsejs["name"]);
                            //记录当前用户的好友列表信息
                            if(responsejs.contains("friends"))//当前用户有无好友
                            {
                                g_currentUserFriendList.clear();
                                vector<string> vec = responsejs["friends"];
                                for(string &str : vec)
                                {
                                    json js = json::parse(str);
                                    User user;
                                    user.setId(js["id"].get<int>());
                                    user.setName(js["name"]);
                                    user.setState(js["state"]);
                                    g_currentUserFriendList.push_back(user);
                                }

                            }
                           // cout<<"保存用户好友成功<<"<<endl;
                            //记录当前用户的群组列表信息
                            if(responsejs.contains("groups"))
                            {
                                g_currentUserGroupList.clear();
                                vector<string> vec1 = responsejs["groups"];
                                for(string &groupstr : vec1)
                                {
                                    json grpjs = json::parse(groupstr);
                                    Group group;
                                    group.setId(grpjs["id"].get<int>());
                                    group.setName(grpjs["groupname"]);
                                    group.setDesc(grpjs["groupdesc"]);

                                    vector<string> vec2 = grpjs["users"];
                                    for(string &userstr : vec2)
                                    {
                                        GroupUser user;
                                        json js = json::parse(userstr);
                                        user.setId(js["id"].get<int>());
                                        user.setName(js["name"]);
                                        user.setState(js["state"]);
                                        user.setRole(js["role"]);

                                        group.getUsers().push_back(user);
                                    }
                                    g_currentUserGroupList.push_back(group);
                                }
                            }
                            //cout<<"保存用户群组成功"<<endl;
                            //显示登录用户的基本信息
                            showCurrentUserData();
                            //cout<<"显示基本信息完成"<<endl;
                            //显示当前用户的离线信息 个人聊天信息或者群组信息
                            if(responsejs.contains("offlinemsg"))
                            {
                                vector<string> vec = responsejs["offlinemsg"];
                                for(string &str : vec)
                                {
                                   // cout<<"客户端登录离线json=:"<<str<<endl;
                                    json js = json::parse(str);
                                    
                                     int msgType = js["msgid"].get<int>();
        
                                    if(msgType == ONE_CHAT_MSG)//一对一发来的消息
                                    {
                                        cout<<endl<<"                  [聊天消息]"<<endl;
                                        cout<<"时间："<<js["time"].get<string>()<<"  账号："<<js["id"].get<int>()<<"  用户："<<js["name"].get<string>()<<"   说:"<<js["msg"].get<string>()<<endl<<endl;
                                        
                                    }
                                    else if(msgType == GROUP_CHAT_MSG)//群聊发来的消息
                                    {
                                        cout<<endl<<"                  [群聊 "<<js["groupid"].get<int>()<<" 消息]"<<endl;
                                        cout<<"时间："<<js["time"].get<string>()<<"  账号："<<js["id"].get<int>()<<"  用户："<<js["name"].get<string>()<<"   说:"<<js["msg"].get<string>()<<endl<<endl;
                                    }
                                }
                            }

                            //登录成功后，启动接受线程负责接受数据，该线程只启动一次
                            static int threadNumber = 0;
                            if(threadNumber == 0)
                            {
                                thread readTask(readTaskHandler,clientfd);
                                readTask.detach();
                                threadNumber++;
                            }
                            

                            //进入聊天主菜单页面
                            ismainMenuRuning = true;
                            mainMenu(clientfd);


                        }
                    }
                }

                break;
            }
            case 2:
            {
                //注册
                cout<<endl<<"===================注册用户======================"<<endl;
                char name[50] = {0};
                char pwd[50] = {0};
                cout<<"用户名:";
                cin.getline(name,50);
                cout<<"密码:";
                cin.getline(pwd,50);

                json js;
                js["msgid"] = REG_MSG;
                js["name"] = name;
                js["password"] = pwd;
                string request = js.dump();
                //strlen不包含字符串结束符'\0'  加一保证完整字符串发送
                int len = send(clientfd,request.c_str(),strlen(request.c_str())+1,0);
                if(len == -1)
                {
                    cerr<<"send register message error:"<<request<<endl;
                }
                else
                {
                    //读取服务器回应消息
                    char buffer[1024] = {0};
                    len = recv(clientfd,buffer,1024,0);
                    if(len == -1)
                    {
                        cerr<<"recv register responce error"<<endl;
                    }
                    else
                    {
                        json responsejs = json::parse(buffer);
                        if(responsejs["errno"].get<int>() !=0 )//注册失败
                        {
                            cout<<name<<"is already exist, register error!"<<endl;
                        }
                        else//注册成功
                        {
                            cout<<name<<"register success, userid is: "<<responsejs["id"]<<" ,do not forget it!"<<endl;
                        }
                    }
                }
                break;
            }
            case 3:
            {
                //退出
                close(clientfd);
                exit(0);
            }
            default:
                cerr<<"invalid input"<<endl;
                break;
        }
    }

    return 0;
}

//显示当前登录成功用户的基本信息
void showCurrentUserData()
{
    system("clear");
    cout<<endl<<"{============================登录用户============================}"<<endl;
    cout<<"                        账号:"<<g_currentUser.getId()<<"    姓名:"<<g_currentUser.getName()<<endl;
    cout<<"    -------------------------好友列表-----------------------      "<<endl;
    if(!g_currentUserFriendList.empty())
    {
        for(User &user:g_currentUserFriendList)
        {
            cout<<"                        "<<user.getId()<<" "<<user.getName()<<" "<<user.getState()<<endl;
        }
    }
    cout<<"    -------------------------群组成员-----------------------      "<<endl;
    if(!g_currentUserGroupList.empty())
    {
        for(Group &group : g_currentUserGroupList)
        {
            cout<<"    "<<"[群号："<<group.getId()<<" 群名："<<group.getName()<<" 群介绍："<<group.getDesc()<<"]"<<endl;
            cout<<"                           成员"<<endl;
            for(GroupUser &user : group.getUsers())
            {
                cout<<"            "<<"账号:"<<user.getId()<<"  姓名:"<<user.getName()<<" 在线:"<<user.getState()<<" 角色:"<<user.getRole()<<endl;
            }
        }cout<<endl;
    }
    cout<<"{==================================================================}"<<endl;;
}

// "help" command handler
void help(int fd = 0, string str = "");
// "chat" command handler
void chat(int, string);
// "addfriend" command handler
void addfriend(int, string);
// "creategroup" command handler
void creategroup(int, string);
// "addgroup" command handler
void addgroup(int, string);
// "groupchat" command handler
void groupchat(int, string);
// "loginout" command handler
void loginout(int, string);

// 系统支持的客户端命令列表
unordered_map<string, string> commandMap = {
    {"显示所有支持的命令","help"},
    { "一对一聊天","chat:friendid:message"},
    { "添加好友","addfriend:friendid"},
    {"创建群组","creategroup:groupname:groupdesc"},
    {"加入群组","addgroup:groupid"},
    {"群聊","groupchat:groupid:message"},
    {"注销","loginout"}};

// 注册系统支持的客户端命令处理
unordered_map<string, function<void(int, string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", loginout}
    };
//主聊天页面
void mainMenu(int clientfd)
{
    help();
    char buffer[1024]={0};
    while(ismainMenuRuning)
    {
        cin.getline(buffer,1024);
        string commandbuf(buffer);
        string command;//命令
        //"chat:friendid:message"
        int idx = commandbuf.find(":");//找到第一个":"位置
        if(idx == -1)
        {
            //没有":" 命令为 help 或者 loginout
            command = commandbuf;
        }
        else
        {
            //将命令截取出来
            command = commandbuf.substr(0,idx);
        }
        auto it = commandHandlerMap.find(command);//找支持命令中搜索这个命令
        if(it == commandHandlerMap.end())//输入命令有误
        {
            cerr<<"invalid input command!"<<endl;
            continue;
        }

        //调用相应命令的事件处理回调，mainMenu对修改封闭，添加新功能不需要修改该函数
        //it->second -> function<void(int, string)>
        it->second(clientfd,commandbuf.substr(idx+1,commandbuf.size()-idx));//调用事件处理回调
    }
    
}
void help(int,string)
{
    cout<<"                      "<<"----可支持命令列表----"<<endl;
    for(auto &p : commandMap)
    {
        cout<<"               "<<p.first<<"："<<p.second<<endl;
    }
    cout<<"===========================[聊天框]==================================";
    cout<<endl;
}
// "chat" command handler
void chat(int clientfd, string str)
{
    int idx = str.find(":");// str = friendid:message
    if(-1 == idx)
    {
        cerr<<"chat command invalid!"<<endl;
        return;
    }

    int friendid = atoi(str.substr(0,idx).c_str());
    string message = str.substr(idx +1,str.size()-idx);

    //组装json发送到服务器
    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["toid"] = friendid;
    js["msg"] = message;
    //cout<<"main 417 客户端一对一聊天 js[msg]="<<message<<endl;
    js["time"] = getCurrentTime();
    string buffer = js.dump();

    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(-1 == len)
    {
        cerr<<"send chat msg error ->"<<buffer<<endl;
    }

}
// 添加好友
void addfriend(int clientfd,string str)
{
    int friendid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;
    string buffer = js.dump();
    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(len == -1)
    {
        cerr<<"send addfriend msg error ->"<<buffer<<endl;
    }
    else
    {
        cout<<"添加好友成功！"<<endl<<endl;
    }
}
// 创建群组
void creategroup(int clientfd,string str)
{
    //str = creategroup:groupname:groupdesc
    int idx = str.find(":");
    if(-1 == idx)
    {
        cerr<<"creategroup command invalid!"<<endl;
        return;
    }
    string groupname = str.substr(0,idx);
    string groupdesc = str.substr(idx+1,str.size()-idx);

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    string buffer = js.dump();
    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(len == -1)
    {
        cerr<<"send creategroup msg error ->"<<buffer<<endl;
    }

}
// 加入群组
void addgroup(int clientfd,string str)
{
    //addgroup:groupid
    int groupid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;
    string buffer = js.dump();
    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(len == -1)
    {
        cerr<<"send addgroup msg error ->"<<buffer<<endl;
    }

}
// 群组聊天
void groupchat(int clientfd,string str)
{
    //groupchat:groupid:message
    int idx = str.find(":");
    if(-1 == idx)
    {
        cerr<<"groupchat command invalid!"<<endl;
        return;
    }
    int groupid = atoi(str.substr(0,idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);
    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buffer = js.dump();
    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(len == -1)
    {
        cerr<<"send groupchat msg error ->"<<buffer<<endl;
    }

}
// 注销
void loginout(int clientfd,string str)
{
    json js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = g_currentUser.getId();
    string buffer = js.dump();
    int len = send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(len == -1)
    {
        cerr<<"send loginout msg error ->"<<buffer<<endl;
    }

    ismainMenuRuning = false;
    system("clear");
}

//接受线程
void readTaskHandler(int clientfd)
{
    for(;;)
    {
        char buffer[1024] = {0};
        int len = recv(clientfd,buffer,1024,0);
        if(len == -1 || len == 0)
        {
            close(clientfd);
            exit(-1);
        }
        //接受服务器转发来的json字符串,序列化为JSON对象
        json js = json::parse(buffer);

        int msgType = js["msgid"].get<int>();
        
        if(msgType == ONE_CHAT_MSG)//一对一发来的消息
        {
            cout<<endl<<"                  [聊天消息]"<<endl;
            cout<<"时间："<<js["time"].get<string>()<<"  账号："<<js["id"].get<int>()<<"  用户："<<js["name"].get<string>()<<"   说:"<<js["msg"].get<string>()<<endl<<endl;
            continue;
        }
        
        if(msgType == GROUP_CHAT_MSG)//群聊发来的消息
        {
            cout<<endl<<"                  [群聊 "<<js["groupid"].get<int>()<<" 消息]"<<endl;
            cout<<"时间："<<js["time"].get<string>()<<"  账号："<<js["id"].get<int>()<<"  用户："<<js["name"].get<string>()<<"   说:"<<js["msg"].get<string>()<<endl<<endl;
            continue;
        }
    }
}

string getCurrentTime()
{
    std::time_t now = std::time(nullptr);
    std::tm* timeinfo = std::localtime(&now);

    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(4) << timeinfo->tm_year + 1900 << "-" 
        << std::setw(2) << timeinfo->tm_mon + 1     << "-" 
        << std::setw(2) << timeinfo->tm_mday        << " "
        << std::setw(2) << timeinfo->tm_hour        << ":"
        << std::setw(2) << timeinfo->tm_min         << ":"
        << std::setw(2) << timeinfo->tm_sec;

    return oss.str();
}