// OpenCL2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#include <iostream>
#include <CL/cl.hpp>
#include <array>
#include <fstream>
#include <ctime>

cl::Device getDevice(int type) {
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	std::vector<cl::Device> devices;

	if (platforms.size() == 0) return false;
	auto platform = platforms.front();
	
	if(type==0) platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
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
	cl::Kernel kernel = getKernel("OpenCL2.cl", "barriers", &device, &context);
	cl::Kernel kernel2 = getKernel("OpenCL2.cl", "barriers", &device, &context);
	cl::Kernel kernel3 = getKernel("OpenCL2.cl", "barriers", &device, &context);
	cl::Kernel kernel4 = getKernel("OpenCL3.cl", "withoutBarriers", &device, &context);
	
	const int rows = 8192;
	const int cols = 8192;
	const int totalElements = rows * cols;
	//std::array<std::array<int, cols>, rows> vec;
	//std::vector< std::vector<int> > vec(cols, std::vector<int>(rows));
	int *vec = new int[cols * rows];
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			vec[i*rows+j] = 1;
		}
	}
	
	auto workGroupSize = kernel.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(device, &err);
	const int outVecCols = sqrt(workGroupSize);
	const int outVecRows = sqrt(workGroupSize);

	int countWorkGroups = totalElements / workGroupSize;
	cl::Buffer inBuf(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, sizeof(int)*totalElements, vec);
	cl::Buffer outBuf(context, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(int) * countWorkGroups);
	cl::Buffer outBufSecond(context, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(int) * (countWorkGroups / workGroupSize));
	cl::Buffer outBufThird(context, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, sizeof(int) * (countWorkGroups / workGroupSize / workGroupSize));
	cl::Buffer outBufFinal(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, sizeof(int));

	int *finalNumber = new int;
	kernel.setArg(0, inBuf);
	kernel.setArg(1, sizeof(int)*workGroupSize, nullptr);
	kernel.setArg(2, outBuf);
	kernel2.setArg(0, outBuf);
	kernel2.setArg(1, sizeof(int)*workGroupSize, nullptr);
	kernel2.setArg(2, outBufSecond);
	kernel3.setArg(0, outBufSecond);
	kernel3.setArg(1, sizeof(int)*workGroupSize, nullptr);
	kernel3.setArg(2, outBufThird);
	kernel4.setArg(0, outBufThird);
	kernel4.setArg(1, outBufFinal);

	cl::CommandQueue queue(context, device);

	clock_t begin = clock();
	err = queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(cols, rows), cl::NDRange(outVecCols, outVecRows));
	err = queue.enqueueNDRangeKernel(kernel2, cl::NullRange, cl::NDRange((cols / outVecCols), (rows / outVecRows)), cl::NDRange(outVecCols, outVecRows));
	err = queue.enqueueNDRangeKernel(kernel3, cl::NullRange, cl::NDRange((cols / outVecCols / outVecCols), (rows / outVecRows / outVecRows)), cl::NDRange(outVecCols, outVecRows));
	err = queue.enqueueNDRangeKernel(kernel4, cl::NullRange, cl::NDRange((cols / outVecCols / outVecCols / outVecCols), (rows / outVecRows / outVecRows / outVecRows)));
	cl::finish();
	clock_t end = clock();
	err = queue.enqueueReadBuffer(outBufFinal, CL_TRUE, 0, sizeof(int), finalNumber);
	cl::finish();


	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

	std::cout << "Vrijeme graficke kartice: " << elapsed_secs << std::endl;
	std::cout << "Izracun graficke kartice: " << *finalNumber << std::endl << std::endl;

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			vec[i*rows + j] = 1;
		}
	}

	begin = clock();
	int sum = 0;
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			sum += vec[i*rows+j];
		}
	}
	end = clock();
	elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	std::cout << "Vrijeme CPU " << elapsed_secs << std::endl;
	std::cout << "Izracun CPU: " << sum << std::endl;
	delete[] vec;
}