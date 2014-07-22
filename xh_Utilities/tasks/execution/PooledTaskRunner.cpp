
///////////////////////////////////////////////////////////////////////////////

class TaskThreadPool::IdSelector	:	public ThreadPool::JobSelector
{
public:
    
	IdSelector (Identifier id)
	:	idToMatch (id)
	{
	}
    
	virtual ~IdSelector ()
	{
	}
    
	virtual bool isJobSuitable (ThreadPoolJob *job) override
	{
		Job* taskJob = static_cast< Job* >(job);
		return (taskJob != nullptr) && (taskJob->getId() == idToMatch);
	}
    
	Identifier idToMatch;
};

///////////////////////////////////////////////////////////////////////////////

TaskThreadPool::Job::Job (TaskHandler* handler, TaskThreadPool& owner_)
:	ThreadPoolJob (handler != nullptr ? handler->getTask().getName() : String::empty),
    taskHandler (handler),
    owner (owner_)
{
}

TaskThreadPool::Job::~Job ()
{
	taskHandler = nullptr;
}

TaskThreadPool& TaskThreadPool::Job::getOwner ()
{
    return owner;
}

TaskHandler* TaskThreadPool::Job::getTaskHandler ()
{
	return taskHandler;
}

TaskHandler* TaskThreadPool::Job::releaseTaskHandler ()
{
	return taskHandler.release();
}

juce::Identifier TaskThreadPool::Job::getId () const
{
	if (taskHandler != nullptr)
	{
		return taskHandler->getId();
	}
	return Identifier::null;
}

ThreadPoolJob::JobStatus TaskThreadPool::Job::runJob ()
{
	if (taskHandler != nullptr)
	{
		taskHandler->performTask (*this);
	}
    
    owner.jobFinishedInternal (*this);

	return ThreadPoolJob::jobHasFinished;
}

bool TaskThreadPool::Job::currentTaskShouldExit ()
{
    return shouldExit();
}


///////////////////////////////////////////////////////////////////////////////

TaskThreadPool::TaskThreadPool (int maxConcurrentTasks)
:	itemsChangedFunc (*this, &TaskThreadPool::itemsChanged),
    maxConcurrentTaskLimit (maxConcurrentTasks)
{
	OwnedArray<ThreadPoolJob> leakDetectorRaceConditionDummy;

	pool = new ThreadPool (maxConcurrentTaskLimit);
}

TaskThreadPool::~TaskThreadPool ()
{
	pool->removeAllJobs (true, 5000);
	pool = nullptr;
}

int TaskThreadPool::getNumTasks () const
{
	ScopedLock lock(getLock());
	return pool->getNumJobs ();
}

TaskHandler* TaskThreadPool::getTaskHandler (int index) const
{
	ScopedLock lock(getLock());
	Job* job = dynamic_cast< Job* > (pool->getJob (index));
	if (job != nullptr)
	{
		return job->getTaskHandler();
	}
	return nullptr;
}

TaskHandler& TaskThreadPool::addTask (ProgressiveTask* taskToRun, Identifier id)
{
	TaskHandler* handler = createHandlerForTask (taskToRun);

	if (handler == nullptr)
		handler = new TaskHandler (taskToRun);

    handler->setId (id);
    
	addHandlerToPool (handler);
	return *handler;
}

void TaskThreadPool::addTask (TaskHandler* handler)
{
    if (handler != nullptr)
    {
        addHandlerToPool (handler);
    }
}

bool TaskThreadPool::removeAllTasks (bool interruptRunningJobs, int timeOutMilliseconds)
{
	return pool->removeAllJobs (interruptRunningJobs, timeOutMilliseconds);
}

bool TaskThreadPool::removeAllTasksWithId (juce::Identifier id, bool interruptRunningJobs, int timeOutMilliseconds)
{
    IdSelector selector (id);
	return pool->removeAllJobs (interruptRunningJobs, timeOutMilliseconds, &selector);
}

TaskHandler* TaskThreadPool::createHandlerForTask (ProgressiveTask* task)
{
	return handlerCreator.create (task);//new TaskHandler (task);
}

TaskThreadPool::Job* TaskThreadPool::createJobForHandler (TaskHandler *handler)
{
    return jobCreator.create (handler, *this);
    //return new Job (handler, *this);
}

void TaskThreadPool::addListener (Listener* listener)
{
	listeners.add (listener);
}

void TaskThreadPool::removeListener (Listener* listener)
{
	listeners.remove (listener);
}

const CriticalSection& TaskThreadPool::getLock () const
{
	return listSection;
}

void TaskThreadPool::addHandlerToPool (TaskHandler* handler)
{
	Job* job = createJobForHandler (handler);

    if (job == nullptr)
    {
        job = new Job (handler, *this);
    }

    // Add to the pool...
    {
        ScopedLock lock (getLock());
        pool->addJob (job, true);
        taskJobAdded (*job);
    }
	
	itemsChangedFunc.trigger();
}

void TaskThreadPool::jobFinishedInternal (TaskThreadPool::Job &taskJob)
{
	ScopedLock lock (getLock());
    taskJobFinished (taskJob);
    itemsChangedFunc.trigger ();
}

void TaskThreadPool::itemsChanged ()
{
	listeners.call (&Listener::pooledTasksChanged, *this);
}

///////////////////////////////////////////////////////////////////////////////
