typedef struct {
	int color;
	const char *text;
	bool copied;
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
	Pixmap pixmap;
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
	int v, anchor;
	int type;
	void *elem;
} Element;

std::vector<Element*> elements;
