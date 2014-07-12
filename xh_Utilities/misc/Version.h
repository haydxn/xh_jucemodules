#ifndef VERSION_H_INCLUDED
#define VERSION_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////

class Version
{
public:

	Version ();
	Version (const Version& other);
	Version (int versionValue);
	Version (int major, int minor, int point);
	~Version ();
	Version& operator= (const Version& other);
	bool operator> (const Version& other) const;
	bool operator>= (const Version& other) const;
	bool operator< (const Version& other) const;
	bool operator<= (const Version& other) const;
	bool operator== (const Version& other) const;
	bool operator!= (const Version& other) const;

	juce::String toString () const;
	int getValue () const;

	void setMajorVersion (int version);
	void setMinorVersion (int version);
	void setPointVersion (int version);

	int getMajorVersion () const;
	int getMinorVersion () const;
	int getPointVersion () const;

	static Version fromString (const juce::String& text);

private:

	int value;

};

///////////////////////////////////////////////////////////////////////////////

#endif  // BASICNODEDOCUMENT_H_INCLUDED
