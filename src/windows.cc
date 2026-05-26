int GCreateWindow(int w, int h, char* name, int bgcolor) {
	win_w = w, win_h = h;

	if (window != None) return 3;

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
		xcolor.red   = color_pallete[i].r * 257;
		xcolor.green = color_pallete[i].g * 257;
		xcolor.blue  = color_pallete[i].b * 257;
		xcolor.flags = DoRed | DoGreen | DoBlue;
		if (XAllocColor(display, colormap, &xcolor)) {
			colors[i] = xcolor.pixel;
		} else {
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
	
	XSelectInput(display, window, ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | KeyPressMask);
	
	XMapWindow(display, window);
	
	XSizeHints hints;
	hints.flags = PMinSize | PMaxSize;
	hints.min_width = w; hints.min_height = h;
	hints.max_width = w; hints.max_height = h;
	XSetWMNormalHints(display, window, &hints);
	
	XSetFont(display, gc, font->fid);
	bb = XCreatePixmap(display, window, w, h, DefaultDepth(display, screen));
	
	static unsigned char stipple_bits[] = {0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55};
	stipple = XCreateBitmapFromData(display, window, (char*)stipple_bits, 8, 8);
	XSetStipple(display, gc, stipple);

	bgcol = bgcolor;

	return 0;
}

void GTerminateWindow() {
	for (int i = 0; i < elements.size(); i++) {
		if (elements[i] != NULL) {
			GDeleteElement(i);
		}
	}
	XFreeFont(display, font);
	XFreeGC(display, gc);
	XFreePixmap(display, bb);
	XFreePixmap(display, stipple);
	XFreeColors(display, DefaultColormap(display, screen), colors, 16, 0);
	XDestroyWindow(display, window);
	XCloseDisplay(display);
}

int GWindowShouldClose() {
	return closing;
}

void GSimpleWindowLoop(void (*on_every)(int)) {
	using namespace std::chrono;
	
	debug_metrics.active = 1;
	const microseconds FRAME_TIME(16667);
	unsigned long long tick = 0;
	auto last_time = steady_clock::now();

	while (!GWindowShouldClose()) {
		auto now = steady_clock::now();
		auto elapsed = duration_cast<microseconds>(now - last_time);
		
		if (elapsed >= FRAME_TIME) {
			auto work_start = steady_clock::now();
			
			GHandleWindowEvents();
			GRenderWindow(); 

			if (on_every != NULL) on_every(tick);
			
			auto work_time = duration_cast<microseconds>(steady_clock::now() - work_start);
			float current_fps = 1000000.0 / elapsed.count(); int index = tick % 60;
			
			debug_metrics.avg_wt[index] = work_time.count();
			debug_metrics.fps = current_fps;
			
			last_time += FRAME_TIME,tick++;
			
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

void GDeleteWindow() {
	closing = 1;
}