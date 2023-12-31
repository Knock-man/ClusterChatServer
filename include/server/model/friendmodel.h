#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H

#include <vector>
#include "user.h"
#include "CommonConnectionPool.h"
using namespace std;
//维护好友消息的操作接口方法
class FriendModel
{
public:

    //添加好友关系
    bool insert(int userid, int friendid);

    //返回用户好友列表
    vector<User> query(int userid);

private:
    ConnectionPool *_cp = ConnectionPool::getconnectionPool();;
};

#endif