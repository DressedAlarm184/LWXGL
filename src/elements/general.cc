EXPORT void GDeleteElement(int id) {
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
	}

	delete elements[id];
	elements[id] = NULL; 
}

EXPORT void GCreateText(int id, int x, int y, const char* text, int color) {
	auto text_elem = new TextElement{color, text, false};
	_allocate_element(id, 0, text_elem, x, y, 0, 0);
}

EXPORT void GCreateCopiedText(int id, int x, int y, const char* text, int color) {
	auto text_elem = new TextElement{color, strdup(text), true};
	_allocate_element(id, 0, text_elem, x, y, 0, 0);
}

EXPORT void GCreateButton(int id, int x, int y, int w, int h, int u, int hvr, int p, const char* label, void (*onclick)(void)) {
	auto btn_elem = new ButtonElement{
		.unpressed = u, .hover = hvr, .pressed = p, .label = label, .onclick = onclick
	};

	_allocate_element(id, 1, btn_elem, x, y, w, h);
}

EXPORT void GCreateInput(int id, int x, int y, int w, int h, int u, int hvr, int max) {
	if (w == -1) w = (max + 1) * 9 + 10;

	auto input = new InputElement{
		.inactive = u, .hover = hvr, .max = std::min(max, 127)
	};

	memset(input->input, 0, 128);
	_allocate_element(id, 2, input, x, y, w, h);
}

EXPORT char* GGetInput(int id) {
	Element *e = elements[id];
	auto input = (InputElement *)e->elem;
	return input->input;
}

EXPORT void GCreateRect(int id, int x, int y, int w, int h, int fg, int bg) {
	auto rect = new RectElement{.fg = fg, .bg = bg};
	_allocate_element(id, 3, rect, x, y, w, h);
}

EXPORT void GCreateCheckbox(int id, int x, int y, int size, int cb_col, int txt_col, const char* label) {
	auto checkbox = new CheckboxElement{
		.cb_col = cb_col, .txt_col = txt_col, .label = label, .checked = 0
	};

	_allocate_element(id, 5, checkbox, x, y, size, size);
}

EXPORT int GGetCheckbox(int id) {
	auto checkbox = (CheckboxElement *)elements[id]->elem;
	return checkbox->checked;
}

EXPORT void GElemModifyBounds(int id, int x, int y, int w, int h) {
	Element *e = elements[id];
	e->x = x, e->y = y;
	if (w != -1) e->w = w;
	if (h != -1) e->h = h;
}

EXPORT void GCreateConsole(int id, int x, int y, int cols, int rows, int con_clr, int txt_clr) {
	auto console = new ConsoleElement{
		.data = std::string{}, .rows = rows, .cols = cols, .scroll = 0, .con_clr = con_clr, .txt_clr = txt_clr, .total_lines = 0
	};

	_allocate_element(id, 6, console, x, y, cols * 9 + 17, rows * 15 + 10);
}

EXPORT void GConsolePrint(int id, const char* format, ...) {
	auto console = (ConsoleElement*)(elements[id]->elem);
	int old_total_lines = console->total_lines;

	va_list args;
	va_start(args, format);

	char* buffer;
	vasprintf(&buffer, format, args);
	va_end(args);

	console->data += buffer;
	free(buffer);

	_console_calc_total_lines(console);

	int old_max_scroll = std::max(0, old_total_lines - console->rows);
	if (console->scroll >= old_max_scroll) {
		console->scroll = std::max(0, console->total_lines - console->rows);
	}
}

EXPORT void GConsoleClear(int id) {
	auto console = (ConsoleElement*)(elements[id]->elem);
	console->data.clear();
	console->total_lines = 0;
	console->scroll = 0;
}

EXPORT int GElemInside(int id) {
	Element* e = elements[id];
	return _inside_elem(e);
}

EXPORT void GElemSetVisible(int id, int visible) {
	Element* e = elements[id];
	e->v = visible;
}

EXPORT void GElemAnchor(int anchor, int ids[], int count) {
	for (int i = 0; i < count; i++) {
		Element* e = elements[ids[i]];
		if (anchor == 0) e->y = e->anchor;
		e->anchor = anchor == 0 ? INT_MIN : e->y;
	}
}

EXPORT void GResolveAnchors() {
	for (Element* e : elements) {
		if (e->anchor == INT_MIN) continue;
		e->y = bb.scroll + e->anchor;
	}
}
