#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef XlibSpecificationRelease

typedef struct {
	Pixmap bb;
	XFontStruct* font;
	Display* dpy;
	Window win;
	GC gc;
	unsigned long clrs[16];
	int scrn;
	Visual* vis;
	int depth;
	Colormap cmap;
} XConnectionData;

void GetXConnection(XConnectionData* data);

#endif

int CreateWindow(int w, int h, const char* name, int bgcolor);
void TerminateWindow();
void CreateText(int id, int x, int y, const char* text, int color);
void CreateButton(int id, int x, int y, int w, int h, int u, int hvr, int p, const char* label, void (*onclick)(void));
void MainWindowLoop(int target_fps, void (*on_every)(int, float));
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
void SpawnModal(int type, const char* msg, void (*on_confirm)(const char* input));
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
void EnableResizing(void (*Resize)(int x, int y));
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
void SetRenderingOrder(int order);
void ImmediateText(int x, int y, const char* str, int color);
void ImmediateEllipse(int x, int y, int w, int h, int fg, int bg);
void ImmediateRect(int x, int y, int w, int h, int fg, int bg);
void ImmediateLine(int x1, int y1, int x2, int y2, int color);
double GetElapsedTime();
void NewQueuedTask(int type, double run_after, void (*task)());
void ImmediateTextF(int x, int y, int color, const char* fmt, ...);
int CreateOpenGL(int id, int x, int y, int w, int h, int border);
void SynchronizeOpenGL();
void ChangeGLXContext(int id);
unsigned int GLConvertTGA(const char* name);
unsigned int GLObjectListify(const char* obj);

#define KEY_LEFT 170
#define KEY_RIGHT 171
#define KEY_UP 172
#define KEY_DOWN 173
#define KEY_FN 150

#define NONE -1

#define CLR_NONE      -1
#define CLR_BLACK     0x0
#define CLR_BLUE      0x1
#define CLR_GREEN     0x2
#define CLR_CYAN      0x3
#define CLR_RED       0x4
#define CLR_MAGENTA   0x5
#define CLR_ORANGE    0x6
#define CLR_LGRAY     0x7
#define CLR_GRAY      0x8
#define CLR_LBLUE     0x9
#define CLR_LGREEN    0xA
#define CLR_LCYAN     0xB
#define CLR_LRED      0xC
#define CLR_LMAGENTA  0xD
#define CLR_YELLOW    0xE
#define CLR_WHITE     0xF

#define ORDER_ELEM_FIRST 1
#define ORDER_ELEM_SECOND 0

#define MODAL_ALERT 0
#define MODAL_CONFIRM 1
#define MODAL_INPUT 2

#define TASK_RUN_AFTER 0
#define TASK_RUN_EVERY 1

#ifdef __cplusplus
}
#endif
