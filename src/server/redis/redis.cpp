#include "redis.h"
#include <iostream>
using namespace std;

/***************************
 订阅：redisCommand = redisAppendCommand(命令添加到本地缓存) + redisBufferWrite(缓存命令刷新到服务器) + redisGetReply(从服务器获取命令的执行结果并返回)
 发布：redisCommand
 原理：因为订阅的过程中会阻塞，所以接受订阅通道的消息（redisGetReply函数）需要开辟单独线程执行observer_channel_message
 
 控制台格式：
 【bash A】
$ redis-cli                                                     
127.0.0.1:6379> subscribe 2
Reading messages... (press Ctrl-C to quit)
1) "subscribe"
2) "2"
3) (integer) 1

1) "message"
2) "2"
3) "hello"


【bash B】
$ redis-cli
127.0.0.1:6379> publish 2 "hello"
(integer) 1


 ****************************/

Redis::Redis()
    : _publish_context(nullptr), _subcribe_context(nullptr)//初始化上下文
{
}

Redis::~Redis()
{
    if (_publish_context != nullptr)
    {
        redisFree(_publish_context);//释放上下文资源
    }

    if (_subcribe_context != nullptr)
    {
        redisFree(_subcribe_context);//释放上下文资源
    }
}
//连接redis服务器
bool Redis::connect()
{
    // 负责publish发布消息的上下文连接
    _publish_context = redisConnect("127.0.0.1", 6379);
    if (nullptr == _publish_context)
    {
        cerr << "connect redis failed!" << endl;
        return false;
    }

    // 负责subscribe订阅消息的上下文连接
    _subcribe_context = redisConnect("127.0.0.1", 6379);
    if (nullptr == _subcribe_context)
    {
        cerr << "connect redis failed!" << endl;
        return false;
    }

    // 在单独的线程中，监听通道上的事件，有消息给业务层进行上报
    thread t([&]() {
        observer_channel_message();
    });
    t.detach();

    cout << "connect redis-server success!" << endl;

    return true;
}

// 向redis指定的通道channel发布消息
bool Redis::publish(int channel, string message)
{
    /*
        redisCommand 是 Redis 提供的 C 语言 API 之一，用于向 Redis 服务器发送命令并获取响应。
            void *redisCommand(redisContext *c, const char *format, ...);
        其中：
            redisContext *c 是与 Redis 服务器建立的连接（redisContext 结构体表示一个 Redis 连接）。
            const char *format 是 Redis 命令的格式字符串，类似于 printf 函数中的格式化字符串，通过该字符串来指定要执行的 Redis 命令和参数。
            ... 是可变参数列表，用于传递 Redis 命令中的参数值。
            返回值为 void * 类型，表示 Redis 命令的执行结果。根据具体的命令和操作，可以使用相应的函数进行结果的解析和处理（如 redisReply 结构体）。

        需要注意的是，使用 redisCommand 函数时需要确保 Redis 服务器连接已经建立，并且在使用完毕后需要通过相应的函数（如 freeReplyObject）释放返回的结果对象，以避免内存泄漏。
            
    */
    redisReply *reply = (redisReply *)redisCommand(_publish_context, "PUBLISH %d %s", channel, message.c_str());
    if (nullptr == reply)
    {
        cerr << "publish command failed!" << endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}

// 向redis指定的通道subscribe订阅消息
bool Redis::subscribe(int channel)
{
    // SUBSCRIBE命令本身会造成线程阻塞等待通道里面发生消息，这里只做订阅通道，不接收通道消息
    // 通道消息的接收专门在observer_channel_message函数中的独立线程中进行
    // 只负责发送命令，不阻塞接收redis server响应消息，否则和notifyMsg线程抢占响应资源

     //redisAppendCommand 将 Redis 命令添加到本地缓存命令队列中，而不立即执行该命令。    subscribe 2
    if (REDIS_ERR == redisAppendCommand(this->_subcribe_context, "SUBSCRIBE %d", channel))
    {
        cerr << "subscribe command failed!" << endl;
        return false;
    }

    //redisBufferWrite 函数用于将累积在 Redis 缓冲区中的命令请求刷新到服务器上
    //redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1)
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(this->_subcribe_context, &done))
        {
            cerr << "subscribe command failed!" << endl;
            return false;
        }
    }
    
    //获取命令执行结果
    //redisGetreply;

    return true;
}

// 向redis指定的通道unsubscribe取消订阅消息
bool Redis::unsubscribe(int channel)
{
    if (REDIS_ERR == redisAppendCommand(this->_subcribe_context, "UNSUBSCRIBE %d", channel))
    {
        cerr << "unsubscribe command failed!" << endl;
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(this->_subcribe_context, &done))
        {
            cerr << "unsubscribe command failed!" << endl;
            return false;
        }
    }
    return true;
}

// 在独立线程中接收订阅通道中的消息
void Redis::observer_channel_message()
{
    /*
        1) "message"
        2) "2"
        3) "hello"

        redisGetReply 函数用于从 Redis 服务器获取命令的执行结果并返回。
        调用此函数后，Redis 服务器将阻塞，直到命令得到执行并返回结果。通过检查返回的状态值，可以判断命令是否成功执行以及如何处理返回结果。
    */
    redisReply *reply = nullptr;
    while (REDIS_OK == redisGetReply(this->_subcribe_context, (void **)&reply))
    {
        // 订阅收到的消息是一个带三元素的数组
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            // 给业务层上报通道上发生的消息
            _notify_message_handler(atoi(reply->element[1]->str) , reply->element[2]->str);
        }

        freeReplyObject(reply);
    }

    cerr << ">>>>>>>>>>>>> observer_channel_message quit <<<<<<<<<<<<<" << endl;
}

//初始化向业务层上报通道消息的回调对象
void Redis::init_notify_handler(function<void(int,string)> fn)
{
    this->_notify_message_handler = fn;
}