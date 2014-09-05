///////////////////////////////////////////////////////////////////////////////

class TaskContext::ScopedRunTime
{
	JUCE_DECLARE_NON_COPYABLE (ScopedRunTime);
public:
	ScopedRunTime (TaskContext& context, TaskThreadBase& threadBase)
		:	owner(context)
	{
		jassert (threadBase.isCurrentTaskThread());
		ScopedLock lock (owner.runtimeLock);

		owner.taskThread = &threadBase;
		owner.setState (taskStarting);
	}

	~ScopedRunTime ()
	{
		ScopedLock lock (owner.runtimeLock);

		owner.setState (taskCompleted);
		owner.taskThread = nullptr;
	}

private:

	TaskContext& owner;
};

///////////////////////////////////////////////////////////////////////////////

TaskContext::TaskContext (ProgressiveTask* taskToRun)
	:	activeTask (taskToRun),
		taskThread (nullptr),
		result (Result::ok()),
		currentState (taskPending)
{
}

TaskContext::~TaskContext ()
{
	// This should always be destroyed from the message thread...
	jassert (MessageManager::getInstance()->isThisTheMessageThread());
	// ... and it should not still be getting executed!
	jassert (taskThread == nullptr);

	flush ();
}

void TaskContext::addCallback (ProgressiveTask::Callback* callback)
{
	callbacks.add (callback);
}

ProgressiveTask& TaskContext::getTask ()
{
	return *activeTask;
}

juce::Result TaskContext::getResult () const
{
	return result;
}

bool TaskContext::wasAborted () const
{
	return getState() == taskAborted;
}

TaskContext::TaskState TaskContext::getState () const
{
	ScopedLock lock (runtimeLock);
	return currentState;
}

bool TaskContext::hasFinished () const
{
	// Probably no real need to lock this...
	return (currentState == taskCompleted) || (currentState == taskAborted);
}

String TaskContext::getStateDescription () const
{
	TaskState state = getState ();
	switch (state)
	{
	case taskPending:	return "Waiting...";
	case taskStarting:	return "Starting...";
	case taskRunning:	return "Running...";
	case taskStopping:	return "Stopping...";
	case taskAborted:	return "Aborted.";

	case taskCompleted:	

		if (getResult().wasOk())
		{
			return "Completed successfully.";
		}
		else
		{
			return "Failed: " + getResult().getErrorMessage();
		}

	default:;
	};
	return String::empty;
}

bool TaskContext::currentTaskShouldExit ()
{
	if (taskThread != nullptr)
	{
		return taskThread->currentTaskShouldExit();
	}
	return true;
}

juce::Result TaskContext::runTask (TaskThreadBase& threadBase)
{
	ScopedRunTime runTime (*this, threadBase);

	ProgressiveTask::ExecutionScope localRunTime (*this, *activeTask, nullptr);

	setState (taskRunning);
	result = activeTask->run ();
	setState (taskStopping);

	return result;
}

void TaskContext::handleAsyncUpdate ()
{
	bool aborted = wasAborted();

	listeners.call (&TaskContext::Listener::aboutToDispatchTaskFinishedCallbacks, *this);

	for (int i=0; i<callbacks.size(); i++)
	{
		ProgressiveTask::Callback* callback = callbacks.getUnchecked (i);
		callback->taskFinishedCallback (result, aborted);
	}
	callbacks.clear ();

	listeners.call (&TaskContext::Listener::taskFinishedCallbacksDispatched, *this);
}

void TaskContext::setState (TaskState state)
{
	ScopedLock lock (runtimeLock);

	if (state != currentState)
	{
		currentState = state;

		switch (currentState)
		{
		case taskStarting:

			taskAboutToStart ();
			break;

		case taskCompleted:
		case taskAborted:

			if (currentTaskShouldExit() || activeTask->abortSignal)
			{
				currentState = taskAborted;
			}

			triggerAsyncUpdate ();

			taskAboutToTerminate ();
			break;

		default:

			break;
		};

		listeners.call (&TaskContext::Listener::taskStateChanged, *this);
	}
}


void TaskContext::taskAboutToStart ()
{

}

void TaskContext::taskAboutToTerminate ()
{

}

void TaskContext::addListener (Listener* listener)
{
	listeners.add (listener);
}

