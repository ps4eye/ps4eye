all:
	./01_remove_UVC_headers.py && \
	./02_extract_images.py && \
	./03_gst-fdsrc-and-videoparse.sh

video:
	./video_convert.py && \
	for X in 1 2; do \
		avconv -y -r 1 -vcodec rawvideo -f rawvideo -pix_fmt yuyv422 -s 1280x800 -i video$${X}.raw -r 1 video$${X}.mp4; \
	done

clean:
	rm -f clean_payload.bin raw_image1.bin raw_image2.bin image1.png image2.png video1.raw video2.raw video1.mp4 video2.mp4
