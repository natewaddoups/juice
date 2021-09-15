/*****************************************************************************

Project:  Juice
Module:   Generics.dll
Filename: GenericCommand.h (unused, untested)

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

Generics.dll and its source code are free software.  Except as noted, you 
may redistribute this code under the terms of the Gnu General Public License,
version 2 (GPL2).   This license shall be terminated if any part of GPL2 
should be overturned in court (any jurisdiction).  In that case, contact the 
author for updated license terms.

*****************************************************************************/
#ifndef __GenericCommand__
#define __GenericCommand__

namespace Generic
{

/****************************************************************************/
/** This is the base class from which all other command classes are derived.
*****************************************************************************/
class Command
{
protected:
	/// Text that will appear after "Redo " or "Undo " on the Edit menu
	std::string m_strMenuText;

	/// whether this command has been invoked
	bool m_fInvoked;

    virtual void vInvokeInternal () = 0;
	virtual void vUndoInternal () = 0;

public:
	Command ();
	virtual ~Command ();

	void vInvoke ();

	virtual bool fCanUndo () = 0;

	void vUndo ();

	std::string& strMenuText ();
};

/****************************************************************************/
/** This class maintains the undo and redo command lists.
*****************************************************************************/
class CommandHistory
{
	std::list<Command*> m_UndoList;
	std::list<Command*> m_RedoList;

public:
	CommandHistory ();
	virtual ~CommandHistory ();

	/// Add an already-executed command to the history, empty the Redo list
	void vAdd (Command *pCommand);

	/// Undo the most recent command, and move the command to the redo list.
	void vUndo ();

	/// Redo the most recent undo, and move the command to the undo list.
	void vRedo ();

	/// Returns true if there are undoable commands in the undo list.
	bool fCanUndo ();

	/// Returns true if there are doable commands in the redo list.
	bool fCanRedo ();
};

}; // end of Juice namespace

#endif

// GenericCommand.h ends here ->