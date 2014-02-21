export CPATH=/usr/include/libusb-1.0
TARGET=ps4eye
SOURCE=main.cpp ps4eye.cpp
LIBS=-lusb-1.0

${TARGET}: ${SOURCE}
	g++ -c $^
	g++ -o $@ $(^:%.cpp=%.o) ${LIBS}

clean:
	rm -rf ${TARGET} $(SOURCE:%.cpp=%.o)
