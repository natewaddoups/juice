/*****************************************************************************

Project:  Juice
Module:   Generics.dll
Filename: XmlInternal.h 

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

Generics.dll and its source code are free software.  Except as noted, you 
may redistribute this code under the terms of the Gnu General Public License,
version 2 (GPL2).   This license shall be terminated if any part of GPL2 
should be overturned in court (any jurisdiction).  In that case, contact the 
author for updated license terms.

*****************************************************************************/

namespace Generic { 
	namespace XML {

extern const char g_szFileVersionAttribute[];
extern const char g_szSerializationVersionAttribute[];
extern const int g_iSerializationVersion;

extern const wchar_t g_szWideFileVersionAttribute[];
extern const wchar_t g_szWideSerializationVersionAttribute[];

} // end of XML namespace
} // end of Generic namespace

// XmlInternal.h ends here ->