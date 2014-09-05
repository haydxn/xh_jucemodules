#ifndef POOLEDTASKLISTVIEW_H_INCLUDED
#define POOLEDTASKLISTVIEW_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////

class TaskContextViewComponent	:	public juce::Component,
									public TaskInterface
{
public:

	TaskContextViewComponent ();
	~TaskContextViewComponent ();

	virtual void refresh (TaskContext& context) override;
	virtual void paint (juce::Graphics& g) override;

};


///////////////////////////////////////////////////////////////////////////////

class TaskContextListBoxModel	:	public juce::ListBoxModel
{
public:

	typedef juce::ReferenceCountedArray< TaskContext > TaskContextArray;

	TaskContextListBoxModel ();
	virtual ~TaskContextListBoxModel ();

	virtual int getNumRows () = 0;
	virtual TaskContext* getTaskForRow (int rowNumber) = 0;

	virtual TaskContextViewComponent* createViewForTaskContext (TaskContext* handler);

	virtual void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
	virtual juce::Component* refreshComponentForRow (int rowNumber, bool isRowSelected, juce::Component* existingComponentToUpdate) override;

	///////////////////////////////////////////////////////////////////////////

	class ItemComponent	:	public TaskContextViewComponent
	{
	public:

		ItemComponent (TaskContext* handler);
		virtual ~ItemComponent ();

		virtual void refresh (TaskContext& context) override;
		virtual void taskContextChanged () override;
		virtual void paint (juce::Graphics& g) override;
		virtual void resized () override;
		virtual void taskProgressChanged (TaskContext& context) override;

	private:

		void setProgressBarVisible (bool shouldBeVisible);
		void setStateMessage (const juce::String& message);

		juce::Label nameLabel;
		juce::ScopedPointer< juce::Label > stateLabel;
		juce::ScopedPointer< juce::ProgressBar > progressBar;
		double progress;
	};

	///////////////////////////////////////////////////////////////////////////

};

///////////////////////////////////////////////////////////////////////////////
/**
*/
///////////////////////////////////////////////////////////////////////////////

class PooledTaskListView	:	public juce::Component,
								public TaskContextListBoxModel,
								public TaskThreadPool::Listener
{
public:

	PooledTaskListView (TaskThreadPool& target);
	virtual ~PooledTaskListView ();

	virtual void resized () override;

	virtual int getNumRows () override;
	virtual TaskContext* getTaskForRow (int rowNumber) override;

	virtual void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override;
	virtual void pooledTasksChanged (TaskThreadPool& source) override;

	void getAllTaskHandlers (TaskContextArray& tasks);

	void refresh ();

private:

	TaskContextArray tasks;
	TaskThreadPool& taskRunner;
	juce::ScopedPointer< juce::ListBox > listBox;
};

///////////////////////////////////////////////////////////////////////////////

#endif//POOLEDTASKLISTVIEW_H_INCLUDED
