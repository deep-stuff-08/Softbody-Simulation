//Linux Windowing
#include"include/windowing.h"
#include<X11/keysym.h>
#include<X11/X.h>

//Helper
#include<SOIL/SOIL.h>

//OpenGL
#include<GL/glew.h>
#include<GL/gl.h>
#include"include/vmath.h"

//Time Lib
#include<sys/time.h>
#include<unistd.h>

//Standard C++
#include<iostream>
#include<vector>
#include<string>

//OpenAL
#include<AL/al.h>
#include<AL/alc.h>
#include<AL/alut.h>

//My Libraries
#include"include/shaderloader.h"
#include"include/opencldl.h"
#include"include/fullscreenquad.h"
#include"include/fpslock.h"
#include"include/softbodycpu.h"
#include"include/softbodyopencl.h"
#include"include/softbodyvertex.h"
#include"include/softbodycompute.h"
#include"include/fontrender.h"
#include"include/beautygui.h"

using namespace std;
using namespace vmath;

// #define DISABLE_OPENCL

//helper objects
glprogram_dl basicRender;
glprogram_dl beautyRender;
clcontext_dl clcontext;
FontRender *basicFont;
fsquad_dl fsquad;

//Vertex Programs;
glprogram_dl physicsCalcFilledCubeVert;
glprogram_dl applyModelMatVert;
glprogram_dl createFilledCubeVert;
glprogram_dl createSphereVert;
glprogram_dl calculateFaceNormalsVert;
glprogram_dl calculateVertexNormalsVert;

//Compute Programs
glprogram_dl physicsCalcFilledCubeComp;
glprogram_dl applyModelMatComp;
glprogram_dl createFilledCubeComp;
glprogram_dl createSphereComp;
glprogram_dl calculateFaceNormalsComp;
glprogram_dl calculateVertexNormalsComp;

//OpenAL
ALuint buffer;
ALuint source;
ALenum error;

ALsizei size, freq;
ALenum format;
ALvoid *data;


#ifndef DISABLE_OPENCL
//CL Programs;
cl_kernel physicsCalcFilledCubeCL;
cl_kernel applyModelMatCL;
cl_kernel createFilledCubeCL;
cl_kernel createSphereCL;
cl_kernel calculateFaceNormalsCL;
cl_kernel calculateVertexNormalsCL;

//CL objects
cl_mem pointsCl;
cl_mem pointsCl2;
cl_mem pointsNorCl;
cl_mem pointsECl;
cl_mem pointsFaceTableCl;
cl_mem spherePosCl;
cl_mem sphereCl;
#endif

//GL objects
GLuint pointsVao;
GLuint pointsVbo;
GLuint pointsVbo2;
GLuint pointsNorVbo;
GLuint pointsEabo;
GLuint pointsFaceTableSsbo;
GLuint floorVao;
GLuint floorVbo;
GLuint sphereVao;
GLuint sphereVbo;
GLuint sphereEabo;
GLuint spherePosSsbo;
GLuint cubeVao;
GLuint cubeVbo;
GLuint fbo;
GLuint statsFbo;
GLuint cpuTex;
GLuint vertTex;
GLuint compTex;
GLuint oclTex;
GLuint plusTex;
GLuint minusTex;
GLuint settingsTex;
GLuint randomTex;
GLuint resetTex;
GLuint toggleTex;
GLuint infoTex;
GLuint aboutTex;
GLuint effectsTex;
GLuint sdkTex;
GLuint referTex;
GLuint aboutTitleTex;
GLuint effectsTitleTex;
GLuint sdkTitleTex;
GLuint referTitleTex;
GLuint statsTex;
GLuint titleTex;
GLuint benchmarkTex;
GLuint demoTex;
GLuint avgfpsTex;

GLfloat sumfps = 0.0f;
GLint frameCount = 0;
GLfloat avgFps[4];

float lerp(float a, float b, float mixFactor) {
	GLfloat mixF = mixFactor > 1.0f ? 1.0f: (mixFactor < 0.0f) ? 0.0f : mixFactor;
	GLfloat f = mixF;
	GLfloat ft = 1.0f - mixF;
	return a * ft + b * f;
}

vector<vec4> spherePositions;

//control parameters
enum Platform {
	CPU,
	Vertex,
	Compute,
#ifndef DISABLE_OPENCL
	OpenCL
#endif
};
Platform runningOn = CPU;

enum Scene {
	Title,
	Main,
	Result
};
Scene currentScene = Title;

enum Mode {
	Demo,
	Benchmark
};
Mode mode = Demo;

int subdivlevel = 3;
float size2 = 2.0f;
int numStacks = 20, numSlices = 20;
int numOfSpheres = 5;
int seed = 346;
float center = 10.0f;
float camangle = 0.0f;

//Update States
bool isMenuVisible = false;
bool isSettingsVisble = false;
bool isInfoVisble = false;
int hoveredPlatform = 0;
int hoveredSettings = 0;
int hoveredInfo = 0;
int selectedInfo = 1;
bool isHoverInfo = false;
bool isSphereWireframeEnabled = false;
bool isSoftbodyWireframeEnabled = false;
bool isStatEnabled = false;
bool isPaused = false;
bool sceneSwitch = false;
bool isInited = false;

//Bouding Boxes;
vec4 bottomArrowBoundingBox;
vec4 leftArrowBoundingBox;
vec4 cpuBoundingBox;
vec4 vertBoundingBox;
vec4 compBoundingBox;
vec4 oclBoundingBox;
vec4 randomGenBox;
vec4 sphereLodPlusBox;
vec4 sphereLodMinuxBox;
vec4 sphereCountPlusBox;
vec4 sphereCountMinuxBox;
vec4 softbodyLodPlusBox;
vec4 softbodyLodMinuxBox;
vec4 sphereWireframeBox;
vec4 softbodyWireframeBox;
vec4 statBox;
vec4 resetBox;
vec4 infoBox;
vec4 aboutBoundingBox;
vec4 effectsBoundingBox;
vec4 sdkBoundingBox;
vec4 referBoundingBox;
vec4 playBoundingBox;
vec4 benchmarkBoundingBox;

//Color
vec4 red = vec4(0.9f, 0.3f, 0.3f, 1.0f);
vec4 green = vec4(0.1f, 0.6f, 0.3f, 1.0f);
vec4 blue = vec4(0.25f, 0.5f, 0.9f, 1.0f);
vec4 darkgray = vec4(0.5f, 0.5f, 0.5f, 1.0f);

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ), type, severity, message);
}

void initShaderProgram() {
	glshader_dl pointVert, pointFrag;
	cout<<glshaderCreate(&pointVert, GL_VERTEX_SHADER, "shaders/point.vert");
	cout<<glshaderCreate(&pointFrag, GL_FRAGMENT_SHADER, "shaders/point.frag");
	cout<<glprogramCreate(&basicRender, "Point Shader", vector<glshader_dl>{ pointVert, pointFrag });

	glshader_dl beautyVert, beautyFrag;
	cout<<glshaderCreate(&beautyVert, GL_VERTEX_SHADER, "shaders/beautyrender.vert");
	cout<<glshaderCreate(&beautyFrag, GL_FRAGMENT_SHADER, "shaders/beautyrender.frag");
	cout<<glprogramCreate(&beautyRender, "Beauty Shader", vector<glshader_dl>{ beautyVert, beautyFrag });

	glshader_dl springmassfilledcube;
	cout<<glshaderCreate(&springmassfilledcube, GL_VERTEX_SHADER, "shaders/springmassfilled.vert");
	cout<<glprogramCreate(&physicsCalcFilledCubeVert, "Spring Mass Shader Filled Cube", vector<glshader_dl>{ springmassfilledcube });

	glshader_dl modelMatShader;
	cout<<glshaderCreate(&modelMatShader, GL_VERTEX_SHADER, "shaders/model.vert");
	cout<<glprogramCreate(&applyModelMatVert, "Apply Model Matrix", vector<glshader_dl>{ modelMatShader });

	glshader_dl filledcubeShader;
	cout<<glshaderCreate(&filledcubeShader, GL_VERTEX_SHADER, "shaders/filledcube.vert");
	cout<<glprogramCreate(&createFilledCubeVert, "Create Filled Cube", vector<glshader_dl>{ filledcubeShader });

	glshader_dl sphereShader;
	cout<<glshaderCreate(&sphereShader, GL_VERTEX_SHADER, "shaders/spheres.vert");
	cout<<glprogramCreate(&createSphereVert, "Create Sphere", vector<glshader_dl>{ sphereShader });

	glshader_dl faceNorShader;
	cout<<glshaderCreate(&faceNorShader, GL_VERTEX_SHADER, "shaders/facenormals.vert");
	cout<<glprogramCreate(&calculateFaceNormalsVert, "Face Normals", vector<glshader_dl>{ faceNorShader });

	glshader_dl vertexNorShader;
	cout<<glshaderCreate(&vertexNorShader, GL_VERTEX_SHADER, "shaders/vertexnormal.vert");
	cout<<glprogramCreate(&calculateVertexNormalsVert, "Vertex Normals", vector<glshader_dl>{ vertexNorShader });
	
	glshader_dl springmassfilledcubeco;
	cout<<glshaderCreate(&springmassfilledcubeco, GL_COMPUTE_SHADER, "shaders/springmassfilled.comp");
	cout<<glprogramCreate(&physicsCalcFilledCubeComp, "Spring Mass Shader Filled Cube", vector<glshader_dl>{ springmassfilledcubeco });

	glshader_dl modelMatShaderco;
	cout<<glshaderCreate(&modelMatShaderco, GL_COMPUTE_SHADER, "shaders/model.comp");
	cout<<glprogramCreate(&applyModelMatComp, "Apply Model Matrix", vector<glshader_dl>{ modelMatShaderco });

	glshader_dl filledcubeShaderco;
	cout<<glshaderCreate(&filledcubeShaderco, GL_COMPUTE_SHADER, "shaders/filledcube.comp");
	cout<<glprogramCreate(&createFilledCubeComp, "Create Filled Cube", vector<glshader_dl>{ filledcubeShaderco });

	glshader_dl sphereShaderco;
	cout<<glshaderCreate(&sphereShaderco, GL_COMPUTE_SHADER, "shaders/spheres.comp");
	cout<<glprogramCreate(&createSphereComp, "Create Sphere", vector<glshader_dl>{ sphereShaderco });

	glshader_dl faceNorShaderco;
	cout<<glshaderCreate(&faceNorShaderco, GL_COMPUTE_SHADER, "shaders/facenormals.comp");
	cout<<glprogramCreate(&calculateFaceNormalsComp, "Face Normals", vector<glshader_dl>{ faceNorShaderco });

	glshader_dl vertexNorShaderco;
	cout<<glshaderCreate(&vertexNorShaderco, GL_COMPUTE_SHADER, "shaders/vertexnormal.comp");
	cout<<glprogramCreate(&calculateVertexNormalsComp, "Vertex Normals", vector<glshader_dl>{ vertexNorShaderco });
}

#ifndef DISABLE_OPENCL
void initOpenCL() {
	vector<cl_kernel> kernels;
	cout<<initOpenCLContext(&clcontext);
	cout<<loadKernelsFromPrograms(&clcontext, vector<string>{"kernels/softbody.cl"}, kernels);
	
	for(int i = 0; i < kernels.size(); i++) {
		char buffer[1024];
		clGetKernelInfo(kernels[i], CL_KERNEL_FUNCTION_NAME, sizeof(buffer), buffer, NULL);
		if(string(buffer).compare("createFilledCube") == 0) {
			createFilledCubeCL = kernels[i];
		} else if(string(buffer).compare("createSphere") == 0) {
			createSphereCL = kernels[i];
		} else if(string(buffer).compare("applyModelMat") == 0) {
			applyModelMatCL = kernels[i];
		} else if(string(buffer).compare("applyPhysics") == 0) {
			physicsCalcFilledCubeCL = kernels[i];
		} else if(string(buffer).compare("calculateFaceNormals") == 0) {
			calculateFaceNormalsCL = kernels[i];
		} else if(string(buffer).compare("calculateVertexNormals") == 0) {
			calculateVertexNormalsCL = kernels[i];
		}
	}
}
#endif

