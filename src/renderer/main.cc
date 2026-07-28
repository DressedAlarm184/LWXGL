EXPORT void _render_window(void (*on_every)(int, float), int tick, float dt) {
	XSetForeground(display, gc, colors[bgcol]);
	XFillRectangle(display, bb, gc, 0, bb.scroll, win_w, win_h);

	bb.glx.rendered = false;

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

EXPORT int InitializeOpenGL(int w, int h) {
	if (bb.glx.enabled) return 0;
	if (depth != 24 && depth != 32) return 0;

	int fb_attribs[] = {
		GLX_X_RENDERABLE,  True,
		GLX_DRAWABLE_TYPE, GLX_PIXMAP_BIT,
		GLX_RENDER_TYPE,   GLX_RGBA_BIT,
		GLX_DOUBLEBUFFER,  False,
		GLX_RED_SIZE,      8,
		GLX_GREEN_SIZE,    8,
		GLX_BLUE_SIZE,     8,
		GLX_ALPHA_SIZE,    8,
		GLX_DEPTH_SIZE,    24,
		GLX_STENCIL_SIZE,  8,
		None
	};

	int num_fbconfigs;
	GLXFBConfig *fbconfigs = glXChooseFBConfig(display, screen, fb_attribs, &num_fbconfigs);
	if (!fbconfigs || num_fbconfigs == 0) return 0;

	GLXFBConfig fbconfig = NULL;
	XVisualInfo *vi = NULL;

	for (int i = 0; i < num_fbconfigs; i++) {
		vi = glXGetVisualFromFBConfig(display, fbconfigs[i]);
		if (vi) {
			if (vi->depth == depth) {
				fbconfig = fbconfigs[i];
				break;
			}
			XFree(vi);
			vi = NULL;
		}
	}

	if (!fbconfig || !vi) {
		XFree(fbconfigs);
		return 0;
	}

	bb.glx.gl_pixmap = XCreatePixmap(display, RootWindow(display, screen), w, h, depth);
	bb.glx.glx_pixmap = glXCreatePixmap(display, fbconfig, bb.glx.gl_pixmap, NULL);

	bb.glx.ctx = glXCreateNewContext(display, fbconfig, GLX_RGBA_TYPE, NULL, True);

	XFree(vi);

	if (!glXMakeContextCurrent(display, bb.glx.glx_pixmap, bb.glx.glx_pixmap, bb.glx.ctx)) {
		XFree(fbconfigs);
		return 0;
	}

	glViewport(0, 0, w, h);
	XFree(fbconfigs);

	bb.glx.enabled = true;
	bb.glx.pm_w = w, bb.glx.pm_h = h;

	return 1;
}

EXPORT void GLXPushPixmap(int x, int y) {
	if (bb.glx.rendered || !bb.glx.enabled) return;
	glFlush();
	glXWaitGL();
	XCopyArea(display, bb.glx.gl_pixmap, bb, gc, 0, 0, bb.glx.pm_w, bb.glx.pm_h, x, y);
	bb.glx.rendered = true;
}
