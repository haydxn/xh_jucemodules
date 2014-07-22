///////////////////////////////////////////////////////////////////////////////

void ProgressiveTask::Context::addListener (Listener* listener)
{
	listeners.add (listener);
}

void ProgressiveTask::Context::removeListener (Listener* listener)
{
	listeners.remove (listener);
}


///////////////////////////////////////////////////////////////////////////////

ProgressiveTask::ExecutionScope::ExecutionScope (Context& executionContext, ProgressiveTask& task_, 
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

ProgressiveTask::Context& ProgressiveTask::ExecutionScope::getContext ()
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
        context.listeners.call (&ProgressiveTask::Context::Listener::taskProgressChanged, &task);
    }
    //task.notifyProgressChanged();
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
        context.listeners.call (&ProgressiveTask::Context::Listener::taskStatusMessageChanged, &task);
    }
    //    task.notifyStatusChanged();
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

Result ProgressiveTask::performTask (Context& context)
{
    if (scope == nullptr)
    {
        ExecutionScope localRunTime (context, *this, nullptr);
        return run ();
    }
    return taskAlreadyRunning;
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

