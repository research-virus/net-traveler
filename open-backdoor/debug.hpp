// Copyright 2015 Christian Roggia. All rights reserved.
// Use of this source code is governed by an Apache 2.0 license that can be
// found in the LICENSE file.

#ifndef __INC_DEBUG_HPP__
#define __INC_DEBUG_HPP__

#ifdef _DEBUG
#define PRINTDBG(...) printf(__VA_ARGS__);
#else // _DEBUG
#define PRINTDBG(...)
#endif // _DEBUG

#endif // __INC_DEBUG_HPP__