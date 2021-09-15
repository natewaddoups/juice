/*****************************************************************************

Project:  Juice
Module:   Generics.dll
Filename: GenericCommand.h (little used, little tested)

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

Generics.dll and its source code are free software.  Except as noted, you 
may redistribute this code under the terms of the Gnu General Public License,
version 2 (GPL2).   This license shall be terminated if any part of GPL2 
should be overturned in court (any jurisdiction).  In that case, contact the 
author for updated license terms.

*****************************************************************************/
#ifndef __GenericError__
#define __GenericError__

#include <list>

namespace Generic
{

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)

#define throw_Exception(msg) throw Exception (__WFILE__ , __LINE__, msg);
#define AddExceptionMessage(e,msg) e.vAddMessage (__WFILE__, __LINE__, msg);

/****************************************************************************/
/** A class to throw around when bad things happen.  
*****************************************************************************/
	class GenericsExport Exception
	{
		class Message
		{
		public:
			Message (wchar_t *szFileName, int iLineNumber, wchar_t *szMessage)  :
				m_szFileName (szFileName), m_iLineNumber (iLineNumber)
			{
				m_szMessage = new wchar_t [wcslen (szMessage) + 2];
				wcscpy (m_szMessage, szMessage);
			}

			~Message ()
			{
				delete [] m_szMessage;
			}

			wchar_t *m_szMessage;
			wchar_t *m_szFileName;
			int m_iLineNumber;
		};

		std::list<Message*> m_Messages;

	public:

		// Default
		Exception () { }

		// Add a text message
		Exception (wchar_t *szFileName, int iLineNumber, wchar_t *szMessage) 
		{ vAddMessage (szFileName, iLineNumber, szMessage); }
       
#ifdef _WINDOWS
#ifdef _WINBASE_
		// Add a message from a Win32 error code
		Exception (wchar_t *szFileName, int iLineNumber, unsigned long uLastError)
		{
			vAddMessage (szFileName, iLineNumber, uLastError);
		}

		void vAddMessage (wchar_t *szFileName, int iLineNumber, unsigned long uLastError)
		{
			wchar_t *szSystemMessage = 0;

			FormatMessageW ( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				szSystemMessage,
				0,
				NULL 
			);

			vAddMessage (szFileName, iLineNumber, szSystemMessage);

			LocalFree (szSystemMessage);
		}
#endif
#endif

		~Exception ()
		{
			std::list<Message*>::iterator iter, iterLast = m_Messages.end ();
			for (iter = m_Messages.begin (); iter != iterLast; iter++)
			{
				delete *iter;
			}

			m_Messages.empty ();
		}
		
		void vAddMessage (wchar_t *szFileName, int iLineNumber, wchar_t *szMessage)
		{
			Message *pNewMessage = new Message (szFileName, iLineNumber, szMessage);
			m_Messages.push_back (pNewMessage);
		}

		// TODO: revise this to add filenames and line numbers
		std::wstring strGetMessages ()
		{
			std::wstring str;

			std::list<Message*>::iterator iter, iterLast = m_Messages.end ();
			for (iter = m_Messages.begin (); iter != iterLast; iter++)
			{
				str.append ((*iter)->m_szMessage);
				str.append (L"\n");
			}

			return str;
		}
	};

	class OutOfMemoryException
	{
		//const wchar_t *m_szFunction;
		const wchar_t *m_szFileName;
		const wchar_t *m_szMessage;
		int m_iLine;

	public:
		OutOfMemoryException (const wchar_t *szFileName, int iLine, const wchar_t *szMessage)
		{
			m_szFileName = szFileName;
			m_iLine = iLine;
			m_szMessage = szMessage;
		}

		//const wchar_t* szFunction () { return m_szFunction; }
		const wchar_t* szFileName () { return m_szFileName; }
		const wchar_t* szMessage () { return m_szMessage; }
	};

#endif

}; // end of Generic namespace

// GenericError.h ends here ->