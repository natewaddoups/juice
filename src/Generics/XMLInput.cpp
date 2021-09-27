/*****************************************************************************

Project:  Juice
Module:   Generics.dll
Filename: XMLInput.cpp 

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

Generics.dll and its source code are free software.  Except as noted, you 
may redistribute this code under the terms of the Gnu General Public License,
version 2 (GPL2).   This license shall be terminated if any part of GPL2 
should be overturned in court (any jurisdiction).  In that case, contact the 
author for updated license terms.

*****************************************************************************/

#include <GenericTypes.h>
#include <malloc.h> // for alloca
#include "MSXML2.h"
#include "XmlInternal.h"
#include <GenericTrace.h>

using namespace Generic;
using namespace Generic::XML;

const wchar_t __declspec (dllexport)  * Generic::XML::g_szSerializationID = L"SerializationID";

/*****************************************************************************
 ** XmlInput::XmlInput
/****************************************************************************/
XmlInput::XmlInput (Node *pRootNode, StringAssignFunc pfnStringAssign) :
	m_pRootNode (pRootNode),
	m_pCurrentNode (m_pRootNode),
	m_pfnStringAssign (pfnStringAssign)
{
}


/*****************************************************************************
 ** XmlInput::~XmlInput
/****************************************************************************/
XmlInput::~XmlInput ()
{
}


/*****************************************************************************
 ** XmlInput::GetChildren
/****************************************************************************/
const Node::List& XmlInput::GetChildren ()
{
	if (m_pCurrentNode)
		return m_pCurrentNode->GetChildren ();

	Breakpoint ("XmlInput::GetChildren: m_pCurrentNode is null");
	return m_pRootNode->GetChildren ();
}


/*****************************************************************************
 ** XmlInput::pGetChild
 ** Get the child of the current node with the given tag name.
 **
 ** TODO: return a list of children with the given name.  
/****************************************************************************/
Node* XmlInput::pGetChild (const wchar_t *szPreferredTag, const wchar_t *szObsoleteTag)
{
	if (!m_pCurrentNode)
	{
		Breakpoint ("XmlInput::GetChildren: m_pCurrentNode is null");
		return null;
	}

	Node *pChild = m_pCurrentNode->pGetChild (szPreferredTag);

	if (!pChild && szObsoleteTag)
		pChild = m_pCurrentNode->pGetChild (szObsoleteTag);

	return pChild;
}


/*****************************************************************************
 ** XmlInput::vAddPointerMapEntry
/****************************************************************************/
void XmlInput::vAddPointerMapEntry (void *pObject)
{
	Node *pObjectIDNode = pGetChild (g_szSerializationID);
	if (pObjectIDNode)
	{
		int iID = 0;
		vSetValue ((void*) &iID, pObjectIDNode->szGetContent ());
		GenericTrace ("Adding map entry:  ID=" << iID << " Pointer=" << ((int) pObject) << endl);
		m_PointerMap.insert (std::pair <void*, void*> ((void*) iID, pObject));
	}
}


/*****************************************************************************
 ** XmlInput::vSetValue (bool...
/****************************************************************************/
void XmlInput::vSetValue (bool *pf, const wchar_t *szValue)
{
	if (!pf || !szValue)
		return;
	
	wchar_t cFirst = szValue[0];

	if ((cFirst == L't') || (cFirst == L'T') || (cFirst == L'y') || (cFirst == L'Y') || (cFirst == L'1'))
	{
		*pf = true;
	}

	if ((cFirst == L'f') || (cFirst == L'F') || (cFirst == L'n') || (cFirst == L'N') || (cFirst == L'0'))
	{
		*pf = false;
	}
}


/*****************************************************************************
 ** XmlInput::vSetValue (int...
/****************************************************************************/
void XmlInput::vSetValue (int *pi, const wchar_t *szValue)
{
	if (!pi || !szValue)
		return;

	*pi = _wtoi (szValue);
}


/*****************************************************************************
 ** XmlInput::vSetValue (unsigned int...
/****************************************************************************/
void XmlInput::vSetValue (unsigned int *pu, const wchar_t *szValue)
{
	if (!pu || !szValue)
		return;

	*((int*) pu) = _wtoi (szValue);
}


