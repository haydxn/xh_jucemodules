#ifndef PROGRESSIVETASK_H_INCLUDED
#define PROGRESSIVETASK_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////
/**
	Base for tasks which will take long enough to need to run on a background
	thread, and provide status and progress feedback to the user.
*/
///////////////////////////////////////////////////////////////////////////////

class ProgressiveTask
{
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProgressiveTask);
public:

	class Listener;

	/// Create a new task with the specified name.
	ProgressiveTask (const juce::String& taskName);

	virtual ~ProgressiveTask ();

	/// Returns this task's name.
	juce::String getName () const;

	/// Perform this task, and returns the result.
	virtual juce::Result performTask () = 0;

	/// Set the current progress for this task.
	void setProgress (double newProgress);

	/// Advances the progress by the given amount.
	void advanceProgress (double amount);

	/// Returns the current progress as set by setProgress().
	double getProgress () const;

	/// Helper to return the amount of progress left up to a target value.
	/// This is useful when you know you what value you want a task to 
	/// finish at.
	double getDistanceToTargetProgress (double target) const;

	/// Set a status message to indicate the task's current activity.
	void setStatusMessage (const juce::String& message);

	/// Returns the status message set by setStatusMessage().
	juce::String getStatusMessage () const;

	/// If called, this will cause shouldAbort to return true. 
	void abort ();

	/// This should be checked regularly within performTask(), as it will
	/// indicate whether or not the task should be aborted early.
	bool shouldAbort () const;

	/// Perform the provided task as a sub-task of this one, taking up the
	/// specified proportion of the overall progress.
	juce::Result performSubTask (ProgressiveTask& taskToPerform, double proportionOfProgress);

	/// Perform all the tasks in the provided sequence as a subtask of this one,
	/// overall taking up the specified proportion of the overall progress.
	juce::Result performSubTaskSequence (const TaskSequence& sequence, double proportionOfProgress, bool stopOnError, Listener* subTaskListener = nullptr);

	/// Called just before a sub-task is started.
	virtual void subTaskStarting (ProgressiveTask* task, int index, int count);

	/// The result of this function is used to set this task's status message when
	/// a sub-task's status changes. By default, it simply uses the subtask's message,
	/// but you might want to prefix it with something else.
	virtual juce::String formatStatusMessageFromSubTask (ProgressiveTask& subTask);

	///////////////////////////////////////////////////////////////////////////
	/**
		Listener to be notified when the status or progress of a task changes.
	*/
	///////////////////////////////////////////////////////////////////////////

	class Listener
	{
	public:
		virtual ~Listener () {}

		/// Called in response to notifyStatusChanged ()
		virtual void taskStatusMessageChanged (ProgressiveTask* task) = 0;
		/// Called in response to notifyProgressChanged ()
		virtual void taskProgressChanged (ProgressiveTask* task) = 0;
	};

	///////////////////////////////////////////////////////////////////////////

	/// Register a listener for this task.
	void addListener (Listener* listener);

	/// Remove a listener from this task.
	void removeListener (Listener* listener);

	/// This simply returns a result to give when a task is aborted, allowing:
	///   if (shouldAbort()) return getAbortResult();
	virtual juce::Result getAbortResult ();

protected:

	void notifyProgressChanged ();
	void notifyStatusChanged ();

private:

	juce::Result performSubTask (ProgressiveTask& taskToPerform, double proportionOfProgress, int index, int count);
	bool threadShouldExit () const;
	
	class ScopedSubTaskTracker;

	juce::String name;
	juce::String statusMessage;
	double progress;
	ScopedSubTaskTracker* currentSubTask;
	juce::ListenerList<Listener> listeners;
	bool abortSignal;
	
};

///////////////////////////////////////////////////////////////////////////////

#endif  // PROGRESSIVETASK_H_INCLUDED
