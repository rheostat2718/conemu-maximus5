
/*
Copyright (c) 2009-2012 Maximus5
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


#define HOOKS_VIRTUAL_ALLOC
#ifdef _DEBUG
	#define REPORT_VIRTUAL_ALLOC
#else
	#undef REPORT_VIRTUAL_ALLOC
#endif

#define DROP_SETCP_ON_WIN2K3R2
//#define SHOWDEBUGSTR -- ���������� ���������, CONEMU_MINIMAL, OutputDebugString ����� �������� ������ ���������
//#define SKIPHOOKLOG

#undef SHOWCREATEPROCESSTICK
#undef SHOWCREATEBUFFERINFO
#ifdef _DEBUG
	#define SHOWCREATEPROCESSTICK
//	#define SHOWCREATEBUFFERINFO
	#define DUMP_VIM_SETCURSORPOS
#endif

#ifdef _DEBUG
	//#define TRAP_ON_MOUSE_0x0
	#undef TRAP_ON_MOUSE_0x0
	#undef LOG_GETANCESTOR
#else
	#undef TRAP_ON_MOUSE_0x0
	#undef LOG_GETANCESTOR
#endif

// ����� �� ����������� GetConsoleAliases (���� �� ������ ���� �������� � Win2k)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

#define DEFINE_HOOK_MACROS

#define SETCONCP_READYTIMEOUT 5000
#define SETCONCP_TIMEOUT 1000

#include <windows.h>
#include <WinError.h>
#include <WinNT.h>
#include <TCHAR.h>
#include <Tlhelp32.h>
#include <shlwapi.h>
#include "../common/common.hpp"
#include "../common/ConEmuCheck.h"
#include "SetHook.h"
#include "../common/execute.h"
#include "ConEmuHooks.h"
#include "RegHooks.h"
#include "ShellProcessor.h"
#include "UserImp.h"
#include "GuiAttach.h"
#include "../common/ConsoleAnnotation.h"
#include "../common/clink.h"
#include "../common/UnicodeChars.h"
#include "../common/WinConsole.h"


bool USE_INTERNAL_QUEUE = true;

#ifdef _DEBUG
// ��� ������� ecompl
INPUT_RECORD gir_Written[16] = {};
INPUT_RECORD gir_Real[16] = {};
INPUT_RECORD gir_Virtual[16] = {};
#endif

#ifdef _DEBUG
// Only for input_bug search purposes in Debug builds
const LONG gn_LogReadCharsMax = 4096; // must be power of 2
wchar_t gs_LogReadChars[gn_LogReadCharsMax*2+1] = L""; // "+1" ��� ASCIIZ
LONG gn_LogReadChars = -1;
#endif


/* Forward declarations */
BOOL IsVisibleRectLocked(COORD& crLocked);
void PatchDialogParentWnd(HWND& hWndParent);


#undef isPressed
#define isPressed(inp) ((user->getKeyState(inp) & 0x8000) == 0x8000)

//110131 ��������� ������ ��������� �� � ExcludedModules
//#include <WinInet.h>
//#pragma comment(lib, "wininet.lib")


#ifdef _DEBUG
extern bool gbShowExeMsgBox;
extern DWORD gnLastShowExeTick;
#define force_print_timings(s) { \
	DWORD w, nCurTick = GetTickCount(); \
	msprintf(szTimingMsg, countof(szTimingMsg), L">>> PID=%u >>> %u >>> %s\n", GetCurrentProcessId(), (nCurTick - gnLastShowExeTick), s); \
	/*OnWriteConsoleW(hTimingHandle, szTimingMsg, lstrlen(szTimingMsg), &w, NULL);*/ UNREFERENCED_PARAMETER(w); \
	gnLastShowExeTick = nCurTick; \
	}
#define print_timings(s) if (gbShowExeMsgBox) { force_print_timings(s); }
#else
#define print_timings(s)
#endif


#ifdef _DEBUG
#define DebugString(x) OutputDebugString(x)
#define DebugStringConSize(x) OutputDebugString(x)
#else
#define DebugString(x) //OutputDebugString(x)
#define DebugStringConSize(x)
#endif


//#ifdef _DEBUG
//	#include <crtdbg.h>
//#else
//	#ifndef _ASSERTE
//		#define _ASSERTE(x)
//	#endif
//#endif

extern HMODULE ghOurModule; // ����� ����� dll'�� (����� ���� �� ��������)
extern DWORD   gnHookMainThreadId;
extern BOOL    gbHooksTemporaryDisabled;
//__declspec( thread )
//static BOOL    gbInShellExecuteEx = FALSE;

//#ifdef USE_INPUT_SEMAPHORE
//HANDLE ghConInSemaphore = NULL;
//#endif

/* ************ Globals for SetHook ************ */
extern HWND    ghConWnd;      // RealConsole
extern HWND    ghConEmuWnd;   // Root! ConEmu window
extern HWND    ghConEmuWndDC; // ConEmu DC window
extern DWORD   gnGuiPID;
extern BOOL    gbWasSucceededInRead;
HDC ghTempHDC = NULL;
GetConsoleWindow_T gfGetRealConsoleWindow = NULL;
//extern HWND WINAPI GetRealConsoleWindow(); // Entry.cpp
extern HANDLE ghCurrentOutBuffer;
HANDLE ghStdOutHandle = NULL;
extern HANDLE ghLastAnsiCapable, ghLastAnsiNotCapable;
HANDLE ghLastConInHandle = NULL, ghLastNotConInHandle = NULL;
/* ************ Globals for SetHook ************ */


/* ************ Globals for Far Hooks ************ */
struct HookModeFar gFarMode = {sizeof(HookModeFar)};
InQueue gInQueue = {};
HANDLE ghConsoleCursorChanged = NULL;
/* ************ Globals for Far Hooks ************ */


/* ************ Globals for clink ************ */
size_t   gcchLastWriteConsoleMax = 0;
wchar_t *gpszLastWriteConsole = NULL;
HMODULE  ghClinkDll = NULL;
call_readline_t gpfnClinkReadLine = NULL;
bool     gbClinkInitialized = false;
DWORD    gnAllowClinkUsage = 0; // cmd.exe only. 0 - ���, 1 - ������ ������ (0.1.1), 2 - ����� ������
/* ************ Globals for clink ************ */

/* ************ Globals for powershell ************ */
bool gbPowerShellMonitorProgress = false;
WORD gnConsolePopupColors = 0x003E;
int  gnPowerShellProgressValue = -1;
/* ************ Globals for powershell ************ */

/* ************ Globals for bash ************ */
bool gbIsBashProcess = false;
/* ************ Globals for bash ************ */

/* ************ Globals for ViM ************ */
bool gbIsVimProcess = false;
bool gbIsVimAnsi = false;
/* ************ Globals for ViM ************ */

/* ************ Globals for HIEW32.EXE ************ */
bool gbIsHiewProcess = false;
/* ************ Globals for HIEW32.EXE ************ */

/* ************ Globals for DosBox.EXE ************ */
bool gbDosBoxProcess = false;
/* ************ Globals for DosBox.EXE ************ */

/* ************ Globals for "Default terminal ************ */
bool gbPrepareDefaultTerminal = false;
ConEmuGuiMapping* gpDefaultTermParm = NULL;

bool isDefaultTerminalEnabled()
{
	if (!gbPrepareDefaultTerminal || !gpDefaultTermParm)
		return false;
	if ((gpDefaultTermParm->cbSize != sizeof(*gpDefaultTermParm)) || (gpDefaultTermParm->nProtocolVersion != CESERVER_REQ_VER))
		return false;
	if (!*gpDefaultTermParm->sConEmuExe || !gpDefaultTermParm->bUseDefaultTerminal)
		return false;
	return true;
}
/* ************ Globals for "Default terminal ************ */

struct ReadConsoleInfo gReadConsoleInfo = {};

int WINAPI OnCompareStringW(LCID Locale, DWORD dwCmpFlags, LPCWSTR lpString1, int cchCount1, LPCWSTR lpString2, int cchCount2);

// ������ ��� cmd.exe (clink)
LONG WINAPI OnRegQueryValueExW(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);


//
//static BOOL WINAPI OnHttpSendRequestA(LPVOID hRequest, LPCSTR lpszHeaders, DWORD dwHeadersLength, LPVOID lpOptional, DWORD dwOptionalLength);
//static BOOL WINAPI OnHttpSendRequestW(LPVOID hRequest, LPCWSTR lpszHeaders, DWORD dwHeadersLength, LPVOID lpOptional, DWORD dwOptionalLength);

//static HookItem HooksFarOnly[] =
//{
////	{OnlstrcmpiA,      "lstrcmpiA",      kernel32, 0},
//	{(void*)OnCompareStringW, "CompareStringW", kernel32},
//
//	/* ************************ */
//	//110131 ��������� ������ ��������� �� � ExcludedModules
//	//{(void*)OnHttpSendRequestA, "HttpSendRequestA", wininet, 0},
//	//{(void*)OnHttpSendRequestW, "HttpSendRequestW", wininet, 0},
//	/* ************************ */
//
//	{0, 0, 0}
//};

// Service functions
//typedef DWORD (WINAPI* GetProcessId_t)(HANDLE Process);
GetProcessId_t gfGetProcessId = NULL;

//BOOL gbShowOnSetForeground = FALSE;



// Forward declarations of the hooks
// Common hooks (except LoadLibrary..., FreeLibrary, GetProcAddress)
BOOL WINAPI OnTrackPopupMenu(HMENU hMenu, UINT uFlags, int x, int y, int nReserved, HWND hWnd, CONST RECT * prcRect);
BOOL WINAPI OnTrackPopupMenuEx(HMENU hmenu, UINT fuFlags, int x, int y, HWND hWnd, LPTPMPARAMS lptpm);
BOOL WINAPI OnShellExecuteExA(LPSHELLEXECUTEINFOA lpExecInfo);
BOOL WINAPI OnShellExecuteExW(LPSHELLEXECUTEINFOW lpExecInfo);
HINSTANCE WINAPI OnShellExecuteA(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd);
HINSTANCE WINAPI OnShellExecuteW(HWND hwnd, LPCWSTR lpOperation, LPCWSTR lpFile, LPCWSTR lpParameters, LPCWSTR lpDirectory, INT nShowCmd);
BOOL WINAPI OnFlashWindow(HWND hWnd, BOOL bInvert);
BOOL WINAPI OnFlashWindowEx(PFLASHWINFO pfwi);
BOOL WINAPI OnSetForegroundWindow(HWND hWnd);
HWND WINAPI OnGetForegroundWindow();
int WINAPI OnCompareStringW(LCID Locale, DWORD dwCmpFlags, LPCWSTR lpString1, int cchCount1, LPCWSTR lpString2, int cchCount2);
DWORD WINAPI OnGetConsoleAliasesW(LPWSTR AliasBuffer, DWORD AliasBufferLength, LPWSTR ExeName);
BOOL WINAPI OnReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
BOOL WINAPI OnReadConsoleA(HANDLE hConsoleInput, LPVOID lpBuffer, DWORD nNumberOfCharsToRead, LPDWORD lpNumberOfCharsRead, LPVOID pInputControl);
BOOL WINAPI OnReadConsoleW(HANDLE hConsoleInput, LPVOID lpBuffer, DWORD nNumberOfCharsToRead, LPDWORD lpNumberOfCharsRead, PCONSOLE_READCONSOLE_CONTROL pInputControl);
BOOL WINAPI OnGetNumberOfConsoleInputEvents(HANDLE hConsoleInput, LPDWORD lpcNumberOfEvents);
BOOL WINAPI OnFlushConsoleInputBuffer(HANDLE hConsoleInput);
BOOL WINAPI OnPeekConsoleInputA(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead);
BOOL WINAPI OnPeekConsoleInputW(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead);
BOOL WINAPI OnReadConsoleInputA(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead);
BOOL WINAPI OnReadConsoleInputW(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead);
BOOL WINAPI OnWriteConsoleInputA(HANDLE hConsoleInput, const INPUT_RECORD *lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsWritten);
BOOL WINAPI OnWriteConsoleInputW(HANDLE hConsoleInput, const INPUT_RECORD *lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsWritten);
//BOOL WINAPI OnPeekConsoleInputAx(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead);
//BOOL WINAPI OnPeekConsoleInputWx(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead);
//BOOL WINAPI OnReadConsoleInputAx(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead);
//BOOL WINAPI OnReadConsoleInputWx(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead);
BOOL WINAPI OnAllocConsole(void);
BOOL WINAPI OnFreeConsole(void);
HWND WINAPI OnGetConsoleWindow();
BOOL WINAPI OnGetConsoleMode(HANDLE hConsoleHandle,LPDWORD lpMode);
BOOL WINAPI OnWriteConsoleOutputA(HANDLE hConsoleOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion);
BOOL WINAPI OnWriteConsoleOutputW(HANDLE hConsoleOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion);
BOOL WINAPI OnSetConsoleTextAttribute(HANDLE hConsoleOutput, WORD wAttributes);
//BOOL WINAPI OnWriteConsoleOutputAx(HANDLE hConsoleOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion);
//BOOL WINAPI OnWriteConsoleOutputWx(HANDLE hConsoleOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion);
BOOL WINAPI OnGetWindowRect(HWND hWnd, LPRECT lpRect);
BOOL WINAPI OnScreenToClient(HWND hWnd, LPPOINT lpPoint);
BOOL WINAPI OnCreateProcessA(LPCSTR lpApplicationName,  LPSTR lpCommandLine,  LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory,  LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
BOOL WINAPI OnCreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
UINT WINAPI OnWinExec(LPCSTR lpCmdLine, UINT uCmdShow);
//BOOL WINAPI OnSetCurrentDirectoryA(LPCSTR lpPathName);
//BOOL WINAPI OnSetCurrentDirectoryW(LPCWSTR lpPathName);

extern HANDLE ghSkipSetThreadContextForThread;
BOOL WINAPI OnSetThreadContext(HANDLE hThread, CONST CONTEXT *lpContext);

HANDLE WINAPI OnOpenFileMappingW(DWORD dwDesiredAccess, BOOL bInheritHandle, LPCWSTR lpName);
LPVOID WINAPI OnMapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap);
BOOL WINAPI OnUnmapViewOfFile(LPCVOID lpBaseAddress);
BOOL WINAPI OnCloseHandle(HANDLE hObject);

