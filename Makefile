.PHONY: build install

O ?= 2

build:
	g++ -std=gnu++17 -D_GNU_SOURCE -fPIC -shared -O${O} -o libLWXGL.so src/main.cc -lX11 -fvisibility=hidden

install:
	cp libLWXGL.so /usr/local/lib
	cp src/libLWXGL.h /usr/local/include
	ldconfig
