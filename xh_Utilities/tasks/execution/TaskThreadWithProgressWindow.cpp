
///////////////////////////////////////////////////////////////////////////////

TaskThreadWithProgressWindow::TaskThreadWithProgressWindow (ProgressiveTask* taskToRun, bool owned, const String& title, Component* comp, Identifier id)
:	ThreadWithProgressWindow (title, true, true, 500000, String::empty, comp),
	task (taskToRun, owned),
	result (Result::ok()),
	taskId (id),
	async (false)
{
	jassert (task != nullptr); // not much use without a task!
}

TaskThreadWithProgressWindow::~TaskThreadWithProgressWindow ()
{
}

ProgressiveTask* TaskThreadWithProgressWindow::getCurrentTask ()
{
	return task;
}

Identifier TaskThreadWithProgressWindow::getId () const
{
	return taskId;
}

bool TaskThreadWithProgressWindow::currentTaskShouldExit ()
{
    return threadShouldExit ();
}

void TaskThreadWithProgressWindow::run ()
{
	if (task != nullptr)
	{
		addListener (this);
		result = task->run ();
		removeListener (this);
	}
}

void TaskThreadWithProgressWindow::threadComplete (bool userPressedCancel)
{
//	listeners.call (&Listener::progressiveTaskFinished, *this, userPressedCancel);

	if (result.failed ())
	{
		AlertWindow::showMessageBox(AlertWindow::WarningIcon, "Error!", result.getErrorMessage());
	}

	if (async)
	{
		delete this;
	}
}

Result TaskThreadWithProgressWindow::getResult () const
{
	return result;
}

Result TaskThreadWithProgressWindow::runTaskSynchronously (ProgressiveTask* task, bool owned, const String& title, Component* comp)
{
	if (task == nullptr)
		return Result::fail ("No task to run!");

	TaskThreadWithProgressWindow runner (task, owned, title, comp, Identifier::null);
	if (runner.runThread ())
	{
		return runner.getResult ();
	}
	return Result::fail ("Aborted");
}

TaskThreadWithProgressWindow& TaskThreadWithProgressWindow::runTask (Identifier id, ProgressiveTask* task, bool owned, const String& title, Component* comp)
{
	TaskThreadWithProgressWindow* runner = new TaskThreadWithProgressWindow (task, owned, title, comp, id);
//	if (listener != nullptr)
//		runner->addListener (listener);
	runner->launchThread ();
	runner->async = true;
	return *runner;
}

void TaskThreadWithProgressWindow::taskStatusMessageChanged (ProgressiveTask* task)
{
	setStatusMessage (task->getStatusMessage());
}

void TaskThreadWithProgressWindow::taskProgressChanged (ProgressiveTask* task)
{
	setProgress (task->getProgress ());
}

///////////////////////////////////////////////////////////////////////////////
