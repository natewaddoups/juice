/*****************************************************************************

Project:  Juice
Module:   Generics.dll
Filename: GenericNotifyListen.h 

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

Generics.dll and its source code are free software.  Except as noted, you 
may redistribute this code under the terms of the Gnu General Public License,
version 2 (GPL2).   This license shall be terminated if any part of GPL2 
should be overturned in court (any jurisdiction).  In that case, contact the 
author for updated license terms.

*****************************************************************************/
#ifndef __GenericNotifyListen__
#define __GenericNotifyListen__

#include <vector>
#include <list>

namespace Generic
{

/****************************************************************************/
/** This allocator ensures that the nodes in the Notifier and Listener
*** lists are always allocated and deallocated from the heap of Generics.dll.
***
*** http://www.cuj.com/experts/1812/austern.htm provided invaluable help
*****************************************************************************/

	void GenericsExport *NotifyListen_New (size_t cb);
	void GenericsExport NotifyListen_Delete (void *p);

	template <class T> 
	class NotifyListenAllocator
	{
	public:
		typedef T value_type;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T& reference;
		typedef const T& const_reference;

		NotifyListenAllocator () {}
		~NotifyListenAllocator () {}
		
		template <class U> 
		NotifyListenAllocator (const NotifyListenAllocator<U>&) {}

		template <class U> 
		struct rebind { typedef NotifyListenAllocator<U> other; };

		void construct(pointer p, const_reference val)
		{
			new (p) T(val);
		}

		void destroy(pointer p)
		{
			p->~T();
		}

		pointer allocate(size_type sz, void* vp = 0)
		{
			pointer p = static_cast<pointer> (NotifyListen_New(sz * sizeof(value_type)));
			return p;
		}

		void deallocate(pointer p, size_type)
		{
			NotifyListen_Delete(p);
		}

		size_type max_size() const 
		{ 
			return static_cast<size_type> (-1) / sizeof (value_type);
		}
	};

/****************************************************************************/
/** Forward declaration of the Listener class
*****************************************************************************/

    template <class Events> class Listener;

/****************************************************************************/
/** The notifier class may be mixed into a class to add notification 
 ** functionality.  
 **
 ** This implementation of the Observer pattern was heavily inspired by the
 ** work of Pete Goodliffe in his TSE3 MIDI sequencer project.  For examples
 ** of better coding than you'll find anywhere in the Juice project, have a 
 ** good look at TSE3: http://tse3.sourceforge.net/
*****************************************************************************/

    template <class Events>
    class Notifier
    {
	protected:
		/// The "peer" class is the class that receives the events sent by this class
		typedef Listener<Events> Peer;

		/// This provides a shorthand way to refer to the class that sends the 
		/// notifications - the class that subclassed this one
		typedef typename Events::EventSource EventSource; 

		/// A shorthand way to refer to a list of peers
		typedef std::list<Peer*, NotifyListenAllocator<Peer*> > ListenerList;

		/// ...and just such a list
		ListenerList m_Listeners;

		/// Allow the peer to see this class's guts (specifically the vNotify methods)
		friend class Peer;

		/// Constructor is protected as a reminder that you must derive from this class in order to use it
		Notifier () {}

	private:

		/// This is not implemented - it's declared private to disable copy construction 
        Notifier (const Notifier<Events> &);

		/// This is not implemented - it's declared private to disable assignment operator 
        Notifier<Events> &operator= (const Notifier<Events> &);

    public:

		/// The destructor notifies all registered peers that this object is being deleted
		virtual ~Notifier()
        {
			// Note that this loop must be done without an iterator, because
			// vPeerDeleted might lead to a call to vDetach (see below), which
			// can make the iterator go stale.
			Peer *pPeer = 0;
			while (m_Listeners.size ())
			{
				pPeer = m_Listeners.front ();
				pPeer->vPeerDeleted (static_cast<EventSource*> (this));
				m_Listeners.pop_front ();
			}
        }
		
    protected:

