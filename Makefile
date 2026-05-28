.PHONY: build install

build:
	g++ -fPIC -shared -O2 -o libLWXGL.so src/main.cc -lX11 -fvisibility=hidden

install:
	sudo cp libLWXGL.so /usr/local/lib
	sudo cp src/libLWXGL.h /usr/local/include
	sudo ldconfig