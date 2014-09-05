#ifndef PROGRESSIVETASK_H_INCLUDED
#define PROGRESSIVETASK_H_INCLUDED

class ProgressiveTask;
class TaskContext;
class TaskThreadBase;

///////////////////////////////////////////////////////////////////////////////
/**
	Base for tasks which will take long enough to need to run on a background
	thread, and provide status and progress feedback to the user.
 
    A task can run sub-tasks, and progress calculations are automatically
    carried out in such a way that allows each task to only concern itself
    with its own local normalised progress value.
*/
///////////////////////////////////////////////////////////////////////////////

class ProgressiveTask
{
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProgressiveTask);
public:

    class ExecutionScope;

	///////////////////////////////////////////////////////////////////////////

	class Callback
	{
	public:

		Callback () {}
		virtual ~Callback () {}

		virtual void taskFinishedCallback (const juce::Result& result, bool wasAborted) = 0;
	private:
	};

    ///////////////////////////////////////////////////////////////////////////

	/** Create a new task with the specified name.
     
        @param taskName             The name to use for this task.
     */
	ProgressiveTask (const juce::String& taskName);

	virtual ~ProgressiveTask ();

	/** Returns this task's name. */
	juce::String getName () const;
    
//     /** Perform the task immediately, using the provided context. */
//     juce::Result performTask (Context& context);
    
    /** Returns true if this task currently has an ExecutionScope. This is
        a volatile property, as it is governed by the context thread. If you
        need to follow up with any further inspection from another thread, you 
        should ensure that you lock the context first. */
    bool isRunning () const;

	/** This must be implemented to perform this task, and should a return a 
		Result to indicate its success or failure. Since it is possible for 
		the task to be aborted, it is important to ensure that you regularly 
		call the 'shouldAbort()' function, and return immediately if it returns
		true.
     
        @returns    a result object indicating the result of this task.
     
        @see getAbortResult
     */
	virtual juce::Result run () = 0;

	/** Set the current progress for this task. Note that this is only really
        safe to call from the context of the task itself.
     
        @param  newProgress          The current progress.
        @see    advanceProgress
     */
	void setProgress (double newProgress);

	/** Advances the progress by the given amount.
     
        @param  amount              The amount to advance the progress by.
     */
	void advanceProgress (double amount);

	/** Returns the current progress (as set by setProgress()). 
     */
	double getProgress () const;

	/** Helper to return the amount of progress left up to a target value.
        This is useful when you know you what value you want a task to
        finish at.
     
        @param  target              The target progress you are interested in.
     
        @returns    the amount of progress to advance by to reach this value.
    */
	double getDistanceToTargetProgress (double target) const;

	/** Set a status message to indicate the task's current activity.
        
        @param  message             The status message to be used.
     */
	void setStatusMessage (const juce::String& message);

	/** Returns the status message set by setStatusMessage().
     */
	juce::String getStatusMessage () const;

	/** If called, this will cause shouldAbort to return true. 
     */
	void abort ();

	/** This should be checked regularly within run(), as it will
        indicate whether or not the task should be aborted early. 
    
        @returns    true if the task should abort. If so, run should
                    return immediately.
     */
	bool shouldAbort () const;

	/** Perform the provided task immediately as a sub-task of this one, taking
        up the specified proportion of the overall progress. Note that this
        should only be called from within a task's run() function!
     
        @param  taskToPerform           The task to perform. It is up to the caller
                                        to ensure that this task is destroyed after
                                        this call (though this is easy if you're
                                        simply using a stack object).
        @param  proportionOfProgress    The proportion of the overall progress that
                                        the subtask should occupy.
     
        @returns    the result of the subtask's execution.
        
        @see    getDistanceToTargetProgress, performSubTaskSequence
     */
	juce::Result performSubTask (ProgressiveTask& taskToPerform, double proportionOfProgress);

	/** Perform the provided task immediately as a sub-task of this one, taking
        up the specified proportion of the overall progress. Note that this
        should only be called from within a task's run() function!
     
        @param  taskToPerform           The task to perform. It is up to the caller
                                        to ensure that this task is destroyed after
                                        this call (though this is easy if you're
                                        simply using a stack object).
        @param  proportionOfProgress    The proportion of the overall progress that
                                        the subtask should occupy.
        @param  index                   The index of this sub-task if it is part
                                        of a sequence.
        @param  count                   The number of sub-tasks in the sequence
                                        that this one is a part of.
     
        @returns    the result of the subtask's execution.
     
     @see    getDistanceToTargetProgress, performSubTaskSequence
     */
    juce::Result performSubTask (ProgressiveTask& taskToPerform, double proportionOfProgress, int index, int count);

	/** Perform all the tasks in the provided sequence as a subtask of this,
        with their overall progress taking up the specified proportion of this
        task's overall progress.
     
        @param  sequence                The sequence of tasks to perform.
        @param  proportionOfProgress    The proportion of the overall progress that 
                                        this sequence as a whole should occupy.
        @param  stopOnError             If true, the sequence will immediately
                                        return the result of any task that returns
                                        a fail result (skipping the rest). If false,
                                        all tasks in the sequence will be run.
     
        @returns    a value of Result::ok() if all of the tasks completed without
                    any failures. If any did fail, then the result will contain
                    a multi-line error message combining all sub-task errors. If
                    stopOnError was true, this would only ever contain one.
     
        @see    performSubTask
     */
	juce::Result performSubTaskSequence (const TaskSequence& sequence, double proportionOfProgress, bool stopOnError);

	/** Called just before a sub-task is started.
        @param  task                    The task that is starting.
        @param  index                   The index of the subtask. If this is not
                                        being performed as part of a sequence, this
                                        will be zero.
        @param  count                   The number of subtasks currently being
                                        performed. If this is not being performed
                                        as part of a sequence, this will be 1.
     */
	virtual void subTaskStarting (ProgressiveTask* task, int index, int count);

	/** The result of this function is used to set this task's status message
        a sub-task's status changes. By default, it simply uses the subtask's message,
        but you might want to prefix it with something else.
     
        @param  subTask                 The subtask whose status message has changed.
     
        @returns    the message that this task should use for its own status.
     */
	virtual juce::String formatStatusMessageFromSubTask (ProgressiveTask& subTask);

    /// This simply returns a result to give when a task is aborted, allowing:
	///   if (shouldAbort()) return getAbortResult();
	virtual juce::Result getAbortResult ();

	bool threadShouldExit () const;
    
    static const juce::Result taskAlreadyRunning;

    ///////////////////////////////////////////////////////////////////////////
    /**
        This wraps up the stuff used in the execution of a task.
     */
    ///////////////////////////////////////////////////////////////////////////
    
    class ExecutionScope
    {
    public:
        
        ~ExecutionScope ();
        
		TaskContext& getContext ();
        ProgressiveTask& getTask ();
		const ExecutionScope* getSubTaskScope () const;
		const ExecutionScope* getParentScope () const;
        
        void setProgress (double progress);
        void setStatusMessage (const juce::String& message);

		double interpolateProgress (double amount) const;

    private:

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExecutionScope);

        ExecutionScope (TaskContext& executionContext, ProgressiveTask& task, ExecutionScope* parentScope = nullptr,
						double proportionOfProgress = 1.0, int index = 0, int count = 1);

        juce::Result performSubTask (ProgressiveTask& task, double proportion, int index, int count);
        
        friend class ProgressiveTask;
		friend class TaskContext;        

        TaskContext& context;
        ProgressiveTask& task;
		ExecutionScope* parentScope;
		ExecutionScope* subTaskScope;
		juce::String statusMessage;
		double progress;
		double progressAtStart;
		double progressAtEnd;
		int index;
		int count;
    };

    ///////////////////////////////////////////////////////////////////////////

    /** Returns this task's execution scope, if it is running. Be aware that
        this is highly volatile, since it is constructed and destroyed on the
        context thread it is performed from. If you need to do anything with
        it, you should lock the context first! */
    const ExecutionScope* getScope () const;
    
