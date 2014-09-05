#ifndef MODALTASKPOPUP_H_INCLUDED
#define MODALTASKPOPUP_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////
/** 
	Manages a simple modal pop-up window while a TaskContext is running.
	This object will automatically destroy itself after the context has
	finished executing.
*/
///////////////////////////////////////////////////////////////////////////////
class ModalTaskPopup	:	public TaskInterface,
							private juce::Timer
{
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModalTaskPopup);
public:

	/** Create a new ModalTaskPopup for the provided task instance, using
		a default TaskContext. */
	ModalTaskPopup (ProgressiveTask* task);

	/** Create a new ModalTaskPopup for the provided TaskContext. */
	ModalTaskPopup (TaskContext* task);
	~ModalTaskPopup ();

	virtual void refresh (TaskContext& context) override;
	virtual void aboutToDispatchTaskFinishedCallbacks (TaskContext&) override;
	virtual void taskFinishedCallbacksDispatched (TaskContext&) override;

	/** Launches a TaskThread for the specified task instance (using an
		automatically created TaskContext), and creates a new ModalTaskPopup
		for it (which will be automatically destroyed when the task finishes).
	*/
	static void launch (ProgressiveTask* task, const juce::String& title,
		int priority = 5, ProgressiveTask::Callback* callback = nullptr);

	/** Launches a TaskThread for the specified task context, and creates a 
		new ModalTaskPopup for it (which will be automatically destroyed when 
		the task finishes).
	*/
	static void launch (TaskContext* task, const juce::String& title,
		int priority = 5, ProgressiveTask::Callback* callback = nullptr);

	virtual void taskStatusMessageChanged (TaskContext& ) override;
	virtual void taskProgressChanged (TaskContext& ) override;

private:

	void taskStart ();
	bool monitor (bool stillRunning);
	virtual void timerCallback () override;

	juce::ScopedPointer< juce::AlertWindow > alertWindow;
	double progress;

};

///////////////////////////////////////////////////////////////////////////////

#endif  // MODALTASKPOPUP_H_INCLUDED
