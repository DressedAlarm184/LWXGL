#pragma once
#ifdef __cplusplus
extern "C" {
#endif

int CreateWindow(int w, int h, const char* name, int bgcol);
void TerminateWindow();
int WindowShouldClose();
void HandleWindowEvents();
void RenderWindow();
void CreateText(int id, int x, int y, const char* text, int color);
void CreateButton(int id, int x, int y, int w, int h, int u, int hvr, int p, const char* label, void (*onclick)(void));
void SimpleWindowLoop(int target_fps, void (*on_every)(int, float));
void DeleteWindow();
void DeleteElement(int index);
void CreateInput(int id, int x, int y, int w, int h, int u, int hvr, int max);
char* GetInput(int id);
void CreateRect(int id, int x, int y, int w, int h, int fg, int bg);
void CreateImage(int id, int x, int y, int w, int h);
unsigned char* GetImageData(int id);
void UpdateImage(int id);
void PrimitiveRect(int id, int x, int y, int w, int h, int fg, int bg);
void PrimitiveCircle(int id, int cx, int cy, int r, int fg, int bg);
void PrimitiveLine(int id, int x1, int y1, int x2, int y2, int color);
void EventAttachKey(void (*Key)(int key));
void EventAttachClick(void (*Click)(int x, int y, int btn));
void PrimitiveSprite(int id, int sx, int sy, int color, const char* sprite, int scale);
void QueryMouse(int* x, int* y, int* btn);
void SpawnModal(int type, const char* msg, void (*on_confirm)());
void EventAttachDelete(int (*on_exit)());
int QueryModalOpen();
unsigned char* QueryKeyboard();
int QueryKeyDown(int ch);
void CreateCheckbox(int id, int x, int y, int size, int cb_col, int txt_col, const char* label);
int GetCheckbox(int id);
void RedrawAllImages();
void PaletteQuery(int idx, unsigned char* r, unsigned char* g, unsigned char* b);
void PaletteModify(int idx, unsigned char r, unsigned char g, unsigned char b, int redraw);
void PaletteReset();
void ClearImage(int id, int c);
void ElemModifyBounds(int id, int x, int y, int w, int h);
void CreateConsole(int id, int x, int y, int cols, int rows, int con_clr, int txt_clr);
void ConsolePrint(int id, const char* format, ...);
void ConsoleClear(int id);
int ElemInside(int id);
void ElemSetVisible(int id, int visible);
void SetWindowTitle(const char* title);
void SetImageFont(int id, unsigned char* font, int h);
void DrawString(int id, int x, int y, const char* txt, int color);
void SetWindowColor(int color);
unsigned char* CaptureRegion(int x, int y, unsigned short w, unsigned short h);
void EnableResizing(void (*Resize)(int w, int h));
void DrawIndexedTGA(int id, int x, int y, const char* name);
int AllocateTGA(const char* name, const char* path, int change_palette, int transparent);
void DeleteTGA(const char* name);
int CreateTGAImage(int id, int x, int y, const char* path, int change_palette);
void ApplyPixelFunc(int id, int (*f)(int, int, int));
void ChangeCursor(int cursor_font_glyph);
void CreateCopiedText(int id, int x, int y, const char* text, int color);
void ReserveScroll(int height, int scrollbar_color, void (*Scroll)(int offset));
int QueryScroll();
int AllocateMemoryTGA(const char* name, const char* buffer, int size, int change_palette, int transparent);
void PrimitiveTriangle(int id, int x1, int y1, int x2, int y2, int x3, int y3, int fg, int bg);
void ElemAnchor(int anchor, int ids[], int count);
void ResolveAnchors();
int SetGlobalBold(int bold);
int AllocateXBM(const char* name, const char* path, int colors, int transparent);
int CreateXBMImage(int id, int x, int y, const char* path, int colors);
void CreateEllipse(int id, int x, int y, int w, int h, int fg, int bg);

#define LWXGL_KEY_LEFT 170
#define LWXGL_KEY_RIGHT 171
#define LWXGL_KEY_UP 172
#define LWXGL_KEY_DOWN 173
#define LWXGL_KEY_FN 150

#ifdef __cplusplus
}
#endif
