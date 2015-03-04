// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#ifndef __INC_SETUP_HPP__
#define __INC_SETUP_HPP__

#include <Windows.h>

bool LoadConfig();
BOOL RemoveALL();
bool GetSystemTechnicalInfo(void);
BOOL GetProcessList();
void GetNetworkInfo();
void WriteLog(LPCVOID lpBuffer, DWORD dwSize);
void GetProgramFilesEx(const char *lpszProgramFiles);
void Setup_();

#endif // __INC_SETUP_HPP__