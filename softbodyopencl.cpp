#include<GL/glew.h>
#include"include/softbodyopencl.h"
#include<iostream>
#include<vector>

using namespace std;

void addToFaceListCL(vector<int> faceList[], vector<ivec3> &indiceList, ivec3 indices) {
	faceList[indices[0]].push_back(indiceList.size());
	faceList[indices[1]].push_back(indiceList.size());
	faceList[indices[2]].push_back(indiceList.size());
	indiceList.push_back(indices);
}

void createFilledCubeOnOpenCL(clcontext_dl *context, cl_kernel kernel, cl_mem vertices, cl_mem verticesOld, vector<ivec3> &indices, cl_mem faceTable, int subdivlevel, float size2) {
	vector<int> facevec[(subdivlevel + 1) * (subdivlevel + 1) * (subdivlevel + 1)];
	cl_int err;
	int *faceList = (int*)clEnqueueMapBuffer(context->cmdQueue, faceTable, CL_TRUE, CL_MAP_WRITE, 0, sizeof(int) * 6 * (subdivlevel + 1) * (subdivlevel + 1) * (subdivlevel + 1), 0, NULL, NULL, &err);
	if(err != CL_SUCCESS) cout<<err<<__FILE__<<__LINE__<<endl;
	
	float restLen = size2 * 2.0f / subdivlevel;

	cout<<setKernelParameters(kernel, vector<kernelparamater_dl>{ param(0, vertices), param(1, verticesOld), param(2, size2), param(3, restLen) });
	size_t globalSize[] = { (size_t)(subdivlevel+1), (size_t)(subdivlevel+1), (size_t)(subdivlevel+1) };
	cout<<runCLKernel(context, kernel, 3, globalSize, NULL, vector<cl_mem>{ vertices, verticesOld});
	clFinish(context->cmdQueue);	

	int count = 0;
	for(int i = 0; i < subdivlevel; i++) {
		for(int j = 0; j < subdivlevel; j++) {
			const short frontoff = (subdivlevel + 1) * (subdivlevel + 1) * subdivlevel;
			// front
			addToFaceListCL(facevec, indices, ivec3(frontoff + i * (subdivlevel+1) + j, frontoff + (i + 1) * (subdivlevel + 1) + j, frontoff + i * (subdivlevel+1) + j + 1));
			addToFaceListCL(facevec, indices, ivec3(frontoff + i * (subdivlevel+1) + j + 1, frontoff + (i + 1) * (subdivlevel + 1) + j, frontoff + (i + 1) * (subdivlevel+1) + j + 1));
			//back
			addToFaceListCL(facevec, indices, ivec3(i * (subdivlevel+1) + j, i * (subdivlevel+1) + j + 1, (i + 1) * (subdivlevel + 1) + j));
			addToFaceListCL(facevec, indices, ivec3(i * (subdivlevel+1) + j + 1, (i + 1) * (subdivlevel+1) + j + 1, (i + 1) * (subdivlevel + 1) + j));
		
			const short rightoff = subdivlevel;
			//left
			addToFaceListCL(facevec, indices, ivec3(i * (subdivlevel+1) * (subdivlevel + 1) + j * (subdivlevel + 1), i * (subdivlevel+1) * (subdivlevel + 1) + (j + 1) * (subdivlevel + 1), (i + 1) * (subdivlevel + 1) * (subdivlevel + 1) + j * (subdivlevel + 1)));
			addToFaceListCL(facevec, indices, ivec3(i * (subdivlevel+1) * (subdivlevel + 1) + (j + 1)  * (subdivlevel + 1), (i + 1) * (subdivlevel+1) * (subdivlevel + 1) + (j + 1) * (subdivlevel + 1), (i + 1) * (subdivlevel + 1) * (subdivlevel + 1) + j * (subdivlevel + 1)));
			//right
			addToFaceListCL(facevec, indices, ivec3(rightoff + i * (subdivlevel+1) * (subdivlevel + 1) + j * (subdivlevel + 1), rightoff + (i + 1) * (subdivlevel + 1) * (subdivlevel + 1) + j * (subdivlevel + 1), rightoff + i * (subdivlevel+1) * (subdivlevel + 1) + (j + 1) * (subdivlevel + 1)));
			addToFaceListCL(facevec, indices, ivec3(rightoff + i * (subdivlevel+1) * (subdivlevel + 1) + (j + 1)  * (subdivlevel + 1), rightoff + (i + 1) * (subdivlevel + 1) * (subdivlevel + 1) + j * (subdivlevel + 1), rightoff + (i + 1) * (subdivlevel+1) * (subdivlevel + 1) + (j + 1) * (subdivlevel + 1)));
	
			const short topoff = (subdivlevel + 1) * subdivlevel;
			//bottom
			addToFaceListCL(facevec, indices, ivec3(i * (subdivlevel+1) * (subdivlevel+1) + j, (i + 1) * (subdivlevel + 1) * (subdivlevel+1) + j, i * (subdivlevel+1) * (subdivlevel+1) + j + 1));
			addToFaceListCL(facevec, indices, ivec3(i * (subdivlevel+1) * (subdivlevel+1) + j + 1, (i + 1) * (subdivlevel + 1) * (subdivlevel+1) + j, (i + 1) * (subdivlevel+1) * (subdivlevel+1) + j + 1));
			//top
			addToFaceListCL(facevec, indices, ivec3(topoff + i * (subdivlevel+1) * (subdivlevel+1) + j, topoff + i * (subdivlevel+1) * (subdivlevel+1) + j + 1, topoff + (i + 1) * (subdivlevel + 1) * (subdivlevel+1) + j));
			addToFaceListCL(facevec, indices, ivec3(topoff + i * (subdivlevel+1) * (subdivlevel+1) + j + 1, topoff + (i + 1) * (subdivlevel+1) * (subdivlevel+1) + j + 1, topoff + (i + 1) * (subdivlevel + 1) * (subdivlevel+1) + j));
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
	err = clEnqueueUnmapMemObject(context->cmdQueue, faceTable, faceList, 0, NULL, NULL);
	if(err != CL_SUCCESS) cout<<err<<__FILE__<<__LINE__<<endl;
}

void applyModelMatrixOnOpenCL(clcontext_dl *context, cl_kernel kernel, cl_mem vertices, cl_mem verticesOld, int subdivlevel, cl_float4 translateMat) {
	cout<<setKernelParameters(kernel, vector<kernelparamater_dl>{ param(0, vertices), param(1, verticesOld), param(2, translateMat) });
	size_t globalSize[] = { (size_t)((subdivlevel+1) * (subdivlevel+1) * (subdivlevel+1)) };
	cout<<runCLKernel(context, kernel, 1, globalSize, NULL, vector<cl_mem>{ vertices, verticesOld });
	clFinish(context->cmdQueue);	
}

void createSphereOnOpenCL(clcontext_dl *context, cl_kernel kernel, cl_mem vertices, GLuint index, int stacks, int slices) {
	ivec3 *indices = (ivec3*)glMapNamedBufferRange(index, 0, sizeof(ivec3) * (stacks - 1) * (slices - 1) * 2, GL_MAP_WRITE_BIT);
	int count = 0;

	cout<<setKernelParameters(kernel, vector<kernelparamater_dl>{ param(0, vertices), param(1, stacks), param(2, slices) });
	size_t globalSize[] = { (size_t)slices, (size_t)(stacks) };
	cout<<runCLKernel(context, kernel, 2, globalSize, NULL, vector<cl_mem>{ vertices });

	clFinish(context->cmdQueue);

	count = 0;

	for(int i = 0; i < stacks - 1; i++) {
		for(int j = 0; j < slices - 1; j++) {
			indices[count++] = ivec3(i * slices + j, (i + 1) * slices + j, (i + 1) * slices + j + 1);
			indices[count++] = ivec3((i + 1) * slices + j + 1, i * slices + j + 1, i * slices + j);
		}
	}
	glUnmapNamedBuffer(index);
}

void updateSoftbodyOnOpenCL(clcontext_dl *context, cl_kernel kernel, cl_mem vertices, cl_mem verticesOld, cl_mem spherePos, int sphereCount, int sheetdims, float size) {
	cout<<setKernelParameters(kernel, vector<kernelparamater_dl>{ param(0, vertices), param(1, verticesOld), param(2, spherePos), param(3, sphereCount), param(4, sheetdims), param(5, size) });
	size_t globalSize[] = { (size_t)(sheetdims * sheetdims * sheetdims) };
	cout<<runCLKernel(context, kernel, 1, globalSize, NULL, vector<cl_mem>{ vertices, verticesOld, spherePos });
	clFinish(context->cmdQueue);
}

void calculateVertexNormalsOnOpenCL(clcontext_dl *context, cl_kernel faceNormalKernel, cl_kernel vertexNormalKernel, cl_mem points, cl_mem pointsNormal, cl_mem pointsIndex, cl_mem faceTable, int subdivlevel) {
	cl_int err;
	cl_mem faceNormals = clCreateBuffer(context->context, CL_MEM_READ_WRITE, sizeof(vec4) * subdivlevel * subdivlevel * 12, NULL, &err);
	if(err != CL_SUCCESS) cout<<err<<__FILE__<<"  "<<__LINE__<<endl;
	
	cout<<setKernelParameters(faceNormalKernel, vector<kernelparamater_dl>{ param(0, points), param(1, pointsIndex), param(2, faceNormals)});
	size_t globalSize1[] = { (size_t)(subdivlevel * subdivlevel * 12) };
	cout<<runCLKernel(context, faceNormalKernel, 1, globalSize1, NULL, vector<cl_mem>{ points, pointsIndex });
	
	cout<<setKernelParameters(vertexNormalKernel, vector<kernelparamater_dl>{ param(0, faceTable), param(1, pointsNormal), param(2, faceNormals)});
	size_t globalSize2[] = { (size_t)((subdivlevel + 1) * (subdivlevel + 1) * (subdivlevel + 1)) };
	cout<<runCLKernel(context, vertexNormalKernel, 1, globalSize2, NULL, vector<cl_mem>{ faceTable, pointsNormal });
	
	clReleaseMemObject(faceNormals);
	clFinish(context->cmdQueue);
}