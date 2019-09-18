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

//XiongMai IP Camera must be set to Black/White mode before running this thread.
//1920*1080*15fps,gray.
void ZRtspThread::run()
{
    //CAUTION HERE!!!
    // videoconvert is more slower than nvvidconv.
    //    QString rtspAddr=QString("rtspsrc latency=0 location=\"rtsp://%1:554/user=admin&password=&channel=1&stream=0.sdp\"").arg(this->m_rtspAddr);
    //    rtspAddr.append(" ! rtph264depay ! h264parse ! omxh264dec ! nvvidconv ! video/x-raw,width=1920,height=1080,format=BGRx ! videoconvert ! appsink sync=false");
    //    qDebug()<<rtspAddr;
    QString rtspAddr=QString("rtspsrc latency=0 location=rtsp://%1:554/user=admin&password=&channel=1&stream=0.sdp").arg(this->m_rtspAddr);
    //rtspAddr.append(QString(" ! rtph264depay ! h264parse ! omxh264dec ! nvvidconv ! video/x-raw,format=(string)GRAY8,width=%1,height=%2,framerate=%3 ! appsink sync=false").arg(RTSP_H264_WIDTH).arg(RTSP_H264_HEIGHT).arg(RTSP_H264_FPS));
    rtspAddr.append(QString(" ! rtph264depay ! h264parse ! omxh264dec ! nvvidconv ! appsink sync=false"));
    qDebug()<<rtspAddr;
    //std::string gstreamer_pipe="rtspsrc location=\"rtsp://192.168.137.10:554/user=admin&password=&channel=1&stream=0.sdp\" ! rtph264depay ! h264parse ! omxh264dec ! nvvidconv ! video/x-raw,width=1920,height=1080,format=BGRx ! videoconvert ! appsink";
    //std::string gstreamer_pipe="rtspsrc location=\"rtsp://192.168.1.89:554/user=admin&password=&channel=1&stream=0.sdp\" ! rtph264depay ! h264parse ! omxh264dec ! nvvidconv ! video/x-raw,width=1920,height=1080,format=BGRx ! videoconvert ! appsink";

    qDebug()<<"rtsp thread"<<this->m_rtspAddr<<"start.";
    qDebug()<<"("<<RTSP_H264_WIDTH<<"*"<<RTSP_H264_HEIGHT<<")-->>("<<ImgResizedWidth<<"*"<<ImgResizedHeight<<")";
    while(!gGblPara.m_bGblRst2Exit)
    {
        if(gGblPara.m_bJsonImgProOn)
        {
            //1.open rtsp connection.
            //cv::VideoCapture cap(rtspAddr.toStdString(),cv::CAP_GSTREAMER);
            cv::VideoCapture cap(rtspAddr.toStdString(),cv::CAP_GSTREAMER);
            if(!cap.isOpened())
            {
                cap.release();
                emit this->ZSigConnectionLost();
                qCritical()<<"failed to open rtsp connection"<<this->m_rtspAddr<<".";
                qCritical()<<"retry after 3 seconds!";
                this->sleep(3);
                continue;
            }
            emit this->ZSigConnected();

            //2.loop to read image.
            cv::Mat mat,matResize;
            while(!gGblPara.m_bGblRst2Exit)
            {
                if(!cap.read(mat))
                {
                    emit this->ZSigConnectionLost();
                    qCritical()<<"failed to read img from rtsp"<<this->m_rtspAddr<<".";
                    qCritical()<<"try to reconnect after 3 seconds!";
                    this->sleep(3);
                    break;
                }
                //qDebug()<<this->m_rtspAddr<<"read img okay"<<mat.cols<<mat.rows<<mat.depth()<<mat.channels();
                //black-white:gray: so channels()=1.
                //RGB: channels()=3.
                //qDebug()<<mat.cols<<mat.rows<<","<<mat.channels();

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

                //here,we resize image size to reduce imgproc time.
                //(RTSP_H264_WIDTH,RTSP_H264_HEIGHT) --> (ImgResizedWidth,ImgResizedHeight)
                cv::resize(mat,matResize,cv::Size(ImgResizedWidth,ImgResizedHeight));

                cv::Mat *matDest=this->m_queueFree->dequeue();
                memcpy(matDest->data,matResize.data,matResize.cols*matResize.rows*matResize.channels());
                this->m_queueUsed->enqueue(matDest);
                this->m_condNotEmpty->wakeAll();
                this->m_mutex->unlock();
                //qDebug()<<this->m_rtspAddr<<":"<<"push one frame";

                //if json protocol turn off imgproc,then stop capturing.
                if(!gGblPara.m_bJsonImgProOn)
                {
                    break;
                }
            }
            cap.release();
        }else{
            //100ms.
            this->usleep(1000*100);
        }
    }
    this->m_bCleanup=true;
    qDebug()<<"rtsp thread"<<this->m_rtspAddr<<"done.";
    return;
}
bool ZRtspThread::ZIsCleanup()
{
    return this->m_bCleanup;
}
