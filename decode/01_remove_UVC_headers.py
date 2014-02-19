#!/usr/bin/env python
#
# Remove the UVC-like headers from chunks from the payload from the .tdc file
#
# Copyright (C) 2014  Antonio Ospite <ao2@ao2.it>
#
# This program is free software. It comes without any warranty, to
# the extent permitted by applicable law. You can redistribute it
# and/or modify it under the terms of the Do What The Fuck You Want
# To Public License, Version 2, as published by Sam Hocevar. See
# http://sam.zoy.org/wtfpl/COPYING for more details.

# The UVC-like headers have the format:
#
# FRAME ID    - 4bytes
# TIMESTAMP 1 - 4 bytes unsigned integer (mixed endian?)
# TIMESTAMP 2 - 4 bytes unsigned integer (mixed endian?)

# By removing all these from a complete payload extracted from the .tdc file we
# get the actual data used by the PS4.

import struct

def dump(f, fmt):
    length = struct.calcsize(fmt)
    data = f.read(length)
    value = struct.unpack_from(fmt, data)
    return value

if __name__ == "__main__":
    f = open("payload.bin", "rb")
    g = open("clean_payload.bin", "wb")

    f.seek(0, 2)
    file_size = f.tell()
    f.seek(0)

    print "File size: %d, (0x%08x)" % (file_size, file_size)

    # All chunks in a frame in the isoc stream start with the same "frame id"
    # see http://www.usb.org/developers/devclass_docs/USB_Video_Class_1_5.zip
    frame_id = dump(f, "<I")[0]
    eof_frame_id = frame_id | (0x02<<8)
    f.seek(0)

    print "frame id %s eof %s" % (hex(frame_id),hex(eof_frame_id))

    while f.tell() < file_size:
        data = f.read(4)
        tmp_frame_id = struct.unpack_from("<I", data)[0]

        # When a new chunk is met, skip the timestamps and read the next 4
        # useful bytes
        if tmp_frame_id == frame_id or tmp_frame_id == eof_frame_id:
            f.read(8)
            # timestamp1_high = dump(f, "<H")
            # timestamp1_low = dump(f, "<H")
            # #print "timestamp1: ", (timestamp1_high << 16) + timestamp1_low
            # timestamp2_high = dump(f, "<H")
            # timestamp2_low = dump(f, "<H")
            # #print "timestamp2: ", (timestamp2_high << 16) + timestamp2_low

            data = f.read(4)

        g.write(data)