#ifdef _DEBUG
HANDLE WINAPI OnCreateNamedPipeW(LPCWSTR lpName, DWORD dwOpenMode, DWORD dwPipeMode, DWORD nMaxInstances,DWORD nOutBufferSize, DWORD nInBufferSize, DWORD nDefaultTimeOut,LPSECURITY_ATTRIBUTES lpSecurityAttributes);
#endif
BOOL WINAPI OnSetConsoleCP(UINT wCodePageID);
BOOL WINAPI OnSetConsoleOutputCP(UINT wCodePageID);
#ifdef _DEBUG
LPVOID WINAPI OnVirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
#endif
BOOL WINAPI OnChooseColorA(LPCHOOSECOLORA lpcc);
BOOL WINAPI OnChooseColorW(LPCHOOSECOLORW lpcc);
//HWND WINAPI OnCreateWindowA(LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
//HWND WINAPI OnCreateWindowW(LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
HWND WINAPI OnCreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
HWND WINAPI OnCreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
BOOL WINAPI OnShowWindow(HWND hWnd, int nCmdShow);
HWND WINAPI OnSetParent(HWND hWndChild, HWND hWndNewParent);
HWND WINAPI OnGetParent(HWND hWnd);
HWND WINAPI OnGetWindow(HWND hWnd, UINT uCmd);
HWND WINAPI OnGetAncestor(HWND hwnd, UINT gaFlags);
int WINAPI OnGetClassNameA(HWND hWnd, LPSTR lpClassName, int nMaxCount);
int WINAPI OnGetClassNameW(HWND hWnd, LPWSTR lpClassName, int nMaxCount);
BOOL WINAPI OnMoveWindow(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint);
BOOL WINAPI OnSetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);
BOOL WINAPI OnGetWindowPlacement(HWND hWnd, WINDOWPLACEMENT *lpwndpl);
BOOL WINAPI OnSetWindowPlacement(HWND hWnd, WINDOWPLACEMENT *lpwndpl);
BOOL WINAPI OnPostMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI OnPostMessageW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI OnSendMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI OnSendMessageW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI OnGetMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
BOOL WINAPI OnGetMessageW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
BOOL WINAPI OnPeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
BOOL WINAPI OnPeekMessageW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
BOOL WINAPI OnSetConsoleKeyShortcuts(BOOL bSet, BYTE bReserveKeys, LPVOID p1, DWORD n1);
HWND WINAPI OnCreateDialogIndirectParamA(HINSTANCE hInstance, LPCDLGTEMPLATE lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM lParamInit);
HWND WINAPI OnCreateDialogIndirectParamW(HINSTANCE hInstance, LPCDLGTEMPLATE lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM lParamInit);
HWND WINAPI OnCreateDialogParamA(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
HWND WINAPI OnCreateDialogParamW(HINSTANCE hInstance, LPCWSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
BOOL WINAPI OnGetCurrentConsoleFont(HANDLE hConsoleOutput, BOOL bMaximumWindow, PCONSOLE_FONT_INFO lpConsoleCurrentFont);
COORD WINAPI OnGetConsoleFontSize(HANDLE hConsoleOutput, DWORD nFont);
HWND WINAPI OnGetActiveWindow();
BOOL WINAPI OnSetMenu(HWND hWnd, HMENU hMenu);
BOOL WINAPI OnSetConsoleMode(HANDLE hConsoleHandle, DWORD dwMode);
HANDLE WINAPI OnCreateConsoleScreenBuffer(DWORD dwDesiredAccess, DWORD dwShareMode, const SECURITY_ATTRIBUTES *lpSecurityAttributes, DWORD dwFlags, LPVOID lpScreenBufferData);
BOOL WINAPI OnSetConsoleActiveScreenBuffer(HANDLE hConsoleOutput);
BOOL WINAPI OnSetConsoleWindowInfo(HANDLE hConsoleOutput, BOOL bAbsolute, const SMALL_RECT *lpConsoleWindow);
BOOL WINAPI OnSetConsoleScreenBufferSize(HANDLE hConsoleOutput, COORD dwSize);
COORD WINAPI OnGetLargestConsoleWindowSize(HANDLE hConsoleOutput);
BOOL WINAPI OnSetConsoleCursorPosition(HANDLE hConsoleOutput, COORD dwCursorPosition);
BOOL WINAPI OnSetConsoleCursorInfo(HANDLE hConsoleOutput, const CONSOLE_CURSOR_INFO *lpConsoleCursorInfo);
INT_PTR WINAPI OnDialogBoxParamW(HINSTANCE hInstance, LPCWSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
HDC WINAPI OnGetDC(HWND hWnd); // user32
HDC WINAPI OnGetDCEx(HWND hWnd, HRGN hrgnClip, DWORD flags); // user32
int WINAPI OnReleaseDC(HWND hWnd, HDC hDC); //user32
int WINAPI OnStretchDIBits(HDC hdc, int XDest, int YDest, int nDestWidth, int nDestHeight, int XSrc, int YSrc, int nSrcWidth, int nSrcHeight, const VOID *lpBits, const BITMAPINFO *lpBitsInfo, UINT iUsage, DWORD dwRop); //gdi32
BOOL WINAPI OnBitBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, HDC hdcSrc, int nXSrc, int nYSrc, DWORD dwRop);
BOOL WINAPI OnStretchBlt(HDC hdcDest, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest, HDC hdcSrc, int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc, DWORD dwRop);

//#ifdef HOOK_ANSI_SEQUENCES
BOOL WINAPI OnWriteConsoleA(HANDLE hConsoleOutput, const VOID *lpBuffer, DWORD nNumberOfCharsToWrite, LPDWORD lpNumberOfCharsWritten, LPVOID lpReserved);
BOOL WINAPI OnWriteConsoleW(HANDLE hConsoleOutput, const VOID *lpBuffer, DWORD nNumberOfCharsToWrite, LPDWORD lpNumberOfCharsWritten, LPVOID lpReserved);
BOOL WINAPI OnWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
BOOL WINAPI OnScrollConsoleScreenBufferA(HANDLE hConsoleOutput, const SMALL_RECT *lpScrollRectangle, const SMALL_RECT *lpClipRectangle, COORD dwDestinationOrigin, const CHAR_INFO *lpFill);
BOOL WINAPI OnScrollConsoleScreenBufferW(HANDLE hConsoleOutput, const SMALL_RECT *lpScrollRectangle, const SMALL_RECT *lpClipRectangle, COORD dwDestinationOrigin, const CHAR_INFO *lpFill);
BOOL WINAPI OnWriteConsoleOutputCharacterA(HANDLE hConsoleOutput, LPCSTR lpCharacter, DWORD nLength, COORD dwWriteCoord, LPDWORD lpNumberOfCharsWritten);
BOOL WINAPI OnWriteConsoleOutputCharacterW(HANDLE hConsoleOutput, LPCWSTR lpCharacter, DWORD nLength, COORD dwWriteCoord, LPDWORD lpNumberOfCharsWritten);
//#ifdef _DEBUG
//BOOL WINAPI OnSetConsoleMode(HANDLE hConsoleHandle, DWORD dwMode);
//#endif
//#endif
DWORD WINAPI OnGetEnvironmentVariableA(LPCSTR lpName, LPSTR lpBuffer, DWORD nSize);
DWORD WINAPI OnGetEnvironmentVariableW(LPCWSTR lpName, LPWSTR lpBuffer, DWORD nSize);
#if 0
LPCH WINAPI OnGetEnvironmentStringsA();
#endif
LPWCH WINAPI OnGetEnvironmentStringsW();



bool InitHooksCommon()
{
#ifndef HOOKS_SKIP_COMMON
	// �������� ����
	HookItem HooksCommon[] =
	{
		/* ***** MOST CALLED ***** */
		#ifndef HOOKS_COMMON_PROCESS_ONLY
		{(void*)OnGetConsoleWindow,     "GetConsoleWindow",     kernel32},
		{(void*)OnGetConsoleMode,		"GetConsoleMode",		kernel32},
		//{(void*)OnWriteConsoleOutputWx,	"WriteConsoleOutputW",  kernel32},
		//{(void*)OnWriteConsoleOutputAx,	"WriteConsoleOutputA",  kernel32},
		{(void*)OnWriteConsoleOutputW,	"WriteConsoleOutputW",  kernel32},
		{(void*)OnWriteConsoleOutputA,	"WriteConsoleOutputA",  kernel32},
		//{(void*)OnPeekConsoleInputWx,	"PeekConsoleInputW",	kernel32},
		//{(void*)OnPeekConsoleInputAx,	"PeekConsoleInputA",	kernel32},
		//{(void*)OnReadConsoleInputWx,	"ReadConsoleInputW",	kernel32},
		//{(void*)OnReadConsoleInputAx,	"ReadConsoleInputA",	kernel32},
		{(void*)OnReadConsoleW,			"ReadConsoleW",			kernel32},
		{(void*)OnReadConsoleA,			"ReadConsoleA",			kernel32},
		{(void*)OnReadFile,				"ReadFile",				kernel32},
		{(void*)OnPeekConsoleInputW,	"PeekConsoleInputW",	kernel32},
		{(void*)OnPeekConsoleInputA,	"PeekConsoleInputA",	kernel32},
		{(void*)OnReadConsoleInputW,	"ReadConsoleInputW",	kernel32},
		{(void*)OnReadConsoleInputA,	"ReadConsoleInputA",	kernel32},
		{(void*)OnWriteConsoleInputA,	"WriteConsoleInputA",	kernel32},
		{(void*)OnWriteConsoleInputW,	"WriteConsoleInputW",	kernel32},
		/* ANSI Escape Sequences SUPPORT */
		//#ifdef HOOK_ANSI_SEQUENCES
		{(void*)OnWriteFile,			"WriteFile",  			kernel32},
		{(void*)OnWriteConsoleA,		"WriteConsoleA",  		kernel32},
		{(void*)OnWriteConsoleW,		"WriteConsoleW",  		kernel32},
		{(void*)OnScrollConsoleScreenBufferA,
										"ScrollConsoleScreenBufferA",
																kernel32},
		{(void*)OnScrollConsoleScreenBufferW,
										"ScrollConsoleScreenBufferW",
																kernel32},
		{(void*)OnWriteConsoleOutputCharacterA,
										"WriteConsoleOutputCharacterA",
																kernel32},
		{(void*)OnWriteConsoleOutputCharacterW,
										"WriteConsoleOutputCharacterW",
																kernel32},
		{(void*)OnSetConsoleMode,		"SetConsoleMode",  		kernel32},
		//#endif
		/* Others console functions */
		{(void*)OnSetConsoleTextAttribute, "SetConsoleTextAttribute", kernel32},
		{(void*)OnSetConsoleKeyShortcuts, "SetConsoleKeyShortcuts", kernel32},
		#endif
		/* ************************ */
		{(void*)OnCreateProcessA,		"CreateProcessA",		kernel32},
		{(void*)OnCreateProcessW,		"CreateProcessW",		kernel32},
		{(void*)OnOpenFileMappingW,		"OpenFileMappingW",		kernel32},
		{(void*)OnMapViewOfFile,		"MapViewOfFile",		kernel32},
		{(void*)OnUnmapViewOfFile,		"UnmapViewOfFile",		kernel32},
		{(void*)OnCloseHandle,			"CloseHandle",			kernel32},
		{(void*)OnSetThreadContext,		"SetThreadContext",		kernel32},
		//{(void*)OnSetCurrentDirectoryA, "SetCurrentDirectoryA", kernel32},
		//{(void*)OnSetCurrentDirectoryW, "SetCurrentDirectoryW", kernel32},
		/* ************************ */
		#ifndef HOOKS_COMMON_PROCESS_ONLY
		{(void*)OnGetConsoleAliasesW,	"GetConsoleAliasesW",	kernel32},
		{(void*)OnAllocConsole,			"AllocConsole",			kernel32},
		{(void*)OnFreeConsole,			"FreeConsole",			kernel32},
		{
			(void*)OnGetNumberOfConsoleInputEvents,
			"GetNumberOfConsoleInputEvents",
			kernel32
		},
		{
			(void*)OnFlushConsoleInputBuffer,
			"FlushConsoleInputBuffer",
			kernel32
		},
		{
			(void*)OnCreateConsoleScreenBuffer,
			"CreateConsoleScreenBuffer",
			kernel32
		},
		{
			(void*)OnSetConsoleActiveScreenBuffer,
			"SetConsoleActiveScreenBuffer",
			kernel32
		},
		{
			(void*)OnSetConsoleWindowInfo,
			"SetConsoleWindowInfo",
			kernel32
		},
		{
			(void*)OnSetConsoleScreenBufferSize,
			"SetConsoleScreenBufferSize",
			kernel32
		},
		{
			(void*)OnGetLargestConsoleWindowSize,
			"GetLargestConsoleWindowSize",
			kernel32
		},
		{
			(void*)OnSetConsoleCursorPosition,
			"SetConsoleCursorPosition",
			kernel32
		},
		{
			(void*)OnSetConsoleCursorInfo,
			"SetConsoleCursorInfo",
			kernel32
		},
		#endif
		{(void*)OnGetEnvironmentVariableA, "GetEnvironmentVariableA", kernel32},
		{(void*)OnGetEnvironmentVariableW, "GetEnvironmentVariableW", kernel32},
		#if 0
		{(void*)OnGetEnvironmentStringsA,  "GetEnvironmentStringsA",  kernel32},
		#endif
		{(void*)OnGetEnvironmentStringsW,  "GetEnvironmentStringsW",  kernel32},
		/* ************************ */
		{(void*)OnGetCurrentConsoleFont, "GetCurrentConsoleFont", kernel32},
		{(void*)OnGetConsoleFontSize,    "GetConsoleFontSize",    kernel32},
		/* ************************ */

		#ifdef _DEBUG
		#ifndef HOOKS_COMMON_PROCESS_ONLY
		{(void*)OnCreateNamedPipeW,		"CreateNamedPipeW",		kernel32},
		#endif
		#endif

		#ifdef _DEBUG
		//#ifdef HOOKS_VIRTUAL_ALLOC
		{(void*)OnVirtualAlloc,			"VirtualAlloc",			kernel32},
		//#endif
		#endif

		// Microsoft bug?
		// http://code.google.com/p/conemu-maximus5/issues/detail?id=60
		#ifndef HOOKS_COMMON_PROCESS_ONLY
		{(void*)OnSetConsoleCP,			"SetConsoleCP",			kernel32},
		{(void*)OnSetConsoleOutputCP,	"SetConsoleOutputCP",	kernel32},
		#endif
		/* ************************ */
		{(void*)OnShellExecuteExA,		"ShellExecuteExA",		shell32},
		{(void*)OnShellExecuteExW,		"ShellExecuteExW",		shell32},
		{(void*)OnShellExecuteA,		"ShellExecuteA",		shell32},
		{(void*)OnShellExecuteW,		"ShellExecuteW",		shell32},
		/* ************************ */
		{(void*)OnChooseColorA,			"ChooseColorA",			comdlg32},
		{(void*)OnChooseColorW,			"ChooseColorW",			comdlg32},
		/* ************************ */
		{0}
	};
	InitHooks(HooksCommon);

	#if 0
	// ��������, ��� ��������� �� �����
	HooksCommon[1].NewAddress = NULL;
	_ASSERTEX(FALSE && "Testing");
	InitHooks(HooksCommon);
	#endif

#endif
	return true;
}

bool InitHooksDefaultTrm()
{
	// ���� ����������� ��� ��������� ConEmu ��� ��������� �� ���������
	HookItem HooksCommon[] =
	{
		{(void*)OnCreateProcessW,		"CreateProcessW",		kernel32},
		// Need to "hook" OnCreateProcessA because it is used in "OnWinExec"
		{(void*)OnCreateProcessA,		"CreateProcessA",		kernel32},
		// Used in some programs, Issue 853
		{(void*)OnWinExec,				"WinExec",				kernel32},
		/* ************************ */
		{0}
	};
	InitHooks(HooksCommon);

	return true;
}

// user32 & gdi32
bool InitHooksUser32()
{
#ifndef HOOKS_SKIP_COMMON
	// �������� ����
	HookItem HooksCommon[] =
	{
		/* ************************ */
		#ifndef HOOKS_COMMON_PROCESS_ONLY
		{(void*)OnTrackPopupMenu,		"TrackPopupMenu",		user32},
		{(void*)OnTrackPopupMenuEx,		"TrackPopupMenuEx",		user32},
		{(void*)OnFlashWindow,			"FlashWindow",			user32},
		{(void*)OnFlashWindowEx,		"FlashWindowEx",		user32},
		{(void*)OnSetForegroundWindow,	"SetForegroundWindow",	user32},
		{(void*)OnGetForegroundWindow,	"GetForegroundWindow",	user32},
		{(void*)OnGetWindowRect,		"GetWindowRect",		user32},
		{(void*)OnScreenToClient,		"ScreenToClient",		user32},
		#endif
		/* ************************ */
		//{(void*)OnCreateWindowA,		"CreateWindowA",		user32}, -- ����� ��������� ���
		//{(void*)OnCreateWindowW,		"CreateWindowW",		user32}, -- ����� ��������� ���
		{(void*)OnCreateWindowExA,		"CreateWindowExA",		user32},
		{(void*)OnCreateWindowExW,		"CreateWindowExW",		user32},
		{(void*)OnShowWindow,			"ShowWindow",			user32},
		{(void*)OnSetParent,			"SetParent",			user32},
		{(void*)OnGetParent,			"GetParent",			user32},
		{(void*)OnGetWindow,			"GetWindow",			user32},
		{(void*)OnGetAncestor,			"GetAncestor",			user32},
		{(void*)OnGetClassNameA,		"GetClassNameA",		user32},
		{(void*)OnGetClassNameW,		"GetClassNameW",		user32},
		{(void*)OnGetActiveWindow,		"GetActiveWindow",		user32},
		{(void*)OnMoveWindow,			"MoveWindow",			user32},
		{(void*)OnSetWindowPos,			"SetWindowPos",			user32},
		{(void*)OnGetWindowPlacement,	"GetWindowPlacement",	user32},
		{(void*)OnSetWindowPlacement,	"SetWindowPlacement",	user32},
		{(void*)OnPostMessageA,			"PostMessageA",			user32},
		{(void*)OnPostMessageW,			"PostMessageW",			user32},
		{(void*)OnSendMessageA,			"SendMessageA",			user32},
		{(void*)OnSendMessageW,			"SendMessageW",			user32},
		{(void*)OnGetMessageA,			"GetMessageA",			user32},
		{(void*)OnGetMessageW,			"GetMessageW",			user32},
		{(void*)OnPeekMessageA,			"PeekMessageA",			user32},
		{(void*)OnPeekMessageW,			"PeekMessageW",			user32},
		{(void*)OnCreateDialogParamA,	"CreateDialogParamA",	user32},
		{(void*)OnCreateDialogParamW,	"CreateDialogParamW",	user32},
		{(void*)OnCreateDialogIndirectParamA, "CreateDialogIndirectParamA", user32},
		{(void*)OnCreateDialogIndirectParamW, "CreateDialogIndirectParamW", user32},
		{(void*)OnDialogBoxParamW,		"DialogBoxParamW",		user32},
		{(void*)OnSetMenu,				"SetMenu",				user32},
		{(void*)OnGetDC,				"GetDC",				user32},
		{(void*)OnGetDCEx,				"GetDCEx",				user32},
		{(void*)OnReleaseDC,			"ReleaseDC",			user32},
		/* ************************ */

		/* ************************ */
		{(void*)OnStretchDIBits,		"StretchDIBits",		gdi32},
		{(void*)OnBitBlt,				"BitBlt",				gdi32},
		{(void*)OnStretchBlt,			"StretchBlt",			gdi32},
		/* ************************ */
		{0}
	};
	InitHooks(HooksCommon);
#endif
	return true;
}

bool InitHooksFar()
{
#ifndef HOOKS_SKIP_COMMON
	// ���������, ��� �� ���? ���� ��� - ����� �� ������� HooksFarOnly
	bool lbIsFar = false;
	wchar_t* pszExe = (wchar_t*)calloc(MAX_PATH+1,sizeof(wchar_t));
	if (pszExe)
	{
		if (GetModuleFileName(NULL, pszExe, MAX_PATH))
		{
			if (IsFarExe(pszExe))
				lbIsFar = true;
		}
		free(pszExe);
	}
	
	if (!lbIsFar)
		return true; // �� ������
	
	HookItem HooksFarOnly[] =
	{
	//	{OnlstrcmpiA,      "lstrcmpiA",      kernel32, 0},
		{(void*)OnCompareStringW, "CompareStringW", kernel32},

		/* ************************ */
		//110131 ��������� ������ ��������� �� � ExcludedModules
		//{(void*)OnHttpSendRequestA, "HttpSendRequestA", wininet, 0},
		//{(void*)OnHttpSendRequestW, "HttpSendRequestW", wininet, 0},
		/* ************************ */

		{0, 0, 0}
	};
	InitHooks(HooksFarOnly);
#endif
	return true;
}

bool InitHooksClink()
{
	if (!gnAllowClinkUsage)
		return true;

	HookItem HooksCmdOnly[] =
	{
		{(void*)OnRegQueryValueExW, "RegQueryValueExW", kernel32},
		{0, 0, 0}
	};

	// ��� Vista � ���� - AdvApi32.dll
	// ������, � WinXP ���� ������ �� ����������� ����������
	OSVERSIONINFO osv = {sizeof(OSVERSIONINFO)};
	GetVersionEx(&osv);
	if ((osv.dwMajorVersion <= 5) || (osv.dwMajorVersion == 6 && osv.dwMinorVersion == 0))
	{
		HooksCmdOnly[0].DllName = advapi32;
	}

	InitHooks(HooksCmdOnly);

	return true;
}



//static HookItem HooksCommon[] =
//{
//	/* ***** MOST CALLED ***** */
////	{(void*)OnGetConsoleWindow,     "GetConsoleWindow",     kernel32}, -- ���� ������ ���. ������� ��� �� �� ������ ��������
//	//{(void*)OnWriteConsoleOutputWx,	"WriteConsoleOutputW",  kernel32},
//	//{(void*)OnWriteConsoleOutputAx,	"WriteConsoleOutputA",  kernel32},
//	{(void*)OnWriteConsoleOutputW,	"WriteConsoleOutputW",  kernel32},
//	{(void*)OnWriteConsoleOutputA,	"WriteConsoleOutputA",  kernel32},
//	/* ************************ */
//	//{(void*)OnPeekConsoleInputWx,	"PeekConsoleInputW",	kernel32},
//	//{(void*)OnPeekConsoleInputAx,	"PeekConsoleInputA",	kernel32},
//	//{(void*)OnReadConsoleInputWx,	"ReadConsoleInputW",	kernel32},
//	//{(void*)OnReadConsoleInputAx,	"ReadConsoleInputA",	kernel32},
//	{(void*)OnPeekConsoleInputW,	"PeekConsoleInputW",	kernel32},
//	{(void*)OnPeekConsoleInputA,	"PeekConsoleInputA",	kernel32},
//	{(void*)OnReadConsoleInputW,	"ReadConsoleInputW",	kernel32},
//	{(void*)OnReadConsoleInputA,	"ReadConsoleInputA",	kernel32},
//	/* ************************ */
//	{(void*)OnCreateProcessA,		"CreateProcessA",		kernel32},
//	{(void*)OnCreateProcessW,		"CreateProcessW",		kernel32},
//	/* ************************ */
//	{(void*)OnGetConsoleAliasesW,	"GetConsoleAliasesW",	kernel32},
//	{(void*)OnAllocConsole,			"AllocConsole",			kernel32},
//	{(void*)OnFreeConsole,			"FreeConsole",			kernel32},
//	{
//		(void*)OnGetNumberOfConsoleInputEvents,
//		"GetNumberOfConsoleInputEvents",
//		kernel32
//	},
//	{
//		(void*)OnCreateConsoleScreenBuffer,
//		"CreateConsoleScreenBuffer",
//		kernel32
//	},
//#ifdef _DEBUG
//	{(void*)OnCreateNamedPipeW,		"CreateNamedPipeW",		kernel32},
//#endif
//#ifdef _DEBUG
//	{(void*)OnVirtualAlloc,			"VirtualAlloc",			kernel32},
//#endif
//	// Microsoft bug?
//	// http://code.google.com/p/conemu-maximus5/issues/detail?id=60
//	{(void*)OnSetConsoleCP,			"SetConsoleCP",			kernel32},
//	{(void*)OnSetConsoleOutputCP,	"SetConsoleOutputCP",	kernel32},
//	/* ************************ */
//	{(void*)OnTrackPopupMenu,		"TrackPopupMenu",		user32},
//	{(void*)OnTrackPopupMenuEx,		"TrackPopupMenuEx",		user32},
//	{(void*)OnFlashWindow,			"FlashWindow",			user32},
//	{(void*)OnFlashWindowEx,		"FlashWindowEx",		user32},
//	{(void*)OnSetForegroundWindow,	"SetForegroundWindow",	user32},
//	{(void*)OnGetWindowRect,		"GetWindowRect",		user32},
//	{(void*)OnScreenToClient,		"ScreenToClient",		user32},
//	/* ************************ */
//	{(void*)OnShellExecuteExA,		"ShellExecuteExA",		shell32},
//	{(void*)OnShellExecuteExW,		"ShellExecuteExW",		shell32},
//	{(void*)OnShellExecuteA,		"ShellExecuteA",		shell32},
//	{(void*)OnShellExecuteW,		"ShellExecuteW",		shell32},
//	/* ************************ */
//	{0}
//};

void __stdcall SetFarHookMode(struct HookModeFar *apFarMode)
{
	if (!apFarMode)
	{
		gFarMode.bFarHookMode = FALSE;
	}
	else if (apFarMode->cbSize != sizeof(HookModeFar))
	{
		_ASSERTE(apFarMode->cbSize == sizeof(HookModeFar));
		/*
		gFarMode.bFarHookMode = FALSE;
		wchar_t szTitle[64], szText[255];
		msprintf(szTitle, SKIPLEN(countof(szTitle)) L"ConEmuHk, PID=%u", GetCurrentProcessId());
		msprintf(szText, SKIPLEN(countof(szText)) L"SetFarHookMode recieved invalid sizeof = %u\nRequired = %u", apFarMode->cbSize, (DWORD)sizeof(HookModeFar));
		Message BoxW(NULL, szText, szTitle, MB_ICONSTOP|MB_SYSTEMMODAL);
		*/
	}
	else
	{
		memmove(&gFarMode, apFarMode, sizeof(gFarMode));
	}
}

//void InitializeConsoleInputSemaphore()
//{
//#ifdef USE_INPUT_SEMAPHORE
//	if (ghConInSemaphore != NULL)
//	{
//		ReleaseConsoleInputSemaphore();
//	}
//	wchar_t szName[128];
//	HWND hConWnd = GetConEmuHWND(2);
//	if (hConWnd != ghConWnd)
//	{
//		_ASSERTE(ghConWnd == hConWnd);
//	}
//	if (hConWnd != NULL)
//	{
//		msprintf(szName, SKIPLEN(countof(szName)) CEINPUTSEMAPHORE, (DWORD)hConWnd);
//		ghConInSemaphore = CreateSemaphore(LocalSecurity(), 1, 1, szName);
//		_ASSERTE(ghConInSemaphore != NULL);
//	}
//#endif
//}
//
//void ReleaseConsoleInputSemaphore()
//{
//#ifdef USE_INPUT_SEMAPHORE
//	if (ghConInSemaphore != NULL)
//	{
//		CloseHandle(ghConInSemaphore);
//		ghConInSemaphore = NULL;
//	}
//#endif
//}

// ��� ������� ����� ������� �� DllMain
BOOL StartupHooks(HMODULE ahOurDll)
{
	//HLOG0("StartupHooks",0);
#ifdef _DEBUG
	// ���������� ���� ��� ������ ���� ��������������� � DllMain
	_ASSERTE(gbAttachGuiClient || gbDosBoxProcess || gbPrepareDefaultTerminal || (ghConWnd != NULL && ghConWnd == GetRealConsoleWindow()));
	wchar_t sClass[128];
	if (ghConWnd)
	{
		user->getClassNameW(ghConWnd, sClass, countof(sClass));
		_ASSERTE(isConsoleClass(sClass));
	}

	wchar_t szTimingMsg[512]; HANDLE hTimingHandle = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

	// -- ghConEmuWnd ��� ������ ���� ���������� � DllMain!!!
	//gbInShellExecuteEx = FALSE;

	WARNING("�������� �� �������� gdwServerPID");

	// ����� LoadLibrary. Kernel-�� ������ ��� ����� ���������� (static link) � �����
	// windows ����������, �� ��� shell32 - �� �����������, � ��� ����� ���� �������������������
	ghKernel32 = LoadLibrary(kernel32);
	// user32/shell32/advapi32 ����� �� ����� ����� ������ ���������, �� ���������, ���� ��� ��� �� ������������
	if (!ghUser32)
	{
		ghUser32 = GetModuleHandle(user32);
		if (ghUser32) ghUser32 = LoadLibrary(user32); // ���� ����������� - ��������� �������
	}
	ghShell32 = GetModuleHandle(shell32);
	if (ghShell32) ghShell32 = LoadLibrary(shell32); // ���� ����������� - ��������� �������
	ghAdvapi32 = GetModuleHandle(advapi32);
	if (ghAdvapi32) ghAdvapi32 = LoadLibrary(advapi32); // ���� ����������� - ��������� �������
	ghComdlg32 = GetModuleHandle(comdlg32);
	if (ghComdlg32) ghComdlg32 = LoadLibrary(comdlg32); // ���� ����������� - ��������� �������

	if (ghKernel32)
		gfGetProcessId = (GetProcessId_t)GetProcAddress(ghKernel32, "GetProcessId");

	if (gbPrepareDefaultTerminal)
	{
		HLOG1("StartupHooks.InitHooks",0);
		InitHooksDefaultTrm();
		HLOGEND1();
	}
	else
	{
		// �����
		HLOG1("StartupHooks.InitHooks",0);
		InitHooksCommon();
		HLOGEND1();

		// user32 & gdi32
		HLOG1_("StartupHooks.InitHooks",1);
		InitHooksUser32();
		HLOGEND1();
		
		// Far only functions
		HLOG1_("StartupHooks.InitHooks",2);
		InitHooksFar();
		HLOGEND1();

		// Cmd.exe only functions
		if (gnAllowClinkUsage)
		{
			HLOG1_("StartupHooks.InitHooks",3);
			InitHooksClink();
			HLOGEND1();
		}

		// ������
		HLOG1_("StartupHooks.InitHooks",4);
		InitHooksReg();
		HLOGEND1();
	}

#if 0
	HLOG1_("InitHooksSort",0);
	InitHooksSort();
	HLOGEND1();
#endif

	print_timings(L"SetAllHooks");

	// ������ ����� ���������� ������
	HLOG1_("SetAllHooks",0);
	bool lbRc = SetAllHooks(ahOurDll, NULL, TRUE);
	HLOGEND1();

	print_timings(L"SetAllHooks - done");

	//HLOGEND(); // StartupHooks - done

	return lbRc;
}




//void ShutdownHooks()
//{
//	UnsetAllHooks();
//	
//	//// ��������� ������ � ��������
//	//DoneHooksReg();
//
//	// ���������� ��������� ��������
//	if (ghKernel32)
//	{
//		FreeLibrary(ghKernel32);
//		ghKernel32 = NULL;
//	}
//	if (ghUser32)
//	{
//		FreeLibrary(ghUser32);
//		ghUser32 = NULL;
//	}
//	if (ghShell32)
//	{
//		FreeLibrary(ghShell32);
//		ghShell32 = NULL;
//	}
//	if (ghAdvapi32)
//	{
//		FreeLibrary(ghAdvapi32);
//		ghAdvapi32 = NULL;
//	}
//	if (ghComdlg32)
//	{
//		FreeLibrary(ghComdlg32);
//		ghComdlg32 = NULL;
//	}
//}



// Forward
BOOL GuiSetForeground(HWND hWnd);
void GuiFlashWindow(BOOL bSimple, HWND hWnd, BOOL bInvert, DWORD dwFlags, UINT uCount, DWORD dwTimeout);
//BOOL PrepareExecuteParmsA(enum CmdOnCreateType aCmd, LPCSTR asAction, DWORD anFlags, 
//				LPCSTR asFile, LPCSTR asParam,
//				HANDLE hStdIn, HANDLE hStdOut, HANDLE hStdErr,
//				LPSTR* psFile, LPSTR* psParam, DWORD& nImageSubsystem, DWORD& nImageBits);
//BOOL PrepareExecuteParmsW(enum CmdOnCreateType aCmd, LPCWSTR asAction, DWORD anFlags, 
//				LPCWSTR asFile, LPCWSTR asParam,
//				HANDLE hStdIn, HANDLE hStdOut, HANDLE hStdErr,
//				LPWSTR* psFile, LPWSTR* psParam, DWORD& nImageSubsystem, DWORD& nImageBits);
//wchar_t* str2wcs(const char* psz, UINT anCP);
//wchar_t* wcs2str(const char* psz, UINT anCP);


BOOL WINAPI OnCreateProcessA(LPCSTR lpApplicationName,  LPSTR lpCommandLine,  LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory,  LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
	typedef BOOL (WINAPI* OnCreateProcessA_t)(LPCSTR lpApplicationName,  LPSTR lpCommandLine,  LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory,  LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
	ORIGINALFAST(CreateProcessA);
	BOOL bMainThread = FALSE; // ����� �� �����
	BOOL lbRc = FALSE;
	DWORD dwErr = 0;
	

	if (ph && ph->PreCallBack)
	{
		SETARGS10(&lbRc, lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);

		// ���� ������� ���������� FALSE - �������� ������ �� ����� �������
		if (!ph->PreCallBack(&args))
			return lbRc;
	}

	CShellProc* sp = new CShellProc();
	if (!sp || !sp->OnCreateProcessA(&lpApplicationName, (LPCSTR*)&lpCommandLine, &lpCurrentDirectory, &dwCreationFlags, lpStartupInfo))
	{
		delete sp;
		SetLastError(ERROR_FILE_NOT_FOUND);
		return FALSE;
	}
	if ((dwCreationFlags & CREATE_SUSPENDED) == 0)
	{
		DebugString(L"CreateProcessA without CREATE_SUSPENDED Flag!\n");
	}

	lbRc = F(CreateProcessA)(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	dwErr = GetLastError();


	// ���� lbParamsChanged == TRUE - �� �������� ����������� ConEmuC.exe
	sp->OnCreateProcessFinished(lbRc, lpProcessInformation);
	delete sp;


	if (ph && ph->PostCallBack)
	{
		SETARGS10(&lbRc, lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
		ph->PostCallBack(&args);
	}
	
	SetLastError(dwErr);
	return lbRc;
}

BOOL WINAPI OnCreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
	typedef BOOL (WINAPI* OnCreateProcessW_t)(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
	ORIGINALFAST(CreateProcessW);
	BOOL bMainThread = FALSE; // ����� �� �����
	BOOL lbRc = FALSE;
	DWORD dwErr = 0;

	if (ph && ph->PreCallBack)
	{
		SETARGS10(&lbRc, lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);

		// ���� ������� ���������� FALSE - �������� ������ �� ����� �������
		if (!ph->PreCallBack(&args))
			return lbRc;
	}

	CShellProc* sp = new CShellProc();
	if (!sp || !sp->OnCreateProcessW(&lpApplicationName, (LPCWSTR*)&lpCommandLine, &lpCurrentDirectory, &dwCreationFlags, lpStartupInfo))
	{
		delete sp;
		SetLastError(ERROR_FILE_NOT_FOUND);
		return FALSE;
	}

	if ((dwCreationFlags & CREATE_SUSPENDED) == 0)
	{
		DebugString(L"CreateProcessW without CREATE_SUSPENDED Flag!\n");
	}

	#ifdef _DEBUG
	SetLastError(0);
	#endif

	#ifdef SHOWCREATEPROCESSTICK
	wchar_t szTimingMsg[512]; HANDLE hTimingHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	force_print_timings(L"CreateProcessW");
	#endif

	lbRc = F(CreateProcessW)(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	dwErr = GetLastError();

	#ifdef SHOWCREATEPROCESSTICK
	force_print_timings(L"CreateProcessW - done");
	#endif

	// ���� lbParamsChanged == TRUE - �� �������� ����������� ConEmuC.exe
	sp->OnCreateProcessFinished(lbRc, lpProcessInformation);
	delete sp;

	#ifdef SHOWCREATEPROCESSTICK
	force_print_timings(L"OnCreateProcessFinished - done");
	#endif


	if (ph && ph->PostCallBack)
	{
		SETARGS10(&lbRc, lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
		ph->PostCallBack(&args);
	}

	SetLastError(dwErr);
	return lbRc;
}

UINT WINAPI OnWinExec(LPCSTR lpCmdLine, UINT uCmdShow)
{
	if (!lpCmdLine || !*lpCmdLine)
	{
		_ASSERTEX(lpCmdLine && *lpCmdLine);
		return 0;
	}

	STARTUPINFOA si = {sizeof(si)};
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = uCmdShow;
	DWORD nCreateFlags = NORMAL_PRIORITY_CLASS;
	PROCESS_INFORMATION pi = {};

	int nLen = lstrlenA(lpCmdLine);
	LPSTR pszCmd = (char*)malloc(nLen+1);
	if (!pszCmd)
	{
		return 0; // out of memory
	}
	lstrcpynA(pszCmd, lpCmdLine, nLen+1);

	UINT nRc = 0;
	BOOL bRc = OnCreateProcessA(NULL, pszCmd, NULL, NULL, FALSE, nCreateFlags, NULL, NULL, &si, &pi);

	free(pszCmd);

	if (bRc)
	{
		// If the function succeeds, the return value is greater than 31.
		nRc = 32;
	}
	else
	{
		nRc = GetLastError();
		if (nRc != ERROR_BAD_FORMAT && nRc != ERROR_FILE_NOT_FOUND && nRc != ERROR_PATH_NOT_FOUND)
			nRc = 0;
	}

	return nRc;

#if 0
	typedef BOOL (WINAPI* OnWinExec_t)(LPCSTR lpCmdLine, UINT uCmdShow);
	ORIGINALFAST(CreateProcessA);
	BOOL bMainThread = FALSE; // ����� �� �����
	BOOL lbRc = FALSE;
	DWORD dwErr = 0;
	
	CShellProc* sp = new CShellProc();
	if (!sp || !sp->OnCreateProcessA(NULL, &lpCmdLine, NULL, &nCreateFlags, sa))
	{
		delete sp;
		SetLastError(ERROR_FILE_NOT_FOUND);
		return ERROR_FILE_NOT_FOUND;
	}

	if ((nCreateFlags & CREATE_SUSPENDED) == 0)
	{
		DebugString(L"CreateProcessA without CREATE_SUSPENDED Flag!\n");
	}

	lbRc = F(CreateProcessA)(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	dwErr = GetLastError();


	// ���� lbParamsChanged == TRUE - �� �������� ����������� ConEmuC.exe
	sp->OnCreateProcessFinished(lbRc, lpProcessInformation);
	delete sp;


	if (ph && ph->PostCallBack)
	{
		SETARGS10(&lbRc, lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
		ph->PostCallBack(&args);
	}
	
	SetLastError(dwErr);
	return lbRc;
#endif
}

HANDLE WINAPI OnOpenFileMappingW(DWORD dwDesiredAccess, BOOL bInheritHandle, LPCWSTR lpName)
{
	typedef HANDLE (WINAPI* OnOpenFileMappingW_t)(DWORD dwDesiredAccess, BOOL bInheritHandle, LPCWSTR lpName);
	ORIGINALFAST(OpenFileMappingW);
	//BOOL bMainThread = FALSE; // ����� �� �����
	HANDLE hRc = FALSE;

	if (ghConEmuWndDC && lpName && *lpName)
	{
		/**
		* Share name to search for
		* Two replacements:
		*   %d sizeof(AnnotationInfo) - compatibility/versioning field.
		*   %d console window handle
		*/
		wchar_t szTrueColorMap[64];
		// #define  L"Console_annotationInfo_%x_%x"
		msprintf(szTrueColorMap, countof(szTrueColorMap), AnnotationShareName, (DWORD)sizeof(AnnotationInfo), ghConEmuWndDC);
		// ��� ������� ������� ������� ��� TrueColor - ������� � ����� ���������� �������
		if (lstrcmpi(lpName, szTrueColorMap) == 0)
		{
			RequestLocalServerParm Parm = {(DWORD)sizeof(Parm), slsf_RequestTrueColor|slsf_GetCursorEvent};
			if (RequestLocalServer(&Parm) == 0)
			{
				if (Parm.pAnnotation)
				{
					gpAnnotationHeader = Parm.pAnnotation;
					hRc = (HANDLE)Parm.pAnnotation;
					goto wrap;
				}
				else
				{
					WARNING("��������� ��������� AnnotationShareName � ����");
				}

				if (ghConsoleCursorChanged && (ghConsoleCursorChanged != Parm.hCursorChangeEvent))
					CloseHandle(ghConsoleCursorChanged);
				ghConsoleCursorChanged = Parm.hCursorChangeEvent;
			}
		}
	}

	hRc = F(OpenFileMappingW)(dwDesiredAccess, bInheritHandle, lpName);

wrap:
	return hRc;
}

LPVOID WINAPI OnMapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap)
{
	typedef LPVOID (WINAPI* OnMapViewOfFile_t)(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap);
	ORIGINALFAST(MapViewOfFile);
	//BOOL bMainThread = FALSE; // ����� �� �����
	LPVOID ptr = NULL;

	if (gpAnnotationHeader && (hFileMappingObject == (HANDLE)gpAnnotationHeader))
	{
		_ASSERTE(!dwFileOffsetHigh && !dwFileOffsetLow && !dwNumberOfBytesToMap);
		ptr = gpAnnotationHeader;
	}
	else
	{
		ptr = F(MapViewOfFile)(hFileMappingObject, dwDesiredAccess, dwFileOffsetHigh, dwFileOffsetLow, dwNumberOfBytesToMap);
	}

	return ptr;
}

BOOL WINAPI OnUnmapViewOfFile(LPCVOID lpBaseAddress)
{
	typedef BOOL (WINAPI* OnUnmapViewOfFile_t)(LPCVOID lpBaseAddress);
	ORIGINALFAST(UnmapViewOfFile);
	//BOOL bMainThread = FALSE; // ����� �� �����
    BOOL lbRc = FALSE;

	if (gpAnnotationHeader && (lpBaseAddress == gpAnnotationHeader))
	{
		lbRc = TRUE;
	}
	else
	{
    	lbRc = F(UnmapViewOfFile)(lpBaseAddress);
	}

    return lbRc;
}

BOOL WINAPI OnCloseHandle(HANDLE hObject)
{
	typedef BOOL (WINAPI* OnCloseHandle_t)(HANDLE hObject);
	ORIGINALFAST(CloseHandle);
	//BOOL bMainThread = FALSE; // ����� �� �����
	BOOL lbRc = FALSE;

	if (ghLastAnsiCapable && (ghLastAnsiCapable == hObject))
	{
		ghLastAnsiCapable = NULL;
	}
	if (ghLastAnsiNotCapable && (ghLastAnsiNotCapable == hObject))
	{
		ghLastAnsiNotCapable = NULL;
	}
	if (ghLastConInHandle && (ghLastConInHandle == hObject))
	{
		ghLastConInHandle = NULL;
	}
	if (ghLastNotConInHandle && (ghLastNotConInHandle == hObject))
	{
		ghLastNotConInHandle = NULL;
	}

	if (gpAnnotationHeader && (hObject == (HANDLE)gpAnnotationHeader))
	{
		lbRc = TRUE;
	}
	else
	{
		lbRc = F(CloseHandle)(hObject);
	}

	if (ghSkipSetThreadContextForThread == hObject)
		ghSkipSetThreadContextForThread = NULL;

	return lbRc;
}

#if 0
BOOL WINAPI OnSetCurrentDirectoryA(LPCSTR lpPathName)
{
	typedef BOOL (WINAPI* OnSetCurrentDirectoryA_t)(LPCSTR lpPathName);
	ORIGINALFAST(SetCurrentDirectoryA);
	BOOL bMainThread = FALSE; // ����� �� �����
	BOOL lbRc = FALSE;

	lbRc = F(SetCurrentDirectoryA)(lpPathName);

	return lbRc;
}

BOOL WINAPI OnSetCurrentDirectoryW(LPCWSTR lpPathName)
{
	typedef BOOL (WINAPI* OnSetCurrentDirectoryW_t)(LPCWSTR lpPathName);
	ORIGINALFAST(SetCurrentDirectoryW);
	BOOL bMainThread = FALSE; // ����� �� �����
	BOOL lbRc = FALSE;

	lbRc = F(SetCurrentDirectoryW)(lpPathName);

	return lbRc;
}
#endif

BOOL WINAPI OnSetThreadContext(HANDLE hThread, CONST CONTEXT *lpContext)
{
	typedef BOOL (WINAPI* OnSetThreadContext_t)(HANDLE hThread, CONST CONTEXT *lpContext);
	ORIGINALFAST(SetThreadContext);
	//BOOL bMainThread = FALSE; // ����� �� �����
	BOOL lbRc = FALSE;

	if (ghSkipSetThreadContextForThread && (hThread == ghSkipSetThreadContextForThread))
	{
		lbRc = FALSE;
		SetLastError(ERROR_INVALID_HANDLE);
	}
	else
	{
		lbRc = F(SetThreadContext)(hThread, lpContext);
	}

	return lbRc;
}

BOOL WINAPI OnTrackPopupMenu(HMENU hMenu, UINT uFlags, int x, int y, int nReserved, HWND hWnd, CONST RECT * prcRect)
{
	typedef BOOL (WINAPI* OnTrackPopupMenu_t)(HMENU hMenu, UINT uFlags, int x, int y, int nReserved, HWND hWnd, CONST RECT * prcRect);
	ORIGINALFASTEX(TrackPopupMenu,NULL);
#ifndef CONEMU_MINIMAL
	WCHAR szMsg[128]; msprintf(szMsg, SKIPLEN(countof(szMsg)) L"TrackPopupMenu(hwnd=0x%08X)\n", (DWORD)hWnd);
	DebugString(szMsg);
#endif

	if (ghConEmuWndDC)
	{
		// ���������� "�������" ������ ���������� ����, �.�. �� ���� ���� ����
		GuiSetForeground(hWnd);

		if (gFarMode.cbSize == sizeof(gFarMode) && gFarMode.bPopupMenuPos)
		{
			gFarMode.bPopupMenuPos = FALSE; // ����������
			POINT pt; GetCursorPos(&pt);
			x = pt.x; y = pt.y;
		}
	}

	BOOL lbRc = FALSE;
	if (F(TrackPopupMenu) != NULL)
		lbRc = F(TrackPopupMenu)(hMenu, uFlags, x, y, nReserved, hWnd, prcRect);
	return lbRc;
}


BOOL WINAPI OnTrackPopupMenuEx(HMENU hmenu, UINT fuFlags, int x, int y, HWND hWnd, LPTPMPARAMS lptpm)
{
	typedef BOOL (WINAPI* OnTrackPopupMenuEx_t)(HMENU hmenu, UINT fuFlags, int x, int y, HWND hWnd, LPTPMPARAMS lptpm);
	ORIGINALFASTEX(TrackPopupMenuEx,NULL);
#ifndef CONEMU_MINIMAL
	WCHAR szMsg[128]; msprintf(szMsg, SKIPLEN(countof(szMsg)) L"TrackPopupMenuEx(hwnd=0x%08X)\n", (DWORD)hWnd);
	DebugString(szMsg);
#endif

	if (ghConEmuWndDC)
	{
		// ���������� "�������" ������ ���������� ����, �.�. �� ���� ���� ����
		GuiSetForeground(hWnd);

		if (gFarMode.cbSize == sizeof(gFarMode) && gFarMode.bPopupMenuPos)
		{
			gFarMode.bPopupMenuPos = FALSE; // ����������
			POINT pt; GetCursorPos(&pt);
			x = pt.x; y = pt.y;
		}
	}

	BOOL lbRc = FALSE;
	if (F(TrackPopupMenuEx) != NULL)
		lbRc = F(TrackPopupMenuEx)(hmenu, fuFlags, x, y, hWnd, lptpm);
	return lbRc;
}

#ifndef SEE_MASK_NOZONECHECKS
#define SEE_MASK_NOZONECHECKS      0x00800000
#endif


BOOL WINAPI OnShellExecuteExA(LPSHELLEXECUTEINFOA lpExecInfo)
{
	typedef BOOL (WINAPI* OnShellExecuteExA_t)(LPSHELLEXECUTEINFOA lpExecInfo);
	ORIGINALFASTEX(ShellExecuteExA,NULL);
	if (!F(ShellExecuteExA))
	{
		SetLastError(ERROR_INVALID_FUNCTION);
		return FALSE;
	}

	//LPSHELLEXECUTEINFOA lpNew = NULL;
	//DWORD dwProcessID = 0;
	//wchar_t* pszTempParm = NULL;
	//wchar_t szComSpec[MAX_PATH+1], szConEmuC[MAX_PATH+1]; szComSpec[0] = szConEmuC[0] = 0;
	//LPSTR pszTempApp = NULL, pszTempArg = NULL;
	//lpNew = (LPSHELLEXECUTEINFOA)malloc(lpExecInfo->cbSize);
	//memmove(lpNew, lpExecInfo, lpExecInfo->cbSize);
	
	CShellProc* sp = new CShellProc();
	if (!sp || !sp->OnShellExecuteExA(&lpExecInfo))
	{
		delete sp;
		SetLastError(ERROR_FILE_NOT_FOUND);
		return FALSE;
	}

	//// Under ConEmu only!
	//if (ghConEmuWndDC)
	//{
	//	if (!lpNew->hwnd || lpNew->hwnd == GetConsoleWindow())
	//		lpNew->hwnd = ghConEmuWnd;

	//	HANDLE hDummy = NULL;
	//	DWORD nImageSubsystem = 0, nImageBits = 0;
	//	if (PrepareExecuteParmsA(eShellExecute, lpNew->lpVerb, lpNew->fMask, 
	//			lpNew->lpFile, lpNew->lpParameters,
	//			hDummy, hDummy, hDummy,
	//			&pszTempApp, &pszTempArg, nImageSubsystem, nImageBits))
	//	{
	//		// ������
	//		lpNew->lpFile = pszTempApp;
	//		lpNew->lpParameters = pszTempArg;
	//	}
	//}
	
	BOOL lbRc;

	//if (gFarMode.bFarHookMode && gFarMode.bShellNoZoneCheck)
	//	lpExecInfo->fMask |= SEE_MASK_NOZONECHECKS;
		
	//gbInShellExecuteEx = TRUE;

	lbRc = F(ShellExecuteExA)(lpExecInfo);
	DWORD dwErr = GetLastError();
	
	//if (lbRc && gfGetProcessId && lpNew->hProcess)
	//{
	//	dwProcessID = gfGetProcessId(lpNew->hProcess);
	//}

	//#ifdef _DEBUG
	//
	//	if (lpNew->lpParameters)
	//	{
	//		DebugStringW(L"After ShellExecuteEx\n");
	//		DebugStringA(lpNew->lpParameters);
	//		DebugStringW(L"\n");
	//	}
	//
	//	if (lbRc && dwProcessID)
	//	{
	//		wchar_t szDbgMsg[128]; msprintf(szDbgMsg, SKIPLEN(countof(szDbgMsg)) L"Process created: %u\n", dwProcessID);
	//		DebugStringW(szDbgMsg);
	//	}
	//
	//#endif
	//lpExecInfo->hProcess = lpNew->hProcess;
	//lpExecInfo->hInstApp = lpNew->hInstApp;
	sp->OnShellFinished(lbRc, lpExecInfo->hInstApp, lpExecInfo->hProcess);
	delete sp;

	//if (pszTempParm)
	//	free(pszTempParm);
	//if (pszTempApp)
	//	free(pszTempApp);
	//if (pszTempArg)
	//	free(pszTempArg);

	//free(lpNew);
	//gbInShellExecuteEx = FALSE;
	SetLastError(dwErr);
	return lbRc;
}

//BOOL OnShellExecuteExW_SEH(OnShellExecuteExW_t f, LPSHELLEXECUTEINFOW lpExecInfo, BOOL* pbRc)
//{
//	BOOL lbOk = FALSE;
//	SAFETRY
//	{
//		*pbRc = f(lpExecInfo);
//		lbOk = TRUE;
//	} SAFECATCH
//	{
//		lbOk = FALSE;
//	}
//	return lbOk;
//}
BOOL WINAPI OnShellExecuteExW(LPSHELLEXECUTEINFOW lpExecInfo)
{
	typedef BOOL (WINAPI* OnShellExecuteExW_t)(LPSHELLEXECUTEINFOW lpExecInfo);
	ORIGINALFASTEX(ShellExecuteExW,NULL);
	if (!F(ShellExecuteExW))
	{
		SetLastError(ERROR_INVALID_FUNCTION);
		return FALSE;
	}
	#ifdef _DEBUG
	BOOL bMainThread = (GetCurrentThreadId() == gnHookMainThreadId);
	#endif

	CShellProc* sp = new CShellProc();
	if (!sp || !sp->OnShellExecuteExW(&lpExecInfo))
	{
		delete sp;
		SetLastError(ERROR_FILE_NOT_FOUND);
		return FALSE;
	}

	BOOL lbRc = FALSE;

	lbRc = F(ShellExecuteExW)(lpExecInfo);
	DWORD dwErr = GetLastError();

	sp->OnShellFinished(lbRc, lpExecInfo->hInstApp, lpExecInfo->hProcess);
	delete sp;

	SetLastError(dwErr);
	return lbRc;
}


HINSTANCE WINAPI OnShellExecuteA(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd)
{
	typedef HINSTANCE(WINAPI* OnShellExecuteA_t)(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd);
	ORIGINALFASTEX(ShellExecuteA,NULL);
	if (!F(ShellExecuteA))
	{
		SetLastError(ERROR_INVALID_FUNCTION);
		return FALSE;
	}

	if (ghConEmuWndDC)
	{
		if (!hwnd || hwnd == GetConsoleWindow())
			hwnd = ghConEmuWnd;
	}
	
	//gbInShellExecuteEx = TRUE;
	CShellProc* sp = new CShellProc();
	if (!sp || !sp->OnShellExecuteA(&lpOperation, &lpFile, &lpParameters, &lpDirectory, NULL, (DWORD*)&nShowCmd))
	{
		delete sp;
		SetLastError(ERROR_FILE_NOT_FOUND);
		return (HINSTANCE)ERROR_FILE_NOT_FOUND;
	}

	HINSTANCE lhRc;
	lhRc = F(ShellExecuteA)(hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);
	DWORD dwErr = GetLastError();
	
	sp->OnShellFinished(((INT_PTR)lhRc > 32), lhRc, NULL); //-V112
	delete sp;

	//gbInShellExecuteEx = FALSE;
	SetLastError(dwErr);
	return lhRc;
}

HINSTANCE WINAPI OnShellExecuteW(HWND hwnd, LPCWSTR lpOperation, LPCWSTR lpFile, LPCWSTR lpParameters, LPCWSTR lpDirectory, INT nShowCmd)
{
	typedef HINSTANCE(WINAPI* OnShellExecuteW_t)(HWND hwnd, LPCWSTR lpOperation, LPCWSTR lpFile, LPCWSTR lpParameters, LPCWSTR lpDirectory, INT nShowCmd);
	ORIGINALFASTEX(ShellExecuteW,NULL);
	if (!F(ShellExecuteW))
	{
		SetLastError(ERROR_INVALID_FUNCTION);
		return FALSE;
	}

	if (ghConEmuWndDC)
	{
		if (!hwnd || hwnd == GetConsoleWindow())
			hwnd = ghConEmuWnd;
	}
	
	//gbInShellExecuteEx = TRUE;
	CShellProc* sp = new CShellProc();
	if (!sp || !sp->OnShellExecuteW(&lpOperation, &lpFile, &lpParameters, &lpDirectory, NULL, (DWORD*)&nShowCmd))
	{
		delete sp;
		SetLastError(ERROR_FILE_NOT_FOUND);
		return (HINSTANCE)ERROR_FILE_NOT_FOUND;
	}

	HINSTANCE lhRc;
	lhRc = F(ShellExecuteW)(hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);
	DWORD dwErr = GetLastError();
	
	sp->OnShellFinished(((INT_PTR)lhRc > 32), lhRc, NULL); //-V112
	delete sp;

	//gbInShellExecuteEx = FALSE;
	SetLastError(dwErr);
	return lhRc;
}


BOOL WINAPI OnFlashWindow(HWND hWnd, BOOL bInvert)
{
	typedef BOOL (WINAPI* OnFlashWindow_t)(HWND hWnd, BOOL bInvert);
	ORIGINALFASTEX(FlashWindow,NULL);

	if (ghConEmuWndDC)
	{
		GuiFlashWindow(TRUE, hWnd, bInvert, 0,0,0);
		return TRUE;
	}

	BOOL lbRc = NULL;
	if (F(FlashWindow) != NULL)
		lbRc = F(FlashWindow)(hWnd, bInvert);
	return lbRc;
}

BOOL WINAPI OnFlashWindowEx(PFLASHWINFO pfwi)
{
	typedef BOOL (WINAPI* OnFlashWindowEx_t)(PFLASHWINFO pfwi);
	ORIGINALFASTEX(FlashWindowEx,NULL);

	if (ghConEmuWndDC)
	{
		GuiFlashWindow(FALSE, pfwi->hwnd, 0, pfwi->dwFlags, pfwi->uCount, pfwi->dwTimeout);
		return TRUE;
	}

	BOOL lbRc = FALSE;
	if (F(FlashWindowEx) != NULL)
		lbRc = F(FlashWindowEx)(pfwi);
	return lbRc;
}

BOOL WINAPI OnGetWindowRect(HWND hWnd, LPRECT lpRect)
{
	typedef BOOL (WINAPI* OnGetWindowRect_t)(HWND hWnd, LPRECT lpRect);
	ORIGINALFASTEX(GetWindowRect,NULL);
	BOOL lbRc = FALSE;

	if ((hWnd == ghConWnd) && ghConEmuWndDC)
	{
		//EMenu gui mode issues (center in window). "Remove" Far window from mouse cursor.
		hWnd = ghConEmuWndDC;
	}

	if (F(GetWindowRect) != NULL)
		lbRc = F(GetWindowRect)(hWnd, lpRect);
	//if (ghConEmuWndDC && lpRect)
	//{
	//	//EMenu text mode issues. "Remove" Far window from mouse cursor.
	//	POINT ptCur = {0};
	//	GetCursorPos(&ptCur);
	//	lpRect->left += ptCur.x;
	//	lpRect->right += ptCur.x;
	//	lpRect->top += ptCur.y;
	//	lpRect->bottom += ptCur.y;
	//}

	_ASSERTRESULT(lbRc);
	return lbRc;
}


BOOL WINAPI OnScreenToClient(HWND hWnd, LPPOINT lpPoint)
{
	typedef BOOL (WINAPI* OnScreenToClient_t)(HWND hWnd, LPPOINT lpPoint);
	ORIGINALFASTEX(ScreenToClient,NULL);
	BOOL lbRc = FALSE;

	if (F(ScreenToClient) != NULL)
		lbRc = F(ScreenToClient)(hWnd, lpPoint);

	if (lbRc && ghConEmuWndDC && lpPoint && (hWnd == ghConWnd))
	{
		//EMenu text mode issues. "Remove" Far window from mouse cursor.
		lpPoint->x = lpPoint->y = -1;
	}

	return lbRc;
}



BOOL WINAPI OnShowWindow(HWND hWnd, int nCmdShow)
{
	typedef BOOL (WINAPI* OnShowWindow_t)(HWND hWnd, int nCmdShow);
	ORIGINALFASTEX(ShowWindow,NULL);
	BOOL lbRc = FALSE, lbGuiAttach = FALSE, lbInactiveTab = FALSE;
	static bool bShowWndCalled = false;
	
	if (ghConEmuWndDC && (hWnd == ghConEmuWndDC || hWnd == ghConWnd))
	{
		#ifdef _DEBUG
		if (hWnd == ghConEmuWndDC)
		{
			static bool bShowWarned = false;
			if (!bShowWarned) { bShowWarned = true; _ASSERTE(hWnd != ghConEmuWndDC && L"OnShowWindow(ghConEmuWndDC)"); }
		}
		else
		{
			static bool bShowWarned = false;
			if (!bShowWarned) { bShowWarned = true; _ASSERTE(hWnd != ghConEmuWndDC && L"OnShowWindow(ghConWnd)"); }
		}
		#endif
		return TRUE; // �������
	}

	if ((ghAttachGuiClient == hWnd) || (!ghAttachGuiClient && gbAttachGuiClient))
		OnShowGuiClientWindow(hWnd, nCmdShow, lbGuiAttach, lbInactiveTab);

	if (F(ShowWindow))
	{
		lbRc = F(ShowWindow)(hWnd, nCmdShow);
		// ������ ����� ����� ���� ��������, ��-�� ����, ��� �������� �������
		// ����������� � wShowCmd=SW_HIDE (����� �� �������)
		if (!bShowWndCalled)
		{
			bShowWndCalled = true;
			if (!lbRc && nCmdShow && !user->isWindowVisible(hWnd))
			{
				F(ShowWindow)(hWnd, nCmdShow);
			}
		}

		// ���� ������� �� �������� - �� ������� ����� � ConEmu
		if (lbGuiAttach && lbInactiveTab && nCmdShow && ghConEmuWnd)
		{
			user->setForegroundWindow(ghConEmuWnd);
		}
	}
	DWORD dwErr = GetLastError();

	if (lbGuiAttach)
		OnPostShowGuiClientWindow(hWnd, nCmdShow);

	SetLastError(dwErr);
	return lbRc;
}


HWND WINAPI OnSetParent(HWND hWndChild, HWND hWndNewParent)
{
	typedef HWND (WINAPI* OnSetParent_t)(HWND hWndChild, HWND hWndNewParent);
	ORIGINALFASTEX(SetParent,NULL);
	HWND lhRc = NULL;

	if (ghConEmuWndDC && hWndChild == ghConEmuWndDC)
	{
		_ASSERTE(hWndChild != ghConEmuWndDC);
		return NULL; // �������
	}
	
	if (hWndNewParent && ghConEmuWndDC)
	{
		_ASSERTE(hWndNewParent!=ghConEmuWnd);
		//hWndNewParent = ghConEmuWndDC;
	}

	if (F(SetParent))
	{
		lhRc = F(SetParent)(hWndChild, hWndNewParent);
	}

	return lhRc;
}


HWND WINAPI OnGetParent(HWND hWnd)
{
	typedef HWND (WINAPI* OnGetParent_t)(HWND hWnd);
	ORIGINALFASTEX(GetParent,NULL);
	HWND lhRc = NULL;

	//if (ghConEmuWndDC && hWnd == ghConEmuWndDC)
	//{
	//	hWnd = ghConEmuWnd;
	//}
	if (ghConEmuWndDC)
	{
		if (ghAttachGuiClient)
		{
			if (hWnd == ghAttachGuiClient || hWnd == ghConEmuWndDC)
			{
				// �������� GUI-�������, ����� �� ������, ��� �� "������"
				hWnd = ghConEmuWnd;
			}
		}
		else if (hWnd == ghConEmuWndDC)
		{
			hWnd = ghConEmuWnd;
		}
	}

	if (F(GetParent))
	{
		lhRc = F(GetParent)(hWnd);
	}

	return lhRc;
}


HWND WINAPI OnGetWindow(HWND hWnd, UINT uCmd)
{
	typedef HWND (WINAPI* OnGetWindow_t)(HWND hWnd, UINT uCmd);
	ORIGINALFASTEX(GetWindow,NULL);
	HWND lhRc = NULL;

	if (ghConEmuWndDC)
	{
		if (ghAttachGuiClient)
		{
			if ((hWnd == ghAttachGuiClient || hWnd == ghConEmuWndDC) && (uCmd == GW_OWNER))
			{
				hWnd = ghConEmuWnd;
			}
		}
		else if ((hWnd == ghConEmuWndDC) && (uCmd == GW_OWNER))
		{
			hWnd = ghConEmuWnd;
		}
	}

	if (F(GetWindow))
	{
		lhRc = F(GetWindow)(hWnd, uCmd);

		if (ghAttachGuiClient && (uCmd == GW_OWNER) && (lhRc == ghConEmuWndDC))
		{
			_ASSERTE(lhRc != ghConEmuWndDC);
			lhRc = ghAttachGuiClient;
		}
	}

	_ASSERTRESULT(lhRc!=NULL);
	return lhRc;
}


HWND WINAPI OnGetAncestor(HWND hWnd, UINT gaFlags)
{
	typedef HWND (WINAPI* OnGetAncestor_t)(HWND hWnd, UINT gaFlags);
	ORIGINALFASTEX(GetAncestor,NULL);
	HWND lhRc = NULL;

	#ifdef LOG_GETANCESTOR
	if (ghAttachGuiClient)
	{
		wchar_t szInfo[1024];
		getWindowInfo(hWnd, szInfo);
		lstrcat(szInfo, L"\n");
		DebugString(szInfo);
	}
	#endif

	//if (ghConEmuWndDC && hWnd == ghConEmuWndDC)
	//{
	//	hWnd = ghConEmuWnd;
	//}
	if (ghConEmuWndDC)
	{
		#ifdef _DEBUG
		if ((GetKeyState(VK_CAPITAL) & 1))
		{
			int nDbg = 0;
		}
		#endif

		if (ghAttachGuiClient)
		{
			// �������� GUI-�������, ����� �� ������, ��� �� "������"
			if (hWnd == ghAttachGuiClient || hWnd == ghConEmuWndDC)
			{
				hWnd = ghConEmuWnd;
			}
			#if 0
			else
			{
				wchar_t szName[255];
				user->getClassNameW(hWnd, szName, countof(szName));
				if (wcsncmp(szName, L"WindowsForms", 12) == 0)
				{
					GetWindowText(hWnd, szName, countof(szName));
					if (wcsncmp(szName, L"toolStrip", 8) == 0 || wcsncmp(szName, L"menuStrip", 8) == 0)
					{
						hWnd = ghConEmuWndDC;
					}
				}
			}
			#endif
		}
		else if (hWnd == ghConEmuWndDC)
		{
			hWnd = ghConEmuWnd;
		}
	}

	if (F(GetAncestor))
	{
		lhRc = F(GetAncestor)(hWnd, gaFlags);

		if (ghAttachGuiClient && (gaFlags == GA_ROOTOWNER || gaFlags == GA_ROOT) && lhRc == ghConEmuWnd)
		{
			lhRc = ghAttachGuiClient;
		}
	}

	_ASSERTRESULT(lhRc);
	return lhRc;
}

//Issue 469: Some programs requires "ConsoleWindowClass" for GetConsoleWindow
int WINAPI OnGetClassNameA(HWND hWnd, LPSTR lpClassName, int nMaxCount)
{
	typedef int (WINAPI *OnGetClassNameA_t)(HWND hWnd, LPSTR lpClassName, int nMaxCount);
	ORIGINALFASTEX(GetClassNameA,NULL);
	int iRc = 0;
	if (ghConEmuWndDC && hWnd == ghConEmuWndDC && lpClassName)
	{
		//lstrcpynA(lpClassName, RealConsoleClass, nMaxCount);
		WideCharToMultiByte(CP_ACP, 0, RealConsoleClass, -1, lpClassName, nMaxCount, 0,0);
		iRc = lstrlenA(lpClassName);
	}
	else if (F(GetClassNameA))
		iRc = F(GetClassNameA)(hWnd, lpClassName, nMaxCount);
	return iRc;
}
int WINAPI OnGetClassNameW(HWND hWnd, LPWSTR lpClassName, int nMaxCount)
{
	typedef int (WINAPI *OnGetClassNameW_t)(HWND hWnd, LPWSTR lpClassName, int nMaxCount);
	ORIGINALFASTEX(GetClassNameW,NULL);
	int iRc = 0;
	if (ghConEmuWndDC && hWnd == ghConEmuWndDC && lpClassName)
	{
		lstrcpynW(lpClassName, RealConsoleClass, nMaxCount);
		iRc = lstrlenW(lpClassName);
	}
	else if (F(GetClassNameW))
		iRc = F(GetClassNameW)(hWnd, lpClassName, nMaxCount);
	return iRc;
}

HWND WINAPI OnGetActiveWindow()
{
	typedef HWND (WINAPI* OnGetActiveWindow_t)();
	ORIGINALFASTEX(GetActiveWindow,NULL);
	HWND hWnd = NULL;

	if (F(GetActiveWindow))
		hWnd = F(GetActiveWindow)();

	#if 1
	if (ghAttachGuiClient)
	{
		if (hWnd == ghConEmuWnd || hWnd == ghConEmuWndDC)
			hWnd = ghAttachGuiClient;
		else if (hWnd == NULL)
			hWnd = ghAttachGuiClient;
	}
	#endif

	return hWnd;
}

HWND WINAPI OnGetForegroundWindow()
{
	typedef HWND (WINAPI* OnGetForegroundWindow_t)();
	ORIGINALFASTEX(GetForegroundWindow,NULL);

	HWND hFore = NULL;
	if (F(GetForegroundWindow))
		hFore = F(GetForegroundWindow)();

	if (ghConEmuWndDC)
	{
		if (ghAttachGuiClient && ((hFore == ghConEmuWnd) || (hFore == ghConEmuWndDC)))
		{
			// �������� GUI-�������, ����� �� ������, ��� �� "������"
			hFore = ghAttachGuiClient;
		}
		else if (hFore == ghConEmuWnd)
		{
			hFore = ghConEmuWndDC;
		}
	}

	return hFore;
}

BOOL WINAPI OnSetForegroundWindow(HWND hWnd)
{
	typedef BOOL (WINAPI* OnSetForegroundWindow_t)(HWND hWnd);
	ORIGINALFASTEX(SetForegroundWindow,NULL);

	BOOL lbRc = FALSE;

	if (ghConEmuWndDC)
	{
		if (hWnd == ghConEmuWndDC)
			hWnd = ghConEmuWnd;
		lbRc = GuiSetForeground(hWnd);
	}

	// ConEmu �������� ��� ��� ������, �� �� ������ ������, ������ � �����
	if (F(SetForegroundWindow) != NULL)
	{
		lbRc = F(SetForegroundWindow)(hWnd);

		//if (gbShowOnSetForeground && lbRc)
		//{
		//	if (user->isWindow(hWnd) && !IsWindowVisible(hWnd))
		//		ShowWindow(hWnd, SW_SHOW);
		//}
	}

	return lbRc;
}

BOOL WINAPI OnSetMenu(HWND hWnd, HMENU hMenu)
{
	typedef BOOL (WINAPI* OnSetMenu_t)(HWND hWnd, HMENU hMenu);
	ORIGINALFASTEX(SetMenu,NULL);

	BOOL lbRc = FALSE;

	if (hMenu && ghAttachGuiClient && hWnd == ghAttachGuiClient)
	{
		if ((gnAttachGuiClientFlags & (agaf_WS_CHILD|agaf_NoMenu|agaf_DotNet)) == (agaf_WS_CHILD|agaf_NoMenu))
		{
			gnAttachGuiClientFlags &= ~(agaf_WS_CHILD|agaf_NoMenu);
			DWORD_PTR dwStyle = user->getWindowLongPtrW(ghAttachGuiClient, GWL_STYLE);
			DWORD_PTR dwNewStyle = (dwStyle & ~WS_CHILD) | (gnAttachGuiClientStyle & WS_POPUP);
			if (dwStyle != dwNewStyle)
			{
				user->setWindowLongPtrW(ghAttachGuiClient, GWL_STYLE, dwNewStyle);
			}
		}
	}

	if (F(SetMenu) != NULL)
	{
		lbRc = F(SetMenu)(hWnd, hMenu);
	}

	return lbRc;
}

BOOL WINAPI OnMoveWindow(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint)
{
	typedef BOOL (WINAPI* OnMoveWindow_t)(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint);
	ORIGINALFASTEX(MoveWindow,NULL);
	BOOL lbRc = FALSE;

	if (ghConEmuWndDC && (hWnd == ghConEmuWndDC || hWnd == ghConEmuWnd))
		return TRUE; // �������. ����������� ��������� "�������" ConEmuDC

	if (ghConEmuWndDC && ghAttachGuiClient && hWnd == ghAttachGuiClient)
	{
		// GUI ������������ ��������� �������������� ��������� ������ ConEmu
		OnSetGuiClientWindowPos(hWnd, NULL, X, Y, nWidth, nHeight, 0);
	}
	
	if (F(MoveWindow))
		lbRc = F(MoveWindow)(hWnd, X, Y, nWidth, nHeight, bRepaint);

	return lbRc;
}

BOOL WINAPI OnSetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
	typedef BOOL (WINAPI* OnSetWindowPos_t)(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);
	ORIGINALFASTEX(SetWindowPos,NULL);
	BOOL lbRc = FALSE;

	if (ghConEmuWndDC && (hWnd == ghConEmuWndDC || hWnd == ghConEmuWnd))
	{
		_ASSERTRESULT(FALSE);
		return TRUE; // �������. ����������� ��������� "�������" ConEmuDC
	}

	if (ghConEmuWndDC && ghAttachGuiClient && hWnd == ghAttachGuiClient)
	{
		// GUI ������������ ��������� �������������� ��������� ������ ConEmu
		OnSetGuiClientWindowPos(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
	}

	if (F(SetWindowPos))
		lbRc = F(SetWindowPos)(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);

	_ASSERTRESULT(lbRc);
	return lbRc;
}

BOOL WINAPI OnGetWindowPlacement(HWND hWnd, WINDOWPLACEMENT *lpwndpl)
{
	typedef BOOL (WINAPI* OnGetWindowPlacement_t)(HWND hWnd, WINDOWPLACEMENT *lpwndpl);
	ORIGINALFASTEX(GetWindowPlacement,NULL);
	BOOL lbRc = FALSE;

	if (F(GetWindowPlacement))
		lbRc = F(GetWindowPlacement)(hWnd, lpwndpl);

	if (lbRc && ghConEmuWndDC && !gbGuiClientExternMode && ghAttachGuiClient
		&& hWnd == ghAttachGuiClient && user)
	{
		user->mapWindowPoints(ghConEmuWndDC, NULL, (LPPOINT)&lpwndpl->rcNormalPosition, 2);
	}

	return lbRc;
}

BOOL WINAPI OnSetWindowPlacement(HWND hWnd, WINDOWPLACEMENT *lpwndpl)
{
	typedef BOOL (WINAPI* OnSetWindowPlacement_t)(HWND hWnd, WINDOWPLACEMENT *lpwndpl);
	ORIGINALFASTEX(SetWindowPlacement,NULL);
	BOOL lbRc = FALSE;
	WINDOWPLACEMENT wpl = {sizeof(wpl)};

	if (ghConEmuWndDC && (hWnd == ghConEmuWndDC || hWnd == ghConEmuWnd))
		return TRUE; // �������. ����������� ��������� "�������" ConEmuDC

	if (lpwndpl && ghConEmuWndDC && ghAttachGuiClient && !gbGuiClientExternMode && hWnd == ghAttachGuiClient)
	{
		// GUI ������������ ��������� �������������� ��������� ������ ConEmu
		int X, Y, cx, cy;
		if (OnSetGuiClientWindowPos(hWnd, NULL, X, Y, cx, cy, 0))
		{
			wpl.showCmd = SW_RESTORE;
			wpl.rcNormalPosition.left = X;
			wpl.rcNormalPosition.top = Y;
			wpl.rcNormalPosition.right = X + cx;
			wpl.rcNormalPosition.bottom = Y + cy;
			wpl.ptMinPosition = lpwndpl->ptMinPosition;
			wpl.ptMaxPosition = lpwndpl->ptMaxPosition;
			lpwndpl = &wpl;
		}
	}

	if (F(SetWindowPlacement))
		lbRc = F(SetWindowPlacement)(hWnd, lpwndpl);

	return lbRc;
}

bool CanSendMessage(HWND& hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, LRESULT& lRc)
{
	if (ghConEmuWndDC && hWnd == ghConEmuWndDC)
	{
		switch (Msg)
		{
			case WM_SIZE:
			case WM_MOVE:
			case WM_SHOWWINDOW:
			case WM_SYSCOMMAND:
				// ��� ��������� - ������ ������������
				return false;
			case WM_INPUTLANGCHANGEREQUEST:
			case WM_INPUTLANGCHANGE:
			case WM_QUERYENDSESSION:
			case WM_ENDSESSION:
			case WM_QUIT:
			case WM_DESTROY:
			case WM_CLOSE:
			case WM_ENABLE:
			case WM_SETREDRAW:
				// ��� ��������� - ����� �������� � RealConsole
				hWnd = ghConWnd;
				return true;
			case WM_SETICON:
				TODO("����� �� �������� ������ � ConEmu � �������� �� �� ����");
				break;
		}
	}
	return true; // ���������
}

void PatchGuiMessage(HWND& hWnd, UINT& Msg, WPARAM& wParam, LPARAM& lParam)
{
	if (!ghAttachGuiClient)
		return;

	switch (Msg)
	{
	case WM_MOUSEACTIVATE:
		if (wParam == (WPARAM)ghConEmuWnd)
		{
			wParam = (WPARAM)ghAttachGuiClient;
		}
		break;
	}
}

//void PatchGuiMessage(LPMSG lpMsg)
//{
//	if (!ghAttachGuiClient)
//		return;
//
//	PatchGuiMessage(lpMsg->hwnd, lpMsg->message, lpMsg->wParam, lpMsg->lParam);
//}

BOOL WINAPI OnPostMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	typedef BOOL (WINAPI* OnPostMessageA_t)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	ORIGINALFASTEX(PostMessageA,NULL);
	BOOL lRc = 0;
	LRESULT ll = 0;

	if (!CanSendMessage(hWnd, Msg, wParam, lParam, ll))
		return TRUE; // �������, ��� ��������� ��������� �������� � ConEmuDC

	if (ghAttachGuiClient)
		PatchGuiMessage(hWnd, Msg, wParam, lParam);

	if (F(PostMessageA))
		lRc = F(PostMessageA)(hWnd, Msg, wParam, lParam);

	return lRc;
}
BOOL WINAPI OnPostMessageW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	typedef BOOL (WINAPI* OnPostMessageW_t)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	ORIGINALFASTEX(PostMessageW,NULL);
	BOOL lRc = 0;
	LRESULT ll = 0;

	if (!CanSendMessage(hWnd, Msg, wParam, lParam, ll))
		return TRUE; // �������, ��� ��������� ��������� �������� � ConEmuDC

	if (ghAttachGuiClient)
		PatchGuiMessage(hWnd, Msg, wParam, lParam);

	if (F(PostMessageW))
		lRc = F(PostMessageW)(hWnd, Msg, wParam, lParam);

	return lRc;
}
LRESULT WINAPI OnSendMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	typedef LRESULT (WINAPI* OnSendMessageA_t)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	ORIGINALFASTEX(SendMessageA,NULL);
	LRESULT lRc = 0;

	if (!CanSendMessage(hWnd, Msg, wParam, lParam, lRc))
		return lRc; // �������, ��� ��������� ��������� �������� � ConEmuDC

	if (ghAttachGuiClient)
		PatchGuiMessage(hWnd, Msg, wParam, lParam);

	if (F(SendMessageA))
		lRc = F(SendMessageA)(hWnd, Msg, wParam, lParam);

	return lRc;
}
LRESULT WINAPI OnSendMessageW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	typedef LRESULT (WINAPI* OnSendMessageW_t)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	ORIGINALFASTEX(SendMessageW,NULL);
	LRESULT lRc = 0;

	if (!CanSendMessage(hWnd, Msg, wParam, lParam, lRc))
		return lRc; // �������, ��� ��������� ��������� �������� � ConEmuDC

	if (ghAttachGuiClient)
		PatchGuiMessage(hWnd, Msg, wParam, lParam);

	if (F(SendMessageW))
		lRc = F(SendMessageW)(hWnd, Msg, wParam, lParam);

	return lRc;
}

BOOL WINAPI OnGetMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax)
{
	typedef BOOL (WINAPI* OnGetMessageA_t)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
	ORIGINALFASTEX(GetMessageA,NULL);
	BOOL lRc = 0;

	if (F(GetMessageA))
		lRc = F(GetMessageA)(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);

	if (lRc && ghAttachGuiClient)
		PatchGuiMessage(lpMsg->hwnd, lpMsg->message, lpMsg->wParam, lpMsg->lParam);
		
	return lRc;
}
BOOL WINAPI OnGetMessageW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax)
{
	typedef BOOL (WINAPI* OnGetMessageW_t)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
	ORIGINALFASTEX(GetMessageW,NULL);
	BOOL lRc = 0;

	if (F(GetMessageW))
		lRc = F(GetMessageW)(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);

	if (lRc && ghAttachGuiClient)
		PatchGuiMessage(lpMsg->hwnd, lpMsg->message, lpMsg->wParam, lpMsg->lParam);
	
	_ASSERTRESULT(TRUE);
	return lRc;
}
BOOL WINAPI OnPeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{
	typedef BOOL (WINAPI* OnPeekMessageA_t)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
	ORIGINALFASTEX(PeekMessageA,NULL);
	BOOL lRc = 0;

	if (F(PeekMessageA))
		lRc = F(PeekMessageA)(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);

	if (lRc && ghAttachGuiClient)
		PatchGuiMessage(lpMsg->hwnd, lpMsg->message, lpMsg->wParam, lpMsg->lParam);
		
	return lRc;
}
BOOL WINAPI OnPeekMessageW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{
	typedef BOOL (WINAPI* OnPeekMessageW_t)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
	ORIGINALFASTEX(PeekMessageW,NULL);
	BOOL lRc = 0;

	if (F(PeekMessageW))
		lRc = F(PeekMessageW)(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);

	if (lRc && ghAttachGuiClient)
		PatchGuiMessage(lpMsg->hwnd, lpMsg->message, lpMsg->wParam, lpMsg->lParam);
		
	return lRc;
}

HWND WINAPI OnCreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	typedef HWND (WINAPI* OnCreateWindowExA_t)(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
	ORIGINALFASTEX(CreateWindowExA,NULL);
	HWND hWnd = NULL;
	BOOL bAttachGui = FALSE, bStyleHidden = FALSE;
	DWORD lStyle = dwStyle, lStyleEx = dwExStyle;

	if (CheckCanCreateWindow(lpClassName, NULL, dwStyle, dwExStyle, hWndParent, bAttachGui, bStyleHidden)
		&& F(CreateWindowExA) != NULL)
	{
		if (bAttachGui)
		{
			if ((grcConEmuClient.right > grcConEmuClient.left) && (grcConEmuClient.bottom > grcConEmuClient.top))
			{
				x = grcConEmuClient.left; y = grcConEmuClient.top;
				nWidth = grcConEmuClient.right - grcConEmuClient.left; nHeight = grcConEmuClient.bottom - grcConEmuClient.top;
			}
			else
			{
				_ASSERTEX((grcConEmuClient.right > grcConEmuClient.left) && (grcConEmuClient.bottom > grcConEmuClient.top));
			}
		}

		hWnd = F(CreateWindowExA)(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		DWORD dwErr = GetLastError();

		if (hWnd && bAttachGui)
		{
			OnGuiWindowAttached(hWnd, hMenu, lpClassName, NULL, lStyle, lStyleEx, bStyleHidden);

			SetLastError(dwErr);
		}
	}

	return hWnd;
}


HWND WINAPI OnCreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	typedef HWND (WINAPI* OnCreateWindowExW_t)(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
	ORIGINALFASTEX(CreateWindowExW,NULL);
	HWND hWnd = NULL;
	BOOL bAttachGui = FALSE, bStyleHidden = FALSE;
	DWORD lStyle = dwStyle, lStyleEx = dwExStyle;

	if (CheckCanCreateWindow(NULL, lpClassName, dwStyle, dwExStyle, hWndParent, bAttachGui, bStyleHidden)
		&& F(CreateWindowExW) != NULL)
	{
		if (bAttachGui)
		{
			x = grcConEmuClient.left; y = grcConEmuClient.top;
			nWidth = grcConEmuClient.right - grcConEmuClient.left; nHeight = grcConEmuClient.bottom - grcConEmuClient.top;
		}

		hWnd = F(CreateWindowExW)(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		DWORD dwErr = GetLastError();

		if (hWnd && bAttachGui)
		{
			OnGuiWindowAttached(hWnd, hMenu, NULL, lpClassName, lStyle, lStyleEx, bStyleHidden);

			SetLastError(dwErr);
		}
	}

	_ASSERTRESULT(hWnd!=NULL);
	return hWnd;
}

HWND WINAPI OnCreateDialogIndirectParamA(HINSTANCE hInstance, LPCDLGTEMPLATE lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM lParamInit)
{
	typedef HWND (WINAPI* OnCreateDialogIndirectParamA_t)(HINSTANCE hInstance, LPCDLGTEMPLATE lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM lParamInit);
	ORIGINALFASTEX(CreateDialogIndirectParamA,NULL);
	HWND hWnd = NULL;
	BOOL bAttachGui = (hWndParent == NULL), bStyleHidden = FALSE;
	// �� ������� - ������ ����� ���� ��������
	DWORD lStyle = lpTemplate ? lpTemplate->style : 0;
	DWORD lStyleEx = lpTemplate ? lpTemplate->dwExtendedStyle : 0;

	if (/*CheckCanCreateWindow((LPCSTR)32770, NULL, lStyle, lStyleEx, hWndParent, bAttachGui, bStyleHidden)
		&&*/ F(CreateDialogIndirectParamA) != NULL)
	{
		//if (bAttachGui)
		//{
		//	x = grcConEmuClient.left; y = grcConEmuClient.top;
		//	nWidth = grcConEmuClient.right - grcConEmuClient.left; nHeight = grcConEmuClient.bottom - grcConEmuClient.top;
		//}

		hWnd = F(CreateDialogIndirectParamA)(hInstance, lpTemplate, hWndParent, lpDialogFunc, lParamInit);
		DWORD dwErr = GetLastError();

		if (hWnd && bAttachGui)
		{
			lStyle = (DWORD)user->getWindowLongPtrW(hWnd, GWL_STYLE);
			lStyleEx = (DWORD)user->getWindowLongPtrW(hWnd, GWL_EXSTYLE);
			if (CheckCanCreateWindow((LPCSTR)32770, NULL, lStyle, lStyleEx, hWndParent, bAttachGui, bStyleHidden) && bAttachGui)
			{
				OnGuiWindowAttached(hWnd, NULL, (LPCSTR)32770, NULL, lStyle, lStyleEx, bStyleHidden);
			}

			SetLastError(dwErr);
		}
	}

	return hWnd;
}

HWND WINAPI OnCreateDialogIndirectParamW(HINSTANCE hInstance, LPCDLGTEMPLATE lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM lParamInit)
{
	typedef HWND (WINAPI* OnCreateDialogIndirectParamW_t)(HINSTANCE hInstance, LPCDLGTEMPLATE lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM lParamInit);
	ORIGINALFASTEX(CreateDialogIndirectParamW,NULL);
	HWND hWnd = NULL;
	BOOL bAttachGui = (hWndParent == NULL), bStyleHidden = FALSE;
	// �� ������� - ������ ����� ���� ��������
	DWORD lStyle = lpTemplate ? lpTemplate->style : 0;
	DWORD lStyleEx = lpTemplate ? lpTemplate->dwExtendedStyle : 0;

	if (/*CheckCanCreateWindow(NULL, (LPCWSTR)32770, lStyle, lStyleEx, hWndParent, bAttachGui, bStyleHidden)
		&&*/ F(CreateDialogIndirectParamW) != NULL)
	{
		//if (bAttachGui)
		//{
		//	x = grcConEmuClient.left; y = grcConEmuClient.top;
		//	nWidth = grcConEmuClient.right - grcConEmuClient.left; nHeight = grcConEmuClient.bottom - grcConEmuClient.top;
		//}

		hWnd = F(CreateDialogIndirectParamW)(hInstance, lpTemplate, hWndParent, lpDialogFunc, lParamInit);
		DWORD dwErr = GetLastError();

		if (hWnd && bAttachGui)
		{
			lStyle = (DWORD)user->getWindowLongPtrW(hWnd, GWL_STYLE);
			lStyleEx = (DWORD)user->getWindowLongPtrW(hWnd, GWL_EXSTYLE);
			if (CheckCanCreateWindow((LPCSTR)32770, NULL, lStyle, lStyleEx, hWndParent, bAttachGui, bStyleHidden) && bAttachGui)
			{
				OnGuiWindowAttached(hWnd, NULL, NULL, (LPCWSTR)32770, lStyle, lStyleEx, bStyleHidden);
			}

			SetLastError(dwErr);
		}
	}

	return hWnd;
}

HWND WINAPI OnCreateDialogParamA(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
	typedef HWND (WINAPI* OnCreateDialogParamA_t)(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
	ORIGINALFASTEX(CreateDialogParamA,NULL);
	HWND hWnd = NULL;
	BOOL bAttachGui = FALSE, bStyleHidden = FALSE;
	LPCDLGTEMPLATE lpTemplate = NULL;
	DWORD lStyle = 0; //lpTemplate ? lpTemplate->style : 0;
	DWORD lStyleEx = 0; //lpTemplate ? lpTemplate->dwExtendedStyle : 0;

	// ��������� ������ �������, � ������� ��� ��������� lStyle/lStyleEx	
	HRSRC hDlgSrc = FindResourceA(hInstance, lpTemplateName, /*RT_DIALOG*/MAKEINTRESOURCEA(5));
	if (hDlgSrc)
	{
		HGLOBAL hDlgHnd = LoadResource(hInstance, hDlgSrc);
		if (hDlgHnd)
		{
			lpTemplate = (LPCDLGTEMPLATE)LockResource(hDlgHnd);
			if (lpTemplate)
			{
				lStyle = lpTemplate ? lpTemplate->style : 0;
				lStyleEx = lpTemplate ? lpTemplate->dwExtendedStyle : 0;
			}
		}
	}

	if ((!lpTemplate || CheckCanCreateWindow((LPSTR)32770, NULL, lStyle, lStyleEx, hWndParent, bAttachGui, bStyleHidden))
		&& F(CreateDialogParamA) != NULL)
	{
		//if (bAttachGui)
		//{
		//	x = grcConEmuClient.left; y = grcConEmuClient.top;
		//	nWidth = grcConEmuClient.right - grcConEmuClient.left; nHeight = grcConEmuClient.bottom - grcConEmuClient.top;
		//}

		hWnd = F(CreateDialogParamA)(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
		DWORD dwErr = GetLastError();

		if (hWnd && bAttachGui)
		{
			OnGuiWindowAttached(hWnd, NULL, (LPCSTR)32770, NULL, lStyle, lStyleEx, bStyleHidden);

			SetLastError(dwErr);
		}
	}

	return hWnd;
}

HWND WINAPI OnCreateDialogParamW(HINSTANCE hInstance, LPCWSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
	typedef HWND (WINAPI* OnCreateDialogParamW_t)(HINSTANCE hInstance, LPCWSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
	ORIGINALFASTEX(CreateDialogParamW,NULL);
	HWND hWnd = NULL;
	BOOL bAttachGui = FALSE, bStyleHidden = FALSE;
	LPCDLGTEMPLATE lpTemplate = NULL;
	DWORD lStyle = 0; //lpTemplate ? lpTemplate->style : 0;
	DWORD lStyleEx = 0; //lpTemplate ? lpTemplate->dwExtendedStyle : 0;

	// ��������� ������ �������, � ������� ��� ��������� lStyle/lStyleEx	
	HRSRC hDlgSrc = FindResourceW(hInstance, lpTemplateName, RT_DIALOG);
	if (hDlgSrc)
	{
		HGLOBAL hDlgHnd = LoadResource(hInstance, hDlgSrc);
		if (hDlgHnd)
		{
			lpTemplate = (LPCDLGTEMPLATE)LockResource(hDlgHnd);
			if (lpTemplate)
			{
				lStyle = lpTemplate ? lpTemplate->style : 0;
				lStyleEx = lpTemplate ? lpTemplate->dwExtendedStyle : 0;
			}
		}
	}

	if ((!lpTemplate || CheckCanCreateWindow(NULL, (LPWSTR)32770, lStyle, lStyleEx, hWndParent, bAttachGui, bStyleHidden))
		&& F(CreateDialogParamW) != NULL)
	{
		//if (bAttachGui)
		//{
		//	x = grcConEmuClient.left; y = grcConEmuClient.top;
		//	nWidth = grcConEmuClient.right - grcConEmuClient.left; nHeight = grcConEmuClient.bottom - grcConEmuClient.top;
		//}

		hWnd = F(CreateDialogParamW)(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
		DWORD dwErr = GetLastError();

		if (hWnd && bAttachGui)
		{
			OnGuiWindowAttached(hWnd, NULL, NULL, (LPCWSTR)32770, lStyle, lStyleEx, bStyleHidden);

			SetLastError(dwErr);
		}
	}

	return hWnd;
}



#ifndef NORM_STOP_ON_NULL
#define NORM_STOP_ON_NULL 0x10000000
#endif

// ANSI ������ ������ ���, �.�. FAR 1.x ���������� ���
int WINAPI OnCompareStringW(LCID Locale, DWORD dwCmpFlags, LPCWSTR lpString1, int cchCount1, LPCWSTR lpString2, int cchCount2)
{
	typedef int (WINAPI* OnCompareStringW_t)(LCID Locale, DWORD dwCmpFlags, LPCWSTR lpString1, int cchCount1, LPCWSTR lpString2, int cchCount2);
	SUPPRESSORIGINALSHOWCALL;
	ORIGINALFAST(CompareStringW);
	int nCmp = -1;

	if (gFarMode.bFarHookMode && gFarMode.bFARuseASCIIsort)
	{
		if (dwCmpFlags == (NORM_IGNORECASE|NORM_STOP_ON_NULL|SORT_STRINGSORT))
		{
			int n = 0;
			WCHAR ch1 = *lpString1++, /*ch10 = 0,*/ ch2 = *lpString2++ /*,ch20 = 0*/;
			int n1 = (cchCount1==cchCount2) ? cchCount1
			         : (cchCount1!=-1 && cchCount2!=-1) ? -1
			         : (cchCount1!=-1) ? cchCount2
			         : (cchCount2!=-1) ? cchCount1 : min(cchCount1,cchCount2);

			while(!n && /*(ch1=*lpString1++) && (ch2=*lpString2++) &&*/ n1--)
			{
				if (ch1==ch2)
				{
					if (!ch1) break;

					// n = 0;
				}
				else if (ch1<0x80 && ch2<0x80)
				{
					if (ch1>=L'A' && ch1<=L'Z') ch1 |= 0x20; //-V112

					if (ch2>=L'A' && ch2<=L'Z') ch2 |= 0x20; //-V112

					n = (ch1==ch2) ? 0 : (ch1<ch2) ? -1 : 1;
				}
				else
				{
					n = CompareStringW(Locale, dwCmpFlags, &ch1, 1, &ch2, 1)-2;
				}

				if (!ch1 || !ch2 || !n1) break;

				ch1 = *lpString1++;
				ch2 = *lpString2++;
			}

			nCmp = (n<0) ? 1 : (n>0) ? 3 : 2;
		}
	}

	if (nCmp == -1)
		nCmp = F(CompareStringW)(Locale, dwCmpFlags, lpString1, cchCount1, lpString2, cchCount2);

	return nCmp;
}


DWORD WINAPI OnGetConsoleAliasesW(LPWSTR AliasBuffer, DWORD AliasBufferLength, LPWSTR ExeName)
{
	typedef DWORD (WINAPI* OnGetConsoleAliasesW_t)(LPWSTR AliasBuffer, DWORD AliasBufferLength, LPWSTR ExeName);
	ORIGINALFAST(GetConsoleAliasesW);
	DWORD nError = 0;
	DWORD nRc = F(GetConsoleAliasesW)(AliasBuffer,AliasBufferLength,ExeName);

	if (!nRc)
	{
		nError = GetLastError();

		// ���� �����
		if (nError == ERROR_NOT_ENOUGH_MEMORY) // && gdwServerPID)
		{
			DWORD nServerPID = 0;
			HWND hConWnd = GetConsoleWindow();
			_ASSERTE(hConWnd == ghConWnd);
			MFileMapping<CESERVER_CONSOLE_MAPPING_HDR> ConInfo;
			ConInfo.InitName(CECONMAPNAME, (DWORD)hConWnd); //-V205
			CESERVER_CONSOLE_MAPPING_HDR *pInfo = ConInfo.Open();
			if (pInfo 
				&& (pInfo->cbSize >= sizeof(CESERVER_CONSOLE_MAPPING_HDR))
				//&& (pInfo->nProtocolVersion == CESERVER_REQ_VER)
				)
			{
				nServerPID = pInfo->nServerPID;
				ConInfo.CloseMap();
			}

			if (nServerPID)
			{
				CESERVER_REQ_HDR In;
				ExecutePrepareCmd(&In, CECMD_GETALIASES, sizeof(CESERVER_REQ_HDR));
				CESERVER_REQ* pOut = ExecuteSrvCmd(nServerPID/*gdwServerPID*/, (CESERVER_REQ*)&In, hConWnd);

				if (pOut)
				{
					size_t nData = min(AliasBufferLength,(pOut->hdr.cbSize-sizeof(pOut->hdr)));

					if (nData)
					{
						memmove(AliasBuffer, pOut->Data, nData);
						nRc = TRUE;
					}

					ExecuteFreeResult(pOut);
				}
			}
		}

		if (!nRc)
			SetLastError(nError); // �������, ����� ����� ������� ��� ��������
	}

	return nRc;
}


// Microsoft bug?
// http://code.google.com/p/conemu-maximus5/issues/detail?id=60
typedef BOOL (WINAPI* OnSetConsoleCP_t)(UINT wCodePageID);
typedef BOOL (WINAPI* OnSetConsoleOutputCP_t)(UINT wCodePageID);
struct SCOCP
{
	UINT  wCodePageID;
	OnSetConsoleCP_t f;
	HANDLE hReady;
	DWORD dwErrCode;
	BOOL  lbRc;
};
DWORD WINAPI SetConsoleCPThread(LPVOID lpParameter)
{
	SCOCP* p = (SCOCP*)lpParameter;
	p->dwErrCode = -1;
	SetEvent(p->hReady);
	p->lbRc = p->f(p->wCodePageID);
	p->dwErrCode = GetLastError();
	return 0;
}
BOOL WINAPI OnSetConsoleCP(UINT wCodePageID)
{
	ORIGINALFAST(SetConsoleCP);
	_ASSERTE(OnSetConsoleCP!=SetConsoleCP);
	BOOL lbRc = FALSE;
	SCOCP sco = {wCodePageID, (OnSetConsoleCP_t)F(SetConsoleCP), CreateEvent(NULL,FALSE,FALSE,NULL)};
	DWORD nTID = 0, nWait = -1, nErr;
	/*
	wchar_t szErrText[255], szTitle[64];
	msprintf(szTitle, SKIPLEN(countof(szTitle)) L"PID=%u, TID=%u", GetCurrentProcessId(), GetCurrentThreadId());
	*/
	#ifdef _DEBUG
	DWORD nPrevCP = GetConsoleCP();
	#endif
	DWORD nCurCP = 0;
	HANDLE hThread = CreateThread(NULL,0,SetConsoleCPThread,&sco,0,&nTID);

	if (!hThread)
	{
		nErr = GetLastError();
		_ASSERTE(hThread!=NULL);
		/*
		msprintf(szErrText, SKIPLEN(countof(szErrText)) L"chcp failed, ErrCode=0x%08X\nConEmuHooks.cpp:OnSetConsoleCP", nErr);
		Message BoxW(NULL, szErrText, szTitle, MB_ICONSTOP|MB_SYSTEMMODAL);
		lbRc = FALSE;
		*/
	}
	else
	{
		HANDLE hEvents[2] = {hThread, sco.hReady};
		nWait = WaitForMultipleObjects(2, hEvents, FALSE, SETCONCP_READYTIMEOUT);

		if (nWait != WAIT_OBJECT_0)
			nWait = WaitForSingleObject(hThread, SETCONCP_TIMEOUT);

		if (nWait == WAIT_OBJECT_0)
		{
			lbRc = sco.lbRc;
		}
		else
		{
			//BUGBUG: �� ��������� �������� (Win2k3, WinXP) SetConsoleCP (� ��� � ����) ������ ��������
			TerminateThread(hThread,100);
			nCurCP = GetConsoleCP();
			if (nCurCP == wCodePageID)
			{
				lbRc = TRUE; // ���� ����������
			}
			else
			{
				_ASSERTE(nCurCP == wCodePageID);
				/*
				msprintf(szErrText, SKIPLEN(countof(szErrText))
				          L"chcp hung detected\n"
				          L"ConEmuHooks.cpp:OnSetConsoleCP\n"
				          L"ReqCP=%u, PrevCP=%u, CurCP=%u\n"
				          //L"\nPress <OK> to stop waiting"
				          ,wCodePageID, nPrevCP, nCurCP);
				Message BoxW(NULL, szErrText, szTitle, MB_ICONSTOP|MB_SYSTEMMODAL);
				*/
			}

			//nWait = WaitForSingleObject(hThread, 0);
			//if (nWait == WAIT_TIMEOUT)
			//	TerminateThread(hThread,100);
			//if (GetConsoleCP() == wCodePageID)
			//	lbRc = TRUE;
		}

		CloseHandle(hThread);
	}

	if (sco.hReady)
		CloseHandle(sco.hReady);

	return lbRc;
}
BOOL WINAPI OnSetConsoleOutputCP(UINT wCodePageID)
{
	ORIGINALFAST(SetConsoleOutputCP);
	_ASSERTE(OnSetConsoleOutputCP!=SetConsoleOutputCP);
	BOOL lbRc = FALSE;
	SCOCP sco = {wCodePageID, (OnSetConsoleCP_t)F(SetConsoleOutputCP), CreateEvent(NULL,FALSE,FALSE,NULL)};
	DWORD nTID = 0, nWait = -1, nErr;
	/*
	wchar_t szErrText[255], szTitle[64];
	msprintf(szTitle, SKIPLEN(countof(szTitle)) L"PID=%u, TID=%u", GetCurrentProcessId(), GetCurrentThreadId());
	*/
	#ifdef _DEBUG
	DWORD nPrevCP = GetConsoleOutputCP();
	#endif
	DWORD nCurCP = 0;
	HANDLE hThread = CreateThread(NULL,0,SetConsoleCPThread,&sco,0,&nTID);

	if (!hThread)
	{
		nErr = GetLastError();
		_ASSERTE(hThread!=NULL);
		/*
		msprintf(szErrText, SKIPLEN(countof(szErrText)) L"chcp(out) failed, ErrCode=0x%08X\nConEmuHooks.cpp:OnSetConsoleOutputCP", nErr);
		Message BoxW(NULL, szErrText, szTitle, MB_ICONSTOP|MB_SYSTEMMODAL);
		lbRc = FALSE;
		*/
	}
	else
	{
		HANDLE hEvents[2] = {hThread, sco.hReady};
		nWait = WaitForMultipleObjects(2, hEvents, FALSE, SETCONCP_READYTIMEOUT);

		if (nWait != WAIT_OBJECT_0)
			nWait = WaitForSingleObject(hThread, SETCONCP_TIMEOUT);

		if (nWait == WAIT_OBJECT_0)
		{
			lbRc = sco.lbRc;
		}
		else
		{
			//BUGBUG: �� ��������� �������� (Win2k3, WinXP) SetConsoleCP (� ��� � ����) ������ ��������
			TerminateThread(hThread,100);
			nCurCP = GetConsoleOutputCP();
			if (nCurCP == wCodePageID)
			{
				lbRc = TRUE; // ���� ����������
			}
			else
			{
				_ASSERTE(nCurCP == wCodePageID);
				/*
				msprintf(szErrText, SKIPLEN(countof(szErrText))
				          L"chcp(out) hung detected\n"
				          L"ConEmuHooks.cpp:OnSetConsoleOutputCP\n"
				          L"ReqOutCP=%u, PrevOutCP=%u, CurOutCP=%u\n"
				          //L"\nPress <OK> to stop waiting"
				          ,wCodePageID, nPrevCP, nCurCP);
				Message BoxW(NULL, szErrText, szTitle, MB_ICONSTOP|MB_SYSTEMMODAL);
				*/
			}
			//nWait = WaitForSingleObject(hThread, 0);
			//if (nWait == WAIT_TIMEOUT)
			//	TerminateThread(hThread,100);
			//if (GetConsoleOutputCP() == wCodePageID)
			//	lbRc = TRUE;
		}

		CloseHandle(hThread);
	}

	if (sco.hReady)
		CloseHandle(sco.hReady);

	return lbRc;
}


void OnReadConsoleStart(BOOL bUnicode, HANDLE hConsoleInput, LPVOID lpBuffer, DWORD nNumberOfCharsToRead, LPDWORD lpNumberOfCharsRead, LPVOID pInputControl)
{
	if (gReadConsoleInfo.InReadConsoleTID)
		gReadConsoleInfo.LastReadConsoleTID = gReadConsoleInfo.InReadConsoleTID;

#ifdef _DEBUG
	wchar_t szCurDir[MAX_PATH+1];
	GetCurrentDirectory(countof(szCurDir), szCurDir);
#endif

	bool bCatch = false;
	CONSOLE_SCREEN_BUFFER_INFO csbi = {};
	HANDLE hConOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD nConIn = 0, nConOut = 0;
	if (GetConsoleScreenBufferInfo(hConOut, &csbi)
		&& GetConsoleMode(hConsoleInput, &nConIn) && GetConsoleMode(hConOut, &nConOut))
	{
		if ((nConIn & ENABLE_ECHO_INPUT) && (nConIn & ENABLE_LINE_INPUT))
		{
			bCatch = true;
			gReadConsoleInfo.InReadConsoleTID = GetCurrentThreadId();
			gReadConsoleInfo.bIsUnicode = bUnicode;
			gReadConsoleInfo.hConsoleInput = hConsoleInput;
			gReadConsoleInfo.crStartCursorPos = csbi.dwCursorPosition;
			gReadConsoleInfo.nConInMode = nConIn;
			gReadConsoleInfo.nConOutMode = nConOut;
		}
	}
	
	if (!bCatch)
	{
		gReadConsoleInfo.InReadConsoleTID = 0;
	}

	if (gbPowerShellMonitorProgress)
	{
		// ��� �������� � Prompt - ����� ���������
		if (gnPowerShellProgressValue != -1)
		{
			gnPowerShellProgressValue = -1;
			GuiSetProgress(0,0);
		}
	}
}

void OnReadConsoleEnd(BOOL bSucceeded, BOOL bUnicode, HANDLE hConsoleInput, LPVOID lpBuffer, DWORD nNumberOfCharsToRead, LPDWORD lpNumberOfCharsRead, LPVOID pInputControl)
{
	if (bSucceeded && !gbWasSucceededInRead && lpNumberOfCharsRead && *lpNumberOfCharsRead)
		gbWasSucceededInRead = TRUE;

	if (gReadConsoleInfo.InReadConsoleTID)
	{
		gReadConsoleInfo.LastReadConsoleTID = gReadConsoleInfo.InReadConsoleTID;
		gReadConsoleInfo.InReadConsoleTID = 0;

		TODO("�������� � ConEmu ��������� ������!");
	}

	// ����� ������������ ��������
	GetConsoleScreenBufferInfoCached(NULL, NULL);
}

bool IsPromptActionAllowed(bool bForce, bool bBashMargin, HANDLE* phConIn)
{
	if (!gReadConsoleInfo.InReadConsoleTID && !gReadConsoleInfo.LastReadConsoleInputTID)
		return false;

	DWORD nConInMode = 0;

	HANDLE hConIn = gReadConsoleInfo.InReadConsoleTID ? gReadConsoleInfo.hConsoleInput : gReadConsoleInfo.hConsoleInput2;
	if (!hConIn)
		return false;

	if (!gReadConsoleInfo.InReadConsoleTID && gReadConsoleInfo.LastReadConsoleInputTID)
	{
		// ���������, ����� ��������� ���� ���� ������������?
		if (!GetConsoleMode(hConIn, &nConInMode))
			return false;
		if (!bForce && ((nConInMode & ENABLE_MOUSE_INPUT) == ENABLE_MOUSE_INPUT))
			return false; // ��������� ������������ ����� ���������
	}

	if (phConIn)
		*phConIn = hConIn;

	return true;
}

BOOL OnPromptBsDeleteWord(bool bForce, bool bBashMargin)
{
	HANDLE hConIn = NULL;
	if (!IsPromptActionAllowed(bForce, bBashMargin, &hConIn))
		return FALSE;

	int iBSCount = 0;
	BOOL lbWrite = FALSE;
	DWORD dwLastError = 0;

	HANDLE hConOut = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_SCREEN_BUFFER_INFO csbi = {};
	if (GetConsoleScreenBufferInfo(hConOut, &csbi) && csbi.dwSize.X && csbi.dwSize.Y)
	{
		if (csbi.dwCursorPosition.X == 0)
		{
			iBSCount = 1;
		}
		else
		{
			bool bDBCS = false;
			DWORD nRead = 0;
			BOOL bReadOk = FALSE;

			// ���� � ������� ������� DBCS ��������� - ��� ��� �� ������
			DWORD nCP = GetConsoleOutputCP();
			if (nCP && nCP != CP_UTF7 && nCP != CP_UTF8 && nCP != 1200 && nCP != 1201)
			{
				CPINFO cp = {};
				if (GetCPInfo(nCP, &cp) && (cp.MaxCharSize > 1))
				{
					bDBCS = true;
				}
			}

			int xPos = csbi.dwCursorPosition.X;
			COORD cr = {0, csbi.dwCursorPosition.Y};
			if ((xPos == 0) && (csbi.dwCursorPosition.Y > 0))
			{
				cr.Y--;
				xPos = csbi.dwSize.X;
			}

			char* pszLine = (char*)malloc(csbi.dwSize.X+1);
			wchar_t* pwszLine = (wchar_t*)malloc((csbi.dwSize.X+1)*sizeof(*pwszLine));


			if (pszLine && pwszLine)
			{
				pszLine[csbi.dwSize.X] = 0;
				pwszLine[csbi.dwSize.X] = 0;

				// ������� ������
				if (bDBCS)
				{
					// �� DBCS ���������� "ReadConsoleOutputCharacterW" ����� ����������
					BOOL bReadOk = ReadConsoleOutputCharacterA(hConOut, pszLine, csbi.dwSize.X, cr, &nRead);
					dwLastError = GetLastError();

					if (!bReadOk || !nRead)
					{
						// ������ � ReadConsoleOutputCharacterA ����� �������, ������� "W"
						bReadOk = ReadConsoleOutputCharacterW(hConOut, pwszLine, csbi.dwSize.X, cr, &nRead);
						dwLastError = GetLastError();

						if (!bReadOk || !nRead)
							bReadOk = FALSE;
						else
							bDBCS = false; // Thread string as simple Unicode.
					}
					else
					{
						nRead = MultiByteToWideChar(nCP, 0, pszLine, nRead, pwszLine, csbi.dwSize.X);
					}

					// Check chars count
					if (((int)nRead) <= 0)
						bReadOk = FALSE;
				}
				else
				{
					bReadOk = ReadConsoleOutputCharacterW(hConOut, pwszLine, csbi.dwSize.X, cr, &nRead);
					if (bReadOk && !nRead)
						bReadOk = FALSE;
				}

				if (bReadOk)
				{
					// Count chars
					if (bDBCS)
					{
						TODO("DBCS!!! Must to convert cursor pos ('DBCS') to char pos!");
						_ASSERTEX(bDBCS==false && "TODO!!!");
					}
					else
					{
						if ((int)nRead >= xPos)
						{
							int i = xPos - 1;
							_ASSERTEX(i >= 0);

							if ((pwszLine[i] == ucSpace) || (pwszLine[i] == ucNoBreakSpace))
							{
								// Delete all spaces
								iBSCount = 1;
								while ((i-- > 0) && ((pwszLine[i] == ucSpace) || (pwszLine[i] == ucNoBreakSpace)))
									iBSCount++;
							}
							else
							{
								// Delete all NON-spaces
								iBSCount = 1;
								wchar_t cBreaks[] = {ucSpace, ucNoBreakSpace, L'>', L']', L'$', L'.', L'\\', L'/', 0};
								while ((i-- > 0) && !wcschr(cBreaks, pwszLine[i]))
									iBSCount++;
							}
						}
					}
				}
			}

			// Done, string was processed
			SafeFree(pszLine);
			SafeFree(pwszLine);
		}
	}

	if (iBSCount > 0)
	{
		INPUT_RECORD* pr = (INPUT_RECORD*)calloc((size_t)iBSCount,sizeof(*pr));
		if (pr != NULL)
		{
			WORD vk = VK_BACK;
			HKL hkl = GetKeyboardLayout(gReadConsoleInfo.InReadConsoleTID ? gReadConsoleInfo.InReadConsoleTID : gReadConsoleInfo.LastReadConsoleInputTID);
			WORD sc = MapVirtualKeyEx(vk, 0/*MAPVK_VK_TO_VSC*/, hkl);
			if (!sc)
			{
				_ASSERTEX(sc!=NULL && "Can't determine SC?");
				sc = 0x0E;
			}

			for (int i = 0; i < iBSCount; i++)
			{
				pr[i].EventType = KEY_EVENT;
				pr[i].Event.KeyEvent.bKeyDown = TRUE;
				pr[i].Event.KeyEvent.wRepeatCount = 1;
				pr[i].Event.KeyEvent.wVirtualKeyCode = vk;
				pr[i].Event.KeyEvent.wVirtualScanCode = sc;
				pr[i].Event.KeyEvent.uChar.UnicodeChar = vk; // BS
				pr[i].Event.KeyEvent.dwControlKeyState = 0;
			}

			DWORD nWritten = 0;

			while (iBSCount > 0)
			{
				lbWrite = WriteConsoleInputW(hConIn, pr, min(iBSCount,256), &nWritten);
				if (!lbWrite || !nWritten)
					break;
				iBSCount -= nWritten;
			}

			free(pr);
		}
	}

	UNREFERENCED_PARAMETER(dwLastError);
	return FALSE;
}

// bBashMargin - sh.exe has pad in one space cell on right edge of window
BOOL OnReadConsoleClick(SHORT xPos, SHORT yPos, bool bForce, bool bBashMargin)
{
	TODO("��� �� ����� ��� ���������, ��� ������� ����� ������������ ����� �� ��������� �����, ���� ��� ENABLE_WRAP_AT_EOL_OUTPUT");
	TODO("��� ���������, ��� �����, ���� ���������� ������ ����� �������� �� ��������� ������ (���� ������ ����, � ������ �����)");

	HANDLE hConIn = NULL;
	if (!IsPromptActionAllowed(bForce, bBashMargin, &hConIn))
		return FALSE;

	BOOL lbRc = FALSE, lbWrite = FALSE;
    int nChars = 0;
    DWORD nWritten = 0, dwLastError = 0;

	HANDLE hConOut = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_SCREEN_BUFFER_INFO csbi = {};
	if (GetConsoleScreenBufferInfo(hConOut, &csbi) && csbi.dwSize.X && csbi.dwSize.Y)
	{
		bool bHomeEnd = false;
		lbRc = TRUE;

		nChars = (csbi.dwSize.X * (yPos - csbi.dwCursorPosition.Y))
			+ (xPos - csbi.dwCursorPosition.X);

		if (nChars != 0)
		{
			char* pszLine = (char*)malloc(csbi.dwSize.X+1);
			wchar_t* pwszLine = (wchar_t*)malloc((csbi.dwSize.X+1)*sizeof(*pwszLine));

			if (pszLine && pwszLine)
			{
				int nChecked = 0;
				int iCount;
				DWORD nRead;
				COORD cr;
				SHORT nPrevSpaces = 0, nPrevChars = 0;
				SHORT nWhole = 0, nPrint = 0;
				bool bDBCS = false;
				// ���� � ������� ������� DBCS ��������� - ��� ��� �� ������
				DWORD nCP = GetConsoleOutputCP();
				if (nCP && nCP != CP_UTF7 && nCP != CP_UTF8 && nCP != 1200 && nCP != 1201)
				{
					CPINFO cp = {};
					if (GetCPInfo(nCP, &cp) && (cp.MaxCharSize > 1))
					{
						bDBCS = true;
					}
				}

				TODO("DBCS!!! Must to convert cursor pos ('DBCS') to char pos!");
				// Ok, ������ ����� ���������, �� ��� �� ���� ������ "�� ��������� ������ �����"
				
				SHORT y = csbi.dwCursorPosition.Y;
				while (true)
				{
					cr.Y = y;
					if (nChars > 0)
					{
						cr.X = (y == csbi.dwCursorPosition.Y) ? csbi.dwCursorPosition.X : 0;
						iCount = (y == yPos) ? (xPos - cr.X) : (csbi.dwSize.X - cr.X);
						if (iCount < 0)
							break;
					}
					else
					{
						cr.X = 0;
						iCount = ((y == csbi.dwCursorPosition.Y) ? csbi.dwCursorPosition.X : csbi.dwSize.X)
							- ((y == yPos) ? xPos : 0);
						if (iCount < 0)
							break;
					}

					// ������� ������
					if (bDBCS)
					{
						// �� DBCS ���������� "ReadConsoleOutputCharacterW" ����� ����������
						BOOL bReadOk = ReadConsoleOutputCharacterA(hConOut, pszLine, iCount, cr, &nRead);
						dwLastError = GetLastError();

						if (!bReadOk || !nRead)
						{
							// ������ � ReadConsoleOutputCharacterA ����� �������, ������� "W"
							bReadOk = ReadConsoleOutputCharacterW(hConOut, pwszLine, iCount, cr, &nRead);
							dwLastError = GetLastError();

							if (!bReadOk || !nRead)
								break;

							bDBCS = false; // Thread string as simple Unicode.
						}
						else
						{
							nRead = MultiByteToWideChar(nCP, 0, pszLine, nRead, pwszLine, csbi.dwSize.X);
						}

						// Check chars count
						if (((int)nRead) <= 0)
							break;
					}
					else
					{
						if (!ReadConsoleOutputCharacterW(hConOut, pwszLine, iCount, cr, &nRead) || !nRead)
							break;
					}

					if (nRead > (DWORD)csbi.dwSize.X)
					{
						_ASSERTEX(nRead <= (DWORD)csbi.dwSize.X);
						break;
					}
					pwszLine[nRead] = 0;

					nWhole = nPrint = (SHORT)nRead;
					// ������� ���������� ������� � ����� ������ ��������
					while ((nPrint > 0) && (pwszLine[nPrint-1] == L' '))
					{
						nPrint--;
					}

					// � ����� ����������� ����
					if (nChars > 0) // ����
					{
						// ���� ������ (�� ��������) ������ 0 - ��������� � �������� ������� ���������� ������
						if (nPrint > 0)
						{
							nChecked += nPrevSpaces + nPrint;
						}
						else
						{
							// ���� �� ���������� ������ �������� �������� �� ���� - ���������
							if (nPrevChars <= 0)
								break;
						}
					}
					else // �����
					{
						if (nPrint <= 0)
							break; // �� ������ �� ������ ������ �� ���������������
						nChecked += nWhole;
					}
					nPrevChars = nPrint;
					nPrevSpaces = nWhole - nPrint;
					_ASSERTEX(nPrevSpaces>=0);
					

					// ���� + �������
					if (nChars > 0)
					{
						if ((++y) > yPos)
							break;
					}
					else
					{
						if ((--y) < yPos)
							break;
					}
				}
				SafeFree(pszLine);
				SafeFree(pwszLine);

				// Changed?
				nChars = (nChars > 0) ? nChecked : -nChecked;
				//nChars = (csbi.dwSize.X * (yPos - csbi.dwCursorPosition.Y))
				//	+ (xPos - csbi.dwCursorPosition.X);
			}
		}

		if (nChars != 0)
		{
			int nCount = bHomeEnd ? 1 : (nChars < 0) ? (-nChars) : nChars;
			if (!bHomeEnd && (nCount > (csbi.dwSize.X * (csbi.srWindow.Bottom - csbi.srWindow.Top))))
			{
				bHomeEnd = true;
				nCount = 1;
			}

			INPUT_RECORD* pr = (INPUT_RECORD*)calloc((size_t)nCount,sizeof(*pr));
			if (pr != NULL)
			{
				WORD vk = bHomeEnd ? ((nChars < 0) ? VK_HOME : VK_END) :
					((nChars < 0) ? VK_LEFT : VK_RIGHT);
				HKL hkl = GetKeyboardLayout(gReadConsoleInfo.InReadConsoleTID ? gReadConsoleInfo.InReadConsoleTID : gReadConsoleInfo.LastReadConsoleInputTID);
				WORD sc = MapVirtualKeyEx(vk, 0/*MAPVK_VK_TO_VSC*/, hkl);
				if (!sc)
				{
					_ASSERTEX(sc!=NULL && "Can't determine SC?");
					sc = (vk == VK_LEFT)  ? 0x4B :
						 (vk == VK_RIGHT) ? 0x4D :
						 (vk == VK_HOME)  ? 0x47 :
						 (vk == VK_RIGHT) ? 0x4F : 0;
				}

				for (int i = 0; i < nCount; i++)
				{
					pr[i].EventType = KEY_EVENT;
					pr[i].Event.KeyEvent.bKeyDown = TRUE;
					pr[i].Event.KeyEvent.wRepeatCount = 1;
					pr[i].Event.KeyEvent.wVirtualKeyCode = vk;
					pr[i].Event.KeyEvent.wVirtualScanCode = sc;
					pr[i].Event.KeyEvent.dwControlKeyState = ENHANCED_KEY;
				}

				while (nCount > 0)
				{
					lbWrite = WriteConsoleInputW(hConIn, pr, min(nCount,256), &nWritten);
					if (!lbWrite || !nWritten)
						break;
					nCount -= nWritten;
				}

				free(pr);
			}
		}
	}

	UNREFERENCED_PARAMETER(dwLastError);
	return lbRc;
}

BOOL OnExecutePromptCmd(LPCWSTR asCmd)
{
	if (!gReadConsoleInfo.InReadConsoleTID && !gReadConsoleInfo.LastReadConsoleInputTID)
		return FALSE;

	HANDLE hConIn = gReadConsoleInfo.InReadConsoleTID ? gReadConsoleInfo.hConsoleInput : gReadConsoleInfo.hConsoleInput2;
	if (!hConIn)
		return FALSE;

	BOOL lbRc = FALSE;
	INPUT_RECORD r[256];
	INPUT_RECORD* pr = r;
	INPUT_RECORD* prEnd = r + countof(r);
	LPCWSTR pch = asCmd;
	DWORD nWrite, nWritten;
	BOOL lbWrite;

	while (*pch)
	{
		// ���� (\r\n)|(\n) - ����� \r
		if ((*pch == L'\r') || (*pch == L'\n'))
		{
			TranslateKeyPress(0, 0, L'\r', -1, pr, pr+1);

			if (*pch == L'\r' && *(pch+1) == L'\n')
				pch += 2;
			else
				pch ++;
		}
		else
		{
			TranslateKeyPress(0, 0, *pch, -1, pr, pr+1);
			pch ++;
		}

		pr += 2;
		if (pr >= prEnd)
		{
			_ASSERTE(pr == prEnd);

			if (pr && (pr > r))
			{
				nWrite = (DWORD)(pr - r);
				lbWrite = WriteConsoleInputW(hConIn, r, nWrite, &nWritten);
				if (!lbWrite)
				{
					pr = NULL;
					lbRc = FALSE;
					break;
				}
				if (*pch) // ����� �� ���� ������������ ������
					Sleep(10);
				lbRc = TRUE;
			}

			pr = r;
		}
	}

	if (pr && (pr > r))
	{
		nWrite = (DWORD)(pr - r);
		lbRc = WriteConsoleInputW(hConIn, r, nWrite, &nWritten);
	}

	return lbRc;
}

BOOL WINAPI OnReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
	typedef BOOL (WINAPI* OnReadFile_t)(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
	SUPPRESSORIGINALSHOWCALL;
	ORIGINAL(ReadFile);
	BOOL lbRc = FALSE;

	bool bConIn = false;

	DWORD nPrevErr = GetLastError();

	if (hFile == ghLastConInHandle)
	{
		bConIn = true;
	}
	else if (hFile != ghLastNotConInHandle)
	{
		DWORD nMode = 0;
		BOOL lbConRc = GetConsoleMode(hFile, &nMode);
		if (lbConRc
			&& ((nMode & (ENABLE_LINE_INPUT & ENABLE_ECHO_INPUT)) == (ENABLE_LINE_INPUT & ENABLE_ECHO_INPUT)))
		{
			bConIn = true;
			ghLastConInHandle = hFile;
		}
		else
		{
			ghLastNotConInHandle = hFile;
		}
	}

	if (bConIn)
		OnReadConsoleStart(FALSE, hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, NULL);

	SetLastError(nPrevErr);
	lbRc = F(ReadFile)(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
	DWORD nErr = GetLastError();

	if (bConIn)
	{
		OnReadConsoleEnd(lbRc, FALSE, hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, NULL);

		#ifdef _DEBUG
		// Only for input_bug search purposes in Debug builds
		const char* pszChr = (const char*)lpBuffer;
		int cchRead = *lpNumberOfBytesRead;
		wchar_t* pszDbgCurChars = NULL;
		while ((cchRead--) > 0)
		{
			LONG idx = (InterlockedIncrement(&gn_LogReadChars) & (gn_LogReadCharsMax-1))*2;
			if (!pszDbgCurChars) pszDbgCurChars = gs_LogReadChars+idx;
			gs_LogReadChars[idx++] = L'|';
			gs_LogReadChars[idx++] = *pszChr ? *pszChr : L'.';
			gs_LogReadChars[idx] = 0;
			pszChr++;
		}
		#endif

		SetLastError(nErr);
	}

	UNREFERENCED_PARAMETER(bMainThread);

	return lbRc;
}

BOOL WINAPI OnReadConsoleA(HANDLE hConsoleInput, LPVOID lpBuffer, DWORD nNumberOfCharsToRead, LPDWORD lpNumberOfCharsRead, LPVOID pInputControl)
{
	typedef BOOL (WINAPI* OnReadConsoleA_t)(HANDLE hConsoleInput, LPVOID lpBuffer, DWORD nNumberOfCharsToRead, LPDWORD lpNumberOfCharsRead, LPVOID pInputControl);
	SUPPRESSORIGINALSHOWCALL;
	ORIGINAL(ReadConsoleA);
	BOOL lbRc = FALSE;

	OnReadConsoleStart(FALSE, hConsoleInput, lpBuffer, nNumberOfCharsToRead, lpNumberOfCharsRead, pInputControl);

	lbRc = F(ReadConsoleA)(hConsoleInput, lpBuffer, nNumberOfCharsToRead, lpNumberOfCharsRead, pInputControl);
	DWORD nErr = GetLastError();

	OnReadConsoleEnd(lbRc, FALSE, hConsoleInput, lpBuffer, nNumberOfCharsToRead, lpNumberOfCharsRead, pInputControl);
	SetLastError(nErr);

	UNREFERENCED_PARAMETER(bMainThread);

	return lbRc;
}

bool InitializeClink()
{
	if (gbClinkInitialized)
		return true;
	gbClinkInitialized = true; // Single

	if (!gnAllowClinkUsage)
		return false;

	CESERVER_CONSOLE_MAPPING_HDR* pConMap = GetConMap();
	if (!pConMap || !pConMap->bUseClink)
	{
		gnAllowClinkUsage = 0;
		return false;
	}

	// ��������� �����
	gnAllowClinkUsage = pConMap->bUseClink;

	BOOL bRunRc = FALSE;
	DWORD nErrCode = 0;

	if (pConMap->bUseClink == 2)
	{
		// New style. TODO
		wchar_t szClinkDir[MAX_PATH+32], szClinkArgs[MAX_PATH+64];

		wcscpy_c(szClinkDir, pConMap->sConEmuBaseDir);
		wcscat_c(szClinkDir, L"\\clink");

		wcscpy_c(szClinkArgs, L"\"");
		wcscat_c(szClinkArgs, szClinkDir);
		wcscat_c(szClinkArgs, WIN3264TEST(L"\\clink_x86.exe",L"\\clink_x64.exe"));
		wcscat_c(szClinkArgs, L"\" inject");

		STARTUPINFO si = {sizeof(si)};
		PROCESS_INFORMATION pi = {};
		bRunRc = CreateProcess(NULL, szClinkArgs, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, szClinkDir, &si, &pi);
		
		if (bRunRc)
		{
			WaitForSingleObject(pi.hProcess, INFINITE);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
		else
		{
			nErrCode = GetLastError();
			_ASSERTEX(FALSE && "Clink loader failed");
			UNREFERENCED_PARAMETER(nErrCode);
			UNREFERENCED_PARAMETER(bRunRc);
		}
	}
	else if (pConMap->bUseClink == 1)
	{
		if (!ghClinkDll)
		{
			wchar_t szClinkModule[MAX_PATH+30];
			_wsprintf(szClinkModule, SKIPLEN(countof(szClinkModule)) L"%s\\clink\\%s",
				pConMap->sConEmuBaseDir, WIN3264TEST(L"clink_dll_x86.dll",L"clink_dll_x64.dll"));
			
			ghClinkDll = LoadLibrary(szClinkModule);
			if (!ghClinkDll)
				return false;
		}

		if (!gpfnClinkReadLine)
		{
			gpfnClinkReadLine = (call_readline_t)GetProcAddress(ghClinkDll, "call_readline");
			_ASSERTEX(gpfnClinkReadLine!=NULL);
		}
	}

	return (gpfnClinkReadLine != NULL);
}

// cmd.exe only!
LONG WINAPI OnRegQueryValueExW(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
	typedef LONG (WINAPI* OnRegQueryValueExW_t)(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
	ORIGINALFASTEX(RegQueryValueExW,NULL);
	BOOL bMainThread = TRUE; // Does not care
	LONG lRc = -1;

	if (!gbClinkInitialized && hKey && lpValueName)
	{
		if (lstrcmpi(lpValueName, L"AutoRun") == 0)
		{
			InitializeClink();
		}
	}

	if (F(RegQueryValueExW))
		lRc = F(RegQueryValueExW)(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);

	return lRc;
}


BOOL WINAPI OnReadConsoleW(HANDLE hConsoleInput, LPVOID lpBuffer, DWORD nNumberOfCharsToRead, LPDWORD lpNumberOfCharsRead, PCONSOLE_READCONSOLE_CONTROL pInputControl)
{
	typedef BOOL (WINAPI* OnReadConsoleW_t)(HANDLE hConsoleInput, LPVOID lpBuffer, DWORD nNumberOfCharsToRead, LPDWORD lpNumberOfCharsRead, PCONSOLE_READCONSOLE_CONTROL pInputControl);
	SUPPRESSORIGINALSHOWCALL;
	ORIGINAL(ReadConsoleW);
	BOOL lbRc = FALSE, lbProcessed = FALSE;
	DWORD nErr = GetLastError();
	DWORD nStartTick = GetTickCount(), nEndTick = 0;

	OnReadConsoleStart(TRUE, hConsoleInput, lpBuffer, nNumberOfCharsToRead, lpNumberOfCharsRead, pInputControl);

	// ����������� ������ � ������� ������
	// ������� ���������� � telnet.exe (���������� ��� ���������� � Win7 x64) ������� � ���������
	if ((gnAllowClinkUsage == 1) && bMainThread && (nNumberOfCharsToRead > 1))
	{
		DWORD nConIn = 0;
		if (GetConsoleMode(hConsoleInput, &nConIn)
			&& (nConIn & ENABLE_ECHO_INPUT) && (nConIn & ENABLE_LINE_INPUT))
		{
			if (!gbClinkInitialized)
			{
				InitializeClink();
			}

			// Old style (clink 0.1.1)
			if (gpfnClinkReadLine)
			{
				lbProcessed = TRUE;
				*((wchar_t*)lpBuffer) = 0;

				gpfnClinkReadLine(gpszLastWriteConsole, (wchar_t*)lpBuffer, nNumberOfCharsToRead);

				// Copy from clink_dll.c::hooked_read_console
				{
				    // Check for control codes and convert them.
				    if (((wchar_t*)lpBuffer)[0] == L'\x03')
				    {
				        // Fire a Ctrl-C exception. Cmd.exe sets a global variable (CtrlCSeen)
				        // and ReadConsole() would normally set error code 0x3e3. Sleep() is to
				        // yield the thread so the global gets set (guess work...).
				        GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
				        SetLastError(0x3e3);
				        Sleep(0);

				        ((wchar_t*)lpBuffer)[0] = '\0';
				    }
			    }

				if (lpNumberOfCharsRead)
					*lpNumberOfCharsRead = lstrlen((wchar_t*)lpBuffer);
				lbRc = TRUE;
			}
		}
	}

	// ���� �� ���������� ����� clink - �� ����������, ����� API
	if (!lbProcessed)
	{
		lbRc = F(ReadConsoleW)(hConsoleInput, lpBuffer, nNumberOfCharsToRead, lpNumberOfCharsRead, pInputControl);
		nErr = GetLastError();
		// Debug purposes
		nEndTick = GetTickCount();
	}

	OnReadConsoleEnd(lbRc, TRUE, hConsoleInput, lpBuffer, nNumberOfCharsToRead, lpNumberOfCharsRead, pInputControl);
	SetLastError(nErr);

	UNREFERENCED_PARAMETER(nStartTick);
	UNREFERENCED_PARAMETER(nEndTick);
	return lbRc;
}


BOOL WINAPI OnGetNumberOfConsoleInputEvents(HANDLE hConsoleInput, LPDWORD lpcNumberOfEvents)
{
	typedef BOOL (WINAPI* OnGetNumberOfConsoleInputEvents_t)(HANDLE hConsoleInput, LPDWORD lpcNumberOfEvents);
	SUPPRESSORIGINALSHOWCALL;
	ORIGINAL(GetNumberOfConsoleInputEvents);
	BOOL lbRc = FALSE;

	//if (gpFarInfo && bMainThread)
	//	TouchReadPeekConsoleInputs();

	if (ph && ph->PreCallBack)
	{
		SETARGS2(&lbRc,hConsoleInput,lpcNumberOfEvents);

		// ���� ������� ���������� FALSE - �������� ������ �� ����� �������
		if (!ph->PreCallBack(&args))
			return lbRc;
	}

	lbRc = F(GetNumberOfConsoleInputEvents)(hConsoleInput, lpcNumberOfEvents);

	// Hiew fails pasting from clipboard on high speed
	if (gbIsHiewProcess && lpcNumberOfEvents && (*lpcNumberOfEvents > 1))
	{
		*lpcNumberOfEvents = 1;
	}

	if (ph && ph->PostCallBack)
	{
		SETARGS2(&lbRc,hConsoleInput,lpcNumberOfEvents);
		ph->PostCallBack(&args);
	}

	return lbRc;
}

BOOL WINAPI OnFlushConsoleInputBuffer(HANDLE hConsoleInput)
{
	typedef BOOL (WINAPI* OnFlushConsoleInputBuffer_t)(HANDLE hConsoleInput);
	SUPPRESSORIGINALSHOWCALL;
	ORIGINALFAST(FlushConsoleInputBuffer);
	BOOL lbRc = FALSE;

	//_ASSERTEX(FALSE && "calling FlushConsoleInputBuffer");
	DebugString(L"### calling FlushConsoleInputBuffer\n");

	lbRc = F(FlushConsoleInputBuffer)(hConsoleInput);

	return lbRc;
}

//typedef BOOL (WINAPI* OnPeekConsoleInputAx_t)(HANDLE,PINPUT_RECORD,DWORD,LPDWORD);
//BOOL WINAPI OnPeekConsoleInputAx(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead)
//{
//	ORIGINALx(PeekConsoleInputA);
//	//if (gpFarInfo && bMainThread)
//	//	TouchReadPeekConsoleInputs(1);
//	BOOL lbRc = FALSE;
//
//	if (ph && ph->PreCallBack)
//	{
//		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsRead);
//
//		// ���� ������� ���������� FALSE - �������� ������ �� ����� �������
//		if (!ph->PreCallBack(&args))
//			return lbRc;
//	}
//
//	#ifdef USE_INPUT_SEMAPHORE
//	DWORD nSemaphore = ghConInSemaphore ? WaitForSingleObject(ghConInSemaphore, INSEMTIMEOUT_READ) : 1;
//	_ASSERTE(nSemaphore<=1);
//	#endif
//
//	lbRc = Fx(PeekConsoleInputA)(hConsoleInput, lpBuffer, nLength, lpNumberOfEventsRead);
//
//	#ifdef USE_INPUT_SEMAPHORE
//	if ((nSemaphore == WAIT_OBJECT_0) && ghConInSemaphore) ReleaseSemaphore(ghConInSemaphore, 1, NULL);
//	#endif
//
//	if (ph && ph->PostCallBack)
//	{
//		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsRead);
//		ph->PostCallBack(&args);
//	}
//
//	return lbRc;
//}
//
//typedef BOOL (WINAPI* OnPeekConsoleInputWx_t)(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead);
//BOOL WINAPI OnPeekConsoleInputWx(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead)
//{
//	ORIGINALx(PeekConsoleInputW);
//	//if (gpFarInfo && bMainThread)
//	//	TouchReadPeekConsoleInputs(1);
//	BOOL lbRc = FALSE;
//
//	if (ph && ph->PreCallBack)
//	{
//		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsRead);
//
//		// ���� ������� ���������� FALSE - �������� ������ �� ����� �������
//		if (!ph->PreCallBack(&args))
//			return lbRc;
//	}
//
//	#ifdef USE_INPUT_SEMAPHORE
//	DWORD nSemaphore = ghConInSemaphore ? WaitForSingleObject(ghConInSemaphore, INSEMTIMEOUT_READ) : 1;
//	_ASSERTE(nSemaphore<=1);
//	#endif
//
//	lbRc = Fx(PeekConsoleInputW)(hConsoleInput, lpBuffer, nLength, lpNumberOfEventsRead);
//
//	#ifdef USE_INPUT_SEMAPHORE
//	if ((nSemaphore == WAIT_OBJECT_0) && ghConInSemaphore) ReleaseSemaphore(ghConInSemaphore, 1, NULL);
//	#endif
//
//	if (ph && ph->PostCallBack)
//	{
//		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsRead);
//		ph->PostCallBack(&args);
//	}
//
//	return lbRc;
//}
//
//typedef BOOL (WINAPI* OnReadConsoleInputAx_t)(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead);
//BOOL WINAPI OnReadConsoleInputAx(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead)
//{
//	ORIGINALx(ReadConsoleInputA);
//	//if (gpFarInfo && bMainThread)
//	//	TouchReadPeekConsoleInputs(0);
//	BOOL lbRc = FALSE;
//
//	if (ph && ph->PreCallBack)
//	{
//		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsRead);
//
//		// ���� ������� ���������� FALSE - �������� ������ �� ����� �������
//		if (!ph->PreCallBack(&args))
//			return lbRc;
//	}
//
//	#ifdef USE_INPUT_SEMAPHORE
//	DWORD nSemaphore = ghConInSemaphore ? WaitForSingleObject(ghConInSemaphore, INSEMTIMEOUT_READ) : 1;
//	_ASSERTE(nSemaphore<=1);
//	#endif
//
//	lbRc = Fx(ReadConsoleInputA)(hConsoleInput, lpBuffer, nLength, lpNumberOfEventsRead);
//
//	#ifdef USE_INPUT_SEMAPHORE
//	if ((nSemaphore == WAIT_OBJECT_0) && ghConInSemaphore) ReleaseSemaphore(ghConInSemaphore, 1, NULL);
//	#endif
//
//	if (ph && ph->PostCallBack)
//	{
//		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsRead);
//		ph->PostCallBack(&args);
//	}
//
//	return lbRc;
//}
//
//typedef BOOL (WINAPI* OnReadConsoleInputWx_t)(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead);
//BOOL WINAPI OnReadConsoleInputWx(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead)
//{
//	ORIGINALx(ReadConsoleInputW);
//	//if (gpFarInfo && bMainThread)
//	//	TouchReadPeekConsoleInputs(0);
//	BOOL lbRc = FALSE;
//
//	if (ph && ph->PreCallBack)
//	{
//		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsRead);
//
//		// ���� ������� ���������� FALSE - �������� ������ �� ����� �������
//		if (!ph->PreCallBack(&args))
//			return lbRc;
//	}
//
//	#ifdef USE_INPUT_SEMAPHORE
//	DWORD nSemaphore = ghConInSemaphore ? WaitForSingleObject(ghConInSemaphore, INSEMTIMEOUT_READ) : 1;
//	_ASSERTE(nSemaphore<=1);
//	#endif
//
//	lbRc = Fx(ReadConsoleInputW)(hConsoleInput, lpBuffer, nLength, lpNumberOfEventsRead);
//
//	#ifdef USE_INPUT_SEMAPHORE
//	if ((nSemaphore == WAIT_OBJECT_0) && ghConInSemaphore) ReleaseSemaphore(ghConInSemaphore, 1, NULL);
//	#endif
//
//	if (ph && ph->PostCallBack)
//	{
//		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsRead);
//		ph->PostCallBack(&args);
//	}
//
//	return lbRc;
//}

// ��� ����������� ������� Debug � ConEmu
void OnPeekReadConsoleInput(char acPeekRead/*'P'/'R'*/, char acUnicode/*'A'/'W'*/, HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nRead)
{
#ifdef TRAP_ON_MOUSE_0x0
	if (lpBuffer && nRead)
	{
		for (UINT i = 0; i < nRead; i++)
		{
			if (lpBuffer[i].EventType == MOUSE_EVENT)
			{
				if (lpBuffer[i].Event.MouseEvent.dwMousePosition.X == 0 && lpBuffer[i].Event.MouseEvent.dwMousePosition.Y == 0)
				{
					_ASSERTE(!(lpBuffer[i].Event.MouseEvent.dwMousePosition.X == 0 && lpBuffer[i].Event.MouseEvent.dwMousePosition.Y == 0));
				}
				//if (lpBuffer[i].Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED && lpBuffer[i].Event.MouseEvent.dwMousePosition.X != 5)
				//{
				//	_ASSERTE(!(lpBuffer[i].Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED && lpBuffer[i].Event.MouseEvent.dwMousePosition.X != 5));
				//}
			}
		}
	}
#endif

	DWORD nCurrentTID = GetCurrentThreadId();

	gReadConsoleInfo.LastReadConsoleInputTID = nCurrentTID;
	gReadConsoleInfo.hConsoleInput2 = hConsoleInput;

	if (nRead)
	{
		if (!gbWasSucceededInRead)
			gbWasSucceededInRead = TRUE;

		// ����� ������������ ��������
		GetConsoleScreenBufferInfoCached(NULL, NULL);
	}

	if (!gFarMode.bFarHookMode || !gFarMode.bMonitorConsoleInput || !nRead || !lpBuffer)
		return;
		
	//// ���� - ������ Read. Peek ����������
	//if (acPeekRead != 'R')
	//	return;
	
	CESERVER_REQ *pIn = ExecuteNewCmd(CECMD_PEEKREADINFO, sizeof(CESERVER_REQ_HDR) //-V119
		+sizeof(CESERVER_REQ_PEEKREADINFO)+(nRead-1)*sizeof(INPUT_RECORD));
	if (pIn)
	{
		pIn->PeekReadInfo.nCount = (WORD)nRead;
		pIn->PeekReadInfo.cPeekRead = acPeekRead;
		pIn->PeekReadInfo.cUnicode = acUnicode;
		pIn->PeekReadInfo.h = hConsoleInput;
		pIn->PeekReadInfo.nTID = nCurrentTID;
		pIn->PeekReadInfo.nPID = GetCurrentProcessId();
		pIn->PeekReadInfo.bMainThread = (pIn->PeekReadInfo.nTID == gnHookMainThreadId);
		memmove(pIn->PeekReadInfo.Buffer, lpBuffer, nRead*sizeof(INPUT_RECORD));
	
		CESERVER_REQ* pOut = ExecuteGuiCmd(ghConWnd, pIn, ghConWnd);
		if (pOut) ExecuteFreeResult(pOut);
		ExecuteFreeResult(pIn);
	}


}

#ifdef _DEBUG
void PreWriteConsoleInput(BOOL abUnicode, const INPUT_RECORD *lpBuffer, DWORD nLength)
{
#ifdef TRAP_ON_MOUSE_0x0
	if (lpBuffer && nLength)
	{
		for (UINT i = 0; i < nLength; i++)
		{
			if (lpBuffer[i].EventType == MOUSE_EVENT)
			{
				if (lpBuffer[i].Event.MouseEvent.dwMousePosition.X == 0 && lpBuffer[i].Event.MouseEvent.dwMousePosition.Y == 0)
				{
					_ASSERTE(!(lpBuffer[i].Event.MouseEvent.dwMousePosition.X == 0 && lpBuffer[i].Event.MouseEvent.dwMousePosition.Y == 0));
				}
				//if (lpBuffer[i].Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED && lpBuffer[i].Event.MouseEvent.dwMousePosition.X != 5)
				//{
				//	_ASSERTE(!(lpBuffer[i].Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED && lpBuffer[i].Event.MouseEvent.dwMousePosition.X != 5));
				//}
			}
		}
	}
#endif
}
#endif


BOOL WINAPI OnPeekConsoleInputA(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead)
{
	typedef BOOL (WINAPI* OnPeekConsoleInputA_t)(HANDLE,PINPUT_RECORD,DWORD,LPDWORD);
	SUPPRESSORIGINALSHOWCALL;
	ORIGINAL(PeekConsoleInputA);
	//if (gpFarInfo && bMainThread)
	//	TouchReadPeekConsoleInputs(1);
	BOOL lbRc = FALSE;

	if (ph && ph->PreCallBack)
	{
		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsRead);

		// ���� ������� ���������� FALSE - �������� ������ �� ����� �������
		if (!ph->PreCallBack(&args))
			return lbRc;
	}

	//#ifdef USE_INPUT_SEMAPHORE
	//DWORD nSemaphore = ghConInSemaphore ? WaitForSingleObject(ghConInSemaphore, INSEMTIMEOUT_READ) : 1;
	//_ASSERTE(nSemaphore<=1);
	//#endif

	lbRc = F(PeekConsoleInputA)(hConsoleInput, lpBuffer, nLength, lpNumberOfEventsRead);

	//#ifdef USE_INPUT_SEMAPHORE
	//if ((nSemaphore == WAIT_OBJECT_0) && ghConInSemaphore) ReleaseSemaphore(ghConInSemaphore, 1, NULL);
	//#endif
	
	if (ph && ph->PostCallBack)
	{
		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsRead);
		ph->PostCallBack(&args);
	}
	
	if (lbRc && lpNumberOfEventsRead && *lpNumberOfEventsRead && lpBuffer)
		OnPeekReadConsoleInput('P', 'A', hConsoleInput, lpBuffer, *lpNumberOfEventsRead);

	return lbRc;
}


BOOL WINAPI OnPeekConsoleInputW(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead)
{
	typedef BOOL (WINAPI* OnPeekConsoleInputW_t)(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead);
	SUPPRESSORIGINALSHOWCALL;
	ORIGINAL(PeekConsoleInputW);
	//if (gpFarInfo && bMainThread)
	//	TouchReadPeekConsoleInputs(1);
	BOOL lbRc = FALSE;

	if (ph && ph->PreCallBack)
	{
		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsRead);

		// ���� ������� ���������� FALSE - �������� ������ �� ����� �������
		if (!ph->PreCallBack(&args))
			return lbRc;
	}

	//#ifdef USE_INPUT_SEMAPHORE
	//DWORD nSemaphore = ghConInSemaphore ? WaitForSingleObject(ghConInSemaphore, INSEMTIMEOUT_READ) : 1;
	//_ASSERTE(nSemaphore<=1);
	//#endif

	if (gFarMode.bFarHookMode && USE_INTERNAL_QUEUE) // ecompl speed-up
	{
		#ifdef _DEBUG
		DWORD nDbgReadReal = countof(gir_Real), nDbgReadVirtual = countof(gir_Virtual);
		BOOL bReadReal = F(PeekConsoleInputW)(hConsoleInput, gir_Real, nDbgReadReal, &nDbgReadReal);
		BOOL bReadVirt = gInQueue.ReadInputQueue(gir_Virtual, &nDbgReadVirtual, TRUE);
		#endif

		if ((!lbRc || !(lpNumberOfEventsRead && *lpNumberOfEventsRead)) && !gInQueue.IsInputQueueEmpty())
		{
			DWORD n = nLength;
			lbRc = gInQueue.ReadInputQueue(lpBuffer, &n, TRUE);
			if (lpNumberOfEventsRead)
				*lpNumberOfEventsRead = lbRc ? n : 0;
		}
		else
		{
			lbRc = F(PeekConsoleInputW)(hConsoleInput, lpBuffer, nLength, lpNumberOfEventsRead);
		}
	}
	else
	{
	lbRc = F(PeekConsoleInputW)(hConsoleInput, lpBuffer, nLength, lpNumberOfEventsRead);
	}

	//#ifdef USE_INPUT_SEMAPHORE
	//if ((nSemaphore == WAIT_OBJECT_0) && ghConInSemaphore) ReleaseSemaphore(ghConInSemaphore, 1, NULL);
	//#endif

	if (ph && ph->PostCallBack)
	{
		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsRead);
		ph->PostCallBack(&args);
	}

	if (lbRc && lpNumberOfEventsRead && *lpNumberOfEventsRead && lpBuffer)
		OnPeekReadConsoleInput('P', 'W', hConsoleInput, lpBuffer, *lpNumberOfEventsRead);
//#ifdef _DEBUG
//	wchar_t szDbg[128];
//	if (lbRc && lpNumberOfEventsRead && *lpNumberOfEventsRead && lpBuffer && lpBuffer->EventType == MOUSE_EVENT)
//		msprintf(szDbg, L"ConEmuHk.OnPeekConsoleInputW(x%04X,x%04X) %u\n",
//			lpBuffer->Event.MouseEvent.dwButtonState, lpBuffer->Event.MouseEvent.dwControlKeyState, *lpNumberOfEventsRead);
//	else
//		lstrcpyW(szDbg, L"ConEmuHk.OnPeekConsoleInputW(Non mouse event)\n");
//	DebugStringW(szDbg);
//#endif

	return lbRc;
}


BOOL WINAPI OnReadConsoleInputA(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead)
{
	typedef BOOL (WINAPI* OnReadConsoleInputA_t)(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead);
	SUPPRESSORIGINALSHOWCALL;
	ORIGINAL(ReadConsoleInputA);
	//if (gpFarInfo && bMainThread)
	//	TouchReadPeekConsoleInputs(0);
	BOOL lbRc = FALSE;

	#if defined(_DEBUG)
	#if 1
	UINT nCp = GetConsoleCP();
	UINT nOutCp = GetConsoleOutputCP();
	UINT nOemCp = GetOEMCP();
	UINT nAnsiCp = GetACP();
	#endif
	#endif

	if (ph && ph->PreCallBack)
	{
		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsRead);

		// ���� ������� ���������� FALSE - �������� ������ �� ����� �������
		if (!ph->PreCallBack(&args))
			return lbRc;
	}

	//#ifdef USE_INPUT_SEMAPHORE
	//DWORD nSemaphore = ghConInSemaphore ? WaitForSingleObject(ghConInSemaphore, INSEMTIMEOUT_READ) : 1;
	//_ASSERTE(nSemaphore<=1);
	//#endif

	lbRc = F(ReadConsoleInputA)(hConsoleInput, lpBuffer, nLength, lpNumberOfEventsRead);

	//#ifdef USE_INPUT_SEMAPHORE
	//if ((nSemaphore == WAIT_OBJECT_0) && ghConInSemaphore) ReleaseSemaphore(ghConInSemaphore, 1, NULL);
	//#endif

	if (ph && ph->PostCallBack)
	{
		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsRead);
		ph->PostCallBack(&args);
	}

	if (lbRc && lpNumberOfEventsRead && *lpNumberOfEventsRead && lpBuffer)
		OnPeekReadConsoleInput('R', 'A', hConsoleInput, lpBuffer, *lpNumberOfEventsRead);

	return lbRc;
}


BOOL WINAPI OnReadConsoleInputW(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead)
{
	typedef BOOL (WINAPI* OnReadConsoleInputW_t)(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead);
	SUPPRESSORIGINALSHOWCALL;
	ORIGINAL(ReadConsoleInputW);
	//if (gpFarInfo && bMainThread)
	//	TouchReadPeekConsoleInputs(0);
	BOOL lbRc = FALSE;

	if (ph && ph->PreCallBack)
	{
		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsRead);

		// ���� ������� ���������� FALSE - �������� ������ �� ����� �������
		if (!ph->PreCallBack(&args))
			return lbRc;
	}

	//#ifdef USE_INPUT_SEMAPHORE
	//DWORD nSemaphore = ghConInSemaphore ? WaitForSingleObject(ghConInSemaphore, INSEMTIMEOUT_READ) : 1;
	//_ASSERTE(nSemaphore<=1);
	//#endif

	if (gFarMode.bFarHookMode && USE_INTERNAL_QUEUE) // ecompl speed-up
	{
		#ifdef _DEBUG
		DWORD nDbgReadReal = countof(gir_Real), nDbgReadVirtual = countof(gir_Virtual);
		BOOL bReadReal = PeekConsoleInputW(hConsoleInput, gir_Real, nDbgReadReal, &nDbgReadReal);
		BOOL bReadVirt = gInQueue.ReadInputQueue(gir_Virtual, &nDbgReadVirtual, TRUE);
		#endif

		if ((!lbRc || !(lpNumberOfEventsRead && *lpNumberOfEventsRead)) && !gInQueue.IsInputQueueEmpty())
		{
			DWORD n = nLength;
			lbRc = gInQueue.ReadInputQueue(lpBuffer, &n, FALSE);
			if (lpNumberOfEventsRead)
				*lpNumberOfEventsRead = lbRc ? n : 0;
		}
		else
		{
			lbRc = F(ReadConsoleInputW)(hConsoleInput, lpBuffer, nLength, lpNumberOfEventsRead);
		}
	}
	else
	{
	lbRc = F(ReadConsoleInputW)(hConsoleInput, lpBuffer, nLength, lpNumberOfEventsRead);
	}

	//#ifdef USE_INPUT_SEMAPHORE
	//if ((nSemaphore == WAIT_OBJECT_0) && ghConInSemaphore) ReleaseSemaphore(ghConInSemaphore, 1, NULL);
	//#endif

	if (ph && ph->PostCallBack)
	{
		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsRead);
		ph->PostCallBack(&args);
	}

	if (lbRc && lpNumberOfEventsRead && *lpNumberOfEventsRead && lpBuffer)
		OnPeekReadConsoleInput('R', 'W', hConsoleInput, lpBuffer, *lpNumberOfEventsRead);

	return lbRc;
}


BOOL WINAPI OnWriteConsoleInputA(HANDLE hConsoleInput, const INPUT_RECORD *lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsWritten)
{
	typedef BOOL (WINAPI* OnWriteConsoleInputA_t)(HANDLE hConsoleInput, const INPUT_RECORD *lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsWritten);
	ORIGINAL(WriteConsoleInputA);
	BOOL lbRc = FALSE;

	#ifdef _DEBUG
	PreWriteConsoleInput(FALSE, lpBuffer, nLength);
	#endif

	if (ph && ph->PreCallBack)
	{
		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsWritten);

		// ���� ������� ���������� FALSE - �������� ������ �� ����� �������
		if (!ph->PreCallBack(&args))
			return lbRc;
	}

	lbRc = F(WriteConsoleInputA)(hConsoleInput, lpBuffer, nLength, lpNumberOfEventsWritten);

	if (ph && ph->PostCallBack)
	{
		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsWritten);
		ph->PostCallBack(&args);
	}

	return lbRc;
}


BOOL WINAPI OnWriteConsoleInputW(HANDLE hConsoleInput, const INPUT_RECORD *lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsWritten)
{
	typedef BOOL (WINAPI* OnWriteConsoleInputW_t)(HANDLE hConsoleInput, const INPUT_RECORD *lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsWritten);
	ORIGINAL(WriteConsoleInputW);
	BOOL lbRc = FALSE;

	#ifdef _DEBUG
	PreWriteConsoleInput(FALSE, lpBuffer, nLength);
	#endif

	if (ph && ph->PreCallBack)
	{
		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsWritten);

		// ���� ������� ���������� FALSE - �������� ������ �� ����� �������
		if (!ph->PreCallBack(&args))
			return lbRc;
	}

	// ecompl speed-up
	if (gFarMode.bFarHookMode && USE_INTERNAL_QUEUE
		&& nLength && lpBuffer && lpBuffer->EventType == KEY_EVENT && lpBuffer->Event.KeyEvent.uChar.UnicodeChar)
	{
		#ifdef _DEBUG
		memset(gir_Written, 0, sizeof(gir_Written));
		memmove(gir_Written, lpBuffer, min(countof(gir_Written),nLength)*sizeof(*lpBuffer));
		#endif
		lbRc = gInQueue.WriteInputQueue(lpBuffer, FALSE, nLength);
	}
	else
	lbRc = F(WriteConsoleInputW)(hConsoleInput, lpBuffer, nLength, lpNumberOfEventsWritten);

	if (ph && ph->PostCallBack)
	{
		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsWritten);
		ph->PostCallBack(&args);
	}

	return lbRc;
}


