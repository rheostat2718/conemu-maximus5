
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


#pragma once


#undef SHOW_SHUTDOWNSRV_STEPS
#ifdef _DEBUG
	#define SHOW_SHUTDOWNSRV_STEPS
#endif


#ifdef _DEBUG
// ����������������� ��� ������ � ������� ���������� ������ Comspec
#define PRINT_COMSPEC(f,a) //wprintf(f,a)
//#define DEBUGSTR(s) OutputDebugString(s)
#elif defined(__GNUC__)
//  �����������������, ����� ����� ����� ������� �������� (conemuc.exe) �������� MessageBox, ����� ����������� ����������
//  #define SHOW_STARTED_MSGBOX
#define PRINT_COMSPEC(f,a) //wprintf(f,a)
//#define DEBUGSTR(s)
#define CRTPRINTF
#else
#define PRINT_COMSPEC(f,a)
#define DEBUGSTR(s)
#endif

#ifdef _DEBUG
#define xf_check() { xf_validate(); xf_dump_chk(); }
#else
#define xf_check()
#endif

#define DEBUGLOG(s) //DEBUGSTR(s)
#define DEBUGLOGSIZE(s) DEBUGSTR(s)
#define DEBUGLOGLANG(s) //DEBUGSTR(s) //; Sleep(2000)

#ifdef _DEBUG
//CRITICAL_ SECTION gcsHeap;
//#define MCHKHEAP { Enter CriticalSection(&gcsHeap); int MDEBUG_CHK=_CrtCheckMemory(); _ASSERTE(MDEBUG_CHK); LeaveCriticalSection(&gcsHeap); }
//#define MCHKHEAP HeapValidate(ghHeap, 0, NULL);
//#define MCHKHEAP { int MDEBUG_CHK=_CrtCheckMemory(); _ASSERTE(MDEBUG_CHK); }
//#define HEAP_LOGGING
#else
//#define MCHKHEAP
#endif




#define CSECTION_NON_RAISE

#include <Windows.h>
#include <WinCon.h>
#ifdef _DEBUG
#include <stdio.h>
#endif
#include <Shlwapi.h>
#include <Tlhelp32.h>

void ShutdownSrvStep(LPCWSTR asInfo, int nParm1 = 0, int nParm2 = 0, int nParm3 = 0, int nParm4 = 0);

void SetTerminateEvent();

BOOL WINAPI HandlerRoutine(DWORD dwCtrlType);

/*  Global  */
extern DWORD   gnSelfPID;
//HANDLE  ghConIn = NULL, ghConOut = NULL;
extern HWND    ghConWnd;
extern HWND    ghConEmuWnd; // Root! window
extern HWND    ghConEmuWndDC; // ConEmu DC window
extern DWORD   gnMainServerPID; // PID ������� (���������������� �� ������, ��� �������� Dll)
extern DWORD   gnAltServerPID; // PID ������� (���������������� �� ������, ��� �������� Dll)
extern BOOL    gbLogProcess; // (pInfo->nLoggingType == glt_Processes)
extern BOOL    gbWasBufferHeight;
extern BOOL    gbNonGuiMode;
extern HANDLE  ghExitQueryEvent; // ������������ ����� � ������� �� �������� ���������
extern int nExitQueryPlace, nExitPlaceStep, nExitPlaceThread;
extern HANDLE  ghQuitEvent;      // ����� �� � �������� �������� (���� ��� ����� ������ "Press to close console")
extern bool    gbQuit;           // ����� �� � �������� �������� (���� ��� ����� ������ "Press to close console")
extern int     gnConfirmExitParm;
extern BOOL    gbAlwaysConfirmExit, gbInShutdown, gbAutoDisableConfirmExit;
extern int     gbRootWasFoundInCon;
extern BOOL    gbAttachMode; // ������ ������� �� �� conemu.exe (� �� �������, �� CmdAutoAttach, ��� -new_console)
extern BOOL    gbAlienMode;  // ������ �� �������� ���������� ������� (�������� ��������� ����� ����������� ����)
extern BOOL    gbForceHideConWnd;
extern DWORD   gdwMainThreadId;
//int       gnBufferHeight = 0;
extern wchar_t* gpszRunCmd;
extern BOOL    gbRunInBackgroundTab;
extern DWORD   gnImageSubsystem;
//HANDLE  ghCtrlCEvent = NULL, ghCtrlBreakEvent = NULL;
extern HANDLE ghHeap; //HeapCreate(HEAP_GENERATE_EXCEPTIONS, nMinHeapSize, 0);
#ifdef _DEBUG
extern size_t gnHeapUsed, gnHeapMax;
extern HANDLE ghFarInExecuteEvent;
#endif

