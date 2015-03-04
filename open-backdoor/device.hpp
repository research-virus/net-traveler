// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#ifndef __INC_DEVICE_HPP__
#define __INC_DEVICE_HPP__

#include <windows.h>

DWORD __stdcall g_fnThread2(LPVOID lpThreadParameter);

LRESULT WaitForDeviceToConnect(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
BYTE ConvertUnitMaskToUnitByte(DWORD dwMask);

#endif // __INC_DEVICE_HPP__