		/// Broadcast an event with no parameters
		template <typename NotificationMethod>
		void vNotify (NotificationMethod pfnMethod)
		{
			ListenerList::iterator last = m_Listeners.end ();
			for (ListenerList::iterator iter = m_Listeners.begin (); iter != last; iter++)
				(*iter->*pfnMethod) (reinterpret_cast<EventSource*> (this));
		}

		/// Broadcast an event with one parameter
		template <typename NotificationMethod, typename ParamType>
		void vNotify (NotificationMethod pfnMethod, const ParamType &tParam)
		{
			ListenerList::iterator last = m_Listeners.end ();
			for (ListenerList::iterator iter = m_Listeners.begin (); iter != last; iter++)
				(*iter->*pfnMethod) (reinterpret_cast<EventSource*> (this), tParam);
		}

		/// Broadcast an event with two parameters
		template <typename NotificationMethod, typename Param1Type, typename Param2Type>
		void vNotify (NotificationMethod pfnMethod, const Param1Type &tParam1, const Param2Type &tParam2)
		{
			ListenerList::iterator last = m_Listeners.end ();
			for (ListenerList::iterator iter = m_Listeners.begin (); iter != last; iter++)
				(*iter->*pfnMethod) (reinterpret_cast<EventSource*> (this), tParam1, tParam2);
		}


	private:

		/// This must not attach the same listener twice.  You'd get two 
		/// notifications of each event, which would be bad.
        bool fAttach (Peer *pListener)
        {
			ListenerList::iterator last = m_Listeners.end ();

			for (ListenerList::iterator iter = m_Listeners.begin (); iter != m_Listeners.end(); iter++)
				if (*iter == pListener)
					return false;

			m_Listeners.push_back (pListener);
			return true;
        }

		/// Detach a listening peer
        void vDetach (Peer *pListener)
        {
			m_Listeners.remove (pListener);
        }
	};

/****************************************************************************/
/** The Listener class is a mixin, similar to Notifer, which provides 
 ** Observer functionality to the class that derives from it.
*****************************************************************************/

    template <class Events>
    class Listener : public Events
    {
    protected:
		/// This is basically shorthand so you don't have to write "Notifier<Events>" all over the place below.
        typedef Notifier<Events> Peer;

		/// Allow the peer objects to see the guts of this object
        friend class Peer;

		/// More shorthand... this identifies the type of object that sends the events
        typedef typename Events::EventSource EventSource;

		/// More shorthand... this defines a list of peers
		typedef std::list<Peer*, NotifyListenAllocator<Peer*> > NotifierList;

		/// ...and this declares such a list
		NotifierList m_Notifiers;

		/// The constructor is protected as a reminder that this must be subclassed 
        Listener() {}

	private:

		/// This is not implemented, it's only declared private so as to disable copy construction
        Listener (const Listener<Events>&);

		/// This is not implemented, it's only declared private so as to disable assignment
        Listener<Events> &operator= (const Listener<Events>&);

    public:

		/// The destructor detaches the listener from its notifiers
        virtual ~Listener()
        {
			NotifierList::iterator last = m_Notifiers.end ();
			for (NotifierList::iterator iter = m_Notifiers.begin (); iter != last; iter++)
                (*iter)->vDetach (this);
        }
	
		/// This registers the listener with the notifier, so that 
		/// future events from the given notifier will be received
        void vRegisterForEventsFrom (Peer *pNotifier)
        {
            if (pNotifier->fAttach (this)) 
				m_Notifiers.push_back (pNotifier);
        }

		/// Unregister the listener; future events will not be received
        void vUnregister (Peer *pNotifier)
        {
            m_Notifiers.remove (pNotifier);
			pNotifier->vDetach (this);
        }

    private:

		/// This is called by the peer's destructor
        void vPeerDeleted(EventSource *src)
        {
			m_Notifiers.remove (src);
            this->Notifier_Deleted(src);
        }

	public:

		/// This is available for child classes to override when the notifier is deleted
        virtual void Notifier_Deleted(EventSource *pNotifier) {}

    };


}; // end of Generic namespace

#endif

// GenericNotifyListen.h ends here ->