#include "zaudiortsp.h"

#include "rtsp/base/Logging.h"
#include "rtsp/net/UsageEnvironment.h"
#include "rtsp/base/ThreadPool.h"
#include "rtsp/net/EventScheduler.h"
#include "rtsp/net/Event.h"
#include "rtsp/net/RtspServer.h"
#include "rtsp/net/MediaSession.h"
#include "rtsp/net/InetAddress.h"
#include "rtsp/extend/alsa/AlsaMediaSource.h"
#include "rtsp/net/AACRtpSink.h"

#include <QDebug>
ZAudioRtsp::ZAudioRtsp()
{
    this->m_bCleanup=false;
}
void ZAudioRtsp::run()
{
    //Logger::setLogFile();
    Logger::setLogLevel(Logger::LogWarning);

    EventScheduler* scheduler = EventScheduler::createNew(EventScheduler::POLLER_SELECT); //创建调度器
    ThreadPool* threadPool = ThreadPool::createNew(2); //创建线程池
    UsageEnvironment* env = UsageEnvironment::createNew(scheduler, threadPool); //创建环境变量

    Ipv4Address ipAddr("0.0.0.0", 8554);
    RtspServer* server = RtspServer::createNew(env, ipAddr);
    MediaSession* session = MediaSession::createNew("live");
    MediaSource* audioSource = AlsaMediaSource::createNew(env,"hw:2,0"); //默认设备"hw:0,0"
    RtpSink* audioRtpSink = AACRtpSink::createNew(env, audioSource);

    session->addRtpSink(MediaSession::TrackId0, audioRtpSink);
    //session->startMulticast();

    server->addMeidaSession(session);
    server->start();

    //std::cout<<"Play the media using the URL \""<<server->getUrl(session)<<"\""<<std::endl;
    qDebug()<<QString(server->getUrl(session).c_str());
    env->scheduler()->loop();

    this->m_bCleanup=true;
    return;
}
bool ZAudioRtsp::ZIsCleanup()
{
    return this->m_bCleanup;
}
