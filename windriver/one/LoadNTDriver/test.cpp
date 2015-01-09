#include <windows.h>  
#include <winsvc.h>  
#include <conio.h>  
#include <stdio.h>

#define DRIVER_NAME "HelloDDK"


#define DRIVER_PATH "..\\HelloDDK.sys"

/** 
 * �豸��������Ķ�̬������Ҫ�ɷ���������������(Service Control Manager,SCM)ϵͳ�����ɡ�
 * SCM���ΪWindows 2000�����е�����ṩ��������,������ֹͣ�Ϳ��Ʒ���
 *
 * ���غ�ж��NT������Ϊ�ĸ�����:
 * 1��ΪNT���������µķ���
 * 2�������˷���
 * 3���رմ˷���
 * 4��ɾ��NT�����������ķ���
 *
 */
BOOL LoadNTDriver(char *lpszDriverName, char *lpszDriverPath)
{

	char szDriverImagePath[256];
	GetFullPathName(lpszDriverPath, 256,szDriverImagePath, NULL);
	printf("12---------\n");

	BOOL bRet = FALSE;

	SC_HANDLE hServiceMgr = NULL;
	SC_HANDLE hServiceDDK = NULL;

	/** 
	 * ��SCM������,����SCM�ĳ�ʼ��
	 * SC_HANDLE OpenSCManagerA(LPCSTR lpMachineName, LPCSTR lpDatabaseName, DWORD dwDesiredAccess);
	 * lpMachineName:ָ�����������,���ΪNULL������
	 * lpDatabaseName:ָ��SCM���ݵ�����,���ΪNULL����ʹ��ȱʡ���ݿ⡣
	 * dwDesiredAccess:ʹ��Ȩ��,һ������ΪSC_MANAGER_ALL_ACCESS
	 * ����ֵ:����ɹ�����SCM�������ľ�������ʧ�ܷ���NULL
	 */
	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if(NULL == hServiceMgr)
	{
		printf("OpenSCManger() Faild %d !\n", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
		printf("OpenSCManger() Ok !\n");
	}

	/** 
	 * ����SCM�������ľ��,����Ĳ������ǻ������������еġ�

	 SC_HANDLE CreateServiceA(
	 SC_HANDLE hSCManager,       // SCM�������ľ��,��OpenSCManager�򿪵ľ��
	 LPCSTR lpServiceName,       // ��������,�������豸�������п������豸����
	 LPCSTR lpDisplayName,       // ������ʾ����������
	 DWORD  dwDesiredAccess,     // ��Ȩ��,���û������Ҫ��,��ΪSERVICE_ALL_ACCESS
	 DWORD  dwServiceType,       // ��������
	 DWORD dwStartType,          // �򿪷���ʱ��
	 DWORD  dwErrorControl,      // ���ڴ�����Ĵ���
	 LPCSTR lpBinaryPathName,    // �������ļ��Ĵ���(�������õĶ����ƴ���)
	 LPCSTR lpLoadOrderGroup,    // �ú��û��鿪������
	 LPDWORD lpdwTagId,          // �����֤��ǩ
	 LPCSTR lpDependencies,      // �������ķ��������
	 LPCSTR lpServiceStartName,  // �û��˺�����
	 LPCSTR lpPassword           // �û�����
	 );

	 */
	hServiceDDK = CreateService(hServiceMgr,
		lpszDriverName,
		lpszDriverName,
		SERVICE_ALL_ACCESS,
		SERVICE_KERNEL_DRIVER,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_IGNORE,
		szDriverImagePath,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL);

	DWORD dwRtn;

	if(NULL == hServiceDDK)
	{
		dwRtn = GetLastError();
		if(dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS)
		{
			printf("CreateService() Faild %d \n", dwRtn);
			bRet = FALSE;
			goto BeforeLeave;
		}
		else
		{
			printf("CreateService() Faild Service is ERROR_IO_PENDING or ERROR_SERVICE_EXISTS !\n");
		}

		/** 
		 * ����Ѿ��������ķ���,�ٴδ򿪴������ 

		 SC_HANDLE OpenService(
		 SC_HANDLE hSCManager,   // SCM���ݿ�ľ��,Ҳ����OpenSCManager�򿪵ľ�� 
		 LPCSTR lpServiceName,   // �Ѿ������ķ�������
		 DWORD dwDesiredAccess   // ��Ȩ��
		 );
		 *
		 */
		hServiceDDK = OpenService(hServiceMgr, lpszDriverName, SERVICE_ALL_ACCESS);

		if(NULL == hServiceDDK)
		{
			dwRtn = GetLastError();
			printf("OpenService() Faild %d !\n", dwRtn);
			bRet = FALSE;
			goto BeforeLeave;
		}
		else
		{
			printf("OpenService() ok !\n");
		}

	}
	else
	{
		printf("OpenService() Ok !\n");
	}


	/** 
	 *

	 BOOL
	 StartService(
	 SC_HANDLE hService,
	 DWORD dwNumServiceArgs,
	 LPCSTR  *lpServiceArgVectors
	 );

	 *
	 */
	bRet = StartService(hServiceDDK, NULL, NULL);
	if(!bRet)
	{
		DWORD dwRtn = GetLastError();
		if(dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_ALREADY_RUNNING)
		{
			printf("StartService() Faild %d !\n", dwRtn);
			bRet = FALSE;
			goto BeforeLeave;
		}
		else
		{
			if(dwRtn == ERROR_IO_PENDING)
			{
				printf("StartService() Faild ERROR_IO_PENDING !\n");
				bRet = FALSE;
				goto BeforeLeave;
			}
			else
			{
				printf("StartService() Faild ERROR_SERVICE_ALREANY_RUNNING !\n");
				bRet = TRUE;
				goto BeforeLeave;
			}
		}
	}
	bRet = TRUE;

BeforeLeave:
	if(hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if(hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}
	return bRet;
}


BOOL UnloadNTDriver(char *szSvrName)
{
	BOOL bRet = FALSE;

	SC_HANDLE hServiceMgr = NULL;
	SC_HANDLE hServiceDDK = NULL;
	SERVICE_STATUS service_sta;

	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(NULL == hServiceMgr)
	{
		printf("OpenSCManger() Faild %d \n", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
		printf("OpenSCManger() ok !\n");
	}

	hServiceDDK = OpenService(hServiceMgr, szSvrName, SERVICE_ALL_ACCESS);

	if(NULL == hServiceDDK)
	{
		printf("OpenService() Faild %d \n", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
		printf("OpenService() ok !\n");
	}

	/** 
	 * ����Ӧ�ķ���,���Ϳ�����,���ݲ�ͬ�Ŀ������������

	 BOOL
	 ControlService(
	 SC_HANDLE hService,                // ����ľ��,Ҳ������CreateService�����ľ��,����OpenService�򿪵ľ��
	 DWORD dwControl,                   // ������
	 LPSERVICE_STATUS lpServiceStatus   // ���񷵻ص�״̬��
	 );

	 */
	if(!ControlService(hServiceDDK, SERVICE_CONTROL_STOP, &service_sta))
	{
		printf("ControlService() Faild %d !\n", GetLastError());
	}
	else
	{
		printf("ControlService() ok !\n");
	}

	if(!DeleteService(hServiceDDK))
	{
		printf("DeleteService() Faild %d !\n", GetLastError());
	}
	else
	{
		printf("DeleteService() ok !\n");
	}
	bRet = TRUE;

BeforeLeave:
	if(hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if(hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}

	return bRet;
}


void TestDriver()
{

	HANDLE hDevice = CreateFile("\\\\.\\HelloDDK",
		GENERIC_WRITE | GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING, 0, NULL);

	if(hDevice != INVALID_HANDLE_VALUE)
	{
		printf("Open Driver ok !\n");
	}
	else
	{
		printf("Open Driver faild %d !\n", GetLastError());
	}

	CloseHandle(hDevice);
}


int main(int argc, char **argv)
{
	printf("1---------\n");
	BOOL bRet = LoadNTDriver(DRIVER_NAME, DRIVER_PATH);
	printf("2---------\n");
	if(!bRet)
	{
		printf("LoadNTDriver error \n");
		return 0;
	}

	printf( "press any to create device!\n" );  
	getch();  

	TestDriver();
 
	printf( "press any to unload the driver!\n" );  
	getch();  




	UnloadNTDriver(DRIVER_NAME);
	if(!bRet)
	{
		printf("UnloadNTDriver error \n");
		return 0;
	}

	return 0;
}

