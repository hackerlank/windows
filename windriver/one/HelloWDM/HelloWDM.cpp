#include "HelloWDM.h"

#pragma INITCODE  //将此函数放在INIT段中，当驱动加载结束后，此函数就可以从内存中卸载掉

/** 
 * 初始化驱动程序，定位和申请硬件资源，创建内核对象
 * pDriverObject:从I/O管理器中传进来的驱动对象
 * pRegistryPath:驱动程序在注册表中的位置
 *
 */
extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPath)
{

	KdPrint(("Enter DriverEntery \n"));

	pDriverObject->DriverExtension->AddDevice = HelloWDMAddDevice;  // 设置AddDevice回调函数，此回调函数只出现在WDM驱动程序中，
	// 而在NT式的驱动中没有此回调函数。此回调函数的作用是创建设备对象并由PNP(即插即用)管理器调用。

	pDriverObject->MajorFunction[IRP_MJ_PNP] = HelloWDMPnp; // 设置IRP_MJ_PNP的IRP的回调函数，对PNP的IRP处理，是NT式驱动和WDM驱动的重大区别

	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HelloWDMDispatchRoutine; // 设置常用IRP函数，这里简单的指向默认函数
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = HelloWDMDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_READ] = HelloWDMDispatchRoutine;

	pDriverObject->DriverUnload = HelloWDMUnload;  // 向系统注册卸载例程。在WDM驱动中，大部分卸载工作已不在此处理，
	// 而是放在IRP_MN_REMOVE_DEVICE的IRP的处理函数中处理



	KdPrint(("Leaver DriverEntery\n"));
	return STATUS_SUCCESS;
}


/** 
 * 添加新设备
 * DriverObject:从I/O管理器中传进来的驱动对象
 * PhysicalDeviceObject:从I/O管理器中传进来的物理设备对象
 *
 * 在WDM的驱动程序中，创建设备对象的任务不再由DriverEntry承担，而需要驱动程序向系统注册一个称做AddDevice的例程。
 * 此例程由PNP管理器负责调用，其函数主要职责是创建设备对象
 * HelloWDMAddDevice例程有两个参数，DriverObject和PhysicalDeviceObject。
 * DriverObject是有PNP管理器传递进来的驱动对象，此对象是DriverEntry中的驱动对象。
 * PhysicalDeviceObject是PNP管理器传递进来的底层驱动设备对象。
 */
#pragma PAGEDCODE  // 用#pragma 指明将此例程分配在分页内存中
NTSTATUS HelloWDMAddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT PhysicalDeviceObject)
{
	PAGED_CODE();  //  PAGED_CODE是一个DDK提供的宏，只在check版中有效。当此例程所在的中段请求级超过APC_LEVEL时，
	               //  会产生一个断言，断言会使程序终止，并报告出差地址。

	KdPrint(("Enter HelloEDMAddDevice\n"));

	NTSTATUS status;
	PDEVICE_OBJECT fdo;
	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, L"\\Device\\MyWDMDevice");

	status = IoCreateDevice(DriverObject,  // 创建设备对象
		sizeof(DEVICE_EXTENSION),
		&(UNICODE_STRING)devName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&fdo);
	if(!NT_SUCCESS(status))
		return status;

	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;  // 得到设备对象扩展数据结构
	pdx->fdo = fdo; // 记录设备扩展中的功能设备对象为其刚才创建的设备

	pdx->NextStackDevice = IoAttachDeviceToDeviceStack(fdo, PhysicalDeviceObject);
	// 用IoAttachDeviceToDeviceStack函数将此fdo(功能设备对象)挂接在设备堆栈上，并将返回值(下层堆栈的位置)，记录在设备扩展结构中。

	UNICODE_STRING symLinkName;
	RtlInitUnicodeString(&symLinkName, L"\\DosDevices\\HelloWDM");
	pdx->ustrDeviceName = devName;
	pdx->ustrSymLinkName = symLinkName; // 创建设备的符号链接

	status = IoCreateSymbolicLink(&(UNICODE_STRING)symLinkName, &(UNICODE_STRING)devName);

	if(!NT_SUCCESS(status))
	{
		IoDeleteSymbolicLink(&pdx->ustrSymLinkName);
		status = IoCreateSymbolicLink(&symLinkName, &devName);
		if(!NT_SUCCESS(status))
			return status;
	}

	fdo->Flags |= DO_BUFFERED_IO | DO_POWER_PAGABLE;  // 设置设备为BUFFERED_IO设备，并指明驱动初始化完成
	fdo->Flags &= ~DO_DEVICE_INITIALIZING;
	KdPrint(("Leave HelloWDMAddDevice\n"));
	return STATUS_SUCCESS;
}



