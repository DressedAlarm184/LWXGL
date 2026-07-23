typedef struct {
	int width, height;
	unsigned char* palette;
	unsigned char* pixels;
	int transparent;
	int change_palette;
} AllocatedTGA;

std::unordered_map<std::string, AllocatedTGA> allocated_TGAs;

EXPORT void GDrawIndexedTGA(int id, int x, int y, const char* name) {
	Element *e = elements[id];
	ImageElement *img = (ImageElement*)e->elem;

	auto it = allocated_TGAs.find(name);
	if (it == allocated_TGAs.end()) return;
	const auto& TGA = it->second;

	if (TGA.change_palette) {
		for (int i = 0; i < 16; i++) {
			unsigned char* c = TGA.palette + i * 3;
			GPaletteModify(i, *(c + 2), *(c + 1), *c, i == 15 ? 1 : 0);
		}
	}

	int width = TGA.width, height = TGA.height, t = TGA.transparent;

	for (int iy = 0; iy < height; iy++) {
		for (int ix = 0; ix < width; ix++) {
			int px = x + ix, py = y + iy;
			int pixel = TGA.pixels[iy * width + ix];
			if (pixel == t) continue;
			if (px >= e->w || px < 0 || py >= e->h || py < 0) continue;
			img->data[py * e->w + px] = pixel;
		}
	}
}

EXPORT int GAllocateTGA(const char* name, const char* path, int change_palette, int transparent) {
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

	allocated_TGAs[name] = {width, height, palette, pixels, transparent, change_palette};

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
		int retval = GAllocateTGA(name.c_str(), path, change_palette, -1);
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

EXPORT int GAllocateMemoryTGA(const char* name, const char* buffer, int size, int change_palette, int transparent) {
	int fd = memfd_create("memory_tga", 0);
	write(fd, buffer, size);
	auto path = "/proc/self/fd/" + std::to_string(fd);
	int retval = GAllocateTGA(name, path.c_str(), transparent, change_palette);
	close(fd);
	return retval;
}

EXPORT int GAllocateXBM(const char* name, const char* path, int colors, int transparent) {
	unsigned char bit1 = H(colors), bit0 = L(colors), *raw_data = NULL;
	int t_value = transparent == 1 ? bit1 : transparent == 0 ? bit0 : -1, hot_x, hot_y;

	unsigned int width, height;

	if (int status = XReadBitmapFileData(path, &width, &height, &raw_data, &hot_x, &hot_y);
		status != BitmapSuccess) return 0;

	auto pixels = (unsigned char*)calloc(width * height, 1);
	int bytes_per_row = (width + 7) / 8;

	for (unsigned int y = 0; y < height; y++) {
		for (unsigned int x = 0; x < width; x++) {
			int byte_index = (y * bytes_per_row) + (x / 8);
			int pixel = (raw_data[byte_index] >> x % 8) & 1;
			pixels[y * width + x] = pixel ? bit1 : bit0;
		}
	}

	GDeleteTGA(name);
	allocated_TGAs[name] = {(int)width, (int)height, NULL, pixels, t_value, 0};

	XFree(raw_data);
	return 1;
}

EXPORT int GCreateXBMImage(int id, int x, int y, const char* path, int colors) {
	using namespace std::string_literals;

	auto name = "XBMImage_"s + path;

	if (allocated_TGAs.find(name) == allocated_TGAs.end()) {
		int retval = GAllocateXBM(name.c_str(), path, colors, -1);
		if (retval != 1) return retval;
	}

	auto image = allocated_TGAs[name];

	GCreateImage(id, x, y, image.width, image.height);
	GDrawIndexedTGA(id, 0, 0, name.c_str());
	GUpdateImage(id);

	return 1;
}
