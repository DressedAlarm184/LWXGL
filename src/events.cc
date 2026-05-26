namespace Events {
	namespace UserProvided {
		void (*Key)(int key) = NULL;
		void (*Click)(int x, int y, int btn) = NULL;
	}

	void EButtonRelease(XEvent event) {
		if (event.xbutton.button == mouse_down) mouse_down = 0;
		for (int i = 0; i < elements.size(); i++) {
			Element *e = elements[i];
			if (e == NULL) continue;
			if (e->type == 1) {
				ButtonElement *btn = (ButtonElement *)e->elem;
				int inside = mouse_x >= btn->x && mouse_x <= btn->x + btn->w &&
				mouse_y >= btn->y && mouse_y <= btn->y + btn->h;
				if (inside) {
					if (event.xbutton.button != 1) return;
					btn->onclick();
					return;
				}
			}
		}
		if (UserProvided::Click != NULL) {
			UserProvided::Click(mouse_x, mouse_y, event.xbutton.button);
		}
	}

	void EKeyPress(XEvent event) {
		XKeyEvent key = event.xkey; KeySym keysym;
		unsigned char ch = 0; int len = XLookupString(&key, (char*)&ch, 1, &keysym, NULL);
		ch = (len == 0) ? 0 : ch;
		if (keysym == XK_F12) {
			debug_metrics.enabled = !debug_metrics.enabled;
			return;
		}
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
				return;
			}
		}
		if (UserProvided::Key != NULL) {
			if (ch == 0) {
				switch (keysym) {
					case XK_Left: ch = GKeyLeft; break;
					case XK_Right: ch = GKeyRight; break;
					case XK_Up: ch = GKeyUp; break;
					case XK_Down: ch = GKeyDown; break;
				}
				if (keysym >= XK_F1 && keysym <= XK_F12) {
					ch = keysym - XK_F1 + GKeyFnBase + 1;
				}
			}
			UserProvided::Key(ch);
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

void GEventAttachKey(void (*Key)(int key)) {
	Events::UserProvided::Key = Key;
}

void GEventAttachClick(void (*Click)(int x, int y, int btn)) {
	Events::UserProvided::Click = Click;
}