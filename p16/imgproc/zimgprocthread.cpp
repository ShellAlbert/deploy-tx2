#include "zimgprocthread.h"
#include "csk_tracker.h"
#include <zgblpara.h>
#include <string.h>
#include <QDebug>
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
    cv::Mat *mat1=new cv::Mat(RTSP_H264_HEIGHT,RTSP_H264_WIDTH,CV_8UC1);
    cv::Mat *mat2=new cv::Mat(RTSP_H264_HEIGHT,RTSP_H264_WIDTH,CV_8UC1);

    //opencv Template Match algorithm initial code here.

    //opencv CSK Tracker algorithm initial code here.
    CSK_Tracker tracker;
    cv::Rect2d trackerBox;
    cv::Point trackerPtCenter;
    cv::Size trackerTargetSize;
    bool bTrackerInitBox=false;

    //the main loop.
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
        memcpy(mat1->data,matBase1->data,matBase1->cols*matBase1->rows*matBase1->channels());
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
        memcpy(mat2->data,matBase2->data,matBase2->cols*matBase2->rows*matBase2->channels());
        this->m_queueFree2->enqueue(matBase2);
        this->m_condNotFull2->wakeAll();
        this->m_mutex2->unlock();

        //3.do image algorithm.
        //qDebug()<<"imgproc fetch 2 images okay.";


        switch(gGblPara.m_nAlgorithm)
        {
        case IMGPROC_BYPASS:
        {
            //bypass imgproc,do nothing here.

            //draw a rectangle on main img for indicate object selection.
            qint32 nMainImgRectX=gGblPara.m_calCenterX1-gGblPara.m_nCutBoxWidth/2;
            qint32 nMainImgRectY=gGblPara.m_calCenterY1-gGblPara.m_nCutBoxHeight/2;
            cv::Rect rectMainImgBox(nMainImgRectX,nMainImgRectY,gGblPara.m_nCutBoxWidth,gGblPara.m_nCutBoxHeight);
            //check coordinates validation.
            if(rectMainImgBox.x<0)
            {
                rectMainImgBox.x=0;
            }
            if(rectMainImgBox.y<0)
            {
                rectMainImgBox.y=0;
            }
            if((rectMainImgBox.x+rectMainImgBox.width)>mat1->cols)
            {
                rectMainImgBox.width=mat1->cols-rectMainImgBox.x-2;
            }
            if((rectMainImgBox.y+rectMainImgBox.height)>mat1->rows)
            {
                rectMainImgBox.height=mat1->rows-rectMainImgBox.y-2;
            }
            cv::rectangle(*mat1,trackerBox,cv::Scalar(0,255,0,255),2);

            //cause to reinit tracker when algorithm changes.
            bTrackerInitBox=false;
        }
            break;
        case OPENCV_TEMPLATE_MATCH:
        {
            //we cut a (w*h) box image centered on calibrate center(x,y).
            qint32 nMainImgRectX=gGblPara.m_calCenterX1-gGblPara.m_nCutBoxWidth/2;
            qint32 nMainImgRectY=gGblPara.m_calCenterY1-gGblPara.m_nCutBoxHeight/2;
            cv::Rect rectMainImgBox(nMainImgRectX,nMainImgRectY,gGblPara.m_nCutBoxWidth,gGblPara.m_nCutBoxHeight);
            //check coordinates validation.
            if(rectMainImgBox.x<0)
            {
                rectMainImgBox.x=0;
            }
            if(rectMainImgBox.y<0)
            {
                rectMainImgBox.y=0;
            }
            if((rectMainImgBox.x+rectMainImgBox.width)>mat1->cols)
            {
                rectMainImgBox.width=mat1->cols-rectMainImgBox.x-2;
            }
            if((rectMainImgBox.y+rectMainImgBox.height)>mat1->rows)
            {
                rectMainImgBox.height=mat1->rows-rectMainImgBox.y-2;
            }


            //prepare a little mat.
            cv::Mat matTemplate(*mat1,rectMainImgBox);

            //do match template algorithm.
            cv::Mat matResult;
            cv::matchTemplate(*mat2,matTemplate,matResult,CV_TM_SQDIFF_NORMED);
            cv::normalize(matResult,matResult,0,1,cv::NORM_MINMAX,-1,cv::Mat());

            //localizing the best match with minMaxLoc.
            double fMinVal,fMaxVal;
            cv::Point ptMinLoc,ptMaxLoc,ptMatched;
            cv::minMaxLoc(matResult,&fMinVal,&fMaxVal,&ptMinLoc,&ptMaxLoc,cv::Mat());
            ptMatched=ptMinLoc;//the minimum is the best for CV_TM_SQDIFF_NORMED.

            //generate the pixel coordinate different value.
            qint32 nDiffX=nMainImgRectX-ptMatched.x;
            qint32 nDiffY=nMainImgRectY-ptMatched.y;
            qDebug()<<nDiffX<<nDiffY;


            //draw a rectangle on main img.
            cv::rectangle(*mat1,rectMainImgBox,cv::Scalar(0,255,0,255),2);

            //draw a rectangle on aux img.
            cv::Rect rectAuxImgBox(ptMatched.x,ptMatched.y,gGblPara.m_nCutBoxWidth,gGblPara.m_nCutBoxHeight);
            cv::rectangle(*mat2,rectAuxImgBox,cv::Scalar(0,255,0,255),2);
        }
            break;
        case OPENCV_CSK_TRACKER:
        {
            if(!bTrackerInitBox)
            {
                //init tracker box.
                trackerBox.x=gGblPara.m_calCenterX1-gGblPara.m_nCutBoxWidth/2;
                trackerBox.y=gGblPara.m_calCenterY1-gGblPara.m_nCutBoxHeight/2;
                trackerBox.width=gGblPara.m_nCutBoxWidth;
                trackerBox.height=gGblPara.m_nCutBoxHeight;
                //check coordinates validation.
                if(trackerBox.x<0)
                {
                    trackerBox.x=0;
                }
                if(trackerBox.y<0)
                {
                    trackerBox.y=0;
                }
                if((trackerBox.x+trackerBox.width)>mat1->cols)
                {
                    trackerBox.width=mat1->cols-trackerBox.x-2;
                }
                if((trackerBox.y+trackerBox.height)>mat1->rows)
                {
                    trackerBox.height=mat1->rows-trackerBox.y-2;
                }

                //define the center point.
                trackerPtCenter.x=trackerBox.x+trackerBox.width/2;
                trackerPtCenter.y=trackerBox.y+trackerBox.height/2;

                //define target size.
                trackerTargetSize.width=trackerBox.width;
                trackerTargetSize.height=trackerBox.height;

                bool bInitOkay=tracker.tracker_init(*mat1,trackerPtCenter,trackerTargetSize);
                if(bInitOkay==false)
                {
                    qDebug()<<"failed to init CSK Tracker.";
                }else{
                    bTrackerInitBox=true;
                }
            }else{
                //update the tracking result.
                bool bResult=tracker.tracker_update(*mat2,trackerPtCenter,trackerTargetSize);
                if(bResult)
                {
                    cv::Rect2d trackedBox;
                    trackedBox.x=trackerPtCenter.x-trackerTargetSize.width/2;
                    trackedBox.y=trackerPtCenter.y-trackerTargetSize.height/2;
                    trackedBox.width=trackerTargetSize.width;
                    trackedBox.height=trackerTargetSize.height;

                    //to avoid reaching the boundary to cause openCV faults.
                    if(trackedBox.x<0)
                    {
                        trackedBox.x=0;
                    }
                    if(trackedBox.y<0)
                    {
                        trackedBox.y=0;
                    }
                    if((trackedBox.x+trackedBox.width)>mat2->cols)
                    {
                        trackedBox.width=mat2->cols-trackedBox.x-2;
                    }
                    if((trackedBox.y+trackedBox.height)>mat2->rows)
                    {
                        trackedBox.height=mat2->rows-trackedBox.y-2;
                    }

                    //draw a rectangle on main img.
                    cv::rectangle(*mat1,trackerBox,cv::Scalar(0,255,0,255),2);
                    //draw a rectangle on aux img.
                    cv::rectangle(*mat2,trackedBox,cv::Scalar(0,255,0,255),2);

                    qDebug()<<"tracked okay:"<<trackedBox.x<<trackedBox.y<<trackedBox.width<<trackedBox.height;
                }else{
                    qDebug()<<"tracking failure detected.ReInit track box.";
                    bTrackerInitBox=false;
                }
            }
        }
            break;
        default:
            break;
        }

        //convert cv::Mat to QImage.
        QImage img1=cvMat2QImage(*mat1);
        emit this->ZSigNewImg1(img1);
        QImage img2=cvMat2QImage(*mat2);
        emit this->ZSigNewImg2(img2);
    }
    delete mat1;
    delete mat2;
}
