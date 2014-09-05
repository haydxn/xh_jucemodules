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

	TaskContext& addTask (ProgressiveTask* taskToRun, juce::Identifier id = juce::Identifier::null);
	void addTask (TaskContext* context);

    bool removeAllTasks (bool interruptRunningTasks, int timeOutMilliseconds);
	bool removeAllTasksWithId (juce::Identifier id, bool interruptRunningTasks, int timeOutMilliseconds);

	int getNumTasks () const;
	TaskContext* getTaskContext (int index) const;
    
    ///////////////////////////////////////////////////////

    class Job	:	public juce::ThreadPoolJob,
                    public TaskThreadBase
    {
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Job);
    public:
        
        Job (TaskContext* handler, TaskThreadPool& owner);
        virtual ~Job ();
        
        TaskThreadPool& getOwner ();
        
		TaskContext* getTaskContext  ();
        
        juce::Identifier getId () const;
        
        virtual juce::ThreadPoolJob::JobStatus runJob () override;
        
        virtual bool currentTaskShouldExit () override;
		virtual bool isCurrentTaskThread () override;

    private:
        
        TaskContext::Ptr taskContext;
        TaskThreadPool& owner;
        
    };

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
    
	virtual TaskContext* createContextForTask (ProgressiveTask* task);
    
    virtual void taskJobAdded (Job& ) {};

    virtual void taskJobFinished (Job& ) {};
    
private:
    
    class IdSelector;
	
	void addContextToPool (TaskContext* task);
	Job* createJobForContext (TaskContext* context);
    void jobFinishedInternal (Job& taskJob);

	void itemsChanged ();

	typedef AsyncCallback< TaskThreadPool > AsyncFunc;
	class CompleteCallback;

	AsyncFunc itemsChangedFunc;
	juce::ScopedPointer< juce::ThreadPool > pool;
   
    juce::ListenerList< Listener > listeners;
	juce::CriticalSection listSection;
	int maxConcurrentTaskLimit;
	
};

///////////////////////////////////////////////////////////////////////////////

#endif//POOLEDTASKRUNNER_H_INCLUDED
