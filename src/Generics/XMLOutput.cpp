/*****************************************************************************

Project:  Juice
Module:   Generics.dll
Filename: XmlOutput.cpp 

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

Generics.dll and its source code are free software.  Except as noted, you 
may redistribute this code under the terms of the Gnu General Public License,
version 2 (GPL2).   This license shall be terminated if any part of GPL2 
should be overturned in court (any jurisdiction).  In that case, contact the 
author for updated license terms.

*****************************************************************************/

#include <GenericTypes.h>
#include "XmlInternal.h"
#include <malloc.h> // for alloca
#include "MSXML2.h"

using namespace Generic;
using namespace Generic::XML;

/****************************************************************************/
/** Constructor
 **
 ** Just intializes m_iIndent to zero
*****************************************************************************/
XmlOutput::XmlOutput () : m_iIndent(0)
{
}

/****************************************************************************/
/** Destructor
 **
 ** Closes the file if it's still open
*****************************************************************************/
XmlOutput::~XmlOutput ()
{ 
	if (m_File.is_open ())
		m_File.close ();
}

/****************************************************************************/
/** XmlXmlOutput::fOpen 
 **
 ** Opens the file at the given path
*****************************************************************************/
bool XmlOutput::fOpen (const char *szPath)
{
	m_File.open (szPath);
	return m_File.is_open ();
}

/****************************************************************************/
/** XmlXmlOutput::vIndentMore 
*****************************************************************************/
void XmlOutput::vIndentMore ()
{
	m_iIndent++;
}

/****************************************************************************/
/** XmlXmlOutput::vIndentLess 
*****************************************************************************/
void XmlOutput::vIndentLess ()
{
	m_iIndent--;
	if (m_iIndent < 0)
		m_iIndent = 0;
}

/****************************************************************************/
/** XmlXmlOutput::vNewline
 **
 ** An argument could be made that this should be done with \r\n instead of
 ** just \n, but I'm not convinced by that argument.*  However, encapsulating
 ** all newlines with this method will make it easy to change the output
 ** format.
 **
 ** * The argument: Microsoft has done it that way since the DOS days.  
 **   Counterargument: Unix has done it this way for longer.  
*****************************************************************************/
void XmlOutput::vNewline ()
{
	m_File << '\n';
}

/****************************************************************************/
/** XmlXmlOutput::vIndent 
 **
 ** Emit the appropriate number of \t characters
*****************************************************************************/
void XmlOutput::vIndent ()
{
	for (int iIndent = 0; iIndent < m_iIndent; iIndent++)
		m_File << "\t";
}

/****************************************************************************/
/** XmlXmlOutput::vOpenTag 
 **
 ** Just makes it a little easier to emit open-tags.
*****************************************************************************/
void XmlOutput::vOpenTag (const wchar_t *wszName)
{
	size_t iLen = wcslen (wszName) + 1;
	char *szTmp = new char[iLen];
	size_t iWritten = wcstombs (szTmp, wszName, iLen);
	szTmp[iWritten] = 0;

	m_File << '<' << szTmp << '>';
	delete [] szTmp;
}


/****************************************************************************/
/** XmlXmlOutput::vCloseTag 
 **
 ** Just makes it a little easier to emit close-tags.
*****************************************************************************/
void XmlOutput::vCloseTag (const wchar_t *wszName)
{
	size_t iLen = wcslen (wszName) + 1;
	char *szTmp = new char[iLen];
	size_t iWritten = wcstombs (szTmp, wszName, iLen);
	szTmp[iWritten] = 0;

	m_File << '<' << '/' << szTmp << '>';

	delete [] szTmp;
}


/****************************************************************************/
/** XmlXmlOutput::vWrite (bool)
*****************************************************************************/
void XmlOutput::vWrite (bool b)
{
	if (b)
		m_File << 1;
	else
		m_File << 0;
}

/****************************************************************************/
/** XmlXmlOutput::vWrite (int)
*****************************************************************************/
void XmlOutput::vWrite (int i)
{
	m_File << i;
}

/****************************************************************************/
/** XmlXmlOutput::vWrite (std:string)
*****************************************************************************/
void XmlOutput::vWrite (std::string &str)
{
	m_File << str;
}

/****************************************************************************/
/** XmlXmlOutput::vWrite (void *)
*****************************************************************************/
void XmlOutput::vWrite (void *pv)
{
	m_File << (unsigned) pv;
}

/****************************************************************************/
/** XmlXmlOutput::vWrite (unsigned)
*****************************************************************************/
void XmlOutput::vWrite (unsigned u)
{
	m_File << u;
}

/****************************************************************************/
/** XmlXmlOutput::vWrite (real)
*****************************************************************************/
void XmlOutput::vWrite (real r)
{
	m_File << r;
}


/****************************************************************************/
/** XmlXmlOutput::Container Constructor
 **
 ** Just increase the indent and write an opening tag for the contained XML
*****************************************************************************/
XmlOutput::Container::Container (XmlOutput &Output, const wchar_t *wszName) :
		m_Output (Output), m_szName (wszName)
{
	if (m_szName)
	{
		m_Output.vIndent ();
		m_Output.vOpenTag (m_szName);
		m_Output.vNewline ();

		m_Output.vIndentMore ();
	}
}


/****************************************************************************/
/** XmlXmlOutput::Container Destructor
 **
 ** Decrease the indent and write a closing tag for the contained XML
*****************************************************************************/
XmlOutput::Container::~Container ()
{
	if (m_szName)
	{
		m_Output.vIndentLess ();

		m_Output.vIndent ();
		m_Output.vCloseTag (m_szName);
		m_Output.vNewline ();
	}
}


/****************************************************************************/
/** XmlStreamWriter::fWrite 
 **
 ** Opens a file, serializes the given XmlStreamable object, closes the file.
 **
 ** TODO: The XML should begin with a DTD reference (and should be unicode,
 ** not ASCII?)
*****************************************************************************/
bool StreamWriter::fWrite (const char *szPath, Serializable *pObject, int iVersion)
{
	// Open the output stream
	XmlOutput File;
	if (!File.fOpen (szPath))
		return false;

	// Create an XML stream with that file
	Stream Stream (&File);

	// Create a buffer for the opening XML tag
	int iCharacters = wcslen (pObject->szGetPreferredTagName ()) + 1;
	char *szTagName = new char [iCharacters];
	wcstombs (szTagName, pObject->szGetPreferredTagName (), iCharacters);
	szTagName[iCharacters-1] = 0;

	// Write the opening tag for the XML file
	File.m_File << "<" << szTagName << " " 
		<< g_szFileVersionAttribute << "=\"" << iVersion << "\" " 
		<< g_szSerializationVersionAttribute << "=\"" << g_iSerializationVersion << "\""
		<< ">";

	File.vNewline ();
	File.vIndentMore ();

	// Serialize the given object
	pObject->vSerialize (Stream);

	// Write the closing tag for the XML file
	File.vIndentLess ();
	File.vCloseTag (pObject->szGetPreferredTagName ());// << "</" << szTagName << ">";

	// release the buffer
	delete [] szTagName;

	// close the file
	File.vNewline ();

	// Yay.
	return true;
}

// XMLOutput.cpp ends here ->