//#include <vector>
#include "../common/common.hpp"
#include "../common/ConEmuCheck.h"
#include "../common/WinObjects.h"
#include "../common/ConsoleAnnotation.h"

#ifdef _DEBUG
extern wchar_t gszDbgModLabel[6];
#endif

#define START_MAX_PROCESSES 1000
#define CHECK_PROCESSES_TIMEOUT 500
#define CHECK_ANTIVIRUS_TIMEOUT 6*1000
#define CHECK_ROOTSTART_TIMEOUT 10*1000
#ifdef _DEBUG
	#define CHECK_ROOTOK_TIMEOUT (IsDebuggerPresent() ? ((DWORD)-1) : (10*1000)) // ��� ���������� - ����� ������
#else
	#define CHECK_ROOTOK_TIMEOUT 10*1000
#endif
#define MAX_FORCEREFRESH_INTERVAL 500
#define MAX_SYNCSETSIZE_WAIT 1000
#define GUI_PIPE_TIMEOUT 300
#define MAX_CONREAD_SIZE 30000 // � ������
#define RELOAD_INFO_TIMEOUT 500
#define EXTCONCOMMIT_TIMEOUT 500
#define REQSIZE_TIMEOUT 5000
#define GUIREADY_TIMEOUT 10000
#define UPDATECONHANDLE_TIMEOUT 1000
#define GUIATTACH_TIMEOUT 10000
#define INPUT_QUEUE_TIMEOUT 100
#define ATTACH2GUI_TIMEOUT 10000
#define GUIATTACHEVENT_TIMEOUT 250

//#define IMAGE_SUBSYSTEM_DOS_EXECUTABLE  255

#define MAX_INPUT_QUEUE_EMPTY_WAIT 1000


//#ifndef _DEBUG
// �������� �����
#define FORCE_REDRAW_FIX
#define RELATIVE_TRANSMIT_DISABLE
//#else
//// ���������� �����
////#define FORCE_REDRAW_FIX
//#endif

#if !defined(CONSOLE_APPLICATION_16BIT)
#define CONSOLE_APPLICATION_16BIT       0x0001
#endif


//#if defined(__GNUC__)
//	//#include "assert.h"
//	#ifndef _ASSERTE
//		#define _ASSERTE(x)
//	#endif
//	#ifndef _ASSERT
//		#define _ASSERT(x)
//	#endif
//#else
//	#include <crtdbg.h>
//#endif

#ifndef EVENT_CONSOLE_CARET
#define EVENT_CONSOLE_CARET             0x4001
#define EVENT_CONSOLE_UPDATE_REGION     0x4002
#define EVENT_CONSOLE_UPDATE_SIMPLE     0x4003
#define EVENT_CONSOLE_UPDATE_SCROLL     0x4004
#define EVENT_CONSOLE_LAYOUT            0x4005
#define EVENT_CONSOLE_START_APPLICATION 0x4006
#define EVENT_CONSOLE_END_APPLICATION   0x4007
#endif

#define SafeCloseHandle(h) { if ((h)!=NULL) { HANDLE hh = (h); (h) = NULL; if (hh!=INVALID_HANDLE_VALUE) CloseHandle(hh); } }

//#undef USE_WINEVENT_SRV

BOOL createProcess(BOOL abSkipWowChange, LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);

