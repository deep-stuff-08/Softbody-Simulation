#ifndef __SHADER_LOADER__
#define __SHADER_LOADER__

#include<GL/glew.h>
#include<GL/gl.h>
#include<iostream>
#include<vector>

struct glprogram_dl {
	GLuint programObject;
};

struct glshader_dl {
	GLuint shaderObject;
};

std::string glshaderCreate(glshader_dl *shader, GLenum shaderType, std::string shaderFileName);
void glshaderDestroy(glshader_dl *shader);

std::string glprogramCreate(glprogram_dl *program, std::string programName, std::vector<glshader_dl> shaderList);
void glprogramDestory(glprogram_dl *program);

#endif