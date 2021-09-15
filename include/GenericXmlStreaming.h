/*****************************************************************************

Project:  Juice
Module:   Generics.dll
Filename: GenericXmlStreaming.h 

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

Generics.dll and its source code are free software.  Except as noted, you 
may redistribute this code under the terms of the Gnu General Public License,
version 2 (GPL2).   This license shall be terminated if any part of GPL2 
should be overturned in court (any jurisdiction).  In that case, contact the 
author for updated license terms.

*****************************************************************************/

#ifndef __GenericXmlStreaming__
#define __GenericXmlStreaming__

#include <list>
#include <hash_map>  // used by XmlInput class
#include <algorithm> // used by XmlOutput class
//#include <fstream>
//#include <malloc.h> // for alloca

using namespace std;
using namespace stdext;

/*****************************************************************************
 ** TODO: separate the XML-helper functionality from the file-specific 
 ** functionality, so it can be used to write to other media, e.g. across 
 ** networks.
/****************************************************************************/

namespace Generic {	
	
/****************************************************************************/
/** Support for serializing objects to XML and deserializing them back into 
 ** memory.
*****************************************************************************/
	
namespace XML {

class Node;
class Attribute;
class XmlInput;
class XmlOutput;
class Stream;
class StreamReader;
class StreamWriter;

/****************************************************************************/
/** This string assignment function is used to avoid memory management 
*** conflicts in multi-heap situations.
*****************************************************************************/
typedef void (*StringAssignFunc) (string *pDest, const char *pSrc);

class GenericsExport Attribute
{
private:
	wchar_t *m_szName;
	wchar_t *m_szValue;

public:
	Attribute (const wchar_t* szGivenName, int cchName, const wchar_t *szGivenValue, int cchValue);
	~Attribute ();
    
	const wchar_t* szGetName () { return m_szName; }
	const wchar_t* szGetValue () { return m_szValue; }
};

/****************************************************************************/
/** This represent a node in a loaded XML document.  This currently supports
 ** only the content between <tag> and </tag>.  Attributes would be nice, and
 ** support should be added later.
*****************************************************************************/
class GenericsExport Node
{
private:
	typedef list <Attribute*> AttributeList;

	wchar_t *m_szName;
	wchar_t *m_szContent;
	UINT32 m_nContentLength;
	Node *m_pParent;
	AttributeList m_Attributes;

public:
	/// Constructor sets the name of the node
	Node (const wchar_t *szName);

	/// Destructor
    virtual ~Node ();

	/// This method appends new content to the node's content string
	void vAddContent (const wchar_t *pContent, Generic::UINT32 nCharacters);

	/// Return the node's name
	const wchar_t* szGetName ();

	/// Return the node's content string
	const wchar_t* szGetContent ();

	/// Return a pointer to the node's parent Node
	Node *pGetParent ();

	/// Add the given node to this node's list of children
	void vAddChild (Node *pChild);

	/// Indicate whether or not the given name is equal to this node's name
	int operator== (const wchar_t *szCompare);

	/// Return a pointer to the child node with the given name (if one exists)
	Node* pGetChild (const wchar_t *szName);

	/// Indicate whether or not the give node has the same name as this node
	int operator== (Node &Val);

	/// This just simplifies syntax a bit
	typedef list <Node*> List;

	/// Return this node's list of children
	const List& GetChildren () { return m_Children; }

	/// Add an attribute to the node's list of attributes
	void AddAttribute (const wchar_t* szName, int cchName, const wchar_t* szValue, int cchValue);

	const wchar_t *szGetAttributeValue (const wchar_t *szName);

protected:

	/// This node's children
	List m_Children;
};


/****************************************************************************/
/** All objects that will be serialized to and from XML must derive from this
 ** base class.  
*****************************************************************************/

class Serializable
{
private:

	/// This is the name under which objects will be saved
	const wchar_t *m_szPreferredTagName;

	/// This is the name under which objects may have been saved in the past - it's retained to aid backward compatibility
	const wchar_t *m_szObsoleteTagName;

public:

