#!/bin/bash

#monitor.sh for p16 project.
#this script will be launched when OS startup.
#it monitor all program pid files,launch execute files once detected failure.
#1811543668@qq.com  August 11,2019.

BASEDIR=/home/zhangshaoyan/p16

#$@ all parameters,$0 app name,$1 parameter1,$2 parameter2,...
function addLog2File()
{
	echo $@

	LogDir=`pwd`/log

	#generate log file like format 20190811.log
	LogFileName=$LogDir/p16.`date "+%Y%m%d"`.log
	#echo $LogFileName
	if [ ! -f $LogFileName ];then
		touch $LogFileName
	fi

	#keep file only hold 100 lines.
	LineCount=`wc -l $LogFileName  | awk '{print $1}'`
	if [ $LineCount -gt 100 ];then
		echo "-gt 100"
		sed -i '$d' $LogFileName
	fi
	#if line is zero,then insert one manual for sed 1i works.(it needs 1 line at least).
	if [ $LineCount -eq 0 ];then
		echo "p16 log file generated:$LogFileName" > $LogFileName
	fi

	#prepend log to the first line.
	sed -i "1i\ $1" $LogFileName


	#keep log directoy less than 100MB size.
	#if exceeds then delete all logs.
	Size=`du -sm $LogDir | awk '{print $1}'`
	if [ $Size -gt 100 ]; then
		echo "warning!!!"
		echo "log directory size exceeds predefined,so truncate it!"
		echo "all data will lost!"
		rm -rf $LogDir
	fi
	return 0
}

#make sure we run this with root priority.
if [ `whoami` != "root" ];then
	echo "<error>: latch me with root priority please."
	exit -1
fi

#only allow one instance to run.
PID=`cat /tmp/monitor.pid`
kill -0 $PID
if [ $? -eq 0 ];then
      echo "only allow one instance to run,so I quit."
      exit 0
fi

cd $BASEDIR

#create log directory.
if [ ! -d "log" ];then
	mkdir log
fi

#elevate privilege.
if [ -e "/dev/ttyTHS2" ];then
	addLog2File "elevate privilege /dev/ttyTHS2."
	chmod 777 /dev/ttyTHS2
fi

#dump myself pid to file.
if [ -f "/tmp/monitor.pid" ];then
	echo "only one instance can be run!"
	exit -1
fi

#clean pid files before running.
#for manual debug.
#rm -rf /tmp/zcambridge.pid
rm -rf /tmp/p16.pid

#dump my pid to file.
echo $$ > /tmp/monitor.pid

#backup previous p16 log file.
cp -f p16.log  p16.log.1

#the main loop.
while true
do
    #check the usb(yuv-h264 rtsp) video server.
    #the middle camera (big view).
#    bStartCamBridge=0
#    if [ -f "/tmp/zcambridge.pid" ];then
#        PID=`cat /tmp/zcambridge.pid`
#        kill -0 $PID
#        if [ $? -eq 0 ];then
#            echo "<okay>:cambridge pid detect okay."
#        else
#	    kill -9 `cat /tmp/zcambridge.pid`
#	    bStartCamBridge=1
#        fi
#    else
#	    bStartCamBridge=1
#    fi
#    if [ $bStartCamBridge -eq 1 ];then
#	    addLog2File "/tmp/zcambridge.pid detected failed,launch it again."
#	    sleep 5
#	    ./zcambridge.bin "v4l2src device=/dev/video0 ! video/x-raw,width=(int)640,height=(int)480,framerate=(fraction)30/1 ! queue ! nvvidconv ! omxh264enc ! rtph264pay name=pay0 pt=96 config-interval=1" &
#    fi

    #check the p16(audio/json/uart) server.
    bStartP16=0
    if [ -f "/tmp/p16.pid" ];then
        PID=`cat /tmp/p16.pid`
        sudo kill -0 $PID
        if [ $? -eq 0 ];then
            echo "<okay>:LizardTx2 pid detect okay."
        else
	    kill -9 `cat /tmp/p16.pid`
	    bStartP16=1
        fi
    else
	    bStartP16=1
    fi
    if [ $bStartP16 -eq 1 ];then
	    addLog2File "/tmp/p16.pid detected failed,launch it again."
	    sleep 5
	    ./p16_no_gui.bin &
    fi

    #check periodly every 10 seconds.
    sleep 10
done

#the end of file.

