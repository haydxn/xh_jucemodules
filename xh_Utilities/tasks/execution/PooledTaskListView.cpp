

///////////////////////////////////////////////////////////////////////////////

TaskContextViewComponent::TaskContextViewComponent ()
{
}

TaskContextViewComponent::~TaskContextViewComponent ()
{
}

void TaskContextViewComponent::refresh (TaskContext&)
{
	repaint ();
}

void TaskContextViewComponent::paint (Graphics& g)
{
	TaskContext* handler = getTaskContext ();
	if (handler != nullptr)
	{

		String name = handler->getTask().getName();
		String msg = handler->getTask().getStatusMessage();

		// 		if (rowIsSelected)
		// 		{
		// 			g.setColour (Colours::darkgrey.withAlpha(0.3f));
		// 			g.fillAll ();
		// 		}

		g.setColour (Colours::black);
		Rectangle<int> area (0,0,getWidth(),getHeight());
		area = area.reduced (4);

		int halfHeight = getHeight()/2;
		g.drawFittedText(handler->getTask().getName(), getLocalBounds().withTrimmedBottom(halfHeight).reduced(4), Justification::centredLeft, 1);
		//g.drawFittedText(handler->getStateDescription(), getLocalBounds().withTrimmedTop(halfHeight).reduced(4), Justification::centredLeft, 1);

		switch (handler->getState())
		{
		case TaskContext::taskPending:
			{
				g.setColour (Colours::blue.withAlpha(0.2f));
				g.fillRect (area.reduced(2));
			}
			break;
		case TaskContext::taskRunning:
			{
				double prog = handler->getTask().getProgress();//getOverallProgress();
				g.setColour (Colours::hotpink.withAlpha(0.5f));
				g.fillRect (area.reduced(2).withTrimmedRight (roundDoubleToInt (area.getWidth() * (1 - prog))));
			}
			break;

		case TaskContext::taskCompleted:
			{
				g.setColour (Colours::green.withAlpha(0.5f));
				g.fillRect (area.reduced(2));
			}
			break;

		case TaskContext::taskAborted:
			{
				g.setColour (Colours::red.withAlpha(0.5f));
				g.fillRect (area.reduced(2));
			}
			break;

		case TaskContext::taskStarting:
			{
				g.setColour (Colours::brown.withAlpha(0.5f));
				g.fillRect (area.reduced(2));
			}
			break;

		default:;
			break;
		}



	}
}

///////////////////////////////////////////////////////////////////////////////

TaskContextListBoxModel::ItemComponent::ItemComponent (TaskContext* handler)
{
	setTaskContext(handler);
	addAndMakeVisible(&nameLabel);

	setInterceptsMouseClicks(false,false);
}

TaskContextListBoxModel::ItemComponent::~ItemComponent ()
{

}

void TaskContextListBoxModel::ItemComponent::setProgressBarVisible (bool shouldBeVisible)
{
	bool isBarVisible = progressBar != nullptr;
	if (shouldBeVisible != isBarVisible)
	{
		if (shouldBeVisible)
		{
			jassert (getTaskContext() != nullptr);

			stateLabel = nullptr;
			progressBar = new ProgressBar (progress);
			addAndMakeVisible(progressBar);
		}
		else
		{
			progressBar = nullptr;
		}
		resized ();
	}
}

void TaskContextListBoxModel::ItemComponent::setStateMessage (const String& message)
{
	setProgressBarVisible (false);

	if (stateLabel == nullptr)
	{
		stateLabel = new Label ();
		addAndMakeVisible(stateLabel);
	}
	stateLabel->setText (message, dontSendNotification);
	resized ();
}

void TaskContextListBoxModel::ItemComponent::taskContextChanged ()
{
	setProgressBarVisible (false);
}

void TaskContextListBoxModel::ItemComponent::refresh (TaskContext& context)
{
	nameLabel.setText (context.getTask().getName(), dontSendNotification);

	TaskContext::TaskState state = context.getState();

	if (state == TaskContext::taskRunning)
	{
		setProgressBarVisible(true);
	}
	else
	{
		setStateMessage (context.getStateDescription());
	}

	if (stateLabel != nullptr)
	{
		stateLabel->toFront (false);
	}
	resized ();
	repaint ();
}

