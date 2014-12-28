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

#define BUFFER_SIZE 1024


VOID TestFileCreateOne()
{
	HANDLE FileHand;
	OBJECT_ATTRIBUTES ObjectAttributes;

	UNICODE_STRING UnicodeString;
	
	IO_STATUS_BLOCK IoStatusBlock;
	RtlInitUnicodeString(&UnicodeString, L"\\??\c:\\test1.log");

	InitializeObjectAttributes(&ObjectAttributes, &UnicodeString, OBJ_CASE_INSENSITIVE, NULL, NULL);

	NTSTATUS ntStatus = ZwCreateFile(&FileHand, GENERIC_WRITE, &ObjectAttributes, &IoStatusBlock, NULL,
		FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ, FILE_OPEN_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

	KdPrint(("%d\n", IoStatusBlock.Information));

	if(NT_SUCCESS(ntStatus))
	{
		KdPrint(("Create File Success \n"));
	}
	else
	{
		KdPrint(("Create File Error \n"));
	}

	ZwClose(FileHand);





	/**
	NTSTATUS ZwCreateFile(
		PHANDLE FileHandle,
		ACCESS_MASK DesiredAccess,
		POBJECT_ATTRIBUTES ObjectAttributes,
		PIO_STATUS_BLOCK IoStatusBlock,
		PLARGE_INTEGER AllocationSize,
		ULONG FileAttributes,
		ULONG ShareAccess,
		ULONG CreateDisposition,
		ULONG CreateOptions,
		PVOID EaBuffer,
		EaLength
		);

		*/
}


VOID TestFileCreateTwo()
{
	HANDLE FileHand;
	OBJECT_ATTRIBUTES ObjectAttributes;
	IO_STATUS_BLOCK IoStatusBlock;

	UNICODE_STRING FileName;
	RtlInitUnicodeString(&FileName, L"\\??\c:\\test2.log");

	InitializeObjectAttributes(&ObjectAttributes, &FileName, OBJ_CASE_INSENSITIVE, NULL, NULL);

	//
	NTSTATUS ntStatus = ZwOpenFile(&FileHand, GENERIC_ALL, &ObjectAttributes, &IoStatusBlock,
		FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_SYNCHRONOUS_IO_NONALERT);

	if(NT_SUCCESS(ntStatus))
	{
		KdPrint(("ok\n"));
	}
	else
	{
		KdPrint(("ERROR\n"));
	}

	PCHAR pBuffer = (PCHAR)ExAllocatePool(PagedPool, BUFFER_SIZE);

	RtlFillMemory(pBuffer, BUFFER_SIZE, 0xAA);

	ZwWriteFile(FileHand, NULL, NULL, NULL, &IoStatusBlock, pBuffer, BUFFER_SIZE, NULL, NULL);

	RtlFillMemory(pBuffer, BUFFER_SIZE, 0xBB);

	LARGE_INTEGER number;
	number.QuadPart = 1024i64;

	ZwWriteFile(FileHand, NULL, NULL, NULL, &IoStatusBlock, pBuffer, BUFFER_SIZE, &number, NULL);

	ExFreePool(pBuffer);

	ZwClose(FileHand);
}





#pragma INITCODE
/**
 * DriverEntry�������ں��е�I/O������������ã��亯������������:pDriverObject��pRegistryPath
 * pDriverObject:��I/O����������������������;
 * pRegistryPath:��һ��Unicode�ַ���,ָ������������ע���
 *
 *
 */
extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPath)
{
	NTSTATUS status;
	

	/** 
	 * ����������Windows��I/O������ע��һЩ�ص�������
	 * �ص��������ɳ���Ա����ĺ�������Щ������������������������ã������ɲ���ϵͳ������á�
	 * ����Ա����Щ��������ڵ�ַ���߲���ϵͳ������ϵͳ�����ʵ���ʱ�������Щ������
	 *
	 */
	KdPrint(("Enter DriverEntry 1 \n"));

	TestFileCreateOne();

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
	 * ����һ��Unicode�ַ��������ַ��������洢���豸��������ơ�
	 */
	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, L"\\Device\\MyDDKDevice");

	/** 
	 * ��IoCreateDevice��������һ���豸����
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
	 * �����������ӡ�����������Ȼ�����豸���ƣ����������豸����ֻ�����ں�̬�ɼ���������Ӧ�ó����ǲ��ɼ��ġ�
	 * ��ˣ�������Ҫ��¶һ���������ӣ�������ָ���������豸���ơ�
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
 * ����ж����������
 */
#pragma PAGEDCODE
VOID HelloDDKUnload(IN PDRIVER_OBJECT pDriverObject)
{

	PDEVICE_OBJECT pNextObj;
	KdPrint(("Enter DriverUnload 1 \n"));

	// ����������õ��豸����
	pNextObj = pDriverObject->DeviceObject;

	while(NULL != pNextObj)
	{
		PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pNextObj->DeviceExtension;

		// ɾ���豸����ķ�������
		UNICODE_STRING pLinkName = pDevExt->ustrSymLinkName;
		IoDeleteSymbolicLink(&pLinkName);
		pNextObj = pNextObj->NextDevice;
		IoDeleteDevice(pDevExt->pDevice);
	}
}

#pragma PAGEDCODE
/** 
 * ���豸����Ĵ������رպͶ�д����������ָ�������Ĭ�ϵ���ǲ�����С�
 *
 */
NTSTATUS HelloDDKDispatchRoutine(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{

	KdPrint(("Enter HelloDDKDispatchRoutine 1 \n"));
	NTSTATUS status = STATUS_SUCCESS;

	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;

	// ֻ����ɴ�IRP
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	KdPrint(("Leave HelloDDKDispatchRountine 1 \n"));

	return status;
}

