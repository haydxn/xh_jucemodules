
///////////////////////////////////////////////////////////////////////////////

DummyTask::DummyTask (const String& taskName, double durationInSeconds)
	:	ProgressiveTask (taskName),
		duration (RelativeTime::seconds(durationInSeconds))
{
}

DummyTask::~DummyTask ()
{
}

Result DummyTask::performTask ()
{
	if (duration.inSeconds() < 0.0)
		return Result::ok ();

	Time startTime = Time::getCurrentTime();
    
    setStatusMessage(getName());

	while (elapsed < duration)
	{
		elapsed = Time::getCurrentTime () - startTime;

		if (shouldAbort())
			return Result::ok();

		double progress = 1.0;
		if (duration.inSeconds() > 0)
			progress = elapsed.inSeconds() / duration.inSeconds();
		setProgress(progress);

		Thread::sleep (100);
	}

	return Result::ok ();
}

///////////////////////////////////////////////////////////////////////////////
