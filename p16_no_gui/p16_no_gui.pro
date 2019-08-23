
QT += core serialport network script


#CONFIG += c++11
#CONFIG -= app_bundle

CONFIG += c++11 console
TARGET = p16_no_gui.bin
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# disable qDebug() output in release mode.
#DEFINES += QT_NO_DEBUG_OUTPUT

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
    zgblpara.cpp \
    alsa/zaudiocapturethread.cpp \
    alsa/zaudioplaythread.cpp \
    alsa/zaudiotask.cpp \
    alsa/zaudiotxthread.cpp \
    alsa/znoisecutthread.cpp \
    webrtc/analog_agc.c \
    webrtc/complex_bit_reverse.c \
    webrtc/complex_fft.c \
    webrtc/copy_set_operations.c \
    webrtc/cross_correlation.c \
    webrtc/digital_agc.c \
    webrtc/division_operations.c \
    webrtc/dot_product_with_scale.c \
    webrtc/downsample_fast.c \
    webrtc/energy.c \
    webrtc/fft4g.c \
    webrtc/get_scaling_square.c \
    webrtc/min_max_operations.c \
    webrtc/noise_suppression.c \
    webrtc/noise_suppression_x.c \
    webrtc/ns_core.c \
    webrtc/nsx_core.c \
    webrtc/nsx_core_c.c \
    webrtc/nsx_core_neon_offsets.c \
    webrtc/real_fft.c \
    webrtc/resample.c \
    webrtc/resample_48khz.c \
    webrtc/resample_by_2.c \
    webrtc/resample_by_2_internal.c \
    webrtc/resample_by_2_mips.c \
    webrtc/resample_fractional.c \
    webrtc/ring_buffer.c \
    webrtc/spl_init.c \
    webrtc/spl_sqrt.c \
    webrtc/spl_sqrt_floor.c \
    webrtc/splitting_filter.c \
    webrtc/vector_scaling_operations.c \
    json/zjsonthread.cpp \
    forward/ztcp2uartthread.cpp \
    #imgproc/zimgprocthread.cpp \
    #imgproc/zrtspthread.cpp \
    #ui/zimgdispui.cpp \
    ui/zmainui.cpp \
    #imgproc/zvideotask.cpp \
    alsa/zaudiortsp.cpp \
    rtsp/base/AsyncLogging.cpp \
    rtsp/base/Condition.cpp \
    rtsp/base/Logging.cpp \
    rtsp/base/Mutex.cpp \
    rtsp/base/Sem.cpp \
    rtsp/base/Thread.cpp \
    rtsp/base/ThreadPool.cpp \
    rtsp/extend/alsa/AlsaMediaSource.cpp \
    rtsp/extend/v4l2/V4l2.cpp \
    rtsp/extend/v4l2/V4l2MediaSource.cpp \
    rtsp/net/poller/EPollPoller.cpp \
    rtsp/net/poller/Poller.cpp \
    rtsp/net/poller/PollPoller.cpp \
    rtsp/net/poller/SelectPoller.cpp \
    rtsp/net/AACFileMediaSource.cpp \
    rtsp/net/AACRtpSink.cpp \
    rtsp/net/Acceptor.cpp \
    rtsp/net/Buffer.cpp \
    rtsp/net/Event.cpp \
    rtsp/net/EventScheduler.cpp \
    rtsp/net/H264FileMediaSource.cpp \
    rtsp/net/H264RtpSink.cpp \
    rtsp/net/InetAddress.cpp \
    rtsp/net/MediaSession.cpp \
    rtsp/net/MediaSource.cpp \
    rtsp/net/RtpSink.cpp \
    rtsp/net/RtspConnection.cpp \
    rtsp/net/RtspServer.cpp \
    rtsp/net/SocketsOps.cpp \
    rtsp/net/TcpConnection.cpp \
    rtsp/net/TcpServer.cpp \
    rtsp/net/TcpSocket.cpp \
    rtsp/net/Timer.cpp \
    rtsp/net/UsageEnvironment.cpp

