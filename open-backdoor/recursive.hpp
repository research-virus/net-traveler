// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#ifndef __INC_RECURSIVE_HPP__
#define __INC_RECURSIVE_HPP__

#include <windows.h>
#include <iostream>

BOOL SendUEnumFSToC2();
void RecursiveFileSearch(const char *lpszUnit, FILE *f);
void ScanDevice(const char *lpszUnit);
void SendTmpFolderToC2();

#endif // __INC_RECURSIVE_HPP__