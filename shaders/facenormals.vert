#version 460 core

layout(binding = 0, std430)buffer curr {
	vec4 pos[];
};

layout(binding = 1, std430)buffer index {
    int indices[][3];
};

layout(binding = 2, std430)buffer nors {
	vec4 facenormals[];
};

void main() {
	int i = gl_VertexID;
	vec4 e1 = pos[indices[i][0]] - pos[indices[i][1]]; 
	vec4 e2 = pos[indices[i][2]] - pos[indices[i][1]];
	facenormals[i] = vec4(cross(e2.xyz, e1.xyz), 0.0);
}
