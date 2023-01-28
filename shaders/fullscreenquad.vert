#version 460 core

vec2 vertices[] = vec2[](
	vec2(-1.0, -1.0),
	vec2(-1.0, 1.0),
	vec2(1.0, -1.0),
	vec2(1.0, 1.0)
);

out VS_OUT {
	vec2 texCoords;
} vs_out;

void main(void) {
	vs_out.texCoords = (vertices[gl_VertexID] + 1.0) / 2.0;
	gl_Position = vec4(vertices[gl_VertexID], 0.0, 1.0);
}