void createSoftBodyBuffers() {
	glGenVertexArrays(1, &pointsVao);
	glBindVertexArray(pointsVao);

	glGenBuffers(1, &pointsVbo2);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointsVbo2);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(vec4) * (subdivlevel + 1) * (subdivlevel + 1) * (subdivlevel + 1), NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &pointsNorVbo);
	glBindBuffer(GL_ARRAY_BUFFER, pointsNorVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * (subdivlevel + 1) * (subdivlevel + 1) * (subdivlevel + 1), NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec4), NULL);

	glGenBuffers(1, &pointsVbo);
	glBindBuffer(GL_ARRAY_BUFFER, pointsVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * (subdivlevel + 1) * (subdivlevel + 1) * (subdivlevel + 1), NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);

	glGenBuffers(1, &pointsEabo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pointsEabo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ivec3) * subdivlevel * subdivlevel * 12, NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &pointsFaceTableSsbo);
	glBindBuffer(GL_UNIFORM_BUFFER, pointsFaceTableSsbo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(int) * 6 * (subdivlevel + 1) * (subdivlevel + 1) * (subdivlevel + 1), NULL, GL_DYNAMIC_DRAW);

#ifndef DISABLE_OPENCL
	cout<<createCLGLBuffer(&clcontext, CL_MEM_READ_WRITE, pointsVbo, &pointsCl);
	cout<<createCLGLBuffer(&clcontext, CL_MEM_READ_WRITE, pointsVbo2, &pointsCl2);
	cout<<createCLGLBuffer(&clcontext, CL_MEM_READ_ONLY, pointsEabo, &pointsECl);
	cout<<createCLGLBuffer(&clcontext, CL_MEM_READ_ONLY, pointsFaceTableSsbo, &pointsFaceTableCl);
	cout<<createCLGLBuffer(&clcontext, CL_MEM_READ_WRITE, pointsNorVbo, &pointsNorCl);
#endif
}

void destroySoftBodyBuffers() {
	if(glIsVertexArray(pointsVao)) {
		glDeleteVertexArrays(1, &pointsVao);
	}
	if(glIsBuffer(pointsVbo)) {
		glDeleteBuffers(1, &pointsVbo);
		clReleaseMemObject(pointsCl);
	}
	if(glIsBuffer(pointsVbo2)) {
		glDeleteBuffers(1, &pointsVbo2);
		clReleaseMemObject(pointsCl2);
	}
	if(glIsBuffer(pointsEabo)) {
		glDeleteBuffers(1, &pointsEabo);
		clReleaseMemObject(pointsECl);
	}
	if(glIsBuffer(pointsFaceTableSsbo)) {
		glDeleteBuffers(1, &pointsFaceTableSsbo);
		clReleaseMemObject(pointsFaceTableCl);
	}
	if(glIsBuffer(pointsNorVbo)) {
		glDeleteBuffers(1, &pointsNorVbo);
		clReleaseMemObject(pointsNorCl);
	}
	glFinish();
	clFinish(clcontext.cmdQueue);
}

void initSoftBody() {
	vector<ivec3> indices;
	if(runningOn == CPU) {
		createFilledCubeOnCpu(pointsVbo, pointsVbo2, indices, pointsFaceTableSsbo, subdivlevel, size2);
		applyModelMatrixOnCpu(pointsVbo, pointsVbo2, subdivlevel, vec4(0.0f, 15.9f, 0.0f, 0.0f));
	} else if(runningOn == Vertex) {
		createFilledCubeOnVertex(pointsVao, createFilledCubeVert, pointsVbo, pointsVbo2, indices, pointsFaceTableSsbo, subdivlevel, size2);	
		applyModelMatrixOnVertex(pointsVao, applyModelMatVert, pointsVbo, pointsVbo2, subdivlevel, vec4(0.0f, 15.9f, 0.0f, 0.0f));
		glFinish();
	} else if(runningOn == Compute) {
		createFilledCubeOnCompute(pointsVao, createFilledCubeComp, pointsVbo, pointsVbo2, indices, pointsFaceTableSsbo, subdivlevel, size2);	
		applyModelMatrixOnCompute(pointsVao, applyModelMatComp, pointsVbo, pointsVbo2, subdivlevel, vec4(0.0f, 15.9f, 0.0f, 0.0f));
		glFinish();
	}
#ifndef DISABLE_OPENCL
	else if(runningOn == OpenCL) {
		createFilledCubeOnOpenCL(&clcontext, createFilledCubeCL, pointsCl, pointsCl2, indices, pointsFaceTableCl, subdivlevel, size2);
		cl_float4 transmat = { 0.0f, 15.9f, 0.0f, 0.0f };
		applyModelMatrixOnOpenCL(&clcontext, applyModelMatCL, pointsCl, pointsCl2, subdivlevel, transmat);
	}
#endif
	glNamedBufferSubData(pointsEabo, 0, sizeof(ivec3) * subdivlevel * subdivlevel * 12, indices.data());
	glFinish();
}

vec3 getRandomVector(vec3 limit) {
	GLfloat x = (float)rand() / RAND_MAX * 2.0f * limit[0] - limit[0];
	GLfloat y = (float)rand() / RAND_MAX * 2.0f * limit[1] - limit[1];
	GLfloat z = (float)rand() / RAND_MAX * 2.0f * limit[2] - limit[2];

	return vec3(x, y, z);
}

void createSphereBuffersPos() {
	glGenBuffers(1, &spherePosSsbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, spherePosSsbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(vec4) * numOfSpheres, NULL, GL_DYNAMIC_DRAW);

#ifndef DISABLE_OPENCL
	cout<<createCLGLBuffer(&clcontext, CL_MEM_READ_ONLY, spherePosSsbo, &spherePosCl);
#endif
}

void destorySphereBuffersPos() {
	if(glIsBuffer(spherePosSsbo)) {
		glDeleteBuffers(1, &spherePosSsbo);
	}
}

void randomizeSpherePos() {
	srand(seed);
	spherePositions.clear();
	for(int i = 0; i < numOfSpheres; i++) {
		spherePositions.push_back(vec4(getRandomVector(vec3(12.5f, 14.0f, 9.5f)), 2.0f));
	}
	glNamedBufferSubData(spherePosSsbo, 0, sizeof(vec4) * numOfSpheres, spherePositions.data());
}

void createSphereBuffers() {
	glGenVertexArrays(1, &sphereVao);
	glBindVertexArray(sphereVao);
	glGenBuffers(1, &sphereVbo);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * numStacks * numSlices, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec4), NULL);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec4), NULL);

	glGenBuffers(1, &sphereEabo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEabo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ivec3) * (numStacks - 1) * (numSlices - 1) * 2, NULL, GL_DYNAMIC_DRAW);

#ifndef DISABLE_OPENCL
	cout<<createCLGLBuffer(&clcontext, CL_MEM_READ_WRITE, sphereVbo, &sphereCl);
#endif
}

void destorySphereBuffers() {
	if(glIsBuffer(sphereVbo)) {
		glDeleteBuffers(1, &sphereVbo);
	}
	if(glIsBuffer(sphereEabo)) {
		glDeleteBuffers(1, &sphereEabo);
	}
	if(glIsVertexArray(sphereVao)) {
		glDeleteVertexArrays(1, &sphereVao);
	}
}

void initSphere() {	
	if(runningOn == CPU) {
		createSphereOnCpu(sphereVbo, sphereEabo, numStacks, numSlices);
	} else if(runningOn == Vertex) {
		createSphereOnVertex(sphereVao, createSphereVert, sphereVbo, sphereEabo, numStacks, numSlices);
	} else if(runningOn == Compute) {
		createSphereOnCompute(sphereVao, createSphereComp, sphereVbo, sphereEabo, numStacks, numSlices);
	}
#ifndef DISABLE_OPENCL
	else if(runningOn == OpenCL) {
		createSphereOnOpenCL(&clcontext, createSphereCL, sphereCl, sphereEabo, numStacks, numSlices);
	} 
#endif
}

void initCube() {
	const GLfloat verticesCube[] = {
		//Front
		1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,		0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,		0.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 1.0f,		0.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,
		//Back
		-1.0f, 1.0f, -1.0f,		0.0f, 0.0f, -1.0f,
		1.0f, 1.0f, -1.0f,		0.0f, 0.0f, -1.0f,
		1.0f, -1.0f, -1.0f,		0.0f, 0.0f, -1.0f,
		1.0f, -1.0f, -1.0f,		0.0f, 0.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,	0.0f, 0.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,		0.0f, 0.0f, -1.0f,
		//Right
		1.0f, 1.0f, -1.0f,		-1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, -1.0f,		-1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, -1.0f,		-1.0f, 0.0f, 0.0f,
		//Left
		-1.0f, 1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
		-1.0f, 1.0f, -1.0f,		1.0f, 0.0f, 0.0f,
		-1.0f, -1.0f, -1.0f,	1.0f, 0.0f, 0.0f,
		-1.0f, -1.0f, -1.0f,	1.0f, 0.0f, 0.0f,
		-1.0f, -1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
		-1.0f, 1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
		//Top
		1.0f, 1.0f, -1.0f,		0.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, -1.0f,		0.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 1.0f,		0.0f, -1.0f, 0.0f,
		1.0f, 1.0f, -1.0f,		0.0f, -1.0f, 0.0f,
		//Bottom
		1.0f, -1.0f, -1.0f,		0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, -1.0f,	0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 1.0f,		0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 1.0f,		0.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 1.0f,		0.0f, 1.0f, 0.0f,
		1.0f, -1.0f, -1.0f,		0.0f, 1.0f, 0.0f
	};
	
	glGenVertexArrays(1, &cubeVao);
	glGenBuffers(1, &cubeVbo);
	glBindVertexArray(cubeVao);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCube), verticesCube, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, NULL);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (void*)(sizeof(GLfloat) * 3));
	glEnableVertexAttribArray(1);
}

