// stdafx.cpp : source file that includes just the standard includes
// UltimateSubtitles.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file


void print(char* str, ...)
{
	char buffer[1024];
	va_list args;
	va_start (args, str);
	vsnprintf_s(buffer, 1023, str, args);
	MessageBoxA(NULL, buffer, "Print", MB_OK);
	//OutputDebugStringA(buffer);
	va_end (args);
}