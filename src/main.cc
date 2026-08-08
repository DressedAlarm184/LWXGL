#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include "libLWXGL.h"
#include <sys/mman.h>
#include <alloca.h>
#include <unistd.h>
#include <cctype>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <algorithm>
#include <chrono>
#include <functional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <fstream>

#define EXPORT __attribute__((visibility("default")))

Display *display; Window window = None; GC gc; Atom wm_delete;
unsigned long colors[16] = {0}; int bgcol, win_w, win_h; XFontStruct* font;
int screen, mouse_x = 0, mouse_y = 0, mouse_down = 0, closing = 0;
Visual* visual; Colormap colormap; int depth; double elapsed_time = 0;

unsigned char pressed_keys[8] = {0};
unsigned char active_keycodes[8] = {0};

struct {
	int avg_wt[60] = {0};
	float fps;
	int enabled = 0;
	int target_fps;
	int target_ft;
} debug_metrics;

struct {
	int active = 0, type = 0;
	char* msg = NULL;
	void (*on_confirm)(const char*);
	int right_edge_x = 0;
	char input[151];
} active_modal_state;

struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} color_palette[16] = {
	{12,  12,  16},    // 00: Black
	{24,  43,  135},   // 01: Dark Blue
	{32,  130, 48},    // 02: Dark Green
	{18,  140, 145},   // 03: Dark Cyan
	{195, 45,  50},    // 04: Dark Red
	{123, 50,  107},   // 05: Dark Magenta
	{210, 105, 34},    // 06: Orange
	{155, 155, 150},   // 07: Light Gray
	{70,  72,  75},    // 08: Dark Gray
	{75,  115, 220},   // 09: Light Blue
	{80,  205, 80},    // 10: Light Green
	{65,  210, 210},   // 11: Light Cyan
	{235, 95,  95},    // 12: Light Red
	{220, 115, 195},   // 13: Light Magenta
	{235, 210, 50},    // 14: Yellow
	{245, 245, 245}    // 15: White
};

struct {
	operator Drawable() const {
		return pixmap;
	}

	Pixmap pixmap = None;
	bool scroll_enabled = false;
	int w, h, scroll = 0;
	bool frame_cb_after_elem = false;
	int scrollbar_color = -1;

	void new_bb(int width, int height) {
		if (pixmap != None) XFreePixmap(display, pixmap);
		w = width, h = height;
		pixmap = XCreatePixmap(display, window, w, h, depth);
		scroll = std::clamp(scroll, 0, std::max(0, h - win_h));
	}
} bb;

typedef struct {
	double target_time;
	void (*task)();
	double repeat_every;
} QueuedTask;

std::vector<QueuedTask> task_queue;

#define L(b)  ((b) & 0x0F)
#define H(b)  (((b) >> 4) & 0x0F)

#include "elements/structs.cc"
#include "elements/helpers.cc"
#include "elements/raster.cc"
#include "elements/tga.cc"
#include "elements/general.cc"

#include "events/handlers.cc"
#include "events/misc.cc"

#include "renderer/elements.cc"
#include "renderer/overlays.cc"
#include "renderer/main.cc"
#include "renderer/immediates.cc"
#include "renderer/opengl.cc"

#include "windowing/misc.cc"
#include "windowing/management.cc"
#include "windowing/config.cc"
