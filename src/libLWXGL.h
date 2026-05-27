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
void GCreateButton(int id, int x, int y, int w, int h, int u, int hvr, int p, char* label, void (*onclick)(void));
void GSimpleWindowLoop(int target_fps, void (*on_every)(int));
void GDeleteWindow();
void GDeleteElement(int index);
void GCreateInput(int id, int x, int y, int w, int h, int u, int hvr, int max);
char* GGetInput(int id);
void GCreateRect(int id, int x, int y, int w, int h, int fg, int bg);
void GCreateImage(int id, int x, int y, int w, int h);
char* GGetImageData(int id);
void GUpdateImage(int id);
void GPrimitiveRect(int id, int x, int y, int w, int h, int fg, int bg);
void GPrimitiveCircle(int id, int cx, int cy, int r, int fg, int bg);
void GPrimitiveLine(int id, int x1, int y1, int x2, int y2, int color);
void GEventAttachKey(void (*Key)(int key));
void GEventAttachClick(void (*Click)(int x, int y, int btn));
void GPrimitiveSprite(int id, int sx, int sy, int color, char* sprite);

#define GKeyLeft 170
#define GKeyRight 171
#define GKeyUp 172
#define GKeyDown 173
#define GKeyFnBase 150

#ifdef __cplusplus
}
#endif