HEADERS += \
    webrtc/analog_agc.h \
    webrtc/complex_fft_tables.h \
    webrtc/cpu_features_wrapper.h \
    webrtc/defines.h \
    webrtc/digital_agc.h \
    webrtc/fft4g.h \
    webrtc/gain_control.h \
    webrtc/noise_suppression.h \
    webrtc/noise_suppression_x.h \
    webrtc/ns_core.h \
    webrtc/nsx_core.h \
    webrtc/nsx_defines.h \
    webrtc/real_fft.h \
    webrtc/resample_by_2_internal.h \
    webrtc/ring_buffer.h \
    webrtc/signal_processing_library.h \
    webrtc/spl_inl.h \
    webrtc/typedefs.h \
    webrtc/windows_private.h \
    zgblpara.h \
    alsa/zaudiocapturethread.h \
    alsa/zaudioplaythread.h \
    alsa/zaudiotask.h \
    alsa/zaudiotxthread.h \
    alsa/znoisecutthread.h \
    json/zjsonthread.h \
    forward/ztcp2uartthread.h \
    #imgproc/zimgprocthread.h \
    #imgproc/zrtspthread.h \
    #ui/zimgdispui.h \
    ui/zmainui.h \
    #imgproc/zvideotask.h \
    alsa/zaudiortsp.h \
    rtsp/base/AsyncLogging.h \
    rtsp/base/Condition.h \
    rtsp/base/Logging.h \
    rtsp/base/Mutex.h \
    rtsp/base/Sem.h \
    rtsp/base/Thread.h \
    rtsp/base/ThreadPool.h \
    rtsp/extend/alsa/AlsaMediaSource.h \
    rtsp/extend/v4l2/V4l2.h \
    rtsp/extend/v4l2/V4l2MediaSource.h \
    rtsp/net/poller/EPollPoller.h \
    rtsp/net/poller/Poller.h \
    rtsp/net/poller/PollPoller.h \
    rtsp/net/poller/SelectPoller.h \
    rtsp/net/AACFileMediaSource.h \
    rtsp/net/AACRtpSink.h \
    rtsp/net/Acceptor.h \
    rtsp/net/Buffer.h \
    rtsp/net/Event.h \
    rtsp/net/EventScheduler.h \
    rtsp/net/H264FileMediaSource.h \
    rtsp/net/H264RtpSink.h \
    rtsp/net/InetAddress.h \
    rtsp/net/MediaSession.h \
    rtsp/net/MediaSource.h \
    rtsp/net/Rtp.h \
    rtsp/net/RtpInstance.h \
    rtsp/net/RtpSink.h \
    rtsp/net/RtspConnection.h \
    rtsp/net/RtspServer.h \
    rtsp/net/SocketsOps.h \
    rtsp/net/TcpConnection.h \
    rtsp/net/TcpServer.h \
    rtsp/net/TcpSocket.h \
    rtsp/net/Timer.h \
    rtsp/net/UsageEnvironment.h

INCLUDEPATH += /usr/include/gstreamer-1.0
INCLUDEPATH += /usr/include/glib-2.0
INCLUDEPATH += /usr/lib/aarch64-linux-gnu/glib-2.0/include

#Qt5QGstreamer
#INCLUDEPATH += /usr/include/Qt5GStreamer
#INCLUDEPATH += /usr/include/orc-0.4
#INCLUDEPATH += /usr/include/aarch64-linux-gnu/qt5/QtCore
#INCLUDEPATH += /usr/include/aarch64-linux-gnu/qt5

#

#alsa & opus.
LIBS += -lasound -lopus
#noise suppression.
#LIBS += -L$$PWD/../libns -lwebrtc -lrnnoise -lns
#gstreamer.
#LIBS += -lgstreamer-1.0 -lgobject-2.0 -lglib-2.0 -pthread

#rtsp.
LIBS += -lfaac -lx264

#openCV.
LIBS += -lopencv_dnn -lopencv_ml -lopencv_objdetect -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_imgproc -lopencv_flann -lopencv_core


#LIBS += -lQt5GStreamerUtils-1.0 -lQt5GStreamer-1.0 -lQt5GLib-2.0
#-lQt5Core