/*****************************************************************************
 ** XmlInput::vSetValue (void *...
/****************************************************************************/
void XmlInput::vSetValue (void *pv, const wchar_t *szValue)
{
	if (!pv || !szValue)
		return;

	*((int*) pv) = wcstoul (szValue, null, 10);
}


/*****************************************************************************
 ** XmlInput::vSetValue (void **...
/****************************************************************************/
void XmlInput::vSetValue (void **ppv, const wchar_t *szValue)
{
	if (!ppv || !szValue)
		return;

	//*((int*) pv) = _wtoi (szValue);
	*ppv = (void*) wcstoul (szValue, null, 10);

	GenericTrace ("Adding pointer to list: " << (int) *ppv);
	m_PointerList.push_front (ppv);
}


/*****************************************************************************
 ** XmlInput::vSetValue (std::string...
/****************************************************************************/
void XmlInput::vSetValue (std::string *pstr, const wchar_t *szValue)
{
	if (!pstr || !szValue)
		return;

	size_t nChars = wcslen (szValue);

	// Convert the string to 8-bit format
	char *szTemp = (char*) alloca (nChars + 1);
	wcstombs (szTemp, szValue, nChars + 1);

	// null-terminate the 8-bit string
	szTemp[nChars] = 0;

	// String copy can be done via an external function, if the caller wishes to
	// manage the buffer that way.
	if (m_pfnStringAssign)
		m_pfnStringAssign (pstr, szTemp);
	else
		*pstr = szTemp;                
}


/*****************************************************************************
 ** XmlInput::vSetValue (real...
/****************************************************************************/
void XmlInput::vSetValue (real *pr, const wchar_t *szValue)
{
	if (!pr || !szValue)
		return;

	*pr = (real) _wtof (szValue);
}


/*****************************************************************************
 ** XmlInput::vFixPointers
/****************************************************************************/
void XmlInput::vFixPointers ()
{
	//std::list<void**> Unmatched;

	// Iterate over the list of pointers
	PointerList::iterator iter, iterLast = m_PointerList.end ();
	for (iter = m_PointerList.begin (); iter != iterLast; iter++)
	{
		void **ppv = *iter;

		GenericTrace ("Looking for match of pointer: " << (int) ppv);

		// Find the new value for this pointer
		PointerMap::iterator iterMap = m_PointerMap.find (*ppv);

		// If no new value was found...
		if (iterMap == m_PointerMap.end ())
		{
			//Unmatched.push_front (ppv);
			Breakpoint ("XmlInput:: No pointer map entry found");
			*ppv = 0;
			continue;
		}

		// Get the map entry 
		std::pair <void*, void*> MapEntry = *iterMap;

		// Get the new value
		void *pNew = MapEntry.second;

		// Set the pointer to the new value
		*ppv = pNew;
	}

	// TODO: Iterate over the unmatched pointers, set them to null
	// This will prevent the app from crashing after loading a corrupt file
	//
	// But if implemented now, would make it impossible for .jm files to get 
	// their pointers reset later in the application code.  So, will have to
	// wait until .jm file support can be removed.
}


/*****************************************************************************
 ** XmlInput::iGetFileVersion 
/****************************************************************************/
int XmlInput::iGetFileVersion ()
{
	if (!m_pRootNode)
		return 0;

	int iVersion = 0;
	const wchar_t *szVersion = m_pRootNode->szGetAttributeValue (g_szWideFileVersionAttribute);
	if (!szVersion)
		return 0;
	
	iVersion = wcstoul (szVersion, null, 10);

	return iVersion;
}


/*****************************************************************************
 ** XmlInput::iGetSerializationVersion 
/****************************************************************************/
int XmlInput::iGetSerializationVersion ()
{
	if (!m_pRootNode)
		return 0;

	int iVersion = 0;
	const wchar_t *szVersion = m_pRootNode->szGetAttributeValue (g_szWideSerializationVersionAttribute);
	if (!szVersion)
		return 0;
	
	iVersion = wcstoul (szVersion, null, 10);

	return iVersion;
}



