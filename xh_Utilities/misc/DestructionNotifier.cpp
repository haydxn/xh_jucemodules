///////////////////////////////////////////////////////////////////////////////

DestructionNotifier::DestructionNotifier ()
	:	notificationSent (false)
{
}

DestructionNotifier::~DestructionNotifier ()
{
	notifyDestruction ();
}

void DestructionNotifier::addDestructionListener (DestructionListener* listener)
{
	listeners.add (listener);
}

void DestructionNotifier::removeDestructionListener (DestructionListener* listener)
{
	listeners.remove (listener);
}

void DestructionNotifier::notifyDestruction ()
{
	if (!notificationSent)
	{
		listeners.call (&DestructionListener::destructionNotifierCallback, this);
		notificationSent = true;
	}
}

///////////////////////////////////////////////////////////////////////////////
