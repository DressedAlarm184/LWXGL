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
	if (id == -1) {
		glXMakeContextCurrent(display, None, None, NULL);
	} else {
		auto opengl = (OpenGLElement*)elements[id]->elem;
		glXMakeContextCurrent(display, opengl->glx_pixmap, opengl->glx_pixmap, opengl->ctx);
	}
}

EXPORT unsigned int GLConvertTGA(const char* name) {
	auto it = allocated_TGAs.find(name);
	if (it == allocated_TGAs.end()) return 0;
	const auto& TGA = it->second;
	if (TGA.palette == NULL) return 0;

	GLuint texture_id;

	glGenTextures(1, &texture_id);

	GLint previous_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous_texture);
	glBindTexture(GL_TEXTURE_2D, texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	std::vector<unsigned char> pixels;
	pixels.reserve(TGA.width * TGA.height * 4);

	for (int y = 0; y < TGA.height; ++y) {
		int tga_y = (TGA.height - 1) - y;

		for (int x = 0; x < TGA.width; ++x) {
			int tga_index = (tga_y * TGA.width) + x;
			int po = TGA.pixels[tga_index] * 3;

			pixels.push_back(TGA.palette[po + 2]);
			pixels.push_back(TGA.palette[po + 1]);
			pixels.push_back(TGA.palette[po + 0]);
			pixels.push_back(TGA.transparent == (po / 3) ? 0 : 255);
		}
	}

	GLint previous_alignment;

	glGetIntegerv(GL_UNPACK_ALIGNMENT, &previous_alignment);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TGA.width, TGA.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
	glPixelStorei(GL_UNPACK_ALIGNMENT, previous_alignment);
	glBindTexture(GL_TEXTURE_2D, previous_texture);

	return texture_id;
}

EXPORT unsigned int GLObjectListify(const char* obj) {
	int read = 0, vertex_count = 0, face_count = 0;

	typedef struct {float x, y, z;} Vertex;
	typedef struct {int type, r, g, b, v0, v1, v2, v3;} Face;

	if (sscanf(obj, "COUNT %d%n", &vertex_count, &read) != 1) return 0; obj += read;
	Vertex* v = (Vertex*)alloca(sizeof(Vertex) * vertex_count);

	for (int i = 0; i < vertex_count; i++) {
		float x, y, z;
		if (sscanf(obj, "V %f %f %f%n", &x, &y, &z, &read) != 3) return 0; obj += read;
		v[i] = (Vertex){x, y, z};
	}

	if (sscanf(obj, "COUNT %d%n", &face_count, &read) != 1) return 0; obj += read;
	Face* faces = (Face*)alloca(sizeof(Face) * face_count);

	for (int i = 0; i < face_count; i++) {
		char type_str[8]; int type, r, g, b, v0, v1, v2, v3 = 0;
		if (sscanf(obj, "%7s %d %d %d%n", type_str, &r, &g, &b, &read) != 4) return 0; obj += read;
		if (strcmp(type_str, "QUAD") == 0) {
			type = 0;
			if (sscanf(obj, "%d %d %d %d%n", &v0, &v1, &v2, &v3, &read) != 4) return 0; obj += read;
		} else if (strcmp(type_str, "TRI") == 0) {
			type = 1;
			if (sscanf(obj, "%d %d %d%n", &v0, &v1, &v2, &read) != 3) return 0; obj += read;
		} else return 0;

		faces[i] = (Face){type, r, g, b, v0, v1, v2, v3};
	}

	GLuint list_id = glGenLists(1);

	glNewList(list_id, GL_COMPILE);
	glPushAttrib(GL_CURRENT_BIT);

	for (int i = 0; i < face_count; i++) {
		Face *f = &faces[i];
		glColor4ub(f->r, f->g, f->b, 255);

		glBegin(f->type == 0 ? GL_QUADS : GL_TRIANGLES);
			glVertex3f(v[f->v0].x, v[f->v0].y, v[f->v0].z);
			glVertex3f(v[f->v1].x, v[f->v1].y, v[f->v1].z);
			glVertex3f(v[f->v2].x, v[f->v2].y, v[f->v2].z);
			if (f->type == 0) glVertex3f(v[f->v3].x, v[f->v3].y, v[f->v3].z);
		glEnd();
	}

	glPopAttrib();
	glEndList();

	return list_id;
}
