// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#ifndef __INC_NETWORK_INFO_HPP__
#define __INC_NETWORK_INFO_HPP__

#include <windows.h>

BOOL GetVolumeSerialNumber(char *szVolSerial, DWORD dwSize);
BOOL GetHostInfo(char *szHostname, DWORD dwSize, char *szHostByName);

#endif // __INC_NETWORK_INFO_HPP__