private:

	friend class TaskContext;        
	
	juce::String name;
    ExecutionScope* scope;
	bool abortSignal;
	
};



///////////////////////////////////////////////////////////////////////////
/**
	Object providing a context for running a ProgressiveTask, allowing you
	to register for execution notifications (via a TaskContext::Listener), 
	and/or to attach ProgressiveTask::Callback objects to respond when it
	completes.

	NOTE: These should only ever be deleted from the message thread.
	They are reference counted, and have a deletion policy to enforce this
	so you shouldn't need to worry about this if you're using appropriate
	containers!

	It is possible to subclass this if you need to include some special
	behaviour when a task is about to start or terminate, but you should
	be able to accomplish most things using this as it is.
*/
///////////////////////////////////////////////////////////////////////////

class TaskContext	:	public juce::ReferenceCountedObject,
						private juce::AsyncUpdater
{
public:

	enum TaskState
	{
		/**	The task is inactive and has not yet been started. */
		taskPending,

		/** The task is about to start. */
		taskStarting,

		/** The task is currently running. */
		taskRunning,

		/** The task has completed and is stopping. */
		taskStopping,

		/** The task has completed. */
		taskCompleted,

		/** The task was aborted. */
		taskAborted,

	};

	typedef juce::ReferenceCountedObjectPtr< TaskContext > Ptr;
	
	TaskContext (ProgressiveTask* taskToRun);

	virtual ~TaskContext ();

    bool currentTaskShouldExit ();

	/** Add a callback to be notified when the task completes. The callback
		will be destroyed automatically once it has been dispatched. */
	void addCallback (ProgressiveTask::Callback* callback);

	/** Returns the task this context is responsible for. */
	ProgressiveTask& getTask ();

	/** Returns the result of this task's execution (if it has finished). */
	juce::Result getResult () const;

	/** Returns true if the task was aborted. */
	bool wasAborted () const;

	/** Returns the current state of the context. */
	TaskState getState () const;

	/** Returns true if the task has completed (or was aborted). */
	bool hasFinished () const;

	/** Helper to get a string describing the current state. */
	juce::String getStateDescription () const;

    ///////////////////////////////////////////////////////////////////////
    /**
        Listener class for receiving notifications from the active task
        currently running on the target Task::Context. Be aware
        that many of these callbacks all occur on the context thread, and 
		so you should take suitable care in communicating with the message 
		thread. Of course, this also means that it is safe to inspect a 
		task's ExecutionScope from within them.
    */
    ///////////////////////////////////////////////////////////////////////

    class Listener
    {
    public:
        virtual ~Listener () {}

		/**
			Called when a task context's execution state changes.
			Note that this is called from the context's execution thread!
		*/
		virtual void taskStateChanged (TaskContext&) {};

		/**
			Called when the task's status message changes.
			Note that this is called from the context's execution thread!
		*/
        virtual void taskStatusMessageChanged (TaskContext& task) = 0;
        
		/**
			Called whenever the task's progress value changes.
			Note that this is called from the context's execution thread!
		*/
        virtual void taskProgressChanged (TaskContext& task) = 0;

		/**
			This is called (from the message thread) just before any
			registered Task::Callback objects are dispatched, after the
			task has finished.
		*/
		virtual void aboutToDispatchTaskFinishedCallbacks (TaskContext&) {};

		/**
			This is called (from the message thread) after all of the
			registered Task::Callback objects have been dispatched.
		*/
		virtual void taskFinishedCallbacksDispatched (TaskContext&) {};
    };

    ///////////////////////////////////////////////////////////////////////

	virtual void taskAboutToStart ();

	virtual void taskAboutToTerminate ();

    /// Register a listener for tasks on this context.
    void addListener (Listener* listener);
    
    /// Remove a listener from this context.
    void removeListener (Listener* listener);

    /** Returns this context's lock, allowing you to ensure that the active
        task's ExecutionScope hierarchy does not change whilst you inspect
        it. */
    const juce::CriticalSection& getLock () const { return runtimeLock; }

	void flush ();

private:

	void handleAsyncUpdate () override;

	//void flushCallbacks (bool aborted);
	void setState (TaskState state);

	juce::Result runTask (TaskThreadBase& threadBase);

    friend class ProgressiveTask::ExecutionScope;
	friend class TaskThreadBase;
	class ScopedRunTime;

	juce::CriticalSection runtimeLock;
	TaskThreadBase* taskThread;
	juce::Result result;
	juce::ScopedPointer<ProgressiveTask> activeTask;
	juce::OwnedArray<ProgressiveTask::Callback> callbacks;
    juce::ListenerList<Listener> listeners;
	TaskState currentState;

};

