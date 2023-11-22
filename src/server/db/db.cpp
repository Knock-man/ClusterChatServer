#include "db.h"

#include <muduo/base/Logging.h>

//初始化数据库连接
 Mysql::Mysql()
 {
    _conn = mysql_init(nullptr);
 }

//释放数据库连接
 Mysql::~Mysql()
{
    if(_conn != nullptr)
    {
        mysql_close(_conn);
    }
 }
//连接数据库
bool  Mysql::connect()
{
       MYSQL *p = mysql_real_connect(_conn,ip.c_str(),user.c_str(),password.c_str(),dbname.c_str(),port,nullptr,0);
       if(p == nullptr)
       {
            LOG_INFO << "connect mysql fail!";
           return false;
       }
       //C和C++代码默认的编码字符是ASCII，如果不设置，从mysql拉下来的中文显示 ?
       mysql_query(_conn,"set names gbk");
       LOG_INFO << "connect mysql success!";
       return true;
}
    
//更新操作
 bool  Mysql::update(string sql)
 {
    if(mysql_query(_conn,sql.c_str()))
    {
        LOG_INFO<<__FILE__<<":"<<__LINE__<<":"<<sql<<" "<<"更新失败!";
        return false;
    }
     return true;
}
//查询操作
MYSQL_RES*  Mysql::query(string sql)
{
    if(mysql_query(_conn,sql.c_str()))
    {
        LOG_INFO<<__FILE__<<":"<<__LINE__<<":"<<sql<<"查询失败!";
        return nullptr;
    }
   return mysql_use_result(_conn);
}

//获取连接
MYSQL* Mysql::getConnection()
{
    return _conn;
}