#!/usr/bin/env python
#
# Split the payload extracted from the .tdc file in the separate isoc chunks
#
# Copyright (C) 2014  Antonio Ospite <ao2@ao2.it>
#
# This program is free software. It comes without any warranty, to
# the extent permitted by applicable law. You can redistribute it
# and/or modify it under the terms of the Do What The Fuck You Want
# To Public License, Version 2, as published by Sam Hocevar. See
# http://sam.zoy.org/wtfpl/COPYING for more details.

# This can help to simulate the actual transfer to derive an interactive
# "cross-chunk" algorithm to extract the image data, to use when the USB
# problems will be solved.

import struct

def dump(f, fmt):
    length = struct.calcsize(fmt)
    data = f.read(length)
    value = struct.unpack_from(fmt, data)
    return value

if __name__ == "__main__":
    f = open("payload", "rb")

    f.seek(0, 2)
    file_size = f.tell()
    f.seek(0)

    print "File size: %d, (0x%08x)" % (file_size, file_size)

    frame_id = dump(f, "<I")[0]
    f.seek(0)

    chunk_index = -1

    while f.tell() < file_size:
        data = f.read(4)
        tmp_frame_id = struct.unpack_from("<I", data)[0]

        if tmp_frame_id == frame_id:
            chunk_index += 1
            g = open("chunk_%04d.bin" % chunk_index, "wb")

        g.write(data)
