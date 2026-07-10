#include "UdpServer.hpp"
#include "ThreadPool.hpp"
#include <iostream>
#include <string>
#include <cstring> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstdlib>
#include <thread>
#include "InetAddr.hpp"
using namespace NS_LOG_MODULE;

// ============================================================================
// 全局运行上下文
// 保存客户端网络资源及目标服务器信息，供发送线程与接收线程共享访问
// ============================================================================
int sockfd = -1;
std::string server_ip;
uint16_t server_port = 0;
std::string nickname;//用户昵称


void Online(InetAddr &serveraddr){
    //默认消息,保证上线
    std::string online_message = nickname + " online!";
    ssize_t n = sendto(sockfd,
                       online_message.c_str(),
                       online_message.size(),
                       0,
                       (struct sockaddr *)serveraddr.GetNetAddress(),
                       serveraddr.len());
    (void)n;
}

// 输出程序启动参数说明
static void Usage(const std::string &proc)
{
    std::cout << "Usage:\n\t";
    std::cout << proc << " server_ip server_port" << std::endl;
}

// ============================================================================
// 消息接收业务模块
// 持续监听服务器返回的数据，实现客户端异步消息接收能力
// ============================================================================
void RecvMessage()
{
    char inbuffer[1024];
    while (true)
    {
        struct sockaddr_in temp;
        socklen_t len = sizeof(temp);

        // 阻塞等待服务器返回的业务数据或广播消息
        ssize_t m = recvfrom(sockfd, inbuffer, sizeof(inbuffer) - 1, 0,
                             (struct sockaddr *)&temp, &len);

        if (m > 0)
        {
            // 补充字符串结束标识，便于后续输出处理
            inbuffer[m] = 0;

            // 恢复输入提示符，保证终端交互体验
            std::cout << "\r" << inbuffer
                      << "\nPlease Enter# "
                      << std::flush;
        }
    }
}

// ============================================================================
// 消息发送业务模块
// 负责采集用户输入并发送至目标服务器
// ============================================================================
void SendMessage()
{
    // 初始化目标服务器地址信息，避免循环过程中重复构造对象
    InetAddr serveraddr(server_ip, server_port);
    std::cout << "Please Set Your Nick Name#";
    std::string message;
    std::getline(std::cin, message);
    nickname = message;               // 保存用户昵称到全局变量
    //那如果昵称一样怎么办呢
    
    Online(serveraddr);

    while (true)
    {
        // 获取用户输入内容
        std::string message;
        std::cout << "Please Enter# ";
        std::getline(std::cin, message);

        // 忽略空消息请求
        if (message.empty())
            continue;

        // 将用户输入的数据发送至服务端
        ssize_t n = sendto(sockfd,
                           message.c_str(),
                           message.size(),
                           0,
                           (struct sockaddr *)serveraddr.GetNetAddress(),
                           serveraddr.len());

        (void)n;
    }
}

// ============================================================================
// 客户端主控制流程
// 完成参数解析、网络资源初始化以及业务线程调度
// ============================================================================
int main(int argc, char *argv[])
{
    // 校验启动参数合法性
    if (argc != 3)
    {
        Usage(argv[0]);
        exit(1);
    }

    // 初始化目标服务器连接信息
    server_ip = argv[1];
    server_port = std::stoi(argv[2]);

    // 创建 UDP 通信套接字
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "Socket create error" << std::endl;
        exit(2);
    }

    // 创建消息收发线程，实现全双工通信模型
    std::thread recver(RecvMessage);
    std::thread sender(SendMessage);

    // 等待业务线程退出并完成资源回收
    recver.join();
    sender.join();

    return 0;
}
