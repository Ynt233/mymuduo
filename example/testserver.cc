/*
 * @Author: Ynt
 * @Date: 2024-11-16 11:30:49
 * @LastEditTime: 2024-11-16 16:35:09
 * @Description: 
 */
#include <mymuduo/TcpServer.h>
#include <mymuduo/Logger.h>
#include <string>

class EchoServer
{
public:
    EchoServer(EventLoop *loop, const InetAddress &addr, const std::string &name) : server_(loop, addr, name), loop_(loop) 
    {
        server_.setConnectionCallback(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        server_.setThreadNum(4);
    }

    void start() { server_.start(); }

private:
    void onConnection(const TcpConnectionPtr &conn) 
    {
        if (conn->connected()) {
            LOG_DEBUG("connect up: %s\n", conn->peerAddress().toIpPort().c_str());
        } else {
            LOG_DEBUG("connect down: %s\n", conn->peerAddress().toIpPort().c_str());
        }
    }

    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
    {
        std::string msg = buf->retrieveAllAsString();
        conn->send(msg);
        conn->shutdown();
    }
    EventLoop *loop_;
    TcpServer server_;
};

int main()
{
    EventLoop loop;
    InetAddress addr(8899);
    EchoServer server(&loop, addr, "TestServer");

    server.start();
    loop.loop();

    return 0;
}
