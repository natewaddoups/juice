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
	/*
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
	*/
///////////////////////////////////////////////////////////////////////////////
template <class ListenerInterface> class Listener;
template <class ListenerInterface> class Notifier;

template<class ListenerInterface>
class Dispatcher
{
	typedef Listener<ListenerInterface> ListenerType;
	typedef Notifier<ListenerInterface> NotifierType;

	friend class ListenerType;
	friend class NotifierType;

private:
	struct Subscription
	{
		Subscription(NotifierType *n, ListenerType *l, ListenerInterface *li)
		{
			notifier = n;
			listener = l;
			listenerInterface = li;
		}

		NotifierType *notifier;
		ListenerType *listener;
		ListenerInterface *listenerInterface;
	};

	typedef std::list<Subscription*
		//, NotifyListenAllocator<Subscription*>
		> SubscriptionList;
	SubscriptionList subscriptions;
	
	Subscription* Subscribe(NotifierType *notifier, ListenerType *listener, ListenerInterface *listenerInterface)
	{
		Subscription *subscription = new Subscription(notifier, listener, listenerInterface);
		subscriptions.push_back(subscription);
		return subscription;
	}

	void Unsubscribe(Subscription *subscription)
	{
		subscriptions.remove(subscription);
		delete subscription;
	}

public:
	template<typename Method>
	void operator() (Method pfn)
	{
		for(SubscriptionList::iterator iter = subscriptions.begin(); iter != subscriptions.end(); iter++)
		{
			Subscription *subscription = *iter;
			ListenerInterface::EventSource* notifier = (ListenerInterface::EventSource*) subscription->notifier;
			ListenerInterface *listenerInterface = subscription->listenerInterface;
			(listenerInterface->*pfn)(notifier);
		}
	}

	template<typename Method, typename T1>
	void operator() (Method pfn, T1 t1)
	{
		for(SubscriptionList::iterator iter = subscriptions.begin(); iter != subscriptions.end(); iter++)
		{
			Subscription *subscription = *iter;
			ListenerInterface::EventSource* notifier = (ListenerInterface::EventSource*) subscription->notifier;
			ListenerInterface *listenerInterface = subscription->listenerInterface;
			(listenerInterface->*pfn)(notifier, t1);
		}
	}

	template<typename Method, typename T1, typename T2>
	void operator() (Method pfn, T1 t1, T2 t2)
	{
		for(SubscriptionList::iterator iter = subscriptions.begin(); iter != subscriptions.end(); iter++)
		{
			Subscription *subscription = *iter;
			ListenerInterface::EventSource* notifier = (ListenerInterface::EventSource*) subscription->notifier;
			ListenerInterface *listenerInterface = subscription->listenerInterface;
			(listenerInterface->*pfn)(notifier, t1, t2);
		}
	}

	~Dispatcher()
	{
		while(subscriptions.size())
		{
			Dispatcher::Subscription *subscription = subscriptions.front();
			subscription->listener->vTerminateSubscription(subscription);
			subscription->listenerInterface->Notifier_Deleted((ListenerInterface::EventSource*) subscription->notifier);
			delete subscription;
			subscriptions.pop_front();
		}
	}

};

///////////////////////////////////////////////////////////////////////////////
template<class ListenerInterface>
class Notifier
{
public:
	Dispatcher<ListenerInterface> vNotify;
};


///////////////////////////////////////////////////////////////////////////////
template <class ListenerInterface>
class Listener : public ListenerInterface
{
	friend class Dispatcher<ListenerInterface>;

	void vTerminateSubscription (typename Dispatcher<ListenerInterface>::Subscription *subscription)
	{
		subscriptions.remove(subscription);
	}

protected:

	typename Dispatcher<ListenerInterface>::SubscriptionList subscriptions;
	typedef typename ListenerInterface::EventSource EventSource;

public:

	~Listener()
	{
		for(Dispatcher<ListenerInterface>::SubscriptionList::iterator iter = subscriptions.begin(); iter != subscriptions.end(); iter++)
		{
			Dispatcher<ListenerInterface>::Subscription *subscription = *iter;
			subscription->notifier->vNotify.Unsubscribe(subscription);
		}
	}

	void vRegisterForEventsFrom (Notifier<ListenerInterface> *notifier)
	{
		ListenerInterface *iface = dynamic_cast<ListenerInterface*> (this);
		Dispatcher<ListenerInterface>::Subscription *subscription = notifier->vNotify.Subscribe(notifier, this, iface);
		subscriptions.push_back(subscription);
	}

	void vUnregister (Notifier<ListenerInterface> *notifier)
	{
		for(Dispatcher<ListenerInterface>::SubscriptionList::iterator iter = subscriptions.begin(); iter != subscriptions.end(); iter++)
		{
			Dispatcher<ListenerInterface>::Subscription *subscription = *iter;
			if (subscription->notifier == notifier)
			{
				subscriptions.remove(subscription);
				notifier->vNotify.Unsubscribe(subscription);
				break;
			}
		}
	}

	virtual void Notifier_Deleted(EventSource *notifier)
	{
	}
};

///////////////////////////////////////////////////////////////////////////////
template <class Source>
class Events
{
public:
	typedef Source EventSource;
	virtual void Notifier_Deleted (Source *pSource) {}
};

}
#endif
// GenericNotifyListen.h ends here ->