#include"include/opencldl.h"
#include<fstream>
#include<sstream>
#include<vector>
#include<CL/cl_gl.h>
#include<GL/glx.h>
#include<cstdarg>

using namespace std;

#define CheckErr if(err != CL_SUCCESS) errorString<<"OpenCL Error: "<<err<<" at Line: "<<__LINE__<<"\n"

string initOpenCLContext(clcontext_dl *context, int platformNo, int deviceNo) {
	cl_platform_id *platforms;
	cl_device_id *devices;
	cl_uint num;
	std::stringstream errorString;
	cl_int err;
	
	err = clGetPlatformIDs(0, NULL, &num);CheckErr;
	if(num >= platformNo) {
		platformNo = 0;
	}
	platforms = (cl_platform_id*)alloca(sizeof(cl_platform_id) * num);
	err = clGetPlatformIDs(num, platforms, NULL);CheckErr;

	err = clGetDeviceIDs(platforms[platformNo], CL_DEVICE_TYPE_ALL, 0, NULL, &num);CheckErr;
	if(num >= deviceNo) {
		deviceNo = 0;
	}
	devices = (cl_device_id*)alloca(sizeof(cl_device_id) * num);
	err = clGetDeviceIDs(platforms[platformNo], CL_DEVICE_TYPE_ALL, num, devices, NULL);CheckErr;

	context->device = devices[deviceNo];

	cl_context_properties contextProp[] = {
		CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
		CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
		CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[platformNo],
		0
	};
	context->context = clCreateContext(contextProp, 1, &devices[deviceNo], NULL, NULL, &err);CheckErr;

	context->cmdQueue = clCreateCommandQueue(context->context, devices[deviceNo], 0, &err);CheckErr;

	return errorString.str();
}

std::string loadKernelsFromPrograms(clcontext_dl *context, vector<string> programNames, vector<cl_kernel> &kernelList) {
	stringstream errorString;
	cl_int err;
	cl_uint num;
	char **src = (char**)alloca(sizeof(char*) * programNames.size());
	size_t *len = (size_t*)alloca(sizeof(size_t) * programNames.size());
	string *strs = new string[programNames.size()];

	for(int i = 0; i < programNames.size(); i++) {
		ifstream prog(programNames[i]);
		string programSrc((istreambuf_iterator<char>(prog)), istreambuf_iterator<char>());
		strs[i] = programSrc;
		src[i] = (char*)strs[i].c_str();
		len[i] = strs[i].length();
	}
	cl_program program = clCreateProgramWithSource(context->context, programNames.size(), (const char**)src, len, &err);CheckErr;
	err = clBuildProgram(program, 1, &context->device, NULL, NULL, NULL);CheckErr;
	if(err != CL_SUCCESS) {
		char *buffer;
		size_t s;
		err = clGetProgramBuildInfo(program, context->device, CL_PROGRAM_BUILD_LOG, 0, NULL, &s);
		buffer = (char*)alloca(s);
		err = clGetProgramBuildInfo(program, context->device, CL_PROGRAM_BUILD_LOG, s, buffer, NULL);
		errorString<<"Error:"<<buffer<<endl;
	}
	err = clCreateKernelsInProgram(program, 0, NULL, &num);CheckErr;
	cl_kernel *kernels = (cl_kernel*)alloca(sizeof(cl_kernel) * num);
	err = clCreateKernelsInProgram(program, num, kernels, NULL);
	for(int i = 0; i < num; i++) {
		kernelList.push_back(kernels[i]);
	}
	delete[] strs;
	return errorString.str();
}

std::string setKernelParameters(cl_kernel kernel, vector<kernelparamater_dl> kernelList) {
	stringstream errorString;
	cl_int err;
	for(int i = 0; i < kernelList.size(); i++) {
		err = clSetKernelArg(kernel, kernelList[i].position, kernelList[i].size, kernelList[i].param); CheckErr;
	}
	return errorString.str();
}

std::string createCLGLBuffer(clcontext_dl *context, cl_mem_flags memFlags, GLuint buffer, cl_mem *ret) {
	stringstream errorString;
	cl_int err;
	*ret = clCreateFromGLBuffer(context->context, memFlags, buffer, &err); CheckErr;
	return errorString.str();
}

std::string createCLGLTexture(clcontext_dl *context, cl_mem_flags memFlags, GLenum texTarget, GLuint mipMapLevel, GLuint texture, cl_mem *ret) {
	stringstream errorString;
	cl_int err;
	*ret = clCreateFromGLTexture(context->context, memFlags, texTarget, mipMapLevel, texture, &err); CheckErr;
	return errorString.str();
}

std::string runCLKernel(clcontext_dl *context, cl_kernel kernel, cl_uint workDim, size_t *globalSize, size_t *localSize, vector<cl_mem> globjects) {
	stringstream errorString;
	cl_int err;
	err = clEnqueueAcquireGLObjects(context->cmdQueue, globjects.size(), globjects.data(), 0, NULL, NULL); CheckErr;
	err = clEnqueueNDRangeKernel(context->cmdQueue, kernel, workDim, NULL, globalSize, localSize, 0, NULL, NULL); CheckErr;
	err = clEnqueueReleaseGLObjects(context->cmdQueue, globjects.size(), globjects.data(), 0, NULL, NULL); CheckErr;
	return errorString.str();
}
