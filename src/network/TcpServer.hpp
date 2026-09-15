#pragma once

#include "Socket.hpp"
#include "ConnectionManager.hpp"

namespace Hogwarts
{
    class TcpServer
    {
    public:
        TcpServer(uint16_t port);
        ~TcpServer();

        bool Start();
        int acceptClient();
        std::size_t connectionCount() const; // 获取当前连接数

    private:
        uint16_t port_;
        Socket listenSocket_;

        // TcpServer 接收到客户端连接后，
        // 创建 TcpConnection，并交给 ConnectionManager 长期管理
        ConnectionManager connectionManager_;
    };
}