//DWORD WINAPI InstanceThread(LPVOID);
//DWORD WINAPI ServerThread(LPVOID lpvParam);
//DWORD WINAPI InputThread(LPVOID lpvParam);
//DWORD WINAPI InputPipeThread(LPVOID lpvParam);
//DWORD WINAPI GetDataThread(LPVOID lpvParam);
BOOL ProcessSrvCommand(CESERVER_REQ& in, CESERVER_REQ** out);
//#ifdef USE_WINEVENT_SRV
//DWORD WINAPI WinEventThread(LPVOID lpvParam);
//void WINAPI WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD anEvent, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
//#endif
void CheckCursorPos();
//void SendConsoleChanges(CESERVER_REQ* pOut);
//CESERVER_REQ* CreateConsoleInfo(CESERVER_CHAR* pRgnOnly, BOOL bCharAttrBuff);
//BOOL ReloadConsoleInfo(CESERVER_CHAR* pChangedRgn=NULL); // ���������� TRUE � ������ ���������
//BOOL ReloadFullConsoleInfo(/*CESERVER_CHAR* pCharOnly=NULL*/); // � ��� ����� ������������ ����������
BOOL ReloadFullConsoleInfo(BOOL abForceSend);
DWORD WINAPI RefreshThread(LPVOID lpvParam); // ����, �������������� ���������� �������
//BOOL ReadConsoleData(CESERVER_CHAR* pCheck = NULL); //((LPRECT)1) ��� �������� LPRECT
void SetConsoleFontSizeTo(HWND inConWnd, int inSizeY, int inSizeX, const wchar_t *asFontName);
int ServerInit(int anWorkMode/*0-Server,1-AltServer,2-Reserved*/); // ������� ����������� ������� � ����
void ServerDone(int aiRc, bool abReportShutdown = false);
int ComspecInit();
void ComspecDone(int aiRc);
BOOL SetConsoleSize(USHORT BufferHeight, COORD crNewSize, SMALL_RECT rNewRect, LPCSTR asLabel = NULL);
void CreateLogSizeFile(int nLevel);
void LogSize(COORD* pcrSize, LPCSTR pszLabel);
void LogString(LPCSTR asText);
void PrintExecuteError(LPCWSTR asCmd, DWORD dwErr, LPCWSTR asSpecialInfo=NULL);
BOOL MyReadConsoleOutput(HANDLE hOut, CHAR_INFO *pData, COORD& bufSize, SMALL_RECT& rgn);
BOOL MyWriteConsoleOutput(HANDLE hOut, CHAR_INFO *pData, COORD& bufSize, COORD& crBufPos, SMALL_RECT& rgn);


#if defined(__GNUC__)
extern "C" {
	BOOL WINAPI HandlerRoutine(DWORD dwCtrlType);
};
#endif

