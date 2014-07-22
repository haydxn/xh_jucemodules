#ifndef SERIALTASK_H_INCLUDED
#define SERIALTASK_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////
/**
	Simple helper task for automatically running multiple tasks in series.
*/
///////////////////////////////////////////////////////////////////////////////

class SerialTask	:	public ProgressiveTask,
								private ProgressiveTask::Listener
{
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SerialTask);
public:

	class SubTaskStatus
	{
	public:

		SubTaskStatus ();
		~SubTaskStatus ();

		double getCurrentProgress () const;
		int getCurrentIndex () const;
		int getTaskCount () const;

		void taskStarted (int index, int count);
		void taskUpdated (double progress);

	private:

		double progress;
		int index;
		int count;
	};

	SerialTask (const juce::String& taskName, bool stopOnSubTaskError = true);
	virtual ~SerialTask ();

	void addTask (ProgressiveTask* taskToAdd, double weight = 1.0);
	TaskSequence& getTasks ();

	void setBaseMessage (const juce::String& message);
	juce::String getBaseMessage () const;

	const SubTaskStatus& getSubTaskStatus () const;

	bool shouldStopOnError () const;

	juce::Result performTask () override;
	juce::String formatStatusMessageFromSubTask (ProgressiveTask& subTask) override;
	void subTaskStarting (ProgressiveTask* task, int index, int count) override;

private:

	void taskStatusMessageChanged (ProgressiveTask* task) override;
	void taskProgressChanged (ProgressiveTask* task) override;


	TaskSequence subTasks;
	juce::String baseMessage;
	SubTaskStatus status;

	double currentSubTaskProgress;
	bool stopOnError;

};

///////////////////////////////////////////////////////////////////////////////

#endif  // SERIALTASK_H_INCLUDED
