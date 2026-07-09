int _translate_keypress(int ch, KeySym& keysym) {
	if (ch == 0) {
		switch (keysym) {
			case XK_Left: ch = LWXGL_KEY_LEFT; break;
			case XK_Right: ch = LWXGL_KEY_RIGHT; break;
			case XK_Up: ch = LWXGL_KEY_UP; break;
			case XK_Down: ch = LWXGL_KEY_DOWN; break;
		}
		if (keysym >= XK_F1 && keysym <= XK_F12) {
			ch = keysym - XK_F1 + LWXGL_KEY_FN + 1;
		}
	}
	return ch;
}

namespace Events {
	namespace UserProvided {
		void (*Key)(int key) = NULL;
		void (*Click)(int x, int y, int btn) = NULL;
		int (*Delete)() = NULL;
		void (*Resize)(int w, int h) = NULL;
	}

	void EClientMessage(XEvent& event) {
		if ((Atom)event.xclient.data.l[0] == wm_delete) GDeleteWindow();
	}

	void EMotionNotify(XEvent& event) {
		mouse_x = event.xmotion.x, mouse_y = event.xmotion.y;
	}

	void ELeaveNotify(XEvent& event) {
		mouse_x = -1, mouse_y = -1;
	}

	void EButtonPress(XEvent& event) {
		if (mouse_down == 0) mouse_down = event.xbutton.button;
	}

	void EButtonRelease(XEvent& event) {
		if (event.xbutton.button == mouse_down) mouse_down = 0;
		if (GQueryModalOpen()) {
			if (mouse_y < 200 && mouse_y > 180 && mouse_x > (win_w / 2 + 120) && mouse_x < (win_w / 2 + 150)) {
				if (active_modal_state.on_confirm != NULL) active_modal_state.on_confirm();
				active_modal_state.active = 0;
			} else if (mouse_y < 200 && mouse_y > 180 && mouse_x > (win_w / 2 + 50) && mouse_x < (win_w / 2 + 115)) {
				if (active_modal_state.type == 1) active_modal_state.active = 0;
			}
			return;
		}
		for (int i = 0; i < elements.size(); i++) {
			Element *e = elements[i];
			if (e == NULL) continue;
			if (!_inside_elem(e)) continue;
			if (e->type == 1) {
				ButtonElement *btn = (ButtonElement *)e->elem;
				if (event.xbutton.button != 1) return;
				if (btn->onclick != NULL) btn->onclick();
				return;
			} else if (e->type == 5) {
				CheckboxElement *checkbox = (CheckboxElement *)e->elem;
				checkbox->checked = !checkbox->checked;
				return;
			} else if (e->type == 6) {
				auto* console = static_cast<ConsoleElement*>(e->elem);
				if (event.xbutton.button == 5) {
					console->scroll += 3;
				} else if (event.xbutton.button == 4) {
					console->scroll -= 3;
				}
				const int max_scroll = std::max(0, console->total_lines - console->rows);
				console->scroll = std::clamp(console->scroll, 0, max_scroll);
				return;
			}
		}
		if (UserProvided::Click != NULL) {
			UserProvided::Click(mouse_x, mouse_y, event.xbutton.button);
		}
	}

	void EKeyPress(XEvent& event) {
		XKeyEvent key = event.xkey; KeySym keysym;
		unsigned char ch = 0; int len = XLookupString(&key, (char*)&ch, 1, &keysym, NULL);
		ch = _translate_keypress(ch, keysym);
		if (ch == 0) return;
		if (keysym == XK_Escape && (key.state & ControlMask)) {
			GDeleteWindow();
			return;
		}
		if (keysym == XK_F12) {
			debug_metrics.enabled = !debug_metrics.enabled;
			return;
		}
		bool already_pressed = false;
		for (int i = 0; i < 8; i++) {
			if (active_keycodes[i] != key.keycode) continue;
			pressed_keys[i] = ch;
			already_pressed = true;
			break;
		}
		if (!already_pressed) {
			for (int i = 0; i < 8; i++) {
				if (active_keycodes[i] != 0) continue;
				pressed_keys[i] = ch;
				active_keycodes[i] = key.keycode;
				break;
			}
		}
		if (GQueryModalOpen()) return;
		for (int i = 0; i < elements.size(); i++) {
			Element *e = elements[i];
			if (e == NULL) continue;
			if (!_inside_elem(e)) continue;
			if (e->type == 2) {
				InputElement *input = (InputElement *)e->elem;
				int length = strlen(input->input);
				if (ch == 8) {
					if (length > 0) input->input[length - 1] = 0;
				} else if (ch >= 32 && ch < 127) {
					if (length < input->max) input->input[length] = ch;
				}
				return;
			} else if (e->type == 6) {
				ConsoleElement *console = (ConsoleElement *)e->elem;
				if (ch == 32) console->scroll = std::max(0, console->total_lines - console->rows);
				return;
			}
		}
		if (UserProvided::Key != NULL) {
			UserProvided::Key(ch);
		}
	}

	void EKeyRelease(XEvent& event) {
		for (int i = 0; i < 8; i++) {
			if (active_keycodes[i] == event.xkey.keycode) {
				pressed_keys[i] = 0;
				active_keycodes[i] = 0;
			}
		}
	}

	void EConfigureNotify(XEvent& event) {
		int new_width = event.xconfigure.width;
		int new_height = event.xconfigure.height;

		XEvent next_event;
		while (XCheckTypedWindowEvent(display, window, ConfigureNotify, &next_event)) {
			new_width = next_event.xconfigure.width, new_height = next_event.xconfigure.height;
		}

		if (new_width != win_w || new_height != win_h) {
			win_w = new_width, win_h = new_height;

			XFreePixmap(display, bb);
			bb = XCreatePixmap(display, window, win_w, win_h, DefaultDepth(display, screen));

			if (Events::UserProvided::Resize != NULL) {
				Events::UserProvided::Resize(win_w, win_h);
			}
		}
	}

	std::unordered_map<int, void(*)(XEvent&)>Handlers = {
		{ClientMessage, EClientMessage},
		{MotionNotify, EMotionNotify},
		{LeaveNotify, ELeaveNotify},
		{ButtonPress, EButtonPress},
		{ButtonRelease, EButtonRelease},
		{ConfigureNotify, EConfigureNotify},
		{KeyPress, EKeyPress},
		{KeyRelease, EKeyRelease},
	};
}

EXPORT void GHandleWindowEvents() {
	XEvent event;

	while (XPending(display) > 0) {
		XNextEvent(display, &event);
		auto it = Events::Handlers.find(event.type);
		if (it != Events::Handlers.end()) {
			it->second(event);
		}
	}
}

EXPORT void GEventAttachKey(void (*Key)(int key)) {
	Events::UserProvided::Key = Key;
}

EXPORT void GEventAttachClick(void (*Click)(int x, int y, int btn)) {
	Events::UserProvided::Click = Click;
}

EXPORT void GQueryMouse(int* x, int* y, int* btn) {
	*x = mouse_x, *y = mouse_y, *btn = mouse_down;
}

EXPORT void GEventAttachDelete(int (*on_exit)()) {
	Events::UserProvided::Delete = on_exit;
}

EXPORT unsigned char* GQueryKeyboard() {
	return pressed_keys;
}

EXPORT int GQueryKeyDown(int ch) {
	for (int i = 0; i < 8; i++) {
		if (pressed_keys[i] == ch) return 1;
	}
	return 0;
}
