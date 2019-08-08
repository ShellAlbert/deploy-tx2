#include <QDebug>
#include <QDateTime>
#include <QSettings>
#include <QCoreApplication>

#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "zaudiotask.h"
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

    ZAudioTask *task=new ZAudioTask;
    if(task->ZStartTask()<0)
    {
        qDebug()<<"<error>:failed to start audio task!";
        return -1;
    }

    //install signal handler.
    //Set the signal callback for Ctrl-C
    signal(SIGINT,gSIGHandler);

    ret=a.exec();

    while(!task->ZIsExitCleanup())
    {
        qDebug()<<"<exit>:waiting for threads...";
    }
    delete task;
    task=NULL;

    qDebug()<<"<exit>:done.";
    return ret;
}
