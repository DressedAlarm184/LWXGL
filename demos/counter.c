#include "../lwxgl/lwxgl.h"
#include <stdio.h>

char buffer[32];
int count = 0;

void onclick_Inc() {
	count++;
	sprintf(buffer, "Count: %d", count);
}

void onclick_Dec() {
	count--;
	sprintf(buffer, "Count: %d", count);
}

int main() {
	sprintf(buffer, "Count: %d", count);
	GCreateWindow(300, 200, "Counter", 2);
	GCreateText(0, 10, 20, 15, buffer);
	GCreateButton(3, 160, 160, 60, 30, 15, 3, 15, 1, 15, 8, "Inc", onclick_Inc);
	GCreateButton(4, 230, 160, 60, 30, 15, 3, 15, 1, 15, 8, "Dec", onclick_Dec);
	GSimpleWindowLoop();
	GTerminateWindow();
}