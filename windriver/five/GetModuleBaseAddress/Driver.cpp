#include "Driver.h"


typedef struct _SYSTEM_MODULE_INFORMATION_ENTRY
{
	ULONG  Unknown1;
	ULONG  Unknown2;
#ifdef _WIN64
	ULONG Unknown3;
	ULONG Unknown4;
#endif
	PVOID	BaseAddress;
	ULONG	Size; 
	ULONG	Flags;
	USHORT	Index;
	USHORT	Rank;
	USHORT	LoadCount;
	USHORT	NameOffset;
	CHAR	Name[256];
} SYSTEM_MODULE_INFORMATION_ENTRY, *PSYSTEM_MODULE_INFORMATION_ENTRY;

typedef struct _SYSTEM_MODULE_INFORMATION
{
	ULONG Count;
	SYSTEM_MODULE_INFORMATION_ENTRY Module[1];
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

#define SystemModuleInformation	11

VOID GetModuleBaseAddress(CHAR Name[16],INT_PTR* BaseAddr,ULONG* Size)
{
	PSYSTEM_MODULE_INFORMATION_ENTRY pModule;
	ULONG	nSize, i, nCount;
	PSYSTEM_MODULE_INFORMATION pBuffer;
	BOOLEAN	bRet=FALSE;
	*BaseAddr = 0;
	*Size = 0;


	ZwQuerySystemInformation(SystemModuleInformation, &nSize, 0, &nSize);
	pBuffer = (PSYSTEM_MODULE_INFORMATION)ExAllocatePool(NonPagedPool, nSize);
	ZwQuerySystemInformation(SystemModuleInformation, pBuffer, nSize, NULL);

	nCount = pBuffer->Count;
	pModule = (PSYSTEM_MODULE_INFORMATION_ENTRY)&pBuffer->Module;

	for(i = 0; i < nCount; i++)
	{
		if(_stricmp(pModule[i].Name+pModule[i].NameOffset, Name) == 0)
		{
			*BaseAddr = (INT_PTR)pModule[i].BaseAddress;
			*Size = pModule[i].Size;
			break;
		}
	}

	ExFreePool(pBuffer);
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
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = HelloDDKDispatchRoutine;

	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = HelloDDKDispatchRoutine;

	pDriverObject->MajorFunction[IRP_MJ_READ] = HelloDDKDispatchRoutine;

	KdPrint(("Enter DriverEntry 5 \n"));
	status = CreateDevice(pDriverObject);


	INT_PTR kbdclassBaseAddress = 0;
	ULONG	ulSize;
	GetModuleBaseAddress("kbdclass.sys",&kbdclassBaseAddress,&ulSize);

	KdPrint(("kbdclassBaseAddress 0x%0x\n", kbdclassBaseAddress));
	if (0 == kbdclassBaseAddress)
	{
		KdPrint(("kbdclassBaseAddress == 0x00000000\n"));
		return 0;
	}

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