void loadTextures() {
	int w, h;
	unsigned char* data;
	
	glGenTextures(1, &cpuTex);
	glBindTexture(GL_TEXTURE_2D, cpuTex);
	data = SOIL_load_image("textures/cpu.png", &w, &h, NULL, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	SOIL_free_image_data(data);

	glGenTextures(1, &vertTex);
	glBindTexture(GL_TEXTURE_2D, vertTex);
	data = SOIL_load_image("textures/vertex.png", &w, &h, NULL, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	SOIL_free_image_data(data);

	glGenTextures(1, &compTex);
	glBindTexture(GL_TEXTURE_2D, compTex);
	data = SOIL_load_image("textures/compute.png", &w, &h, NULL, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	SOIL_free_image_data(data);

	glGenTextures(1, &oclTex);
	glBindTexture(GL_TEXTURE_2D, oclTex);
	data = SOIL_load_image("textures/opencl.png", &w, &h, NULL, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	SOIL_free_image_data(data);

	glGenTextures(1, &infoTex);
	glBindTexture(GL_TEXTURE_2D, infoTex);
	data = SOIL_load_image("textures/info.png", &w, &h, NULL, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	SOIL_free_image_data(data);

	glGenTextures(1, &titleTex);
	glBindTexture(GL_TEXTURE_2D, titleTex);
	data = SOIL_load_image("textures/title.png", &w, &h, NULL, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	SOIL_free_image_data(data);
}

void initFont() {
	GLenum drawer[] = { GL_COLOR_ATTACHMENT0 };
	
	glGenFramebuffers(1, &statsFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, statsFbo);
	glDrawBuffers(1, drawer);
	
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glDrawBuffers(1, drawer);

	glGenTextures(1, &plusTex);
	glBindTexture(GL_TEXTURE_2D, plusTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 256, 256);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &minusTex);
	glBindTexture(GL_TEXTURE_2D, minusTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 256, 256);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &settingsTex);
	glBindTexture(GL_TEXTURE_2D, settingsTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 512, 512);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &randomTex);
	glBindTexture(GL_TEXTURE_2D, randomTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 512, 256);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &resetTex);
	glBindTexture(GL_TEXTURE_2D, resetTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 512, 256);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &toggleTex);
	glBindTexture(GL_TEXTURE_2D, toggleTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 512, 256);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &aboutTitleTex);
	glBindTexture(GL_TEXTURE_2D, aboutTitleTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 512, 512);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &effectsTitleTex);
	glBindTexture(GL_TEXTURE_2D, effectsTitleTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 512, 512);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &sdkTitleTex);
	glBindTexture(GL_TEXTURE_2D, sdkTitleTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 512, 512);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &referTitleTex);
	glBindTexture(GL_TEXTURE_2D, referTitleTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 512, 512);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &aboutTex);
	glBindTexture(GL_TEXTURE_2D, aboutTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 512, 512);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &effectsTex);
	glBindTexture(GL_TEXTURE_2D, effectsTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 512, 512);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &sdkTex);
	glBindTexture(GL_TEXTURE_2D, sdkTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 512, 512);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &referTex);
	glBindTexture(GL_TEXTURE_2D, referTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 512, 512);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &statsTex);
	glBindTexture(GL_TEXTURE_2D, statsTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 512, 512);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glGenTextures(1, &demoTex);
	glBindTexture(GL_TEXTURE_2D, demoTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 512, 512);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glGenTextures(1, &benchmarkTex);
	glBindTexture(GL_TEXTURE_2D, benchmarkTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 512, 512);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glGenTextures(1, &avgfpsTex);
	glBindTexture(GL_TEXTURE_2D, avgfpsTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 512, 512);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, plusTex, 0);
	glClearBufferfv(GL_COLOR, 0, green);
	glViewport(0, 0, 256, 256);
	basicFont->renderFont("+", vec2(-380.0f, -280.0f), vec3(1.0f, 1.0f, 1.0f), 15.0f);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, minusTex, 0);
	glClearBufferfv(GL_COLOR, 0, red);
	glViewport(0, 0, 256, 256);
	basicFont->renderFont("-", vec2(-380.0f, -280.0f), vec3(1.0f, 1.0f, 1.0f), 15.0f);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, settingsTex, 0);
	glClearBufferfv(GL_COLOR, 0, vmath::vec4(1.0f, 1.0f, 1.0f, 0.0f));
	glViewport(0, 0, 512, 512);
	basicFont->renderFont("Random Seed", vec2(-499.0f, 400.0f), vec3(1.0f, 1.0f, 1.0f), 1.25f);
	basicFont->renderFont("Sphere LoD", vec2(-499.0f, 270.0f), vec3(1.0f, 1.0f, 1.0f), 1.25f);
	basicFont->renderFont("Sphere Count", vec2(-499.0f, 140.0f), vec3(1.0f, 1.0f, 1.0f), 1.25f);
	basicFont->renderFont("SoftBody LoD", vec2(-499.0f, 10.0f), vec3(1.0f, 1.0f, 1.0f), 1.25f);
	basicFont->renderFont("Sphere Wireframe", vec2(-499.0f, -120.0f), vec3(1.0f, 1.0f, 1.0f), 1.25f);
	basicFont->renderFont("Softbody Wireframe", vec2(-499.0f, -250.0f), vec3(1.0f, 1.0f, 1.0f), 1.25f);
	basicFont->renderFont("Statistics Menu", vec2(-499.0f, -380.0f), vec3(1.0f, 1.0f, 1.0f), 1.25f);
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, randomTex, 0);
	glClearBufferfv(GL_COLOR, 0, blue);
	glViewport(32, 0, 512 - 128, 512);
	basicFont->renderFont("GEN", vec2(-460.0f, -380.0f), vec3(1.0f, 1.0f, 1.0f), 7.0f);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resetTex, 0);
	glClearBufferfv(GL_COLOR, 0, blue);
	glViewport(0, 0, 512, 1024);
	basicFont->renderFont("RESET", vec2(-420.0f, -450.0f), vec3(1.0f, 1.0f, 1.0f), 4.0f);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, aboutTex, 0);
	glClearBufferfv(GL_COLOR, 0, vec4(1.0f, 1.0f, 1.0f, 0.0f));
	glViewport(0, 0, 512, 512);
	basicFont->renderFont("    You are running demo mode so the", vec2(-490.0f, 450.0f), vec3(1.0f, 1.0f, 1.0f), 0.6f);
	basicFont->renderFont("FPS is locked at 60 for your viewing", vec2(-489.0f, 390.0f), vec3(1.0f, 1.0f, 1.0f), 0.6f);
	basicFont->renderFont("pleasure.", vec2(-485.0f, 330.0f), vec3(1.0f, 1.0f, 1.0f), 0.6f);
	basicFont->renderFont("    Softbodies are objects mostly used", vec2(-485.0f, 240.0f), vec3(1.0f, 1.0f, 1.0f), 0.6f);
	basicFont->renderFont("to render organic bodies such as foods", vec2(-485.0f, 180.0f), vec3(1.0f, 1.0f, 1.0f), 0.6f);
	basicFont->renderFont("or organs and soft materials such as", vec2(-485.0f, 120.0f), vec3(1.0f, 1.0f, 1.0f), 0.6f);
	basicFont->renderFont("rubber or plastics.", vec2(-485.0f, 60.0f), vec3(1.0f, 1.0f, 1.0f), 0.6f);
	basicFont->renderFont("    The program uses a Spring - Mass", vec2(-485.0f, -30.0f), vec3(1.0f, 1.0f, 1.0f), 0.6f);
	basicFont->renderFont("System which assumes each point like a", vec2(-485.0f, -90.0f), vec3(1.0f, 1.0f, 1.0f), 0.6f);
	basicFont->renderFont("particle with 'm' mass connected to", vec2(-485.0f, -150.0f), vec3(1.0f, 1.0f, 1.0f), 0.6f);
	basicFont->renderFont("neighboring points with springs. These", vec2(-485.0f, -210.0f), vec3(1.0f, 1.0f, 1.0f), 0.6f);
	basicFont->renderFont("springs use hooke's law to calculate", vec2(-485.0f, -270.0f), vec3(1.0f, 1.0f, 1.0f), 0.6f);
	basicFont->renderFont("force which keeps the body intact but", vec2(-485.0f, -330.0f), vec3(1.0f, 1.0f, 1.0f), 0.6f);
	basicFont->renderFont("also flexible.", vec2(-485.0f, -390.0f), vec3(1.0f, 1.0f, 1.0f), 0.6f);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, effectsTex, 0);
	glClearBufferfv(GL_COLOR, 0, vec4(1.0f, 1.0f, 1.0f, 0.0f));
	glViewport(0, 0, 512, 512);
	basicFont->renderFont("Basic 3D Physics", vec2(-490.0f, 450.0f), vec3(1.0f, 1.0f, 1.0f), 0.7f);
	basicFont->renderFont("3D Collision Detectection", vec2(-489.0f, 370.0f), vec3(1.0f, 1.0f, 1.0f), 0.7f);
	basicFont->renderFont("Spring-Mass System", vec2(-485.0f, 290.0f), vec3(1.0f, 1.0f, 1.0f), 0.7f);
	basicFont->renderFont("Phong Lighting", vec2(-485.0f, 210.0f), vec3(1.0f, 1.0f, 1.0f), 0.7f);
	basicFont->renderFont("Font Rendering", vec2(-485.0f, 130.0f), vec3(1.0f, 1.0f, 1.0f), 0.7f);
	basicFont->renderFont("Mouse Picking", vec2(-485.0f, 50.0f), vec3(1.0f, 1.0f, 1.0f), 0.7f);
	basicFont->renderFont("FPS Locking & Monitoring", vec2(-485.0f, -30.0f), vec3(1.0f, 1.0f, 1.0f), 0.7f);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sdkTex, 0);
	glClearBufferfv(GL_COLOR, 0, vec4(1.0f, 1.0f, 1.0f, 0.0f));
	glViewport(0, 0, 512, 512);
	basicFont->renderFont("X Windows", vec2(-490.0f, 450.0f), vec3(0.0f), 0.7f);
	basicFont->renderFont("OpenGL", vec2(-489.0f, 370.0f), vec3(0.0f), 0.7f);
	basicFont->renderFont("OpenCL", vec2(-485.0f, 290.0f), vec3(0.0f), 0.7f);
	basicFont->renderFont("OpenAL", vec2(-485.0f, 210.0f), vec3(0.0f), 0.7f);
	basicFont->renderFont("FreeType", vec2(-485.0f, 130.0f), vec3(0.0f), 0.7f);
	basicFont->renderFont("SOIL", vec2(-485.0f, 50.0f), vec3(0.0f), 0.7f);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, referTex, 0);
	glClearBufferfv(GL_COLOR, 0, vec4(1.0f, 1.0f, 1.0f, 0.0f));
	glViewport(0, 0, 512, 512);
	basicFont->renderFont("Sir's Assignment", vec2(-490.0f, 450.0f), vec3(0.0f), 0.8f);
	basicFont->renderFont("OpenGL Programming Guide 3rd Edition", vec2(-489.0f, 370.0f), vec3(0.0f), 0.6f);
	basicFont->renderFont("OpenGL Programming Guide 9th Edition", vec2(-485.0f, 290.0f), vec3(0.0f), 0.6f);
	basicFont->renderFont("OpenGL Superbible 7th Edition", vec2(-485.0f, 130.0f), vec3(0.0f), 0.6f);
	basicFont->renderFont("OpenCL Programming Guide 1st Edition", vec2(-485.0f, 210.0f), vec3(0.0f), 0.6f);
	basicFont->renderFont("Soft Objects using Spring Mass System", vec2(-485.0f, 50.0f), vec3(0.0f), 0.6f);
	basicFont->renderFont("Simulate Deformable Objects in GLSL", vec2(-485.0f, -30.0f), vec3(0.0f), 0.6f);
	basicFont->renderFont("Softbody Simulation with CUDA", vec2(-485.0f, -110.0f), vec3(0.0f), 0.6f);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, aboutTitleTex, 0);
	glClearBufferfv(GL_COLOR, 0, vec4(1.0f, 1.0f, 1.0f, 0.0f));
	glViewport(0, 0, 512, 1024);
	basicFont->renderFont("About", vec2(-400.0f, -300.0f), vec3(0.0f, 1.0f, 0.0f), 3.8f);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, effectsTitleTex, 0);
	glClearBufferfv(GL_COLOR, 0, vec4(1.0f, 1.0f, 1.0f, 0.0f));
	glViewport(0, 0, 512, 1024);
	basicFont->renderFont("Effect", vec2(-490.0f, -300.0f), vec3(0.0f, 0.0f, 0.0f), 3.8f);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sdkTitleTex, 0);
	glClearBufferfv(GL_COLOR, 0, vec4(1.0f, 1.0f, 1.0f, 0.0f));
	glViewport(0, 0, 512, 1024);
	basicFont->renderFont("SDKs", vec2(-350.0f, -300.0f), vec3(0.0f, 0.0f, 0.0f), 3.8f);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, referTitleTex, 0);
	glClearBufferfv(GL_COLOR, 0, vec4(1.0f, 1.0f, 1.0f, 0.0f));
	glViewport(0, 0, 512, 1024);
	basicFont->renderFont("Refers", vec2(-480.0f, -300.0f), vec3(0.0f, 0.0f, 0.0f), 3.8f);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, demoTex, 0);
	glClearBufferfv(GL_COLOR, 0, vec4(1.0f, 1.0f, 1.0f, 0.0f));
	glViewport(0, 0, 512, 1024);
	basicFont->renderFont("  Play", vec2(-480.0f, -300.0f), vec3(1.0f), 2.5f);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, benchmarkTex, 0);
	glClearBufferfv(GL_COLOR, 0, vec4(1.0f, 1.0f, 1.0f, 0.0f));
	glViewport(0, 0, 512, 1024);
	basicFont->renderFont("Benchmark", vec2(-480.0f, -300.0f), vec3(1.0f, 1.0f, 1.0f), 2.5f);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, avgfpsTex, 0);
	glClearBufferfv(GL_COLOR, 0, vec4(1.0f, 1.0f, 1.0f, 0.0f));
	glViewport(0, 0, 512, 1024);
	basicFont->renderFont("Average FPS", vec2(-480.0f, -300.0f), vec3(0.0f, 1.0f, 1.0f), 2.0f);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void initDemoMode() {
	createSoftBodyBuffers();
	initSoftBody();
	
	createSphereBuffers();
	initSphere();
	createSphereBuffersPos();
	randomizeSpherePos();
}