	/// Constructor
	Serializable (const wchar_t *szPreferredTagName = null, const wchar_t *szObsoleteTagName = null) : 
		m_szPreferredTagName (szPreferredTagName), 
		m_szObsoleteTagName (szObsoleteTagName)
	{
	}

	/// Returns the object's preferred XML tag
	const wchar_t *szGetPreferredTagName () const { return m_szPreferredTagName; }

	/// Set's the object's preferred XML tag
	void vSetTagName (const wchar_t *szPreferredTagName) { m_szPreferredTagName = szPreferredTagName; }

	/// Pure-virtual method for serializing the object.  The implementation 
	/// is expected to call Stream::Serialize to read and write data members
	/// from and to the Stream object.
	virtual void vSerialize (Stream &Stream) = 0;
};


/****************************************************************************/
/** This string constant is required by the XmlInput and XmlOutput classes.
*****************************************************************************/
extern const wchar_t GenericsExport *g_szSerializationID;

/****************************************************************************/
/** This class helps write XML files, and is a component of the bidirectional
 ** "Stream" class, defined below.  It wraps std::ofstream and provides some 
 ** methods that facilitate XML-formatted output.
 **
 ** This class is intended for use primarily only by the Stream classes.  
 ** Application code will not need to use this class directly.
*****************************************************************************/
class GenericsExport XmlOutput
{
	friend class StreamWriter;

	/// XmlOutput exists primarily to provide an XML-friendly interface
	/// to the file-writing functionality of this class.
	ofstream m_File;

	/// Current indent level
	int m_iIndent;

	/// Write m_iIndent tab characters
	void vIndent ();

	/// Write "<szName>"
	void vOpenTag (const wchar_t *szName);

	/// Write "</szName>"
	void vCloseTag (const wchar_t *szName);

	/// You can probably guess
	void vIndentMore ();

	/// You can probably guess this, too
	void vIndentLess ();

	/// Insert a newline character (\n)
	void vNewline ();

	/// Write a boolean
	void vWrite (bool b);

	/// Write an integer
	void vWrite (int i);

	/// Write a string
	void vWrite (string &str);

	/// Write a pointer
	void vWrite (void *pv);

	/// Write an unsigned integer
	void vWrite (unsigned u);

	/// Write a 'real' value (may be single- or double-precision float)
	void vWrite (real r);

public:

	/// Constructor
	XmlOutput ();

	/// Destructor
	virtual ~XmlOutput ();

	/// Open an output file
	bool fOpen (const char *szPath);

	/// Write a primitive data type, surrounded by open and close tags, like <szTag>Foo</szTag>.
	template <class T>
	void vWrite (const wchar_t *szTag, T t)
	{
		vIndent ();

		vOpenTag (szTag);

		vWrite (t); // m_File << t;

		vCloseTag (szTag);
		vNewline ();
	}

	/// Write a Serializable data type.  The object's address is also written,
	/// and it will be used during deserialization to fix pointer relationships
	/// between objects deserialized from the XML document.
	///
	/// The pObject pointer will usually not be equal to the 'true' pointer to 
	/// the object, since it points to a base class.  Therefore, a void pointer
	/// is used to persist the object's true address.  This is critical for
	/// deserialization, when other objects' references to this object will be
	/// via the true pointer, not via pointer to a base class.
	///
	/// I also tried using only the void pointer, but dynamic_cast won't accept
	/// a void pointer and static_cast doesn't properly convert a pointer to an
	/// arbitrary type to a usable Serializable pointer.
	///
	/// TODO: Find a less ugly way to do this.  If I recall correctly, this WAS
	/// working without any of this ugliness before I switched to VC 7.1, but 
	/// that switch happened while I was meddling in this code anyhow so I'm not
	/// totally sure.

	void vWrite (Stream &Stream, Serializable* pObject, void *pObjectPtr, const wchar_t *szTag)
	{
		Container Foo (*this, szTag);
		vWrite (g_szSerializationID, pObjectPtr);

		if (pObject)
		{
			pObject->vSerialize (Stream);
		}
		else
		{
			Breakpoint ("Attempted to write non-Serializable object");
		}
	}

