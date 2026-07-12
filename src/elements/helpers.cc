Element *_allocate_element(int id, int type, void *data, int x, int y, int w, int h) {
	if (id >= elements.size()) elements.resize(id + 1, NULL);
	if (elements[id] != NULL) GDeleteElement(id);
	Element *e = (Element*)malloc(sizeof(Element));
	e->type = type, e->elem = data;
	e->w = w, e->h = h, e->x = x, e->y = y; e->v = 1; e->screen = 0; e->anchor = INT_MIN;
	elements[id] = e;
	return e;
}

void _console_calc_total_lines(ConsoleElement* console) {
	int total_lines = 1, current_len = 0;

	for (char c : console->data) {
		if (c == '\n') {
			total_lines++;
			current_len = 0;
		} else {
			current_len++;
			if (current_len == console->cols) {
				total_lines++;
				current_len = 0;
			}
		}
	}

	console->total_lines = total_lines;
}

int _inside_elem(Element* e) {
	int right_extent = e->x + e->w;
	if (e->type == 5) {
		CheckboxElement* checkbox = (CheckboxElement*)e->elem;
		if (checkbox->label != NULL) right_extent += 10 + (int)strlen(checkbox->label) * 9;
	}
	int mouse_inside = mouse_x >= e->x && mouse_x < right_extent && mouse_y >= e->y && mouse_y < e->y + e->h;
	int is_on_screen = e->screen == active_screen || e->screen == -1;
	return mouse_inside && is_on_screen && e->v;
}
