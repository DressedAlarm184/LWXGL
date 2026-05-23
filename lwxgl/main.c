#include "lwxgl.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

Display *display; Window window; GC gc; Pixmap back_buffer;
unsigned long colors[256] = {0}; int bgcol;
int screen, cursor_x = -1, cursor_y = -1, mouse_x = 0, mouse_y = 0, mouse_down = 0, closing = 0;
Atom wm_delete; XEvent event; Pixmap stipple; XFontStruct* font;

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

typedef struct {
	int x, y, color;
	const char *text;
} TextElement;

typedef struct {
	int type;
	void *elem;
} Element;

Element *elements[512];

int GCreateWindow(int w, int h, char* name, int bgcolor) {
	display = XOpenDisplay(NULL);
	if (display == NULL) return 1;

	font = XLoadQueryFont(display, "9x15");
	if (!font) {
		XCloseDisplay(display);
		return 2;
	}

	screen = DefaultScreen(display);
	Colormap colormap = DefaultColormap(display, screen);
	XColor dummy_exact, xcolor;

	for (int i = 0; i < 16; i++) {
		xcolor.red   = color_pallete[i].r * 257;
		xcolor.green = color_pallete[i].g * 257;
		xcolor.blue  = color_pallete[i].b * 257;
		xcolor.flags = DoRed | DoGreen | DoBlue;
		if (XAllocColor(display, colormap, &xcolor)) {
			colors[i] = xcolor.pixel;
		} else {
			XFreeFont(display, font);
			XCloseDisplay(display);
			return 127 + i;
		}
	}

	srand(time(NULL));
	gc = XCreateGC(display, RootWindow(display, screen), 0, NULL);
	XSetLineAttributes(display, gc, 1, LineSolid, CapButt, JoinMiter);
	
	window = XCreateSimpleWindow(
		display,
		RootWindow(display, screen),
		0, 0, w, h, 1,
		BlackPixel(display, screen),
		WhitePixel(display, screen)
	);

	XStoreName(display, window, name);

	wm_delete = XInternAtom(display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(display, window, &wm_delete, 1);
	
	XSelectInput(display, window, ExposureMask);
	
	XMapWindow(display, window);
	
	XSizeHints hints;
	hints.flags = PMinSize | PMaxSize;
	hints.min_width = w; hints.min_height = h;
	hints.max_width = w; hints.max_height = h;
	XSetWMNormalHints(display, window, &hints);
	
	XSetFont(display, gc, font->fid);
	back_buffer = XCreatePixmap(display, window, w, h, DefaultDepth(display, screen));
	
	static char stipple_bits[] = {0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55};
	stipple = XCreateBitmapFromData(display, window, stipple_bits, 8, 8);
	XSetStipple(display, gc, stipple);

	bgcol = bgcolor;

	return 0;
}

void GTerminateWindow() {
	XFreeFont(display, font);
	XFreeGC(display, gc);
	XFreePixmap(display, back_buffer);
	XFreePixmap(display, stipple);
	XFreeColors(display, DefaultColormap(display, screen), colors, 16, 0);
	XDestroyWindow(display, window);
	XCloseDisplay(display);
	for (int i = 0; i < 512; i++) {
		if (elements[i] != NULL) {
			free(elements[i]->elem);
			free(elements[i]);
			elements[i] = NULL;
		}
	}
}

void GRenderWindow() {
	XSetForeground(display, gc, colors[bgcol]);
	XFillRectangle(display, back_buffer, gc, 0, 0, 840, 600);

	for (int i = 0; i < 512; i++) {
		Element *e = elements[i];
		if (e != NULL) {
			if (e->type == 0) {
				TextElement *txt = (TextElement *)e->elem;
				XSetForeground(display, gc, colors[txt->color]);
				XDrawString(display, back_buffer, gc, txt->x, txt->y, txt->text, strlen(txt->text));
			}
		}
	}

	XCopyArea(display, back_buffer, window, gc, 0, 0, 840, 600, 0, 0);
	XSync(display, False);
}

void GHandleWindowEvents() {
	while (XPending(display) > 0) {
		XNextEvent(display, &event);
		if (event.type == Expose) {
			GRenderWindow();
		} else if (event.type == ClientMessage && (Atom)event.xclient.data.l[0] == wm_delete) {
			closing = 1;
		} 
	}
}

Element *allocate_element(int index, int type, void *data) {
    if (elements[index] != NULL) {
        free(elements[index]->elem);
        free(elements[index]);
        elements[index] = NULL;
    }
    Element *e = malloc(sizeof(Element));
    e->type = type;e->elem = data;
    elements[index] = e;
    return e;
}

int GWindowShouldClose() {
	return closing;
}

void GCreateText(int id, int x, int y, int color, char* text) {
	TextElement *text_elem = malloc(sizeof(TextElement));
	text_elem->x = x; text_elem->y = y; text_elem->text = text; text_elem->color = color;
	allocate_element(id, 0, text_elem);
}