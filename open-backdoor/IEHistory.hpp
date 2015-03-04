// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#ifndef __INC_IEHISTORY_HPP__
#define __INC_IEHISTORY_HPP__

#include <windows.h>

typedef struct _HASH_RECORD_HEADER {
	char szSignature[4];
	DWORD dwBlockCount;
	DWORD dwNextRecord;
	DWORD dwUnknown;
} HASH_RECORD_HEADER;

typedef struct _HASH_RECORD_BLOCK {
	DWORD dwUnknown;
	DWORD dwURLAddr;
} HASH_RECORD_BLOCK;

class IEHistory {
	private:
		HANDLE	m_lpHistoryMapView;
		DWORD	m_dwHistorySize;
	public:
		IEHistory(void);
		virtual ~IEHistory();
		
		HANDLE Initialize(LPCSTR lpszFileName);
		
		int GetHistory(WCHAR **lpszPool);
};

#endif // __INC_IEHISTORY_HPP__