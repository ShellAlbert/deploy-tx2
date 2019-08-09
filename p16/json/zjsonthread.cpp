#include "zjsonthread.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <sys/socket.h>
#include <zgblpara.h>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QDateTime>
#include <QProcess>

ZJsonThread::ZJsonThread(QObject *parent):QThread(parent)
{
    this->m_tcpSocket=NULL;
    this->m_nJsonLen=0;
}
qint32 ZJsonThread::ZStartThread()
{
    this->start();
    return 0;
}
void ZJsonThread::run()
{
    QTcpServer *tcpServer=new QTcpServer;
    int on=1;
    int sockFd=tcpServer->socketDescriptor();
    setsockopt(sockFd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

    this->m_recvBuffer=new qint8[BUFSIZE_1MB];
    while(!gGblPara.m_bGblRst2Exit)
    {
        //tcp disconnected.
        gGblPara.m_bJsonConnected=false;

        //listen.
        if(!tcpServer->listen(QHostAddress::Any,TCP_PORT_CTL))
        {
            qDebug()<<"<Error>: Ctl server listen error on port:"<<TCP_PORT_CTL;
            gGblPara.m_bGblRst2Exit=true;
            break;
        }

        //wait until connected.
        while(!gGblPara.m_bGblRst2Exit)
        {
            if(tcpServer->waitForNewConnection(1000*1))//1s.
            {
                //tcp connected.
                gGblPara.m_bJsonConnected=true;
                break;
            }
        }


        this->m_tcpSocket=tcpServer->nextPendingConnection();
        if(NULL==this->m_tcpSocket)
        {
            qDebug()<<"<Error>:empty next pending connection.";
            continue;
        }

        //同一时刻只允许一个客户端连接，当连接上时，则关闭服务端.
        tcpServer->close();

        //read & write.
        this->m_nJsonLen=0;
        while(!gGblPara.m_bGblRst2Exit)
        {
            if(this->m_tcpSocket->waitForReadyRead(SOCKET_TX_TIMEOUT))//100ms.
            {
                QByteArray baJsonData=this->m_tcpSocket->readAll();
                if(baJsonData.size()<=0)
                {
                    qDebug()<<"<Warning>:not read any json data.";
                    continue;
                }
                //                qDebug()<<"----------------read all begin-----------";
                //                qDebug()<<baJsonData;
                //                qDebug()<<"----------------read all end-----------";
                if(baJsonData.size()>0)
                {
                    memcpy(this->m_recvBuffer+this->m_nJsonLen,baJsonData.data(),baJsonData.size());
                    this->m_nJsonLen+=baJsonData.size();
                    this->m_recvBuffer[this->m_nJsonLen]='\0';
                    this->ZScanRecvBuffer();
                }
            }
            //只有开启图像匹配功能时才上传匹配结果集.
#if 1
            if(gGblPara.m_bJsonImgProOn)
            {
                QJsonObject jsonObj;
                //                jsonObj.insert("ImgMatched",///<
                //                               QString("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11")///<
                //                               .arg(gGblPara.m_video.m_rectTemplate.x()).arg(gGblPara.m_video.m_rectTemplate.y())///<
                //                               .arg(gGblPara.m_video.m_rectTemplate.width()).arg(gGblPara.m_video.m_rectTemplate.height())///<
                //                               .arg(gGblPara.m_video.m_rectMatched.x()).arg(gGblPara.m_video.m_rectMatched.y())///<
                //                               .arg(gGblPara.m_video.m_rectMatched.width()).arg(gGblPara.m_video.m_rectMatched.height())///<
                //                               .arg(gGblPara.m_video.m_nDiffX).arg(gGblPara.m_video.m_nDiffY).arg(gGblPara.m_video.m_nCostMs));
                QJsonDocument jsonDoc;
                jsonDoc.setObject(jsonObj);
                QByteArray baTx=jsonDoc.toJson();
                QByteArray baTxLen=qint32ToQByteArray(baTx.size());
                //qDebug()<<baTx;
                this->m_tcpSocket->write(baTxLen);
                if(!this->m_tcpSocket->waitForBytesWritten(1000))
                {
                    qDebug()<<"<Error>:json tcp socket timeout.reset";
                    break;
                }
                this->m_tcpSocket->write(baTx);
                if(!this->m_tcpSocket->waitForBytesWritten(1000))
                {
                    qDebug()<<"<Error>:json tcp socket timeout.reset";
                    break;
                }
            }
#endif
            if(this->m_tcpSocket->state()!=QAbstractSocket::ConnectedState)
            {
                qDebug()<<"<Error>:tcp socket broken.";
                break;
            }
        }
    }
    delete []this->m_recvBuffer;
    this->m_recvBuffer=NULL;

    tcpServer->close();
    delete tcpServer;
    tcpServer=NULL;

    emit this->ZSigThreadExited();
}

qint32 ZJsonThread::ZScanRecvBuffer()
{
    //    QByteArray prt((char*)this->m_recvBuffer,this->m_nJsonLen);
    //    qDebug()<<prt;
    qint32 nOffset=0;
    qint32 nRemainingBytes=this->m_nJsonLen;
    while(nRemainingBytes>0)
    {
        //fetch 4 bytes json pkt len.
        //check the remainings bytes is feet or not.
        if(nRemainingBytes<4)//we need more data,check next time.
        {
            break;
        }
        QByteArray baJsonLen((char*)(this->m_recvBuffer+nOffset+0),4);
        //convert to int.
        qint32 nJsonLen=QByteArrayToqint32(baJsonLen);
        if(nJsonLen<0)
        {
            qDebug()<<"<Error>:error convert json pkt len.reset.";
            this->m_nJsonLen=0;
            return -1;
        }
        //qDebug()<<"json len:"<<nJsonLen;
        //fetch n bytes json pkt data.
        //check the remaining bytes is feet or not.
        if(nRemainingBytes<(4+nJsonLen))//we need more data,check next time.
        {
            break;
        }
        QByteArray baJsonData((char*)(this->m_recvBuffer+nOffset+4),nJsonLen);
        //here we use (json pkt len+json pkt data) as a unit.
        //so here we add/sub in a unit.
        nOffset+=(4+nJsonLen);
        nRemainingBytes-=(4+nJsonLen);
        //qDebug()<<baJsonData;
#if 1
        //parse out json.
        QJsonParseError jsonErr;
        QJsonDocument jsonDoc=QJsonDocument::fromJson(baJsonData,&jsonErr);
        if(jsonErr.error==QJsonParseError::NoError)
        {
            QByteArray baFeedBack=this->ZParseJson(jsonDoc);
            if(baFeedBack.size()>0)
            {
                QByteArray baFeedBackLen=qint32ToQByteArray(baFeedBack.size());
                this->m_tcpSocket->write(baFeedBackLen);
                this->m_tcpSocket->waitForBytesWritten(100);
                this->m_tcpSocket->write(baFeedBack);
                this->m_tcpSocket->waitForBytesWritten(100);
            }
        }
        //        qDebug()<<"nRemaingBytes:"<<nRemainingBytes;
#endif
    }
    if(nRemainingBytes>0 && nOffset>0)
    {
        memmove(this->m_recvBuffer,this->m_recvBuffer+nOffset,nRemainingBytes);
        this->m_nJsonLen=nRemainingBytes;
        this->m_recvBuffer[nRemainingBytes]='\0';
        //        qDebug()<<"after processed:remaing:"<<this->m_nJsonLen;
        //        qDebug()<<QByteArray((const char*)this->m_recvBuffer,this->m_nJsonLen);
    }else{
        //processed all bytes.nRemainingBytes=0,so reset here.
        memset(this->m_recvBuffer,0,BUFSIZE_1MB);
        this->m_nJsonLen=0;
    }
    return 0;
}
QByteArray ZJsonThread::ZParseJson(const QJsonDocument &jsonDoc)
{
    bool bWrCfgFileFlag=false;
    QJsonObject jsonObjFeedBack;
    if(jsonDoc.isObject())
    {
        QJsonObject jsonObj=jsonDoc.object();
        if(jsonObj.contains("ImgPro"))
        {
            QJsonValue val=jsonObj.take("ImgPro");
            if(val.isString())
            {
                QString method=val.toVariant().toString();
                if(method=="on")
                {
                    //设置图像比对开启标志位.
                    //这将引起采集线程向Process队列扔图像数据.
                    //从而ImgProcess线程解除等待开始处理图像.
                    gGblPara.m_bJsonImgProOn=true;
                }else if(method=="off")
                {
                    //设置图像比对暂停标志位.
                    //这将引起采集线程不再向Process队列扔图像数据.
                    //使得ImgProcess线程等待信号量从而暂停.
                    gGblPara.m_bJsonImgProOn=false;
                }else if(method=="query")
                {
                    //仅用于查询当前状态.
                }
                jsonObjFeedBack.insert("ImgPro",gGblPara.m_bJsonImgProOn?"on":"false");
            }
        }
        if(jsonObj.contains("RTC"))
        {
            QJsonValue val=jsonObj.take("RTC");
            if(val.isString())
            {
                QString rtcStr=val.toVariant().toString();
                qDebug()<<"RTC:"<<rtcStr;
                QString cmdSetRtc=QString("date -s %1").arg(rtcStr);
                qDebug()<<"cmdSetRtc:"<<cmdSetRtc;
                QProcess::startDetached(cmdSetRtc);
                QProcess::startDetached("hwclock -w");
                QProcess::startDetached("sync");
                jsonObjFeedBack.insert("RTC",QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss"));
            }
        }
        if(jsonObj.contains("DeNoise"))
        {
            QJsonValue val=jsonObj.take("DeNoise");
            if(val.isString())
            {
                QString deNoise=val.toVariant().toString();
                qDebug()<<"deNoise:"<<deNoise;
                if(deNoise=="off")
                {
                    gGblPara.m_audio.m_nDeNoiseMethod=0;
                }else if(deNoise=="Strong")
                {
                    gGblPara.m_audio.m_nDeNoiseMethod=1;
                }else if(deNoise=="WebRTC")
                {
                    gGblPara.m_audio.m_nDeNoiseMethod=2;
                }else if(deNoise=="Bevis")
                {
                    gGblPara.m_audio.m_nDeNoiseMethod=3;
                }else if(deNoise=="mmse")
                {
                    gGblPara.m_audio.m_nDeNoiseMethod=4;
                }else if(deNoise=="query")
                {
                    //仅用于查询当前状态.
                }
                switch(gGblPara.m_audio.m_nDeNoiseMethod)
                {
                case 0:
                    jsonObjFeedBack.insert("DeNoise","off");
                    break;
                case 1:
                    jsonObjFeedBack.insert("DeNoise","Strong");
                    break;
                case 2:
                    jsonObjFeedBack.insert("DeNoise","WebRTC");
                    break;
                case 3:
                    jsonObjFeedBack.insert("DeNoise","Bevis");
                    break;
                case 4:
                    jsonObjFeedBack.insert("DeNoise","mmse");
                    break;
                default:
                    break;
                }
            }
        }
        if(jsonObj.contains("WebRtcGrade"))
        {
            QJsonValue val=jsonObj.take("WebRtcGrade");
            if(val.isString())
            {
                QString webRtcGrade=val.toVariant().toString();
                //qDebug()<<"webRtcGrade:"<<webRtcGrade;
                if(webRtcGrade=="0")
                {
                    gGblPara.m_audio.m_nWebRtcNsPolicy=0;
                }else if(webRtcGrade=="1")
                {
                    gGblPara.m_audio.m_nWebRtcNsPolicy=1;
                }else if(webRtcGrade=="2")
                {
                    gGblPara.m_audio.m_nWebRtcNsPolicy=2;
                }else if(webRtcGrade=="query")
                {
                    //仅用于查询当前状态.
                }
                switch(gGblPara.m_audio.m_nWebRtcNsPolicy)
                {
                case 0:
                    jsonObjFeedBack.insert("WebRtcGrade","0");
                    break;
                case 1:
                    jsonObjFeedBack.insert("WebRtcGrade","1");
                    break;
                case 2:
                    jsonObjFeedBack.insert("WebRtcGrade","2");
                    break;
                default:
                    break;
                }
            }
        }
        if(jsonObj.contains("DGain"))
        {
            QJsonValue val=jsonObj.take("DGain");
            if(val.isString())
            {
                QString dGain=val.toVariant().toString();
                if(dGain=="query")
                {
                    jsonObjFeedBack.insert("DGain",QString::number(gGblPara.m_audio.m_nGaindB));
                }else{
                    qint32 dGain=val.toVariant().toInt();
                    //qDebug()<<"DGain:"<<dGain;
                    gGblPara.m_audio.m_nGaindB=dGain;
                    jsonObjFeedBack.insert("DGain",QString::number(gGblPara.m_audio.m_nGaindB));
                }
            }
        }
        if(jsonObj.contains("Cam1CenterXY"))
        {
            QJsonValue val=jsonObj.take("Cam1CenterXY");
            if(val.isString())
            {
                QString cam1CenterXY=val.toVariant().toString();
                if(cam1CenterXY=="query")
                {
                    jsonObjFeedBack.insert("Cam1CenterXY",QString("%1,%2").arg(gGblPara.m_calCenterX1).arg(gGblPara.m_calCenterY1));
                }else{
                    QStringList xyList=cam1CenterXY.split(",");
                    if(xyList.size()!=2)
                    {
                        qDebug()<<"<Error>:failed to parse out (x,y) in Cam1CenterXY json.";
                    }else{
                        qint32 x=xyList.at(0).toInt();
                        qint32 y=xyList.at(1).toInt();
                        gGblPara.m_calCenterX1=x;
                        gGblPara.m_calCenterY1=y;
                        bWrCfgFileFlag=true;
                        jsonObjFeedBack.insert("Cam1CenterXY",QString("%1,%2").arg(gGblPara.m_calCenterX1).arg(gGblPara.m_calCenterY1));
                    }
                }
            }
        }
        if(jsonObj.contains("Cam2CenterXY"))
        {
            QJsonValue val=jsonObj.take("Cam2CenterXY");
            if(val.isString())
            {
                QString cam2CenterXY=val.toVariant().toString();
                if(cam2CenterXY=="query")
                {
                    jsonObjFeedBack.insert("Cam2CenterXY",QString("%1,%2").arg(gGblPara.m_calCenterX2).arg(gGblPara.m_calCenterY2));
                }else{
                    QStringList xyList=cam2CenterXY.split(",");
                    if(xyList.size()!=2)
                    {
                        qDebug()<<"<Error>:failed to parse out (x,y) in Cam1CenterXY json.";
                    }else{
                        qint32 x=xyList.at(0).toInt();
                        qint32 y=xyList.at(1).toInt();
                        gGblPara.m_calCenterX2=x;
                        gGblPara.m_calCenterY2=y;
                        bWrCfgFileFlag=true;
                        jsonObjFeedBack.insert("Cam2CenterXY",QString("%1,%2").arg(gGblPara.m_calCenterX2).arg(gGblPara.m_calCenterY2));
                    }
                }
            }
        }
        if(jsonObj.contains("Cam3CenterXY"))
        {
            QJsonValue val=jsonObj.take("Cam3CenterXY");
            if(val.isString())
            {
                QString cam3CenterXY=val.toVariant().toString();
                if(cam3CenterXY=="query")
                {
                    jsonObjFeedBack.insert("Cam3CenterXY",QString("%1,%2").arg(gGblPara.m_calCenterX3).arg(gGblPara.m_calCenterY3));
                }else{
                    QStringList xyList=cam3CenterXY.split(",");
                    if(xyList.size()!=2)
                    {
                        qDebug()<<"<Error>:failed to parse out (x,y) in Cam3CenterXY json.";
                    }else{
                        qint32 x=xyList.at(0).toInt();
                        qint32 y=xyList.at(1).toInt();
                        gGblPara.m_calCenterX3=x;
                        gGblPara.m_calCenterY3=y;
                        bWrCfgFileFlag=true;
                        jsonObjFeedBack.insert("Cam3CenterXY",QString("%1,%2").arg(gGblPara.m_calCenterX3).arg(gGblPara.m_calCenterY3));
                    }
                }
            }
        }
        if(jsonObj.contains("Accumulated"))
        {
            QJsonValue val=jsonObj.take("Accumulated");
            if(val.isString())
            {
                QString str=val.toVariant().toString();
                if(str=="query")
                {
                    jsonObjFeedBack.insert("Accumulated","0");
                }else{
                    qDebug()<<"<Error>:unknow cmd in Accumulated json.";
                }
            }
        }
        if(jsonObj.contains("Accumulated"))
        {
            QJsonValue val=jsonObj.take("Accumulated");
            if(val.isString())
            {
                QString str=val.toVariant().toString();
                if(str=="query")
                {
                    jsonObjFeedBack.insert("Accumulated",QString::number(gGblPara.m_nAccumulatedSec));
                }else{
                    qDebug()<<"<Error>:unknow cmd in Accumulated json.";
                }
            }
        }

        if(jsonObj.contains("StrongMode"))
        {
            QJsonValue val=jsonObj.take("StrongMode");
            if(val.isString())
            {
                QString noiseView=val.toVariant().toString();
                if(noiseView=="mode1")
                {
                    gGblPara.m_audio.m_nRNNoiseView=0;
                    jsonObjFeedBack.insert("StrongMode","mode1");
                }else if(noiseView=="mode2")
                {
                    gGblPara.m_audio.m_nRNNoiseView=1;
                    jsonObjFeedBack.insert("StrongMode","mode2");
                }else if(noiseView=="mode3")
                {
                    gGblPara.m_audio.m_nRNNoiseView=2;
                    jsonObjFeedBack.insert("StrongMode","mode3");
                }else if(noiseView=="mode4")
                {
                    gGblPara.m_audio.m_nRNNoiseView=3;
                    jsonObjFeedBack.insert("StrongMode","mode4");
                }else if(noiseView=="mode5")
                {
                    gGblPara.m_audio.m_nRNNoiseView=4;
                    jsonObjFeedBack.insert("StrongMode","mode5");
                }else if(noiseView=="mode6")
                {
                    gGblPara.m_audio.m_nRNNoiseView=5;
                    jsonObjFeedBack.insert("StrongMode","mode6");
                }else if(noiseView=="mode7")
                {
                    gGblPara.m_audio.m_nRNNoiseView=6;
                    jsonObjFeedBack.insert("StrongMode","mode7");
                }else if(noiseView=="mode8")
                {
                    gGblPara.m_audio.m_nRNNoiseView=7;
                    jsonObjFeedBack.insert("StrongMode","mode8");
                }else if(noiseView=="mode9")
                {
                    gGblPara.m_audio.m_nRNNoiseView=8;
                    jsonObjFeedBack.insert("StrongMode","mode9");
                }else if(noiseView=="mode10")
                {
                    gGblPara.m_audio.m_nRNNoiseView=9;
                    jsonObjFeedBack.insert("StrongMode","mode10");
                }
            }
        }
        //noise reduction and equalier.
        if(jsonObj.contains("NRAEPara"))
        {
            //qDebug()<<"has NRAEPara";
            QJsonValue val=jsonObj.take("NRAEPara");
            //qDebug()<<val;
            QByteArray baNRAEPara=val.toString().toUtf8();
            //qDebug()<<baNRAEPara;
            QJsonParseError err2;
            QJsonDocument doc2=QJsonDocument::fromJson(baNRAEPara,&err2);
            if(err2.error==QJsonParseError::NoError)
            {
                //qDebug()<<"no error";
                QJsonArray array=doc2.array();
                for(qint32 i=0;i<array.size();i++)
                {
                    QJsonObject obj=array.at(i).toObject();
                    if(obj.contains("BandGain0"))
                    {
                        gGblPara.m_audio.m_nBandGain0=obj.value("BandGain0").toInt();
                    }else if(obj.contains("BandGain1"))
                    {
                        gGblPara.m_audio.m_nBandGain1=obj.value("BandGain1").toInt();
                    }else if(obj.contains("BandGain2"))
                    {
                        gGblPara.m_audio.m_nBandGain2=obj.value("BandGain2").toInt();
                    }else if(obj.contains("BandGain3"))
                    {
                        gGblPara.m_audio.m_nBandGain3=obj.value("BandGain3").toInt();
                    }else if(obj.contains("BandGain4"))
                    {
                        gGblPara.m_audio.m_nBandGain4=obj.value("BandGain4").toInt();
                    }else if(obj.contains("BandGain5"))
                    {
                        gGblPara.m_audio.m_nBandGain5=obj.value("BandGain5").toInt();
                    }else if(obj.contains("BandGain6"))
                    {
                        gGblPara.m_audio.m_nBandGain6=obj.value("BandGain6").toInt();
                    }else if(obj.contains("BandGain7"))
                    {
                        gGblPara.m_audio.m_nBandGain7=obj.value("BandGain7").toInt();
                    }else if(obj.contains("EnhanceStyle"))
                    {
                        gGblPara.m_audio.m_nEnhanceStyle=obj.value("EnhanceStyle").toInt();
                    }else if(obj.contains("EnhanceGrade"))
                    {
                        gGblPara.m_audio.m_nEnhanceGrade=obj.value("EnhanceGrade").toInt();
                    }else if(obj.contains("DenoiseGrade"))
                    {
                        gGblPara.m_audio.m_nDenoiseGrade=obj.value("DenoiseGrade").toInt();
                    }else if(obj.contains("PreEnhance"))
                    {
                        //qDebug()<<obj.value("PreEnhance").toBool();
                        if(obj.value("PreEnhance").toBool())
                        {
                            gGblPara.m_audio.m_bPreEnhance=true;
                        }else{
                            gGblPara.m_audio.m_bPreEnhance=false;
                        }

                    }
                }
                gGblPara.m_audio.m_bNsProfessionalFlag=true;
                /*
                qDebug()<<"NRAE:"<<gGblPara.m_audio.m_nBandGain0 \
                       <<gGblPara.m_audio.m_nBandGain1 \
                      <<gGblPara.m_audio.m_nBandGain2 \
                     <<gGblPara.m_audio.m_nBandGain3 \
                    <<gGblPara.m_audio.m_nBandGain4 \
                   <<gGblPara.m_audio.m_nBandGain5 \
                  <<gGblPara.m_audio.m_nBandGain6 \
                 <<gGblPara.m_audio.m_nBandGain7 \
                <<gGblPara.m_audio.m_nEnhanceStyle \
                <<gGblPara.m_audio.m_nEnhanceGrade \
                <<gGblPara.m_audio.m_nDenoiseGrade \
                <<gGblPara.m_audio.m_bPreEnhance;
                */


                //generate feedback json data.
                QJsonObject jsonObjFeedBackNRAE;
                jsonObjFeedBackNRAE.insert("BandGain0",QString("%1").arg(gGblPara.m_audio.m_nBandGain0));
                jsonObjFeedBackNRAE.insert("BandGain1",QString("%1").arg(gGblPara.m_audio.m_nBandGain1));
                jsonObjFeedBackNRAE.insert("BandGain2",QString("%1").arg(gGblPara.m_audio.m_nBandGain2));
                jsonObjFeedBackNRAE.insert("BandGain3",QString("%1").arg(gGblPara.m_audio.m_nBandGain3));
                jsonObjFeedBackNRAE.insert("BandGain4",QString("%1").arg(gGblPara.m_audio.m_nBandGain4));
                jsonObjFeedBackNRAE.insert("BandGain5",QString("%1").arg(gGblPara.m_audio.m_nBandGain5));
                jsonObjFeedBackNRAE.insert("BandGain6",QString("%1").arg(gGblPara.m_audio.m_nBandGain6));
                jsonObjFeedBackNRAE.insert("BandGain7",QString("%1").arg(gGblPara.m_audio.m_nBandGain7));
                jsonObjFeedBackNRAE.insert("EnhanceStyle",QString("%1").arg(gGblPara.m_audio.m_nEnhanceStyle));
                jsonObjFeedBackNRAE.insert("EnhanceGrade",QString("%1").arg(gGblPara.m_audio.m_nEnhanceGrade));
                jsonObjFeedBackNRAE.insert("DenoiseGrade",QString("%1").arg(gGblPara.m_audio.m_nDenoiseGrade));
                if(gGblPara.m_audio.m_bPreEnhance)
                {
                    jsonObjFeedBackNRAE.insert("PreEnhance","true");
                }else{
                    jsonObjFeedBackNRAE.insert("PreEnhance","false");
                }
                jsonObjFeedBack.insert("NRAEPara",jsonObjFeedBackNRAE);
            }else{
                qDebug()<<"has error";
            }
        }

        if(bWrCfgFileFlag)
        {
            gGblPara.writeCfgFile();
        }
    }else{
        qDebug()<<"<Error>:CtlThread,failed to parse json.";
    }
    QJsonDocument jsonDocFeedBack;
    jsonDocFeedBack.setObject(jsonObjFeedBack);
    QByteArray baFeedBack=jsonDocFeedBack.toJson();
    return baFeedBack;
}