int GetProcessCount(DWORD *rpdwPID, UINT nMaxCount);
SHORT CorrectTopVisible(int nY);
BOOL CorrectVisibleRect(CONSOLE_SCREEN_BUFFER_INFO* pSbi);
WARNING("������ GetConsoleScreenBufferInfo ����� ������������ MyGetConsoleScreenBufferInfo!");
BOOL MyGetConsoleScreenBufferInfo(HANDLE ahConOut, PCONSOLE_SCREEN_BUFFER_INFO apsc);
//void EnlargeRegion(CESERVER_CHAR_HDR& rgn, const COORD crNew);
void CmdOutputStore(bool abCreateOnly = false);
void CmdOutputRestore();
//LPVOID Alloc(size_t nCount, size_t nSize);
//void Free(LPVOID ptr);
void CheckConEmuHwnd();
HWND FindConEmuByPID();
typedef BOOL (__stdcall *FGetConsoleKeyboardLayoutName)(wchar_t*);
extern FGetConsoleKeyboardLayoutName pfnGetConsoleKeyboardLayoutName;
void CheckKeyboardLayout();
int CALLBACK FontEnumProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, DWORD FontType, LPARAM lParam);
typedef DWORD (WINAPI* FGetConsoleProcessList)(LPDWORD lpdwProcessList, DWORD dwProcessCount);
extern FGetConsoleProcessList pfnGetConsoleProcessList;
//BOOL HookWinEvents(int abEnabled);
BOOL CheckProcessCount(BOOL abForce = FALSE);
BOOL ProcessAdd(DWORD nPID, MSectionLock *pCS = NULL);
//BOOL IsExecutable(LPCWSTR aszFilePathName);
//BOOL IsNeedCmd(LPCWSTR asCmdLine, BOOL *rbNeedCutStartEndQuot, wchar_t (&szExe)[MAX_PATH+1]);
//BOOL FileExists(LPCWSTR asFile);
//extern bool GetImageSubsystem(const wchar_t *FileName,DWORD& ImageSubsystem,DWORD& ImageBits/*16/32/64*/,DWORD& FileAttrs);
void SendStarted();
CESERVER_REQ* SendStopped(CONSOLE_SCREEN_BUFFER_INFO* psbi = NULL);
BOOL SendConsoleEvent(INPUT_RECORD* pr, UINT nCount);
typedef BOOL (WINAPI *FDebugActiveProcessStop)(DWORD dwProcessId);
extern FDebugActiveProcessStop pfnDebugActiveProcessStop;
typedef BOOL (WINAPI *FDebugSetProcessKillOnExit)(BOOL KillOnExit);
extern FDebugSetProcessKillOnExit pfnDebugSetProcessKillOnExit;
void ProcessDebugEvent();
BOOL IsUserAdmin();
void _wprintf(LPCWSTR asBuffer);
#ifdef CRTPRINTF
void _printf(LPCSTR asBuffer);
void _printf(LPCSTR asFormat, DWORD dwErr);
void _printf(LPCSTR asFormat, DWORD dwErr, LPCWSTR asAddLine);
void _printf(LPCSTR asFormat, DWORD dw1, DWORD dw2, LPCWSTR asAddLine=NULL);
#else
#define _printf printf
//#define _wprintf(s) wprintf(L"%s",s)
#endif
HWND Attach2Gui(DWORD nTimeout);

int InjectRemote(DWORD nRemotePID);
int InfiltrateDll(HANDLE hProcess, LPCWSTR dll);

int ParseCommandLine(LPCWSTR asCmdLine /*, wchar_t** psNewCmd, BOOL* pbRunInBackgroundTab*/); // ������ ���������� ��������� ������
void Help();
void DosBoxHelp();
void ExitWaitForKey(WORD* pvkKeys, LPCWSTR asConfirm, BOOL abNewLine, BOOL abDontShowConsole);
bool IsMainServerPID(DWORD nPID);

bool AltServerWasStarted(DWORD nPID, HANDLE hAltServer, bool ForceThaw = false);

int CreateMapHeader();
void CloseMapHeader();
void UpdateConsoleMapHeader();
BOOL ReloadGuiSettings(ConEmuGuiMapping* apFromCmd);

//void EmergencyShow();

//int CreateColorerHeader();

void DisableAutoConfirmExit(BOOL abFromFarPlugin=FALSE);

int MySetWindowRgn(CESERVER_REQ_SETWINDOWRGN* pRgn);

//int InjectHooks(PROCESS_INFORMATION pi, BOOL abForceGui);

#ifdef _DEBUG
	#undef WAIT_INPUT_READY
#else
	#define WAIT_INPUT_READY
#endif
////#define USE_INPUT_SEMAPHORE
//#undef USE_INPUT_SEMAPHORE
//#define INSEMTIMEOUT_WRITE 250
//#define INSEMTIMEOUT_READ  500
//#ifdef USE_INPUT_SEMAPHORE
//extern HANDLE ghConInSemaphore;
//#endif
//void InitializeConsoleInputSemaphore();
//void ReleaseConsoleInputSemaphore();

/* Console Handles */
//extern MConHandle ghConIn;
extern MConHandle ghConOut;



typedef enum tag_RunMode
{
	RM_UNDEFINED = 0,
	RM_SERVER,
	RM_COMSPEC,
	RM_SETHOOK64,
	RM_ALTSERVER,
	RM_APPLICATION,
} RunMode;

extern RunMode gnRunMode;

extern BOOL gbNoCreateProcess;
extern BOOL gbDebugProcess;
extern int  gnCmdUnicodeMode;
extern BOOL gbUseDosBox;
extern BOOL gbRootIsCmdExe;
extern BOOL gbAttachFromFar;
extern BOOL gbConsoleModeFlags;
extern DWORD gnConsoleModeFlags;

