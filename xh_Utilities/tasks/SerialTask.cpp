
///////////////////////////////////////////////////////////////////////////////

SerialTask::SubTaskStatus::SubTaskStatus ()
	:	progress (0.0),
		index (0),
		count (0)
{
}

SerialTask::SubTaskStatus::~SubTaskStatus ()
{

}

double SerialTask::SubTaskStatus::getCurrentProgress () const
{
	return progress;
}

int SerialTask::SubTaskStatus::getCurrentIndex () const
{
	return index;
}

int SerialTask::SubTaskStatus::getTaskCount () const
{
	return count;
}

void SerialTask::SubTaskStatus::taskStarted (int index_, int count_)
{
	index = index_;
	count = count_;
	progress = 0.0;
}

void SerialTask::SubTaskStatus::taskUpdated (double progress_)
{
	progress = progress_;
}

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

const SerialTask::SubTaskStatus& SerialTask::getSubTaskStatus () const
{
	return status;
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
	status.taskStarted (index, count);
}

void SerialTask::taskStatusMessageChanged (ProgressiveTask*)
{
}

void SerialTask::taskProgressChanged (ProgressiveTask* task)
{
	status.taskUpdated (task->getProgress());
}

Result SerialTask::performTask ()
{
	return performSubTaskSequence (subTasks, 1.0, stopOnError, this);
}

///////////////////////////////////////////////////////////////////////////////
