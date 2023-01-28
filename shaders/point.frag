#version 460 core

in VS_OUT {
	vec3 N;
	vec3 L[4];
	vec3 V;
	flat int instanceID;
} fs_in;

layout(location = 0)out vec4 color;
layout(location = 2)uniform vec4 acolor;
layout(location = 3)uniform int type;

uniform vec3 matcolors[5] = vec3[5](
	vec3(0.8862, 0.4901, 0.3764),
	vec3(0.5215, 0.8039, 0.7921),
	vec3(0.9092, 0.6588, 0.4862),
	vec3(0.7647, 0.5529, 0.6156),
	vec3(0.2509, 0.7019, 0.6352)
);

uniform vec3 lightColor = vec3(0.8, 0.8, 0.75);
// uniform vec3 lightColor = vec3(0.7, 0.6, 0.5);
uniform vec3 specularColor = vec3(0.2, 0.2, 0.15);
// uniform vec3 specularColor = vec3(0.7, 0.5, 0.3);

void main(void) {
	if(type == 1 || type == 2) {
		vec3 lcolor = vec3(0.0);
		vec3 N = normalize(fs_in.N);
		vec3 V = normalize(fs_in.V);
		int instance = fs_in.instanceID % 5;

		for(int i = 0; i < 4; i++) {
			vec3 L = normalize(fs_in.L[i]);
			vec3 R = reflect(-L, N);
			vec3 diffuse = vec3(0.0);
			if(type == 2) { 
				diffuse = matcolors[instance] * lightColor * max(dot(N, L), 0.0);
			} else {
				diffuse = acolor.rgb * lightColor * max(dot(N, L), 0.0);
			}
			vec3 specular = specularColor * pow(max(dot(R, V), 0.0), 10.0);
			lcolor += diffuse + specular;
		}
		vec3 ambient = vec3(0.05, 0.04, 0.02);
		// lcolor += ambient;
		color = vec4(lcolor, acolor.a);
	} else if(type == 0) {
		color = acolor;
	}
}