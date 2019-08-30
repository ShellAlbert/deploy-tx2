#include "zmainui.h"
#include <QDebug>
#include "alsa/zaudiortsp.h"
#include "alsa/zaudiotask.h"
#include "forward/ztcp2uartthread.h"
#include "json/zjsonthread.h"
#include "imgproc/zvideotask.h"
#include <QCoreApplication>
ZMainObj::ZMainObj(QObject *parent):QObject(parent)
{
}
ZMainObj::~ZMainObj()
{
    delete this->m_timerExit;
}
void ZMainObj::showMaximized()
{

}
qint32 ZMainObj::ZDoInit()
{
    return 0;
}
qint32 ZMainObj::ZManageThreads(ZAudioTask/*ZAudioRtsp*/ *audio,ZTcp2UartForwardThread *tcp2uart,ZJsonThread *json,ZVideoTask *video)
{
    this->m_audio=audio;
    this->m_tcp2uart=tcp2uart;
    this->m_json=json;
    this->m_video=video;

    this->m_timerExit=new QTimer;
    connect(this->m_timerExit,SIGNAL(timeout()),this,SLOT(ZSlotHelp2Exit()));
    this->m_timerExit->start(2000);
    return 0;
}

void ZMainObj::ZSlotHelp2Exit()
{
    bool bExitAllFlag=true;
    if(this->m_audio!=NULL)
    {
        if(!this->m_audio->ZIsCleanup())
        {
            bExitAllFlag=false;
        }
    }
    if(this->m_tcp2uart!=NULL)
    {
        if(!this->m_tcp2uart->ZIsCleanup())
        {
            bExitAllFlag=false;
        }
    }
    if(this->m_json!=NULL)
    {
        if(!this->m_json->ZIsCleanup())
        {
            bExitAllFlag=false;
        }
    }


    if(bExitAllFlag)
    {
        this->m_timerExit->stop();
        qApp->exit(0);
    }
}