BOOL WINAPI OnAllocConsole(void)
{
	typedef BOOL (WINAPI* OnAllocConsole_t)(void);
	ORIGINALFAST(AllocConsole);
	BOOL bMainThread = (GetCurrentThreadId() == gnHookMainThreadId);
	BOOL lbRc = FALSE, lbAllocated = FALSE;
	COORD crLocked;
	HMODULE hKernel = NULL;
	DWORD nErrCode = 0;
	BOOL lbAttachRc = FALSE;

	if (ph && ph->PreCallBack)
	{
		SETARGS(&lbRc);

		if (!ph->PreCallBack(&args))
			return lbRc;
	}

	// GUI ���������� �� �������. ���� ���� ������� ��� ��� - ����������� �����������
	// � ������������ ������� (������� ���������� ��������)
	if (gbAttachGuiClient || ghAttachGuiClient)
	{
		HWND hCurCon = GetRealConsoleWindow();
		if (hCurCon == NULL && gnServerPID != 0)
		{
			// ������� ���� ������ � WinXP � ����
			typedef BOOL (WINAPI* AttachConsole_t)(DWORD dwProcessId);
			hKernel = GetModuleHandle(L"kernel32.dll");
			AttachConsole_t _AttachConsole = hKernel ? (AttachConsole_t)GetProcAddress(hKernel, "AttachConsole") : NULL;
			if (_AttachConsole)
			{
				lbAttachRc = _AttachConsole(gnServerPID);
				if (lbAttachRc)
				{
					lbAllocated = TRUE; // ������� ��� ����, ������ �� ����
				}
				else
				{
					nErrCode = GetLastError();
					_ASSERTE(nErrCode==0 && lbAttachRc);
				}
			}
		}
	}

	if (!lbAllocated && F(AllocConsole))
	{
		lbRc = F(AllocConsole)();

		if (lbRc && IsVisibleRectLocked(crLocked))
		{
			// ������ _�������_ �������. ���������� ����������� ��������� ������ ��� "�������".
			// ������ ����� ������ ������ ������������ �������� ���� ConEmu
			HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
			CONSOLE_SCREEN_BUFFER_INFO csbi = {};
			if (GetConsoleScreenBufferInfo(hStdOut, &csbi))
			{
				//specified width and height cannot be less than the width and height of the console screen buffer's window
				SMALL_RECT rNewRect = {0, 0, crLocked.X-1, crLocked.Y-1};
				OnSetConsoleWindowInfo(hStdOut, TRUE, &rNewRect);
				#ifdef _DEBUG
				COORD crNewSize = {crLocked.X, max(crLocked.Y, csbi.dwSize.Y)};
				#endif
				SetConsoleScreenBufferSize(hStdOut, crLocked);
			}
		}
	}

	//InitializeConsoleInputSemaphore();

	if (ph && ph->PostCallBack)
	{
		SETARGS(&lbRc);
		ph->PostCallBack(&args);
	}

	// �������� ghConWnd � �������
	OnConWndChanged(GetRealConsoleWindow());

	TODO("����� �� �� ��������� ���������� ���������. ������� ��������, ��������");

	return lbRc;
}


