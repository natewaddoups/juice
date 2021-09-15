/*****************************************************************************
Project:  Generic library
Filename: GenericLog.h

Copyright (c) 2001 Nate Waddoups, coding@natew.com

*****************************************************************************/

#ifndef __GenericLog__
#define __GenericLog__

namespace Generic
{
/****************************************************************************/
/** 
*****************************************************************************/
	class Log : public ostream
	{
	protected:
		virtual basic_ostream& write (const char *sz, streamsize count);
		virtual basic_ostream& put(char ch);

		std::ofstream m_File;

	public:

		Log ();
		~Log ();

		bool fOpen (const char *szPath);
		void vClose ();

		enum OutputMode
		{
			omInvalid = 0, omStdErr, omFile
		};

		void vSetOutputMode (OutputMode om);

	protected:
		OutputMode m_eOutputMode;
	};

	extern Log g_Log;

#ifdef DEBUG
#define GenericLog(x) { g_Log << x }
#else
#define GenericLog(x)
#endif

};

#endif

// JuiceTypes.h ends here ->