#include "groupmodel.h"
#include "db.h"
#include "groupuser.h"
//创建群组
bool GroupModel::createGroup(Group &group)
{
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"insert into AllGroup(groupname,groupdesc) values('%s','%s')",
            group.getName().c_str(),group.getDesc().c_str());
    //2 建立连接 执行sql语句
    Mysql mysql;
    if(mysql.connect()){
        if(mysql.update(sql))
        {
            //获取一下插入成功的用户数据生成的主键id  mysql提供了全局方法 mysql_insert_id
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }

    }
    return false;
}

//加入群组
void GroupModel::addGroup(int userid, int groupid, string role)
{
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"insert into GroupUser values(%d,%d,'%s')",
            groupid,userid,role.c_str());
    //2 建立连接 执行sql语句
    Mysql mysql;
    if(mysql.connect()){
        mysql.update(sql);

    }
}

//查询用户所在的群组信息
vector<Group> GroupModel::queryGroup(int userid)
{
    vector<Group> Groupvec;
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"select AllGroup.id,AllGroup.groupname,AllGroup.groupdesc from `AllGroup`, GroupUser \
    where AllGroup.id = GroupUser.groupid and GroupUser.userid = %d",userid);
    //2 建立连接 执行sql语句
    Mysql mysql;
    if(mysql.connect()){
        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr)
        {
            //查询成功
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                Groupvec.push_back(group);
            }
            mysql_free_result(res);
      }
    }
    
    //查询群组的用户
    for(Group &group : Groupvec)
    {
        sprintf(sql,"SELECT User.id,User.name,User.state,GroupUser.grouprole \
        from User,GroupUser WHERE GroupUser.userid = User.id and GroupUser.groupid = %d",group.getId());

        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group.getUsers().push_back(user);
            }
            mysql_free_result(res);
        }
    }
    return Groupvec;
}

//根据用户指定的group查询群组用户id列表，除userid自己，主要用户群聊业务给群组其他成员群发消息
vector<int> GroupModel::queryGroupUsers(int userid,int groupid)
{
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"select userid from GroupUser WHERE groupid = %d and userid != %d",groupid,userid);
    //2 建立连接 执行sql语句
    vector<int> idVec;
    Mysql mysql;
    if(mysql.connect()){
        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr)
        {
            //查询成功
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                idVec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
      }
    }
    return idVec;
}