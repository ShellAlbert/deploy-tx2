#ifndef ZMAINUI_H
#define ZMAINUI_H


#include <QTimer>
#include "zgblpara.h"
class ZAudioTask;
class ZTcp2UartForwardThread;
class ZJsonThread;
class ZVideoTask;

class ZMainObj : public QObject
{
    Q_OBJECT
public:
    ZMainObj(QObject *parent = 0);
    ~ZMainObj();

    void showMaximized();
public:
    qint32 ZDoInit();
    qint32 ZDoClean();
public:
    qint32 ZManageThreads(ZAudioTask *audio,ZTcp2UartForwardThread *tcp2uart,ZJsonThread *json,ZVideoTask *video);
private slots:
    void ZSlotHelp2Exit();
private:
    QTimer *m_timerExit;
    ZAudioTask *m_audio;
    ZTcp2UartForwardThread *m_tcp2uart;
    ZJsonThread *m_json;
    ZVideoTask *m_video;
};

#endif // ZMAINUI_H
