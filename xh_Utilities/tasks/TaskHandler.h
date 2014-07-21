#ifndef TASKHANDLER_H_INCLUDED
#define TASKHANDLER_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////
/**
    Base providing a simple interface for building classes that can easily
    handle some customised execution of a ProgressiveTask.

    This is not essential to make use of ProgressiveTasks. It is just one
    approach you can use, but it has been designed to be customisable.
 
    The thread used to drive the task only needs to call run() on
    this object. The main TaskHandler callbacks will automatically be called
    from the message thread, making it safe to call Component functions.
 
    The Listener callbacks, however, are still called from the task thread.
 
    When a task finishes, any registered Callback objects will be notified
    (and then destroyed).
 
 */
///////////////////////////////////////////////////////////////////////////////

class TaskHandler
{
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TaskHandler);
public:

	typedef juce::WeakReference< TaskHandler > WeakRef;

	///////////////////////////////////////////////////////////////////////////

	enum TaskState
	{
		/**	The task is inactive and has not yet been started. */
		taskPending,

		/** The task is about to start. */
		taskStarting,

		/** The task is currently running. */
		taskRunning,

		/** The task has completed and is stopping. */
		taskStopping,

		/** The task has completed. */
		taskCompleted,

		/** The task was aborted. */
		taskAborted,

	};

	///////////////////////////////////////////////////////////////////////////

	TaskHandler (ProgressiveTask* taskToRun);
	virtual ~TaskHandler ();

	/** Returns the current state of this task handler. */
	TaskState getState () const;

	virtual juce::String getStateDescription () const;

	/** Returns the task this handler is responsible for running. */
	ProgressiveTask& getTask ();

	/** Returns the result of the task (if it has finished). */
	juce::Result getResult () const;

	/** Specify an identifier for this task. */
	void setId (juce::Identifier id);

	/** Returns this task's identifier. */
	juce::Identifier getId () const;

	/** This will perform the designated task, and will block until it has
		finished. Obviously you should call this from some thread other than
		the message thread! The various callbacks will be dispatched from the
		message thread automatically at the appropriate times. 
	*/
	void performTask (ProgressiveTask::Context& context);

	void flushState ();

	/** Returns true if the task has finished (even if aborted). */
	bool hasFinished () const;

	/** Returns the last received status message. */
	juce::String getStatusMessage () const;

	/** Returns the last received overall progress value. */
	double getOverallProgress () const;

	/** Returns a reference to the overall progress variable, which is useful
		for linking to a ProgressBar component. */
	double& getOverallProgressVariable ();

	///////////////////////////////////////////////////////////////////////////
	/**	
		Listener for receiving notifications during the lifetime of a task.
	*/
	///////////////////////////////////////////////////////////////////////////

	class Listener
	{
	public:
		Listener () {}
		virtual ~Listener () {}
		virtual void taskHandlerStateChanged (TaskHandler& taskHandler) = 0;

	};

	void addListener (Listener* listener);

	void removeListener (Listener* listener);

	///////////////////////////////////////////////////////////////////////////
	/**	
		Callback object for responding to the completion of a ProgressiveTask
		being run by a TaskHandler.
	*/
	///////////////////////////////////////////////////////////////////////////

	class Callback
	{
	public:
		Callback () {}
		virtual ~Callback () {}
		virtual void progressiveTaskFinished (TaskHandler& taskHandler, bool wasAborted) = 0;
	};

	///////////////////////////////////////////////////////////////////////////

	void addCallback (Callback* callbackToAdd);

protected:

	///////////////////////////////////////////////////////////////////////////

	/**	Called from the message thread just before the task execution starts. 
	*/
	virtual void taskStart ();

	/**	Called from the message thread periodically whilst the task is running.
		This function also has control over when the task finishes, as the
		calling thread will remain busy with the task until this returns false
		(even if the task has actually finished). 
		
		The default approach is to just return the stillRunning parameter, as
		this will keep the task running until it finishes. If you wish to 
		abort the task, you can simply return false while the task is still
		running.

		@param	stillRunning			This indicates whether or not the task
										is actually still running.

		@returns	whether or not the task should continue to run on the
					execution thread. The task thread will remain busy with 
					this task for as long as this returns true.
	*/
	virtual bool taskMonitor (bool stillRunning);

	/**	Called from the message thread once the task has finished (before
		any attached Callback objects have been notified).

		@param	aborted					Indicates whether or not the task
										was aborted.
	*/
	virtual void taskFinish (bool aborted);

	///////////////////////////////////////////////////////////////////////////

private:

	friend class juce::WeakReference< TaskHandler >;

	void setState (TaskState newState);
    void callCallbacks (bool aborted);
    
	class Monitor;

	juce::WeakReference< TaskHandler >::Master masterReference;
	juce::ScopedPointer< ProgressiveTask > task;
	TaskState state;
	juce::Identifier id;
	juce::ScopedPointer< Monitor > monitor;
	juce::ListenerList< Listener > listeners;
	juce::OwnedArray< Callback > callbacks;
	juce::Result result;
	juce::CriticalSection messageLock;
	juce::String statusMessage;
	double overallProgress;

};

