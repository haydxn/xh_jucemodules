#ifndef TASKSEQUENCE_H_INCLUDED
#define TASKSEQUENCE_H_INCLUDED

class ProgressiveTask;

///////////////////////////////////////////////////////////////////////////////
/**
	Helper container for holding a sequence of tasks with associated relative
	weights. This makes it very easy to automatically calculate the proportion
	of the overall progress that each one takes up.
*/
///////////////////////////////////////////////////////////////////////////////

class TaskSequence
{
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TaskSequence);
public:

	TaskSequence ();
	~TaskSequence ();

	int size () const;
	void addTask (ProgressiveTask* taskToAdd, double weight = 1.0);
	ProgressiveTask* getTask (int index) const;
	double getTaskProportion (int taskIndex) const;
	double getWeightOfTask (int index);
	int indexOfTask (ProgressiveTask* task) const;
	void removeTask (ProgressiveTask* taskToRemove, bool deleteObject = true);
	void clear (bool deleteObjects = true);

private:

	juce::OwnedArray< ProgressiveTask > tasks;
	RelativeWeightSequence weights;

};

///////////////////////////////////////////////////////////////////////////////

#endif  // TASKSEQUENCE_H_INCLUDED
