#include "HelloWDM.h"

#pragma INITCODE  //���˺�������INIT���У����������ؽ����󣬴˺����Ϳ��Դ��ڴ���ж�ص�

/** 
 * ��ʼ���������򣬶�λ������Ӳ����Դ�������ں˶���
 * pDriverObject:��I/O�������д���������������
 * pRegistryPath:����������ע����е�λ��
 *
 */
extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPath)
{

	KdPrint(("Enter DriverEntery \n"));

	pDriverObject->DriverExtension->AddDevice = HelloWDMAddDevice;  // ����AddDevice�ص��������˻ص�����ֻ������WDM���������У�
	// ����NTʽ��������û�д˻ص��������˻ص������������Ǵ����豸������PNP(���弴��)���������á�

	pDriverObject->MajorFunction[IRP_MJ_PNP] = HelloWDMPnp; // ����IRP_MJ_PNP��IRP�Ļص���������PNP��IRP������NTʽ������WDM�������ش�����

	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HelloWDMDispatchRoutine; // ���ó���IRP����������򵥵�ָ��Ĭ�Ϻ���
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = HelloWDMDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_READ] = HelloWDMDispatchRoutine;

	pDriverObject->DriverUnload = HelloWDMUnload;  // ��ϵͳע��ж�����̡���WDM�����У��󲿷�ж�ع����Ѳ��ڴ˴���
	// ���Ƿ���IRP_MN_REMOVE_DEVICE��IRP�Ĵ������д���



	KdPrint(("Leaver DriverEntery\n"));
	return STATUS_SUCCESS;
}


/** 
 * ������豸
 * DriverObject:��I/O�������д���������������
 * PhysicalDeviceObject:��I/O�������д������������豸����
 *
 * ��WDM�����������У������豸�������������DriverEntry�е�������Ҫ����������ϵͳע��һ������AddDevice�����̡�
 * ��������PNP������������ã��亯����Ҫְ���Ǵ����豸����
 * HelloWDMAddDevice����������������DriverObject��PhysicalDeviceObject��
 * DriverObject����PNP���������ݽ������������󣬴˶�����DriverEntry�е���������
 * PhysicalDeviceObject��PNP���������ݽ����ĵײ������豸����
 */
#pragma PAGEDCODE  // ��#pragma ָ���������̷����ڷ�ҳ�ڴ���
NTSTATUS HelloWDMAddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT PhysicalDeviceObject)
{
	PAGED_CODE();  //  PAGED_CODE��һ��DDK�ṩ�ĺֻ꣬��check������Ч�������������ڵ��ж����󼶳���APC_LEVELʱ��
	               //  �����һ�����ԣ����Ի�ʹ������ֹ������������ַ��

	KdPrint(("Enter HelloEDMAddDevice\n"));

	NTSTATUS status;
	PDEVICE_OBJECT fdo;
	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, L"\\Device\\MyWDMDevice");

	status = IoCreateDevice(DriverObject,  // �����豸����
		sizeof(DEVICE_EXTENSION),
		&(UNICODE_STRING)devName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&fdo);
	if(!NT_SUCCESS(status))
		return status;

	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;  // �õ��豸������չ���ݽṹ
	pdx->fdo = fdo; // ��¼�豸��չ�еĹ����豸����Ϊ��ղŴ������豸

	pdx->NextStackDevice = IoAttachDeviceToDeviceStack(fdo, PhysicalDeviceObject);
	// ��IoAttachDeviceToDeviceStack��������fdo(�����豸����)�ҽ����豸��ջ�ϣ���������ֵ(�²��ջ��λ��)����¼���豸��չ�ṹ�С�

	UNICODE_STRING symLinkName;
	RtlInitUnicodeString(&symLinkName, L"\\DosDevices\\HelloWDM");
	pdx->ustrDeviceName = devName;
	pdx->ustrSymLinkName = symLinkName; // �����豸�ķ�������

	status = IoCreateSymbolicLink(&(UNICODE_STRING)symLinkName, &(UNICODE_STRING)devName);

	if(!NT_SUCCESS(status))
	{
		IoDeleteSymbolicLink(&pdx->ustrSymLinkName);
		status = IoCreateSymbolicLink(&symLinkName, &devName);
		if(!NT_SUCCESS(status))
			return status;
	}

	fdo->Flags |= DO_BUFFERED_IO | DO_POWER_PAGABLE;  // �����豸ΪBUFFERED_IO�豸����ָ��������ʼ�����
	fdo->Flags &= ~DO_DEVICE_INITIALIZING;
	KdPrint(("Leave HelloWDMAddDevice\n"));
	return STATUS_SUCCESS;
}



