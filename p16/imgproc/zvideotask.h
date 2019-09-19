#ifndef ZVIDEOTASK_H
#define ZVIDEOTASK_H

#include <QObject>
#include <QTimer>
//#include <QWidget>
#include "zrtspthread.h"
#include "zrtspthread2.h"
#include "zimgprocthread.h"
#include <QQueue>
#include <QByteArray>
#include <QMutex>
#include <QWaitCondition>
#include <opencv2/opencv.hpp>
#define FIFO_DEPTH 30
class ZVideoTask:public QObject
{
    Q_OBJECT
public:
    ZVideoTask(QObject *parent = 0);
    ~ZVideoTask();
    qint32 ZStartTask(QWidget *mainUI);
    bool ZIsCleanup();
private:
    ZRtspThread *m_rtsp1;
    ZRtspThread *m_rtsp2;
    ZImgProcThread *m_imgProc;
    ZRtspThread2 *m_rtsp3;
private:
    QTimer *m_timerExit;
private:
    //rtsp1->queue->imgProc.
    cv::Mat* m_rtsp2imgProc1[FIFO_DEPTH];
    QQueue<cv::Mat*> m_rtsp2imgProcFree1;
    QQueue<cv::Mat*> m_rtsp2imgProcUsed1;
    QMutex m_rtsp2imgProcMux1;
    QWaitCondition m_condRtsp2imgProcNotEmpty1;
    QWaitCondition m_condRtsp2imgProcNotFull1;

    //rtsp2->queue->imgProc.
    cv::Mat* m_rtsp2imgProc2[FIFO_DEPTH];
    QQueue<cv::Mat*> m_rtsp2imgProcFree2;
    QQueue<cv::Mat*> m_rtsp2imgProcUsed2;
    QMutex m_rtsp2imgProcMux2;
    QWaitCondition m_condRtsp2imgProcNotEmpty2;
    QWaitCondition m_condRtsp2imgProcNotFull2;
};

#endif // ZVIDEOTASK_H
