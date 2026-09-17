#include "TcpServer.hpp"
#include "ConnectionManager.hpp"
namespace Hogwarts
{

TcpServer::TcpServer(uint16_t port, const std::string& bindAddress)
    : port_(port)
    , bindAddress_(bindAddress)
{
}

TcpServer::~TcpServer()
{
    // 释放监听 socket，避免 fd 泄漏（虽然进程退出 OS 会回收，但显式关闭更规范）。
    listenSocket_.close();
}
//start完成对服务器的监听Socket做创建、绑定和监听
bool TcpServer::Start()
//Socket listenSocket_是TcpServer中的Socket的实例化
{ 
   if (!listenSocket_.create())
{
    return false;
}
 if(!listenSocket_.bind(bindAddress_, port_))
{
    return false;
}
if(!listenSocket_.listen())
{
    return false;
}
return true;}
//接受一个客户端的连接

int TcpServer::acceptClient()
{
   int fd =  listenSocket_.accept();
   if(fd<0)
   {
        return -1;
   }
    auto connection = std::make_shared<TcpConnection>(fd);
    connectionManager_.add(connection);

    return fd;
}

std::size_t TcpServer::connectionCount() const
{
    return connectionManager_.size();
}
}