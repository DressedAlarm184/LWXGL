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
	colormap = DefaultColormap(display, screen);
	depth = DefaultDepth(display, screen);
	visual = DefaultVisual(display, screen);

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
	bb.new_bb(w, bb.scroll_enabled ? bb.h : h);

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
	XFreeColors(display, colormap, colors, 16, 0);
	XDestroyWindow(display, window);
	XCloseDisplay(display);
}

EXPORT void MainWindowLoop(int target_fps, void (*on_every)(int, float)) {
	using namespace std::chrono;
	
	const microseconds FRAME_TIME(1000000 / target_fps);
	unsigned long long tick = 0;
	auto last_time = steady_clock::now();
	std::vector<QueuedTask> active_tasks;

	while (!_window_should_close()) {
		auto now = steady_clock::now();
		auto elapsed = duration_cast<microseconds>(now - last_time);
		
		if (elapsed >= FRAME_TIME) {
			double delta_time_d = elapsed.count() / 1000000.0;
			elapsed_time += delta_time_d;
			float delta_time = static_cast<float>(delta_time_d);

			auto work_start = steady_clock::now();
			
			_handle_window_events();
			_render_window(on_every, tick, delta_time);
			
			std::swap(task_queue, active_tasks);

			for (auto& task_item : active_tasks) {
				if (task_item.target_time <= elapsed_time) {
					task_item.task();
					if (task_item.repeat_every >= 0) {
						task_item.target_time += task_item.repeat_every;
						task_queue.push_back(std::move(task_item));
					}
				} else {
					task_queue.push_back(std::move(task_item));
				}
			}

			active_tasks.clear();

			auto work_time = duration_cast<microseconds>(steady_clock::now() - work_start);
			float current_fps = 1000000.0 / elapsed.count();
			
			for (int i = 0; i < 59; i++) debug_metrics.avg_wt[i] = debug_metrics.avg_wt[i + 1];
			debug_metrics.avg_wt[59] = work_time.count(), debug_metrics.fps = current_fps;

			last_time = now, tick++;
		} else {
			auto time_to_sleep = FRAME_TIME - elapsed;
			if (time_to_sleep > milliseconds(2)) {
				std::this_thread::sleep_for(time_to_sleep - milliseconds(1));
			} else std::this_thread::yield();
		}
	}
}
