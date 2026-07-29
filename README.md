# LWXGL - Lightweight X11 Graphics Library
LWXGL is a graphics library and simple UI toolkit for the X Window System. It provides standard UI elements like text labels, buttons, inputs, and more, along with raster image canvases, support for TGA and XBM images, drawing primitives, event handling, window resizing and scrolling, OpenGL integration, and more.

## Dependencies

- X11 development headers and runtime library
- OpenGL development headers and runtime library
- GCC capable of full C++17 support
- The C and C++ language standard libraries

## Building & Installing

Run the `make` command at the project root to build the `libLWXGL.so` compiled library file. Run `make install` to copy the shared object to `/usr/local/lib` and the header file to `/usr/local/include`. You may need to provide superuser privileges to install the library. Attach the `-lLWXGL` flag to your compiler command to link the library.

## Example Program

This is a minimal example program to get you started. Compile with: `gcc -o example example.c -lLWXGL`

```c
#include <libLWXGL.h>

void on_every_frame(int frame, float dt) {
	static float circle_x = 50, circle_speed = 150;
	ImmediateTextF(10, 10, CLR_WHITE, "Elapsed time: %.1f seconds", GetElapsedTime());
	circle_x += circle_speed * dt;
	if (circle_x >= 336 || circle_x <= 0) circle_speed *= -1;
	ImmediateEllipse(circle_x, 80, 64, 64, CLR_WHITE, CLR_ORANGE);
}

int main() {
	CreateWindow(400, 250, "Example Window", CLR_BLUE);
	CreateButton(0, 270, 210, 120, 30, 0xF4, 0xF5, 0xF2, "Quit", DeleteWindow);
	MainWindowLoop(60, on_every_frame);
	TerminateWindow();
}
```

## Documentation

LWXGL does not have any documentation. Please reference the source code and headers for details on how to use the library.
