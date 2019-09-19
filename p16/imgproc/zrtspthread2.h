#ifndef ZRTSPTHREAD2_H
#define ZRTSPTHREAD2_H

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
class ZRtspThread2 : public QThread
{
    Q_OBJECT
public:
    ZRtspThread2(QString rtspAddr);
    bool ZIsCleanup();
protected:
    void run();
signals:
    void ZSigConnected();
    void ZSigConnectionLost();
    void ZSigNewImg(const QImage &img);
private:
    bool m_bCleanup;
private:
    QString m_rtspAddr;
};

#endif // ZRTSPTHREAD2_H
