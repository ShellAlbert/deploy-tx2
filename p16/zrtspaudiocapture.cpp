#include "zrtspaudiocapture.h"
#include <QDebug>
#include <QFile>

#include <Qt5GStreamer/QGlib/Connect>
#include <Qt5GStreamer/QGlib/Error>
#include <Qt5GStreamer/QGst/Init>
#include <Qt5GStreamer/QGst/Bus>
#include <Qt5GStreamer/QGst/Pipeline>
#include <Qt5GStreamer/QGst/Parse>
#include <Qt5GStreamer/QGst/Message>
#include <Qt5GStreamer/QGst/Utils/ApplicationSink>
#include <Qt5GStreamer/QGst/Utils/ApplicationSource>

ZRtspAudioCapture::ZRtspAudioCapture()
{

}
void ZRtspAudioCapture::run()
{
    QGst::Utils::ApplicationSink *sink;
    QGst::PipelinePtr pipeline;

    //init.
    QGst::init(NULL,NULL);
    //create ApplicatioinSink object.
    sink=new QGst::Utils::ApplicationSink;

    QString pipelineDesc=QString("rtspsrc latency=0 location=\"rtsp://192.168.137.12:554/user=admin&password=&channel=1&stream=0.sdp?real_stream\" ! decodebin ! audioconvert ! appsink name=\"mysink\"");
    pipeline=QGst::Parse::launch(pipelineDesc).dynamicCast<QGst::Pipeline>();
    sink->setElement(pipeline->getElementByName("mysink"));
    sink->enableDrop(false);

    //QGlib::connect(this->m_pipeline->bus(),"message::error",this,&onBusMessage);
    //pipeline->bus()->addSignalWatch();

    QFile filePcm("p16.pcm");
    if(!filePcm.open(QIODevice::WriteOnly))
    {
        qDebug()<<"failed to open p16.pcm";
        return;
    }
    QByteArray ba;
    ba.resize(1024);
    int i=0;
    //start playing.
    pipeline->setState(QGst::StatePlaying);
    while(1)
    {
        QGst::SamplePtr sam=sink->pullSample();
        if(!sam.isNull())
        {
            QGst::BufferPtr buf=sam->buffer();
            quint32 nSize=buf->size();
            if(nSize>0 && (int)nSize<ba.size())
            {
                buf->extract(0,ba.data(),nSize);
                filePcm.write(ba.data(),nSize);
                filePcm.flush();
            }else{
                qDebug()<<"memory leak!";
            }
            qDebug()<<"read pcm:"<<nSize<<",i="<<i++;
        }
        if(sink->isEos())
        {
            break;
        }
    }
    delete sink;
    return;
}
