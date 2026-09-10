class TcpServer
{
public:
    TcpServer(uint16_t port);
    ~TcpServer();

    bool Start();
    int acceptClient();

private:
    uint16_t port_;
    Socket listenSocket_;
};