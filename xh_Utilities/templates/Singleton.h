#ifndef SINGLETON_H_INCLUDED
#define SINGLETON_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////

#define FUNCTION_STATIC_STRUCT_PTR(StructType,varName) \
	static void* varName [(sizeof (StructType) + sizeof(void*) - 1) / sizeof(void*)] = { 0 };

#define RETURN_FUNCTION_STATIC_STRUCT(StructType) \
	FUNCTION_STATIC_STRUCT_PTR(StructType, instance); \
	return *reinterpret_cast<StructType*> (instance);

///////////////////////////////////////////////////////////////////////////////
/**
*/
///////////////////////////////////////////////////////////////////////////////


template <class BaseClass>
class Singleton
{
public:
	
	Singleton ()
	{

	}

	~Singleton ()
	{
		clearSingletonInstance ();
	} 

	static BaseClass* getInstance ()
	{
		return getInstanceHolder().getInstance ();
	}

	static void destroyInstance ()
	{
		getInstanceHolder().clear (true);
	}

protected:

	void clearSingletonInstance ()
	{
		getInstanceHolder().clear (false);
	}

private:

	struct InstanceHolder
	{
		juce::SpinLock lock;
		juce::ScopedPointer< BaseClass > instance;
		bool created;

		void clear (bool destroy = true)
		{
			const juce::SpinLock::ScopedLockType sl (lock);

			if (!destroy)
			{
				instance.release ();
			}
			else
			{
				instance = nullptr;
			}
		}

		BaseClass* getInstance ()
		{
			const juce::SpinLock::ScopedLockType sl (lock);
			if (instance == nullptr)
			{
				instance = new BaseClass();
				created = true;
			}
			return instance;
		}
	};

 	static InstanceHolder& getInstanceHolder() noexcept
 	{
		RETURN_FUNCTION_STATIC_STRUCT(InstanceHolder);
 	}

};

///////////////////////////////////////////////////////////////////////////////

#endif//SINGLETON_H_INCLUDED