void initBenchmark() {
	numOfSpheres = 30;
	subdivlevel = 17;
	seed = 436;


	static timeval startTime[4];
	static timeval endTime[4];

	gettimeofday(&startTime[3], 0);
	runningOn = OpenCL;
	createSoftBodyBuffers();
	initSoftBody();
	createSphereBuffers();
	initSphere();
	createSphereBuffersPos();
	randomizeSpherePos();
	gettimeofday(&endTime[3], 0);

	gettimeofday(&startTime[1], 0);
	runningOn = Vertex;
	createSoftBodyBuffers();
	initSoftBody();
	createSphereBuffers();
	initSphere();
	createSphereBuffersPos();
	randomizeSpherePos();
	gettimeofday(&endTime[1], 0);

	gettimeofday(&startTime[2], 0);
	runningOn = Compute;
	createSoftBodyBuffers();
	initSoftBody();
	createSphereBuffers();
	initSphere();
	createSphereBuffersPos();
	randomizeSpherePos();
	gettimeofday(&endTime[2], 0);

	gettimeofday(&startTime[0], 0);
	runningOn = CPU;
	createSoftBodyBuffers();
	initSoftBody();
	createSphereBuffers();
	initSphere();
	createSphereBuffersPos();
	randomizeSpherePos();
	gettimeofday(&endTime[0], 0);

	for(int i = 0; i < 4; i++) {
		cout<<endTime[i].tv_sec - startTime[i].tv_sec<<"."<<endTime[i].tv_usec - startTime[i].tv_usec<<endl;
	}
}

void init(void) {
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);

	initFPSLock();

	//Create Program
	initShaderProgram();

#ifndef DISABLE_OPENCL
	initOpenCL();
#endif
	initCube();

	initTopArrow();
	initRoundedQuad();

	basicFont = FontRender::createFontRenderer("fonts/FreeMono.otf", "_:().'&|+- :abcdefghijklmnopqrstuvwxyABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890", 72);
	initFont();
	loadTextures();

	fsquadInit(&fsquad, "", "");

	glPointSize(6.0f);
	glEnable(GL_DEPTH_TEST);
}

void printBuffer(GLuint vbo, size_t s) {
	static int count  = 0;
	vec4 *a = (vec4*)glMapNamedBufferRange(vbo, 0, s, GL_MAP_READ_BIT);
	for(int i = 0; i < (subdivlevel + 1) * (subdivlevel + 1) * (subdivlevel + 1); i++) {
		printf("%f %f %f\n", a[i][0], a[i][1], a[i][2]);
	}
	// printf("%d\n", *a);
	printf("\n\n");
	glUnmapNamedBuffer(vbo);
	if(count == 0) {
		// exit(0);
	}
	count++;
}

#define len(x) sizeof(x) / sizeof(x[0])

void updateSoftBodyPosition() {
	if(runningOn == CPU) {
		updateSoftbodyOnCpu(pointsVbo, pointsVbo2, spherePositions, subdivlevel + 1, size2 * 2.0f);
	} else if(runningOn == Vertex) {
		updateSoftbodyOnVertex(pointsVao, physicsCalcFilledCubeVert, pointsVbo, pointsVbo2, spherePosSsbo, numOfSpheres, subdivlevel + 1, size2 * 2.0f);
	} else if(runningOn == Compute) {
		updateSoftbodyOnVertex(pointsVao, physicsCalcFilledCubeVert, pointsVbo, pointsVbo2, spherePosSsbo, numOfSpheres, subdivlevel + 1, size2 * 2.0f);
	}
#ifndef DISABLE_OPENCL
	else if(runningOn == OpenCL) {
		updateSoftbodyOnOpenCL(&clcontext, physicsCalcFilledCubeCL, pointsCl, pointsCl2, spherePosCl, numOfSpheres, subdivlevel + 1, size2 * 2.0f);
	}
#endif
}

void renderObstacleSpheres() {
	if(isSphereWireframeEnabled) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	
	glBindVertexArray(sphereVao);

	vector<mat4> pos;
	for(int i = 0; i < numOfSpheres; i++) {
		pos.push_back(translate(spherePositions[i][0], spherePositions[i][1], spherePositions[i][2]) * scale(spherePositions[i][3]));
	}

	glUniform4f(2, 0.15f, 1.0f, 0.5f, 0.0f);
	glUniform1i(3, 2);
	glUniformMatrix4fv(4, numOfSpheres, GL_FALSE, (float*)pos.data());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEabo);
	glDrawElementsInstanced(GL_TRIANGLES, ((numStacks - 1) * (numSlices - 1) * 2) * 3, GL_UNSIGNED_INT, NULL, numOfSpheres);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void calculateNormalVectors() {
	if(runningOn == CPU) {
		calculateVertexNormalsOnCpu(pointsVbo, pointsNorVbo, pointsEabo, pointsFaceTableSsbo, subdivlevel);
	} else if(runningOn == Vertex) {
		calculateVertexNormalsOnVertex(pointsVao, calculateFaceNormalsVert, calculateVertexNormalsVert, pointsVbo, pointsNorVbo, pointsEabo, pointsFaceTableSsbo, subdivlevel);
	} else if(runningOn == Compute) {
		calculateVertexNormalsOnCompute(pointsVao, calculateFaceNormalsComp, calculateVertexNormalsComp, pointsVbo, pointsNorVbo, pointsEabo, pointsFaceTableSsbo, subdivlevel);
	}
#ifndef DISABLE_OPENCL
	else if(runningOn == OpenCL) {
		calculateVertexNormalsOnOpenCL(&clcontext, calculateFaceNormalsCL, calculateVertexNormalsCL, pointsCl, pointsNorCl, pointsECl, pointsFaceTableCl, subdivlevel);
	}
#endif
}

void renderSoftBody() {
	if(isSoftbodyWireframeEnabled) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	glBindVertexArray(pointsVao);

	glUniform4f(2, 1.0f, 0.0f, 0.0f, 1.0f);
	glUniform1i(3, 1);
	glUniformMatrix4fv(4, 1, GL_FALSE, mat4::identity());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pointsEabo);
	glDrawElements(GL_TRIANGLES, subdivlevel * subdivlevel * 36, GL_UNSIGNED_INT, NULL);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void renderLightSources() {
	glBindVertexArray(sphereVao);

	glUniform4f(2, 1.0f, 1.0f, 1.0f, 1.0f);
	glUniform1i(3, 0);
	glUniformMatrix4fv(4, 1, GL_FALSE, translate(40.0f, 30.0f, 40.0f));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEabo);
	glDrawElements(GL_TRIANGLES, ((numStacks - 1) * (numSlices - 1)) * 3, GL_UNSIGNED_INT, NULL);
	glUniformMatrix4fv(4, 1, GL_FALSE, translate(40.0f, 30.0f, -40.0f));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEabo);
	glDrawElements(GL_TRIANGLES, ((numStacks - 1) * (numSlices - 1)) * 3, GL_UNSIGNED_INT, NULL);
	glUniformMatrix4fv(4, 1, GL_FALSE, translate(-40.0f, 30.0f, -40.0f));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEabo);
	glDrawElements(GL_TRIANGLES, ((numStacks - 2) * (numSlices - 1)) * 3, GL_UNSIGNED_INT, NULL);
	glUniformMatrix4fv(4, 1, GL_FALSE, translate(-40.0f, 30.0f, 40.0f));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEabo);
	glDrawElements(GL_TRIANGLES, ((numStacks - 1) * (numSlices - 1)) * 3, GL_UNSIGNED_INT, NULL);
}

void renderOutsideBox() {
	glBindVertexArray(cubeVao);
	glUniform4f(2, 0.2f, 0.3f, 0.2f, 1.0f);
	glUniform1i(3, 1);
	
	glUniformMatrix4fv(4, 1, GL_FALSE, translate(0.0f, 0.0f, 0.0f) * scale(15.0f, 18.0f, 12.0f));
	glDrawArrays(GL_TRIANGLES, 24, 12);

	//Front-Back
	//Right Side
	glUniformMatrix4fv(4, 1, GL_FALSE, translate(14.5f, 0.0f, 0.0f) * scale(0.5f, 18.0f, 12.0f));
	glDrawArrays(GL_TRIANGLES, 0, 12);
	//Left Side
	glUniformMatrix4fv(4, 1, GL_FALSE, translate(-14.5f, 0.0f, 0.0f) * scale(0.5f, 18.0f, 12.0f));
	glDrawArrays(GL_TRIANGLES, 0, 12);
	//Bottom Side
	glUniformMatrix4fv(4, 1, GL_FALSE, translate(0.0f, -17.0f, 0.0f) * scale(14.0f, 1.0f, 12.0f));
	glDrawArrays(GL_TRIANGLES, 0, 12);
	//Top Side
	glUniformMatrix4fv(4, 1, GL_FALSE, translate(0.0f, 17.0f, 0.0f) * scale(14.0f, 1.0f, 12.0f));
	glDrawArrays(GL_TRIANGLES, 0, 12);
	
	
	//Right-Left
	//Front Side
	glUniformMatrix4fv(4, 1, GL_FALSE, translate(0.0f, 0.0f, 11.5f) * scale(15.0f, 18.0f, 0.5f));
	glDrawArrays(GL_TRIANGLES, 12, 12);

	//Back Side
	glUniformMatrix4fv(4, 1, GL_FALSE, translate(0.0f, 0.0f, -11.5f) * scale(15.0f, 18.0f, 0.5f));
	glDrawArrays(GL_TRIANGLES, 12, 12);

	//Bottom Side
	glUniformMatrix4fv(4, 1, GL_FALSE, translate(0.0f, -17.0f, 0.0f) * scale(15.0f, 1.0f, 11.0f));
	glDrawArrays(GL_TRIANGLES, 12, 12);

	//Top Side
	glUniformMatrix4fv(4, 1, GL_FALSE, translate(0.0f, 17.0f, 0.0f) * scale(15.0f, 1.0f, 11.0f));
	glDrawArrays(GL_TRIANGLES, 12, 12);
}

void renderBoxLegs() {
	glBindVertexArray(cubeVao);
	glUniform4f(2, 0.2f, 0.3f, 0.2f, 1.0f);
	glUniform1i(3, 1);
	
	glUniformMatrix4fv(4, 1, GL_FALSE, translate(14.0f, -28.0f, 11.0f) * scale(1.0f, 10.0f, 1.0f));
	glDrawArrays(GL_TRIANGLES, 0, 24);
	glUniformMatrix4fv(4, 1, GL_FALSE, translate(14.0f, -28.0f, -11.0f) * scale(1.0f, 10.0f, 1.0f));
	glDrawArrays(GL_TRIANGLES, 0, 24);
	glUniformMatrix4fv(4, 1, GL_FALSE, translate(-14.0f, -28.0f, -11.0f) * scale(1.0f, 10.0f, 1.0f));
	glDrawArrays(GL_TRIANGLES, 0, 24);
	glUniformMatrix4fv(4, 1, GL_FALSE, translate(-14.0f, -28.0f, 11.0f) * scale(1.0f, 10.0f, 1.0f));
	glDrawArrays(GL_TRIANGLES, 0, 24);
}

void renderGlassBox() {
	glBindVertexArray(cubeVao);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glUniform4f(2, 0.8f, 0.9f, 1.0f, 0.2f);
	glUniform1i(3, 0);
	glUniformMatrix4fv(4, 1, GL_FALSE, scale(14.99f, 17.99f, 11.99f));
	glDrawArrays(GL_TRIANGLES, 0, 24);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}

void calculateBoundingBox(vec4 &boundBox, mat4 mvp, vec4 posPos, vec4 posNeg) {
	vec4 top = (mvp * posPos)[0];
	vec4 bottom = (mvp * posNeg)[0];
	
	boundBox = vec4(top[0], top[1], bottom[0], bottom[1]);
}

