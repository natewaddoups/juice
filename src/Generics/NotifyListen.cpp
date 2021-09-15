/*****************************************************************************

Project:  Juice
Module:   Generics.dll
Filename: NotifyListen.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

Generics.dll and its source code are free software.  Except as noted, you 
may redistribute this code under the terms of the Gnu General Public License,
version 2 (GPL2).   This license shall be terminated if any part of GPL2 
should be overturned in court (any jurisdiction).  In that case, contact the 
author for updated license terms.

*****************************************************************************/

#include <GenericTypes.h>
#include <GenericNotifyListen.h>

namespace Generic
{

	void GenericsExport *NotifyListen_New (size_t cb)
	{
		return (void*) new char [cb];
	}

	void  GenericsExport NotifyListen_Delete (void *p)
	{
		delete [] p;
	}
}

// NotifyListen.cpp ends here ->