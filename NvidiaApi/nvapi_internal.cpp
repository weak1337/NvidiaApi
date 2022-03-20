#include "nvapi.h"

struct query_basic_adapter {
	uint32_t signature;
	uint32_t size;
	uint32_t command;
	uint32_t unk3;
	uint8_t unkbuffer[0xA391];
};

bool nvapi::internal::collect_adapters() {
	printf("ENTERING %s\n", __FUNCTION__);
	D3DKMT_ENUMADAPTERS2 adapters = { 0 };

	D3DKMTEnumAdapters2(&adapters);

	if (!adapters.NumAdapters)
	{
		printf("Couldn't find any adapters!\n");
		return false;
	}

	adapters.pAdapters = (D3DKMT_ADAPTERINFO*)malloc(adapters.NumAdapters + 0x14);

	if (D3DKMTEnumAdapters2(&adapters))
	{
		printf("D3DKMTEnumAdapters2 failed!\n");
		return false;
	}
	
	for (int i = 0; i < adapters.NumAdapters - 1; i++) {
		nvidia_adapters[i].adapter_handle = adapters.pAdapters[i].hAdapter;
	}
	return true;

}

NTSTATUS nvapi::internal::escape(uint32_t main_adapter, D3DKMT_ESCAPETYPE type, uint32_t size, uintptr_t private_data) {

	D3DKMT_ESCAPE data = { 0 };
	data.hAdapter = main_adapter;
	data.Type = type;
	data.pPrivateDriverData = (void*)private_data;
	data.PrivateDriverDataSize = size;

	return D3DKMTEscape(&data);

}

int nvapi::internal::make_call(int handle, int handle2, int id, uint64_t* argbuffer, int buffersize) {
	DWORD* buffer = (DWORD*)malloc(buffersize + 0x44);
	memset(buffer, 0, buffersize + 0x44);
	buffer[0] = 0x4E564441;
	buffer[1] = 0x10002;
	buffer[2] = buffersize + 0x44;
	buffer[3] = 0x4E562A2A;
	buffer[4] = 0x500002B;
	buffer[12] = handle;
	buffer[13] = handle2;
	buffer[14] = id;
	buffer[15] = buffersize;


	memcpy((void*)((uintptr_t)buffer + 0x44), (void*)argbuffer, buffersize);

	NTSTATUS status = internal::escape(nvapi::nvidia_adapters[0].adapter_handle, D3DKMT_ESCAPE_DRIVERPRIVATE, buffersize + 0x44, (uintptr_t)buffer);
	if (status)
		return status;
	memcpy((void*)argbuffer, (void*)((uintptr_t)buffer + 0x44), buffersize);

	return buffer[16];
}

int nvapi::internal::make_call2(int adapter, int handle, int val1, int val2, int id, uint64_t* argbuffer, int buffersize) {
	DWORD* buffer = (DWORD*)malloc(buffersize + 0x48);
	memset(buffer, 0, buffersize + 0x48);
	buffer[0] = 0x4E564441;
	buffer[1] = 0x10002;
	buffer[2] = buffersize + 0x48;
	buffer[3] = 0x4E562A2A;
	buffer[4] = 0x500002A;
	buffer[12] = handle;
	buffer[13] = val1;
	buffer[14] = val2;
	buffer[15] = id;
	buffer[16] = buffersize;

	memcpy((void*)((uintptr_t)buffer + 0x48), (void*)argbuffer, buffersize);
	NTSTATUS status = internal::escape(nvapi::nvidia_adapters[0].adapter_handle, D3DKMT_ESCAPE_DRIVERPRIVATE, buffersize + 0x48, (uintptr_t)buffer);
	if (status)
		return status;

	memcpy((void*)argbuffer, (void*)((uintptr_t)buffer + 0x48), buffersize);

	return buffer[17];
}

struct query_private_handle {
	uint32_t unk1;
	uint32_t unk2;
	uint32_t unk3;
	uint32_t unk4;
	uint32_t unk5;
	uint32_t unk6;
	uint32_t unk7;
	uint32_t unk8;
	uint32_t unk9;
	uint32_t unk10;
	uint32_t unk11;
	uint32_t unk12;
	uint32_t unk13;
	uint32_t unk14;
	uint32_t unk15;
	uint32_t unk16;
	uint32_t unk17;
	uint32_t unk18;
	uint32_t unk19;

};

