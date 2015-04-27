//this file is part of notepad++
//Copyright (C)2003 Don HO <donho@altern.org>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "PluginDefinition.h"
#include "menuCmdID.h"

//
// put the headers you need here
//
#include <stdlib.h>
#include <time.h>
#include <shlwapi.h>

#include <iostream>
#include <string>
#include <regex>

using namespace std::tr1;

const TCHAR configFileName[] = TEXT("SyntaxSense.ini");
const int MAX_LANGUAGES = 32;

#ifdef UNICODE 
	#define generic_itoa _itow
#else
	#define generic_itoa itoa
#endif

FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;


//buffer for analyzing texts
const int buflen = 1000;
char* buffer;

struct KnownLanguage {
	LangType langType;
	regex regex;
};

//array of known languages
KnownLanguage* knownLanguages;
int noLanguages = 0;


//
// Initialize your plugin data here
// It will be called while plugin loading
void pluginInit(HANDLE hModule)
{
	buffer = new char[buflen + 24];
}



//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
	delete []buffer;
	delete []knownLanguages;
}

//
// get path of plugin configuration file
//
void findConfigFile(wchar_t* iniFilePath) {
	
	::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)iniFilePath);

	// make your plugin config file full file path name
	PathAppend(iniFilePath, configFileName);

	if (PathFileExists(iniFilePath) == FALSE)
	{		
		iniFilePath = L"";
	}
}


//
//Read config and initialize regular expressions
//
void initLanguages() 
{
	noLanguages = 0;
	knownLanguages = new KnownLanguage[MAX_LANGUAGES];

	wchar_t regexWString[128];
	char regexString[128];

	// get path of configuration file
	wchar_t iniFilePath[MAX_PATH];
	findConfigFile(iniFilePath);
	if (lstrlenW(iniFilePath) == 0) {
		::MessageBox(nppData._nppHandle, L"Config file not found", L"Syntax Sense Plugin", MB_OK | MB_ICONWARNING);
		return;
	}

	// get sections - separated by '\0'
	wchar_t sections[1024];
	int chars = ::GetPrivateProfileSectionNames(sections, 1024, iniFilePath);
	if (chars == 0) return;

	// parse every section
	wchar_t* section;
	int p = 0;
	while (p < chars)
	{
		//point to next section
		section = &sections[p];
		p += lstrlenW(section) + 1;	//add length and step over terminator
	
		//get language type
		knownLanguages[noLanguages].langType = (LangType) ::GetPrivateProfileInt(section, L"langcode", 0, iniFilePath);

		//get regex
		::GetPrivateProfileString(section, L"regex", L"", regexWString, 128, iniFilePath);
		wcstombs(regexString, regexWString, 128); // convert to char*

		//flags for regex builder
		int flags = ::GetPrivateProfileInt(section, L"flags", 1, iniFilePath);

		regex_constants::syntax_option_type options = regex_constants::ECMAScript;
		if (flags > 0) {
			options |= static_cast<regex_constants::syntax_option_type>(flags);
		}

		knownLanguages[noLanguages].regex = regex(regexString, options);
		noLanguages++;
	}
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{
	initLanguages();


    //--------------------------------------------//
    //-- STEP 3. CUSTOMIZE YOUR PLUGIN COMMANDS --//
    //--------------------------------------------//
    // with function :
    // setCommand(int index,                      // zero based number to indicate the order of command
    //            TCHAR *commandName,             // the command name that you want to see in plugin menu
    //            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
    //            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
    //            bool check0nInit                // optional. Make this menu item be checked visually
    //            );
    setCommand(0, TEXT("About"), about, NULL, false);
	setCommand(1, TEXT("Open config"), openConfig, NULL, false);
}


//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
}

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//
void about()
{
	const TCHAR about[] = L"SyntaxSense is a magical plugin!";
	::MessageBox(nppData._nppHandle, about, NPP_PLUGIN_NAME, MB_OK);
}

void openConfig() {
	wchar_t iniFilePath[MAX_PATH];
	findConfigFile(iniFilePath);

	if (lstrlenW(iniFilePath) == 0) {
		::MessageBox(nppData._nppHandle, L"Config file not found", L"Syntax Sense Plugin", MB_OK | MB_ICONERROR);
		return;
	}

	::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)iniFilePath);
}


//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit) 
{
    if (index >= nbFunc)
        return false;

    if (!pFunc)
        return false;

    lstrcpy(funcItem[index]._itemName, cmdName);
    funcItem[index]._pFunc = pFunc;
    funcItem[index]._init2Check = check0nInit;
    funcItem[index]._pShKey = sk;

    return true;
}


TCHAR formatString[128];

void analyzeSyntax(int bufferid)
{
	//query lang type
	LangType docType;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTLANGTYPE, 0, (LPARAM)&docType);
	if (docType != L_TXT) return;

	//get current editor
	int currentEdit;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
	HWND hCurrentEditView = (currentEdit == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

	int textLen = ::SendMessage(hCurrentEditView, SCI_GETLENGTH, 0, 0);
	if (textLen == 0) return;	

	//get first n characters of file	
	int len = textLen < buflen ? textLen : buflen;
	::SendMessage(hCurrentEditView, SCI_GETTEXT, (WPARAM)len, (LPARAM)buffer);

	//try each regex until match
	for (int i = 0; i < noLanguages; i++) {
		if (regex_search(buffer, knownLanguages[i].regex)) {
			::SendMessage(nppData._nppHandle, NPPM_SETCURRENTLANGTYPE, 0, (LPARAM)knownLanguages[i].langType);

			wsprintf(formatString, L"Found match for language: %d\n", knownLanguages[i].langType);
			OutputDebugString(formatString);
			return;
		}
	}

	OutputDebugString(L"No match :(\n");
}