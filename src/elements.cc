typedef struct {
	int x, y, color;
	char *text;
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
	int x, y, w, h, fg, bg;
} RectElement;

typedef struct {
	int x, y, w, h;
	XImage *ximage;
	char *data, *imgdata;
} ImageElement;

typedef struct {
	int type;
	void *elem;
} Element;

std::vector<Element*> elements;

void GDeleteElement(int id) {
	if (id >= elements.size() || elements[id] == NULL) return;
	if (elements[id]->type == 4) {
		ImageElement *img = (ImageElement *)elements[id]->elem;
		XDestroyImage(img->ximage);
		free(img->data);
	}
	free(elements[id]->elem);
	free(elements[id]);
	elements[id] = NULL; 
}

Element *allocate_element(int id, int type, void *data) {
	if (id >= elements.size()) elements.resize(id + 1, NULL);
	if (elements[id] != NULL) GDeleteElement(id);
	Element *e = (Element*)malloc(sizeof(Element));
	e->type = type;
	e->elem = data;
	elements[id] = e;
	return e;
}

void GCreateText(int id, int x, int y, int color, char* text) {
	TextElement *text_elem = (TextElement*)malloc(sizeof(TextElement));
	text_elem->x = x; text_elem->y = y; text_elem->text = text; text_elem->color = color;
	allocate_element(id, 0, text_elem);
}

void GCreateButton(int id, int x, int y, int w, int h, int u, int hvr, int p, char* label, void (*onclick)(void)) {
	ButtonElement *btn_elem = (ButtonElement*)malloc(sizeof(ButtonElement));

	*btn_elem = (ButtonElement){
		.x = x, .y = y, .w = w, .h = h, .fgu = H(u), .bgu = L(u), .fgp = H(p), .bgp = L(p), .bgh = L(hvr), .fgh = H(hvr), .label = label, .onclick = onclick
	};

	allocate_element(id, 1, btn_elem);
}

void GCreateInput(int id, int x, int y, int w, int h, int u, int hvr, int max) {
	InputElement *input = (InputElement*)malloc(sizeof(InputElement));

	*input = (InputElement){
		.x = x, .y = y, .w = w, .h = h, .fgu = H(u), .bgu = L(u), .bgh = L(hvr), .fgh = H(hvr), .max = max
	};

	memset(input->input, 0, 128);
	allocate_element(id, 2, input);

	if (input->w == -1) input->w = (input->max + 1) * 9 + 10;
}

char* GGetInput(int id) {
	Element *e = elements[id];
	InputElement *input = (InputElement *)e->elem;
	return input->input;
}

void GCreateRect(int id, int x, int y, int w, int h, int fg, int bg) {
	RectElement *rect = (RectElement*)malloc(sizeof(RectElement));

	*rect = (RectElement){
		.x = x, .y = y, .w = w, .h = h, .fg = fg, .bg = bg
	};

	allocate_element(id, 3, rect);
}

void GCreateImage(int id, int x, int y, int w, int h) {
	ImageElement *img = (ImageElement *)malloc(sizeof(ImageElement));
	img->x = x; img->y = y; img->w = w, img->h = h;
	img->ximage = XCreateImage(display, DefaultVisual(display, screen), DefaultDepth(display, screen), ZPixmap, 0, NULL, w, h, 32, 0);
	img->data = (char *)calloc(w * h, 1), img->imgdata = (char *)calloc(h * img->ximage->bytes_per_line, 1);
	img->ximage->data = img->imgdata;
	allocate_element(id, 4, img);
}

char* GGetImageData(int id) {
	ImageElement *img = (ImageElement *)elements[id]->elem;
	return img->data;
}

void GUpdateImage(int id) {
	ImageElement *img = (ImageElement *)elements[id]->elem;;
	for (int y = 0; y < img->h; y++) {
		for (int x = 0; x < img->w; x++) {
			XPutPixel(img->ximage, x, y, colors[img->data[y * img->w + x]]);
		}
	}
}

void GPrimitiveRect(int id, int x, int y, int w, int h, int fg, int bg) {
	if (fg == -1) fg = bg;
	ImageElement *img = (ImageElement *)elements[id]->elem;
	for (int cy = y; cy < y + h; cy++) {
		for (int cx = x; cx < x + w; cx++) {
			int color = (cx == x || cx == w + x - 1) ? fg : (cy == y || cy == y + h - 1) ? fg : bg;
			if (color != -1) img->data[cx + cy * img->w] = color;
		}
	}
}

void GPrimitiveCircle(int id, int cx, int cy, int r, int fg, int bg) {
	ImageElement *img = (ImageElement *)elements[id]->elem;
	for (int y = cy - r; y <= cy + r; y++) {
		for (int x = cx - r; x <= cx + r; x++) {
			int dx = x - cx, dy = y - cy;
			int d2 = dx*dx + dy*dy;
			int inside = (d2 <= r * r);
			if (!inside) continue;
			int on_border = (d2 <= r * r && d2 >= (r-1)*(r-1));
			if (fg != -1 && on_border) {
				img->data[x + y * img->w] = (uint32_t)fg;
			} else if (bg != -1) {
				img->data[x + y * img->w] = (uint32_t)bg;
			}
		}
	}
}

void GPrimitiveLine(int id, int x1, int y1, int x2, int y2, int color) {
	ImageElement *img = (ImageElement *)elements[id]->elem;
	int dx = x2 - x1, dy = y2 - y1;
	int steps = std::max(std::abs(dx), std::abs(dy));
	float x_inc = dx / (float)steps, y_inc = dy / (float)steps;
	float x = x1, y = y1;
	for (int i = 0; i <= steps; i++) {
		img->data[(int)std::round(x) + (int)std::round(y) * img->w] = color;
		x += x_inc, y += y_inc;
	}
}

void GPrimitiveSprite(int id, int sx, int sy, int color, char* sprite) {
	ImageElement *img = (ImageElement *)elements[id]->elem;
	int x = sx, y = sy;

	std::function<void(char*, int)> draw = [&](char* rle, int len) {
		int count = 0;
		for (int i = 0; (*rle != 0 && *rle != '!' && i < len); i++) {
			if (*rle >= '0' && *rle <= '9') {
				count = count * 10 + (*rle - '0');
			} else if (*rle == '[') {
				rle++;
				char *start = rle, *end = rle;
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
					while (runs--) img->data[y * img->w + x++] = color;
				} else if (*rle == '.') {
					while (runs--) img->data[y * img->w + x++] = 0;
				} else if (*rle == '$') {
					x = sx, y += runs;
				}
				count = 0;
			}
			rle++;
		}
	};

	draw(sprite, strlen(sprite));
}