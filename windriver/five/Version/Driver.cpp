#include "Driver.h"






// Winbase.h��Winnt.h
/**
BOOL Is_WinXP_SP_3_or_Later()
{

	OSVERSIONINFOEX osVersionInfo;
	ULONGLONG ConditionMask;

	int op = VER_GREATER_EQUAL;

	// Initialize the OSVERSIONINFOEX structure.
	ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
	osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osVersionInfo.dwMajorVersion = 5;
	osVersionInfo.dwMinorVersion = 1;
	osVersionInfo.wServicePackMajor = 3;
	osVersionInfo.wServicePackMinor = 0;

	// Initialize the condition mask.
	VER_SET_CONDITION(ConditionMask, VER_MAJORVERSION, op);
	VER_SET_CONDITION(ConditionMask, VER_MINORVERSION, op);
	VER_SET_CONDITION(ConditionMask, VER_SERVICEPACKMAJOR, op);
	VER_SET_CONDITION(ConditionMask, VER_SERVICEPACKMINOR, op);

	// Perform the test.
	return VerifyVersionInfo(&osVersionInfo, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR,
		ConditionMask);

}
*/

VOID TestVersion()
{

	RTL_OSVERSIONINFOEXW osVersionInfoXw;
	RTL_OSVERSIONINFOW   osVersionInfoOw;
	NTSTATUS status_Rtl;

	NTSTATUS status = RtlGetVersion(&osVersionInfoOw);

	if(!NT_SUCCESS(status))
	{
		KdPrint(("RtlGetVersion error \n"));
		return;
	}
	else
	{
		KdPrint(("RtlGetVersion ok \n"));
		
	}

	/*
	ULONG dwOSVersionInfoSize;
	ULONG dwMajorVersion;
	ULONG dwMinorVersion;
	ULONG dwBuildNumber;
	ULONG dwPlatformId;
	WCHAR  szCSDVersion[ 128 ];     // Maintenance string for PSS usage
	*/

	KdPrint(("dwMajorVersion: %d  %d \n",osVersionInfoOw.dwMajorVersion, osVersionInfoOw.dwMinorVersion));
	KdPrint(("dwBuildNumber:  %d  %d \n",osVersionInfoOw.dwBuildNumber, osVersionInfoOw.dwPlatformId));
	KdPrint(("szCSDVersion: %S \n", osVersionInfoOw.szCSDVersion));





	/**
	typedef struct _OSVERSIONINFOEXW {
	ULONG  dwOSVersionInfoSize;
	ULONG  dwMajorVersion;
	ULONG  dwMinorVersion;
	ULONG  dwBuildNumber;
	ULONG  dwPlatformId;
	WCHAR  szCSDVersion[ 128 ];     // Maintenance string for PSS usage
	USHORT  wServicePackMajor;
	USHORT  wServicePackMinor;
	USHORT  wSuiteMask;
	UCHAR  wProductType;
	UCHAR  wReserved;
	} RTL_OSVERSIONINFOEXW;
	 */

	ULONG TypeMask = VER_BUILDNUMBER | VER_MAJORVERSION | VER_MINORVERSION | VER_PLATFORMID | VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR | VER_SUITENAME | VER_PRODUCT_TYPE;

	ULONGLONG ConditionMask = 0;
	int op = VER_GREATER_EQUAL;


	VER_SET_CONDITION(ConditionMask, VER_BUILDNUMBER ,op);
	VER_SET_CONDITION(ConditionMask, VER_MAJORVERSION ,op);
	VER_SET_CONDITION(ConditionMask, VER_MINORVERSION ,op);
	VER_SET_CONDITION(ConditionMask, VER_PLATFORMID ,op);



	status_Rtl = RtlVerifyVersionInfo(&osVersionInfoXw, TypeMask, ConditionMask);

	if(!NT_SUCCESS(status_Rtl))
	{
		KdPrint(("RtlVerifyVersionInfo error \n"));
		return;
	}
	else
	{
		KdPrint(("RtlVerifyVersionInfo ok \n"));

		KdPrint(("%d  %d \n", osVersionInfoXw.dwMajorVersion, osVersionInfoXw.dwMinorVersion));

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
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = HelloDDKDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = HelloDDKDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_READ] = HelloDDKDispatchRoutine;
	status = CreateDevice(pDriverObject);

	TestVersion();


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

