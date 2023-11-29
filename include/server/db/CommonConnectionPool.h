#ifndef COMMONCONNECTIONPOOL_H
#define COMMONCONNECTIONPOOL_H

#include<queue>
#include<mutex>
#include<atomic>
#include<thread>
#include<condition_variable>
#include<memory>
#include<functional>
#include<string>
#include<mysql/mysql.h>
#include<stdio.h>
#include<iostream>
#include "db.h"
using namespace std;
/*
实现连接池
*/
class ConnectionPool
{
    public:
        static ConnectionPool* getconnectionPool();//获取线程池
       shared_ptr<Mysql> getConnection();//外部获取空闲连接接口 返回智能指针
    private:
        ConnectionPool();//单例构造函数私有化

        bool loadConfigFile();//从配置文件中加载配置项

        //运行在独立线程中，专门负责生产新连接
        void produceConnectionTask();

        //运行在独立线程中，专门负责回收超时连接
        void recycleConnectionTask();
        
        //添加添加进队列
        void addConnection();

        string _ip;
        unsigned short _port;
        string _username;
        string _password;
        string _dbname;
        int _initSize;//连接池初始连接量
        int _maxSize;//最大连接量
        int _maxIdleTime;//最大空闲时间
        int _connectionTimeout;//获取连接的超时时间

        queue<Mysql*> _connectionQue;//存储mysql连接队列
        mutex _queueMutex;//维护线程安全互斥锁
        atomic<int> _connectionCnt;//记录连接所创建的connection连接的总数量,原子整型对象
        condition_variable cv;//设置条件变量，用于连接生产线程和消费线程通信
};

#endif