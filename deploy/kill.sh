#!/bin/bash

#make sure we run this with root priority.
if [ `whoami` != "root" ];then
	echo "<error>: latch me with root priority please."
	exit -1
fi

#kill monitor script firstly.
if [ -e "/tmp/monitor.pid" ];then
	kill -9 `cat /tmp/monitor.pid`
	rm -rf /tmp/monitor.pid
fi

#kill cambridge.
if [ -e "/tmp/zcambridge.pid" ];then
	kill -9 `cat /tmp/zcambridge.pid`
	rm -rf /tmp/zcambridge.pid
fi

#kill p16.
if [ -e "/tmp/p16.pid" ];then
	kill -9 `cat /tmp/p16.pid`
	rm -rf /tmp/p16.pid
fi

#the end of file.
