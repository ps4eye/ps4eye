export SHELL=bash
export CPATH=../libusb/include/libusb-1.0
TARGET=ps4eye
SOURCE=main.cpp ps4eye.cpp
DEPENDS=$(shell grep '^dependency_libs=' ../libusb/lib/libusb-1.0.la | cut -f2 -d= | perl -pe "s|'||g")
LINKER_FLAGS=$(shell grep '^inherited_linker_flags=' ../libusb/lib/libusb-1.0.la | cut -f2 -d= | perl -pe "s|'||g")
LIBS+=../libusb/lib/libusb-1.0.a ${DEPENDS} ${LINKER_FLAGS}
CFLAGS+=-g

${TARGET}: ${SOURCE}
	g++ -c ${CFLAGS} $^
	g++ -o $@ $(^:%.cpp=%.o) ${LIBS}

clean:
	rm -rf ${TARGET} $(SOURCE:%.cpp=%.o)
