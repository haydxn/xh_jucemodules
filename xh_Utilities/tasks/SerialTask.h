#ifndef SERIALTASK_H_INCLUDED
#define SERIALTASK_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////
/**
	Simple helper task for automatically running multiple tasks in series.
*/
///////////////////////////////////////////////////////////////////////////////

class SerialTask	:	public ProgressiveTask
{
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SerialTask);
public:

	SerialTask (const juce::String& taskName, bool stopOnSubTaskError = true);
	virtual ~SerialTask ();

	void addTask (ProgressiveTask* taskToAdd, double weight = 1.0);
	TaskSequence& getTasks ();

	void setBaseMessage (const juce::String& message);
	juce::String getBaseMessage () const;

	bool shouldStopOnError () const;

	juce::Result run () override;
	juce::String formatStatusMessageFromSubTask (ProgressiveTask& subTask) override;
	void subTaskStarting (ProgressiveTask* task, int index, int count) override;

private:

	TaskSequence subTasks;
	juce::String baseMessage;

	double currentSubTaskProgress;
	bool stopOnError;

};

///////////////////////////////////////////////////////////////////////////////

#endif  // SERIALTASK_H_INCLUDED
