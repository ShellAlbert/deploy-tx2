#ifndef ZAUDIORTSP_H
#define ZAUDIORTSP_H

#include <QThread>
class ZAudioRtsp : public QThread
{
    Q_OBJECT
public:
    ZAudioRtsp();

    bool ZIsCleanup();
protected:
    void run();
private:
    bool m_bCleanup;
};

#endif // ZAUDIORTSP_H
