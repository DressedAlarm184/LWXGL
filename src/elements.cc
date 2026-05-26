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
	img->x = x; img->y = y; img->w = h, img->h = h;
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