void renderPlatformSelector(mat4 proj) {
	const GLfloat maxPlatformY = 6.0f;
	static GLfloat platformChanger = 0.0f;

	if(isMenuVisible) {
		if(platformChanger < 1.0f) {
			platformChanger += 0.02f;
		}
	} else {
		if(platformChanger > 0.0f) {
			platformChanger -= 0.02f;
		}
	}

	GLfloat platformSelectorY = lerp(0.0f, maxPlatformY, platformChanger);

	renderTopArrow(beautyRender, proj * translate(0.0f, -9.7f + platformSelectorY, 0.0f) * rotate(180.0f, 0.0f, 0.0f, 1.0f), vec4(0.8f, 0.8f, 0.8f, 1.0f));
	calculateBoundingBox(bottomArrowBoundingBox, proj * translate(0.0f, -9.7f + platformSelectorY, 0.0f), vec4(topArrowX, topArrowY, 0.0f, 1.0f), vec4(-topArrowX, -topArrowY, 0.0f, 1.0f));
	
	renderRoundedQuad(beautyRender, proj * translate(0.0f, -20.5f + platformSelectorY, 0.0f) * scale(10.5f), vec4(0.5f, 0.5f, 0.5f, 0.8f), 0, true);
	
	vec4 currentColor;
	
	currentColor = darkgray;
	if(runningOn == CPU) {
		currentColor = darkgray * 0.7f;
	} else if(hoveredPlatform == 1) {
		currentColor = darkgray * 1.7f;
	}
	renderRoundedQuad(beautyRender, proj * translate(-7.5f, -13.0f + platformSelectorY, 0.0f) * scale(2.25f), currentColor, 0, true);
	renderRoundedQuad(beautyRender, proj * translate(-7.5f, -13.0f + platformSelectorY, 0.0f) * scale(2.35f), currentColor, cpuTex, false);
	calculateBoundingBox(cpuBoundingBox, proj * translate(-7.5f, -13.0f + platformSelectorY, 0.0f) * scale(2.25f), vec4(roundedQuadSize, roundedQuadSize, 0.0f, 1.0f), vec4(-roundedQuadSize, -roundedQuadSize, 0.0f, 1.0f));

	currentColor = darkgray;
	if(runningOn == Vertex) {
		currentColor = darkgray * 0.7f;
	} else if(hoveredPlatform == 2) {
		currentColor = darkgray * 1.7f;
	}
	renderRoundedQuad(beautyRender, proj * translate(-2.5f, -13.0f + platformSelectorY, 0.0f) * scale(2.25f), currentColor, 0, true);
	renderRoundedQuad(beautyRender, proj * translate(-2.5f, -13.0f + platformSelectorY, 0.0f) * scale(2.35f), currentColor, vertTex, false);
	calculateBoundingBox(vertBoundingBox, proj * translate(-2.5f, -13.0f + platformSelectorY, 0.0f) * scale(2.25f), vec4(roundedQuadSize, roundedQuadSize, 0.0f, 1.0f), vec4(-roundedQuadSize, -roundedQuadSize, 0.0f, 1.0f));
	
	currentColor = darkgray;
	if(runningOn == Compute) {
		currentColor = darkgray * 0.7f;
	} else if(hoveredPlatform == 3) {
		currentColor = darkgray * 1.7f;
	}
	renderRoundedQuad(beautyRender, proj * translate(2.5f, -13.0f + platformSelectorY, 0.0f) * scale(2.25f), currentColor, 0, true);
	renderRoundedQuad(beautyRender, proj * translate(2.5f, -13.0f + platformSelectorY, 0.0f) * scale(2.35f), currentColor, compTex, false);
	calculateBoundingBox(compBoundingBox, proj * translate(2.5f, -13.0f + platformSelectorY, 0.0f) * scale(2.25f), vec4(roundedQuadSize, roundedQuadSize, 0.0f, 1.0f), vec4(-roundedQuadSize, -roundedQuadSize, 0.0f, 1.0f));
	
	currentColor = darkgray;
	if(runningOn == OpenCL) {
		currentColor = darkgray * 0.7f;
	} else if(hoveredPlatform == 4) {
		currentColor = darkgray * 1.7f;
	}
	renderRoundedQuad(beautyRender, proj * translate(7.5f, -13.0f + platformSelectorY, 0.0f) * scale(2.25f), currentColor, 0, true);
	renderRoundedQuad(beautyRender, proj * translate(7.5f, -13.0f + platformSelectorY, 0.0f) * scale(2.35f), currentColor, oclTex, false);
	calculateBoundingBox(oclBoundingBox, proj * translate(7.5f, -13.0f + platformSelectorY, 0.0f) * scale(2.25f), vec4(roundedQuadSize, roundedQuadSize, 0.0f, 1.0f), vec4(-roundedQuadSize, -roundedQuadSize, 0.0f, 1.0f));
}

void renderSettingsChanger(mat4 proj) {
	const GLfloat maxSettingsX = 11.5f;
	static GLfloat settingsChanger = 0.0f;
	if(isSettingsVisble) {
		if(settingsChanger < 1.0f) {
			settingsChanger += 0.02f;
		}
	} else {
		if(settingsChanger > 0.0f) {
			settingsChanger -= 0.02f;
		}
	}
	GLfloat settingsChangerX = lerp(0.0f, maxSettingsX, settingsChanger);
	
	const GLfloat maxToggleX = 0.6f;
	static GLfloat sphereChanger = 0.0f;
	if(isSphereWireframeEnabled) {
		if(sphereChanger < 1.0f) {
			sphereChanger += 0.04f;
		}
	} else {
		if(sphereChanger > 0.0f) {
			sphereChanger -= 0.04f;
		}
	}
	GLfloat sphereChangerX = lerp(-maxToggleX, maxToggleX, sphereChanger);
	static GLfloat softbodyChanger = 0.0f;
	if(isSoftbodyWireframeEnabled) {
		if(softbodyChanger < 1.0f) {
			softbodyChanger += 0.04f;
		}
	} else {
		if(softbodyChanger > 0.0f) {
			softbodyChanger -= 0.04f;
		}
	}
	GLfloat softbodyChangerX = lerp(-maxToggleX, maxToggleX, softbodyChanger);
	
	static GLfloat statChanger = 0.0f;
	if(isStatEnabled) {
		if(statChanger < 1.0f) {
			statChanger += 0.04f;
		}
	} else {
		if(statChanger > 0.0f) {
			statChanger -= 0.04f;
		}
	}
	GLfloat statChangerX = lerp(-maxToggleX, maxToggleX, statChanger);
	
	GLfloat transconst = -10.0f * (winSize.w / winSize.h);

	renderTopArrow(beautyRender, proj * translate(transconst + topArrowY + settingsChangerX, 0.0f, 0.0f) * rotate(90.0f, 0.0f, 0.0f, 1.0f), vec4(0.8f, 0.8f, 0.8f, 1.0f));
	calculateBoundingBox(leftArrowBoundingBox, proj * translate(transconst + topArrowY + settingsChangerX, 0.0f, 0.0f) * rotate(90.0f, 0.0f, 0.0f, 1.0f), vec4(topArrowX, -topArrowY, 0.0f, 1.0f), vec4(-topArrowX, topArrowY, 0.0f, 1.0f));

	renderRoundedQuad(beautyRender, proj * translate(transconst - 8.5f + settingsChangerX, 0.0f, 0.0f) * scale(8.5f), vec4(0.5f, 0.5f, 0.5f, 0.8f), 0, true);

	renderRoundedQuad(beautyRender, proj * translate(transconst - 7.0f + settingsChangerX, 0.0f, 0.0f) * scale(4.5f, 8.0f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f), settingsTex, true);

	vec4 color;
	const vec4 unselected = vec4(0.8f, 0.8f, 0.8f, 1.0f);
	const vec4 hover = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	
	//1
	if(hoveredSettings == 1) {
		color = hover;
	} else {
		color = unselected;
	}
	renderRoundedQuad(beautyRender, proj * translate(transconst - 1.5f + settingsChangerX, 6.1f, 0.0f) * scale(1.2f, 0.5f, 1.0f), color, randomTex, true);
	calculateBoundingBox(randomGenBox, proj * translate(transconst - 1.5f + settingsChangerX, 6.1f, 0.0f) * scale(1.2f, 0.5f, 1.0f), vec4(roundedQuadSize, roundedQuadSize, 0.0f, 1.0f), vec4(-roundedQuadSize, -roundedQuadSize, 0.0f, 1.0f));

	//2
	if(hoveredSettings == 2) {
		color = hover;
	} else {
		color = unselected;
	}
	renderRoundedQuad(beautyRender, proj * translate(transconst - 0.9f + settingsChangerX, 4.2f, 0.0f) * scale(0.5f, 0.5f, 1.0f), color, plusTex, true);
	calculateBoundingBox(sphereLodPlusBox, proj * translate(transconst - 0.9f + settingsChangerX, 4.2f, 0.0f) * scale(0.5f, 0.5f, 1.0f), vec4(roundedQuadSize, roundedQuadSize, 0.0f, 1.0f), vec4(-roundedQuadSize, -roundedQuadSize, 0.0f, 1.0f));
	if(hoveredSettings == 3) {
		color = hover;
	} else {
		color = unselected;
	}
	renderRoundedQuad(beautyRender, proj * translate(transconst - 2.1f + settingsChangerX, 4.2f, 0.0f) * scale(0.5f, 0.5f, 1.0f), color, minusTex, true);
	calculateBoundingBox(sphereLodMinuxBox, proj * translate(transconst - 2.1f + settingsChangerX, 4.2f, 0.0f) * scale(0.5f, 0.5f, 1.0f), vec4(roundedQuadSize, roundedQuadSize, 0.0f, 1.0f), vec4(-roundedQuadSize, -roundedQuadSize, 0.0f, 1.0f));

	//3
	if(hoveredSettings == 4) {
		color = hover;
	} else {
		color = unselected;
	}
	renderRoundedQuad(beautyRender, proj * translate(transconst - 0.9f + settingsChangerX, 2.4f, 0.0f) * scale(0.5f, 0.5f, 1.0f), color, plusTex, true);
	calculateBoundingBox(sphereCountPlusBox, proj * translate(transconst - 0.9f + settingsChangerX, 2.4f, 0.0f) * scale(0.5f, 0.5f, 1.0f), vec4(roundedQuadSize, roundedQuadSize, 0.0f, 1.0f), vec4(-roundedQuadSize, -roundedQuadSize, 0.0f, 1.0f));
	if(hoveredSettings == 5) {
		color = hover;
	} else {
		color = unselected;
	}
	renderRoundedQuad(beautyRender, proj * translate(transconst - 2.1f + settingsChangerX, 2.4f, 0.0f) * scale(0.5f, 0.5f, 1.0f), color, minusTex, true);
	calculateBoundingBox(sphereCountMinuxBox, proj * translate(transconst - 2.1f + settingsChangerX, 2.4f, 0.0f) * scale(0.5f, 0.5f, 1.0f), vec4(roundedQuadSize, roundedQuadSize, 0.0f, 1.0f), vec4(-roundedQuadSize, -roundedQuadSize, 0.0f, 1.0f));

	//4
	if(hoveredSettings == 6) {
		color = hover;
	} else {
		color = unselected;
	}
	renderRoundedQuad(beautyRender, proj * translate(transconst - 0.9f + settingsChangerX, 0.5f, 0.0f) * scale(0.5f, 0.5f, 1.0f), color, plusTex, true);
	calculateBoundingBox(softbodyLodPlusBox, proj * translate(transconst - 0.9f + settingsChangerX, 0.5f, 0.0f) * scale(0.5f, 0.5f, 1.0f), vec4(roundedQuadSize, roundedQuadSize, 0.0f, 1.0f), vec4(-roundedQuadSize, -roundedQuadSize, 0.0f, 1.0f));
	if(hoveredSettings == 7) {
		color = hover;
	} else {
		color = unselected;
	}
	renderRoundedQuad(beautyRender, proj * translate(transconst - 2.1f + settingsChangerX, 0.5f, 0.0f) * scale(0.5f, 0.5f, 1.0f), color, minusTex, true);
	calculateBoundingBox(softbodyLodMinuxBox, proj * translate(transconst - 2.1f + settingsChangerX, 0.5f, 0.0f) * scale(0.5f, 0.5f, 1.0f), vec4(roundedQuadSize, roundedQuadSize, 0.0f, 1.0f), vec4(-roundedQuadSize, -roundedQuadSize, 0.0f, 1.0f));

	//5
	if(hoveredSettings == 8) {
		color = hover;
	} else {
		color = unselected;
	}
	if(isSphereWireframeEnabled) {
		color *= green;
	} else {
		color *= red;
	}
	renderRoundedQuad(beautyRender, proj * translate(transconst - 1.5f + settingsChangerX, -1.4f, 0.0f) * scale(1.2f, 0.4f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f), 0, true);
	renderRoundedQuad(beautyRender, proj * translate(transconst - 1.5f + sphereChangerX + settingsChangerX, -1.4f, 0.0f) * scale(0.6f, 0.5f, 1.0f), color, 0, true);
	calculateBoundingBox(sphereWireframeBox, proj * translate(transconst - 1.5f + sphereChangerX + settingsChangerX, -1.4f, 0.0f) * scale(0.6f, 0.5f, 1.0f), vec4(roundedQuadSize, roundedQuadSize, 0.0f, 1.0f), vec4(-roundedQuadSize, -roundedQuadSize, 0.0f, 1.0f));
	
	//6
	if(hoveredSettings == 9) {
		color = hover;
	} else {
		color = unselected;
	}
	if(isSoftbodyWireframeEnabled) {
		color *= green;
	} else {
		color *= red;
	}
	renderRoundedQuad(beautyRender, proj * translate(transconst - 1.5f + settingsChangerX, -3.2f, 0.0f) * scale(1.2f, 0.4f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f), 0, true);
	renderRoundedQuad(beautyRender, proj * translate(transconst - 1.5f + softbodyChangerX + settingsChangerX, -3.2f, 0.0f) * scale(0.6f, 0.5f, 1.0f), color, 0, true);
	calculateBoundingBox(softbodyWireframeBox, proj * translate(transconst - 1.5f + softbodyChangerX + settingsChangerX, -3.2f, 0.0f) * scale(0.6f, 0.5f, 1.0f), vec4(roundedQuadSize, roundedQuadSize, 0.0f, 1.0f), vec4(-roundedQuadSize, -roundedQuadSize, 0.0f, 1.0f));
	
	if(hoveredSettings == 10) {
		color = hover;
	} else {
		color = unselected;
	}
	if(isStatEnabled) {
		color *= green;
	} else {
		color *= red;
	}
	renderRoundedQuad(beautyRender, proj * translate(transconst - 1.5f + settingsChangerX, -5.1f, 0.0f) * scale(1.2f, 0.4f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f), 0, true);
	renderRoundedQuad(beautyRender, proj * translate(transconst - 1.5f + statChangerX + settingsChangerX, -5.1f, 0.0f) * scale(0.6f, 0.5f, 1.0f), color, 0, true);
	calculateBoundingBox(statBox, proj * translate(transconst - 1.5f + statChangerX + settingsChangerX, -5.1f, 0.0f) * scale(0.6f, 0.5f, 1.0f), vec4(roundedQuadSize, roundedQuadSize, 0.0f, 1.0f), vec4(-roundedQuadSize, -roundedQuadSize, 0.0f, 1.0f));

	//8
	if(hoveredSettings == 11) {
		color = hover;
	} else {
		color = unselected;
	}
	renderRoundedQuad(beautyRender, proj * translate(transconst - 5.5f + settingsChangerX, -6.7f, 0.0f) * scale(1.8f, 0.5f, 1.0f), color, resetTex, true);
	calculateBoundingBox(resetBox, proj * translate(transconst - 5.5f + settingsChangerX, -6.7f, 0.0f) * scale(1.8f, 0.5f, 1.0f), vec4(roundedQuadSize, roundedQuadSize, 0.0f, 1.0f), vec4(-roundedQuadSize, -roundedQuadSize, 0.0f, 1.0f));
}

