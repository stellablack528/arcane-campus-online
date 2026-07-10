#pragma once
#include <iostream>
#include <memory>   // 智能指针要包的头文件
#include <string>
#include "UserManager.hpp"  // 注意：这里类名要跟下面用到的类型统一（UserManager / UsageManager 二选一）
#include "Mutex.hpp"        // 安全保障：加锁保护多线程共享数据

// =========================================================================
// Route 类的定位：
// 它是"网络收发"和"业务数据（在线用户列表）"之间的调度层。
// 服务器收到一条消息后，不会直接自己维护用户列表、自己拼发送逻辑，
// 而是统一交给 Route 来处理"这个用户上线了/下线了/该把消息发给谁"这类业务判断，
// 这样网络层（UdpServer）只管收发字节流，完全不需要关心"用户管理"这些业务细节，
// 两者职责分离，以后业务逻辑变复杂了，只需要改 Route，不用动 UdpServer。
// =========================================================================
class Route
{
public:
    // 构造函数：创建一个 UserManager 对象，用智能指针托管它的生命周期，
    // Route 销毁时，_uma 也会自动被释放，不需要手动 delete
    Route() : _uma(std::make_unique<UserManager>())
    {}

    // 业务功能：有新消息进来时，先确认/记录这个用户"在线"
    // 场景举例：客户端第一次发消息过来，说明它上线了，得把它加进在线用户表，
    //           后续广播消息的时候才能发给它
    void CheckUser(const InetAddr &addr)
    {
        // 现在这样写只是测试，之后肯定要优化的
        // 加锁原因：_uma 内部维护的用户列表是"临界资源"——
        // 多线程环境下（线程池里多个 worker 线程可能同时处理不同客户端的消息），
        // 如果不加锁，多个线程同时往列表里增删用户，会导致数据结构损坏
        LockGuard lockguard(_lock);
        _uma->AddUser(addr);
    }

    // 业务功能：某个用户下线了（比如检测到连接断开、或者客户端主动退出），
    // 把它从在线用户表里移除，避免继续给一个已经不在线的用户发消息
    void OfflineUser(const InetAddr &addr)
    {
        LockGuard lockguard(_lock);
        _uma->DelUser(addr);
    }

    // 1. 转发消息
    // 业务功能：聊天室的核心动作——把一条消息广播给当前所有在线用户
    // 参数说明：sockfd 是服务器监听的那个 socket，message 是要发送的内容
    void Broadcast(int sockfd, std::string message)
    {
        // 通过文件描述符广播给所有人
        // 加锁原因：读取用户列表的同时，可能有另一个线程正在 AddUser/DelUser 修改它，
        // 必须保证"读列表"和"改列表"不会同时发生，否则可能遍历到一半列表被改，程序崩溃
        LockGuard lockguard(_lock);

        // 获取每一个人——vector
        // 调用 UserManager 对象的 Users() 拿到所有在线用户的列表，然后遍历发送
        auto &users = _uma->Users();

        // 把消息遍历发送给所有人
        for (auto &user : users)
        {
            sendto(sockfd, message.c_str(), message.size(), 0,
                   (struct sockaddr *)user.GetNetAddress(), user.len());
            // TODO: 确认 InetAddr 里拿地址的方法名和返回类型，
            // 以及是否需要补一个返回 socklen_t 的 Len() 方法
        }
    }

    ~Route()
    {}

private:
    // 在线用户列表是一份"临界资源"（会被多个线程同时访问），所以需要 Mutex，
    // 这里用智能指针管理 UserManager 对象的生命周期
    std::unique_ptr<UserManager> _uma;
    Mutex _lock;   // 专门用来保护 _uma 内部数据的锁，CheckUser/OfflineUser/Broadcast 三个操作都要靠它互斥
};