/****************************************************************************/
/** StreamReader::StreamReader
****************************************************************************/
StreamReader::StreamReader () : m_pBaseNode (null), m_pCurrentNode (null)
{
}


/****************************************************************************/
/** StreamReader::~StreamReader
****************************************************************************/
StreamReader::~StreamReader ()
{
	delete m_pBaseNode;
}


/****************************************************************************/
/** StreamReader::vStartElement 
****************************************************************************/
void StreamReader::vStartElement (const wchar_t *pwchLocalName)
{
	Node *pNewNode = new Node (pwchLocalName);

	if (!m_pBaseNode)
		m_pBaseNode = pNewNode;
	else
		m_pCurrentNode->vAddChild (pNewNode);

	m_pCurrentNode = pNewNode;
}


/****************************************************************************/
/** StreamReader::vEndElement 
****************************************************************************/
void StreamReader::vEndElement (const wchar_t *pwchLocalName)
{
	if (m_pCurrentNode)
		m_pCurrentNode = m_pCurrentNode->pGetParent ();
}


/****************************************************************************/
/** StreamReader::vCharacters
****************************************************************************/
void StreamReader::vCharacters (const wchar_t *_pwchData, int iCharacters)
{
	bool fUse = false;

	for (int i = 0; i < iCharacters; i++)
	{
		if (iswalnum (_pwchData[i]))
		{
			fUse = true;
			break;
		}
	}

	if (!fUse)
		return;

	m_pCurrentNode->vAddContent (_pwchData, iCharacters);
}


/****************************************************************************/
/** This class implements the ISAXContentHandler interface, declared in the 
 ** MSXML DLL (this declaration was generated from MSXML type info).  
 **
 ** Most of its methods are unimplemented, and the few that are implemented 
 ** basically just delegate to methods of XmlStreamReader.
/****************************************************************************/

class XML::StreamReaderSaxHandler : public ISAXContentHandler
{
	/// This points to the object that will do most of the 'real' work
	StreamReader *m_pReader;

public:
	StreamReaderSaxHandler (StreamReader *pReader) : m_pReader (pReader) {}
	~StreamReaderSaxHandler () {}

protected:
	/// This must be correctly implemented if the handler needs to be a COM Object (in this case it does not)
	long __stdcall QueryInterface(const struct _GUID &,void ** ) { return 0; }
		unsigned long __stdcall AddRef(void) { return 0; }
		unsigned long __stdcall Release(void) { return 0; }


	/// Uninteresting...
	virtual HRESULT STDMETHODCALLTYPE putDocumentLocator( 
			/* [in] */ ISAXLocator __RPC_FAR *pLocator)
		{ return S_OK; }
	
	/// Uninteresting...
	virtual HRESULT STDMETHODCALLTYPE startDocument( void)
	    { return S_OK; }
	    
	/// Uninteresting...
	virtual HRESULT STDMETHODCALLTYPE endDocument( void)
		{ return S_OK; }
	    
	/// Uninteresting...
	virtual HRESULT STDMETHODCALLTYPE startPrefixMapping( 
			/* [in] */ const wchar_t __RPC_FAR *pwchPrefix,
			/* [in] */ int cchPrefix,
			/* [in] */ const wchar_t __RPC_FAR *pwchUri,
			/* [in] */ int cchUri)
		{ return S_OK; }
	    
	/// Uninteresting...
	virtual HRESULT STDMETHODCALLTYPE endPrefixMapping( 
			/* [in] */ const wchar_t __RPC_FAR *pwchPrefix,
			/* [in] */ int cchPrefix)
		{ return S_OK; }
	    
