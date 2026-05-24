#include "../lwxgl/lwxgl.h"
#include <stdio.h>
#include <string.h>

char buffer[32];

void onclick_Hash() {
	char* input = GGetInput(0);
	unsigned long long hash = 0xcBF29CE484222325ULL;
	while (*input) {
		hash ^= (unsigned long long)(*input++);
		hash *= 0x100000001B3ULL;
	}
	sprintf(buffer, "0x%016llX", hash);
}

int main() {
	sprintf(buffer, "0x????????????????");
	GCreateWindow(500, 100, "FNV-1a Hasher", 1);
	GCreateInput(0, 10, 60, 420, 30, 15, 8, 14, 8, 45);
	GCreateButton(1, 440, 60, 50, 30, 15, 8, 14, 8, 14, 0, "Hash", onclick_Hash);
	GCreateText(2, 10, 20, 15, "Generated Hash:");
	GCreateText(3, 10, 35, 15, buffer);
	GSimpleWindowLoop();
	GTerminateWindow();
}