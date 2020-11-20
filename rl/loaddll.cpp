#include "loaddll.h"

#include <cstdio>
#include <direct.h>

#include <Windows.h>

#include <iostream>

static HMODULE hGetProcIDDLL;

bool LoadDLL(InitFn& init, MainLoopFn& ml, const char* dllname)
{
	char buff[FILENAME_MAX * 2];
	_getcwd(buff, FILENAME_MAX);

	strcat(buff, dllname);

	hGetProcIDDLL = LoadLibraryA(buff);

	if (!hGetProcIDDLL)
	{
		std::cerr << "could not open file: " << static_cast<const char*>(buff) << "\n";
		return false;
	}

	init = (InitFn)GetProcAddress(hGetProcIDDLL, "Init");
	if (!init)
	{
		FreeLibrary(hGetProcIDDLL);
		return false;
	}

	ml = (MainLoopFn)GetProcAddress(hGetProcIDDLL, "MainLoop");
	if (!ml)
	{
		FreeLibrary(hGetProcIDDLL);
		return false;
	}

	return true;
}

void UnloadDLL()
{
	FreeLibrary(hGetProcIDDLL);
}