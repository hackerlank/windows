/** 
 * LoadNTDriver.h
 */

#pragma one

#ifdef __cplusplus
extern "C"
{
#endif

#include <ntddk.h>

#ifdef __cplusplus
}
#endif

#define PAGEDCODE  code_seg("PAGE")
#define LOCKEDCODE code_seg()
#define INITCODE   code_seg("INIT")

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

// ��������
NTSTATUS CreateDevice(IN PDRIVER_OBJECT pDriverObject);
VOID HelloDDKUnload(IN PDRIVER_OBJECT pDriverObject);
NTSTATUS HelloDDKDispatchRoutine(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);
