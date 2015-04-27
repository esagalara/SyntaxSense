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
#include <shlwapi.h>


extern FuncItem funcItem[nbFunc];
extern NppData nppData;
//extern bool doCloseTag;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  reasonForCall, 
                       LPVOID lpReserved )
{
    switch (reasonForCall)
    {
      case DLL_PROCESS_ATTACH:
        pluginInit(hModule);
        break;

      case DLL_PROCESS_DETACH:
        pluginCleanUp();
        break;

      case DLL_THREAD_ATTACH:
        break;

      case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}


extern "C" __declspec(dllexport) void setInfo(NppData notpadPlusData)
{
	nppData = notpadPlusData;
	commandMenuInit();
}

extern "C" __declspec(dllexport) const TCHAR * getName()
{
	return NPP_PLUGIN_NAME;
}

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbF)
{
	*nbF = nbFunc;
	return funcItem;
}



//last activated NPP buffer
int lastBufferId = 0;
UINT_PTR timer = 0;

VOID CALLBACK timerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
	::KillTimer(nppData._nppHandle, timer);
	timer = 0;

	TCHAR formatString[128];
	wsprintf(formatString, L"Timer fired for buffer %d\n", idEvent);
	OutputDebugString(formatString);

	analyzeSyntax((int) idEvent);
}


extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode)
{
	static bool fileOpened = false, isReady = false;	
	static TCHAR formatString[128];	
	
	switch (notifyCode->nmhdr.code) 
	{
		case NPPN_READY:
			OutputDebugString(L"NPP is ready\n");
			isReady = true;
			break;

		case NPPN_SHUTDOWN:
			OutputDebugString(L"NPP is closing\n");
			commandMenuCleanUp();
			break;

		case NPPN_FILEOPENED: 
			if (isReady) fileOpened = true;
			break;

		case NPPN_BUFFERACTIVATED:
			lastBufferId = notifyCode->nmhdr.idFrom;
			if (!fileOpened) break; //just switched active file

#if _DEBUG		
			//get file name
			TCHAR fileName[MAX_PATH];
			::SendMessage(nppData._nppHandle, NPPM_GETFILENAME, MAX_PATH, (LPARAM)&fileName);

			wsprintf(formatString, L"Buffer with opened file '%s' activated %d\n", fileName, notifyCode->nmhdr.idFrom);
			OutputDebugString(formatString);
#endif
			//text or unkown format, analyze contents
			analyzeSyntax(notifyCode->nmhdr.idFrom);

			fileOpened = false;
			break;

		case SCN_MODIFIED:		
			if (!isReady) break;
			if (notifyCode->modificationType && SC_MOD_INSERTTEXT == SC_MOD_INSERTTEXT && notifyCode->linesAdded > 1 && notifyCode->position == 0) {
#if _DEBUG
				wsprintf(formatString, L"SCN_MODIFIED %d - set timer to analyze this later...\n", lastBufferId);
				OutputDebugString(formatString);
#endif
				//set timer, since we can't do anything in this event handler
				timer = ::SetTimer(nppData._nppHandle, lastBufferId, 50, timerProc);
			}

			break;

		default:
			return;
	}
}


// Here you can process the Npp Messages 
// I will make the messages accessible little by little, according to the need of plugin development.
// Please let me know if you need to access to some messages :
// http://sourceforge.net/forum/forum.php?forum_id=482781
//
extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}

#ifdef UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode()
{
    return TRUE;
}
#endif //UNICODE
