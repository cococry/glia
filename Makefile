CC=gcc
LIBS=-lglfw
CFLAGS+=${LIBS} -O3 -ffast-math
all: lib/glia.a
lib/glia.a: lib/glia.o
	ar cr lib/libglia.a lib/*.o
lib/glia.o: lib
	${CC} -c glia.c -o lib/glia.o ${CFLAGS}
lib:
	mkdir lib
clean:
	rm -r ./lib

install: all
	cp lib/libglia.a /usr/local/lib/ 
	cp -r include/glia /usr/local/include/ 

uninstall:
	rm -f /usr/local/lib/libglia.a
	rm -rf /usr/local/include/glia/

.PHONY: all test clean
