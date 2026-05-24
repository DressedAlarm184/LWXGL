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
int screen, mouse_x = 0, mouse_y = 0, mouse_down = 0, closing = 0;
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
	int x, y, w, h, fgu, bgu, fgp, bgp, bgh, fgh;
	char* label; void (*onclick)(void);
} ButtonElement;

typedef struct {
	int x, y, w, h, fgu, bgu, bgh, fgh, max;
	char input[128];
} InputElement;

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
	
	XSelectInput(display, window, ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | KeyPressMask);
	
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
			GDeleteElement(i);
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
			} else if (e->type == 1) {
				ButtonElement *btn = (ButtonElement *)e->elem;
				int inside = mouse_x >= btn->x && mouse_x <= btn->x + btn->w &&
					mouse_y >= btn->y && mouse_y <= btn->y + btn->h;
				if (inside) {
					XSetForeground(display, gc, mouse_down == 1 ? colors[btn->bgp] : colors[btn->bgh]);
				} else XSetForeground(display, gc, colors[btn->bgu]);
				XFillRectangle(display, back_buffer, gc, btn->x + 1, btn->y + 1, btn->w - 1, btn->h - 1);
				if (inside) {
					XSetForeground(display, gc, mouse_down == 1 ? colors[btn->fgp] : colors[btn->fgh]);
				} else XSetForeground(display, gc, colors[btn->fgu]);
				XDrawRectangle(display, back_buffer, gc, btn->x, btn->y, btn->w - 1, btn->h - 1);
				XDrawString(display, back_buffer, gc, btn->x + (btn->w / 2) - (strlen(btn->label) * 9) / 2, btn->y + btn->h / 2 + 4, btn->label, strlen(btn->label));
			} else if (e->type == 2) {
				InputElement *input = (InputElement *)e->elem;
				int inside = mouse_x >= input->x && mouse_x <= input->x + input->w &&
					mouse_y >= input->y && mouse_y <= input->y + input->h;
				if (inside) {
					XSetForeground(display, gc, colors[input->bgh]);
				} else XSetForeground(display, gc, colors[input->bgu]);
				XFillRectangle(display, back_buffer, gc, input->x + 1, input->y + 1, input->w - 1, input->h - 1);
				if (inside) {
					XSetForeground(display, gc, colors[input->fgh]);
				} else XSetForeground(display, gc, colors[input->fgu]);
				XDrawRectangle(display, back_buffer, gc, input->x, input->y, input->w - 1, input->h - 1);
				char buffer[128]; sprintf(buffer, "%s%c", input->input, inside ? '_' : ' ');
				XDrawString(display, back_buffer, gc, input->x + 5, input->y + input->h / 2 + 4, buffer, strlen(buffer));
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
		} else if (event.type == MotionNotify) {
			mouse_x = event.xmotion.x, mouse_y = event.xmotion.y;
		} else if (event.type == ButtonPress) {
			mouse_down = event.xbutton.button;
		} else if (event.type == LeaveNotify) {
			mouse_x = -1, mouse_x = -1;
		} else if (event.type == ButtonPress) {
			mouse_down = event.xbutton.button;
		} else if (event.type == ButtonRelease) {
			mouse_down = 0;
			for (int i = 0; i < 512; i++) {
				Element *e = elements[i];
				if (e != NULL) {
					if (e->type == 1) {
						ButtonElement *btn = (ButtonElement *)e->elem;
						int inside = mouse_x >= btn->x && mouse_x <= btn->x + btn->w &&
							mouse_y >= btn->y && mouse_y <= btn->y + btn->h;
						if (inside) btn->onclick();
					}
				}
			}
		} else if (event.type == KeyPress) {
			XKeyEvent key = event.xkey; KeySym keysym;
			char ch; int len = XLookupString(&key, &ch, 1, &keysym, NULL);
			ch = (len == 0) ? 0 : ch;
			for (int i = 0; i < 512; i++) {
				Element *e = elements[i];
				if (e != NULL) {
					if (e->type == 2) {
						InputElement *input = (InputElement *)e->elem;
						int inside = mouse_x >= input->x && mouse_x <= input->x + input->w &&
						             mouse_y >= input->y && mouse_y <= input->y + input->h;
						if (!inside) continue;
						int length = strlen(input->input);
						if (ch == 8) {
							if (length > 0) input->input[length - 1] = 0;
						} else if (ch >= 32 && ch < 127) {
							if (length < input->max) input->input[length] = ch;
						}
					}
				}
			}
		}
	}
}

void GDeleteElement(int index) {
	free(elements[index]->elem);
	free(elements[index]);
	elements[index] = NULL;
}

Element *allocate_element(int index, int type, void *data) {
	if (elements[index] != NULL) {
		GDeleteElement(index);
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

void GCreateButton(int id, int x, int y, int w, int h,
				int fgu, int bgu, int fgh, int bgh,
				int fgp, int bgp, char *label, void (*onclick)(void)) {

	ButtonElement *btn_elem = malloc(sizeof(ButtonElement));

	*btn_elem = (ButtonElement){
		.x = x, .y = y, .w = w, .h = h, .fgu = fgu, .bgh = bgh, .fgh = fgh,
		.bgu = bgu, .fgp = fgp, .bgp = bgp, .label = label, .onclick = onclick
	};

	allocate_element(id, 1, btn_elem);
}

void GSimpleWindowLoop() {
	int tick = 0;
	while (!GWindowShouldClose()) {
		if (tick % 16 == 0) {
			GRenderWindow();
		}
		GHandleWindowEvents();
		usleep(1000);
		tick++;
	}
}

void GDeleteWindow() {
	closing = 1;
}

void GCreateInput(int id, int x, int y, int w, int h, int fgu, int bgu, int fgh, int bgh, int max) {
	InputElement *input = malloc(sizeof(InputElement));

	*input = (InputElement){
		.x = x, .y = y, .w = w, .h = h, .fgu = fgu, .bgh = bgh, .fgh = fgh, .bgu = bgu, .max = max
	};

	memset(input->input, 0, 128);

	allocate_element(id, 2, input);
}

char* GGetInput(int id) {
	Element *e = elements[id];
	InputElement *input = (InputElement *)e->elem;
	return input->input;
}