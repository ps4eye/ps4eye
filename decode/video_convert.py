#!/usr/bin/env python

import struct
#from array import array

def dump(f, fmt):
    length = struct.calcsize(fmt)
    data = f.read(length)
    value = struct.unpack_from(fmt, data)
    return value

if __name__ == "__main__":
    f = open("video.bin", "rb")
    v1 = open("video1.raw", "wb")
    v2 = open("video2.raw", "wb")

    null_row = chr(0x00) * 1280*2

    f.seek(0, 2)
    file_size = f.tell()
    f.seek(0)
    frame_count = 0

    while f.tell() < file_size:
        # All chunks in a frame in the isoc stream start with the same "frame id"
        pos = f.tell()
        frame_id = dump(f, "<I")[0]
        eof_frame_id = frame_id | (0x02<<8)
        frame = ''
        f.seek(pos)
        frame_count += 1
        print "frame %d id %s eof %s at %d" % (frame_count,hex(frame_id),hex(eof_frame_id),pos)
        frame_head = frame_id & 0x0000FFFF
        frame_next = (frame_id & 0x0000FFFF) ^ 0x100
        #print "head %s and next %s" % (hex(frame_head),hex(frame_next))
        eof_pos = 0

        while f.tell() < file_size:
            pos = f.tell()
            data = f.read(4)
            tmp_frame_id = struct.unpack_from("<I", data)[0]
            eof_pos_diff = pos-eof_pos

            # check for known payload sizes due to insufficient heuristic for uvc headers
            if eof_pos > 0 and \
               eof_pos_diff in [872,5992,6244,7016,8040,13160,13412,18268,21340,22364,24412,24676,30556,30820,34652,35676]:
                frame_head = frame_id & 0x0000FFFF
                tmp_head = (tmp_frame_id & 0x0000FFFF) ^ 0x100
                #print "at eof, comparing %s to %s" % (hex(frame_head),hex(tmp_head))
                zeros = tmp_frame_id & 0xF0F0F0F0

                if frame_head == tmp_head and zeros != 0:
                    f.seek(pos)
                    print "new frame found at %d with diff %d" % (pos,pos-eof_pos)
                    break

            if tmp_frame_id == eof_frame_id:
                eof_pos = pos
                print "eof found at %d" % (eof_pos)

            # When a new chunk is met, skip the timestamps and read the next 4
            # useful bytes
            if tmp_frame_id == frame_id or tmp_frame_id == eof_frame_id:
                f.read(8)
                data = f.read(4)

            frame += data

        denom = 32+64+(1280*4)+840

        #print "Found frame! %d %d" % (len(frame),denom)
        #frame = frame_str[::-1]

        row_image1 = 0
        row_image2 = 0

        for row in range(len(frame)/denom):
            offset = row * denom
            unknown_header = frame[offset:offset+32]
            offset += 32
            unknown_data = frame[offset:offset+64]
            offset += 64
            #print ''.join('{:02x} '.format(x) for x in array("B",unknown_header))
            #print ''.join('{:02x} '.format(x) for x in array("B",unknown_data)[0::2])

            image1_data = frame[offset:offset+(1280*2)]
            if image1_data != null_row and row_image1 < 800:
                row_image1 += 1
                v1.write(image1_data)
            offset += 1280*2

            image2_data = frame[offset:offset+(1280*2)]
            if image2_data != null_row and row_image2 < 800:
                row_image2 += 1
                v2.write(image2_data)
            offset += 1280*2

            other_unknown_data = frame[offset:offset+840]
            offset += 840
