#ifdef __cplusplus

extern "C"    // 保持符号链接的正确性
{
#endif
#include <wdm.h>
#ifdef __cplusplus
}
#endif


typedef struct _DEVICE_EXTENSION   // 定义设备扩展结构体
{
	PDEVICE_OBJECT fdo;
	PDEVICE_OBJECT NextStackDevice;
	UNICODE_STRING ustrDeviceName;    // 设备名
	UNICODE_STRING ustrSymLinkName;   // 符号链接名
}DEVICE_EXTENSION, *PDEVICE_EXTENSION;


#define PAGEDCODE  code_seg("PAGE")  // 定义分页内存、非分页内存和INIT段内存的标志+
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