BOOL WINAPI OnFreeConsole(void)
{
	typedef BOOL (WINAPI* OnFreeConsole_t)(void);
	ORIGINALFAST(FreeConsole);
	BOOL bMainThread = (GetCurrentThreadId() == gnHookMainThreadId);
	BOOL lbRc = FALSE;

	if (ph && ph->PreCallBack)
	{
		SETARGS(&lbRc);

		if (!ph->PreCallBack(&args))
			return lbRc;
	}

	//ReleaseConsoleInputSemaphore();

	lbRc = F(FreeConsole)();

	if (ph && ph->PostCallBack)
	{
		SETARGS(&lbRc);
		ph->PostCallBack(&args);
	}

	return lbRc;
}

HWND WINAPI OnGetConsoleWindow(void)
{
	typedef HWND (WINAPI* OnGetConsoleWindow_t)(void);
	ORIGINALFAST(GetConsoleWindow);

	_ASSERTE(F(GetConsoleWindow) != GetRealConsoleWindow);

	if (ghConEmuWndDC && user->isWindow(ghConEmuWndDC) /*ghConsoleHwnd*/)
	{
		if (ghAttachGuiClient)
		{
			// � GUI ������ (notepad, putty) �������� �������� ��������� GetConsoleWindow()
			// � ���� ������ �� ����� �������� �� ghConEmuWndDC, �� ��������� �������
			HWND hReal = GetRealConsoleWindow();
			return hReal;
		}
		else
		{
			//return ghConsoleHwnd;
			return ghConEmuWndDC;
		}
	}

	HWND h;
	h = F(GetConsoleWindow)();
	return h;
}

