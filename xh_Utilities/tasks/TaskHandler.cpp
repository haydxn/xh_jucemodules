
///////////////////////////////////////////////////////////////////////////////

class TaskHandler::Monitor	:	public Timer
{
public:

	enum State
	{
		initial,
		starting,
		running,
		stopping,
	};

	Monitor (TaskHandler& owner_)
		:	owner (owner_),
		state (initial),
		aborted (false)
	{
	}

	void begin ()
	{
		startTimer(100);

		if (MessageManager::getInstance()->isThisTheMessageThread())
		{
			owner.taskStarted();
			state = running;
		}
		else
		{
			state = starting;
			blocker.wait (); // wait until taskStarted has been called
		}
	}

	void finish ()
	{
		state = stopping;

		if (!MessageManager::getInstance()->isThisTheMessageThread())
		{
			blocker.wait ();
		}
	}

	void timerCallback () override
	{
		switch (state)
		{
		case starting:

			owner.taskStarted();
			blocker.signal ();
			state = running;
			break;

		case running:
		case stopping:

			if (!owner.taskMonitor (state == running))
			{
				if (state == running)
				{
					owner.getTask().abort();
					aborted = true;
				}

				stopTimer ();

				owner.finished = true;
				owner.taskFinished (aborted);
				owner.callCallbacks (aborted);
				blocker.signal ();
			}

		default:;
		};
	}

private:

	TaskHandler& owner;
	WaitableEvent blocker;
	State state;
	bool aborted;

};


///////////////////////////////////////////////////////////////////////////////

TaskHandler::TaskHandler (ProgressiveTask* taskToRun, bool owned)
	:	task (taskToRun, owned),
		result (Result::ok()),
		finished (false)
{
	task->addListener(this);
}

TaskHandler::~TaskHandler ()
{
	task->removeListener (this);
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
	return finished;
}

ProgressiveTask& TaskHandler::getTask ()
{
	return *task;
}

juce::Result TaskHandler::getResult () const
{
	return result;
}

void TaskHandler::performTask ()
{
	Monitor monitor (*this);

	monitor.begin ();

	result = task->performTask ();

	monitor.finish ();
}

void TaskHandler::callCallbacks (bool aborted)
{
	jassert (MessageManager::getInstance()->isThisTheMessageThread());

	for (int j = callbacks.size(); --j >= 0;)
		callbacks.getUnchecked (j)->progressiveTaskFinished (*this, aborted);

	callbacks.clear ();
}

void TaskHandler::addCallback (Callback* callbackToAdd)
{
	callbacks.add (callbackToAdd);
}


///////////////////////////////////////////////////////////////////////////////
