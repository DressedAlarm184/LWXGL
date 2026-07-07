typedef struct {
	int color;
	const char *text;
} TextElement;

typedef struct {
	int unpressed, hover, pressed;
	const char* label;
	void (*onclick)(void);
} ButtonElement;

typedef struct {
	int inactive, hover;
	int max;
	char input[128];
} InputElement;

typedef struct {
	int fg, bg;
} RectElement;

typedef struct {
	XImage *ximage;
	unsigned char *data, *prev;
	char *imgdata;
	struct {
		int height;
		unsigned char* buffer;
	} fontdata;
} ImageElement;

typedef struct {
	int cb_col, txt_col;
	const char* label;
	int checked;
} CheckboxElement;

typedef struct {
	std::string data;
	int rows, cols;
	int scroll;
	int con_clr, txt_clr;
	int total_lines;
} ConsoleElement;

typedef struct {
	int x, y, w, h;
	int v, screen;
	int type;
	void *elem;
} Element;

std::vector<Element*> elements;

EXPORT void GDeleteElement(int id) {
	if (id >= elements.size() || elements[id] == NULL) return;
	int type = elements[id]->type;

	if (type == 0) {
		delete (TextElement*)elements[id]->elem;
	} else if (type == 1) {
		delete (ButtonElement*)elements[id]->elem;
	} else if (type == 2) {
		delete (InputElement*)elements[id]->elem;
	} else if (type == 3) {
		delete (RectElement*)elements[id]->elem;
	} else if (type == 4) {
		ImageElement *img = (ImageElement*)elements[id]->elem;
		XDestroyImage(img->ximage);
		free(img->data);
		free(img->prev);
		if (img->fontdata.buffer != NULL) free(img->fontdata.buffer);
		delete img;
	} else if (type == 5) {
		delete (CheckboxElement*)elements[id]->elem;
	} else if (type == 6) {
		delete (ConsoleElement*)elements[id]->elem;
	}

	free(elements[id]);
	elements[id] = NULL; 
}

Element *_allocate_element(int id, int type, void *data, int x, int y, int w, int h) {
	if (id >= elements.size()) elements.resize(id + 1, NULL);
	if (elements[id] != NULL) GDeleteElement(id);
	Element *e = (Element*)malloc(sizeof(Element));
	e->type = type, e->elem = data;
	e->w = w, e->h = h, e->x = x, e->y = y; e->v = 1; e->screen = 0;
	elements[id] = e;
	return e;
}

EXPORT void GCreateText(int id, int x, int y, int color, const char* text) {
	TextElement *text_elem = new TextElement;
	text_elem->text = text; text_elem->color = color;
	_allocate_element(id, 0, text_elem, x, y, 0, 0);
}

EXPORT void GCreateButton(int id, int x, int y, int w, int h, int u, int hvr, int p, const char* label, void (*onclick)(void)) {
	ButtonElement *btn_elem = new ButtonElement;

	*btn_elem = (ButtonElement){
		.unpressed = u, .hover = hvr, .pressed = p, .label = label, .onclick = onclick
	};

	_allocate_element(id, 1, btn_elem, x, y, w, h);
}

EXPORT void GCreateInput(int id, int x, int y, int w, int h, int u, int hvr, int max) {
	if (w == -1) w = (max + 1) * 9 + 10;
	InputElement *input = new InputElement;

	*input = (InputElement){
		.inactive = u, .hover = hvr, .max = max
	};

	memset(input->input, 0, 128);
	_allocate_element(id, 2, input, x, y, w, h);
}

EXPORT char* GGetInput(int id) {
	Element *e = elements[id];
	InputElement *input = (InputElement *)e->elem;
	return input->input;
}

EXPORT void GCreateRect(int id, int x, int y, int w, int h, int fg, int bg) {
	RectElement *rect = new RectElement;
	*rect = (RectElement){.fg = fg, .bg = bg};
	_allocate_element(id, 3, rect, x, y, w, h);
}

