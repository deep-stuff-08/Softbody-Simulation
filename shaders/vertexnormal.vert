#version 460 core

layout(binding = 0, std430)buffer face {
	int faceList[];
};

layout(binding = 1, std430)buffer nors {
    vec4 nor[];
};

layout(binding = 2, std430)buffer fnors {
	vec4 facenormals[];
};

void main() {
	int i = gl_VertexID;
	nor[i] = vec4(0.0);
	for(int j = 0; j < 6; j++) {
		int f = faceList[i * 6 + j];
		if(f != -1) {
			nor[i] += facenormals[f];
		}
	}
}
