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
ZAudioRtsp::~ZAudioRtsp()
{
    for(qint32 i=0;i<5;i++)
    {
        this->m_Cap2NsFIFO[i]->resize(0);
        delete this->m_Cap2NsFIFO[i];
    }
}
//play with VLC Media Play.
//rtsp://192.168.137.100:8554:live   :network-caching=500ms
//if we set 0ms the VLC will kadun seriously.
//so we should increase the delay time to 100ms at least.
//here I set to 500ms to get more effects.
void ZAudioRtsp::run()
{
    //initial Queues.
    //capture to noise queue(fifo).
    for(qint32 i=0;i<5;i++)
    {
        this->m_Cap2NsFIFO[i]=new QByteArray;
        this->m_Cap2NsFIFO[i]->resize(1024);
        this->m_Cap2NsFIFOFree.enqueue(this->m_Cap2NsFIFO[i]);
    }
    this->m_Cap2NsFIFOUsed.clear();

    //Logger::setLogFile();
    Logger::setLogLevel(Logger::LogWarning);

    EventScheduler* scheduler = EventScheduler::createNew(EventScheduler::POLLER_SELECT); //创建调度器
    ThreadPool* threadPool = ThreadPool::createNew(2); //创建线程池
    UsageEnvironment* env = UsageEnvironment::createNew(scheduler, threadPool); //创建环境变量

    Ipv4Address ipAddr("0.0.0.0", 8554);
    RtspServer* server = RtspServer::createNew(env, ipAddr);
    MediaSession* session = MediaSession::createNew("live");
    //MediaSource* audioSource=AlsaMediaSource::createNew(env,"hw:2,0"); //默认设备"hw:0,0"
    MediaSource* audioSource=AlsaMediaSource::createNew(env,"plughw:CARD=USBSA,DEV=0"); //默认设备"hw:0,0"
    audioSource->ZBindFIFO(&this->m_Cap2NsFIFOFree,&this->m_Cap2NsFIFOUsed,&this->m_Cap2NsFIFOMutex,&this->m_condCap2NsFIFOEmpty,&this->m_condCap2NsFIFOFull);
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
