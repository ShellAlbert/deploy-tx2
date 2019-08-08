#include "zgblpara.h"
#include <QDebug>
#include <QSettings>
#include <unistd.h>
#include <stdio.h>
#include <QFile>
ZAudioParam::ZAudioParam()
{

    //run mode.
    //0:Only do capture and write pcm to wav file.
    //1:realtime stream process,capture-process-playback.
    //1 at default.
    this->m_runMode=1;

    //capture audio sample rate.
    this->m_nCaptureSampleRate=48000;
    //playback audio sample rate.
    this->m_nPlaybackSampleRate=48000;
    //channel number for capture & playback.
    this->m_nChannelNum=2;
    //de-noise control.
    this->m_nDeNoiseMethod=0;

    this->m_nGaindB=0;
    this->m_nBevisGrade=1;
    //capture thread thread buffer overrun.
    this->m_nCapOverrun=0;
    //playback thread buffer underrun.
    this->m_nPlayUnderrun=0;

    //同一时刻我们仅允许一个audio tcp客户端连接.
    this->m_bAudioTcpConnected=false;

    //for noise view.
    this->m_nRNNoiseView=0;

    //default webRTC grade is 0.
    //valid range is 0.1.2.
    this->m_nWebRtcNsPolicy=0;

    //libns_professional 2019/6/27.
    this->m_nBandGain0=0;
    this->m_nBandGain1=0;
    this->m_nBandGain2=0;
    this->m_nBandGain3=0;
    this->m_nBandGain4=0;
    this->m_nBandGain5=0;
    this->m_nBandGain6=0;
    this->m_nBandGain7=0;
    this->m_nEnhanceStyle=0;
    this->m_nEnhanceGrade=0;
    this->m_nDenoiseGrade=0;
    this->m_bPreEnhance=false;
    this->m_bNsProfessionalFlag=false;
}
ZGblPara::ZGblPara()
{
    this->m_bDebugMode=false;
    this->m_bDumpCamInfo2File=false;
    this->m_bCaptureLog=false;
    this->m_bTransfer2PC=false;
    this->m_bTransferSpeedMonitor=false;
    this->m_bDumpUART=false;
    //default xMode is on.fMode is off.
    this->m_bXMode=true;
    this->m_bFMode=false;

    this->m_bGblRst2Exit=false;
    this->m_nAccumulatedSec=0;

    //Tcp2Uart thread exit flag.
    this->m_bTcp2UartConnected=false;
    this->m_nTcp2UartBytes=0;
    this->m_nUart2TcpBytes=0;


    //json thread.
    this->m_bJsonConnected=false;
    this->m_bJsonImgProOn=false;
}
ZGblPara::~ZGblPara()
{
}
void ZGblPara::readCfgFile()
{
    //read calibrate center.
    QSettings iniFile("AVLizard.ini",QSettings::IniFormat);
    iniFile.beginGroup("CAM1");
    gGblPara.m_widthCAM1=iniFile.value("width",0).toInt();
    gGblPara.m_heightCAM1=iniFile.value("height",0).toInt();
    gGblPara.m_fpsCAM1=iniFile.value("fps",0).toInt();
    gGblPara.m_calibrateX1=iniFile.value("x1",0).toInt();
    gGblPara.m_calibrateY1=iniFile.value("y1",0).toInt();
    iniFile.endGroup();
    iniFile.beginGroup("CAM2");
    gGblPara.m_widthCAM2=iniFile.value("width",0).toInt();
    gGblPara.m_heightCAM2=iniFile.value("height",0).toInt();
    gGblPara.m_fpsCAM2=iniFile.value("fps",0).toInt();
    gGblPara.m_calibrateX2=iniFile.value("x2",0).toInt();
    gGblPara.m_calibrateY2=iniFile.value("y2",0).toInt();
    iniFile.endGroup();
    iniFile.beginGroup("CAM3");
    gGblPara.m_calibrateX3=iniFile.value("x3",0).toInt();
    gGblPara.m_calibrateY3=iniFile.value("y3",0).toInt();
    iniFile.endGroup();
    iniFile.beginGroup("CuteTemplate");
    gGblPara.m_nCutTemplateWidth=iniFile.value("width",0).toInt();
    gGblPara.m_nCutTemplateHeight=iniFile.value("height",0).toInt();
    iniFile.endGroup();

#if 0
    if(1)
    {
        qDebug()<<"CAM1 parameters:";
        qDebug()<<"resolution:"<<gGblPara.m_widthCAM1<<"*"<<gGblPara.m_heightCAM1<<",fps:"<<gGblPara.m_fpsCAM1;
        qDebug()<<"calibrate center point ("<<gGblPara.m_calibrateX1<<","<<gGblPara.m_calibrateY1<<")";

        qDebug()<<"CAM2 parameters:";
        qDebug()<<"resolution:"<<gGblPara.m_widthCAM2<<"*"<<gGblPara.m_heightCAM2<<",fps:"<<gGblPara.m_fpsCAM2;
        qDebug()<<"calibrate center point ("<<gGblPara.m_calibrateX2<<","<<gGblPara.m_calibrateY2<<")";
        qDebug()<<"Cut Template size: ("<<gGblPara.m_nCutTemplateWidth<<"*"<<gGblPara.m_nCutTemplateHeight<<")";
    }
#endif
}
void ZGblPara::writeCfgFile()
{
    //write calibrate center.
    QSettings iniFile("AVLizard.ini",QSettings::IniFormat);
    iniFile.beginGroup("CAM1");
    iniFile.setValue("width",gGblPara.m_widthCAM1);
    iniFile.setValue("height",gGblPara.m_heightCAM1);
    iniFile.setValue("fps",gGblPara.m_fpsCAM1);
    iniFile.setValue("x1",gGblPara.m_calibrateX1);
    iniFile.setValue("y1",gGblPara.m_calibrateY1);
    iniFile.endGroup();
    iniFile.beginGroup("CAM2");
    iniFile.setValue("width",gGblPara.m_widthCAM2);
    iniFile.setValue("height",gGblPara.m_heightCAM2);
    iniFile.setValue("fps",gGblPara.m_fpsCAM2);
    iniFile.setValue("x2",gGblPara.m_calibrateX2);
    iniFile.setValue("y2",gGblPara.m_calibrateY2);
    iniFile.endGroup();
    iniFile.beginGroup("CAM3");
    iniFile.setValue("x3",gGblPara.m_calibrateX3);
    iniFile.setValue("y3",gGblPara.m_calibrateY3);
    iniFile.endGroup();

    iniFile.beginGroup("CuteTemplate");
    iniFile.setValue("width",gGblPara.m_nCutTemplateWidth);
    iniFile.setValue("height",gGblPara.m_nCutTemplateHeight);
    iniFile.endGroup();

}
void ZGblPara::resetCfgFile()
{
    gGblPara.m_widthCAM1=640;
    gGblPara.m_heightCAM1=480;
    gGblPara.m_fpsCAM1=15;
    gGblPara.m_calibrateX1=320;
    gGblPara.m_calibrateY1=240;


    gGblPara.m_widthCAM2=640;
    gGblPara.m_heightCAM2=480;
    gGblPara.m_fpsCAM2=15;
    gGblPara.m_calibrateX2=400;
    gGblPara.m_calibrateY2=310;


    gGblPara.m_nCutTemplateWidth=200;
    gGblPara.m_nCutTemplateHeight=200;

    this->writeCfgFile();
}
uint8_t ZGblPara::ZDoCheckSum(uint8_t *pack,uint8_t pack_len)
{
    uint8_t i;
    uint8_t check_sum=0;
    for(i=0;i<pack_len;i++)
    {
        check_sum+=*(pack+i);
    }
    return check_sum;
}
void ZGblPara::int32_char8x4(qint32 int32,char *char8x4)
{
    char *pInt32=(char*)&int32;
    char8x4[0]=pInt32[0];
    char8x4[1]=pInt32[1];
    char8x4[2]=pInt32[2];
    char8x4[3]=pInt32[3];
}
void ZGblPara::int32_char8x2_low(qint32 int32,char *char8x2)
{
    char *pInt32=(char*)&int32;
    char8x2[0]=pInt32[0];
    char8x2[1]=pInt32[1];
    if(int32<0)
    {
        char8x2[1]|=0x80;
    }
}
ZGblPara gGblPara;


QByteArray qint32ToQByteArray(qint32 val)
{
    QByteArray ba;
    ba.resize(4);
    ba[0]=(uchar)((0x000000FF & val)>>0);
    ba[1]=(uchar)((0x0000FF00 & val)>>8);
    ba[2]=(uchar)((0x00FF0000 & val)>>16);
    ba[3]=(uchar)((0xFF000000 & val)>>24);
    return ba;
}
qint32 QByteArrayToqint32(QByteArray ba)
{
    qint32 val=0;
    val|=((ba[0]<<0)&0x000000FF);
    val|=((ba[1]<<8)&0x0000FF00);
    val|=((ba[2]<<16)&0x00FF0000);
    val|=((ba[3]<<24)&0xFF000000);
    return val;
}