#ifdef WIN64
#ifndef __GNUC__
#pragma message("ComEmuC compiled in X64 mode")
#endif
#define NTVDMACTIVE FALSE
#else
#ifndef __GNUC__
#pragma message("ComEmuC compiled in X86 mode")
#endif
#define NTVDMACTIVE (gpSrv->bNtvdmActive)
#endif

#include "../common/PipeServer.h"
#include "../common/MMap.h"

struct AltServerInfo
{
	DWORD  nPID; // ��� ����������
	HANDLE hPrev;
	DWORD  nPrevPID;
};

struct SrvInfo
{
	HANDLE hRootProcess, hRootThread; DWORD dwRootProcess, dwRootThread; DWORD dwRootStartTime;
	HANDLE hMainServer; DWORD dwMainServerPID;

	HANDLE hAltServer, hCloseAltServer, hAltServerChanged;
	DWORD dwAltServerPID; DWORD dwPrevAltServerPID;
	MMap<DWORD,AltServerInfo> AltServers;

	HANDLE hFreezeRefreshThread;
	HWND   hRootProcessGui; // ���� �������� � Gui-������ (Notepad, Putty, ...), ((HWND)-1) ���� ��������� ���� ��� �� �������, �� exe-���� ��� ����
	BOOL   bDebuggerActive, bDebuggerRequestDump; HANDLE hDebugThread, hDebugReady; DWORD dwDebugThreadId;
	DWORD  dwGuiPID; // GUI PID (�� �������� ����������� ����� ConEmu)
	HWND   hGuiWnd; // ���������� ����� �������� "/GHWND=%08X", ����� ���� �� ������
	DWORD  nActiveFarPID; // PID ���������� ��������� Far
	BOOL   bWasDetached; // ������������ � TRUE ��� ��������� CECMD_DETACHCON
	BOOL   bWasReattached; // ���� TRUE - �� ��� ��������� ����� ����� ����������� ReloadFullConsoleInfo(true)
	//
	PipeServer<CESERVER_REQ> CmdServer;
	HANDLE hRefreshThread;  DWORD dwRefreshThread;  BOOL bRefreshTermination;
	PipeServer<MSG64> InputServer;
	PipeServer<CESERVER_REQ> DataServer;
	//
	OSVERSIONINFO osv;
	BOOL bReopenHandleAllowed;
	UINT nMaxFPS;
	//
	MSection *csProc;
	MSection *csAltSrv;
	// ������ ��������� ��� �����, ����� ����������, ����� ������� ��� �� �����.
	// ��������, ��������� FAR, �� �������� Update, FAR �����������...
	UINT nProcessCount, nMaxProcesses;
	DWORD *pnProcesses, *pnProcessesGet, *pnProcessesCopy, nProcessStartTick;
	DWORD dwProcessLastCheckTick;
#ifndef WIN64
	BOOL bNtvdmActive; DWORD nNtvdmPID;
#endif
	//BOOL bTelnetActive;
	//
	wchar_t szPipename[MAX_PATH], szInputname[MAX_PATH], szGuiPipeName[MAX_PATH], szQueryname[MAX_PATH];
	wchar_t szGetDataPipe[MAX_PATH], szDataReadyEvent[64];
	HANDLE /*hInputPipe,*/ hQueryPipe/*, hGetDataPipe*/;
	//
	MFileMapping<CESERVER_CONSAVE_MAPHDR> *pStoredOutputHdr;
	MFileMapping<CESERVER_CONSAVE_MAP> *pStoredOutputItem;
	//
	MFileMapping<CESERVER_CONSOLE_MAPPING_HDR> *pConsoleMap;
	ConEmuGuiMapping guiSettings;
	CESERVER_REQ_CONINFO_FULL *pConsole;
	CHAR_INFO *pConsoleDataCopy; // Local (Alloc)
	// Input
	HANDLE hInputThread, hInputEvent; DWORD dwInputThread; BOOL bInputTermination;
	int nInputQueue, nMaxInputQueue;
	INPUT_RECORD* pInputQueue;
	INPUT_RECORD* pInputQueueEnd;
	INPUT_RECORD* pInputQueueRead;
	INPUT_RECORD* pInputQueueWrite;
	// TrueColorer buffer
	//HANDLE hColorerMapping;
	MFileMapping<const AnnotationHeader>* pColorerMapping; // ��������� Colorer TrueMod
	AnnotationHeader ColorerHdr; // ��� ��������� ��������
	//
	HANDLE hConEmuGuiAttached;
	HWINEVENTHOOK /*hWinHook,*/ hWinHookStartEnd; //BOOL bWinHookAllow; int nWinHookMode;
	DWORD dwCiRc; CONSOLE_CURSOR_INFO ci; // GetConsoleCursorInfo
	DWORD dwConsoleCP, dwConsoleOutputCP, dwConsoleMode;
	DWORD dwSbiRc; CONSOLE_SCREEN_BUFFER_INFO sbi; // MyGetConsoleScreenBufferInfo
	DWORD dwDisplayMode;
	//USHORT nUsedHeight; // ������, ������������ � GUI - ������ ���� ���������� gcrBufferSize.Y
	SHORT nTopVisibleLine; // ��������� � GUI ����� ���� �������������. ���� -1 - ��� ����������, ���������� ������� ��������
	SHORT nVisibleHeight;  // �� ����, ������ ���� ����� (gcrBufferSize.Y). ��� ��������������� ���������� ����� psChars & pnAttrs
	DWORD nMainTimerElapse;
	HANDLE hRefreshEvent; // ServerMode, ���������� �������, � ���� ���� ��������� - �������� � GUI
	HANDLE hRefreshDoneEvent; // ServerMode, ������������ ����� hRefreshEvent
	HANDLE hDataReadyEvent; // ����, ��� � ������� ���� ��������� (GUI ������ ���������� ������)
	HANDLE hExtConsoleCommit; // Event ��� ������������� (������������ �� Commit);
	DWORD  nExtConsolePID;
	BOOL bForceConsoleRead; // ����� ���� ������ ������� RefreshThread ����� ��� ��� �������� ���������� ����������
	// ����� ������� ������� ����� RefreshThread
	int nRequestChangeSize;
	BOOL bRequestChangeSizeResult;
	USHORT nReqSizeBufferHeight;
	COORD crReqSizeNewSize;
	SMALL_RECT rReqSizeNewRect;
	LPCSTR sReqSizeLabel;
	HANDLE hReqSizeChanged;
	//
	HANDLE hAllowInputEvent; BOOL bInSyncResize;
	//
	DWORD nLastPacketID; // �� ������ ��� �������� � GUI