void TaskContextListBoxModel::ItemComponent::paint (juce::Graphics& g)
{
	Colour colour (Colours::lightgrey);

	if (getTaskContext() != nullptr)
	{
		if (getTaskContext()->getState() == TaskContext::taskCompleted)
		{
			if (getTaskContext()->getResult().wasOk())
				colour = Colours::lightgreen;
			else colour = Colours::red.brighter();
		}
	}

	g.setColour (colour);
	g.fillRect(getLocalBounds());
	g.setColour (colour.darker(0.05f));
	g.drawHorizontalLine(getHeight()-1,0.0f,(float)getWidth());
}

void TaskContextListBoxModel::ItemComponent::resized ()
{
	nameLabel.setBounds (getLocalBounds().withTrimmedBottom(2*getHeight()/3));


	Rectangle<int> infoArea = getLocalBounds().withTrimmedTop(getHeight()/3).reduced(4);
	if (progressBar != nullptr)
	{
		progressBar->setBounds (infoArea);
	}
	if (stateLabel != nullptr)
	{
		stateLabel->setBounds (infoArea);
	}
}

void TaskContextListBoxModel::ItemComponent::taskProgressChanged (TaskContext& context)
{
	progress = context.getTask().getProgress();
}

///////////////////////////////////////////////////////////////////////////////

TaskContextListBoxModel::TaskContextListBoxModel ()
{

}

TaskContextListBoxModel::~TaskContextListBoxModel ()
{

}

TaskContextViewComponent* TaskContextListBoxModel::createViewForTaskContext (TaskContext* handler)
{
	return new ItemComponent (handler);
}

void TaskContextListBoxModel::paintListBoxItem (int, juce::Graphics&, int, int, bool)
{
}

Component* TaskContextListBoxModel::refreshComponentForRow (int rowNumber, bool , Component* existingComponentToUpdate)
{
	ScopedPointer<Component> comp (existingComponentToUpdate);

	if (rowNumber < getNumRows())
	{
		if (comp != nullptr)
		{
			TaskContextViewComponent* viewComp = dynamic_cast< TaskContextViewComponent* >(existingComponentToUpdate);
			if (viewComp != nullptr)
			{
				comp.release ();
				viewComp->setTaskContext (getTaskForRow (rowNumber));
				return viewComp;
			}
		}

		TaskContextViewComponent* viewComp = createViewForTaskContext (getTaskForRow (rowNumber));
		if (viewComp != nullptr)
		{
			viewComp->triggerRefresh ();
			return viewComp;
		}
	}

	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

PooledTaskListView::PooledTaskListView (TaskThreadPool& target)
:	taskRunner(target)
{
	taskRunner.addListener(this);
	listBox = new ListBox ("List",this);
	listBox->setRowHeight(35);
	addAndMakeVisible(listBox);
	listBox->updateContent();
}

PooledTaskListView::~PooledTaskListView ()
{
	taskRunner.removeListener(this);
}

void PooledTaskListView::resized ()
{
	listBox->setBoundsInset (BorderSize<int>(0));
}

void PooledTaskListView::refresh ()
{
	tasks.clear();
	getAllTaskHandlers(tasks);
	listBox->updateContent();
	repaint();
}

void PooledTaskListView::getAllTaskHandlers (TaskContextArray& tasks)
{
	ScopedLock lock (taskRunner.getLock());

	int n = 0;
	
	n = taskRunner.getNumTasks();
	for (int i = 0; i < n; i++)
	{
		tasks.add (taskRunner.getTaskContext(i));
	}
//	n = taskRunner.getNumUnfinishedTasks ();
//	for (int i = 0; i < n; i++)
//	{
//		tasks.add (taskRunner.getUnfinishedTask(i));
//	}
}

int PooledTaskListView::getNumRows ()
{
	return tasks.size();
}

TaskContext* PooledTaskListView::getTaskForRow (int rowNumber)
{
	return tasks[rowNumber];
}

void PooledTaskListView::listBoxItemDoubleClicked(int row, const MouseEvent&)
{
	TaskContext* handler = getTaskForRow (row);
	if (handler != nullptr)
	{
		handler->getTask().abort();
	}
}

void PooledTaskListView::pooledTasksChanged (TaskThreadPool&)
{
	refresh();
}

///////////////////////////////////////////////////////////////////////////////
