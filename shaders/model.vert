#version 460 core

layout(binding = 0, std430)buffer curr {
	vec4 cposMass[];
};

layout(binding = 1, std430)buffer prev {
	vec4 pposMass[];
};

layout(location = 0)uniform vec4 translate;

void main(void) {
	int id = gl_VertexID;
	cposMass[id] = cposMass[id] + translate;
	pposMass[id] = pposMass[id] + translate;
}