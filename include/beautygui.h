#ifndef __BEAUTY_GUI__
#define __BEAUTY_GUI__

#include"shaderloader.h"
#include"vmath.h"

using namespace vmath;

const float topArrowX = 0.6f;
const float topArrowY = 0.3f;
const float roundedQuadSize = 1.0f;

void initTopArrow();
void initRoundedQuad();
void renderTopArrow(glprogram_dl program, mat4 mvp, vec4 color);
void renderRoundedQuad(glprogram_dl program, mat4 mvp, vec4 color, unsigned texture, bool renderEdges);

#endif