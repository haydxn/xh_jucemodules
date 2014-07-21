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

ProgressiveTask::ExecutionScope::SubTaskInfo::SubTaskInfo (ExecutionScope& scope,
                                                         ProgressiveTask& taskToPerform,
                                                         double proportion,
                                                         int index_, int count_)
:   parentScope (scope),
    task (taskToPerform),
    index (index_),
    count (count_),
    progressAtStart(scope.progress),
    progressAtEnd (scope.progress + proportion)
{
    parentScope.subTask = this;
}

ProgressiveTask::ExecutionScope::SubTaskInfo::~SubTaskInfo ()
{
    parentScope.subTask = nullptr;
}

double ProgressiveTask::ExecutionScope::SubTaskInfo::interpolateProgress (double amount) const
{
    double progress = progressAtStart + (amount * (progressAtEnd - progressAtStart));
    return jlimit (0.0, 1.0, progress);
}

///////////////////////////////////////////////////////////////////////////////

ProgressiveTask::ExecutionScope::ExecutionScope (Context& executionContext,
                                             ProgressiveTask& ownerTask, ProgressiveTask* parentTask)
:   context (executionContext),
    task (ownerTask),
    parent (parentTask),
    subTask(nullptr),
    progress (0.0)
{
    ScopedLock lock (context.getLock());
    
    jassert (task.scope == nullptr);
    task.scope = this;
}

ProgressiveTask::ExecutionScope::~ExecutionScope ()
{
    ScopedLock lock (context.getLock());

    jassert (subTask == nullptr);
    task.scope = nullptr;
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
    
    if (parent != nullptr)
    {
        double parentProgress = parent->scope->subTask->interpolateProgress (progress);
        parent->scope->setProgress (parentProgress);
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
    
    if (parent != nullptr)
    {
        parent->scope->setStatusMessage (parent->formatStatusMessageFromSubTask (task));
    }
    else
    {
        context.listeners.call (&ProgressiveTask::Context::Listener::taskStatusMessageChanged, &task);
    }
    //    task.notifyStatusChanged();
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
        if (scope->subTask != nullptr)
        {
            scope->subTask->task.abort ();
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
        //todo: maybe just make SubTaskInfo have/be a scope, so that the lock
        //      can be safely scoped in the constructor. Even simpler would be
        //      to make the scope just hold potential for subtask stuff, so that
        //      there is a single common scope type - though that would mean
        //      a chunk of stuff is unused for basic tasks...
        //      For now, i'll just enter and exit here...

        scope->getContext().getLock().enter ();
        ExecutionScope::SubTaskInfo info (*scope, taskToPerform, proportionOfProgress, index, count);
        ExecutionScope subTaskScope (scope->getContext(), taskToPerform, this);
        scope->getContext().getLock().exit ();

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

