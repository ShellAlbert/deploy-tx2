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

    //channel number for capture & playback.
    this->m_nChannelNum=2;
    //de-noise control.
    this->m_nDeNoiseMethod=0;

    this->m_nGaindB=0;

    //capture thread thread buffer overrun.
    this->m_nCapOverrun=0;
    //playback thread buffer underrun.
    this->m_nPlayUnderrun=0;

    //only one audio tcp client connect allowed.
    this->m_bAudioTcpConnected=false;

    //for noise view.
    this->m_nRNNoiseView=0;

    //default webRTC grade is 0.
    //valid range is 0.1.2.
    this->m_nWebRtcNsPolicy=0;
    this->m_bWebRtcInitFlag=false;

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
    this->m_bGblRst2Exit=false;
    this->m_nAccumulatedSec=0;

    //Tcp2Uart thread.
    this->m_bTcp2UartConnected=false;
    this->m_nTcp2UartBytes=0;
    this->m_nUart2TcpBytes=0;


    //json thread.
    this->m_bJsonConnected=false;
    this->m_bJsonImgProOn=false;
    this->m_nDiffX=0;
    this->m_nDiffY=0;

    //imgproc algorithm.
    this->m_nAlgorithm=IMGPROC_BYPASS;
}
ZGblPara::~ZGblPara()
{
}
void ZGblPara::readCfgFile()
{
    //read calibrate center.
    QSettings iniFile("p16.ini",QSettings::IniFormat);
    iniFile.beginGroup("CAM1");
    gGblPara.m_calCenterX1=iniFile.value("x1",0).toInt();
    gGblPara.m_calCenterY1=iniFile.value("y1",0).toInt();
    iniFile.endGroup();

    iniFile.beginGroup("CAM2");
    gGblPara.m_calCenterX2=iniFile.value("x2",0).toInt();
    gGblPara.m_calCenterY2=iniFile.value("y2",0).toInt();
    iniFile.endGroup();

    iniFile.beginGroup("CAM3");
    gGblPara.m_calCenterX3=iniFile.value("x3",0).toInt();
    gGblPara.m_calCenterY3=iniFile.value("y3",0).toInt();
    iniFile.endGroup();

    iniFile.beginGroup("CutBox");
    gGblPara.m_nCutBoxWidth=iniFile.value("width",0).toInt();
    gGblPara.m_nCutBoxHeight=iniFile.value("height",0).toInt();
    iniFile.endGroup();
}
void ZGblPara::writeCfgFile()
{
    //write calibrate center.
    QSettings iniFile("p16.ini",QSettings::IniFormat);
    iniFile.beginGroup("CAM1");
    iniFile.setValue("x1",gGblPara.m_calCenterX1);
    iniFile.setValue("y1",gGblPara.m_calCenterY1);
    iniFile.endGroup();

    iniFile.beginGroup("CAM2");
    iniFile.setValue("x2",gGblPara.m_calCenterX2);
    iniFile.setValue("y2",gGblPara.m_calCenterY2);
    iniFile.endGroup();

    iniFile.beginGroup("CAM3");
    iniFile.setValue("x3",gGblPara.m_calCenterX3);
    iniFile.setValue("y3",gGblPara.m_calCenterY3);
    iniFile.endGroup();

    iniFile.beginGroup("CutBox");
    iniFile.setValue("width",gGblPara.m_nCutBoxWidth);
    iniFile.setValue("height",gGblPara.m_nCutBoxHeight);
    iniFile.endGroup();

}
void ZGblPara::initCfgFile()
{
    gGblPara.m_calCenterX1=RTSP_H264_WIDTH/2;
    gGblPara.m_calCenterY1=RTSP_H264_HEIGHT/2;

    gGblPara.m_calCenterX2=RTSP_H264_WIDTH/2;
    gGblPara.m_calCenterY2=RTSP_H264_HEIGHT/2;

    gGblPara.m_calCenterX3=RTSP_H264_WIDTH/2;
    gGblPara.m_calCenterY3=RTSP_H264_HEIGHT/2;

    gGblPara.m_nCutBoxWidth=50;
    gGblPara.m_nCutBoxHeight=50;

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
cv::Mat QImage2cvMat(const QImage &img)
{
    cv::Mat mat;
    //qDebug()<<"format:"<<img.format();
    switch(img.format())
    {
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32_Premultiplied:
        mat=cv::Mat(img.height(),img.width(),CV_8UC4,(void*)img.constBits(),img.bytesPerLine());
        break;
    case QImage::Format_RGB888:
        mat=cv::Mat(img.height(),img.width(),CV_8UC3,(void*)img.constBits(),img.bytesPerLine());
        cv::cvtColor(mat,mat,CV_BGR2RGB);
        break;
    case QImage::Format_Indexed8:
        mat=cv::Mat(img.height(),img.width(),CV_8UC1,(void*)img.constBits(),img.bytesPerLine());
        break;
    default:
        break;
    }
    return mat;
}
QImage cvMat2QImage(const cv::Mat &mat)
{
    //8-bit unsigned ,number of channels=1.
    if(mat.type()==CV_8UC1)
    {
        QImage img(mat.cols,mat.rows,QImage::Format_Indexed8);
        //set the color table
        //used to translate colour indexes to qRgb values.
        img.setColorCount(256);
        for(qint32 i=0;i<256;i++)
        {
            img.setColor(i,qRgb(i,i,i));
        }
        //copy input mat.
        uchar *pSrc=mat.data;
        for(qint32 row=0;row<mat.rows;row++)
        {
            uchar *pDst=img.scanLine(row);
            memcpy(pDst,pSrc,mat.cols);
            pSrc+=mat.step;
        }
        return img;
    }else if(mat.type()==CV_8UC3)
    {
        //8-bits unsigned,number of channels=3.
        const uchar *pSrc=(const uchar*)mat.data;
        QImage img(pSrc,mat.cols,mat.rows,mat.step,QImage::Format_RGB888);
        return img.rgbSwapped();
    }else if(mat.type()==CV_8UC4)
    {
        const uchar *pSrc=(const uchar*)mat.data;
        QImage img(pSrc,mat.cols,mat.rows,mat.step,QImage::Format_ARGB32);
        return img.copy();
    }else{
        qDebug()<<"<Error>:failed to convert cvMat to QImage!";
        return QImage();
    }
}
