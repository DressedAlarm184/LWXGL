namespace Renderers {
	void Text(Element* e) {
		TextElement *txt = (TextElement *)e->elem;
		XSetForeground(display, gc, colors[txt->color]);
		const char *str = txt->text;
		int y = e->y + 11;
		while (*str != '\0') {
			int len = 0;
			while (str[len] != '\0' && str[len] != '\n') len++;
			XDrawString(display, bb, gc, e->x, y, str, len);
			str += len, y += 15;
			if (*str == '\n') str++;
		}
	}

	void Button(Element* e) {
		ButtonElement *btn = (ButtonElement *)e->elem;
		int inside = _inside_elem(e) && !GQueryModalOpen();
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

	void Input(Element* e) {
		InputElement *input = (InputElement *)e->elem;
		int inside = _inside_elem(e) && !GQueryModalOpen();
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

	void Rect(Element* e) {
		RectElement *rect = (RectElement *)e->elem;
		if (rect->bg >= 0) {
			XSetForeground(display, gc, colors[rect->bg]);
			XFillRectangle(display, bb, gc, e->x, e->y, e->w, e->h);
		}
		if (rect->fg >= 0) {
			XSetForeground(display, gc, colors[rect->fg]);
			XDrawRectangle(display, bb, gc, e->x, e->y, e->w - 1, e->h - 1);
		}
	}

	void Image(Element* e) {
		ImageElement *img = (ImageElement *)e->elem;
		XCopyArea(display, img->pixmap, bb, gc, 0, 0, e->w, e->h, e->x, e->y);
	}

	void Checkbox(Element *e) {
		CheckboxElement *checkbox = (CheckboxElement *)e->elem;
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

	void Console(Element* e) {
		ConsoleElement *console = (ConsoleElement *)e->elem;
		XSetForeground(display, gc, colors[L(console->con_clr)]);
		XFillRectangle(display, bb, gc, e->x + 1, e->y + 1, e->w - 1, e->h - 1);
		XSetForeground(display, gc, colors[H(console->con_clr)]);
		XDrawRectangle(display, bb, gc, e->x, e->y, e->w - 1, e->h - 1);
		int thumb_height = std::max((console->total_lines <= 0)
			? (e->h - 6)
			: std::min(e->h - 6, std::max(1, int((e->h - 6) * ((float)console->rows / console->total_lines)))), 16);
		int denom = console->total_lines - console->rows, thumb_y = e->y + 3;
		if (denom > 0) thumb_y += ((e->h - 6) - thumb_height) * console->scroll / denom;
		XFillRectangle(display, bb, gc, e->x + e->w - 8, thumb_y, 5, thumb_height);
		XSetForeground(display, gc, colors[H(console->txt_clr)]);
		std::string expanded_data;
		expanded_data.reserve(console->data.length());
		for (char c : console->data) {
			if (c == '\t') {
				expanded_data.append("    ");
			} else {
				int is_valid = c == '\n' || (c >= 27 && c <= 126);
				expanded_data.push_back(is_valid ? c : '?');
			}
		}
		int current_line_idx = 0, line_start = 0, line_len = 0;
		int data_len = expanded_data.length();
		for (int i = 0; i <= data_len; ++i) {
			bool is_end = (i == data_len);
			bool is_newline = (!is_end && expanded_data[i] == '\n');
			if (line_len == console->cols || is_newline || is_end) {
				if (current_line_idx >= console->scroll && current_line_idx < console->scroll + console->rows) {
					int display_row = current_line_idx - console->scroll;
					XDrawString(display, bb, gc, e->x + 5, e->y + 16 + (display_row * 15), 
								expanded_data.c_str() + line_start, line_len);
				}
				current_line_idx++, line_start = i, line_len = 0;
				if (current_line_idx >= console->scroll + console->rows) break;
				!is_newline && !is_end ? line_len = 1 : line_start = i + 1;
			} else {
				line_len++;
			}
		}
		if (_inside_elem(e)) {
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

	void (*Functions[])(Element*) = {
		Text, Button, Input, Rect, Image, Checkbox, Console
	};

	void DrawDebugOverlay() {
		int wt = 0; for (int i = 0; i < 60; i++) wt += debug_metrics.avg_wt[i]; wt /= 60;
		XSetForeground(display, gc, colors[0]);
		XFillRectangle(display, bb, gc, 5, bb.scroll + 5, 140, 40);
		XSetForeground(display, gc, colors[15]);
		XDrawRectangle(display, bb, gc, 7, bb.scroll + 7, 135, 35);
		char wt_buffer[32] = {0}, fps_buffer[32] = {0};
		int wt_len = sprintf(wt_buffer, "FT: %d (us)", wt);
		int fps_len = sprintf(fps_buffer, "FPS: %.1f", debug_metrics.fps);
		XDrawString(display, bb, gc, 11, bb.scroll + 23, wt_buffer, wt_len);
		XDrawString(display, bb, gc, 11, bb.scroll + 37, fps_buffer, fps_len);
	}

	void DrawActiveModal() {
		int max_chars = (std::clamp((int)(win_w / 1.5f), 300, 550) - 9) / 9;
		int width = (max_chars * 9) + 9;
		XSetForeground(display, gc, colors[0]);
		XFillRectangle(display, bb, gc, win_w / 2 - (width + 4) / 2, bb.scroll + 47, width + 5, 156);
		XSetForeground(display, gc, colors[15]);
		XDrawRectangle(display, bb, gc, win_w / 2 - width / 2, bb.scroll + 49, width, 151);
		int y = bb.scroll + 68; const char* str = active_modal_state.msg;
		while (*str != '\0') {
			int len = 0;
			while (str[len] != '\0' && str[len] != '\n' && len < max_chars) len++;
			XDrawString(display, bb, gc, win_w / 2 - (width - 10) / 2, y, str, len);
			str += len, y += 15;
			if (*str == '\n') str++;
		}
		int edge = win_w / 2 + width / 2;
		XSetForeground(display, gc, colors[10]);
		XDrawString(display, bb, gc, edge - 25, bb.scroll + 193, "OK", 2);
		if (active_modal_state.type == 1) {
			XSetForeground(display, gc, colors[12]);
			XDrawString(display, bb, gc, edge - 95, bb.scroll + 193, "Cancel", 6);
		}
		active_modal_state.right_edge_x = edge;
	}
}

EXPORT void GRenderWindow() {
	XSetForeground(display, gc, colors[bgcol]);
	XFillRectangle(display, bb, gc, 0, bb.scroll, win_w, win_h);

	for (int i = 0; i < elements.size(); i++) {
		Element *e = elements[i];
		if (e == NULL) continue;

		if (e->v && (e->screen == active_screen || e->screen == -1))
			if (e->type == 0 || (e->y + e->h >= bb.scroll && e->y < bb.scroll + win_h))
				Renderers::Functions[e->type](e);
	}

	if (bb.scrollbar_color >= 0 && bb.h > win_h) {
		XSetForeground(display, gc, colors[L(bb.scrollbar_color)]);
		XFillRectangle(display, bb, gc, win_w - 9, bb.scroll, 9, win_h);
		XSetForeground(display, gc, colors[H(bb.scrollbar_color)]);
		int height = win_h * ((float)win_h / (float)bb.h);
		int y = bb.scroll + 2 + (int)(((float)bb.scroll / (bb.h - win_h)) * (win_h - height - 4));
		XFillRectangle(display, bb, gc, win_w - 7, y, 5, height);
	}

	if (GQueryModalOpen()) Renderers::DrawActiveModal();

	if (debug_metrics.active == 1 && debug_metrics.enabled == 1)
		Renderers::DrawDebugOverlay();

	XCopyArea(display, bb, window, gc, 0, bb.scroll, win_w, win_h, 0, 0);
	XSync(display, False);
}
