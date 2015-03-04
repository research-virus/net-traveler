// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#ifndef __INC_UTILS_HPP__
#define __INC_UTILS_HPP__

#include <windows.h>

enum EConnMethod
{
	NO_CONNECTION,
	CONNECTION_NO_PROXY,
	CONNECTION_METHOD2,
	CONNECTION_METHOD3,
	CONNECTION_METHOD4,
};

void SetNewProxyConfig();
void SetProxyDisabled();
void SetCheckSuccess();
int RetriveInfo();

#endif // __INC_UTILS_HPP__