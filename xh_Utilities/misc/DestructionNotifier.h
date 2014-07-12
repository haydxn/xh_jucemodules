#ifndef DESTRUCTIONNOTIFIER_H_INCLUDED
#define DESTRUCTIONNOTIFIER_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////

class DestructionNotifier;

///////////////////////////////////////////////////////////////////////////////
/**
	Listener for detecting when a DestructionNotifier is being destroyed.
*/
///////////////////////////////////////////////////////////////////////////////

class DestructionListener
{
public:
	virtual ~DestructionListener () {};
	/// Called when a registered DestructionNotifier is being destroyed.
	virtual void destructionNotifierCallback (DestructionNotifier* objectBeingDestroyed) = 0;
};

///////////////////////////////////////////////////////////////////////////////
/**
	Really basic object which automatically sends a notification to registered
	listeners when it is destroyed. There is a protected notifyDestruction()
	function which may be called by subclasses if it is required at an earlier
	time than when the base gets around to it - the notification is guaranteed
	to only be sent once.
*/
///////////////////////////////////////////////////////////////////////////////

class DestructionNotifier
{
public:

	DestructionNotifier ();
	~DestructionNotifier ();

	void addDestructionListener (DestructionListener* listener);
	void removeDestructionListener (DestructionListener* listener);

protected:

	void notifyDestruction ();
	
private:

	juce::ListenerList< DestructionListener > listeners;
	bool notificationSent;
};

///////////////////////////////////////////////////////////////////////////////

#endif  // DESTRUCTIONNOTIFIER_H_INCLUDED
