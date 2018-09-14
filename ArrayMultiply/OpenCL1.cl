__kernel void arrayMultiply(__global int* data, __global int* outData)
{
	size_t tid = get_global_id(0);
	outData[tid] = data[tid] * 2;
}