all:
	./01_remove_UVC_headers.py && \
	./02_extract_images.py && \
	./03_gst-fdsrc-and-videoparse.sh

clean:
	rm -f clean_payload.bin raw_image1.bin raw_image2.bin image1.png image2.png
