#version 460 core

layout(location = 0)in vec4 vPos;
layout(location = 1)in vec2 vTex;

layout(location = 0)uniform mat4 mvpMat;

out VS_OUT {
	vec2 tex;
} vs_out;

void main(void) {
	vs_out.tex = vTex;
	gl_Position = mvpMat * vPos;
}