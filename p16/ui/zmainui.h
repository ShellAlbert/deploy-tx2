#ifndef ZMAINUI_H
#define ZMAINUI_H


#include <QTimer>
#include <ui/zimgdispui.h>
#include "zgblpara.h"
class ZAudioTask;
class ZTcp2UartForwardThread;
class ZJsonThread;
class ZVideoTask;
#ifdef BUILD_ZSY_GUI_SUPPORT
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
class ZMainUI : public QWidget
{
    Q_OBJECT
public:
    ZMainUI(QWidget *parent = 0);
    ~ZMainUI();
public:
    qint32 ZDoInit();
    qint32 ZDoClean();

    ZImgDispUI* ZGetDispUI(qint32 index);
private:
    QLabel *m_llTop;//top label.
    ZImgDispUI *m_UILft;//the left main camera.
    ZImgDispUI *m_UIRht;//the right aux camera.
    QHBoxLayout *m_hLayout;
    QVBoxLayout *m_vLayout;


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
#else
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
#endif

#endif // ZMAINUI_H
