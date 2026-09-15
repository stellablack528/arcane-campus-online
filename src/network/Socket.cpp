//把我们在.hpp里声明的抽象接口，落到具体的 linux api，数据结构，错误处理和资源管理上
#include "Socket.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>

namespace Hogwarts
{
    // 头文件已经定义过类，不用重新再写
    // 下面是两个构造函数：无参和有参的
    Socket::Socket()
        : fd_(-1)
    {
    }

    Socket::Socket(int fd)
        : fd_(fd)
    {
    }

    Socket::~Socket()
    {
        // 防止默认构造（fd_ == -1）时调用 ::close(-1) 触发 EBADF。
        if (fd_ >= 0)
        {
            ::close(fd_);
            fd_ = -1;
        }
    }

    bool Socket::create()
    {
        fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd_ < 0)
        {
            std::cerr << "socket failed: " << std::strerror(errno) << std::endl;
            return false;
        }
        return true;
    }

    bool Socket::bind(const std::string& ip, uint16_t port)
    {
        // 一个装 ipv4 地址信息的盒子结构体
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);

        const int i = inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
        if (i == 0)
        {
            std::cerr << "invalid IPV4 address" << std::endl;
            return false;
        }
        if (i < 0)
        {
            std::cerr << "inet_pton failed: " << std::strerror(errno) << std::endl;
            return false;
        }

        if (::bind(fd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0)
        {
            std::cerr << "bind failed: " << std::strerror(errno) << std::endl;
            return false;
        }
        return true;
    }

    bool Socket::listen(int backlog)
    {
        if (::listen(fd_, backlog) < 0)
        {
            std::cerr << "listen failed: " << std::strerror(errno) << std::endl;
            return false;
        }
        return true;
    }

    int Socket::accept()
    {
        const int clientFd = ::accept(fd_, nullptr, nullptr);
        if (clientFd < 0)
        {
            std::cerr << "accept failed: " << std::strerror(errno) << std::endl;
        }
        return clientFd;
    }

    void Socket::close()
    {
        if (fd_ >= 0)
        {
            ::close(fd_);
            fd_ = -1;
        }
    }

    int Socket::fd() const
    {
        return fd_;
    }
}