	/// A container indents its children one more tab, with open and close
	/// tags at each end.  Kind of silly really, but it seemed like a neat
	/// idea at the time.
	class GenericsExport Container
	{
		XmlOutput &m_Output;
		const wchar_t *m_szName;

	public:
		Container (XmlOutput &Output, const wchar_t *szName);

		~Container ();
	};

	/// This gives the container class access to the indent level methods
	friend class Container;
};


/****************************************************************************/
/** This class helps read XML document trees, and is a component of the 
 ** bidirectional "Stream" class, defined below.  
 **
 ** This class is intended for use primarily only by the Stream classes.  
 ** Application code will not need to use this class directly.
*****************************************************************************/
class GenericsExport XmlInput
{
private:
	/// The root node of the current XML document
	Node *m_pRootNode;

	/// The node that is currently being used to initialize new objects
	Node *m_pCurrentNode;

	/// This function ensures that string copies are done on the appropriate heap
	StringAssignFunc m_pfnStringAssign;

	/// A map from one void pointer (a serialization ID) to another (a new object's pointer)
	typedef hash_map <void*, void*> PointerMap;

	/// A list of pointers to void pointers
	typedef list<void**> PointerList;

	/// This maps from serialized ID values to the new object's pointer
	PointerMap m_PointerMap;

	/// This is a list of object pointers that have been deserialized with ID values
	PointerList m_PointerList;

	/// Get the serialization ID of the current object
	void vAddPointerMapEntry (void *pObject);

	/// Read a boolean
	void vSetValue (bool *pf, const wchar_t *szValue);

	/// Read an integer
	void vSetValue (int *pi, const wchar_t *szValue);

	/// Read an unsigned integer
	void vSetValue (unsigned int *pu, const wchar_t *szValue);

	/// Read a pointer
	void vSetValue (void *pv, const wchar_t *szValue);

	/// Read a pointer to a pointer, and prepare to reassign the underlying 
	/// pointer to point to an object created during deserialization.
	void vSetValue (void **pv, const wchar_t *szValue);

	/// Read a string
	void vSetValue (string *pstr, const wchar_t *szValue);

	/// Read a 'real' value
    void vSetValue (real *pr, const wchar_t *szValue);

public:
	/// Constructor
	XmlInput (Node *pRootNode, StringAssignFunc pfnStringAssign = null);

	/// Destructor
	virtual ~XmlInput ();

	/// Get the children of the current node
	const Node::List& GetChildren ();

	/// Get the child of the current node with the given tag name
	Node* pGetChild (const wchar_t *szPreferredTag, const wchar_t *szObsoleteTag = null);

	/// Initialize a primitive data type
	template <class Primitive>
	void vRead (Primitive *pPrimitive, const wchar_t *szPreferredTag, const wchar_t *szObsoleteTag)
	{
		// Get the node with the matching name
		Node *pThisNode = pGetChild (szPreferredTag, szObsoleteTag);

		// Initialize the primitive with the value from the node
		if (pThisNode)
			vSetValue (pPrimitive, pThisNode->szGetContent ());
	}

	/// Initialize a Serializable object
	template <class Serializable>
	void vRead (Stream &Stream, Node *pThisNode, Serializable *pObject)
	{
		// Store away a pointer to the current node
		Node *pCurrentNode = m_pCurrentNode;

		// [Spaghetti alert]  Since m_pCurrentNode is Stream.m_pInput->m_pCurrentNode,
		// this line set's the stream's m_pCurrentNode. [Spaghetti alert]
		m_pCurrentNode = pThisNode;

		// Get this object's serialization ID, if it has one
		vAddPointerMapEntry (pObject);

		// Initialize the object with date from the given node (note the aforementioned spaghetti)
		pObject->vSerialize (Stream);

		// Restore the current node pointer 
		m_pCurrentNode = pCurrentNode;
	}

