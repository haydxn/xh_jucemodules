#ifndef ARRAYDUPLICATESCANNER_H_INCLUDED
#define ARRAYDUPLICATESCANNER_H_INCLUDED

template <class ValueType>
class ArrayDuplicateScanner
{
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArrayDuplicateScanner);
public:

	ArrayDuplicateScanner ()
	{
	}

	~ArrayDuplicateScanner ()
	{
	}

	void reset ()
	{
		checkBuffer.clear ();
		duplicateValues.clear ();
		duplicateCounts.clear ();
	}

	void prepare (int arraySize)
	{
		reset ();
		checkBuffer.ensureStorageAllocated (arraySize);
		duplicateCounts.ensureStorageAllocated (arraySize);
		duplicateValues.ensureStorageAllocated (arraySize);
	}

	void processElement (const ValueType& valueToCheck)
	{
		if (expectedSize >= 0)
		{
			jassert (checkBuffer.size () < expectedSize);
		}

		if (checkBuffer.contains (valueToCheck))
		{
			int dupeIndex = duplicateValues.indexOf (valueToCheck);
			if (dupeIndex == -1)
			{
				duplicateValues.add (valueToCheck);
				duplicateCounts.add (1); // We've seen 2 instances now, but we'll only count dupes
			}
			else
			{
				duplicateCounts.getReference (dupeIndex)++;
			}

			checkBuffer.add (valueToCheck);
		}
	}

	template <class ArrayType>
	void processArray (const ArrayType& source)
	{
		int size = source.size ();
		prepare (size);
		for (int i=0; i<size; ++i)
		{
			processElement (source.getUnchecked (i));
		}
	}

	bool anyFound () const
	{
		return getNumDifferentDuplicatesFound() > 0;
	}

	int getNumDifferentDuplicatesFound () const
	{
		return duplicateValues.size ();
	}

	ValueType getDuplicateValueAt (int index) const
	{
		return duplicateValues[index];
	}

	int getNumDuplicatesOfValueAt (int index) const
	{
		return duplicateCounts[index];
	}

	int getNumExtraValues () const
	{
		int totalDupes = 0;
		for (int i=0; i<duplicateCounts.size(); i++)
		{
			totalDupes += duplicateCounts.getUnchecked (i);
		}
		return totalDupes;
	}

private:

	juce::Array< ValueType > checkBuffer;
	juce::Array< ValueType > duplicateValues;
	juce::Array< int > duplicateCounts;
	int expectedSize;

};



#endif  // ARRAYDUPLICATESCANNER_H_INCLUDED
