EXPORT int CreateWindow(int w, int h, const char* name, int bgcolor) {
	if (window != None) return 3;

	win_w = w, win_h = h;

	display = XOpenDisplay(NULL);
	if (display == NULL) return 1;

	font = XLoadQueryFont(display, "9x15");
	if (!font) {
		XCloseDisplay(display);
		return 2;
	}

	screen = DefaultScreen(display);
	Colormap colormap = DefaultColormap(display, screen);
	XColor dummy_exact, xcolor;

	for (int i = 0; i < 16; i++) {
		xcolor.red   = color_palette[i].r * 257;
		xcolor.green = color_palette[i].g * 257;
		xcolor.blue  = color_palette[i].b * 257;
		xcolor.flags = DoRed | DoGreen | DoBlue;
		if (XAllocColor(display, colormap, &xcolor)) {
			colors[i] = xcolor.pixel;
		} else {
			if (i > 0) {
				XFreeColors(display, colormap, colors, i, 0);
			}
			XFreeFont(display, font);
			XCloseDisplay(display);
			return 127 + i;
		}
	}

	srand(time(NULL));

	gc = XCreateGC(display, RootWindow(display, screen), 0, NULL);
	XSetLineAttributes(display, gc, 1, LineSolid, CapButt, JoinMiter);
	XSetGraphicsExposures(display, gc, False);
	
	window = XCreateSimpleWindow(
		display,
		RootWindow(display, screen),
		0, 0, w, h, 1,
		BlackPixel(display, screen),
		WhitePixel(display, screen)
	);

	XStoreName(display, window, name);

	wm_delete = XInternAtom(display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(display, window, &wm_delete, 1);
	
	XSelectInput(display, window,
		ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
		KeyPressMask | LeaveWindowMask | KeyReleaseMask | StructureNotifyMask);
	
	XMapWindow(display, window);
	
	XSizeHints hints = {0};
	hints.flags = PMinSize | PMaxSize;
	hints.min_width = w, hints.min_height = h;
	hints.max_width = w, hints.max_height = h;
	XSetWMNormalHints(display, window, &hints);
	
	XSetFont(display, gc, font->fid);
	bb.new_bb(w, h);

	XkbSetDetectableAutoRepeat(display, True, NULL);

	bgcol = bgcolor;

	ChangeCursor(68);
	XSync(display, False);

	return 0;
}

EXPORT void TerminateWindow() {
	for (int i = 0; i < elements.size(); i++) {
		if (elements[i] != NULL) DeleteElement(i);
	}

	while (!allocated_TGAs.empty()) {
		DeleteTGA(allocated_TGAs.begin()->first.c_str());
	}

	if (active_modal_state.msg != NULL) {
		free(active_modal_state.msg);
	}

	XFreeFont(display, font);
	XFreeGC(display, gc);
	XFreePixmap(display, bb);
	XFreeColors(display, DefaultColormap(display, screen), colors, 16, 0);
	XDestroyWindow(display, window);
	XCloseDisplay(display);
}

EXPORT int WindowShouldClose() {
	return closing;
}

EXPORT void SimpleWindowLoop(int target_fps, void (*on_every)(int, float)) {
	using namespace std::chrono;
	
	debug_metrics.active = 1;
	const microseconds FRAME_TIME(1000000 / target_fps);
	unsigned long long tick = 0;
	auto last_time = steady_clock::now();

	while (!WindowShouldClose()) {
		auto now = steady_clock::now();
		auto elapsed = duration_cast<microseconds>(now - last_time);
		
		if (elapsed >= FRAME_TIME) {
			float delta_time = elapsed.count() / 1000000.0f;
			auto work_start = steady_clock::now();
			
			HandleWindowEvents();
			RenderWindow();

			if (on_every != NULL) on_every(tick, delta_time);
			
			auto work_time = duration_cast<microseconds>(steady_clock::now() - work_start);
			float current_fps = 1000000.0 / elapsed.count();
			
			for (int i = 0; i < 59; i++) debug_metrics.avg_wt[i] = debug_metrics.avg_wt[i + 1];
			debug_metrics.avg_wt[59] = work_time.count(), debug_metrics.fps = current_fps;
			
			last_time += FRAME_TIME, tick++;
			
			if (now - last_time > FRAME_TIME * 2) last_time = now;
		} else {
			auto time_to_sleep = FRAME_TIME - elapsed;
			if (time_to_sleep > milliseconds(2)) {
				std::this_thread::sleep_for(time_to_sleep - milliseconds(1));
			} else std::this_thread::yield();
		}
	}
}

EXPORT void DeleteWindow() {
	if (Events::UserProvided::Delete != NULL) {
		closing = Events::UserProvided::Delete();
	} else closing = 1;
}

EXPORT void SpawnModal(int type, const char* msg, void (*on_confirm)()) {
	if (active_modal_state.msg != NULL) {
		free(active_modal_state.msg);
	}

	active_modal_state.active = 1;
	active_modal_state.msg = strdup(msg);
	active_modal_state.on_confirm = on_confirm;
	active_modal_state.type = type;
}

EXPORT int QueryModalOpen() {
	return active_modal_state.active;
}

EXPORT void PaletteQuery(int idx, unsigned char* r, unsigned char* g, unsigned char* b) {
	XColor color;
	color.pixel = colors[idx];
	XQueryColor(display, DefaultColormap(display, screen), &color);
	*r = color.red / 257;
	*g = color.green / 257;
	*b = color.blue / 257;
}

EXPORT void PaletteModify(int idx, unsigned char r, unsigned char g, unsigned char b, int redraw) {
	XColor color;
	color.red   = r * 257;
	color.green = g * 257;
	color.blue  = b * 257;
	color.flags = DoRed | DoGreen | DoBlue;
	XFreeColors(display, DefaultColormap(display, screen), &colors[idx], 1, 0);
	XAllocColor(display, DefaultColormap(display, screen), &color);
	colors[idx] = color.pixel;
	if (redraw) RedrawAllImages();
}

EXPORT void PaletteReset() {
	XFreeColors(display, DefaultColormap(display, screen), colors, 16, 0);
	XColor color;
	for (int i = 0; i < 16; i++) {
		color.red   = color_palette[i].r * 257;
		color.green = color_palette[i].g * 257;
		color.blue  = color_palette[i].b * 257;
		color.flags = DoRed | DoGreen | DoBlue;
		XAllocColor(display, DefaultColormap(display, screen), &color);
		colors[i] = color.pixel;
	}
	RedrawAllImages();
}

EXPORT void SetWindowTitle(const char* title) {
	XStoreName(display, window, title);
}

EXPORT void SetWindowColor(int color) {
	bgcol = color;
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

EXPORT void ChangeCursor(int cursor_font_glyph) {
	Cursor cursor;

	if (cursor_font_glyph == 255) {
		char data[1] = {0};
		Pixmap blank = XCreateBitmapFromData(display, window, data, 1, 1);
		XColor dummy = {0};
		cursor = XCreatePixmapCursor(display, blank, blank, &dummy, &dummy, 0, 0);
		XFreePixmap(display, blank);
	} else {
		cursor = XCreateFontCursor(display, cursor_font_glyph);
	}

	XDefineCursor(display, window, cursor);
	XFreeCursor(display, cursor);
}

EXPORT void ReserveScroll(int height, int scrollbar_color, void (*Scroll)(int offset)) {
	bb.scroll_enabled = true;
	bb.scrollbar_color = scrollbar_color;
	bb.new_bb(win_w, height);
	Events::UserProvided::Scroll = Scroll;
}

EXPORT int QueryScroll() {
	return bb.scroll;
}

EXPORT int SetGlobalBold(int bold) {
	const char* xlfd = bold ? "9x15bold" : "9x15";
	XFontStruct* new_font = XLoadQueryFont(display, xlfd);
	if (!new_font) return 0;
	XFreeFont(display, font), font = new_font;
	XSetFont(display, gc, font->fid);
	return 1;
}
