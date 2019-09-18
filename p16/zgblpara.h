#ifndef ZGBLPARA_H
#define ZGBLPARA_H

#include <QString>
#include <QQueue>
#include <QSemaphore>
#include <QMutex>
#include <QMap>
#include <QVector>


//build GUI support or not.
//#define BUILD_ZSY_GUI_SUPPORT    1
//build ImgProc support.
#define BUILD_ZSY_IMGPROC   1


//#define APP_VERSION "0.1.0" //2018/07/10.
#define APP_VERSION "0.2.0" //2019/1/7.

//1920*1080,15fps,gray.
#define RTSP_H264_WIDTH 1920
#define RTSP_H264_HEIGHT 1080
#define RTSP_H264_FPS   15

//we resize the captured image to reduce imgproc time.
//resize range:0.1~1.0
#define gImgResizeRatioW (0.3)
#define gImgResizeRatioH (0.3)
#define ImgResizedWidth  (RTSP_H264_WIDTH*gImgResizeRatioW)
#define ImgResizedHeight (RTSP_H264_HEIGHT*gImgResizeRatioH)

//imgproc algorithms.
enum ImgProcAlgorithms
{
    IMGPROC_BYPASS=0,//bypass imgproc,do nothing.
    OPENCV_TEMPLATE_MATCH,
    OPENCV_CSK_TRACKER,
    OPENCV_KCF_TRACKER,
};

//port define.
#define TCP_PORT_AUDIO  6801 //传输opus音频
#define TCP_PORT_CTL    6802 //传输音频算法启停控制
#define TCP_PORT_VIDEO  6803 //传输h264视频
#define TCP_PORT_FORWARD   6804 //串口透传，用于Android手工调节电机，测距，设置参数等.
#define TCP_PORT_VIDEO2  6805 //传输h264视频

#define DRW_IMG_SIZE_W  500
#define DRW_IMG_SIZE_H  500

//use usleep() to reduce cpu heavy load.
#define AUDIO_THREAD_SCHEDULE_US    (1000*10) //10ms.
#define VIDEO_THREAD_SCHEDULE_US    (1000*10) //10ms.

//socket tx timeout.
#define SOCKET_TX_TIMEOUT   (5000) //5000ms.
/////////////////////////////////////////////////////////////////////////////

//当声卡工作时，数据总是连续地在硬件缓冲区与应用程序之间传输。
//在录音时，如果应用程序读取数据不够快，将导致缓冲区旧数据被新数据覆盖，这种数据丢失叫overrun.
//在回放时，如果应用程序填充硬件缓冲区速度不够快，将导致缓冲区被饿死，这种现在叫underrun.

//* The sample rate of the audio codec **
#define     SAMPLE_RATE      48000

//frame帧是播放样本的一个计量单位，由通道数和比特数决定。
//立体声48KHz 16-bit的PCM，那么一帧的大小就是4字节(2 Channels*16-bit=32bit/8bit=4 bytes)
//5.1通道48KHz 16-bit的PCM，那么一帧的大小就是12字节(5.1这里取6,6Channels*16bit=96bit/8bit=12 bytes)
#define CHANNELS_NUM    2
#define BYTES_PER_FRAME 4

//period:周期，是指每两个硬件中断之间的帧数，poll会在每个周期返回一次.
//alsa将内部的缓冲区拆分成period(周期）又叫fragments（片断）
//alsa以period为单元来传送数据，一个period存储一些frames,一个frames中包含了左右声道的数据采样点.
//硬件的buffer被拆分成period，一个period包含多个frame，一个frame包含多个数据采样点.
//#define ALSA_PERIOD     20//10//4
////////////1秒中断4次会出现卡顿很严重的情况。
////////////修改为1秒中断10次，现象好多了。

//立体声，16-bit采样位数，44.1KHz采样率
//立体声，ChannelNum=2
//1次模拟采样就是16-bit（2字节），因为是双通道，所以是4字节
//1个frame中最小的传输单位，
//1 frame=(ChannelNum)*(1 sample in bytes)=(2通道）×16bit=4字节。
//为了达到2*44.1KHz的采样率，系统必须达到该值的传输速率
//传输速率=(ChannelNum)*(1 sample in bytes)*(SampleRate)
//=2通道*一次采样16bit×44.1KHz采样率
//=2×16bit×44100Hz=1411200bits_per_seconds=176400 Bytes_per_second.
//现在，如果ALSA每秒中断一次，那么我们就至少需要准备好176400字节数据发送给声卡，才不会导致播放卡
//如果中断每500ms发生一次，那么我们就至少需要176400Bytes/(1s/500ms)=88200bytes的数据
//如果中断每100ms发生一次，那么我们就至少要准备176400Bytes/(1s/100ms)=17640Bytes的数据
///////////////////////////////////////////////////////////////////////////////////
//这里我们使用48KHz的采样率，双声道，采样位数16bit
//则有bps=2*16bit*48000=1536000bits/s=192000Bytes
//这里我们设置period为4，即1秒发生4次中断，则中断间隔为1s/4=250ms.
//则每次中断发生时，我们至少需要填充192000Bytes/(1s/250ms)=48000Bytes
//#define     BLOCK_SIZE        48000	// Number of bytes