EXPORT void GCreateImage(int id, int x, int y, int w, int h) {
	ImageElement *img = new ImageElement;
	img->ximage = XCreateImage(display, DefaultVisual(display, screen), DefaultDepth(display, screen), ZPixmap, 0, NULL, w, h, 32, 0);
	img->data = (unsigned char *)calloc(w * h, 1);
	img->imgdata = (char *)calloc(h * img->ximage->bytes_per_line, 1);
	img->prev = (unsigned char *)calloc(w * h, 1);
	img->ximage->data = img->imgdata;
	img->fontdata.buffer = NULL;
	img->fontdata.height = 0;
	_allocate_element(id, 4, img, x, y, w, h);
}

EXPORT unsigned char* GGetImageData(int id) {
	ImageElement *img = (ImageElement *)elements[id]->elem;
	return img->data;
}

EXPORT void GUpdateImage(int id) {
	ImageElement *img = (ImageElement *)elements[id]->elem;
	int w = elements[id]->w, h = elements[id]->h;
	unsigned char *src = (unsigned char*)img->data;
	unsigned char *prev = (unsigned char*)img->prev; 
	int (*put_pixel)(XImage *, int, int, unsigned long) = img->ximage->f.put_pixel;
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			if (*src != *prev) {
				put_pixel(img->ximage, x, y, colors[*src]);
				*prev = *src; 
			}
			src++, prev++;
		}
	}
}

EXPORT void GPrimitiveRect(int id, int x, int y, int w, int h, int fg, int bg) {
	if (fg == -1) fg = bg;
	ImageElement *img = (ImageElement *)elements[id]->elem;
	int img_w = elements[id]->w, img_h = elements[id]->h;
	for (int cy = y; cy < y + h; cy++) {
		for (int cx = x; cx < x + w; cx++) {
			if (cx < 0 || cx >= img_w || cy < 0 || cy >= img_h) continue;
			int color = (cx == x || cx == w + x - 1) ? fg : (cy == y || cy == y + h - 1) ? fg : bg;
			if (color != -1) img->data[cx + cy * img_w] = color;
		}
	}
}

EXPORT void GPrimitiveCircle(int id, int cx, int cy, int r, int fg, int bg) {
	ImageElement *img = (ImageElement *)elements[id]->elem;
	int w = elements[id]->w, h = elements[id]->h;
	for (int y = cy - r; y <= cy + r; y++) {
		for (int x = cx - r; x <= cx + r; x++) {
			if (x < 0 || x >= w || y < 0 || y >= h) continue;
			int dx = x - cx, dy = y - cy, d2 = dx * dx + dy * dy;
			int on_border = (d2 <= r * r && d2 >= (r - 1) * (r - 1));
			if (fg != -1 && on_border) {
				img->data[x + y * w] = (uint32_t)fg;
			} else if (bg != -1 && d2 <= r * r) {
				img->data[x + y * w] = (uint32_t)bg;
			}
		}
	}
}

EXPORT void GPrimitiveLine(int id, int x1, int y1, int x2, int y2, int color) {
	ImageElement *img = (ImageElement *)elements[id]->elem;
	int w = elements[id]->w, h = elements[id]->h;
	int dx = x2 - x1, dy = y2 - y1;
	int steps = std::max(std::abs(dx), std::abs(dy));
	float x_inc = dx / (float)steps, y_inc = dy / (float)steps;
	float x = x1, y = y1;
	for (int i = 0; i <= steps; i++) {
		int pixel_x = (int)std::round(x), pixel_y = (int)std::round(y);
		if (pixel_x < 0 || pixel_x >= w || pixel_y < 0 || pixel_y >= h) continue;
		img->data[pixel_x + pixel_y * w] = color;
		x += x_inc, y += y_inc;
	}
}

