record: record.C
	g++ record.C -o record -lusb-1.0 -I/usr/include/libusb-1.0

clean:
	rm record
