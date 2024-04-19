#include <iostream>
#include <windows.h>
#include <tlhelp32.h>



// INSERT FUNCTIONS TO GET PROCESS ID AND HANDLE HERE ++ BASEADDR

namespace driver {
    namespace ioctlcodes {
        constexpr ULONG attach = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x123, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
        constexpr ULONG ReadMemory = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x124, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
        constexpr ULONG WriteMemory = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x125, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
    } // namespace ioctlcodes
    struct Request {
      HANDLE process_id;
      PVOID target;
      PVOID buffer;
      SIZE_T size;
      SIZE_T returned_size;
    };
    bool attach_to_process(HANDLE driver_handle, DWORD process_id) {
		Request request;
		request.process_id = reinterpret_cast<HANDLE>(process_id);
		request.target = nullptr;
		request.buffer = nullptr;
        return DeviceIoControl(driver_handle, ioctlcodes::attach, &request, sizeof(request), &request, sizeof(request), nullptr, nullptr);
	}
    template <class T>
    T read_memory(HANDLE driver_handle, const std::uintptr_t addr) {
      T temp = {};
      Request r;
      r.target = reinterpret_cast<PVOID>(addr);
      r.buffer = &temp;
      r.size = sizeof(T);
      DeviceIoControl(driver_handle, ioctlcodes::ReadMemory, &r, sizeof(r), &r,
                      sizeof(r), nullptr, nullptr);
      return temp;
    }

    template <class T>
    void write_memory(HANDLE driver_handle, const std::uintptr_t addr,
                      const T &value) {
      Request r;
      r.target = reinterpret_cast<PVOID>(addr);
      r.buffer = (PVOID)&value;
      r.size = sizeof(T);
      DeviceIoControl(driver_handle, ioctlcodes::WriteMemory, &r, sizeof(r), &r,
                      sizeof(r), nullptr, nullptr);
    }
} // namespace driver



int main() {
    //const DWORD pid = get_process_id(L"notepad.exe");
    //const HANDLE driver = CreateFile(L"\\\\.\\KernelMode", GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    //if (driver == INVALID_HANDLE_VALUE) {
	//	std::cout << "Failed to open driver" << std::endl;
	//	std::cin.get();
	//	return 1;
	//}
    //if (driver::attach_to_process(driver, pid) == true) {
	//	std::cout << "Attached to process" << std::endl;
	//} else {
	//	std::cout << "Failed to attach to process" << std::endl;
 // }

    //CloseHandle(driver);
	//std::cin.get();
	return 0;
}