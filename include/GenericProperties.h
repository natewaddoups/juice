/*****************************************************************************

Project:  Juice
Module:   Generics.dll
Filename: GenericProperties.h 

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

Generics.dll and its source code are free software.  Except as noted, you 
may redistribute this code under the terms of the Gnu General Public License,
version 2 (GPL2).   This license shall be terminated if any part of GPL2 
should be overturned in court (any jurisdiction).  In that case, contact the 
author for updated license terms.

*****************************************************************************/
#ifndef __GenericProperties__
#define __GenericProperties__

namespace Generic
{
	class PropertyTree;
	class HasProperties;

	typedef void (HasProperties::*fnSetBool)   (bool f);
	typedef void (HasProperties::*fnSetInt)    (int i);
	typedef void (HasProperties::*fnSetReal)   (real r);
	typedef void (HasProperties::*fnSetString) (const char *sz);

	typedef bool (HasProperties::*fnGetBool) ();
	typedef int  (HasProperties::*fnGetInt)  ();
	typedef real (HasProperties::*fnGetReal) ();
	typedef const char* (HasProperties::*fnGetString) ();

/****************************************************************************/
/** The Property class is a key part of the infrastructure behind the 
 ** property pane in Juice.  Each item in the pane is handled by an object of
 ** a class derived from Property.  
 **
 ** Property is an abstract base class; in order to use it, you must derive 
 ** a concrete class.  The derived class should include a member variable to
 ** hold the actual property value.
 **
 ** Derived classes for boolean, integer, real, string, and list properties 
 ** are provided.  The application derives classes for other types, e.g.
 ** pointers to bodies and joints.
*****************************************************************************/

	class GenericsExport Property
	{
	protected:
		/// This is a bit of a hack; keeping a local copy of the value string
		/// stops the EXE from trying to delete something on the DLL's heap.
		/// Note that derived classes in the same DLL can access this without
		/// causing problems, but it's const so that derived classes in other
		/// DLLs won't be tempted to assign new values to it.
		const std::string m_strValueString;

		/// This is only here to circumvent the const attribute on m_strValueString (see notes on m_strValueString)
		void vSetValueString (const char *sz);

		/// Name of the property, for use in the UI
		std::string m_strName;

		/// Reference count for memory management
		Generic::UINT32 m_uReferenceCount;

		/// User-definable attribute flags
		Generic::UINT32 m_uFlags;

		/// Making the destructor protected forces the client to use vRelease instead
		virtual ~Property ();

	public:
		/// Constructor
		Property (const char *szName, Generic::UINT32 uFlags);

		/// Reference count increment
		void vAddReference ();

		/// Reference count decrement
		void vRelease ();

		/// Convert value to string for display in the UI
		virtual const char* szGetValueToString (HasProperties *pObject) = 0;

		/// Convert string from the UI to binary value in this property object
		virtual bool fSetValueFromString (HasProperties *pObject, const char *sz) = 0;

		/// Indicates whether or not the property can be set by the user
		virtual bool fReadOnly () { return false; }

		/// Returns the property name
		const char* szGetName () const { return m_strName.c_str (); }

		/// Returns the property's flags
		Generic::UINT32 uGetFlags () { return m_uFlags; }
	};

/****************************************************************************/
/**
*****************************************************************************/
	class GenericsExport BoolProperty : public Property
	{
		fnGetBool m_fnGet;
		fnSetBool m_fnSet;

	public:
		BoolProperty (const char* szName, fnGetBool fnGet, fnSetBool fnSet, Generic::UINT32 uFlags = 0);

		virtual const char* szGetValueToString (HasProperties *pObject);
		virtual bool fSetValueFromString (HasProperties *pObject, const char *sz);
	};

/****************************************************************************/
/**
*****************************************************************************/
	class GenericsExport IntProperty : public Property
	{
		fnGetInt m_fnGet;
		fnSetInt m_fnSet;

	public:
		IntProperty (const char *szName, fnGetInt fnGet, fnSetInt fnSet, Generic::UINT32 uFlags = 0);
        
		virtual const char *szGetValueToString (HasProperties *pObject);
		virtual bool fSetValueFromString (HasProperties *pObject, const char *sz);
	};


/****************************************************************************/
/**
*****************************************************************************/
	class GenericsExport RealProperty : public Property
	{
		fnGetReal m_fnGet;
		fnSetReal m_fnSet;

	public:
		RealProperty (const char* szName, fnGetReal fnGet, fnSetReal fnSet, Generic::UINT32 uFlags = 0);

		virtual const char* szGetValueToString (HasProperties *pObject);
		virtual bool fSetValueFromString (HasProperties *pObject, const char *sz);
	};

/****************************************************************************/
/**
*****************************************************************************/
	class GenericsExport StringProperty : public Property
	{
		fnGetString m_fnGet;
		fnSetString m_fnSet;

