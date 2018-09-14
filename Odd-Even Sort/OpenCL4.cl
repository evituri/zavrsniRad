__kernel void odd(__global int* data)
{
	size_t id = get_global_id(0)*2;
	int temp;
	if(get_global_id(0)<=(get_global_size(0)-1)){
		if(data[id] > data[id+1]){
			temp = data[id];
			data[id] = data[id+1];
			data[id+1] = temp;
		}
	}
}

__kernel void even(__global int* data)
{
	size_t id = get_global_id(0)*2+1;
	int temp;
	if(get_global_id(0)<=(get_global_size(0)-2)){
		if(data[id] > data[id+1]){
			temp = data[id];
			data[id] = data[id+1];
			data[id+1] = temp;
		}
	}
}