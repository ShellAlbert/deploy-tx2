#include "znoisecutthread.h"
#include "zgblpara.h"
#include <QDebug>
#include <QFile>
#include "../libns/libns.h"

//use following empty code to bypass libns.
#if 0
int ns_init(int mode)
{
    return 0;
}

int ns_custom_init(int denoiseAlgorithm,
                   int denoiseLevel,
                   int enhancedType,
                   int enhancedLevel,
                   char* customBandGains,
                   char preEmphasisFlag)
{
    return 0;
}



int ns_processing(char *pPcmAudio,const int nPcmLength)
{
    return 0;
}

int ns_uninit(void)
{
    return 0;
}
#endif

#define FRAME_SIZE_SHIFT 2
#define FRAME_SIZE (120<<FRAME_SIZE_SHIFT)

void f32_to_s16(int16_t *pOut,const float *pIn,size_t sampleCount)
{
    if(pOut==NULL || pIn==NULL)
    {
        return;
    }
    for(size_t i=0;i<sampleCount;i++)
    {
        *pOut++=(short)pIn[i];
    }
}
void s16_to_f32(float *pOut,const int16_t *pIn,size_t sampleCount)
{
    if(pOut==NULL || pIn==NULL)
    {
        return;
    }
    for(size_t i=0;i<sampleCount;i++)
    {
        *pOut++=pIn[i];
    }
}
ZNoiseCutThread::ZNoiseCutThread()
{
    this->m_bCleanup=true;
}
void ZNoiseCutThread::ZBindInFIFO(QQueue<QByteArray*> *freeQueue,QQueue<QByteArray*> *usedQueue,///<
                                  QMutex *mutex,QWaitCondition *condQueueEmpty,QWaitCondition *condQueueFull)
{
    this->m_freeQueueIn=freeQueue;
    this->m_usedQueueIn=usedQueue;
    this->m_mutexIn=mutex;
    this->m_condQueueEmptyIn=condQueueEmpty;
    this->m_condQueueFullIn=condQueueFull;
}
void ZNoiseCutThread::ZBindOut1FIFO(QQueue<QByteArray*> *freeQueue,QQueue<QByteArray*> *usedQueue,///<
                                    QMutex *mutex,QWaitCondition *condQueueEmpty,QWaitCondition *condQueueFull)
{
    this->m_freeQueueOut1=freeQueue;
    this->m_usedQueueOut1=usedQueue;
    this->m_mutexOut1=mutex;
    this->m_condQueueEmptyOut1=condQueueEmpty;
    this->m_condQueueFullOut1=condQueueFull;
}
void ZNoiseCutThread::ZBindOut2FIFO(QQueue<QByteArray*> *freeQueue,QQueue<QByteArray*> *usedQueue,///<
                                    QMutex *mutex,QWaitCondition *condQueueEmpty,QWaitCondition *condQueueFull)
{
    this->m_freeQueueOut2=freeQueue;
    this->m_usedQueueOut2=usedQueue;
    this->m_mutexOut2=mutex;
    this->m_condQueueEmptyOut2=condQueueEmpty;
    this->m_condQueueFullOut2=condQueueFull;
}

