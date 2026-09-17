#pragma once

#include "Socket.hpp"
#include "ConnectionManager.hpp"

#include <string>

namespace Hogwarts
{
    class TcpServer
    {
    public:
        // bindAddress 默认 "0.0.0.0"：监听所有网卡，云服务器外部可连。
        // 想只允许本机连，传 "127.0.0.1"。
        explicit TcpServer(uint16_t port, const std::string& bindAddress = "0.0.0.0");
        ~TcpServer();

        bool Start();
        int acceptClient();
        std::size_t connectionCount() const; // 获取当前连接数
     
    private:
        uint16_t port_;
        std::string bindAddress_;
        Socket listenSocket_;

        // TcpServer 接收到客户端连接后，
        // 创建 TcpConnection，并交给 ConnectionManager 长期管理
        ConnectionManager connectionManager_;
    };
}