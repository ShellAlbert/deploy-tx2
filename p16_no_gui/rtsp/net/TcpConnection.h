#ifndef _TCP_CONNECTION_H_
#define _TCP_CONNECTION_H_
#include "rtsp/net/UsageEnvironment.h"
#include "rtsp/net/Event.h"
#include "rtsp/net/TcpSocket.h"
#include "rtsp/net/Buffer.h"

class TcpConnection
{
public:
    typedef void (*DisconnectionCallback)(void*, int);

    virtual ~TcpConnection();

    void setDisconnectionCallback(DisconnectionCallback cb, void* arg);

protected:
    TcpConnection(UsageEnvironment* env, int sockfd);
    void enableReadHandling();
    void enableWriteHandling();
    void enableErrorHandling();
    void disableReadeHandling();
    void disableWriteHandling();
    void disableErrorHandling();

    void handleRead();
    virtual void handleReadBytes();
    virtual void handleWrite();
    virtual void handleError();

    void handleDisconnection();

private:
    static void readCallback(void* arg);
    static void writeCallback(void* arg);
    static void errorCallback(void* arg);

protected:
    UsageEnvironment* mEnv;
    TcpSocket mSocket;
    IOEvent* mTcpConnIOEvent;
    DisconnectionCallback mDisconnectionCallback;
    void* mArg;
    Buffer mInputBuffer;
    Buffer mOutBuffer;
    char mBuffer[2048];
};

#endif //_TCP_CONNECTION_H_
