
///////////////////////////////////////////////////////////////////////////////

SerialTask::SerialTask (const String& taskName, bool stopOnSubTaskError)
	:	ProgressiveTask(taskName),
		stopOnError (stopOnSubTaskError)
{

}

SerialTask::~SerialTask ()
{

}

TaskSequence& SerialTask::getTasks ()
{
	return subTasks;
}

void SerialTask::addTask (ProgressiveTask* taskToAdd, double weight)
{
	getTasks().addTask (taskToAdd, weight);
}

bool SerialTask::shouldStopOnError () const
{
	return stopOnError;
}

void SerialTask::setBaseMessage (const String& message)
{
	baseMessage = message;
}

String SerialTask::getBaseMessage () const
{
	return baseMessage;
}

String SerialTask::formatStatusMessageFromSubTask (ProgressiveTask& subTask)
{
	if (baseMessage.isNotEmpty())
	{
		return baseMessage + newLine + subTask.getStatusMessage();
	}
	else return ProgressiveTask::formatStatusMessageFromSubTask (subTask);
}

void SerialTask::subTaskStarting (ProgressiveTask*, int index, int count)
{
}

Result SerialTask::run ()
{
	return performSubTaskSequence (subTasks, 1.0, stopOnError);
}

///////////////////////////////////////////////////////////////////////////////
