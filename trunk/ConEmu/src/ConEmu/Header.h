
/*
Copyright (c) 2009-2010 Maximus5
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

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
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

#pragma once

#define NEWMOUSESTYLE
#define NEWRUNSTYLE
//#undef NEWRUNSTYLE

//#define USEPORTABLEREGISTRY
#undef USEPORTABLEREGISTRY

#include <windows.h>
#include <Shlwapi.h>
#include <vector>
//#if !defined(__GNUC__)
//#include <crtdbg.h>
//#endif

#include "../common/Memory.h"

#ifdef __GNUC__
#define wmemmove_s(d,ds,s,ss) wmemmove(d,s,ss)
#define SecureZeroMemory(p,s) memset(p,0,s)
#endif

#ifndef TimeGetTime
	#ifdef __GNUC__
		#define TimeGetTime GetTickCount
	#else
		#define TimeGetTime timeGetTime
	#endif
#endif

#ifdef KL_MEM
#include "c:\\lang\\kl.h"
#else
#include "kl_parts.h"
#endif

// Array sizes
#define MAX_CONSOLE_COUNT 12


#include "globals.h"
#include "resource.h"


#define DRAG_L_ALLOWED 0x01
#define DRAG_R_ALLOWED 0x02
#define DRAG_L_STARTED 0x10
#define DRAG_R_STARTED 0x20
#define MOUSE_R_LOCKED 0x40
#define MOUSE_SIZING_BEGIN  0x080
#define MOUSE_SIZING_TODO   0x100
#define MOUSE_SIZING_DBLCKL 0x200
#define MOUSE_DRAGPANEL_SPLIT 0x400
#define MOUSE_DRAGPANEL_LEFT  0x800
#define MOUSE_DRAGPANEL_RIGHT 0x1000
#define MOUSE_DRAGPANEL_SHIFT 0x2000
//#define MOUSE_SIZING   0x80

#define MOUSE_DRAGPANEL_ALL (MOUSE_DRAGPANEL_SPLIT|MOUSE_DRAGPANEL_LEFT|MOUSE_DRAGPANEL_RIGHT|MOUSE_DRAGPANEL_SHIFT)

#define MAX_TITLE_SIZE 0x400

#define CON_RECREATE_TIMEOUT 30000
#define CON_REDRAW_TIMOUT 5

#ifndef CLEARTYPE_NATURAL_QUALITY
#define CLEARTYPE_QUALITY       5
#define CLEARTYPE_NATURAL_QUALITY       6
#endif

#ifndef EVENT_CONSOLE_START_APPLICATION
#define EVENT_CONSOLE_START_APPLICATION 0x4006
#define EVENT_CONSOLE_END_APPLICATION   0x4007
#endif

#if !defined(CONSOLE_APPLICATION_16BIT)
#define CONSOLE_APPLICATION_16BIT       0x0001
#endif

// Undocumented console message
#define WM_SETCONSOLEINFO           (WM_USER+201)
// and others
#define SC_RESTORE_SECRET 0x0000f122
#define SC_MAXIMIZE_SECRET 0x0000f032
#define SC_PROPERTIES_SECRET 0x0000fff7
#define SC_MARK_SECRET 0x0000fff2
#define SC_COPY_ENTER_SECRET 0x0000fff0
#define SC_PASTE_SECRET 0x0000fff1
#define SC_SELECTALL_SECRET 0x0000fff5
#define SC_SCROLL_SECRET 0x0000fff3
#define SC_FIND_SECRET 0x0000fff4
#define SC_SYSMENUPOPUP_SECRET 0x0000f093

#define MAX_CMD_HISTORY 100
#define MAX_CMD_HISTORY_SHOW 16
#define MAX_CMD_GROUP_SHOW 16

#define MBox(rt) {BOOL b = gbDontEnable; gbDontEnable = TRUE; (int)MessageBox(gbMessagingStarted ? ghWnd : NULL, rt, Title, /*MB_SYSTEMMODAL |*/ MB_ICONINFORMATION); gbDontEnable = b; }
#define MBoxA(rt) {BOOL b = gbDontEnable; gbDontEnable = TRUE; (int)MessageBox(gbMessagingStarted ? ghWnd : NULL, rt, _T("ConEmu"), /*MB_SYSTEMMODAL |*/ MB_ICONINFORMATION); gbDontEnable = b; }

//#define MBoxAssert(V) if ((V)==FALSE) { TCHAR szAMsg[MAX_PATH*2]; StringCchPrintf(szAMsg, countof(szAMsg), _T("Assertion (%s) at\n%s:%i"), _T(#V), _T(__FILE__), __LINE__); MBoxA(szAMsg); }
#define MBoxAssert(V) _ASSERTE(V)
//__inline BOOL isMeForeground() {
//	HWND h = GetForegroundWindow();
//	return h && (h == ghWnd || h == ghOpWnd || h == ghConWnd);
//}
//#endif
//#define isPressed(inp) ((GetKeyState(inp) & 0x8000) == 0x8000)
//#define isDriveLetter(c) ((c>=L'A' && c<=L'Z') || (c>=L'a' && c<=L'z'))
//#define isDigit(c) (c>=L'0' && c<=L'9')

