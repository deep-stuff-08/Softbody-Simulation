#ifndef __OPENCLDL__
#define __OPENCLDL__

#define CL_TARGET_OPENCL_VERSION 120
#include<CL/cl.h>
#include<vector>
#include<iostream>
#include<GL/gl.h>

#define param(p, x) {p, &x, sizeof(x)}

struct clcontext_dl {
	cl_context context;
	cl_device_id device;
	cl_command_queue cmdQueue;
};

struct kernelparamater_dl {
	int position;
	void *param;
	size_t size;
};

std::string initOpenCLContext(clcontext_dl *context, int platformNo = 0, int deviceNo = 0);
std::string loadKernelsFromPrograms(clcontext_dl *context, std::vector<std::string> programNames, std::vector<cl_kernel> &kernelList);
std::string setKernelParameters(cl_kernel kernel, std::vector<kernelparamater_dl> kernelList);
std::string createCLGLBuffer(clcontext_dl *context, cl_mem_flags memFlags, GLuint buffer, cl_mem *ret);
std::string createCLGLTexture(clcontext_dl *context, cl_mem_flags memFlags, GLenum texTarget, GLuint mipMapLevel, GLuint texture, cl_mem *ret);
std::string runCLKernel(clcontext_dl *context, cl_kernel kernel, cl_uint workDims, size_t *globalSize, size_t *localSize, std::vector<cl_mem> globjects);

#endif