#ifndef OVERRIDABLESHAREDRESOURCEPOINTER_H_INCLUDED
#define OVERRIDABLESHAREDRESOURCEPOINTER_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////
// This is just a base so that there is something in common between all
// types of OverridableSharedResourcePtr, making it possible to hold them
// in a list.
///////////////////////////////////////////////////////////////////////////////

class OverridableResourcePtrBase
{
public:
	virtual ~OverridableResourcePtrBase ()
	{
	}
};

///////////////////////////////////////////////////////////////////////////////

class DefaultSharedResourceVariant {};

///////////////////////////////////////////////////////////////////////////////
/**
	A pointer to an automatically-created shared instance of the template type
	(see SharedResourcePointer).

	It is possible to override the local instance referenced by a specific 
	OverridableSharedResourcePtr object (if this is null, the default shared 
	instance is used). It is also possible to globally replace the default
	instance is used by all objects.

	The 'Variant' template parameter makes it possible to have multiple
	default instances of a specific class for different uses; all instances of
	any given pointer variant will refer to the same default instance.
*/
///////////////////////////////////////////////////////////////////////////////

template <typename SharedObjectType, typename Variant = DefaultSharedResourceVariant>
class OverridableSharedResourcePtr	:	public OverridableResourcePtrBase
{
public:

    /** Creates an instance of the shared object, which can be overridden.
        If other OverridableSharedResourcePtr objects for this type already 
		exist, then this one will simply point to the same shared object that 
		they are already using. Otherwise, if this is the first pointer object 
		to be created, then a shared object will be created automatically.
    */
    OverridableSharedResourcePtr()
    {
        SharedObjectHolder& holder = getSharedObjectHolder();

        if (++(holder.refCount) == 1)
		{
			holder.resetInstance();
		}
    }

    /** Destructor.
        If no other OverridableSharedResourcePtr objects exist, this will also
		delete the shared object to which it refers.
    */
    ~OverridableSharedResourcePtr()
    {
        SharedObjectHolder& holder = getSharedObjectHolder();

		if (--(holder.refCount) == 0)
		{
            holder.clear ();
		}
    }

	/** Returns the current default shared instance. */
	SharedObjectType* getSharedInstance () const
	{ 
		return getSharedObjectHolder().get(); 
	}

	/** Replace the current default shared instance. */
	void setSharedInstance (SharedObjectType* object)
	{
		if (object == nullptr)
		{
			resetSharedInstance();
		}
		else
		{
			getSharedObjectHolder().set (object);
		}
	}

	/** Recreates the default shared instance. */
	void resetSharedInstance ()
	{
		getSharedObjectHolder().resetInstance();
	}

	/** Overrides the local instance referred to by this pointer. */
	void setLocalInstance (SharedObjectType* object, bool takeOwnership)
	{
		localOverride.set (object, takeOwnership);
	}

    /** Returns the current instance. */
    operator SharedObjectType*() const noexcept         { return &get(); }

    /** Returns the instance this pointer refers to (if no local override has
		been set, this will return the default shared instance. */
    SharedObjectType& get() const
	{
		if (localOverride.get() != nullptr)
		{
			return *localOverride.get();
		}
		return *getSharedInstance();
	}

    SharedObjectType* operator->() const noexcept       { return &get(); }

private:

    struct SharedObjectHolder
    {
        juce::SpinLock lock;
        juce::ScopedPointer<SharedObjectType> sharedInstance;
        int refCount;

		void resetInstance ()
		{
			const juce::SpinLock::ScopedLockType sl (lock);
			sharedInstance = new SharedObjectType ();
		}

		void set (SharedObjectType* newInstance)
		{
			const juce::SpinLock::ScopedLockType sl (lock);
			if (newInstance != sharedInstance)
			{
				jassert (newInstance != nullptr); // Must always have a valid default instance!
				sharedInstance = newInstance;
			}
		}

		void clear ()
		{
			const juce::SpinLock::ScopedLockType sl (lock);
			sharedInstance = nullptr;
		}

		SharedObjectType* get () const
		{
			const juce::SpinLock::ScopedLockType sl (lock);
			return sharedInstance;
		}
    };

    static SharedObjectHolder& getSharedObjectHolder() noexcept
    {
		RETURN_FUNCTION_STATIC_STRUCT(SharedObjectHolder);
    }

