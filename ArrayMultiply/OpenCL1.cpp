// OpenCL1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#include <iostream>
#include <CL/cl.hpp>
#include <vector>
#include <fstream>
#include <ctime>

cl::Device getDevice(int type) {
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	std::vector<cl::Device> devices;

	if (platforms.size() == 0) return false;
	auto platform = platforms.front();

	if (type == 0) platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
	else platform.getDevices(CL_DEVICE_TYPE_CPU, &devices);
	if (devices.size() == 0) return false;
	auto device = devices.front();

	return device;
}

cl::Kernel getKernel(std::string fileName, const char functionName[], cl::Device* device, cl::Context* context) {

	cl_int err;
	std::ifstream exampleKernel(fileName);
	std::string src(std::istreambuf_iterator<char>(exampleKernel), (std::istreambuf_iterator<char>()));

	cl::Program::Sources sources(1, std::make_pair(src.c_str(), src.length() + 1));

	cl::Program program(*context, sources);

	err = program.build("-cl-std=CL1.2");
	cl::Kernel kernel(program, functionName, &err);

	return kernel;
}

int main(void)
{
	cl_int err;
	cl::Device device = getDevice(0);
	cl::Context context(device);
	cl::Kernel kernel = getKernel("OpenCL1.cl", "arrayMultiply", &device, &context);

	std::vector<int> vec(1000000, 1);

	cl::Buffer inBuf(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, sizeof(int)*vec.size(), vec.data());
	cl::Buffer outBuf(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, sizeof(int) * vec.size());

	kernel.setArg(0, inBuf);
	kernel.setArg(1, outBuf);

	cl::CommandQueue queue(context, device, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
	
	clock_t begin = clock();
	queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(vec.size()));
	cl::finish();
	clock_t end = clock();
	queue.enqueueReadBuffer(outBuf, CL_FALSE, 0, sizeof(int) * vec.size(), vec.data());
	cl::finish();

	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

	std::cout << "Vrijeme graficke kartice " << elapsed_secs << std::endl;
	
	begin = clock();
	for (int i = 0; i < 1000000; i++) {
		vec[i] = vec[i]*2;
	}
	end = clock();
	elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	std::cout << "Vrijeme CPU " << elapsed_secs << std::endl;
	std::cin.get();
}

