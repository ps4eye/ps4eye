export CPATH=../libusb/include/libusb-1.0
TARGET=ps4eye
SOURCE=main.cpp ps4eye.cpp
DEPENDS=$(grep '^dependency_libs=' ../libusb/lib/libusb-1.0.la | cut -f2 -d=)
LIBS=../libusb/lib/libusb-1.0.a ${DEPENDS}
CFLAGS+=-g

${TARGET}: ${SOURCE}
	g++ -c ${CFLAGS} $^
	g++ -o $@ $(^:%.cpp=%.o) ${LIBS}

clean:
	rm -rf ${TARGET} $(SOURCE:%.cpp=%.o)