	juce::OptionalScopedPointer< SharedObjectType > localOverride;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OverridableSharedResourcePtr)
};


///////////////////////////////////////////////////////////////////////////////
/**
	List class for setting overrides for specific OverridableSharedResourcePtr
	types. This provides a place to keep instances of the pointers alive, thus
	preserving the override instances they are made to refer to.
*/
///////////////////////////////////////////////////////////////////////////////

class SharedResourceOverrideList
{
	class OverrideHolder;
public:
	
	SharedResourceOverrideList () {}

	~SharedResourceOverrideList ()
	{
		overrides.deleteAll();
	}

	template <typename SharedObjectType>
	void set (SharedObjectType* object)
	{
		set< SharedObjectType, DefaultSharedResourceVariant > (object);
	}

	template <typename SharedObjectType>
	SharedObjectType* get ()
	{
		return get< SharedObjectType, DefaultSharedResourceVariant >();
	}

	template <typename SharedObjectType, typename Variant>
	void set (SharedObjectType* object)
	{
		OverrideHolder* holder = getOrCreateOverrideHolder< SharedObjectType, Variant > (false);
		holder->createOverride< SharedObjectType, Variant >(object);
	}

	template <typename SharedObjectType, typename Variant>
	void set (const OverridableSharedResourcePtr< SharedObjectType, Variant >& ptr)
	{
		set< SharedObjectType, Variant >(ptr.getSharedInstance());
	}

	template <typename SharedObjectType, typename Variant>
	SharedObjectType* get ()
	{
		OverrideHolder* holder = getOrCreateOverrideHolder< SharedObjectType, Variant > (true);
		return holder->getOverride< SharedObjectType, Variant >();
	}

private:

	template <typename SharedObjectType, typename Variant>
	OverrideHolder* findOverrideHolder ()
	{
		OverrideHolder* holder = overrides.get();
		while (holder != nullptr)
		{
			if (holder->isForType< SharedObjectType, Variant >())
				return holder;

			holder = holder->nextListItem.get();
		}
		return nullptr;
	}

	template <typename SharedObjectType, typename Variant>
	OverrideHolder* getOrCreateOverrideHolder (bool initialise)
	{
		OverrideHolder* holder = findOverrideHolder< SharedObjectType, Variant > ();
		if (holder == nullptr)
		{
			holder = new OverrideHolder;
			overrides.append (holder);

			if (initialise)
			{
				holder->createOverride< SharedObjectType, Variant > (nullptr);
			}
		}
		return holder;
	}

	class OverrideHolder
	{
	public:

		OverrideHolder () {}

		template <typename SharedObjectType, typename Variant>
		SharedObjectType* createOverride (SharedObjectType* object)
		{
			typedef OverridableSharedResourcePtr< SharedObjectType, Variant > PtrType;

			PtrType* ptr = new PtrType;
			base = ptr;

			if (object != nullptr)
			{
				ptr->setSharedInstance (object);
			}
			return ptr->getSharedInstance ();
		}

		template <typename SharedObjectType, typename Variant>
		SharedObjectType* getOverride ()
		{
			typedef OverridableSharedResourcePtr< SharedObjectType, Variant > PtrType;
			PtrType* ptr = dynamic_cast<PtrType*> (base.get());
			jassert (ptr != nullptr); // should only be getting called if we know it's the right type
			return  ptr->getSharedInstance();
		}

		template <typename SharedObjectType, typename Variant>
		bool isForType ()
		{
			typedef OverridableSharedResourcePtr< SharedObjectType, Variant > PtrType;
			return dynamic_cast<PtrType*> (base.get()) != nullptr;
		}

	private:

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OverrideHolder);

		friend class SharedResourceOverrideList;
		friend class juce::LinkedListPointer< OverrideHolder >;
		juce::LinkedListPointer< OverrideHolder > nextListItem;
		juce::ScopedPointer< OverridableResourcePtrBase > base;

	};

	juce::LinkedListPointer< OverrideHolder > overrides;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SharedResourceOverrideList);
};

///////////////////////////////////////////////////////////////////////////////

#endif  // OVERRIDABLESHAREDRESOURCEPOINTER_H_INCLUDED