	// Keyboard layout name
	wchar_t szKeybLayout[KL_NAMELENGTH+1];

	// Optional console font (may be specified in registry)
	wchar_t szConsoleFont[LF_FACESIZE];
	//wchar_t szConsoleFontFile[MAX_PATH]; -- �� ��������
	SHORT nConFontWidth, nConFontHeight;

	// ����� ���� ��������� ���������������� ����������
	DWORD dwLastUserTick;

	// ���� ����� ������������� ���� RefreshThread
	//HANDLE hLockRefreshBegin, hLockRefreshReady;

	// Console Aliases
	wchar_t* pszAliases; DWORD nAliasesSize;

	// ComSpec mode
	BOOL  bK;
	BOOL  bNewConsole;
	wchar_t szComSpecName[32];
	wchar_t szSelfName[32];
	wchar_t *pszPreAliases;
	DWORD nPreAliasSize;
};

extern SrvInfo *gpSrv;
extern OSVERSIONINFO gOSVer;
extern WORD gnOsVer;
extern bool gbIsWine;
extern bool gbIsDBCS;

#define USER_IDLE_TIMEOUT ((DWORD)1000)
#define CHECK_IDLE_TIMEOUT 250 /* 1000 / 4 */
#define USER_ACTIVITY ((gnBufferHeight == 0) || ((GetTickCount() - gpSrv->dwLastUserTick) <= USER_IDLE_TIMEOUT))


//#pragma pack(push, 1)
//extern CESERVER_CONSAVE* gpStoredOutput;
//#pragma pack(pop)
//extern MSection* gpcsStoredOutput;

