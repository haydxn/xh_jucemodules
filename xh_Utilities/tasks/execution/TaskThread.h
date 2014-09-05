#ifndef TASKTHREAD_H_INCLUDED
#define TASKTHREAD_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////
/**
    Simple thread which can be used to drive a TaskHandler.
*/
///////////////////////////////////////////////////////////////////////////////

class TaskThread	:	public juce::Thread,
                        public TaskThreadBase
{
public:

	TaskThread (const juce::String& runnerTitle);
	~TaskThread ();

	juce::String getTitle () const;

	void setTask (TaskContext* task, bool owned);
	TaskContext* getTaskContext ();
	ProgressiveTask* getTask () const;

	void run () override;

	void launchThread (int priority, bool destroyWhenComplete);
	juce::Result runSynchronously (int priority);
    
    virtual bool currentTaskShouldExit () override;
	virtual bool isCurrentTaskThread () override;

	static juce::Result runSynchronously (TaskContext* context, const juce::String& title, int priority = 5);
	static void launch (TaskContext* context, const juce::String& title, int priority = 5);

private:

	typedef AsyncCallback< TaskThread > AsyncFunc;
	void selfDestruct ();

	juce::ScopedPointer< AsyncFunc > selfDestructCallback;
	juce::String title;

	TaskContext::Ptr activeTask;
	
	int timeoutMsWhenCancelling;
	bool wasCancelled;

};

///////////////////////////////////////////////////////////////////////////////

#endif  // TASKTHREAD_H_INCLUDED
