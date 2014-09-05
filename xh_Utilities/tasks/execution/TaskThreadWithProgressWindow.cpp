
///////////////////////////////////////////////////////////////////////////////

// class TaskThreadWithProgressWindow::LegacyListenerCallback	:	public Task::Callback
// {
// public:
// 
// 	LegacyListenerCallback (TaskThreadWithProgressWindow& owner_)
// 		:	owner(owner_)
// 	{
// 
// 	}
// 
// 	virtual void taskFinishedCallback (const juce::Result& result, bool wasAborted) override
// 	{
// 
// 	}
// 
// private:
// 
// 	TaskThreadWithProgressWindow& owner;
// };

TaskThreadWithProgressWindow::TaskThreadWithProgressWindow (ProgressiveTask* taskToRun, const String& title, Component* comp, Identifier id)
:	ThreadWithProgressWindow (title, true, true, 500000, String::empty, comp),
	task (new TaskContext(taskToRun)),
	result (Result::ok()),
	taskId (id),
	async (false)
{
//	jassert (task != nullptr); // not much use without a task!
}

TaskThreadWithProgressWindow::~TaskThreadWithProgressWindow ()
{
}

ProgressiveTask* TaskThreadWithProgressWindow::getCurrentTask ()
{
	return &task->getTask();//&getTask();
}

Identifier TaskThreadWithProgressWindow::getId () const
{
	return taskId;
}

bool TaskThreadWithProgressWindow::currentTaskShouldExit ()
{
    return threadShouldExit ();
}

bool TaskThreadWithProgressWindow::isCurrentTaskThread ()
{
	return getCurrentThread() == this;
}

void TaskThreadWithProgressWindow::run ()
{
// 	if (getTask!= nullptr)
// 	{
		task->addListener (this);

		TaskThreadBase::runTask (task);
//		result = task->run ();
		task->removeListener (this);
// 	}
}

// void TaskThreadWithProgressWindow::addLegacyListener (LegacyListener* listener)
// {
// 	legacyListeners.add (listener);
// }
// 
// void TaskThreadWithProgressWindow::removeLegacyListener (LegacyListener* listener)
// {
// 	legacyListeners.remove (listener);
// }

void TaskThreadWithProgressWindow::threadComplete (bool)
{
// 	legacyListeners.call (&LegacyListener::progressiveTaskFinished, *this, userAborted);

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

Result TaskThreadWithProgressWindow::runTaskSynchronously (ProgressiveTask* task, const String& title, Component* comp)
{
	if (task == nullptr)
		return Result::fail ("No task to run!");

	TaskThreadWithProgressWindow runner (task, title, comp, Identifier::null);
	if (runner.runThread ())
	{
		return runner.getResult ();
	}
	return Result::fail ("Aborted");
}

//TaskThreadWithProgressWindow& TaskThreadWithProgressWindow::runTask (Identifier id, Task* task, bool owned, const String& title, Component* comp)
TaskThreadWithProgressWindow& TaskThreadWithProgressWindow::runTask (Identifier id, ProgressiveTask* task, const String& title, Component* comp)
{
	TaskThreadWithProgressWindow* runner = new TaskThreadWithProgressWindow (task, title, comp, id);

// 	if (listener != nullptr)
// 		runner->addLegacyListener (listener);

	runner->launchThread ();
	runner->async = true;
	return *runner;
}

void TaskThreadWithProgressWindow::taskStatusMessageChanged (TaskContext& context)
{
	setStatusMessage (context.getTask().getStatusMessage());
}

void TaskThreadWithProgressWindow::taskProgressChanged (TaskContext& context)
{
	setProgress (context.getTask().getProgress ());
}

///////////////////////////////////////////////////////////////////////////////