int retrieve_private_handle(uint32_t* handle) {

	query_private_handle query_private = { 0 };
	query_private.unk1 = 0x4E564441;
	query_private.unk2 = 0x10002;
	query_private.unk3 = 0x4C;
	query_private.unk4 = 0x4E562A2A;
	query_private.unk5 = 0x500002A;
	query_private.unk16 = 0x41;
	query_private.unk17 = 4;

	if (nvapi::internal::escape(nvapi::nvidia_adapters[0].adapter_handle, D3DKMT_ESCAPE_DRIVERPRIVATE, 0x4C, (uintptr_t)&query_private))
		return 1;
	*handle = query_private.unk19;
	return query_private.unk18;


}




bool nvapi::internal::init_variables() {
	
	int error = retrieve_private_handle(&nvapi::private_nvidia_handle);
	
	if (error)
	{
		printf("Couldn't retrieve private handle!\n");
		return false;
	}
	return true;
}

uint32_t global_index = 0xA55A0000;

void get_index(uint32_t* idx) {
	uint32_t current = global_index;
	global_index += 0x10;
	*idx = current;
	return;
}

bool nvapi::internal::prepare_handle(uint32_t* adapterinfo) {
	printf("ENTERING : %s\n", __FUNCTION__);

	if (!adapterinfo[4]) {
		adapterinfo[3] = 1;
		adapterinfo[4] = 1;
	}

	uint32_t init = adapterinfo[6];
	if (!init) {

		if (!adapterinfo || !adapterinfo[4] || adapterinfo[6] || !adapterinfo[3])
			return false;

		uint32_t idx;
		get_index(&idx);
		uint32_t buffer[0x40 / 4] = { 0 };
		
		uint32_t error = 0;
		if (error = internal::make_call2(nvapi::nvidia_adapters[0].adapter_handle, nvapi::private_nvidia_handle, nvapi::private_nvidia_handle, idx, 0x80, (uintptr_t*)buffer, 0x40))
		{
			printf("make_call2 failed with error : %x\n", error);
			return false;
		}

		uint32_t result;
		if (error = internal::make_call(nvapi::private_nvidia_handle, idx, 0x800289, (uint64_t*)&result, 4))
		{
			printf("make_call failed with error : %x\n", error);
			return false;
		}
	}
	
	return true;
}



void nvapi::internal::get_uuid_internal(uint64_t* adapter, std::string& uuid) {
	printf("ENTERING : %s\n", __FUNCTION__);
	uint32_t idx;
	get_index(&idx);
	DWORD old = idx - 0x10;
	DWORD idk = 0;
	uint32_t error = 0;
	if (error = internal::make_call2(nvapi::nvidia_adapters[0].adapter_handle, nvapi::private_nvidia_handle, old, idx, 0x2080, (uintptr_t*)&idk, 4))
	{
		printf("make_call2 failed with error : %x\n", error);
		return;
	}

	uint32_t buffer[0x10C / 4] = { 0 };

	if (error = internal::make_call(nvapi::private_nvidia_handle, idx, 0x2080014A, (uintptr_t*)&buffer, 0x10C))
	{
		printf("make_call2 failed with error : %x\n", error);
		return;
	}
	
	uuid = (char*)(&buffer[3]);

}

void nvapi::internal::get_name_internal(uint64_t* adapter, std::string& name) {
	printf("ENTERING : %s\n", __FUNCTION__);
	uint32_t buffer[0x10C / 4] = { 0 };
	uint32_t error = 0;
	if (error = internal::make_call(nvapi::private_nvidia_handle, 0xA55A0000 + 0x10, 0x20800110, (uintptr_t*)&buffer, 0x84)) //We can use previous discovered handle
	{
		printf("make_call failed with error : %x\n", error);
		return;

	}
	name = (char*)(&buffer[1]);
}