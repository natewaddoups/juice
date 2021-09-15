#include <list>

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

	typedef std::list<Subscription*> SubscriptionList;
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
	void operator() (Method pf)
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
			subscription->listener->TerminateSubscription(subscription);
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

	void TerminateSubscription (typename Dispatcher<ListenerInterface>::Subscription *subscription)
	{
		subscriptions.remove(subscription);
	}

protected:

	typename Dispatcher<ListenerInterface>::SubscriptionList subscriptions;

	void Subscribe (Notifier<ListenerInterface> *notifier)
	{
		ListenerInterface *iface = dynamic_cast<ListenerInterface*> (this);
		Dispatcher<ListenerInterface>::Subscription *subscription = notifier->vNotify.Subscribe(notifier, this, iface);
		subscriptions.push_back(subscription);
	}

	void Unsubscribe (Notifier<ListenerInterface> *notifier)
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

public:
	~Listener()
	{
		for(Dispatcher<ListenerInterface>::SubscriptionList::iterator iter = subscriptions.begin(); iter != subscriptions.end(); iter++)
		{
			Dispatcher<ListenerInterface>::Subscription *subscription = *iter;
			subscription->notifier->vNotify.Unsubscribe(subscription);
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

