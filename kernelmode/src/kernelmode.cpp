#include <ntifs.h>


extern "C" {
	// Manually declare the functions we want to use from ntoskrnl.exe
	NTKERNELAPI NTSTATUS IoCreateDriver(PUNICODE_STRING DriverName, PDRIVER_INITIALIZE InitializationFunction);
	NTKERNELAPI NTSTATUS MmCopyVirtualMemory(PEPROCESS SourceProcess, PVOID SourceAddress, PEPROCESS TargetProcess, PVOID TargetAddress, SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode, PSIZE_T ReturnSize);
}

void DebugPrint(PCSTR text) {
	// Convenient function to print debug messages to the kernel debugger
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, text));
}


namespace driver {
	namespace ioctlcodes {
		constexpr ULONG attach = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x123, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
		constexpr ULONG ReadMemory = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x124, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
		constexpr ULONG WriteMemory = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x125, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
	}
	struct Request {
		HANDLE process_id;
		PVOID target;
		PVOID buffer;
		SIZE_T size;
		SIZE_T returned_size;
	};
	NTSTATUS create(PDEVICE_OBJECT device_object, PIRP irp) {
		UNREFERENCED_PARAMETER(device_object);
		IoCompleteRequest(irp, IO_NO_INCREMENT);
		return irp->IoStatus.Status;
	}
	NTSTATUS close(PDEVICE_OBJECT device_object, PIRP irp) {
		UNREFERENCED_PARAMETER(device_object);
		IoCompleteRequest(irp, IO_NO_INCREMENT);
		return irp->IoStatus.Status;
	}
    NTSTATUS device_control(PDEVICE_OBJECT device_object, PIRP irp) {
		DebugPrint("Device Control called\n");
        UNREFERENCED_PARAMETER(device_object);
        NTSTATUS status = STATUS_UNSUCCESSFUL;
        auto stack_irp = IoGetCurrentIrpStackLocation(irp);
        auto request =
            reinterpret_cast<Request *>(irp->AssociatedIrp.SystemBuffer);

        if (stack_irp == nullptr || request == nullptr) {
          IoCompleteRequest(irp, IO_NO_INCREMENT);
          return status;
        }
        static PEPROCESS target_process = nullptr;
        const ULONG control_code = stack_irp->Parameters.DeviceIoControl.IoControlCode;
        switch (control_code) {
			case ioctlcodes::attach: {
			  status = PsLookupProcessByProcessId(request->process_id, &target_process);
			  break;
			}
			case ioctlcodes::ReadMemory:{
			  if (target_process != nullptr)
				status = MmCopyVirtualMemory(target_process, request->target,PsGetCurrentProcess(), request->buffer,request->size, KernelMode,&request->returned_size);
			  break;
			}
			case ioctlcodes::WriteMemory: {
			  if (target_process != nullptr) 
				  status = MmCopyVirtualMemory(PsGetCurrentProcess(), request->buffer,target_process, request->target,request->size, KernelMode,&request->returned_size);
			  break;
			}
			default:{
				status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}   
        }
		irp->IoStatus.Status = status;
		irp->IoStatus.Information = sizeof(Request);
        IoCompleteRequest(irp, IO_NO_INCREMENT);
        return status;


        }


}

NTSTATUS driver_main(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path) {
	UNREFERENCED_PARAMETER(registry_path);
	// Actual "main" function for the driver

	//DebugPrint("Driver Loaded\n");

	UNICODE_STRING device_name = { 0 };
	RtlInitUnicodeString(&device_name, L"\\Device\\KernelMode");


	PDEVICE_OBJECT device_object = nullptr;
	auto status = IoCreateDevice(driver_object, 0, &device_name, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &device_object);

	if (!NT_SUCCESS(status)) {
		DebugPrint("Failed to create device\n");
		return status;
	}

	DebugPrint("Device Created\n");
	UNICODE_STRING symbolic_link = { 0 };
	RtlInitUnicodeString(&symbolic_link, L"\\DosDevices\\KernelMode");

	status = IoCreateSymbolicLink(&symbolic_link, &device_name);
	if (!NT_SUCCESS(status)) {
		DebugPrint("Failed to create symbolic link\n");
		IoDeleteDevice(device_object);
		return status;
	}

	DebugPrint("Symboliclink Created\n");
	SetFlag(device_object->Flags, DO_BUFFERED_IO);

	driver_object->MajorFunction[IRP_MJ_CREATE] = driver::create;
	driver_object->MajorFunction[IRP_MJ_CLOSE] = driver::close;
	driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = driver::device_control;

	ClearFlag(device_object->Flags, DO_DEVICE_INITIALIZING);

	DebugPrint("Driver Initialized\n");

	return status;
}







NTSTATUS DriverEntry() {
	// This is the FIRST entry point. This is made like this, so it can be manually mapped
	//DebugPrint("Driver Loaded\n");
	//UNICODE_STRING driver_name = { 0 };
	//RtlInitUnicodeString(&driver_name, L"\\Driver\\KernelMode");
	//return IoCreateDriver(&driver_name, &driver_main);
	// Purpusely commenting this out, so the potential kid trying to load this isnt BSODing himself
	DebugPrint("...");
	return STATUS_SUCCESS;
}
