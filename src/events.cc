namespace Events {
	void EButtonRelease(XEvent event) {
		if (event.xbutton.button == mouse_down) mouse_down = 0;
		if (event.xbutton.button != 1) return;
		for (int i = 0; i < elements.size(); i++) {
			Element *e = elements[i];
			if (e == NULL) continue;
			if (e->type == 1) {
				ButtonElement *btn = (ButtonElement *)e->elem;
				int inside = mouse_x >= btn->x && mouse_x <= btn->x + btn->w &&
					mouse_y >= btn->y && mouse_y <= btn->y + btn->h;
				if (inside) btn->onclick();
			}
		}	
	} 

	void EKeyPress(XEvent event) {
		XKeyEvent key = event.xkey; KeySym keysym;
		char ch; int len = XLookupString(&key, &ch, 1, &keysym, NULL);
		ch = (len == 0) ? 0 : ch;
		for (int i = 0; i < elements.size(); i++) {
			Element *e = elements[i];
			if (e == NULL) continue;
			if (e->type == 2) {
				InputElement *input = (InputElement *)e->elem;
				int inside = mouse_x >= input->x && mouse_x <= input->x + input->w &&
								mouse_y >= input->y && mouse_y <= input->y + input->h;
				if (!inside) continue;
				int length = strlen(input->input);
				if (ch == 8) {
					if (length > 0) input->input[length - 1] = 0;
				} else if (ch >= 32 && ch < 127) {
					if (length < input->max) input->input[length] = ch;
				}
			}
		}
	}
}

void GHandleWindowEvents() {
	while (XPending(display) > 0) {
		XNextEvent(display, &event);
		if (event.type == Expose) {
			GRenderWindow();
		} else if (event.type == ClientMessage && (Atom)event.xclient.data.l[0] == wm_delete) {
			closing = 1;
		} else if (event.type == MotionNotify) {
			mouse_x = event.xmotion.x, mouse_y = event.xmotion.y;
		} else if (event.type == LeaveNotify) {
			mouse_x = -1, mouse_x = -1;
		} else if (event.type == ButtonPress) {
			if (mouse_down == 0) mouse_down = event.xbutton.button;
		} else if (event.type == ButtonRelease) {
			Events::EButtonRelease(event);
		} else if (event.type == KeyPress) {
			Events::EKeyPress(event);
		}
	}
}