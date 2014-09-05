
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

TaskThreadPool::Job::Job (TaskContext* context, TaskThreadPool& owner_)
:	ThreadPoolJob (context != nullptr ? context->getTask().getName() : String::empty),
    taskContext (context),
    owner (owner_)
{
}

TaskThreadPool::Job::~Job ()
{
	taskContext = nullptr;
}

TaskThreadPool& TaskThreadPool::Job::getOwner ()
{
    return owner;
}

TaskContext* TaskThreadPool::Job::getTaskContext()
{
	return taskContext;
}

juce::Identifier TaskThreadPool::Job::getId () const
{
	if (taskContext != nullptr)
	{
		//return taskHandler->getId();
	}
	return Identifier::null;
}

ThreadPoolJob::JobStatus TaskThreadPool::Job::runJob ()
{
	if (taskContext != nullptr)
	{
		runTask (taskContext);
	}
    
    owner.jobFinishedInternal (*this);

	return ThreadPoolJob::jobHasFinished;
}

bool TaskThreadPool::Job::currentTaskShouldExit ()
{
    return shouldExit();
}

bool TaskThreadPool::Job::isCurrentTaskThread ()
{
	return getCurrentThreadPoolJob() == this;
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

TaskContext* TaskThreadPool::getTaskContext (int index) const
{
	ScopedLock lock(getLock());
	Job* job = dynamic_cast< Job* > (pool->getJob (index));
	if (job != nullptr)
	{
		return job->getTaskContext();
	}
	return nullptr;
}

TaskContext& TaskThreadPool::addTask (ProgressiveTask* taskToRun, Identifier id)
{
	TaskContext* context = createContextForTask (taskToRun);

	if (context == nullptr)
		context = new TaskContext (taskToRun);

    //context->setId (id);
    
	addContextToPool (context);
	return *context;
}

void TaskThreadPool::addTask (TaskContext* context)
{
    if (context != nullptr)
    {
        addContextToPool (context);
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

TaskContext* TaskThreadPool::createContextForTask (ProgressiveTask* task)
{
	return new TaskContext (task);//handlerCreator.create (task);//new TaskHandler (task);
}

TaskThreadPool::Job* TaskThreadPool::createJobForContext (TaskContext* context)
{
    return new Job (context, *this);
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

void TaskThreadPool::addContextToPool (TaskContext* context)
{
	Job* job = createJobForContext (context);

    if (job == nullptr)
    {
        job = new Job (context, *this);
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
