#include "zvideotask.h"
#include "../ui/zmainui.h"
ZVideoTask::ZVideoTask(QObject *parent):QObject(parent)
{
    this->m_rtsp1=NULL;
    this->m_rtsp2=NULL;
}

qint32 ZVideoTask::ZStartTask(QWidget *mainUI)
{
    ZMainUI *mainUI2=qobject_cast<ZMainUI*>(mainUI);
    this->m_rtsp1=new ZRtspThread("192.168.137.12");
    this->m_rtsp2=new ZRtspThread("192.168.137.20");

    QObject::connect(this->m_rtsp1,SIGNAL(ZSigNewImg(QImage)),mainUI2->ZGetDispUI(0),SLOT(ZSlotFlushImg(QImage)));
    QObject::connect(this->m_rtsp2,SIGNAL(ZSigNewImg(QImage)),mainUI2->ZGetDispUI(1),SLOT(ZSlotFlushImg(QImage)));

    this->m_rtsp1->start();
    this->m_rtsp2->start();
    return 0;
}
bool ZVideoTask::ZIsCleanup()
{
    bool bIsCleanup=true;
    if(!this->m_rtsp1->ZIsCleanup())
    {
        bIsCleanup=false;
    }
    if(!this->m_rtsp2->ZIsCleanup())
    {
        bIsCleanup=false;
    }
    return bIsCleanup;
}
