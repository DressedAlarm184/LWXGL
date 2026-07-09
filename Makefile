.PHONY: build install

O ?= 2

build:
	g++ -fPIC -shared -O${O} -o libLWXGL.so src/main.cc -lX11 -fvisibility=hidden

install:
	cp libLWXGL.so /usr/local/lib
	cp src/libLWXGL.h /usr/local/include
	ldconfig
