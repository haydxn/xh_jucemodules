#include "RelativeWeightSequence.h"


RelativeWeightSequence::RelativeWeightSequence ()
{
    
}

RelativeWeightSequence::RelativeWeightSequence (const RelativeWeightSequence& other)
{
    values = other.values;
}

RelativeWeightSequence& RelativeWeightSequence::operator= (const RelativeWeightSequence& other)
{
    values = other.values;
    return *this;
}

RelativeWeightSequence::~RelativeWeightSequence ()
{
    
}

void RelativeWeightSequence::add (double value)
{
    values.add (value);
}

void RelativeWeightSequence::insert (int indexToInsertAt, double value)
{
    values.insert (indexToInsertAt, value);
}

void RelativeWeightSequence::set (int indexToSetAt, double value)
{
    values.set (indexToSetAt, value);
}

double RelativeWeightSequence::operator[] (int index) const
{
    return values [index];
}

void RelativeWeightSequence::remove (int index)
{
    values.remove (index);
}

void RelativeWeightSequence::clear ()
{
    values.clear ();
}

int RelativeWeightSequence::size () const
{
    return values.size ();
}

void RelativeWeightSequence::normalise ()
{
    double totalWeight = getTotalWeight();
    int n = size ();
    if (totalWeight > 0)
    {
        for (int i=0; i<n; i++)
        {
            values.getReference(i) /= totalWeight;
        }
    }
    else
    {
        values.clearQuick();
        values.insertMultiple (0, 0.0, n);
    }
}

double RelativeWeightSequence::sumRange (int startIndex, int count) const
{
    startIndex = jmax (startIndex, 0);
    
    int end = startIndex + count;
    if (end > size ())
        end = size ();
    
    double result = 0.0;
    
    for (int i = startIndex; i < end; ++i)
    {
        result += values.getUnchecked(i);
    }
    
    return result;
}

double RelativeWeightSequence::accumulateToIndex (int index, double proportionOfIndex) const
{
    proportionOfIndex = jlimit (0.0, 1.0,proportionOfIndex);
    index = jlimit (0, size(), index);
    
    double result = sumRange (0, index);
    result += values[index] * proportionOfIndex;
    
    return result;
}

double RelativeWeightSequence::accumulateToIndexNormalised (int index, double proportionOfIndex) const
{
    return normaliseValue (accumulateToIndex (index, proportionOfIndex));
}

double RelativeWeightSequence::getTotalWeight () const
{
    return sumRange (0, size());
}

double RelativeWeightSequence::getNormalised (int index) const
{
    return normaliseValue (values[index]);
}

double RelativeWeightSequence::normaliseValue (double weightedValue) const
{
    double totalWeight = getTotalWeight();
    if (totalWeight > 0)
    {
        return weightedValue / totalWeight;
    }
    return 0.0;
}


////////////////////////////////////////////////////////////////////////////////

class ProportionalWeightSequenceTests   :   public UnitTest
{
public:
    
    ProportionalWeightSequenceTests () : UnitTest ("RelativeWeightSequence") {}
    
    virtual void runTest ()
    {
        beginTest ("Basic sequence");
        
        RelativeWeightSequence weights;
        weights.add (1);
        weights.add (1);
        weights.add (1);
        
        expectEquals (weights.getTotalWeight(), 3.0);
        
        double w = weights.getNormalised (0);
        expect (jlimit (0.33,0.34, w) == w);
        
        expectEquals (weights.accumulateToIndex (0),        0.0);
        expectEquals (weights.accumulateToIndex (0, 0.5),   0.5);
        expectEquals (weights.accumulateToIndex (1),        1.0);
        expectEquals (weights.accumulateToIndex (1, 0.5),   1.5);
        expectEquals (weights.accumulateToIndex (1, 1.0),   2.0);
        expectEquals (weights.accumulateToIndex (1, 1.5),   2.0);
        expectEquals (weights.accumulateToIndex (2),        2.0);
        expectEquals (weights.accumulateToIndex (2, 0.5),   2.5);
        expectEquals (weights.accumulateToIndex (3),        3.0);
        expectEquals (weights.accumulateToIndex (3, 0.5),   3.0);
        
        expectEquals (weights.accumulateToIndexNormalised (1, 0.5), 0.5);
        

        beginTest ("Normalise");

        weights.clear ();
        weights.add (1);
        weights.add (2);
        weights.add (1);

        expectEquals (weights.getTotalWeight(), 4.0);
        weights.normalise ();
        expectEquals (weights.getTotalWeight(), 1.0);

        expectEquals (weights.accumulateToIndex (0),        0.0);
        expectEquals (weights.accumulateToIndex (0, 0.5),   0.125);
        expectEquals (weights.accumulateToIndex (1),        0.25);
        expectEquals (weights.accumulateToIndex (1, 0.5),   0.5);
        expectEquals (weights.accumulateToIndex (1, 1.0),   0.75);
        expectEquals (weights.accumulateToIndex (1, 1.5),   0.75);
        expectEquals (weights.accumulateToIndex (2),        0.75);
        expectEquals (weights.accumulateToIndex (2, 1.0),   1.0);
        
        expectEquals (weights.accumulateToIndex (1, 0.3),   weights.accumulateToIndexNormalised (1, 0.3));

    }
    
};

static ProportionalWeightSequenceTests proportionalWeightSequenceTests;

