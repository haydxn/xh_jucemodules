#ifndef POOLEDTASKLISTVIEW_H_INCLUDED
#define POOLEDTASKLISTVIEW_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////

class TaskHandlerViewComponent	:	public juce::Component,
                                    public TaskHandler::Listener
{
public:

	TaskHandlerViewComponent (TaskHandler* taskToView = nullptr);
	~TaskHandlerViewComponent ();

	void setTask (TaskHandler* taskToView, bool triggerRefresh = true);
	TaskHandler* getTask ();

	virtual void refresh ();
	virtual void taskChanged ();

	virtual void paint (juce::Graphics& g) override;
	virtual void taskHandlerStateChanged (TaskHandler& taskHandler) override;

private:

	juce::WeakReference< TaskHandler > task;

};

///////////////////////////////////////////////////////////////////////////////

class TaskHandlerListBoxModel	:	public juce::ListBoxModel
{
public:

	typedef juce::Array< TaskHandler::WeakRef > TaskHandlerArray;

	TaskHandlerListBoxModel ();
	virtual ~TaskHandlerListBoxModel ();

	virtual int getNumRows () = 0;
	virtual TaskHandler* getTaskForRow (int rowNumber) = 0;

	virtual TaskHandlerViewComponent* createViewForTaskHandler (TaskHandler* handler);

	virtual void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
	virtual juce::Component* refreshComponentForRow (int rowNumber, bool isRowSelected, juce::Component* existingComponentToUpdate) override;

	///////////////////////////////////////////////////////////////////////////

	class ItemComponent	:	public TaskHandlerViewComponent
	{
	public:

		ItemComponent (TaskHandler* handler);
		virtual ~ItemComponent ();

		virtual void refresh () override;
		virtual void taskChanged () override;
		virtual void paint (juce::Graphics& g) override;
		virtual void resized () override;

	private:

		void setProgressBarVisible (bool shouldBeVisible);
		void setStateMessage (const juce::String& message);

		juce::Label nameLabel;
		juce::ScopedPointer< juce::Label > stateLabel;
		juce::ScopedPointer< juce::ProgressBar > progressBar;

	};

	///////////////////////////////////////////////////////////////////////////

};


///////////////////////////////////////////////////////////////////////////////
/**
*/
///////////////////////////////////////////////////////////////////////////////

//class PooledTaskListView	:	public juce::Component,
//								public TaskHandlerListBoxModel,
//								public TaskThreadPool::Listener
//{
//public:
//
//	PooledTaskListView (TaskThreadPool& target);
//	virtual ~PooledTaskListView ();
//
//	virtual void resized () override;
//
//	virtual void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override;
//	virtual void pooledTasksChanged (TaskThreadPool& source) override;
//
//	virtual void getAllTaskHandlers (TaskHandlerArray& tasks) override;
//
//	void refresh ();
//
//private:
//
//	TaskThreadPool& taskRunner;
//	juce::ScopedPointer< juce::ListBox > listBox;
//};

///////////////////////////////////////////////////////////////////////////////

#endif//POOLEDTASKLISTVIEW_H_INCLUDED
