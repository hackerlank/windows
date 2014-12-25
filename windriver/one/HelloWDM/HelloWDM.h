#ifdef __cplusplus

extern "C"    // ���ַ������ӵ���ȷ��
{
#endif
#include <wdm.h>
#ifdef __cplusplus
}
#endif


typedef struct _DEVICE_EXTENSION   // �����豸��չ�ṹ��
{
	PDEVICE_OBJECT fdo;
	PDEVICE_OBJECT NextStackDevice;
	UNICODE_STRING ustrDeviceName;    // �豸��
	UNICODE_STRING ustrSymLinkName;   // ����������
}DEVICE_EXTENSION, *PDEVICE_EXTENSION;


#define PAGEDCODE  code_seg("PAGE")  // �����ҳ�ڴ桢�Ƿ�ҳ�ڴ��INIT���ڴ�ı�־+
#define LOCKEDCODE code_seg()
#define INITCODE   code_seg("INIT")

#define PAGEDDATE  data_seg("PAGE")
#define LOCKEDDATA data_seg()
#define INITDATA   data_seg("INIT")

#define arraysize(p) (sizeof(p) / sizeof((p)[0]))

NTSTATUS HelloWDMAddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT PhysicalDeviceObject);

NTSTATUS HelloWDMPnp(IN PDEVICE_OBJECT fdo, IN PIRP Irp);

NTSTATUS HelloWDMDispatchRoutine(IN PDEVICE_OBJECT fdo, IN PIRP Irp);

NTSTATUS DefaultPnpHandler(PDEVICE_EXTENSION pdx, PIRP Irp);

void HelloWDMUnload(IN PDRIVER_OBJECT DriverObject);

extern "C"
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);
