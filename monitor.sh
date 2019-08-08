#!/bin/bash

#check 2 pids.
#launch them again if not detected.

#make sure we run this with root priority.
if [ `whoami` != "root" ];then
	echo "<error>: latch me with root priority please."
	exit -1
fi

chmod 777 /dev/ttyTHS2

#the main loop.
while true
do
    #make sure we have one usb camera connected at least.
    CAM_NUM=`ls -l /dev/video* | wc -l`
    if [ $CAM_NUM -lt 1 ];then
        echo "<error>: I need one usb camera at least."
    	sleep 5
    fi

    #check the usb(yuv-h264 rtsp) video server.
    if [ -f "/tmp/ZUsbCamRtspServer.pid" ];then
        PID=`cat /tmp/ZUsbCamRtspServer.pid`
        kill -0 $PID
        if [ $? -eq 0 ];then
            echo "<okay>:UsbCamRtspServer pid detect okay."
        else
            echo "<error>:UsbCamRtspServer pid detect error,launch it."
	    ZUsbCamRtspServer.bin &
        fi
    else
	    echo "<error>:UsbCamRtspServer pid detect error,launch it."
	    ZUsbCamRtspServer.bin &
    fi

    #check the lizard-tx2(audio/json/uart) server.
    if [ -f "/tmp/ZLizardTx2.pid" ];then
        PID=`cat /tmp/ZLizardTx2.pid`
        kill -0 $PID
        if [ $? -eq 0 ];then
            echo "<okay>:LizardTx2 pid detect okay."
        else
            echo "<error>:LizardTx2 pid detect error,launch it."
            ZLizardTx2.bin &
        fi
    else
            echo "<error>:LizardTx2 pid detect error,launch it."
            ZLizardTx2.bin &
    fi

    #check periodly every 10 seconds.
    sleep 10
done

#the end of file.

