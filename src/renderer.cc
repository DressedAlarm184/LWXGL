namespace Renderers {
	void Text(Element* e) {
		TextElement *txt = (TextElement *)e->elem;
		XSetForeground(display, gc, colors[txt->color]);
		const char *start = txt->text, *p = txt->text;
		int y = txt->y;
		while (1) {
			if (*p == '\n' || *p == '\0') {
				int len = p - start;
				XDrawString(display, back_buffer, gc, txt->x, y, start, len);
				y += 16;
				if (*p == '\0') break;
				start = p + 1;
			}
			p++;
		}
	}

	void Button(Element* e) {
		ButtonElement *btn = (ButtonElement *)e->elem;
		int inside = mouse_x >= btn->x && mouse_x <= btn->x + btn->w &&
			mouse_y >= btn->y && mouse_y <= btn->y + btn->h;
		if (inside) {
			XSetForeground(display, gc, mouse_down == 1 ? colors[btn->bgp] : colors[btn->bgh]);
		} else XSetForeground(display, gc, colors[btn->bgu]);
		XFillRectangle(display, back_buffer, gc, btn->x + 1, btn->y + 1, btn->w - 1, btn->h - 1);
		if (inside) {
			XSetForeground(display, gc, mouse_down == 1 ? colors[btn->fgp] : colors[btn->fgh]);
		} else XSetForeground(display, gc, colors[btn->fgu]);
		XDrawRectangle(display, back_buffer, gc, btn->x, btn->y, btn->w - 1, btn->h - 1);
		XDrawString(display, back_buffer, gc, btn->x + (btn->w / 2) - (strlen(btn->label) * 9) / 2, btn->y + btn->h / 2 + 4, btn->label, strlen(btn->label));
	}

	void Input(Element* e) {
		InputElement *input = (InputElement *)e->elem;
		int inside = mouse_x >= input->x && mouse_x <= input->x + input->w &&
			mouse_y >= input->y && mouse_y <= input->y + input->h;
		if (inside) {
			XSetForeground(display, gc, colors[input->bgh]);
		} else XSetForeground(display, gc, colors[input->bgu]);
		XFillRectangle(display, back_buffer, gc, input->x + 1, input->y + 1, input->w - 1, input->h - 1);
		if (inside) {
			XSetForeground(display, gc, colors[input->fgh]);
		} else XSetForeground(display, gc, colors[input->fgu]);
		XDrawRectangle(display, back_buffer, gc, input->x, input->y, input->w - 1, input->h - 1);
		char buffer[128]; sprintf(buffer, "%s%c", input->input, inside ? '_' : ' ');
		XDrawString(display, back_buffer, gc, input->x + 5, input->y + input->h / 2 + 4, buffer, strlen(buffer));
	}

	void Rect(Element* e) {
		RectElement *rect = (RectElement *)e->elem;
		if (rect->bg >= 0) {
			XSetForeground(display, gc, colors[rect->bg]);
			XFillRectangle(display, back_buffer, gc, rect->x, rect->y, rect->w, rect->h);
		}
		if (rect->fg >= 0) {
			XSetForeground(display, gc, colors[rect->fg]);
			XDrawRectangle(display, back_buffer, gc, rect->x, rect->y, rect->w - 1, rect->h - 1);
		}
	}

	void Image(Element* e) {
		ImageElement *img = (ImageElement *)e->elem;
		XPutImage(display, back_buffer, gc, img->ximage, 0, 0, img->x, img->y, img->w, img->h);
	}

	void (*Functions[])(Element*) = {
		Text, Button, Input, Rect, Image
	};
}

void GRenderWindow() {
	XSetForeground(display, gc, colors[bgcol]);
	XFillRectangle(display, back_buffer, gc, 0, 0, win_w, win_h);

	for (int i = 0; i < elements.size(); i++) {
		Element *e = elements[i];
		if (e == NULL) continue;
		Renderers::Functions[e->type](e);
	}

	XCopyArea(display, back_buffer, window, gc, 0, 0, win_w, win_h, 0, 0);
	XSync(display, False);
}