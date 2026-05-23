#include "../lwxgl/lwxgl.h"
#include "unistd.h"
#include "stdio.h"

int main() {
	int tick = 0;
	if (GCreateWindow(400, 200, "Test Window", 1) != 0) {
		fprintf(stderr, "Cannot spawn window!\n");
		return 1;
	}
	GCreateText(0, 10, 20, 15, "Hello, World!");
	GCreateButton(1, 330, 160, 60, 30, 0, 15, 15, 0, 14, 0, "Test");
	while (!GWindowShouldClose()) {
		if (tick % 16 == 0) GRenderWindow();
		GHandleWindowEvents();
		usleep(1000);
		tick++;
	}
	GTerminateWindow();
	return 0;
}