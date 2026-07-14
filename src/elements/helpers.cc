void _allocate_element(int id, int type, void *data, int x, int y, int w, int h) {
	if (id >= elements.size()) elements.resize(id + 1, NULL);
	if (elements[id] != NULL) GDeleteElement(id);
	elements[id] = new Element{x, y, w, h, 1, INT_MIN, type, data};
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
		auto checkbox = (CheckboxElement*)e->elem;
		if (checkbox->label != NULL) right_extent += 10 + strlen(checkbox->label) * 9;
	}

	int x_inside = mouse_x >= e->x && mouse_x < right_extent;
	int y_inside = mouse_y + bb.scroll >= e->y && mouse_y + bb.scroll < e->y + e->h;

	return x_inside && y_inside && e->v && !GQueryModalOpen();
}