BOOL WINAPI OnGetConsoleMode(HANDLE hConsoleHandle,LPDWORD lpMode)
{
	typedef BOOL (WINAPI* OnGetConsoleMode_t)(HANDLE,LPDWORD);
	ORIGINALFAST(GetConsoleMode);
	BOOL b;

	#if 0
	if (gbIsBashProcess)
	{
		if (lpMode)
			*lpMode = 0;
		b = FALSE;
	}
	else
	#endif
	{
		b = F(GetConsoleMode)(hConsoleHandle,lpMode);
	}

	return b;
}


BOOL WINAPI OnWriteConsoleOutputA(HANDLE hConsoleOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion)
{
	typedef BOOL (WINAPI* OnWriteConsoleOutputA_t)(HANDLE hConsoleOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion);
	ORIGINAL(WriteConsoleOutputA);
	BOOL lbRc = FALSE;

	#ifdef _DEBUG
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	GetConsoleMode(hConsoleOutput, &dwMode);
	#endif

	if (ph && ph->PreCallBack)
	{
		SETARGS5(&lbRc, hConsoleOutput, lpBuffer, &dwBufferSize, &dwBufferCoord, lpWriteRegion);
		ph->PreCallBack(&args);
	}

	lbRc = F(WriteConsoleOutputA)(hConsoleOutput, lpBuffer, dwBufferSize, dwBufferCoord, lpWriteRegion);

	if (ph && ph->PostCallBack)
	{
		SETARGS5(&lbRc, hConsoleOutput, lpBuffer, &dwBufferSize, &dwBufferCoord, lpWriteRegion);
		ph->PostCallBack(&args);
	}

	return lbRc;
}

