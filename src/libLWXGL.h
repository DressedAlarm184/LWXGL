#pragma once
#ifdef __cplusplus
extern "C" {
#endif
int GCreateWindow(int w, int h, char* name, int bgcol);
void GTerminateWindow();
int GWindowShouldClose();
void GHandleWindowEvents();
void GRenderWindow();
void GCreateText(int id, int x, int y, int color, char* text);
void GCreateButton(int id, int x, int y, int w, int h, int fgu, int bgu, int fgh, int bgh, int fgp, int bgp, char* label, void (*onclick)(void));
void GSimpleWindowLoop();
void GDeleteWindow();
void GDeleteElement(int index);
void GCreateInput(int id, int x, int y, int w, int h, int fgu, int bgu, int fgh, int bgh, int max);
char* GGetInput(int id);
void GCreateRect(int id, int x, int y, int w, int h, int fg, int bg);
void GCreateImage(int id, int x, int y, int w, int h);
char* GGetImageData(int id);
void GUpdateImage(int id);
#ifdef __cplusplus
}
#endif