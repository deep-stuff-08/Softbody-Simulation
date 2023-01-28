#include<GL/glew.h>
#include<GL/gl.h>
#include"include/softbodycpu.h"
#include<iostream>
#include<vector>
#include<set>
using namespace std;

void addToFaceList(vector<int> faceList[], vector<ivec3> &indiceList, ivec3 indices) {
	faceList[indices[0]].push_back(indiceList.size());
	faceList[indices[1]].push_back(indiceList.size());
	faceList[indices[2]].push_back(indiceList.size());
	indiceList.push_back(indices);
}

void createFilledCubeOnCpu(GLuint points, GLuint pointsOld, vector<ivec3> &indices, GLuint faceTable, int subdivlevel, float size2) {
	vec4 *vertices = (vec4*)glMapNamedBufferRange(points, 0, sizeof(vec4) * (subdivlevel + 1) * (subdivlevel + 1) * (subdivlevel + 1), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
	vec4 *verticesOld = (vec4*)glMapNamedBufferRange(pointsOld, 0, sizeof(vec4) * (subdivlevel + 1) * (subdivlevel + 1) * (subdivlevel + 1), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
	int *faceList = (int*)glMapNamedBufferRange(faceTable, 0, sizeof(int) * 6 * (subdivlevel + 1) * (subdivlevel + 1) * (subdivlevel + 1), GL_MAP_WRITE_BIT);

	vector<int> facevec[(subdivlevel + 1) * (subdivlevel + 1) * (subdivlevel + 1)];

	const float restLen = size2 * 2.0f / subdivlevel;

	int count = 0;
	for(int i = 0; i <= subdivlevel; i++) {
		for(int j = 0; j <= subdivlevel; j++) {
			for(int k = 0; k <= subdivlevel; k++) {
				verticesOld[count] = (vec4(-size2 + k * restLen, -size2 + j * restLen, -size2 + i * restLen, 1.0f));
				vertices[count++] = (vec4(-size2 + k * restLen, -size2 + j * restLen, -size2 + i * restLen, 1.0f));
			}
		}
	}

	for(int i = 0; i < subdivlevel; i++) {
		for(int j = 0; j < subdivlevel; j++) {
			const short frontoff = (subdivlevel + 1) * (subdivlevel + 1) * subdivlevel;
			// front
			addToFaceList(facevec, indices, ivec3(frontoff + i * (subdivlevel+1) + j, frontoff + (i + 1) * (subdivlevel + 1) + j, frontoff + i * (subdivlevel+1) + j + 1));
			addToFaceList(facevec, indices, ivec3(frontoff + i * (subdivlevel+1) + j + 1, frontoff + (i + 1) * (subdivlevel + 1) + j, frontoff + (i + 1) * (subdivlevel+1) + j + 1));
			//back
			addToFaceList(facevec, indices, ivec3(i * (subdivlevel+1) + j, i * (subdivlevel+1) + j + 1, (i + 1) * (subdivlevel + 1) + j));
			addToFaceList(facevec, indices, ivec3(i * (subdivlevel+1) + j + 1, (i + 1) * (subdivlevel+1) + j + 1, (i + 1) * (subdivlevel + 1) + j));
		
			const short rightoff = subdivlevel;
			//left
			addToFaceList(facevec, indices, ivec3(i * (subdivlevel+1) * (subdivlevel + 1) + j * (subdivlevel + 1), i * (subdivlevel+1) * (subdivlevel + 1) + (j + 1) * (subdivlevel + 1), (i + 1) * (subdivlevel + 1) * (subdivlevel + 1) + j * (subdivlevel + 1)));
			addToFaceList(facevec, indices, ivec3(i * (subdivlevel+1) * (subdivlevel + 1) + (j + 1)  * (subdivlevel + 1), (i + 1) * (subdivlevel+1) * (subdivlevel + 1) + (j + 1) * (subdivlevel + 1), (i + 1) * (subdivlevel + 1) * (subdivlevel + 1) + j * (subdivlevel + 1)));
			//right
			addToFaceList(facevec, indices, ivec3(rightoff + i * (subdivlevel+1) * (subdivlevel + 1) + j * (subdivlevel + 1), rightoff + (i + 1) * (subdivlevel + 1) * (subdivlevel + 1) + j * (subdivlevel + 1), rightoff + i * (subdivlevel+1) * (subdivlevel + 1) + (j + 1) * (subdivlevel + 1)));
			addToFaceList(facevec, indices, ivec3(rightoff + i * (subdivlevel+1) * (subdivlevel + 1) + (j + 1)  * (subdivlevel + 1), rightoff + (i + 1) * (subdivlevel + 1) * (subdivlevel + 1) + j * (subdivlevel + 1), rightoff + (i + 1) * (subdivlevel+1) * (subdivlevel + 1) + (j + 1) * (subdivlevel + 1)));
	
			const short topoff = (subdivlevel + 1) * subdivlevel;
			//bottom
			addToFaceList(facevec, indices, ivec3(i * (subdivlevel+1) * (subdivlevel+1) + j, (i + 1) * (subdivlevel + 1) * (subdivlevel+1) + j, i * (subdivlevel+1) * (subdivlevel+1) + j + 1));
			addToFaceList(facevec, indices, ivec3(i * (subdivlevel+1) * (subdivlevel+1) + j + 1, (i + 1) * (subdivlevel + 1) * (subdivlevel+1) + j, (i + 1) * (subdivlevel+1) * (subdivlevel+1) + j + 1));
			//top
			addToFaceList(facevec, indices, ivec3(topoff + i * (subdivlevel+1) * (subdivlevel+1) + j, topoff + i * (subdivlevel+1) * (subdivlevel+1) + j + 1, topoff + (i + 1) * (subdivlevel + 1) * (subdivlevel+1) + j));
			addToFaceList(facevec, indices, ivec3(topoff + i * (subdivlevel+1) * (subdivlevel+1) + j + 1, topoff + (i + 1) * (subdivlevel+1) * (subdivlevel+1) + j + 1, topoff + (i + 1) * (subdivlevel + 1) * (subdivlevel+1) + j));
		}
	}

	for(int i = 0; i < (subdivlevel+1) * (subdivlevel+1) * (subdivlevel+1); i++) {
		for(int j = 0; j < facevec[i].size(); j++) {
			faceList[i * 6 + j] = facevec[i][j];
		}
		for(int j = facevec[i].size(); j < 6; j++) {
			faceList[i * 6 + j] = -1;
		}
	}
	glUnmapNamedBuffer(points);
	glUnmapNamedBuffer(pointsOld);
	glUnmapNamedBuffer(faceTable);
}

void applyModelMatrixOnCpu(GLuint points, GLuint pointsOld, int subdivlevel, vec4 translate) {
	vec4 *vertices = (vec4*)glMapNamedBufferRange(points, 0, sizeof(vec4) * (subdivlevel + 1) * (subdivlevel + 1) * (subdivlevel + 1), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
	vec4 *verticesOld = (vec4*)glMapNamedBufferRange(pointsOld, 0, sizeof(vec4) * (subdivlevel + 1) * (subdivlevel + 1) * (subdivlevel + 1), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
	int count = 0;
	for(int i = 0; i <= subdivlevel; i++) {
		for(int j = 0; j <= subdivlevel; j++) {
			for(int k = 0; k <= subdivlevel; k++) {
				verticesOld[count] = verticesOld[count] + translate;
				vertices[count++] = vertices[count] + translate;
			}
		}
	}
	glUnmapNamedBuffer(points);
	glUnmapNamedBuffer(pointsOld);
}

void createSphereOnCpu(GLuint spherePoints, GLuint sphereIndices, int stacks, int slices) {
	vec4 *vertices = (vec4*)glMapNamedBufferRange(spherePoints, 0, sizeof(vec3) * stacks * slices, GL_MAP_WRITE_BIT);
	ivec3 *indices = (ivec3*)glMapNamedBufferRange(sphereIndices, 0, sizeof(ivec3) * (stacks - 1) * (slices - 1) * 2, GL_MAP_WRITE_BIT);	
	int count = 0;

	for(int i = 0; i < stacks; i++) {
		float phi = M_PI * ((float)i / (stacks - 1));
		for(int j = 0; j < slices; j++) {
			float theta = 2.0f * M_PI * ((float)j / (slices - 1));
			vertices[count++] = vec4(sin(phi) * sin(theta), cos(phi), sin(phi) * cos(theta), 1.0f);
		}
	}

	count = 0;

	for(int i = 0; i < stacks - 1; i++) {
		for(int j = 0; j < slices - 1; j++) {
			indices[count++] = ivec3(i * slices + j, (i + 1) * slices + j, (i + 1) * slices + j + 1);
			indices[count++] = ivec3((i + 1) * slices + j + 1, i * slices + j + 1, i * slices + j);
		}
	}
	glUnmapNamedBuffer(spherePoints);
	glUnmapNamedBuffer(sphereIndices);
}

void updateSoftbodyOnCpu(GLuint points, GLuint pointsOld, vector<vec4> spheres, int sheetDims, float size) {
	const float dt = 1.0f / 5.0f;
	const vec3 gravity = vec3(0.0f, -0.00981f, 0.0f);
	const float defDamp = -0.11f;
	const vec3 ks = vec3(2.5f, 0.9f, 0.6f);
	const vec3 kd = vec3(-0.1f, -0.2f, -0.3f);

	vec4 *cposMass = (vec4*)glMapNamedBufferRange(points, 0, sizeof(vec4) * sheetDims * sheetDims * sheetDims, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
	vec4 *pposMass = (vec4*)glMapNamedBufferRange(pointsOld, 0, sizeof(vec4) * sheetDims * sheetDims * sheetDims, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
	
	vec4 *ncposMass = (vec4*)malloc(sizeof(vec4) * sheetDims * sheetDims * sheetDims);
	vec4 *npposMass = (vec4*)malloc(sizeof(vec4) * sheetDims * sheetDims * sheetDims);

	for(int i = 0; i < sheetDims * sheetDims * sheetDims; i++) {
		ncposMass[i] = cposMass[i];
		npposMass[i] = pposMass[i];
	}

	for(int j = 0; j < sheetDims * sheetDims * sheetDims; j++) {
		const ivec4 neighbourArray[40] = {
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
		};

		int id = j;
		float m = 1.0;
		vec3 cpos = vec3(cposMass[id][0], cposMass[id][1], cposMass[id][2]);
		vec3 ppos = vec3(pposMass[id][0], pposMass[id][1], pposMass[id][2]);
		vec3 v = (cpos - ppos) / dt;

		vec3 f = gravity * m + (defDamp * v);
		ivec3 currcoords = ivec3(id / (sheetDims * sheetDims), (id / sheetDims) % sheetDims, id % sheetDims);
		for(int i = 0; i < 40; i++) {
			ivec3 ncoord = currcoords + ivec3(neighbourArray[i][0], neighbourArray[i][1], neighbourArray[i][2]);
			if(ncoord[0] < 0 || ncoord[0] > (sheetDims-1) || ncoord[1] < 0 || ncoord[1] > (sheetDims-1) || ncoord[2] < 0 || ncoord[2] > (sheetDims-1)) {
				continue;
			}
			int nid = (ncoord[0] * sheetDims + ncoord[1]) * sheetDims + ncoord[2];
			vec3 ncpos = vec3(ncposMass[nid][0], ncposMass[nid][1], ncposMass[nid][2]);
			vec3 nppos = vec3(npposMass[nid][0], npposMass[nid][1], npposMass[nid][2]);
			float restLen = length(vec3(neighbourArray[i][0], neighbourArray[i][1], neighbourArray[i][2]) * (size / (sheetDims - 1)));

			vec3 nv = (ncpos - nppos) / dt;
			vec3 dpos = cpos - ncpos;
			vec3 dv = v - nv;
			float dist = length(dpos);

			vec3 cf = ((-ks[neighbourArray[i][3]] * (dist - restLen)) + (kd[neighbourArray[i][3]] * (dot(dpos, dv) / dist))) * normalize(dpos);

			f += cf;
		}
		vec3 a = f / m;
		
		vec3 temp = cpos;
		cpos = cpos + ((cpos - ppos) * 1.0f) + a * dt * dt;
		ppos = temp;

		float dist;
		vec4 box[5] = {
			vec4(1.0, 0.0, 0.0, 14.9),
			vec4(-1.0, 0.0, 0.0, 14.9),
			vec4(0.0, 1.0, 0.0, 17.9),
			// vec4(0.0, -1.0, 0.0, 18.0),
			vec4(0.0, 0.0, 1.0, 11.9),
			vec4(0.0, 0.0, -1.0, 11.9)
		};

		for(int i = 0; i < 5; i++) {
			vec3 boxcoord = vec3(box[i][0], box[i][1], box[i][2]);
			dist = dot(boxcoord, cpos) + box[i][3];
			if(dist < 0.0) {
				cpos += boxcoord * -dist;
			}
		}

		for(int i = 0; i < spheres.size(); i++) {
			vec3 spherePos = vec3(spheres[i][0], spheres[i][1], spheres[i][2]);
			vec3 delta = cpos - spherePos;
			dist = length(delta);
			if(dist < spheres[i][3]) {  
				cpos = spherePos + delta * (spheres[i][3] / dist);
				ppos = cpos;
			}
		}

		cposMass[id] = vec4(cpos, 1.0f);
		pposMass[id] = vec4(ppos, 1.0f);
	}
	free(ncposMass);
	free(npposMass);

	glUnmapNamedBuffer(points);
	glUnmapNamedBuffer(pointsOld);
}

void calculateVertexNormalsOnCpu(GLuint points, GLuint pointsNormal, GLuint pointsIndex, GLuint faceTable, int subdivlevel) {
	vec4* nor = (vec4*)glMapNamedBufferRange(pointsNormal, 0, sizeof(vec4) * (subdivlevel + 1) * (subdivlevel + 1) * (subdivlevel + 1), GL_MAP_WRITE_BIT);
	vec4* pos = (vec4*)glMapNamedBufferRange(points, 0, sizeof(vec4) * (subdivlevel + 1) * (subdivlevel + 1) * (subdivlevel + 1), GL_MAP_READ_BIT);
	int* faceList = (int*)glMapNamedBufferRange(faceTable, 0, sizeof(int) * 6 * (subdivlevel + 1) * (subdivlevel + 1) * (subdivlevel + 1), GL_MAP_READ_BIT);
	ivec3* indices = (ivec3*)glMapNamedBufferRange(pointsIndex, 0, sizeof(ivec3) * subdivlevel * subdivlevel * 12, GL_MAP_READ_BIT);
	vec3* facenormals = (vec3*)malloc(sizeof(vec3) * subdivlevel * subdivlevel * 12);
	for(int i = 0; i < subdivlevel * subdivlevel * 12; i++) {
		const vec4 e2 = pos[indices[i][0]] - pos[indices[i][1]]; 
		const vec4 e1 = pos[indices[i][2]] - pos[indices[i][1]];
		facenormals[i] = cross(vec3(e1[0], e1[1], e1[2]), vec3(e2[0], e2[1], e2[2]));
	}
	for(int i = 0; i < (subdivlevel + 1) * (subdivlevel + 1) * (subdivlevel + 1); i++) {
		nor[i] = vec4(0.0f);
		for(int j = 0; j < 6; j++) {
			int f = faceList[i * 6 + j];
			if(f != -1) {
				nor[i] += vec4(facenormals[f], 0.0);
			}
		}
	}
	free(facenormals);
	glUnmapNamedBuffer(pointsNormal);
	glUnmapNamedBuffer(points);
	glUnmapNamedBuffer(faceTable);
	glUnmapNamedBuffer(pointsIndex);
}