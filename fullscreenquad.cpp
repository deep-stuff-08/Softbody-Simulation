#include<GL/glew.h>
#include<GL/gl.h>
#include"include/fullscreenquad.h"
#include"include/shaderloader.h"

std::string fsquadInit(fsquad_dl* fsquad, std::string vertexShader, std::string fragmentShader) {
	if(vertexShader.empty()) {
		vertexShader = "shaders/fullscreenquad.vert";
	}
	if(fragmentShader.empty()) {
		fragmentShader = "shaders/fullscreenquad.frag";
	}

	glshader_dl sv, sf;
	std::string err = glshaderCreate(&sv, GL_VERTEX_SHADER, vertexShader);
	if(!err.empty()) {
		return err;
	}
	err = glshaderCreate(&sf, GL_FRAGMENT_SHADER, fragmentShader);
	if(!err.empty()) {
		return err;
	}
	err = glprogramCreate(&fsquad->program, "test", std::vector<glshader_dl>{sv, sf});
	if(!err.empty()) {
		return err;
	}

	glGenVertexArrays(1, &fsquad->vao);
	return "";
}

void fsquadRender(fsquad_dl* fsquad, FSRenderFunc preRenderSetup, GLuint texObj) {
	glUseProgram(fsquad->program.programObject);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texObj);
	glBindVertexArray(fsquad->vao);
	if(preRenderSetup != NULL) {
		preRenderSetup();
	}
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}