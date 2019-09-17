#include "zmainui.h"
#include <QDebug>
#include "alsa/zaudiotask.h"
#include "forward/ztcp2uartthread.h"
#include "json/zjsonthread.h"
#include "imgproc/zvideotask.h"
#include <QApplication>
ZMainUI::ZMainUI(QWidget *parent):QWidget(parent)
{
    this->m_UILft=NULL;
    this->m_UIRht=NULL;
    this->m_hLayout=NULL;


    this->m_btnTM=NULL;
    this->m_btnCSK=NULL;
    this->m_btnKCF=NULL;
    this->m_btnTrackOff=NULL;
    this->m_btnExit=NULL;
    this->m_hLayoutBtn=NULL;

    this->m_vLayout=NULL;
}

ZMainUI::~ZMainUI()
{
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
    if(this->m_btnTM)
    {
        delete this->m_btnTM;
    }
    if(this->m_btnCSK)
    {
        delete this->m_btnCSK;
    }
    if(this->m_btnKCF)
    {
        this->m_btnKCF;
    }
    if(this->m_btnTrackOff)
    {
        delete this->m_btnTrackOff;
    }
    if(this->m_btnExit)
    {
        delete this->m_btnExit;
    }
    if(this->m_hLayoutBtn)
    {
        delete this->m_hLayoutBtn;
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
        this->m_UILft=new ZImgDispUI("MAIN",true);
        this->m_UIRht=new ZImgDispUI("AUX");
        this->m_hLayout=new QHBoxLayout;

        this->m_btnTM=new QPushButton(tr("TemplateMatch"));
        this->m_btnCSK=new QPushButton(tr("CSK"));
        this->m_btnKCF=new QPushButton(tr("KCF"));
        this->m_btnTrackOff=new QPushButton(tr("OFF"));

        this->m_btnExit=new QPushButton(tr("EXIT"));
        this->m_hLayoutBtn=new QHBoxLayout;
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

    QObject::connect(this->m_btnTM,SIGNAL(clicked(bool)),this,SLOT(ZSlotTemplateMatch()));
    QObject::connect(this->m_btnCSK,SIGNAL(clicked(bool)),this,SLOT(ZSlotTrackCSK()));
    QObject::connect(this->m_btnKCF,SIGNAL(clicked(bool)),this,SLOT(ZSlotTrackKCF()));
    QObject::connect(this->m_btnTrackOff,SIGNAL(clicked(bool)),this,SLOT(ZSlotTrackOff()));

    QObject::connect(this->m_btnExit,SIGNAL(clicked(bool)),this,SLOT(ZSlotExit()));

    this->m_hLayoutBtn->addWidget(this->m_btnTM);
    this->m_hLayoutBtn->addWidget(this->m_btnCSK);
    this->m_hLayoutBtn->addWidget(this->m_btnKCF);
    this->m_hLayoutBtn->addWidget(this->m_btnTrackOff);
    this->m_hLayoutBtn->addStretch(1);
    this->m_hLayoutBtn->addWidget(this->m_btnExit);

    this->m_vLayout->addLayout(this->m_hLayout);
    this->m_vLayout->addLayout(this->m_hLayoutBtn);
    this->m_vLayout->addStretch(1);
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
    if(this->m_audio && !this->m_audio->ZIsCleanup())
    {
        bExitAllFlag=false;
    }
    if(this->m_tcp2uart && !this->m_tcp2uart->ZIsCleanup())
    {
        bExitAllFlag=false;
    }
    if(this->m_json && !this->m_json->ZIsCleanup())
    {
        bExitAllFlag=false;
    }
    if(this->m_video && !this->m_video->ZIsCleanup())
    {
        bExitAllFlag=false;
    }

    if(bExitAllFlag)
    {
        this->m_timerExit->stop();
        qApp->exit(0);
    }
}

void ZMainUI::closeEvent(QCloseEvent *event)
{
    event->ignore();
}
void ZMainUI::ZSlotTemplateMatch()
{
    gGblPara.m_nAlgorithm=OPENCV_TEMPLATE_MATCH;
}
void ZMainUI::ZSlotTrackCSK()
{
    gGblPara.m_nAlgorithm=OPENCV_CSK_TRACKER;
}
void ZMainUI::ZSlotTrackKCF()
{
    gGblPara.m_nAlgorithm=OPENCV_KCF_TRACKER;
}
void ZMainUI::ZSlotTrackOff()
{
    gGblPara.m_nAlgorithm=IMGPROC_BYPASS;
}
void ZMainUI::ZSlotExit()
{
    gGblPara.m_bGblRst2Exit=true;
}
