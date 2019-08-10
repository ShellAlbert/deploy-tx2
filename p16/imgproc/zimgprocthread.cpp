#include "zimgprocthread.h"
#include <zgblpara.h>
#include <string.h>
#include <QDebug>
ZImgProcThread::ZImgProcThread()
{

}
qint32 ZImgProcThread::ZBindQueue1(QMutex *mutex,///<
                                   QWaitCondition *condNotEmpty,QWaitCondition *condNotFull,///<
                                   QQueue<cv::Mat*> *queueFree,QQueue<cv::Mat*> *queueUsed)
{
    this->m_mutex1=mutex;
    this->m_condNotEmpty1=condNotEmpty;
    this->m_condNotFull1=condNotFull;
    this->m_queueFree1=queueFree;
    this->m_queueUsed1=queueUsed;

    return 0;
}
qint32 ZImgProcThread::ZBindQueue2(QMutex *mutex,///<
                                   QWaitCondition *condNotEmpty,QWaitCondition *condNotFull,///<
                                   QQueue<cv::Mat*> *queueFree,QQueue<cv::Mat*> *queueUsed)
{
    this->m_mutex2=mutex;
    this->m_condNotEmpty2=condNotEmpty;
    this->m_condNotFull2=condNotFull;
    this->m_queueFree2=queueFree;
    this->m_queueUsed2=queueUsed;

    return 0;
}

void ZImgProcThread::run()
{
    cv::Mat *mat1=new cv::Mat(1080,1920,CV_8UC1);
    cv::Mat *mat2=new cv::Mat(1080,1920,CV_8UC1);
    while(!gGblPara.m_bGblRst2Exit)
    {

        bool bProcNextTime=false;
        //1.fetch image from queue1.
        this->m_mutex1->lock();
        while(this->m_queueUsed1->size()<=0)
        {
            if(!this->m_condNotEmpty1->wait(this->m_mutex1,5000))
            {
                this->m_mutex1->unlock();
                bProcNextTime=true;
                break;
            }
        }
        if(bProcNextTime)
        {
            continue;
        }
        cv::Mat *matBase1=this->m_queueUsed1->dequeue();
        memcpy(mat1->data,matBase1->data,matBase1->cols*matBase1->rows*matBase1->channels());
        this->m_queueFree1->enqueue(matBase1);
        this->m_condNotFull1->wakeAll();
        this->m_mutex1->unlock();


        //2.fetch image from queue2.
        this->m_mutex2->lock();
        while(this->m_queueUsed2->size()<=0)
        {
            if(!this->m_condNotEmpty2->wait(this->m_mutex2,5000))
            {
                this->m_mutex2->unlock();
                break;
            }
        }
        cv::Mat *matBase2=this->m_queueUsed2->dequeue();
        memcpy(mat2->data,matBase2->data,matBase2->cols*matBase2->rows*matBase2->channels());
        this->m_queueFree2->enqueue(matBase2);
        this->m_condNotFull2->wakeAll();
        this->m_mutex2->unlock();

        //3.do image algorithm.
        //4.output result.
        qDebug()<<"imgproc fetch 2 images okay.";
    }
}
