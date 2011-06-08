#pragma once

/*
global.hpp

�������� ���������� ����������
�������� ���������.
*/
/*
Copyright � 1996 Eugene Roshal
Copyright � 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

extern clock_t StartIdleTime;

extern OSVERSIONINFOW WinVer;

extern int WaitInMainLoop;
extern int WaitInFastFind;

extern string g_strFarModuleName;
extern string g_strFarINI;
extern string g_strFarPath;

extern string strGlobalSearchString;
extern int GlobalSearchCase;
extern int GlobalSearchWholeWords; // �������� "Whole words" ��� ������
extern int GlobalSearchHex; // �������� "Search for hex" ��� ������

extern int GlobalSearchReverse;

extern int ScreenSaverActive;

extern int CloseFAR, CloseFARMenu, AllowCancelExit;

extern int DisablePluginsOutput;

extern BOOL IsProcessAssignMacroKey;
extern BOOL IsRedrawFramesInProcess;

extern const wchar_t* Version;
extern const wchar_t* Copyright;

extern int WidthNameForMessage;

extern BOOL ProcessException;

extern BOOL ProcessShowClock;

extern const wchar_t *HelpFileMask;
extern const wchar_t *HelpFormatLinkModule;

#if defined(SYSLOG)
extern BOOL StartSysLog;
extern long CallNewDelete;
extern long CallMallocFree;
#endif

class SaveScreen;
extern SaveScreen *GlobalSaveScrPtr;

extern int CriticalInternalError;

extern int _localLastError;

extern int KeepUserScreen;
extern string g_strDirToSet; //RAVE!!!

extern int Macro_DskShowPosType; // ��� ����� ������ �������� ���� ������ ������ (0 - ������� �� ��������, 1 - ����� (AltF1), 2 - ������ (AltF2))

// Macro Const
extern const wchar_t constMsX[];
extern const wchar_t constMsY[];
extern const wchar_t constMsButton[];
extern const wchar_t constMsCtrlState[];
extern const wchar_t constMsEventFlags[];
extern const wchar_t constRCounter[];

extern DWORD RedrawTimeout;

extern SYSTEM_INFO SystemInfo;

extern FormatScreen FS;

extern DWORD ErrorMode;
