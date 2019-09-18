#include "zvideotask.h"
#include "../ui/zmainui.h"
ZVideoTask::ZVideoTask(QObject *parent):QObject(parent)
{
    this->m_rtsp1=NULL;
    this->m_rtsp2=NULL;
}
ZVideoTask::~ZVideoTask()
{
    for(qint32 i=0;i<FIFO_DEPTH;i++)
    {
        delete this->m_rtsp2imgProc1[i];
        delete this->m_rtsp2imgProc2[i];
    }
}
qint32 ZVideoTask::ZStartTask(QWidget *mainUI)
{
    //rtsp1->queue->imgProc.
    for(qint32 i=0;i<FIFO_DEPTH;i++)
    {
        this->m_rtsp2imgProc1[i]=new cv::Mat(ImgResizedHeight,ImgResizedWidth,CV_8UC1);
        this->m_rtsp2imgProcFree1.enqueue(this->m_rtsp2imgProc1[i]);
    }
    this->m_rtsp2imgProcUsed1.clear();

    //rtsp2->queue->imgProc.
    for(qint32 i=0;i<FIFO_DEPTH;i++)
    {
        this->m_rtsp2imgProc2[i]=new cv::Mat(ImgResizedHeight,ImgResizedWidth,CV_8UC1);
        this->m_rtsp2imgProcFree2.enqueue(this->m_rtsp2imgProc2[i]);
    }
    this->m_rtsp2imgProcUsed2.clear();

    //create work threads.
    ZMainUI *mainUI2=qobject_cast<ZMainUI*>(mainUI);

    //left video.
    this->m_rtsp1=new ZRtspThread("192.168.137.12");
    this->m_rtsp1->ZBindQueue(&this->m_rtsp2imgProcMux1,///<
                                 &this->m_condRtsp2imgProcNotEmpty1,&this->m_condRtsp2imgProcNotFull1,///<
                                 &this->m_rtsp2imgProcFree1,&this->m_rtsp2imgProcUsed1);
    //right video.
    this->m_rtsp2=new ZRtspThread("192.168.137.10");
    this->m_rtsp2->ZBindQueue(&this->m_rtsp2imgProcMux2,///<
                                 &this->m_condRtsp2imgProcNotEmpty2,&this->m_condRtsp2imgProcNotFull2,///<
                                 &this->m_rtsp2imgProcFree2,&this->m_rtsp2imgProcUsed2);
    //opencv imgproc.
    this->m_imgProc=new ZImgProcThread;
    this->m_imgProc->ZBindQueue1(&this->m_rtsp2imgProcMux1,///<
                                 &this->m_condRtsp2imgProcNotEmpty1,&this->m_condRtsp2imgProcNotFull1,///<
                                 &this->m_rtsp2imgProcFree1,&this->m_rtsp2imgProcUsed1);
    this->m_imgProc->ZBindQueue2(&this->m_rtsp2imgProcMux2,///<
                                 &this->m_condRtsp2imgProcNotEmpty2,&this->m_condRtsp2imgProcNotFull2,///<
                                 &this->m_rtsp2imgProcFree2,&this->m_rtsp2imgProcUsed2);
    QObject::connect(this->m_imgProc,SIGNAL(ZSigNewImg1(QImage)),mainUI2->ZGetDispUI(0),SLOT(ZSlotFlushImg(QImage)));
    QObject::connect(this->m_imgProc,SIGNAL(ZSigNewImg2(QImage)),mainUI2->ZGetDispUI(1),SLOT(ZSlotFlushImg(QImage)));

    this->m_rtsp1->start();
    this->m_rtsp2->start();
    this->m_imgProc->start();
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
