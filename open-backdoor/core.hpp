// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#ifndef __INC_CORE_HPP__
#define __INC_CORE_HPP__

#include <windows.h>

DWORD __stdcall g_fnThread1(LPVOID lpThreadParameter);

void TaskReset();

#endif // __INC_CORE_HPP__