qint32 ZNoiseCutThread::ZStartThread()
{
    this->start();
    return 0;
}
qint32 ZNoiseCutThread::ZStopThread()
{
    return 0;
}
bool ZNoiseCutThread::ZIsExitCleanup()
{
    return this->m_bCleanup;
}
void ZNoiseCutThread::run()
{
    //WebRTC.
    //use this variable to control webRtc noiseSuppression grade.
    //valid range is 0,1,2.default is 0.
    qint32 nWebRtcNsPolicy=gGblPara.m_audio.m_nWebRtcNsPolicy;
    if(0!=WebRtcNs_Create(&this->m_pNS_inst))
    {
        qCritical()<<"NoiseCut,error at WebRtcNs_Create().";
        //set global request to exit flag to cause other threads to exit.
        gGblPara.m_bGblRst2Exit=true;
        return;
    }
    //webRtc only supports 8KHz,16KHz,32KHz.
    //CAUTION HERE: it does not support 48khz!!!
    if(0!=WebRtcNs_Init(this->m_pNS_inst,/*32000*/16000))
    {
        qCritical()<<"NoiseCut,error at WebRtcNs_Init().";
        //set global request to exit flag to cause other threads to exit.
        gGblPara.m_bGblRst2Exit=true;
        return;
    }
    if(0!=WebRtcNs_set_policy(this->m_pNS_inst,nWebRtcNsPolicy))
    {
        qCritical()<<"NoiseCut,error at WebRtcNs_set_policy().";
        //set global request to exit flag to cause other threads to exit.
        gGblPara.m_bGblRst2Exit=true;
        return;
    }
    //auto gain control.
    WebRtcAgc_Create(&this->m_agcHandle);
    int minLevel = 0;
    int maxLevel = 255;
    //使用自动增益放大的音量比较有限，所以此处使用固定增益模式.
    int agcMode  = kAgcModeFixedDigital;
    WebRtcAgc_Init(this->m_agcHandle, minLevel, maxLevel, agcMode,8000);
    //1.compressionGaindB,在Fixed模式下，该值越大声音越大.
    //2.targetLevelDbfs,表示相对于Full Scale的下降值，0表示full scale,该值越小声音越大.
    WebRtcAgc_config_t agcConfig;
    //compressionGaindB.
    //Set the maximum gain the digital compression stage many apply in dB.
    //A higher number corresponds to greater compression.
    //while a value of 0 will leave the signal uncompressed.
    //value range: limited to [0,90].

    //targetLevelDbfs.
    //According the description of webrtc:
    //change of this parameter will set the target peak level of the AGC in dBFs.
    //the conversion is to use positive values.
    //For instance,passing in a value of 3 corresponds to -3 dBFs or a target level 3dB below full-scale.
    //value range: limited to [0,31].

    //limiterEnable.
    //When enabled,the compression stage will hard limit the signal to the target level.
    //otherwise,the signal will be compressed but not limited above the target level.

    agcConfig.compressionGaindB=gGblPara.m_audio.m_nGaindB;
    agcConfig.targetLevelDbfs=3;//dBFs表示相对于full scale的下降值,0表示full scale.3dB below full-scale.
    agcConfig.limiterEnable=1;
    WebRtcAgc_set_config(this->m_agcHandle,agcConfig);
    WebRtcSpl_ResetResample48khzTo16khz(&m_state4816);
    WebRtcSpl_ResetResample16khzTo48khz(&m_state1648);

    int frameSize=160;//80;
    //int len=frameSize*sizeof(short);
    this->m_pDataIn=(short*)malloc(frameSize*sizeof(short));
    this->m_pDataOut=(short*)malloc(frameSize*sizeof(short));

    //这个变量用于控制通过json动态调整增益.
    qint32 nGaindBShadow=gGblPara.m_audio.m_nGaindB;

    this->m_pcm16k=new char[PERIOD_SIZE];
    if(NULL==this->m_pcm16k)
    {
        qCritical()<<"NoiseCut,failed to allocate buffer form pcm16k!";
        return;
    }

    qInfo()<<"NoiseCut,main loop starts.";
    this->m_bCleanup=false;

    //for libns.
    qint32 nLibNsModeShadow=gGblPara.m_audio.m_nRNNoiseView;
    //here call ns_init() or ns_custom_init() for call ns_uninit() later.
    ns_init(0);//0~5.
    int denoiseAlgorithm = 3;
    int denoiseLevel = 0;
    int enhancedType = 0;
    int enhancedLevel = 0;
    char* customBandGains = NULL;
    char preEmphasisFlag = 0;
    customBandGains=new char[8];
    memset(customBandGains,0,8);
    if(ns_custom_init(denoiseAlgorithm, denoiseLevel, enhancedType, enhancedLevel, customBandGains, preEmphasisFlag))
    {
        qCritical()<<"NoiseCut,failed to init ns_custom_init().";
        //set global request to exit flag to cause other threads to exit.
        gGblPara.m_bGblRst2Exit=true;
        return;
    }


    //the main loop.
    while(!gGblPara.m_bGblRst2Exit)
    {
#if 1
        //WebRTC降噪级别被json修改了，这里重新初始化WebRTC.
        if(nWebRtcNsPolicy!=gGblPara.m_audio.m_nWebRtcNsPolicy)
        {
            WebRtcNs_Free(this->m_pNS_inst);
            WebRtcAgc_Free(this->m_agcHandle);
            this->m_pNS_inst=NULL;
            this->m_agcHandle=NULL;

            //update flag.
            nWebRtcNsPolicy=gGblPara.m_audio.m_nWebRtcNsPolicy;
            //re-init webRTC.
            if(0!=WebRtcNs_Create(&this->m_pNS_inst))
            {
                qCritical()<<"NoiseCut,error at WebRtcNs_Create().";
                //set global request to exit flag to cause other threads to exit.
                gGblPara.m_bGblRst2Exit=true;
                break;
            }
            if(0!=WebRtcNs_Init(this->m_pNS_inst,/*32000*/16000))
            {
                qCritical()<<"NoiseCut,error at WebRtcNs_Init().";
                //set global request to exit flag to cause other threads to exit.
                gGblPara.m_bGblRst2Exit=true;
                break;
            }
            if(0!=WebRtcNs_set_policy(this->m_pNS_inst,nWebRtcNsPolicy))
            {
                qCritical()<<"NoiseCut,error at WebRtcNs_set_policy().";
                //set global request to exit flag to cause other threads to exit.
                gGblPara.m_bGblRst2Exit=true;
                break;
            }
            //auto gain control.
            WebRtcAgc_Create(&this->m_agcHandle);
            int minLevel = 0;
            int maxLevel = 255;
            //使用自动增益放大的音量比较有限，所以此处使用固定增益模式.
            int agcMode  = kAgcModeFixedDigital;
            WebRtcAgc_Init(this->m_agcHandle, minLevel, maxLevel, agcMode,8000);
            //1.compressionGaindB,在Fixed模式下，该值越大声音越大.
            //2.targetLevelDbfs,表示相对于Full Scale的下降值，0表示full scale,该值越小声音越大.
            WebRtcAgc_config_t agcConfig;
            //compressionGaindB.
            //Set the maximum gain the digital compression stage many apply in dB.
            //A higher number corresponds to greater compression.
            //while a value of 0 will leave the signal uncompressed.
            //value range: limited to [0,90].

            //targetLevelDbfs.
            //According the description of webrtc:
            //change of this parameter will set the target peak level of the AGC in dBFs.
            //the conversion is to use positive values.
            //For instance,passing in a value of 3 corresponds to -3 dBFs or a target level 3dB below full-scale.
            //value range: limited to [0,31].

            //limiterEnable.
            //When enabled,the compression stage will hard limit the signal to the target level.
            //otherwise,the signal will be compressed but not limited above the target level.

            agcConfig.compressionGaindB=nGaindBShadow;
            agcConfig.targetLevelDbfs=3;//dBFs表示相对于full scale的下降值,0表示full scale.3dB below full-scale.
            agcConfig.limiterEnable=1;
            WebRtcAgc_set_config(this->m_agcHandle,agcConfig);

            WebRtcSpl_ResetResample48khzTo16khz(&m_state4816);
            WebRtcSpl_ResetResample16khzTo48khz(&m_state1648);
        }
#endif

#if 1
        //动态增益变量肯定被json协议修改了,我们重新调整动态增益的相关设置。
        if(nGaindBShadow!=gGblPara.m_audio.m_nGaindB)
        {
            //重新初始化.
            WebRtcAgc_Create(&this->m_agcHandle);
            WebRtcAgc_Init(this->m_agcHandle,0,255,kAgcModeFixedDigital,8000);
            WebRtcAgc_config_t agcConfig;
            agcConfig.compressionGaindB=gGblPara.m_audio.m_nGaindB;
            agcConfig.targetLevelDbfs=3;
            agcConfig.limiterEnable=1;
            WebRtcAgc_set_config(this->m_agcHandle,agcConfig);

            //update flag.
            nGaindBShadow=gGblPara.m_audio.m_nGaindB;
            qDebug()<<"NoiseCut,update DGain to "<<nGaindBShadow;
        }

#endif

        //get a data buffer from fifo.
        this->m_mutexIn->lock();
        while(this->m_usedQueueIn->isEmpty())
        {//timeout 5s to check exit flag.
            if(!this->m_condQueueFullIn->wait(this->m_mutexIn,5000))
            {
                this->m_mutexIn->unlock();
                if(gGblPara.m_bGblRst2Exit)
                {
                    break;
                }
            }
        }
        if(gGblPara.m_bGblRst2Exit)
        {
            break;
        }
        QByteArray *pcmIn=this->m_usedQueueIn->dequeue();
        this->m_mutexIn->unlock();
        //qDebug()<<"noisecut,get one frame";

        //noise cut processing by different algorithm.
        //qDebug()<<"noisecut:size:"<<pcmIn->size();
        switch(gGblPara.m_audio.m_nDeNoiseMethod)
        {
        case 0:
            //qDebug()<<"DeNoise:disabled";
            break;
        case 1:
            //qDebug()<<"DeNoise:RNNoise Enabled";
            if(nLibNsModeShadow!=gGblPara.m_audio.m_nRNNoiseView)
            {
                ns_uninit();
                nLibNsModeShadow=gGblPara.m_audio.m_nRNNoiseView;
                ns_init(nLibNsModeShadow);
            }
            this->ZCutNoiseByRNNoise(pcmIn);
            break;
        case 2:
            //qDebug()<<"DeNoise:WebRTC Enabled";
            if(gGblPara.m_audio.m_bWebRtcInitFlag)
            {
                ns_uninit();
                char customBandGains[8]={0};
                ns_custom_init(6,gGblPara.m_audio.m_nWebRtcNsPolicy,0,0,customBandGains,0);
                gGblPara.m_audio.m_bWebRtcInitFlag=false;
            }
            this->ZCutNoiseByWebRTC(pcmIn);
            break;
        case 3:
            //qDebug()<<"DeNoise:Bevis Enabled";
            //this->ZCutNoiseByBevis(pcmIn);
            break;
        case 4:
            //logMMSE.
            //this->ZCutNoiseByLogMMSE(pcmIn);
            break;
        case 5:
            //NRAE.
            if(gGblPara.m_audio.m_bNsProfessionalFlag)
            {
                ns_uninit();
                qint8 customBandGains[8]={gGblPara.m_audio.m_nBandGain0,///<
                                          gGblPara.m_audio.m_nBandGain1,///<
                                          gGblPara.m_audio.m_nBandGain2,///<
                                          gGblPara.m_audio.m_nBandGain3,///<
                                          gGblPara.m_audio.m_nBandGain4,///<
                                          gGblPara.m_audio.m_nBandGain5,///<
                                          gGblPara.m_audio.m_nBandGain6,///<
                                          gGblPara.m_audio.m_nBandGain7};
                ns_custom_init(3,gGblPara.m_audio.m_nDenoiseGrade,gGblPara.m_audio.m_nEnhanceStyle,///<
                               gGblPara.m_audio.m_nEnhanceGrade,(char*)customBandGains,gGblPara.m_audio.m_bPreEnhance?1:0);
                gGblPara.m_audio.m_bNsProfessionalFlag=false;
            }
            this->ZCutNoiseByRNNoise(pcmIn);
            break;
        default:
            break;
        }


        if(gGblPara.m_audio.m_nGaindB>0)
        {
            this->ZDGainByWebRTC(pcmIn);
        }

        //move data from IN fifo to OUT1 fifo.
        this->m_mutexOut1->lock();
        while(this->m_freeQueueOut1->isEmpty())
        {//timeout 5s to check exit flag.
            if(!this->m_condQueueEmptyOut1->wait(this->m_mutexOut1,5000))
            {
                this->m_mutexOut1->unlock();
                if(gGblPara.m_bGblRst2Exit)
                {
                    break;
                }
            }
        }
        if(gGblPara.m_bGblRst2Exit)
        {
            break;
        }
        QByteArray *bufferOut1=this->m_freeQueueOut1->dequeue();
        this->m_mutexOut1->unlock();
        //move data from one fifo to another fifo.
        memcpy(bufferOut1->data(),pcmIn->data(),PERIOD_SIZE);

        this->m_mutexOut1->lock();
        this->m_usedQueueOut1->enqueue(bufferOut1);
        this->m_condQueueFullOut1->wakeAll();
        this->m_mutexOut1->unlock();

        //qDebug()<<"copy data from in fifo to out1 fifo";
        this->m_mutexIn->lock();
        this->m_freeQueueIn->enqueue(pcmIn);
        this->m_condQueueEmptyIn->wakeAll();
        this->m_mutexIn->unlock();

        //move data from IN fifo to OUT2 fifo.
        //当有客户端连接上时，才将采集到的数据放到发送队列.
        //如果TCP传输速度慢，则会导致发送队列满，所以这里使用try防止本地阻塞，可能造成音频丢帧现象。
        if(gGblPara.m_audio.m_bAudioTcpConnected)
        {
            if(this->m_mutexOut2->tryLock())
            {
                while(this->m_freeQueueOut2->isEmpty())
                {
                    if(!this->m_condQueueEmptyOut2->wait(this->m_mutexOut2,5000))
                    {
                        this->m_mutexOut2->unlock();
                        if(gGblPara.m_bGblRst2Exit)
                        {
                            return;
                        }
                    }
                }
                QByteArray *bufferOut2=this->m_freeQueueOut2->dequeue();
                //move data from one fifo to another fifo.
                memcpy(bufferOut2->data(),pcmIn->data(),PERIOD_SIZE);
                this->m_usedQueueOut2->enqueue(bufferOut2);
                this->m_condQueueFullOut2->wakeAll();
                this->m_mutexOut2->unlock();
            }
        }


    }

    //rnnoise_destroy(this->m_st);
    WebRtcNs_Free(this->m_pNS_inst);
    WebRtcAgc_Free(this->m_agcHandle);
    delete [] this->m_pcm16k;

    //uninit libns.
    ns_uninit();

    qInfo()<<"NoiseCut,main loop ends.";
    //set global request to exit flag to help other thread to exit.
    gGblPara.m_bGblRst2Exit=true;
    emit this->ZSigThreadFinished();
    this->m_bCleanup=true;
    return;
}

