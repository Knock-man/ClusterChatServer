#ifndef OFFLINEMESSAGEMODE_H
#define OFFLINEMESSAGEMODE_H

#include<string>
#include<vector>
using namespace std;
//提供离线消息表的操作方法
class OfflineMsgModel
{
public:
    //存储用户的离线消息
    void insertOffMsg(int userid,string msg);

    //删除用户的离线消息
    void removeOffMsg(int userid);

    //查询用户的离线消息
    vector<string> queryOffMsg(int userid);
};

#endif