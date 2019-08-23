#ifndef ZAUDIORTSP_H
#define ZAUDIORTSP_H

#include <QThread>
class ZAudioRtsp : public QThread
{
    Q_OBJECT
public:
    ZAudioRtsp();

protected:
    void run();

private:
};

#endif // ZAUDIORTSP_H
