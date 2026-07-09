EXPORT void GDrawIndexedTGA(int id, int x, int y, const char* name) {
	Element *e = elements[id];
	ImageElement *img = (ImageElement*)e->elem;

	const auto& TGA = allocated_TGAs[name];

	if (TGA.change_palette) {
		for (int i = 0; i < 16; i++) {
			unsigned char* c = TGA.palette + i * 3;
			GPaletteModify(i, *(c + 2), *(c + 1), *c, i == 15 ? 1 : 0);
		}
	}

	int width = TGA.width, height = TGA.height;

	for (int iy = 0; iy < height; iy++) {
		for (int ix = 0; ix < width; ix++) {
			int px = x + ix, py = y + iy;
			if (px >= e->w || px < 0 || py >= e->h || py < 0) continue;
			img->data[py * e->w + px] = TGA.pixels[iy * width + ix];
		}
	}
}

EXPORT int GAllocateTGA(const char* name, const char* path, int change_palette) {
	std::ifstream file(path, std::ios::binary);
	if (!file) return 1;

	unsigned char header[18] = {0};

	file.read((char*)header, 18);

	if (auto h = header; !(h[1] == 1 && (h[2] == 1 || h[2] == 9) && h[3] == 0 && h[4] == 0
		&& h[5] == 16 && h[6] == 0 && h[7] == 24 && h[16] == 8 && (h[17] == 32 || h[17] == 0))) return 2;

	GDeleteTGA(name);

	int width = header[12] | (header[13] << 8);
	int height = header[14] | (header[15] << 8);

	unsigned char* palette = (unsigned char*)calloc(48, 1);
	unsigned char* pixels = (unsigned char*)calloc(width * height, 1);

	allocated_TGAs[name] = {width, height, palette, pixels, change_palette};

	file.seekg(header[0], std::ios::cur);
	file.read((char*)palette, 48);

	int start_y = header[17] == 32 ? 0 : height - 1;
	int run_until = header[17] == 32 ? height : -1;
	int y_inc = header[17] == 32 ? 1 : -1;

	if (header[2] == 1) {
		std::vector<unsigned char> scanline(width);
		for (int iy = start_y; iy != run_until; iy += y_inc) {
			file.read((char*)scanline.data(), width);
			for (int ix = 0; ix < width; ix++) {
				pixels[iy * width + ix] = scanline[ix];
			}
		}
	} else for (int ix = 0, iy = start_y; iy != run_until;) {
		unsigned char packet, value, data[128];
		file.read((char*)&packet, 1);
		int count = (packet & 0x7F) + 1;
		if (packet & 0x80) {
			file.read((char*)&value, 1);
			while (count--) {
				pixels[iy * width + ix++] = value;
				if (ix == width) ix = 0, iy += y_inc;
			}
		} else {
			file.read((char*)data, count);
			for (int i = 0; i < count; ++i) {
				pixels[iy * width + ix++] = data[i];
				if (ix == width) ix = 0, iy += y_inc;
			}
		}
	}

	return 0;
}

EXPORT void GDeleteTGA(const char* name) {
	if (allocated_TGAs.find(name) != allocated_TGAs.end()) {
		free(allocated_TGAs[name].pixels);
		free(allocated_TGAs[name].palette);
		allocated_TGAs.erase(name);
	}
}

EXPORT int GCreateTGAImage(int id, int x, int y, const char* path, int change_palette) {
	using namespace std::string_literals;

	auto name = "TGAImage_"s + path;

	if (allocated_TGAs.find(name) == allocated_TGAs.end()) {
		int retval = GAllocateTGA(name.c_str(), path, change_palette);
		if (retval != 0) return retval;
	}

	auto image = allocated_TGAs[name];

	GCreateImage(id, x, y, image.width, image.height);
	GDrawIndexedTGA(id, 0, 0, name.c_str());
	GUpdateImage(id);

	return 0;
}

EXPORT unsigned char* GCaptureRegion(int x, int y, unsigned short w, unsigned short h) {
	XImage *image = XGetImage(display, bb, x, y, w, h, AllPlanes, ZPixmap);

	unsigned char header[18] = {0, 1, 1, 0, 0, 16, 0, 24, 0, 0, 0, 0,
								(unsigned char)(w & 0xFF), (unsigned char)((w >> 8) & 0xFF),
								(unsigned char)(h & 0xFF), (unsigned char)((h >> 8) & 0xFF),
								8, 32};

	unsigned char* buffer = (unsigned char*)calloc(1, 66 + w * h);
	memcpy(buffer, header, 18);

	unsigned char red, green, blue;

	for (int i = 0; i < 16; i++) {
		GPaletteQuery(i, &red, &green, &blue);
		int o = 18 + i * 3;
		buffer[o] = blue, buffer[o + 1] = green, buffer[o + 2] = red;
	}

	for (int py = 0; py < h; py++) {
		for (int px = 0; px < w; px++) {
			unsigned long pixel = XGetPixel(image, px, py);

			for (int i = 0; i < 16; i++) {
				if (pixel == colors[i]) {
					buffer[66 + py * w + px] = i;
					break;
				}
			}
		}
	}

	XDestroyImage(image);
	return buffer;
}
