#ifndef __PROPORTIONALWEIGHTSEQUENCE_H_A2EE473A__
#define __PROPORTIONALWEIGHTSEQUENCE_H_A2EE473A__

////////////////////////////////////////////////////////////////////////////////
/**
    Describes a sequence of relative arbitrary weights, making it simple to
    calculate the normalised contribution of each.
 */
////////////////////////////////////////////////////////////////////////////////

class RelativeWeightSequence
{
public:
    
    RelativeWeightSequence ();
    RelativeWeightSequence (const RelativeWeightSequence& other);
    RelativeWeightSequence& operator= (const RelativeWeightSequence& other);
    ~RelativeWeightSequence ();
    
	/// Add a weight to the end of the sequence.
    void add (double value);

	/// Insert a weight at a specific position in the sequence.
    void insert (int indexToInsertAt, double value);

	/// Returns the weight at the specified index.
    double operator[] (int index) const;

	/// Replaces the weight at the specified index.
    void set (int indexToSet, double newValue);

	/// Remove the weight at the specified index.
    void remove (int index);

	/// Returns the total number of weights in the sequence.
    int size () const;

	/// Removes all weights.
    void clear ();
    
    /// Normalise all values in the sequence such that the total weight is 1.
    void normalise ();

	/// Returns the sum of all contained weight values.
    double getTotalWeight () const;

	/// Sums a specific range of weight values.
    double sumRange (int startIndex, int count) const;

	/// Returns the normalised sum of a specific range of weight values.
    double sumRangeNormalised (int startIndex, int count) const;

	/// Sums the weights up to (and including an optional proportion of) the specified index.
    double accumulateToIndex (int index, double proportionOfIndex = 0.0) const;

	/// Sums the weights up to (and including an optional proportion of) the specified index,
	/// and then normalises the result.
    double accumulateToIndexNormalised (int index, double proportionOfIndex = 0.0) const;

	/// Returns the normalised value of the weight at the specified index.
	double getNormalised (int index) const;
    
	/// Normalises the provided value against the total weight of the sequence.
    double normaliseValue (double weightedValue) const;
    
private:
    
    juce::Array<double> values;

};

////////////////////////////////////////////////////////////////////////////////

#endif  // __PROPORTIONALWEIGHTSEQUENCE_H_A2EE473A__
