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

    /** This is the prototype for a member function capable of being wrapped
        in a MemberFunctionTask. It needs to return a Result, and it must take
        a single ProgressiveTask& parameter. 
     */
	typedef juce::Result (TargetClass::*TaskFunction) (ProgressiveTask& functionTask);

    /** Create a new task to wrap a member function.
        
        @param taskName             The name for this task.
        @param instance             The instance of the target class to call the
                                    function from.
        @param function             A pointer to the function to be called.
     */
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

    /** Variant of the MemberFunctionTask for calling a member function which 
        takes a single extra parameter (on top of the ProgressiveTask parameter)
        as specified by the template arguments.
     */
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

    /** Variant of the MemberFunctionTask which takes two extra parameters (on
        top of the ProgressiveTask parameter) as specified by the template 
        arguments.
     */
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
