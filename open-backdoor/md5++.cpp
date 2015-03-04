// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#include "md5++.hpp"
#include "StdAfx.hpp"
#include "3rd-party\md5.h"

const char *CMD5::GetStringMD5(const char *c_lpszString)
{
	MD5_CTX lpCTX;
	char szBuf[32];
	unsigned char lpMD5[16];

	MD5_Init(&lpCTX);
	MD5_Update(&lpCTX, c_lpszString, strlen(c_lpszString));
	MD5_Final(lpMD5, &lpCTX);

	char *c_lpszBuf = szBuf;
	for (int i = 0; i < 16; i++)
	{
		sprintf(c_lpszBuf, "%02x", lpMD5[i]);
		c_lpszBuf += 2;
	}

	memcpy(g_szLastHashMD5, szBuf, 32);
	return g_szLastHashMD5;
}