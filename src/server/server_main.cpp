#include "../network/TcpServer.hpp"

#include <iostream>

int main()
{
    // 调试服务器：每次 cout 输出立即 flush，避免后台/管道场景下输出卡在缓冲区。
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