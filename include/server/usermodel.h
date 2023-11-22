#ifndef USERMODEL_H
#define USERMODEL_H

#include"user.h"

//user表对的数据操作类
class UserModel
{
public:
    //user表增加方法
    bool insert(User &user);

    //根据用户号码查询用户信息
    User query(int id);

    //更新用户的状态信息
    bool updateState(User &user);
    
};


#endif