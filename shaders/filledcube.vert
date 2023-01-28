#version 460 core

layout(binding = 0, std430)buffer curr {
	vec4 cposMass[];
};

layout(binding = 1, std430)buffer prev {
	vec4 pposMass[];
};

layout(location = 0)uniform float size2;
layout(location = 1)uniform float restlen;
layout(location = 2)uniform int subdivlevel;

void main() {
	int i = gl_VertexID % subdivlevel;
	int j = gl_VertexID / subdivlevel % subdivlevel;
	int k = gl_VertexID / (subdivlevel * subdivlevel);
	cposMass[gl_VertexID] = vec4(-size2 + k * restlen, -size2 + j * restlen, -size2 + i * restlen, 1.0);
	pposMass[gl_VertexID] = vec4(-size2 + k * restlen, -size2 + j * restlen, -size2 + i * restlen, 1.0);
}
