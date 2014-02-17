#!/usr/bin/env bash

set -e

WIDTH=1280
HEIGHT=800

# using avconv due to issues with gstreamer creating null file
# second image maybe luma?
# - ew
FORMAT="yuyv422"
#FORMAT="gray16be"
#FORMAT="y400a"
for X in 1 2; do
    avconv \
        -vframes 1 -vcodec rawvideo -f rawvideo -pix_fmt ${FORMAT} \
        -s ${WIDTH}x${HEIGHT} -i raw_image${X}.bin -vcodec png image${X}.png
done

# -original gstreamer code by ao2
# FORMAT="yuy2"

# cat raw_image1.bin |
# gst-launch-1.0 \
#   fdsrc blocksize=$(($WIDTH*$HEIGHT*2)) num-buffers=1 ! \
#   videoparse format=$FORMAT width=$WIDTH height=$HEIGHT framerate=1 ! \
#   queue min-threshold-buffers=1 ! \
#   videoconvert ! videorate ! pngenc ! filesink location=image1.png

# # Not sure about this one.

# # Maybe it is indeed a 16bit depth map but in order to appreciate the
# # precision visually we should use some pseudo-color representation (heatmap)
# # like done in the kinect programs. Or maybe save to a format other than png
# # (some HDR?) in order to see the higher depth.
# FORMAT="gray16-be"

# cat raw_image2.bin |
# gst-launch-1.0 \
#   fdsrc blocksize=$(($WIDTH*$HEIGHT*2)) num-buffers=1 ! \
#   videoparse format=$FORMAT width=$WIDTH height=$HEIGHT framerate=1 ! \
#   queue min-threshold-buffers=1 ! \
#   videoconvert ! videorate ! pngenc ! filesink location=image2.png
