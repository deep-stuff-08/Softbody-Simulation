#ifndef __WINDOWING__
#define __WINDOWING__

typedef struct winParam_t {
	float w;
	float h;
} winParam;

extern winParam winSize;

typedef void (*keyboardfunc)(unsigned int key, int state);
typedef void (*mousefunc)(int x, int y, int state, unsigned button);
typedef void (*mousemotionfunc)(int x, int y);

void toggleFullscreen(void);
void createOpenGLWindow(void);
void processEvents(void);
void closeOpenGLWindow(void);
void destroyOpenGLWindow(void);
bool isOpenGLWindowClosed();
void swapBuffers(void);
void setKeyboardFunc(keyboardfunc key);
void setMouseFunc(mousefunc mouse);
void setMouseMotionFunc(mousemotionfunc mousemotion);

#endif