#!/bin/bash
gcc zlaunch.c -o zcambridge.bin $(pkg-config --cflags --libs gstreamer-1.0 gstreamer-rtsp-server-1.0)
