EXPORT int GCreateWindow(int w, int h, const char* name, int bgcolor) {
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
		ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
		KeyPressMask | LeaveWindowMask | KeyReleaseMask);
	
	XMapWindow(display, window);
	
	XSizeHints hints;
	hints.flags = PMinSize | PMaxSize;
	hints.min_width = w; hints.min_height = h;
	hints.max_width = w; hints.max_height = h;
	XSetWMNormalHints(display, window, &hints);
	
	XSetFont(display, gc, font->fid);
	bb = XCreatePixmap(display, window, w, h, DefaultDepth(display, screen));

	XkbSetDetectableAutoRepeat(display, True, NULL);

	bgcol = bgcolor;

	XSync(display, False);

	return 0;
}

EXPORT void GTerminateWindow() {
	for (int i = 0; i < elements.size(); i++) {
		if (elements[i] != NULL) {
			GDeleteElement(i);
		}
	}
	XFreeFont(display, font);
	XFreeGC(display, gc);
	XFreePixmap(display, bb);
	XFreeColors(display, DefaultColormap(display, screen), colors, 16, 0);
	XDestroyWindow(display, window);
	XCloseDisplay(display);
}

EXPORT int GWindowShouldClose() {
	return closing;
}

EXPORT void GSimpleWindowLoop(int target_fps, void (*on_every)(int, float)) {
	using namespace std::chrono;
	
	debug_metrics.active = 1;
	const microseconds FRAME_TIME(1000000/target_fps);
	unsigned long long tick = 0;
	auto last_time = steady_clock::now();

	while (!GWindowShouldClose()) {
		auto now = steady_clock::now();
		auto elapsed = duration_cast<microseconds>(now - last_time);
		
		if (elapsed >= FRAME_TIME) {
			float delta_time = elapsed.count() / 1000000.0f;
			auto work_start = steady_clock::now();
			
			GHandleWindowEvents();
			GRenderWindow(); 

			if (on_every != NULL) on_every(tick, delta_time);
			
			auto work_time = duration_cast<microseconds>(steady_clock::now() - work_start);
			float current_fps = 1000000.0 / elapsed.count();
			
			for (int i = 0; i < 59; i++) debug_metrics.avg_wt[i] = debug_metrics.avg_wt[i + 1];
			debug_metrics.avg_wt[59] = work_time.count();
			debug_metrics.fps = current_fps;
			
			last_time += FRAME_TIME, tick++;
			
			if (now - last_time > FRAME_TIME * 2) {
				last_time = now;
			}
		} else {
			auto time_to_sleep = FRAME_TIME - elapsed;
			if (time_to_sleep > milliseconds(2)) {
				std::this_thread::sleep_for(time_to_sleep - milliseconds(1));
			} else std::this_thread::yield();
		}
	}
}

EXPORT void GDeleteWindow() {
	if (Events::UserProvided::Delete != NULL) {
		closing = Events::UserProvided::Delete();
	} else closing = 1;
}

EXPORT void GSpawnModal(int type, const char* msg, void (*on_confirm)()) {
	active_modal_state.active = 1;
	active_modal_state.msg = msg;
	active_modal_state.on_confirm = on_confirm;
	active_modal_state.type = type;
}

EXPORT int GQueryModalOpen() {
	return active_modal_state.active;
}

EXPORT void GPaletteQuery(int idx, unsigned char* r, unsigned char* g, unsigned char* b) {
	XColor color;
	color.pixel = colors[idx];
	XQueryColor(display, DefaultColormap(display, screen), &color);
	*r = color.red / 257;
	*g = color.green / 257;
	*b = color.blue / 257;
}

EXPORT void GPaletteModify(int idx, unsigned char r, unsigned char g, unsigned char b, int redraw) {
	XColor color;
	color.red   = r * 257;
	color.green = g * 257;
	color.blue  = b * 257;
	color.flags = DoRed | DoGreen | DoBlue;
	XFreeColors(display, DefaultColormap(display, screen), &colors[idx], 1, 0);
	XAllocColor(display, DefaultColormap(display, screen), &color);
	colors[idx] = color.pixel;
	if (redraw) GRedrawAllImages();
}

EXPORT void GPaletteReset() {
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
	GRedrawAllImages();
}

EXPORT int* GScreenActive() {
	return &active_screen;
}

EXPORT void GScreenApply(int s, int ids[], int count) {
	for (int i = 0; i < count; i++) {
		elements[ids[i]]->screen = s;
	}
}

EXPORT void GSetWindowTitle(const char* title) {
	XStoreName(display, window, title);
}

EXPORT void GSetWindowColor(int color) {
	bgcol = color;
}

EXPORT unsigned char* GCaptureRegion(int x, int y, int w, int h, int* size) {
	XImage *image = XGetImage(display, bb, x, y, w, h, AllPlanes, ZPixmap);
	Colormap colormap = DefaultColormap(display, screen);
	XColor color;

	char header[64] = {0};
	int header_size = snprintf(header, sizeof header, "P6\n%d %d\n255\n", w, h);
	int buffer_size = header_size + (w * h * 3);
	unsigned char* buffer = (unsigned char*)malloc(buffer_size);
	memcpy(buffer, header, header_size);

	for (int py = 0; py < h; py++) {
		for (int px = 0; px < w; px++) {
			color.pixel = XGetPixel(image, px, py);
			XQueryColor(display, colormap, &color);

			int byte_offset = header_size + ((py * w + px) * 3);

			buffer[byte_offset + 0] = color.red >> 8;
			buffer[byte_offset + 1] = color.green >> 8;
			buffer[byte_offset + 2] = color.blue >> 8;
		}
	}

	XDestroyImage(image);
	*size = buffer_size;
	return buffer;
}