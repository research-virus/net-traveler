// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#include "system_info.hpp"

#include <iostream>
#include <Shlobj.h>
#include "StdAfx.hpp"
#include "setup.hpp"
#include "IEHistory.hpp"

void GetProgramFiles()
{
	CHAR szProgramFiles[260] = {};
	
	if (FALSE == SHGetSpecialFolderPath(NULL, szProgramFiles, CSIDL_PROGRAM_FILES, TRUE))
		return;
	
	strcat(szProgramFiles, "\\");
	GetProgramFilesEx(szProgramFiles);
}

void GetIEHistory()
{
	CHAR szBuf[10240];
	WCHAR *szPool[512] = {};
	
	IEHistory pkHistory;
	int nCount = pkHistory.GetHistory(szPool);
	
	LOG("\r\n\r\n///////////////////////////////////////////////\r\n\r\n");
	LOG("\r\nIE History:\r\n");
	
	if (nCount > 0)
	{
		WCHAR **lpszPool = szPool;
		for (int i = nCount; i > 0; i--, lpszPool++)
		{
			sprintf(szBuf, "%S", *lpszPool);
			
			LOG(szBuf);
			LOG("\r\n");
		}
		
		for (lpszPool = szPool; nCount > 0; nCount--, lpszPool++)
			delete *lpszPool;
	}
}

void GetOSVersion()
{
	CHAR szLog[260] = {};
	strcat(szLog, "OSVersion: ");
	
	OSVERSIONINFO osInfo = {};
	osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	
	if (FALSE == GetVersionEx(&osInfo))
		return;
	
	if (osInfo.dwPlatformId == 1)
	{
		if (osInfo.dwMajorVersion == 4)
		{
			if (osInfo.dwMinorVersion == 0)
			{
				strcat(szLog, "Microsoft Windows 95 ");
				
				if (osInfo.szCSDVersion[1] == 'C' || osInfo.szCSDVersion[1] == 'B')
					strcat(szLog, "OSR2 ");
			}
			else if (osInfo.dwMinorVersion == 10)
			{
				strcat(szLog, "Microsoft Windows 98 ");
				
				if (osInfo.szCSDVersion[1] == 'A')
					strcat(szLog, "SE ");
			}
			else if (osInfo.dwMinorVersion == 90)
			{
				strcat(szLog, "Microsoft Windows Millennium Edition ");
			}
		}
	}
	else if(osInfo.dwPlatformId == 2)
	{
		if (osInfo.dwMajorVersion <= 4)
		{
			strcat(szLog, "Microsoft Windows NT ");
		}
		else if (osInfo.dwMajorVersion == 5)
		{
			if (osInfo.dwMinorVersion == 0)
				strcat(szLog, "Microsoft Windows 2000 ");
			else if (osInfo.dwMinorVersion == 1)
				strcat(szLog, "Microsoft Windows XP ");
			else if (osInfo.dwMinorVersion == 2)
				strcat(szLog, "Microsoft Windows 2003 ");
		}
		else if (osInfo.dwMajorVersion == 6)
		{
			if (osInfo.dwMinorVersion == 0)
				strcat(szLog, "Microsoft Windows Vista ");
			else if (osInfo.dwMinorVersion == 1)
				strcat(szLog, "Microsoft Windows7 ");
			else if (osInfo.dwMinorVersion == 2)
				strcat(szLog, "Microsoft Windows8 ");
		}
		
		CHAR szProductType[80];
		DWORD cbData; // Check this, shouldn't it be the size of szProductType?
		HKEY hKey;
		
		RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\ProductOptions", 0, 1, &hKey);
		RegQueryValueEx(hKey, "ProductType", NULL, NULL, (LPBYTE)szProductType, &cbData);
		RegCloseKey(hKey);
		
		if (!lstrcmpi("WINNT", szProductType))
			strcat(szLog, "Professional, ");
		
		if (!lstrcmpi("LANMANNT", szProductType))
			strcat(szLog, "Server, ");
		
		if (!lstrcmpi("SERVERNT", szProductType))
			strcat(szLog, "Advanced Server, ");
		
		CHAR szBuild[256];
		if (osInfo.dwMajorVersion > 4)
		{
			sprintf(szBuild, "%s (Build %d)\n", osInfo.szCSDVersion, (WORD)osInfo.dwBuildNumber);
			strcat(szLog, szBuild);
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
			
			strcat(szLog, szBuild);
		}
	}
	
	LOG("\r\n\r\n///////////////////////////////////////////////\r\n\r\n");
	LOG(szLog);
}

void GetIEVersion()
{
	CHAR Buffer[260] = {};
	strcat(Buffer, "IEVersion: Internet Explorer ");
	
	HKEY hKey;
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Internet Explorer", 0, KEY_QUERY_VALUE , &hKey))
	{
		char Source[260] = {};
		DWORD dwSize = 260;
		
		if (ERROR_SUCCESS == RegQueryValueEx(hKey, "Version", NULL, NULL, (LPBYTE)Source, &dwSize))
		{
			LOG("\r\n\r\n///////////////////////////////////////////////\r\n\r\n");
			
			strcat(Buffer, Source);
			LOG(Buffer);
		}
	}
	
	RegCloseKey(hKey);
}