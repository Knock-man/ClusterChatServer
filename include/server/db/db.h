#ifndef DB_H
#define DB_H

#include <mysql/mysql.h>
#include <string>


using namespace std;

class Mysql
{
public:
    //初始化数据库连接
    Mysql();
    //释放数据库连接
    ~Mysql();
    //连接数据库
    bool connect();
    
    //更新操作
    bool update(string sql);
    //查询操作
    MYSQL_RES* query(string sql);
    //获取连接
    MYSQL* getConnection();

private:
    MYSQL *_conn;
    string ip = "127.0.0.1"; 
    unsigned short port = 3306;
    string user = "xbj";
    string password = "xbj";
    string dbname = "ClusterChat";
};

#endif