// PowerShell AI ��� ����������� ��������� � �������
void CheckPowerShellProgress(HANDLE hConsoleOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion)
{
	_ASSERTE(dwBufferSize.Y >= 5);
	int nProgress = -1;
	WORD nNeedAttr = lpBuffer->Attributes;

	for (SHORT Y = dwBufferSize.Y - 2; Y > 0; Y--)
	{
		const CHAR_INFO* pLine = lpBuffer + dwBufferSize.X * Y;

		// 120720 - PS ���������� PopupColors � �������. ����� ��������� ������ ���� 0x3E
		if (nNeedAttr/*gnConsolePopupColors*/ != 0)
		{
			if ((pLine[4].Attributes != nNeedAttr/*gnConsolePopupColors*/)
				|| (pLine[dwBufferSize.X - 7].Attributes != nNeedAttr/*gnConsolePopupColors*/))
				break; // �� ���
		}

		if ((pLine[4].Char.UnicodeChar == L'[') && (pLine[dwBufferSize.X - 7].Char.UnicodeChar == L']'))
		{
			// ������� ��������
			SHORT nLen = dwBufferSize.X - 7 - 5;
			if (nLen > 0)
			{
				nProgress = 100;
				for (SHORT X = 5; X < (dwBufferSize.X - 8); X++)
				{
					if (pLine[X].Char.UnicodeChar == L' ')
					{
						nProgress = (X - 5) * 100 / nLen;
						break;
					}
				}
			}
			break;
		}
	}

	if (nProgress != gnPowerShellProgressValue)
	{
		gnPowerShellProgressValue = nProgress;
		GuiSetProgress((nProgress != -1) ? 1 : 0, nProgress);
	}
}


