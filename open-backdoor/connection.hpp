// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#ifndef __INC_CONNECTION_HPP__
#define __INC_CONNECTION_HPP__

#include <windows.h>

//#define TEST_CONNECTION_VERBOSE // Print output of connection test

BOOL TestConnection_NoProxy();
BOOL sub_1000605B();
BOOL sub_10006637();
BOOL TestProxyConnection();

#endif // __INC_CONNECTION_HPP__