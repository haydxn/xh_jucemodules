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

	/** Create a new task with the specified name.
     
        @param taskName             The name to use for this task.
     */
	ProgressiveTask (const juce::String& taskName);

	virtual ~ProgressiveTask ();

	/** Returns this task's name. */
	juce::String getName () const;

	/** This must be implemented to perform this task, and should a Result
        to indicate its success or failure. Since it is possible for the task
        to be aborted, it is important to ensure that you regularly call the
        'shouldAbort()' function, and return immediately if it returns true.
     
        @returns    a result object indicating the result of this task.
     
        @see getAbortResult
     */
	virtual juce::Result performTask () = 0;

	/** Set the current progress for this task.
     
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

	/** This should be checked regularly within performTask(), as it will
        indicate whether or not the task should be aborted early. 
    
        @returns    true if the task should abort. If so, performTask should
                    return immediately.
     */
	bool shouldAbort () const;

	/** Perform the provided task immediately as a sub-task of this one, taking
        up the specified proportion of the overall progress.
     
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
        @param  subTaskListener         If this is not null, it will be registered
                                        with each task as it is performed, so that
                                        their status can be monitored independently
                                        of this task.
     
        @returns    a value of Result::ok() if all of the tasks completed without
                    any failures. If any did fail, then the result will contain
                    a multi-line error message combining all sub-task errors. If
                    stopOnError was true, this would only ever contain one.
     
        @see    performSubTask
     */
	juce::Result performSubTaskSequence (const TaskSequence& sequence, double proportionOfProgress, bool stopOnError, Listener* subTaskListener = nullptr);

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
