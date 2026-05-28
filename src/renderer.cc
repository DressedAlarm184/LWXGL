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
		int inside = mouse_x >= btn->x && mouse_x <= btn->x + btn->w &&
			mouse_y >= btn->y && mouse_y <= btn->y + btn->h && !State::active_modal_state.active;
		if (inside) {
			XSetForeground(display, gc, mouse_down == 1 ? colors[btn->bgp] : colors[btn->bgh]);
		} else XSetForeground(display, gc, colors[btn->bgu]);
		XFillRectangle(display, bb, gc, btn->x + 1, btn->y + 1, btn->w - 1, btn->h - 1);
		if (inside) {
			XSetForeground(display, gc, mouse_down == 1 ? colors[btn->fgp] : colors[btn->fgh]);
		} else XSetForeground(display, gc, colors[btn->fgu]);
		XDrawRectangle(display, bb, gc, btn->x, btn->y, btn->w - 1, btn->h - 1);
		XDrawString(display, bb, gc, btn->x + (btn->w / 2) - (strlen(btn->label) * 9) / 2, btn->y + btn->h / 2 + 4, btn->label, strlen(btn->label));
	}

	void Input(Element* e) {
		InputElement *input = (InputElement *)e->elem;
		int inside = mouse_x >= input->x && mouse_x <= input->x + input->w &&
			mouse_y >= input->y && mouse_y <= input->y + input->h && !State::active_modal_state.active;
		if (inside) {
			XSetForeground(display, gc, colors[input->bgh]);
		} else XSetForeground(display, gc, colors[input->bgu]);
		XFillRectangle(display, bb, gc, input->x + 1, input->y + 1, input->w - 1, input->h - 1);
		if (inside) {
			XSetForeground(display, gc, colors[input->fgh]);
		} else XSetForeground(display, gc, colors[input->fgu]);
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

	void (*Functions[])(Element*) = {
		Text, Button, Input, Rect, Image
	};

	void DrawDebugOverlay() {
		int wt = 0; for (int i = 0; i < 60; i++) wt += State::debug_metrics.avg_wt[i]; wt /= 60;
		XSetForeground(display, gc, colors[0]);
		XFillRectangle(display, bb, gc, 5, 5, 140, 40);
		XSetForeground(display, gc, colors[15]);
		XDrawRectangle(display, bb, gc, 7, 7, 135, 35);
		char wt_buffer[32] = {0}, fps_buffer[32] = {0};
		int wt_len = sprintf(wt_buffer, "FT: %d (us)", wt);
		int fps_len = sprintf(fps_buffer, "FPS: %.1f", State::debug_metrics.fps);
		XDrawString(display, bb, gc, 11, 23, wt_buffer, wt_len);
		XDrawString(display, bb, gc, 11, 37, fps_buffer, fps_len);
	}

	void DrawActiveModal() {
		XSetForeground(display, gc, colors[0]);
		XFillRectangle(display, bb, gc, win_w / 2 - 153, 47, 306, 156);
		XSetForeground(display, gc, colors[15]);
		XDrawRectangle(display, bb, gc, win_w / 2 - 151, 49, 301, 151);
		int y = 68; const char* str = State::active_modal_state.msg;
		while (*str != '\0') {
			int len = 0;
			while (str[len] != '\0' && str[len] != '\n' && len < 31) len++;
			XDrawString(display, bb, gc, win_w / 2 - 141, y, str, len);
			str += len, y += 16;;
			if (*str == '\n') str++;
		}
		XSetForeground(display, gc, colors[10]);
		XDrawString(display, bb, gc, win_w / 2 + 125, 193, "OK", 2);
		if (State::active_modal_state.type == 1) {
			XSetForeground(display, gc, colors[12]);
			XDrawString(display, bb, gc, win_w / 2 + 55, 193, "Cancel", 6);
		}
	}
}

void GRenderWindow() {
	XSetForeground(display, gc, colors[bgcol]);
	XFillRectangle(display, bb, gc, 0, 0, win_w, win_h);

	for (int i = 0; i < elements.size(); i++) {
		Element *e = elements[i];
		if (e == NULL) continue;
		Renderers::Functions[e->type](e);
	}

	if (State::active_modal_state.active == 1) Renderers::DrawActiveModal();

	if (State::debug_metrics.active == 1 && State::debug_metrics.enabled == 1)
		Renderers::DrawDebugOverlay();

	XCopyArea(display, bb, window, gc, 0, 0, win_w, win_h, 0, 0);
	XSync(display, False);
}