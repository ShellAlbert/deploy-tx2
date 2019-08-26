/**
  * project code: p16
  * config file: p16.ini

json控制端口协议
tcp流边界:length+json data

采用request-response通信方式，Android端作为请求方，ARM Linux端作为应答方。

一、json协议格式规定¶

{
    "name":"zhangshaoyan",
    "age":30,
    "country":"America"
}
二、json协议
2.1 Android请求设置数据

{
    "ImgPro":"on/off/query"  请求开启/关闭/查询图像处理功能
    "RTC":"2018/07/19 14:26:53"  请求更新ARMLinux的硬件时间
    "DeNoise":"off/Strong/WebRTC/mmse/Bevis/NRAE/query"  请求关闭或打开音频噪声抑制算法
    "BevisGrade":"0/1/2/query"  设置Bevis降噪算法等级
    "DGain":"[0-90]/query"  设置音频数字增益,有效范围[0-90],query为查询当前值
    "FlushUI":"on/off/query"   刷新本地UI
    "Cam1CenterXY":"320,240/query"  设置或查询1号摄像头标定中心点坐标
    "Cam2CenterXY":"320,240/query"  设置或查询2号摄像头标定中心点坐标
    "Accumulated":"query"  查询设备累计运行秒数
    "StrongMode":"mode1/mode2/mode3/mode4/mode5/mode6/mode7/mode8/mode9/mode10/query" 设置/查询不同的噪声模式
    "MainCamSw":"sw/query"  请求切换主摄像头视场/查询当前视场
    "WebRtcGrade":"0/1/2/query"  设置WebRtc降噪算法等级

    "NRAEPara":[
        {"BandGain0":0~25},
        {"BandGain1":0~25},
        {"BandGain2":0~25},
        {"BandGain3":0~25},
        {"BandGain4":0~25},
        {"BandGain5":0~25},
        {"BandGain6":0~25},
        {"BandGain7":0~25},
        {"EnhanceStyle":0~4},
        {"EnhanceGrade":0~2},
        {"DenoiseGrade":0~3},
        {"PreEnhance":true/false}
    ]

}
2.2 ARM Linux返回响应结果¶

{
    "ImgPro":"on/off"  返回当前图像处理功能的状态是开启还是关闭
    "RTC":"2018/07/19 14:26:54"  返回当前设备的RTC时间
    "DeNoise":"off/RNNoise/WebRTC/mmse/Bevis"  返回音频噪声抑制算法的当前状态
    "BevisGrade":"0/1/2"  返回Bevis降噪算法等级
    "DGain":"off/[0-90]"  返回数字增益当前值
    "FlushUI":"on/off"  返回是否刷新本地UI
    "Cam1CenterXY":"320,240"  返回1号摄像头标定中心点坐标
    "Cam2CenterXY":"320,240"  返回2号摄像头标定中心点坐标
    "Accumulated":"1202323"  返回设备累计运行秒数
    "ImgMatched":"x1,y1,w1,h1,x2,y2,w2,h2,diffX,diffY,costMs" 返回图像比对结果数据
    "StrongMode":"mode1/mode2/mode3/mode4/mode5/mode6/mode7/mode8/mode9/mode10/query"  返回当前的噪声模式
    "MainCamSw":"big/small" 返回当时是标准视场还是扩展视场
    "WebRtcGrade":"0/1/2"  返回WebRtc降噪算法等级
}
(x1,y1,w1,h1):1号摄像头的模板选定区域
(x2,y2,w2,h2):2号摄像头的匹配区域
(diffX,diffY):在2号摄像头图像中匹配到的区域中心点坐标与标定中心点坐标的差值
costMs:算法实际消耗的时间(毫秒)
 *
 */

#include "alsa/zaudiotask.h"
#include "alsa/zaudiortsp.h"
#include "forward/ztcp2uartthread.h"
#include "json/zjsonthread.h"
#include "ui/zmainui.h"
#include "imgproc/zvideotask.h"
#include "rtsp/zrtspaudiocapture.h"
#include "zgblpara.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QSettings>
#include <QFile>
#include <QDebug>
#include <QMutex>
#include <QProcess>

