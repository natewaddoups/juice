/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: DynamicsFactories.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#include <JuiceEngine.h>
#include <Model.h>

using namespace Juice;

/****************************************************************************/
/** 
*****************************************************************************/
DynamicsLibraryList::Info::Info (const char *szDisplayName, const wchar_t *szXmlTag, const char *szModuleName, Generic::INT32 iWorldID, bool fShowError) :
	m_szDisplayName (szDisplayName),
	m_szXmlTag (szXmlTag),
	m_szModuleName (szModuleName),
	m_hDLL (null),
	m_CreateWorld (null),
	m_pWorld (null)	
{
	m_hDLL = LoadLibrary (m_szModuleName);
    if (!m_hDLL) 
	{
		if (fShowError)
		{
			TCHAR *szSystemMessage = 0;

			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &szSystemMessage,
				0,
				NULL 
			);

			char szCaption[MAX_PATH + 100];
			wsprintf (szCaption, "Error Loading %s", szModuleName);

			MessageBox (NULL, szSystemMessage, szCaption, MB_OK);

			LocalFree (szSystemMessage);
		}

		return;
	}

    m_CreateWorld = (FactoryProc) GetProcAddress (m_hDLL, "CreateWorld");
	if (!m_CreateWorld)
		return;
    
	m_pWorld = m_CreateWorld (iWorldID);
}

/****************************************************************************/
/** 
*****************************************************************************/
DynamicsLibraryList::Info::~Info ()
{
	if (m_pWorld)
		m_pWorld->vDelete ();

	if (m_hDLL)
		FreeLibrary (m_hDLL);
}

/****************************************************************************/
/** 
*****************************************************************************/
DynamicsLibraryList::DynamicsLibraryList () :
	m_pActiveLibrary (null)
{
	// the first one will be the default active factory
	Info *p = new Info ("ODE/Standard", L"ODE.Standard", "JuiceDynamicsODE.dll", 0, true);
	if (p->pGetWorld ())
		m_Libraries.push_back (p);
	else
		delete p;

	p = new Info ("ODE/Iterative", L"ODE.Iterative", "JuiceDynamicsODE.dll", 1);
	if (p->pGetWorld ())
		m_Libraries.push_back (p);
	else
		delete p;

	p = new Info ("Novodex", L"Novodex", "JuiceDynamicsNovodex.dll", 0);
	if (p->pGetWorld ())
		m_Libraries.push_back (p);
	else
		delete p;

	// Get the default library
	if (m_Libraries.size ())
		m_pActiveLibrary = m_Libraries.front ();
}

DynamicsLibraryList::~DynamicsLibraryList ()
{
	//std::for_each (m_Libraries.begin (); m_Libraries.end (); DeleteWrapper);
	while (m_Libraries.size ())
	{
		Info *pInfo = m_Libraries.front ();
		m_Libraries.pop_front ();
		delete pInfo;
	}
}

void DynamicsLibraryList::vSerialize (XML::Stream &Stream)
{
	if (Stream.eGetOperation () == XML::Stream::Operation::oWrite)
	{
		DynamicsLibraryList::List::iterator iter, iterLast = m_Libraries.end ();
		for (iter = m_Libraries.begin (); iter != iterLast; iter++)
		{
			DynamicsLibraryList::Info *pInfo = *iter;
			XML::Serializable &Constants = pInfo->pGetWorld ()->GetConstants ();

			XML::XmlOutput::Container Container (*Stream.m_pOutput, pInfo->szGetXmlTag ());
            Constants.vSerialize (Stream);
		}
	}
	else
	{
		const XML::Node::List &Children = Stream.m_pInput->GetChildren ();
		XML::Node::List::const_iterator iter, iterLast = Children.end ();

		for (iter = Children.begin (); iter != iterLast; iter++)
		{
			XML::Node *pNode = *iter;

			DynamicsLibraryList::List::iterator iter, iterLast = m_Libraries.end ();
			for (iter = m_Libraries.begin (); iter != iterLast; iter++)
			{
				DynamicsLibraryList::Info *pInfo = *iter;

				if (!wcscmp (pNode->szGetName (), pInfo->szGetXmlTag ()))
				{
					Serializable &Constants = pInfo->pGetWorld ()->GetConstants ();

					Stream.m_pInput->vRead (Stream, pNode, &Constants);

//					Stream.m_pInput = pNode;
//					Constants.vSerialize (Stream);
//					Stream.m_pInput = pList;
				}
			}
		}
	}
}

// DynamicsFactories.cpp ends here ->