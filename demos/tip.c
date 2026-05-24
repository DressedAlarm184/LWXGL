#include "../lwxgl/lwxgl.h"
#include <stdlib.h>
#include <stdio.h>

char buffer[128];

void onclick_Calculate() {
	double amount = atof(GGetInput(5)), tip = atof(GGetInput(6)), people = atof(GGetInput(7));
	double tip_amount = (tip * amount / 100), total = amount + tip_amount;
	sprintf(buffer, "Tips - $%.2f\nBill - $%.2f\nEach - $%.2f", tip_amount, total, total / people);
}

int main() {
	sprintf(buffer, "(Awaiting...)");
	GCreateWindow(200, 250, "Tip Calculator", 4);
	GCreateText(0, 35, 20, 15, "Tip Calculator");
	GCreateText(1, 5, 60, 15, "Amount:");
	GCreateText(2, 5, 90, 15, "Tip (%):");
	GCreateText(4, 5, 120, 15, "People:");
	GCreateInput(5, 80, 45, 110, 20, 15, 0, 15, 8, 10);
	GCreateInput(6, 80, 75, 110, 20, 15, 0, 15, 8, 10);
	GCreateInput(7, 80, 105, 110, 20, 15, 0, 15, 8, 10);
	GCreateButton(8, 80, 135, 110, 30, 15, 2, 15, 1, 15, 8, "Calculate", onclick_Calculate);
	GCreateText(9, 5, 200, 15, buffer);
	GSimpleWindowLoop();
	GTerminateWindow();
}