#!/usr/bin/env python
import usb.core
import usb.util
import sys
import time

# check if initialized device already exists
dev = usb.core.find(idVendor=0x05a9, idProduct=0x058a)
if dev is None:
    print 'PS4 camera not initialized'
    sys.exit()

# set the active configuration. With no arguments, the first
# configuration will be the active one
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
        data      = commands.read(dlength)
        if len(data) != dlength:
            print "stopping after reading data of size %d, expecting %d" % (len(data), dlength)
            break
        if wtype == 0xC0:
            #print "not sending request..."
            dev_data = dev.ctrl_transfer(wtype, wrequest, wvalue, windex, dlength)
            if data != dev_data: print "unexpected request data: %s" % (''.join('{:02x} '.format(x) for x in dev_data))
        else:
            if wtype != 0x40: print "unknown type, assuming write"
            dev_length = dev.ctrl_transfer(wtype, wrequest, wvalue, windex, data)
            if dev_length != dlength: print "unexpected write of %d/%d bytes" % (dev_length, dlength)
            time.sleep(0.01)

commands.close()
print "PS4 camera commands done"
