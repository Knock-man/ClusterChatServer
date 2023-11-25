#include "offlinemessagemodel.h"
#include "db.h"

#include "iostream"
using namespace std;
//存储用户的离线消息
void OfflineMsgModel::insertOffMsg(int userid,string msg)
{
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"insert into offlineMessage values(%d,'%s')",userid,msg.c_str());
    //cout<<"offlineMessage 12 存储进数据库的json="<<msg<<endl;
    //2 建立连接 执行sql语句
    Mysql mysql;
    if(mysql.connect()){
        mysql.update(sql);
    }
}

//删除用户的离线消息
void OfflineMsgModel::removeOffMsg(int userid)
{
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"delete from offlineMessage where userid = %d",userid);
    //2 建立连接 执行sql语句
    Mysql mysql;
    if(mysql.connect()){
        mysql.update(sql);
    }
}

//查询用户的离线消息
vector<string> OfflineMsgModel::queryOffMsg(int userid)
{
    vector<string> vec;
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"select message from offlineMessage where userid = %d",userid);
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
                //把userid用户的所有离线消息放入vec中返回
                //cout<<"OfflineMsgModel::queryOffMsg  51 提取离线消息："<<row[0]<<endl;
                vec.push_back(row[0]);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}