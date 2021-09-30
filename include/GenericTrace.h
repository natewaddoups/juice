/*****************************************************************************

Project:  Juice
Module:   Generics.dll
Filename: GenericTrace.h (little used, little tested)

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

Generics.dll and its source code are free software.  Except as noted, you 
may redistribute this code under the terms of the Gnu General Public License,
version 2 (GPL2).   This license shall be terminated if any part of GPL2 
should be overturned in court (any jurisdiction).  In that case, contact the 
author for updated license terms.

*****************************************************************************/

#ifndef __GenericTrace__
#define __GenericTrace__

namespace Generic
{
/****************************************************************************/
/** 
*****************************************************************************/
	class GenericsExport Trace : public std::ofstream
	{
	public:

		Trace ();
		~Trace ();

		bool fOpen (const char *szPath);
		void vClose ();

		enum OutputMode
		{
			omInvalid = 0, omStdErr, omFile
		};

		void vSetOutputMode (OutputMode om);

		static void Write (const char *sz);
		static void Unexpected (const char *szFile, long iLine, const char *szMsg);

	protected:
		OutputMode m_eOutputMode;

		std::ofstream& write (const char *sz, std::streamsize count);
		
		std::ofstream& put(char ch);

	};

	extern GenericsExport Trace g_Trace;

#ifdef _DEBUG
#define GenericTrace(x) { g_Trace << x; }
#else
#define GenericTrace(x)
#endif

#ifdef _DEBUG
#define GenericUnexpected(x) { Generic::Trace::Unexpected (__FILE__, __LINE__, #x); }
#else
#define GenericUnexpected(x)
#endif

};

#endif

// GenericTrace.h ends here ->