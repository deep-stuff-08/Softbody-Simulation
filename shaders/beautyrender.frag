#version 460 core

layout(location = 0)out vec4 color;
layout(location = 1)uniform vec4 acolor;
layout(location = 2)uniform bool isTexture;
layout(binding = 0)uniform sampler2D texsam;

in VS_OUT {
	vec2 tex;
} fs_in;

void main(void) {
    color = acolor;
    if(isTexture) {
        vec4 a = texture(texsam, fs_in.tex);
        color *= a;
    }
}