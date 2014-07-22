
///////////////////////////////////////////////////////////////////////////////

ModalTaskPopup::ModalTaskPopup (ProgressiveTask* task, bool owned)
	:	TaskHandler (task, owned),
		progress (0.0)
{

}

ModalTaskPopup::~ModalTaskPopup ()
{

}

void ModalTaskPopup::taskStarted ()
{
	String cancelButtonText = "Cancel";
	bool hasCancelButton = true;
	Component* componentToCentreAround = nullptr;
	bool hasProgressBar = true;

	alertWindow = LookAndFeel::getDefaultLookAndFeel()
			.createAlertWindow (getTask().getName(), String(),
			cancelButtonText.isEmpty() ? TRANS("Cancel") : cancelButtonText,
			String(), String(),
			AlertWindow::NoIcon, hasCancelButton ? 1 : 0,
			componentToCentreAround);

	alertWindow->setEscapeKeyCancels (false);

	if (hasProgressBar)
		alertWindow->addProgressBarComponent (progress);

	alertWindow->enterModalState();
}

bool ModalTaskPopup::taskMonitor (bool stillRunning)
{
	if (stillRunning && alertWindow->isCurrentlyModal())
	{
		const ScopedLock sl (messageLock);
		alertWindow->setMessage (message);
		return true;
	}
	return false;
}

void ModalTaskPopup::taskFinished (bool aborted)
{
	alertWindow->exitModalState (aborted ? 1 : 0);
	alertWindow->setVisible (false);
}

void ModalTaskPopup::taskStatusMessageChanged (ProgressiveTask* task)
{
	const ScopedLock sl (messageLock);
	message = task->getStatusMessage();
}

void ModalTaskPopup::taskProgressChanged (ProgressiveTask* task)
{
	progress = task->getProgress();
}

void ModalTaskPopup::launch (ProgressiveTask* task, bool owned, const String& title, int priority, Callback* callback)
{
	ModalTaskPopup* handler = new ModalTaskPopup (task, owned);

	if (callback != nullptr)
	{
		handler->addCallback (callback);
	}

	TaskThread::launch (handler, title, priority);
}

///////////////////////////////////////////////////////////////////////////////

