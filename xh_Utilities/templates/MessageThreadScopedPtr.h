#ifndef MESSAGETHREADSCOPEDPTR_H_INCLUDED
#define MESSAGETHREADSCOPEDPTR_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////
/**
*/
///////////////////////////////////////////////////////////////////////////////

template <class ObjectType>
class AsyncDestroyer	:	public Singleton< AsyncDestroyer< ObjectType > >,
							public juce::DeletedAtShutdown,
							private juce::AsyncUpdater
{
public:
    
    typedef Singleton< AsyncDestroyer< ObjectType > > Singleton;
	
	AsyncDestroyer ()
	{
		jassert (juce::MessageManager::getInstanceWithoutCreating() != nullptr);
	}

	~AsyncDestroyer ()
	{
		jassert (juce::MessageManager::getInstance()->isThisTheMessageThread());

		flushQueue ();
		this->clearSingletonInstance();
	}

	void flushQueue ()
	{
		jassert (juce::MessageManager::getInstanceWithoutCreating() != nullptr);

		if (juce::MessageManager::getInstance()->isThisTheMessageThread())
		{
			objectsToDelete.clearQuick (true);
		}
	}

	void queueObject (ObjectType* object)
	{
		jassert (!objectsToDelete.contains(object));

		objectsToDelete.add (object);
		triggerAsyncUpdate ();
	}

	static void deleteAsynchronously (ObjectType* object)
	{
		if (object != nullptr)
		{
			AsyncDestroyer< ObjectType >* instance = Singleton::getInstance ();
			if (instance != nullptr)
			{
				instance->queueObject (object);
			}
			else
			{
				jassertfalse; // The queue no longer exists!
			}
		}
	}

	static void deleteOnMessageThread (ObjectType* object)
	{
		if (juce::MessageManager::getInstance()->isThisTheMessageThread())
		{
			delete object;
		}
		else
		{
			deleteAsynchronously (object);

		}
	}

	static void flush ()
	{
		AsyncDestroyer< ObjectType >* instance = Singleton::getInstance ();
		if (instance != nullptr)
		{
			instance->flushQueue();
		}
	}

private:

	virtual void handleAsyncUpdate () override
	{
		flushQueue ();
	}

	juce::OwnedArray< ObjectType, juce::CriticalSection > objectsToDelete;

};

///////////////////////////////////////////////////////////////////////////////

#endif//MESSAGETHREADSCOPEDPTR_H_INCLUDED
