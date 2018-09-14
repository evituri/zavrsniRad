__kernel void barriers(__global int* data, __local int* localData, __global int* outData)
{
	size_t arrayId = get_global_id(1)*get_global_size(0)+get_global_id(0);
	size_t localArrayId = get_local_id(1)*get_local_size(0)+get_local_id(0);

	localData[localArrayId] = data[arrayId];
	barrier(CLK_LOCAL_MEM_FENCE);

	for(int i=(get_local_size(0)*get_local_size(1)) >> 1;i>0;i>>=1){
		if(localArrayId < i){
			localData[localArrayId] += localData[localArrayId + i];
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}
	if(localArrayId == 0){
		outData[get_group_id(1)*(get_global_size(0)/get_local_size(0))+get_group_id(0)] = localData[0];
	}
}