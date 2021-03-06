// OpenCL3.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#include <stdlib.h>
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

bool isSorted(int* vec, int elements) {
	for (int i = 1; i < elements; i++) {
		if (vec[i] < vec[i - 1]) {
			return false;
		}
	}
	return true;
}

void swap(int* a, int* b)
{
	int t = *a;
	*a = *b;
	*b = t;
}

// A function to implement bubble sort
void bubbleSort(int arr[], int n)
{
	int i, j;
	for (i = 0; i < n - 1; i++)

		// Last i elements are already in place   
		for (j = 0; j < n - i - 1; j++)
			if (arr[j] > arr[j + 1])
				swap(&arr[j], &arr[j + 1]);
}

int partition(int arr[], int low, int high)
{
	int pivot = arr[high];
	int i = (low - 1); 

	for (int j = low; j <= high - 1; j++)
	{
		if (arr[j] <= pivot)
		{
			i++;
			swap(&arr[i], &arr[j]);
		}
	}
	swap(&arr[i + 1], &arr[high]);
	return (i + 1);
}

void quickSort(int arr[], int low, int high)
{
	if (low < high)
	{

		int pi = partition(arr, low, high);
		quickSort(arr, low, pi - 1);
		quickSort(arr, pi + 1, high);
	}
}


int main(void)
{
	cl_int err;
	cl::Device device = getDevice(0);
	cl::Context context(device);
	cl::Kernel kernelEven = getKernel("OpenCL4.cl", "even", &device, &context);
	cl::Kernel kernelOdd = getKernel("OpenCL4.cl", "odd", &device, &context);

	const int elements = 10000;

	int *vec = new int[elements];
	int *vecCPU = new int[elements];
	int *vecCPU2 = new int[elements];
	for (int i = 0; i < elements; i++) {
		vec[i] = rand() % elements + 1;
	}

	memcpy(vecCPU, vec, elements * sizeof(int));
	memcpy(vecCPU2, vec, elements * sizeof(int));

	cl::Buffer inBuf(context, CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int)*elements, vec);

	kernelEven.setArg(0, inBuf);
	kernelOdd.setArg(0, inBuf);


	cl::CommandQueue queue(context, device);

	clock_t begin = clock();
	for (int i = 0; i < elements; i++) {
		if (i % 2) {
			err = queue.enqueueNDRangeKernel(kernelEven, cl::NullRange, cl::NDRange(elements / 2));
		}
		else {
			err = queue.enqueueNDRangeKernel(kernelOdd, cl::NullRange, cl::NDRange(elements / 2));
		}
	}
	
	cl::finish();
	clock_t end = clock();
	err = queue.enqueueReadBuffer(inBuf, CL_TRUE, 0, sizeof(int)*elements, vec);
	cl::finish();


	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

	std::cout << "Vrijeme graficke kartice: " << elapsed_secs << std::endl;
	//std::cout << "Izracun graficke kartice: " << isSorted(vec, elements) << std::endl;

	begin = clock();
	quickSort(vecCPU, 0, elements - 1);
	end = clock();
	elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	std::cout << "Vrijeme CPU(quick sort): " << elapsed_secs << std::endl;
	//std::cout << "Izracun CPU(quick sort): " << isSorted(vec, elements) << std::endl;

	begin = clock();
	bubbleSort(vecCPU2, elements);
	end = clock();
	elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	std::cout << "Vrijeme CPU(bubble sort): " << elapsed_secs << std::endl;
	//std::cout << "Izracun CPU(bubble sort): " << isSorted(vec, elements) << std::endl;
	delete[] vec;
}

