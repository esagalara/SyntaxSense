#include "utf.h"
#include <windows.h>

/// Construct from UTF-16
cUTF::cUTF(const wchar_t * ws)
{
	// store copy of UTF16
	myString16 = (wchar_t *)malloc(wcslen(ws) * 2 + 2);
	wcscpy(myString16, ws);
	// How long will the UTF-8 string be
	int len = WideCharToMultiByte(CP_UTF8, 0,
		ws, wcslen(ws),
		NULL, NULL, NULL, NULL);
	// allocate a buffer
	myString8 = (char *)malloc(len + 1);
	// convert to UTF-8
	WideCharToMultiByte(CP_UTF8, 0,
		ws, wcslen(ws),
		myString8, len, NULL, NULL);
	// null terminate
	*(myString8 + len) = '\0';
}

///  Construct from UTF8
cUTF::cUTF(const char * s)
{
	myString8 = (char *)malloc(strlen(s) + 1);
	strcpy(myString8, s);
	// How long will the UTF-16 string be
	int len = MultiByteToWideChar(CP_UTF8, 0,
		s, strlen(s),
		NULL, NULL);
	// allocate a buffer
	myString16 = (wchar_t *)malloc(len * 2 + 2);
	// convert to UTF-16
	MultiByteToWideChar(CP_UTF8, 0,
		s, strlen(s),
		myString16, len);
	// null terminate
	*(myString16 + len) = L'\0';
}