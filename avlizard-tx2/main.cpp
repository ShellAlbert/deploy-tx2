#include <QDebug>
#include <QDateTime>
#include <QSettings>
#include <QFile>
#include <QCoreApplication>

#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "zaudiotask.h"
#include "forward/ztcp2uartthread.h"
#include "json/zjsonthread.h"
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
    QCoreApplication a(argc, argv);

    //1.audio thread: capture -> noise suppression -> tx & play.
    ZAudioTask *task=new ZAudioTask;
    if(task->ZStartTask()<0)
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

    //install signal handler.
    //Set the signal callback for Ctrl-C
    signal(SIGINT,gSIGHandler);

    //write pid to file.
    QFile filePID("/tmp/ZAudioServer.pid");
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
    qDebug()<<"write pid to /tmp/ZAudioServer.pid,"<<pidBuffer<<".";


    //enter event loop until exit() was called.
    ret=a.exec();

    while(!task->ZIsExitCleanup())
    {
        qDebug()<<"<exit>:waiting for threads...";
    }

    delete task;
    task=NULL;

    delete tcp2uart;
    tcp2uart=NULL;

    delete json;
    json=NULL;

    qDebug()<<"<exit>:done.";
    return ret;
}
