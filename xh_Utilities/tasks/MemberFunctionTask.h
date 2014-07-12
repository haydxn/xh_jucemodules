#ifndef MEMBERFUNCTIONTASK_H_INCLUDED
#define MEMBERFUNCTIONTASK_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////
/** 
    Template class for wrapping a member function as a task. This isn't at all
	necessary, but it can be very useful, depending on the nature of the task,
	particularly if the progress calculation is complicated; each function can
	work with a normalised progress range, as it is provided with a parameter
	of the subtask it represents.

	It's easiest to use with a local typedef, especially if you're going to be
	using a few functions!
	e.g.

	typedef MemberFunctionTask< CurrentLocalClass > FunctionTask;

	TaskSequence tasks;
	tasks.addTask (new FunctionTask ("Do something", this, &CurrentLocalTask::doSomething));
	tasks.addTask (new FunctionTask ("Do something else", this, &CurrentLocalTask::doSomethingElse));
	tasks.addTask (new FunctionTask ("Do something else entirely", this, &CurrentLocalTask::doSomethingElseEntirely));
	performSubTaskSequence (tasks, 0.5, true);

	// You can also make them on the stack within a performing task...
	FunctionTask task ("And then this...", this, &CurrentLocalTask::lastly);
	performSubTask (task, 0.5);
 
    // You can also have additional parameters...
    typedef FunctionTask::With1Param< File > FileTask;
    FileTask someFileTask ("Operate upon a file", this, &CurrentLocalTask::doWithFile, file);
 
    Result doWithFile (ProgressiveTask& task, File file)
    {
        return Result::ok();
    }
 
*/
///////////////////////////////////////////////////////////////////////////////

template <class TargetClass>
class MemberFunctionTask	:	public ProgressiveTask
{
public:

	typedef juce::Result (TargetClass::*TaskFunction) (ProgressiveTask& functionTask);

	MemberFunctionTask (const juce::String& taskName, TargetClass* instance, TaskFunction function)
		:   ProgressiveTask (taskName)
	{
		targetInstance = instance;
		taskFunction = function;
	}

	juce::Result performTask () override
	{
		return (targetInstance->*(taskFunction)) (*this);
	}

	///////////////////////////////////////////////////////////////////////////

	template <typename ParamType1>
	class With1Param	:   public ProgressiveTask
	{
	public:

		typedef juce::Result (TargetClass::*TaskFunction) (ProgressiveTask& functionTask, ParamType1 param1_);

		With1Param (const juce::String& taskName, TargetClass* instance, TaskFunction function, ParamType1 param1_)
			:   ProgressiveTask (taskName),
				targetInstance (instance),
				taskFunction (function),
				param1 (param1_)
		{
		}

		juce::Result performTask () override
		{
			return (targetInstance->*(taskFunction)) (*this, param1);
		}

	private:

		TargetClass* targetInstance;
		TaskFunction taskFunction;
		ParamType1 param1;

	};

	///////////////////////////////////////////////////////////////////////////

	template <typename ParamType1, typename ParamType2>
	class With2Params	:   public ProgressiveTask
	{
	public:

		typedef juce::Result (TargetClass::*TaskFunction) (ProgressiveTask& functionTask, ParamType1 param1_, ParamType2 param2_);

		With2Params (const juce::String& taskName, TargetClass* instance, TaskFunction function, ParamType1 param1_, ParamType2 param2_)
			:   ProgressiveTask (taskName),
			targetInstance (instance),
			taskFunction (function),
			param1 (param1_),
			param2 (param2_)
		{
		}

		juce::Result performTask () override
		{
			return (targetInstance->*(taskFunction)) (*this, param1, param2);
		}

	private:

		TargetClass* targetInstance;
		TaskFunction taskFunction;
		ParamType1 param1;
		ParamType2 param2;

	};

	///////////////////////////////////////////////////////////////////////////

private:

	TaskFunction taskFunction;
	TargetClass* targetInstance;


};


///////////////////////////////////////////////////////////////////////////////

#endif  // MEMBERFUNCTIONTASK_H_INCLUDED
