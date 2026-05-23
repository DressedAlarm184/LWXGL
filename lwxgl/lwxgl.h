int GCreateWindow(int w, int h, char* name, int bgcol);
void GTerminateWindow();
int GWindowShouldClose();
void GHandleWindowEvents();
void GRenderWindow();
void GCreateText(int id, int x, int y, int color, char* text);
void GCreateButton(int id, int x, int y, int w, int h, int fgu, int bgu, int fgh, int bgh, int fgp, int bgp, char* label);
void GSimpleWindowLoop();