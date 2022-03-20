#include "nvapi.h"

uint32_t* nvapi::nvidia_buffer;

void nvapi::pub::nvmlInitWithFlags(int flags) {

	nvapi::nvidia_buffer = (uint32_t*)malloc(0xB5D66 * 4);

	memset((void*)nvapi::nvidia_buffer, 0, sizeof(0xB5D66 * 4));

	//DeviceIoNotNeeded
	if (!internal::collect_adapters())
		return;
	internal::init_variables();

}

void nvapi::pub::nvmlInit_v2()
{
	pub::nvmlInitWithFlags(0);
}

bool nvapi::pub::nvmlDeviceGetHandleByIndex_v2(uintptr_t* out) {
	uint64_t address = (uintptr_t)nvapi::nvidia_buffer;
	*out = address;
	return internal::prepare_handle((uint32_t*)(address + 0x38));
}

void nvapi::pub::nvmlDeviceGetUUID(uint64_t* adapter, std::string& uuid) {

	internal::get_uuid_internal(adapter, uuid);

}

void nvapi::pub::nvmlDeviceGetName(uint64_t* adapter, std::string& name) {

	internal::get_name_internal(adapter, name);

}