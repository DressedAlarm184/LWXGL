#include "libLWXGL.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <cmath>

Display *display; Window window = None; GC gc; Pixmap bb;
unsigned long colors[256] = {0}; int bgcol, win_w, win_h;
int screen, mouse_x = 0, mouse_y = 0, mouse_down = 0, closing = 0;
Atom wm_delete; XEvent event; Pixmap stipple; XFontStruct* font;

struct {
	int avg_wt[60] = {0};
	float fps;
	int active = 0, enabled = 0;
} debug_metrics;

static const struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} color_pallete[16] = {
	{0,   0,   0},     // 00: Black
	{3,   3,   173},   // 01: Dark Blue
	{0,   170, 0},     // 02: Dark Green
	{0,   168, 168},   // 03: Dark Cyan
	{186, 6,   6},     // 04: Dark Red
	{168, 0,   168},   // 05: Dark Magenta
	{230, 126, 34},    // 06: Orange
	{168, 168, 168},   // 07: Light Gray
	{85,  87,  83},    // 08: Dark Gray
	{87,  87,  255},   // 09: Light Blue
	{85,  255, 85},    // 10: Light Green
	{96,  240, 240},   // 11: Light Cyan
	{255, 85,  85},    // 12: Light Red
	{240, 84,  240},   // 13: Light Magenta
	{244, 242, 54},    // 14: Yellow
	{255, 255, 255}    // 15: White
};

#define L(b)   ((b) & 0x0F)
#define H(b)  (((b) >> 4) & 0x0F)

#include "elements.cc"
#include "windows.cc"
#include "renderer.cc"
#include "events.cc"