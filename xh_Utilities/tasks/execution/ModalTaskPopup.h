#ifndef MODALTASKPOPUP_H_INCLUDED
#define MODALTASKPOPUP_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////

class ModalTaskPopup	:	public TaskHandler,
	public juce::Timer
{
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModalTaskPopup);
public:

	ModalTaskPopup (ProgressiveTask* task);
	~ModalTaskPopup ();

	virtual void taskStart () override;
	virtual bool taskMonitor (bool stillRunning) override;
	virtual void taskFinish (bool aborted) override;

	static void launch (ProgressiveTask* task, const juce::String& title,
                        int priority = 5, Callback* callback = nullptr);

	virtual void timerCallback () override;

private:

	juce::ScopedPointer< juce::AlertWindow > alertWindow;
 	double progress;

};

///////////////////////////////////////////////////////////////////////////////

#endif  // MODALTASKPOPUP_H_INCLUDED
