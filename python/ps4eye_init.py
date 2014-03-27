#!/usr/bin/env python
import usb.core
import usb.util
import sys

# check if initialized device already exists
dev = usb.core.find(idVendor=0x05a9, idProduct=0x058a)
if dev is not None:
    print('PS4 camera already initialized')
    sys.exit()

# find uninitialized device
dev = usb.core.find(idVendor=0x05a9, idProduct=0x0580)
if dev is None:
    print('PS4 camera not found')
    sys.exit()

# set the active configuration. With no arguments, the first
# configuration will be the active one
dev.set_configuration()

# helper function for chunking a file
def read_chunks(infile, chunk_size):
    while True:
        chunk = infile.read(chunk_size)
        if chunk:
            yield chunk
        else:
            return

chunk_size=512
index=0x14
value=0
firmware=open("firmware.bin","rb")

# transfer 512b chunks of the firmware
for chunk in read_chunks(firmware, chunk_size):
    ret = dev.ctrl_transfer(0x40, 0x0, value, index, chunk)
    value+=chunk_size
    if value>=65536:
        value=0
        index+=1
    if len(chunk)!=ret:
        print("sent %d/%d bytes" % (ret,len(chunk)))

# command reboots device with new firmware and product id
try:
    ret = dev.ctrl_transfer(0x40, 0x0, 0x2200, 0x8018, [0x5b])
except:
    print('PS4 camera firmware uploaded and device reset')