	public:
		StringProperty (const char* szName, fnGetString fnGet, fnSetString fnSet, Generic::UINT32 uFlags = 0);

		virtual const char *szGetValueToString (HasProperties *pObject);
		virtual bool fSetValueFromString (HasProperties *pObject, const char *sz);

		virtual bool fReadOnly ();
	};

/****************************************************************************/
/**
*****************************************************************************/
	class GenericsExport ListProperty : public Property
	{
		fnGetInt m_fnGet;
		fnSetInt m_fnSet;

		class Item
		{
		public:
			Item (int iID, const char *szText);
			int m_iID;
			std::string m_strText;
		};

		std::list<Item> m_lsItems;

	public:
		ListProperty (const char *szName, fnGetInt fnGet, fnSetInt fnSet, Generic::UINT32 uFlags = 0);
        
		virtual const char *szGetValueToString (HasProperties *pObject);
		virtual bool fSetValueFromString (HasProperties *pObject, const char *sz);

		virtual bool fSetValueFromInt (HasProperties *pObject, int iID);
		virtual int iGetValueToInt (HasProperties *pObject);

		void vAddItem (int iID, const char *szText);
		Generic::UINT32 uItems ();

		bool fGetItem (Generic::UINT32 uItem, int *piID, const char **pszText);
	};

/****************************************************************************/
/** Causes a function to be called, rather than dialog display
*****************************************************************************/	

	typedef void (Generic::HasProperties::*fnInvoke) ();

	class GenericsExport ActionProperty : public Generic::Property
	{
		fnInvoke m_fnInvoke;

	public:
		ActionProperty (const char *zzName, fnInvoke fn, Generic::UINT32 uFlags = 0);
		virtual ~ActionProperty ();

		virtual const char* szGetValueToString (HasProperties *pObject) { return ""; }
		virtual bool fSetValueFromString (HasProperties *pObject, const char *sz) { return false; }

		virtual void vInvoke (HasProperties *pObject);
	};


/****************************************************************************/
/** Objects whose properties are to appear in the property window must derive
 ** from this class.  It basically provides an interface through which the 
 ** property window can retrieve the object's property tree.
*****************************************************************************/
	class GenericsExport HasProperties
	{
	protected:
		friend class Property;
	
	public:
		HasProperties ();
		virtual ~HasProperties ();

		virtual PropertyTree* pGetPropertyTree () = 0; 
	};

/****************************************************************************/
/** This is a simple two-level tree.  A multi-level implementation would
*** just be a matter of replacing the property list (m_lsProperties) with a
*** list of nodes.
*****************************************************************************/
	class GenericsExport PropertyTree
	{
	public:
		class GenericsExport Node
		{
		public:
			enum Type
			{
				ntGroup,
				ntProperty
			};

		protected:
			Type m_eType;

			// Group traits
			const std::string m_strName;
			std::list<Property*> m_lsProperties;

			// Property traits
			Property *m_pProperty;

		public:
			Node (const char* szName, const std::list<Property*> &lsProperties);
			Node (Generic::Property *pNewProperty);
			Node (Node &Source);
			virtual ~Node ();

			Type eGetType () const { return m_eType; }

			// Get group info
			const char* szGetName () const;
			const std::list<Property*>& lsGetProperties () const;

			// Get property info
			Property* pGetProperty () const;
		};

	protected:
		std::list <Node*> m_Nodes;

	public:
		PropertyTree ();
		PropertyTree (PropertyTree &Source);
		virtual ~PropertyTree ();

		void vAddNode (const char *szName, const std::list<Property*> &lsProperties);
		void vAddNode (Property *pNewProperty);

		const std::list<Node*>& lsGetNodes () { return m_Nodes; }
	};

/****************************************************************************/
/** The property trees tend to have the application's lifetime, so it's most
*** convenient to delete them from the destructor of a static object.  This
*** not-exceptionally-smart-pointer class was SUPPOSED to take care of that,
*** but it seems that memory leaks are dumped before static object dtors are
*** called, so it's not really all that useful.  In order to get a clean 
*** memory leak report, you still have to call the delete method explicitly.
*****************************************************************************/

	class PropertyTreePtr
	{
		PropertyTree* m_pTree;
	public:
		PropertyTreePtr () : m_pTree (null) {}
		~PropertyTreePtr () { vDelete (); }

		void vDelete () { delete m_pTree; m_pTree = null; }

		PropertyTreePtr& operator= (PropertyTree *pTree) { m_pTree = pTree; return *this; }
		operator PropertyTree* () { return m_pTree; }
		PropertyTree* operator-> () { return m_pTree; }
	};

};
#endif

// GenericProperties.h ends here ->