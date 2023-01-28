#version 460 core

#define NoOfSpheres 100

layout(location = 0)in vec4 vPos;
layout(location = 1)in vec3 vNor;

layout(location = 0)uniform mat4 pMat;
layout(location = 1)uniform mat4 vMat;
layout(location = 4)uniform mat4 mMat[NoOfSpheres];

out VS_OUT {
	vec3 N;
	vec3 L[4];
	vec3 V;
	flat int instanceID;
} vs_out;

void main(void) {
	vs_out.instanceID = gl_InstanceID;
	mat4 mvMat = vMat * mMat[gl_InstanceID]; 
	vec4 P = mvMat * vPos;
	vs_out.N = mat3(mvMat) * vNor;
	vec4 lightDir[4] = vec4[4](
		vec4(40.0, 20.0, 40.0, 1.0),
		vec4(40.0, 20.0, -40.0, 1.0),
		vec4(-40.0, 20.0, -40.0, 1.0),
		vec4(-40.0, 20.0, 40.0, 1.0)
	);
	for(int i = 0; i < 4; i++) {
		vs_out.L[i] = ((vMat * lightDir[i]) - P).xyz;
	}
	vs_out.V = -P.xyz;
	gl_Position = pMat * P;
}