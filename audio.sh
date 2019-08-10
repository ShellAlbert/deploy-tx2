#!/bin/bash 
#gst-launch-1.0 rtspsrc latency=0 location="rtsp://192.168.137.12:554/user=admin&password=&channel=1&stream=0.sdp?real_stream" ! decodebin ! queue ! audioconvert ! alsasink device="plughw:USBSA,DEV=0"
#gst-launch-1.0 rtspsrc latency=0 location="rtsp://192.168.137.12:554/user=admin&password=&channel=1&stream=0.sdp?real_stream" ! decodebin ! audioconvert ! alsasink device="plughw:USBSA,DEV=0"


#test.pcm should be import in audacity with 8khz/signed 16-bit pcm/little endian/1 channel
gst-launch-1.0 rtspsrc latency=0 location="rtsp://192.168.137.12:554/user=admin&password=&channel=1&stream=0.sdp?real_stream" ! decodebin ! audioconvert ! filesink location="test.pcm"

