/*****************************************************************************

Project:  Juice
Module:   Generics.dll
Filename: Generics.h 

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

Generics.dll and its source code are free software.  Except as noted, you 
may redistribute this code under the terms of the Gnu General Public License,
version 2 (GPL2).   This license shall be terminated if any part of GPL2 
should be overturned in court (any jurisdiction).  In that case, contact the 
author for updated license terms.

*****************************************************************************/

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the GENERICS_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// GENERICS_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef GENERICS_EXPORTS
#define Generics_Export __declspec(dllexport)
#else
#define Generics_Export  __declspec(dllimport)
#endif

// Generics.h ends here ->