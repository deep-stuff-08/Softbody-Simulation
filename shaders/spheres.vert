#version 460 core

#define M_PI 3.14159265358979323846

layout(binding = 0, std430)buffer curr {
	vec4 vert[];
};

layout(location = 0)uniform ivec2 stacks_slices;

void main() {
    int stacks = stacks_slices.x;
    int slices = stacks_slices.y;
	int i = gl_VertexID % stacks;
	int j = gl_VertexID / stacks;
	int id = gl_VertexID;
	float phi = M_PI * (float(i) / (stacks - 1));
	float theta = 2.0f * M_PI * (float(j) / (slices - 1));
	vert[id] = vec4(sin(phi) * sin(theta), cos(phi), sin(phi) * cos(theta), 1.0);
}