//#define PTDIFFTEST(C,D) (((abs(C.x-LOWORD(lParam)))<D) && ((abs(C.y-HIWORD(lParam)))<D))

//#define INVALIDATE() InvalidateRect(HDCWND, NULL, FALSE)

#define SafeCloseHandle(h) { if ((h)!=NULL) { HANDLE hh = (h); (h) = NULL; if (hh!=INVALID_HANDLE_VALUE) CloseHandle(hh); } }
//#define SafeFree(p) { if ((p)!=NULL) { LPVOID pp = (p); (p) = NULL; free(pp); } }

#ifdef MSGLOGGER
#define POSTMESSAGE(h,m,w,l,e) {MCHKHEAP; DebugLogMessage(h,m,w,l,TRUE,e); PostMessage(h,m,w,l);}
#define SENDMESSAGE(h,m,w,l) {MCHKHEAP;  DebugLogMessage(h,m,w,l,FALSE,FALSE); SendMessage(h,m,w,l);}
#define SETWINDOWPOS(hw,hp,x,y,w,h,f) {MCHKHEAP; DebugLogPos(hw,x,y,w,h,"SetWindowPos"); SetWindowPos(hw,hp,x,y,w,h,f);}
#define MOVEWINDOW(hw,x,y,w,h,r) {MCHKHEAP; DebugLogPos(hw,x,y,w,h,"MoveWindow"); MoveWindow(hw,x,y,w,h,r);}
#define SETCONSOLESCREENBUFFERSIZERET(h,s,r) {MCHKHEAP; DebugLogBufSize(h,s); r=SetConsoleScreenBufferSize(h,s);}
#define SETCONSOLESCREENBUFFERSIZE(h,s) {BOOL lb; SETCONSOLESCREENBUFFERSIZERET(h,s,lb);}
#define DEBUGLOGFILE(m) DebugLogFile(m)
#else
#define POSTMESSAGE(h,m,w,l,e) PostMessage(h,m,w,l)
#define SENDMESSAGE(h,m,w,l) SendMessage(h,m,w,l)
#define SETWINDOWPOS(hw,hp,x,y,w,h,f) SetWindowPos(hw,hp,x,y,w,h,f)
#define MOVEWINDOW(hw,x,y,w,h,r) MoveWindow(hw,x,y,w,h,r)
#define SETCONSOLESCREENBUFFERSIZERET(h,s,r) r=SetConsoleScreenBufferSize(h,s)
#define SETCONSOLESCREENBUFFERSIZE(h,s) SetConsoleScreenBufferSize(h,s)
#define DEBUGLOGFILE(m)
#endif


//#if !defined(__GNUC__)
//#pragma warning (disable : 4005)
//#if !defined(_WIN32_WINNT)
//#define _WIN32_WINNT 0x0502
//#endif
//#endif




//------------------------------------------------------------------------
///| Code optimizing |////////////////////////////////////////////////////
//------------------------------------------------------------------------

//#if !defined(__GNUC__)
//#include <intrin.h>
//#pragma function(memset, memcpy)
//__forceinline void *memset(void *_Dst, int _Val, size_t _Size)
//{
//	__stosb((unsigned char*)_Dst, _Val, _Size);
//	return _Dst;
//}
//__forceinline void *memcpy(void *_Dst, const void *_Src, size_t _Size)
//{
//	__movsb((unsigned char*)_Dst, (unsigned const char*)_Src, _Size);
//	return _Dst;
//}
//#endif

extern BOOL gbInDisplayLastError;
int DisplayLastError(LPCTSTR asLabel, DWORD dwError = 0, DWORD dwMsgFlags = 0, LPCWSTR asTitle = NULL);

COORD /*__forceinline*/ MakeCoord(int W,int H);
//{
//	COORD rc; rc.X=W; rc.Y=H;
//	return rc;
//}

POINT /*__forceinline*/ MakePoint(int W,int H);

RECT /*__forceinline*/ MakeRect(int W,int H);
//{
//	RECT rc; rc.left=0; rc.top=0; rc.right=W; rc.bottom=H;
//	return rc;
//}

RECT /*__forceinline*/ MakeRect(int X1, int Y1,int X2,int Y2);
//{
//	RECT rc; rc.left=X1; rc.top=Y1; rc.right=X2; rc.bottom=Y2;
//	return rc;
//}

BOOL /*__forceinline*/ CoordInRect(const COORD& c, const RECT& r);
//{
//	return (c.X >= r.left && c.X <= r.right) && (c.Y >= r.top && c.Y <= r.bottom);
//}

BOOL IntersectSmallRect(RECT& rc1, SMALL_RECT& rc2);

wchar_t* GetDlgItemText(HWND hDlg, WORD nID);

//#pragma warning(disable: 4311) // 'type cast' : pointer truncation from 'HBRUSH' to 'BOOL'



#include "../common/RConStartArgs.h"



