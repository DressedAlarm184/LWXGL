namespace Renderers {
	void DrawDebugOverlay() {
		int wt = 0; for (int i = 0; i < 60; i++) wt += debug_metrics.avg_wt[i]; wt /= 60;
		XSetForeground(display, gc, colors[0]);
		XFillRectangle(display, bb, gc, 5, bb.scroll + 5, 140, 40);
		XSetForeground(display, gc, colors[15]);
		XDrawRectangle(display, bb, gc, 7, bb.scroll + 7, 135, 35);
		char wt_buffer[32] = {0}, fps_buffer[32] = {0};
		int wt_len = sprintf(wt_buffer, "FT: %d (us)", wt);
		int fps_len = sprintf(fps_buffer, "FPS: %.1f", debug_metrics.fps);
		XDrawString(display, bb, gc, 11, bb.scroll + 23, wt_buffer, wt_len);
		XDrawString(display, bb, gc, 11, bb.scroll + 37, fps_buffer, fps_len);
	}

	void DrawActiveModal() {
		int max_chars = (std::clamp((int)(win_w / 1.5f), 300, 550) - 9) / 9;
		int width = (max_chars * 9) + 9;
		int r_edge = win_w / 2 + width / 2, l_edge = win_w / 2 - width / 2;

		auto draw_wrapped_str = [&](const char* str, int max_chars, int y, int offset) {
			int newlines = 0;

			while (*str != '\0') {
				int len = 0;
				while (str[len] != '\0' && str[len] != '\n' && len < max_chars) len++;
				XDrawString(display, bb, gc, (win_w / 2 - (width - 10) / 2) + (9*offset), y, str, len);
				str += len, y += 15, newlines++;
				if (*str == '\n') str++;
			}

			return newlines;
		};

		XSetForeground(display, gc, colors[0]);
		XFillRectangle(display, bb, gc, win_w / 2 - (width + 4) / 2, bb.scroll + 47, width + 5, 156);
		XSetForeground(display, gc, colors[15]);
		XDrawRectangle(display, bb, gc, l_edge, bb.scroll + 49, width, 151);

		int y = bb.scroll + 68; const char* str = active_modal_state.msg;
		int lines = draw_wrapped_str(str, max_chars, y, 0);

		if (active_modal_state.type == MODAL_INPUT) {
			char input[152] = {0};
			int input_len = strlen(active_modal_state.input);
			memcpy(input, active_modal_state.input, 150);
			input[input_len] = '_';

			char chars[32];
			int chars_len = snprintf(chars, sizeof chars, "Limit: %d / 150", input_len);
			XDrawString(display, bb, gc, l_edge + 5, bb.scroll + 193, chars, chars_len);

			XSetForeground(display, gc, colors[CLR_LCYAN]);
			draw_wrapped_str(input, max_chars - 2, y + lines * 15 + 7, 1);
		}

		XSetForeground(display, gc, colors[10]);
		XDrawString(display, bb, gc, r_edge - 25, bb.scroll + 193, "OK", 2);
		if (active_modal_state.type != MODAL_ALERT) {
			XSetForeground(display, gc, colors[12]);
			XDrawString(display, bb, gc, r_edge - 95, bb.scroll + 193, "Cancel", 6);
		}

		active_modal_state.right_edge_x = r_edge;
	}
}
