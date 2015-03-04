// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#ifndef __INC_MD5_HPP__
#define __INC_MD5_HPP__

#include <windows.h>
#include <iostream>

class CMD5 {
public:
	CMD5(void) {}
	virtual ~CMD5(void) {}

	const char *GetStringMD5(const char *c_lpszString);
};

#endif