DECLARE_MESSAGETHREAD_DELETE_POLICY(TaskContext);

///////////////////////////////////////////////////////////////////////////////
/**
	Base interface for threading classes intended to drive a TaskContext.
*/
///////////////////////////////////////////////////////////////////////////////

class TaskThreadBase
{
public:

	TaskThreadBase () {}

	virtual ~TaskThreadBase () {}

	/** This function basically causes everything to happen, executing the
		task held by the context, updating its state, and dispatching any
		notifications at the appropriate time. Obviously this call will block
		on the thread it is called from, and so it should only be called from
		the thread that this object represents. */
	void runTask (TaskContext::Ptr taskContext);

	/** This must return true if the current task needs to exit. */
	virtual bool currentTaskShouldExit () = 0;

	/** This must return true only when called from the thread this object 
		is representing. */
	virtual bool isCurrentTaskThread () = 0;

private:

};

///////////////////////////////////////////////////////////////////////////////
/**
	A simple base class for objects intended to act as a user-facing interface
	to a running task. This doesn't do very much, but it is useful as it has
	an asynchrounous refresh() callback called from the message thread when
	a task context's state changes, or a new context is assigned, or if the
	triggerRefresh() function is called directly.
*/
///////////////////////////////////////////////////////////////////////////////
class TaskInterface	:	public TaskContext::Listener
{
public:

