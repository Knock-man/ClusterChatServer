#ifndef CONNECTION_H
#define CONNECTION_H

#include<mysql/mysql.h>
#include<ctime>
#include<string>
using namespace std;

// 数据库操作类
class Mysql
{
public:
    // 初始化数据库连接
    Mysql();
    // 释放数据库连接资源
    ~Mysql();
    // 连接数据库
    bool connect(string ip, unsigned short port, string user, string password,string dbname);
    // 更新操作 insert、delete、update
    bool update(string sql);
    // 查询操作 select
    MYSQL_RES* query(string sql);

    //刷新一下连接起始的空闲时间点
    void refreshAliveTime();
    
    //返回存活的时间段
    clock_t getAlieTime() const;//常量成员函数，不能对类成员数据进行修改

    //返回当前连接
    MYSQL * getConnection();

private:
    MYSQL *_conn; // 表示和MySQL Server的一条连接
    clock_t _alivetime;//进入空闲状态的起始时间点
};

#endif