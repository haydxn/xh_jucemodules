
///////////////////////////////////////////////////////////////////////////////

ModalTaskPopup::ModalTaskPopup (ProgressiveTask* task)
	:	TaskHandler (task),
		progress (0.0)
{

}

ModalTaskPopup::~ModalTaskPopup ()
{
	flushState();
}

void ModalTaskPopup::taskStart ()
{
	String cancelButtonText = "Cancel";
	bool hasCancelButton = true;
	Component* componentToCentreAround = nullptr;
	bool hasProgressBar = true;

	progress = 0.0;

	alertWindow = LookAndFeel::getDefaultLookAndFeel()
			.createAlertWindow (getTask().getName(), String(),
			cancelButtonText.isEmpty() ? TRANS("Cancel") : cancelButtonText,
			String(), String(),
			AlertWindow::NoIcon, hasCancelButton ? 1 : 0,
			componentToCentreAround);

	alertWindow->setEscapeKeyCancels (false);

	if (hasProgressBar)
		alertWindow->addProgressBarComponent (getOverallProgressVariable());

	alertWindow->enterModalState();

	startTimer (10);
}

bool ModalTaskPopup::taskMonitor (bool stillRunning)
{
	if (stillRunning && alertWindow->isCurrentlyModal())
	{
		alertWindow->setMessage (getStatusMessage());
		return true;
	}
	return false;
}

void ModalTaskPopup::taskFinish (bool aborted)
{
	alertWindow->exitModalState (aborted ? 1 : 0);
	alertWindow->setVisible (false);
	alertWindow = nullptr;

	stopTimer();
}

void ModalTaskPopup::launch (ProgressiveTask* task, const String& title, int priority, Callback* callback)
{
	ModalTaskPopup* handler = new ModalTaskPopup (task);

	if (callback != nullptr)
	{
		handler->addCallback (callback);
	}

	TaskThread::launch (handler, title, priority);
}

void ModalTaskPopup::timerCallback ()
{
	if (!taskMonitor (getState() == TaskHandler::taskRunning))
	{
		getTask().abort();
	}
}


///////////////////////////////////////////////////////////////////////////////