void TaskContext::removeListener (Listener* listener)
{
	listeners.remove (listener);
}

void TaskContext::flush ()
{
	handleUpdateNowIfNeeded ();
}

///////////////////////////////////////////////////////////////////////////////

ProgressiveTask::ExecutionScope::ExecutionScope (TaskContext& executionContext, ProgressiveTask& task_, 
												 ExecutionScope* parentScope_,
												double proportionOfProgress, 
												int index_, int count_)
:   context (executionContext),
	task (task_),
	parentScope (parentScope_),
	subTaskScope (nullptr),
	progress (0.0),
	progressAtStart (0.0),
	progressAtEnd (1.0),
	index (index_),
	count (count_)
{
	ScopedLock lock (context.getLock());

	jassert (task.scope == nullptr);

	task.scope = this;

	if (parentScope != nullptr)
	{
		parentScope->subTaskScope = this;
		progressAtStart = parentScope->progress;
		progressAtEnd = jmin (progressAtStart + proportionOfProgress, 1.0);
	}
}

ProgressiveTask::ExecutionScope::~ExecutionScope ()
{
    ScopedLock lock (context.getLock());

	jassert (subTaskScope == nullptr);
    
	task.scope = nullptr;

	if (parentScope != nullptr)
	{
		jassert (parentScope->subTaskScope == this);
		parentScope->subTaskScope = nullptr;
	}
}

ProgressiveTask& ProgressiveTask::ExecutionScope::getTask ()
{
    return task;
}

TaskContext& ProgressiveTask::ExecutionScope::getContext ()
{
    return context;
}

void ProgressiveTask::ExecutionScope::setProgress (double newProgress)
{
    progress = jlimit (0.0, 1.0, newProgress);
    
    if (parentScope != nullptr)
    {
        parentScope->setProgress (interpolateProgress (progress));
    }
    else
    {
        context.listeners.call (&TaskContext::Listener::taskProgressChanged, context);
    }
}

void ProgressiveTask::ExecutionScope::setStatusMessage (const String& message)
{
    statusMessage = message;
    
    if (parentScope != nullptr)
    {
        parentScope->setStatusMessage (parentScope->getTask().formatStatusMessageFromSubTask (task));
    }
    else
    {
        context.listeners.call (&TaskContext::Listener::taskStatusMessageChanged, context);
    }
}

double ProgressiveTask::ExecutionScope::interpolateProgress (double amount) const
{
	double progress = progressAtStart + (amount * (progressAtEnd - progressAtStart));
	return jlimit (0.0, 1.0, progress);
}

///////////////////////////////////////////////////////////////////////////////

const juce::Result ProgressiveTask::taskAlreadyRunning (Result::fail("Task already running"));

ProgressiveTask::ProgressiveTask (const String& taskName)
	:	name (taskName),
        scope (nullptr),
		abortSignal (false)
{

}

ProgressiveTask::~ProgressiveTask ()
{
    jassert (scope == nullptr);
}

String ProgressiveTask::getName () const
{
	return name;
}

bool ProgressiveTask::isRunning () const
{
    return scope != nullptr;
}

void ProgressiveTask::setProgress (double newProgress)
{
    if (scope != nullptr)
    {
        scope->setProgress (newProgress);
    }
}

void ProgressiveTask::advanceProgress (double amount)
{
	setProgress (getProgress() + amount);
}

double ProgressiveTask::getProgress () const
{
    if (scope != nullptr)
        return scope->progress;
    return 0.0;
}

double ProgressiveTask::getDistanceToTargetProgress (double target) const
{
    if (scope != nullptr)
    {
        return jmax (0.0, target - scope->progress);
    }
    return 0.0;
}

void ProgressiveTask::setStatusMessage (const String& message)
{
    if (scope != nullptr)
    {
        scope->setStatusMessage (message);
    }
}

String ProgressiveTask::getStatusMessage () const
{
    if (scope != nullptr)
    {
        return scope->statusMessage;
    }
    return String::empty;
}

void ProgressiveTask::abort ()
{
	abortSignal = true;
	if (scope != nullptr)
    {
        if (scope->subTaskScope != nullptr)
        {
            scope->subTaskScope->task.abort ();
        }
    }
}

