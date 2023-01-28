#include<sys/time.h>
#include<stdio.h>
#include<math.h>
#include<unistd.h>
#include"include/fpslock.h"

int desiredFPS = 60;
timeval programStartTime;

void initFPSLock() {
	gettimeofday(&programStartTime, 0);
}

void setDesiredFPS(int fps) {
	desiredFPS = fps;
}

int getDesiredFPS() {
	return desiredFPS;
}

int cfps;

void renderWithFPSLocked(renderFunc render, bool showFPS, bool implement) {
	static timeval startTime;
	static timeval nextTime;
	static int actualFPS = 0;
	static float lastTime = 0.0f;

	gettimeofday(&startTime, 0);
	render();
	gettimeofday(&nextTime, 0);
	long int difference = 1000000 * ( nextTime.tv_sec - startTime.tv_sec ) + (nextTime.tv_usec - startTime.tv_usec);
	long int sleepDuration = (long int)ceil(1000000.0f / desiredFPS) - difference;
	if(sleepDuration > 0) {
		if(implement) {
			usleep(sleepDuration);
		}
	}
	timeval currentTimeVal;
	gettimeofday(&currentTimeVal, 0);
	float currentTime = (1000.0f * (currentTimeVal.tv_sec - programStartTime.tv_sec)) + (0.001f * (currentTimeVal.tv_usec - programStartTime.tv_usec));
	actualFPS++;
	if(currentTime - lastTime >= 1000.0f) {
		lastTime = currentTime;
		cfps = actualFPS;
		if(showFPS) {
			printf("fps : %d\n", actualFPS);
		}
		actualFPS = 0;
	}
}