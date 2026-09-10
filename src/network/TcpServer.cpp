#include "TcpServer.hpp"

namespace Hogwarts
{

TcpServer::TcpServer(uint16_t port)
    : port_(port)
{
}

TcpServer::~TcpServer()
{
}
//start完成对服务器的监听Socket做创建、绑定和监听
bool TcpServer::Start()
//Socket listenSocket_是TcpServer中的Socket的实例化
{ 
   if (!listenSocket_.create())
{
    return false;
}
 if(!listenSocket_.bind("127.0.0.1", port_))
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
    return listenSocket_.accept();
}


}