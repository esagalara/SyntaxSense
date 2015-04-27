#include <stdlib.h>

/**

From https://ravenspoint.wordpress.com/2010/05/30/world-wide-characters/

Conversion between UTF-8 and UTF-16 strings.

UTF-8 is used by web pages.  It is a variable byte length encoding
of UNICODE characters which is independant of the byte order in a computer word.

UTF-16 is the native Windows UNICODE encoding.

The class stores two copies of the string, one in each encoding,
so should only exist briefly while conversion is done.

This is a wrapper for the WideCharToMultiByte and MultiByteToWideChar
*/
class cUTF
{
	wchar_t * myString16;       ///&amp;lt; string in UTF-16
	char * myString8;           ///&amp;lt; string in UTF-6

public:
	/// Construct from UTF-16
	cUTF(const wchar_t * ws);
	///  Construct from UTF8
	cUTF(const char * s);
	/// get UTF16 version
	const wchar_t * get16() { return myString16; }
	/// get UTF8 version
	const char * get8() { return myString8; }
	/// free buffers
	~cUTF() { free(myString8); free(myString16); }
};