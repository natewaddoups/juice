/*****************************************************************************

Project:  Juice
Module:   Generics.dll
Filename: Trace.cpp (unfinished, barely tested)

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

Generics.dll and its source code are free software.  Except as noted, you 
may redistribute this code under the terms of the Gnu General Public License,
version 2 (GPL2).   This license shall be terminated if any part of GPL2 
should be overturned in court (any jurisdiction).  In that case, contact the 
author for updated license terms.

*****************************************************************************/

#include <GenericTypes.h>
#include <GenericTrace.h>

using namespace Generic;

__declspec (dllexport) Generic::Trace Generic::g_Trace;

Trace::Trace () : m_eOutputMode (omStdErr)
{
}

Trace::~Trace ()
{
}

bool Trace::fOpen (const char *szPath)
{
	open (szPath, ios_base::out | ios_base::trunc);

	if (!is_open ())
	{
		m_eOutputMode = omStdErr;
		return false;
	}

	m_eOutputMode = omFile;
	return true;
}

void Trace::vClose ()
{
	close ();
}

void Trace::vSetOutputMode (OutputMode om)
{
	m_eOutputMode = om;
}

void Trace::Unexpected (const char *szFile, long iLine, const char *szMsg)
{
	char szLine[100];
	wsprintf (szLine, "%d", iLine);

	OutputDebugString (szFile);
	OutputDebugString (" ");
	OutputDebugString (szLine);
	OutputDebugString (": Unexpected circumstance ");
	OutputDebugString (szMsg);
	OutputDebugString ("\r\n");
}

std::ofstream::_Myt& Trace::write (const char *sz, std::streamsize count)
{
	if (m_eOutputMode == omStdErr)
	{
		OutputDebugString (sz);
	}
	else
	{
		if (is_open ())
		{
			std::ostream::write (sz, count);
		}
	}

	return *this;
}

std::ofstream::_Myt& Trace::put(char ch)
{
	if (m_eOutputMode == omStdErr)
	{
		char sz[2];
		sz[0] = ch;
		sz[1] = 0;

		OutputDebugString (sz);
	}
	else
	{
		if (is_open ())
		{
			std::ostream::put (ch);
		}
	}

	return *this;
}


// Trace.cpp ends here ->