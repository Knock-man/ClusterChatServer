#include"CommonConnectionPool.h"
#include"muduo/base/Logging.h"

#include "json.hpp"
#include <fstream>
using json = nlohmann::json;

//线程安全的懒汉单例函数接口
ConnectionPool* ConnectionPool::getconnectionPool(){
    static ConnectionPool pool;//静态局部变量由编译器自动lock和unlock
    return &pool;
}


ConnectionPool::ConnectionPool(){
    //加载配置项
    if(!loadConfigFile())return;
    //创建初始数量的连接
    for(int i=0; i<_initSize; i++){
        addConnection();
    }
    //启动一个新的线程用于连接生产者
   thread producer(bind(&ConnectionPool::produceConnectionTask,this));
   producer.detach();
   //启动一个新的定时线程，扫描多余的空闲连接，超过maxIdleTime时间的空闲连接，进行回收
   thread recycler(bind(&ConnectionPool::recycleConnectionTask,this));
   recycler.detach();
}
//从json配置文件中加载配置项
bool ConnectionPool::loadConfigFile()
{
    std::ifstream f("../mysql.json");
    if(!f.is_open())//文件打开失败
    {
        LOG_ERROR<<"mysql.json file is not exist!";
        return false;
    }
    else//打开文件成功
    {   
        //读取json文件
        json data = json::parse(f);
        f.close();
        //初始化配置项
        _ip = data["ip"];
        _port = data["port"].get<int>();
        _username = data["username"];
        _password = data["password"];
        _dbname = data["dbname"];
        _initSize = data["initSize"].get<int>();
        _maxSize = data["maxSize"].get<int>();
        _maxIdleTime = data["maxIdleTime"].get<int>();
        _connectionTimeout = data["maxConnectionTimeQue"].get<int>();
        return true;
    }
    

}

// //从配置文件中加载配置项
// bool ConnectionPool::loadConfigFile(){
//     FILE* pf = fopen("mysql.cnf", "r");
//     if(pf == nullptr){//配置文件打开失败
//         cout<<"mysql.cnf file is not exist!"<<endl;
//         //LOG("mysql.cnf file is not exist!");
//         return false;
//     }
//     while(!feof(pf)){//返回非零，到达文件末尾
//         char line[1024]={0};
//         fgets(line,1024,pf);//读取一行字符串
//         //password=123456
//         string str = line;
//         int idx = str.find('=',0);//从0位置开始找'='
//         if(idx == -1){//无效配置项
//             continue;
//         }
//         int endidx = str.find('\n',idx);//从idx位置开始找找到'\n'
//         string key = str.substr(0,idx);
//         string value = str.substr(idx+1,endidx-idx-1);

//         if(key == "ip"){
//             _ip = value;
//         }else if(key == "port"){
//             _port = atoi(value.c_str());
//         }else if(key == "username"){
//             _username = value;
//         }else if(key == "password"){
//             _password = value;
//         }else if(key == "dbname"){
//             _dbname = value;
//         }else if(key == "initSize"){
//             _initSize = atoi(value.c_str());
//         }else if(key == "maxSize"){
//             _maxSize = atoi(value.c_str());
//         }else if(key == "maxIdleTime"){
//             _maxIdleTime =  atoi(value.c_str());
//         }else if(key == "maxConnectionTimeQue"){
//             _connectionTimeout = atoi(value.c_str());
//         }
//     }
//     return true;
// }



void ConnectionPool::addConnection(){
            Mysql* conn = new Mysql();
            //connect(string ip, unsigned short port, string user, string password,string dbname);
            conn->connect(_ip,_port,_username,_password,_dbname);
            conn->refreshAliveTime();//更新进入空闲的起始时间
            _connectionQue.push(conn);
            _connectionCnt.fetch_add(1);    
}
////运行在独立线程中，专门负责生产新连接
void ConnectionPool::produceConnectionTask()
{
    for(;;){
        unique_lock<mutex> lock(_queueMutex);
        while(!_connectionQue.empty()){//队列不空，此处生产线程进入等待状态
            cv.wait(lock);
        }

        if(_connectionCnt < _maxSize){//连接数量没有达到上限，继续创建新的连接
            addConnection();
        }
        //通知消费者线程，可以消费连接了
        cv.notify_all();
        
    }
}
//运行在独立线程中，专门负责回收超时连接
void ConnectionPool::recycleConnectionTask(){
    for(;;){
        //定时轮询回收
        this_thread::sleep_for(chrono::milliseconds(500));
        //依次取对头连接判断是否超时
         unique_lock<mutex> lock(_queueMutex);
         while(_connectionCnt > _initSize){
            Mysql * conn = _connectionQue.front();
            if(conn->getAlieTime() >= (_maxIdleTime)){
                _connectionQue.pop();
                _connectionCnt.fetch_sub(1);
                delete conn;//调用~Connection(释放连接)
            }else{
                break;//对头元素连接没有超时，其他连接肯定没有超时
            }
         }
    }

}
//外部获取空闲连接接口 返回智能指针
shared_ptr<Mysql> ConnectionPool::getConnection(){
    unique_lock<mutex> lock(_queueMutex);
    while(_connectionQue.empty()){//队列为空
        if(cv_status::no_timeout ==  cv.wait_for(lock,chrono::milliseconds(_connectionTimeout))){
            if(_connectionQue.empty()){//等待一定的时间队列依然为空
            LOG_ERROR<<"获取空闲连接超时了...获取连接失败!";
             return nullptr;
            }
        }
        
    }

    /*
    shared_ptr智能指针析构时，会把connection资源直接delete掉，相当于调用connection的析构函数,connection就被close掉了
    这里需要自定义shared_ptr的释放资源的方式（第二个参数使用匿名函数），把connection直接归还到queue当中
    */
    shared_ptr<Mysql> connptr(_connectionQue.front(),[&](Mysql* conn){
        lock_guard<mutex> locker(_queueMutex);
        conn->refreshAliveTime();//更新空闲进入时间点
        _connectionQue.push(conn);//放回连接池
    });

    _connectionQue.pop();
    cv.notify_all();//通知生产者如果队列为空赶紧生产连接
    
    return connptr;

}