	/// This is just delegated to the StreamReader
	virtual HRESULT STDMETHODCALLTYPE startElement( 
			/* [in] */ const wchar_t __RPC_FAR *pwchNamespaceUri,
			/* [in] */ int cchNamespaceUri,
			/* [in] */ const wchar_t __RPC_FAR *_pwchLocalName,
			/* [in] */ int cchLocalName,
			/* [in] */ const wchar_t __RPC_FAR *pwchRawName,
			/* [in] */ int cchRawName,
			/* [in] */ ISAXAttributes __RPC_FAR *pAttributes)
	{
		// Only do stuff if there's valid object to delegate to
		if (m_pReader)
		{
			// Copy the tag name for zero-termination
			wchar_t *pwchLocalName = (wchar_t*) alloca (cchLocalName*3);
			memcpy (pwchLocalName, _pwchLocalName, cchLocalName * 2);
			pwchLocalName[cchLocalName] = 0;

			m_pReader->vStartElement (pwchLocalName);

			if (m_pReader->m_pCurrentNode)
			{
				int iLength = 0;
				if (SUCCEEDED (pAttributes->getLength (&iLength)))
				{
					for (int i = 0; i < iLength; i++)
					{
						const wchar_t *szUri = 0, *szLocalName = 0, *szQName = 0, *szValue = 0;
						int cchUri = 0, cchLocalName = 0, cchQName = 0, cchValue = 0;

						if (SUCCEEDED (pAttributes->getName (i, 
							&szUri, &cchUri, 
							&szLocalName, &cchLocalName, 
							&szQName, &cchQName)) &&
							SUCCEEDED (pAttributes->getValue (i, &szValue, &cchValue)))
						{
							m_pReader->m_pCurrentNode->AddAttribute (szLocalName, cchLocalName, szValue, cchValue);
						}
					}
				}
			}
		}
		return S_OK;
	}
	    
	/// This is just delegated to the StreamReader
	virtual HRESULT STDMETHODCALLTYPE endElement( 
			/* [in] */ const wchar_t __RPC_FAR *pwchNamespaceUri,
			/* [in] */ int cchNamespaceUri,
			/* [in] */ const wchar_t __RPC_FAR *_pwchLocalName,
			/* [in] */ int cchLocalName,
			/* [in] */ const wchar_t __RPC_FAR *pwchRawName,
			/* [in] */ int cchRawName)
	{
		// Only do stuff if there's valid object to delegate to
		if (m_pReader)
		{
			// Copy the tag name for zero-termination
			wchar_t *pwchLocalName = (wchar_t*) alloca (cchLocalName*3);
			memcpy (pwchLocalName, _pwchLocalName, cchLocalName * 2);
			pwchLocalName[cchLocalName] = 0;

			m_pReader->vEndElement (pwchLocalName);
		}
		return S_OK;
	}
	
	/// This is just delegated to the StreamReader
	virtual HRESULT STDMETHODCALLTYPE characters( 
			/* [in] */ const wchar_t __RPC_FAR *pwchChars,
			/* [in] */ int cchChars)
	{
		if (m_pReader)
			m_pReader->vCharacters (pwchChars, cchChars);

		return S_OK;
	}

	/// Uninteresting...
	virtual HRESULT STDMETHODCALLTYPE ignorableWhitespace( 
			/* [in] */ const wchar_t __RPC_FAR *pwchChars,
			/* [in] */ int cchChars)
		{ return S_OK; }
	
	/// Uninteresting...
	virtual HRESULT STDMETHODCALLTYPE processingInstruction( 
			/* [in] */ const wchar_t __RPC_FAR *pwchTarget,
			/* [in] */ int cchTarget,
			/* [in] */ const wchar_t __RPC_FAR *pwchData,
			/* [in] */ int cchData)
		{ return S_OK; }
	    
	/// Uninteresting...
	virtual HRESULT STDMETHODCALLTYPE skippedEntity( 
			/* [in] */ const wchar_t __RPC_FAR *pwchName,
			/* [in] */ int cchName)
		{ return S_OK; }
};


/****************************************************************************/
/** Implements the ISAXErrorHandler interface set forth in MSXML.DLL.
 **
 ** TODO: at least use OutputDebugString to log the errors.
/****************************************************************************/
class StreamReaderErrorHandler : public ISAXErrorHandler  
{
public:
	StreamReaderErrorHandler() {}
	virtual ~StreamReaderErrorHandler() {}

	/// This must be correctly implemented, if your handler must be a COM Object (in this example it does not)
	long __stdcall QueryInterface(const struct _GUID &,void ** ) { return 0; }
		unsigned long __stdcall AddRef(void) { return 0; }
		unsigned long __stdcall Release(void) { return 0; }