qint32 ZNoiseCutThread::ZCutNoiseByRNNoise(QByteArray *baPCM)
{
    //because original pcm data is 48000 bytes.
    //libns only process 960 bytes each time.
    //so 48000/960=50.
    qint32 nOffset=0;
    for(qint32 i=0;i<50;i++)
    {
        char *pPcmAudio=baPCM->data()+nOffset;
        ns_processing(pPcmAudio,960);
        nOffset+=960;
    }
    return 0;
}

//WebRTC.
qint32 ZNoiseCutThread::ZCutNoiseByWebRTC(QByteArray *baPCM)
{
    //because original pcm data is 48000 bytes.
    //libns only process 960 bytes each time.
    //so 48000/960=50.
    qint32 nOffset=0;
    for(qint32 i=0;i<50;i++)
    {
        char *pPcmAudio=baPCM->data()+nOffset;
        ns_processing(pPcmAudio,960);
        nOffset+=960;
    }
#if 0
    //48khz downsample to 16khz,so data is decrease 3.
    //we dump data from baPCM to m_pcm16k.
    qint32 nOffset16k=0;
    //downsample 48khz to 16khz.48000/(480*2)=50.
    qint32 nLoopNum1=baPCM->size()/(480*sizeof(int16_t));
    for(qint32 i=0;i<nLoopNum1;i++)
    {

        int16_t tmpIn[480];//48khz,480=10ms.
        int16_t tmpOut[160];//16khz,160=10ms.
        int tmpMem[960];
        memcpy(tmpIn,baPCM->data()+i*sizeof(tmpIn),sizeof(tmpIn));
        WebRtcSpl_Resample48khzTo16khz(tmpIn,tmpOut,&m_state4816,tmpMem);
        //save data.
        memcpy(this->m_pcm16k+nOffset16k,tmpOut,sizeof(tmpOut));
        nOffset16k+=sizeof(tmpOut);
    }
    //qDebug()<<"48khz-16khz okay,bytes:"<<nOffset16k;

    ////////////////////////////////////////////////////////////

    int i;
    //    char *pcmData=baPCM->data();
    //    qint32 nPCMDataLen=baPCM->size();
    char *pcmData=this->m_pcm16k;
    qint32 nPCMDataLen=nOffset16k;
    int  filter_state1[6],filter_state12[6];
    int  Synthesis_state1[6],Synthesis_state12[6];

    memset(filter_state1,0,sizeof(filter_state1));
    memset(filter_state12,0,sizeof(filter_state12));
    memset(Synthesis_state1,0,sizeof(Synthesis_state1));
    memset(Synthesis_state12,0,sizeof(Synthesis_state12));

    for(i=0;i<nPCMDataLen;i+=320)
    {
        if((nPCMDataLen-i)>=320)
        {
            short shInL[160]={0},shInH[160]={0};
            short shOutL[160]={0},shOutH[160]={0};
            memcpy(shInL,(char*)(pcmData+i),160*sizeof(short));
            if (0==WebRtcNs_Process(this->m_pNS_inst,shInL,shInH,shOutL,shOutH))
            {
                memcpy(pcmData+i,shOutL,160*sizeof(short));
            }
        }
    }



    //processing finished,we dump data from m_pcm16k to baPCM.
    //16000/(160*2)=50.
    qint32 xTmpOffset=0;
    qint32 nLoopNum2=nOffset16k/(sizeof(int16_t)*160);
    for(qint32 i=0;i<nLoopNum2;i++)
    {

        int16_t xTmpIn[160];
        int16_t xTmpOut[480];
        int xTmpMem[960];
        memcpy(xTmpIn,this->m_pcm16k+i*sizeof(xTmpIn),sizeof(xTmpIn));
        WebRtcSpl_Resample16khzTo48khz(xTmpIn,xTmpOut,&m_state1648,xTmpMem);
        //save data.
        memcpy(baPCM->data()+xTmpOffset,xTmpOut,sizeof(xTmpOut));
        xTmpOffset+=sizeof(xTmpOut);
    }
    //qDebug()<<"16khz-48khz okay,bytes:"<<xTmpOffset;
#endif

    return 0;
}

