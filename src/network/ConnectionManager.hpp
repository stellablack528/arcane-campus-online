//管理在线连接
#pragma once
#include "TcpConnection.hpp"

#include <memory>
#include <unordered_map>

namespace Hogwarts
{
    class ConnectionManager
    {
    public:
        //添加一个客户端连接
        //使用 shared_ptr 持有并共享 TcpConnection 对象
        void add(const std::shared_ptr<TcpConnection>& connection);

        //根据fd删除客户端连接
        void remove(int fd);

        //根据fd查找客户端连接
        std::shared_ptr<TcpConnection> find(int fd) const;

        //获取当前连接数量
        std::size_t size() const;

        //清空所有连接
        void clear();

    private:
        std::unordered_map<int, std::shared_ptr<TcpConnection>> connections_;
    };
}
