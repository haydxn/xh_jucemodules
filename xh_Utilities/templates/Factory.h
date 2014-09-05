#ifndef FACTORY_H_INCLUDED
#define FACTORY_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////
/**

	e.g.

	typedef Factory<MyClass> MyClassFactory;
	typedef MyClassFactory::Interface<> MyClassCreator;
*/
///////////////////////////////////////////////////////////////////////////////

template <class BaseObjectType>
class Factory
{
public:

	typedef Factory< BaseObjectType > FactoryBase;

	virtual ~Factory () {}
	virtual BaseObjectType* createInstance () = 0;

	template <class ObjectType>
	class Type  :   public FactoryBase
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

#endif//FACTORY_H_INCLUDED
