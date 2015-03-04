// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#ifndef __INC_CONFIG_HPP__
#define __INC_CONFIG_HPP__

#include <windows.h>

BOOL PropagateConfig();
void LoadProxyConfig();

void GetFolderRecursive(char *szFolder, int *k);
void GetFolderFirstLevel(char *szFolder, int *k);

#endif // __INC_CONFIG_HPP__