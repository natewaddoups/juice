class Observer;
class Subject;

// EVENT
template<class T1, class T2>
class Event
{
	friend class Subject;
	friend class Observer;

private:
	struct Subscription
	{
		Subscription(Event<T1,T2> *e, Observer o, void (Observer::*p)(T1 t2, T2 t2))
		{
			event = e;
			observer = o;
			pfn = p;
		}

		Event<T1,T2> *event;
		Observer *observer;
		void(Observer::*pfn)(T1 t1, T2 t2);
	};

	typedef std::list<Subscription*> SubscriptionList;
	SubscriptionList Subscriptions;

	Subscription* AddSubscriber(Observer observer, void (Observer::*pfn)(T1 t2, T2 t2))
	{
		Subscription *subscription = new Subscription(observer, pfn);
		Subscriptions.push_back(subscription);
		return Subscription;
	}
protected:
	~Event()
	{
		for(SubscriptionList::iterator iter = Subscriptions.begin(); iter != Subscriptions.end(); iter++)
		{
			ObserverSubscription *subscription = *iter;
			subscription->observer->EventDeleted(this);
		}
	}

	void Send(T1 t1, T2 t2)
	{
		for(SubscriptionList::iterator iter = Subscriptions.begin(); iter != Subscriptions.end(); iter++)
		{
			Subscription *subscription = *iter;
			subscription->observer->*Subscription.pfn(t1, t2);
		}
	}
};


/// SUBJECT
class Subject
{
public:
	Event<int,char*> OnChange;

	void StuffHappens ()
	{
		OnChange.Send(1,"message");
	}
}

/// OBSERVER
template <class T1, class T2>
class Observer
{
	friend class Event;

private:
	Event::SubscriptionList subscriptions;

	void EventDeleted(Event* event)
	{
		for(SubscriptionList::iterator iter = subscriptions.begin(); iter != subscriptions.end(); iter++)
		{
			Event::Subscription *subscription = *iter;
			if (subscription->event == event)
			{
				subscriptions.remove_at(iter);
				break;
			}
		}
	}


protected:
	void Subscribe(Subject subject, Event<T1,T2> event)
	{
		Event::Subscription *subscription = event.AddSubscriber(this,Subscription);
		subscriptions.Add(SubjectChanged);
	}

	~Observer()
	{
		while(subscriptions.size())
		{
			Event::Subscription *subscription = subscribedEvents.front();
			subscription->event->RemoveSubscriber(subscription);
			delete subscription;
			subscriptions.pop_front();
		}
	}

	// in the derived class...
	void SubjectChanged(int i, char* msg)
	{
	}
}