//------------------------------------------------------------------------
///| Registry |///////////////////////////////////////////////////////////
//------------------------------------------------------------------------

#include "Registry.h"

//------------------------------------------------------------------------
///| Global variables |///////////////////////////////////////////////////
//------------------------------------------------------------------------

#define pHelp \
	L"Console emulation program.\n" \
	L"Home page: http://conemu-maximus5.googlecode.com\n\n" \
	L"By default this program launches \"Far.exe\" (if exists) or \"cmd.exe\"/\"tcc.exe\".\n" \
	L"\n" \
	L"Command line switches:\n" \
	L"/? - This help screen.\n" \
	L"/config <configname> - Use alternative named configuration\n" \
	L"/ct - Clear Type anti-aliasing.\n" \
	L"/fs | /max - (Full screen) | (Maximized) mode.\n" \
	L"/multi | /nomulti - enable or disable multiconsole features\n" \
	L"/font <fontname> - Specify the font name.\n" \
	L"/size <fontsize> - Specify the font size.\n" \
	L"/fontfile <fontfilename> - Loads fonts from file.\n" \
	L"/BufferHeight <lines> - may be used with cmd.exe/tcc.exe\n" \
	/* L"/Attach [PID] - intercept console of specified process\n" */ \
	L"\n" \
	L"/cmd <commandline>|@<commandfile> - Command line to start. This must be the last used switch.\n" \
	L"\n" \
	L"Command line examples:\n" \
	L"ConEmu.exe /ct /font \"Lucida Console\" /size 16 /cmd Far.exe /w\n" \
	L"\n" \
	L"\x00A9 2006-2008 Zoin (based on console emulator by SEt)\n" \
	CECOPYRIGHTSTRING_W /*\x00A9 2009-2011 ConEmu.Maximus5@gmail.com*/ L"\n" \
	L"\n" \
	L"Contributors\n" \
	L"thecybershadow: bdf support\n" \
	L"NightRoman: drawing optimization, BufferHeight and other fixes\n" \
	L"dolzenko_: windows switching via GUI tabs\n" \
	L"alex_itd: Drag'n'Drop, RightClick, AltEnter\n" \
	L"Mors: loading font from file."


//#include "VirtualConsole.h"
//#include "options.h"
//#include "DragDrop.h"
//#include "TrayIcon.h"
//#include "VConChild.h"
//#include "ConEmu.h"
//#include "ConEmuApp.h"
//#include "tabbar.h"
//#include "TrayIcon.h"
//#include "ConEmuPipe.h"
#include "../common/UnicodeChars.h"
#include "../common/defines.h"
#include "../common/WinObjects.h"

#define IsWindows7 ((gOSVer.dwMajorVersion > 6) || (gOSVer.dwMajorVersion == 6 && gOSVer.dwMinorVersion > 0))

// GNU C HEADER PATCH
#ifdef __GNUC__
typedef BOOL (WINAPI* AlphaBlend_t)(HDC hdcDest, int xoriginDest, int yoriginDest, int wDest, int hDest, HDC hdcSrc, int xoriginSrc, int yoriginSrc, int wSrc, int hSrc, BLENDFUNCTION ftn);
typedef BOOL (WINAPI* SetLayeredWindowAttributes_t)(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
typedef BOOL (WINAPI* CreateRestrictedToken_t)(HANDLE ExistingTokenHandle, DWORD Flags, DWORD DisableSidCount, PSID_AND_ATTRIBUTES SidsToDisable, DWORD DeletePrivilegeCount, PLUID_AND_ATTRIBUTES PrivilegesToDelete, DWORD RestrictedSidCount, PSID_AND_ATTRIBUTES SidsToRestrict, PHANDLE NewTokenHandle);
#endif
// GetLayeredWindowAttributes ��������� ������ � XP
typedef BOOL (WINAPI* GetLayeredWindowAttributes_t)(HWND hwnd, COLORREF *pcrKey, BYTE *pbAlpha, DWORD *pdwFlags);

#ifdef __GNUC__
	#define MAPVK_VK_TO_VSC     (0)
	#define MAPVK_VSC_TO_VK     (1)
	#define MAPVK_VK_TO_CHAR    (2)
	#define MAPVK_VSC_TO_VK_EX  (3)
#endif

#ifndef SEE_MASK_NOASYNC
	#define SEE_MASK_NOASYNC    0x00000100   // block on the call until the invoke has completed, use for callers that exit after calling ShellExecuteEx()
#endif

#ifndef TTM_SETTITLEW
	#define TTM_SETTITLEW           (WM_USER + 33)  // wParam = TTI_*, lParam = wchar* szTitle
#endif
#ifndef TTM_SETTITLE
	#define TTM_SETTITLE            TTM_SETTITLEW
#endif
#ifndef TTI_WARNING
	#define TTI_WARNING             2
#endif

#ifndef INPUTLANGCHANGE_SYSCHARSET
#define INPUTLANGCHANGE_SYSCHARSET 0x0001
#endif

#ifndef DISABLE_MAX_PRIVILEGE
#define DISABLE_MAX_PRIVILEGE   0x1 
#endif
