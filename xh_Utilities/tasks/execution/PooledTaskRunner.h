#ifndef POOLEDTASKRUNNER_H_INCLUDED
#define POOLEDTASKRUNNER_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////
/**
*/
///////////////////////////////////////////////////////////////////////////////

class TaskThreadPool
{
public:

	///////////////////////////////////////////////////////////////////////////

	TaskThreadPool (int maxConcurrentTasks = 1);
	virtual ~TaskThreadPool ();

	TaskHandler& addTask (ProgressiveTask* taskToRun, juce::Identifier id = juce::Identifier::null);
	void addTask (TaskHandler* handler);

    bool removeAllTasks (bool interruptRunningTasks, int timeOutMilliseconds);
	bool removeAllTasksWithId (juce::Identifier id, bool interruptRunningTasks, int timeOutMilliseconds);

	int getNumTasks () const;
	TaskHandler* getTaskHandler (int index) const;
    
    ///////////////////////////////////////////////////////

    class Job	:	public juce::ThreadPoolJob,
                    public ProgressiveTask::Context
    {
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Job);
    public:
        
        Job (TaskHandler* handler, TaskThreadPool& owner);
        virtual ~Job ();
        
        TaskThreadPool& getOwner ();
        
        TaskHandler* getTaskHandler ();
        TaskHandler* releaseTaskHandler ();
        
        juce::Identifier getId () const;
        
        virtual juce::ThreadPoolJob::JobStatus runJob () override;
        
        virtual bool currentTaskShouldExit () override;

    private:
        
        juce::ScopedPointer< TaskHandler > taskHandler;
        TaskThreadPool& owner;
        
    };

    typedef Factory2Param< Job, TaskHandler*, TaskThreadPool& > JobFactory;
    typedef JobFactory::Interface<> JobCreator;

    ///////////////////////////////////////////////////////

    class Listener
	{
	public:
		virtual ~Listener () {};
		virtual void pooledTasksChanged (TaskThreadPool& source) = 0;
	};

    ///////////////////////////////////////////////////////

	void addListener (Listener* listener);
	void removeListener (Listener* listener);

    ///////////////////////////////////////////////////////
    
    class Tracker
    {
    public:
        
        virtual ~Tracker () {}
        
        virtual void taskQueueJobAdded (Job& taskJob) = 0;
        virtual void taskQueueJobFinished (Job& taskJob) = 0;

    private:
        
    };

    ///////////////////////////////////////////////////////

	const juce::CriticalSection& getLock () const;
    
    virtual TaskHandler* createHandlerForTask (ProgressiveTask* task);
    
    virtual Job* createJobForHandler (TaskHandler* handler);
    
    virtual void taskJobAdded (Job& ) {};

    virtual void taskJobFinished (Job& ) {};

    TaskHandlerCreator& getHandlerCreator () { return handlerCreator; }

    JobCreator& getJobCreator () { return jobCreator; }
    
private:
    
    class IdSelector;
	
	void addHandlerToPool (TaskHandler* task);
    void jobFinishedInternal (Job& taskJob);

	void itemsChanged ();

	typedef AsyncCallback< TaskThreadPool > AsyncFunc;
	class CompleteCallback;

	AsyncFunc itemsChangedFunc;
	juce::ScopedPointer< juce::ThreadPool > pool;
    
    TaskHandlerCreator handlerCreator;
    JobCreator jobCreator;
    
    juce::ListenerList< Listener > listeners;
	juce::CriticalSection listSection;
	int maxConcurrentTaskLimit;
	
};

///////////////////////////////////////////////////////////////////////////////

#endif//POOLEDTASKRUNNER_H_INCLUDED
