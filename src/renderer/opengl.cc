EXPORT int CreateOpenGL(int id, int x, int y, int w, int h, int border) {
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

	XFree(vi);
	XFree(fbconfigs);

	Pixmap x_pixmap = XCreatePixmap(display, RootWindow(display, screen), w, h, depth);
	GLXPixmap glx_pixmap = glXCreatePixmap(display, fbconfig, x_pixmap, NULL);
	GLXContext ctx = glXCreateNewContext(display, fbconfig, GLX_RGBA_TYPE, NULL, True);

	if (border >= 0) {
		w += 2, h += 2;
	}

	_allocate_element(id, 8, new OpenGLElement{
		glx_pixmap, x_pixmap, ctx, border
	}, x, y, w, h);

	return 1;
}

EXPORT void SynchronizeOpenGL() {
	glFlush();
	glXWaitGL();
}

EXPORT void ChangeGLXContext(int id) {
	auto opengl = (OpenGLElement*)elements[id]->elem;
	glXMakeContextCurrent(display, opengl->glx_pixmap, opengl->glx_pixmap, opengl->ctx);
}
