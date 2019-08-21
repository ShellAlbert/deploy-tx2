#include "zmainui.h"
#include <QDebug>
#include "alsa/zaudiotask.h"
#include "forward/ztcp2uartthread.h"
#include "json/zjsonthread.h"
#include "imgproc/zvideotask.h"
#ifdef BUILD_ZSY_GUI_SUPPORT
#include <QApplication>
ZMainUI::ZMainUI(QWidget *parent):QWidget(parent)
{
    this->m_UILft=NULL;
    this->m_UIRht=NULL;
    this->m_hLayout=NULL;
    this->m_vLayout=NULL;
}

ZMainUI::~ZMainUI()
{
    if(this->m_llTop)
    {
        delete this->m_llTop;
    }
    if(this->m_UILft)
    {
        delete this->m_UILft;
    }
    if(this->m_UIRht)
    {
        delete this->m_UIRht;
    }
    if(this->m_hLayout)
    {
        delete this->m_hLayout;
    }
    if(this->m_vLayout)
    {
        delete this->m_vLayout;
    }
    delete this->m_timerExit;
}
ZImgDispUI* ZMainUI::ZGetDispUI(qint32 index)
{
    switch(index)
    {
    case 0:
        return this->m_UILft;
        break;
    case 1:
        return this->m_UIRht;
        break;
    default:
        break;
    }
    return NULL;
}
qint32 ZMainUI::ZDoInit()
{
    try{
        this->m_llTop=new QLabel;
        this->m_llTop->setText(tr("TWO RTSP ETHERNET CAMERA"));
        this->m_UILft=new ZImgDispUI("MAIN");
        this->m_UIRht=new ZImgDispUI("AUX");
        this->m_hLayout=new QHBoxLayout;
        this->m_vLayout=new QVBoxLayout;
    }catch(...)
    {
        qDebug()<<"<error>:new failed,low memory.";
        return -1;
    }

    if(this->m_UILft->ZDoInit()<0)
    {
        return -1;
    }
    if(this->m_UIRht->ZDoInit()<0)
    {
        return -1;
    }
    this->m_hLayout->addWidget(this->m_UILft);
    this->m_hLayout->addWidget(this->m_UIRht);
    this->m_vLayout->addLayout(this->m_hLayout);
    this->m_vLayout->addWidget(this->m_llTop);
    this->setLayout(this->m_vLayout);

    return 0;
}
qint32 ZMainUI::ZManageThreads(ZAudioTask *audio,ZTcp2UartForwardThread *tcp2uart,ZJsonThread *json,ZVideoTask *video)
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

void ZMainUI::ZSlotHelp2Exit()
{
    bool bExitAllFlag=true;
    if(!this->m_audio->ZIsCleanup())
    {
        bExitAllFlag=false;
    }
    if(!this->m_tcp2uart->ZIsCleanup())
    {
        bExitAllFlag=false;
    }
    if(!this->m_json->ZIsCleanup())
    {
        bExitAllFlag=false;
    }
    if(!this->m_video->ZIsCleanup())
    {
        bExitAllFlag=false;
    }

    if(bExitAllFlag)
    {
        this->m_timerExit->stop();
        qApp->exit(0);
    }
}
#else
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
qint32 ZMainObj::ZManageThreads(ZAudioTask *audio,ZTcp2UartForwardThread *tcp2uart,ZJsonThread *json,ZVideoTask *video)
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
    if(this->m_video!=NULL)
    {
        if(!this->m_video->ZIsCleanup())
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
#endif
