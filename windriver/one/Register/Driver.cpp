#include "Driver.h"

// 
// VOID DisplayItsProcessName()
// {
// 	PEPROCESS pEProcess = PsGetCurrentProcess();
// 
// 	PTSTR ProcessName = (PTSTR)((ULONG)pEProcess + 0x174);
// 	KdPrint(("%s \n", ProcessName));
// }
// 

#define  MY_REG_SOFTWARE_KEY_NAME		L"\\Registry\\Machine\\Software\\Zhangfan"

VOID TestCreateKey()
{
	HANDLE KeyHandle;

	UNICODE_STRING RegisterUnicode;

	RtlInitUnicodeString(&RegisterUnicode, MY_REG_SOFTWARE_KEY_NAME)

	ZwCreateKey(&KeyHandle,)
}


#pragma INITCODE
/**
 * DriverEntry函数由内核中的I/O管理器负责调用，其函数有两个参数:pDriverObject和pRegistryPath
 * pDriverObject:是I/O管理器传进来的驱动对象;
 * pRegistryPath:是一个Unicode字符串,指向此驱动负责的注册表
 *
 *
 */
extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPath)
{
	NTSTATUS status;
	

	/** 
	 * 驱动程序向Windows的I/O管理器注册一些回调函数。
	 * 回调函数是由程序员定义的函数，这些函数不是由驱动程序本身负责调用，而是由操作系统负责调用。
	 * 程序员将这些函数的入口地址告诉操作系统，操作系统会在适当的时候调用这些函数。
	 *
	 */
	KdPrint(("Enter DriverEntry 1 \n"));
	pDriverObject->DriverUnload = HelloDDKUnload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = HelloDDKDispatchRoutine;

	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = HelloDDKDispatchRoutine;

	pDriverObject->MajorFunction[IRP_MJ_READ] = HelloDDKDispatchRoutine;

	status = CreateDevice(pDriverObject);

	/*DisplayItsProcessName();*/
	KdPrint(("DriverEntry end \n"));
	return status;
}


#pragma INITCODE
NTSTATUS CreateDevice(IN PDRIVER_OBJECT pDriverObject)
{
	NTSTATUS status;
	PDEVICE_OBJECT pDevObj;
	PDEVICE_EXTENSION pDevExt;

	KdPrint(("Enter CreateDevice 1 \n"));

	/**
	 * 构造一个Unicode字符串，此字符串用来存储此设备对象的名称。
	 */
	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, L"\\Device\\MyDDKDevice");

	/** 
	 * 用IoCreateDevice函数创建一个设备对象。
	 */
	status = IoCreateDevice(pDriverObject,
		sizeof(PDEVICE_EXTENSION),
		&(UNICODE_STRING)devName,
		FILE_DEVICE_UNKNOWN,
		0, TRUE, &pDevObj);

	if(!NT_SUCCESS(status))
		return status;

	pDevObj->Flags |= DO_BUFFERED_IO;
	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	pDevExt->pDevice = pDevObj;
	pDevExt->ustrDeviceName = devName;

	/** 
	 * 创建符号链接。驱动程序虽然有了设备名称，但是这种设备名称只能在内核态可见，而对于应用程序是不可见的。
	 * 因此，驱动需要暴露一个符号链接，该链接指向真正的设备名称。
	 */
	UNICODE_STRING symLinkName;
	RtlInitUnicodeString(&symLinkName, L"\\??\\HelloDDK");
	pDevExt->ustrSymLinkName = symLinkName;
	status = IoCreateSymbolicLink(&symLinkName, &devName);
	if(!NT_SUCCESS(status))
	{
		IoDeleteDevice(pDevObj);
		return status;
	}
	KdPrint(("Enter CreateDevice 2 \n"));

	return STATUS_SUCCESS;
}

/** 
 * 负责卸载驱动程序
 */
#pragma PAGEDCODE
VOID HelloDDKUnload(IN PDRIVER_OBJECT pDriverObject)
{

	PDEVICE_OBJECT pNextObj;
	KdPrint(("Enter DriverUnload 1 \n"));

	// 由驱动对象得到设备对象
	pNextObj = pDriverObject->DeviceObject;

	while(NULL != pNextObj)
	{
		PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pNextObj->DeviceExtension;

		// 删除设备对象的符号链接
		UNICODE_STRING pLinkName = pDevExt->ustrSymLinkName;
		IoDeleteSymbolicLink(&pLinkName);
		pNextObj = pNextObj->NextDevice;
		IoDeleteDevice(pDevExt->pDevice);
	}
}

#pragma PAGEDCODE
/** 
 * 对设备对象的创建、关闭和读写操作，都被指定到这个默认的派遣例程中。
 *
 */
NTSTATUS HelloDDKDispatchRoutine(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{

	KdPrint(("Enter HelloDDKDispatchRoutine 1 \n"));
	NTSTATUS status = STATUS_SUCCESS;

	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;

	// 只是完成此IRP
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	KdPrint(("Leave HelloDDKDispatchRountine 1 \n"));

	return status;
}

