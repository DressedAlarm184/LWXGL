build:
	#!/bin/bash
	g++ -fPIC -shared -o libLWXGL.so src/main.cc -lX11

install:
	#!/bin/bash
	sudo cp libLWXGL.so /usr/local/lib
	sudo cp src/libLWXGL.h /usr/local/include
	sudo ldconfig