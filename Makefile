.PHONY: build install

O ?= 2

build:
	g++ -std=gnu++17 -D_GNU_SOURCE -fPIC -shared -O${O} -o libLWXGL.so source/main.cc -lX11 -lGL -fvisibility=hidden

install:
	cp libLWXGL.so /usr/local/lib
	cp source/LWXGL.h /usr/local/include
	ldconfig
