#ifndef SUBCLASSWEAKREFERENCE_H_INCLUDED
#define SUBCLASSWEAKREFERENCE_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////
/**
	Template helper for keeping WeakReference to a subclass of a class with
	a private master reference.
*/
///////////////////////////////////////////////////////////////////////////////

template <class WeakReferableBaseClass, class SubClass>
class SubClassWeakReference
{
public:
	/** Creates a null SubClassWeakReference. */
	SubClassWeakReference() noexcept {}

	/** Creates a SubClassWeakReference that points at the given object. */
	SubClassWeakReference (SubClass* const object) : weakRef (object) {}

	/** Creates a copy of another SubClassWeakReference. */
	SubClassWeakReference (const SubClassWeakReference& other) noexcept : weakRef (other.weakRef) {}

	/** Copies another pointer to this one. */
	SubClassWeakReference& operator= (const SubClassWeakReference& other) { weakRef = other.weakRef; return *this; }

	/** Copies another pointer to this one. */
	SubClassWeakReference& operator= (SubClass* const newObject) { weakRef = newObject; return *this; }

	/** Returns the object that this pointer refers to, or null if the object no longer exists. */
	SubClass* getObject () const noexcept        { return dynamic_cast <SubClass*> (weakRef.get()); }

	/** Returns the object that this pointer refers to, or null if the object no longer exists. */
	operator SubClass*() const noexcept          { return getObject(); }

	/** Returns the object that this pointer refers to, or null if the object no longer exists. */
	SubClass* operator->() noexcept              { return getObject(); }

	/** Returns the object that this pointer refers to, or null if the object no longer exists. */
	const SubClass* operator->() const noexcept  { return getObject(); }

	/** If the object is valid, this deletes it and sets this pointer to null. */
	void deleteAndZero()                         { delete getObject(); }

	bool operator== (SubClass* object) const noexcept   { return weakRef == object; }
	bool operator!= (SubClass* object) const noexcept   { return weakRef != object; }

private:
	juce::WeakReference< WeakReferableBaseClass > weakRef;
};

///////////////////////////////////////////////////////////////////////////////

#endif  // SUBCLASSWEAKREFERENCE_H_INCLUDED
