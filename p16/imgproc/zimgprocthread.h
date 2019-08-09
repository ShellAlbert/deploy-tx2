#ifndef ZIMGPROCTHREAD_H
#define ZIMGPROCTHREAD_H

#include <QThread>
#include <QMutex>
#include <QQueue>
#include <QWaitCondition>
#include <opencv2/opencv.hpp>
class ZImgProcThread : public QThread
{
    Q_OBJECT
public:
    ZImgProcThread();

    qint32 ZBindQueue1(QMutex *mutex,///<
                     QWaitCondition *condNotEmpty,QWaitCondition *condNotFull,///<
                     QQueue<cv::Mat*> *queueFree,QQueue<cv::Mat*> *queueUsed);
    qint32 ZBindQueue2(QMutex *mutex,///<
                     QWaitCondition *condNotEmpty,QWaitCondition *condNotFull,///<
                     QQueue<cv::Mat*> *queueFree,QQueue<cv::Mat*> *queueUsed);
protected:
    void run();
private:
    //FIFO1.
    QMutex *m_mutex1;
    QWaitCondition *m_condNotEmpty1;
    QWaitCondition *m_condNotFull1;
    QQueue<cv::Mat*> *m_queueFree1;
    QQueue<cv::Mat*> *m_queueUsed1;
    //FIFO2.
    QMutex *m_mutex2;
    QWaitCondition *m_condNotEmpty2;
    QWaitCondition *m_condNotFull2;
    QQueue<cv::Mat*> *m_queueFree2;
    QQueue<cv::Mat*> *m_queueUsed2;
};

#endif // ZIMGPROCTHREAD_H
