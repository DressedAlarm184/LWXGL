EXPORT void _handle_window_events() {
	XEvent event;

	while (XPending(display) > 0) {
		XNextEvent(display, &event);
		auto it = Events::Handlers.find(event.type);
		if (it != Events::Handlers.end()) {
			it->second(event);
		}
	}
}

EXPORT void EventAttachKey(void (*Key)(int key)) {
	Events::UserProvided::Key = Key;
}

EXPORT void EventAttachClick(void (*Click)(int x, int y, int btn)) {
	Events::UserProvided::Click = Click;
}

EXPORT void QueryMouse(int* x, int* y, int* btn) {
	*x = mouse_x, *y = mouse_y, *btn = mouse_down;
}

EXPORT void EventAttachDelete(int (*on_exit)()) {
	Events::UserProvided::Delete = on_exit;
}

EXPORT unsigned char* QueryKeyboard() {
	return pressed_keys;
}

EXPORT int QueryKeyDown(int ch) {
	for (int i = 0; i < 8; i++) {
		if (pressed_keys[i] == ch) return 1;
	}
	return 0;
}

EXPORT unsigned char* QueryKeyboardRaw() {
	return active_keycodes;
}

EXPORT int QueryKeysymDown(unsigned long keysym) {
	unsigned char keycode = XKeysymToKeycode(display, keysym);
	if (keycode == 0) return 0;

	for (int i = 0; i < 8; i++) {
		if (active_keycodes[i] == keycode) return 1;
	}

	return 0;
}

EXPORT void EnableResizing(void (*Resize)(int x, int y)) {
	XSizeHints hints = {0};
	hints.flags = bb.scroll_enabled ? PMaxSize : 0;
	if (bb.scroll_enabled) {
		hints.max_width = 32767;
		hints.max_height = bb.h;
	}
	XSetWMNormalHints(display, window, &hints);
	Events::UserProvided::Resize = Resize;
}
