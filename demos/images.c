#include "../lwxgl/lwxgl.h"
#include "time.h"
#include "stdlib.h"

void onclick_Randomize() {
	char* data = GGetImageData(3);
	for (int y = 0; y < 200; y++) {
		for (int x = 0; x < 200; x++) {
			data[y * 200 + x] = rand() % 16;
		}
	}
	GUpdateImage(3);
}

int main() {
	srand(time(NULL));
	GCreateWindow(500, 300, "Rectangles & Images", 15);
	GCreateRect(0, 10, 10, 200, 30, 0, 6);
	GCreateRect(1, 10, 45, 200, 30, -1, 3);
	GCreateRect(2, 10, 80, 200, 30, 4, -1);
	GCreateImage(3, 250, 10, 200, 200);
	GCreateButton(4, 250, 215, 200, 24, 0, 7, 0, 10, 0, 13, "Randomize", onclick_Randomize);
	GSimpleWindowLoop();
	GTerminateWindow();
}