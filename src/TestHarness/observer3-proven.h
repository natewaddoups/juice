#include <list>

///////////////////////////////////////////////////////////////////////////////
template <class ListenerInterface> class Listener;
template <class ListenerInterface> class Notifier;

template<class ListenerInterface>
class Dispatcher
{
	typedef Listener<ListenerInterface> Observer;
	typedef Notifier<ListenerInterface> Subject;

	friend class Observer;
	friend class Subject;

private:
	struct Subscription
	{
		Subscription(Subject *s, Observer *o, ListenerInterface *l)
		{
			subject = s;
			observer = o;
			listener = l;
		}

		Subject *subject;
		Observer *observer;
		ListenerInterface *listener;
	};

	typedef std::list<Subscription*> SubscriptionList;
	SubscriptionList subscriptions;
	
	Subscription* Subscribe(Subject *subject, Observer *observer, ListenerInterface *listener)
	{
		Subscription *subscription = new Subscription(subject, observer, listener);
		subscriptions.push_back(subscription);
		return subscription;
	}

	void Unsubscribe(Subscription *subscription)
	{
		subscriptions.remove(subscription);
		delete subscription;
	}

public:
/*
	template<typename Method>
	void Publish(Method pf)
	{
		for(SubscriptionList::iterator iter = subscriptions.begin(); iter != subscriptions.end(); iter++)
		{
			Subscription *subscription = *iter;
			ListenerInterface *listener = subscription->listener;
			(listener->*pfn)();
		}
	}

	template<typename Method, typename T1>
	void Publish(Method pfn, T1 t1)
	{
		for(SubscriptionList::iterator iter = subscriptions.begin(); iter != subscriptions.end(); iter++)
		{
			Subscription *subscription = *iter;
			ListenerInterface *listener = subscription->listener;
			(listener->*pfn)(t1);
		}
	}*/

	template<typename Method, typename T1, typename T2>
	void Publish(Method pfn, T1 t1, T2 t2)
	{
		for(SubscriptionList::iterator iter = subscriptions.begin(); iter != subscriptions.end(); iter++)
		{
			Subscription *subscription = *iter;
			ListenerInterface::EventSource* notifier = (ListenerInterface::EventSource*) subscription->subject;
			ListenerInterface *listener = subscription->listener;
			(listener->*pfn)(notifier, t1, t2);
		}
	}

	~Dispatcher()
	{
		while(subscriptions.size())
		{
			Dispatcher::Subscription *subscription = subscriptions.front();
			subscription->observer->TerminateSubscription(subscription);
			subscription->listener->Notifier_Deleted((ListenerInterface::EventSource*) subscription->subject);
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
	Dispatcher<ListenerInterface> m_Dispatcher;
};


///////////////////////////////////////////////////////////////////////////////
template <class ListenerInterface>
class Listener
{
	friend class Dispatcher<ListenerInterface>;

	void TerminateSubscription (typename Dispatcher<ListenerInterface>::Subscription *subscription)
	{
		subscriptions.remove(subscription);
	}

protected:

	typename Dispatcher<ListenerInterface>::SubscriptionList subscriptions;

	void Subscribe (Notifier<ListenerInterface> *notifier)
	{
		ListenerInterface *iface = dynamic_cast<ListenerInterface*> (this);
		Dispatcher<ListenerInterface>::Subscription *subscription = notifier->m_Dispatcher.Subscribe(notifier, this, iface);
		subscriptions.push_back(subscription);
	}

	/*void EndSubscription(Notifier<ListenerInterface> *notifier)
	{
		for(SubscriptionList::iterator iter = subscriptions.begin(); iter != subscriptions.end(); iter++)
		{
			Dispatcher<ListenerInterface>::Subscription *subscription = *iter;
			if (subscription->notifier == notifier)
			{
				subscriptions.remove_at(iter);
				break;
			}
		}
	}*/

public:
	~Listener()
	{
		for(Dispatcher<ListenerInterface>::SubscriptionList::iterator iter = subscriptions.begin(); iter != subscriptions.end(); iter++)
		{
			Dispatcher<ListenerInterface>::Subscription *subscription = *iter;
			subscription->subject->m_Dispatcher.Unsubscribe(subscription);
		}
	}

	typedef typename ListenerInterface::EventSource EventSource;

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


///////////////////////////////////////////////////////////////////////////////
#define ListensTo(x) public Listener<x>, public x

///////////////////////////////////////////////////////////////////////////////

