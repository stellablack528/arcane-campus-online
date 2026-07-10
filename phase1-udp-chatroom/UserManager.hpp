//对网络聊天室进行用户管理
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include "InetAddr.hpp"
#include "Logger.hpp"

using namespace NS_LOG_MODULE;

// class User
// {
//     std::string usernamel
//     std::string userstatus;
//     InetAddr address;
// }

//对用户要有增删查改的功能（基础）
class UserManager
{
    public:
        UserManager()
        {}
        void AddUser(const InetAddr &addr)
        {
           //先搜索该用户是否存在
           if(SearchUser(addr)){
             return;
           } else{
            _users.push_back(addr); 
           }
        }
        void DelUser(const InetAddr &addr)
        {
            for(auto iter = _users.begin(); iter != _users.end(); iter++){
                
                if(*iter == addr){
                    _users.erase(iter);
                    break;
                }
            }
        }
        bool SearchUser(const InetAddr &addr)
        {
            for(auto &user : _users){
                if(user == addr){
                    return true;
                }
            }
            return false;
        }
        void ModUser(const InetAddr &addr)
        {
            DelUser(addr);
            //test:这里简单粗暴一点 删了再重新做一个
            AddUser(addr);
        }
        std::vector<InetAddr> &Users(){
            return _users;//返回在线用户
        }
        ~UserManager()
        {

        }

    private:
        //"ip:port"作为key->InetAddr作为value
        std::vector<InetAddr> _users;//users就是InetAddr类型的vector
};