	TaskInterface ();
	~TaskInterface ();

	void setTaskContext (TaskContext* taskToView, bool alsoTriggerRefresh = true);
	TaskContext* getTaskContext ();

	/** Called when a new TaskContext is assigned to this object. */
	virtual void taskContextChanged ();

	/** Called from the message thread, providing a place to safely update
		the state of this object based on the current state of the context.
		This is called automatically when the context's state changes, or
		in response to triggerRefresh(). */
	virtual void refresh (TaskContext& taskHandler);

	/** Called from the message thread just before a task's callbacks
		are dispatched. */
	virtual void aboutToDispatchTaskFinishedCallbacks (TaskContext&) override {};

	/** Called from the message thread just after a task's callbacks
		are dispatched. */
	virtual void taskFinishedCallbacksDispatched (TaskContext&) override {};

	/** Called from the *task* thread when the context's state changes. */
	virtual void taskStateChanged (TaskContext& taskHandler) override;
	/** Called from the *task* thread when the status message changes. */
	virtual void taskStatusMessageChanged (TaskContext& ) override {};
	/** Called from the *task* thread when the progress changes. */
	virtual void taskProgressChanged (TaskContext& ) override {};

	/** Triggers an asynchronous call to refresh(). */
	void triggerRefresh ();

private:

	void refreshInternal ();

	class AsyncRefresh;

	TaskContext::Ptr task;
	juce::ScopedPointer<AsyncRefresh> refreshCallback;

};

///////////////////////////////////////////////////////////////////////////////

#endif  // PROGRESSIVETASK_H_INCLUDED
