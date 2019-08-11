#ifndef ZRTSPAUDIOCAPTURE_H
#define ZRTSPAUDIOCAPTURE_H

#include <QThread>
class ZRtspAudioCapture : public QThread
{
    Q_OBJECT
public:
    ZRtspAudioCapture();
protected:
    void run();
private:
};

#endif // ZRTSPAUDIOCAPTURE_H
