///////////////////////////////////////////////////////////////////////////////

Version::Version ()
	:	value (0)
{

}

Version::Version (const Version& other)
	:	value (other.value)
{
}

Version::Version (int versionValue)
	:	value (versionValue)
{
}

Version::Version (int major, int minor, int point)
{
	value = ((major & 0x0000ffff) << 16) 
		|	((minor & 0x000000ff) << 8) 
		|	(point & 0x000000ff);
}

Version::~Version ()
{

}

Version& Version::operator= (const Version& other)
{
	value = other.value;
	return *this;
}

bool Version::operator> (const Version& other) const
{
	return value > other.value;
}

bool Version::operator>= (const Version& other) const
{
	return value > other.value;
}

bool Version::operator< (const Version& other) const
{
	return value < other.value;
}

bool Version::operator<= (const Version& other) const
{
	return value < other.value;
}

bool Version::operator== (const Version& other) const
{
	return value == other.value;
}

bool Version::operator!= (const Version& other) const
{
	return value != other.value;
}

String Version::toString () const
{
	return String(getMajorVersion()) + "." 
		+ String(getMinorVersion()) + "."
		+ String(getPointVersion());
}

int Version::getValue () const
{
	return value;
}

void Version::setMajorVersion (int version)
{
	value = (value & 0x0000ffff) | ((version & 0x0000ffff) << 16);
}

void Version::setMinorVersion (int version)
{
	value = (value & 0xffff00ff) | ((version & 0x000000ff) << 8);
}

void Version::setPointVersion (int version)
{
	value = (value & 0xffffff00) | (version & 0x000000ff);
}

int Version::getMajorVersion () const
{
	return (value & 0xffff0000) >> 16;
}

int Version::getMinorVersion () const
{
	return (value & 0x0000ff00) >> 8;
}

int Version::getPointVersion () const
{
	return (value & 0x000000ff);
}

Version Version::fromString (const juce::String& text)
{
	StringArray segments;
	segments.addTokens (text, ",.", "");
	segments.trim();
	segments.removeEmptyStrings();

	int value = (segments[0].getIntValue() << 16)
		+ (segments[1].getIntValue() << 8)
		+ segments[2].getIntValue();

	if (segments.size() >= 4)
		value = (value << 8) + segments[3].getIntValue();

	return value;
}

