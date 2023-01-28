#version 460 core

layout(location = 0) out vec4 color;

layout(binding = 0) uniform sampler2D texsam;

in VS_OUT {
	vec2 texCoords;
} fs_in;

void main(void) {
	color = texture(texsam, fs_in.texCoords);
}