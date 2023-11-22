#ifndef USERMODEL_CPP
#define USERMODEL_CPP

#include "usermodel.h"
#include "db.h"

#include<iostream>
using namespace std;

//user表增加方法
bool UserModel::insert(User &user)
{
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"insert into User(name,password,state) values('%s','%s','%s')",
        user.getName().c_str(),user.getPwd().c_str(),user.getState().c_str());
    //2 建立连接 执行sql语句
    Mysql mysql;
    if(mysql.connect()){
        if(mysql.update(sql))
        {
            //获取一下插入成功的用户数据生成的主键id  mysql提供了全局方法 mysql_insert_id
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }

    }
    return false;

}

//根据用户id查询用户信息
User UserModel::query(int id)
{
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"select * from User where id=%d",id);
    //2 建立连接 执行sql语句
    Mysql mysql;
    if(mysql.connect()){
        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr)
        {
            //查询成功
            MYSQL_ROW row =  mysql_fetch_row(res);
            if(row != nullptr)
            {
                //查询有数据
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
            }
        }
       

    }
    return User();
}


//更新用户的状态信息
bool UserModel::updateState(User &user)
{
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"update User set state = '%s' where id = %d",user.getState().c_str(),user.getId());
    //2 建立连接 执行sql语句
    Mysql mysql;
    if(mysql.connect()){
        if(mysql.update(sql)) return true;
    }
    return false;
}
#endif