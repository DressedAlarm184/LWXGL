#pragma once
#ifdef __cplusplus
extern "C" {
#endif

int GCreateWindow(int w, int h, const char* name, int bgcol);
void GTerminateWindow();
int GWindowShouldClose();
void GHandleWindowEvents();
void GRenderWindow();
void GCreateText(int id, int x, int y, int color, const char* text);
void GCreateButton(int id, int x, int y, int w, int h, int u, int hvr, int p, const char* label, void (*onclick)(void));
void GSimpleWindowLoop(int target_fps, void (*on_every)(int));
void GDeleteWindow();
void GDeleteElement(int index);
void GCreateInput(int id, int x, int y, int w, int h, int u, int hvr, int max);
char* GGetInput(int id);
void GCreateRect(int id, int x, int y, int w, int h, int fg, int bg);
void GCreateImage(int id, int x, int y, int w, int h);
unsigned char* GGetImageData(int id);
void GUpdateImage(int id);
void GPrimitiveRect(int id, int x, int y, int w, int h, int fg, int bg);
void GPrimitiveCircle(int id, int cx, int cy, int r, int fg, int bg);
void GPrimitiveLine(int id, int x1, int y1, int x2, int y2, int color);
void GEventAttachKey(void (*Key)(int key));
void GEventAttachClick(void (*Click)(int x, int y, int btn));
void GPrimitiveSprite(int id, int sx, int sy, int color, const char* sprite, int scale);
void GQueryMouse(int* x, int* y, int* btn);
void GSpawnModal(int type, const char* msg, void (*on_confirm)());
void GEventAttachDelete(int (*on_exit)());
int GQueryModalOpen();
unsigned char* GQueryKeyboard();
int GQueryKeyDown(int ch);
void GCreateCheckbox(int id, int x, int y, int size, int cb_col, int txt_col, const char* label);
int GGetCheckbox(int id);
void GRedrawAllImages();
void GPaletteQuery(int idx, unsigned char* r, unsigned char* g, unsigned char* b);
void GPaletteModify(int idx, unsigned char r, unsigned char g, unsigned char b, int redraw);
void GPaletteReset();
void GClearImage(int id, int c);
void GElemModifyBounds(int id, int x, int y, int w, int h);
void GCreateConsole(int id, int x, int y, int cols, int rows, int clr);
void GConsolePrint(int id, const char* format, ...);
void GConsoleClear(int id);
int GElemInside(int id);
void GElemSetVisible(int id, int visible);

#define LWXGL_KEY_LEFT 170
#define LWXGL_KEY_RIGHT 171
#define LWXGL_KEY_UP 172
#define LWXGL_KEY_DOWN 173
#define LWXGL_KEY_FN 150

#ifdef __cplusplus
}
#endif
