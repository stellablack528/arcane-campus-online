class TcpServer
{
public:
    TcpServer(uint16_t port);
    ~TcpServer();

    bool Start();

private:
    uint16_t port_;
    Socket listenSocket_;
};