//auto gain control.
qint32 ZNoiseCutThread::ZDGainByWebRTC(QByteArray *baPCM)
{
    int frameSize=160;//80;
    int len=frameSize*sizeof(short);

    int micLevelIn=0;
    int micLevelOut=0;

    char *pcmData=(char*)baPCM->data();
    int nRemaingBytes=baPCM->size();
    int nProcessedBytes=0;

    while(nRemaingBytes>=len)
    {
        //prepare data.
        memcpy(this->m_pDataIn,pcmData+nProcessedBytes,len);

        int inMicLevel=micLevelOut;
        int outMicLevel=0;
        uint8_t saturationWarning;
        int nAgcRet = WebRtcAgc_Process(this->m_agcHandle,this->m_pDataIn,NULL,frameSize,this->m_pDataOut,NULL,inMicLevel,&outMicLevel,0,&saturationWarning);
        if(nAgcRet!=0)
        {
            qCritical()<<"NoiseCut,error at WebRtcAgc_Process().";
            return -1;
        }
        micLevelIn=outMicLevel;

        //copy data out.
        memcpy(pcmData+nProcessedBytes,this->m_pDataOut,len);

        //update the processed and remaing bytes.
        nProcessedBytes+=len;
        nRemaingBytes-=len;
    }
    return 0;
}
