#ifndef MODALTASKPOPUP_H_INCLUDED
#define MODALTASKPOPUP_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////

class ModalTaskPopup	:	public TaskHandler
{
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModalTaskPopup);
public:

	ModalTaskPopup (ProgressiveTask* task, bool owned);
	~ModalTaskPopup ();

	virtual void taskStarted () override;
	virtual bool taskMonitor (bool stillRunning) override;
	virtual void taskFinished (bool aborted) override;

	virtual void taskStatusMessageChanged (ProgressiveTask* task) override;
	virtual void taskProgressChanged (ProgressiveTask* task) override;

	static void launch (ProgressiveTask* task, bool owned, const juce::String& title,
                        int priority = 5, Callback* callback = nullptr);

private:

	juce::ScopedPointer< juce::AlertWindow > alertWindow;
	juce::String message;
	double progress;
	juce::CriticalSection messageLock;

};

///////////////////////////////////////////////////////////////////////////////

#endif  // MODALTASKPOPUP_H_INCLUDED
