#include <Windows.h>
#include "nvapi.h"
int main() {
	
	SetConsoleTitleA("Nvidia Api");

	nvapi::pub::nvmlInit_v2();

	uint64_t handle;
	nvapi::pub::nvmlDeviceGetHandleByIndex_v2(&handle);

	std::string uuid;
	nvapi::pub::nvmlDeviceGetUUID((uint64_t*)handle, uuid);

	std::string devname;
	nvapi::pub::nvmlDeviceGetName((uint64_t*)handle, devname);

	printf("LEAKED: [%s]  -> %s\n", devname.c_str(),  uuid.c_str() );

	system("pause");
	return 0;
}