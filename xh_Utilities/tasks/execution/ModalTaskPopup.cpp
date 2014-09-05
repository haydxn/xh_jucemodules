
///////////////////////////////////////////////////////////////////////////////

ModalTaskPopup::ModalTaskPopup (ProgressiveTask* task)
	:	progress (0.0)
{
	setTaskContext (new TaskContext (task));
}

ModalTaskPopup::ModalTaskPopup (TaskContext* task)
	:	progress (0.0)
{
	setTaskContext (task);
}

ModalTaskPopup::~ModalTaskPopup ()
{
}

void ModalTaskPopup::refresh (TaskContext& context)
{
	if (alertWindow == nullptr && context.getState() != TaskContext::taskPending)
	{
		taskStart ();
	}
}

void ModalTaskPopup::aboutToDispatchTaskFinishedCallbacks (TaskContext& context)
{
	if (alertWindow)
	{
		alertWindow->exitModalState (context.wasAborted() ? 1 : 0);
		alertWindow->setVisible (false);
		alertWindow = nullptr;
	}

	stopTimer();
}

void ModalTaskPopup::taskFinishedCallbacksDispatched (TaskContext&)
{
	delete this;
}

void ModalTaskPopup::taskStatusMessageChanged (TaskContext& )
{

}

void ModalTaskPopup::taskProgressChanged (TaskContext& context)
{
	progress = context.getTask().getProgress();
}

void ModalTaskPopup::taskStart ()
{
	String cancelButtonText = "Cancel";
	bool hasCancelButton = true;
	Component* componentToCentreAround = nullptr;
	bool hasProgressBar = true;

	progress = 0.0;

	alertWindow = LookAndFeel::getDefaultLookAndFeel()
		.createAlertWindow (getTaskContext()->getTask().getName(), String(),
		cancelButtonText.isEmpty() ? TRANS("Cancel") : cancelButtonText,
		String(), String(),
		AlertWindow::NoIcon, hasCancelButton ? 1 : 0,
		componentToCentreAround);

	alertWindow->setEscapeKeyCancels (false);

	if (hasProgressBar)
		alertWindow->addProgressBarComponent (progress);

	monitor (true);

	alertWindow->enterModalState();

	startTimer (10);
}

bool ModalTaskPopup::monitor (bool stillRunning)
{
	if (stillRunning && alertWindow->isCurrentlyModal())
	{
		alertWindow->setMessage (getTaskContext()->getTask().getStatusMessage());
		return true;
	}
	return false;
}

void ModalTaskPopup::launch (ProgressiveTask* task, const String& title, int priority, 
							 ProgressiveTask::Callback* callback)
{
	launch (new TaskContext(task), title, priority, callback);
}

void ModalTaskPopup::launch (TaskContext* context, const String& title, int priority, 
							 ProgressiveTask::Callback* callback)
{
	if (context)
	{
		if (callback != nullptr)
		{
			context->addCallback (callback);
		}

		new ModalTaskPopup (context);
		TaskThread::launch (context, title, priority);
	}
}

void ModalTaskPopup::timerCallback ()
{
	TaskContext* context = getTaskContext();
	if (context != nullptr)
	{
		if (!monitor (context->getState() == TaskContext::taskRunning))
		{
			context->getTask().abort();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

