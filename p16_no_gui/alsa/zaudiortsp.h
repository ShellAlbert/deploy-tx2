#ifndef ZAUDIORTSP_H
#define ZAUDIORTSP_H

#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
class ZAudioRtsp : public QThread
{
    Q_OBJECT
public:
    ZAudioRtsp();
    ~ZAudioRtsp();

    bool ZIsCleanup();
protected:
    void run();
private:
    bool m_bCleanup;

    //capture to noise queue(fifo).
    QByteArray* m_Cap2NsFIFO[5];
    QQueue<QByteArray*> m_Cap2NsFIFOFree;
    QQueue<QByteArray*> m_Cap2NsFIFOUsed;
    QMutex m_Cap2NsFIFOMutex;
    QWaitCondition m_condCap2NsFIFOFull;
    QWaitCondition m_condCap2NsFIFOEmpty;
};

#endif // ZAUDIORTSP_H
