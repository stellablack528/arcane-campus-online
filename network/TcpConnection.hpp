// 有了 socket 这样的网络资源后，需要一个具体的客户端连接
/* TcpServer
       |
       | 创建
       ↓
   TcpConnection
       |
       | 管理一个客户端
       ↓
    Socket(fd) */

#pragma once

#include "Socket.hpp"

#include <functional>
#include <memory>
#include <string>

namespace Hogwarts
{
    class TcpConnection
    {
    public:
        explicit TcpConnection(int fd);

        ~TcpConnection();

        // 发送数据
        bool send(const std::string& message);

        // 读取数据
        void receive();

        // 关闭连接
        void close();

        // 设置消息回调（用来修改 messageCallback_ 的）
        void setMessageCallback(std::function<void(std::string)> callback);

    private:
        Socket socket_;

        // 收到消息后的回调，当收到消息的时候我调用一个函数，把消息传出去
        std::function<void(std::string)> messageCallback_;
    };
}