void renderInfoBox(mat4 proj) {
	GLfloat transconst = -10.0f * (winSize.w / winSize.h);
	vec4 color = darkgray + 0.1f;
	if(isHoverInfo) {
		color = darkgray * 1.7f;
	}
	renderRoundedQuad(beautyRender, proj * translate(transconst + 0.65f, 9.35f, 0.0f) * scale(0.65f), color, infoTex, false);
	calculateBoundingBox(infoBox, proj * translate(transconst + 0.65f, 9.35f, 0.0f) * scale(0.65f), vec4(roundedQuadSize, roundedQuadSize, 0.0f, 1.0f), vec4(-roundedQuadSize, -roundedQuadSize, 0.0f, 1.0f));
}

void renderInfoPanel(mat4 proj) {
	GLfloat transconst = -10.0f * (winSize.w / winSize.h);
	const GLfloat maxInfoX = transconst - 8.0f;
	const GLfloat maxInfoY = 18.0f;
	static GLfloat infoChanger = 0.0f;
	if(isInfoVisble) {
		if(infoChanger < 1.0f) {
			infoChanger += 0.02f;
		}
	} else {
		if(infoChanger > 0.0f) {
			infoChanger -= 0.02f;
		}
	}
	GLfloat infoChangerX = lerp(maxInfoX, 0.0f, infoChanger);
	GLfloat infoChangerY = lerp(maxInfoY, 0.0f, infoChanger);

	renderRoundedQuad(beautyRender, proj * translate(infoChangerX, infoChangerY, 0.0f) * scale(8.0f), vec4(0.5f, 0.5f, 0.5f, 0.9f), 0, true);
	GLuint tex;
	if(selectedInfo == 1) {
		tex = aboutTex;
	} else if(selectedInfo == 2) {
		tex = effectsTex;
	} else if(selectedInfo == 3) {
		tex = sdkTex;
	} else if(selectedInfo == 4) {
		tex = referTex;
	}
	renderRoundedQuad(beautyRender, proj * translate(infoChangerX, infoChangerY - 1.0f, 0.0f) * scale(8.0f, 7.0f, 1.0f), vec4(0.1f, 0.1f, 0.1f, 1.0f), tex, true);

	vec4 color;
	
	color = darkgray;
	if(selectedInfo == 1) {
		color = darkgray * 0.7f;
	} else if(hoveredInfo == 1) {
		color = darkgray * 1.7f;
	}
	renderRoundedQuad(beautyRender, proj * translate(infoChangerX + 5.75f, infoChangerY + 7.0f, 0.0f) * scale(1.5f, 0.5f, 1.0f), color, 0, true);
	renderRoundedQuad(beautyRender, proj * translate(infoChangerX + 5.75f, infoChangerY + 7.0f, 0.0f) * scale(1.5f, 0.5f, 1.0f), vec4(0.1f, 0.1f, 0.1f, 1.0f), aboutTitleTex, true);
	calculateBoundingBox(aboutBoundingBox, proj * translate(infoChangerX + 5.75f, infoChangerY + 7.0f, 0.0f) * scale(1.5f, 0.5f, 1.0f), vec4(roundedQuadSize, roundedQuadSize, 0.0f, 1.0f), vec4(-roundedQuadSize, -roundedQuadSize, 0.0f, 1.0f));
	
	color = darkgray;
	if(selectedInfo == 2) {
		color = darkgray * 0.7f;
	} else if(hoveredInfo == 2) {
		color = darkgray * 1.7f;
	}
	renderRoundedQuad(beautyRender, proj * translate(infoChangerX + 1.875f, infoChangerY + 7.0f, 0.0f) * scale(1.5f, 0.5f, 1.0f), color, 0, true);
	renderRoundedQuad(beautyRender, proj * translate(infoChangerX + 1.875f, infoChangerY + 7.0f, 0.0f) * scale(1.5f, 0.5f, 1.0f), vec4(0.1f, 0.1f, 0.1f, 1.0f), effectsTitleTex, true);
	calculateBoundingBox(effectsBoundingBox, proj * translate(infoChangerX + 1.875f, infoChangerY + 7.0f, 0.0f) * scale(1.5f, 0.5f, 1.0f), vec4(roundedQuadSize, roundedQuadSize, 0.0f, 1.0f), vec4(-roundedQuadSize, -roundedQuadSize, 0.0f, 1.0f));
	
	color = darkgray;
	if(selectedInfo == 3) {
		color = darkgray * 0.7f;
	} else if(hoveredInfo == 3) {
		color = darkgray * 1.7f;
	}
	renderRoundedQuad(beautyRender, proj * translate(infoChangerX - 1.875f, infoChangerY + 7.0f, 0.0f) * scale(1.5f, 0.5f, 1.0f), color, 0, true);
	renderRoundedQuad(beautyRender, proj * translate(infoChangerX - 1.875f, infoChangerY + 7.0f, 0.0f) * scale(1.5f, 0.5f, 1.0f), vec4(0.1f, 0.1f, 0.1f, 1.0f), sdkTitleTex, true);
	calculateBoundingBox(sdkBoundingBox, proj * translate(infoChangerX - 1.875f, infoChangerY + 7.0f, 0.0f) * scale(1.5f, 0.5f, 1.0f), vec4(roundedQuadSize, roundedQuadSize, 0.0f, 1.0f), vec4(-roundedQuadSize, -roundedQuadSize, 0.0f, 1.0f));
	
	color = darkgray;
	if(selectedInfo == 4) {
		color = darkgray * 0.7f;
	} else if(hoveredInfo == 4) {
		color = darkgray * 1.7f;
	}
	renderRoundedQuad(beautyRender, proj * translate(infoChangerX - 5.75f, infoChangerY + 7.0f, 0.0f) * scale(1.5f, 0.5f, 1.0f), color, 0, true);
	renderRoundedQuad(beautyRender, proj * translate(infoChangerX - 5.75f, infoChangerY + 7.0f, 0.0f) * scale(1.5f, 0.5f, 1.0f), vec4(0.1f, 0.1f, 0.1f, 1.0f), referTitleTex, true);
	calculateBoundingBox(referBoundingBox, proj * translate(infoChangerX - 5.75f, infoChangerY + 7.0f, 0.0f) * scale(1.5f, 0.5f, 1.0f), vec4(roundedQuadSize, roundedQuadSize, 0.0f, 1.0f), vec4(-roundedQuadSize, -roundedQuadSize, 0.0f, 1.0f));
}

void renderStats(mat4 proj, bool isVisible) {
	static GLfloat statAlpha = 0.0f;
	if(isStatEnabled) {
		if(statAlpha < 1.0f) {
			statAlpha += 0.02f;
		}
	} else {
		if(statAlpha > 0.0f) {
			statAlpha -= 0.02f;
		}
	}
	GLfloat transconst = 10.0f * (winSize.w / winSize.h);
	
	renderRoundedQuad(beautyRender, proj * translate(transconst - 5.0f, 3.0f, 0.0f) * scale(5.0f, 7.0f, 1.0f), vec4(0.2f, 0.2f, 0.2f, 0.5f * isVisible ? 1.0f : statAlpha), 0, true);
	renderRoundedQuad(beautyRender, proj * translate(transconst - 5.0f, 3.0f, 0.0f) * scale(5.0f, 7.2f, 1.0f), vec4(1.0f, 1.0f, 1.0f, isVisible ? 1.0f : statAlpha), statsTex, false);
}

