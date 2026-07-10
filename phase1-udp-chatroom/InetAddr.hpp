#pragma once

#include <iostream>
#include <string>
#include <strings.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// 对客户端地址进行封装（因为客户端很多，要管理，才能完成并发处理）
//对客户端先描述再组织

// 把 sockaddr_in 这种"裸的、不可读"的 C 结构体，包装成人类友好、随取随用的对象
class InetAddr
{
public:
    // 场景一：已经有一个 sockaddr_in 了（比如 recvfrom 拿到的 peer），包装成 InetAddr
    InetAddr(const struct sockaddr_in &address) : _address(address),_len(sizeof(address))
    {
        _ip = inet_ntoa(_address.sin_addr);   // 转成人类可读的 IP 字符串，只做一次
        _port = ntohs(_address.sin_port);     // 转成主机字节序端口号，只做一次
    }

    // 场景二：只有 IP 字符串和端口号，想构造出对应的 sockaddr_in
    InetAddr(const std::string &ip, uint16_t port) : _ip(ip), _port(port)
    {
        bzero(&_address, sizeof(_address));
        _address.sin_family = AF_INET;
        _address.sin_addr.s_addr = inet_addr(_ip.c_str()); // 把字符串 IP 转成网络字节序二进制
        _address.sin_port = htons(_port);
        _len = sizeof(_address);
    }

    // 场景三：只传端口号，IP 默认绑定所有网卡（INADDR_ANY），用于服务端 bind
    InetAddr(uint16_t port) : _port(port)
    {
        bzero(&_address, sizeof(_address));
        _address.sin_family = AF_INET;
        _address.sin_addr.s_addr = INADDR_ANY;
        _address.sin_port = htons(_port);
        _ip = "0.0.0.0";
        _len = sizeof(_address);
    }
    //重载==，有应用场景就写了，因为比如有新用户注册它要先用==比较一下这个用户是否存在了才能继续进行注册逻辑
    bool operator == (const InetAddr &addr){
        
        return (this -> _ip == addr._ip) && (this->_port == addr._port);
    }
    std::string ToString()
    {
        return "[" + _ip +":" + std::to_string(_port) + "";
    }
     InetAddr()
     {
        
     }

     struct sockaddr_in *GetNetAddress()
     {
        return &_address;
     }
     socklen_t len(){
        return _len;
     }
    ~InetAddr()
    {}

    // 提供几个 getter，外部才能拿到内部存的这些信息
    std::string Ip() const { return _ip; }
    uint16_t Port() const { return _port; }
    struct sockaddr_in Addr() const { return _address; }

private:
    struct sockaddr_in _address; // 套接字属性（网络字节序的原始数据）
    socklen_t _len;
    std::string _ip;             // 人类可读的 IP 字符串
    uint16_t _port;              // 主机字节序的端口号
};
