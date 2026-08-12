EXPORT void DeleteElement(int id) {
	if (id >= elements.size() || elements[id] == NULL) return;
	int type = elements[id]->type;

	if (type == 0) {
		auto text = (TextElement*)elements[id]->elem;
		if (text->copied) free((void*)text->text);
		delete text;
	} else if (type == 1) {
		delete (ButtonElement*)elements[id]->elem;
	} else if (type == 2) {
		delete (InputElement*)elements[id]->elem;
	} else if (type == 3) {
		delete (RectElement*)elements[id]->elem;
	} else if (type == 4) {
		auto img = (ImageElement*)elements[id]->elem;
		XDestroyImage(img->ximage);
		free(img->data);
		free(img->prev);
		if (img->fontdata.buffer != NULL) free(img->fontdata.buffer);
		XFreePixmap(display, img->pixmap);
		delete img;
	} else if (type == 5) {
		delete (CheckboxElement*)elements[id]->elem;
	} else if (type == 6) {
		delete (ConsoleElement*)elements[id]->elem;
	} else if (type == 7) {
		delete (EllipseElement*)elements[id]->elem;
	} else if (type == 8) {
		auto opengl = (OpenGLElement*)elements[id]->elem;
		glXDestroyContext(display, opengl->ctx);
		glXDestroyPixmap(display, opengl->glx_pixmap);
		XFreePixmap(display, opengl->x_pixmap);
		delete opengl;
	}

	if (elements[id]->tooltip != NULL) free(elements[id]->tooltip);
	delete elements[id];
	elements[id] = NULL; 
}

EXPORT void CreateText(int id, int x, int y, const char* text, int color) {
	auto text_elem = new TextElement{color, text, false};
	_allocate_element(id, 0, text_elem, x, y, 0, 0);
}

EXPORT void CreateCopiedText(int id, int x, int y, const char* text, int color) {
	auto text_elem = new TextElement{color, strdup(text), true};
	_allocate_element(id, 0, text_elem, x, y, 0, 0);
}

EXPORT void CreateButton(int id, int x, int y, int w, int h, int u, int hvr, int p, const char* label, void (*onclick)(void)) {
	auto btn_elem = new ButtonElement{
		.unpressed = u, .hover = hvr, .pressed = p, .label = label, .onclick = onclick
	};

	_allocate_element(id, 1, btn_elem, x, y, w, h);
}

EXPORT void CreateInput(int id, int x, int y, int w, int h, int u, int hvr, int max) {
	if (w == -1) w = (max + 1) * 9 + 10;
	if (hvr == CLR_NONE) hvr = u;

	auto input = new InputElement{
		.inactive = u, .hover = hvr, .max = std::min(max, 127)
	};

	memset(input->input, 0, 128);
	_allocate_element(id, 2, input, x, y, w, h);
}

EXPORT char* GetInput(int id) {
	Element *e = elements[id];
	auto input = (InputElement *)e->elem;
	return input->input;
}

EXPORT void CreateRect(int id, int x, int y, int w, int h, int fg, int bg) {
	auto rect = new RectElement{.fg = fg, .bg = bg};
	_allocate_element(id, 3, rect, x, y, w, h);
}

EXPORT void CreateCheckbox(int id, int x, int y, int size, int cb_col, const char* label) {
	auto checkbox = new CheckboxElement{
		.cb_col = cb_col, .txt_col = H(cb_col), .label = label, .checked = 0
	};

	_allocate_element(id, 5, checkbox, x, y, size, size);
}

EXPORT int GetCheckbox(int id) {
	auto checkbox = (CheckboxElement *)elements[id]->elem;
	return checkbox->checked;
}

EXPORT void ElemModifyBounds(int id, int x, int y, int w, int h) {
	Element *e = elements[id];
	e->x = x, e->y = y;
	if (w != -1) e->w = w;
	if (h != -1) e->h = h;
}

EXPORT void CreateConsole(int id, int x, int y, int cols, int rows, int con_clr, int txt_clr) {
	auto console = new ConsoleElement{
		.data = std::string{}, .rows = rows, .cols = cols, .scroll = 0, .con_clr = con_clr, .txt_clr = txt_clr, .total_lines = 0
	};

	_allocate_element(id, 6, console, x, y, cols * 9 + 17, rows * 15 + 10);
}

EXPORT void ConsolePrint(int id, const char* format, ...) {
	auto console = (ConsoleElement*)(elements[id]->elem);
	int old_total_lines = console->total_lines;

	va_list args;
	va_start(args, format);

	char* buffer = NULL;
	int ret = vasprintf(&buffer, format, args);
	va_end(args);

	if (ret >= 0 && buffer) {
		console->data += buffer;
		free(buffer);
	}

	_console_calc_total_lines(console);

	int old_max_scroll = std::max(0, old_total_lines - console->rows);
	if (console->scroll >= old_max_scroll) {
		console->scroll = std::max(0, console->total_lines - console->rows);
	}
}

EXPORT void ConsoleClear(int id) {
	auto console = (ConsoleElement*)(elements[id]->elem);
	console->data.clear();
	console->total_lines = 0;
	console->scroll = 0;
}

EXPORT int ElemInside(int id) {
	Element* e = elements[id];
	return _inside_elem(e);
}

EXPORT void ElemSetVisible(int id, int visible) {
	Element* e = elements[id];
	e->v = visible;
}

EXPORT void CreateEllipse(int id, int x, int y, int w, int h, int fg, int bg) {
	auto rect = new EllipseElement{.fg = fg, .bg = bg};
	_allocate_element(id, 7, rect, x, y, w, h);
}

EXPORT void SetRenderingOrder(int order) {
	bb.frame_cb_after_elem = order;
}

EXPORT void CreateLabeledInput(int id1, int id2, int x, int y, const char* label, int clr1, int clr2, int max) {
	CreateCopiedText(id1, x, y, label, H(clr1));
	CreateInput(id2, x, y + 18, -1, 24, clr1, clr2, max);
}

EXPORT Element* GetElement(int id) {
	return elements[id];
}

EXPORT void SetTooltip(int id, const char* tooltip) {
	Element* e = elements[id];
	if (tooltip == NULL) {
		if (e->tooltip != NULL) free(e->tooltip);
		e->tooltip = NULL;
	} else {
		e->tooltip = strdup(tooltip);
	}
}
