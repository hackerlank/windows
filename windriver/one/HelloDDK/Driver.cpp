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

VOID DisplayItsProcessName()
{
	// �õ���ǰ���еĽ���
	PEPROCESS pEProcess = PsGetCurrentProcess();

	// �õ���ǰ��������
	PTSTR ProcessName = (PTSTR)((ULONG)pEProcess + 0x174);

	KdPrint(("DisplayItsProcessName %s \n", ProcessName));
}

VOID LinkListTest()
{
	LIST_ENTRY LinkListHead;

	// ��ʼ������
	InitializeListHead(&LinkListHead);

	PMYDATASTRUCT pData;
	ULONG i = 0;

	KdPrint(("Begin Insert to link list"));
	for(i = 0; i < 10; i++)
	{
		//
		pData = (PMYDATASTRUCT)ExAllocatePool(PagedPool, sizeof(MYDATASTRUCT));

		pData->x = i;

		InsertHeadList(&LinkListHead, &pData->ListEntry);
	}

	KdPrint(("begin remove from link list \n"));
	while(!IsListEmpty(&LinkListHead))
	{
		PLIST_ENTRY pEntry = RemoveTailList(&LinkListHead);

		pData = CONTAINING_RECORD(pEntry, MYDATASTRUCT, ListEntry);

		KdPrint(("%d   ---\n", pData->x));
		ExFreePool(pData);
	}
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
	pDriverObject->DriverUnload = HelloDDKUnload;

	LinkListTest();

	KdPrint(("Enter DriverEntry 2 \n"));
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = HelloDDKDispatchRoutine;

	DisplayItsProcessName();

	KdPrint(("Enter DriverEntry 3 \n"));
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = HelloDDKDispatchRoutine;

	KdPrint(("Enter DriverEntry 4 \n"));
	pDriverObject->MajorFunction[IRP_MJ_READ] = HelloDDKDispatchRoutine;

	KdPrint(("Enter DriverEntry 5 \n"));
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

	DisplayItsProcessName();

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

