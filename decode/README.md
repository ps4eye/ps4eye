
## Extracting Individual Frames using Total Data Center

1. Set "Class View" in the main view.
2. Use the Quick Filters to show only the payloads of the same
   frame (i.e. same "frame id": the 4 bytes starting with 0xc).
   Use the "Exclude indices before/after" to isolate one
   "frame".
3. Disable the control transfers by using "Show except
   endpoint" on one of them.
4. Export to payload.bin, only full matches and visible records.

## Converting Frames to PNG files

Ensure that libav-tools (avconv) is installed. Export a frame
using the above instructions. Extract two image files,
image1.png and image2.png, by running `make`:

```
$ make
./01_remove_UVC_headers.py && \
	./02_extract_images.py && \
	./03_gst-fdsrc-and-videoparse.sh
File size: 4912988, (0x004af75c)
File size: 4911428, (0x004af144)
avconv version 0.8.9-6:0.8.9-0ubuntu0.13.10.1, Copyright (c) 2000-2013 the Libav developers
  built on Nov  9 2013 19:09:46 with gcc 4.8.1
[rawvideo @ 0x902cc0] Estimating duration from bitrate, this may be inaccurate
Input #0, rawvideo, from 'raw_image1.bin':
  Duration: N/A, start: 0.000000, bitrate: N/A
    Stream #0.0: Video: rawvideo, yuyv422, 1280x800, 25 tbr, 25 tbn, 25 tbc
Incompatible pixel format 'yuyv422' for codec 'png', auto-selecting format 'rgb24'
[buffer @ 0x903680] w:1280 h:800 pixfmt:yuyv422
[avsink @ 0x90c8e0] auto-inserting filter 'auto-inserted scaler 0' between the filter 'src' and the filter 'out'
[scale @ 0x90cfc0] w:1280 h:800 fmt:yuyv422 -> w:1280 h:800 fmt:rgb24 flags:0x4
Output #0, image2, to 'image1.png':
  Metadata:
    encoder         : Lavf53.21.1
    Stream #0.0: Video: png, rgb24, 1280x800, q=2-31, 200 kb/s, 90k tbn, 25 tbc
Stream mapping:
  Stream #0:0 -> #0:0 (rawvideo -> png)
Press ctrl-c to stop encoding
Error while decoding stream #0:0
frame=    1 fps=  0 q=0.0 Lsize=      -0kB time=0.04 bitrate=  -4.4kbits/s    
video:1909kB audio:0kB global headers:0kB muxing overhead -100.001126%
avconv version 0.8.9-6:0.8.9-0ubuntu0.13.10.1, Copyright (c) 2000-2013 the Libav developers
  built on Nov  9 2013 19:09:46 with gcc 4.8.1
[rawvideo @ 0x1259cc0] Estimating duration from bitrate, this may be inaccurate
Input #0, rawvideo, from 'raw_image2.bin':
  Duration: N/A, start: 0.000000, bitrate: N/A
    Stream #0.0: Video: rawvideo, yuyv422, 1280x800, 25 tbr, 25 tbn, 25 tbc
Incompatible pixel format 'yuyv422' for codec 'png', auto-selecting format 'rgb24'
[buffer @ 0x125a680] w:1280 h:800 pixfmt:yuyv422
[avsink @ 0x12638e0] auto-inserting filter 'auto-inserted scaler 0' between the filter 'src' and the filter 'out'
[scale @ 0x1263fc0] w:1280 h:800 fmt:yuyv422 -> w:1280 h:800 fmt:rgb24 flags:0x4
Output #0, image2, to 'image2.png':
  Metadata:
    encoder         : Lavf53.21.1
    Stream #0.0: Video: png, rgb24, 1280x800, q=2-31, 200 kb/s, 90k tbn, 25 tbc
Stream mapping:
  Stream #0:0 -> #0:0 (rawvideo -> png)
Press ctrl-c to stop encoding
Error while decoding stream #0:0
frame=    1 fps=  0 q=0.0 Lsize=      -0kB time=0.04 bitrate=  -4.4kbits/s    
video:729kB audio:0kB global headers:0kB muxing overhead -100.002946%
```
