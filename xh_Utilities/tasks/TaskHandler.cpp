
///////////////////////////////////////////////////////////////////////////////

class TaskHandler::Monitor	:	public AsyncUpdater,
                                public ProgressiveTask::Context::Listener
{
public:

	Monitor (TaskHandler& owner_)
	:	owner (owner_),
		state (taskPending)
	{
		jassert (owner.getState() == TaskHandler::taskPending);
	}

	~Monitor()
	{
	}

	void begin ()
	{
		setState (taskStarting);
	}

	void finish ()
	{
		if (state == taskRunning)
		{
			setState (taskStopping);
		}
	}

	void forceFinish ()
	{
		if (state == taskRunning)
		{
			owner.getTask().abort();
		}

		bool aborted = owner.getTask().shouldAbort();

		setState (aborted ? taskAborted : taskCompleted);
	}

	void setState (TaskState newState)
	{
		if (state != newState)
		{
			state = newState;

			triggerAsyncUpdate();

			if (!MessageManager::getInstance()->isThisTheMessageThread())
			{
				// Called from the task thread, so we'll wait for the main
				// thread to pick it up and let us continue...
				while (!blocker.wait(20))
				{
					// ... still need to make sure that the task thread can
					// be terminated if necessary!
					if (owner.getTask().threadShouldExit())
						break;
				}
			}
			else
			{
				// Called from the message thread, so we can assume that it
				// is safe to allow the task thread to continue.
				owner.setState (newState);
				blocker.signal();
			}
		}
	}

	void handleAsyncUpdate () override
	{
		// Flush pending state change since we know we're on the message thread
		if (state != owner.getState())
		{
			owner.setState (state);
		}

		switch (state)
		{
		case taskStarting:

			setState (taskRunning);
			break;

		case taskRunning:
		case taskStopping:

			if (!owner.taskMonitor (state == taskRunning))
			{
				forceFinish ();
			}
			break;

		case taskAborted:
		case taskCompleted:

			//owner.monitor = nullptr;
			break;

		default:;
		};
	}

	void taskStatusMessageChanged (ProgressiveTask* task) override
	{
		ScopedLock lock(owner.messageLock);
		owner.statusMessage = task->getStatusMessage();
	}

	void taskProgressChanged (ProgressiveTask* task) override
	{
		owner.overallProgress = task->getProgress();
	}

private:

	TaskHandler& owner;
	TaskState state;
	WaitableEvent blocker;
};


///////////////////////////////////////////////////////////////////////////////

TaskHandler::TaskHandler (ProgressiveTask* taskToRun)
	:	task (taskToRun),
        state (taskPending),
        result (Result::ok()),
		overallProgress (0.0)
{
}

TaskHandler::~TaskHandler ()
{
	masterReference.clear ();
	jassert (MessageManager::getInstance()->isThisTheMessageThread());
	// It's not safe to destroy one of these from another thread, since their
	// callbacks are meant to happen on the message thread. It's actually
	// only really a problem if the task hasn't finished yet, but you might
	// not actually encounter that during testing despite it being something
	// that could happen in your code. For now, I'm leaving this assert in.
}

TaskHandler::TaskState TaskHandler::getState () const
{
	return state;
}

juce::String TaskHandler::getStateDescription () const
{
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

void TaskHandler::setId (Identifier taskId)
{
	id = taskId;
}

Identifier TaskHandler::getId () const
{
	return id;
}

bool TaskHandler::hasFinished () const
{
	return (state == taskCompleted || state == taskAborted);
}

String TaskHandler::getStatusMessage () const
{
	ScopedLock lock (messageLock);
	return statusMessage;
}

double TaskHandler::getOverallProgress () const
{
	return overallProgress;
}

double& TaskHandler::getOverallProgressVariable ()
{
	return overallProgress;
}

ProgressiveTask& TaskHandler::getTask ()
{
	return *task;
}

juce::Result TaskHandler::getResult () const
{
	return result;
}

void TaskHandler::performTask (ProgressiveTask::Context& context)
{
	if (state == taskPending)
	{
		monitor = new Monitor (*this);
        context.addListener (monitor);

		monitor->begin ();

		result = task->performTask (context);

        context.removeListener (monitor);
		if (monitor != nullptr)
			monitor->finish ();
	}
}

void TaskHandler::flushState ()
{
	if (MessageManager::getInstance()->isThisTheMessageThread())
	{
		if (monitor != nullptr)
		{
			monitor->forceFinish ();
		}
	}
}

void TaskHandler::setState (TaskState newState)
{
	if (state != newState)
	{
		state = newState;

		switch (newState)
		{
		case taskStarting:

			taskStart ();
			break;

		case taskCompleted:
		case taskAborted:

			taskFinish (newState == taskAborted);
			callCallbacks (newState == taskAborted);
			break;

		default:;
		}

		listeners.call (&Listener::taskHandlerStateChanged, *this);
	}
}

void TaskHandler::taskStart ()
{
}

bool TaskHandler::taskMonitor (bool stillRunning)
{
	return stillRunning;
}

void TaskHandler::taskFinish (bool)
{
}

void TaskHandler::callCallbacks (bool aborted)
{
	jassert (MessageManager::getInstance()->isThisTheMessageThread());

	for (int j = callbacks.size(); --j >= 0;)
		callbacks.getUnchecked (j)->progressiveTaskFinished (*this, aborted);

	callbacks.clear ();
}

void TaskHandler::addListener (Listener* listener)
{
	listeners.add (listener);
}

void TaskHandler::removeListener (Listener* listener)
{
	listeners.remove (listener);
}

void TaskHandler::addCallback (Callback* callbackToAdd)
{
	if (callbackToAdd != nullptr)
		callbacks.add (callbackToAdd);
}

///////////////////////////////////////////////////////////////////////////////
