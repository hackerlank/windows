#include <windows.h>  
#include <winsvc.h>  
#include <conio.h>  
#include <stdio.h>

#define DRIVER_NAME "HelloDDK"


#define DRIVER_PATH "..\\HelloDDK.sys"

/** 
 * 设备驱动程序的动态加载主要由服务控制器管理程序(Service Control Manager,SCM)系统组件完成。
 * SCM组件为Windows 2000中运行的组件提供服务。例如,启动、停止和控制服务。
 *
 * 加载和卸载NT驱动分为四个步骤:
 * 1、为NT驱动创建新的服务
 * 2、开启此服务
 * 3、关闭此服务
 * 4、删除NT驱动所创建的服务
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
	 * 打开SCM管理器,用于SCM的初始化
	 * SC_HANDLE OpenSCManagerA(LPCSTR lpMachineName, LPCSTR lpDatabaseName, DWORD dwDesiredAccess);
	 * lpMachineName:指定计算机名称,如果为NULL代表本机
	 * lpDatabaseName:指定SCM数据的名称,如果为NULL代表使用缺省数据库。
	 * dwDesiredAccess:使用权限,一般设置为SC_MANAGER_ALL_ACCESS
	 * 返回值:如果成功返回SCM管理器的句柄，如果失败返回NULL
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
	 * 创建SCM管理器的句柄,后面的操作都是基于这个句柄进行的。

	 SC_HANDLE CreateServiceA(
	 SC_HANDLE hSCManager,       // SCM管理器的句柄,即OpenSCManager打开的句柄
	 LPCSTR lpServiceName,       // 服务名称,就是在设备管理器中看到的设备名称
	 LPCSTR lpDisplayName,       // 服务显示出来的名称
	 DWORD  dwDesiredAccess,     // 打开权限,如果没有特殊要求,设为SERVICE_ALL_ACCESS
	 DWORD  dwServiceType,       // 服务类型
	 DWORD dwStartType,          // 打开服务时间
	 DWORD  dwErrorControl,      // 关于错误处理的代码
	 LPCSTR lpBinaryPathName,    // 二进制文件的代码(服务所用的二进制代码)
	 LPCSTR lpLoadOrderGroup,    // 用何用户组开启服务
	 LPDWORD lpdwTagId,          // 输出验证标签
	 LPCSTR lpDependencies,      // 所依赖的服务的名称
	 LPCSTR lpServiceStartName,  // 用户账号名称
	 LPCSTR lpPassword           // 用户口令
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
		 * 针对已经创建过的服务,再次打开此项服务 

		 SC_HANDLE OpenService(
		 SC_HANDLE hSCManager,   // SCM数据库的句柄,也就是OpenSCManager打开的句柄 
		 LPCSTR lpServiceName,   // 已经创建的服务名称
		 DWORD dwDesiredAccess   // 打开权限
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
	 * 对相应的服务,发送控制码,根据不同的控制码操作服务

	 BOOL
	 ControlService(
	 SC_HANDLE hService,                // 服务的句柄,也就是用CreateService创建的句柄,或者OpenService打开的句柄
	 DWORD dwControl,                   // 控制码
	 LPSERVICE_STATUS lpServiceStatus   // 服务返回的状态码
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

