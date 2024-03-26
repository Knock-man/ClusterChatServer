# ClusterChatServer  
## 基于muduo网络库实现的chatServer服务器  
开发环境：vscode远程linux开发，cmake构建，linux shell输出项目编译脚本
* 1、使用muduo网络库作为项目的网络核心模块，提供高并发网络服务，解耦网络和业务模块代码  
* 2、使用json序列化和反序列化消息作为私有通信协议  
* 3、配置nginx基于tcp的负载均衡，实现聊天服务器的集群功能，提高后端服务的并发能力  
* 4、基于redis的发布-订阅功能，实现跨服务器的消息通信  
* 5、使用mysql关系型数据库作为项目数据的落地存储
* 6、使用连接池有效管理和利用连接资源、提升程序性能

***  
### 框架
![image](https://github.com/Knock-man/ClusterChatServer/assets/66514322/8a5b9a23-333b-4469-801c-3cb96b337e34)




### 快速运行
* 搭建环境
  * linux
  * MySQL
  * Cmake
  * 配置redis
  * 配置nginx  
* 数据库表准备
```sql
#创建数据库
CREATE DATABASE ClusterChat

#创建用户表
CREATE TABLE `User` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(50) NOT NULL,
  `password` varchar(50) NOT NULL,
  `state` enum('online','offline') DEFAULT 'online',
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=15 DEFAULT CHARSET=utf8mb3

#创建群组表
CREATE TABLE `AllGroup` (
  `id` int NOT NULL AUTO_INCREMENT,
  `groupname` varchar(50) NOT NULL,
  `groupdesc` varchar(200) DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `groupname` (`groupname`)
) ENGINE=InnoDB AUTO_INCREMENT=8 DEFAULT CHARSET=utf8mb3

#创建好友表
-- Active: 1697722813029@@127.0.0.1@3306@ClusterChat
CREATE TABLE `Friend` (
  `userid` int NOT NULL,
  `friendid` int NOT NULL,
  PRIMARY KEY (`userid`,`friendid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3


#创建离线消息表
CREATE TABLE `offlineMessage` (
  `userid` int NOT NULL,
  `message` varchar(500) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3
```  
* 打开 mysql.json 文件配置数据库连接池基本信息
```sql
{
    "ip": "127.0.0.1", //数据库服务器ip
    "port": 3306, //端口
    "username": "xbj", //用户名
    "password": "xbj", //密码
    "dbname": "ClusterChat", //数据库名
    "initSize": 10, //初始连接池大小
    "maxSize": 1024, //连接池最大容量
    "maxIdleTime": 60, //连接最大空闲时间（秒）
    "maxConnectionTimeQue": 100 //连接超时时间(毫秒)
  }
```
* nginx负载均衡端口配置 路径：/usr/local/nginx/nginx.conf  【单台服务器测试无需负载均衡可省略此步】
  ![image](https://github.com/Knock-man/ClusterChatServer/assets/66514322/9a0c861e-b8b4-420f-8c76-4faba2a6dde7)  
  配置两台服务器端口分别为 127.0.0.1:6000 和 127.0.0.1:6002 采用轮询方式负载均衡  nginx连接端口为8000 3次心跳 30秒进行一次心跳
* 编译
  ```bash
  ./autobuild.sh
  ```
* 服务器运行
   ```bash
   #开启第一台服务器
  ./chatServer 172.0.0.1 6000
   #开启第二台服务器
   ./chatServer 172.0.0.1 6002
   ```
* 客户端运行
  ```bash
  #开启第一台客户端
  ./chatClient 127.0.0.1 8000 #nginx连接宽口800  单台服务器直接连接服务器端口
  #开启第二台客户端
  ./chatClient 127.0.0.1 8000
  ```
