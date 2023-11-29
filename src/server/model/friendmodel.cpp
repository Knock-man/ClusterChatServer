#include "friendmodel.h"
#include "db.h"
#include "CommonConnectionPool.h"

//添加好友关系
bool FriendModel::insert(int userid, int friendid)
{
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"insert into Friend values(%d,%d)",userid,friendid);
    // 从连接池中取一个连接
    shared_ptr<Mysql> mysql = _cp->getConnection();
    if(mysql->update(sql)) return true;
    
    return false;
}

//返回用户好友列表
vector<User> FriendModel::query(int userid)
{
    vector<User> friendvec;
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"select id,name,state from User,Friend where userid = %d and User.id = friendid",userid);
    // 从连接池中取一个连接
    shared_ptr<Mysql> mysql = _cp->getConnection();
        MYSQL_RES* res = mysql->query(sql);
        if(res != nullptr)
        {
            //查询成功
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                friendvec.push_back(user);
            }
            mysql_free_result(res);
            return friendvec;
        }
    return friendvec;
}