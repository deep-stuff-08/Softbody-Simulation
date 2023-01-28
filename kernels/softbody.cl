#ifndef M_PI
#define M_PI 3.1415
#endif

kernel void createFilledCube(global float4 cposMass[], global float4 pposMass[], float size2, float restlen) {
	int id = (get_global_id(2) * get_global_size(1) + get_global_id(1)) * get_global_size(0) + get_global_id(0);
	int i = get_global_id(2);
	int j = get_global_id(1);
	int k = get_global_id(0);
	cposMass[id] = (float4)(-size2 + k * restlen, -size2 + j * restlen, -size2 + i * restlen, 1.0f);
	pposMass[id] = (float4)(-size2 + k * restlen, -size2 + j * restlen, -size2 + i * restlen, 1.0f);
}


kernel void createSphere(global float4 vert[], int stacks, int slices) {
	int i = get_global_id(1);
	int j = get_global_id(0);
	int id = i * get_global_size(0) + j;
	float phi = M_PI * ((float)i / (stacks - 1));
	float theta = 2.0f * M_PI * ((float)j / (slices - 1));
	vert[id] = (float4)(sin(phi) * sin(theta), cos(phi), sin(phi) * cos(theta), 1.0f);
}

kernel void applyModelMat(global float4 cposMass[], global float4 pposMass[], float4 transMat) {
	int id = get_global_id(0);
	cposMass[id] += transMat;
	pposMass[id] += transMat;
}

