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
