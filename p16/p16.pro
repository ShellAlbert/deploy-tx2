#QT -= gui

QT += core gui serialport network script
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#CONFIG += c++11 console
#CONFIG -= app_bundle

CONFIG += c++11
TARGET = p16.bin
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
    zgblpara.cpp \
    zaudiocapturethread.cpp \
    zaudioplaythread.cpp \
    zaudiotask.cpp \
    zaudiotxthread.cpp \
    znoisecutthread.cpp \
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
    forward/ztcp2uartthread.cpp
    #libns.c


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
    zaudiocapturethread.h \
    zaudioplaythread.h \
    zaudiotask.h \
    zaudiotxthread.h \
    znoisecutthread.h \
    json/zjsonthread.h \
    forward/ztcp2uartthread.h

LIBS += -lasound -lopus
LIBS += -L$$PWD/../libns -lwebrtc -lrnnoise -lns
