#include "zimgprocthread.h"
#include "csk_tracker.h"
#include "imgproc/KCF/kcftracker.hpp"
#include <zgblpara.h>
#include <string.h>
#include <QDebug>
#include <opencv2/opencv.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/highgui.hpp>
ZImgProcThread::ZImgProcThread()
{

}
qint32 ZImgProcThread::ZBindQueue1(QMutex *mutex,///<
                                   QWaitCondition *condNotEmpty,QWaitCondition *condNotFull,///<
                                   QQueue<cv::Mat*> *queueFree,QQueue<cv::Mat*> *queueUsed)
{
    this->m_mutex1=mutex;
    this->m_condNotEmpty1=condNotEmpty;
    this->m_condNotFull1=condNotFull;
    this->m_queueFree1=queueFree;
    this->m_queueUsed1=queueUsed;

    return 0;
}
qint32 ZImgProcThread::ZBindQueue2(QMutex *mutex,///<
                                   QWaitCondition *condNotEmpty,QWaitCondition *condNotFull,///<
                                   QQueue<cv::Mat*> *queueFree,QQueue<cv::Mat*> *queueUsed)
{
    this->m_mutex2=mutex;
    this->m_condNotEmpty2=condNotEmpty;
    this->m_condNotFull2=condNotFull;
    this->m_queueFree2=queueFree;
    this->m_queueUsed2=queueUsed;

    return 0;
}