///////////////////////////////////////////////////////////////////////////////

template <>
struct juce::ContainerDeletePolicy<TaskHandler>
{
	static void destroy (TaskHandler* object)
	{
		AsyncDestroyer< TaskHandler >::deleteOnMessageThread (object);
	}
};



///////////////////////////////////////////////////////////////////////////////

template <class BaseObjectType>
class Factory
{
public:
    
    virtual ~Factory () {}
    virtual BaseObjectType* createInstance () = 0;
    
    template <class ObjectType>
    class Type  :   public Factory< BaseObjectType >
    {
    public:
        virtual ~Type () {}
        virtual BaseObjectType* createInstance ()
        {
            return new ObjectType;
        }
    };
    
    template <class DefaultObjectType = BaseObjectType>
    class Interface
    {
    public:
        
        BaseObjectType* create ()
        {
            if (factory != nullptr)
            {
                return factory->createInstance ();
            }
            return new DefaultObjectType;
        }
        
        void setFactory (Factory* factoryToUse)
        {
            factory = factoryToUse;
        }
        
        template <class ObjectType>
        void setType ()
        {
            setFactory (new Type< ObjectType >());
        }
        
    private:
        juce::ScopedPointer< Factory > factory;
    };
};

///////////////////////////////////////////////////////////////////////////////

template <class BaseObjectType, typename ParamType1>
class Factory1Param
{
public:
    
    typedef Factory1Param< BaseObjectType, ParamType1 > FactoryBase;
    
    virtual ~Factory1Param () {}
    virtual BaseObjectType* createInstance (ParamType1) = 0;
    
    template <class ObjectType>
    class Type  :   public FactoryBase
    {
    public:
        virtual ~Type () {}
        virtual BaseObjectType* createInstance (ParamType1 p1)
        {
            return new ObjectType (p1);
        }
    };
    
    template <class DefaultObjectType = BaseObjectType>
    class Interface
    {
    public:
        
        BaseObjectType* create (ParamType1 p1)
        {
            if (factory != nullptr)
            {
                return factory->createInstance (p1);
            }
            return new DefaultObjectType (p1);
        }
        
        void setFactory (FactoryBase* factoryToUse)
        {
            factory = factoryToUse;
        }
        
        template <class ObjectType>
        void setType ()
        {
            setFactory (new Type< ObjectType >());
        }
        
    private:
        juce::ScopedPointer< FactoryBase > factory;
    };
};

///////////////////////////////////////////////////////////////////////////////

template <class BaseObjectType, typename ParamType1, typename ParamType2>
class Factory2Param
{
public:
    
    typedef Factory2Param< BaseObjectType, ParamType1, ParamType2 > FactoryBase;
    
    virtual ~Factory2Param () {}
    virtual BaseObjectType* createInstance (ParamType1, ParamType2) = 0;
    
    template <class ObjectType>
    class Type  :   public FactoryBase
    {
    public:
        virtual ~Type () {}
        virtual BaseObjectType* createInstance (ParamType1 p1, ParamType2 p2)
        {
            return new ObjectType (p1, p2);
        }
    };
    
    template <class DefaultObjectType = BaseObjectType>
    class Interface
    {
    public:
        
        BaseObjectType* create (ParamType1 p1, ParamType2 p2)
        {
            if (factory != nullptr)
            {
                return factory->createInstance (p1, p2);
            }
            return new DefaultObjectType (p1, p2);
        }
        
        void setFactory (FactoryBase* factoryToUse)
        {
            factory = factoryToUse;
        }
        
        template <class ObjectType>
        void setType ()
        {
            setFactory (new Type< ObjectType >());
        }
        
    private:
        juce::ScopedPointer< FactoryBase > factory;
    };
};


///////////////////////////////////////////////////////////////////////////////

typedef Factory1Param< TaskHandler, ProgressiveTask* > TaskHandlerFactory;
typedef TaskHandlerFactory::Interface<> TaskHandlerCreator;

///////////////////////////////////////////////////////////////////////////////

//class TaskHandlerFactory
//{
//public:
//    virtual ~TaskHandlerFactory ();
//    virtual TaskHandler* createHandlerForTask (ProgressiveTask* task) = 0;
//};

///////////////////////////////////////////////////////////////////////////////



//template <class HandlerClass>
//class DefaultTaskHandlerFactory :   public TaskHandlerFactory
//{
//    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DefaultTaskHandlerFactory);
//public:
//    DefaultTaskHandlerFactory () {}
//    virtual ~DefaultTaskHandlerFactory () {}
//
//    virtual TaskHandler* createHandlerForTask (ProgressiveTask* task) override
//    {
//        return new HandlerClass (task);
//    }
//};

///////////////////////////////////////////////////////////////////////////////

#endif  // TASKHANDLER_H_INCLUDED
