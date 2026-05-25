build: mkdir
	#!/bin/bash
	gcc -fPIC -shared -o build/libLWXGL.so lwxgl/main.c -lX11

build-demo demo: build mkdir
	#!/bin/bash
	gcc -o build/main demos/{{demo}}.c -Lbuild -lLWXGL

run: mkdir
	#!/bin/bash
	LD_LIBRARY_PATH=build build/main

run-cros: mkdir
	#!/bin/bash
	sommelier -X --scale=0.8 sh -c "xset fp+ /usr/share/fonts/X11/misc && LD_LIBRARY_PATH=build build/main"

mkdir:
	#!/bin/bash
	mkdir -p build