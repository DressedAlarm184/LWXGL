#include "../lwxgl/lwxgl.h"
#include "unistd.h"
#include "stdio.h"

void onclick_Quit() {
	GDeleteWindow();
}

void onclick_Test() {
	printf("Something happened.\n");
}

int main() {
	if (GCreateWindow(400, 200, "Test Window", 1) != 0) {
		fprintf(stderr, "Cannot spawn window!\n");
		return 1;
	}
	GCreateText(0, 10, 20, 15, "Hello, World!");
	GCreateText(1, 10, 35, 15, "Demo program to show capabilities.");
	GCreateButton(3, 260, 160, 60, 30, 0, 15, 15, 0, 14, 0, "Quit", onclick_Quit);
	GCreateButton(4, 330, 160, 60, 30, 0, 15, 15, 0, 14, 0, "Test", onclick_Test);
	GSimpleWindowLoop();
	printf("Exiting...\n");
	GTerminateWindow();
	return 0;
}