#ifndef TASKHANDLER_H_INCLUDED
#define TASKHANDLER_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////
/**
    Base providing a simple interface for building classes that can easily
    handle some customised execution of a ProgressiveTask.

    This is not essential to make use of ProgressiveTasks. It is just one
    approach you can use, but it has been designed to be customisable.
 
    The thread used to drive the task only needs to call performTask() on
    this object. The main TaskHandler callbacks will automatically be called
    from the message thread, making it safe to call Component functions.
 
    The Listener callbacks, however, are still called from the task thread.
 
    When a task finishes, any registered Callback objects will be notified
    (and then destroyed).
 
 */
///////////////////////////////////////////////////////////////////////////////

class TaskHandler	:	public ProgressiveTask::Listener
{
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TaskHandler);
public:

	TaskHandler (ProgressiveTask* taskToRun, bool owned);
	~TaskHandler ();

	ProgressiveTask& getTask ();

	juce::Result getResult () const;

	void setId (juce::Identifier id);

	juce::Identifier getId () const;

	void performTask ();

	bool hasFinished () const;

	class Callback
	{
	public:
		Callback () {}
		virtual ~Callback () {}
		virtual void progressiveTaskFinished (TaskHandler& taskHandler, bool wasAborted) = 0;
	};

	void addCallback (Callback* callbackToAdd);

protected:

	virtual void taskStarted () = 0;

	virtual bool taskMonitor (bool stillRunning) = 0;

	virtual void taskFinished (bool aborted) = 0;

	virtual void taskStatusMessageChanged (ProgressiveTask* task) override = 0;

	virtual void taskProgressChanged (ProgressiveTask* task) override = 0;

private:

    void callCallbacks (bool aborted);
    
	class Monitor;

	juce::Identifier id;
	juce::OptionalScopedPointer< ProgressiveTask > task;
	juce::ScopedPointer< Monitor > monitor;
	juce::OwnedArray< Callback > callbacks;
	juce::Result result;
	bool finished;

};

///////////////////////////////////////////////////////////////////////////////

#endif  // TASKHANDLER_H_INCLUDED
