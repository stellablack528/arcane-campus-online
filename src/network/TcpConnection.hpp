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

        // 设置消息回调（收到完整消息后调用）
        void setMessageCallback(std::function<void(std::string)> callback);

        // 设置连接关闭回调
        void setCloseCallback(std::function<void()> callback);

        // 设置错误回调
        void setErrorCallback(std::function<void(const std::string&)> callback);

        int fd() const;//转发/访问接口
    private:
        // 从读缓冲区中按 \n 拆分完整消息并回调
        void processMessages();

        Socket socket_;
        std::string readBuffer_;

        // 收到消息后的回调，当收到消息的时候调用一个函数把消息传出去
        std::function<void(std::string)> messageCallback_;
        // 连接关闭回调
        std::function<void()> closeCallback_;
        // 错误回调
        std::function<void(const std::string&)> errorCallback_;
    };
}
