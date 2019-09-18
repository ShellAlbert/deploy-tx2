#ifndef ZMAINUI_H
#define ZMAINUI_H


#include <QTimer>
#include <ui/zimgdispui.h>
#include "zgblpara.h"
class ZAudioTask;
class ZTcp2UartForwardThread;
class ZJsonThread;
class ZVideoTask;

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QCloseEvent>
#include <QPushButton>
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
protected:
    void closeEvent(QCloseEvent *event);
private:
    QLabel *m_llTop;//top label.
    ZImgDispUI *m_UILft;//the left main camera.
    ZImgDispUI *m_UIRht;//the right aux camera.
    QHBoxLayout *m_hLayout;

    QPushButton *m_btnImgProcOn;
    QPushButton *m_btnTM;
    QPushButton *m_btnCSK;
    QPushButton *m_btnKCF;
    QPushButton *m_btnTrackOff;
    QPushButton *m_btnExit;
    QHBoxLayout *m_hLayoutBtn;
    QVBoxLayout *m_vLayout;


public:
    qint32 ZManageThreads(ZAudioTask *audio,ZTcp2UartForwardThread *tcp2uart,ZJsonThread *json,ZVideoTask *video);
private slots:
    void ZSlotHelp2Exit();
    void ZSlotImgProcOn();
    void ZSlotTemplateMatch();
    void ZSlotTrackCSK();
    void ZSlotTrackKCF();
    void ZSlotTrackOff();
    void ZSlotExit();
private:
    QTimer *m_timerExit;
    ZAudioTask *m_audio;
    ZTcp2UartForwardThread *m_tcp2uart;
    ZJsonThread *m_json;
    ZVideoTask *m_video;
};

#endif // ZMAINUI_H
