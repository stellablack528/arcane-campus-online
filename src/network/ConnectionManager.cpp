#include "ConnectionManager.hpp"

//这里是纯cpp，Linux的接口已经封装好了
namespace Hogwarts
{
    void ConnectionManager::add(const std::shared_ptr<TcpConnection>& connection)
    {
        if (!connection)
        {
            return;//没有这个连接，加入失败
        }
        //如果有，加入我们的哈希表去管理
        connections_[connection->fd()] = connection;
    }

    void ConnectionManager::remove(int fd)
    {
        connections_.erase(fd);
    }

    std::shared_ptr<TcpConnection> ConnectionManager::find(int fd) const
    {
        auto it = connections_.find(fd);// 根据 fd 查找对应的键值对，返回迭代器

        if (it == connections_.end())
        {
            return nullptr;
        }

        return it->second;//返回这个fd对应的TcpConnection智能指针
    }

    std::size_t ConnectionManager::size() const
    {
        return connections_.size();
    }

    void ConnectionManager::clear()
    {
        connections_.clear();
    }
}
