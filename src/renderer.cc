namespace Renderers {
	void Text(Element* e) {
		TextElement *txt = (TextElement *)e->elem;
		XSetForeground(display, gc, colors[txt->color]);
		const char *str = txt->text;
		int y = txt->y + 11;
		while (*str != '\0') {
			int len = 0;
			while (str[len] != '\0' && str[len] != '\n') len++;
			XDrawString(display, bb, gc, txt->x, y, str, len);
			str += len, y += 16;;
			if (*str == '\n') str++;
		}
	}

	void Button(Element* e) {
		ButtonElement *btn = (ButtonElement *)e->elem;
		int inside = mouse_x >= btn->x && mouse_x < btn->x + btn->w &&
		mouse_y >= btn->y && mouse_y < btn->y + btn->h && !GQueryModalOpen();
		if (inside) {
			XSetForeground(display, gc, colors[mouse_down == 1 ? L(btn->pressed) : L(btn->hover)]);
		} else XSetForeground(display, gc, colors[L(btn->unpressed)]);
		XFillRectangle(display, bb, gc, btn->x + 1, btn->y + 1, btn->w - 1, btn->h - 1);
		if (inside) {
			XSetForeground(display, gc, colors[mouse_down == 1 ? H(btn->pressed) : H(btn->hover)]);
		} else XSetForeground(display, gc, colors[H(btn->unpressed)]);
		XDrawRectangle(display, bb, gc, btn->x, btn->y, btn->w - 1, btn->h - 1);
		XDrawString(display, bb, gc, btn->x + (btn->w / 2) - (strlen(btn->label) * 9) / 2, btn->y + btn->h / 2 + 4, btn->label, strlen(btn->label));
	}

	void Input(Element* e) {
		InputElement *input = (InputElement *)e->elem;
		int inside = mouse_x >= input->x && mouse_x < input->x + input->w &&
		mouse_y >= input->y && mouse_y < input->y + input->h && !GQueryModalOpen();
		if (inside) {
			XSetForeground(display, gc, colors[L(input->hover)]);
		} else XSetForeground(display, gc, colors[L(input->inactive)]);
		XFillRectangle(display, bb, gc, input->x + 1, input->y + 1, input->w - 1, input->h - 1);
		if (inside) {
			XSetForeground(display, gc, colors[H(input->hover)]);
		} else XSetForeground(display, gc, colors[H(input->inactive)]);
		XDrawRectangle(display, bb, gc, input->x, input->y, input->w - 1, input->h - 1);
		char buffer[128]; sprintf(buffer, "%s%c", input->input, inside ? '_' : ' ');
		XDrawString(display, bb, gc, input->x + 5, input->y + input->h / 2 + 4, buffer, strlen(buffer));
	}

	void Rect(Element* e) {
		RectElement *rect = (RectElement *)e->elem;
		if (rect->bg >= 0) {
			XSetForeground(display, gc, colors[rect->bg]);
			XFillRectangle(display, bb, gc, rect->x, rect->y, rect->w, rect->h);
		}
		if (rect->fg >= 0) {
			XSetForeground(display, gc, colors[rect->fg]);
			XDrawRectangle(display, bb, gc, rect->x, rect->y, rect->w - 1, rect->h - 1);
		}
	}

	void Image(Element* e) {
		ImageElement *img = (ImageElement *)e->elem;
		XPutImage(display, bb, gc, img->ximage, 0, 0, img->x, img->y, img->w, img->h);
	}

	void Checkbox(Element *e) {
		CheckboxElement *checkbox = (CheckboxElement *)e->elem;
		XSetForeground(display, gc, colors[L(checkbox->cb_col)]);
		XFillRectangle(display, bb, gc, checkbox->x + 1, checkbox->y + 1, checkbox->s - 1, checkbox->s - 1);
		XSetForeground(display, gc, colors[H(checkbox->cb_col)]);
		XDrawRectangle(display, bb, gc, checkbox->x, checkbox->y, checkbox->s - 1, checkbox->s - 1);
		if (checkbox->checked) XFillRectangle(display, bb, gc, checkbox->x + 4, checkbox->y + 4, checkbox->s - 8, checkbox->s - 8);
		if (checkbox->label != NULL) {
			XSetForeground(display, gc, colors[checkbox->txt_col]);
			XDrawString(display, bb, gc, checkbox->x + checkbox->s + 3, checkbox->y + checkbox->s / 2 + 5, checkbox->label, strlen(checkbox->label));
		}
	}

	void (*Functions[])(Element*) = {
		Text, Button, Input, Rect, Image, Checkbox
	};

	void DrawDebugOverlay() {
		int wt = 0; for (int i = 0; i < 60; i++) wt += debug_metrics.avg_wt[i]; wt /= 60;
		XSetForeground(display, gc, colors[0]);
		XFillRectangle(display, bb, gc, 5, 5, 140, 40);
		XSetForeground(display, gc, colors[15]);
		XDrawRectangle(display, bb, gc, 7, 7, 135, 35);
		char wt_buffer[32] = {0}, fps_buffer[32] = {0};
		int wt_len = sprintf(wt_buffer, "FT: %d (us)", wt);
		int fps_len = sprintf(fps_buffer, "FPS: %.1f", debug_metrics.fps);
		XDrawString(display, bb, gc, 11, 23, wt_buffer, wt_len);
		XDrawString(display, bb, gc, 11, 37, fps_buffer, fps_len);
	}

	void DrawActiveModal() {
		XSetForeground(display, gc, colors[0]);
		XFillRectangle(display, bb, gc, win_w / 2 - 153, 47, 306, 156);
		XSetForeground(display, gc, colors[15]);
		XDrawRectangle(display, bb, gc, win_w / 2 - 151, 49, 301, 151);
		int y = 68; const char* str = active_modal_state.msg;
		while (*str != '\0') {
			int len = 0;
			while (str[len] != '\0' && str[len] != '\n' && len < 31) len++;
			XDrawString(display, bb, gc, win_w / 2 - 141, y, str, len);
			str += len, y += 16;;
			if (*str == '\n') str++;
		}
		XSetForeground(display, gc, colors[10]);
		XDrawString(display, bb, gc, win_w / 2 + 125, 193, "OK", 2);
		if (active_modal_state.type == 1) {
			XSetForeground(display, gc, colors[12]);
			XDrawString(display, bb, gc, win_w / 2 + 55, 193, "Cancel", 6);
		}
	}
}

EXPORT void GRenderWindow() {
	XSetForeground(display, gc, colors[bgcol]);
	XFillRectangle(display, bb, gc, 0, 0, win_w, win_h);

	for (int i = 0; i < elements.size(); i++) {
		Element *e = elements[i];
		if (e == NULL) continue;
		Renderers::Functions[e->type](e);
	}

	if (GQueryModalOpen()) Renderers::DrawActiveModal();

	if (debug_metrics.active == 1 && debug_metrics.enabled == 1)
		Renderers::DrawDebugOverlay();

	XCopyArea(display, bb, window, gc, 0, 0, win_w, win_h, 0, 0);
	XSync(display, False);
}