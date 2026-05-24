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

void onclick_Set() {
	int value = 0, sign = 1;
	char* input = GGetInput(5);
	if (*input == '-') {
		sign = -1, input++;
	} 
	while (*input != 0) {
		value = value * 10 + (*input++ - '0');
	}
	count = value * sign;
	sprintf(buffer, "Count: %d", count);
}

int main() {
	sprintf(buffer, "Count: %d", count);
	GCreateWindow(300, 200, "Counter", 2);
	GCreateText(0, 10, 20, 15, buffer);
	GCreateButton(3, 160, 166, 60, 24, 15, 3, 15, 1, 15, 8, "Inc", onclick_Inc);
	GCreateButton(4, 230, 166, 60, 24, 15, 3, 15, 1, 15, 8, "Dec", onclick_Dec);
	GCreateInput(5, 10, 137, 100, 24, 15, 4, 15, 8, 9);
	GCreateButton(6, 10, 166, 60, 24, 15, 3, 15, 1, 15, 8, "Set", onclick_Set);
	GSimpleWindowLoop();
	GTerminateWindow();
}