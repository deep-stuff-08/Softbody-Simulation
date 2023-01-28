#ifndef __FULLSCREEN_QUAD__
#define __FULLSCREEN_QUAD__

#include<iostream>
#include<GL/gl.h>
#include"shaderloader.h"

typedef void (*FSRenderFunc)(void);

struct fsquad_dl {
	glprogram_dl program;
	GLuint vao;
};

std::string fsquadInit(fsquad_dl* fsquad, std::string vertexShader, std::string fragmentShader);
void fsquadRender(fsquad_dl* fsquad, FSRenderFunc preRenderSetup, GLuint texObj);

#endif