#!/usr/bin/env python
import usb.core
import usb.util
import sys
import time
from array import array

#import cv2

# check if initialized device already exists
dev = usb.core.find(idVendor=0x05a9, idProduct=0x058a)
if dev is None:
    print 'PS4 camera not initialized'
    sys.exit()

# already configured from v4l
#dev.set_configuration()

commands=open(sys.argv[1],"rb")

print "PS4 camera command file %s opened" % (sys.argv[1])

while 1:
    setup = commands.read(8)
    setup_len = len(setup)
    if setup_len != 8:
        if setup_len != 0: print "stopping after reading setup of size %d" % (len(setup))
        break
    else:
        wtype     = ord(setup[0])
        wrequest  = ord(setup[1])
        wvalue    = ord(setup[2]) + ord(setup[3])*0x100
        windex    = ord(setup[4]) + ord(setup[5])*0x100
        dlength   = ord(setup[6]) + ord(setup[7])*0x100
        print "setupTxn: %02X %02X %04X %04X %04X (%d)" % (wtype, wrequest, wvalue, windex, dlength, dlength)
        data      = array("B",commands.read(dlength))
        if len(data) != dlength:
            print "stopping after reading data of size %d, expecting %d" % (len(data), dlength)
            break
        if wtype == 0xC0:
            dev_data = dev.ctrl_transfer(wtype, wrequest, wvalue, windex, dlength)
            if data != dev_data:
                print "unexpected request data:\n expected: %s\n received: %s\n zip diff: %s" % (
                    ''.join('{:02x} '.format(x) for x in data),
                    ''.join('{:02x} '.format(x) for x in dev_data),
                    ''.join(['{:02x} '.format(b) if a != b else '.. ' for a, b in zip(data, dev_data)])
                )
        else:
            if wtype != 0x40: print "unknown type, assuming write"
            dev_length = dev.ctrl_transfer(wtype, wrequest, wvalue, windex, data)
            if dev_length != dlength: print "unexpected write of %d/%d bytes" % (dev_length, dlength)
            time.sleep(0.01) # Bus error if we don't sleep... ?

commands.close()
print "PS4 camera commands done"

for i,d in enumerate(dev):
    print "device %d:" % (i)
    intf = d[(1,1)]
    for j,e in enumerate(intf):
        print "endpoint %d : %x" % (j,e.bEndpointAddress)

# while(True):
#     cap = cv2.VideoCapture(int(sys.argv[2]))

#     # Capture frame-by-frame
#     ret, frame = cap.read()

#     try:
#         # Our operations on the frame come here
#         #rgb = cv2.cvtColor(frame, cv2.COLOR_YUV2RGB_Y422)

#         # Display the resulting frame
#         cv2.imshow('frame',frame)
#         print "got frame!"
#     except:
#         print "no frame..."

#     if cv2.waitKey(1) & 0xFF == ord('q'):
#         break

if dev.is_kernel_driver_active(0) is True:
    print "detaching kernel driver..."
    dev.detach_kernel_driver(0)

dev.set_interface_altsetting(1,1)

try:
    while 1:
        #time.sleep(0.01)
        dev.set_interface_altsetting(1,1)
        data = []
        try:
            data = dev.read(0x81,5571968,1,1000)
            print "*** received %d bytes" % (len(data))
        except usb.core.USBError as e:
            print "*** usb transfer timeout %s" % (e.args)
            continue
except:
    print "aborting camera read"

if dev.is_kernel_driver_active(0) is False:
    print "attaching kernel driver..."
    dev.attach_kernel_driver(0)