void ZImgProcThread::run()
{
    //allocate temporary buffer.
    cv::Mat *matMainImg=new cv::Mat(ImgResizedHeight,ImgResizedWidth,CV_8UC1);
    cv::Mat *matAuxImg=new cv::Mat(ImgResizedHeight,ImgResizedWidth,CV_8UC1);

    //rectangle for main&aux img.
    cv::Rect rectMain;
    cv::Rect rectAux;

    //opencv Template Match algorithm initial code here.

    //opencv CSK Tracker algorithm initial code here.
    CSK_Tracker tracker;
    cv::Point trackerPtCenter;
    cv::Size trackerTargetSize;
    bool bTrackerInitBox=false;

#if 1
    //openCV KCF tracker algorithm initial code here.
    bool HOG=false; //true for RGB,false for gray.
    bool FIXEDWINDOW=false;
    bool MULTISCALE=true;
    bool LAB=false;
    // Create KCFTracker object
    KCFTracker trackerKCF(HOG, FIXEDWINDOW, MULTISCALE, LAB);
    bool bKCFTrackerInit=false;
#endif

    //the main loop.
    qDebug()<<"imgproc thread start.";
    while(!gGblPara.m_bGblRst2Exit)
    {

        bool bProcNextTime=false;

        //1.fetch image from queue1.
        this->m_mutex1->lock();
        while(this->m_queueUsed1->size()<=0)
        {
            if(!this->m_condNotEmpty1->wait(this->m_mutex1,5000))
            {
                this->m_mutex1->unlock();
                bProcNextTime=true;
                break;
            }
        }
        if(bProcNextTime)
        {
            continue;
        }
        cv::Mat *matBase1=this->m_queueUsed1->dequeue();
        memcpy(matMainImg->data,matBase1->data,matBase1->cols*matBase1->rows*matBase1->channels());
        this->m_queueFree1->enqueue(matBase1);
        this->m_condNotFull1->wakeAll();
        this->m_mutex1->unlock();


        //2.fetch image from queue2.
        this->m_mutex2->lock();
        while(this->m_queueUsed2->size()<=0)
        {
            if(!this->m_condNotEmpty2->wait(this->m_mutex2,5000))
            {
                this->m_mutex2->unlock();
                break;
            }
        }
        cv::Mat *matBase2=this->m_queueUsed2->dequeue();
        memcpy(matAuxImg->data,matBase2->data,matBase2->cols*matBase2->rows*matBase2->channels());
        this->m_queueFree2->enqueue(matBase2);
        this->m_condNotFull2->wakeAll();
        this->m_mutex2->unlock();

        //3.do image algorithm.
        //qDebug()<<"imgproc fetch 2 images okay.";
        qint32 nCalCenterX1=gGblPara.m_calCenterX1*gImgResizeRatioW;
        qint32 nCalCenterY1=gGblPara.m_calCenterY1*gImgResizeRatioH;
        switch(gGblPara.m_nAlgorithm)
        {
        case IMGPROC_BYPASS:
        {
            //bypass imgproc,do nothing here.

            //draw a rectangle on main img for indicate object selection.
            qint32 nMainImgRectX=nCalCenterX1-gGblPara.m_nCutBoxWidth/2;
            qint32 nMainImgRectY=nCalCenterY1-gGblPara.m_nCutBoxHeight/2;
            rectMain=cv::Rect(nMainImgRectX,nMainImgRectY,gGblPara.m_nCutBoxWidth,gGblPara.m_nCutBoxHeight);
            //check coordinates validation.
            if(rectMain.x<0)
            {
                rectMain.x=0;
            }
            if(rectMain.y<0)
            {
                rectMain.y=0;
            }
            if((rectMain.x+rectMain.width)>matMainImg->cols)
            {
                rectMain.width=matMainImg->cols-rectMain.x-2;
            }
            if((rectMain.y+rectMain.height)>matMainImg->rows)
            {
                rectMain.height=matMainImg->rows-rectMain.y-2;
            }
            cv::rectangle(*matMainImg,rectMain,cv::Scalar(0,255,0,255),2);

            //cause to reinit tracker when algorithm changes.
            bTrackerInitBox=false;
            //bKCFTrackerInit=false;
        }
            break;
        case OPENCV_TEMPLATE_MATCH:
        {
            //we cut a (w*h) box image centered on calibrate center(x,y).
            qint32 nMainImgRectX=nCalCenterX1-gGblPara.m_nCutBoxWidth/2;
            qint32 nMainImgRectY=nCalCenterY1-gGblPara.m_nCutBoxHeight/2;
            rectMain=cv::Rect(nMainImgRectX,nMainImgRectY,gGblPara.m_nCutBoxWidth,gGblPara.m_nCutBoxHeight);
            //check coordinates validation.
            if(rectMain.x<0)
            {
                rectMain.x=0;
            }
            if(rectMain.y<0)
            {
                rectMain.y=0;
            }
            if((rectMain.x+rectMain.width)>matMainImg->cols)
            {
                rectMain.width=matMainImg->cols-rectMain.x-2;
            }
            if((rectMain.y+rectMain.height)>matMainImg->rows)
            {
                rectMain.height=matMainImg->rows-rectMain.y-2;
            }


            //prepare a little mat.
            cv::Mat matTemplate(*matMainImg,rectMain);

            //do match template algorithm.
            cv::Mat matResult;
            cv::matchTemplate(*matAuxImg,matTemplate,matResult,CV_TM_SQDIFF_NORMED);
            cv::normalize(matResult,matResult,0,1,cv::NORM_MINMAX,-1,cv::Mat());

            //localizing the best match with minMaxLoc.
            double fMinVal,fMaxVal;
            cv::Point ptMinLoc,ptMaxLoc,ptMatched;
            cv::minMaxLoc(matResult,&fMinVal,&fMaxVal,&ptMinLoc,&ptMaxLoc,cv::Mat());
            qDebug("min=%.2f,max=%.2f\n",fMinVal,fMaxVal);
            ptMatched=ptMinLoc;//the minimum is the best for CV_TM_SQDIFF_NORMED.

            //generate the pixel coordinate different value.
            qint32 nDiffX=nMainImgRectX-ptMatched.x;
            qint32 nDiffY=nMainImgRectY-ptMatched.y;
            qDebug()<<nDiffX<<nDiffY;


            //draw a rectangle on main img.
            cv::rectangle(*matMainImg,rectMain,cv::Scalar(0,255,0,255),2);

            //draw a rectangle on aux img.
            rectAux=cv::Rect(ptMatched.x,ptMatched.y,matTemplate.cols,matTemplate.rows);
            cv::rectangle(*matAuxImg,rectAux,cv::Scalar(0,255,0,255),2);
        }
            break;
        case OPENCV_CSK_TRACKER:
        {
            if(!bTrackerInitBox)
            {
                //init tracker box.
                rectMain.x=nCalCenterX1-gGblPara.m_nCutBoxWidth/2;
                rectMain.y=nCalCenterY1-gGblPara.m_nCutBoxHeight/2;
                rectMain.width=gGblPara.m_nCutBoxWidth;
                rectMain.height=gGblPara.m_nCutBoxHeight;
                //check coordinates validation.
                if(rectMain.x<0)
                {
                    rectMain.x=0;
                }
                if(rectMain.y<0)
                {
                    rectMain.y=0;
                }
                if((rectMain.x+rectMain.width)>matMainImg->cols)
                {
                    rectMain.width=matMainImg->cols-rectMain.x-2;
                }
                if((rectMain.y+rectMain.height)>matMainImg->rows)
                {
                    rectMain.height=matMainImg->rows-rectMain.y-2;
                }

                //define the center point.
                trackerPtCenter.x=rectMain.x+rectMain.width/2;
                trackerPtCenter.y=rectMain.y+rectMain.height/2;

                //define target size.
                trackerTargetSize.width=rectMain.width;
                trackerTargetSize.height=rectMain.height;

                bool bInitOkay=tracker.tracker_init(*matMainImg,trackerPtCenter,trackerTargetSize);
                if(bInitOkay==false)
                {
                    qDebug()<<"failed to init CSK Tracker.";
                }else{
                    bTrackerInitBox=true;
                }
            }else{
                //update the tracking result.
                bool bResult=tracker.tracker_update(*matAuxImg,trackerPtCenter,trackerTargetSize);
                if(bResult)
                {
                    rectAux.x=trackerPtCenter.x-trackerTargetSize.width/2;
                    rectAux.y=trackerPtCenter.y-trackerTargetSize.height/2;
                    rectAux.width=trackerTargetSize.width;
                    rectAux.height=trackerTargetSize.height;

                    //to avoid reaching the boundary to cause openCV faults.
                    if(rectAux.x<0)
                    {
                        rectAux.x=0;
                    }
                    if(rectAux.y<0)
                    {
                        rectAux.y=0;
                    }
                    if((rectAux.x+rectAux.width)>matAuxImg->cols)
                    {
                        rectAux.width=matAuxImg->cols-rectAux.x-2;
                    }
                    if((rectAux.y+rectAux.height)>matAuxImg->rows)
                    {
                        rectAux.height=matAuxImg->rows-rectAux.y-2;
                    }

                    //draw a rectangle on main img.
                    cv::rectangle(*matMainImg,rectMain,cv::Scalar(0,255,0,255),2);
                    //draw a rectangle on aux img.
                    cv::rectangle(*matAuxImg,rectAux,cv::Scalar(0,255,0,255),2);

                    qDebug()<<"tracked okay:"<<rectAux.x<<rectAux.y<<rectAux.width<<rectAux.height;
                }else{
                    qDebug()<<"tracking failure detected.ReInit track box.";
                    bTrackerInitBox=false;
                }
            }
        }
            break;
        case OPENCV_KCF_TRACKER:
#if 1
        {
            if(!bKCFTrackerInit)
            {
                //we cut a (w*h) box image centered on calibrate center(x,y).
                qint32 nMainImgRectX=nCalCenterX1-gGblPara.m_nCutBoxWidth/2;
                qint32 nMainImgRectY=nCalCenterY1-gGblPara.m_nCutBoxHeight/2;
                cv::Rect rectMain(nMainImgRectX,nMainImgRectY,gGblPara.m_nCutBoxWidth,gGblPara.m_nCutBoxHeight);
                //check coordinates validation.
                if(rectMain.x<0)
                {
                    rectMain.x=0;
                }
                if(rectMain.y<0)
                {
                    rectMain.y=0;
                }
                if((rectMain.x+rectMain.width)>matMainImg->cols)
                {
                    rectMain.width=matMainImg->cols-rectMain.x-2;
                }
                if((rectMain.y+rectMain.height)>matMainImg->rows)
                {
                    rectMain.height=matMainImg->rows-rectMain.y-2;
                }
                trackerKCF.init(rectMain,*matMainImg);

                bKCFTrackerInit=true;
            }else{
                rectAux=trackerKCF.update(*matAuxImg);
                //draw a rectangle on main img.
                cv::rectangle(*matMainImg,rectMain,cv::Scalar(0,255,0,255),2);
                //draw a rectangle on aux img.
                cv::rectangle(*matAuxImg,rectAux,cv::Scalar(0,255,0,255),2);
            }
        }
#endif
            break;
        default:
            break;
        }

        //convert cv::Mat to QImage.
        QImage img1=cvMat2QImage(*matMainImg);
        emit this->ZSigNewImg1(img1);
        QImage img2=cvMat2QImage(*matAuxImg);
        emit this->ZSigNewImg2(img2);
    }

    delete matMainImg;
    delete matAuxImg;

    qDebug()<<"imgproc thread done.";
}