/** 
 * 对即插即用IRP进行处理
 * fdo:功能设备对象                  Irp:从I/O请求包
 *
 */
#pragma PAGEDCODE
NTSTATUS HelloWDMPnp(IN PDEVICE_OBJECT fdo, IN PIRP Irp)
{
	PAGED_CODE();  // 用PAGED_CODE宏确保该例程运行在低于APC_LEVEL的中断优先级的级别上
	KdPrint(("Enter HelloWDMPnp\n"));
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;  // 得到设备扩展结构
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);  // 得到当前IRP的堆栈
	static NTSTATUS (*fcntab[])(PDEVICE_EXTENSION pdx, PIRP Irp) = 
	{
		DefaultPnpHandler, DefaultPnpHandler, DefaultPnpHandler, DefaultPnpHandler,
		DefaultPnpHandler, DefaultPnpHandler, DefaultPnpHandler, DefaultPnpHandler,
		DefaultPnpHandler, DefaultPnpHandler, DefaultPnpHandler, DefaultPnpHandler,
		DefaultPnpHandler, DefaultPnpHandler, DefaultPnpHandler, DefaultPnpHandler,
	};
	ULONG fcn = stack->MinorFunction;
	if(fcn >= arraysize(fcntab))
	{
		status = DefaultPnpHandler(pdx, Irp);
		return status;
	}
#if DBG
	static char *fcnname[] = 
	{
		"IRP_MN_START_DEVICE",
		"IRP_MN_QUERY_REMOVE_DEVICE",
		"IRP_MN_REMOVE_DEVICE"
	}
	KdPrint(("PNP Request"));
#endif

	status = (*fcntab[fcn])(pdx, Irp);
	KdPrint(("Leave HelloWDMPnp\n"));
	return status;
}



/** 
 * 除了IRP_MN_STOP_DEVICE以外，HelloWDM对其他PNP的IRP做同样的处理，即直接传递到底层驱动，并将底层驱动的结果返回。
 * 
 * 对PNP IRP进行默认处理
 * pdx:设备对象扩展    Irp:从I/O请求包
 *
 */
#pragma PAGEDCODE
NTSTATUS DefaultPnpHandler(PDEVICE_EXTENSION pdx, PIRP Irp)
{
	PAGED_CODE();  // 确保该例程处于APC_LEVEL之下
	KdPrint(("Enter DefaultPnpHandler\n"));
	IoSkipCurrentIrpStackLocation(Irp);  // 略过当前堆栈
	KdPrint(("Leave DefaultPnpHandler\n"));
	return IoCallDriver(pdx->NextStackDevice, Irp); // 用下层堆栈的驱动设备对象处理此IRP
}



/** 
 * 对IRP_MN_REMOVE_DEVICE IRP进行处理
 *
 * fdo:功能设备对象        Irp:从I/O请求包
 */
#pragma PAGEDCODE
NTSTATUS HandleRemoveDevice(PDEVICE_EXTENSION pdx, PIRP Irp)
{
	PAGED_CODE();
	KdPrint(("Enter HandleRemoveDevice\n"));
	Irp->IoStatus.Status = STATUS_SUCCESS; // 设置此IRP的状态为顺利完成
	NTSTATUS status = DefaultPnpHandler(pdx, Irp);  // 调用默认的PNP的IRP的处理函数
	IoDeleteSymbolicLink(&(UNICODE_STRING)pdx->ustrSymLinkName);  // 删除此设备对象的符号链接

	// 调用IoDetachDevice()把fdo从设备栈中脱开
	if(pdx->NextStackDevice)
		IoDetachDevice(pdx->NextStackDevice);

	// 删除fdo
	IoDeleteDevice(pdx->fdo);   // 删除设备对象
	KdPrint(("Leave HandleRemoveDevice\n"));
	return status;
}

/** 
 * 对默认IRP进行处理
 * fdo:功能设备对象    Irp:从I/O请求包
 *
 */
#pragma PAGEDCODE
NTSTATUS HelloWDMDispatchRoutine(IN PDEVICE_OBJECT fdo, IN PIRP Irp)
{
	PAGED_CODE();
	KdPrint(("Enter HelloWDMDispatchRoutine\n"));
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;  // no bytes xfered
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	KdPrint(("Leave HelloWDMDispatchRoutine\n"));
	return STATUS_SUCCESS;
}



/** 
 * 负责驱动程序的卸载
 * DriverObject:驱动对象
 *
 */
#pragma PAGEDCODE
void HelloWDMUnload(IN PDRIVER_OBJECT DriverObject)
{
	PAGED_CODE();
	KdPrint(("Enter HelloWDMUnload\n"));
	KdPrint(("Leave HelloWDMUnload\n"));
}