	/// Reassign pointers between objects recently deserialized
	void vFixPointers ();

	/// Returns the FileVersion attribute of the root node
	int iGetFileVersion ();

	/// Returns the SerializationVersion attribute of the root node
	int iGetSerializationVersion ();
};

/****************************************************************************/
/** This object wraps XML input and output streams.
 **
 ** A single object is used for both purposes so that streamable objects need
 ** not implement separate methods for input and output.  The 'stream' object
 ** is composed of an XmlInput and XmlOutput member, only one of which is 
 ** valid, depending on whether stream is being used to serialize to XML or 
 ** deserialize from XML.
*****************************************************************************/
class GenericsExport Stream
{
	/// The sax handler neds to be able to reach in to set the file 
	/// version (which is private).
	friend class StreamReaderSaxHandler;

private:

	/// Version of the XML scheme (written, but not yet read)
	int m_iFileVersion;

public:

	/// Pointer to the output stream (valid when saving, not when loading)
	XmlOutput *m_pOutput;

	/// Pointer to the XML document's current node (valid when loading, not when saving)
	XmlInput *m_pInput;

	/// This function may be used to copy strings (may be valid when loading, but not always)
	StringAssignFunc m_pfnStringAssign;

	/// Possible operations
	enum Operation { oInvalid = 0, oRead, oWrite };

private:

	/// Indicates what the stream is coming in from, or going out to, the disk
	Operation m_eOperation;

public:

	/// Accessor for the operation member
	Operation eGetOperation () { return m_eOperation; }

	/// Constructor for use when writing to an XML file
	Stream (XmlOutput *pOutput) : 
		m_eOperation (oWrite), 
		m_pInput (null),
		m_pOutput (pOutput),
		m_pfnStringAssign (null),
		m_iFileVersion (0)
		{}

	/// Constructor for use when reading from an XML file
	Stream (XmlInput *pInput, StringAssignFunc pfn) : 
		m_eOperation (oRead), 
		m_pInput (pInput),
		m_pOutput (null), 
		m_pfnStringAssign (pfn),
		m_iFileVersion (0)
		{}

	/// Read and write Serializable data types
	void vSerialize (Serializable *pObject, const wchar_t *szPreferredTag, const wchar_t *szObsoleteTag = null)
	{
		if (pObject)
		{
			if (eGetOperation () == Stream::oWrite)
			{
				// Write the object's information to the stream, wrapped in 
				// tags identifying the object class.
				XmlOutput::Container Foo (*m_pOutput, szPreferredTag);

				pObject->vSerialize (*this);
			}
			else
			{
				// Read an object's information from the stream.
				//
				// TODO: pGetChild should actually return a list of nodes.
				// This method should use only the first one, ignoring the 
				// rest.  Lists should not be handled by this class anyhow,
				// see below for serializable list classes.
				Node *pThisNode = m_pInput->pGetChild (szPreferredTag, szObsoleteTag);

				if (pThisNode)
					m_pInput->vRead (*this, pThisNode, pObject);
			}
		}
	}

	/// Read and write primitive data types
	template <class Primitive>
	void vSerialize (Primitive *pObject, const wchar_t *szPreferredTag, const wchar_t *szObsoleteTag = null)
	{
		if (eGetOperation () == Stream::oWrite)
		{
			// Write the value to the stream
			m_pOutput->vWrite (szPreferredTag, *pObject);
		}
		else
		{
			// Read the value from the stream
			m_pInput->vRead (pObject, szPreferredTag, szObsoleteTag);
		}
	}
};


/****************************************************************************/
/** This class helps write objects to XML streams.  The application need only
*** create a StreamWriter, and call the fWrite method.
*****************************************************************************/
class GenericsExport StreamWriter
{
public:
	/// Create a file at the given path, and write the given object to that file.
	/// Version number is optional, maybe even deprecated.
	bool fWrite (const char *szPath, Serializable *pObject, int iVersion = 0);
};


/****************************************************************************/
/** This class helps read objects from XML stream.  The application need only
*** create a StreamReader, and call the fRead method.
*****************************************************************************/
class GenericsExport StreamReader
{
	/// The root node of the XML document
	Node *m_pBaseNode;

