#include "Driver.h"

#define BUFFER_SIZE 1024

// 
// VOID DisplayItsProcessName()
// {
// 	PEPROCESS pEProcess = PsGetCurrentProcess();
// 
// 	PTSTR ProcessName = (PTSTR)((ULONG)pEProcess + 0x174);
// 	KdPrint(("%s \n", ProcessName));
// }
// 

VOID InitStingOne()
{
	ANSI_STRING AnsiString;
	UNICODE_STRING UnicodeString;

	CHAR *AString = "AnsiString hello";
	WCHAR *UString = L"Unicode_string hello";

	RtlInitAnsiString(&AnsiString, AString);
	RtlInitUnicodeString(&UnicodeString, UString);
	KdPrint(("--------------InitSting1--------------\n"));
	KdPrint(("ANSI_STRING_1 --> %Z\n", &AnsiString));

// KdPrint(("UNICODE_STRING --> %Z\n", &UnicodeString));
// UNICODE_STRING --> U     �������w
	KdPrint(("UNICODE_STRING --> %Z\n", &UnicodeString));
	KdPrint(("UNICODE_STRING --> %wZ\n", &UnicodeString));

	AString[0] = 'H';
	AString[1] = 'E';
	AString[2] = 'L';
	AString[3] = 'L';
	AString[4] = 'O';



	KdPrint(("ANSI_STRING_2 --> %Z\n", &AnsiString));
	KdPrint(("--------------InitSting1--------------\n"));

}

VOID InitStringTwo()
{
	UNICODE_STRING Unicode_String = {0};
	Unicode_String.MaximumLength = BUFFER_SIZE;
	Unicode_String.Buffer = (PWCH)ExAllocatePool(PagedPool, BUFFER_SIZE);
	WCHAR *wideString = L"Hello Wide String";
	Unicode_String.Length = 2 * wcslen(wideString);

	KdPrint(("--------------InitSting2--------------\n"));

	ASSERT(Unicode_String.Buffer, wideString, Unicode_String.Length);
	RtlCopyMemory(Unicode_String.Buffer, wideString, Unicode_String.Length);

	KdPrint(("unicode String: %wZ\n", &Unicode_String));

	ExFreePool(Unicode_String.Buffer);
	Unicode_String.Buffer = NULL;
	Unicode_String.Length = Unicode_String.MaximumLength = 0;

	ANSI_STRING AnsiString = {0};
	CHAR *CString = "CHAR Hello";
	AnsiString.Buffer = (PCHAR)ExAllocatePool(PagedPool, BUFFER_SIZE);
	AnsiString.MaximumLength = BUFFER_SIZE;
	AnsiString.Length = strlen(CString);

	RtlCopyMemory(AnsiString.Buffer, CString, AnsiString.Length);
	KdPrint(("AnsiString : %Z\n", &AnsiString));

	ExFreePool(AnsiString.Buffer);
	AnsiString.Buffer = NULL;
	AnsiString.Length = AnsiString.MaximumLength = 0;
	KdPrint(("--------------InitSting2--------------\n"));
}


VOID TestCopy()
{
	ANSI_STRING AnsiString;

	KdPrint(("--------------TestCopy--------------\n"));

	RtlInitAnsiString(&AnsiString, "AnsiString");

	ANSI_STRING AnsiString1 = {0};
	AnsiString1.Buffer = (PCHAR)ExAllocatePool(PagedPool, BUFFER_SIZE);
	AnsiString1.Length = AnsiString.Length;
	AnsiString1.MaximumLength = BUFFER_SIZE;

	RtlCopyString(&AnsiString1, &AnsiString);

	KdPrint(("AnsiString1:%Z\n", &AnsiString1));
	RtlFreeAnsiString(&AnsiString1);

	UNICODE_STRING UnicodeString;
	RtlInitUnicodeString(&UnicodeString, L"UnicodeString");

	UNICODE_STRING UnicodeString1 = {0};

	UnicodeString1.Buffer = (PWCH)ExAllocatePool(PagedPool, BUFFER_SIZE);
	UnicodeString1.Length = UnicodeString.Length;
	UnicodeString1.MaximumLength = BUFFER_SIZE;

	KdPrint(("UnicodeString:%wZ\n", &UnicodeString1));
	RtlFreeUnicodeString(&UnicodeString1);
	KdPrint(("--------------TestCopy--------------\n"));
}

