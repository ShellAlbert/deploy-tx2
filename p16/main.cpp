/**
  * project code: p16
  * config file: p16.ini
  *
  *
  */
#include <QApplication>
#include <QDateTime>
#include <QSettings>
#include <QFile>
#include <QDebug>

#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "alsa/zaudiotask.h"
#include "forward/ztcp2uartthread.h"
#include "json/zjsonthread.h"
#include "ui/zmainui.h"
#include "imgproc/zvideotask.h"
#include "rtsp/zrtspaudiocapture.h"
#include "zgblpara.h"
void gSIGHandler(int sigNo)
{
    switch(sigNo)
    {
    case SIGINT:
    case SIGKILL:
    case SIGTERM:
        qDebug()<<"prepare to exit...";
        gGblPara.m_bGblRst2Exit=true;
        break;
    default:
        break;
    }
}


int main(int argc, char *argv[])
{
    int ret;
    QApplication p16(argc, argv);
//    ZRtspAudioCapture *cap=new ZRtspAudioCapture;
//    cap->start();
//    return p16.exec();
#if 1

    //read config file.
    QFile fileCfg("p16.ini");
    if(!fileCfg.exists())
    {
        gGblPara.initCfgFile();
    }
    gGblPara.readCfgFile();

    //1.audio thread: capture -> noise suppression -> tx & play.
    ZAudioTask *audio=new ZAudioTask;
    if(audio->ZStartTask()<0)
    {
        qDebug()<<"<error>:failed to start audio task!";
        return -1;
    }

    //2.tcp to uart forward thread.
    ZTcp2UartForwardThread *tcp2uart=new ZTcp2UartForwardThread;
    if(tcp2uart->ZStartThread()<0)
    {
        qDebug()<<"<error>:failed to start tcp2uart thread!";
        return -1;
    }

    //3.json control thread.
    ZJsonThread *json=new ZJsonThread;
    if(json->ZStartThread()<0)
    {
        qDebug()<<"<error>:failed to start json thread!";
        return -1;
    }

    //4.ui.
    ZMainUI *ui=new ZMainUI;
    if(ui->ZDoInit()<0)
    {
        qDebug()<<"<error>:failed to initial main window.";
        return -1;
    }


    //5.video task.
//    ZVideoTask *video=new ZVideoTask;
//    if(video->ZStartTask(ui)<0)
//    {
//        qDebug()<<"<error>:failed to start video task.";
//        return -1;
//    }

    //6.manage threads.
    ui->ZManageThreads(audio,tcp2uart,json,NULL/*video*/);
    ui->showMaximized();
    //install signal handler.
    //Set the signal callback for Ctrl-C
    signal(SIGINT,gSIGHandler);

    //write pid to file.
    QFile filePID("/tmp/p16.pid");
    if(!filePID.open(QIODevice::WriteOnly))
    {
        qDebug()<<"<error>:error to write pid file."<<filePID.errorString();
        return -1;
    }
    char pidBuffer[32];
    memset(pidBuffer,0,sizeof(pidBuffer));
    sprintf(pidBuffer,"%d",getpid());
    filePID.write(pidBuffer,strlen(pidBuffer));
    filePID.close();
    qDebug()<<"write pid to /tmp/p16.pid,"<<pidBuffer<<".";

    //enter event loop until exit() was called.
    ret=p16.exec();

    //free resources.
    delete audio;
    audio=NULL;

//    delete video;
//    video=NULL;

    delete tcp2uart;
    tcp2uart=NULL;

    delete json;
    json=NULL;

    delete ui;
    ui=NULL;

    qDebug()<<"<exit>:done.";
    return ret;
#endif
}
