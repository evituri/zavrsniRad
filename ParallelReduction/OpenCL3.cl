__kernel void withoutBarriers(__global int* data, __global int* outData)
{
	size_t arrayId = get_global_id(1)*get_global_size(0)+get_global_id(0);

	for(int i=(get_global_size(0)*get_global_size(1)) >> 1;i>0;i>>=1){
		if(arrayId < i){
			data[arrayId] += data[arrayId + i];
		}
		barrier(CLK_GLOBAL_MEM_FENCE);
	}
	if(arrayId == 0){
		outData[0] = data[0];
	}
}