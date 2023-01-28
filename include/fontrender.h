#ifndef __FONT_RENDER__
#define __FONT_RENDER__

#include"vmath.h"

struct Character {
	unsigned int TextureID;
	vmath::ivec2 Size;
	vmath::ivec2 Bearing;
	unsigned int Advance;
	bool isValid;
};

class FontRender {
private:
	unsigned vao;
	unsigned vbo;
	int programObj;
	Character characterList[90];
	FontRender() {

	}
public:
	static FontRender* createFontRenderer(const char* fontfilename, const char* symbols, int size);
	void renderFont(const char* string, vmath::vec2 position, vmath::vec3 color, float scale);
};

#endif