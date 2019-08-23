#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_
#include <map>

#include "rtsp/net/Acceptor.h"
#include "rtsp/net/UsageEnvironment.h"
#include "rtsp/net/InetAddress.h"
#include "rtsp/net/TcpConnection.h"

//class TcpConnection;

class TcpServer
{
public:
    virtual ~TcpServer();

    void start();

protected:
    TcpServer(UsageEnvironment* env, Ipv4Address& addr);
    virtual void handleNewConnection(int connfd) = 0;
    //virtual void handleDisconnection(int sockfd);

private:
    static void newConnectionCallback(void* arg, int connfd);
    //static void disconnectionCallback(void* arg, int sockfd);

protected:
    UsageEnvironment* mEnv;
    Acceptor* mAcceptor;
    Ipv4Address mAddr;
//    std::map<int, TcpConnection*> mTcpConnections;
};

#endif //_TCP_SERVER_H_