BOOL WINAPI OnWriteConsoleOutputW(HANDLE hConsoleOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion)
{
	typedef BOOL (WINAPI* OnWriteConsoleOutputW_t)(HANDLE hConsoleOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion);
	ORIGINAL(WriteConsoleOutputW);
	BOOL lbRc = FALSE;

	if (ph && ph->PreCallBack)
	{
		SETARGS5(&lbRc, hConsoleOutput, lpBuffer, &dwBufferSize, &dwBufferCoord, lpWriteRegion);
		ph->PreCallBack(&args);
	}

	// PowerShell AI ��� ����������� ��������� � �������
	if (gbPowerShellMonitorProgress)
	{
		// ��������� �������� "�������� �� ���"
		if ((dwBufferSize.Y >= 5) && !dwBufferCoord.X && !dwBufferCoord.Y
			&& lpWriteRegion && !lpWriteRegion->Left && (lpWriteRegion->Right == (dwBufferSize.X - 1))
			&& lpBuffer && (lpBuffer->Char.UnicodeChar == L' '))
		{
			#ifdef _DEBUG
			MY_CONSOLE_SCREEN_BUFFER_INFOEX csbi6 = {sizeof(csbi6)};
			apiGetConsoleScreenBufferInfoEx(hConsoleOutput, &csbi6);
			#endif
			// 120720 - PS ���������� PopupColors � �������. ����� ��������� ������ ���� 0x3E
			//&& (!gnConsolePopupColors || (lpBuffer->Attributes == gnConsolePopupColors)))
			if (lpBuffer->Attributes == 0x3E)
			{
				CheckPowerShellProgress(hConsoleOutput, lpBuffer, dwBufferSize, dwBufferCoord, lpWriteRegion);
			}
		}
	}

	lbRc = F(WriteConsoleOutputW)(hConsoleOutput, lpBuffer, dwBufferSize, dwBufferCoord, lpWriteRegion);

	if (ph && ph->PostCallBack)
	{
		SETARGS5(&lbRc, hConsoleOutput, lpBuffer, &dwBufferSize, &dwBufferCoord, lpWriteRegion);
		ph->PostCallBack(&args);
	}

	return lbRc;
}


// Sets the attributes of characters written to the console screen buffer by
// the WriteFile or WriteConsole function, or echoed by the ReadFile or ReadConsole function.
// This function affects text written after the function call.
BOOL WINAPI OnSetConsoleTextAttribute(HANDLE hConsoleOutput, WORD wAttributes)
{
	typedef BOOL (WINAPI* OnSetConsoleTextAttribute_t)(HANDLE hConsoleOutput, WORD wAttributes);
	ORIGINALFAST(SetConsoleTextAttribute);

	BOOL lbRc = FALSE;

	if (ph && ph->PreCallBack)
	{
		BOOL bMainThread = (GetCurrentThreadId() == gnHookMainThreadId);
		SETARGS2(&lbRc, hConsoleOutput, wAttributes);
		ph->PreCallBack(&args);
	}
	
	#ifdef _DEBUG
	// ���� �� ������� ConEmu ����������� GUI ���������� - ��� ������
	// ��� ��� ������ �� ����� ��������
	if ((ghAttachGuiClient == NULL) && !gbAttachGuiClient && (wAttributes != 7))
	{
		//// ���-�� � ��������� ������� ��������� ���� ������ ��� printf
		//_ASSERTE("SetConsoleTextAttribute" && (wAttributes==0x07));
		wchar_t szDbgInfo[128];
		msprintf(szDbgInfo, countof(szDbgInfo), L"PID=%u, SetConsoleTextAttribute=0x%02X(%u)\n", GetCurrentProcessId(), (int)wAttributes, (int)wAttributes);
		DebugString(szDbgInfo);
	}
	#endif

	lbRc = F(SetConsoleTextAttribute)(hConsoleOutput, wAttributes);

	if (ph && ph->PostCallBack)
	{
		BOOL bMainThread = (GetCurrentThreadId() == gnHookMainThreadId);
		SETARGS2(&lbRc, hConsoleOutput, wAttributes);
		ph->PostCallBack(&args);
	}

	return lbRc;
}

typedef BOOL (WINAPI* SimpleApiFunction_t)(void*);
struct SimpleApiFunctionArg
{
	enum SimpleApiFunctionType
	{
		sft_ChooseColorA = 1,
		sft_ChooseColorW = 2,
	} funcType;
	SimpleApiFunction_t funcPtr;
	void  *pArg;
	BOOL   bResult;
	DWORD  nLastError;
};

INT_PTR CALLBACK SimpleApiDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SimpleApiFunctionArg* P = NULL;

	//typedef LONG_PTR (WINAPI* SetWindowLongPtr_t)(HWND,int,LONG_PTR);
	//SetWindowLongPtr_t SetWindowLongPtr_f = (SetWindowLongPtr_t)GetProcAddress(ghUser32,WIN3264TEST("SetWindowLongW","SetWindowLongPtrW"));
	
	if (uMsg == WM_INITDIALOG)
	{
		P = (SimpleApiFunctionArg*)lParam;
		user->setWindowLongPtrW(hwndDlg, GWLP_USERDATA, lParam);
		
		int x = 0, y = 0;
		RECT rcDC = {};
		if (!user->getWindowRect(ghConEmuWndDC, &rcDC))
			user->systemParametersInfoW(SPI_GETWORKAREA, 0, &rcDC, 0);
		if (rcDC.right > rcDC.left && rcDC.bottom > rcDC.top)
		{
			x = max(rcDC.left, ((rcDC.right+rcDC.left-300)>>1));
			y = max(rcDC.top, ((rcDC.bottom+rcDC.top-200)>>1));
		}
		
		//typedef BOOL (WINAPI* SetWindowText_t)(HWND,LPCWSTR);
		//SetWindowText_t SetWindowText_f = (SetWindowText_t)GetProcAddress(ghUser32, "SetWindowTextW");

		
		switch (P->funcType)
		{
			case SimpleApiFunctionArg::sft_ChooseColorA:
				((LPCHOOSECOLORA)P->pArg)->hwndOwner = hwndDlg;
				user->setWindowTextW(hwndDlg, L"ConEmu ColorPicker");
				break;
			case SimpleApiFunctionArg::sft_ChooseColorW:
				((LPCHOOSECOLORW)P->pArg)->hwndOwner = hwndDlg;
				user->setWindowTextW(hwndDlg, L"ConEmu ColorPicker");
				break;
		}
		
		user->setWindowPos(hwndDlg, HWND_TOP, x, y, 0, 0, SWP_SHOWWINDOW);
		
		P->bResult = P->funcPtr(P->pArg);
		P->nLastError = GetLastError();
		
		//typedef BOOL (WINAPI* EndDialog_t)(HWND,INT_PTR);
		//EndDialog_t EndDialog_f = (EndDialog_t)GetProcAddress(ghUser32, "EndDialog");
		user->endDialog(hwndDlg, 1);
		
		return FALSE;
	}

	switch (uMsg)
	{
	case WM_GETMINMAXINFO:
		{
			MINMAXINFO* p = (MINMAXINFO*)lParam;
			p->ptMinTrackSize.x = p->ptMinTrackSize.y = 0;
			user->setWindowLongPtrW(hwndDlg, DWLP_MSGRESULT, 0);
		}
		return TRUE;
	}
	
	return FALSE;
}

BOOL MyChooseColor(SimpleApiFunction_t funcPtr, void* lpcc, BOOL abUnicode)
{
	BOOL lbRc = FALSE;

	SimpleApiFunctionArg Arg =
	{
		abUnicode?SimpleApiFunctionArg::sft_ChooseColorW:SimpleApiFunctionArg::sft_ChooseColorA,
		funcPtr, lpcc
	};

	DLGTEMPLATE* pTempl = (DLGTEMPLATE*)GlobalAlloc(GPTR, sizeof(DLGTEMPLATE)+3*sizeof(WORD)); //-V119
	if (!pTempl)
	{
		_ASSERTE(pTempl!=NULL);
		Arg.bResult = Arg.funcPtr(Arg.pArg);
		Arg.nLastError = GetLastError();
		return Arg.bResult;
	}
	pTempl->style = WS_POPUP|/*DS_MODALFRAME|DS_CENTER|*/DS_SETFOREGROUND;
	
	
	//typedef INT_PTR (WINAPI* DialogBoxIndirectParam_t)(HINSTANCE,LPCDLGTEMPLATE,HWND,DLGPROC,LPARAM);
	//DialogBoxIndirectParam_t DialogBoxIndirectParam_f = (DialogBoxIndirectParam_t)GetProcAddress(ghUser32, "DialogBoxIndirectParamW");
	
	//if (DialogBoxIndirectParam_f)
	//{

	INT_PTR iRc = user->dialogBoxIndirectParamW(ghOurModule, pTempl, NULL, SimpleApiDialogProc, (LPARAM)&Arg);
	if (iRc == 1)
	{
		lbRc = Arg.bResult;
		SetLastError(Arg.nLastError);
	}

	//}
	
	return lbRc;
}

BOOL WINAPI OnChooseColorA(LPCHOOSECOLORA lpcc)
{
	ORIGINALFASTEX(ChooseColorA,ChooseColorA_f);
	BOOL lbRc;
	if (lpcc->hwndOwner == NULL || lpcc->hwndOwner == GetConsoleWindow())
		lbRc = MyChooseColor((SimpleApiFunction_t)F(ChooseColorA), lpcc, FALSE);
	else
		lbRc = F(ChooseColorA)(lpcc);
	return lbRc;
}

BOOL WINAPI OnChooseColorW(LPCHOOSECOLORW lpcc)
{
	ORIGINALFASTEX(ChooseColorW,ChooseColorW_f);
	BOOL lbRc;
 	if (lpcc->hwndOwner == NULL || lpcc->hwndOwner == GetConsoleWindow())
		lbRc = MyChooseColor((SimpleApiFunction_t)F(ChooseColorW), lpcc, TRUE);
	else
		lbRc = F(ChooseColorW)(lpcc);
	return lbRc;
}

// Undocumented function
BOOL WINAPI OnSetConsoleKeyShortcuts(BOOL bSet, BYTE bReserveKeys, LPVOID p1, DWORD n1)
{
	typedef BOOL (WINAPI* OnSetConsoleKeyShortcuts_t)(BOOL,BYTE,LPVOID,DWORD);
	ORIGINALFASTEX(SetConsoleKeyShortcuts,NULL);
	BOOL lbRc = FALSE;

	if (F(SetConsoleKeyShortcuts))
		lbRc = F(SetConsoleKeyShortcuts)(bSet, bReserveKeys, p1, n1);

	if (ghConEmuWnd && user->isWindow(ghConEmuWnd))
	{
		DWORD nLastErr = GetLastError();
		DWORD nSize = sizeof(CESERVER_REQ_HDR)+sizeof(BYTE)*2;
		CESERVER_REQ *pIn = ExecuteNewCmd(CECMD_KEYSHORTCUTS, nSize);
		if (pIn)
		{
			pIn->Data[0] = bSet;
			pIn->Data[1] = bReserveKeys;

			wchar_t szGuiPipeName[128];
			msprintf(szGuiPipeName, countof(szGuiPipeName), CEGUIPIPENAME, L".", (DWORD)ghConWnd);

			CESERVER_REQ* pOut = ExecuteCmd(szGuiPipeName, pIn, 1000, NULL);

			if (pOut)
				ExecuteFreeResult(pOut);
			ExecuteFreeResult(pIn);
		}
		SetLastError(nLastErr);
	}

	return lbRc;
}

//typedef BOOL (WINAPI* OnWriteConsoleOutputAx_t)(HANDLE hConsoleOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion);
//BOOL WINAPI OnWriteConsoleOutputAx(HANDLE hConsoleOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion)
//{
//	ORIGINALx(WriteConsoleOutputA);
//	BOOL lbRc = FALSE;
//
//	if (ph && ph->PreCallBack)
//	{
//		SETARGS5(&lbRc, hConsoleOutput, lpBuffer, &dwBufferSize, &dwBufferCoord, lpWriteRegion);
//		ph->PreCallBack(&args);
//	}
//
//	lbRc = Fx(WriteConsoleOutputA)(hConsoleOutput, lpBuffer, dwBufferSize, dwBufferCoord, lpWriteRegion);
//
//	if (ph && ph->PostCallBack)
//	{
//		SETARGS5(&lbRc, hConsoleOutput, lpBuffer, &dwBufferSize, &dwBufferCoord, lpWriteRegion);
//		ph->PostCallBack(&args);
//	}
//
//	return lbRc;
//}
//
//typedef BOOL (WINAPI* OnWriteConsoleOutputWx_t)(HANDLE hConsoleOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion);
//BOOL WINAPI OnWriteConsoleOutputWx(HANDLE hConsoleOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion)
//{
//	ORIGINALx(WriteConsoleOutputW);
//	BOOL lbRc = FALSE;
//
//	if (ph && ph->PreCallBack)
//	{
//		SETARGS5(&lbRc, hConsoleOutput, lpBuffer, &dwBufferSize, &dwBufferCoord, lpWriteRegion);
//		ph->PreCallBack(&args);
//	}
//
//	lbRc = Fx(WriteConsoleOutputW)(hConsoleOutput, lpBuffer, dwBufferSize, dwBufferCoord, lpWriteRegion);
//
//	if (ph && ph->PostCallBack)
//	{
//		SETARGS5(&lbRc, hConsoleOutput, lpBuffer, &dwBufferSize, &dwBufferCoord, lpWriteRegion);
//		ph->PostCallBack(&args);
//	}
//
//	return lbRc;
//}

#ifdef _DEBUG
HANDLE WINAPI OnCreateNamedPipeW(LPCWSTR lpName, DWORD dwOpenMode, DWORD dwPipeMode, DWORD nMaxInstances,DWORD nOutBufferSize, DWORD nInBufferSize, DWORD nDefaultTimeOut,LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	typedef HANDLE(WINAPI* OnCreateNamedPipeW_t)(LPCWSTR lpName, DWORD dwOpenMode, DWORD dwPipeMode, DWORD nMaxInstances,DWORD nOutBufferSize, DWORD nInBufferSize, DWORD nDefaultTimeOut,LPSECURITY_ATTRIBUTES lpSecurityAttributes);
	ORIGINALFAST(CreateNamedPipeW);
#ifdef _DEBUG

	if (!lpName || !*lpName)
	{
		_ASSERTE(lpName && *lpName);
	}
	else
	{
		int nLen = lstrlen(lpName)+64;
		wchar_t* psz = (wchar_t*)malloc(nLen*sizeof(*psz));
		if (psz)
		{
			msprintf(psz, nLen, L"CreateNamedPipeW(%s)\n", lpName);
			DebugString(psz);
			free(psz);
		}
	}

#endif
	HANDLE hPipe = F(CreateNamedPipeW)(lpName, dwOpenMode, dwPipeMode, nMaxInstances, nOutBufferSize, nInBufferSize, nDefaultTimeOut, lpSecurityAttributes);
	return hPipe;
}
#endif

#ifdef _DEBUG
LPVOID WINAPI OnVirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect)
{
	typedef HANDLE(WINAPI* OnVirtualAlloc_t)(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
	ORIGINALFAST(VirtualAlloc);
	LPVOID lpResult = F(VirtualAlloc)(lpAddress, dwSize, flAllocationType, flProtect);
	if (lpResult == NULL)
	{
		DWORD nErr = GetLastError();
		//_ASSERTE(lpResult != NULL);
		wchar_t szText[MAX_PATH*2], szTitle[64];
		msprintf(szTitle, countof(szTitle), L"ConEmuHk, PID=%u, TID=%u", GetCurrentProcessId(), GetCurrentThreadId());
		msprintf(szText, countof(szText),
			L"\nVirtualAlloc " WIN3264TEST(L"%u",L"x%X%08X") L" bytes failed (0x%08X..0x%08X)\nErrorCode=%u %s\n\nWarning! This will be an error in Release!\n\n",
			WIN3264WSPRINT(dwSize),
			(DWORD)lpAddress, (DWORD)((LPBYTE)lpAddress+dwSize),
			nErr, (nErr == 487) ? L"(ERROR_INVALID_ADDRESS)" : L"");
		GetModuleFileName(NULL, szText+lstrlen(szText), MAX_PATH);

	#if defined(REPORT_VIRTUAL_ALLOC)
		int iBtn = GuiMessageBox(NULL, szText, szTitle, MB_SYSTEMMODAL|MB_OKCANCEL|MB_ICONSTOP);
		SetLastError(nErr);
		if (iBtn == IDOK)
		{
			lpResult = F(VirtualAlloc)(NULL, dwSize, flAllocationType, flProtect);
		}
	#else
		OutputDebugString(szText);
		SetLastError(nErr);
	#endif
	}
	return lpResult;
}
#endif

//110131 ��������� ������ ��������� �� � ExcludedModules
//// WinInet.dll
//typedef BOOL (WINAPI* OnHttpSendRequestA_t)(LPVOID hRequest, LPCSTR lpszHeaders, DWORD dwHeadersLength, LPVOID lpOptional, DWORD dwOptionalLength);
//typedef BOOL (WINAPI* OnHttpSendRequestW_t)(LPVOID hRequest, LPCWSTR lpszHeaders, DWORD dwHeadersLength, LPVOID lpOptional, DWORD dwOptionalLength);
//// ������ ��� - __try �� ��������
////BOOL OnHttpSendRequestA_SEH(OnHttpSendRequestA_t f, LPVOID hRequest, LPCSTR lpszHeaders, DWORD dwHeadersLength, LPVOID lpOptional, DWORD dwOptionalLength, BOOL* pbRc)
////{
////	BOOL lbOk = FALSE;
////	SAFETRY {
////		*pbRc = f(hRequest, lpszHeaders, dwHeadersLength, lpOptional, dwOptionalLength);
////		lbOk = TRUE;
////	} SAFECATCH {
////		lbOk = FALSE;
////	}
////	return lbOk;
////}
////BOOL OnHttpSendRequestW_SEH(OnHttpSendRequestW_t f, LPVOID hRequest, LPCWSTR lpszHeaders, DWORD dwHeadersLength, LPVOID lpOptional, DWORD dwOptionalLength, BOOL* pbRc)
////{
////	BOOL lbOk = FALSE;
////	SAFETRY {
////		*pbRc = f(hRequest, lpszHeaders, dwHeadersLength, lpOptional, dwOptionalLength);
////		lbOk = TRUE;
////	} SAFECATCH {
////		lbOk = FALSE;
////	}
////	return lbOk;
////}
//BOOL WINAPI OnHttpSendRequestA(LPVOID hRequest, LPCSTR lpszHeaders, DWORD dwHeadersLength, LPVOID lpOptional, DWORD dwOptionalLength)
//{
//	//MessageBoxW(NULL, L"HttpSendRequestA (1)", L"ConEmu plugin", MB_SETFOREGROUND|MB_SYSTEMMODAL|MB_ICONEXCLAMATION);
//	ORIGINALFAST(HttpSendRequestA);
//
//	BOOL lbRc;
//
//	gbHooksTemporaryDisabled = TRUE;
//	//MessageBoxW(NULL, L"HttpSendRequestA (2)", L"ConEmu plugin", MB_SETFOREGROUND|MB_SYSTEMMODAL|MB_ICONEXCLAMATION);
//	lbRc = F(HttpSendRequestA)(hRequest, lpszHeaders, dwHeadersLength, lpOptional, dwOptionalLength);
//	//if (!OnHttpSendRequestA_SEH(F(HttpSendRequestA), hRequest, lpszHeaders, dwHeadersLength, lpOptional, dwOptionalLength)) {
//	//	MessageBoxW(NULL, L"Exception in HttpSendRequestA", L"ConEmu plugin", MB_SETFOREGROUND|MB_SYSTEMMODAL|MB_ICONSTOP);
//	//}
//	gbHooksTemporaryDisabled = FALSE;
//	//MessageBoxW(NULL, L"HttpSendRequestA (3)", L"ConEmu plugin", MB_SETFOREGROUND|MB_SYSTEMMODAL|MB_ICONEXCLAMATION);
//
//	return lbRc;
//}
//BOOL WINAPI OnHttpSendRequestW(LPVOID hRequest, LPCWSTR lpszHeaders, DWORD dwHeadersLength, LPVOID lpOptional, DWORD dwOptionalLength)
//{
//	//MessageBoxW(NULL, L"HttpSendRequestW (1)", L"ConEmu plugin", MB_SETFOREGROUND|MB_SYSTEMMODAL|MB_ICONEXCLAMATION);
//	ORIGINALFAST(HttpSendRequestW);
//
//	BOOL lbRc;
//
//	gbHooksTemporaryDisabled = TRUE;
//	//MessageBoxW(NULL, L"HttpSendRequestW (2)", L"ConEmu plugin", MB_SETFOREGROUND|MB_SYSTEMMODAL|MB_ICONEXCLAMATION);
//	lbRc = F(HttpSendRequestW)(hRequest, lpszHeaders, dwHeadersLength, lpOptional, dwOptionalLength);
//	//if (!OnHttpSendRequestW_SEH(F(HttpSendRequestW), hRequest, lpszHeaders, dwHeadersLength, lpOptional, dwOptionalLength, &lbRc)) {
//	//	MessageBoxW(NULL, L"Exception in HttpSendRequestW", L"ConEmu plugin", MB_SETFOREGROUND|MB_SYSTEMMODAL|MB_ICONSTOP);
//	//}
//	gbHooksTemporaryDisabled = FALSE;
//	//MessageBoxW(NULL, L"HttpSendRequestW (3)", L"ConEmu plugin", MB_SETFOREGROUND|MB_SYSTEMMODAL|MB_ICONEXCLAMATION);
//
//	return lbRc;
//}

BOOL GuiSetForeground(HWND hWnd)
{
	BOOL lbRc = FALSE;

	if (ghConEmuWndDC)
	{
		CESERVER_REQ *pIn = (CESERVER_REQ*)malloc(sizeof(*pIn)), *pOut;
		if (pIn)
		{
			ExecutePrepareCmd(pIn, CECMD_SETFOREGROUND, sizeof(CESERVER_REQ_HDR)+sizeof(u64)); //-V119

			DWORD nConEmuPID = ASFW_ANY;
			user->getWindowThreadProcessId(ghConEmuWndDC, &nConEmuPID);
			user->allowSetForegroundWindow(nConEmuPID);

			pIn->qwData[0] = (u64)hWnd;
			HWND hConWnd = GetRealConsoleWindow();
			pOut = ExecuteGuiCmd(hConWnd, pIn, hConWnd);

			if (pOut)
			{
				if (pOut->hdr.cbSize == (sizeof(CESERVER_REQ_HDR)+sizeof(DWORD)))
					lbRc = pOut->dwData[0];
				ExecuteFreeResult(pOut);
			}
			free(pIn);
		}
	}

	return lbRc;
}

void GuiFlashWindow(BOOL bSimple, HWND hWnd, BOOL bInvert, DWORD dwFlags, UINT uCount, DWORD dwTimeout)
{
	if (ghConEmuWndDC)
	{
		CESERVER_REQ *pIn = (CESERVER_REQ*)malloc(sizeof(*pIn)), *pOut;
		if (pIn)
		{
			ExecutePrepareCmd(pIn, CECMD_FLASHWINDOW, sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_FLASHWINFO)); //-V119
			pIn->Flash.bSimple = bSimple;
			pIn->Flash.hWnd = hWnd;
			pIn->Flash.bInvert = bInvert;
			pIn->Flash.dwFlags = dwFlags;
			pIn->Flash.uCount = uCount;
			pIn->Flash.dwTimeout = dwTimeout;
			HWND hConWnd = GetConsoleWindow();
			pOut = ExecuteGuiCmd(hConWnd, pIn, hConWnd);

			if (pOut) ExecuteFreeResult(pOut);
			free(pIn);
		}
	}
}

#ifdef _DEBUG
// IsConsoleHandle
bool IsHandleConsole(HANDLE handle, bool output = true)
{
	// MSDN ����������� ���������� GetConsoleMode (��� � Input � Output �����)

	// �������?
	if (((DWORD)handle & 0x10000003) != 3)
		return false;

	// �������� ���� ����������� ������ (In/Out)
	DWORD num;

	if (!output)
		if (GetNumberOfConsoleInputEvents(handle, &num))
			return true;
		else
			return false;
	else if (GetNumberOfConsoleInputEvents(handle, &num))
		return false;
	else
		return true;
}
#endif

static BOOL MyGetConsoleFontSize(COORD& crFontSize)
{
	BOOL lbRc = FALSE;

	if (ghConEmuWnd)
	{
		_ASSERTE(gnGuiPID!=0);
		ConEmuGuiMapping* inf = (ConEmuGuiMapping*)malloc(sizeof(ConEmuGuiMapping));
		if (inf)
		{
			if (LoadGuiMapping(gnGuiPID, *inf))
			{
				crFontSize.Y = (SHORT)inf->MainFont.nFontHeight;
				crFontSize.X = (SHORT)inf->MainFont.nFontCellWidth;
				lbRc = TRUE;
			}
			free(inf);
		}

		_ASSERTEX(lbRc && (crFontSize.X > 3) && (crFontSize.Y > 4));
	}

	return lbRc;
}

