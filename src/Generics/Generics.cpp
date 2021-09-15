/*****************************************************************************

Project:  Juice
Module:   Generics.dll
Filename: Generics.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

Generics.dll and its source code are free software.  Except as noted, you 
may redistribute this code under the terms of the Gnu General Public License,
version 2 (GPL2).   This license shall be terminated if any part of GPL2 
should be overturned in court (any jurisdiction).  In that case, contact the 
author for updated license terms.

I should note that most of the stuff in this file is pretty basic and there's
no way I could be bothered to care if you re-used it in any way.
*****************************************************************************/

#include "stdafx.h"
#include "Generics.h"
#include <GenericTypes.h>
#include <GenericError.h>
#include <GenericTrace.h>
#include <crtdbg.h>

using namespace Generic;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//_CrtSetBreakAlloc (219);
		g_Trace.fOpen ("c:\\Temp\\GenericTrace.log");
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		OutputDebugString ("Generics.dll memory leak test...");
		if (_CrtDumpMemoryLeaks ())
		{
			OutputDebugString ("Bummer!\n");
		}
		else
		{
			OutputDebugString ("none found.\n");
		}
		break;
	}
    return TRUE;
}

real GenericsExport Generic::rRound (real rInput, real rResolution)
{
	return float(int(rInput * (1/rResolution) + (rResolution / 2))) / (1/rResolution);
}

void GenericsExport Generic::DoBreakpoint (const char *szReason)
{
	if (szReason)
	{
		OutputDebugString (szReason);
		OutputDebugString ("\n");
	}
	else
	{
		OutputDebugString ("Eek!  Bad thing happen!\n");
	}
}

Generic::UINT32 GenericsExport Generic::WinColorRef (Color clr)
{
	UINT8 Red   = (char) (255.0 * clr.r);
	UINT8 Green = (char) (255.0 * clr.g);
	UINT8 Blue  = (char) (255.0 * clr.b);

	return RGB (Red, Green, Blue);
}

void GenericsExport Generic::vShowLastError (const char *szCaption)
{
	DWORD dwLastError = GetLastError();

	if (!dwLastError)
		return;

	TCHAR *szSystemMessage = 0;	

	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwLastError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &szSystemMessage,
		0,
		NULL 
	);

	// Breakpoint ("Generic::vShowLastError.");
	MessageBox (NULL, szSystemMessage, szCaption, MB_OK);

	LocalFree (szSystemMessage);
}

// Generics.cpp ends here ->