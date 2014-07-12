
///////////////////////////////////////////////////////////////////////////////

class ProgressiveTask::ScopedSubTaskTracker	:	public ProgressiveTask::Listener
{
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScopedSubTaskTracker);
public:

	ScopedSubTaskTracker (ProgressiveTask& parentTask, ProgressiveTask& taskToRun, double proportion)
		:	owner (parentTask),
			subTask (taskToRun),
			progressAtStart (parentTask.getProgress()),
			progressAtEnd (parentTask.getProgress() + proportion)
	{
		jassert (owner.currentSubTask == nullptr); // You can't run more than one sub-task at a time!
		owner.currentSubTask = this;

		subTask.addListener (this);

		if (progressAtEnd > 1.0)
			progressAtEnd = 1.0;
	}

	~ScopedSubTaskTracker ()
	{
		subTask.removeListener (this);
		owner.currentSubTask = nullptr;
	}

	double interpolateProgress (double amount) const
	{
		double progress = progressAtStart + (amount * (progressAtEnd - progressAtStart));
		return jlimit (0.0, 1.0, progress);
	}

	void taskStatusMessageChanged (ProgressiveTask* task) override
	{
		owner.setStatusMessage (owner.formatStatusMessageFromSubTask(*task));
	}

	void taskProgressChanged (ProgressiveTask* task) override
	{
		owner.setProgress (interpolateProgress (task->getProgress ()));
	}

	void abort ()
	{
		subTask.abort ();
	}
	
	ProgressiveTask& owner;
	ProgressiveTask& subTask;
	double progressAtStart;
	double progressAtEnd;
};

///////////////////////////////////////////////////////////////////////////////

ProgressiveTask::ProgressiveTask (const String& taskName)
	:	name (taskName),
        progress (0.0),
        currentSubTask (nullptr),
		abortSignal (false)
{

}

ProgressiveTask::~ProgressiveTask ()
{

}

String ProgressiveTask::getName () const
{
	return name;
}

void ProgressiveTask::setProgress (double newProgress)
{
	newProgress = jlimit (0.0, 1.0, newProgress);
	if (progress != newProgress)
	{
		progress = newProgress;
		notifyProgressChanged ();
	}
}

void ProgressiveTask::advanceProgress (double amount)
{
	setProgress (getProgress() + amount);
}

double ProgressiveTask::getProgress () const
{
	return progress;
}

double ProgressiveTask::getDistanceToTargetProgress (double target) const
{
	return jmax (0.0, target - progress);
}

void ProgressiveTask::setStatusMessage (const String& message)
{
	statusMessage = message;
	notifyStatusChanged ();
}

String ProgressiveTask::getStatusMessage () const
{
	return statusMessage;
}

void ProgressiveTask::abort ()
{
	abortSignal = true;
	if (currentSubTask != nullptr)
	{
		currentSubTask->abort ();
	}
}

bool ProgressiveTask::shouldAbort () const
{
	return abortSignal || threadShouldExit ();
}

Result ProgressiveTask::performSubTask (ProgressiveTask& taskToPerform, double proportionOfProgress, int index, int count)
{
	ScopedSubTaskTracker subTask (*this, taskToPerform, proportionOfProgress);
	subTaskStarting (&taskToPerform, index, count);
	return taskToPerform.performTask ();
}

Result ProgressiveTask::performSubTask (ProgressiveTask& taskToPerform, double proportionOfProgress)
{
	return performSubTask (taskToPerform, proportionOfProgress, 0, 1);
}

Result ProgressiveTask::performSubTaskSequence (const TaskSequence& sequence, double proportionOfProgress, bool stopOnError, Listener* subTaskListener)
{
	StringArray errorMessages;

	double endProgress = getProgress() + proportionOfProgress;

	for (int i=0; i<sequence.size(); i++)
	{
		if (shouldAbort())
			return Result::ok();

		ProgressiveTask* subTask = sequence.getTask (i);

		notifyStatusChanged ();

		if (subTaskListener != nullptr)
		{
			subTask->addListener(subTaskListener);
		}

		Result subTaskResult = performSubTask (*subTask, sequence.getTaskProportion (i) * proportionOfProgress, i, sequence.size());

		if (subTaskListener != nullptr)
		{
			subTask->removeListener(subTaskListener);
		}

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

void ProgressiveTask::addListener (Listener* listener)
{
	listeners.add (listener);
}

void ProgressiveTask::removeListener (Listener* listener)
{
	listeners.remove (listener);
}

void ProgressiveTask::notifyProgressChanged ()
{
	listeners.call (&Listener::taskProgressChanged, this);
}

void ProgressiveTask::notifyStatusChanged ()
{
	listeners.call (&Listener::taskStatusMessageChanged, this);
}

Result ProgressiveTask::getAbortResult ()
{
	return Result::ok ();
}

bool ProgressiveTask::threadShouldExit () const
{
	Thread* currentThread = Thread::getCurrentThread ();
	if (currentThread != nullptr)
	{
		return currentThread->threadShouldExit ();
	}

	ThreadPoolJob* currentJob = ThreadPoolJob::getCurrentThreadPoolJob();
	if (currentJob != nullptr)
	{
		return currentJob->shouldExit();
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////

