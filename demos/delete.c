#include "../lwxgl/lwxgl.h"

void onclick_Delete() {
	GDeleteElement(0);
}

int main() {
	GCreateWindow(400, 300, "Delete Example", 6);
	GCreateText(0, 10, 50, 15, "Press the button to delete this text!");
	GCreateButton(1, 10, 250, 80, 30, 15, 3, 15, 1, 15, 8, "Delete", onclick_Delete);
	GSimpleWindowLoop();
	GTerminateWindow();
}