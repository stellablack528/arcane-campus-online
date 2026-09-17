#include "../network/TcpServer.hpp"

#include <chrono>
#include <iostream>
#include <thread>

int main()
{
    // 调试服务器：每次 cout 输出立即 flush
    std::cout.setf(std::ios::unitbuf);

    Hogwarts::TcpServer server(8888);

    if (!server.Start())
    {
        std::cerr << "Server start failed." << std::endl;
        return 1;
    }

    std::cout << "Server started on port 8888." << std::endl;

    while (true)
    {
        int clientFd = server.acceptClient();

        if (clientFd < 0)
        {
            // 当前没有新的客户端连接，避免非阻塞 accept 忙等导致 CPU 占用过高
            std::this_thread::sleep_for(
                std::chrono::milliseconds(10)
            );
            continue;
        }

        std::cout << "Client connected, fd = "
                  << clientFd
                  << ", current connections = "
                  << server.connectionCount()
                  << std::endl;
    }

    return 0;
}