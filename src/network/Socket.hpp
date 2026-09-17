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

        // 创建 socket
        bool create();
        // 绑定地址
        bool bind(const std::string& ip, std::uint16_t port);
        // 监听
        bool listen(int backlog = 128);
        // 接受连接，返回新的客户端 fd
        int accept();
        // 关闭 socket
        void close();
        // 获取文件描述符
        int fd() const;
        //把当前Socket对应的文件描述符设置为非阻塞模式，否则阻塞socket调用recv()时如果暂时没有数据，她会一直卡在那里
        bool setNonBlocking();

    private:
        int fd_;
    };
}
