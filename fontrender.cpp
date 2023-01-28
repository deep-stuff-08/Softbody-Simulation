#include"include/fontrender.h"
#include<ft2build.h>
#include<freetype/freetype.h>
#include<GL/glew.h>
#include<GL/gl.h>

enum Action {
	compile = 0,
	link = 1
};

void debugProgram(GLuint obj, GLchar message[], Action action) {
	typedef void(*GetStatusProc)(GLuint, GLenum, GLint*);
	typedef void(*GetInfoProc)(GLuint, GLsizei, GLsizei*, GLchar*);
	struct {
		GLenum type;
		GetStatusProc getStatus;
		GetInfoProc getInfo;
	} actionsArr[2] = {
		{ GL_COMPILE_STATUS, glGetShaderiv, glGetShaderInfoLog},
		{ GL_LINK_STATUS, glGetProgramiv, glGetProgramInfoLog}
	};

	GLint status;
	actionsArr[action].getStatus(obj, actionsArr[action].type, &status);
	if(status == GL_FALSE) {
		char buffer[1024];
		actionsArr[action].getInfo(obj, 1024, NULL, buffer);
		printf("%s error: %s\n", message, buffer);
	}
}

FontRender* FontRender::createFontRenderer(const char* filename, const char* symbols, int size) {	
	FontRender* font = new FontRender();
	GLchar const *vertexShaderSource = 
	"#version 450 core\n"\
	"layout(location = 0)in vec4 position;\n"\
	"layout(location = 1)in vec2 texcoord;\n"\
	"layout(location = 1)uniform mat4 mvp;\n"\
	"out vec2 tex;\n"\
	"void main(void) {\n"\
	"gl_Position = mvp * position;\n"\
	"tex = texcoord;\n"\
	"}\n";
	GLchar const *fragmentShaderSource = 
	"#version 450 core\n"\
	"layout(binding = 0) uniform sampler2D sam;\n"\
	"layout(location = 2) uniform vec3 textcolor;\n"\
	"in vec2 tex;\n"\
	"out vec4 color;\n"\
	"void main(void) {\n"\
	"color = vec4(vec3(1.0), texture(sam, tex).r);\n"\
	"}\n";

	GLint compileStatus;
	GLint vertShaderObj = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertShaderObj, 1, &vertexShaderSource, NULL);
	glCompileShader(vertShaderObj);
	debugProgram(vertShaderObj, (char*)"Vertex", compile);

	GLint fragShaderObj = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShaderObj, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragShaderObj);
	debugProgram(fragShaderObj, (char*)"Fragment", compile);

	font->programObj = glCreateProgram();
	glAttachShader(font->programObj, vertShaderObj);
	glAttachShader(font->programObj, fragShaderObj);
	glLinkProgram(font->programObj);
	debugProgram(font->programObj, (char*)"Program", link);
	
	FT_Library ft;
	if(FT_Init_FreeType(&ft)) {
		printf("FT_Init_FreeType Failed\n");
		return NULL;
	}
	FT_Face face;
	if(FT_New_Face(ft, filename, 0, &face)) {
		printf("FT_New_Face Failed\n");
		return NULL;
	}
	if(symbols == NULL) {
		printf("No Symbols\n");
		return NULL;
	}
	FT_Set_Pixel_Sizes(face, 0, size);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	for(unsigned char c = 0; symbols[c] != '\0'; c++) {
		if(FT_Load_Char(face, symbols[c], FT_LOAD_RENDER)) {
			printf("FT_Load_Char Failed\n");
			return NULL;
		}
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
		font->characterList[symbols[c] - 32].TextureID = texture;
		font->characterList[symbols[c] - 32].Size = vmath::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
		font->characterList[symbols[c] - 32].Bearing = vmath::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
		font->characterList[symbols[c] - 32].Advance = face->glyph->advance.x;
		font->characterList[symbols[c] - 32].isValid = true;
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
	glGenVertexArrays(1, &font->vao);
	glGenBuffers(1, &font->vbo);
	glBindVertexArray(font->vao);
	glBindBuffer(GL_ARRAY_BUFFER, font->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	return font;
}

void FontRender::renderFont(const char* string, vmath::vec2 position, vmath::vec3 color, GLfloat scale) {
	glDepthFunc(GL_LEQUAL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(programObj);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(vao);
	glUniformMatrix4fv(1, 1, GL_FALSE, vmath::ortho(-500.0f, 500.0f, -500.0f, 500.0f, -1.0f, 1.0f));
	glUniform3fv(2, 1, color);
	int x = position[0];
	int y = position[1];
	glEnable(GL_BLEND);
	for(int i = 0; i < strlen(string); i++) {
		if(!characterList[string[i] - 32].isValid) {
			printf("Character not found:%c\n",string[i]);
		}
		Character ch = characterList[string[i]-32];
		float xpos = x + ch.Bearing[0] * scale;
		float ypos = y - (ch.Size[1] - ch.Bearing[1]) * scale;

		float w = ch.Size[0] * scale;
		float h = ch.Size[1] * scale;

		float vertices[6][4] = {
			{xpos, ypos + h, 0.0f, 0.0f},
			{xpos, ypos, 0.0f, 1.0f},
			{xpos + w, ypos, 1.0f, 1.0f},
			{xpos, ypos + h, 0.0f, 0.0f},
			{xpos + w, ypos, 1.0f, 1.0f},
			{xpos + w, ypos + h, 1.0f, 0.0f}
		};
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		x += (ch.Advance >> 6) * scale;
	}
	glDisable(GL_BLEND);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}