#!/usr/bin/env python
#
# Extract images from the payload cleaned-up from the UVC-like headers
#
# Copyright (C) 2014  Antonio Ospite <ao2@ao2.it>
#
# This program is free software. It comes without any warranty, to
# the extent permitted by applicable law. You can redistribute it
# and/or modify it under the terms of the Do What The Fuck You Want
# To Public License, Version 2, as published by Sam Hocevar. See
# http://sam.zoy.org/wtfpl/COPYING for more details.

# The data has this format for each "row":
# UNKNOWN HEADER     - 32 bytes
# UNKNOWN DATA       - 64 bytes
# RAW IMAGE1 DATA    - 1280 * 2 bytes
# RAW IMAGE2 DATA    - 1280 * 2 bytes
# OTHER UNKNOWN DATA - 840 bytes
#
# There are 800 rows for each payload.
# The images are hence 1280 * 800 * 2 bytes, at 2 bytes per pixel.

import struct
from array import array

def dump(f, fmt):
    length = struct.calcsize(fmt)
    data = f.read(length)
    value = struct.unpack_from(fmt, data)
    return value

if __name__ == "__main__":
    f = open("clean_payload.bin", "rb")
    image1 = open("raw_image1.bin", "wb")
    image2 = open("raw_image2.bin", "wb")

    f.seek(0, 2)
    file_size = f.tell()
    f.seek(0)

    print "File size: %d, (0x%08x)" % (file_size, file_size)

    row_image1 = 0
    row_image2 = 0
    row = 0
    null_row = chr(0x00) * 1280*2

    while f.tell() < file_size:
        row += 1
        unknown_header = f.read(32)
        unknown_data = f.read(64)
        #print ''.join('{:02x} '.format(x) for x in array("B",unknown_header))
        #print ''.join('{:02x} '.format(x) for x in array("B",unknown_data)[0::2])

        image1_data = f.read(1280*2)
        if image1_data != null_row and row_image1 < 800:
            row_image1 += 1
            image1.write(image1_data)

        image2_data = f.read(1280*2)
        if image2_data != null_row and row_image2 < 800:
            row_image2 += 1
            image2.write(image2_data)

        other_unknown_data = f.read(840)
        #print ''.join('{:02x} '.format(x) for x in array("B",other_unknown_data))

    image1.close()
    image2.close()