/** 
 * �Լ��弴��IRP���д���
 * fdo:�����豸����                  Irp:��I/O�����
 *
 */
#pragma PAGEDCODE
NTSTATUS HelloWDMPnp(IN PDEVICE_OBJECT fdo, IN PIRP Irp)
{
	PAGED_CODE();  // ��PAGED_CODE��ȷ�������������ڵ���APC_LEVEL���ж����ȼ��ļ�����
	KdPrint(("Enter HelloWDMPnp\n"));
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;  // �õ��豸��չ�ṹ
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);  // �õ���ǰIRP�Ķ�ջ
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
 * ����IRP_MN_STOP_DEVICE���⣬HelloWDM������PNP��IRP��ͬ���Ĵ�����ֱ�Ӵ��ݵ��ײ������������ײ������Ľ�����ء�
 * 
 * ��PNP IRP����Ĭ�ϴ���
 * pdx:�豸������չ    Irp:��I/O�����
 *
 */
#pragma PAGEDCODE
NTSTATUS DefaultPnpHandler(PDEVICE_EXTENSION pdx, PIRP Irp)
{
	PAGED_CODE();  // ȷ�������̴���APC_LEVEL֮��
	KdPrint(("Enter DefaultPnpHandler\n"));
	IoSkipCurrentIrpStackLocation(Irp);  // �Թ���ǰ��ջ
	KdPrint(("Leave DefaultPnpHandler\n"));
	return IoCallDriver(pdx->NextStackDevice, Irp); // ���²��ջ�������豸�������IRP
}



/** 
 * ��IRP_MN_REMOVE_DEVICE IRP���д���
 *
 * fdo:�����豸����        Irp:��I/O�����
 */
#pragma PAGEDCODE
NTSTATUS HandleRemoveDevice(PDEVICE_EXTENSION pdx, PIRP Irp)
{
	PAGED_CODE();
	KdPrint(("Enter HandleRemoveDevice\n"));
	Irp->IoStatus.Status = STATUS_SUCCESS; // ���ô�IRP��״̬Ϊ˳�����
	NTSTATUS status = DefaultPnpHandler(pdx, Irp);  // ����Ĭ�ϵ�PNP��IRP�Ĵ�����
	IoDeleteSymbolicLink(&(UNICODE_STRING)pdx->ustrSymLinkName);  // ɾ�����豸����ķ�������

	// ����IoDetachDevice()��fdo���豸ջ���ѿ�
	if(pdx->NextStackDevice)
		IoDetachDevice(pdx->NextStackDevice);

	// ɾ��fdo
	IoDeleteDevice(pdx->fdo);   // ɾ���豸����
	KdPrint(("Leave HandleRemoveDevice\n"));
	return status;
}

/** 
 * ��Ĭ��IRP���д���
 * fdo:�����豸����    Irp:��I/O�����
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
 * �������������ж��
 * DriverObject:��������
 *
 */
#pragma PAGEDCODE
void HelloWDMUnload(IN PDRIVER_OBJECT DriverObject)
{
	PAGED_CODE();
	KdPrint(("Enter HelloWDMUnload\n"));
	KdPrint(("Leave HelloWDMUnload\n"));
}