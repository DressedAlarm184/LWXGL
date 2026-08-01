EXPORT void ImmediateText(int x, int y, const char* str, int color) {
	XSetForeground(display, gc, colors[color]);
	y += 11;
	while (*str != '\0') {
		int len = 0;
		while (str[len] != '\0' && str[len] != '\n') len++;
		XDrawString(display, bb, gc, x, y, str, len);
		str += len, y += 15;
		if (*str == '\n') str++;
	}
}

EXPORT void ImmediateTextF(int x, int y, int color, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	va_list ap_copy;
	va_copy(ap_copy, ap);

	int req_len = vsnprintf(NULL, 0, fmt, ap_copy);
	va_end(ap_copy);

	if (req_len >= 0) {
		auto result = (char*)alloca((size_t)req_len + 1);
		vsnprintf(result, (size_t)req_len + 1, fmt, ap);
		ImmediateText(x, y, result, color);
	}

	va_end(ap);
}

EXPORT void ImmediateEllipse(int x, int y, int w, int h, int fg, int bg) {
	if (bg >= 0) {
		XSetForeground(display, gc, colors[bg]);
		XFillArc(display, bb, gc, x, y, w, h, 0, 23040);
	}
	if (fg >= 0) {
		XSetForeground(display, gc, colors[fg]);
		XDrawArc(display, bb, gc, x, y, w, h, 0, 23040);
	}
}

EXPORT void ImmediateRect(int x, int y, int w, int h, int fg, int bg) {
	if (bg >= 0) {
		XSetForeground(display, gc, colors[bg]);
		XFillRectangle(display, bb, gc, x, y, w, h);
	}
	if (fg >= 0) {
		XSetForeground(display, gc, colors[fg]);
		XDrawRectangle(display, bb, gc, x, y, w - 1, h - 1);
	}
}

EXPORT void ImmediateLine(int x1, int y1, int x2, int y2, int color) {
	XSetForeground(display, gc, colors[color]);
	XDrawLine(display, bb, gc, x1, y1, x2, y2);
}