//typedef struct tag_CmdInfo
//{
//	DWORD dwFarPID;
//	//DWORD dwSrvPID;
//	BOOL  bK;
//	//BOOL  bNonGuiMode; // ���� ������� �� � �������, ����������� � GUI. ����� ���� ��-�� ����, ��� �������� ��� COMSPEC
//	CONSOLE_SCREEN_BUFFER_INFO sbi;
//	BOOL  bNewConsole;
//	//DWORD nExitCode;
//	wchar_t szComSpecName[32];
//	wchar_t szSelfName[32];
//	wchar_t *pszPreAliases;
//	DWORD nPreAliasSize;
//	// �� ���������� ComSpec �� ��������� �����
//	//BOOL  bWasBufferHeight;
//} CmdInfo;
//
//extern CmdInfo* gpSrv;

extern COORD gcrVisibleSize;
extern BOOL  gbParmVisibleSize, gbParmBufSize;
extern SHORT gnBufferHeight, gnBufferWidth;
extern wchar_t* gpszPrevConTitle;

extern HANDLE ghLogSize;
extern wchar_t* wpszLogSizeFile;


extern BOOL gbInRecreateRoot;

//enum CmdOnCreateType
//{
//	eShellExecute = 1,
//	eCreateProcess,
//	eInjectingHooks,
//	eHooksLoaded,
//};

//CESERVER_REQ* NewCmdOnCreateA(
//				enum CmdOnCreateType aCmd, LPCSTR asAction, DWORD anFlags, 
//				LPCSTR asFile, LPCSTR asParam, int nImageBits, int nImageSubsystem,
//				HANDLE hStdIn, HANDLE hStdOut, HANDLE hStdErr);
//CESERVER_REQ* NewCmdOnCreateW(
//				enum CmdOnCreateType aCmd, LPCWSTR asAction, DWORD anFlags, 
//				LPCWSTR asFile, LPCWSTR asParam, int nImageBits, int nImageSubsystem,
//				HANDLE hStdIn, HANDLE hStdOut, HANDLE hStdErr,
//				wchar_t (&szBaseDir)[MAX_PATH+2]);

//#define CES_NTVDM 0x10 -- common.hpp
//DWORD dwActiveFlags = 0;


//#define CERR_GETCOMMANDLINE 100
//#define CERR_CARGUMENT 101
//#define CERR_CMDEXENOTFOUND 102
//#define CERR_NOTENOUGHMEM1 103
//#define CERR_CREATESERVERTHREAD 104
//#define CERR_CREATEPROCESS 105
//#define CERR_WINEVENTTHREAD 106
//#define CERR_CONINFAILED 107
//#define CERR_GETCONSOLEWINDOW 108
//#define CERR_EXITEVENT 109
//#define CERR_GLOBALUPDATE 110
//#define CERR_WINHOOKNOTCREATED 111
//#define CERR_CREATEINPUTTHREAD 112
//#define CERR_CONOUTFAILED 113
//#define CERR_PROCESSTIMEOUT 114
//#define CERR_REFRESHEVENT 115
//#define CERR_CREATEREFRESHTHREAD 116
//#define CERR_HELPREQUESTED 118
//#define CERR_ATTACHFAILED 119
//#define CERR_RUNNEWCONSOLE 121
//#define CERR_CANTSTARTDEBUGGER 122
//#define CERR_CREATEMAPPINGERR 123
//#define CERR_MAPVIEWFILEERR 124
//#define CERR_COLORERMAPPINGERR 125
//#define CERR_EMPTY_COMSPEC_CMDLINE 126
//#define CERR_CONEMUHK_NOTFOUND 127
//#define CERR_CONSOLEMAIN_NOTFOUND 128
//#define CERR_HOOKS_WAS_SET 129
//#define CERR_HOOKS_FAILED 130
//#define CERR_DLLMAIN_SKIPPED 131
//#define CERR_ATTACH_NO_CONWND 132
//#define CERR_GUIMACRO_SUCCEEDED 133
//#define CERR_GUIMACRO_FAILED 134
#include "ExitCodes.h"

