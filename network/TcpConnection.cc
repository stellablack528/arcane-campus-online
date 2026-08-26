#include "TcpConnection.hpp"

// 在写好的骨架里面调用 linux 接口

namespace Hogwarts
{
    TcpConnection::TcpConnection(int fd)
        : socket_(fd)
    {
    }

    TcpConnection::~TcpConnection()
    {
    }

    bool TcpConnection::send(const std::string& message)
    {
        // TODO: 调用 ::send(fd_, message.data(), message.size(), 0)
        (void)message;
        return false;
    }

    void TcpConnection::receive()
    {
        // TODO: 调用 ::recv(fd_, buffer, sizeof(buffer), 0)，收到数据后调用 messageCallback_
    }

    void TcpConnection::close()
    {
        // TODO: 调用 ::close(fd_)
    }

    void TcpConnection::setMessageCallback(std::function<void(std::string)> callback)
    {
        messageCallback_ = callback;
    }
}
