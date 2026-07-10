#ifndef __ECHOSERVER_HPP
#define __ECHOSERVER_HPP

#include <iostream> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <cerrno>
#include <functional>
#include <cstring>
#include <cstdlib>
#include <unistd.h>       // close()
#include <string>
#include "Logger.hpp"     // 能够打印日志
#include "ThreadPool.hpp" // 线程池，真正处理消息的地方
#include "InetAddr.hpp"

using namespace NS_LOG_MODULE;

// =========================================================================
// 定义两种回调类型：
// UdpServer 自己只负责收发字节流，"收到新用户该怎么办""收到消息该怎么处理"
// 这类业务判断，不属于网络层该管的事，所以统一"扔"给上层（比如 Route）来处理。
// UdpServer 从头到尾都不需要认识 Route 这个类，靠的就是下面这两个 std::function 类型。
// =========================================================================
using handler_addr_t = std::function<void(const InetAddr &)>;      // 处理"新用户/地址"相关逻辑
using handler_msg_t  = std::function<void(int sockfd, std::string msg)>; // 处理"消息内容"相关逻辑

// 因为套接字之后是要用到的，增强健壮性，这里先加缺省值
const static int default_fd = -1; 
const static int default_port = 8888; // 缺省值

enum
{
    SUCCESS = 0, 
    USAGE_ERR,
    SOCKET_ERR,
    BIND_ERR   // bind失败时用这个错误码退出
};

class UdpServer
{
public:
    // 构造函数：只负责把端口存起来，顺便决定线程池要开几个线程
    // 注意：初始化列表的顺序要跟下面成员变量"声明"的顺序保持一致，
    // 否则编译器会给出"初始化顺序与声明顺序不符"的警告（不是报错，但最好避免）
    UdpServer(uint16_t port, int thread_num = default_thread_num) : 
        _sockfd(default_fd),
        _port(port), 
        _pool(thread_num)   // 线程池的线程数量，可以在创建 UdpServer 时指定，不指定就用缺省值
    {}
    
    ~UdpServer()
    {
        if (_sockfd != default_fd) close(_sockfd);
    }

    // 关键接口：把外部（比如 main.cc）准备好的两个回调"注册"进来，
    // 存进成员变量 _handler_addr / _handler_msg，Start() 收到消息后会调用它们
    void RegisterService(handler_addr_t handler_addr, handler_msg_t handler_msg)
    {
        _handler_addr = handler_addr; // 把参数赋值给成员变量，之前这里漏写了，
        _handler_msg  = handler_msg;  // 只声明了两个同名局部变量，根本没有真正"注册"进去
    }
    
    // 初始化：让它具备收发网络数据的能力
    void Init()
    {
        // ============ 第一步：创建套接字 ============
        _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (_sockfd < 0) {
            LOG(FATAL) << "create socket error, reason: " << strerror(errno);
            exit(SOCKET_ERR);
        }
        LOG(INFO) << "create socket success, sockfd: " << _sockfd;

        // ============ 第二步：填充网络地址信息，服务端绑定任意IP ============
        // 这里假设 InetAddr 提供了一个"只传端口"的构造函数，内部会把 IP 设成 INADDR_ANY
        // （如果 InetAddr.hpp 里还没有这个构造函数，需要补一个，之后可以单独帮你加）
        InetAddr local(_port);

        // 从 InetAddr 对象里，取出真正的 sockaddr_in 结构体，才能传给 bind()
        struct sockaddr_in local_addr = local.Addr();

        // ============ 第三步：bind绑定，真正把数据传到内核，所以要强转为通用结构体 ============
        // bind() 只接受 3 个参数：fd、地址、地址长度，原来的代码多传了一个参数
        // 长度直接用 sizeof(local_addr) 就够了，不需要额外定义一个 len() 方法
        int n = bind(_sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr));
        if (n < 0) {
            LOG(FATAL) << "bind socket error, reason: " << strerror(errno);
            exit(BIND_ERR);
        }

        LOG(INFO) << "bind socket success, port: " << _port;

        // ============ 第四步：把线程池也启动起来 ============
        _pool.Start();
    }

    // 启动服务：主线程只负责"收消息 + 触发回调"，重活丢给线程池，自己绝不阻塞
    void Start()
    {
        char inbuffer[1024];

        while (true)
        {   
            struct sockaddr_in peer;      // 用来存"谁发来的数据"
            socklen_t len = sizeof(peer); // 必须提前初始化！

            // recvfrom 收到对应的内容放到 inbuffer 里面
            // 接收用户发来的数据以及用户的 socket 信息
            ssize_t n = recvfrom(_sockfd, inbuffer, sizeof(inbuffer) - 1, 0,
                                  (struct sockaddr *)&peer, &len);
            if (n > 0)
            {   
                inbuffer[n] = '\0'; // 正确：在这里安全地把收到的数据当成标准字符串处理
                InetAddr clientaddress(peer);   // 先构造 clientaddress，后面才能用
                std::string msg(inbuffer);      // 转成 std::string，方便后面按值捕获进 Lambda

                LOG(DEBUG) << clientaddress.ToString() << "#" << inbuffer;

                // 1. 地址相关的处理（比如记录用户上线），逻辑很轻，直接同步调用即可，
                //    不需要额外丢进线程池，省一次任务调度的开销
                _handler_addr(clientaddress);

                // 2. 消息内容的处理，是真正可能耗时的业务逻辑（比如广播给所有人），
                //    这里丢进线程池异步执行，主线程立刻回到循环开头继续 recvfrom，
                //    不会被这一条消息的处理时间卡住
                int sockfd = _sockfd; // 拷贝一份，避免 Lambda 里直接依赖 this 指向的可变状态
                _pool.Push([this, sockfd, msg]() {
                    _handler_msg(sockfd, msg);
                });
            }
            else if (n == 0)
            {
                LOG(INFO) << "peer closed"; 
            }
            else
            {
                LOG(WARNING) << "recvfrom error, reason: " << strerror(errno);
            }
        }
    }
    
private:
    int _sockfd; 
    uint16_t _port;    
    ThreadPool _pool;             // 真正处理消息的线程池，UdpServer 自己持有、自己启动
    handler_addr_t _handler_addr; // 外部注入的"地址处理"回调
    handler_msg_t  _handler_msg;  // 外部注入的"消息处理"回调
   
}; 

#endif