	/// The node currently being processed during XML parsing
	Node *m_pCurrentNode;

	/// The SAX content handler interface needs to be able to call this 
	/// class's start/end/characters methods 
	friend class StreamReaderSaxHandler;

	/// The SAX wrapper will call this each time it encounters an <XML_Tag>
	void vStartElement (const wchar_t *pwchLocalName);

	/// The SAX wrapper will call this each time it encounters an </XML_Tag>
	void vEndElement (const wchar_t *pwchLocalName);

	/// The SAX wrapper will call this with the character data <XML_Tag>between the XML tags</XML_Tag>
	void vCharacters (const wchar_t *_pwchData, int iCharacters);

	/// This is set with the root node's SerializationVersion attribute value
	int m_iFileVersion;

public:
	/// The constructor
	StreamReader ();

	/// Destructor
	virtual ~StreamReader ();

	/// This method opens the file and reads the object, optionally with a string assignment helper function
	bool fRead (const char *szPath, Serializable *pObject, StringAssignFunc pfn = null);

	/// This is provided for the convenience of the consuming code
	int iGetFileVersion () { return m_iFileVersion; }
};


/****************************************************************************/
/** This class makes it easy to serialize lists of XmlStreamable objects.
*****************************************************************************/
template <class Class>
class SerializablePointerList : 
	public list<Class*>, 
	public Serializable
{
protected:
	/// This is the tag that the list will be written with
	const wchar_t *m_szPreferredTag;

	/// This is the tag that the list may have been written with in the past, it helps with backward compatibility
	const wchar_t *m_szObsoleteTag;
	
	/// The derived class is expected to override this function to create new objects.
	/// If the derived class implementation adds the new object to the list, *pfAddToList should be set to false
	/// Otherwise, it should be set to true - in which case this class will call push_front() with the new object
	virtual void vCreateNew (Class **, bool *pfAddToList) = 0;

	/// This is where the action is, it reads or writes the list from or to disk
	virtual void vSerialize (Stream &Stream)
	{
		// Writing is pretty trivial:
		if (Stream.eGetOperation () == Stream::oWrite)
		{
			list<Class*>::iterator iter;
			list<Class*>::iterator done = end ();

			// Interate the list...
			for (iter = begin (); iter != done; iter++)
			{
				Class *p = *iter;

				Stream.m_pOutput->vWrite (Stream, p, p, p->szGetPreferredTagName ());
			}
		}

		// Reading is only a bit more complex.
		else if (Stream.eGetOperation () == Stream::oRead)
		{
			// Iterate over the child nodes
			const Node::List &Children = Stream.m_pInput->GetChildren ();
			Node::List::const_iterator iter, iterLast = Children.end ();

			for (iter = Children.begin (); iter != iterLast; iter++)
			{
				Node *pNode = *iter;
                
				// Create a new object
				Class *pNew = null;
				bool fAddToList = true;
				vCreateNew (&pNew, &fAddToList);

				// (sanity check)
				if (!pNew)
					return;

				// Add the new object to the list (unless the creator deemed otherwise),
				if (fAddToList)
					push_front (pNew);

				// Initialize the new object with data from the stream
				Stream.m_pInput->vRead (Stream, pNode, pNew);
			}
		}
	}
public:

	/// Constructor takes preferred and optional obsolete tag names
	SerializablePointerList (const wchar_t *szPreferredTag, const wchar_t *szObsoleteTag = null) : 
		m_szPreferredTag (szPreferredTag), m_szObsoleteTag (szObsoleteTag) {}
};


/****************************************************************************/
/** SerializablePolymorphicPointerList
 ** 
 ** This is a list of pointers to objects derived from a common base class.
*****************************************************************************/
template <class BaseClass, typename CreatorParamType>
class SerializablePolymorphicPointerList : 
	public list<BaseClass*>, 
	public Serializable
{
protected:

	/// This tag will encapsuolate the whole list in the XML document
	const wchar_t *m_szContentTag;

	/// This object will be passed to the creator function when list elements are created from XML
	CreatorParamType m_CreatorParam;
	
	/// This typedef specifies the signature of the list item creator function.
	/// When implementing creator functions, remember to set *pfAddToList to true
	/// if you want the XmlStreamablePolymorphicPointerList to add the created object
	/// to the list after the creator function returns.  Or set it to false if the
	/// creator function will do the adding.
	typedef BaseClass* (*CreatorFunc) (CreatorParamType Param, bool *pfAddToList);

	/// This structure associates a class's tag with a creator function for objects of that class
	class SubclassType
	{
	public:

		const wchar_t *m_szTag;
		CreatorFunc m_fnCreator;

		SubclassType (const wchar_t *sz, CreatorFunc fn) : m_szTag (sz), m_fnCreator (fn) {}

	};

	/// This list contains a SubclassType object for each of the classes that the list can instantiate from XML
	list<SubclassType> m_lsSubclasses;

	/// This is where the action is... 
	virtual void vSerialize (Stream &Stream)
	{
		// Writing is almost trivial.
		if (Stream.eGetOperation () == Stream::oWrite)
		{
			list<BaseClass*>::iterator iter;
			list<BaseClass*>::iterator done = end ();

			// for each object in the list...
			for (iter = begin (); iter != done; iter++)
			{
				BaseClass *p = *iter;

				Stream.m_pOutput->vWrite (Stream, p, p, p->szGetPreferredTagName ());
			}
		}

		// Reading is a bit more complex.
		else if (Stream.eGetOperation () == Stream::oRead)
		{
			// Iterate over the list of nodes in the document
			const Node::List &Children = Stream.m_pInput->GetChildren ();
			Node::List::const_iterator iter, iterLast = Children.end ();
			for (iter = Children.begin (); iter != iterLast; iter++)
			{
				Node *pNode = *iter;
                
				// Iterate over the object types
				list<SubclassType>::iterator iter, iterLast = m_lsSubclasses.end ();
				for (iter = m_lsSubclasses.begin (); iter != iterLast; iter++)
				{
					SubclassType& Type = *iter;

					// Do the tags match?
					if (!wcscmp (pNode->szGetName (), Type.m_szTag))
					{
						// Create a new object, of the appropriate type
						bool fAddToList = true;
						BaseClass *pNew = (Type.m_fnCreator) (m_CreatorParam, &fAddToList);

						// (sanity check)
						if (!pNew)
							break;

						// The creator function might have taken care of this
						if (fAddToList)
							push_front (pNew);

						// Initialize the new object with data in the child node
						Stream.m_pInput->vRead (Stream, pNode, pNew);

						break;
					}
				}

				if (iter == iterLast)
				{
					// If this happens, an unrecognized tag has been found
					Breakpoint ("SerializablePolymorphicPointerList::vSerialize: Unrecognized tag.");
				}
			}
		}		
	}

public:
	/// The constructor takes the name of the tag that the list will be 
	/// enveloped in (in the XML) and the object that will be passed to
	/// list items before they are instantiated from the XML.  
	///
	/// Intended usage: define these parameters in the base-class-initializer 
	/// list of a class derived from this one.
	SerializablePolymorphicPointerList (const wchar_t *szContentTag, CreatorParamType CreatorParam) : 
		m_szContentTag (szContentTag), m_CreatorParam (CreatorParam) {}

	/// In the constructor of the class that contains the list, call this 
	/// function once for each of the classes that may be present in the list.
	void vAddClass (const wchar_t *szTag, CreatorFunc fnCreator)
	{
		SubclassType tmp (szTag, fnCreator);
		m_lsSubclasses.push_front (tmp);
	}
};

}; // end of XML namespace

}; // end of Generic namespace

#endif // ifdef __GenericXmlStreaming__

// GenericXmlStreaming.h ends here ->