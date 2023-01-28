#ifndef __FPS_LOCK__
#define __FPS_LOCK__

typedef void(*renderFunc)(void);

void initFPSLock();
void setDesiredFPS(int fps);
int getDesiredFPS();
void renderWithFPSLocked(renderFunc render, bool showFPS, bool implement);

extern int cfps;

#endif