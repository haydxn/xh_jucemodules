
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

void TaskThread::setTask (TaskContext* handler, bool)
{
	activeTask = handler;
}

TaskContext* TaskThread::getTaskContext ()
{
	return activeTask;
}

ProgressiveTask* TaskThread::getTask () const
{
	if (activeTask != nullptr)
	{
		return &activeTask->getTask ();
	}
	return nullptr;
}

Result TaskThread::runSynchronously (int priority)
{
	launchThread (priority, false);

	while (!activeTask->hasFinished() || isThreadRunning())
		MessageManager::getInstance()->runDispatchLoopUntil (5);

	return activeTask->getResult();
}

void TaskThread::launchThread (int priority, bool destroyWhenComplete)
{
	jassert (activeTask != nullptr);
	
	if (destroyWhenComplete)
		selfDestructCallback = new AsyncFunc (*this, &TaskThread::selfDestruct);

	startThread (priority);
}

void TaskThread::run ()
{
	if (activeTask)
	{
		runTask (activeTask);
	}
		//taskHandler->performTask (*this);

	if (selfDestructCallback != nullptr)
		selfDestructCallback->trigger();
}

bool TaskThread::currentTaskShouldExit ()
{
    return threadShouldExit();
}

bool TaskThread::isCurrentTaskThread ()
{
	return getCurrentThread() == this;
}

void TaskThread::selfDestruct()
{
	delete this;
}

juce::Result TaskThread::runSynchronously (TaskContext* task, const juce::String& title, int priority)
{
	TaskThread runner (title);
	runner.setTask (task, true);
	return runner.runSynchronously (priority);
}

void TaskThread::launch (TaskContext* task, const juce::String& title, int priority)
{
	TaskThread* runner = new TaskThread (title);
	runner->setTask (task, true);
	runner->launchThread (priority, true);
}

///////////////////////////////////////////////////////////////////////////////