bool ProgressiveTask::shouldAbort () const
{
	return abortSignal || threadShouldExit ();
}

Result ProgressiveTask::performSubTask (ProgressiveTask& taskToPerform, double proportionOfProgress, int index, int count)
{
    jassert (scope != nullptr);
    
    if (scope != nullptr && !taskToPerform.isRunning())
    {
		ExecutionScope subTask (scope->getContext(), taskToPerform, scope, proportionOfProgress, index, count);
        return taskToPerform.run ();
    }
    return taskAlreadyRunning;
}

Result ProgressiveTask::performSubTask (ProgressiveTask& taskToPerform, double proportionOfProgress)
{
	return performSubTask (taskToPerform, proportionOfProgress, 0, 1);
}

Result ProgressiveTask::performSubTaskSequence (const TaskSequence& sequence,
                                                double proportionOfProgress, bool stopOnError)
{
	StringArray errorMessages;

	double endProgress = getProgress() + proportionOfProgress;

	for (int i=0; i<sequence.size(); i++)
	{
		if (shouldAbort())
			return Result::ok();

		ProgressiveTask* subTask = sequence.getTask (i);

		//notifyStatusChanged ();

		Result subTaskResult = performSubTask (*subTask, sequence.getTaskProportion (i) * proportionOfProgress, i, sequence.size());

		if (subTaskResult.failed())
		{
			errorMessages.add (subTaskResult.getErrorMessage());

			if (stopOnError)
			{
				setProgress (endProgress);
				break;
			}
		}
	}

	if (errorMessages.size () > 0)
	{
		String message = errorMessages.joinIntoString (newLine);
		return Result::fail (message);
	}

	return Result::ok ();

}

void ProgressiveTask::subTaskStarting (ProgressiveTask*, int, int)
{

}

String ProgressiveTask::formatStatusMessageFromSubTask (ProgressiveTask& subTask)
{
	return subTask.getStatusMessage ();
}

Result ProgressiveTask::getAbortResult ()
{
	return Result::ok ();
}

bool ProgressiveTask::threadShouldExit () const
{
    if (scope != nullptr)
    {
        return scope->getContext().currentTaskShouldExit();
    }
    return true;
}

const ProgressiveTask::ExecutionScope* ProgressiveTask::getScope () const
{
    return scope;
}


///////////////////////////////////////////////////////////////////////////////

void TaskThreadBase::runTask (TaskContext::Ptr taskContext)
{
	jassert (isCurrentTaskThread());

	if (taskContext != nullptr)
	{
		taskContext->runTask (*this);
	}
}



///////////////////////////////////////////////////////////////////////////////

class TaskInterface::AsyncRefresh	:	public AsyncUpdater
{
public:

	AsyncRefresh (TaskInterface& owner_)
		:	owner (owner_)
	{
	}

	virtual void handleAsyncUpdate () override
	{
		if (owner.task != nullptr)
		{
			ScopedLock lock (owner.task->getLock());
			owner.refreshInternal ();
		}
	}

private:

	TaskInterface& owner;
};

///////////////////////////////////////////////////////////////////////////////

TaskInterface::TaskInterface ()
{
	refreshCallback = new AsyncRefresh (*this);
}

TaskInterface::~TaskInterface ()
{
	setTaskContext (nullptr, false);
}

void TaskInterface::setTaskContext (TaskContext* taskToView, bool alsoTriggerRefresh)
{
	if (taskToView != task)
	{
		if (task != nullptr)
		{
			task->removeListener (this);
		}

		task = taskToView;

		if (taskToView != nullptr)
		{
			taskToView->addListener (this);
		}

		if (alsoTriggerRefresh)
		{
			taskContextChanged ();
			triggerRefresh ();
		}
	}
}

TaskContext* TaskInterface::getTaskContext ()
{
	return task;
}

void TaskInterface::refreshInternal ()
{
	if (task != nullptr)
	{
		refresh (*task);
	}
}

void TaskInterface::refresh (TaskContext&)
{

}

void TaskInterface::triggerRefresh ()
{
	refreshCallback->triggerAsyncUpdate ();
}

void TaskInterface::taskContextChanged ()
{
}

void TaskInterface::taskStateChanged (TaskContext&)
{
	triggerRefresh ();
}


///////////////////////////////////////////////////////////////////////////////
