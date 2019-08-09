#ifndef ZRTSPTHREAD_H
#define ZRTSPTHREAD_H

#include <QThread>
#include <QImage>
#include <QQueue>
#include <QByteArray>
#include <QMutex>
#include <QWaitCondition>
extern "C"
{
#include <gst/gst.h>
#include <glib.h>
}
#include "opencv2/opencv.hpp"
class ZRtspThread : public QThread
{
    Q_OBJECT
public:
    ZRtspThread(QString rtspAddr);
    qint32 ZBindQueue(QMutex *mutex,///<
                     QWaitCondition *condNotEmpty,QWaitCondition *condNotFull,///<
                     QQueue<cv::Mat*> *queueFree,QQueue<cv::Mat*> *queueUsed);
protected:
    void run();
signals:
    void ZSigConnected();
    void ZSigConnectionLost();
    void ZSigNewImg(const QImage &img);

private:
    QString m_rtspAddr;

private:
    QMutex *m_mutex;
    QWaitCondition *m_condNotEmpty;
    QWaitCondition *m_condNotFull;
    QQueue<cv::Mat*> *m_queueFree;
    QQueue<cv::Mat*> *m_queueUsed;
};

#endif // ZRTSPTHREAD_H