kernel void applyPhysics(global float4 cposMass[], global float4 pposMass[], constant float4 spheres[], int noOfSpheres, int sheetDims, float size) {
	const float dt = 1.0f / 5.0f;
    const float defDamp = -0.11f; 
    const float3 gravity = (float3)(0.0f, -0.00981f, 0.0f);
    const float3 ks = (float3)(5.5f, 1.9f, 0.6f);
    const float3 kd = (float3)(-0.1f, -0.2f, -0.3f);

    const int4 neighbourArray[40] = {
		//strucure
		(int4)(0, 0, 1, 0),
		(int4)(0, 0, -1, 0),
		(int4)(0, 1, 0, 0),
		(int4)(0, -1, 0, 0),
		(int4)(1, 0, 0, 0),
		(int4)(-1, 0, 0, 0),

		//shear xy plane
		(int4)(0, 1, 1, 1),
		(int4)(0, 1, -1, 1),
		(int4)(0, -1, 1, 1),
		(int4)(0, -1, -1, 1),

		//shear xz plane
		(int4)(1, 0, 1, 1),
		(int4)(1, 0, -1, 1),
		(int4)(-1, 0, 1, 1),
		(int4)(-1, 0, -1, 1),
		
		//shear yz plane
		(int4)(1, 1, 0, 1),
		(int4)(1, -1, 0, 1),
		(int4)(-1, 1, 0, 1),
		(int4)(-1, -1, 0, 1),

		//shear 3d
		(int4)(1, 1, 1, 1),
		(int4)(1, 1, -1, 1),
		(int4)(1, -1, 1, 1),
		(int4)(1, -1, -1, 1),
		(int4)(-1, 1, 1, 1),
		(int4)(-1, 1, -1, 1),
		(int4)(-1, -1, 1, 1),
		(int4)(-1, -1, -1, 1),

		//bending
		(int4)(0, 0, 2, 2),
		(int4)(0, 0, -2, 2),
		(int4)(0, 2, 0, 2),
		(int4)(0, -2, 0, 2),
		(int4)(2, 0, 0, 2),
		(int4)(-2, 0, 0, 2),

		(int4)(2, 2, 2, 2),
		(int4)(2, 2, -2, 2),
		(int4)(2, -2, 2, 2),
		(int4)(2, -2, -2, 2),
		(int4)(-2, 2, 2, 2),
		(int4)(-2, 2, -2, 2),
		(int4)(-2, -2, 2, 2),
		(int4)(-2, -2, -2, 2)
    };

	int id = get_global_id(0);
	float m = 1.0;
	float3 cpos = cposMass[id].xyz;
	float3 ppos = pposMass[id].xyz;
	float3 v = (cpos - ppos) / dt;
	
	float3 f = gravity * m + (defDamp * v);
	int3 currcoords = (int3)(id / (sheetDims * sheetDims), (id / sheetDims) % sheetDims, id % sheetDims);
	for(int i = 0; i < 40; i++) {
		int3 ncoord = currcoords + neighbourArray[i].xyz;
		if(ncoord.x < 0 || ncoord.x > (sheetDims-1) || ncoord.y < 0 || ncoord.y > (sheetDims-1) || ncoord.z < 0 || ncoord.z > (sheetDims-1)) {
			continue;
		}
		int nid = (ncoord.x * sheetDims + ncoord.y) * sheetDims + ncoord.z;
		float3 ncpos = cposMass[nid].xyz;
		float3 nppos = pposMass[nid].xyz;
		float restLen = length(convert_float3(neighbourArray[i].xyz) * (size / (sheetDims - 1)));

		float3 nv = (ncpos - nppos) / dt;
		float3 dpos = cpos - ncpos;
		float3 dv = v - nv;
		float dist = length(dpos);

		float3 cf = ((-ks[neighbourArray[i].w] * (dist - restLen)) + (kd[neighbourArray[i].w] * (dot(dpos, dv) / dist))) * normalize(dpos);

		f += cf;
	}
	float3 a = f / m;
	
	float3 temp = cpos;
	cpos = cpos + ((cpos - ppos)) + a * dt * dt;
	ppos = temp;

	float4 box[5] = {
		(float4)(1.0, 0.0, 0.0, 14.9),
		(float4)(-1.0, 0.0, 0.0, 14.9),
		(float4)(0.0, 1.0, 0.0, 17.9),
		// vec4(0.0, -1.0, 0.0, 18.0),
		(float4)(0.0, 0.0, 1.0, 11.9),
		(float4)(0.0, 0.0, -1.0, 11.9)
    };

	float dist;
	for(int i = 0; i < 5; i++) {
		dist = dot(box[i].xyz, cpos) + box[i].w;
		if(dist < 0.0) {
			cpos += box[i].xyz * -dist;
		}
	}
	
	for(int i = 0; i < noOfSpheres; i++) {
		float3 delta = cpos - spheres[i].xyz;
		dist = length(delta);
		if(dist < spheres[i].w + 0.05f) {  
			cpos = spheres[i].xyz + delta * ((spheres[i].w + 0.05f) / dist);
			ppos = cpos;
		}
	}

// 	for(int i = 0; i < NoOfSpheres; i++) {
// 		float3 x0 = vec3(inverse(ellipsoidMat[i]) * vec4(cpos, 1.0));
// 		float3 delta0 = x0;
// 		dist = length(delta0);
// 		if(dist < 1.0) {  
// 			delta0 = (1.0 - dist) * delta0 / dist;
// 			vec3 delta;

// 			vec3 transformInv = normalize(vec3(ellipsoidMat[i][0].x, ellipsoidMat[i][1].x, ellipsoidMat[i][2].x));
// 			delta.x = dot(delta0, transformInv);

// 			transformInv = normalize(vec3(ellipsoidMat[i][0].y, ellipsoidMat[i][1].y, ellipsoidMat[i][2].y));
// 			delta.y = dot(delta0, transformInv);

// 			transformInv = normalize(vec3(ellipsoidMat[i][0].z, ellipsoidMat[i][1].z, ellipsoidMat[i][2].z));
// 			delta.z = dot(delta0, transformInv); 
// 			cpos += delta;

// 			ppos = cpos;
// 		}
// 	}

	cposMass[id] = (float4)(cpos, 1.0);
	pposMass[id] = (float4)(ppos, 1.0);
}

kernel void calculateFaceNormals(global float4 *pos, global uint (*indices)[3], global float4 *facenormals) {
	int i = get_global_id(0);
	float4 e2 = pos[indices[i][0]] - pos[indices[i][1]]; 
	float4 e1 = pos[indices[i][2]] - pos[indices[i][1]];
	facenormals[i] = (float4)(cross(e1.xyz, e2.xyz), 0.0);
}

kernel void calculateVertexNormals(global int *faceList, global float4 *nor, global float4 *facenormals) {
	int i = get_global_id(0);
	nor[i] = (float4)(0.0);
	for(int j = 0; j < 6; j++) {
		int f = faceList[i * 6 + j];
		if(f != -1) {
			nor[i] += facenormals[f];
		}
	}
}