extern "C"
{
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
}
void gSIGHandler(int sigNo)
{
    switch(sigNo)
    {
    case SIGINT:
    case SIGKILL:
    case SIGTERM:
        qInfo()<<"got signal,prepare to exit...";
        gGblPara.m_bGblRst2Exit=true;
        break;
    default:
        break;
    }
}
void gCustomMessageHandler(QtMsgType type,const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);

    QString msgType;
    static QMutex mutex;
    mutex.lock();
    switch(type)
    {
    case QtInfoMsg:
        msgType=QString("<INFO>");
        break;
    case QtDebugMsg:
        msgType=QString("<DEBUG>");
        break;
    case QtWarningMsg:
        msgType=QString("<WARNING>");
        break;
    case QtCriticalMsg:
        msgType=QString("<CRITICAL>");
        break;
    case QtFatalMsg:
        msgType=QString("<FATAL>");
        break;
    default:
        break;
    }
    QString logMsg=QString("%1:%2:%3\r\n").arg(msgType).arg(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss")).arg(msg);
    QFile fileLog("p16.log");
    fileLog.open(QIODevice::WriteOnly|QIODevice::Append);
    fileLog.write(logMsg.toLatin1());
    fileLog.close();
    mutex.unlock();

    if(type==QtFatalMsg)
    {
        abort();
    }
}


int main(int argc, char *argv[])
{
    int ret;

    QCoreApplication p16(argc, argv);

    //install signal handler.
    //Set the signal callback for Ctrl-C
    signal(SIGINT,gSIGHandler);

    //install qt message handler.
    qInstallMessageHandler(gCustomMessageHandler);

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

    //initial USBPlaybackVolume to previous setting value.
    qint32 nVolume=gGblPara.m_audio.m_nSpeakerPlaybackVolume;
    if(nVolume>=0 && nVolume<=30)
    {
        qDebug()<<"init SpeakerPlaybackVolume to "<<nVolume;
    }else{
        qWarning()<<"SpeakerPlaybackVolume"<<nVolume<<"not in valid range[0~30],reset to 10.";
        nVolume=10;
    }
    //system("amixer -c 1 cset numid=6,iface=MIXER,name='Speaker Playback Volume' 10");
    QStringList argList;
    argList<<"-c";
    argList<<"1";
    argList<<"cset";
    argList<<"numid=6,iface=MIXER,name='Speaker Playback Volume'";
    argList<<QString::number(nVolume);
    QProcess process;
    process.start("/usr/bin/amixer",argList);
    process.waitForFinished();


    //1.audio thread: capture -> noise suppression -> tx & play.
//    ZAudioTask *audio=new ZAudioTask;
//    if(audio->ZStartTask()<0)
//    {
//        qCritical()<<"failed to start audio task!";
//        return -1;
//    }
     ZAudioRtsp *audio=new ZAudioRtsp;
     audio->start();

    //2.tcp to uart forward thread.
    ZTcp2UartForwardThread *tcp2uart=new ZTcp2UartForwardThread;
    if(tcp2uart->ZStartThread()<0)
    {
        qCritical()<<"failed to start tcp2uart thread!";
        return -1;
    }

    //3.json control thread.
    ZJsonThread *json=new ZJsonThread;
    if(json->ZStartThread()<0)
    {
        qCritical()<<"failed to start json thread!";
        return -1;
    }

    //4.ui.
    ZMainObj *ui=new ZMainObj;
    if(ui->ZDoInit()<0)
    {
        qCritical()<<"failed to initial main window.";
        return -1;
    }
    //6.manage threads.
    ui->ZManageThreads(audio,tcp2uart,json,NULL);
    ui->showMaximized();



    //write pid to file.
    QFile filePID("/tmp/p16.pid");
    if(!filePID.open(QIODevice::WriteOnly))
    {
        qCritical()<<"failed to write pid to file:"<<filePID.errorString()<<".";
        return -1;
    }
    char pidBuffer[32];
    memset(pidBuffer,0,sizeof(pidBuffer));
    sprintf(pidBuffer,"%d",getpid());
    filePID.write(pidBuffer,strlen(pidBuffer));
    filePID.close();

    //enter event loop until exit() was called.
    ret=p16.exec();

    //free resources.
    delete audio;
    audio=NULL;

    delete tcp2uart;
    tcp2uart=NULL;

    delete json;
    json=NULL;

    delete ui;
    ui=NULL;

    qInfo()<<"p16 done.\r\n\r\n";
    return ret;
#endif
}
