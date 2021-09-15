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

class XmlReader
{
public:
	virtual bool fRead (void *pPrimitive, type_info &Type const wchar_t *szPreferredTag, const wchar_t *szObsoleteTag = null) = 0;
	virtual bool fRead (Serializable *pObject, const wchar_t *szPreferredTag, const wchar_t *szObsoleteTag = null) = 0;
};

class XmlWriter
{
public:
	virtual bool fWrite (void *pPrimitive, type_info &Type const wchar_t *szPreferredTag, const wchar_t *szObsoleteTag = null) = 0;
	virtual bool fWrite (Serializable *pObject, const wchar_t *szPreferredTag, const wchar_t *szObsoleteTag = null) = 0;
};

////

class XmlReaderImpl : public XmlReader
{
	XmlNode *m_pRootNode;
	XmlNode *m_pCurrentNode;

public:
	virtual bool fRead (void *pPrimitive, type_info &Type const wchar_t *szPreferredTag, const wchar_t *szObsoleteTag = null);
	virtual bool fRead (Serializable *pObject, const wchar_t *szPreferredTag, const wchar_t *szObsoleteTag = null);
};

class XmlWriterImpl : public XmlWriter
{
	std::ofstream m_File;

public:
	virtual bool fWrite (void *pPrimitive, type_info &Type const wchar_t *szPreferredTag, const wchar_t *szObsoleteTag = null);
	virtual bool fWrite (Serializable *pObject, const wchar_t *szPreferredTag, const wchar_t *szObsoleteTag = null);
};

////

bool XmlInputStream::fRead (void *pPrimitive, type_info &Type const wchar_t *szPreferredTag, const wchar_t *szObsoleteTag = null)
{
}

bool XmlInputStream::fRead (Serializable *pObject, const wchar_t *szPreferredTag, const wchar_t *szObsoleteTag = null)
{
}

////

// XmlInternal.h ends here ->