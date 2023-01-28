#include<GL/glew.h>
#include<GL/gl.h>
#include"include/softbodycompute.h"
#include<iostream>
#include<vector>
#include<set>
using namespace std;

void addToFaceListCompute(vector<int> faceList[], vector<ivec3> &indiceList, ivec3 indices) {
	faceList[indices[0]].push_back(indiceList.size());
	faceList[indices[1]].push_back(indiceList.size());
	faceList[indices[2]].push_back(indiceList.size());
	indiceList.push_back(indices);
}

void createFilledCubeOnCompute(GLuint vao, glprogram_dl program, GLuint points, GLuint pointsOld, vector<ivec3> &indices, GLuint faceTable, int subdivlevel, float size2) {
	int *faceList = (int*)glMapNamedBufferRange(faceTable, 0, sizeof(int) * 6 * (subdivlevel + 1) * (subdivlevel + 1) * (subdivlevel + 1), GL_MAP_WRITE_BIT);

	vector<int> facevec[(subdivlevel + 1) * (subdivlevel + 1) * (subdivlevel + 1)];

	const float restLen = size2 * 2.0f / subdivlevel;

	glUseProgram(program.programObject);
	glUniform1f(0, size2);
	glUniform1f(1, restLen);
	glUniform1i(2, subdivlevel+1);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, points);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pointsOld);
	glDispatchCompute(subdivlevel + 1, subdivlevel + 1, subdivlevel + 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	for(int i = 0; i < subdivlevel; i++) {
		for(int j = 0; j < subdivlevel; j++) {
			const short frontoff = (subdivlevel + 1) * (subdivlevel + 1) * subdivlevel;
			// front
			addToFaceListCompute(facevec, indices, ivec3(frontoff + i * (subdivlevel+1) + j, frontoff + (i + 1) * (subdivlevel + 1) + j, frontoff + i * (subdivlevel+1) + j + 1));
			addToFaceListCompute(facevec, indices, ivec3(frontoff + i * (subdivlevel+1) + j + 1, frontoff + (i + 1) * (subdivlevel + 1) + j, frontoff + (i + 1) * (subdivlevel+1) + j + 1));
			//back
			addToFaceListCompute(facevec, indices, ivec3(i * (subdivlevel+1) + j, i * (subdivlevel+1) + j + 1, (i + 1) * (subdivlevel + 1) + j));
			addToFaceListCompute(facevec, indices, ivec3(i * (subdivlevel+1) + j + 1, (i + 1) * (subdivlevel+1) + j + 1, (i + 1) * (subdivlevel + 1) + j));
		
			const short rightoff = subdivlevel;
			//left
			addToFaceListCompute(facevec, indices, ivec3(i * (subdivlevel+1) * (subdivlevel + 1) + j * (subdivlevel + 1), i * (subdivlevel+1) * (subdivlevel + 1) + (j + 1) * (subdivlevel + 1), (i + 1) * (subdivlevel + 1) * (subdivlevel + 1) + j * (subdivlevel + 1)));
			addToFaceListCompute(facevec, indices, ivec3(i * (subdivlevel+1) * (subdivlevel + 1) + (j + 1)  * (subdivlevel + 1), (i + 1) * (subdivlevel+1) * (subdivlevel + 1) + (j + 1) * (subdivlevel + 1), (i + 1) * (subdivlevel + 1) * (subdivlevel + 1) + j * (subdivlevel + 1)));
			//right
			addToFaceListCompute(facevec, indices, ivec3(rightoff + i * (subdivlevel+1) * (subdivlevel + 1) + j * (subdivlevel + 1), rightoff + (i + 1) * (subdivlevel + 1) * (subdivlevel + 1) + j * (subdivlevel + 1), rightoff + i * (subdivlevel+1) * (subdivlevel + 1) + (j + 1) * (subdivlevel + 1)));
			addToFaceListCompute(facevec, indices, ivec3(rightoff + i * (subdivlevel+1) * (subdivlevel + 1) + (j + 1)  * (subdivlevel + 1), rightoff + (i + 1) * (subdivlevel + 1) * (subdivlevel + 1) + j * (subdivlevel + 1), rightoff + (i + 1) * (subdivlevel+1) * (subdivlevel + 1) + (j + 1) * (subdivlevel + 1)));
	
			const short topoff = (subdivlevel + 1) * subdivlevel;
			//bottom
			addToFaceListCompute(facevec, indices, ivec3(i * (subdivlevel+1) * (subdivlevel+1) + j, (i + 1) * (subdivlevel + 1) * (subdivlevel+1) + j, i * (subdivlevel+1) * (subdivlevel+1) + j + 1));
			addToFaceListCompute(facevec, indices, ivec3(i * (subdivlevel+1) * (subdivlevel+1) + j + 1, (i + 1) * (subdivlevel + 1) * (subdivlevel+1) + j, (i + 1) * (subdivlevel+1) * (subdivlevel+1) + j + 1));
			//top
			addToFaceListCompute(facevec, indices, ivec3(topoff + i * (subdivlevel+1) * (subdivlevel+1) + j, topoff + i * (subdivlevel+1) * (subdivlevel+1) + j + 1, topoff + (i + 1) * (subdivlevel + 1) * (subdivlevel+1) + j));
			addToFaceListCompute(facevec, indices, ivec3(topoff + i * (subdivlevel+1) * (subdivlevel+1) + j + 1, topoff + (i + 1) * (subdivlevel+1) * (subdivlevel+1) + j + 1, topoff + (i + 1) * (subdivlevel + 1) * (subdivlevel+1) + j));
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
	glUnmapNamedBuffer(faceTable);
}

void applyModelMatrixOnCompute(GLuint vao, glprogram_dl program, GLuint points, GLuint pointsOld, int subdivlevel, vec4 translate) {
	glBindVertexArray(vao);
	glUseProgram(program.programObject);
	glUniform4fv(0, 1, translate);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, points);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pointsOld);
	glDispatchCompute((subdivlevel + 1) * (subdivlevel + 1) * (subdivlevel + 1), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void createSphereOnCompute(GLuint vao, glprogram_dl program, GLuint spherePoints, GLuint sphereIndices, int stacks, int slices) {
	ivec3 *indices = (ivec3*)glMapNamedBufferRange(sphereIndices, 0, sizeof(ivec3) * (stacks - 1) * (slices - 1) * 2, GL_MAP_WRITE_BIT);	

	glBindVertexArray(vao);
	glUseProgram(program.programObject);
	glUniform2i(0, stacks, slices);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, spherePoints);
	glDispatchCompute(stacks, slices, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	int count = 0;

	for(int i = 0; i < stacks - 1; i++) {
		for(int j = 0; j < slices - 1; j++) {
			indices[count++] = ivec3(i * slices + j, (i + 1) * slices + j, (i + 1) * slices + j + 1);
			indices[count++] = ivec3((i + 1) * slices + j + 1, i * slices + j + 1, i * slices + j);
		}
	}
	glUnmapNamedBuffer(sphereIndices);
}

void updateSoftbodyOnCompute(GLuint vao, glprogram_dl program, GLuint points, GLuint pointsOld, GLuint spherePosSsbo, int numSpheres, int sheetDims, float size) {
	const float dt = 1.0f / 5.0f;
	const vec3 gravity = vec3(0.0f, -0.00981f, 0.0f);
	const float defDamp = -0.11f;
	const vec3 ks = vec3(2.5f, 0.9f, 0.6f);
	const vec3 kd = vec3(-0.1f, -0.2f, -0.3f);

	glUseProgram(program.programObject);
	glUniform1f(0, 1.0f / 5.0f);
	glUniform3f(1, 0.0f, -0.00981f, 0.0f);
	glUniform1i(3, sheetDims);
	glUniform1f(4, size);
	glUniform1i(5, numSpheres);
	
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, points);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pointsOld);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, spherePosSsbo);
	glBindVertexArray(vao);
	glDispatchCompute(sheetDims * sheetDims * sheetDims, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glFinish();
}

void calculateVertexNormalsOnCompute(GLuint vao, glprogram_dl faceprogram, glprogram_dl vertprogram, GLuint points, GLuint pointsNormal, GLuint pointsIndex, GLuint faceTable, int subdivlevel) {
	GLuint faceNormals;
	glGenBuffers(1, &faceNormals);
	glBindBuffer(GL_ARRAY_BUFFER, faceNormals);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * subdivlevel * subdivlevel * 12, NULL, GL_DYNAMIC_DRAW);
	
	glUseProgram(faceprogram.programObject);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, points);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pointsIndex);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, faceNormals);
	glBindVertexArray(vao);
	glDispatchCompute(subdivlevel * subdivlevel * 12, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	
	glUseProgram(vertprogram.programObject);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, faceTable);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pointsNormal);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, faceNormals);
	glBindVertexArray(vao);
	glDispatchCompute((subdivlevel + 1) * (subdivlevel + 1) * (subdivlevel + 1), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glDeleteBuffers(1, &faceNormals);
}