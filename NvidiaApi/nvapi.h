#pragma once
#include <Windows.h>
#include <iostream>
#include <d3dkmthk.h>
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
namespace nvapi {
	
	extern uint32_t* nvidia_buffer;

	namespace pub{
		void nvmlInit_v2();
		void nvmlInitWithFlags(int flags);
		bool nvmlDeviceGetHandleByIndex_v2(uint64_t* out);
		void nvmlDeviceGetUUID(uint64_t* adapter, std::string& uuid);
		void nvmlDeviceGetName(uint64_t* adapter, std::string& name);
	}

	struct nv_adapter_info {
		uint32_t adapter_handle;
		uint32_t adapter_private;
	};

	static nv_adapter_info nvidia_adapters[10];
	static uint32_t private_nvidia_handle;

	namespace internal {

		NTSTATUS escape(uint32_t main_adapter, D3DKMT_ESCAPETYPE type, uint32_t size, uintptr_t private_data);
		int make_call(int handle, int handle2, int id, uint64_t* argbuffer, int buffersize);
		int make_call2(int adapter, int handle, int val1, int val2, int id, uint64_t* argbuffer, int buffersize);

		bool collect_adapters();
		bool init_variables();
		bool prepare_handle(uint32_t* adapterinfo);
		void get_uuid_internal(uint64_t* adapter, std::string& uuid);
		void get_name_internal(uint64_t* adapter, std::string&name);
	}

}