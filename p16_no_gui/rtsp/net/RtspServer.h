#ifndef _RTSPSERVER_H_
#define _RTSPSERVER_H_
#include <map>
#include <vector>
#include <string>

#include "rtsp/net/TcpServer.h"
#include "rtsp/net/UsageEnvironment.h"
#include "rtsp/net/RtspConnection.h"
#include "rtsp/net/MediaSession.h"
#include "rtsp/net/Event.h"
#include "rtsp/base/Mutex.h"

class RtspConnection;

class RtspServer : public TcpServer
{
public:
    static RtspServer* createNew(UsageEnvironment* env, Ipv4Address& addr);
    virtual ~RtspServer();

    UsageEnvironment* envir() const { return mEnv; }
    bool addMeidaSession(MediaSession* mediaSession);
    MediaSession* loopupMediaSession(std::string name);
    std::string getUrl(MediaSession* session);

protected:
    RtspServer(UsageEnvironment* env, Ipv4Address& addr);
    virtual void handleNewConnection(int connfd);
    static void disconnectionCallback(void* arg, int sockfd);
    void handleDisconnection(int sockfd);
    static void triggerCallback(void*);
    void handleDisconnectionList();

private:
    std::map<std::string, MediaSession*> mMediaSessions;
    std::map<int, RtspConnection*> mConnections;
    std::vector<int> mDisconnectionlist;
    TriggerEvent* mTriggerEvent;
    Mutex* mMutex;
};

#endif //_RTSPSERVER_H_
