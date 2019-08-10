#include "zrtspthread.h"
#include <zgblpara.h>

#include <QDebug>
ZRtspThread::ZRtspThread(QString rtspAddr)
{
    this->m_rtspAddr=rtspAddr;
    this->m_bCleanup=false;
}
qint32 ZRtspThread::ZBindQueue(QMutex *mutex,///<
                 QWaitCondition *condNotEmpty,QWaitCondition *condNotFull,///<
                 QQueue<cv::Mat*> *queueFree,QQueue<cv::Mat*> *queueUsed)
{
    this->m_mutex=mutex;
    this->m_condNotEmpty=condNotEmpty;
    this->m_condNotFull=condNotFull;
    this->m_queueFree=queueFree;
    this->m_queueUsed=queueUsed;

    return 0;
}
void ZRtspThread::run()
{
    //CAUTION HERE!!!
    // videoconvert is more slower than nvvidconv.
//    QString rtspAddr=QString("rtspsrc latency=0 location=\"rtsp://%1:554/user=admin&password=&channel=1&stream=0.sdp\"").arg(this->m_rtspAddr);
//    rtspAddr.append(" ! rtph264depay ! h264parse ! omxh264dec ! nvvidconv ! video/x-raw,width=1920,height=1080,format=BGRx ! videoconvert ! appsink sync=false");
//    qDebug()<<rtspAddr;
    QString rtspAddr=QString("rtspsrc latency=0 location=rtsp://%1:554/user=admin&password=&channel=1&stream=0.sdp").arg(this->m_rtspAddr);
    rtspAddr.append(" ! rtph264depay ! h264parse ! omxh264dec ! nvvidconv ! appsink sync=false");
    qDebug()<<rtspAddr;
    //std::string gstreamer_pipe="rtspsrc location=\"rtsp://192.168.137.10:554/user=admin&password=&channel=1&stream=0.sdp\" ! rtph264depay ! h264parse ! omxh264dec ! nvvidconv ! video/x-raw,width=1920,height=1080,format=BGRx ! videoconvert ! appsink";
    //std::string gstreamer_pipe="rtspsrc location=\"rtsp://192.168.1.89:554/user=admin&password=&channel=1&stream=0.sdp\" ! rtph264depay ! h264parse ! omxh264dec ! nvvidconv ! video/x-raw,width=1920,height=1080,format=BGRx ! videoconvert ! appsink";

    while(!gGblPara.m_bGblRst2Exit)
    {
        //1.open rtsp connection.
        //cv::VideoCapture cap(rtspAddr.toStdString(),cv::CAP_GSTREAMER);
        cv::VideoCapture cap(rtspAddr.toStdString());
        if(!cap.isOpened())
        {
            cap.release();
            emit this->ZSigConnectionLost();
            qDebug()<<"<error>:failed to open rtsp!";
            qDebug()<<"<info>:retry after 3 seconds!";
            this->sleep(3);
            continue;
        }
        //qDebug()<<"open rtsp okay";
        emit this->ZSigConnected();

        //2.loop to read image.
        cv::Mat mat;
        while(!gGblPara.m_bGblRst2Exit)
        {
            if(!cap.read(mat))
            {
                emit this->ZSigConnectionLost();
                qDebug()<<"<error>:failed to read img!";
                qDebug()<<"<info>:try to reconnect after 3 seconds!";
                this->sleep(3);
                break;
            }
            //qDebug()<<this->m_rtspAddr<<"read img okay"<<mat.cols<<mat.rows<<mat.depth()<<mat.channels();
            //black-white:gray: so channels()=1.
            //RGB: channels()=3.
            qDebug()<<mat.cols<<mat.rows<<","<<mat.channels();

            //4.add image to fifo.
            //4.1 fetch a free buffer in freeQueue.
            this->m_mutex->lock();
            while(this->m_queueFree->isEmpty())
            {
                if(!this->m_condNotFull->wait(this->m_mutex,5000))
                {
                    this->m_mutex->unlock();
                    if(gGblPara.m_bGblRst2Exit)
                    {
                        break;
                    }
                }
            }
            if(gGblPara.m_bGblRst2Exit)
            {
                break;
            }
            cv::Mat *matDest=this->m_queueFree->dequeue();
            memcpy(matDest->data,mat.data,mat.cols*mat.rows*mat.channels());
            this->m_queueUsed->enqueue(matDest);
            this->m_condNotEmpty->wakeAll();
            this->m_mutex->unlock();
            //qDebug()<<this->m_rtspAddr<<":"<<"push one frame";

            //convert cv::Mat to QImage.
            //QImage img=cvMat2QImage(mat);
            //emit this->ZSigNewImg(img);
        }
        cap.release();
    }
    this->m_bCleanup=true;
    return;
}
bool ZRtspThread::ZIsCleanup()
{
    return this->m_bCleanup;
}
