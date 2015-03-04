// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#ifndef __INC_PROXY_HPP__
#define __INC_PROXY_HPP__

#include <windows.h>
#include <Wininet.h>

BOOL GetProcessHandle(PHANDLE lpTokenHandle, char *lpszProcName);
BOOL SearchCredentials(const char *lpszSearchTerm, char *lpszUser, char *lpszPsw);

LPVOID GetURL_CustomProxy(LPCSTR lpszURL, DWORD *lpdwMemLength, LPINTERNET_PROXY_INFO lpProxy, DWORD dwProxySize, const char *lpszUser, const char *lpszPsw);
LPVOID GetURL_NoProxy(LPCSTR lpszURL, DWORD *lpdwMemLength);

#endif // __INC_PROXY_HPP__