EXPORT void GPrimitiveSprite(int id, int sx, int sy, int color, const char* sprite, int scale) {
	ImageElement *img = (ImageElement *)elements[id]->elem;
	int img_w = elements[id]->w, img_h = elements[id]->h;
	int x = sx, y = sy;

	auto set_pixel = [&](int color) {
		for (int yoff = 0; yoff < scale; yoff++) {
			for (int xoff = 0; xoff < scale; xoff++) {
				int px = x + xoff, py = y + yoff;
				if (px >= 0 && px < img_w && py >= 0 && py < img_h) {
					img->data[py * img_w + px] = color;
				}
			}
		}
	};

	std::function<void(const char*, int)> draw = [&](const char* rle, int len) {
		int count = 0;
		for (int i = 0; (*rle != 0 && *rle != '!' && i < len); i++) {
			if (*rle >= '0' && *rle <= '9') {
				count = count * 10 + (*rle - '0');
			} else if (*rle == '[') {
				rle++;
				const char *start = rle, *end = rle;
				for (int depth = 1; *end && depth > 0;) {
					if (*end == '[') depth++;
					else if (*end == ']') depth--;
					end++;
				}
				int len = (int)((end - 1) - start);
				int runs = (count > 0) ? count : 1;
				while (runs--) draw(rle, len);
				rle = end - 1, count = 0, i += len + 1;
			} else {
				int runs = (count == 0) ? 1 : count;
				if (*rle == '#') {
				while (runs-- > 0) {
					set_pixel(color);
					x += scale;
				}
			} else if (*rle == '.') {
				while (runs-- > 0) {
					set_pixel(0);
					x += scale;
				}
			} else if (*rle == '$') {
					x = sx, y += runs * scale;
				} else if (*rle == '>') {
					x += runs * scale;
				}
				count = 0;
			}
			rle++;
		}
	};

	draw(sprite, strlen(sprite));
}

EXPORT void GCreateCheckbox(int id, int x, int y, int size, int cb_col, int txt_col, const char* label) {
	CheckboxElement *checkbox = new CheckboxElement;

	*checkbox = (CheckboxElement){
		.cb_col = cb_col, .txt_col = txt_col, .label = label, .checked = 0
	};

	_allocate_element(id, 5, checkbox, x, y, size, size);
}

EXPORT int GGetCheckbox(int id) {
	CheckboxElement *checkbox = (CheckboxElement *)elements[id]->elem;
	return checkbox->checked;
}

EXPORT void GClearImage(int id, int c) {
	ImageElement *img = (ImageElement *)elements[id]->elem;
	memset(img->data, c, elements[id]->w * elements[id]->h);
}

EXPORT void GElemModifyBounds(int id, int x, int y, int w, int h) {
	Element *e = elements[id];
	e->x = x, e->y = y, e->w = w, e->h = h;
}

EXPORT void GCreateConsole(int id, int x, int y, int cols, int rows, int con_clr, int txt_clr) {
	ConsoleElement* console = new ConsoleElement{
		.data = std::string{}, .rows = rows, .cols = cols, .scroll = 0, .con_clr = con_clr, .txt_clr = txt_clr, .total_lines = 0
	};

	_allocate_element(id, 6, console, x, y, cols * 9 + 17, rows * 15 + 10);
}

void _console_calc_total_lines(ConsoleElement* console) {
	int total_lines = 1, current_len = 0;

	for (char c : console->data) {
		if (c == '\n') {
			total_lines++;
			current_len = 0;
		} else {
			current_len++;
			if (current_len == console->cols) {
				total_lines++;
				current_len = 0;
			}
		}
	}

	console->total_lines = total_lines;
}

EXPORT void GConsolePrint(int id, const char* format, ...) {
	ConsoleElement* console = (ConsoleElement*)(elements[id]->elem);
	int old_total_lines = console->total_lines;
	va_list args;
	va_start(args, format);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);
	console->data += buffer;
	_console_calc_total_lines(console);
	if (console->scroll == old_total_lines - console->rows)
		console->scroll = std::max(0, console->total_lines - console->rows);
}