void renderStatsFont() {
	glBindFramebuffer(GL_FRAMEBUFFER, statsFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, statsTex, 0);
	glClearBufferfv(GL_COLOR, 0, vec4(1.0f, 1.0f, 1.0f, 0.0f));
	glViewport(0, 0, 512, 512);
	string buffer;
	buffer = "FPS            " + to_string(cfps);
	basicFont->renderFont(buffer.c_str(), vec2(-490.0f, 445.0f), vec3(1.0f, 1.0f, 1.0f), 1.2f);
	buffer = "Running On           ";
	if(runningOn == CPU) {
		buffer += "CPU";
	} else if(runningOn == Vertex) {
		buffer += "OpenGL(Vertex)";
	} else if(runningOn == Compute) {
		buffer += "OpenGL(Compute)";
	} else if(runningOn == OpenCL) {
		buffer += "OpenCL";
	}
	basicFont->renderFont(buffer.c_str(), vec2(-490.0f, 385.0f), vec3(1.0f, 1.0f, 1.0f), 0.65f);
	buffer = "Current Seed         " + to_string(seed);
	basicFont->renderFont(buffer.c_str(), vec2(-490.0f, 325.0f), vec3(1.0f, 1.0f, 1.0f), 0.65f);
	buffer = "Sphere LoD           " + to_string(numStacks) + " x " + to_string(numSlices);
	basicFont->renderFont(buffer.c_str(), vec2(-490.0f, 265.0f), vec3(1.0f, 1.0f, 1.0f), 0.65f);
	buffer = "Sphere Count         " + to_string(numOfSpheres);
	basicFont->renderFont(buffer.c_str(), vec2(-490.0f, 205.0f), vec3(1.0f, 1.0f, 1.0f), 0.65f);
	buffer = "Softbody LoD         " + to_string(subdivlevel) + " x " + to_string(subdivlevel) + " x " + to_string(subdivlevel);
	basicFont->renderFont(buffer.c_str(), vec2(-490.0f, 145.0f), vec3(1.0f, 1.0f, 1.0f), 0.65f);
	
	buffer = "Points Calculated at Init:";
	basicFont->renderFont(buffer.c_str(), vec2(-490.0f, 55.0f), vec3(1.0f, 1.0f, 1.0f), 0.8f);
	buffer = "Sphere                      " + to_string(numStacks * numSlices);
	basicFont->renderFont(buffer.c_str(), vec2(-490.0f, -10.0f), vec3(1.0f, 1.0f, 1.0f), 0.65f);
	buffer = "Softbody                    " + to_string((subdivlevel+1)*(subdivlevel+1)*(subdivlevel+1));
	basicFont->renderFont(buffer.c_str(), vec2(-490.0f, -70.0f), vec3(1.0f, 1.0f, 1.0f), 0.65f);
	buffer = "__________________________________";
	basicFont->renderFont(buffer.c_str(), vec2(-490.0f, -85.0f), vec3(1.0f, 1.0f, 1.0f), 0.65f);
	buffer = "Total Points                " + to_string(numStacks * numSlices + (subdivlevel+1)*(subdivlevel+1)*(subdivlevel+1));
	basicFont->renderFont(buffer.c_str(), vec2(-490.0f, -135.0f), vec3(1.0f, 1.0f, 1.0f), 0.65f);

	buffer = "Calculations at Render:";
	basicFont->renderFont(buffer.c_str(), vec2(-490.0f, -210.0f), vec3(1.0f, 1.0f, 1.0f), 0.8f);
	buffer = "Softbody Physics            " + to_string((subdivlevel+1)*(subdivlevel+1)*(subdivlevel+1)*40);
	basicFont->renderFont(buffer.c_str(), vec2(-490.0f, -275.0f), vec3(1.0f, 1.0f, 1.0f), 0.65f);
	buffer = "Softbody Collision          " + to_string((subdivlevel+1)*(subdivlevel+1)*(subdivlevel+1)*(numOfSpheres+6));
	basicFont->renderFont(buffer.c_str(), vec2(-490.0f, -340.0f), vec3(1.0f, 1.0f, 1.0f), 0.65f);
	buffer = "Normal Calculation          " + to_string((subdivlevel+1)*(subdivlevel+1)*12 + (subdivlevel+1)*(subdivlevel+1)*(subdivlevel+1)*6);
	basicFont->renderFont(buffer.c_str(), vec2(-490.0f, -405.0f), vec3(1.0f, 1.0f, 1.0f), 0.65f);
	buffer = "___________________________________";
	basicFont->renderFont(buffer.c_str(), vec2(-490.0f, -420.0f), vec3(1.0f, 1.0f, 1.0f), 0.65f);
	buffer = "Total Calculations          " + to_string(((subdivlevel+1)*(subdivlevel+1)*(subdivlevel+1)*40) + ((subdivlevel+1)*(subdivlevel+1)*(subdivlevel+1)*(numOfSpheres+6)) + ((subdivlevel+1)*(subdivlevel+1)*12 + (subdivlevel+1)*(subdivlevel+1)*(subdivlevel+1)*6));
	basicFont->renderFont(buffer.c_str(), vec2(-490.0f, -470.0f), vec3(1.0f, 1.0f, 1.0f), 0.65f);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderMainScreenDemoMode() {
	if(isStatEnabled) {
		renderStatsFont();
	}
	glViewport(0, 0, winSize.w, winSize.h);
	if(!isPaused) {
		camangle += 0.002f;

		updateSoftBodyPosition();
		calculateNormalVectors();
		clFinish(clcontext.cmdQueue);
	}
	glUseProgram(basicRender.programObject);
	center = *(float*)glMapNamedBufferRange(pointsVbo, sizeof(float), sizeof(float), GL_MAP_READ_BIT) + 2.0f;
	glUniformMatrix4fv(0, 1, GL_FALSE, perspective(45.0f, (float)winSize.w / winSize.h, 0.1f, 1000.0f));
	glUniformMatrix4fv(1, 1, GL_FALSE, lookat(vec3(30.0f * sin(camangle), center, 30.0f * cos(camangle)), vec3(0.0f, center, 0.0f), vec3(0.0f, 1.0f, 0.0f)));
	glUnmapNamedBuffer(pointsVbo);

	renderSoftBody();
	
	renderLightSources();
	renderOutsideBox();
	renderBoxLegs();
	renderObstacleSpheres();

	renderGlassBox();

	mat4 proj = ortho(-10.0f * (winSize.w / winSize.h), 10.0f * (winSize.w / winSize.h), -10.0f, 10.0f, -10.0f, 10.0f);
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	renderPlatformSelector(proj);
	renderSettingsChanger(proj);
	renderInfoBox(proj);
	renderInfoPanel(proj);
	renderStats(proj, false);
	glDisable(GL_BLEND);
	glFinish();
}

void renderMainScreenBenchmark() {
	sumfps += cfps;
	frameCount++;
	renderStatsFont();
	glViewport(0, 0, winSize.w, winSize.h);
	
	updateSoftBodyPosition();
	calculateNormalVectors();
	clFinish(clcontext.cmdQueue);

	glUseProgram(basicRender.programObject);
	glUniformMatrix4fv(0, 1, GL_FALSE, perspective(45.0f, (float)winSize.w / winSize.h, 0.1f, 1000.0f));
	glUniformMatrix4fv(1, 1, GL_FALSE, lookat(vec3(0.0f, 0.0f, 40.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)));

	renderSoftBody();
	renderLightSources();
	renderOutsideBox();
	renderBoxLegs();
	renderObstacleSpheres();
	renderGlassBox();

	// mat4 proj = ortho(-10.0f * (winSize.w / winSize.h), 10.0f * (winSize.w / winSize.h), -10.0f, 10.0f, -10.0f, 10.0f);
	
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// glEnable(GL_BLEND);
	// renderStats(proj, true);
	// glDisable(GL_BLEND);

}

void renderMainScreen() {
	if(mode == Demo) {
		renderMainScreenDemoMode();
	} else if(mode == Benchmark) {
		renderMainScreenBenchmark();
	}
}

void renderTitleScreen() {
	glViewport(0, 0, winSize.w, winSize.h);
	fsquadRender(&fsquad, NULL, titleTex);
	mat4 proj = ortho(-10.0f * (winSize.w / winSize.h), 10.0f * (winSize.w / winSize.h), -10.0f, 10.0f, -10.0f, 10.0f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	renderRoundedQuad(beautyRender, proj * translate(5.0f, -3.0f, 0.0f) * scale(4.0f, 2.0f, 1.0f), vec4(darkgray), 0, true);
	renderRoundedQuad(beautyRender, proj * translate(5.0f, -3.0f, 0.0f) * scale(4.0f, 2.0f, 1.0f), vec4(1.0f), benchmarkTex, true);
	calculateBoundingBox(benchmarkBoundingBox, proj * translate(5.0f, -3.0f, 0.0f) * scale(4.0f, 2.0f, 1.0f), vec4(roundedQuadSize, roundedQuadSize, 0.0f, 1.0f), vec4(-roundedQuadSize, -roundedQuadSize, 0.0f, 1.0f));
	renderRoundedQuad(beautyRender, proj * translate(-5.0f, -3.0f, 0.0f) * scale(4.0f, 2.0f, 1.0f), vec4(darkgray), 0, true);
	renderRoundedQuad(beautyRender, proj * translate(-5.0f, -3.0f, 0.0f) * scale(4.0f, 2.0f, 1.0f), vec4(1.0f), demoTex, true);
	calculateBoundingBox(playBoundingBox, proj * translate(-5.0f, -3.0f, 0.0f) * scale(4.0f, 2.0f, 1.0f), vec4(roundedQuadSize, roundedQuadSize, 0.0f, 1.0f), vec4(-roundedQuadSize, -roundedQuadSize, 0.0f, 1.0f));
	glDisable(GL_BLEND);
}

void renderResults() {
	glViewport(0, 0, winSize.w, winSize.h);
	vec4 currentColor = darkgray;
	mat4 proj = ortho(-10.0f * (winSize.w / winSize.h), 10.0f * (winSize.w / winSize.h), -10.0f, 10.0f, -10.0f, 10.0f);
	GLfloat transconst = -10.0f * (winSize.w / winSize.h);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	renderRoundedQuad(beautyRender, proj * translate(transconst + 4.0f, 4.5f, 0.0f) * scale(1.75f), currentColor, 0, true);
	renderRoundedQuad(beautyRender, proj * translate(transconst + 4.0f, 4.5f, 0.0f) * scale(1.85f), currentColor, cpuTex, false);

	renderRoundedQuad(beautyRender, proj * translate(transconst + 4.0f, 0.5f, 0.0f) * scale(1.75f), currentColor, 0, true);
	renderRoundedQuad(beautyRender, proj * translate(transconst + 4.0f, 0.5f, 0.0f) * scale(1.85f), currentColor, vertTex, false);

	renderRoundedQuad(beautyRender, proj * translate(transconst + 4.0f, -3.5f, 0.0f) * scale(1.75f), currentColor, 0, true);
	renderRoundedQuad(beautyRender, proj * translate(transconst + 4.0f, -3.5f, 0.0f) * scale(1.85f), currentColor, compTex, false);
	
	renderRoundedQuad(beautyRender, proj * translate(transconst + 4.0f, -7.5f, 0.0f) * scale(1.75f), currentColor, 0, true);
	renderRoundedQuad(beautyRender, proj * translate(transconst + 4.0f, -7.5f, 0.0f) * scale(1.85f), currentColor, oclTex, false);

	GLfloat scaleX;
	scaleX = avgFps[0] / 2000.0f * 9.0f;
	renderRoundedQuad(beautyRender, proj * translate(transconst + 8.0f + 7.5f - (9.0f - scaleX), 4.5f, 0.0f) * scale(scaleX, 1.5f, 1.0f), red, 0, true);
	// renderRoundedQuad(beautyRender, proj * translate(transconst + 8.0f + 7.5f - (9.0f - scaleX), 4.5f, 0.0f) * scale(scaleX, 1.6f, 1.0f), currentColor, oclTex, false);

	scaleX = avgFps[1] / 2000.0f * 9.0f;
	renderRoundedQuad(beautyRender, proj * translate(transconst + 8.0f + 7.5f - (9.0f - scaleX), 0.5f, 0.0f) * scale(scaleX, 1.5f, 1.0f), blue, 0, true);
	// renderRoundedQuad(beautyRender, proj * translate(transconst + 8.0f + 7.5f - (9.0f - scaleX), 0.5f, 0.0f) * scale(scaleX, 1.6f, 1.0f), currentColor, oclTex, false);

	scaleX = avgFps[2] / 2000.0f * 9.0f;
	renderRoundedQuad(beautyRender, proj * translate(transconst + 8.0f + 7.5f - (9.0f - scaleX), -3.5f, 0.0f) * scale(scaleX, 1.5f, 1.0f), blue + 0.2f, 0, true);
	// renderRoundedQuad(beautyRender, proj * translate(transconst + 8.0f + 7.5f - (9.0f - scaleX), -3.5f, 0.0f) * scale(scaleX, 1.6f, 1.0f), currentColor, oclTex, false);

	scaleX = avgFps[3] / 2000.0f * 9.0f;
	renderRoundedQuad(beautyRender, proj * translate(transconst + 8.0f + 7.5f - (9.0f - scaleX), -7.5f, 0.0f) * scale(scaleX, 1.5f, 1.0f), green, 0, true);
	// renderRoundedQuad(beautyRender, proj * translate(transconst + 8.0f + 7.5f - (9.0f - scaleX), -7.5f, 0.0f) * scale(scaleX, 1.6f, 1.0f), currentColor, oclTex, false);

	renderRoundedQuad(beautyRender, proj * translate(0.0f, 8.0f, 0.0f) * scale(5.0f), vec4(1.0f), avgfpsTex, false);

	glDisable(GL_BLEND);
}

void render(void) {
	glClearBufferfv(GL_COLOR, 0, vec4(0.0f, 0.0f, 0.0f, 1.0f));
	glClearBufferfv(GL_DEPTH, 0, vec1(1.0f));	
	
	if(currentScene == Title) {
		renderTitleScreen();
	} else if(currentScene == Main) {
		renderMainScreen();
	} else if(currentScene == Result) {
		renderResults();
	}
}

void uninit(void) {
}

bool checkBoundingBox(vec4 boundBox, vec2 point) {
	return (point[0] < boundBox[0] && point[0] > boundBox[2] && point[1] < boundBox[1] && point[1] > boundBox[3]);
}

void mousemotion(int x, int y) {
	if(isMenuVisible) {
		vec2 eye = vec2((x / winSize.w * 2.0f - 1.0f), (-y / winSize.h * 2.0f + 1.0f));
		if(checkBoundingBox(cpuBoundingBox, eye)) {
			hoveredPlatform = 1;
		} else if(checkBoundingBox(vertBoundingBox, eye)) {
			hoveredPlatform = 2;
		} else if(checkBoundingBox(compBoundingBox, eye)) {
			hoveredPlatform = 3;
		} else if(checkBoundingBox(oclBoundingBox, eye)) {
			hoveredPlatform = 4;
		} else {
			hoveredPlatform = 0;
		}
	} else if(isSettingsVisble) {
		vec2 eye = vec2((x / winSize.w * 2.0f - 1.0f), (-y / winSize.h * 2.0f + 1.0f));
		if(checkBoundingBox(randomGenBox, eye)) {
			hoveredSettings = 1;
		} else if(checkBoundingBox(sphereLodPlusBox, eye)) {
			hoveredSettings = 2;
		} else if(checkBoundingBox(sphereLodMinuxBox, eye)) {
			hoveredSettings = 3;
		} else if(checkBoundingBox(sphereCountPlusBox, eye)) {
			hoveredSettings = 4;
		} else if(checkBoundingBox(sphereCountMinuxBox, eye)) {
			hoveredSettings = 5;
		} else if(checkBoundingBox(softbodyLodPlusBox, eye)) {
			hoveredSettings = 6;
		} else if(checkBoundingBox(softbodyLodMinuxBox, eye)) {
			hoveredSettings = 7;
		} else if(checkBoundingBox(sphereWireframeBox, eye)) {
			hoveredSettings = 8;
		} else if(checkBoundingBox(softbodyWireframeBox, eye)) {
			hoveredSettings = 9;
		} else if(checkBoundingBox(statBox, eye)) {
			hoveredSettings = 10;
		} else if(checkBoundingBox(resetBox, eye)) {
			hoveredSettings = 11;
		} else {
			hoveredSettings = 0;
		}
	} else if(isInfoVisble) {
		vec2 eye = vec2((x / winSize.w * 2.0f - 1.0f), (-y / winSize.h * 2.0f + 1.0f));
		if(checkBoundingBox(aboutBoundingBox, eye)) {
			hoveredInfo = 1;
		} else if(checkBoundingBox(effectsBoundingBox, eye)) {
			hoveredInfo = 2;
		} else if(checkBoundingBox(sdkBoundingBox, eye)) {
			hoveredInfo = 3;
		} else if(checkBoundingBox(referBoundingBox, eye)) {
			hoveredInfo = 4;
		} else {
			hoveredInfo = 0;
		}
	} else {
		vec2 eye = vec2((x / winSize.w * 2.0f - 1.0f), (-y / winSize.h * 2.0f + 1.0f));
		if(checkBoundingBox(infoBox, eye)) {
			isHoverInfo = true;
		} else {
			isHoverInfo = false;
		}
	}
}

void mouse(int x, int y, int state, unsigned button) {
	if(button == Button1) {
		vec2 eye = vec2((x / winSize.w * 2.0f - 1.0f), (-y / winSize.h * 2.0f + 1.0f));
		if(currentScene == Main) {
			if(checkBoundingBox(bottomArrowBoundingBox, eye)) {
				isMenuVisible = !isMenuVisible;
				if(isMenuVisible) {
					isSettingsVisble = false;
					isInfoVisble = false;
				}
				isPaused = (isMenuVisible || isSettingsVisble || isInfoVisble);
			} else if(checkBoundingBox(leftArrowBoundingBox, eye)) { 
				isSettingsVisble = !isSettingsVisble;
				if(isSettingsVisble) {
					isMenuVisible = false;
					isInfoVisble = false;
				}
				isPaused = (isMenuVisible || isSettingsVisble || isInfoVisble);
			} else if(checkBoundingBox(infoBox, eye)) {
				isInfoVisble = !isInfoVisble;
				if(isInfoVisble) {
					isMenuVisible = false;
					isSettingsVisble = false;
				}
				isPaused = (isMenuVisible || isSettingsVisble || isInfoVisble);
			} else {
				if(isMenuVisible) {
					if(checkBoundingBox(cpuBoundingBox, eye)) {
						runningOn = CPU;
						isMenuVisible = !isMenuVisible;
						if(isMenuVisible) {
							isSettingsVisble = false;
							isInfoVisble = false;
						}
						isPaused = (isMenuVisible || isSettingsVisble || isInfoVisble);
					} else if(checkBoundingBox(vertBoundingBox, eye)) {
						runningOn = Vertex;
						isMenuVisible = !isMenuVisible;
						if(isMenuVisible) {
							isSettingsVisble = false;
							isInfoVisble = false;
						}
						isPaused = (isMenuVisible || isSettingsVisble || isInfoVisble);
					} else if(checkBoundingBox(compBoundingBox, eye)) {
						runningOn = Compute;
						isMenuVisible = !isMenuVisible;
						if(isMenuVisible) {
							isSettingsVisble = false;
							isInfoVisble = false;
						}
						isPaused = (isMenuVisible || isSettingsVisble || isInfoVisble);
					} else if(checkBoundingBox(oclBoundingBox, eye)) {
						runningOn = OpenCL;
						isMenuVisible = !isMenuVisible;
						if(isMenuVisible) {
							isSettingsVisble = false;
							isInfoVisble = false;
						}
						isPaused = (isMenuVisible || isSettingsVisble || isInfoVisble);
					}
				} else if(isSettingsVisble) {
					if(checkBoundingBox(randomGenBox, eye)) {
						seed = rand();
						initSoftBody();
						randomizeSpherePos();
					} else if(checkBoundingBox(sphereLodPlusBox, eye)) {
						if(numStacks < 100) {
							numStacks += 5;
							numSlices += 5;
						}
						destorySphereBuffers();
						createSphereBuffers();
						initSphere();
					} else if(checkBoundingBox(sphereLodMinuxBox, eye)) {
						if(numStacks > 5) {
							numStacks -= 5;
							numSlices -= 5;
						}
						destorySphereBuffers();
						createSphereBuffers();
						initSphere();
					} else if(checkBoundingBox(sphereCountPlusBox, eye)) {
						if(numOfSpheres < 30) {
							numOfSpheres++;
						}
						destorySphereBuffersPos();
						createSphereBuffersPos();
						randomizeSpherePos();
						initSoftBody();
					} else if(checkBoundingBox(sphereCountMinuxBox, eye)) {
						if(numOfSpheres > 1) {
							numOfSpheres--;
						}
						destorySphereBuffersPos();
						createSphereBuffersPos();
						randomizeSpherePos();
						initSoftBody();
					} else if(checkBoundingBox(softbodyLodPlusBox, eye)) {
						if(subdivlevel < 17) {
							subdivlevel++;
						}
						destroySoftBodyBuffers();
						createSoftBodyBuffers();
						initSoftBody();
					} else if(checkBoundingBox(softbodyLodMinuxBox, eye)) {
						if(subdivlevel > 2) {
							subdivlevel--;
						}
						destroySoftBodyBuffers();
						createSoftBodyBuffers();
						initSoftBody();
					} else if(checkBoundingBox(sphereWireframeBox, eye)) {
						isSphereWireframeEnabled = ! isSphereWireframeEnabled;
					} else if(checkBoundingBox(softbodyWireframeBox, eye)) {
						isSoftbodyWireframeEnabled = ! isSoftbodyWireframeEnabled;
					} else if(checkBoundingBox(statBox, eye)) {
						isStatEnabled = ! isStatEnabled;
					} else if(checkBoundingBox(resetBox, eye)) {
						initSoftBody();
						initSphere();
					}	
				} else if(isInfoVisble) {
					if(checkBoundingBox(aboutBoundingBox, eye)) {
						selectedInfo = 1;
					} else if(checkBoundingBox(effectsBoundingBox, eye)) {
						selectedInfo = 2;
					} else if(checkBoundingBox(sdkBoundingBox, eye)) {
						selectedInfo = 3;
					} else if(checkBoundingBox(referBoundingBox, eye)) {
						selectedInfo = 4;
					}
				}
			}
		} else if(currentScene == Title) {
			if(checkBoundingBox(playBoundingBox, eye)) {
				currentScene = Main;
				mode = Demo;
				initDemoMode();
			} else if(checkBoundingBox(benchmarkBoundingBox, eye)) {
				currentScene = Main;
				mode = Benchmark;
				initBenchmark();
			}
		}
	}
}

void keyboard(unsigned int key, int state) {
	switch(key) {
	case XK_F: case XK_f:
		toggleFullscreen();
		break;
	case XK_Escape:
		closeOpenGLWindow();
		break;
	}
}

int main(int argc, char** argv) {
	setKeyboardFunc(keyboard);
	setMouseFunc(mouse);
	setMouseMotionFunc(mousemotion);
	createOpenGLWindow();
	init();
	if(!alutInit(&argc, argv)) {
		ALenum error = alutGetError();
    	printf("%s\n", alutGetErrorString(error));
    	exit(1);
    }

	buffer = alutCreateBufferFromFile("music/main.wav");
	if(buffer == AL_NONE) {
		error = alutGetError ();
		printf("Error loading file: %x '%s'\n", error, alutGetErrorString (error));
		alutExit();
		exit(EXIT_FAILURE);
	}

	alGenSources(1, &source);
	if((error = alGetError()) != AL_NO_ERROR) {
		printf("alGenSources: %d", error);
		alDeleteBuffers(1, &buffer);
		exit(1);
	}

	alSourcei(source, AL_BUFFER, buffer);
	if((error = alGetError()) != AL_NO_ERROR) {
		printf("alSource: %d", error);
		alDeleteBuffers(1, &buffer);
		alDeleteSources(1, &source);
		exit(1);
	}
	
	alSourcePlay(source);
	if((error = alGetError()) != AL_NO_ERROR) {
		printf("alSourcePlay: %d", error);
		alDeleteBuffers(1, &buffer);
		alDeleteSources(1, &source);
		exit(1);
	}
	timeval start;
	timeval current;
	int c = 0;
	gettimeofday(&start, 0);
	while(!isOpenGLWindowClosed()) {
		processEvents();
		if(mode == Demo) {
			renderWithFPSLocked(render, false, true);
		} else if(mode == Benchmark) {
			gettimeofday(&current, 0);
			if(current.tv_sec - start.tv_sec == 10) {
				avgFps[c++] = sumfps / frameCount;
				sumfps = 0.0f;
				frameCount = 0;
				if(runningOn == CPU) {
					runningOn = Vertex;
				} else if(runningOn == Vertex) {
					runningOn = Compute;
				} else if(runningOn == Compute) {
					runningOn = OpenCL;
				} else if(runningOn == OpenCL) {
					currentScene = Result;
				}
				initSoftBody();
				gettimeofday(&start, 0);
			}
			renderWithFPSLocked(render, false, false);
		}
		swapBuffers();
	}
	uninit();
	destroyOpenGLWindow();
}