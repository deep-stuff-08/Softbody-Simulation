#include"include/beautygui.h"
#include<vector>
#include<GL/glew.h>
#include<GL/gl.h>
#include"include/vmath.h"
#include"include/windowing.h"

using namespace std;
using namespace vmath;

GLuint topArrowVao;
GLuint roundedQuadVao;

GLfloat orthoParam = 10.0f;

vector<vec4> topArrow;
vector<vec4> roundedQuad;
vector<vec2> roundedQuadTexCoords;

void initTopArrow() {
    float x = topArrowX;
    float y = topArrowY;
    float thickness = 0.15f;
    topArrow.push_back(vec4(-x + thickness, y, 0.0f, 1.0f));
    topArrow.push_back(vec4(-x, y - thickness, 0.0f, 1.0f));
    
    topArrow.push_back(vec4(0.0f, -y + (thickness * 2.0f), 0.0f, 1.0f));
    topArrow.push_back(vec4(0.0f, -y, 0.0f, 1.0f));
    
    topArrow.push_back(vec4(x - thickness, y, 0.0f, 1.0f));
    topArrow.push_back(vec4(x, y - thickness, 0.0f, 1.0f));

    GLuint vbo;
    glGenVertexArrays(1, &topArrowVao);
    glBindVertexArray(topArrowVao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, topArrow.size() * sizeof(vec4), topArrow.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
}

void initRoundedQuad() {
    float radius = 0.1f;
    float side = roundedQuadSize - radius;

    roundedQuad.push_back(vec4(side, side, 0.0f, 1.0f));
    roundedQuadTexCoords.push_back(vec2(1.0f, 1.0f));
    roundedQuad.push_back(vec4(-side, side, 0.0f, 1.0f));
    roundedQuadTexCoords.push_back(vec2(0.0f, 1.0f));
    roundedQuad.push_back(vec4(-side, -side, 0.0f, 1.0f));
    roundedQuadTexCoords.push_back(vec2(0.0f, 0.0f));
    roundedQuad.push_back(vec4(side, -side, 0.0f, 1.0f));
    roundedQuadTexCoords.push_back(vec2(1.0f, 0.0f));
    
    roundedQuad.push_back(vec4(side, side, 0.0f, 1.0f));
    roundedQuadTexCoords.push_back(vec2(1.0f, 1.0f));
    for(int i = 0; i < 26; i++) {
        GLfloat theta = 2.0f * M_PI * ((float)i / 100);
        roundedQuad.push_back(vec4(radius * sin(theta) + side, radius * cos(theta) + side, 0.0f, 1.0f));
        roundedQuadTexCoords.push_back(vec2(1.0f, 1.0f));
    }
    roundedQuad.push_back(vec4(side, -side, 0.0f, 1.0f));
    roundedQuadTexCoords.push_back(vec2(1.0f, 0.0f));
    for(int i = 0; i < 26; i++) {
        GLfloat theta = 2.0f * M_PI * ((float)i / 100) + (M_PI / 2.0f);
        roundedQuad.push_back(vec4(radius * sin(theta) + side, radius * cos(theta) - side, 0.0f, 1.0f));
        roundedQuadTexCoords.push_back(vec2(1.0f, 0.0f));
    }
    roundedQuad.push_back(vec4(-side, -side, 0.0f, 1.0f));
    roundedQuadTexCoords.push_back(vec2(0.0f, 0.0f));
    for(int i = 0; i < 26; i++) {
        GLfloat theta = 2.0f * M_PI * ((float)i / 100) + M_PI;
        roundedQuad.push_back(vec4(radius * sin(theta) - side, radius * cos(theta) - side, 0.0f, 1.0f));
        roundedQuadTexCoords.push_back(vec2(0.0f, 0.0f));
    }
    roundedQuad.push_back(vec4(-side, side, 0.0f, 1.0f));
    roundedQuadTexCoords.push_back(vec2(0.0f, 1.0f));
    for(int i = 0; i < 26; i++) {
        GLfloat theta = 2.0f * M_PI * ((float)i / 100) + 3.0f / 2.0f * M_PI;
        roundedQuad.push_back(vec4(radius * sin(theta) - side, radius * cos(theta) + side, 0.0f, 1.0f));
        roundedQuadTexCoords.push_back(vec2(0.0f, 1.0f));
    }
    roundedQuad.push_back(vec4(side + radius, side, 0.0f, 1.0f));
    roundedQuadTexCoords.push_back(vec2(1.0f, 1.0f));
    roundedQuad.push_back(vec4(side, side, 0.0f, 1.0f));
    roundedQuadTexCoords.push_back(vec2(1.0f, 1.0f));
    roundedQuad.push_back(vec4(side, -side, 0.0f, 1.0f));
    roundedQuadTexCoords.push_back(vec2(1.0f, 0.0f));
    roundedQuad.push_back(vec4(side + radius, -side, 0.0f, 1.0f));
    roundedQuadTexCoords.push_back(vec2(1.0f, 0.0f));
    
    roundedQuad.push_back(vec4(-side, side, 0.0f, 1.0f));
    roundedQuadTexCoords.push_back(vec2(0.0f, 1.0f));
    roundedQuad.push_back(vec4(-side - radius, side, 0.0f, 1.0f));
    roundedQuadTexCoords.push_back(vec2(0.0f, 1.0f));
    roundedQuad.push_back(vec4(-side - radius, -side, 0.0f, 1.0f));
    roundedQuadTexCoords.push_back(vec2(0.0f, 0.0f));
    roundedQuad.push_back(vec4(-side, -side, 0.0f, 1.0f));
    roundedQuadTexCoords.push_back(vec2(0.0f, 0.0f));
    
    roundedQuad.push_back(vec4(side, side + radius, 0.0f, 1.0f));
    roundedQuadTexCoords.push_back(vec2(1.0f, 1.0f));
    roundedQuad.push_back(vec4(-side, side + radius, 0.0f, 1.0f));
    roundedQuadTexCoords.push_back(vec2(0.0f, 1.0f));
    roundedQuad.push_back(vec4(-side, side, 0.0f, 1.0f));
    roundedQuadTexCoords.push_back(vec2(0.0f, 1.0f));
    roundedQuad.push_back(vec4(side, side, 0.0f, 1.0f));
    roundedQuadTexCoords.push_back(vec2(1.0f, 1.0f));
    
    roundedQuad.push_back(vec4(side, -side, 0.0f, 1.0f));
    roundedQuadTexCoords.push_back(vec2(1.0f, 0.0f));
    roundedQuad.push_back(vec4(-side, -side, 0.0f, 1.0f));
    roundedQuadTexCoords.push_back(vec2(0.0f, 0.0f));
    roundedQuad.push_back(vec4(-side, -side - radius, 0.0f, 1.0f));
    roundedQuadTexCoords.push_back(vec2(0.0f, 0.0f));
    roundedQuad.push_back(vec4(side, -side - radius, 0.0f, 1.0f));
    roundedQuadTexCoords.push_back(vec2(1.0f, 0.0f));
    
    GLuint vbo[2];
    glGenVertexArrays(1, &roundedQuadVao);
    glBindVertexArray(roundedQuadVao);
    glGenBuffers(2, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, roundedQuad.size() * sizeof(vec4), roundedQuad.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, roundedQuadTexCoords.size() * sizeof(vec2), roundedQuadTexCoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
}
void renderTopArrow(glprogram_dl program, mat4 mvp, vec4 color) {
    glUseProgram(program.programObject);
    glUniformMatrix4fv(0, 1, GL_FALSE, mvp);
    glUniform4fv(1, 1, color);
    glUniform1i(2, GL_FALSE);
    glBindVertexArray(topArrowVao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, topArrow.size());
}

void renderRoundedQuad(glprogram_dl program, mat4 mvp, vec4 color, GLuint texture, bool renderEdges) {
    glUseProgram(program.programObject);
    glUniformMatrix4fv(0, 1, GL_FALSE, mvp);
    glUniform4fv(1, 1, color);
    if(texture != 0) {
        glUniform1i(2, GL_TRUE);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
    } else {
        glUniform1i(2, GL_FALSE);
    }
    glBindVertexArray(roundedQuadVao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    if(renderEdges) {
        glDrawArrays(GL_TRIANGLE_FAN, 4, 27);
        glDrawArrays(GL_TRIANGLE_FAN, 31, 27);
        glDrawArrays(GL_TRIANGLE_FAN, 58, 27);
        glDrawArrays(GL_TRIANGLE_FAN, 85, 27);
        glDrawArrays(GL_TRIANGLE_FAN, 112, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 116, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 120, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 124, 4);
    }
}