EXPORT void GCreateImage(int id, int x, int y, int w, int h) {
	if (w <= 0) w = bb.w - x + w;
	if (h <= 0) h = bb.h - y + h;

	auto img = new ImageElement;
	img->ximage = XCreateImage(display, DefaultVisual(display, screen), DefaultDepth(display, screen), ZPixmap, 0, NULL, w, h, 32, 0);
	img->data = (unsigned char *)calloc(w * h, 1);
	img->prev = (unsigned char *)calloc(w * h, 1);
	img->imgdata = (char *)calloc(h * img->ximage->bytes_per_line, 1);
	img->ximage->data = img->imgdata;
	img->fontdata.buffer = NULL, img->fontdata.height = 0;
	img->pixmap = XCreatePixmap(display, window, w, h, DefaultDepth(display, screen));

	_allocate_element(id, 4, img, x, y, w, h);
}

EXPORT unsigned char* GGetImageData(int id) {
	auto img = (ImageElement*)elements[id]->elem;
	return img->data;
}

EXPORT void GUpdateImage(int id) {
	auto img = (ImageElement *)elements[id]->elem;
	int w = elements[id]->w, h = elements[id]->h;

	unsigned char *src = (unsigned char*)img->data;
	unsigned char *prev = (unsigned char*)img->prev; 
	int (*put_pixel)(XImage*, int, int, unsigned long) = img->ximage->f.put_pixel;
	bool has_changed = false;

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			if (*src != *prev) {
				put_pixel(img->ximage, x, y, colors[*src]);
				*prev = *src, has_changed = true;
			}
			src++, prev++;
		}
	}

	if (has_changed)
		XPutImage(display, img->pixmap, gc, img->ximage, 0, 0, 0, 0, w, h);
}

EXPORT void GPrimitiveRect(int id, int x, int y, int w, int h, int fg, int bg) {
	if (fg == -1) fg = bg;

	auto img = (ImageElement *)elements[id]->elem;
	int img_w = elements[id]->w, img_h = elements[id]->h;

	for (int cy = y; cy < y + h; cy++) {
		for (int cx = x; cx < x + w; cx++) {
			if (cx < 0 || cx >= img_w || cy < 0 || cy >= img_h) continue;
			int color = (cx == x || cx == w + x - 1) ? fg : (cy == y || cy == y + h - 1) ? fg : bg;
			if (color != -1) img->data[cx + cy * img_w] = color;
		}
	}
}

EXPORT void GPrimitiveCircle(int id, int cx, int cy, int r, int fg, int bg) {
	auto img = (ImageElement *)elements[id]->elem;
	int w = elements[id]->w, h = elements[id]->h;

	for (int y = cy - r; y <= cy + r; y++) {
		for (int x = cx - r; x <= cx + r; x++) {
			if (x < 0 || x >= w || y < 0 || y >= h) continue;
			int dx = x - cx, dy = y - cy, d2 = dx * dx + dy * dy;
			int on_border = std::abs(d2 - r * r) < r;
			if (fg != -1 && on_border) {
				img->data[x + y * w] = (uint32_t)fg;
			} else if (bg != -1 && d2 <= r * r) {
				img->data[x + y * w] = (uint32_t)bg;
			}
		}
	}
}

EXPORT void GPrimitiveLine(int id, int x1, int y1, int x2, int y2, int color) {
	auto img = (ImageElement *)elements[id]->elem;
	int w = elements[id]->w, h = elements[id]->h;

	int dx = x2 - x1, dy = y2 - y1;
	int steps = std::max(std::abs(dx), std::abs(dy));
	float x_inc = dx / (float)steps, y_inc = dy / (float)steps;
	float x = x1, y = y1;

	for (int i = 0; i <= steps; i++) {
		int pixel_x = (int)std::round(x), pixel_y = (int)std::round(y);
		if (pixel_x < 0 || pixel_x >= w || pixel_y < 0 || pixel_y >= h) continue;
		img->data[pixel_x + pixel_y * w] = color;
		x += x_inc, y += y_inc;
	}
}

