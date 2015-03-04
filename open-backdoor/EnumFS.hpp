// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#ifndef __INC_ENUMFS_HPP__
#define __INC_ENUMFS_HPP__

#include <windows.h>
#include <iostream>

BOOL GenerateEnumFS();
void RecursiveDriveScan();
void RecursiveFolderScan(const char *lpszRootDir, FILE *f);
void LogFolderContent(const char *lpszRootDir, FILE *f);

#endif // __INC_ENUMFS_HPP__