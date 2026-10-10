void _allocate_element(int id, int type, void *data, int x, int y, int w, int h) {
	if (id >= elements.size()) elements.resize(id + 1, NULL);
	if (elements[id] != NULL) DeleteElement(id);
	elements[id] = new Element{x, y, w, h, 1, type, data, NULL, id};
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
	if (QueryModalOpen()) return 0;

	int scroll_y = mouse_y + bb.scroll;

	auto coords_inside = [scroll_y](Element* el) -> bool {
		if (el == nullptr || !el->v) return false;

		int right_extent = el->x + el->w;
		if (el->type == 5) {
			auto checkbox = (CheckboxElement*)el->elem;
			if (checkbox->label != NULL) right_extent += 10 + strlen(checkbox->label) * 9;
		}

		return mouse_x >= el->x && mouse_x < right_extent &&
		       scroll_y >= el->y && scroll_y < el->y + el->h;
	};

	if (!coords_inside(e)) return 0;

	bool found_self = false;
	for (Element* other : elements) {
		if (!found_self) {
			if (other == e) found_self = true;
			continue;
		}
		if (coords_inside(other)) {
			return 0;
		}
	}

	return 1;
}