EXPORT void GPrimitiveSprite(int id, int sx, int sy, int color, const char* sprite, int scale) {
	auto img = (ImageElement *)elements[id]->elem;
	int img_w = elements[id]->w, img_h = elements[id]->h;
	int x = sx, y = sy;

	auto set_pixel = [&](int color) {
		for (int yoff = 0; yoff < scale; yoff++) {
			for (int xoff = 0; xoff < scale; xoff++) {
				int px = x + xoff, py = y + yoff;
				if (px >= 0 && px < img_w && py >= 0 && py < img_h) {
					img->data[py * img_w + px] = color;
				}
			}
		}
	};

	std::function<void(const char*, int)> draw = [&](const char* rle, int len) {
		int count = 0;
		for (int i = 0; (*rle != 0 && *rle != '!' && i < len); i++) {
			if (*rle >= '0' && *rle <= '9') {
				count = count * 10 + (*rle - '0');
			} else if (*rle == '[') {
				rle++;
				const char *start = rle, *end = rle;
				for (int depth = 1; *end && depth > 0;) {
					if (*end == '[') depth++;
					else if (*end == ']') depth--;
					end++;
				}
				int len = (int)((end - 1) - start);
				int runs = (count > 0) ? count : 1;
				while (runs--) draw(rle, len);
				rle = end - 1, count = 0, i += len + 1;
			} else {
				int runs = (count == 0) ? 1 : count;
				if (*rle == '#') {
				while (runs-- > 0) {
					set_pixel(color);
					x += scale;
				}
			} else if (*rle == '.') {
				while (runs-- > 0) {
					set_pixel(0);
					x += scale;
				}
			} else if (*rle == '$') {
					x = sx, y += runs * scale;
				} else if (*rle == '>') {
					x += runs * scale;
				}
				count = 0;
			}
			rle++;
		}
	};

	draw(sprite, strlen(sprite));
}

EXPORT void GClearImage(int id, int c) {
	auto img = (ImageElement *)elements[id]->elem;
	memset(img->data, c, elements[id]->w * elements[id]->h);
}

EXPORT void GRedrawAllImages() {
	for (int i = 0; i < elements.size(); i++) {
		Element *e = elements[i];
		if (e == NULL) continue;
		if (e->type != 4) continue;
		ImageElement* img = (ImageElement *)e->elem;
		memset(img->prev, 255, e->w * e->h);
		GUpdateImage(i);
	}
}

EXPORT void GSetImageFont(int id, unsigned char* font, int h) {
	auto img = (ImageElement*)elements[id]->elem;
	img->fontdata.height = h;
	if (img->fontdata.buffer != NULL) free(img->fontdata.buffer);
	img->fontdata.buffer = (unsigned char*)malloc(256 * h);
	memcpy(img->fontdata.buffer, font, 256 * h);
}

EXPORT void GDrawString(int id, int x, int y, const char* txt, int color) {
	Element *e = elements[id];
	auto img = (ImageElement*)e->elem;
	int height = img->fontdata.height;
	unsigned char* font = img->fontdata.buffer;

	for (int cx = 0; *txt; cx++) {
		if (*txt == 10) {
			y += height, cx = -1;
		} else for (int i = 0; i < height; i++) {
			unsigned char row_bitmap = font[(unsigned int)*txt * height + i];
			for (int rx = 0; rx < 8; rx++) {
				if (!(row_bitmap & (0x80 >> rx))) continue;
				int px = x + rx + cx * 9, py = y + i;
				if (px >= e->w || px < 0 || py >= e->h || py < 0) continue;
				img->data[py * e->w + px] = color;
			}
		}

		txt++;
	}
}

EXPORT void GApplyPixelFunc(int id, int (*f)(int, int, int)) {
	Element *e = elements[id];
	auto img = (ImageElement*)e->elem;

	for (int py = 0; py < e->h; py++) {
		for (int px = 0; px < e->w; px++) {
			unsigned char* pixel = &img->data[py * e->w + px];
			*pixel = f(px, py, *pixel) % 16;
		}
	}
}

EXPORT void GPrimitiveTriangle(int id, int x1, int y1, int x2, int y2, int x3, int y3, int fg, int bg) {
	Element *e = elements[id];
	auto img = (ImageElement*)e->elem;

	auto edge_function = [](int ax, int ay, int bx, int by, int px, int py) {
		return (bx - ax) * (py - ay) - (by - ay) * (px - ax);
	};

	if (bg != -1) {
		int min_x = std::max(0, std::min({x1, x2, x3}));
		int min_y = std::max(0, std::min({y1, y2, y3}));
		int max_x = std::min(e->w - 1, std::max({x1, x2, x3}));
		int max_y = std::min(e->h - 1, std::max({y1, y2, y3}));

		for (int py = min_y; py <= max_y; py++) {
			for (int px = min_x; px <= max_x; px++) {
				int w0 = edge_function(x2, y2, x3, y3, px, py);
				int w1 = edge_function(x3, y3, x1, y1, px, py);
				int w2 = edge_function(x1, y1, x2, y2, px, py);

				if ((w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0)) {
					img->data[py * e->w + px] = bg;
				}
			}
		}
	}

	if (fg != -1) {
		GPrimitiveLine(id, x1, y1, x2, y2, fg);
		GPrimitiveLine(id, x2, y2, x3, y3, fg);
		GPrimitiveLine(id, x3, y3, x1, y1, fg);
	}
}
