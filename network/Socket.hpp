// 封装 Linux socket，如果不封装的话项目大了会到处都是系统调用
#pragma once

#include <cstdint>
#include <string>

namespace Hogwarts
{
    class Socket
    {
    public:
        Socket();
        explicit Socket(int fd);

        ~Socket();

        // 本来是返回 int fd 的如果失败了就是 -1，但是调用者不需要关心这个
        // 创建 socket
        bool create();
        // 绑定地址
        bool bind(const std::string& ip, std::uint16_t port);
        // 监听
        bool listen(int backlog = 128);
        // 接受连接，返回新的客户端 fd
        int accept();
        // 获取文件描述符，底层系统调用需要 fd
        int fd() const;

    private:
        int fd_;
    };
}
