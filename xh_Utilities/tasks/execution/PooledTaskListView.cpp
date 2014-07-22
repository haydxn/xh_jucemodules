
///////////////////////////////////////////////////////////////////////////////

TaskHandlerViewComponent::TaskHandlerViewComponent (TaskHandler* taskToView)
{
	setTask (taskToView, false);
}

TaskHandlerViewComponent::~TaskHandlerViewComponent ()
{
	setTask (nullptr, false);
}

void TaskHandlerViewComponent::setTask (TaskHandler* taskToView, bool triggerRefresh)
{
	if (taskToView != task)
	{
		if (task != nullptr)
		{
			task->removeListener (this);
		}

		task = taskToView;

		if (taskToView != nullptr)
		{
			taskToView->addListener (this);
		}

		if (triggerRefresh)
		{
			taskChanged ();
			refresh ();
		}
	}
}

TaskHandler* TaskHandlerViewComponent::getTask ()
{
	return task.get();
}

void TaskHandlerViewComponent::refresh ()
{
	repaint ();
}

void TaskHandlerViewComponent::taskChanged ()
{
}

void TaskHandlerViewComponent::paint (Graphics& g)
{
	TaskHandler* handler = getTask ();
	if (handler != nullptr)
	{



		String name = handler->getTask().getName();
		String msg = handler->getStatusMessage();
		
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
		g.drawFittedText(handler->getStateDescription(), getLocalBounds().withTrimmedTop(halfHeight).reduced(4), Justification::centredLeft, 1);

		switch (handler->getState())
		{
		case TaskHandler::taskPending:
			{
				g.setColour (Colours::blue.withAlpha(0.2f));
				g.fillRect (area.reduced(2));
			}
			break;
		case TaskHandler::taskRunning:
			{
				double prog = handler->getOverallProgress();
				g.setColour (Colours::hotpink.withAlpha(0.5f));
				g.fillRect (area.reduced(2).withTrimmedRight (roundDoubleToInt (area.getWidth() * (1 - prog))));
			}
			break;

		case TaskHandler::taskCompleted:
			{
				g.setColour (Colours::green.withAlpha(0.5f));
				g.fillRect (area.reduced(2));
			}
			break;

		case TaskHandler::taskAborted:
			{
				g.setColour (Colours::red.withAlpha(0.5f));
				g.fillRect (area.reduced(2));
			}
			break;

		case TaskHandler::taskStarting:
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

void TaskHandlerViewComponent::taskHandlerStateChanged (TaskHandler&)
{
	refresh ();
}




///////////////////////////////////////////////////////////////////////////////

TaskHandlerListBoxModel::ItemComponent::ItemComponent (TaskHandler* handler)
	:	TaskHandlerViewComponent (handler)
{
	addAndMakeVisible(&nameLabel);

	setInterceptsMouseClicks(false,false);
}

TaskHandlerListBoxModel::ItemComponent::~ItemComponent ()
{

}

void TaskHandlerListBoxModel::ItemComponent::setProgressBarVisible (bool shouldBeVisible)
{
	bool isBarVisible = progressBar != nullptr;
	if (shouldBeVisible != isBarVisible)
	{
		if (shouldBeVisible)
		{
			jassert (getTask() != nullptr);

			stateLabel = nullptr;
			progressBar = new ProgressBar (getTask()->getOverallProgressVariable());
			addAndMakeVisible(progressBar);
		}
		else
		{
			progressBar = nullptr;
		}
		resized ();
	}
}

void TaskHandlerListBoxModel::ItemComponent::setStateMessage (const String& message)
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

void TaskHandlerListBoxModel::ItemComponent::taskChanged ()
{
	setProgressBarVisible (false);
}

void TaskHandlerListBoxModel::ItemComponent::refresh ()
{
	TaskHandler* task = getTask ();
	if (task)
	{
		nameLabel.setText (task->getTask().getName(), dontSendNotification);

		TaskHandler::TaskState state = task->getState();

		if (state == TaskHandler::taskRunning)
		{
			setProgressBarVisible(true);
		}
		else
		{
			setStateMessage(task->getStateDescription());
		}

		if (stateLabel != nullptr)
		{
			stateLabel->toFront (false);
		}
		resized ();
		repaint ();
	}
}

void TaskHandlerListBoxModel::ItemComponent::paint (juce::Graphics& g)
{
	Colour colour (Colours::lightgrey);

	if (getTask() != nullptr)
	{
		if (getTask()->getState() == TaskHandler::taskCompleted)
		{
			if (getTask()->getResult().wasOk())
				colour = Colours::lightgreen;
			else colour = Colours::red.brighter();
		}
	}

	g.setColour (colour);
	g.fillRect(getLocalBounds());
	g.setColour (colour.darker(0.05f));
	g.drawHorizontalLine(getHeight()-1,0.0f,(float)getWidth());
}

void TaskHandlerListBoxModel::ItemComponent::resized ()
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

///////////////////////////////////////////////////////////////////////////////

TaskHandlerListBoxModel::TaskHandlerListBoxModel ()
{

}

TaskHandlerListBoxModel::~TaskHandlerListBoxModel ()
{

}

TaskHandlerViewComponent* TaskHandlerListBoxModel::createViewForTaskHandler (TaskHandler* handler)
{
	return new ItemComponent (handler);
}

void TaskHandlerListBoxModel::paintListBoxItem (int, juce::Graphics&, int, int, bool)
{
}

Component* TaskHandlerListBoxModel::refreshComponentForRow (int rowNumber, bool , Component* existingComponentToUpdate)
{
	ScopedPointer<Component> comp (existingComponentToUpdate);

	if (rowNumber < getNumRows())
	{
		if (comp != nullptr)
		{
			TaskHandlerViewComponent* viewComp = dynamic_cast< TaskHandlerViewComponent* >(existingComponentToUpdate);
			if (viewComp != nullptr)
			{
				comp.release ();
				viewComp->setTask (getTaskForRow (rowNumber));
				return viewComp;
			}
		}

		TaskHandlerViewComponent* viewComp = createViewForTaskHandler (getTaskForRow (rowNumber));
		if (viewComp != nullptr)
		{
			viewComp->refresh ();
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

void PooledTaskListView::getAllTaskHandlers (TaskHandlerArray& tasks)
{
	ScopedLock lock (taskRunner.getLock());

	int n = 0;
	
	n = taskRunner.getNumTasks();
	for (int i = 0; i < n; i++)
	{
		tasks.add (taskRunner.getTaskHandler(i));
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

TaskHandler* PooledTaskListView::getTaskForRow (int rowNumber)
{
	return tasks[rowNumber];
}

void PooledTaskListView::listBoxItemDoubleClicked(int row, const MouseEvent&)
{
	TaskHandler* handler = getTaskForRow (row);
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
