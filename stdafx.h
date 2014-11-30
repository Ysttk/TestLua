// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define MAC
//#define WIN


#ifdef WIN
#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						
#endif

#ifndef WIN
#define _TCHAR char
#endif

#include <stdio.h>
#include <cstdint>

#define INT16 int16_t
#define INT32 int32_t
#define INT64 int64_t
#define UINT32 uint32_t
#define UINT64 uint64_t

#ifdef WIN
#include <tchar.h>
#endif

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
};


#include <cassert>
#include <vector>
// TODO: reference additional headers your program requires here
