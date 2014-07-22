#ifndef DUMMYTASK_H_INCLUDED
#define DUMMYTASK_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////
/**
	A dummy ProgressiveTask that simply waits for a specified amount of time.
*/
///////////////////////////////////////////////////////////////////////////////

class DummyTask	:	public ProgressiveTask
{
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DummyTask);
public:
    
    /** Create a new dummy task.
     
        @param taskName             The name for this task.
        @param durationInSeconds    The time this task should take to complete.
     */
	DummyTask (const juce::String& taskName, double durationInSeconds);
    
	virtual ~DummyTask ();

	virtual juce::Result performTask ();
    
private:
	
    juce::RelativeTime duration;
	juce::RelativeTime elapsed;
};

///////////////////////////////////////////////////////////////////////////////

#endif  // DUMMYTASK_H_INCLUDED
