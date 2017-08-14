#pragma once

#ifdef _WIN32
#include "resource.h"
#elif __linux__
#include "stdafx.h"
#endif

void __stdcall funcCallback01(HWND, UINT, UINT_PTR, DWORD);
void OnCtrlStartClick();
void OnCtrlPauseClick();
void OnCtrlResumeClick();
