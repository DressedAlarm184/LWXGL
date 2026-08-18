EXPORT int _window_should_close() {
	return closing;
}

EXPORT void DeleteWindow() {
	if (Events::UserProvided::Delete != NULL) {
		closing = Events::UserProvided::Delete();
	} else closing = 1;
}

EXPORT void SpawnModal(int type, const char* msg, void (*on_confirm)(const char* input)) {
	if (active_modal_state.msg != NULL) {
		free(active_modal_state.msg);
	}

	active_modal_state.active = 1;
	active_modal_state.msg = strdup(msg);
	active_modal_state.on_confirm = on_confirm;
	active_modal_state.type = type;

	memset(active_modal_state.input, 0, 151);
}

EXPORT int QueryModalOpen() {
	return active_modal_state.active;
}

EXPORT int QueryScroll() {
	return bb.scroll;
}

EXPORT void GetXConnection(XConnectionData* data) {
	data->win = window;
	data->dpy = display;
	data->bb = bb;
	data->font = font;
	data->gc = gc;
	data->scrn = screen;
	data->vis = visual;
	data->depth = depth;
	data->cmap = colormap;

	for (int i = 0; i < 16; ++i) {
		data->clrs[i] = colors[i];
	}
}

EXPORT double GetElapsedTime() {
	return elapsed_time;
}

EXPORT void NewQueuedTask(int type, double run_after, void (*task)()) {
	if (type == TASK_RUN_AFTER) {
		task_queue.push_back({elapsed_time + run_after, task, -1});
	} else if (type == TASK_RUN_EVERY) {
		double start_target = std::ceil(elapsed_time / run_after) * run_after;
		task_queue.push_back({start_target, task, run_after});
	}
}

EXPORT void ManagedConsoleWindow(const char* name, int cols, int rows, void (*state)(CONSOLE_STATE_ARGS)) {
	if (window != None) {
		printf("LWXGL Error: A window already exists!\n");
		exit(3);
	}

	static void (*callback)(CONSOLE_STATE_ARGS) = state;

	static void (*next)(const char*) = [](const char* input) {
		static unsigned long long i = 0;
		i++;
		callback(input, i - 1, next);
	};

	CreateConsole(0, 10, 10, cols, rows, 0xF0, 0xAE);

	if (int err = CreateWindow(elements[0]->w + 20, elements[0]->h + 20, name, CLR_BLUE); err != 0) {
		DeleteElement(0);
		const char* error = "An unknown error has occurred!";
		if (err == 3) error = "A window already exists!";
		else if (err == 1) error = "Could not open display! Is $DISPLAY set?";
		else if (err == 2) error = "Failed to load font! Ensure the 9x15 X11 font is available.";
		else if (err >= 127) error = "Failed to allocate one or more colors!";
		printf("LWXGL Error: %s\n", error);
		exit(err);
	}

	next(NULL);

	MainWindowLoop(60, NULL);
	TerminateWindow();
}
