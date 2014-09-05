#ifndef TASKTHREADWITHPROGRESSWINDOW_H_INCLUDED
#define TASKTHREADWITHPROGRESSWINDOW_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////

class TaskThreadWithProgressWindow	:	public juce::ThreadWithProgressWindow,
                                        public TaskContext::Listener,
                                        public TaskThreadBase
{
public:

	~TaskThreadWithProgressWindow ();

	ProgressiveTask* getCurrentTask ();
	juce::Identifier getId () const;

	void run () override;
	void threadComplete (bool userPressedCancel) override;

	virtual bool currentTaskShouldExit () override;
    virtual bool isCurrentTaskThread () override;

	juce::Result getResult () const;

	void taskStatusMessageChanged (TaskContext& task) override;
	void taskProgressChanged (TaskContext& task) override;

// 	class LegacyListener
// 	{
// 	public:
// 		virtual ~LegacyListener () {}
// 		virtual void progressiveTaskFinished (TaskThreadWithProgressWindow& runner, bool userCancelled) = 0;
// 	private:
// 	};
// 
// 	void addLegacyListener (LegacyListener* listener);
// 	void removeLegacyListener (LegacyListener* listener);
// 
	static juce::Result runTaskSynchronously (ProgressiveTask* task, const juce::String& title, juce::Component* comp);
//	static TaskThreadWithProgressWindow& runTask (juce::Identifier id, Task* task, bool owned, const juce::String& title, juce::Component* comp);
	static TaskThreadWithProgressWindow& runTask (juce::Identifier id, ProgressiveTask* task, const juce::String& title, juce::Component* comp);

private:

	class LegacyListenerCallback;

	TaskThreadWithProgressWindow (ProgressiveTask* taskToRun, const juce::String& title, juce::Component* comp, juce::Identifier id);

	TaskContext::Ptr task;////juce::OptionalScopedPointer< Task > task;
	juce::Result result;
	juce::Identifier taskId;
// 	juce::ListenerList< LegacyListener > legacyListeners;
	bool async;
};

///////////////////////////////////////////////////////////////////////////////

#endif  // TASKTHREADWITHPROGRESSWINDOW_H_INCLUDED
