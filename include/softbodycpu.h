#ifndef __SOFTBODY_CPU__
#define __SOFTBODY_CPU__

#include<vector>
#include"vmath.h"
#include<GL/gl.h>

using namespace vmath;
using namespace std;

//InitRelatedCalls
void createFilledCubeOnCpu(GLuint points, GLuint pointsOld, vector<ivec3> &indices, GLuint faceTable, int subdivlevel, float size2);
void applyModelMatrixOnCpu(GLuint points, GLuint pointsOld, int subdivlevel, vec4 translate);
void createSphereOnCpu(GLuint spherePoints, GLuint sphereIndices, int stacks, int slices);

//Render
void updateSoftbodyOnCpu(GLuint points, GLuint pointsOld, vector<vec4> spheres, int sheetDims, float size);
void calculateVertexNormalsOnCpu(GLuint points, GLuint pointsNormal, GLuint pointsIndex, GLuint faceTable, int subdivlevel);

#endif