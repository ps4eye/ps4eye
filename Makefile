.PHONY: libusb record

all: libusb record

libusb:
	if [ ! -d libusb ]; then \
		git submodule init; \
	fi;
	git submodule update --remote;
	( \
		cd libusb && \
		./autogen.sh --prefix=`pwd` && \
		make clean && make -j4 && make install \
	)

record:
	(cd record && make clean && make)
