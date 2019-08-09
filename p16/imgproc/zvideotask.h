#ifndef ZVIDEOTASK_H
#define ZVIDEOTASK_H

#include <QObject>
#include <QTimer>
#include <QWidget>
#include "zrtspthread.h"
#include "zimgprocthread.h"
class ZVideoTask:public QObject
{
    Q_OBJECT
public:
    ZVideoTask(QObject *parent = 0);

    qint32 ZStartTask(QWidget *mainUI);
    bool ZIsCleanup();
private:
    ZRtspThread *m_rtsp1;
    ZRtspThread *m_rtsp2;
private:
    QTimer *m_timerExit;
};

#endif // ZVIDEOTASK_H
