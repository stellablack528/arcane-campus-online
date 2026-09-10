#include "TcpConnection.hpp"

#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>

namespace Hogwarts
{

TcpConnection::TcpConnection(int fd)
    :
    socket_(fd)
{
}


TcpConnection::~TcpConnection()
{
}


void TcpConnection::setMessageCallback(
    std::function<void(std::string)> callback
)//传入的就是参数为string返回值为void的函数，这里是类似定义int x的x一样 不过这里的x更复杂所以要用一个函数盒子来预处理
{
    messageCallback_ = std::move(callback);//这里就是表明这个注入进来的函数里面的资源是可以移动的，这里就是注册
}


bool TcpConnection::send(const std::string& message)//发送消息
{
    std::size_t totalSent = 0;

    while (totalSent < message.size())
    {
        ssize_t result = ::send(
            socket_.fd(),
            message.data() + totalSent,
            message.size() - totalSent,
            0
        );

        if (result > 0)
        {
            totalSent += static_cast<std::size_t>(result);
            continue;
        }

        if (result == 0)
        {
            return false;
        }

        if (errno == EINTR)
        {
            continue;
        }

        return false;
    }

    return true;
}


void TcpConnection::receive()
{
    char buffer[4096];

    while (true)
    {
        ssize_t result = ::recv(
            socket_.fd(),
            buffer,
            sizeof(buffer),
            0
        );

        if (result > 0)
        {
            readBuffer_.append(
                buffer,
                static_cast<std::size_t>(result)
            );

            processMessages();
            continue;
        }

        if (result == 0)
        {
            // 对端正常关闭连接
            close();

            if (closeCallback_)
            {
                closeCallback_();
            }

            return;
        }

        if (errno == EINTR)
        {
            continue;
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // 当前已经没有更多数据可读
            return;
        }

        // 其他错误
        if (errorCallback_)
        {
            errorCallback_(std::strerror(errno));
        }

        return;
    }
}


void TcpConnection::processMessages()
{
    while (true)
    {
        std::size_t pos = readBuffer_.find('\n');

        if (pos == std::string::npos)
        {
            return;
        }

        std::string message =
            readBuffer_.substr(0, pos);

        readBuffer_.erase(
            0,
            pos + 1
        );

        if (messageCallback_)
        {
            messageCallback_(
                std::move(message)
            );
        }
    }
}


void TcpConnection::close()
{
    socket_.close();
}

}