BOOL WINAPI OnGetCurrentConsoleFont(HANDLE hConsoleOutput, BOOL bMaximumWindow, PCONSOLE_FONT_INFO lpConsoleCurrentFont)
{
	typedef BOOL (WINAPI* OnGetCurrentConsoleFont_t)(HANDLE hConsoleOutput, BOOL bMaximumWindow, PCONSOLE_FONT_INFO lpConsoleCurrentFont);
	ORIGINALFASTEX(GetCurrentConsoleFont,NULL);
	BOOL lbRc = FALSE;
	COORD crSize = {};

	if (lpConsoleCurrentFont && MyGetConsoleFontSize(crSize))
	{
		lpConsoleCurrentFont->dwFontSize = crSize;
		lpConsoleCurrentFont->nFont = 1;
		lbRc = TRUE;
	}
	else if (F(GetCurrentConsoleFont))
		lbRc = F(GetCurrentConsoleFont)(hConsoleOutput, bMaximumWindow, lpConsoleCurrentFont);

	return lbRc;
}

COORD WINAPI OnGetConsoleFontSize(HANDLE hConsoleOutput, DWORD nFont)
{
	typedef COORD (WINAPI* OnGetConsoleFontSize_t)(HANDLE hConsoleOutput, DWORD nFont);
	ORIGINALFASTEX(GetConsoleFontSize,NULL);
	COORD cr = {};

	if (!MyGetConsoleFontSize(cr))
	{
		if (F(GetConsoleFontSize))
			cr = F(GetConsoleFontSize)(hConsoleOutput, nFont);
	}

	return cr;
}

void CheckVariables()
{
	// ���� ��� �� ��������� � ������ ������ ENV_CONEMUANSI_VAR_W ("ConEmuANSI")
	GetConMap(FALSE);
}

DWORD WINAPI OnGetEnvironmentVariableA(LPCSTR lpName, LPSTR lpBuffer, DWORD nSize)
{
	typedef DWORD (WINAPI* OnGetEnvironmentVariableA_t)(LPCSTR lpName, LPSTR lpBuffer, DWORD nSize);
	ORIGINALFAST(GetEnvironmentVariableA);
	//BOOL bMainThread = FALSE; // ����� �� �����

	if (lpName && (
			(lstrcmpiA(lpName, ENV_CONEMUANSI_VAR_A) == 0)
			|| (lstrcmpiA(lpName, ENV_CONEMUHWND_VAR_A) == 0)
			|| (lstrcmpiA(lpName, ENV_CONEMUDIR_VAR_A) == 0)
			|| (lstrcmpiA(lpName, ENV_CONEMUBASEDIR_VAR_A) == 0)
		))
	{
		CheckVariables();
	}

	BOOL lbRc = F(GetEnvironmentVariableA)(lpName, lpBuffer, nSize);

	return lbRc;
}

DWORD WINAPI OnGetEnvironmentVariableW(LPCWSTR lpName, LPWSTR lpBuffer, DWORD nSize)
{
	typedef DWORD (WINAPI* OnGetEnvironmentVariableW_t)(LPCWSTR lpName, LPWSTR lpBuffer, DWORD nSize);
	ORIGINALFAST(GetEnvironmentVariableW);
	//BOOL bMainThread = FALSE; // ����� �� �����

	if (lpName && (
			(lstrcmpiW(lpName, ENV_CONEMUANSI_VAR_W) == 0)
			|| (lstrcmpiW(lpName, ENV_CONEMUHWND_VAR_W) == 0)
			|| (lstrcmpiW(lpName, ENV_CONEMUDIR_VAR_W) == 0)
			|| (lstrcmpiW(lpName, ENV_CONEMUBASEDIR_VAR_W) == 0)
		))
	{
		CheckVariables();
	}

	BOOL lbRc = F(GetEnvironmentVariableW)(lpName, lpBuffer, nSize);
	return lbRc;
}

#if 0
LPCH WINAPI OnGetEnvironmentStringsA()
{
	typedef LPCH (WINAPI* OnGetEnvironmentStringsA_t)();
	ORIGINALFAST(GetEnvironmentStringsA);
	BOOL bMainThread = FALSE; // ����� �� �����

	CheckVariables();

	LPCH lpRc = F(GetEnvironmentStringsA)();
	return lpRc;
}
#endif

LPWCH WINAPI OnGetEnvironmentStringsW()
{
	typedef LPWCH (WINAPI* OnGetEnvironmentStringsW_t)();
	ORIGINALFAST(GetEnvironmentStringsW);
	//BOOL bMainThread = FALSE; // ����� �� �����

	CheckVariables();

	LPWCH lpRc = F(GetEnvironmentStringsW)();
	return lpRc;
}

BOOL IsVisibleRectLocked(COORD& crLocked)
{
	CESERVER_CONSOLE_MAPPING_HDR SrvMap;
	if (LoadSrvMapping(ghConWnd, SrvMap))
	{
		if (SrvMap.bLockVisibleArea && (SrvMap.crLockedVisible.X > 0) && (SrvMap.crLockedVisible.Y > 0))
		{
			crLocked = SrvMap.crLockedVisible;
			return TRUE;
		}
	}
	return FALSE;
}

HANDLE WINAPI OnCreateConsoleScreenBuffer(DWORD dwDesiredAccess, DWORD dwShareMode, const SECURITY_ATTRIBUTES *lpSecurityAttributes, DWORD dwFlags, LPVOID lpScreenBufferData)
{
	typedef HANDLE(WINAPI* OnCreateConsoleScreenBuffer_t)(DWORD dwDesiredAccess, DWORD dwShareMode, const SECURITY_ATTRIBUTES *lpSecurityAttributes, DWORD dwFlags, LPVOID lpScreenBufferData);
	ORIGINALFAST(CreateConsoleScreenBuffer);

	#ifdef SHOWCREATEBUFFERINFO
	wchar_t szDebugInfo[255];
	msprintf(szDebugInfo, countof(szDebugInfo), L"CreateConsoleScreenBuffer(0x%X,0x%X,0x%X,0x%X,0x%X)",
		dwDesiredAccess, dwShareMode, (DWORD)(DWORD_PTR)lpSecurityAttributes, dwFlags, (DWORD)(DWORD_PTR)lpScreenBufferData);
		
	#endif

	if ((dwShareMode & (FILE_SHARE_READ|FILE_SHARE_WRITE)) != (FILE_SHARE_READ|FILE_SHARE_WRITE))
		dwShareMode |= (FILE_SHARE_READ|FILE_SHARE_WRITE);

	if ((dwDesiredAccess & (GENERIC_READ|GENERIC_WRITE)) != (GENERIC_READ|GENERIC_WRITE))
		dwDesiredAccess |= (GENERIC_READ|GENERIC_WRITE);

	if (!ghStdOutHandle)
		ghStdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

	HANDLE h = INVALID_HANDLE_VALUE;
	if (F(CreateConsoleScreenBuffer))
		h = F(CreateConsoleScreenBuffer)(dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwFlags, lpScreenBufferData);

#ifdef SHOWCREATEBUFFERINFO
	msprintf(szDebugInfo+lstrlen(szDebugInfo), 32, L"=0x%X", (DWORD)(DWORD_PTR)h);
	GuiMessageBox(ghConEmuWnd, szDebugInfo, L"ConEmuHk", MB_SETFOREGROUND|MB_SYSTEMMODAL);
#endif

	return h;
}

BOOL WINAPI OnSetConsoleActiveScreenBuffer(HANDLE hConsoleOutput)
{
	typedef BOOL (WINAPI* OnSetConsoleActiveScreenBuffer_t)(HANDLE hConsoleOutput);
	ORIGINALFAST(SetConsoleActiveScreenBuffer);

	if (!ghStdOutHandle)
		ghStdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

	#ifdef _DEBUG
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE hIn  = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);
	#endif

	BOOL lbRc = FALSE;
	if (F(SetConsoleActiveScreenBuffer))
		lbRc = F(SetConsoleActiveScreenBuffer)(hConsoleOutput);

	if (lbRc && (ghCurrentOutBuffer || (hConsoleOutput != ghStdOutHandle)))
	{
		#ifdef SHOWCREATEBUFFERINFO
		CONSOLE_SCREEN_BUFFER_INFO lsbi = {};
		BOOL lbTest = GetConsoleScreenBufferInfo(hConsoleOutput, &lsbi);
		DWORD nErrCode = GetLastError();
		_ASSERTE(lbTest && lsbi.dwSize.Y && "GetConsoleScreenBufferInfo(hConsoleOutput) failed");
		#endif

		ghCurrentOutBuffer = hConsoleOutput;
		RequestLocalServerParm Parm = {(DWORD)sizeof(Parm), slsf_SetOutHandle, &ghCurrentOutBuffer};
		RequestLocalServer(&Parm);
	}
	
	return lbRc;
}

BOOL WINAPI OnSetConsoleWindowInfo(HANDLE hConsoleOutput, BOOL bAbsolute, const SMALL_RECT *lpConsoleWindow)
{
	typedef BOOL (WINAPI* OnSetConsoleWindowInfo_t)(HANDLE hConsoleOutput, BOOL bAbsolute, const SMALL_RECT *lpConsoleWindow);
	ORIGINALFAST(SetConsoleWindowInfo);
	BOOL lbRc = FALSE;
	SMALL_RECT tmp;
	COORD crLocked;

	#ifdef _DEBUG
	CONSOLE_SCREEN_BUFFER_INFO sbi = {};
	BOOL lbSbi = GetConsoleScreenBufferInfo(hConsoleOutput, &sbi);
	UNREFERENCED_PARAMETER(lbSbi);

	wchar_t szDbgSize[512];
	msprintf(szDbgSize, countof(szDbgSize), L"SetConsoleWindowInfo(%08X, %s, {%ix%i}-{%ix%i}), Current={%ix%i}, Wnd={%ix%i}-{%ix%i}\n",
		(DWORD)hConsoleOutput, bAbsolute ? L"ABS" : L"REL",
		lpConsoleWindow->Left, lpConsoleWindow->Top, lpConsoleWindow->Right, lpConsoleWindow->Bottom,
		sbi.dwSize.X, sbi.dwSize.Y,
		sbi.srWindow.Left, sbi.srWindow.Top, sbi.srWindow.Right, sbi.srWindow.Bottom);
	DebugStringConSize(szDbgSize);
	#endif

	BOOL lbLocked = IsVisibleRectLocked(crLocked);

	if (lpConsoleWindow && lbLocked)
	{
		tmp = *lpConsoleWindow;
		if (((tmp.Right - tmp.Left + 1) != crLocked.X) || ((tmp.Bottom - tmp.Top + 1) != crLocked.Y))
		{
			
			// ������ _�������_ �������. ���������� ����������� ��������� ������ ��� "�������".
			// ������ ����� ������ ������ ������������ �������� ���� ConEmu
			if ((tmp.Right - tmp.Left + 1) != crLocked.X)
			{
				if (!bAbsolute)
					tmp.Left = 0;
				tmp.Right = tmp.Left + crLocked.X - 1;
				lpConsoleWindow = &tmp;
			}
			if ((tmp.Bottom - tmp.Top + 1) != crLocked.Y)
			{
				if (!bAbsolute)
					tmp.Top = 0;
				tmp.Bottom = tmp.Top + crLocked.Y - 1;
				lpConsoleWindow = &tmp;
			}

			#ifdef _DEBUG
			msprintf(szDbgSize, countof(szDbgSize), L"---> IsVisibleRectLocked, lpConsoleWindow was patched {%ix%i}-{%ix%i}\n",
				tmp.Left, tmp.Top, tmp.Right, tmp.Bottom);
			DebugStringConSize(szDbgSize);
			#endif
		}
	}

	if (F(SetConsoleWindowInfo))
		lbRc = F(SetConsoleWindowInfo)(hConsoleOutput, bAbsolute, lpConsoleWindow);

	return lbRc;
}

BOOL WINAPI OnSetConsoleScreenBufferSize(HANDLE hConsoleOutput, COORD dwSize)
{
	typedef BOOL (WINAPI* OnSetConsoleScreenBufferSize_t)(HANDLE hConsoleOutput, COORD dwSize);
	ORIGINALFAST(SetConsoleScreenBufferSize);
	BOOL lbRc = FALSE, lbRetry = FALSE;
	COORD crLocked;
	DWORD dwErr = -1;

	CONSOLE_SCREEN_BUFFER_INFO sbi = {};
	BOOL lbSbi = GetConsoleScreenBufferInfo(hConsoleOutput, &sbi);
	UNREFERENCED_PARAMETER(lbSbi);

	#ifdef _DEBUG
	wchar_t szDbgSize[512];
	msprintf(szDbgSize, countof(szDbgSize), L"SetConsoleScreenBufferSize(%08X, {%ix%i}), Current={%ix%i}, Wnd={%ix%i}\n",
		(DWORD)hConsoleOutput, dwSize.X, dwSize.Y, sbi.dwSize.X, sbi.dwSize.Y,
		sbi.srWindow.Right-sbi.srWindow.Left+1, sbi.srWindow.Bottom-sbi.srWindow.Top+1);
	DebugStringConSize(szDbgSize);
	#endif

	BOOL lbLocked = IsVisibleRectLocked(crLocked);

	if (lbLocked && ((crLocked.X > dwSize.X) || (crLocked.Y > dwSize.Y)))
	{
		// ������ _�������_ �������. ���������� ����������� ��������� ������ ��� "�������".
		// ������ ����� ������ ������ ������������ �������� ���� ConEmu
		if (crLocked.X > dwSize.X)
			dwSize.X = crLocked.X;
		if (crLocked.Y > dwSize.Y)
			dwSize.Y = crLocked.Y;

		#ifdef _DEBUG
		msprintf(szDbgSize, countof(szDbgSize), L"---> IsVisibleRectLocked, dwSize was patched {%ix%i}\n",
			dwSize.X, dwSize.Y);
		DebugStringConSize(szDbgSize);
		#endif
	}

	if (F(SetConsoleScreenBufferSize))
	{
		lbRc = F(SetConsoleScreenBufferSize)(hConsoleOutput, dwSize);
		dwErr = GetLastError();

		// The specified dimensions also cannot be less than the minimum size allowed
		// by the system. This minimum depends on the current font size for the console
		// (selected by the user) and the SM_CXMIN and SM_CYMIN values returned by the
		// GetSystemMetrics function.
		if (!lbRc && (dwErr == ERROR_INVALID_PARAMETER))
		{
			// ���������� ���������/��������� ����� � �������
			lbRetry = apiFixFontSizeForBufferSize(hConsoleOutput, dwSize);

			if (lbRetry)
			{
				lbRc = F(SetConsoleScreenBufferSize)(hConsoleOutput, dwSize);
				if (lbRc)
					dwErr = 0;
			}

			// ������ ��� �������� Far ��������� ERROR_INVALID_PARAMETER
			// ��� ����, szDbgSize:
			// SetConsoleScreenBufferSize(0000000C, {80x1000}), Current={94x33}, Wnd={94x33}
			// �.�. ��������� �� �����������
			_ASSERTE(lbRc && (dwErr != ERROR_INVALID_PARAMETER));
			if (!lbRc)
				SetLastError(dwErr); // ������� "������"
		}
	}

	return lbRc;
}

COORD WINAPI OnGetLargestConsoleWindowSize(HANDLE hConsoleOutput)
{
	typedef COORD (WINAPI* OnGetLargestConsoleWindowSize_t)(HANDLE hConsoleOutput);
	ORIGINALFAST(GetLargestConsoleWindowSize);
	COORD cr = {80,25}, crLocked = {0,0};
	
	if (ghConEmuWndDC && IsVisibleRectLocked(crLocked))
	{
		cr = crLocked;
	}
	else
	{
		if (F(GetLargestConsoleWindowSize))
		{
			cr = F(GetLargestConsoleWindowSize)(hConsoleOutput);
		}

		// Wine BUG
		//if (!cr.X || !cr.Y)
		if ((cr.X == 80 && cr.Y == 24) && IsWine())
		{
			cr.X = 255;
			cr.Y = 255;
		}
	}
	
	return cr;
}

BOOL WINAPI OnSetConsoleCursorPosition(HANDLE hConsoleOutput, COORD dwCursorPosition)
{
	typedef BOOL (WINAPI* OnSetConsoleCursorPosition_t)(HANDLE,COORD);
	ORIGINALFAST(SetConsoleCursorPosition);

	BOOL lbRc;
	
	if (gbIsVimAnsi)
	{
		#ifdef DUMP_VIM_SETCURSORPOS
		wchar_t szDbg[80]; msprintf(szDbg, countof(szDbg), L"ViM trying to set cursor pos: {%i,%i}\n", (UINT)dwCursorPosition.X, (UINT)dwCursorPosition.Y);
		OutputDebugString(szDbg);
		#endif
		lbRc = FALSE;
	}
	else
	{
		lbRc = F(SetConsoleCursorPosition)(hConsoleOutput, dwCursorPosition);
	}

	if (ghConsoleCursorChanged)
		SetEvent(ghConsoleCursorChanged);

	return lbRc;
}

BOOL WINAPI OnSetConsoleCursorInfo(HANDLE hConsoleOutput, const CONSOLE_CURSOR_INFO *lpConsoleCursorInfo)
{
	typedef BOOL (WINAPI* OnSetConsoleCursorInfo_t)(HANDLE,const CONSOLE_CURSOR_INFO *);
	ORIGINALFAST(SetConsoleCursorInfo);

	BOOL lbRc = F(SetConsoleCursorInfo)(hConsoleOutput, lpConsoleCursorInfo);

	if (ghConsoleCursorChanged)
		SetEvent(ghConsoleCursorChanged);

	return lbRc;
}

void PatchDialogParentWnd(HWND& hWndParent)
{
	WARNING("��������� ��� ��������� �������� (A/W). �� ����, ���� ������ hWndParent, � �� ������� ��������");
	// Re: conemu + emenu/{Run Sandboxed} ������������ ���

	if (ghConEmuWndDC)
	{
		if (!hWndParent || !user->isWindowVisible(hWndParent))
			hWndParent = ghConEmuWnd;
	}
}

// ����� ��� "��������" ����������� ���� ��� ������ Shell ��������
INT_PTR WINAPI OnDialogBoxParamW(HINSTANCE hInstance, LPCWSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
	typedef INT_PTR (WINAPI* OnDialogBoxParamW_t)(HINSTANCE hInstance, LPCWSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
	ORIGINALFASTEX(DialogBoxParamW,NULL);
	INT_PTR iRc = 0;

	if (ghConEmuWndDC)
	{
		// ���������� "�������" ������ ���������� ����, ����� Shell-������ ������ �������� ��� ConEmu
		GuiSetForeground(hWndParent ? hWndParent : ghConWnd);
		// bugreport from Andrey Budko: conemu + emenu/{Run Sandboxed} ������������ ���
		PatchDialogParentWnd(hWndParent);
	}

	if (F(DialogBoxParamW))
		iRc = F(DialogBoxParamW)(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);

	return iRc;
}

HDC WINAPI OnGetDC(HWND hWnd)
{
	typedef HDC (WINAPI* OnGetDC_t)(HWND hWnd);
	ORIGINALFASTEX(GetDC,NULL);
	HDC hDC = NULL;
	
	if (F(GetDC))
		hDC = F(GetDC)(hWnd);
	
	if (hDC && ghConEmuWndDC && hWnd == ghConEmuWndDC)
		ghTempHDC = hDC;
	
	return hDC;
}

HDC WINAPI OnGetDCEx(HWND hWnd, HRGN hrgnClip, DWORD flags)
{
	typedef HDC (WINAPI* OnGetDCEx_t)(HWND hWnd, HRGN hrgnClip, DWORD flags);
	ORIGINALFASTEX(GetDCEx,NULL);
	HDC hDC = NULL;
	
	if (F(GetDCEx))
		hDC = F(GetDCEx)(hWnd, hrgnClip, flags);
	
	if (hDC && ghConEmuWndDC && hWnd == ghConEmuWndDC)
		ghTempHDC = hDC;
	
	return hDC;
}

int WINAPI OnReleaseDC(HWND hWnd, HDC hDC)
{
	typedef int (WINAPI* OnReleaseDC_t)(HWND hWnd, HDC hDC);
	ORIGINALFASTEX(ReleaseDC,NULL);
	int iRc = 0;
	
	if (F(ReleaseDC))
		iRc = F(ReleaseDC)(hWnd, hDC);
	
	if (ghTempHDC == hDC)
		ghTempHDC = NULL;
	
	return iRc;
}

int WINAPI OnStretchDIBits(HDC hdc, int XDest, int YDest, int nDestWidth, int nDestHeight, int XSrc, int YSrc, int nSrcWidth, int nSrcHeight, const VOID *lpBits, const BITMAPINFO *lpBitsInfo, UINT iUsage, DWORD dwRop)
{
	typedef int (WINAPI* OnStretchDIBits_t)(HDC hdc, int XDest, int YDest, int nDestWidth, int nDestHeight, int XSrc, int YSrc, int nSrcWidth, int nSrcHeight, const VOID *lpBits, const BITMAPINFO *lpBitsInfo, UINT iUsage, DWORD dwRop);
	ORIGINALFASTEX(StretchDIBits,NULL);
	int iRc = 0;
	
	if (F(StretchDIBits))
		iRc = F(StretchDIBits)(hdc, XDest, YDest, nDestWidth, nDestHeight, XSrc, YSrc, nSrcWidth, nSrcHeight, lpBits, lpBitsInfo, iUsage, dwRop);

	// ���� ������ _�����_ �� ������� ConEmu
	if (iRc != (int)GDI_ERROR && hdc && hdc == ghTempHDC)
	{
		// ��������� GUI, ��� � ���� ����� �� ������� ���-�� ���-�� ��������� :)
		CESERVER_REQ* pIn = ExecuteNewCmd(CECMD_LOCKDC, sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_LOCKDC));
		if (pIn)
		{
			pIn->LockDc.hDcWnd = ghConEmuWndDC; // �� ������ ������
			pIn->LockDc.bLock = TRUE;
			pIn->LockDc.Rect.left = XDest;
			pIn->LockDc.Rect.top = YDest;
			pIn->LockDc.Rect.right = XDest+nDestWidth-1;
			pIn->LockDc.Rect.bottom = YDest+nDestHeight-1;

			CESERVER_REQ* pOut = ExecuteGuiCmd(ghConWnd, pIn, ghConWnd);

			if (pOut)
				ExecuteFreeResult(pOut);
			ExecuteFreeResult(pIn);
		}
	}
		
	return iRc;
}

BOOL WINAPI OnBitBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, HDC hdcSrc, int nXSrc, int nYSrc, DWORD dwRop)
{
	typedef int (WINAPI* OnBitBlt_t)(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, HDC hdcSrc, int nXSrc, int nYSrc, DWORD dwRop);
	ORIGINALFASTEX(BitBlt,NULL);
	BOOL bRc = FALSE;
	
	if (F(BitBlt))
		bRc = F(BitBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);

	// ���� ������ _�����_ �� ������� ConEmu
	if (bRc && hdcDest && hdcDest == ghTempHDC)
	{
		// ��������� GUI, ��� � ���� ����� �� ������� ���-�� ���-�� ��������� :)
		CESERVER_REQ* pIn = ExecuteNewCmd(CECMD_LOCKDC, sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_LOCKDC));
		if (pIn)
		{
			pIn->LockDc.hDcWnd = ghConEmuWndDC; // �� ������ ������
			pIn->LockDc.bLock = TRUE;
			pIn->LockDc.Rect.left = nXDest;
			pIn->LockDc.Rect.top = nYDest;
			pIn->LockDc.Rect.right = nXDest+nWidth-1;
			pIn->LockDc.Rect.bottom = nYDest+nHeight-1;

			CESERVER_REQ* pOut = ExecuteGuiCmd(ghConWnd, pIn, ghConWnd);

			if (pOut)
				ExecuteFreeResult(pOut);
			ExecuteFreeResult(pIn);
		}
	}
		
	return bRc;
}

// ��������� ������ �� GdipDrawImageRectRectI
static RECT StretchBltBatch = {};

BOOL WINAPI OnStretchBlt(HDC hdcDest, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest, HDC hdcSrc, int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc, DWORD dwRop)
{
	typedef int (WINAPI* OnStretchBlt_t)(HDC hdcDest, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest, HDC hdcSrc, int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc, DWORD dwRop);
	ORIGINALFASTEX(StretchBlt,NULL);
	BOOL bRc = FALSE;

	//#ifdef _DEBUG
	//HWND h = WindowFromDC(hdcDest);
	//#endif

	// ���� ������ _�����_ �� ������� ConEmu
	if (/*bRc &&*/ hdcDest && hdcDest == ghTempHDC)
	{
		if (
			(!StretchBltBatch.bottom && !StretchBltBatch.top)
			|| (nYOriginDest <= StretchBltBatch.top)
			|| (nXOriginDest != StretchBltBatch.left)
			|| (StretchBltBatch.right != (nXOriginDest+nWidthDest-1))
			|| (StretchBltBatch.bottom != (nYOriginDest-1))
			)
		{
			// ����� �����
			StretchBltBatch.left = nXOriginDest;
			StretchBltBatch.top = nYOriginDest;
			StretchBltBatch.right = nXOriginDest+nWidthDest-1;
			StretchBltBatch.bottom = nYOriginDest+nHeightDest-1;
		}
		else
		{
			StretchBltBatch.bottom = nYOriginDest+nHeightDest-1;
		}

		// ��������� GUI, ��� � ���� ����� �� ������� ���-�� ���-�� ��������� :)
		CESERVER_REQ* pIn = ExecuteNewCmd(CECMD_LOCKDC, sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_LOCKDC));
		if (pIn)
		{
			pIn->LockDc.hDcWnd = ghConEmuWndDC; // �� ������ ������
			pIn->LockDc.bLock = TRUE;
			pIn->LockDc.Rect = StretchBltBatch;

			CESERVER_REQ* pOut = ExecuteGuiCmd(ghConWnd, pIn, ghConWnd);

			if (pOut)
				ExecuteFreeResult(pOut);
			ExecuteFreeResult(pIn);
		}
	}

	if (F(StretchBlt))
		bRc = F(StretchBlt)(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, hdcSrc, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, dwRop);

	return bRc;
}

BOOL GetConsoleScreenBufferInfoCached(HANDLE hConsoleOutput, PCONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo, BOOL bForced /*= FALSE*/)
{
	BOOL lbRc = FALSE;

	static DWORD s_LastCheckTick = 0;
	static CONSOLE_SCREEN_BUFFER_INFO s_csbi = {};
	static HANDLE s_hConOut = NULL;
	//DWORD nTickDelta = 0;
	//const DWORD TickDeltaMax = 250;

	if (hConsoleOutput == NULL)
	{
		// �����
		s_hConOut = NULL;
		GetConsoleModeCached(NULL, NULL);
		return FALSE;
	}
	
	if (!lpConsoleScreenBufferInfo)
	{
		_ASSERTEX(lpConsoleScreenBufferInfo!=NULL);
		return FALSE;
	}

#if 0
	if (s_hConOut && (s_hConOut == hConsoleOutput))
	{
		nTickDelta = GetTickCount() - s_LastCheckTick;
		if (nTickDelta <= TickDeltaMax)
		{
			if (bForced)
			{
				#ifdef _DEBUG
				lbRc = FALSE;
				#endif
			}
			else
			{
				*lpConsoleScreenBufferInfo = s_csbi;
				lbRc = TRUE;
			}
		}
	}
#endif

	if (!lbRc)
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi = {};
		lbRc = GetConsoleScreenBufferInfo(hConsoleOutput, &csbi);
		*lpConsoleScreenBufferInfo = csbi;
		if (lbRc)
		{
			s_csbi = csbi;
			s_LastCheckTick = GetTickCount();
			s_hConOut = hConsoleOutput;
		}
	}

	return lbRc;
}

BOOL GetConsoleModeCached(HANDLE hConsoleHandle, LPDWORD lpMode, BOOL bForced /*= FALSE*/)
{
	BOOL lbRc = FALSE;

	static DWORD s_LastCheckTick = 0;
	static DWORD s_dwMode = 0;
	static HANDLE s_hConHandle = NULL;
	DWORD nTickDelta = 0;
	const DWORD TickDeltaMax = 250;

	if (hConsoleHandle == NULL)
	{
		// �����
		s_hConHandle = NULL;
		return FALSE;
	}

	if (!lpMode)
	{
		_ASSERTEX(lpMode!=NULL);
		return FALSE;
	}
	
	if (s_hConHandle && (s_hConHandle == hConsoleHandle))
	{
		nTickDelta = GetTickCount() - s_LastCheckTick;
		if (nTickDelta <= TickDeltaMax)
		{
			if (bForced)
			{
				#ifdef _DEBUG
				lbRc = FALSE;
				#endif
			}
			else
			{
				*lpMode = s_dwMode;
				lbRc = TRUE;
			}
		}
	}

	if (!lbRc)
	{
		DWORD dwMode = 0;
		lbRc = GetConsoleMode(hConsoleHandle, &dwMode);
		*lpMode = dwMode;
		if (lbRc)
		{
			s_dwMode = dwMode;
			s_LastCheckTick = GetTickCount();
			s_hConHandle = hConsoleHandle;
		}
	}

	return lbRc;
}