//这里我们设置period为10，即1秒发生10次中断，则中断间隔为1s/10=100ms.
//则每次中断发生时，我们至少需要填充192000Bytes/(1s/100ms)=19200Bytes

#define PERIODS 4
#define PERIOD_SIZE 48000

//for opus encode/decode.
#define OPUS_SAMPLE_FRMSIZE     (960) //frame size in 16 bit sample.
#define OPUS_BLKFRM_SIZEx2      (OPUS_SAMPLE_FRMSIZE*CHANNELS_NUM*sizeof(opus_int16)) //2 channels.
#define OPUS_BITRATE            64000


//#define SPLIT_IMG_3X3   1
#define SPLIT_IMG_2X2   1
//Video related parameters.

#define BUFSIZE_1MB     (1*1024*1024)
#define BUFSIZE_2MB     (2*1024*1024)
#define BUFSIZE_4MB     (4*1024*1024)
#define BUFSIZE_8MB     (8*1024*1024)
#define BUFSIZE_10MB    (10*1024*1024)


#define FIFO_DEPTH   30
#define FIFO_SIZE   (gGblPara.m_widthCAM1*gGblPara.m_heightCAM1*3) //RGB888.

//audio related parameters.
class ZAudioParam
{
public:
    ZAudioParam();

public:
    //run mode.
    //0:Only do capture and write pcm to wav file.
    //1:realtime stream process,capture-process-playback.
    //1 at default.
    qint32 m_runMode;

    //channel number for capture & playback.
    qint32 m_nChannelNum;
    //de-noise control.
    qint32 m_nDeNoiseMethod;

    qint32 m_nGaindB;
    //default webRTC grade.0,1,2.
    qint32 m_nWebRtcNsPolicy;
    bool m_bWebRtcInitFlag;
public:
    //capture thread thread buffer overrun.
    qint32 m_nCapOverrun;
    //playback thread buffer underrun.
    qint32 m_nPlayUnderrun;

public:
    //同一时刻我们仅允许一个tcp客户端连接.
    bool m_bAudioTcpConnected;

    //for noise view.
    qint32 m_nRNNoiseView;

    //libns_professional 2019/6/27.
    qint8 m_nBandGain0;
    qint8 m_nBandGain1;
    qint8 m_nBandGain2;
    qint8 m_nBandGain3;
    qint8 m_nBandGain4;
    qint8 m_nBandGain5;
    qint8 m_nBandGain6;
    qint8 m_nBandGain7;
    qint32 m_nEnhanceStyle;
    qint32 m_nEnhanceGrade;
    qint32 m_nDenoiseGrade;
    bool m_bPreEnhance;
    bool m_bNsProfessionalFlag;
};

class ZGblPara
{
public:
    ZGblPara();
    ~ZGblPara();

public:
    uint8_t ZDoCheckSum(uint8_t *pack,uint8_t pack_len);
    void int32_char8x2_low(qint32 int32,char *char8x2);
    void int32_char8x4(qint32 int32,char *char8x4);

    void readCfgFile();
    void writeCfgFile();
    void initCfgFile();
public:
    //(x1,y1) calibrate center of camera1.
    qint32 m_calCenterX1;
    qint32 m_calCenterY1;
    //(x2,y2) calibrate center of camera2.
    qint32 m_calCenterX2;
    qint32 m_calCenterY2;

    //(x3,y3) calibrate center of camera3.
    qint32 m_calCenterX3;
    qint32 m_calCenterY3;

    //cut box size.
    qint32 m_nCutBoxWidth;
    qint32 m_nCutBoxHeight;
public:
    //the global request to exit flag.
    //it will cause every thread occurs errors.
    bool m_bGblRst2Exit;

public:
    //accumulated run seconds.
    qint64 m_nAccumulatedSec;

public:
    //Android(tcp) <--> STM32(uart).
    bool m_bTcp2UartConnected;
    qint64 m_nTcp2UartBytes;
    qint64 m_nUart2TcpBytes;

public:
    //json thread.
    bool m_bJsonConnected;
    //image processing on flag.
    bool m_bJsonImgProOn;

    //imgproc algorithm.
    ImgProcAlgorithms m_nAlgorithm;
public:
    //audio related parameters.
    ZAudioParam m_audio;
};
extern ZGblPara gGblPara;


extern QByteArray qint32ToQByteArray(qint32 val);
extern qint32 QByteArrayToqint32(QByteArray ba);


#include <opencv2/opencv.hpp>
#include <QImage>
extern cv::Mat QImage2cvMat(const QImage &img);
extern QImage cvMat2QImage(const cv::Mat &mat);


#define MAX_AUDIO_RING_BUFFER  (30)
#define MAX_VIDEO_RING_BUFFER  (30)


#endif // ZGBLPARA_H
