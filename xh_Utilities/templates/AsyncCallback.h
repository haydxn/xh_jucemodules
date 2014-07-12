#ifndef ASYNCCALLBACK_H_INCLUDED
#define ASYNCCALLBACK_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////
/**
	Handy object to wrap a member function with an AsyncUpdater, providing an 
	easy way to implement multiple asynchronous callbacks.

	e.g. 

	class MyClass
	{
		AsyncCallback< MyClass > notifier;

	public:
		
		MyClass () : notifier (*this, &MyClass::notified)
		{
			notifier.trigger ();
		}

		void notified ()
		{
			// called asynchronously after constructor
		}

	};
*/
///////////////////////////////////////////////////////////////////////////////

template <class OwnerClass>
class AsyncCallback :   private juce::AsyncUpdater
{
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AsyncCallback);
public:
    
    typedef void (OwnerClass::*CallbackFunction) ();

    AsyncCallback (OwnerClass& ownerInstance, CallbackFunction functionToCall)
    :   owner (ownerInstance),
        function (functionToCall)
    {
    }
    
    ~AsyncCallback ()
    {
		cancel ();
        function = nullptr; // Mark as invalid to avoid further triggers during destruction
    }
    
    void cancel ()
    {
        cancelPendingUpdate ();
    }
    
    void trigger ()
    {
        if (function != nullptr)
        {
            triggerAsyncUpdate ();
        }
    }
    
    void triggerSynchronously ()
    {
        if (function != nullptr)
        {
            cancel ();
            handleAsyncUpdate ();
        }
    }
    
    void handleAsyncUpdate () override
    {
		if (function != nullptr)
		{
	        (owner.*(function)) ();
		}
    }
    
private:
    
	OwnerClass& owner;
	CallbackFunction function;

};

///////////////////////////////////////////////////////////////////////////////


#endif  // ASYNCCALLBACK_H_INCLUDED