VOID TestCompare()
{
	LONG ret;
	ANSI_STRING AnsiString1, AnsiString2;
	RtlInitAnsiString(&AnsiString1, "AnsiString");
	RtlInitAnsiString(&AnsiString2, "Ansistring");

	KdPrint(("--------------TestCompare--------------\n"));

	ret = RtlCompareString(&AnsiString1, &AnsiString2, TRUE);
	KdPrint(("TRUE ret = %d \n", ret));

	ret = RtlCompareString(&AnsiString1, &AnsiString2, FALSE);
	KdPrint(("FALSE ret = %d \n", ret));

	/** 
	 * UNICODE_STRING �ַ����ȽϺ���
	 * LONG RtlCompareUnicodeString(PCUNICODE_STRING String1, PCUNICODE_STRING String2, BOOLEAN CaseInSensitive);
	 * CaseInSensitive:�Ƿ�Դ�Сд����
	 */

	UNICODE_STRING UnicodeString1, UnicodeString2;
	RtlInitUnicodeString(&UnicodeString1, L"UnicodeString");
	RtlInitUnicodeString(&UnicodeString2, L"unicodeString");

	ret = RtlEqualUnicodeString(&UnicodeString1, &UnicodeString2, TRUE);
	KdPrint(("TRUE ret = %d \n", ret));

	ret = RtlEqualUnicodeString(&UnicodeString1, &UnicodeString2, FALSE);
	KdPrint(("FALSE ret = %d \n", ret));

	KdPrint(("--------------TestCompare--------------\n"));
}

VOID TestUpCase()
{
	ANSI_STRING AnsiString;
	RtlInitAnsiString(&AnsiString, "Ansi case");

	KdPrint(("--------------TestUpCase--------------\n"));

	RtlUpperString(&AnsiString, &AnsiString);
	KdPrint(("AnsiString : %Z \n", &AnsiString));

	UNICODE_STRING UnicodeString;
	RtlInitUnicodeString(&UnicodeString, L"Unicode case");

	RtlUpcaseUnicodeString(&UnicodeString, &UnicodeString, FALSE);

	KdPrint(("UnicodeString : %wZ\n", &UnicodeString));

	KdPrint(("--------------TestUpCase--------------\n"));
}

VOID TestStringToInteger()
{

	UNICODE_STRING Unicode_String;
	RtlInitUnicodeString(&Unicode_String, L"12121212");
	KdPrint(("--------------TestStringToInteger--------------\n"));
	
	ULONG number;
	RtlUnicodeStringToInteger(&Unicode_String, 10, &number);
	KdPrint(("number : %d\n", number));

	UNICODE_STRING Unicode_string1 = {0};
	Unicode_string1.Buffer = (PWCH)ExAllocatePool(PagedPool, BUFFER_SIZE);
	Unicode_string1.MaximumLength = BUFFER_SIZE;
	RtlIntegerToUnicodeString(20880809, 10, &Unicode_string1);
	KdPrint(("Unicode_string1 : %wZ\n", &Unicode_string1));

	KdPrint(("--------------TestStringToInteger--------------\n"));

	RtlFreeUnicodeString(&Unicode_string1);
}

VOID TestAnsiToUnicode()
{
	UNICODE_STRING Unicode_string;
	ANSI_STRING Ansi_String;
	RtlInitUnicodeString(&Unicode_string, L"Unicode String");

	KdPrint(("--------------TestAnsiToUnicode--------------\n"));

	Ansi_String.Buffer = (PCHAR)ExAllocatePool(PagedPool, BUFFER_SIZE);
	Ansi_String.MaximumLength = BUFFER_SIZE;

	RtlUnicodeStringToAnsiString(&Ansi_String, &Unicode_string, FALSE);
	KdPrint(("Ansi_String:%Z\n", &Ansi_String));

	RtlFreeAnsiString(&Ansi_String);

	UNICODE_STRING Unicode_String1;
	ANSI_STRING Ansi_String1;
	RtlInitAnsiString(&Ansi_String1, "Ansi String");
	RtlAnsiStringToUnicodeString(&Unicode_String1, &Ansi_String1, TRUE);

	KdPrint(("Unicode_String1:%wZ\n", &Unicode_String1));
	KdPrint(("--------------TestAnsiToUnicode--------------\n"));

}

VOID TestString()
{

	InitStingOne();
	InitStringTwo();
	TestCopy();
	TestCompare();
	TestUpCase();
	TestStringToInteger();
	TestAnsiToUnicode();
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

	// ANSI_STRING 
	ANSI_STRING AnsiString_1;

	TestString();

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

	NTSTATUS status = STATUS_SUCCESS;

	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;

	// ֻ����ɴ�IRP
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	KdPrint(("Leave HelloDDKDispatchRountine 1 \n"));

	return status;
}

