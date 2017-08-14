// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#ifdef XXX
#elif __linux__

#include <unistd.h>
#define __stdcall
#define CALLBACK __stdcall
#define Sleep usleep

typedef int HWND;
typedef struct tagRECT {
    long left;
    long top;
    long right;
    long bottom;
} RECT, *PRECT;

typedef void VOID, *LPVOID;
typedef unsigned int UINT;
typedef unsigned long UINT_PTR;
typedef long INT_PTR;
typedef unsigned short ATOM;
typedef wchar_t TCHAR;
typedef unsigned long DWORD;

#endif

// TODO: reference additional headers your program requires here
