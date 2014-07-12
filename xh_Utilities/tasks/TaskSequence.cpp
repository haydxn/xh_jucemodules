
///////////////////////////////////////////////////////////////////////////////

TaskSequence::TaskSequence ()
{

}

TaskSequence::~TaskSequence ()
{

}

int TaskSequence::size () const
{
	return tasks.size ();
}

void TaskSequence::addTask (ProgressiveTask* taskToAdd, double weight)
{
	tasks.add (taskToAdd);
	weights.add (weight);
}

void TaskSequence::removeTask (ProgressiveTask* taskToRemove, bool deleteObject)
{
	int index = indexOfTask (taskToRemove);
	if (index > -1)
	{
		tasks.remove (index, deleteObject);
		weights.remove (index);
	}
}

void TaskSequence::clear (bool deleteObjects)
{
	tasks.clear (deleteObjects);
	weights.clear ();
}

int TaskSequence::indexOfTask (ProgressiveTask* task) const
{
	return tasks.indexOf (task);
}

ProgressiveTask* TaskSequence::getTask (int index) const
{
	return tasks[index];
}

double TaskSequence::getTaskProportion (int taskIndex) const
{
	return weights.getNormalised (taskIndex);
}

double TaskSequence::getWeightOfTask (int index)
{
	return weights [index];
}

///////////////////////////////////////////////////////////////////////////////
