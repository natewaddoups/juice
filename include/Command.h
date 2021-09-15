/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: Command.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#ifndef __GenericCommand__
#define __GenericCommand__

namespace Juice
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
	~Command ();

	void vInvoke ();

	virtual bool fCanUndo ();
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
	!CommandHistory ();

	/// Adds an already-executed command to the history
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

// Command.h ends here ->