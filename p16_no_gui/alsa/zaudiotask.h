#ifndef ZAUDIOTASK_H
#define ZAUDIOTASK_H
#include "zgblpara.h"
#include "zaudiocapturethread.h"
#include "znoisecutthread.h"
#include "zaudioplaythread.h"
#include "zaudiotxthread.h"
#include <QTimer>

/**
 *
zhangshaoyan@YNTP16:~/p16$ aplay -l
List of PLAYBACK Hardware Devices ****
card 0: tegrahda [tegra-hda], device 3: HDMI 0 [HDMI 0]
  Subdevices: 1/1
  Subdevice #0: subdevice #0
card 0: tegrahda [tegra-hda], device 7: HDMI 0 [HDMI 0]
  Subdevices: 1/1
  Subdevice #0: subdevice #0
card 1: Device [USB Audio Device], device 0: USB Audio [USB Audio]
  Subdevices: 0/1
  Subdevice #0: subdevice #0
zhangshaoyan@YNTP16:~/p16$

zhangshaoyan@YNTP16:~/p16$ amixer -c 1 controls
numid=3,iface=MIXER,name='Mic Playback Switch'
numid=4,iface=MIXER,name='Mic Playback Volume'
numid=7,iface=MIXER,name='Mic Capture Switch'
numid=8,iface=MIXER,name='Mic Capture Volume'
numid=9,iface=MIXER,name='Auto Gain Control'
numid=5,iface=MIXER,name='Speaker Playback Switch'
numid=6,iface=MIXER,name='Speaker Playback Volume'
numid=2,iface=PCM,name='Capture Channel Map'
numid=1,iface=PCM,name='Playback Channel Map'
zhangshaoyan@YNTP16:~/p16$



zhangshaoyan@YNTP16:~/p16$ amixer -c 1 cset numid=6,iface=MIXER,name='Speaker Playback Volume' 10
numid=6,iface=MIXER,name='Speaker Playback Volume'
  ; type=INTEGER,access=rw---R--,values=2,min=0,max=30,step=0
  : values=10,10
  | dBminmax-min=-45.00dB,max=0.00dB

zhangshaoyan@YNTP16:~/p16$ amixer -c 1 cget numid=8,iface=MIXER,name='Mic Capture Volume'
numid=8,iface=MIXER,name='Mic Capture Volume'
  ; type=INTEGER,access=rw---R--,values=1,min=0,max=30,step=0
  : values=12
  | dBminmax-min=-12.00dB,max=33.00dB
zhangshaoyan@YNTP16:~/p16$


zhangshaoyan@YNTP16:~/p16$ amixer -c 1 cset numid=8,iface=MIXER,name='Mic Capture Volume' 20
numid=8,iface=MIXER,name='Mic Capture Volume'
  ; type=INTEGER,access=rw---R--,values=1,min=0,max=30,step=0
  : values=20
  | dBminmax-min=-12.00dB,max=33.00dB

zhangshaoyan@YNTP16:~/p16$ amixer -c 1 cset numid=9,iface=MIXER,name='Auto Gain Control' 1
numid=9,iface=MIXER,name='Auto Gain Control'
  ; type=BOOLEAN,access=rw------,values=1
  : values=on
zhangshaoyan@YNTP16:~/p16$
 *
 */
class ZAudioTask : public QObject
{
    Q_OBJECT
public:
     ZAudioTask(QObject *parent = 0);
    ~ZAudioTask();
     qint32 ZStartTask();
     bool ZIsCleanup();

private slots:
    void ZSlotHelp2Exit();
    void ZSlotTimeout();
private:
    //Audio Capture --noise queue-->  Noise Cut --play queue--> Local Play.
    //                                          -- tx queue --> Tcp Tx.
    ZAudioCaptureThread *m_capThread;
    ZNoiseCutThread *m_cutThread;
    ZAudioPlayThread *m_playThread;
    ZAudioTxThread *m_txThread;
private:
    QTimer *m_timerExit;
private:
    //capture to noise queue(fifo).
    QByteArray* m_Cap2NsFIFO[5];
    QQueue<QByteArray*> m_Cap2NsFIFOFree;
    QQueue<QByteArray*> m_Cap2NsFIFOUsed;
    QMutex m_Cap2NsFIFOMutex;
    QWaitCondition m_condCap2NsFIFOFull;
    QWaitCondition m_condCap2NsFIFOEmpty;
    //noise to playback fifo.
    QByteArray* m_Ns2PbFIFO[5];
    QQueue<QByteArray*> m_Ns2PbFIFOFree;
    QQueue<QByteArray*> m_Ns2PbFIFOUsed;
    QMutex m_Ns2PbFIFOMutex;
    QWaitCondition m_condNs2PbFIFOFull;
    QWaitCondition m_condNs2PbFIFOEmpty;
    //noise to tx fifo.
    QByteArray* m_Ns2TxFIFO[5];
    QQueue<QByteArray*> m_Ns2TxFIFOFree;
    QQueue<QByteArray*> m_Ns2TxFIFOUsed;
    QMutex m_Ns2TxFIFOMutex;
    QWaitCondition m_condNs2TxFIFOFull;
    QWaitCondition m_condNs2TxFIFOEmpty;
};

#endif // ZAUDIOTASK_H
