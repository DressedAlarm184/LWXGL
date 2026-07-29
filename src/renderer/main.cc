EXPORT void _render_window(void (*on_every)(int, float), int tick, float dt) {
	XSetForeground(display, gc, colors[bgcol]);
	XFillRectangle(display, bb, gc, 0, bb.scroll, win_w, win_h);

	if (!bb.frame_cb_after_elem && on_every != NULL) on_every(tick, dt);

	for (Element* e : elements) {
		if (e == NULL) continue;

		if ((e->type == 0 || (e->y + e->h >= bb.scroll && e->y < bb.scroll + win_h)) && e->v)
			Renderers::Functions[e->type](e);
	}

	if (bb.frame_cb_after_elem && on_every != NULL) on_every(tick, dt);

	if (bb.scrollbar_color >= 0 && bb.h > win_h) {
		XSetForeground(display, gc, colors[L(bb.scrollbar_color)]);
		XFillRectangle(display, bb, gc, win_w - 9, bb.scroll, 9, win_h);
		XSetForeground(display, gc, colors[H(bb.scrollbar_color)]);
		int height = (win_h * ((float)win_h / (float)bb.h)) - 4;
		int y = bb.scroll + 2 + ((float)bb.scroll / (bb.h - win_h)) * (win_h - height - 4);
		XFillRectangle(display, bb, gc, win_w - 7, y, 5, height);
	}

	if (QueryModalOpen()) Renderers::DrawActiveModal();
	if (debug_metrics.enabled == 1) Renderers::DrawDebugOverlay();

	XCopyArea(display, bb, window, gc, 0, bb.scroll, win_w, win_h, 0, 0);
	XSync(display, False);
}
