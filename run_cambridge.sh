#!/bin/bash

#./zcambridge.bin "v4l2src device=/dev/video0 ! video/x-raw,width=(int)640,height=(int)480,framerate=(fraction)30/1 ! queue ! nvvidconv ! omxh264enc ! rtph264pay name=pay0 pt=96"
./zcambridge.bin "v4l2src device=/dev/video0 ! video/x-raw,width=(int)640,height=(int)480,framerate=(fraction)30/1 ! nvvidconv ! omxh264enc insert-sps-pps=true ! rtph264pay name=pay0 pt=96"
