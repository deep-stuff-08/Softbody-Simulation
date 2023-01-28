#version 460 core

layout(binding = 0, std430)buffer curr {
	vec4 cposMass[];
};

layout(binding = 1, std430)buffer prev {
	vec4 pposMass[];
};

layout(binding = 2, std430)buffer sphereData {
	vec4 spheres[];
};

layout(location = 0)uniform float dt;
layout(location = 1)uniform vec3 gravity;
layout(location = 2)uniform float defDamp = -0.11;
layout(location = 3)uniform int sheetDims;
layout(location = 4)uniform float size;
layout(location = 5)uniform int noOfSpheres;
uniform vec3 ks = vec3(5.5, 1.9, 0.6);
uniform vec3 kd = vec3(-0.1, -0.2, -0.3);

void planeCollision(inout vec3 cpos) {
	vec4 box[5] = vec4[5](
		vec4(1.0, 0.0, 0.0, 14.9),
		vec4(-1.0, 0.0, 0.0, 14.9),
		vec4(0.0, 1.0, 0.0, 17.9),
		// vec4(0.0, -1.0, 0.0, 18.0),
		vec4(0.0, 0.0, 1.0, 11.9),
		vec4(0.0, 0.0, -1.0, 11.9)
	);

	float dist;
	for(int i = 0; i < 5; i++) {
		dist = dot(box[i].xyz, cpos) + box[i].w;
		if(dist < 0.0) {
			cpos += box[i].xyz * -dist;
		}
	}
}

void main(void) {
	const ivec4 neighbourArray[40] = ivec4[40](
		//strucure
		ivec4(0, 0, 1, 0),
		ivec4(0, 0, -1, 0),
		ivec4(0, 1, 0, 0),
		ivec4(0, -1, 0, 0),
		ivec4(1, 0, 0, 0),
		ivec4(-1, 0, 0, 0),

		//shear xy plane
		ivec4(0, 1, 1, 1),
		ivec4(0, 1, -1, 1),
		ivec4(0, -1, 1, 1),
		ivec4(0, -1, -1, 1),

		//shear xz plane
		ivec4(1, 0, 1, 1),
		ivec4(1, 0, -1, 1),
		ivec4(-1, 0, 1, 1),
		ivec4(-1, 0, -1, 1),
		
		//shear yz plane
		ivec4(1, 1, 0, 1),
		ivec4(1, -1, 0, 1),
		ivec4(-1, 1, 0, 1),
		ivec4(-1, -1, 0, 1),

		//shear 3d
		ivec4(1, 1, 1, 1),
		ivec4(1, 1, -1, 1),
		ivec4(1, -1, 1, 1),
		ivec4(1, -1, -1, 1),
		ivec4(-1, 1, 1, 1),
		ivec4(-1, 1, -1, 1),
		ivec4(-1, -1, 1, 1),
		ivec4(-1, -1, -1, 1),

		//bending
		ivec4(0, 0, 2, 2),
		ivec4(0, 0, -2, 2),
		ivec4(0, 2, 0, 2),
		ivec4(0, -2, 0, 2),
		ivec4(2, 0, 0, 2),
		ivec4(-2, 0, 0, 2),

		ivec4(2, 2, 2, 2),
		ivec4(2, 2, -2, 2),
		ivec4(2, -2, 2, 2),
		ivec4(2, -2, -2, 2),
		ivec4(-2, 2, 2, 2),
		ivec4(-2, 2, -2, 2),
		ivec4(-2, -2, 2, 2),
		ivec4(-2, -2, -2, 2)
	);

	int id = gl_VertexID;
	float m = 1.0;
	vec3 cpos = cposMass[id].xyz;
	vec3 ppos = pposMass[id].xyz;
	vec3 v = (cpos - ppos) / dt;
	
	// if(id == sheetDims.y - 1 || id == sheetDims.x * sheetDims.y - 1) {
	// 	m = 0.0;
	// }

	vec3 f = gravity * m + (defDamp * v);
	ivec3 currcoords = ivec3(id / (sheetDims * sheetDims), (id / sheetDims) % sheetDims, id % sheetDims);
	for(int i = 0; i < 40; i++) {
		ivec3 ncoord = currcoords + neighbourArray[i].xyz;
		if(ncoord.x < 0 || ncoord.x > (sheetDims-1) || ncoord.y < 0 || ncoord.y > (sheetDims-1) || ncoord.z < 0 || ncoord.z > (sheetDims-1)) {
			continue;
		}
		int nid = (ncoord.x * sheetDims + ncoord.y) * sheetDims + ncoord.z;
		vec3 ncpos = cposMass[nid].xyz;
		vec3 nppos = pposMass[nid].xyz;
		float restLen = length(neighbourArray[i].xyz * (size / (sheetDims - 1)));

		vec3 nv = (ncpos - nppos) / dt;
		vec3 dpos = cpos - ncpos;
		vec3 dv = v - nv;
		float dist = length(dpos);

		vec3 cf = ((-ks[neighbourArray[i].w] * (dist - restLen)) + (kd[neighbourArray[i].w] * (dot(dpos, dv) / dist))) * normalize(dpos);

		// if(dist < restLen * 0.3 * 2.0) {
		// 	cf += -v;
		// }
		f += cf;
	}
	vec3 a = f / m;
	
	vec3 temp = cpos;
	cpos = cpos + ((cpos - ppos) * 1.0) + a * dt * dt;
	ppos = temp;

	planeCollision(cpos);

	float dist;

	for(int i = 0; i < noOfSpheres; i++) {
		vec3 delta = cpos - spheres[i].xyz;
		dist = length(delta);
		if(dist < spheres[i].w) {  
			cpos = spheres[i].xyz + delta * (spheres[i].w / dist);
			ppos = cpos;
		}
	}
	cposMass[id] = vec4(cpos, 1.0);
	pposMass[id] = vec4(ppos, 1.0);
}