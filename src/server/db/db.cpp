#include"db.h"
#include "muduo/base/Logging.h"
#include <string>
#include<mysql/mysql.h>
#include<stdio.h>
#include<iostream>
using namespace std;
// 数据库操作类

    // 初始化数据库连接
    Mysql::Mysql()
    {
        _conn = mysql_init(nullptr);
    }

    // 释放数据库连接资源
    Mysql::~Mysql()
    {
        if (_conn != nullptr)
        mysql_close(_conn);
    }

    // 连接数据库
    bool Mysql::connect(string ip, unsigned short port, string user, string password,string dbname)
    {
        MYSQL *p = mysql_real_connect(_conn,ip.c_str(),user.c_str(),password.c_str(),dbname.c_str(),port,nullptr,0);
        if(p == nullptr)
        {
            return false;
        }
        //C和C++代码默认的编码字符是ASCII，如果不设置，从mysql拉下来的中文显示 ?
        mysql_query(_conn,"set names gbk");
        return true;
    }
    

    // 更新操作 insert、delete、update
    bool Mysql::update(string sql)
    {
        if (mysql_query(_conn, sql.c_str()))
        {
            LOG_ERROR<<"更新失败";
            return false;
        }
        return true;
    }

    // 查询操作 select
    MYSQL_RES* Mysql::query(string sql)
    {
        if (mysql_query(_conn, sql.c_str()))
        {
            LOG_ERROR<<"查询失败";
            return nullptr;
        }
        return mysql_use_result(_conn);
    }
    //刷新一下连接起始的空闲时间点
    void Mysql::refreshAliveTime(){
        _alivetime = clock();
    }
    //返回存活的时间段
    clock_t Mysql::getAlieTime() const{
        return clock() - _alivetime;

    }

    //返回当前连接
    MYSQL * Mysql::getConnection()
    {
        return _conn;
    }