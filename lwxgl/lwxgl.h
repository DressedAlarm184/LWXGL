int GCreateWindow(int w, int h, char* name, int bgcol);
void GTerminateWindow();
int GWindowShouldClose();
void GHandleWindowEvents();
void GRenderWindow();
void GCreateText(int id, int x, int y, int color, char* text);