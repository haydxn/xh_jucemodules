
///////////////////////////////////////////////////////////////////////////////

TaskThread::TaskThread (const String& runnerTitle)
	:	Thread (runnerTitle),
		title (runnerTitle),
		timeoutMsWhenCancelling(5000)
{

}

TaskThread::~TaskThread ()
{

}

juce::String TaskThread::getTitle () const
{
	return title;
}

void TaskThread::setTask (TaskHandler* handler, bool)
{
	taskHandler = handler;
}

TaskHandler* TaskThread::getTaskHandler ()
{
	return taskHandler;
}

ProgressiveTask* TaskThread::getTask () const
{
	if (taskHandler != nullptr)
	{
		return &taskHandler->getTask ();
	}
	return nullptr;
}

Result TaskThread::runSynchronously (int priority)
{
	launchThread (priority, false);

	while (!taskHandler->hasFinished() || isThreadRunning())
		MessageManager::getInstance()->runDispatchLoopUntil (5);

	return taskHandler->getResult();
}

void TaskThread::launchThread (int priority, bool destroyWhenComplete)
{
	jassert (taskHandler != nullptr);
	
	if (destroyWhenComplete)
		selfDestructCallback = new AsyncFunc (*this, &TaskThread::selfDestruct);

	startThread (priority);
}

void TaskThread::run ()
{
	if (taskHandler)
		taskHandler->performTask();

	if (selfDestructCallback != nullptr)
		selfDestructCallback->trigger();
}

void TaskThread::selfDestruct()
{
	delete this;
}

juce::Result TaskThread::runSynchronously (TaskHandler* task, const juce::String& title, int priority)
{
	TaskThread runner (title);
	runner.setTask (task, true);
	return runner.runSynchronously (priority);
}

void TaskThread::launch (TaskHandler* handler, const juce::String& title, int priority)
{
	TaskThread* runner = new TaskThread (title);
	runner->setTask (handler, true);
	runner->launchThread (priority, true);
}

///////////////////////////////////////////////////////////////////////////////
