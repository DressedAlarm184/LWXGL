namespace Renderers {
	void Text(Element* e, bool inside) {
		auto txt = (TextElement *)e->elem;
		ImmediateText(e->x, e->y, txt->text, txt->color);
	}

	void Button(Element* e, bool inside) {
		auto btn = (ButtonElement *)e->elem;
		if (inside) {
			XSetForeground(display, gc, colors[mouse_down == 1 ? L(btn->pressed) : L(btn->hover)]);
		} else XSetForeground(display, gc, colors[L(btn->unpressed)]);
		XFillRectangle(display, bb, gc, e->x + 1, e->y + 1, e->w - 1, e->h - 1);
		if (inside) {
			XSetForeground(display, gc, colors[mouse_down == 1 ? H(btn->pressed) : H(btn->hover)]);
		} else XSetForeground(display, gc, colors[H(btn->unpressed)]);
		XDrawRectangle(display, bb, gc, e->x, e->y, e->w - 1, e->h - 1);
		XDrawString(display, bb, gc, e->x + (e->w / 2) - (strlen(btn->label) * 9) / 2, e->y + e->h / 2 + 4, btn->label, strlen(btn->label));
	}

	void Input(Element* e, bool inside) {
		auto input = (InputElement *)e->elem;
		if (inside) {
			XSetForeground(display, gc, colors[L(input->hover)]);
		} else XSetForeground(display, gc, colors[L(input->inactive)]);
		XFillRectangle(display, bb, gc, e->x + 1, e->y + 1, e->w - 1, e->h - 1);
		if (inside) {
			XSetForeground(display, gc, colors[H(input->hover)]);
		} else XSetForeground(display, gc, colors[H(input->inactive)]);
		XDrawRectangle(display, bb, gc, e->x, e->y, e->w - 1, e->h - 1);
		char buffer[128]; sprintf(buffer, "%s%c", input->input, inside ? '_' : ' ');
		XDrawString(display, bb, gc, e->x + 5, e->y + e->h / 2 + 4, buffer, strlen(buffer));
	}

	void Rect(Element* e, bool inside) {
		auto rect = (RectElement *)e->elem;
		ImmediateRect(e->x, e->y, e->w, e->h, rect->fg, rect->bg);
	}

	void Image(Element* e, bool inside) {
		auto img = (ImageElement *)e->elem;
		XCopyArea(display, img->pixmap, bb, gc, 0, 0, e->w, e->h, e->x, e->y);
	}

	void Checkbox(Element* e, bool inside) {
		auto checkbox = (CheckboxElement *)e->elem;
		XSetForeground(display, gc, colors[L(checkbox->cb_col)]);
		XFillRectangle(display, bb, gc, e->x + 1, e->y + 1, e->w - 1, e->h - 1);
		XSetForeground(display, gc, colors[H(checkbox->cb_col)]);
		XDrawRectangle(display, bb, gc, e->x, e->y, e->w - 1, e->h - 1);
		if (checkbox->checked) XFillRectangle(display, bb, gc, e->x + 4, e->y + 4, e->w - 8, e->h - 8);
		if (checkbox->label != NULL) {
			XSetForeground(display, gc, colors[checkbox->txt_col]);
			XDrawString(display, bb, gc, e->x + e->w + 5, e->y + e->h / 2 + 5, checkbox->label, strlen(checkbox->label));
		}
	}

	void Console(Element* e, bool inside) {
		auto console = (ConsoleElement *)e->elem;
		auto input = console->input;
		XSetForeground(display, gc, colors[L(console->con_clr)]);
		XFillRectangle(display, bb, gc, e->x + 1, e->y + 1, e->w - 1, e->h - 1);
		XSetForeground(display, gc, colors[H(console->con_clr)]);
		XDrawRectangle(display, bb, gc, e->x, e->y, e->w - 1, e->h - 1);
		if (console->total_lines > console->rows) {
			int thumb_height = std::max((console->total_lines <= 0)
				? (e->h - 6)
				: std::min(e->h - 6, std::max(1, int((e->h - 6) * ((float)console->rows / console->total_lines)))), 16);
			int denom = console->total_lines - console->rows, thumb_y = e->y + 3;
			if (denom > 0) thumb_y += ((e->h - 6) - thumb_height) * console->scroll / denom;
			XFillRectangle(display, bb, gc, e->x + e->w - 8, thumb_y, 5, thumb_height);
		}
		std::string expanded_data;
		int reserved_length = console->data.length();
		if (input.is_input_active) reserved_length += inside + input.data.length() + input.prompt.length();
		expanded_data.reserve(reserved_length);
		for (char c : console->data) {
			if (c == '\t') {
				expanded_data.append("    ");
			} else {
				int is_valid = c == '\n' || (c >= 27 && c <= 126);
				expanded_data.push_back(is_valid ? c : '?');
			}
		}
		if (input.is_input_active) {
			expanded_data.append(input.prompt);
			expanded_data.append(input.data);
			if (inside) expanded_data.push_back('_');
		}
		const char* str = expanded_data.c_str();
		for (int i = 0; i < console->scroll && *str; ++i) {
			int len = 0; while (str[len] && str[len] != '\n' && len < console->cols) len++;
			str += len + (str[len] == '\n');
		}
		const char* end = str;
		for (int i = 0; i < console->rows && *end; ++i) {
			int len = 0; while (end[len] && end[len] != '\n' && len < console->cols) len++;
			end += len + (end[len] == '\n');
		}
		int txt_clr = console->txt_clr < 0 ? (-console->txt_clr - 1) : H(console->txt_clr);
		ImmediateTextW(e->x + 5, e->y + 5, std::string(str, end).c_str(), txt_clr, console->cols);
		if (inside && console->txt_clr >= 0) {
			XSetForeground(display, gc, colors[L(console->txt_clr)]);
			char buffer_1[64], buffer_2[64];
			snprintf(buffer_1, sizeof buffer_1, "Viewing: %d - %d / %d",
				console->scroll + 1, console->scroll + console->rows, console->total_lines);
			snprintf(buffer_2, sizeof buffer_2, "Scroll: %d%%",
				((console->scroll) * 100) / std::max(1, (console->total_lines - console->rows)));
			int length_1 = strlen(buffer_1), length_2 = strlen(buffer_2);
			XSetBackground(display, gc, colors[L(console->con_clr)]);
			XDrawImageString(display, bb, gc, e->x + e->w - length_1 * 9 - 12, e->y + 16, buffer_1, length_1);
			XDrawImageString(display, bb, gc, e->x + e->w - length_2 * 9 - 12, e->y + 31, buffer_2, length_2);
		}
	}

	void Ellipse(Element* e, bool inside) {
		auto ellipse = (EllipseElement *)e->elem;
		ImmediateEllipse(e->x, e->y, e->w, e->h, ellipse->fg, ellipse->bg);
	}

	void OpenGL(Element* e, bool inside) {
		auto opengl = (OpenGLElement *)e->elem;
		int x = opengl->border_color >= 0 ? e->x + 1 : e->x;
		int y = opengl->border_color >= 0 ? e->y + 1 : e->y;
		XCopyArea(display, opengl->x_pixmap, bb, gc, 0, 0, e->w, e->h, x, y);
		if (opengl->border_color >= 0) {
			XSetForeground(display, gc, colors[opengl->border_color]);
			XDrawRectangle(display, bb, gc, e->x, e->y, e->w - 1, e->h - 1);
		}
	}

	void (*Functions[])(Element*, bool) = {
		Text, Button, Input, Rect, Image, Checkbox, Console, Ellipse, OpenGL
	};
}
