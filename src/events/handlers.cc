int _translate_keypress(int ch, KeySym& keysym) {
	if (ch == 0) {
		switch (keysym) {
			case XK_Left: ch = KEY_LEFT; break;
			case XK_Right: ch = KEY_RIGHT; break;
			case XK_Up: ch = KEY_UP; break;
			case XK_Down: ch = KEY_DOWN; break;
		}
		if (keysym >= XK_F1 && keysym <= XK_F12) {
			ch = keysym - XK_F1 + KEY_FN + 1;
		}
	}
	if (ch == 13) ch = 10;
	return ch;
}

namespace Events {
	namespace UserProvided {
		void (*Key)(int key) = NULL;
		void (*Click)(int x, int y, int btn) = NULL;
		int (*Delete)() = NULL;
		void (*Resize)(int w, int h) = NULL;
		void (*Scroll)(int offset) = NULL;
	}

	void EClientMessage(XEvent& event) {
		if ((Atom)event.xclient.data.l[0] == wm_delete) DeleteWindow();
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
		int button = event.xbutton.button;
		if (button == mouse_down) mouse_down = 0;
		if (QueryModalOpen()) {
			int edge = active_modal_state.right_edge_x;
			if (mouse_y < 200 && mouse_y > 180 && mouse_x > edge - 35 && mouse_x < edge) {
				active_modal_state.active = 0;
				if (active_modal_state.on_confirm != NULL) active_modal_state.on_confirm();
			} else if (mouse_y < 200 && mouse_y > 180 && mouse_x > edge - 105 && mouse_x < edge - 35) {
				if (active_modal_state.type != MODAL_ALERT) active_modal_state.active = 0;
			}
			return;
		}
		for (Element* e : elements) {
			if (e == NULL) continue;
			if (!_inside_elem(e)) continue;
			if (e->type == 1 && button == 1) {
				auto btn = (ButtonElement *)e->elem;
				if (btn->onclick != NULL) btn->onclick();
				return;
			} else if (e->type == 5 && button == 1) {
				auto checkbox = (CheckboxElement *)e->elem;
				checkbox->checked = !checkbox->checked;
				return;
			} else if (e->type == 6) {
				auto console = (ConsoleElement*)e->elem;
				console->scroll += (button == 5) ? 3 : -3;
				const int max_scroll = std::max(0, console->total_lines - console->rows);
				console->scroll = std::clamp(console->scroll, 0, max_scroll);
				return;
			}
		}
		if (bb.scroll_enabled && (button == 5 || button == 4)) {
			int old_scroll = bb.scroll;
			int delta = button == 5 ? 45 : -45;
			bb.scroll = std::clamp(bb.scroll + delta, 0, std::max(0, bb.h - win_h));
			if (bb.scroll != old_scroll && UserProvided::Scroll != NULL) {
				UserProvided::Scroll(bb.scroll);
			}
			return;
		}
		if (UserProvided::Click != NULL) {
			UserProvided::Click(mouse_x, mouse_y, button);
		}
	}

	void EKeyPress(XEvent& event) {
		XKeyEvent key = event.xkey; KeySym keysym;
		unsigned char ch = 0; XLookupString(&key, (char*)&ch, 1, &keysym, NULL);
		if ((ch = _translate_keypress(ch, keysym)) == 0) return;
		if (keysym == XK_Escape && (key.state & ControlMask)) {
			DeleteWindow();
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
		if (QueryModalOpen()) {
			if (active_modal_state.type != MODAL_INPUT) return;
			char* input = active_modal_state.input;
			int length = strlen(input);
			if (ch == 8) {
				if (length > 0) input[length - 1] = 0;
			} else if ((ch >= 32 && ch < 127) || ch == 10) {
				if (length < 150) input[length] = ch;
			}
			return;
		}
		for (Element* e : elements) {
			if (e == NULL) continue;
			if (!_inside_elem(e)) continue;
			if (e->type == 2) {
				auto input = (InputElement *)e->elem;
				int length = strlen(input->input);
				if (ch == 8) {
					if (length > 0) input->input[length - 1] = 0;
				} else if (ch >= 32 && ch < 127) {
					if (length < input->max) input->input[length] = ch;
				}
				return;
			} else if (e->type == 6) {
				auto console = (ConsoleElement *)e->elem;
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

			int old_scroll = bb.scroll;
			bb.new_bb(new_width, bb.scroll_enabled ? bb.h : new_height);

			if (Events::UserProvided::Resize != NULL) {
				Events::UserProvided::Resize(win_w, win_h);
			}

			if (bb.scroll != old_scroll && Events::UserProvided::Scroll != NULL) {
				Events::UserProvided::Scroll(bb.scroll);
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
