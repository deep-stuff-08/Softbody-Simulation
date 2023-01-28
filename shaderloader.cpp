#include<iostream>
#include<fstream>
#include<sstream>
#include<vector>
#include<GL/glew.h>
#include"include/shaderloader.h"

std::string glshaderCreate(glshader_dl *shader, GLenum shaderType, std::string shaderFileName) {
	std::ifstream file(shaderFileName, std::ifstream::in);
	if(!file.is_open()) {
		return shaderFileName + ": file not found\n";
	}
	shader->shaderObject = glCreateShader(shaderType);
	std::string filestr(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));
	const char* source = filestr.c_str();
	int flen = filestr.length();
	glShaderSource(shader->shaderObject, 1, &source, &flen);
	glCompileShader(shader->shaderObject);
	GLint compiledStatus;
	glGetShaderiv(shader->shaderObject, GL_COMPILE_STATUS, &compiledStatus);
	if(!compiledStatus) {
		char buffer[1024];
		glGetShaderInfoLog(shader->shaderObject, 1024, NULL, buffer);
		std::string s(buffer);
		return shaderFileName + ": compilation failed.\n" + s + "\n";
	}
	return "";
}

void glshaderDestroy(glshader_dl *shader) {
	glDeleteShader(shader->shaderObject);
}

std::string glprogramCreate(glprogram_dl *program, std::string programName, std::vector<glshader_dl> shaderList) {
	program->programObject = glCreateProgram();
	
	if(shaderList.empty()) {
		return programName + ": No shaders passed\n";
	}
	for(int i = 0; i < shaderList.size(); i++) {
		glAttachShader(program->programObject, shaderList[i].shaderObject);
	}
	glLinkProgram(program->programObject);
	GLint linkedStatus;
	glGetProgramiv(program->programObject, GL_LINK_STATUS, &linkedStatus);
	if(!linkedStatus) {
		char buffer[1024];
		glGetProgramInfoLog(program->programObject, 1024, NULL, buffer);
		std::string s(buffer);
		return programName + ": linking failed.\n" + s + "\n";
	}
	for(int i = 0; i < shaderList.size(); i++) {
		glDetachShader(program->programObject, shaderList[i].shaderObject);
	}
	return "";
}

void glprogramDestory(glprogram_dl *program) {
	glDeleteProgram(program->programObject);
}