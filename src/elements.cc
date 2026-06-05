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
} ImageElement;

typedef struct {
	int cb_col, txt_col;
	const char* label;
	int checked;
} CheckboxElement;

typedef struct {
	int x, y, w, h;
	int type;
	void *elem;
} Element;

std::vector<Element*> elements;

EXPORT void GDeleteElement(int id) {
	if (id >= elements.size() || elements[id] == NULL) return;
	if (elements[id]->type == 4) {
		ImageElement *img = (ImageElement *)elements[id]->elem;
		XDestroyImage(img->ximage);
		free(img->data);
		free(img->prev);
	}
	free(elements[id]->elem);
	free(elements[id]);
	elements[id] = NULL; 
}

Element *allocate_element(int id, int type, void *data, int x, int y, int w, int h) {
	if (id >= elements.size()) elements.resize(id + 1, NULL);
	if (elements[id] != NULL) GDeleteElement(id);
	Element *e = (Element*)malloc(sizeof(Element));
	e->type = type, e->elem = data;
	e->w = w, e->h = h, e->x = x, e->y = y;
	elements[id] = e;
	return e;
}

EXPORT void GCreateText(int id, int x, int y, int color, const char* text) {
	TextElement *text_elem = (TextElement*)malloc(sizeof(TextElement));
	text_elem->text = text; text_elem->color = color;
	allocate_element(id, 0, text_elem, x, y, 0, 0);
}

EXPORT void GCreateButton(int id, int x, int y, int w, int h, int u, int hvr, int p, const char* label, void (*onclick)(void)) {
	ButtonElement *btn_elem = (ButtonElement*)malloc(sizeof(ButtonElement));

	*btn_elem = (ButtonElement){
		.unpressed = u, .hover = hvr, .pressed = p, .label = label, .onclick = onclick
	};

	allocate_element(id, 1, btn_elem, x, y, w, h);
}

EXPORT void GCreateInput(int id, int x, int y, int w, int h, int u, int hvr, int max) {
	if (w == -1) w = (max + 1) * 9 + 10;
	InputElement *input = (InputElement*)malloc(sizeof(InputElement));

	*input = (InputElement){
		.inactive = u, .hover = hvr, .max = max
	};

	memset(input->input, 0, 128);
	allocate_element(id, 2, input, x, y, w, h);
}

EXPORT char* GGetInput(int id) {
	Element *e = elements[id];
	InputElement *input = (InputElement *)e->elem;
	return input->input;
}

EXPORT void GCreateRect(int id, int x, int y, int w, int h, int fg, int bg) {
	RectElement *rect = (RectElement*)malloc(sizeof(RectElement));
	*rect = (RectElement){.fg = fg, .bg = bg};
	allocate_element(id, 3, rect, x, y, w, h);
}

EXPORT void GCreateImage(int id, int x, int y, int w, int h) {
	ImageElement *img = (ImageElement *)malloc(sizeof(ImageElement));
	img->ximage = XCreateImage(display, DefaultVisual(display, screen), DefaultDepth(display, screen), ZPixmap, 0, NULL, w, h, 32, 0);
	img->data = (unsigned char *)calloc(w * h, 1);
	img->imgdata = (char *)calloc(h * img->ximage->bytes_per_line, 1);
	img->prev = (unsigned char *)calloc(w * h, 1);
	img->ximage->data = img->imgdata;
	allocate_element(id, 4, img, x, y, w, h);
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
				rle = end - 1, count = 0;
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
	CheckboxElement *checkbox = (CheckboxElement*)malloc(sizeof(CheckboxElement));

	*checkbox = (CheckboxElement){
		.cb_col = cb_col, .txt_col = txt_col, .label = label, .checked = 0
	};

	allocate_element(id, 5, checkbox, x, y, size, size);
}

EXPORT int GGetCheckbox(int id) {
	CheckboxElement *checkbox = (CheckboxElement *)elements[id]->elem;
	return checkbox->checked;
}

EXPORT void GClearImage(int id, int c) {
	ImageElement *img = (ImageElement *)elements[id]->elem;
	memset(img->data, c, elements[id]->w * elements[id]->h);
}