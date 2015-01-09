/** 
 * Driver.h
 */

#pragma one

#ifdef __cplusplus
extern "C"
{
#endif

#include <ntddk.h>

	/************************************************************************/
	/* #include <ntddk.h>

	#include <initguid.h>
	#include <wdmguid.h>
	#include <ntddkbd.h>

	#include <stdarg.h>
	#include <stdlib.h>
	#include <stdio.h>                                                                     */
	/************************************************************************/

#ifdef __cplusplus
}
#endif

#define PAGEDCODE  code_seg("PAGE")  // �����ǽ��˲��ִ�������ҳ�ڴ�������
#define LOCKEDCODE code_seg()        // �����������ΪĬ�ϵĴ����
#define INITCODE   code_seg("INIT")  // ���ص�INIT�ڴ������У��ɹ����غ󣬿����˳��ڴ�


#define PAGEDDATA  data_seg("PAGE")
#define LOCKEDDATA data_seg()
#define INITDATA   data_seg("INIT")

#define arrarysize(P)  (sizeof(P) / sizeof((P)[0]))

typedef struct _DEVICE_EXTENSION
{
	PDEVICE_OBJECT pDevice;
	UNICODE_STRING ustrDeviceName;   // �豸����
	UNICODE_STRING ustrSymLinkName;  // ����������
}DEVICE_EXTENSION, *PDEVICE_EXTENSION;


typedef struct _MYDATASTRUCT
{
	// List Entry ��Ҫ��Ϊ_MYDATASTRUCT�ṹ���һ����
	LIST_ENTRY ListEntry;

	// �Զ������ݽṹ
	ULONG x;
}MYDATASTRUCT, *PMYDATASTRUCT;

// ��������
NTSTATUS CreateDevice(IN PDRIVER_OBJECT pDriverObject);
VOID HelloDDKUnload(IN PDRIVER_OBJECT pDriverObject);
NTSTATUS HelloDDKDispatchRoutine(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);


extern "C" NTSYSAPI NTSTATUS NTAPI ZwQuerySystemInformation(
						 IN ULONG SystemInformationClass,
						 IN PVOID SystemInformation,
						 IN ULONG SystemInformationLength,
						 OUT PULONG ReturnLength);