EXPORT void GConsoleClear(int id) {
	ConsoleElement* console = (ConsoleElement*)(elements[id]->elem);
	console->data.clear();
	console->total_lines = 0;
	console->scroll = 0;
}

int _inside_elem(Element* e) {
	int right_extent = e->x + e->w;
	if (e->type == 5) {
		CheckboxElement* checkbox = (CheckboxElement*)e->elem;
		if (checkbox->label != NULL) right_extent += 6 + (int)strlen(checkbox->label) * 9;
	}
	int mouse_inside = mouse_x >= e->x && mouse_x < right_extent && mouse_y >= e->y && mouse_y < e->y + e->h;
	int is_on_screen = e->screen == active_screen || e->screen == -1;
	return mouse_inside && is_on_screen && e->v;
}

EXPORT int GElemInside(int id) {
	Element* e = elements[id];
	return _inside_elem(e);
}

EXPORT void GElemSetVisible(int id, int visible) {
	Element* e = elements[id];
	e->v = visible;
}

EXPORT void GRedrawAllImages() {
	for (int i = 0; i < elements.size(); i++) {
		Element *e = elements[i];
		if (e == NULL) continue;
		if (e->type != 4) continue;
		ImageElement* img = (ImageElement *)e->elem;
		memset(img->prev, 255, e->w * e->h);
		GUpdateImage(i);
	}
}

EXPORT void GImageSetFont(int id, unsigned char* font, int h) {
	ImageElement *img = (ImageElement*)elements[id]->elem;
	img->fontdata.height = h;
	if (img->fontdata.buffer != NULL) free(img->fontdata.buffer);
	img->fontdata.buffer = (unsigned char*)malloc(256 * h);
	memcpy(img->fontdata.buffer, font, 256 * h);
}

EXPORT void GDrawString(int id, int x, int y, const char* txt, int color) {
	Element *e = elements[id];
	ImageElement *img = (ImageElement*)e->elem;
	int height = img->fontdata.height;
	unsigned char* font = img->fontdata.buffer;

	for (int cx = 0; *txt; cx++) {
		if (*txt == 10) {
			y += height, cx = -1;
		} else for (int i = 0; i < height; i++) {
			unsigned char row_bitmap = font[(unsigned int)*txt * height + i];
			for (int rx = 0; rx < 8; rx++) {
				if (!(row_bitmap & (0x80 >> rx))) continue;
				int px = x + rx + cx * 9, py = y + i;
				if (px >= e->w || px < 0 || py >= e->h || py < 0) continue;
				img->data[py * e->w + px] = color;
			}
		}

		txt++;
	}
}

EXPORT int GDrawIndexedTGA(int id, int x, int y, const char* path, int change_palette) {
	std::ifstream file(path, std::ios::binary);
	if (!file) return 1;

	unsigned char expected_header[18] = {0, 1, 1, 0, 0, 16, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 8, 32};
	unsigned char tga_header[18] = {0};

	file.read((char*)tga_header, 18);
	memcpy(expected_header + 8, tga_header + 8, 8);
	if (memcmp(tga_header, expected_header, 18) != 0) return 2;

	int width = tga_header[12] | (tga_header[13] << 8);
	int height = tga_header[14] | (tga_header[15] << 8);

	Element *e = elements[id];
	ImageElement *img = (ImageElement*)e->elem;

	unsigned char palette[48] = {0};
	std::vector<unsigned char> scanline(width);

	file.read((char*)palette, 48);

	if (change_palette) {
		for (int i = 0; i < 16; i++) {
			int o = i * 3;
			GPaletteModify(i, palette[o + 2], palette[o + 1], palette[o], i == 15 ? 1 : 0);
		}
	}

	for (int iy = 0; iy < height; iy++) {
		file.read((char*)scanline.data(), width);
		for (int ix = 0; ix < width; ix++) {
			int px = x + ix, py = y + iy;
			if (px >= e->w || px < 0 || py >= e->h || py < 0) continue;
			img->data[py * e->w + px] = scanline[ix];
		}
	}

	return 0;
}
