#ifndef __SOFTBODY_COMPUTE__
#define __SOFTBODY_COMPUTE__

#include<vector>
#include"vmath.h"
#include<GL/gl.h>
#include"shaderloader.h"

using namespace vmath;
using namespace std;

//InitRelatedCalls
void createFilledCubeOnCompute(GLuint vao, glprogram_dl program, GLuint points, GLuint pointsOld, vector<ivec3> &indices, GLuint faceTable, int subdivlevel, float size2);
void applyModelMatrixOnCompute(GLuint vao, glprogram_dl program, GLuint points, GLuint pointsOld, int subdivlevel, vec4 translate);
void createSphereOnCompute(GLuint vao, glprogram_dl program, GLuint spherePoints, GLuint sphereIndices, int stacks, int slices);

//Render
void updateSoftbodyOnCompute(GLuint vao, glprogram_dl program, GLuint points, GLuint pointsOld, GLuint spherePosSsbo, int numSpheres, int sheetDims, float size);
void calculateVertexNormalsOnCompute(GLuint vao, glprogram_dl faceprogram, glprogram_dl vertprogram, GLuint points, GLuint pointsNormal, GLuint pointsIndex, GLuint faceTable, int subdivlevel);

#endif