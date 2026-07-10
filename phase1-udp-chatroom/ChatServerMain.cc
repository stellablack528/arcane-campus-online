#include "UdpServer.hpp"//获取事件
#include "ThreadPool.hpp"//执行处理动作
#include "Route.hpp"//任务
#include <memory>
#include <string>
#include <cstdlib>
using namespace NS_LOG_MODULE;

using task_t = std::function<void()>;

/**
 * =========================================================================
 * 业务功能：命令行使用规范提示函数
 * =========================================================================
 */
static void Usage(const std::string &process)
{
    std::cerr << "Usage:\n\t";
    std::cerr << process << "   local_port" << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        Usage(argv[0]);
        exit(USAGE_ERR);   // 参数不对，必须退出，否则往下 argv[1] 可能越界（USAGE_ERR 定义在 UdpServer.hpp）
    }

    uint16_t server_port = static_cast<uint16_t>(std::stoi(argv[1]));

    // ENABLE_CONSOLE_LOG_STRATEGY();

    // 1. 线程池模块（单例，全局只有一份）
    auto thread_pool = ThreadPool::Instance();
    thread_pool->Start();             // 启动单例线程池的工作线程

    // 2. 路由模块（业务逻辑本体）
    Route r;

    // 3. 网络模块（只管收发，完全不认识 ThreadPool / Route）
    UdpServer usvr(server_port);

    // 4. 注册两个回调：
    //    - 第一个回调（地址处理）：直接同步调用即可，逻辑很轻，不需要扔进线程池
    //    - 第二个回调（消息处理）：真正耗时的业务逻辑，打包成任务丢进线程池，
    //      不在 UdpServer 的主循环里同步执行，避免卡住收包
    usvr.RegisterService(
        [&r](const InetAddr &addr) {
            r.CheckUser(addr);
        },
        [&r, thread_pool](int sockfd, std::string msg) {
            // 捕获 r 和 thread_pool 这两个各自独立的对象；
            // 而是把"调用 r.Broadcast(...)"这个动作打包成一个任务，扔给线程池，
            // 由线程池里的某个 worker 线程去真正执行
            thread_pool->Push([&r, sockfd, msg]() {
                r.Broadcast(sockfd, msg);
            });
        }
    );

    // 5. 初始化 + 启动
    usvr.Init();
    usvr.Start();

    return 0;
}
