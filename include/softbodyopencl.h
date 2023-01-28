#ifndef __SOFTBODY_OPENCL__
#define __SOFTBODY_OPENCL__

#include"vmath.h"
#include"opencldl.h"
#include<vector>
#include<GL/gl.h>

using namespace std;
using namespace vmath;

//InitRelatedCalls
void createFilledCubeOnOpenCL(clcontext_dl *context, cl_kernel kernel, cl_mem vertices, cl_mem verticesOld, vector<ivec3> &indices, cl_mem faceTable, int subdivlevel, float size2);
void applyModelMatrixOnOpenCL(clcontext_dl *context, cl_kernel kernel, cl_mem vertices, cl_mem verticesOld, int subdivlevel, cl_float4 translateMat);
void createSphereOnOpenCL(clcontext_dl *context, cl_kernel kernel, cl_mem vertices, GLuint index, int stacks, int slices);

//Render
void updateSoftbodyOnOpenCL(clcontext_dl *context, cl_kernel kernel, cl_mem vertices, cl_mem verticesOld, cl_mem spherePos, int sphereCount, int subdivlevel, float size2);
void calculateVertexNormalsOnOpenCL(clcontext_dl *context, cl_kernel faceNormalKernel, cl_kernel vertexNormalKernel, cl_mem points, cl_mem pointsNormal, cl_mem pointsIndex, cl_mem faceTable, int subdivlevel);

#endif