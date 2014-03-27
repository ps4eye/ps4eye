export CPATH=/usr/local/include/libusb-1.0
TARGET=ps4eye
SOURCE=main.cpp ps4eye.cpp
LIBS=-L/usr/local/lib -lusb-1.0
CFLAGS+=-g

${TARGET}: ${SOURCE}
	g++ -c ${CFLAGS} $^
	g++ -o $@ $(^:%.cpp=%.o) ${LIBS}

clean:
	rm -rf ${TARGET} $(SOURCE:%.cpp=%.o)