	/// Uninteresting
    virtual HRESULT STDMETHODCALLTYPE error( 
            /* [in] */ ISAXLocator __RPC_FAR *pLocator,
            /* [in] */ const wchar_t *pwchErrorMessage,
			/* [in] */ HRESULT errCode)
		{
			return S_OK; 
		}
        
	// Uninteresting
    virtual HRESULT STDMETHODCALLTYPE fatalError( 
            /* [in] */ ISAXLocator __RPC_FAR *pLocator,
            /* [in] */ const wchar_t *pwchErrorMessage,
			/* [in] */ HRESULT errCode)
		{
			return S_OK; 
		}
       
	// Uninteresting
    virtual HRESULT STDMETHODCALLTYPE ignorableWarning( 
            /* [in] */ ISAXLocator __RPC_FAR *pLocator,
            /* [in] */ const wchar_t *pwchErrorMessage,
			/* [in] */ HRESULT errCode)
		{
			return S_OK; 
		}
};


/****************************************************************************/
/** XmlStreamReader::fRead 
 **
 ** This method opens an XML stream on the file at the given path, then
 ** uses the given Serializable object to read the contents of the file.
 ** A string-assignment function may optionally be specified - it will be
 ** used to allocate buffers and copy strings.
 **
 ** TODO: Get the "Juice" name out of the error message.  Maybe get rid of
 ** the error MessageBox altogether and just throw a string?
*****************************************************************************/
bool StreamReader::fRead (const char *szPath, Serializable *pObject, StringAssignFunc pfn)
{
	// Expect success.  Think positive.
	bool fResult = true;

	// This is necessary before any COM stuff happens, and the XML reader is a COM object
	CoInitialize(NULL); 

	// Create the XML reader
	ISAXXMLReader* pRdr = NULL;
	CLSID clsidSAX;
	CLSID clsidSAX_hardcoded = __uuidof(SAXXMLReader);

    CLSIDFromProgID (L"Msxml2.SAXXMLReader.6.0", &clsidSAX);

	HRESULT hr = CoCreateInstance(
						clsidSAX,
						NULL, 
						CLSCTX_ALL, 
						__uuidof(ISAXXMLReader), 
						(void **)&pRdr);

	// This is what happens when people don't read the installation instructions.
	if (FAILED (hr))
	{
		MessageBox (NULL, "Can't find the MSXML stuff.  Can't load files without that.\nAre you sure it's properly installed?", "Unable to create SAX XML reader.", MB_OK);
		return false;
	}

	// Create the Juice half of the XML reader/parser
	StreamReaderSaxHandler *pContentHandler = new StreamReaderSaxHandler (this);
	hr = pRdr->putContentHandler(pContentHandler);

	// This is for error handling, it's pretty much not ignored so far though
	StreamReaderErrorHandler * pErrorHandler = new StreamReaderErrorHandler ();
	hr = pRdr->putErrorHandler(pErrorHandler);

	// http URLs can be loaded directly; anything else is assumed to be a local file
	std::string strPath;

	if (!strstr (szPath, "http://"))
	{
		strPath = "file:///";
		strPath += szPath;
	}
	else
	{
		strPath = szPath;
	}

	// put the URL into wide (unicode) format
	const char *szURL = strPath.c_str();
	static wchar_t wszURL[1000];
	mbstowcs( wszURL, szURL, 998 );

	// Build DOM from XML
	hr = pRdr->parseURL(wszURL);

	// Create Stream object
	XmlInput TheRestOfTheParser (m_pBaseNode, pfn);
	Stream Input (&TheRestOfTheParser, pfn);

	// Load object from stream
	pObject->vSerialize (Input);

	// Get file and serialization version numbers
	int iVersion = TheRestOfTheParser.iGetSerializationVersion ();
	m_iFileVersion = TheRestOfTheParser.iGetFileVersion ();

	// Reset pointers if the file was created by recent version of the serialization code
	if (iVersion > 1)
		TheRestOfTheParser.vFixPointers ();

	// Cleanup
	pRdr->Release();

	delete pErrorHandler;
	delete pContentHandler;

	return (!(FAILED(hr)));
}

// XMLInput.cpp ends here ->