#ifndef TASKTHREADWITHPROGRESSWINDOW_H_INCLUDED
#define TASKTHREADWITHPROGRESSWINDOW_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////

class TaskThreadWithProgressWindow	:	public juce::ThreadWithProgressWindow,
                                        public ProgressiveTask::Listener
{
public:

	~TaskThreadWithProgressWindow ();

	ProgressiveTask* getCurrentTask ();
	juce::Identifier getId () const;

	void run () override;
	void threadComplete (bool userPressedCancel) override;

	juce::Result getResult () const;

	void taskStatusMessageChanged (ProgressiveTask* task) override;
	void taskProgressChanged (ProgressiveTask* task) override;

	class Listener
	{
	public:
		virtual ~Listener () {}
		virtual void progressiveTaskFinished (TaskThreadWithProgressWindow& runner, bool userCancelled) = 0;
	private:
	};

	void addListener (Listener* listener);
	void removeListener (Listener* listener);

	static juce::Result runTaskSynchronously (ProgressiveTask* task, bool owned, const juce::String& title, juce::Component* comp);
	static TaskThreadWithProgressWindow& runTask (juce::Identifier id, ProgressiveTask* task, bool owned, const juce::String& title, juce::Component* comp, Listener* listener);

private:

	TaskThreadWithProgressWindow (ProgressiveTask* taskToRun, bool owned, const juce::String& title, juce::Component* comp, juce::Identifier id);

	juce::OptionalScopedPointer< ProgressiveTask > task;
	juce::Result result;
	juce::Identifier taskId;
	juce::ListenerList< Listener > listeners;
	bool async;
};

///////////////////////////////////////////////////////////////////////////////

#endif  // TASKTHREADWITHPROGRESSWINDOW_H_INCLUDED
