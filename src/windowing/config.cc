EXPORT void SetWindowTitle(const char* title) {
	XStoreName(display, window, title);
}

EXPORT void SetWindowColor(int color) {
	bgcol = color;
}

EXPORT void ChangeCursor(int cursor_font_glyph) {
	Cursor cursor;

	if (cursor_font_glyph == 255) {
		char data[1] = {0};
		Pixmap blank = XCreateBitmapFromData(display, window, data, 1, 1);
		XColor dummy = {0};
		cursor = XCreatePixmapCursor(display, blank, blank, &dummy, &dummy, 0, 0);
		XFreePixmap(display, blank);
	} else {
		cursor = XCreateFontCursor(display, cursor_font_glyph);
	}

	XDefineCursor(display, window, cursor);
	XFreeCursor(display, cursor);
}

EXPORT void ReserveScroll(int height, int scrollbar_color, void (*Scroll)(int offset)) {
	if (window != None) return;
	bb.scroll_enabled = true;
	bb.scrollbar_color = scrollbar_color;
	bb.h = height;
	Events::UserProvided::Scroll = Scroll;
}

EXPORT int SetGlobalBold(int bold) {
	const char* xlfd = bold ? "9x15bold" : "9x15";
	XFontStruct* new_font = XLoadQueryFont(display, xlfd);
	if (!new_font) return 0;
	XFreeFont(display, font), font = new_font;
	XSetFont(display, gc, font->fid);
	return 1;
}

EXPORT void PaletteQuery(int idx, unsigned char* r, unsigned char* g, unsigned char* b) {
	XColor color;
	color.pixel = colors[idx];
	XQueryColor(display, colormap, &color);
	*r = color.red / 257;
	*g = color.green / 257;
	*b = color.blue / 257;
}

EXPORT void PaletteModify(int idx, unsigned char r, unsigned char g, unsigned char b, int redraw) {
	XColor color;
	color.red   = r * 257;
	color.green = g * 257;
	color.blue  = b * 257;
	color.flags = DoRed | DoGreen | DoBlue;
	XFreeColors(display, colormap, &colors[idx], 1, 0);
	XAllocColor(display, colormap, &color);
	colors[idx] = color.pixel;
	if (redraw) RedrawAllImages();
}

EXPORT void PaletteReset() {
	XFreeColors(display, colormap, colors, 16, 0);
	XColor color;
	for (int i = 0; i < 16; i++) {
		color.red   = color_palette[i].r * 257;
		color.green = color_palette[i].g * 257;
		color.blue  = color_palette[i].b * 257;
		color.flags = DoRed | DoGreen | DoBlue;
		XAllocColor(display, colormap, &color);
		colors[i] = color.pixel;
	}
	RedrawAllImages();
}
