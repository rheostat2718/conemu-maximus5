
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


#ifdef _DEBUG
//  �����������������, ����� ����� ����� ������� �������� (conemuc.exe) �������� MessageBox, ����� ����������� ����������
//  #define SHOW_STARTED_MSGBOX
//  #define SHOW_DEBUG_STARTED_MSGBOX
//  #define SHOW_COMSPEC_STARTED_MSGBOX
//  #define SHOW_SERVER_STARTED_MSGBOX
//  #define SHOW_STARTED_ASSERT
//  #define SHOW_STARTED_PRINT
//  #define SHOW_INJECT_MSGBOX
//	#define SHOW_ATTACH_MSGBOX
//  #define SHOW_ROOT_STARTED
#elif defined(__GNUC__)
//  �����������������, ����� ����� ����� ������� �������� (conemuc.exe) �������� MessageBox, ����� ����������� ����������
//  #define SHOW_STARTED_MSGBOX
#else
//
#endif

#define SHOWDEBUGSTR
#define DEBUGSTRCMD(x) DEBUGSTR(x)
#define DEBUGSTRFIN(x) DEBUGSTR(x)

//#define SHOW_INJECT_MSGBOX

#include "ConEmuC.h"
#include "../ConEmu/version.h"
#include "../common/execute.h"
#include "../ConEmuHk/Injects.h"
#include "../common/RConStartArgs.h"
#include "TokenHelper.h"


#ifdef __GNUC__
	#include "../common/DbgHlpGcc.h"
#else
	#include <Dbghelp.h>
#endif

extern HMODULE ghOurModule;

WARNING("����������� ����� ������� ������� apiSetForegroundWindow �� GUI ����, ���� � ������ �������");
WARNING("����������� �������� ��� � ��� ������������� ��������");


#ifdef _DEBUG
wchar_t gszDbgModLabel[6] = {0};
#endif

WARNING("!!!! ���� ����� ��� ��������� ������� ���������� ������� ���");
// � ��������� ��� � RefreshThread. ���� �� �� 0 - � ������ ������ (100��?)
// �� ������������� ���������� ������� � �������� ��� � 0.


WARNING("�������� ���-�� ����� ����������� ������������� ������ ����������� �������, � �� ������ �� �������");

WARNING("����� ������ ����� ������������ �������� ������� GUI ���� (���� ��� ����). ���� ����� ���� ������� �� far, � CMD.exe");

WARNING("���� GUI ����, ��� �� ���������� �� �������� - �������� ���������� ���� � �������� ���������� ����� ������");

WARNING("� ��������� ������� �� ����������� �� EVENT_CONSOLE_UPDATE_SIMPLE �� EVENT_CONSOLE_UPDATE_REGION");
// ������. ��������� cmd.exe. �������� �����-�� ����� � ��������� ������ � �������� 'Esc'
// ��� Esc ������� ������� ������ �� ���������, � ����� � ������� ���������!



FGetConsoleKeyboardLayoutName pfnGetConsoleKeyboardLayoutName = NULL;
FGetConsoleProcessList pfnGetConsoleProcessList = NULL;
FDebugActiveProcessStop pfnDebugActiveProcessStop = NULL;
FDebugSetProcessKillOnExit pfnDebugSetProcessKillOnExit = NULL;


/* Console Handles */
//MConHandle ghConIn ( L"CONIN$" );
MConHandle ghConOut(L"CONOUT$");

// ����� �������� ���������� ���������� ���������, ����� ���� ����� ������� � ���������� ����
// The system also displays this dialog box if the process does not respond within a certain time-out period 
// (5 seconds for CTRL_CLOSE_EVENT, and 20 seconds for CTRL_LOGOFF_EVENT or CTRL_SHUTDOWN_EVENT).
// ������� ����� �������� - �� ������ 4 ���
#define CLOSE_CONSOLE_TIMEOUT 4000

/*  Global  */
HMODULE ghOurModule = NULL; // ConEmuCD.dll
DWORD   gnSelfPID = 0;
BOOL    gbTerminateOnExit = FALSE;
//HANDLE  ghConIn = NULL, ghConOut = NULL;
HWND    ghConWnd = NULL;
HWND    ghConEmuWnd = NULL; // Root! window
HWND    ghConEmuWndDC = NULL; // ConEmu DC window
DWORD   gnServerPID = 0;
BOOL    gbLogProcess = FALSE;
BOOL    gbWasBufferHeight = FALSE;
BOOL    gbNonGuiMode = FALSE;
DWORD   gnExitCode = 0;
HANDLE  ghExitQueryEvent = NULL; int nExitQueryPlace = 0, nExitPlaceStep = 0, nExitPlaceThread = 0;
HANDLE  ghQuitEvent = NULL;
bool    gbQuit = false;
BOOL	gbInShutdown = FALSE;
BOOL    gbCtrlBreakStopWaitingShown = FALSE;
BOOL	gbTerminateOnCtrlBreak = FALSE;
int     gnConfirmExitParm = 0; // 1 - CONFIRM, 2 - NOCONFIRM
BOOL    gbAlwaysConfirmExit = FALSE;
BOOL	gbAutoDisableConfirmExit = FALSE; // ���� �������� ������� ���������� ���������� (10 ���) - ����� ������� gbAlwaysConfirmExit
BOOL    gbRootAliveLess10sec = FALSE; // �������� ������� ���������� ����� CHECK_ROOTOK_TIMEOUT
int     gbRootWasFoundInCon = 0;
BOOL    gbAttachMode = FALSE; // ������ ������� �� �� conemu.exe (� �� �������, �� CmdAutoAttach, ��� -new_console)
BOOL    gbAlienMode = FALSE;  // ������ �� �������� ���������� ������� (�������� ��������� ����� ����������� ����)
BOOL    gbForceHideConWnd = FALSE;
DWORD   gdwMainThreadId = 0;
wchar_t* gpszRunCmd = NULL;
BOOL    gbRunInBackgroundTab = FALSE;
BOOL    gbRunViaCmdExe = FALSE;
DWORD   gnImageSubsystem = 0, gnImageBits = 32;
//HANDLE  ghCtrlCEvent = NULL, ghCtrlBreakEvent = NULL;
//HANDLE ghHeap = NULL; //HeapCreate(HEAP_GENERATE_EXCEPTIONS, nMinHeapSize, 0);
#ifdef _DEBUG
size_t gnHeapUsed = 0, gnHeapMax = 0;
HANDLE ghFarInExecuteEvent;
#endif

RunMode gnRunMode = RM_UNDEFINED;

BOOL  gbNoCreateProcess = FALSE;
BOOL  gbDebugProcess = FALSE;
int   gnCmdUnicodeMode = 0;
BOOL  gbUseDosBox = FALSE; HANDLE ghDosBoxProcess = NULL; DWORD gnDosBoxPID = 0;
BOOL  gbRootIsCmdExe = TRUE;
BOOL  gbAttachFromFar = FALSE;
BOOL  gbSkipWowChange = FALSE;
BOOL  gbConsoleModeFlags = TRUE;
DWORD gnConsoleModeFlags = 0; //(ENABLE_QUICK_EDIT_MODE|ENABLE_INSERT_MODE);
OSVERSIONINFO gOSVer;


SrvInfo* gpSrv = NULL;

#pragma pack(push, 1)
CESERVER_CONSAVE* gpStoredOutput = NULL;
#pragma pack(pop)
MSection* gpcsStoredOutput = NULL;

//CmdInfo* gpSrv = NULL;

COORD gcrVisibleSize = {80,25}; // gcrBufferSize ������������ � gcrVisibleSize
BOOL  gbParmVisibleSize = FALSE;
BOOL  gbParmBufSize = FALSE;
SHORT gnBufferHeight = 0;
SHORT gnBufferWidth = 0; // ������������ � MyGetConsoleScreenBufferInfo
wchar_t* gpszPrevConTitle = NULL;

HANDLE ghLogSize = NULL;
wchar_t* wpszLogSizeFile = NULL;


BOOL gbInRecreateRoot = FALSE;



#if defined(__GNUC__)
extern "C" {
	BOOL WINAPI DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved);
};
#endif

//extern UINT_PTR gfnLoadLibrary;
UINT gnMsgActivateCon = 0;
UINT gnMsgSwitchCon = 0;
UINT gnMsgHookedKey = 0;
UINT gnMsgConsoleHookedKey = 0;

BOOL WINAPI DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch(ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			ghOurModule = (HMODULE)hModule;

			DWORD nImageBits = WIN3264TEST(32,64), nImageSubsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
			GetImageSubsystem(nImageSubsystem,nImageBits);
			if (nImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI)
				ghConWnd = NULL;
			else
				ghConWnd = GetConEmuHWND(2);

			gnSelfPID = GetCurrentProcessId();
			ghWorkingModule = (u64)hModule;

			#ifdef _DEBUG
			HANDLE hProcHeap = GetProcessHeap();
			gAllowAssertThread = am_Pipe;
			#endif

			#ifdef SHOW_STARTED_MSGBOX
			if (!IsDebuggerPresent())
			{
				char szMsg[128]; msprintf(szMsg, countof(szMsg), "ConEmuCD*.dll loaded, PID=%u, TID=%u", GetCurrentProcessId(), GetCurrentThreadId());
				MessageBoxA(NULL, szMsg, "ConEmu server" WIN3264TEST(""," x64"), 0);
			}
			#endif
			#ifdef _DEBUG
			DWORD dwConMode = -1;
			GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &dwConMode);
			#endif

			//_ASSERTE(ghHeap == NULL);
			//ghHeap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 200000, 0);
			HeapInitialize();
			
			// ��������� ��������� � �������� ����� ���� ���-�� �����������, ����� ������ �����
			// iFindAddress = FindKernelAddress(pi.hProcess, pi.dwProcessId, &fLoadLibrary);
			//gfnLoadLibrary = (UINT_PTR)::GetProcAddress(::GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
			GetLoadLibraryAddress(); // ��������� gfnLoadLibrary
			
			//#ifndef TESTLINK
			gpLocalSecurity = LocalSecurity();
			gnMsgActivateCon = RegisterWindowMessage(CONEMUMSG_ACTIVATECON);
			gnMsgSwitchCon = RegisterWindowMessage(CONEMUMSG_SWITCHCON);
			gnMsgHookedKey = RegisterWindowMessage(CONEMUMSG_HOOKEDKEY);
			gnMsgConsoleHookedKey = RegisterWindowMessage(CONEMUMSG_CONSOLEHOOKEDKEY);
			//#endif
			//wchar_t szSkipEventName[128];
			//_wsprintf(szSkipEventName, SKIPLEN(countof(szSkipEventName)) CEHOOKDISABLEEVENT, GetCurrentProcessId());
			//HANDLE hSkipEvent = OpenEvent(EVENT_ALL_ACCESS , FALSE, szSkipEventName);
			////BOOL lbSkipInjects = FALSE;

			//if (hSkipEvent)
			//{
			//	gbSkipInjects = (WaitForSingleObject(hSkipEvent, 0) == WAIT_OBJECT_0);
			//	CloseHandle(hSkipEvent);
			//}
			//else
			//{
			//	gbSkipInjects = FALSE;
			//}

			// ������� ������� ������� � ���������� �������� HWND GUI, PID �������, � ��...
			if (ghConWnd)
			{
				MFileMapping<CESERVER_CONSOLE_MAPPING_HDR> ConInfo;
				ConInfo.InitName(CECONMAPNAME, (DWORD)ghConWnd); //-V205
				CESERVER_CONSOLE_MAPPING_HDR *pInfo = ConInfo.Open();
				if (pInfo)
				{
					if (pInfo->cbSize >= sizeof(CESERVER_CONSOLE_MAPPING_HDR))
					{
						if (pInfo->hConEmuRoot && IsWindow(pInfo->hConEmuRoot))
						{
							ghConEmuWnd = pInfo->hConEmuRoot;
							if (pInfo->hConEmuWnd && IsWindow(pInfo->hConEmuWnd))
								ghConEmuWndDC = pInfo->hConEmuWnd;
						}
						if (pInfo->nServerPID && pInfo->nServerPID != gnSelfPID)
							gnServerPID = pInfo->nServerPID;

						gbLogProcess = (pInfo->nLoggingType == glt_Processes);
						if (gbLogProcess)
						{
							wchar_t szExeName[MAX_PATH], szDllName[MAX_PATH]; szExeName[0] = szDllName[0] = 0;
							GetModuleFileName(NULL, szExeName, countof(szExeName));
							GetModuleFileName((HMODULE)hModule, szDllName, countof(szDllName));
							int ImageBits = 0, ImageSystem = 0;
							#ifdef _WIN64
							ImageBits = 64;
							#else
							ImageBits = 32;
							#endif
							CESERVER_REQ* pIn = ExecuteNewCmdOnCreate(eSrvLoaded,
								L"", szExeName, szDllName, NULL, NULL, NULL, NULL, 
								ImageBits, ImageSystem, 
								GetStdHandle(STD_INPUT_HANDLE), GetStdHandle(STD_OUTPUT_HANDLE), GetStdHandle(STD_ERROR_HANDLE));
							if (pIn)
							{
								CESERVER_REQ* pOut = ExecuteGuiCmd(ghConWnd, pIn, ghConWnd);
								ExecuteFreeResult(pIn);
								if (pOut) ExecuteFreeResult(pOut);
							}
						}

						ConInfo.CloseMap();
					}
					else
					{
						_ASSERTE(pInfo->cbSize == sizeof(CESERVER_CONSOLE_MAPPING_HDR));
					}
				}
			}

			//if (!gbSkipInjects && ghConWnd)
			//{
			//	InitializeConsoleInputSemaphore();
			//}
			
			
			//#ifdef _WIN64
			//DWORD nImageBits = 64, nImageSubsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
			//#else
			//DWORD nImageBits = 32, nImageSubsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
			//#endif
			//GetImageSubsystem(nImageSubsystem,nImageBits);

			//CShellProc sp;
			//if (sp.LoadGuiMapping())
			//{
			//	wchar_t szExeName[MAX_PATH+1]; //, szBaseDir[MAX_PATH+2];
			//	//BOOL lbDosBoxAllowed = FALSE;
			//	if (!GetModuleFileName(NULL, szExeName, countof(szExeName))) szExeName[0] = 0;
			//	CESERVER_REQ* pIn = sp.NewCmdOnCreate(
			//		gbSkipInjects ? eHooksLoaded : eInjectingHooks,
			//		L"", szExeName, GetCommandLineW(),
			//		NULL, NULL, NULL, NULL, // flags
			//		nImageBits, nImageSubsystem,
			//		GetStdHandle(STD_INPUT_HANDLE), GetStdHandle(STD_OUTPUT_HANDLE), GetStdHandle(STD_ERROR_HANDLE));
			//	if (pIn)
			//	{
			//		//HWND hConWnd = GetConEmuHWND(2);
			//		CESERVER_REQ* pOut = ExecuteGuiCmd(ghConWnd, pIn, ghConWnd);
			//		ExecuteFreeResult(pIn);
			//		if (pOut) ExecuteFreeResult(pOut);
			//	}
			//}

			//if (!gbSkipInjects)
			//{
			//	#ifdef _DEBUG
			//	wchar_t szModule[MAX_PATH+1]; szModule[0] = 0;
			//	#endif

			//	#ifdef SHOW_INJECT_MSGBOX
			//	wchar_t szDbgMsg[1024], szTitle[128];//, szModule[MAX_PATH];
			//	if (!GetModuleFileName(NULL, szModule, countof(szModule)))
			//		wcscpy_c(szModule, L"GetModuleFileName failed");
			//	_wsprintf(szTitle, SKIPLEN(countof(szTitle)) L"ConEmuHk, PID=%u", GetCurrentProcessId());
			//	_wsprintf(szDbgMsg, SKIPLEN(countof(szDbgMsg)) L"SetAllHooks, ConEmuHk, PID=%u\n%s", GetCurrentProcessId(), szModule);
			//	MessageBoxW(NULL, szDbgMsg, szTitle, MB_SYSTEMMODAL);
			//	#endif
			//	gnRunMode = RM_APPLICATION;

			//	#ifdef _DEBUG
			//	//wchar_t szModule[MAX_PATH+1]; szModule[0] = 0;
			//	GetModuleFileName(NULL, szModule, countof(szModule));
			//	const wchar_t* pszName = PointToName(szModule);
			//	_ASSERTE((nImageSubsystem==IMAGE_SUBSYSTEM_WINDOWS_CUI) || (lstrcmpi(pszName, L"DosBox.exe")==0));
			//	//if (!lstrcmpi(pszName, L"far.exe") || !lstrcmpi(pszName, L"mingw32-make.exe"))
			//	//if (!lstrcmpi(pszName, L"as.exe"))
			//	//	MessageBoxW(NULL, L"as.exe loaded!", L"ConEmuHk", MB_SYSTEMMODAL);
			//	//else if (!lstrcmpi(pszName, L"cc1plus.exe"))
			//	//	MessageBoxW(NULL, L"cc1plus.exe loaded!", L"ConEmuHk", MB_SYSTEMMODAL);
			//	//else if (!lstrcmpi(pszName, L"mingw32-make.exe"))
			//	//	MessageBoxW(NULL, L"mingw32-make.exe loaded!", L"ConEmuHk", MB_SYSTEMMODAL);
			//	//if (!lstrcmpi(pszName, L"g++.exe"))
			//	//	MessageBoxW(NULL, L"g++.exe loaded!", L"ConEmuHk", MB_SYSTEMMODAL);
			//	//{
			//	#endif

			//	gbHooksWasSet = StartupHooks(ghOurModule);

			//	#ifdef _DEBUG
			//	//}
			//	#endif

			//	// ���� NULL - ������ ��� "Detached" ���������� �������, �������� "Started" � ������ ������ ���
			//	if (ghConWnd != NULL)
			//	{
			//		SendStarted();
			//		//#ifdef _DEBUG
			//		//// ����� ��� �������� � ������ _chkstk,
			//		//// ������ ��-�� ����, ��� dll-�� ��������� �� �� ��������� �������,
			//		//// � �� ���������� ��������������� ����� ������
			//		// -- � ����� �� �������, ��� ��������� ���������� ���������� ������� ����� ������
			//		// -- ��������� � malloc/free, ��� ����������
			//		//TestShellProcessor();
			//		//#endif
			//	}
			//}
			//else
			//{
			//	gbHooksWasSet = FALSE;
			//}
		}
		break;
		case DLL_PROCESS_DETACH:
		{
			//if (!gbSkipInjects && gbHooksWasSet)
			//{
			//	gbHooksWasSet = FALSE;
			//	ShutdownHooks();
			//}

			if (gnRunMode == RM_APPLICATION)
			{
				SendStopped();
			}

			//#ifndef TESTLINK
			CommonShutdown();

			//ReleaseConsoleInputSemaphore();

			//#endif
			//if (ghHeap)
			//{
			//	HeapDestroy(ghHeap);
			//	ghHeap = NULL;
			//}
			HeapDeinitialize();
		}
		break;
	}

	return TRUE;
}

//#if defined(CRTSTARTUP)
//extern "C" {
//	BOOL WINAPI _DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
//};
//
//BOOL WINAPI _DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
//{
//	DllMain(hDll, dwReason, lpReserved);
//	return TRUE;
//}
//#endif

#ifdef _DEBUG
void OnProcessCreatedDbg(BOOL bRc, DWORD dwErr, LPPROCESS_INFORMATION pProcessInformation, LPSHELLEXECUTEINFO pSEI)
{
	int iDbg = 0;
#ifdef SHOW_ROOT_STARTED
	if (bRc)
	{
		wchar_t szTitle[64], szMsg[128];
		_wsprintf(szTitle, SKIPLEN(countof(szTitle)) L"ConEmuSrv, PID=%u", GetCurrentProcessId());
		_wsprintf(szMsg, SKIPLEN(countof(szMsg)) L"Root process started, PID=%u", pProcessInformation->dwProcessId);
		MessageBox(NULL,szMsg,szTitle,0);
	}
#endif
}
#endif

BOOL createProcess(BOOL abSkipWowChange, LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
	MWow64Disable wow;
	if (!abSkipWowChange)
		wow.Disable();

	SetLastError(0);

	BOOL lbRc = CreateProcess(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	DWORD dwErr = GetLastError();

	#ifdef _DEBUG
	OnProcessCreatedDbg(lbRc, dwErr, lpProcessInformation, NULL);
	#endif

	if (!abSkipWowChange)
	{
		wow.Restore();
		SetLastError(dwErr);
	}

	return lbRc;
}

#if defined(__GNUC__)
extern "C" {
	int __stdcall ConsoleMain2(BOOL abAlternative);
};
#endif

// Main entry point for ConEmuC.exe
int __stdcall ConsoleMain2(BOOL abAlternative)
{
	TODO("����� ��� ������� �������� �������, �������������� �������� 80x25 � ��������� ������� �����");
	
	WARNING("����� �������� AltServer - ��������� � ConEmuCD ��� ������� �� RM_SERVER!!!");
	_ASSERTE(abAlternative==FALSE);

	//#ifdef _DEBUG
	//InitializeCriticalSection(&gcsHeap);
	//#endif

	if (!abAlternative)
	{
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleMode(hOut, ENABLE_PROCESSED_OUTPUT|ENABLE_WRAP_AT_EOL_OUTPUT);
	}


	// �� ������ ������ - �������
	gnRunMode = RM_UNDEFINED;

	if (ghOurModule == NULL)
	{
		wchar_t szTitle[128]; _wsprintf(szTitle, SKIPLEN(countof(szTitle)) L"ConEmuHk, PID=%u", GetCurrentProcessId());
		MessageBox(NULL, L"ConsoleMain2. ghOurModule is NULL\nDllMain was not executed", szTitle, MB_ICONSTOP|MB_SYSTEMMODAL);
		return CERR_DLLMAIN_SKIPPED;
	}

	gpSrv = (SrvInfo*)calloc(sizeof(SrvInfo),1);
	if (gpSrv && ghConEmuWnd)
	{
		GetWindowThreadProcessId(ghConEmuWnd, &gpSrv->dwGuiPID);
	}

	RemoveOldComSpecC();

	//if (ghHeap == NULL)
	//{
	//	#ifdef _DEBUG
	//	_ASSERTE(ghHeap != NULL);
	//	#else
	//	wchar_t szTitle[128]; swprintf_c(szTitle, L"ConEmuHk, PID=%u", GetCurrentProcessId());
	//	MessageBox(NULL, L"ConsoleMain2. ghHeap is NULL", szTitle, MB_ICONSTOP|MB_SYSTEMMODAL);
	//	#endif
	//	return CERR_NOTENOUGHMEM1;
	//}
#ifdef SHOW_STARTED_PRINT
	BOOL lbDbgWrite; DWORD nDbgWrite; HANDLE hDbg; char szDbgString[255], szHandles[128];
	sprintf(szDbgString, "ConEmuC: PID=%u", GetCurrentProcessId());
	MessageBoxA(0, GetCommandLineA(), szDbgString, MB_SYSTEMMODAL);
	hDbg = CreateFile(L"CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_READ,
	                  0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	sprintf(szHandles, "STD_OUTPUT_HANDLE(0x%08X) STD_ERROR_HANDLE(0x%08X) CONOUT$(0x%08X)",
	        (DWORD)GetStdHandle(STD_OUTPUT_HANDLE), (DWORD)GetStdHandle(STD_ERROR_HANDLE), (DWORD)hDbg);
	printf("ConEmuC: Printf: %s\n", szHandles);
	sprintf(szDbgString, "ConEmuC: STD_OUTPUT_HANDLE: %s\n", szHandles);
	lbDbgWrite = WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), szDbgString, lstrlenA(szDbgString), &nDbgWrite, NULL);
	sprintf(szDbgString, "ConEmuC: STD_ERROR_HANDLE:  %s\n", szHandles);
	lbDbgWrite = WriteFile(GetStdHandle(STD_ERROR_HANDLE), szDbgString, lstrlenA(szDbgString), &nDbgWrite, NULL);
	sprintf(szDbgString, "ConEmuC: CONOUT$: %s", szHandles);
	lbDbgWrite = WriteFile(hDbg, szDbgString, lstrlenA(szDbgString), &nDbgWrite, NULL);
	CloseHandle(hDbg);
	//sprintf(szDbgString, "ConEmuC: PID=%u", GetCurrentProcessId());
	//MessageBoxA(0, "Press Ok to continue", szDbgString, MB_SYSTEMMODAL);
#endif
	int iRc = 100;
	PROCESS_INFORMATION pi; memset(&pi, 0, sizeof(pi));
	STARTUPINFOW si = {sizeof(STARTUPINFOW)};
	// ConEmuC ������ ���� ����������� ��������� ��� ��������� ��������
	GetStartupInfo(&si);
	DWORD dwErr = 0, nWait = 0, nWaitExitEvent = -1, nWaitDebugExit = -1, nWaitComspecExit = -1;
	BOOL lbRc = FALSE;
	DWORD mode = 0;
	//BOOL lb = FALSE;
	//ghHeap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 200000, 0);
	memset(&gOSVer, 0, sizeof(gOSVer));
	gOSVer.dwOSVersionInfoSize = sizeof(gOSVer);
	GetVersionEx(&gOSVer);
	gpLocalSecurity = LocalSecurity();
	HMODULE hKernel = GetModuleHandleW(L"kernel32.dll");

	if (hKernel)
	{
		pfnGetConsoleKeyboardLayoutName = (FGetConsoleKeyboardLayoutName)GetProcAddress(hKernel, "GetConsoleKeyboardLayoutNameW");
		pfnGetConsoleProcessList = (FGetConsoleProcessList)GetProcAddress(hKernel, "GetConsoleProcessList");
	}

	// ����� ����������� ����
	ghConWnd = GetConEmuHWND(2);
	// ����� ������������� ����� ���� NULL ��� ������� ��� detached comspec
	//_ASSERTE(ghConWnd!=NULL);
	//if (!ghConWnd)
	//{
	//	dwErr = GetLastError();
	//	_printf("ghConWnd==NULL, ErrCode=0x%08X\n", dwErr);
	//	iRc = CERR_GETCONSOLEWINDOW; goto wrap;
	//}
	// PID
	gnSelfPID = GetCurrentProcessId();
	gdwMainThreadId = GetCurrentThreadId();
#ifdef _DEBUG

	if (ghConWnd)
	{
		// ��� ������� ��������� � ���������� (���� ������������) ������ ����
		wchar_t szEvtName[64]; _wsprintf(szEvtName, SKIPLEN(countof(szEvtName)) L"FARconEXEC:%08X", (DWORD)ghConWnd);
		ghFarInExecuteEvent = CreateEvent(0, TRUE, FALSE, szEvtName);
	}

#endif
#if defined(SHOW_STARTED_MSGBOX) || defined(SHOW_COMSPEC_STARTED_MSGBOX)

	if (!IsDebuggerPresent())
	{
		wchar_t szTitle[100]; _wsprintf(szTitle, SKIPLEN(countof(szTitle)) L"ConEmuC Loaded (PID=%i)", gnSelfPID);
		const wchar_t* pszCmdLine = GetCommandLineW();
		MessageBox(NULL,pszCmdLine,szTitle,0);
	}

#endif
#ifdef SHOW_STARTED_ASSERT

	if (!IsDebuggerPresent())
	{
		_ASSERT(FALSE);
	}

#endif
	PRINT_COMSPEC(L"ConEmuC started: %s\n", GetCommandLineW());
	nExitPlaceStep = 50;
	xf_check();

	if ((iRc = ParseCommandLine(GetCommandLineW()/*, &gpszRunCmd, &gbRunInBackgroundTab*/)) != 0)
		goto wrap;

	//#ifdef _DEBUG
	//CreateLogSizeFile();
	//#endif
	nExitPlaceStep = 100;
	xf_check();
#ifdef SHOW_SERVER_STARTED_MSGBOX

	if ((gnRunMode == RM_SERVER) && !IsDebuggerPresent())
	{
		wchar_t szTitle[100]; _wsprintf(szTitle, SKIPLEN(countof(szTitle)) L"ConEmuC [Server] started (PID=%i)", gnSelfPID);
		const wchar_t* pszCmdLine = GetCommandLineW();
		MessageBox(NULL,pszCmdLine,szTitle,0);
	}

#endif
	/* ***************************** */
	/* *** "�����" ������������� *** */
	/* ***************************** */
	nExitPlaceStep = 150;
	// ������� ������������ ��� ���� �������
	ghExitQueryEvent = CreateEvent(NULL, TRUE/*������������ � ���������� �����, manual*/, FALSE, NULL);

	if (!ghExitQueryEvent)
	{
		dwErr = GetLastError();
		_printf("CreateEvent() failed, ErrCode=0x%08X\n", dwErr);
		iRc = CERR_EXITEVENT; goto wrap;
	}
 
	ResetEvent(ghExitQueryEvent);
	ghQuitEvent = CreateEvent(NULL, TRUE/*������������ � ���������� �����, manual*/, FALSE, NULL);

	if (!ghQuitEvent)
	{
		dwErr = GetLastError();
		_printf("CreateEvent() failed, ErrCode=0x%08X\n", dwErr);
		iRc = CERR_EXITEVENT; goto wrap;
	}

	ResetEvent(ghQuitEvent);
	xf_check();

	// �����������
	//ghConIn  = CreateFile(L"CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_READ,
	//            0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	//if ((HANDLE)ghConIn == INVALID_HANDLE_VALUE) {
	//    dwErr = GetLastError();
	//    _printf("CreateFile(CONIN$) failed, ErrCode=0x%08X\n", dwErr);
	//    iRc = CERR_CONINFAILED; goto wrap;
	//}
	// �����������
	//ghConOut = CreateFile(L"CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_READ,
	//            0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (gnRunMode == RM_SERVER)
	{
		if ((HANDLE)ghConOut == INVALID_HANDLE_VALUE)
		{
			dwErr = GetLastError();
			_printf("CreateFile(CONOUT$) failed, ErrCode=0x%08X\n", dwErr);
			iRc = CERR_CONOUTFAILED; goto wrap;
		}
	}

	nExitPlaceStep = 200;
	//2009-05-30 ��������� ��� ����� ?
	//SetHandleInformation(GetStdHandle(STD_INPUT_HANDLE), HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
	//SetHandleInformation(GetStdHandle(STD_OUTPUT_HANDLE), HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
	//SetHandleInformation(GetStdHandle(STD_ERROR_HANDLE), HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
	mode = 0;
	/*lb = GetConsoleMode(ghConIn, &mode);
	if (!(mode & ENABLE_MOUSE_INPUT)) {
		mode |= ENABLE_MOUSE_INPUT;
		lb = SetConsoleMode(ghConIn, mode);
	}*/

	//110131 - �������� � ConEmuC.exe, � ��� - dll-��
	//// �����������, ����� �� CtrlC �� ��������
	//SetConsoleCtrlHandler((PHANDLER_ROUTINE)HandlerRoutine, true);
	//SetConsoleMode(ghConIn, 0);

	/* ******************************** */
	/* *** "��������" ������������� *** */
	/* ******************************** */
	if (gnRunMode == RM_SERVER)
	{
		if ((iRc = ServerInit(abAlternative)) != 0)
		{
			nExitPlaceStep = 250;
			goto wrap;
		}
	}
	else
	{
		xf_check();

		if ((iRc = ComspecInit()) != 0)
		{
			nExitPlaceStep = 300;
			goto wrap;
		}
	}

	/* ********************************* */
	/* *** ������ ��������� �������� *** */
	/* ********************************* */
#ifdef SHOW_STARTED_PRINT
	sprintf(szDbgString, "ConEmuC: PID=%u", GetCurrentProcessId());
	MessageBoxA(0, "Press Ok to continue", szDbgString, MB_SYSTEMMODAL);
#endif

	// CREATE_NEW_PROCESS_GROUP - ����, ��������� �������� Ctrl-C
	// ����� CreateProcess ����� ������� 0, ����� ��-�� ����������� ����� ���������
	// timeout �������� ��������� �������� ��� �� ������ �� CreateProcess
	if (!gbAttachMode)
		gpSrv->nProcessStartTick = 0;

	if (gbNoCreateProcess)
	{
		lbRc = TRUE; // ������� ��� �������, ������ ��������� � ConEmu (GUI)
		pi.hProcess = gpSrv->hRootProcess;
		pi.dwProcessId = gpSrv->dwRootProcess;
	}
	else
	{
		nExitPlaceStep = 350;
#ifdef _DEBUG

		if (ghFarInExecuteEvent && wcsstr(gpszRunCmd,L"far.exe"))
			ResetEvent(ghFarInExecuteEvent);

#endif
		LPCWSTR pszCurDir = NULL;
		wchar_t szSelf[MAX_PATH*2];
		WARNING("The process handle must have the PROCESS_VM_OPERATION access right!");

		if (gbUseDosBox)
		{
			DosBoxHelp();
		}
		else if (gnRunMode == RM_SERVER && !gbRunViaCmdExe)
		{
			// ���������, ����� �������� ��������� GUI ���������� ��� ������� � ConEmu?
			if (((si.dwFlags & STARTF_USESHOWWINDOW) && (si.wShowWindow == SW_HIDE))
				|| !(si.dwFlags & STARTF_USESHOWWINDOW) || (si.wShowWindow != SW_SHOWNORMAL))
			{
				//_ASSERTEX(si.wShowWindow != SW_HIDE); -- ��, ���� ������� (�������) ��������

				// ����� �����, ������ ���� ���� ����� ���������� ��������
				const wchar_t *psz = gpszRunCmd, *pszStart;
				wchar_t szExe[MAX_PATH+1];
				if (NextArg(&psz, szExe, &pszStart) == 0)
				{
					MWow64Disable wow;
					if (!gbSkipWowChange) wow.Disable();
					
					DWORD RunImageSubsystem = 0, RunImageBits = 0, RunFileAttrs = 0;
					bool bSubSystem = GetImageSubsystem(szExe, RunImageSubsystem, RunImageBits, RunFileAttrs);

					if (!bSubSystem)
					{
						// szExe may be simple "notepad", we must seek for executable...
						wchar_t szFound[MAX_PATH+1];
						LPWSTR pszFile = NULL;
						// We are interesting only on ".exe" files,
						// supposing that other executable extensions can't be GUI applications
						DWORD n = SearchPath(NULL, szExe, L".exe", countof(szFound), szFound, &pszFile);
						if (n && (n < countof(szFound)))
							bSubSystem = GetImageSubsystem(szFound, RunImageSubsystem, RunImageBits, RunFileAttrs);
					}

					if (bSubSystem)
					{
						if (RunImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI)
						{
							si.dwFlags |= STARTF_USESHOWWINDOW;
							si.wShowWindow = SW_SHOWNORMAL;
						}
					}
				}
			}
		}

		// ConEmuC ������ ���� ����������� ��������� ��� ��������� ��������
		WARNING("��� ���������� gcc ��� ����� ��������� �� ����������");
		BOOL lbInheritHandle = (gnRunMode!=RM_SERVER);
		// ���� �� ������ ������� ConEmuC.exe � ���������� ����� g++.exe � (as.exe ��� cc1plus.exe)
		// �� ��� ������, ���� ��������� - �� ����� ���� ������ ����
		// c:/gcc/mingw/bin/../libexec/gcc/mingw32/4.3.2/cc1plus.exe:1: error: stray '\220' in program
		// c:/gcc/mingw/bin/../libexec/gcc/mingw32/4.3.2/cc1plus.exe:1:4: warning: null character(s) ignored
		// c:/gcc/mingw/bin/../libexec/gcc/mingw32/4.3.2/cc1plus.exe:1: error: stray '\3' in program
		// c:/gcc/mingw/bin/../libexec/gcc/mingw32/4.3.2/cc1plus.exe:1:6: warning: null character(s) ignored
		// ��������, �������� � ������������ pipe-��, ��������� ��... ��� � ������ SecurityDescriptor.


		//MWow64Disable wow;
		////#ifndef _DEBUG
		//if (!gbSkipWowChange) wow.Disable();
		////#endif

		LPSECURITY_ATTRIBUTES lpSec = LocalSecurity();
		//#ifdef _DEBUG
		//		lpSec = NULL;
		//#endif
		// �� ����� ��������� ������������, ���� ����� - ������� DuplicateHandle
		lbRc = createProcess(!gbSkipWowChange, NULL, gpszRunCmd, lpSec,lpSec, lbInheritHandle,
		                      NORMAL_PRIORITY_CLASS/*|CREATE_NEW_PROCESS_GROUP*/
		                      |CREATE_SUSPENDED/*((gnRunMode == RM_SERVER) ? CREATE_SUSPENDED : 0)*/,
		                      NULL, pszCurDir, &si, &pi);
		dwErr = GetLastError();

		if (!lbRc && (gnRunMode == RM_SERVER) && dwErr == ERROR_FILE_NOT_FOUND)
		{
			// ���� ��� ����������� ConEmu.exe � �������� ����. �.�. far.exe ��������� �� ���� ����� ����
			if (GetModuleFileNameW(NULL, szSelf, countof(szSelf)))
			{
				wchar_t* pszSlash = wcsrchr(szSelf, L'\\');

				if (pszSlash)
				{
					*pszSlash = 0; // �������� ����� � exe-������
					pszSlash = wcsrchr(szSelf, L'\\');

					if (pszSlash)
					{
						*pszSlash = 0; // �������� ������������ �����
						pszCurDir = szSelf;
						SetCurrentDirectory(pszCurDir);
						// ������� ��� ���, � ������������ ����������
						// �� ����� ��������� ������������, ���� ����� - ������� DuplicateHandle
						lbRc = createProcess(!gbSkipWowChange, NULL, gpszRunCmd, NULL,NULL, FALSE/*TRUE*/,
						                      NORMAL_PRIORITY_CLASS/*|CREATE_NEW_PROCESS_GROUP*/
						                      |CREATE_SUSPENDED/*((gnRunMode == RM_SERVER) ? CREATE_SUSPENDED : 0)*/,
						                      NULL, pszCurDir, &si, &pi);
						dwErr = GetLastError();
					}
				}
			}
		}

		//wow.Restore();

		if (lbRc) // && (gnRunMode == RM_SERVER))
		{
			nExitPlaceStep = 400;
			TODO("�� ������ � �������, �� � � ComSpec, ����� �������� ���������� �������� ����� ������������ �����������");
			//""F:\VCProject\FarPlugin\ConEmu\Bugs\DOS\TURBO.EXE ""
			TODO("��� ���������� DOS ���������� - VirtualAllocEx(hProcess, ������������!");
			TODO("� �������� - ��������, �� � ��������� � Anamorphosis ���������� �������� ������������ far->conemu->anamorph->conemu");
#ifdef SHOW_INJECT_MSGBOX
			wchar_t szDbgMsg[128], szTitle[128];
			swprintf_c(szTitle, SKIPLEN(countof(szTitle)) L"ConEmuC, PID=%u", GetCurrentProcessId());
			swprintf_c(szDbgMsg, SKIPLEN(countof(szDbgMsg)) L"ConEmuC, PID=%u\nInjecting hooks into PID=%u", GetCurrentProcessId(), pi.dwProcessId);
			MessageBoxW(NULL, szDbgMsg, szTitle, MB_SYSTEMMODAL);
#endif
			//BOOL gbLogProcess = FALSE;
			//TODO("�������� �� �������� glt_Process");
			//#ifdef _DEBUG
			//gbLogProcess = TRUE;
			//#endif
			int iHookRc = -1;
			if (((gnImageSubsystem == IMAGE_SUBSYSTEM_DOS_EXECUTABLE) || (gnImageBits == 16))
				&& !gbUseDosBox)
			{
				// ���� ����������� ntvdm.exe - ���-����� ��� ��������� �� ����
				iHookRc = 0;
			}
			else
			{
				iHookRc = InjectHooks(pi, FALSE, gbLogProcess);
			}

			if (iHookRc != 0)
			{
				DWORD nErrCode = GetLastError();
				//_ASSERTE(iHookRc == 0); -- ������ �� �����, ���� MsgBox
				wchar_t szDbgMsg[255], szTitle[128];
				_wsprintf(szTitle, SKIPLEN(countof(szTitle)) L"ConEmuC, PID=%u", GetCurrentProcessId());
				_wsprintf(szDbgMsg, SKIPLEN(countof(szDbgMsg)) L"ConEmuC.M, PID=%u\nInjecting hooks into PID=%u\nFAILED, code=%i:0x%08X", GetCurrentProcessId(), pi.dwProcessId, iHookRc, nErrCode);
				MessageBoxW(NULL, szDbgMsg, szTitle, MB_SYSTEMMODAL);
			}
			
			if (gbUseDosBox)
			{
				// ���� ���������� - �� ����� ������� � ������ ��������� (���� �� � �� ����������)
				ghDosBoxProcess = pi.hProcess; gnDosBoxPID = pi.dwProcessId;
				//ProcessAdd(pi.dwProcessId);
			}

			// ��������� ������� (��� �������� ������� �������, �������� far.exe)
			ResumeThread(pi.hThread);
		}

		if (!lbRc && dwErr == 0x000002E4)
		{
			nExitPlaceStep = 450;
			// ��������� ������ � ������ comspec - ����� ���������� ����� �������
			_ASSERTE(gnRunMode != RM_SERVER);
			PRINT_COMSPEC(L"Vista+: The requested operation requires elevation (ErrCode=0x%08X).\n", dwErr);
			// Vista: The requested operation requires elevation.
			LPCWSTR pszCmd = gpszRunCmd;
			wchar_t szVerb[10], szExec[MAX_PATH+1];

			if (NextArg(&pszCmd, szExec) == 0)
			{
				SHELLEXECUTEINFO sei = {sizeof(SHELLEXECUTEINFO)};
				sei.hwnd = ghConEmuWnd;
				sei.fMask = SEE_MASK_NO_CONSOLE; //SEE_MASK_NOCLOSEPROCESS; -- ������ ����� ���������� ��� - ������� ����������� � ����� �������
				wcscpy_c(szVerb, L"open"); sei.lpVerb = szVerb;
				sei.lpFile = szExec;
				sei.lpParameters = pszCmd;
				sei.lpDirectory = pszCurDir;
				sei.nShow = SW_SHOWNORMAL;
				MWow64Disable wow; wow.Disable();
				lbRc = ShellExecuteEx(&sei);
				dwErr = GetLastError();
				#ifdef _DEBUG
				OnProcessCreatedDbg(lbRc, dwErr, NULL, &sei);
				#endif
				wow.Restore();

				if (lbRc)
				{
					// OK
					pi.hProcess = NULL; pi.dwProcessId = 0;
					pi.hThread = NULL; pi.dwThreadId = 0;
					// �.�. ����������� ����� ������� - ������������� �� �������� ���� ����� �� �����
					DisableAutoConfirmExit();
					iRc = 0; goto wrap;
				}
			}
		}
	}

	if (!lbRc)
	{
		nExitPlaceStep = 900;

		PrintExecuteError(gpszRunCmd, dwErr);

		iRc = CERR_CREATEPROCESS; goto wrap;
	}

	if (gbAttachMode)
	{
		// �� ��������� � ��� ������������� ��������:
		// ����� �� ��� ������� ��� ������ dos-������� � ����� ������� ����� -new_console
		// � ��������� ������ ���������� ������������� �������� ���������� �����������
		// -- DisableAutoConfirmExit(); - ����
	}
	else
	{
		if (!gpSrv->nProcessStartTick) // ��� ��� ���� ������������������ �� cmd_CmdStartStop
			gpSrv->nProcessStartTick = GetTickCount();
	}

	if (pi.dwProcessId)
		AllowSetForegroundWindow(pi.dwProcessId);

#ifdef _DEBUG
	xf_validate(NULL);
#endif

	/* ************************ */
	/* *** �������� ������� *** */
	/* ************************ */

	if (gnRunMode == RM_SERVER)
	{
		DWORD dwWaitGui = -1;

		nExitPlaceStep = 500;
		gpSrv->hRootProcess  = pi.hProcess; pi.hProcess = NULL; // Required for Win2k
		gpSrv->hRootThread   = pi.hThread;  pi.hThread  = NULL;
		gpSrv->dwRootProcess = pi.dwProcessId;
		gpSrv->dwRootThread  = pi.dwThreadId;
		gpSrv->dwRootStartTime = GetTickCount();
		// ������ ����� ������� � ���������� ������ ��� �����
		CheckProcessCount(TRUE);

		#ifdef _DEBUG
		if (gpSrv->nProcessCount && !gpSrv->bDebuggerActive)
		{
			_ASSERTE(gpSrv->pnProcesses[gpSrv->nProcessCount-1]!=0);
		}
		#endif

		//if (pi.hProcess) SafeCloseHandle(pi.hProcess);
		//if (pi.hThread) SafeCloseHandle(pi.hThread);

		if (gpSrv->hConEmuGuiAttached)
		{
			DEBUGTEST(DWORD t1 = timeGetTime());
			
			dwWaitGui = WaitForSingleObject(gpSrv->hConEmuGuiAttached, 1000);

			#ifdef _DEBUG
			DWORD t2 = timeGetTime(), tDur = t2-t1;
			if (tDur > GUIATTACHEVENT_TIMEOUT)
			{
				_ASSERTE(tDur <= GUIATTACHEVENT_TIMEOUT);
			}
			#endif

			if (dwWaitGui == WAIT_OBJECT_0)
			{
				// GUI ���� �����
				_wsprintf(gpSrv->szGuiPipeName, SKIPLEN(countof(gpSrv->szGuiPipeName)) CEGUIPIPENAME, L".", (DWORD)ghConWnd); // ��� gnSelfPID //-V205
			}
		}

		// ����, ���� � ������� �� ��������� ��������� (����� ������)
		TODO("���������, ����� �� ��� ����������, ��� CreateProcess ������, � � ������� �� �� ����������? �����, ���� ������� GUI");
		// "�����������" �������� � ������� �������� ����� ��������� ���������
		nWait = nWaitExitEvent = WaitForSingleObject(ghExitQueryEvent, CHECK_ANTIVIRUS_TIMEOUT);

		if (nWait != WAIT_OBJECT_0)  // ���� �������
		{
			iRc = gpSrv->nProcessCount 
				+ (((gpSrv->nProcessCount==1) && gbUseDosBox && (WaitForSingleObject(ghDosBoxProcess,0)==WAIT_TIMEOUT)) ? 1 : 0);

			// � ��������� � ������� ��� ��� ���
			if (iRc == 1 && !gpSrv->bDebuggerActive)
			{
				if (!gbInShutdown)
				{
					gbTerminateOnCtrlBreak = TRUE;
					gbCtrlBreakStopWaitingShown = TRUE;
					//_printf("Process was not attached to console. Is it GUI?\nCommand to be executed:\n");
					//_wprintf(gpszRunCmd);
					PrintExecuteError(gpszRunCmd, 0, L"Process was not attached to console. Is it GUI?\n");
					_printf("\n\nPress Ctrl+Break to stop waiting\n");

					while (!gbInShutdown && (nWait != WAIT_OBJECT_0))
					{
						nWait = nWaitExitEvent = WaitForSingleObject(ghExitQueryEvent, 250);

						if ((nWait != WAIT_OBJECT_0) && (gpSrv->nProcessCount > 1))
						{
							gbTerminateOnCtrlBreak = FALSE;
							gbCtrlBreakStopWaitingShown = FALSE; // �������, ����� ������� �� �����
							goto wait; // OK, ��������� � �������� ���� �������� ����������
						}
					}
				}

				// ���-�� ��� �������� ����� ������ ���-���� �� ����������, ����� ������� � ������� ��������
				_ASSERTE(FALSE && gbTerminateOnCtrlBreak && gbInShutdown);
				iRc = CERR_PROCESSTIMEOUT; goto wrap;
			}
		}
	}
	else
	{
		// � ������ ComSpec ��� ���������� ���������� ������ ��������� ��������
		//wchar_t szEvtName[128];
		//
		//_wsprintf(szEvtName, SKIPLEN(countof(szEvtName)) CESIGNAL_C, pi.dwProcessId);
		//ghCtrlCEvent = CreateEvent(NULL, FALSE, FALSE, szEvtName);
		//_wsprintf(szEvtName, SKIPLEN(countof(szEvtName)) CESIGNAL_BREAK, pi.dwProcessId);
		//ghCtrlBreakEvent = CreateEvent(NULL, FALSE, FALSE, szEvtName);
	}

	/* *************************** */
	/* *** �������� ���������� *** */
	/* *************************** */
wait:
#ifdef _DEBUG
	xf_validate(NULL);
#endif

	if (gnRunMode == RM_SERVER)
	{
		nExitPlaceStep = 550;
		// �� ������� ���� ���� ������� � ������� ����������. ���� ���� � ������� �� ��������� ������ ����� ���
		nWait = WAIT_TIMEOUT; nWaitExitEvent = -2;

		if (!gpSrv->bDebuggerActive)
		{
#ifdef _DEBUG

			while(nWait == WAIT_TIMEOUT)
			{
				nWait = nWaitExitEvent = WaitForSingleObject(ghExitQueryEvent, 100);
				// ���-�� ��� �������� ����� ������ ���-���� �� ����������, ����� ������� � ������� ��������
				_ASSERTE(!(gbCtrlBreakStopWaitingShown && (nWait != WAIT_TIMEOUT)));
			}

#else
			nWait = nWaitExitEvent = WaitForSingleObject(ghExitQueryEvent, INFINITE);
			_ASSERTE(!(gbCtrlBreakStopWaitingShown && (nWait != WAIT_TIMEOUT)));
#endif
		}
		else
		{
			// ������� ��������� ���������� ������� � ��������� ����, ����� �������� �� ��������������� � �������
			nWaitDebugExit = WaitForSingleObject(gpSrv->hDebugThread, INFINITE);
			nWait = WAIT_OBJECT_0;
			//while (nWait == WAIT_TIMEOUT)
			//{
			//	ProcessDebugEvent();
			//	nWait = WaitForSingleObject(ghExitQueryEvent, 0);
			//}
			//gbAlwaysConfirmExit = TRUE;
		}

		#ifdef _DEBUG
		xf_validate(NULL);
		#endif


		// �������� ExitCode
		GetExitCodeProcess(gpSrv->hRootProcess, &gnExitCode);


		#ifdef _DEBUG
		if (nWait == WAIT_OBJECT_0)
		{
			DEBUGSTRFIN(L"*** FinilizeEvent was set!\n");
		}
		#endif

	}
	else
	{
		nExitPlaceStep = 600;
		//HANDLE hEvents[3];
		//hEvents[0] = pi.hProcess;
		//hEvents[1] = ghCtrlCEvent;
		//hEvents[2] = ghCtrlBreakEvent;
		//WaitForSingleObject(pi.hProcess, INFINITE);
		#ifdef _DEBUG
		xf_validate(NULL);
		#endif

		DWORD dwWait = 0;
		dwWait = nWaitComspecExit = WaitForSingleObject(pi.hProcess, INFINITE);

		#ifdef _DEBUG
		xf_validate(NULL);
		#endif

		// �������� ExitCode
		GetExitCodeProcess(pi.hProcess, &gnExitCode);

		#ifdef _DEBUG
		xf_validate(NULL);
		#endif

		// ����� ������� ������
		if (pi.hProcess)
			SafeCloseHandle(pi.hProcess);

		if (pi.hThread)
			SafeCloseHandle(pi.hThread);

		#ifdef _DEBUG
		xf_validate(NULL);
		#endif
	}

	/* ************************* */
	/* *** ���������� ������ *** */
	/* ************************* */
	iRc = 0;
wrap:
#ifdef _DEBUG
	// �������� ����
	xf_validate(NULL);
	// ����� ��������� ������ ������
	if (gnRunMode == RM_SERVER)
		Sleep(1000);
#endif
	// � ���������, HandlerRoutine ����� ���� ��� �� ������, �������
	// � ����� ��������� ExitWaitForKey ��������� �������� ����� gbInShutdown
	PRINT_COMSPEC(L"Finalizing. gbInShutdown=%i\n", gbInShutdown);
#ifdef SHOW_STARTED_MSGBOX
	MessageBox(GetConEmuHWND(2), L"Finalizing", (gnRunMode == RM_SERVER) ? L"ConEmuC.Server" : L"ConEmuC.ComSpec", 0);
#endif
#ifdef _DEBUG
	xf_validate(NULL);
#endif

	if (iRc == CERR_GUIMACRO_SUCCEEDED)
		iRc = 0;


	if (gnRunMode == RM_SERVER && gpSrv->hRootProcess)
		GetExitCodeProcess(gpSrv->hRootProcess, &gnExitCode);
	else if (pi.hProcess)
		GetExitCodeProcess(pi.hProcess, &gnExitCode);

	if (!gbInShutdown  // ������ ���� ���� �� ����� ������� � ��������� ����, ��� �� ������ /ATTACH (����� � ������� �� ������)
	        && ((iRc!=0 && iRc!=CERR_RUNNEWCONSOLE && iRc!=CERR_EMPTY_COMSPEC_CMDLINE)
	            || gbAlwaysConfirmExit)
	  )
	{
		BOOL lbProcessesLeft = FALSE, lbDontShowConsole = FALSE;

		if (pfnGetConsoleProcessList)
		{
			DWORD nProcesses[10];
			DWORD nProcCount = pfnGetConsoleProcessList(nProcesses, 10);

			if (nProcCount > 1)
				lbProcessesLeft = TRUE;
		}

		LPCWSTR pszMsg = NULL;

		if (lbProcessesLeft)
		{
			pszMsg = L"\n\nPress Enter or Esc to exit...";
			lbDontShowConsole = gnRunMode != RM_SERVER;
		}
		else
		{
			if (gbRootWasFoundInCon == 1)
			{
				if (gbRootAliveLess10sec && (gnConfirmExitParm != 1))  // �������� ������� ���������� ����� CHECK_ROOTOK_TIMEOUT
				{
					static wchar_t szMsg[255];
					_wsprintf(szMsg, SKIPLEN(countof(szMsg))
						L"\n\nConEmuC: Root process was alive less than 10 sec, ExitCode=%u.\nPress Enter or Esc to close console...",
						gnExitCode);
					pszMsg = szMsg;
				}
				else
					pszMsg = L"\n\nPress Enter or Esc to close console...";
			}
		}

		if (!pszMsg)  // ����� - ��������� �� ���������
		{
			pszMsg = L"\n\nPress Enter or Esc to close console, or wait...";

			#ifdef _DEBUG
			static wchar_t szDbgMsg[255];
			_wsprintf(szDbgMsg, SKIPLEN(countof(szDbgMsg))
			          L"\n\ngbInShutdown=%i, iRc=%i, gbAlwaysConfirmExit=%i, nExitQueryPlace=%i"
			          L"%s",
			          (int)gbInShutdown, iRc, (int)gbAlwaysConfirmExit, nExitQueryPlace,
			          pszMsg);
			pszMsg = szDbgMsg;
			#endif
		}

		WORD vkKeys[3]; vkKeys[0] = VK_RETURN; vkKeys[1] = VK_ESCAPE; vkKeys[2] = 0;
		ExitWaitForKey(vkKeys, pszMsg, TRUE, lbDontShowConsole);

		if (iRc == CERR_PROCESSTIMEOUT)
		{
			int nCount = gpSrv->nProcessCount;

			if (nCount > 1 || gpSrv->bDebuggerActive)
			{
				// ������� ���� ����������!
				goto wait;
			}
		}
	}

	// �� ������ ������ - �������� �������
	if (ghExitQueryEvent)
	{
		_ASSERTE(gbTerminateOnCtrlBreak==FALSE);
		if (!nExitQueryPlace) nExitQueryPlace = 11+(nExitPlaceStep+nExitPlaceThread);

		SetEvent(ghExitQueryEvent);
	}

	// ���������� RefreshThread, InputThread, ServerThread
	if (ghQuitEvent) SetEvent(ghQuitEvent);

#ifdef _DEBUG
	xf_validate(NULL);
#endif

	/* ***************************** */
	/* *** "��������" ���������� *** */
	/* ***************************** */

	if (gnRunMode == RM_SERVER)
	{
		ServerDone(iRc, true);
		//MessageBox(0,L"Server done...",L"ConEmuC",0);
		SafeCloseHandle(gpSrv->hDebugReady);
		SafeCloseHandle(gpSrv->hDebugThread);
	}
	else if (gnRunMode == RM_COMSPEC)
	{
		ComspecDone(iRc);
		//MessageBox(0,L"Comspec done...",L"ConEmuC",0);
	}

	/* ************************** */
	/* *** "�����" ���������� *** */
	/* ************************** */

	if (gpszPrevConTitle)
	{
		if (ghConWnd)
			SetConsoleTitleW(gpszPrevConTitle);

		free(gpszPrevConTitle);
	}

	LogSize(NULL, "Shutdown");
	//ghConIn.Close();
	ghConOut.Close();
	SafeCloseHandle(ghLogSize);

	if (wpszLogSizeFile)
	{
		//DeleteFile(wpszLogSizeFile);
		free(wpszLogSizeFile); wpszLogSizeFile = NULL;
	}

#ifdef _DEBUG
	SafeCloseHandle(ghFarInExecuteEvent);
#endif

	if (gpszRunCmd) { delete gpszRunCmd; gpszRunCmd = NULL; }

	CommonShutdown();

	// -> DllMain
	//if (ghHeap)
	//{
	//	HeapDestroy(ghHeap);
	//	ghHeap = NULL;
	//}

	// ���� ����� ComSpec - ������� ��� �������� �� ����������� ��������
	if (iRc == 0 && gnRunMode == RM_COMSPEC)
		iRc = gnExitCode;

#ifdef SHOW_STARTED_MSGBOX
	MessageBox(GetConEmuHWND(2), L"Exiting", (gnRunMode == RM_SERVER) ? L"ConEmuC.Server" : L"ConEmuC.ComSpec", 0);
#endif
	if (gpSrv)
	{
		free(gpSrv);
		gpSrv = NULL;
	}
	return iRc;
}

//#if defined(CRTSTARTUP)
//extern "C"{
//  BOOL WINAPI _DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
//};
//
//BOOL WINAPI mainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
//{
//  DllMain(hDll, dwReason, lpReserved);
//  return TRUE;
//}
//#endif

void PrintVersion()
{
	char szProgInfo[255];
	_wsprintfA(szProgInfo, SKIPLEN(countof(szProgInfo)) "ConEmuC build %s. " CECOPYRIGHTSTRING_A "\n", CONEMUVERS);
	_printf(szProgInfo);
}

void PrintDebugInfo()
{
	_printf("Debugger successfully attached to PID=%u\n", gpSrv->dwRootProcess);
	TODO("������� ���������� � ����������� �������, �������, � ����� �������");
}

void Help()
{
	PrintVersion();
	_printf(
	    "This is a console part of ConEmu product.\n"
	    "Usage: ConEmuC [switches] [/U | /A] /C <command line, passed to %%COMSPEC%%>\n"
	    "   or: ConEmuC [switches] /ROOT <program with arguments, far.exe for example>\n"
	    "   or: ConEmuC /ATTACH /NOCMD\n"
		"   or: ConEmuC /ATTACH /[FAR]PID=<PID>\n"
	    "   or: ConEmuC /GUIMACRO <ConEmu GUI macro command>\n"
		"   or: ConEmuC /DEBUGPID=<Debugging PID>\n"
#ifdef _DEBUG
		"   or: ConEmuC /REGCONFONT=<FontName> -> RegisterConsoleFontHKLM\n"
#endif
	    "   or: ConEmuC /?\n"
	    "Switches:\n"
	    "     /[NO]CONFIRM    - [don't] confirm closing console on program termination\n"
	    "     /ATTACH         - auto attach to ConEmu GUI\n"
	    "     /NOCMD          - attach current (existing) console to GUI\n"
		"     /[FAR]PID=<PID> - use <PID> as root process\n"
	    "     /B{W|H|Z}       - define window width, height and buffer height\n"
#ifdef _DEBUG
		"     /BW=<WndX> /BH=<WndY> /BZ=<BufY>\n"
#endif
	    "     /F{N|W|H}    - define console font name, width, height\n"
#ifdef _DEBUG
		"     /FN=<ConFontName> /FH=<FontHeight> /FW=<FontWidth>\n"
#endif
	    "     /LOG[N]      - create (debug) log file, N is number from 0 to 3\n"
#ifdef _DEBUG
		"     /CINMODE==<hex:gnConsoleModeFlags>\n"
		"     /HIDE -> gbForceHideConWnd=TRUE\n"
		"     /GID=<ConEmu.exe PID>\n"
		"     /SETHOOKS=HP{16},PID{10},HT{16},TID{10},ForceGui\n"
		"     /INJECT=PID{10}\n"
		"     /DOSBOX -> use DosBox\n"
#endif
	    "\n"
	    "When you run application from ConEmu console, you may use\n"
        "  Switch: -new_console[:abch[N]rx[N]y[N]u[:user:pwd]]\n"
        "     a - RunAs shell verb (as Admin on Vista+, login/passw in Win2k and WinXP)\n"
        "     b - Create background tab\n"
        "     c - force enable 'Press Enter or Esc to close console' (default)\n"
        "     h<height> - i.e., h0 - turn buffer off, h9999 - switch to 9999 lines\n"
        "     l - lock console size, do not sync it to ConEmu window\n"
        "     n - disable 'Press Enter or Esc to close console'\n"
        "     r - run as restricted user\n"
        "     x<width>, y<height> - change size of visible area, use with 'l'\n"
        "     u - ConEmu choose user dialog\n"
        "     u:<user>:<pwd> - specify user/pwd in args, MUST BE LAST OPTION\n"
        "  Warning: Option 'Inject ConEmuHk' must be enabled in ConEmu settings!\n"
        "  Example: dir \"-new_console:bh9999c\" c:\\ /s\n");
}

void DosBoxHelp()
{
	_printf(
		"Starting DosBox, You may use following default combinations in DosBox window\n"
		"ALT-ENTER     Switch to full screen and back.\n"
		"ALT-PAUSE     Pause emulation (hit ALT-PAUSE again to continue).\n"
		"CTRL-F1       Start the keymapper.\n"
		"CTRL-F4       Change between mounted floppy/CD images. Update directory cache \n"
		"              for all drives.\n"
		"CTRL-ALT-F5   Start/Stop creating a movie of the screen. (avi video capturing)\n"
		"CTRL-F5       Save a screenshot. (PNG format)\n"
		"CTRL-F6       Start/Stop recording sound output to a wave file.\n"
		"CTRL-ALT-F7   Start/Stop recording of OPL commands. (DRO format)\n"
		"CTRL-ALT-F8   Start/Stop the recording of raw MIDI commands.\n"
		"CTRL-F7       Decrease frameskip.\n"
		"CTRL-F8       Increase frameskip.\n"
		"CTRL-F9       Kill DOSBox.\n"
		"CTRL-F10      Capture/Release the mouse.\n"
		"CTRL-F11      Slow down emulation (Decrease DOSBox Cycles).\n"
		"CTRL-F12      Speed up emulation (Increase DOSBox Cycles).\n"
		"ALT-F12       Unlock speed (turbo button/fast forward).\n"
		"F11, ALT-F11  (machine=cga) change tint in NTSC output modes\n"
		"F11           (machine=hercules) cycle through amber, green, white colouring\n"
	);
}

void PrintExecuteError(LPCWSTR asCmd, DWORD dwErr, LPCWSTR asSpecialInfo/*=NULL*/)
{
	if (asSpecialInfo)
	{
		if (*asSpecialInfo)
			_wprintf(asSpecialInfo);
	}
	else
	{
		wchar_t* lpMsgBuf = NULL;
		DWORD nFmtRc, nFmtErr = 0;

		if (dwErr == 5)
		{
			lpMsgBuf = (wchar_t*)LocalAlloc(LPTR, 128*sizeof(wchar_t));
			_wcscpy_c(lpMsgBuf, 128, L"Access is denied.\nThis may be cause of antiviral or file permissions denial.");
		}
		else
		{
			nFmtRc = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, dwErr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMsgBuf, 0, NULL);

			if (!nFmtRc)
				nFmtErr = GetLastError();
		}

		_printf("Can't create process, ErrCode=0x%08X, Description:\n", dwErr);
		_wprintf((lpMsgBuf == NULL) ? L"<Unknown error>" : lpMsgBuf);
		if (lpMsgBuf) LocalFree(lpMsgBuf);
	}

	size_t nCchMax = MAX_PATH*2+32;
	wchar_t* lpInfo = (wchar_t*)calloc(nCchMax,sizeof(*lpInfo));
	if (lpInfo)
	{
		_wcscpy_c(lpInfo, nCchMax, L"\nCurrent directory:\n");
			_ASSERTE(nCchMax>=(MAX_PATH*2+32));
		GetCurrentDirectory(MAX_PATH*2, lpInfo+lstrlen(lpInfo));
		_wcscat_c(lpInfo, nCchMax, L"\n");
		_wprintf(lpInfo);
		free(lpInfo);
	}

	_printf("\nCommand to be executed:\n");
	_wprintf(asCmd ? asCmd : L"<null>");
	_printf("\n");
}

void CheckUnicodeMode()
{
	if (gnCmdUnicodeMode) return;

	wchar_t szValue[16] = {0};

	if (GetEnvironmentVariable(L"ConEmuOutput", szValue, sizeof(szValue)/sizeof(szValue[0])))
	{
		if (lstrcmpi(szValue, L"UNICODE") == 0)
			gnCmdUnicodeMode = 2;
		else if (lstrcmpi(szValue, L"ANSI") == 0)
			gnCmdUnicodeMode = 1;
	}
}

void RegisterConsoleFontHKLM(LPCWSTR pszFontFace)
{
	if (!pszFontFace || !*pszFontFace)
		return;

	HKEY hk;

	if (!RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Console\\TrueTypeFont",
	                  0, KEY_ALL_ACCESS, &hk))
	{
		wchar_t szId[32] = {0}, szFont[255]; DWORD dwLen, dwType;

		for(DWORD i = 0; i <20; i++)
		{
			szId[i] = L'0'; szId[i+1] = 0; wmemset(szFont, 0, 255);

			if (RegQueryValueExW(hk, szId, NULL, &dwType, (LPBYTE)szFont, &(dwLen = 255*2)))
			{
				RegSetValueExW(hk, szId, 0, REG_SZ, (LPBYTE)pszFontFace, (lstrlen(pszFontFace)+1)*2);
				break;
			}

			if (lstrcmpi(szFont, pszFontFace) == 0)
			{
				break; // �� ��� ��������
			}
		}

		RegCloseKey(hk);
	}
}

DWORD WINAPI DebugThread(LPVOID lpvParam);

int CheckAttachProcess()
{
	HWND hConEmu = GetConEmuHWND(TRUE);

	if (hConEmu && IsWindow(hConEmu))
	{
		// Console already attached
		// �������� �������� ������ �� �����
		gbInShutdown = TRUE;
		return CERR_CARGUMENT;
	}

	BOOL lbArgsFailed = FALSE;
	wchar_t szFailMsg[512]; szFailMsg[0] = 0;

	if (pfnGetConsoleProcessList==NULL)
	{
		wcscpy_c(szFailMsg, L"Attach to GUI was requested, but required WinXP or higher!");
		lbArgsFailed = TRUE;
		//_wprintf (GetCommandLineW());
		//_printf ("\n");
		//_ASSERTE(FALSE);
		//gbInShutdown = TRUE; // ����� � ������� �� ������
		//return CERR_CARGUMENT;
	}
	else
	{
		DWORD nProcesses[20];
		DWORD nProcCount = pfnGetConsoleProcessList(nProcesses, 20);

		// 2 ��������, ������ ��� ��� �� ���� � ������� ��� ���� ������� � ���� �������,
		// ����� ������ � ������ ���
		if (nProcCount < 2)
		{
			wcscpy_c(szFailMsg, L"Attach to GUI was requested, but there is no console processes!");
			lbArgsFailed = TRUE;
			//_wprintf (GetCommandLineW());
			//_printf ("\n");
			//_ASSERTE(FALSE);
			//return CERR_CARGUMENT;
		}
		// �� �����, ����� ����� �������� ���� �������, �� (nProcCount > 2) ������ ������.
		// � ������ ������� ������� (/ATTACH /PID=n) ��� ��� ������������ (/ATTACH /NOCMD)
		//// ���� cmd.exe ������� �� cmd.exe (� ������� ��� ������ ���� ���������) - ������ �� ������
		else if ((gpSrv->dwRootProcess != 0) || (nProcCount > 2))
		{
			BOOL lbRootExists = (gpSrv->dwRootProcess == 0);
			// � �������� ������ ��� ����������
			wchar_t szProc[255] = {0}, szTmp[10]; //StringCchPrintf(szProc, countof(szProc), L"%i, %i, %i", nProcesses[0], nProcesses[1], nProcesses[2]);
			DWORD nFindId = 0;

			for(int n = ((int)nProcCount-1); n >= 0; n--)
			{
				if (szProc[0]) wcscat_c(szProc, L", ");

				_wsprintf(szTmp, SKIPLEN(countof(szTmp)) L"%i", nProcesses[n]);
				wcscat_c(szProc, szTmp);

				if (gpSrv->dwRootProcess)
				{
					if (!lbRootExists && nProcesses[n] == gpSrv->dwRootProcess)
						lbRootExists = TRUE;
				}
				else if ((nFindId == 0) && (nProcesses[n] != gnSelfPID))
				{
					// ����� ������� ��� ��������.
					// ����������, ���� ������� �������� �� �����, �.�.
					// ������ �� ��������� �� ��� ��� ���� ��� ���� �� ���� �������
					nFindId = nProcesses[n];
				}
			}

			if ((gpSrv->dwRootProcess == 0) && (nFindId != 0))
			{
				gpSrv->dwRootProcess = nFindId;
				lbRootExists = TRUE;
			}

			if ((gpSrv->dwRootProcess != 0) && !lbRootExists)
			{
				_wsprintf(szFailMsg, SKIPLEN(countof(szFailMsg)) L"Attach to GUI was requested, but\n" L"root process (%u) does not exists", gpSrv->dwRootProcess);
				lbArgsFailed = TRUE;
			}
			else if ((gpSrv->dwRootProcess == 0) && (nProcCount > 2))
			{
				_wsprintf(szFailMsg, SKIPLEN(countof(szFailMsg)) L"Attach to GUI was requested, but\n" L"there is more than 2 console processes: %s\n", szProc);
				lbArgsFailed = TRUE;
			}

			//PRINT_COMSPEC(L"Attach to GUI was requested, but there is more then 2 console processes: %s\n", szProc);
			//_ASSERTE(FALSE);
			//return CERR_CARGUMENT;
		}
	}

	if (lbArgsFailed)
	{

		LPCWSTR pszCmdLine = GetCommandLineW(); if (!pszCmdLine) pszCmdLine = L"";

		int nCmdLen = lstrlen(szFailMsg) + lstrlen(pszCmdLine) + 16;
		wchar_t* pszMsg = (wchar_t*)malloc(nCmdLen*2);
		_wcscpy_c(pszMsg, nCmdLen, szFailMsg);
		_wcscat_c(pszMsg, nCmdLen, L"\n\n");
		_wcscat_c(pszMsg, nCmdLen, pszCmdLine);
		wchar_t szTitle[64]; _wsprintf(szTitle, SKIPLEN(countof(szTitle)) L"ConEmuC, PID=%u", GetCurrentProcessId());
		MessageBox(NULL, pszMsg, szTitle, MB_ICONSTOP|MB_SYSTEMMODAL);
		free(pszMsg);
		gbInShutdown = TRUE;
		return CERR_CARGUMENT;
	}

	return 0; // OK
}

// ������ ���������� ��������� ������
int ParseCommandLine(LPCWSTR asCmdLine/*, wchar_t** psNewCmd, BOOL* pbRunInBackgroundTab*/)
{
	//if (!psNewCmd || !pbRunInBackgroundTab)
	//{
	//	_ASSERTE(psNewCmd && pbRunInBackgroundTab);
	//	return CERR_CARGUMENT;
	//}
	
	int iRc = 0;
	wchar_t szArg[MAX_PATH+1] = {0}, szExeTest[MAX_PATH+1];
	LPCWSTR pszArgStarts = NULL;
	wchar_t szComSpec[MAX_PATH+1] = {0};
	LPCWSTR pwszCopy = NULL;
	wchar_t* psFilePart = NULL;
	//BOOL bViaCmdExe = TRUE;
	gbRunViaCmdExe = TRUE;
	gbRootIsCmdExe = TRUE;
	gbRunInBackgroundTab = FALSE;
	size_t nCmdLine = 0;
	LPCWSTR pwszStartCmdLine = asCmdLine;
	BOOL lbNeedCutStartEndQuot = FALSE;

	if (!asCmdLine || !*asCmdLine)
	{
		DWORD dwErr = GetLastError();
		_printf("GetCommandLineW failed! ErrCode=0x%08X\n", dwErr);
		return CERR_GETCOMMANDLINE;
	}

	#ifdef _DEBUG
	// ��� ������ ������� ���������
	//_ASSERTE(wcsstr(asCmdLine, L"/DEBUGPID=")==0);
	#endif

	gnRunMode = RM_UNDEFINED;

	while ((iRc = NextArg(&asCmdLine, szArg, &pszArgStarts)) == 0)
	{
		xf_check();

		if ((szArg[0] == L'/' || szArg[0] == L'-')
		        && (szArg[1] == L'?' || ((szArg[1] & ~0x20) == L'H'))
		        && szArg[2] == 0)
		{
			Help();
			return CERR_HELPREQUESTED;
		}

		// ����� - ��������� ����� � ��������� ��� "/"
		if (szArg[0] != L'/')
			continue;

		if (wcsncmp(szArg, L"/REGCONFONT=", 12)==0)
		{
			RegisterConsoleFontHKLM(szArg+12);
			return CERR_EMPTY_COMSPEC_CMDLINE;
		}
		else if (wcscmp(szArg, L"/CONFIRM")==0)
		{
			TODO("��������, ��� ����� � gbAutoDisableConfirmExit");
			gnConfirmExitParm = 1;
			gbAlwaysConfirmExit = TRUE; gbAutoDisableConfirmExit = FALSE;
		}
		else if (wcscmp(szArg, L"/NOCONFIRM")==0)
		{
			gnConfirmExitParm = 2;
			gbAlwaysConfirmExit = FALSE; gbAutoDisableConfirmExit = FALSE;
		}
		else if (wcscmp(szArg, L"/ATTACH")==0)
		{
			#if defined(SHOW_ATTACH_MSGBOX)
			if (!IsDebuggerPresent())
			{
				wchar_t szTitle[100]; _wsprintf(szTitle, SKIPLEN(countof(szTitle)) L"ConEmuC (PID=%i) /ATTACH", gnSelfPID);
				const wchar_t* pszCmdLine = GetCommandLineW();
				MessageBox(NULL,pszCmdLine,szTitle,MB_SYSTEMMODAL);
			}
			#endif

			gbAttachMode = TRUE;
			gnRunMode = RM_SERVER;
		}
		else if (wcscmp(szArg, L"/NOCMD")==0)
		{
			gnRunMode = RM_SERVER;
			gbNoCreateProcess = TRUE;
			gbAlienMode = TRUE;
		}
		else if (wcsncmp(szArg, L"/PID=", 5)==0 || wcsncmp(szArg, L"/FARPID=", 8)==0)
		{
			gnRunMode = RM_SERVER;
			gbNoCreateProcess = TRUE;
			gbAlienMode = TRUE;
			wchar_t* pszEnd = NULL, *pszStart;

			if (wcsncmp(szArg, L"/FARPID=", 8)==0)
			{
				gbAttachFromFar = TRUE;
				gbRootIsCmdExe = FALSE;
				pszStart = szArg+8;
			}
			else
			{
				pszStart = szArg+5;
			}

			gpSrv->dwRootProcess = wcstoul(pszStart, &pszEnd, 10);

			if (gpSrv->dwRootProcess == 0)
			{
				_printf("Attach to GUI was requested, but invalid PID specified:\n");
				_wprintf(GetCommandLineW());
				_printf("\n");
				_ASSERTE(FALSE);
				return CERR_CARGUMENT;
			}
		}
		else if (wcsncmp(szArg, L"/CINMODE=", 9)==0)
		{
			wchar_t* pszEnd = NULL, *pszStart = szArg+9;
			gnConsoleModeFlags = wcstoul(pszStart, &pszEnd, 16);
			// ���� ������� 0 - ��������� (ENABLE_QUICK_EDIT_MODE|ENABLE_EXTENDED_FLAGS|ENABLE_INSERT_MODE)
			gbConsoleModeFlags = (gnConsoleModeFlags != 0);
		}
		else if (wcscmp(szArg, L"/HIDE")==0)
		{
			gbForceHideConWnd = TRUE;
		}
		else if (wcsncmp(szArg, L"/B", 2)==0)
		{
			wchar_t* pszEnd = NULL;

			if (wcsncmp(szArg, L"/BW=", 4)==0)
			{
				gcrVisibleSize.X = /*_wtoi(szArg+4);*/(SHORT)wcstol(szArg+4,&pszEnd,10); gbParmVisibleSize = TRUE;
			}
			else if (wcsncmp(szArg, L"/BH=", 4)==0)
			{
				gcrVisibleSize.Y = /*_wtoi(szArg+4);*/(SHORT)wcstol(szArg+4,&pszEnd,10); gbParmVisibleSize = TRUE;
			}
			else if (wcsncmp(szArg, L"/BZ=", 4)==0)
			{
				gnBufferHeight = /*_wtoi(szArg+4);*/(SHORT)wcstol(szArg+4,&pszEnd,10); gbParmBufSize = TRUE;
			}
			TODO("/BX ��� ������ ������?");
		}
		else if (wcsncmp(szArg, L"/F", 2)==0)
		{
			wchar_t* pszEnd = NULL;

			if (wcsncmp(szArg, L"/FN=", 4)==0) //-V112
			{
				lstrcpynW(gpSrv->szConsoleFont, szArg+4, 32); //-V112
			}
			else if (wcsncmp(szArg, L"/FW=", 4)==0) //-V112
			{
				gpSrv->nConFontWidth = /*_wtoi(szArg+4);*/(SHORT)wcstol(szArg+4,&pszEnd,10);
			}
			else if (wcsncmp(szArg, L"/FH=", 4)==0) //-V112
			{
				gpSrv->nConFontHeight = /*_wtoi(szArg+4);*/(SHORT)wcstol(szArg+4,&pszEnd,10);
				//} else if (wcsncmp(szArg, L"/FF=", 4)==0) {
				//  lstrcpynW(gpSrv->szConsoleFontFile, szArg+4, MAX_PATH);
			}
		}
		else if (wcsncmp(szArg, L"/LOG",4)==0) //-V112
		{
			int nLevel = 0;
			if (szArg[4]==L'1') nLevel = 1; else if (szArg[4]>=L'2') nLevel = 2;

			CreateLogSizeFile(nLevel);
		}
		else if (wcsncmp(szArg, L"/GID=", 5)==0)
		{
			gnRunMode = RM_SERVER;
			wchar_t* pszEnd = NULL;
			gpSrv->dwGuiPID = wcstoul(szArg+5, &pszEnd, 10);

			if (gpSrv->dwGuiPID == 0)
			{
				_printf("Invalid GUI PID specified:\n");
				_wprintf(GetCommandLineW());
				_printf("\n");
				_ASSERTE(FALSE);
				return CERR_CARGUMENT;
			}
		}
		else if (wcsncmp(szArg, L"/DEBUGPID=", 10)==0)
		{
			gnRunMode = RM_SERVER;
			gbNoCreateProcess = gbDebugProcess = TRUE;
			wchar_t* pszEnd = NULL;
			//gpSrv->dwRootProcess = _wtol(szArg+10);
			gpSrv->dwRootProcess = wcstoul(szArg+10, &pszEnd, 10);

			if (gpSrv->dwRootProcess == 0)
			{
				_printf("Debug of process was requested, but invalid PID specified:\n");
				_wprintf(GetCommandLineW());
				_printf("\n");
				_ASSERTE(FALSE);
				return CERR_CARGUMENT;
			}
		}
		else if (wcscmp(szArg, L"/A")==0 || wcscmp(szArg, L"/a")==0)
		{
			gnCmdUnicodeMode = 1;
		}
		else if (wcscmp(szArg, L"/U")==0 || wcscmp(szArg, L"/u")==0)
		{
			gnCmdUnicodeMode = 2;
		}
		// ����� ���� ���������� - ���� ��, ��� ���������� � CreateProcess!
		else if (wcscmp(szArg, L"/ROOT")==0 || wcscmp(szArg, L"/root")==0)
		{
			gnRunMode = RM_SERVER; gbNoCreateProcess = FALSE;
			break; // asCmdLine ��� ��������� �� ����������� ���������
		}
		else if (wcsncmp(szArg, L"/SETHOOKS=", 10) == 0)
		{
#ifdef SHOW_INJECT_MSGBOX
			wchar_t szDbgMsg[128], szTitle[128];
			swprintf_c(szTitle, SKIPLEN(countof(szTitle)) L"ConEmuHk, PID=%u", GetCurrentProcessId());
			swprintf_c(szDbgMsg, SKIPLEN(countof(szDbgMsg)) L"%s\nConEmuHk, PID=%u", szArg, GetCurrentProcessId());
			MessageBoxW(NULL, szDbgMsg, szTitle, MB_SYSTEMMODAL);
#endif
			gbInShutdown = TRUE; // ����� �� �������� �������� ��� ������
			gnRunMode = RM_SETHOOK64;
			LPWSTR pszNext = szArg+10;
			LPWSTR pszEnd = NULL;
			BOOL lbForceGui = FALSE;
			PROCESS_INFORMATION pi = {NULL};
			pi.hProcess = (HANDLE)wcstoul(pszNext, &pszEnd, 16);

			if (pi.hProcess && pszEnd && *pszEnd)
			{
				pszNext = pszEnd+1;
				pi.dwProcessId = wcstoul(pszNext, &pszEnd, 10);
			}

			if (pi.dwProcessId && pszEnd && *pszEnd)
			{
				pszNext = pszEnd+1;
				pi.hThread = (HANDLE)wcstoul(pszNext, &pszEnd, 16);
			}

			if (pi.hThread && pszEnd && *pszEnd)
			{
				pszNext = pszEnd+1;
				pi.dwThreadId = wcstoul(pszNext, &pszEnd, 10);
			}

			if (pi.dwThreadId && pszEnd && *pszEnd)
			{
				pszNext = pszEnd+1;
				lbForceGui = wcstoul(pszNext, &pszEnd, 10);
			}
			
			if (pi.hProcess && pi.hThread && pi.dwProcessId && pi.dwThreadId)
			{
				//BOOL gbLogProcess = FALSE;
				//TODO("�������� �� �������� glt_Process");
				//#ifdef _DEBUG
				//gbLogProcess = TRUE;
				//#endif
				int iHookRc = InjectHooks(pi, lbForceGui, gbLogProcess);

				if (iHookRc == 0)
					return CERR_HOOKS_WAS_SET;

				// ������ (���� �� ������ ������) ����� ��������, ��� ������ ��������� �������
				DWORD nErrCode = GetLastError();
				//_ASSERTE(iHookRc == 0); -- ������ �� �����, ���� MsgBox
				wchar_t szDbgMsg[255], szTitle[128];
				_wsprintf(szTitle, SKIPLEN(countof(szTitle)) L"ConEmuC, PID=%u", GetCurrentProcessId());
				_wsprintf(szDbgMsg, SKIPLEN(countof(szDbgMsg)) L"ConEmuC.X, PID=%u\nInjecting hooks into PID=%u\nFAILED, code=%i:0x%08X", GetCurrentProcessId(), pi.dwProcessId, iHookRc, nErrCode);
				MessageBoxW(NULL, szDbgMsg, szTitle, MB_SYSTEMMODAL);
			}
			else
			{
				//_ASSERTE(pi.hProcess && pi.hThread && pi.dwProcessId && pi.dwThreadId);
				wchar_t szDbgMsg[512], szTitle[128];
				_wsprintf(szTitle, SKIPLEN(countof(szTitle)) L"ConEmuC, PID=%u", GetCurrentProcessId());
				_wsprintf(szDbgMsg, SKIPLEN(countof(szDbgMsg)) L"ConEmuC.X, PID=%u\nCmdLine parsing FAILED (%u,%u,%u,%u,%u)!\n%s",
					GetCurrentProcessId(), (DWORD)pi.hProcess, (DWORD)pi.hThread, pi.dwProcessId, pi.dwThreadId, lbForceGui, //-V205
					szArg);
				MessageBoxW(NULL, szDbgMsg, szTitle, MB_SYSTEMMODAL);
				//return CERR_HOOKS_FAILED;
			}

			return CERR_HOOKS_FAILED;
		}
		else if (wcsncmp(szArg, L"/INJECT=", 8) == 0)
		{
			gbInShutdown = TRUE; // ����� �� �������� �������� ��� ������
			gnRunMode = RM_SETHOOK64;
			LPWSTR pszNext = szArg+8;
			LPWSTR pszEnd = NULL;
			DWORD nRemotePID = wcstoul(pszNext, &pszEnd, 10);
			
			if (nRemotePID)
			{
				#if defined(SHOW_ATTACH_MSGBOX)
				if (!IsDebuggerPresent())
				{
					wchar_t szTitle[100]; _wsprintf(szTitle, SKIPLEN(countof(szTitle)) L"ConEmuC (PID=%i) /INJECT", gnSelfPID);
					const wchar_t* pszCmdLine = GetCommandLineW();
					MessageBox(NULL,pszCmdLine,szTitle,MB_SYSTEMMODAL);
				}
				#endif

				int iHookRc = InjectRemote(nRemotePID);

				if (iHookRc == 0)
					return CERR_HOOKS_WAS_SET;

				// ������ (���� �� ������ ������) ����� ��������, ��� ������ ��������� �������
				DWORD nErrCode = GetLastError();
				//_ASSERTE(iHookRc == 0); -- ������ �� �����, ���� MsgBox
				wchar_t szDbgMsg[255], szTitle[128];
				_wsprintf(szTitle, SKIPLEN(countof(szTitle)) L"ConEmuC, PID=%u", GetCurrentProcessId());
				_wsprintf(szDbgMsg, SKIPLEN(countof(szDbgMsg)) L"ConEmuC.X, PID=%u\nInjecting remote into PID=%u\nFAILED, code=%i:0x%08X", GetCurrentProcessId(), nRemotePID, iHookRc, nErrCode);
				MessageBoxW(NULL, szDbgMsg, szTitle, MB_SYSTEMMODAL);
			}
			else
			{
				//_ASSERTE(pi.hProcess && pi.hThread && pi.dwProcessId && pi.dwThreadId);
				wchar_t szDbgMsg[512], szTitle[128];
				_wsprintf(szTitle, SKIPLEN(countof(szTitle)) L"ConEmuC, PID=%u", GetCurrentProcessId());
				_wsprintf(szDbgMsg, SKIPLEN(countof(szDbgMsg)) L"ConEmuC.X, PID=%u\nCmdLine parsing FAILED (%u)!\n%s",
					GetCurrentProcessId(), nRemotePID,
					szArg);
				MessageBoxW(NULL, szDbgMsg, szTitle, MB_SYSTEMMODAL);
				//return CERR_HOOKS_FAILED;
			}

			return CERR_HOOKS_FAILED;
		}
		else if (lstrcmpi(szArg, L"/GUIMACRO")==0)
		{
			// ��� ��� � asCmdLine - ��������� � Gui
			int iRc = CERR_GUIMACRO_FAILED;
			int nLen = lstrlen(asCmdLine);
			//SetEnvironmentVariable(CEGUIMACRORETENVVAR, NULL);
			CESERVER_REQ *pIn = NULL, *pOut = NULL;
			pIn = ExecuteNewCmd(CECMD_GUIMACRO, sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_GUIMACRO)+nLen*sizeof(wchar_t));
			lstrcpyW(pIn->GuiMacro.sMacro, asCmdLine);
			pOut = ExecuteGuiCmd(ghConWnd, pIn, ghConWnd);
			if (pOut)
			{
				if (pOut->GuiMacro.nSucceeded)
				{
					// ������ ���, ���������� � ������������ ������� ���-����� �� �������
					//SetEnvironmentVariable(CEGUIMACRORETENVVAR,
					//	pOut->GuiMacro.nSucceeded ? pOut->GuiMacro.sMacro : NULL);
					_wprintf(pOut->GuiMacro.sMacro);
					iRc = CERR_GUIMACRO_SUCCEEDED;
				}
				ExecuteFreeResult(pOut);
			}
			ExecuteFreeResult(pIn);
			return iRc;
		}
		else if (lstrcmpi(szArg, L"/DOSBOX")==0)
		{
			gbUseDosBox = TRUE;
		}
		// ����� ���� ���������� - ���� ��, ��� ���������� � COMSPEC (CreateProcess)!
		//if (wcscmp(szArg, L"/C")==0 || wcscmp(szArg, L"/c")==0 || wcscmp(szArg, L"/K")==0 || wcscmp(szArg, L"/k")==0) {
		else if (szArg[0] == L'/' && (((szArg[1] & ~0x20) == L'C') || ((szArg[1] & ~0x20) == L'K')))
		{
			gbNoCreateProcess = FALSE;

			if (szArg[2] == 0)  // "/c" ��� "/k"
				gnRunMode = RM_COMSPEC;

			if (gnRunMode == RM_UNDEFINED && szArg[4] == 0
			        && ((szArg[2] & ~0x20) == L'M') && ((szArg[3] & ~0x20) == L'D'))
				gnRunMode = RM_SERVER;

			// ���� ��� ������ �� ��� ��� �� ���������� - ������� ��� ����� ComSpec
			// � ������� ���������� ����� ����� /c (����� ���� "cmd /cecho xxx")
			if (gnRunMode == RM_UNDEFINED)
			{
				gnRunMode = RM_COMSPEC;
				// ��������� ����������� "cmd /cecho xxx"
				asCmdLine = pszArgStarts + 2;

				while(*asCmdLine==L' ' || *asCmdLine==L'\t') asCmdLine++;
			}

			if (gnRunMode == RM_COMSPEC)
			{
				gpSrv->bK = (szArg[1] & ~0x20) == L'K';
			}

			break; // asCmdLine ��� ��������� �� ����������� ���������
		}
	}

	// Issue 364, ��������, ���� ���� � VS, ����������� CustomStep, � ���� ������ ��������� ����� �� �����
	// ������������, � ������ �� ������ �� ���� ������� ConEmuC.exe, �� �� ����� ��������� � "COMSPEC", ��� ��� ��������.
	if (gbAttachMode && (gnRunMode == RM_SERVER) && (gpSrv->dwGuiPID == 0))
	{
		BOOL lbIsWindowVisible = FALSE;
		if (!ghConWnd || !(lbIsWindowVisible = IsWindowVisible(ghConWnd)))
		{
			// �� ��� ����� ���� ���-���� ���� ������. ��� ���������...
			// ������ ������ ��������
			LPCWSTR pszSlash = asCmdLine ? wcschr(asCmdLine, L'/') : NULL;
			if (pszSlash)
			{
				// � ������� � ������������� � ���. �������� ����� ��� ���-�� �������� ��������
				if (wmemcmp(pszSlash, L"/DEBUGPID=", 10) != 0)
					pszSlash = NULL;
			}
			if (pszSlash == NULL)
			{
				// �� ���� ������, �������
				return CERR_ATTACH_NO_CONWND;
			}
		}
	}

	xf_check();

	if (gnRunMode == RM_SERVER)
	{
		if (gbDebugProcess)
		{
			// ������� � ������� ���������� � ������.
			PrintVersion();
#ifdef SHOW_DEBUG_STARTED_MSGBOX
			wchar_t szInfo[128];
			StringCchPrintf(szInfo, countof(szInfo), L"Attaching debugger...\nConEmuC PID = %u\nDebug PID = %u",
			                GetCurrentProcessId(), gpSrv->dwRootProcess);
			MessageBox(GetConEmuHWND(2), szInfo, L"ConEmuC.Debugger", 0);
#endif
			//if (!DebugActiveProcess(gpSrv->dwRootProcess))
			//{
			//	DWORD dwErr = GetLastError();
			//	_printf("Can't start debugger! ErrCode=0x%08X\n", dwErr);
			//	return CERR_CANTSTARTDEBUGGER;
			//}
			//// �������������� �������������, ����� �������� �������� (��� �������) �� �������
			//// � �������� "������������" ���������
			//pfnDebugActiveProcessStop = (FDebugActiveProcessStop)GetProcAddress(GetModuleHandle(L"kernel32.dll"),"DebugActiveProcessStop");
			//pfnDebugSetProcessKillOnExit = (FDebugSetProcessKillOnExit)GetProcAddress(GetModuleHandle(L"kernel32.dll"),"DebugSetProcessKillOnExit");
			//if (pfnDebugSetProcessKillOnExit)
			//	pfnDebugSetProcessKillOnExit(FALSE/*KillOnExit*/);
			//gpSrv->bDebuggerActive = TRUE;
			//PrintDebugInfo();
			gpSrv->hDebugReady = CreateEvent(NULL, FALSE, FALSE, NULL);
			// ������� ��������� ���������� ������� � ��������� ����, ����� �������� �� ��������������� � �������
			gpSrv->hDebugThread = CreateThread(NULL, 0, DebugThread, NULL, 0, &gpSrv->dwDebugThreadId);
			HANDLE hEvents[2] = {gpSrv->hDebugReady, gpSrv->hDebugThread};
			DWORD nReady = WaitForMultipleObjects(countof(hEvents), hEvents, FALSE, INFINITE);

			if (nReady != WAIT_OBJECT_0)
			{
				DWORD nExit = 0;
				GetExitCodeThread(gpSrv->hDebugThread, &nExit);
				return nExit;
			}

			gpszRunCmd = (wchar_t*)calloc(1,2);

			if (!gpszRunCmd)
			{
				_printf("Can't allocate 1 wchar!\n");
				return CERR_NOTENOUGHMEM1;
			}

			gpszRunCmd[0] = 0;
			gpSrv->bDebuggerActive = TRUE;
			return 0;
		}
		else if (gbNoCreateProcess && gbAttachMode)
		{
			// ��������� �������� � �������, ��������� ���, ������� ����� ������� "��������"
			int nChk = CheckAttachProcess();

			if (nChk != 0)
				return nChk;

			gpszRunCmd = (wchar_t*)calloc(1,2);

			if (!gpszRunCmd)
			{
				_printf("Can't allocate 1 wchar!\n");
				return CERR_NOTENOUGHMEM1;
			}

			gpszRunCmd[0] = 0;
			return 0;
		}
	}

	xf_check();

	if (iRc != 0)
	{
		if (iRc == CERR_CMDLINEEMPTY)
		{
			Help();
			_printf("\n\nParsing command line failed (/C argument not found):\n");
			_wprintf(GetCommandLineW());
			_printf("\n");
		}
		else
		{
			_printf("Parsing command line failed:\n");
			_wprintf(asCmdLine);
			_printf("\n");
		}

		return iRc;
	}

	if (gnRunMode == RM_UNDEFINED)
	{
		_printf("Parsing command line failed (/C argument not found):\n");
		_wprintf(GetCommandLineW());
		_printf("\n");
		_ASSERTE(FALSE);
		return CERR_CARGUMENT;
	}

	xf_check();

	if (gnRunMode == RM_COMSPEC)
	{
		// ����� ������� ������� ����� �������?
		int nArgLen = lstrlenA("-new_console");
		pwszCopy = (wchar_t*)wcsstr(asCmdLine, L"-new_console");

		// ���� ����� -new_console ���� ������, ��� ��� ������ ����� ������
		// 111211 - ����� -new_console: ����������� ���������
		if (pwszCopy
			&& ((pwszCopy > asCmdLine) || (*(pwszCopy-1) == L' ') || (*(pwszCopy-1) == L'"'))
			&& (pwszCopy[nArgLen]==L' ' || pwszCopy[nArgLen]==L':' || pwszCopy[nArgLen]==0
		         || (pwszCopy[nArgLen]==L'"' || pwszCopy[nArgLen+1]==0)))
		{
			if (!ghConWnd)
			{
				// �����������!
				_ASSERTE(ghConWnd != NULL);
			}
			else
			{
				xf_check();
				// ����� ������������
				gpSrv->bNewConsole = TRUE;

				// �� ����, ������ ����������� � ���� ConEmu (� ������������ �������), �� ���� ���
				HWND hConEmu = ghConEmuWnd;
				if (!hConEmu || !IsWindow(hConEmu))
				{
					_ASSERTE(ghConEmuWnd!=NULL && IsWindow(ghConEmuWnd));
					// ���������� ����� �������� ConEmu
					hConEmu = FindWindowEx(NULL, NULL, VirtualConsoleClassMain, NULL);
					if (hConEmu)
						gbNonGuiMode = TRUE; // ����� �� �������� ��������� SendStopped (��� ������)
				}

				int iNewConRc = CERR_RUNNEWCONSOLE;

				DWORD nCmdLen = lstrlen(asCmdLine);
				CESERVER_REQ* pIn = ExecuteNewCmd(CECMD_NEWCMD, sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_NEWCMD)+(nCmdLen*sizeof(wchar_t)));
				if (pIn)
				{
					pIn->NewCmd.hFromConWnd = ghConWnd;
					GetCurrentDirectory(countof(pIn->NewCmd.szCurDir), pIn->NewCmd.szCurDir);
					lstrcpyn(pIn->NewCmd.szCommand, asCmdLine, nCmdLen+1);

					CESERVER_REQ* pOut = ExecuteGuiCmd(hConEmu, pIn, ghConWnd);
					if (pOut)
					{
						if (pOut->hdr.cbSize <= sizeof(pOut->hdr) || pOut->Data[0] == FALSE)
						{
							iNewConRc = CERR_RUNNEWCONSOLEFAILED;
						}
						ExecuteFreeResult(pOut);
					}
					else
					{
						_ASSERTE(pOut!=NULL);
						iNewConRc = CERR_RUNNEWCONSOLEFAILED;
					}
					ExecuteFreeResult(pIn);
				}
				else
				{
					iNewConRc = CERR_NOTENOUGHMEM1;
				}

				DisableAutoConfirmExit();
				return iNewConRc;

				#if 0
				gpSrv->bNewConsole = TRUE;
				//
				size_t nNewLen = lstrlen(pwszStartCmdLine) + 200;
				//
				BOOL lbIsNeedCmd = IsNeedCmd(asCmdLine, &lbNeedCutStartEndQuot, szExeTest, gbRootIsCmdExe, gbAlwaysConfirmExit, gbAutoDisableConfirmExit);
				xf_check();
				//Warning. ParseCommandLine ���������� �� ComSpecInit, � ������� �������
				//         CECMD_CMDSTARTSTOP, ������� ������ ������ ��� �� ���� �����������.
				SendStarted();
				xf_check();
				// Font, size, etc.
				CESERVER_REQ *pIn = NULL, *pOut = NULL;
				wchar_t* pszAddNewConArgs = NULL;

				//
				if ((pIn = ExecuteNewCmd(CECMD_GETNEWCONPARM, sizeof(CESERVER_REQ_HDR)+2*sizeof(DWORD))) != NULL)
				{
					pIn->dwData[0] = gnSelfPID;
					pIn->dwData[1] = lbIsNeedCmd;
					xf_check();
					PRINT_COMSPEC(L"Retrieve new console add args (begin)\n",0);
					pOut = ExecuteGuiCmd(ghConWnd, pIn, ghConWnd);
					PRINT_COMSPEC(L"Retrieve new console add args (begin)\n",0);
					xf_check();

					if (pOut)
					{
						pszAddNewConArgs = (wchar_t*)pOut->Data;

						if (*pszAddNewConArgs == 0)
						{
							ExecuteFreeResult(pOut); pOut = NULL; pszAddNewConArgs = NULL;
							xf_check();
						}
						else
						{
							nNewLen += lstrlen(pszAddNewConArgs) + 1;
						}
					}

					ExecuteFreeResult(pIn); pIn = NULL;
					xf_check();
				}

				//
				size_t nCchNew = nNewLen+1;
				xf_check();
				wchar_t* pszNewCmd = (wchar_t*)calloc(nCchNew,2);

				if (!pszNewCmd)
				{
					_printf("Can't allocate %i wchars!\n", (DWORD)nNewLen);
					return CERR_NOTENOUGHMEM1;
				}

				// ������� ����������� ���, ��� ���� �� /c
				const wchar_t* pszC = asCmdLine;

				while(*pszC != L'/') pszC --;

				nNewLen = pszC - pwszStartCmdLine;
				_ASSERTE(nNewLen>0);
				_wcscpyn_c(pszNewCmd, nCchNew, pwszStartCmdLine, nNewLen);
				pszNewCmd[nNewLen] = 0; // !!! wcsncpy �� ������ ����������� '\0'
				xf_check();

				// �������� ������ ��������
				if (!gbAttachMode)  // ���� ����� ��� ��� � ���.������ - �������
					_wcscat_c(pszNewCmd, nCchNew, L" /ATTACH ");

				if (!gbAlwaysConfirmExit)  // ���� ����� ��� ��� � ���.������ - �������
					_wcscat_c(pszNewCmd, nCchNew, L" /CONFIRM ");

				xf_check();

				if (pszAddNewConArgs)
				{
					_wcscat_c(pszNewCmd, nCchNew, L" ");
					_wcscat_c(pszNewCmd, nCchNew, pszAddNewConArgs);
					xf_check();
				}
				else
				{
					// -new_console ���������� � ������ ComSpec. ������ �� ������ ������� ������� ������� �� ������, �������� GuiPID � �������� � ���������
					MFileMapping<CESERVER_CONSOLE_MAPPING_HDR> ConMap;
					ConMap.InitName(CECONMAPNAME, (DWORD)ghConWnd);
					const CESERVER_CONSOLE_MAPPING_HDR* pConMap = ConMap.Open();

					if (pConMap)
					{
						if (pConMap->nGuiPID)
						{
							int nCurLen = lstrlen(pszNewCmd);
							_wsprintf(pszNewCmd+nCurLen, SKIPLEN(nCchNew-nCurLen)
							          L" /GID=%i ", pConMap->nGuiPID);
						}

						ConMap.CloseMap();
					}

					xf_check();

					// ������� ������ ���� ������-��
					//2009-08-13 ���� �������������� (� ������ ComSpec ��������� /BW /BH /BZ �����������, �.�. ������ ���� �� FAR)
					//			 ������ ����������, ��� ��������� ������ (�� ������������� �� GUI)
					//			 �� ������� ������������ � � ��������� ������� ��������
					//			 ���� ������� (������� ������ � GUI) � ReadConsoleData
					if (MyGetConsoleScreenBufferInfo(ghConOut, &gpSrv->sbi))
					{
						int nBW = gpSrv->sbi.dwSize.X;
						int nBH = gpSrv->sbi.srWindow.Bottom - gpSrv->sbi.srWindow.Top + 1;
						int nBZ = gpSrv->sbi.dwSize.Y;

						if (nBZ <= nBH) nBZ = 0;

						int nCurLen = lstrlen(pszNewCmd);
						_wsprintf(pszNewCmd+nCurLen, SKIPLEN(nCchNew-nCurLen)
						          L" /BW=%i /BH=%i /BZ=%i ", nBW, nBH, nBZ);
					}

					xf_check();
					//lstrcatW(pszNewCmd, L" </BW=9999 /BH=9999 /BZ=9999> ");
				}

				// ������������ ����� �������
				// "cmd" ������ ��� ���� �� ������� �������� ������� � ������, ������� �� �� �����
				// cmd /c ""c:\program files\arc\7z.exe" -?"   // �� ��� � ������ ����� ���� ��������...
				// cmd /c "dir c:\"
				// � ��.
				// ���������� ���������� ������������� cmd
				if (lbIsNeedCmd)
				{
					CheckUnicodeMode();

					if (gnCmdUnicodeMode == 2)
						_wcscat_c(pszNewCmd, nCchNew, L" /ROOT cmd /U /C ");
					else if (gnCmdUnicodeMode == 1)
						_wcscat_c(pszNewCmd, nCchNew, L" /ROOT cmd /A /C ");
					else
						_wcscat_c(pszNewCmd, nCchNew, L" /ROOT cmd /C ");

					xf_check();
				}
				else
				{
					_wcscat_c(pszNewCmd, nCchNew, L" /ROOT ");
					xf_check();
				}

				// ������ �� ����������� ������� "-new_console"
				nNewLen = pwszCopy - asCmdLine;
				int nCurLen = lstrlen(pszNewCmd);
				psFilePart = pszNewCmd + nCurLen;
				xf_check();
				_wcscpyn_c(psFilePart, nCchNew-nCurLen, asCmdLine, nNewLen);
				xf_check();
				psFilePart[nNewLen] = 0; // !!! wcsncpy �� ������ ����������� '\0'
				psFilePart += nNewLen;
				pwszCopy += nArgLen;
				xf_check();

				// �������� � ������� ������� ���������� ��������� � �����������
				if (*pwszCopy)
					_wcscpy_c(psFilePart, nCchNew-(psFilePart-pszNewCmd), pwszCopy);

				xf_check();
				//MessageBox(NULL, pszNewCmd, L"CmdLine", 0);
				//return 200;
				// ����� �����������
				gpszRunCmd = pszNewCmd;
				// 26.06.2009 Maks - ����� ����� ����� - ��� ��������� ����� � ����� �������.
				DisableAutoConfirmExit();
				xf_check();
				//gpSrv->nProcessStartTick = GetTickCount() - 2*CHECK_ROOTSTART_TIMEOUT; //2010-03-06
				return 0;
				#endif
			}
		}

		//pwszCopy = asCmdLine;
		//if ((iRc = NextArg(&pwszCopy, szArg)) != 0) {
		//    wprintf (L"Parsing command line failed:\n%s\n", asCmdLine);
		//    return iRc;
		//}
		//pwszCopy = wcsrchr(szArg, L'\\'); if (!pwszCopy) pwszCopy = szArg;
		//#pragma warning( push )
		//#pragma warning(disable : 6400)
		//if (lstrcmpiW(pwszCopy, L"cmd")==0 || lstrcmpiW(pwszCopy, L"cmd.exe")==0) {
		//    gbRunViaCmdExe = FALSE; // ��� ������ ��������� ���������, cmd.exe � ������ ��������� �� �����
		//}
		//#pragma warning( pop )
		//} else {
		//    gbRunViaCmdExe = FALSE; // ��������� ����������� ��������� ��� ConEmuC (��������� �����)
	}

	if (gnRunMode == RM_COMSPEC && (!asCmdLine || !*asCmdLine))
	{
		if (gpSrv->bK)
		{
			gbRunViaCmdExe = TRUE;
		}
		else
		{
			// � ���� ����� �������� ������ ���������� �� �����
			// *.ini -> "@" - ����� ��� ��� �� ������ �� ������ ��� ������� ����� �����, �� ComSpec �����...
			gbNonGuiMode = TRUE;
			DisableAutoConfirmExit();
			return CERR_EMPTY_COMSPEC_CMDLINE;
		}
	}
	else
	{
		BOOL bAlwaysConfirmExit = gbAlwaysConfirmExit, bAutoDisableConfirmExit = gbAutoDisableConfirmExit;

		gbRunViaCmdExe = IsNeedCmd(asCmdLine, &lbNeedCutStartEndQuot, szExeTest, gbRootIsCmdExe, bAlwaysConfirmExit, bAutoDisableConfirmExit);

		if (gnConfirmExitParm == 0)
		{
			gbAlwaysConfirmExit = bAlwaysConfirmExit;
			gbAutoDisableConfirmExit = bAutoDisableConfirmExit;
		}
	}

#ifndef WIN64

	// ������� ����: C:\Windows\SysNative\reg.exe Query "HKCU\Software\Far2"|find "Far"
	// ��� ��� ������ ��������� ���������� (wow.Disable()), ����� SysNative ����� ����������
	if (IsWindows64(NULL))
	{
		LPCWSTR pszTest = asCmdLine;
		wchar_t szApp[MAX_PATH+1];

		if (NextArg(&pszTest, szApp) == 0)
		{
			wchar_t szSysnative[MAX_PATH+32];
			int nLen = GetWindowsDirectory(szSysnative, MAX_PATH);

			if (nLen >= 2 && nLen < MAX_PATH)
			{
				if (szSysnative[nLen-1] != L'\\')
				{
					szSysnative[nLen++] = L'\\'; szSysnative[nLen] = 0;
				}

				lstrcatW(szSysnative, L"Sysnative\\");
				nLen = lstrlenW(szSysnative);
				int nAppLen = lstrlenW(szApp);

				if (nAppLen > nLen)
				{
					szApp[nLen] = 0;

					if (lstrcmpiW(szApp, szSysnative) == 0)
					{
						gbSkipWowChange = TRUE;
					}
				}
			}
		}
	}

#endif
	nCmdLine = lstrlenW(asCmdLine);

	if (!gbRunViaCmdExe)
	{
		nCmdLine += 1; // ������ ����� ��� 0
	}
	else
	{
		// ���� ���������� ComSpecC - ������ ConEmuC ������������� ����������� ComSpec
		if (!GetEnvironmentVariable(L"ComSpecC", szComSpec, MAX_PATH) || szComSpec[0] == 0)
			if (!GetEnvironmentVariable(L"ComSpec", szComSpec, MAX_PATH) || szComSpec[0] == 0)
				szComSpec[0] = 0;

		if (szComSpec[0] != 0)
		{
			// ������ ���� ��� (��������) �� conemuc.exe
			//pwszCopy = wcsrchr(szComSpec, L'\\'); if (!pwszCopy) pwszCopy = szComSpec;
			pwszCopy = PointToName(szComSpec);
#ifndef __GNUC__
#pragma warning( push )
#pragma warning(disable : 6400)
#endif

			if (lstrcmpiW(pwszCopy, L"ConEmuC")==0 || lstrcmpiW(pwszCopy, L"ConEmuC.exe")==0
			        /*|| lstrcmpiW(pwszCopy, L"ConEmuC64")==0 || lstrcmpiW(pwszCopy, L"ConEmuC64.exe")==0*/)
				szComSpec[0] = 0;

#ifndef __GNUC__
#pragma warning( pop )
#endif
		}

		// ComSpec/ComSpecC �� ���������, ���������� cmd.exe
		if (szComSpec[0] == 0)
		{
			if (!SearchPathW(NULL, L"cmd.exe", NULL, MAX_PATH, szComSpec, &psFilePart))
			{
				_printf("Can't find cmd.exe!\n");
				return CERR_CMDEXENOTFOUND;
			}
		}

		nCmdLine += lstrlenW(szComSpec)+15; // "/C", ������� � ��������� "/U"
	}

	size_t nCchLen = nCmdLine+1; // nCmdLine ��������� ������ asCmdLine + szComSpec + ��� ����-���� �� "/C" � ������
	gpszRunCmd = (wchar_t*)calloc(nCchLen,2);

	if (!gpszRunCmd)
	{
		_printf("Can't allocate %i wchars!\n", (DWORD)nCmdLine);
		return CERR_NOTENOUGHMEM1;
	}

	// ��� ����� ��� ����� ��������� �������. ��� ������������� COMSPEC ������ ����, ����� �����
	_wcscpy_c(gpszRunCmd, nCchLen, asCmdLine);
	// !!! gpszRunCmd ����� ���������� ����!

	// ������ ��������� �������
	if (*asCmdLine == L'"')
	{
		if (asCmdLine[1])
		{
			wchar_t *pszTitle = gpszRunCmd;
			wchar_t *pszEndQ = pszTitle + lstrlenW(pszTitle) - 1;

			if (pszEndQ > (pszTitle+1) && *pszEndQ == L'"'
			        && wcschr(pszTitle+1, L'"') == pszEndQ)
			{
				*pszEndQ = 0; pszTitle ++;
				bool lbCont = true;

				// "F:\Temp\1\ConsoleTest.exe ." - ������� �� �����, ����� ��������� ���� ���������
				if (lbCont && (*pszTitle != L'"') && ((*(pszEndQ-1) == L'.') ||(*(pszEndQ-1) == L' ')))
				{
					pwszCopy = pszTitle;
					wchar_t szTemp[MAX_PATH+1];

					if (NextArg(&pwszCopy, szTemp) == 0)
					{
						// � ���������� ���� � ����� (������������) �� ������ ���� ��������?
						if (!wcschr(szTemp, ' ') && IsFilePath(szTemp) && FileExists(szTemp))
						{
							lbCont = false;
							lbNeedCutStartEndQuot = TRUE;
						}
					}
				}

				// "C:\Program Files\FAR\far.exe" - ������� �����, ����� �� ����������
				if (lbCont)
				{
					if (IsFilePath(pszTitle) && FileExists(pszTitle))
					{
						lbCont = false;
						lbNeedCutStartEndQuot = FALSE;
					}

					//DWORD dwFileAttr = GetFileAttributes(pszTitle);
					//if (dwFileAttr != INVALID_FILE_ATTRIBUTES && !(dwFileAttr & FILE_ATTRIBUTE_DIRECTORY))
					//	lbNeedCutStartEndQuot = FALSE;
					//else
					//	lbNeedCutStartEndQuot = TRUE;
				}
			}
			else
			{
				pszEndQ = NULL;
			}

			int nLen = 4096; //GetWindowTextLength(ghConWnd); -- KIS2009 ������ "������� �������� ���������"...

			if (nLen > 0)
			{
				gpszPrevConTitle = (wchar_t*)calloc(nLen+1,2);

				if (gpszPrevConTitle)
				{
					if (!GetConsoleTitleW(gpszPrevConTitle, nLen+1))
					{
						free(gpszPrevConTitle); gpszPrevConTitle = NULL;
					}
				}
			}

			SetConsoleTitleW(pszTitle);

			if (pszEndQ) *pszEndQ = L'"';
		}
	}
	else if (*asCmdLine)
	{
		int nLen = 4096; //GetWindowTextLength(ghConWnd); -- KIS2009 ������ "������� �������� ���������"...

		if (nLen > 0)
		{
			gpszPrevConTitle = (wchar_t*)calloc(nLen+1,2);

			if (gpszPrevConTitle)
			{
				if (!GetConsoleTitleW(gpszPrevConTitle, nLen+1))
				{
					free(gpszPrevConTitle); gpszPrevConTitle = NULL;
				}
			}
		}

		SetConsoleTitleW(asCmdLine);
	}

	if (gbRunViaCmdExe)
	{
		CheckUnicodeMode();

		if (wcschr(szComSpec, L' '))
		{
			gpszRunCmd[0] = L'"';
			_wcscpy_c(gpszRunCmd+1, nCchLen-1, szComSpec);

			if (gnCmdUnicodeMode)
				_wcscat_c(gpszRunCmd, nCchLen, (gnCmdUnicodeMode == 2) ? L" /U" : L" /A");

			_wcscat_c(gpszRunCmd, nCchLen, gpSrv->bK ? L"\" /K " : L"\" /C ");
		}
		else
		{
			_wcscpy_c(gpszRunCmd, nCchLen, szComSpec);

			if (gnCmdUnicodeMode)
				_wcscat_c(gpszRunCmd, nCchLen, (gnCmdUnicodeMode == 2) ? L" /U" : L" /A");

			_wcscat_c(gpszRunCmd, nCchLen, gpSrv->bK ? L" /K " : L" /C ");
		}

		// �������� ����� ���������� �� ���, � �� �������� ��������������
		//BOOL lbNeedQuatete = FALSE;
		// ������� � cmd.exe ����� ���������� ���:
		// ""c:\program files\arc\7z.exe" -?"
		//int nLastChar = lstrlenW(asCmdLine) - 1;
		//if (asCmdLine[0] == L'"' && asCmdLine[nLastChar] == L'"') {
		//	// �������� ����� ���������� �� ���, � �� �������� ��������������
		//	if (gnRunMode == RM_COMSPEC)
		//		lbNeedQuatete = FALSE;
		//	//if (asCmdLine[1] == L'"' && asCmdLine[2])
		//	//	lbNeedQuatete = FALSE; // ���
		//	//else if (wcschr(asCmdLine+1, L'"') == (asCmdLine+nLastChar))
		//	//	lbNeedQuatete = FALSE; // �� ���������. ������ ������� ���
		//}
		//if (lbNeedQuatete) { // ����
		//	lstrcatW(gpszRunCmd, L"\"" );
		//}
		// ����������, ��������� ������
		_wcscat_c(gpszRunCmd, nCchLen, asCmdLine);
		//if (lbNeedQuatete)
		//	lstrcatW(gpszRunCmd, L"\"" );
	}
	else if (lbNeedCutStartEndQuot)
	{
		// ""c:\arc\7z.exe -?"" - �� ����������!
		_wcscpy_c(gpszRunCmd, nCchLen, asCmdLine+1);
		wchar_t *pszEndQ = gpszRunCmd + lstrlenW(gpszRunCmd) - 1;
		_ASSERTE(pszEndQ && *pszEndQ == L'"');

		if (pszEndQ && *pszEndQ == L'"') *pszEndQ = 0;
	}
	
	// ������ �������� � ���������� "-new_console" / "-cur_console"
	RConStartArgs args;
	args.pszSpecialCmd = gpszRunCmd;
	args.ProcessNewConArg();
	args.pszSpecialCmd = NULL;
	gbRunInBackgroundTab = args.bBackgroundTab;
	if (args.bBufHeight)
	{
		TODO("gcrBufferSize - � ������ ������");
		gnBufferHeight = args.nBufHeight;
		gbParmBufSize = TRUE;
	}

#ifdef _DEBUG
	OutputDebugString(gpszRunCmd); OutputDebugString(L"\n");
#endif
	return 0;
}

//int NextArg(LPCWSTR &asCmdLine, wchar_t* rsArg/*[MAX_PATH+1]*/)
//{
//    LPCWSTR psCmdLine = asCmdLine, pch = NULL;
//    wchar_t ch = *psCmdLine;
//    int nArgLen = 0;
//
//    while (ch == L' ' || ch == L'\t' || ch == L'\r' || ch == L'\n') ch = *(++psCmdLine);
//    if (ch == 0) return CERR_CMDLINEEMPTY;
//
//    // �������� ���������� � "
//    if (ch == L'"') {
//        psCmdLine++;
//        pch = wcschr(psCmdLine, L'"');
//        if (!pch) return CERR_CMDLINE;
//        while (pch[1] == L'"') {
//            pch += 2;
//            pch = wcschr(pch, L'"');
//            if (!pch) return CERR_CMDLINE;
//        }
//        // ������ � pch ������ �� ��������� "
//    } else {
//        // �� ����� ������ ��� �� ������� �������
//        //pch = wcschr(psCmdLine, L' ');
//        // 09.06.2009 Maks - ��������� ��: cmd /c" echo Y "
//        pch = psCmdLine;
//        while (*pch && *pch!=L' ' && *pch!=L'"') pch++;
//        //if (!pch) pch = psCmdLine + lstrlen(psCmdLine); // �� ����� ������
//    }
//
//    nArgLen = pch - psCmdLine;
//    if (nArgLen > MAX_PATH) return CERR_CMDLINE;
//
//    // ������� ��������
//    memcpy(rsArg, psCmdLine, nArgLen*sizeof(wchar_t));
//    rsArg[nArgLen] = 0;
//
//    psCmdLine = pch;
//
//    // Finalize
//    ch = *psCmdLine; // ����� ��������� �� ����������� �������
//    if (ch == L'"') ch = *(++psCmdLine);
//    while (ch == L' ' || ch == L'\t' || ch == L'\r' || ch == L'\n') ch = *(++psCmdLine);
//    asCmdLine = psCmdLine;
//
//    return 0;
//}

//void CorrectConsolePos()
//{
//	RECT rcNew = {};
//	if (GetWindowRect(ghConWnd, &rcNew))
//	{
//		HMONITOR hMon = MonitorFromWindow(ghConWnd, MONITOR_DEFAULTTOPRIMARY);
//		MONITORINFO mi = {sizeof(mi)};
//		int nMaxX = 0, nMaxY = 0;
//		if (GetMonitorInfo(hMon, &mi))
//		{
//			int newW = (rcNew.right-rcNew.left), newH = (rcNew.bottom-rcNew.top);
//			int newX = rcNew.left, newY = rcNew.top;
//
//			if (newX < mi.rcWork.left)
//				newX = mi.rcWork.left;
//			else if (rcNew.right > mi.rcWork.right)
//				newX = max(mi.rcWork.left,(mi.rcWork.right-newW));
//
//			if (newY < mi.rcWork.top)
//				newY = mi.rcWork.top;
//			else if (rcNew.bottom > mi.rcWork.bottom)
//				newY = max(mi.rcWork.top,(mi.rcWork.bottom-newH));
//
//			if ((newX != rcNew.left) || (newY != rcNew.top))
//				SetWindowPos(ghConWnd, HWND_TOP, newX, newY,0,0, SWP_NOSIZE);
//		}
//	}
//}
//
//void EmergencyShow()
//{
//	if (ghConWnd)
//	{
//		CorrectConsolePos();
//
//		if (!IsWindowVisible(ghConWnd))
//		{
//			SetWindowPos(ghConWnd, HWND_NOTOPMOST, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE);
//			//SetWindowPos(ghConWnd, HWND_TOP, 50,50,0,0, SWP_NOSIZE);
//			apiShowWindowAsync(ghConWnd, SW_SHOWNORMAL);
//		}
//		else
//		{
//			// ����� TOPMOST
//			SetWindowPos(ghConWnd, HWND_NOTOPMOST, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE);
//		}
//
//		if (!IsWindowEnabled(ghConWnd))
//			EnableWindow(ghConWnd, true);
//	}
//}

void ExitWaitForKey(WORD* pvkKeys, LPCWSTR asConfirm, BOOL abNewLine, BOOL abDontShowConsole)
{
	// ����� ������ ���� ��������� �����
	if (!abDontShowConsole)
	{
		BOOL lbNeedVisible = FALSE;

		if (!ghConWnd) ghConWnd = GetConEmuHWND(2);

		if (ghConWnd)  // ���� ������� ���� ������
		{
			WARNING("���� GUI ��� - �������� �� ������� SendMessageTimeout - ���������� ������� �� �����. �� ������� ����������");

			if (!IsWindowVisible(ghConWnd))
			{
				BOOL lbGuiAlive = FALSE;

				if (ghConEmuWnd && IsWindow(ghConEmuWnd))
				{
					DWORD_PTR dwLRc = 0;

					if (SendMessageTimeout(ghConEmuWnd, WM_NULL, 0, 0, SMTO_ABORTIFHUNG, 1000, &dwLRc))
						lbGuiAlive = TRUE;
				}

				if (!lbGuiAlive && !IsWindowVisible(ghConWnd))
				{
					lbNeedVisible = TRUE;
					// �� ���� ��������... // ��������� "�����������" 80x25, ��� ��, ��� ���� �������� � ���.������
					//SMALL_RECT rcNil = {0}; SetConsoleSize(0, gcrVisibleSize, rcNil, ":Exiting");
					//SetConsoleFontSizeTo(ghConWnd, 8, 12); // ��������� ����� ��������
					//apiShowWindow(ghConWnd, SW_SHOWNORMAL); // � ������� ������
					EmergencyShow(ghConWnd);
				}
			}
		}
	}

	// ������� ��������� �����
	INPUT_RECORD r = {0}; DWORD dwCount = 0;
	FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
	PRINT_COMSPEC(L"Finalizing. gbInShutdown=%i\n", gbInShutdown);

	if (gbInShutdown)
		return; // Event �������� ��� �������������

	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleMode(hOut, ENABLE_PROCESSED_OUTPUT|ENABLE_WRAP_AT_EOL_OUTPUT);

	//
	_wprintf(asConfirm);

	//if (lbNeedVisible)
	// ���� ������ ����� ���� ������ - ������ GUI ����� ����, � ����� ������������ �� �����
	//while (PeekConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &r, 1, &dwCount)) {
	//    if (dwCount)
	//        ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &r, 1, &dwCount);
	//    else
	//        Sleep(100);
	//    if (lbNeedVisible && !IsWindowVisible(ghConWnd)) {
	//        apiShowWindow(ghConWnd, SW_SHOWNORMAL); // � ������� ������
	//    }
	while(TRUE)
	{
		if (!PeekConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &r, 1, &dwCount))
			dwCount = 0;

		if (gnRunMode == RM_SERVER)
		{
			int nCount = gpSrv->nProcessCount;

			if (nCount > 1)
			{
				// ������ Peek, ��� ��� ������ �������
				//// ! ������� ���� ����������, ����������� �� �����. ������� ������� � �����!
				//WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &r, 1, &dwCount);
				break;
			}
		}

		if (gbInShutdown)
		{
			break;
		}

		if (dwCount)
		{
			if (ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &r, 1, &dwCount) && dwCount)
			{
				bool lbMatch = false;

				if (r.EventType == KEY_EVENT && r.Event.KeyEvent.bKeyDown)
				{
					if (pvkKeys)
					{
						for(int i = 0; !lbMatch && pvkKeys[i]; i++)
							lbMatch = (r.Event.KeyEvent.wVirtualKeyCode == pvkKeys[i]);
					}
					else
					{
						lbMatch = (r.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE);
					}
				}

				if (lbMatch)
					break;
			}
		}

		Sleep(50);
	}

	//MessageBox(0,L"Debug message...............1",L"ConEmuC",0);
	//int nCh = _getch();
	if (abNewLine)
		_printf("\n");
}





void SendStarted()
{
	WARNING("����������, ��� ������� ����� ������� ��� ������ �������. �����������");

	static bool bSent = false;

	if (bSent)
		return; // �������� ������ ���� ���

	//crNewSize = gpSrv->sbi.dwSize;
	//_ASSERTE(crNewSize.X>=MIN_CON_WIDTH && crNewSize.Y>=MIN_CON_HEIGHT);
	HWND hConWnd = GetConEmuHWND(2);

	if (!gnSelfPID)
	{
		_ASSERTE(gnSelfPID!=0);
		gnSelfPID = GetCurrentProcessId();
	}

	if (!hConWnd)
	{
		// ��� Detached �������. ������ ����� ������� ������ COMSPEC
		_ASSERTE(gnRunMode == RM_COMSPEC);
		gbNonGuiMode = TRUE; // �� �������� ExecuteGuiCmd ��� ������. ��� �� ���� �������
		return;
	}

	_ASSERTE(hConWnd == ghConWnd);
	ghConWnd = hConWnd;

	DWORD nServerPID = 0, nGuiPID = 0;

	// ��� ComSpec-� ����� ����� ���������, � ����-�� ������ � ���� �������...
	if (gnRunMode /*== RM_COMSPEC*/ > RM_SERVER)
	{
		MFileMapping<CESERVER_CONSOLE_MAPPING_HDR> ConsoleMap;
		ConsoleMap.InitName(CECONMAPNAME, (DWORD)hConWnd); //-V205
		const CESERVER_CONSOLE_MAPPING_HDR* pConsoleInfo = ConsoleMap.Open();

		//WCHAR sHeaderMapName[64];
		//StringCchPrintf(sHeaderMapName, countof(sHeaderMapName), CECONMAPNAME, (DWORD)hConWnd);
		//HANDLE hFileMapping = OpenFileMapping(FILE_MAP_READ/*|FILE_MAP_WRITE*/, FALSE, sHeaderMapName);
		//if (hFileMapping) {
		//	const CESERVER_CONSOLE_MAPPING_HDR* pConsoleInfo
		//		= (CESERVER_CONSOLE_MAPPING_HDR*)MapViewOfFile(hFileMapping, FILE_MAP_READ/*|FILE_MAP_WRITE*/,0,0,0);
		if (pConsoleInfo)
		{
			nServerPID = pConsoleInfo->nServerPID;
			nGuiPID = pConsoleInfo->nGuiPID;

			if (pConsoleInfo->cbSize >= sizeof(CESERVER_CONSOLE_MAPPING_HDR))
			{
				if (pConsoleInfo->nLogLevel)
					CreateLogSizeFile(pConsoleInfo->nLogLevel);
			}

			//UnmapViewOfFile(pConsoleInfo);
			ConsoleMap.CloseMap();
		}

		//	CloseHandle(hFileMapping);
		//}

		if (nServerPID == 0)
		{
			gbNonGuiMode = TRUE; // �� �������� ExecuteGuiCmd ��� ������. ��� �� ���� �������
			return; // ����� ComSpec, �� ������� ���, ��������������, � GUI ������ �������� �� �����
		}
	}

	CESERVER_REQ *pIn = NULL, *pOut = NULL;
	int nSize = sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_STARTSTOP);
	pIn = ExecuteNewCmd(CECMD_CMDSTARTSTOP, nSize);

	if (pIn)
	{
		if (!GetModuleFileName(NULL, pIn->StartStop.sModuleName, countof(pIn->StartStop.sModuleName)))
			pIn->StartStop.sModuleName[0] = 0;
		#ifdef _DEBUG
		LPCWSTR pszFileName = PointToName(pIn->StartStop.sModuleName);
		#endif

		// Cmd/Srv ����� �����
		switch (gnRunMode)
		{
		case RM_SERVER:
			pIn->StartStop.nStarted = sst_ServerStart; break;
		case RM_COMSPEC:
			pIn->StartStop.nStarted = sst_ComspecStart; break;
		default:
			pIn->StartStop.nStarted = sst_AppStart;
		}
		pIn->StartStop.hWnd = ghConWnd;
		pIn->StartStop.dwPID = gnSelfPID;
		#ifdef _WIN64
		pIn->StartStop.nImageBits = 64;
		#else
		pIn->StartStop.nImageBits = 32;
		#endif
		TODO("Ntvdm/DosBox -> 16");

		//pIn->StartStop.dwInputTID = (gnRunMode == RM_SERVER) ? gpSrv->dwInputThreadId : 0;
		if (gnRunMode == RM_SERVER)
			pIn->StartStop.bUserIsAdmin = IsUserAdmin();

		// ����� �������� 16��� ���������� ����� ������������ �������...
		gnImageSubsystem = 0;
		LPCWSTR pszTemp = gpszRunCmd;
		wchar_t lsRoot[MAX_PATH+1] = {0};

		if (gnRunMode == RM_SERVER && gpSrv->bDebuggerActive)
		{
			// "��������"
			gnImageSubsystem = 0x101;
			gbRootIsCmdExe = TRUE; // ����� ����� ��������
		}
		else if (/*!gpszRunCmd &&*/ gbAttachFromFar)
		{
			// ����� �� ���-�������
			gnImageSubsystem = 0x100;
		}
		else if (gpszRunCmd && ((0 == NextArg(&pszTemp, lsRoot))))
		{
			PRINT_COMSPEC(L"Starting: <%s>", lsRoot);

			MWow64Disable wow;
			if (!gbSkipWowChange) wow.Disable();
			
			DWORD nImageFileAttr = 0;
			if (!GetImageSubsystem(lsRoot, gnImageSubsystem, gnImageBits, nImageFileAttr))
				gnImageSubsystem = 0;

			PRINT_COMSPEC(L", Subsystem: <%i>\n", gnImageSubsystem);
			PRINT_COMSPEC(L"  Args: %s\n", pszTemp);
		}
		else
		{
			GetImageSubsystem(gnImageSubsystem, gnImageBits);
		}

		pIn->StartStop.nSubSystem = gnImageSubsystem;
		if ((gnImageSubsystem == IMAGE_SUBSYSTEM_DOS_EXECUTABLE) || (gbUseDosBox))
			pIn->StartStop.nImageBits = 16;
		else if (gnImageBits)
			pIn->StartStop.nImageBits = gnImageBits;

		pIn->StartStop.bRootIsCmdExe = gbRootIsCmdExe; //2009-09-14
		// �� MyGet..., � �� ����� ���������������...
		HANDLE hOut = NULL;

		if (gnRunMode == RM_SERVER)
			hOut = (HANDLE)ghConOut;
		else
			hOut = GetStdHandle(STD_OUTPUT_HANDLE);

		DWORD dwErr1 = 0;
		BOOL lbRc1 = GetConsoleScreenBufferInfo(hOut, &pIn->StartStop.sbi);

		if (!lbRc1) dwErr1 = GetLastError();

		pIn->StartStop.crMaxSize = GetLargestConsoleWindowSize(hOut);

		// ���� (��� ComSpec) ������ �������� "-cur_console:h<N>"
		if (gbParmBufSize)
		{
			pIn->StartStop.bForceBufferHeight = TRUE;
			TODO("gcrBufferSize - � ������ ������");
			pIn->StartStop.nForceBufferHeight = gnBufferHeight;
		}

		PRINT_COMSPEC(L"Starting %s mode (ExecuteGuiCmd started)\n",(RunMode==RM_SERVER) ? L"Server" : L"ComSpec");
		// CECMD_CMDSTARTSTOP
		if (nServerPID != gnSelfPID)
			pOut = ExecuteSrvCmd(nServerPID, pIn, ghConWnd);
		if (gnRunMode != RM_APPLICATION)
			pOut = ExecuteGuiCmd(ghConWnd, pIn, ghConWnd);

		// ����� ��� ������ �������� ����� �������� � �� ����� - ��� ��� ����������, ������
		// ��� ������� � ServerInit, � ComSpec - �� ��������
		//if (!pOut) {
		//	// ��� ������ ������� GUI ����� �� ������ ������� ��������� �����, �.�.
		//	// �� ����� �������� �� ����������� ����������� ����, � ��� ������� GUI �� �����
		//	// ������� ����� ����-���� ���������, ���� GUI ������� �������
		//	// (anEvent == EVENT_CONSOLE_START_APPLICATION && idObject == (LONG)mn_ConEmuC_PID)
		//	DWORD dwStart = GetTickCount(), dwDelta = 0;
		//	while (!gbInShutdown && dwDelta < GUIREADY_TIMEOUT) {
		//		Sleep(10);
		//		pOut = ExecuteGuiCmd(ghConWnd, pIn, ghConWnd);
		//		if (pOut) break;
		//		dwDelta = GetTickCount() - dwStart;
		//	}
		//	if (!pOut) {
		//		// �������� ��� ����������, ��� ������ ����� GUI ��������
		//		_ASSERTE(pOut != NULL);
		//	}
		//}
		PRINT_COMSPEC(L"Starting %s mode (ExecuteGuiCmd finished)\n",(RunMode==RM_SERVER) ? L"Server" : L"ComSpec");

		if (pOut)
		{
			bSent = true;
			BOOL  bAlreadyBufferHeight = pOut->StartStopRet.bWasBufferHeight;
			DWORD nGuiPID = pOut->StartStopRet.dwPID;
			ghConEmuWnd = pOut->StartStopRet.hWnd;
			ghConEmuWndDC = pOut->StartStopRet.hWndDC;
			if (gpSrv)
			{
				gpSrv->dwGuiPID = pOut->StartStopRet.dwPID;
				#ifdef _DEBUG
				DWORD dwPID; GetWindowThreadProcessId(ghConEmuWnd, &dwPID);
				_ASSERTE(ghConEmuWnd==NULL || dwPID==gpSrv->dwGuiPID);
				#endif
			}

			if (gnRunMode == RM_SERVER)
			{
				if (gpSrv)
				{
					gpSrv->bWasDetached = FALSE;
				}
				else
				{
					_ASSERTE(gpSrv!=NULL);
				}
			}

			UpdateConsoleMapHeader();

			gnServerPID = pOut->StartStopRet.dwSrvPID;

			AllowSetForegroundWindow(nGuiPID);
			TODO("gnBufferHeight->gcrBufferSize");
			TODO("gcrBufferSize - � ������ ������");
			gnBufferHeight  = (SHORT)pOut->StartStopRet.nBufferHeight;
			gbParmBufSize = TRUE;
			// gcrBufferSize ������������ � gcrVisibleSize
			gcrVisibleSize.X = (SHORT)pOut->StartStopRet.nWidth;
			gcrVisibleSize.Y = (SHORT)pOut->StartStopRet.nHeight;
			gbParmVisibleSize = TRUE;			

			if (gnRunMode == RM_SERVER)
			{
				// ���� ����� ��������� - ������������� �������� ���������
				if (gpSrv->bDebuggerActive && !gnBufferHeight) gnBufferHeight = 9999;

				SMALL_RECT rcNil = {0};
				SetConsoleSize(gnBufferHeight, gcrVisibleSize, rcNil, "::SendStarted");

				// ����� ��������� ����������
				if (pOut->StartStopRet.bNeedLangChange)
				{
#ifndef INPUTLANGCHANGE_SYSCHARSET
#define INPUTLANGCHANGE_SYSCHARSET 0x0001
#endif
					WPARAM wParam = INPUTLANGCHANGE_SYSCHARSET;
					TODO("��������� �� x64, �� ����� �� ������� � 0xFFFFFFFFFFFFFFFFFFFFF");
					LPARAM lParam = (LPARAM)(DWORD_PTR)pOut->StartStopRet.NewConsoleLang;
					SendMessage(ghConWnd, WM_INPUTLANGCHANGEREQUEST, wParam, lParam);
				}
			}
			else
			{
				// ����� ��� ����������, ��� ���� COMSPEC ������� �� �������.
				// 100628 - �����������. COMSPEC ������������ � cmd.exe
				//if (bAlreadyBufferHeight)
				//	gpSrv->bNonGuiMode = TRUE; // �� �������� ExecuteGuiCmd ��� ������ - ��������� ������ ��������
				gbWasBufferHeight = bAlreadyBufferHeight;
			}

			//nNewBufferHeight = ((DWORD*)(pOut->Data))[0];
			//crNewSize.X = (SHORT)((DWORD*)(pOut->Data))[1];
			//crNewSize.Y = (SHORT)((DWORD*)(pOut->Data))[2];
			TODO("���� �� ������� ��� COMSPEC - �� � GUI �������� ��������� ����� �� ������");
			//if (rNewWindow.Right >= crNewSize.X) // ������ ��� �������� �� ���� ������ ���������
			//    rNewWindow.Right = crNewSize.X-1;
			ExecuteFreeResult(pOut); pOut = NULL;
			//gnBufferHeight = nNewBufferHeight;
		}
		else
		{
			gbNonGuiMode = TRUE; // �� �������� ExecuteGuiCmd ��� ������. ��� �� ���� �������
		}

		ExecuteFreeResult(pIn); pIn = NULL;
	}
}

CESERVER_REQ* SendStopped(CONSOLE_SCREEN_BUFFER_INFO* psbi)
{
	CESERVER_REQ *pIn = NULL, *pOut = NULL;
	int nSize = sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_STARTSTOP);
	pIn = ExecuteNewCmd(CECMD_CMDSTARTSTOP,nSize);

	if (pIn)
	{
		// �� ����, sst_ServerStop �� ����������
		_ASSERTE(gnRunMode != RM_SERVER);
		switch (gnRunMode)
		{
		case RM_SERVER:
			pIn->StartStop.nStarted = sst_ServerStop; break;
		case RM_COMSPEC:
			pIn->StartStop.nStarted = sst_ComspecStop; break;
		default:
			pIn->StartStop.nStarted = sst_AppStop;
		}

		if (!GetModuleFileName(NULL, pIn->StartStop.sModuleName, countof(pIn->StartStop.sModuleName)))
			pIn->StartStop.sModuleName[0] = 0;

		pIn->StartStop.hWnd = ghConWnd;
		pIn->StartStop.dwPID = gnSelfPID;
		pIn->StartStop.nSubSystem = gnImageSubsystem;
		if ((gnImageSubsystem == IMAGE_SUBSYSTEM_DOS_EXECUTABLE) || (gbUseDosBox))
			pIn->StartStop.nImageBits = 16;
		else if (gnImageBits)
			pIn->StartStop.nImageBits = gnImageBits;
		else
		{
			#ifdef _WIN64
			pIn->StartStop.nImageBits = 64;
			#else
			pIn->StartStop.nImageBits = 32;
			#endif
		}
		pIn->StartStop.bWasBufferHeight = gbWasBufferHeight;

		if (psbi != NULL)
		{
			pIn->StartStop.sbi = *psbi;
		}
		else
		{
			// �� MyGet..., � �� ����� ���������������...
			// ghConOut ����� ���� NULL, ���� ������ ��������� �� ����� ������� ����������
			GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &pIn->StartStop.sbi);
		}

		pIn->StartStop.crMaxSize = GetLargestConsoleWindowSize(GetStdHandle(STD_OUTPUT_HANDLE));

		PRINT_COMSPEC(L"Finalizing comspec mode (ExecuteGuiCmd started)\n",0);
		if (gnRunMode == RM_APPLICATION)
		{
			if (gnServerPID != 0)
				pOut = ExecuteSrvCmd(gnServerPID, pIn, ghConWnd);
		}
		else
			pOut = ExecuteGuiCmd(ghConWnd, pIn, ghConWnd);
		PRINT_COMSPEC(L"Finalizing comspec mode (ExecuteGuiCmd finished)\n",0);

		ExecuteFreeResult(pIn); pIn = NULL;
	}

	return pOut;
}


WARNING("�������� LogInput(INPUT_RECORD* pRec) �� ��� ����� ������� 'ConEmuC-input-%i.log'");
void CreateLogSizeFile(int nLevel)
{
	if (ghLogSize) return;  // ���

	DWORD dwErr = 0;
	wchar_t szFile[MAX_PATH+64], *pszDot;

	if (!GetModuleFileName(NULL, szFile, MAX_PATH))
	{
		dwErr = GetLastError();
		_printf("GetModuleFileName failed! ErrCode=0x%08X\n", dwErr);
		return; // �� �������
	}

	if ((pszDot = wcsrchr(szFile, L'.')) == NULL)
	{
		_printf("wcsrchr failed!\n", 0, szFile); //-V576
		return; // ������
	}

	_wsprintf(pszDot, SKIPLEN(countof(szFile)-(pszDot-szFile)) L"-size-%i.log", gnSelfPID);
	ghLogSize = CreateFileW(szFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (ghLogSize == INVALID_HANDLE_VALUE)
	{
		ghLogSize = NULL;
		dwErr = GetLastError();
		_printf("Create console log file failed! ErrCode=0x%08X\n", dwErr, szFile); //-V576
		return;
	}

	int nCchLen = lstrlen(szFile)+1;
	wpszLogSizeFile = /*lstrdup(szFile);*/(wchar_t*)calloc(nCchLen,2);
	_wcscpy_c(wpszLogSizeFile, nCchLen, szFile);
	// OK, ��� �������
	LPCSTR pszCmdLine = GetCommandLineA();

	if (pszCmdLine)
	{
		WriteFile(ghLogSize, pszCmdLine, (DWORD)strlen(pszCmdLine), &dwErr, 0);
		WriteFile(ghLogSize, "\r\n", 2, &dwErr, 0);
	}

	LogSize(NULL, "Startup");
}

void LogString(LPCSTR asText)
{
	if (!ghLogSize) return;

	char szInfo[255]; szInfo[0] = 0;
	LPCSTR pszThread = "<unknown thread>";
	DWORD dwId = GetCurrentThreadId();

	if (dwId == gdwMainThreadId)
		pszThread = "MainThread";
	else if (gpSrv->CmdServer.IsPipeThread(dwId))
		pszThread = "ServerThread";
	else if (dwId == gpSrv->dwRefreshThread)
		pszThread = "RefreshThread";
	//#ifdef USE_WINEVENT_SRV
	//else if (dwId == gpSrv->dwWinEventThread)
	//	pszThread = "WinEventThread";
	//#endif
	else if (gpSrv->InputServer.IsPipeThread(dwId))
		pszThread = "InputPipeThread";
	else if (gpSrv->DataServer.IsPipeThread(dwId))
		pszThread = "DataPipeThread";

	SYSTEMTIME st; GetLocalTime(&st);
	_wsprintfA(szInfo, SKIPLEN(countof(szInfo)) "%i:%02i:%02i.%03i ",
	           st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	int nCur = lstrlenA(szInfo);
	lstrcpynA(szInfo+nCur, asText ? asText : "", 255-nCur-3);
	StringCchCatA(szInfo, countof(szInfo), "\r\n");
	DWORD dwLen = 0;
	WriteFile(ghLogSize, szInfo, (DWORD)strlen(szInfo), &dwLen, 0);
	FlushFileBuffers(ghLogSize);
}

void LogSize(COORD* pcrSize, LPCSTR pszLabel)
{
	if (!ghLogSize) return;

	CONSOLE_SCREEN_BUFFER_INFO lsbi = {{0,0}};
	// � �������� ��� �������� �������� ��������
	GetConsoleScreenBufferInfo(ghConOut ? ghConOut : GetStdHandle(STD_OUTPUT_HANDLE), &lsbi);
	char szInfo[192]; szInfo[0] = 0;
	LPCSTR pszThread = "<unknown thread>";
	DWORD dwId = GetCurrentThreadId();

	if (dwId == gdwMainThreadId)
		pszThread = "MainThread";
	else if (gpSrv->CmdServer.IsPipeThread(dwId))
		pszThread = "ServerThread";
	else if (dwId == gpSrv->dwRefreshThread)
		pszThread = "RefreshThread";
	//#ifdef USE_WINEVENT_SRV
	//else if (dwId == gpSrv->dwWinEventThread)
	//	pszThread = "WinEventThread";
	//#endif
	else if (gpSrv->InputServer.IsPipeThread(dwId))
		pszThread = "InputPipeThread";
	else if (gpSrv->DataServer.IsPipeThread(dwId))
		pszThread = "DataPipeThread";

	/*HDESK hDesk = GetThreadDesktop ( GetCurrentThreadId() );
	HDESK hInp = OpenInputDesktop ( 0, FALSE, GENERIC_READ );*/
	SYSTEMTIME st; GetLocalTime(&st);
	//char szMapSize[32]; szMapSize[0] = 0;
	//if (gpSrv->pConsoleMap->IsValid()) {
	//	StringCchPrintfA(szMapSize, countof(szMapSize), " CurMapSize={%ix%ix%i}",
	//		gpSrv->pConsoleMap->Ptr()->sbi.dwSize.X, gpSrv->pConsoleMap->Ptr()->sbi.dwSize.Y,
	//		gpSrv->pConsoleMap->Ptr()->sbi.srWindow.Bottom-gpSrv->pConsoleMap->Ptr()->sbi.srWindow.Top+1);
	//}

	if (pcrSize)
	{
		_wsprintfA(szInfo, SKIPLEN(countof(szInfo)) "%i:%02i:%02i.%03i CurSize={%ix%i} ChangeTo={%ix%i} %s %s\r\n",
		           st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
		           lsbi.dwSize.X, lsbi.dwSize.Y, pcrSize->X, pcrSize->Y, pszThread, (pszLabel ? pszLabel : ""));
	}
	else
	{
		_wsprintfA(szInfo, SKIPLEN(countof(szInfo)) "%i:%02i:%02i.%03i CurSize={%ix%i} %s %s\r\n",
		           st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
		           lsbi.dwSize.X, lsbi.dwSize.Y, pszThread, (pszLabel ? pszLabel : ""));
	}

	//if (hInp) CloseDesktop ( hInp );
	DWORD dwLen = 0;
	WriteFile(ghLogSize, szInfo, (DWORD)strlen(szInfo), &dwLen, 0);
	FlushFileBuffers(ghLogSize);
}


void ProcessCountChanged(BOOL abChanged, UINT anPrevCount, MSectionLock *pCS)
{
	// �������������, ���� ����� ��� �� �������
	MSectionLock CS;
	if (!pCS)
		CS.Lock(gpSrv->csProc);

	// ���� ���-�� ��������������� ��� "ExtendedConsole.dll"
	// �� ���������, � �� �������� �� ������� ��� ���������
	if (gpSrv && gpSrv->nExtConsolePID)
	{
		DWORD nExtPID = gpSrv->nExtConsolePID;
		bool bExist = false;
		for (UINT i = 0; i < gpSrv->nProcessCount; i++)
		{
			if (gpSrv->pnProcesses[i] == nExtPID)
			{
				bExist = true; break;
			}
		}
		if (!bExist)
		{
			if (gpSrv->hExtConsoleCommit)
			{
				CloseHandle(gpSrv->hExtConsoleCommit);
				gpSrv->hExtConsoleCommit = NULL;
			}
			gpSrv->nExtConsolePID = 0;
		}
	}

	if (abChanged)
	{
		BOOL lbFarExists = FALSE, lbTelnetExist = FALSE;

		if (gpSrv->nProcessCount > 1)
		{
			HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);

			if (hSnap != INVALID_HANDLE_VALUE)
			{
				PROCESSENTRY32 prc = {sizeof(PROCESSENTRY32)};

				if (Process32First(hSnap, &prc))
				{
					do
					{
						for (UINT i = 0; i < gpSrv->nProcessCount; i++)
						{
							if (prc.th32ProcessID != gnSelfPID
							        && prc.th32ProcessID == gpSrv->pnProcesses[i])
							{
								if (lstrcmpiW(prc.szExeFile, L"far.exe")==0)
								{
									lbFarExists = TRUE;
									//if (gpSrv->nProcessCount <= 2) // ����� ��������� � ntvdm
									//	break; // ��������, � ������� ��� ���� � telnet?
								}

								#ifndef WIN64
								else if (!gpSrv->nNtvdmPID && lstrcmpiW(prc.szExeFile, L"ntvdm.exe")==0)
								{
									gpSrv->nNtvdmPID = prc.th32ProcessID;
								}
								#endif

								// 23.04.2010 Maks - telnet ����� ����������, �.�. � ���� �������� � Ins � ��������
								else if (lstrcmpiW(prc.szExeFile, L"telnet.exe")==0)
								{
									lbTelnetExist = TRUE;
								}

								// �� ����� ������ Telnet ���� ����� ������ ��� �������!
								//2009-12-28 �����. ��� ������ ���� ����...
								//if (lstrcmpiW(prc.szExeFile, L"telnet.exe")==0) {
								//	// ����� ������ �����������
								//	ghConIn.Close(); ghConOut.Close();
								//	//gpSrv->bWinHookAllow = TRUE; // ��������� ��������� ������� ��� �������
								//	lbFarExists = TRUE; lbTelnetExist = TRUE; break;
								//}
							}
						}

						if (lbFarExists && lbTelnetExist
							#ifndef WIN64
						        && gpSrv->nNtvdmPID
							#endif
						    )
						{
							break; // ����� ������� ������ ������� ����
						}
					}
					while(Process32Next(hSnap, &prc));
				}

				CloseHandle(hSnap);
			}
		}

		gpSrv->bTelnetActive = lbTelnetExist;
		//if (gpSrv->nProcessCount >= 2
		//	&& ( (gpSrv->hWinHook == NULL && gpSrv->bWinHookAllow) || (gpSrv->hWinHook != NULL) )
		//    )
		//{
		//	if (lbFarExists) gpSrv->nWinHookMode = 2; else gpSrv->nWinHookMode = 1;
		//
		// 			if (lbFarExists && gpSrv->hWinHook == NULL && gpSrv->bWinHookAllow) {
		// 				HookWinEvents(2);
		// 			} else if (!lbFarExists && gpSrv->hWinHook) {
		// 				HookWinEvents(0);
		// 			}
		//}
	}

	gpSrv->dwProcessLastCheckTick = GetTickCount();

	// ���� �������� ������� ���������� ���������� (10 ���), ������ �� ����� � gbAlwaysConfirmExit ����� ��������
	// ���� gbAutoDisableConfirmExit==FALSE - ����� ������������� �������� ������� �� �����������
	if (gbAlwaysConfirmExit  // ���� ��� �� �������
	        && gbAutoDisableConfirmExit // ��������� �� ������ ����� �������� (10 ���)
	        && anPrevCount > 1 // ���� � ������� ��� ������������ ���������� �������
	        && gpSrv->hRootProcess) // � �������� ������� ��� ������ �������
	{
		if ((gpSrv->dwProcessLastCheckTick - gpSrv->nProcessStartTick) > CHECK_ROOTOK_TIMEOUT)
		{
			_ASSERTE(gnConfirmExitParm==0);
			// ��� �������� ����������� ���� ���
			gbAutoDisableConfirmExit = FALSE;
			// 10 ���. ������, ������ ���������� ���������, � ��� �� �������?
			DWORD dwProcWait = WaitForSingleObject(gpSrv->hRootProcess, 0);

			if (dwProcWait == WAIT_OBJECT_0)
			{
				// �������� ������� ��������, ��������, ���� �����-�� ��������?
				gbRootAliveLess10sec = TRUE; // �������� ������� ���������� ����� 10 ���
			}
			else
			{
				// �������� ������� ��� ��� ��������, ������� ��� ��� �� � ������������� �������� ������� �� �����������
				DisableAutoConfirmExit();
				//gpSrv->nProcessStartTick = GetTickCount() - 2*CHECK_ROOTSTART_TIMEOUT; // ������ nProcessStartTick �� �����. �������� ������ �� �������
			}
		}
	}

	if (gbRootWasFoundInCon == 0 && gpSrv->nProcessCount > 1 && gpSrv->hRootProcess && gpSrv->dwRootProcess)
	{
		if (WaitForSingleObject(gpSrv->hRootProcess, 0) == WAIT_OBJECT_0)
		{
			gbRootWasFoundInCon = 2; // � ������� ������� �� �����, � ��� ����������
		}
		else
		{
			for(UINT n = 0; n < gpSrv->nProcessCount; n++)
			{
				if (gpSrv->dwRootProcess == gpSrv->pnProcesses[n])
				{
					// ������� ����� � �������
					gbRootWasFoundInCon = 1; break;
				}
			}
		}
	}

	// ��������� � ������� �� ��������?
#ifndef WIN64

	if (gpSrv->nProcessCount == 2 && !gpSrv->bNtvdmActive && gpSrv->nNtvdmPID)
	{
		// �������� ���� �������� 16������ ����������, � ntvdm.exe �� ����������� ��� ��� ��������
		// gnSelfPID �� ����������� ����� � gpSrv->pnProcesses[0]
		if ((gpSrv->pnProcesses[0] == gnSelfPID && gpSrv->pnProcesses[1] == gpSrv->nNtvdmPID)
		        || (gpSrv->pnProcesses[1] == gnSelfPID && gpSrv->pnProcesses[0] == gpSrv->nNtvdmPID))
		{
			// ������� � ���� ������� ������� ��������
			PostMessage(ghConWnd, WM_CLOSE, 0, 0);
		}
	}

#endif
	WARNING("���� � ������� �� ����� ���� �������� - ��� ������� ���� 'gpSrv->nProcessCount == 1' ����������");

	// ������ - ����������� �� ����. ���������� ��������� ���������� - 5
	// cmd ������������ ����� (path not found)
	// ���������� ��������� �������� 5 � �� ���� �� ���� ������� �� ��������
	if (anPrevCount == 1 && gpSrv->nProcessCount == 1 && gpSrv->nProcessStartTick
		&& ((gpSrv->dwProcessLastCheckTick - gpSrv->nProcessStartTick) > CHECK_ROOTSTART_TIMEOUT)
		&& WaitForSingleObject(ghExitQueryEvent,0) == WAIT_TIMEOUT
		// �������� ����� ������ ���� �������� ������� ����������
		&& gpSrv->hRootProcess && WaitForSingleObject(gpSrv->hRootProcess,0) != WAIT_TIMEOUT)
	{
		anPrevCount = 2; // ����� ��������� ��������� �������
		//2010-03-06 - ��������� ������ ������ ���� ��� ������ �������
		//if (!gbAlwaysConfirmExit) gbAlwaysConfirmExit = TRUE; // ����� ������� �� �����������
	}

	if (anPrevCount > 1 && gpSrv->nProcessCount == 1)
	{
		if (gpSrv->pnProcesses[0] != gnSelfPID)
		{
			_ASSERTE(gpSrv->pnProcesses[0] == gnSelfPID);
		}
		else
		{
			// !!! �� ����� ������� �������� ���������� ������������
			// !!! ���������, ��� ConEmu ������������ �������, ��� �
			// !!! ������� �������� ���. �������� �������� �� nPrevProcessedDbg[]
			// !!! � ���������� �������. ������ ��� ��������� 2 ��������,
			// !!! � ������ ���� ������ ���� ������ 1?
			_ASSERTE(gbTerminateOnCtrlBreak==FALSE);
			// !!! ****


			if (pCS)
				pCS->Unlock();
			else
				CS.Unlock();

			//2010-03-06 ��� �� �����, �������� �������� �� �������
			//if (!gbAlwaysConfirmExit && (gpSrv->dwProcessLastCheckTick - gpSrv->nProcessStartTick) <= CHECK_ROOTSTART_TIMEOUT) {
			//	gbAlwaysConfirmExit = TRUE; // ����� ������� �� �����������
			//}
			if (gbAlwaysConfirmExit && (gpSrv->dwProcessLastCheckTick - gpSrv->nProcessStartTick) <= CHECK_ROOTSTART_TIMEOUT)
				gbRootAliveLess10sec = TRUE; // �������� ������� ���������� ����� 10 ���

			if (!nExitQueryPlace) nExitQueryPlace = 2+(nExitPlaceStep+nExitPlaceThread);

			SetEvent(ghExitQueryEvent);
		}
	}
}

BOOL ProcessAdd(DWORD nPID, MSectionLock *pCS /*= NULL*/)
{
	MSectionLock CS;
	if ((pCS == NULL) && (gpSrv->csProc != NULL))
	{
		pCS = &CS;
		CS.Lock(gpSrv->csProc);
	}
	
	UINT nPrevCount = gpSrv->nProcessCount;
	BOOL lbChanged = FALSE;
	_ASSERTE(nPID!=0);
	
	// �������� ������� � ������
	_ASSERTE(gpSrv->pnProcesses[0] == gnSelfPID);
	BOOL lbFound = FALSE;
	for (DWORD n = 0; n < nPrevCount; n++)
	{
		if (gpSrv->pnProcesses[n] == nPID)
		{
			lbFound = TRUE;
			break;
		}
	}
	if (!lbFound)
	{
		if (nPrevCount < gpSrv->nMaxProcesses)
		{
			pCS->RelockExclusive(200);
			gpSrv->pnProcesses[gpSrv->nProcessCount++] = nPID;
			lbChanged = TRUE;
		}
		else
		{
			_ASSERTE(nPrevCount < gpSrv->nMaxProcesses);
		}
	}
	
	return lbChanged;
}

BOOL CheckProcessCount(BOOL abForce/*=FALSE*/)
{
	//static DWORD dwLastCheckTick = GetTickCount();
	UINT nPrevCount = gpSrv->nProcessCount;
#ifdef _DEBUG
	DWORD nCurProcessesDbg[128]; // ��� �������, ��������� �������� ��������� �������
	DWORD nPrevProcessedDbg[128] = {}; // ��� �������, ��������� ���������� ��������� �������
	if (gpSrv->pnProcesses && gpSrv->nProcessCount)
		memmove(nPrevProcessedDbg, gpSrv->pnProcesses, min(countof(nPrevProcessedDbg),gpSrv->nProcessCount)*sizeof(*gpSrv->pnProcesses));
#endif

	if (gpSrv->nProcessCount <= 0)
	{
		abForce = TRUE;
	}

	if (!abForce)
	{
		DWORD dwCurTick = GetTickCount();

		if ((dwCurTick - gpSrv->dwProcessLastCheckTick) < (DWORD)CHECK_PROCESSES_TIMEOUT)
			return FALSE;
	}

	BOOL lbChanged = FALSE;
	MSectionLock CS; CS.Lock(gpSrv->csProc);

	if (gpSrv->nProcessCount == 0)
	{
		gpSrv->pnProcesses[0] = gnSelfPID;
		gpSrv->nProcessCount = 1;
	}

	if (gpSrv->bDebuggerActive)
	{
		//if (gpSrv->hRootProcess) {
		//	if (WaitForSingleObject(gpSrv->hRootProcess, 0) == WAIT_OBJECT_0) {
		//		gpSrv->nProcessCount = 1;
		//		return TRUE;
		//	}
		//}
		//gpSrv->pnProcesses[1] = gpSrv->dwRootProcess;
		//gpSrv->nProcessCount = 2;
		return FALSE;
	}

	if (!pfnGetConsoleProcessList || gpSrv->hRootProcessGui)
	{
		_ASSERTE(gpSrv->pnProcesses[0] == gnSelfPID);
		gpSrv->pnProcesses[0] = gnSelfPID;

		if (gpSrv->hRootProcess)
		{
			if (WaitForSingleObject(gpSrv->hRootProcess, 0) == WAIT_OBJECT_0)
			{
				gpSrv->pnProcesses[1] = 0;
				lbChanged = gpSrv->nProcessCount != 1;
				gpSrv->nProcessCount = 1;
			}
			else
			{
				gpSrv->pnProcesses[1] = gpSrv->dwRootProcess;
				lbChanged = gpSrv->nProcessCount != 2;
				_ASSERTE(nExitQueryPlace == 0);
				gpSrv->nProcessCount = 2;
			}
		}
	}
	else
	{
		WARNING("����������, ���-�� ������� ������ ����������");
		DWORD nCurCount = 0;
		nCurCount = pfnGetConsoleProcessList(gpSrv->pnProcessesGet, gpSrv->nMaxProcesses);
		#ifdef _DEBUG
		int nCurCountDbg = pfnGetConsoleProcessList(nCurProcessesDbg, countof(nCurProcessesDbg));
		#endif
		lbChanged = gpSrv->nProcessCount != nCurCount;

		if (nCurCount == 0)
		{
			_ASSERTE(gbTerminateOnCtrlBreak==FALSE);

			// ��� ������ � Win7 �������� conhost.exe
			#ifdef _DEBUG
			DWORD dwErr = GetLastError();
			#endif
			gpSrv->nProcessCount = 1;
			SetEvent(ghQuitEvent);

			if (!nExitQueryPlace) nExitQueryPlace = 1+(nExitPlaceStep+nExitPlaceThread);

			SetEvent(ghExitQueryEvent);
			return TRUE;
		}

		if (nCurCount > gpSrv->nMaxProcesses)
		{
			DWORD nSize = nCurCount + 100;
			DWORD* pnPID = (DWORD*)calloc(nSize, sizeof(DWORD));

			if (pnPID)
			{
				CS.RelockExclusive(200);
				nCurCount = pfnGetConsoleProcessList(pnPID, nSize);

				if (nCurCount > 0 && nCurCount <= nSize)
				{
					free(gpSrv->pnProcessesGet);
					gpSrv->pnProcessesGet = pnPID; pnPID = NULL;
					free(gpSrv->pnProcesses);
					gpSrv->pnProcesses = (DWORD*)calloc(nSize, sizeof(DWORD));
					_ASSERTE(nExitQueryPlace == 0 || nCurCount == 1);
					gpSrv->nProcessCount = nCurCount;
					gpSrv->nMaxProcesses = nSize;
				}

				if (pnPID)
					free(pnPID);
			}
		}
		
		// PID-� ��������� � ������� ����� ��������� ����������. ��� �� ���������� gnSelfPID �� ������ �����
		gpSrv->pnProcesses[0] = gnSelfPID;
		DWORD nCorrect = 1, nMax = gpSrv->nMaxProcesses, nDosBox = 0;
		if (gbUseDosBox)
		{
			if (ghDosBoxProcess && WaitForSingleObject(ghDosBoxProcess, 0) == WAIT_TIMEOUT)
			{
				gpSrv->pnProcesses[nCorrect++] = gnDosBoxPID;
				nDosBox = 1;
			}
			else if (ghDosBoxProcess)
			{
				ghDosBoxProcess = NULL;
			}
		}
		for (DWORD n = 0; n < nCurCount && nCorrect < nMax; n++)
		{
			if (gpSrv->pnProcessesGet[n] != gnSelfPID)
			{
				if (gpSrv->pnProcesses[nCorrect] != gpSrv->pnProcessesGet[n])
				{
					gpSrv->pnProcesses[nCorrect] = gpSrv->pnProcessesGet[n];
					lbChanged = TRUE;
				}
				nCorrect++;
			}
		}
		nCurCount += nDosBox;


		if (nCurCount < gpSrv->nMaxProcesses)
		{
			// �������� � 0 ������ �� ������� ����������
			_ASSERTE(gpSrv->nProcessCount < gpSrv->nMaxProcesses);

			if (nCurCount < gpSrv->nProcessCount)
			{
				UINT nSize = sizeof(DWORD)*(gpSrv->nProcessCount - nCurCount);
				memset(gpSrv->pnProcesses + nCurCount, 0, nSize);
			}

			_ASSERTE(nCurCount>0);
			_ASSERTE(nExitQueryPlace == 0 || nCurCount == 1);
			gpSrv->nProcessCount = nCurCount;
		}

		if (!lbChanged)
		{
			UINT nSize = sizeof(DWORD)*min(gpSrv->nMaxProcesses,START_MAX_PROCESSES);
#ifdef _DEBUG
			_ASSERTE(!IsBadWritePtr(gpSrv->pnProcessesCopy,nSize));
			_ASSERTE(!IsBadWritePtr(gpSrv->pnProcesses,nSize));
#endif
			lbChanged = memcmp(gpSrv->pnProcessesCopy, gpSrv->pnProcesses, nSize) != 0;
			MCHKHEAP;

			if (lbChanged)
				memmove(gpSrv->pnProcessesCopy, gpSrv->pnProcesses, nSize);

			MCHKHEAP;
		}

	}

	gpSrv->dwProcessLastCheckTick = GetTickCount();

	ProcessCountChanged(lbChanged, nPrevCount, &CS);


	return lbChanged;
}

int CALLBACK FontEnumProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, DWORD FontType, LPARAM lParam)
{
	if ((FontType & TRUETYPE_FONTTYPE) == TRUETYPE_FONTTYPE)
	{
		// OK, ��������
		wcscpy_c(gpSrv->szConsoleFont, lpelfe->elfLogFont.lfFaceName);
		return 0;
	}

	return TRUE; // ���� ��������� ����
}

DWORD WINAPI DebugThread(LPVOID lpvParam)
{
	if (!DebugActiveProcess(gpSrv->dwRootProcess))
	{
		DWORD dwErr = GetLastError();
		wchar_t szInfo[1024];
		wchar_t szProc[64]; szProc[0] = 0;
		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
		if (hSnap != INVALID_HANDLE_VALUE)
		{
			PROCESSENTRY32 pi = {sizeof(pi)};
			if (Process32First(hSnap, &pi))
			{
				while (pi.th32ProcessID != gpSrv->dwRootProcess)
				{
					if (!Process32Next(hSnap, &pi))
						break;
				}
				if (pi.th32ProcessID == gpSrv->dwRootProcess)
					_wcscpyn_c(szProc, countof(szProc), pi.szExeFile, countof(szProc));
			}
			CloseHandle(hSnap);
		}
		_wsprintf(szInfo, SKIPLEN(countof(szInfo)) L"Can't attach debugger to '%s' PID=%i. ErrCode=0x%08X\n",
			szProc[0] ? szProc : L"not found", gpSrv->dwRootProcess, dwErr);
		_wprintf(szInfo);
		return CERR_CANTSTARTDEBUGGER;
	}

	// �������������� �������������, ����� �������� �������� (��� �������) �� �������
	// � �������� "������������" ���������
	pfnDebugActiveProcessStop = (FDebugActiveProcessStop)GetProcAddress(GetModuleHandle(L"kernel32.dll"),"DebugActiveProcessStop");
	pfnDebugSetProcessKillOnExit = (FDebugSetProcessKillOnExit)GetProcAddress(GetModuleHandle(L"kernel32.dll"),"DebugSetProcessKillOnExit");

	if (pfnDebugSetProcessKillOnExit)
		pfnDebugSetProcessKillOnExit(FALSE/*KillOnExit*/);

	gpSrv->bDebuggerActive = TRUE;
	PrintDebugInfo();
	SetEvent(gpSrv->hDebugReady);
	DWORD nWait = WAIT_TIMEOUT;

	while(nWait == WAIT_TIMEOUT)
	{
		ProcessDebugEvent();

		if (ghExitQueryEvent)
			nWait = WaitForSingleObject(ghExitQueryEvent, 0);
	}

	gbRootAliveLess10sec = FALSE;
	gbInShutdown = TRUE;
	gbAlwaysConfirmExit = FALSE;

	_ASSERTE(gbTerminateOnCtrlBreak==FALSE);

	if (!nExitQueryPlace) nExitQueryPlace = 3+(nExitPlaceStep+nExitPlaceThread);

	SetEvent(ghExitQueryEvent);
	return 0;
}

void WriteMiniDump(DWORD dwThreadId, EXCEPTION_RECORD *pExceptionRecord, LPCSTR asConfirmText = NULL)
{
	MINIDUMP_TYPE dumpType = MiniDumpNormal;
	
	char szTitleA[64];
	_wsprintfA(szTitleA, SKIPLEN(countof(szTitleA)) "ConEmuC Debugging PID=%u, Debugger PID=%u", gpSrv->dwRootProcess, GetCurrentProcessId());
	wchar_t szTitle[64];
	_wsprintf(szTitle, SKIPLEN(countof(szTitle)) L"ConEmuC Debugging PID=%u, Debugger PID=%u", gpSrv->dwRootProcess, GetCurrentProcessId());

	int nBtn = 	MessageBoxA(NULL, asConfirmText ? asConfirmText : "Create minidump (<No> - fulldump)?", szTitleA, MB_YESNOCANCEL|MB_SYSTEMMODAL);
	switch (nBtn)
	{
	case IDYES:
		break;
	case IDNO:
		dumpType = MiniDumpWithFullMemory;
		break;
	default:
		return;
	}

	HANDLE hDmpFile = NULL;
	HMODULE hDbghelp = NULL;
	wchar_t szErrInfo[MAX_PATH*2];
	wchar_t dmpfile[MAX_PATH]; dmpfile[0] = 0;
	HMODULE hCOMDLG32 = NULL;
	typedef BOOL (WINAPI* GetSaveFileName_t)(LPOPENFILENAMEW lpofn);
	GetSaveFileName_t _GetSaveFileName = NULL;
	typedef BOOL (WINAPI* MiniDumpWriteDump_t)(HANDLE hProcess, DWORD ProcessId, HANDLE hFile, MINIDUMP_TYPE DumpType,
	        PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	        PMINIDUMP_CALLBACK_INFORMATION CallbackParam);
	MiniDumpWriteDump_t MiniDumpWriteDump_f = NULL;
	
	if (!hCOMDLG32)
		hCOMDLG32 = LoadLibraryW(L"COMDLG32.dll");
	if (hCOMDLG32 && !_GetSaveFileName)
		_GetSaveFileName = (GetSaveFileName_t)GetProcAddress(hCOMDLG32, "GetSaveFileNameW");

	while (_GetSaveFileName)
	{
		OPENFILENAMEW ofn; memset(&ofn,0,sizeof(ofn));
		ofn.lStructSize=sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFilter = L"Debug dumps (*.mdmp)\0*.mdmp\0\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFile = dmpfile;
		ofn.nMaxFile = countof(dmpfile);
		ofn.lpstrTitle = L"Save debug dump";
		ofn.lpstrDefExt = L"mdmp";
		ofn.Flags = OFN_ENABLESIZING|OFN_NOCHANGEDIR
		            | OFN_PATHMUSTEXIST|OFN_EXPLORER|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;

		if (!_GetSaveFileName(&ofn))
			break;

		if (hDmpFile != INVALID_HANDLE_VALUE && hDmpFile != NULL)
		{
			CloseHandle(hDmpFile); hDmpFile = NULL;
		}
		
		hDmpFile = CreateFileW(dmpfile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH, NULL);

		if (hDmpFile == INVALID_HANDLE_VALUE)
		{
			DWORD nErr = GetLastError();
			_wsprintf(szErrInfo, SKIPLEN(countof(szErrInfo)) L"Can't create debug dump file\n%s\nErrCode=0x%08X\n\nChoose another name?", dmpfile, nErr);

			if (MessageBoxW(NULL, szErrInfo, szTitle, MB_YESNO|MB_SYSTEMMODAL|MB_ICONSTOP)!=IDYES)
				break;

			continue; // ��� ��� �������
		}

		if (!hDbghelp)
		{
			hDbghelp = LoadLibraryW(L"Dbghelp.dll");

			if (hDbghelp == NULL)
			{
				DWORD nErr = GetLastError();
				_wsprintf(szErrInfo, SKIPLEN(countof(szErrInfo)) L"Can't load debug library 'Dbghelp.dll'\nErrCode=0x%08X\n\nTry again?", nErr);

				if (MessageBoxW(NULL, szErrInfo, szTitle, MB_YESNO|MB_SYSTEMMODAL|MB_ICONSTOP)!=IDYES)
					break;

				continue; // ��� ��� �������
			}
		}

		if (!MiniDumpWriteDump_f)
		{
			MiniDumpWriteDump_f = (MiniDumpWriteDump_t)GetProcAddress(hDbghelp, "MiniDumpWriteDump");

			if (!MiniDumpWriteDump_f)
			{
				DWORD nErr = GetLastError();
				_wsprintf(szErrInfo, SKIPLEN(countof(szErrInfo)) L"Can't locate 'MiniDumpWriteDump' in library 'Dbghelp.dll', ErrCode=%u", nErr);
				MessageBoxW(NULL, szErrInfo, szTitle, MB_ICONSTOP|MB_SYSTEMMODAL);
				break;
			}
		}

		if (MiniDumpWriteDump_f)
		{
			MINIDUMP_EXCEPTION_INFORMATION mei = {dwThreadId};
			EXCEPTION_POINTERS ep = {pExceptionRecord};
			ep.ContextRecord = NULL; // ���������, ������ ��� ����� �����
			mei.ExceptionPointers = &ep;
			mei.ClientPointers = FALSE;
			PMINIDUMP_EXCEPTION_INFORMATION pmei = NULL; // ����
			_printf("Creating minidump: ");
			_wprintf(dmpfile);
			_printf("...");
			BOOL lbDumpRc = MiniDumpWriteDump_f(
			                    gpSrv->hRootProcess, gpSrv->dwRootProcess,
			                    hDmpFile,
			                    dumpType,
			                    pmei,
			                    NULL, NULL);

			if (!lbDumpRc)
			{
				DWORD nErr = GetLastError();
				_wsprintf(szErrInfo, SKIPLEN(countof(szErrInfo)) L"MiniDumpWriteDump failed.\nErrorCode=0x%08X", nErr);
				_printf("\nFailed, ErrorCode=0x%08X\n", nErr);
				MessageBoxW(NULL, szErrInfo, szTitle, MB_ICONSTOP|MB_SYSTEMMODAL);
			}
			else
			{
				_printf("\nMiniDumpWriteDump succeeded\n");
			}

			break;
		}

	}

	if (hDmpFile != INVALID_HANDLE_VALUE && hDmpFile != NULL)
	{
		CloseHandle(hDmpFile);
	}

	if (hDbghelp)
	{
		FreeLibrary(hDbghelp);
	}

	if (hCOMDLG32)
	{
		FreeLibrary(hCOMDLG32);
	}
}

void ProcessDebugEvent()
{
	static wchar_t wszDbgText[1024];
	static char szDbgText[1024];
	BOOL lbNonContinuable = FALSE;
	DEBUG_EVENT evt = {0};
	BOOL lbEvent = WaitForDebugEvent(&evt,10);
	#ifdef _DEBUG
	DWORD dwErr = GetLastError();
	#endif
	static bool bFirstExitThreadEvent = false; // ����� ������� �� ����� ��������� �� ������������ "���������"
	//HMODULE hCOMDLG32 = NULL;
	//typedef BOOL (WINAPI* GetSaveFileName_t)(LPOPENFILENAMEW lpofn);
	//GetSaveFileName_t _GetSaveFileName = NULL;

	if (lbEvent)
	{
		lbNonContinuable = FALSE;

		switch(evt.dwDebugEventCode)
		{
			case CREATE_PROCESS_DEBUG_EVENT:
			case CREATE_THREAD_DEBUG_EVENT:
			case EXIT_PROCESS_DEBUG_EVENT:
			case EXIT_THREAD_DEBUG_EVENT:
			case RIP_EVENT:
			{
				LPCSTR pszName = "Unknown";

				switch(evt.dwDebugEventCode)
				{
					case CREATE_PROCESS_DEBUG_EVENT: pszName = "CREATE_PROCESS_DEBUG_EVENT"; break;
					case CREATE_THREAD_DEBUG_EVENT: pszName = "CREATE_THREAD_DEBUG_EVENT"; break;
					case EXIT_PROCESS_DEBUG_EVENT: pszName = "EXIT_PROCESS_DEBUG_EVENT"; break;
					case EXIT_THREAD_DEBUG_EVENT: pszName = "EXIT_THREAD_DEBUG_EVENT"; break;
					case RIP_EVENT: pszName = "RIP_EVENT"; break;
				}

				_wsprintfA(szDbgText, SKIPLEN(countof(szDbgText)) "{%i.%i} %s\n", evt.dwProcessId,evt.dwThreadId, pszName);
				_printf(szDbgText);
				if (!bFirstExitThreadEvent && evt.dwDebugEventCode == EXIT_THREAD_DEBUG_EVENT)
				{
					bFirstExitThreadEvent = true;
					_printf("ConEmuC: Press Ctrl-Break to create minidump of debugging process\n");
				}
				break;
			}
			case LOAD_DLL_DEBUG_EVENT:
			{
				LPCSTR pszName = "Unknown";

				switch(evt.dwDebugEventCode)
				{
					case LOAD_DLL_DEBUG_EVENT: pszName = "LOAD_DLL_DEBUG_EVENT";

						if (evt.u.LoadDll.hFile)
						{
							//BY_HANDLE_FILE_INFORMATION inf = {0};
							//if (GetFileInformationByHandle(evt.LoadDll.hFile,
						}

						break;
						//6 Reports a load-dynamic-link-library (DLL) debugging event. The value of u.LoadDll specifies a LOAD_DLL_DEBUG_INFO structure.
					case UNLOAD_DLL_DEBUG_EVENT: pszName = "UNLOAD_DLL_DEBUG_EVENT"; break;
						//7 Reports an unload-DLL debugging event. The value of u.UnloadDll specifies an UNLOAD_DLL_DEBUG_INFO structure.
				}

				_wsprintfA(szDbgText, SKIPLEN(countof(szDbgText)) "{%i.%i} %s\n", evt.dwProcessId,evt.dwThreadId, pszName);
				_printf(szDbgText);
				break;
			}
			case EXCEPTION_DEBUG_EVENT:
				//1 Reports an exception debugging event. The value of u.Exception specifies an EXCEPTION_DEBUG_INFO structure.
			{
				lbNonContinuable = (evt.u.Exception.ExceptionRecord.ExceptionFlags&EXCEPTION_NONCONTINUABLE)==EXCEPTION_NONCONTINUABLE;

				//static bool bAttachEventRecieved = false;
				//if (!bAttachEventRecieved)
				//{
				//	bAttachEventRecieved = true;
				//	StringCchPrintfA(szDbgText, countof(szDbgText),"{%i.%i} Debugger attached successfully. (0x%08X address 0x%08X flags 0x%08X%s)\n",
				//		evt.dwProcessId,evt.dwThreadId,
				//		evt.u.Exception.ExceptionRecord.ExceptionCode,
				//		evt.u.Exception.ExceptionRecord.ExceptionAddress,
				//		evt.u.Exception.ExceptionRecord.ExceptionFlags,
				//		(evt.u.Exception.ExceptionRecord.ExceptionFlags&EXCEPTION_NONCONTINUABLE) ? "(EXCEPTION_NONCONTINUABLE)" : "");
				//}
				//else
				switch(evt.u.Exception.ExceptionRecord.ExceptionCode)
				{
					case EXCEPTION_ACCESS_VIOLATION: // The thread tried to read from or write to a virtual address for which it does not have the appropriate access.
					{
						if (evt.u.Exception.ExceptionRecord.NumberParameters>=2)
						{
							_wsprintfA(szDbgText, SKIPLEN(countof(szDbgText))
							           "{%i.%i} EXCEPTION_ACCESS_VIOLATION at " WIN3264TEST("0x%08X","0x%08X%08X") " flags 0x%08X%s %s of " WIN3264TEST("0x%08X","0x%08X%08X") " FC=%u\n", evt.dwProcessId,evt.dwThreadId,
							           WIN3264WSPRINT((DWORD_PTR)evt.u.Exception.ExceptionRecord.ExceptionAddress),
							           evt.u.Exception.ExceptionRecord.ExceptionFlags,
							           ((evt.u.Exception.ExceptionRecord.ExceptionFlags&EXCEPTION_NONCONTINUABLE) ? "(EXCEPTION_NONCONTINUABLE)" : ""),
							           ((evt.u.Exception.ExceptionRecord.ExceptionInformation[0]==0) ? "Read" :
							            (evt.u.Exception.ExceptionRecord.ExceptionInformation[0]==1) ? "Write" :
							            (evt.u.Exception.ExceptionRecord.ExceptionInformation[0]==8) ? "DEP" : "???"),
							           WIN3264WSPRINT(evt.u.Exception.ExceptionRecord.ExceptionInformation[1]),
							           evt.u.Exception.dwFirstChance
							          );
						}
						else
						{
							_wsprintfA(szDbgText, SKIPLEN(countof(szDbgText))
							           "{%i.%i} EXCEPTION_ACCESS_VIOLATION at " WIN3264TEST("0x%08X","0x%08X%08X") " flags 0x%08X%s FC=%u\n", evt.dwProcessId,evt.dwThreadId,
							           WIN3264WSPRINT((DWORD_PTR)evt.u.Exception.ExceptionRecord.ExceptionAddress),
							           evt.u.Exception.ExceptionRecord.ExceptionFlags,
							           (evt.u.Exception.ExceptionRecord.ExceptionFlags&EXCEPTION_NONCONTINUABLE) ? "(EXCEPTION_NONCONTINUABLE)" : "",
							           evt.u.Exception.dwFirstChance);
						}

						_printf(szDbgText);
					}
					break;
					default:
					{
						char szName[32]; LPCSTR pszName; pszName = szName;
#define EXCASE(s) case s: pszName = #s; break

							switch(evt.u.Exception.ExceptionRecord.ExceptionCode)
							{
									EXCASE(EXCEPTION_ARRAY_BOUNDS_EXCEEDED); // The thread tried to access an array element that is out of bounds and the underlying hardware supports bounds checking.
									EXCASE(EXCEPTION_BREAKPOINT); // A breakpoint was encountered.
									EXCASE(EXCEPTION_DATATYPE_MISALIGNMENT); // The thread tried to read or write data that is misaligned on hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte boundaries; 32-bit values on 4-byte boundaries, and so on.
									EXCASE(EXCEPTION_FLT_DENORMAL_OPERAND); // One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value.
									EXCASE(EXCEPTION_FLT_DIVIDE_BY_ZERO); // The thread tried to divide a floating-point value by a floating-point divisor of zero.
									EXCASE(EXCEPTION_FLT_INEXACT_RESULT); // The result of a floating-point operation cannot be represented exactly as a decimal fraction.
									EXCASE(EXCEPTION_FLT_INVALID_OPERATION); // This exception represents any floating-point exception not included in this list.
									EXCASE(EXCEPTION_FLT_OVERFLOW); // The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type.
									EXCASE(EXCEPTION_FLT_STACK_CHECK); // The stack overflowed or underflowed as the result of a floating-point operation.
									EXCASE(EXCEPTION_FLT_UNDERFLOW); // The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type.
									EXCASE(EXCEPTION_ILLEGAL_INSTRUCTION); // The thread tried to execute an invalid instruction.
									EXCASE(EXCEPTION_IN_PAGE_ERROR); // The thread tried to access a page that was not present, and the system was unable to load the page. For example, this exception might occur if a network connection is lost while running a program over the network.
									EXCASE(EXCEPTION_INT_DIVIDE_BY_ZERO); // The thread tried to divide an integer value by an integer divisor of zero.
									EXCASE(EXCEPTION_INT_OVERFLOW); // The result of an integer operation caused a carry out of the most significant bit of the result.
									EXCASE(EXCEPTION_INVALID_DISPOSITION); // An exception handler returned an invalid disposition to the exception dispatcher. Programmers using a high-level language such as C should never encounter this exception.
									EXCASE(EXCEPTION_NONCONTINUABLE_EXCEPTION); // The thread tried to continue execution after a noncontinuable exception occurred.
									EXCASE(EXCEPTION_PRIV_INSTRUCTION); // The thread tried to execute an instruction whose operation is not allowed in the current machine mode.
									EXCASE(EXCEPTION_SINGLE_STEP); // A trace trap or other single-instruction mechanism signaled that one instruction has been executed.
									EXCASE(EXCEPTION_STACK_OVERFLOW); // The thread used up its stack.
								default:
									_wsprintfA(szName, SKIPLEN(countof(szName))
									           "Exception 0x%08X", evt.u.Exception.ExceptionRecord.ExceptionCode);
							}

							_wsprintfA(szDbgText, SKIPLEN(countof(szDbgText))
							           "{%i.%i} %s at " WIN3264TEST("0x%08X","0x%08X%08X") " flags 0x%08X%s FC=%u\n",
							           evt.dwProcessId,evt.dwThreadId,
							           pszName,
							           WIN3264WSPRINT((DWORD_PTR)evt.u.Exception.ExceptionRecord.ExceptionAddress),
							           evt.u.Exception.ExceptionRecord.ExceptionFlags,
							           (evt.u.Exception.ExceptionRecord.ExceptionFlags&EXCEPTION_NONCONTINUABLE)
							           ? "(EXCEPTION_NONCONTINUABLE)" : "",
							           evt.u.Exception.dwFirstChance);
							_printf(szDbgText);
						}
				}

				if (gpSrv->bDebuggerRequestDump ||
					(!lbNonContinuable && (evt.u.Exception.ExceptionRecord.ExceptionCode != EXCEPTION_BREAKPOINT)))
				{
					gpSrv->bDebuggerRequestDump = FALSE; // ���� ���

					char szConfirm[2048];
					_wsprintfA(szConfirm, SKIPLEN(countof(szConfirm)) "Non continuable exception (FC=%u)\n", evt.u.Exception.dwFirstChance);
					StringCchCatA(szConfirm, countof(szConfirm), szDbgText);
					StringCchCatA(szConfirm, countof(szConfirm), "\nCreate minidump (<No> - fulldump)?");
					//typedef BOOL (WINAPI* MiniDumpWriteDump_t)(HANDLE hProcess, DWORD ProcessId, HANDLE hFile, MINIDUMP_TYPE DumpType,
					//        PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
					//        PMINIDUMP_CALLBACK_INFORMATION CallbackParam);
					//MiniDumpWriteDump_t MiniDumpWriteDump_f = NULL;

					//if (MessageBoxA(NULL, szConfirm, "ConEmuC Debuger", MB_YESNO|MB_SYSTEMMODAL) == IDYES)
					{
						WriteMiniDump(evt.dwThreadId, &evt.u.Exception.ExceptionRecord, szConfirm);

						//TODO("���� ����� ������� ����, ������� HANDLE ��� hDumpFile, ������� MiniDumpWriteDump");
						//HANDLE hDmpFile = NULL;
						//HMODULE hDbghelp = NULL;
						//wchar_t szErrInfo[MAX_PATH*2];
						//wchar_t dmpfile[MAX_PATH]; dmpfile[0] = 0;
						//
						//if (!hCOMDLG32)
						//	hCOMDLG32 = LoadLibraryW(L"COMDLG32.dll");
						//if (hCOMDLG32 && !_GetSaveFileName)
						//	_GetSaveFileName = (GetSaveFileName_t)GetProcAddress(hCOMDLG32, "GetSaveFileNameW");

						//while (_GetSaveFileName)
						//{
						//	OPENFILENAMEW ofn; memset(&ofn,0,sizeof(ofn));
						//	ofn.lStructSize=sizeof(ofn);
						//	ofn.hwndOwner = NULL;
						//	ofn.lpstrFilter = L"Debug dumps (*.mdmp)\0*.mdmp\0\0";
						//	ofn.nFilterIndex = 1;
						//	ofn.lpstrFile = dmpfile;
						//	ofn.nMaxFile = countof(dmpfile);
						//	ofn.lpstrTitle = L"Save debug dump";
						//	ofn.lpstrDefExt = L"mdmp";
						//	ofn.Flags = OFN_ENABLESIZING|OFN_NOCHANGEDIR
						//	            | OFN_PATHMUSTEXIST|OFN_EXPLORER|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;

						//	if (!_GetSaveFileName(&ofn))
						//		break;

						//	hDmpFile = CreateFileW(dmpfile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH, NULL);

						//	if (hDmpFile == INVALID_HANDLE_VALUE)
						//	{
						//		DWORD nErr = GetLastError();
						//		_wsprintf(szErrInfo, SKIPLEN(countof(szErrInfo)) L"Can't create debug dump file\n%s\nErrCode=0x%08X\n\nChoose another name?", dmpfile, nErr);

						//		if (MessageBoxW(NULL, szErrInfo, L"ConEmuC Debuger", MB_YESNO|MB_SYSTEMMODAL|MB_ICONSTOP)!=IDYES)
						//			break;

						//		continue; // ��� ��� �������
						//	}

						//	if (!hDbghelp)
						//	{
						//		hDbghelp = LoadLibraryW(L"Dbghelp.dll");

						//		if (hDbghelp == NULL)
						//		{
						//			DWORD nErr = GetLastError();
						//			_wsprintf(szErrInfo, SKIPLEN(countof(szErrInfo)) L"Can't load debug library 'Dbghelp.dll'\nErrCode=0x%08X\n\nTry again?", nErr);

						//			if (MessageBoxW(NULL, szErrInfo, L"ConEmuC Debuger", MB_YESNO|MB_SYSTEMMODAL|MB_ICONSTOP)!=IDYES)
						//				break;

						//			continue; // ��� ��� �������
						//		}
						//	}

						//	if (!MiniDumpWriteDump_f)
						//	{
						//		MiniDumpWriteDump_f = (MiniDumpWriteDump_t)GetProcAddress(hDbghelp, "MiniDumpWriteDump");

						//		if (!MiniDumpWriteDump_f)
						//		{
						//			DWORD nErr = GetLastError();
						//			_wsprintf(szErrInfo, SKIPLEN(countof(szErrInfo)) L"Can't locate 'MiniDumpWriteDump' in library 'Dbghelp.dll'", nErr);
						//			MessageBoxW(NULL, szErrInfo, L"ConEmuC Debuger", MB_ICONSTOP|MB_SYSTEMMODAL);
						//			break;
						//		}
						//	}

						//	if (MiniDumpWriteDump_f)
						//	{
						//		MINIDUMP_EXCEPTION_INFORMATION mei = {evt.dwThreadId};
						//		EXCEPTION_POINTERS ep = {&evt.u.Exception.ExceptionRecord};
						//		ep.ContextRecord = NULL; // ���������, ������ ��� ����� �����
						//		mei.ExceptionPointers = &ep;
						//		mei.ClientPointers = FALSE;
						//		PMINIDUMP_EXCEPTION_INFORMATION pmei = NULL; // ����
						//		BOOL lbDumpRc = MiniDumpWriteDump_f(
						//		                    gpSrv->hRootProcess, gpSrv->dwRootProcess,
						//		                    hDmpFile,
						//		                    MiniDumpNormal /*MiniDumpWithDataSegs*/,
						//		                    pmei,
						//		                    NULL, NULL);

						//		if (!lbDumpRc)
						//		{
						//			DWORD nErr = GetLastError();
						//			_wsprintf(szErrInfo, SKIPLEN(countof(szErrInfo)) L"MiniDumpWriteDump failed.\nErrorCode=0x%08X", nErr);
						//			MessageBoxW(NULL, szErrInfo, L"ConEmuC Debuger", MB_ICONSTOP|MB_SYSTEMMODAL);
						//		}

						//		break;
						//	}
						//}

						//if (hDmpFile != INVALID_HANDLE_VALUE && hDmpFile != NULL)
						//{
						//	CloseHandle(hDmpFile);
						//}

						//if (hDbghelp)
						//{
						//	FreeLibrary(hDbghelp);
						//}
					}
				}
			}
			break;
			case OUTPUT_DEBUG_STRING_EVENT:
				//8 Reports an output-debugging-string debugging event. The value of u.DebugString specifies an OUTPUT_DEBUG_STRING_INFO structure.
			{
				wszDbgText[0] = 0;

				if (evt.u.DebugString.nDebugStringLength >= 1024) evt.u.DebugString.nDebugStringLength = 1023;

				DWORD_PTR nRead = 0;

				if (evt.u.DebugString.fUnicode)
				{
					if (!ReadProcessMemory(gpSrv->hRootProcess, evt.u.DebugString.lpDebugStringData, wszDbgText, 2*evt.u.DebugString.nDebugStringLength, &nRead))
						wcscpy_c(wszDbgText, L"???");
					else
						wszDbgText[min(1023,nRead+1)] = 0;
				}
				else
				{
					if (!ReadProcessMemory(gpSrv->hRootProcess, evt.u.DebugString.lpDebugStringData, szDbgText, evt.u.DebugString.nDebugStringLength, &nRead))
					{
						wcscpy_c(wszDbgText, L"???");
					}
					else
					{
						szDbgText[min(1023,nRead+1)] = 0;
						MultiByteToWideChar(CP_ACP, 0, szDbgText, -1, wszDbgText, 1024);
					}
				}

				WideCharToMultiByte(CP_OEMCP, 0, wszDbgText, -1, szDbgText, 1024, 0, 0);
#ifdef CRTPRINTF
				_printf("{PID=%i.TID=%i} ", evt.dwProcessId,evt.dwThreadId, wszDbgText);
#else
				_printf("{PID=%i.TID=%i} %s", evt.dwProcessId,evt.dwThreadId, szDbgText);
				int nLen = lstrlenA(szDbgText);

				if (nLen > 0 && szDbgText[nLen-1] != '\n')
					_printf("\n");

#endif
			}
			break;
		}

		// ���������� ������������ �������
		ContinueDebugEvent(evt.dwProcessId, evt.dwThreadId, DBG_EXCEPTION_NOT_HANDLED);
	}
	
	//if (hCOMDLG32)
	//	FreeLibrary(hCOMDLG32);
}




//DWORD WINAPI ServerThread(LPVOID lpvParam)
//{
//	BOOL fConnected = FALSE;
//	DWORD dwInstanceThreadId = 0, dwErr = 0;
//	HANDLE hPipe = NULL, hInstanceThread = NULL;
//
//// The main loop creates an instance of the named pipe and
//// then waits for a client to connect to it. When the client
//// connects, a thread is created to handle communications
//// with that client, and the loop is repeated.
//
//	for(;;)
//	{
//		MCHKHEAP;
//		hPipe = Create NamedPipe(
//		            gpSrv->szPipename,               // pipe name
//		            PIPE_ACCESS_DUPLEX,       // read/write access
//		            PIPE_TYPE_MESSAGE |       // message type pipe
//		            PIPE_READMODE_MESSAGE |   // message-read mode
//		            PIPE_WAIT,                // blocking mode
//		            PIPE_UNLIMITED_INSTANCES, // max. instances
//		            PIPEBUFSIZE,              // output buffer size
//		            PIPEBUFSIZE,              // input buffer size
//		            0,                        // client time-out
//		            gpLocalSecurity);          // default security attribute
//		_ASSERTE(hPipe != INVALID_HANDLE_VALUE);
//
//		if (hPipe == INVALID_HANDLE_VALUE)
//		{
//			dwErr = GetLastError();
//			_printf("Create NamedPipe failed, ErrCode=0x%08X\n", dwErr);
//			Sleep(10);
//			continue;
//		}
//
//		// Wait for the client to connect; if it succeeds,
//		// the function returns a nonzero value. If the function
//		// returns zero, GetLastError returns ERROR_PIPE_CONNECTED.
//		fConnected = ConnectNamedPipe(hPipe, NULL) ?
//		             TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
//
//		if (WaitForSingleObject(ghQuitEvent, 0) == WAIT_OBJECT_0)
//			break;
//
//		MCHKHEAP;
//
//		if (fConnected)
//		{
//			// Create a thread for this client.
//			hInstanceThread = CreateThread(
//			                      NULL,              // no security attribute
//			                      0,                 // default stack size
//			                      InstanceThread,    // thread proc
//			                      (LPVOID) hPipe,    // thread parameter
//			                      0,                 // not suspended
//			                      &dwInstanceThreadId);      // returns thread ID
//
//			if (hInstanceThread == NULL)
//			{
//				dwErr = GetLastError();
//				_printf("CreateThread(Instance) failed, ErrCode=0x%08X\n", dwErr);
//				Sleep(10);
//				continue;
//			}
//			else
//			{
//				SafeCloseHandle(hInstanceThread);
//			}
//		}
//		else
//		{
//			// The client could not connect, so close the pipe.
//			SafeCloseHandle(hPipe);
//		}
//
//		MCHKHEAP;
//	}
//
//	return 1;
//}

//DWORD WINAPI InstanceThread(LPVOID lpvParam)
//{
//	CESERVER_REQ in= {{0}}, *pIn=NULL, *pOut=NULL;
//	DWORD cbBytesRead, cbWritten, dwErr = 0;
//	BOOL fSuccess;
//	HANDLE hPipe;
//	// The thread's parameter is a handle to a pipe instance.
//	hPipe = (HANDLE) lpvParam;
//	MCHKHEAP;
//	// Read client requests from the pipe.
//	memset(&in, 0, sizeof(in));
//	fSuccess = ReadFile(
//	               hPipe,        // handle to pipe
//	               &in,          // buffer to receive data
//	               sizeof(in),   // size of buffer
//	               &cbBytesRead, // number of bytes read
//	               NULL);        // not overlapped I/O
//
//	if ((!fSuccess && ((dwErr = GetLastError()) != ERROR_MORE_DATA)) ||
//	        cbBytesRead < sizeof(CESERVER_REQ_HDR) || in.hdr.cbSize < sizeof(CESERVER_REQ_HDR))
//	{
//		goto wrap;
//	}
//
//	if (in.hdr.cbSize > cbBytesRead)
//	{
//		DWORD cbNextRead = 0;
//		// ��� ������ calloc, � �� ExecuteNewCmd, �.�. ������ ������ �������, � �� ����������� �����
//		pIn = (CESERVER_REQ*)calloc(in.hdr.cbSize, 1);
//
//		if (!pIn)
//			goto wrap;
//
//		memmove(pIn, &in, cbBytesRead); // ������ ��������� ����������
//		fSuccess = ReadFile(
//		               hPipe,        // handle to pipe
//		               ((LPBYTE)pIn)+cbBytesRead,  // buffer to receive data
//		               in.hdr.cbSize - cbBytesRead,   // size of buffer
//		               &cbNextRead, // number of bytes read
//		               NULL);        // not overlapped I/O
//
//		if (fSuccess)
//			cbBytesRead += cbNextRead;
//	}
//
//	if (!ProcessSrvCommand(pIn ? *pIn : in, &pOut) || pOut==NULL)
//	{
//		// ���� ���������� ��� - ��� ����� ���-������ �������, ����� TransactNamedPipe ����� �������?
//		CESERVER_REQ_HDR Out;
//		ExecutePrepareCmd(&Out, in.hdr.nCmd, sizeof(Out));
//		fSuccess = WriteFile(
//		               hPipe,        // handle to pipe
//		               &Out,         // buffer to write from
//		               Out.cbSize,    // number of bytes to write
//		               &cbWritten,   // number of bytes written
//		               NULL);        // not overlapped I/O
//	}
//	else
//	{
//		MCHKHEAP;
//		// Write the reply to the pipe.
//		fSuccess = WriteFile(
//		               hPipe,        // handle to pipe
//		               pOut,         // buffer to write from
//		               pOut->hdr.cbSize,  // number of bytes to write
//		               &cbWritten,   // number of bytes written
//		               NULL);        // not overlapped I/O
//
//		// ���������� ������
//		if ((LPVOID)pOut != (LPVOID)gpStoredOutput)  // ���� ��� �� ����������� �����
//			ExecuteFreeResult(pOut);
//	}
//
//	if (pIn)    // �� �������������, ����, ����� ������� ������ �������� �� ����
//	{
//		free(pIn); pIn = NULL;
//	}
//
//	MCHKHEAP;
//	//if (!fSuccess || pOut->hdr.cbSize != cbWritten) break;
//// Flush the pipe to allow the client to read the pipe's contents
//// before disconnecting. Then disconnect the pipe, and close the
//// handle to this pipe instance.
//wrap: // Flush � Disconnect ������ ������
//	FlushFileBuffers(hPipe);
//	DisconnectNamedPipe(hPipe);
//	SafeCloseHandle(hPipe);
//	return 1;
//}

BOOL cmd_SetSizeXXX_CmdStartedFinished(CESERVER_REQ& in, CESERVER_REQ** out)
{
	BOOL lbRc = FALSE;
	
		//case CECMD_SETSIZESYNC:
		//case CECMD_SETSIZENOSYNC:
		//case CECMD_CMDSTARTED:
		//case CECMD_CMDFINISHED:
		
	MCHKHEAP;
	int nOutSize = sizeof(CESERVER_REQ_HDR) + sizeof(CONSOLE_SCREEN_BUFFER_INFO) + sizeof(DWORD);
	*out = ExecuteNewCmd(0,nOutSize);

	if (*out == NULL) return FALSE;

	MCHKHEAP;

	if (in.hdr.cbSize >= (sizeof(CESERVER_REQ_HDR) + sizeof(CESERVER_REQ_SETSIZE)))
	{
		USHORT nBufferHeight = 0;
		COORD  crNewSize = {0,0};
		SMALL_RECT rNewRect = {0};
		SHORT  nNewTopVisible = -1;
		//memmove(&nBufferHeight, in.Data, sizeof(USHORT));
		nBufferHeight = in.SetSize.nBufferHeight;

		if (nBufferHeight == -1)
		{
			// ��� 'far /w' ����� �������� ������ ������!
			if (in.SetSize.size.Y < gpSrv->sbi.dwSize.Y
			        && gpSrv->sbi.dwSize.Y > (gpSrv->sbi.srWindow.Bottom - gpSrv->sbi.srWindow.Top + 1))
			{
				nBufferHeight = gpSrv->sbi.dwSize.Y;
			}
			else
			{
				nBufferHeight = 0;
			}
		}

		//memmove(&crNewSize, in.Data+sizeof(USHORT), sizeof(COORD));
		crNewSize = in.SetSize.size;
		//memmove(&nNewTopVisible, in.Data+sizeof(USHORT)+sizeof(COORD), sizeof(SHORT));
		nNewTopVisible = in.SetSize.nSendTopLine;
		//memmove(&rNewRect, in.Data+sizeof(USHORT)+sizeof(COORD)+sizeof(SHORT), sizeof(SMALL_RECT));
		rNewRect = in.SetSize.rcWindow;
		MCHKHEAP;
		(*out)->hdr.nCmd = in.hdr.nCmd;
		// ��� ��������� ���� ��������� ��� ��������� � ExecuteNewCmd

		//#ifdef _DEBUG
		if (in.hdr.nCmd == CECMD_CMDFINISHED)
		{
			PRINT_COMSPEC(L"CECMD_CMDFINISHED, Set height to: %i\n", crNewSize.Y);
			DEBUGSTRCMD(L"\n!!! CECMD_CMDFINISHED !!!\n\n");
			// ������� �����������
			TODO("����� ������ ������� �������")
			//if (gpSrv->dwWinEventThread != 0)
			//	PostThreadMessage(gpSrv->dwWinEventThread, gpSrv->nMsgHookEnableDisable, TRUE, 0);
		}
		else if (in.hdr.nCmd == CECMD_CMDSTARTED)
		{
			PRINT_COMSPEC(L"CECMD_CMDSTARTED, Set height to: %i\n", nBufferHeight);
			DEBUGSTRCMD(L"\n!!! CECMD_CMDSTARTED !!!\n\n");
			// ��������� �����������
			TODO("����� ������ ������� �������")
			//if (gpSrv->dwWinEventThread != 0)
			//	PostThreadMessage(gpSrv->dwWinEventThread, gpSrv->nMsgHookEnableDisable, FALSE, 0);
		}

		//#endif

		if (in.hdr.nCmd == CECMD_CMDFINISHED)
		{
			// ��������� ������ ���� �������
			PRINT_COMSPEC(L"Storing long output\n", 0);
			CmdOutputStore();
			PRINT_COMSPEC(L"Storing long output (done)\n", 0);
		}

		MCHKHEAP;

		if (in.hdr.nCmd == CECMD_SETSIZESYNC)
		{
			ResetEvent(gpSrv->hAllowInputEvent);
			gpSrv->bInSyncResize = TRUE;
		}

		gpSrv->nTopVisibleLine = nNewTopVisible;
		WARNING("���� ������ dwFarPID - ��� ���-�� ��� ���� ������ ����������?");
		SetConsoleSize(nBufferHeight, crNewSize, rNewRect, ":CECMD_SETSIZESYNC");
		WARNING("!! �� ����� �� ���������� �������� � ��������� ������ ��� �������� ����� ���������?");

		if (in.hdr.nCmd == CECMD_SETSIZESYNC)
		{
			CESERVER_REQ *pPlgIn = NULL, *pPlgOut = NULL;

			//TODO("���� �������������, ����� GUI ����������� ���������");
			if (in.SetSize.dwFarPID /*&& !nBufferHeight*/)
			{
				// �� ��������� �����-�� �������� FAR (�� ������� ���� � /w)
				// ���� ��������� ������ ������ ����� �������� ������ ��� ��������
				CONSOLE_SCREEN_BUFFER_INFO sc = {{0,0}};
				GetConsoleScreenBufferInfo(ghConOut, &sc);
				HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
				INPUT_RECORD r = {MOUSE_EVENT};
				r.Event.MouseEvent.dwMousePosition.X = sc.srWindow.Right-1;
				r.Event.MouseEvent.dwMousePosition.Y = sc.srWindow.Bottom-1;
				r.Event.MouseEvent.dwEventFlags = MOUSE_MOVED;
				DWORD cbWritten = 0;
				WriteConsoleInput(hIn, &r, 1, &cbWritten);
				// ������� ����� ��������� ����� ������ FAR�
				wchar_t szPipeName[128];
				_wsprintf(szPipeName, SKIPLEN(countof(szPipeName)) CEPLUGINPIPENAME, L".", in.SetSize.dwFarPID);
				//DWORD nHILO = ((DWORD)crNewSize.X) | (((DWORD)(WORD)crNewSize.Y) << 16);
				//pPlgIn = ExecuteNewCmd(CMD_SETSIZE, sizeof(CESERVER_REQ_HDR)+sizeof(nHILO));
				pPlgIn = ExecuteNewCmd(CMD_REDRAWFAR, sizeof(CESERVER_REQ_HDR));
				//pPlgIn->dwData[0] = nHILO;
				pPlgOut = ExecuteCmd(szPipeName, pPlgIn, 500, ghConWnd);

				if (pPlgOut) ExecuteFreeResult(pPlgOut);
			}

			SetEvent(gpSrv->hAllowInputEvent);
			gpSrv->bInSyncResize = FALSE;
			// ����������� RefreshThread - ���������� �������
			ReloadFullConsoleInfo(FALSE); // ������� Refresh � ���� Refresh
		}

		MCHKHEAP;

		if (in.hdr.nCmd == CECMD_CMDSTARTED)
		{
			// ������������ ����� ������� (������������ �����) ����� �������
			CmdOutputRestore();
		}
	}

	MCHKHEAP;
	PCONSOLE_SCREEN_BUFFER_INFO psc = &((*out)->SetSizeRet.SetSizeRet);
	MyGetConsoleScreenBufferInfo(ghConOut, psc);
	DWORD lnNextPacketId = ++gpSrv->nLastPacketID;
	(*out)->SetSizeRet.nNextPacketId = lnNextPacketId;
	//gpSrv->bForceFullSend = TRUE;
	SetEvent(gpSrv->hRefreshEvent);
	MCHKHEAP;
	lbRc = TRUE;

	return lbRc;
}

BOOL cmd_GetOutput(CESERVER_REQ& in, CESERVER_REQ** out)
{
	BOOL lbRc = FALSE;
	MSectionLock CS; CS.Lock(gpcsStoredOutput, FALSE);

	if (gpStoredOutput)
	{
		_ASSERTE(sizeof(CESERVER_CONSAVE_HDR) > sizeof(gpStoredOutput->hdr.hdr));
		DWORD nSize = sizeof(CESERVER_CONSAVE_HDR)
		              + min((int)gpStoredOutput->hdr.cbMaxOneBufferSize,
		                    (gpStoredOutput->hdr.sbi.dwSize.X*gpStoredOutput->hdr.sbi.dwSize.Y*2));
		*out = ExecuteNewCmd(CECMD_GETOUTPUT, nSize);
		if (*out)
		{
			memmove((*out)->Data, ((LPBYTE)gpStoredOutput) + sizeof(gpStoredOutput->hdr.hdr), nSize - sizeof(gpStoredOutput->hdr.hdr));
			lbRc = TRUE;
		}
	}
	else
	{
		*out = ExecuteNewCmd(CECMD_GETOUTPUT, sizeof(CESERVER_REQ_HDR));
		lbRc = (*out != NULL);
	}

	return lbRc;
}

BOOL cmd_Attach2Gui(CESERVER_REQ& in, CESERVER_REQ** out)
{
	BOOL lbRc = FALSE;
	
	// ����� ������ �� Attach2Gui() �������
	HWND hDc = Attach2Gui(ATTACH2GUI_TIMEOUT);

	if (hDc != NULL)
	{
		int nOutSize = sizeof(CESERVER_REQ_HDR) + sizeof(DWORD);
		*out = ExecuteNewCmd(CECMD_ATTACH2GUI,nOutSize);

		if (*out != NULL)
		{
			// ����� �� ������������ "Press any key to close console"
			DisableAutoConfirmExit();
			//
			(*out)->dwData[0] = (DWORD)hDc; //-V205 // ���������� ����
			lbRc = TRUE;

			gpSrv->bWasReattached = TRUE;
			if (gpSrv->hRefreshEvent)
			{
				SetEvent(gpSrv->hRefreshEvent);
			}
			else
			{
				_ASSERTE(gpSrv->hRefreshEvent!=NULL);
			}
		}
	}
	
	return lbRc;
}

BOOL cmd_FarLoaded(CESERVER_REQ& in, CESERVER_REQ** out)
{
	BOOL lbRc = FALSE;
	
	// gnConfirmExitParm==1 ����������, ����� ������� ����������� ����� "-new_console"
	// ���� ������ ���� ���������� - ����� ����� ��������� ������������� �������� �������
	if ((gbAutoDisableConfirmExit || (gnConfirmExitParm == 1)) && gpSrv->dwRootProcess == in.dwData[0])
	{
		// FAR ��������� ����������, ������� ��� ��� �� � ������������� �������� ������� �� �����������
		DisableAutoConfirmExit(TRUE);
	}
	
	int nOutSize = sizeof(CESERVER_REQ_HDR) + sizeof(DWORD);
	*out = ExecuteNewCmd(CECMD_FARLOADED,nOutSize);
	if (*out != NULL)
	{
		(*out)->dwData[0] = GetCurrentProcessId();
		lbRc = TRUE;
	}

	return lbRc;
}

BOOL cmd_PostConMsg(CESERVER_REQ& in, CESERVER_REQ** out)
{
	BOOL lbRc = FALSE;
	
	HWND hSendWnd = (HWND)in.Msg.hWnd;

	// Info & Log
	if (in.Msg.nMsg == WM_INPUTLANGCHANGE || in.Msg.nMsg == WM_INPUTLANGCHANGEREQUEST)
	{
		WPARAM wParam = (WPARAM)in.Msg.wParam;
		LPARAM lParam = (LPARAM)in.Msg.lParam;
		unsigned __int64 l = lParam;
		
		#ifdef _DEBUG
		wchar_t szDbg[255];
		_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"ConEmuC: %s(0x%08X, %s, CP:%i, HKL:0x%08I64X)\n",
		          in.Msg.bPost ? L"PostMessage" : L"SendMessage", (DWORD)hSendWnd,
		          (in.Msg.nMsg == WM_INPUTLANGCHANGE) ? L"WM_INPUTLANGCHANGE" :
		          (in.Msg.nMsg == WM_INPUTLANGCHANGEREQUEST) ? L"WM_INPUTLANGCHANGEREQUEST" :
		          (in.Msg.nMsg == WM_SHOWWINDOW) ? L"WM_SHOWWINDOW" :
		          L"<Other message>",
		          (DWORD)wParam, l);
		DEBUGLOGLANG(szDbg);
		#endif

		if (ghLogSize)
		{
			char szInfo[255];
			_wsprintfA(szInfo, SKIPLEN(countof(szInfo)) "ConEmuC: %s(0x%08X, %s, CP:%i, HKL:0x%08I64X)",
			           in.Msg.bPost ? "PostMessage" : "SendMessage", (DWORD)hSendWnd, //-V205
			           (in.Msg.nMsg == WM_INPUTLANGCHANGE) ? "WM_INPUTLANGCHANGE" :
			           (in.Msg.nMsg == WM_INPUTLANGCHANGEREQUEST) ? "WM_INPUTLANGCHANGEREQUEST" :
			           "<Other message>",
			           (DWORD)wParam, l);
			LogString(szInfo);
		}
	}

	if (in.Msg.nMsg == WM_SHOWWINDOW)
	{
		DWORD lRc = 0;

		if (in.Msg.bPost)
			lRc = apiShowWindowAsync(hSendWnd, (int)(in.Msg.wParam & 0xFFFF));
		else
			lRc = apiShowWindow(hSendWnd, (int)(in.Msg.wParam & 0xFFFF));

		// ���������� ���������
		DWORD dwErr = GetLastError();
		int nOutSize = sizeof(CESERVER_REQ_HDR) + 2*sizeof(DWORD);
		*out = ExecuteNewCmd(CECMD_POSTCONMSG,nOutSize);

		if (*out != NULL)
		{
			(*out)->dwData[0] = lRc;
			(*out)->dwData[1] = dwErr;
			lbRc = TRUE;
		}
	}
	else if (in.Msg.nMsg == WM_SIZE)
	{
		//
	}
	else if (in.Msg.bPost)
	{
		PostMessage(hSendWnd, in.Msg.nMsg, (WPARAM)in.Msg.wParam, (LPARAM)in.Msg.lParam);
	}
	else
	{
		LRESULT lRc = SendMessage(hSendWnd, in.Msg.nMsg, (WPARAM)in.Msg.wParam, (LPARAM)in.Msg.lParam);
		// ���������� ���������
		int nOutSize = sizeof(CESERVER_REQ_HDR) + sizeof(u64);
		*out = ExecuteNewCmd(CECMD_POSTCONMSG,nOutSize);

		if (*out != NULL)
		{
			(*out)->qwData[0] = lRc;
			lbRc = TRUE;
		}
	}
	
	return lbRc;
}

BOOL cmd_SaveAliases(CESERVER_REQ& in, CESERVER_REQ** out)
{
	BOOL lbRc = FALSE;
	
	//wchar_t* pszAliases; DWORD nAliasesSize;
	// ��������� ������
	DWORD nNewSize = in.hdr.cbSize - sizeof(in.hdr);

	if (nNewSize > gpSrv->nAliasesSize)
	{
		MCHKHEAP;
		wchar_t* pszNew = (wchar_t*)calloc(nNewSize, 1);

		if (!pszNew)
			goto wrap;  // �� ������� ������� ������

		MCHKHEAP;

		if (gpSrv->pszAliases) free(gpSrv->pszAliases);

		MCHKHEAP;
		gpSrv->pszAliases = pszNew;
		gpSrv->nAliasesSize = nNewSize;
	}

	if (nNewSize > 0 && gpSrv->pszAliases)
	{
		MCHKHEAP;
		memmove(gpSrv->pszAliases, in.Data, nNewSize);
		MCHKHEAP;
	}
	
wrap:
	return lbRc;
}

BOOL cmd_GetAliases(CESERVER_REQ& in, CESERVER_REQ** out)
{
	BOOL lbRc = FALSE;
	
	// ���������� ����������� ������
	int nOutSize = sizeof(CESERVER_REQ_HDR) + gpSrv->nAliasesSize;
	*out = ExecuteNewCmd(CECMD_GETALIASES,nOutSize);

	if (*out != NULL)
	{
		if (gpSrv->pszAliases && gpSrv->nAliasesSize)
		{
			memmove((*out)->Data, gpSrv->pszAliases, gpSrv->nAliasesSize);
		}
		lbRc = TRUE;
	}
	
	return lbRc;
}

BOOL cmd_SetDontClose(CESERVER_REQ& in, CESERVER_REQ** out)
{
	BOOL lbRc = FALSE;
	
	// ����� ������ � ���� ������� (�������� dir) ����������, �����
	// ������� ���������� �� ���������...
	gbAutoDisableConfirmExit = FALSE;
	gbAlwaysConfirmExit = TRUE;
	
	return lbRc;
}

BOOL cmd_OnActivation(CESERVER_REQ& in, CESERVER_REQ** out)
{
	BOOL lbRc = FALSE;
	
	if (gpSrv->pConsole)
	{
		gpSrv->pConsole->hdr.bConsoleActive = in.dwData[0];
		gpSrv->pConsole->hdr.bThawRefreshThread = in.dwData[1];
		//gpSrv->pConsoleMap->SetFrom(&(gpSrv->pConsole->hdr));
		UpdateConsoleMapHeader();

		// ���� ������� ������������ - �� ������������� ���������� �� ����������
		if (gpSrv->pConsole->hdr.bConsoleActive)
			ReloadFullConsoleInfo(TRUE);
	}
	
	return lbRc;
}

BOOL cmd_SetWindowPos(CESERVER_REQ& in, CESERVER_REQ** out)
{
	BOOL lbRc = FALSE;
	
	SetWindowPos(in.SetWndPos.hWnd, in.SetWndPos.hWndInsertAfter,
	             in.SetWndPos.X, in.SetWndPos.Y, in.SetWndPos.cx, in.SetWndPos.cy,
	             in.SetWndPos.uFlags);
	
	return lbRc;
}

BOOL cmd_SetFocus(CESERVER_REQ& in, CESERVER_REQ** out)
{
	BOOL lbRc = FALSE, lbForeRc = FALSE;
	HWND hFocusRc = NULL;
	
	if (in.setFocus.bSetForeground)
		lbForeRc = SetForegroundWindow(in.setFocus.hWindow);
	else
		hFocusRc = SetFocus(in.setFocus.hWindow);
	
	return lbRc;
}

BOOL cmd_SetParent(CESERVER_REQ& in, CESERVER_REQ** out)
{
	BOOL lbRc = FALSE; //, lbForeRc = FALSE;

	HWND h = SetParent(in.setParent.hWnd, in.setParent.hParent);

	int nOutSize = sizeof(CESERVER_REQ_HDR) + sizeof(CESERVER_REQ_SETPARENT);
	*out = ExecuteNewCmd(CECMD_SETPARENT,nOutSize);

	if (*out != NULL)
	{
		(*out)->setParent.hWnd = (HWND)GetLastError();
		(*out)->setParent.hParent = h;

		lbRc = TRUE;
	}

	return lbRc;
}

BOOL cmd_SetWindowRgn(CESERVER_REQ& in, CESERVER_REQ** out)
{
	BOOL lbRc = FALSE;
	
	MySetWindowRgn(&in.SetWndRgn);
	
	return lbRc;
}

BOOL cmd_DetachCon(CESERVER_REQ& in, CESERVER_REQ** out)
{
	BOOL lbRc = FALSE;

	//if (gOSVer.dwMajorVersion >= 6)
	//{
	//	if ((gcrVisibleSize.X <= 0) && (gcrVisibleSize.Y <= 0))
	//	{
	//		_ASSERTE((gcrVisibleSize.X > 0) && (gcrVisibleSize.Y > 0));
	//	}
	//	else
	//	{
	//		int curSizeY, curSizeX;
	//		wchar_t sFontName[LF_FACESIZE];
	//		HANDLE hOutput = (HANDLE)ghConOut;

	//		if (apiGetConsoleFontSize(hOutput, curSizeY, curSizeX, sFontName) && curSizeY && curSizeX)
	//		{
	//			COORD crLargest = GetLargestConsoleWindowSize(hOutput);
	//			HMONITOR hMon = MonitorFromWindow(ghConWnd, MONITOR_DEFAULTTOPRIMARY);
	//			MONITORINFO mi = {sizeof(mi)};
	//			int nMaxX = 0, nMaxY = 0;
	//			if (GetMonitorInfo(hMon, &mi))
	//			{
	//				nMaxX = mi.rcWork.right - mi.rcWork.left - 2*GetSystemMetrics(SM_CXSIZEFRAME) - GetSystemMetrics(SM_CYCAPTION);
	//				nMaxY = mi.rcWork.bottom - mi.rcWork.top - 2*GetSystemMetrics(SM_CYSIZEFRAME);
	//			}
	//			
	//			if ((nMaxX > 0) && (nMaxY > 0))
	//			{
	//				int nFontX = nMaxX  / gcrVisibleSize.X;
	//				int nFontY = nMaxY / gcrVisibleSize.Y;
	//				// Too large height?
	//				if (nFontY > 28)
	//				{
	//					nFontX = 28 * nFontX / nFontY;
	//					nFontY = 28;
	//				}

	//				// Evaluate default width for the font
	//				int nEvalX = EvaluateDefaultFontWidth(nFontY, sFontName);
	//				if (nEvalX > 0)
	//					nFontX = nEvalX;

	//				// Look in the registry?
	//				HKEY hk;
	//				DWORD nRegSize = 0, nLen;
	//				if (!RegOpenKeyEx(HKEY_CURRENT_USER, L"Console", 0, KEY_READ, &hk))
	//				{
	//					if (RegQueryValueEx(hk, L"FontSize", NULL, NULL, (LPBYTE)&nRegSize, &(nLen=sizeof(nRegSize))) || nLen!=sizeof(nRegSize))
	//						nRegSize = 0;
	//					RegCloseKey(hk);
	//				}
	//				if (!nRegSize && !RegOpenKeyEx(HKEY_CURRENT_USER, L"Console\\%SystemRoot%_system32_cmd.exe", 0, KEY_READ, &hk))
	//				{
	//					if (RegQueryValueEx(hk, L"FontSize", NULL, NULL, (LPBYTE)&nRegSize, &(nLen=sizeof(nRegSize))) || nLen!=sizeof(nRegSize))
	//						nRegSize = 0;
	//					RegCloseKey(hk);
	//				}
	//				if ((HIWORD(nRegSize) > curSizeY) && (HIWORD(nRegSize) < nFontY)
	//					&& (LOWORD(nRegSize) > curSizeX) && (LOWORD(nRegSize) < nFontX))
	//				{
	//					nFontY = HIWORD(nRegSize);
	//					nFontX = LOWORD(nRegSize);
	//				}

	//				if ((nFontX > curSizeX) || (nFontY > curSizeY))
	//				{
	//					apiSetConsoleFontSize(hOutput, nFontY, nFontX, sFontName);
	//				}
	//			}
	//		}
	//	}
	//}
	
	gpSrv->bWasDetached = TRUE;
	ghConEmuWnd = NULL;
	ghConEmuWndDC = NULL;
	gpSrv->dwGuiPID = 0;
	UpdateConsoleMapHeader();
	EmergencyShow(ghConWnd);

	int nOutSize = sizeof(CESERVER_REQ_HDR);
	*out = ExecuteNewCmd(CECMD_DETACHCON,nOutSize);
	lbRc = (*out != NULL);
	
	return lbRc;
}

BOOL cmd_CmdStartStop(CESERVER_REQ& in, CESERVER_REQ** out)
{
	BOOL lbRc = FALSE;
	
	MSectionLock CS; CS.Lock(gpSrv->csProc);
	
	UINT nPrevCount = gpSrv->nProcessCount;
	BOOL lbChanged = FALSE;
	_ASSERTE(in.StartStop.dwPID!=0);
	DWORD nPID = in.StartStop.dwPID;

	if (!gpSrv->nProcessStartTick && (gpSrv->dwRootProcess == in.StartStop.dwPID))
	{
		gpSrv->nProcessStartTick = GetTickCount();
	}
	
#ifdef _DEBUG
	wchar_t szDbg[128];

	switch (in.StartStop.nStarted)
	{
		case sst_ServerStart:
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"SRV received CECMD_CMDSTARTSTOP(ServerStart,%i,PID=%u)\n", in.hdr.nCreateTick, in.StartStop.dwPID);
			break;
		case sst_ServerStop:
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"SRV received CECMD_CMDSTARTSTOP(ServerStop,%i,PID=%u)\n", in.hdr.nCreateTick, in.StartStop.dwPID);
			break;
		case sst_ComspecStart:
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"SRV received CECMD_CMDSTARTSTOP(ComspecStart,%i,PID=%u)\n", in.hdr.nCreateTick, in.StartStop.dwPID);
			break;
		case sst_ComspecStop:
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"SRV received CECMD_CMDSTARTSTOP(ComspecStop,%i,PID=%u)\n", in.hdr.nCreateTick, in.StartStop.dwPID);
			break;
		case sst_AppStart:
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"SRV received CECMD_CMDSTARTSTOP(AppStart,%i,PID=%u)\n", in.hdr.nCreateTick, in.StartStop.dwPID);
			break;
		case sst_AppStop:
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"SRV received CECMD_CMDSTARTSTOP(AppStop,%i,PID=%u)\n", in.hdr.nCreateTick, in.StartStop.dwPID);
			break;
		case sst_App16Start:
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"SRV received CECMD_CMDSTARTSTOP(App16Start,%i,PID=%u)\n", in.hdr.nCreateTick, in.StartStop.dwPID);
			break;
		case sst_App16Stop:
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"SRV received CECMD_CMDSTARTSTOP(App16Stop,%i,PID=%u)\n", in.hdr.nCreateTick, in.StartStop.dwPID);
			break;
	}

	DEBUGSTRCMD(szDbg);
#endif
	_ASSERTE(in.StartStop.dwPID!=0);

	if ((in.StartStop.nStarted == sst_ComspecStart) || (in.StartStop.nStarted == sst_AppStart))
	{
		// �������� ������� � ������
		_ASSERTE(gpSrv->pnProcesses[0] == gnSelfPID);
		lbChanged = ProcessAdd(nPID, &CS);
	}
	else if ((in.StartStop.nStarted == sst_ComspecStop) || (in.StartStop.nStarted == sst_AppStop))
	{
		// ������� ������� �� ������
		_ASSERTE(gpSrv->pnProcesses[0] == gnSelfPID);
		DWORD nChange = 0;
		for (DWORD n = 0; n < nPrevCount; n++)
		{
			if (gpSrv->pnProcesses[n] == nPID)
			{
				CS.RelockExclusive(200);
				lbChanged = TRUE;
				gpSrv->nProcessCount--;
				continue;
			}
			if (lbChanged)
			{
				gpSrv->pnProcesses[nChange] = gpSrv->pnProcesses[n];
			}
			nChange++;
		}
	}
	else
	{
		_ASSERTE(in.StartStop.nStarted==sst_AppStart || in.StartStop.nStarted==sst_AppStop || in.StartStop.nStarted==sst_ComspecStart || in.StartStop.nStarted==sst_ComspecStop);
	}
	
	// ***
	if (lbChanged)
		ProcessCountChanged(TRUE, nPrevCount, &CS);
	CS.Unlock();
	// ***

	int nOutSize = sizeof(CESERVER_REQ_HDR) + sizeof(CESERVER_REQ_STARTSTOPRET);
	*out = ExecuteNewCmd(CECMD_CMDSTARTSTOP,nOutSize);

	if (*out != NULL)
	{
		(*out)->StartStopRet.bWasBufferHeight = (gnBufferHeight != 0);
		(*out)->StartStopRet.hWnd = ghConEmuWnd;
		(*out)->StartStopRet.hWndDC = ghConEmuWndDC;
		(*out)->StartStopRet.dwPID = gpSrv->dwGuiPID;
		(*out)->StartStopRet.nBufferHeight = gnBufferHeight;
		(*out)->StartStopRet.nWidth = gpSrv->sbi.dwSize.X;
		(*out)->StartStopRet.nHeight = (gpSrv->sbi.srWindow.Bottom - gpSrv->sbi.srWindow.Top + 1);
		(*out)->StartStopRet.dwSrvPID = GetCurrentProcessId();

		lbRc = TRUE;
	}
	
	return lbRc;
}

BOOL cmd_SetFarPID(CESERVER_REQ& in, CESERVER_REQ** out)
{
	BOOL lbRc = FALSE;

	gpSrv->nActiveFarPID = in.hdr.nSrcPID;
	UpdateConsoleMapHeader();

	return lbRc;
}

BOOL cmd_GuiChanged(CESERVER_REQ& in, CESERVER_REQ** out)
{
	// ���������� � ������, ����� �� ������� � ���� ConEmuGuiMapping->CESERVER_CONSOLE_MAPPING_HDR
	BOOL lbRc = FALSE;

	if (gpSrv && gpSrv->pConsole)
	{
		ReloadGuiSettings(&in.GuiInfo);
		//// !!! Warning !!! ������� �����, ������� � CreateMapHeader() !!!
		//gpSrv->pConsole->hdr.nLoggingType = in.GuiInfo.nLoggingType;
		//gpSrv->pConsole->hdr.bDosBox = in.GuiInfo.bDosBox;
		//gpSrv->pConsole->hdr.bUseInjects = in.GuiInfo.bUseInjects;
		//UpdateConsoleMapHeader();
	}

	return lbRc;
}

BOOL cmd_TerminatePid(CESERVER_REQ& in, CESERVER_REQ** out)
{
	// ������� ������� (TerminateProcess)
	BOOL lbRc = FALSE;
	HANDLE hProcess = NULL;
	BOOL bNeedClose = FALSE;
	DWORD nErrCode = 0;

	if (gpSrv && gpSrv->pConsole)
	{
		if (in.dwData[0] == gpSrv->dwRootProcess)
		{
			hProcess = gpSrv->hRootProcess;
			bNeedClose = FALSE;
		}
	}

	if (!hProcess)
	{
		hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, in.dwData[0]);
		if (hProcess != NULL)
		{
			bNeedClose = TRUE;
		}
		else
		{
			nErrCode = GetLastError();
		}
	}

	if (hProcess != NULL)
	{
		if (TerminateProcess(hProcess, 100))
		{
			lbRc = TRUE;
		}
		else
		{
			nErrCode = GetLastError();
			if (!bNeedClose)
			{
				hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, in.dwData[0]);
				if (hProcess != NULL)
				{
					bNeedClose = TRUE;
					if (TerminateProcess(hProcess, 100))
						lbRc = TRUE;
				}
			}
		}
	}

	int nOutSize = sizeof(CESERVER_REQ_HDR) + 2*sizeof(DWORD);
	*out = ExecuteNewCmd(CECMD_TERMINATEPID,nOutSize);

	if (*out != NULL)
	{
		(*out)->dwData[0] = lbRc;
		(*out)->dwData[1] = nErrCode;
	}

	if (hProcess && bNeedClose)
		CloseHandle(hProcess);
	return lbRc;
}

BOOL cmd_GuiAppAttached(CESERVER_REQ& in, CESERVER_REQ** out)
{
	BOOL lbRc = FALSE;

	if (!gpSrv)
	{
		_ASSERTE(gpSrv!=NULL);
	}
	else
	{
		_ASSERTEX(in.AttachGuiApp.nPID == gpSrv->dwRootProcess && gpSrv->dwRootProcess && in.AttachGuiApp.nPID);

		wchar_t szInfo[MAX_PATH*2];

		// ���������� ��� ����. ������ (��� ������� exe) ahGuiWnd==NULL, ������ - ����� ������������ �������� ����
		if (gpSrv->hRootProcessGui == NULL)
		{
			_wsprintf(szInfo, SKIPLEN(countof(szInfo)) L"GUI application (PID=%u) was attached to ConEmu:\n%s\n",
				in.AttachGuiApp.nPID, in.AttachGuiApp.sAppFileName);
			_wprintf(szInfo);
		}

		if (in.AttachGuiApp.hWindow && (gpSrv->hRootProcessGui != in.AttachGuiApp.hWindow))
		{
			wchar_t szTitle[MAX_PATH];
			GetWindowText(in.AttachGuiApp.hWindow, szTitle, countof(szTitle));
			_wsprintf(szInfo,  SKIPLEN(countof(szInfo)) L"\nWindow (x%08X) was attached to ConEmu:\n%s\n", (DWORD)in.AttachGuiApp.hWindow, szTitle);
			_wprintf(szInfo);
		}

		if (in.AttachGuiApp.hWindow == NULL)
		{
			gpSrv->hRootProcessGui = (HWND)-1;
		}
		else
		{
			gpSrv->hRootProcessGui = in.AttachGuiApp.hWindow;
		}
		// ������ � ������������� ��� - GUI ���������� � ������� ������ �� �������
		gbAlwaysConfirmExit = FALSE;
		CheckProcessCount(TRUE);
		lbRc = TRUE;
	}

	int nOutSize = sizeof(CESERVER_REQ_HDR) + sizeof(DWORD);
	*out = ExecuteNewCmd(CECMD_ATTACHGUIAPP,nOutSize);

	if (*out != NULL)
	{
		(*out)->dwData[0] = lbRc;
	}

	return TRUE;
}

BOOL cmd_RegExtConsole(CESERVER_REQ& in, CESERVER_REQ** out)
{
	BOOL lbRc = FALSE;

	if (!gpSrv)
	{
		_ASSERTE(gpSrv!=NULL);
	}
	else
	{
		if (in.RegExtCon.hCommitEvent != NULL)
		{
			if (gpSrv->hExtConsoleCommit && gpSrv->hExtConsoleCommit != (HANDLE)in.RegExtCon.hCommitEvent)
				CloseHandle(gpSrv->hExtConsoleCommit);
			gpSrv->hExtConsoleCommit = in.RegExtCon.hCommitEvent;
			gpSrv->nExtConsolePID = in.hdr.nSrcPID;
		}
		else
		{
			if (gpSrv->hExtConsoleCommit == (HANDLE)in.RegExtCon.hCommitEvent)
			{
				CloseHandle(gpSrv->hExtConsoleCommit);
				gpSrv->hExtConsoleCommit = NULL;
				gpSrv->nExtConsolePID = 0;
			}
		}

		lbRc = TRUE;
	}

	int nOutSize = sizeof(CESERVER_REQ_HDR) + sizeof(DWORD);
	*out = ExecuteNewCmd(CECMD_REGEXTCONSOLE, nOutSize);

	if (*out != NULL)
	{
		(*out)->dwData[0] = lbRc;
	}

	return TRUE;
}

BOOL ProcessSrvCommand(CESERVER_REQ& in, CESERVER_REQ** out)
{
	BOOL lbRc = FALSE;
	MCHKHEAP;

	switch(in.hdr.nCmd)
	{
		//case CECMD_GETCONSOLEINFO:
		//case CECMD_REQUESTCONSOLEINFO:
		//{
		//	if (gpSrv->szGuiPipeName[0] == 0) { // ��������� ���� � CVirtualConsole ��� ������ ���� �������
		//		StringCchPrintf(gpSrv->szGuiPipeName, countof(gpSrv->szGuiPipeName), CEGUIPIPENAME, L".", (DWORD)ghConWnd); // ��� gnSelfPID
		//	}
		//	_ASSERT(ghConOut && ghConOut!=INVALID_HANDLE_VALUE);
		//	if (ghConOut==NULL || ghConOut==INVALID_HANDLE_VALUE)
		//		return FALSE;
		//	ReloadFullConsoleInfo(TRUE);
		//	MCHKHEAP;
		//	// �� ������ �� GUI (ProcessSrvCommand)
		//	if (in.hdr.nCmd == CECMD_GETCONSOLEINFO) {
		//		//*out = CreateConsoleInfo(NULL, (in.hdr.nCmd == CECMD_GETFULLINFO));
		//		int nOutSize = sizeof(CESERVER_REQ_HDR) + sizeof(CESERVER_CONSOLE_MAPPING_HDR);
		//		*out = ExecuteNewCmd(0,nOutSize);
		//		(*out)->ConInfo = gpSrv->pConsole->info;
		//	}
		//	MCHKHEAP;
		//	lbRc = TRUE;
		//} break;
		
		//case CECMD_SETSIZE:
		case CECMD_SETSIZESYNC:
		case CECMD_SETSIZENOSYNC:
		case CECMD_CMDSTARTED:
		case CECMD_CMDFINISHED:
		{
			lbRc = cmd_SetSizeXXX_CmdStartedFinished(in, out);
		} break;
		case CECMD_GETOUTPUT:
		{
			lbRc = cmd_GetOutput(in, out);
		} break;
		case CECMD_ATTACH2GUI:
		{
			lbRc = cmd_Attach2Gui(in, out);
		}  break;
		case CECMD_FARLOADED:
		{
			lbRc = cmd_FarLoaded(in, out);
		} break;
		case CECMD_POSTCONMSG:
		{
			lbRc = cmd_PostConMsg(in, out);
		} break;
		case CECMD_SAVEALIASES:
		{
			// ��������� ������
			lbRc = cmd_SaveAliases(in, out);
		} break;
		case CECMD_GETALIASES:
		{
			// ���������� ����������� ������
			lbRc = cmd_GetAliases(in, out);
		} break;
		case CECMD_SETDONTCLOSE:
		{
			// ����� ������ � ���� ������� (�������� dir) ����������, �����
			// ������� ���������� �� ���������...
			lbRc = cmd_SetDontClose(in, out);
		} break;
		case CECMD_ONACTIVATION:
		{
			lbRc = cmd_OnActivation(in, out);
		} break;
		case CECMD_SETWINDOWPOS:
		{
			//SetWindowPos(in.SetWndPos.hWnd, in.SetWndPos.hWndInsertAfter,
			//             in.SetWndPos.X, in.SetWndPos.Y, in.SetWndPos.cx, in.SetWndPos.cy,
			//             in.SetWndPos.uFlags);
			lbRc = cmd_SetWindowPos(in, out);
		} break;
		case CECMD_SETFOCUS:
		{
			lbRc = cmd_SetFocus(in, out);
		} break;
		case CECMD_SETPARENT:
		{
			lbRc = cmd_SetParent(in, out);
		} break;
		case CECMD_SETWINDOWRGN:
		{
			//MySetWindowRgn(&in.SetWndRgn);
			lbRc = cmd_SetWindowRgn(in, out);
		} break;
		case CECMD_DETACHCON:
		{
			lbRc = cmd_DetachCon(in, out);
		} break;
		case CECMD_CMDSTARTSTOP:
		{
			lbRc = cmd_CmdStartStop(in, out);
		} break;
		case CECMD_SETFARPID:
		{
			lbRc = cmd_SetFarPID(in, out);
		} break;
		case CECMD_GUICHANGED:
		{
			lbRc = cmd_GuiChanged(in, out);
		} break;
		case CECMD_TERMINATEPID:
		{
			lbRc = cmd_TerminatePid(in, out);
		} break;
		case CECMD_ATTACHGUIAPP:
		{
			lbRc = cmd_GuiAppAttached(in, out);
		} break;
		case CECMD_ALIVE:
		{
			*out = ExecuteNewCmd(CECMD_ALIVE, sizeof(CESERVER_REQ_HDR));
			lbRc = TRUE;
		} break;
		case CECMD_REGEXTCONSOLE:
		{
			lbRc = cmd_RegExtConsole(in, out);
		} break;
	}

	if (gbInRecreateRoot) gbInRecreateRoot = FALSE;

	return lbRc;
}







// ��������� ���������� ������� WinApi (GetConsoleScreenBufferInfo), �� � ������ �������:
// 1. ��������� (�� ���� ��������) ������������ ����������� ����
// 2. ������������ srWindow: ���������� �������������� ���������,
//    � ���� �� ��������� "�������� �����" - �� � ������������.
BOOL MyGetConsoleScreenBufferInfo(HANDLE ahConOut, PCONSOLE_SCREEN_BUFFER_INFO apsc)
{
	BOOL lbRc = FALSE, lbSetRc = TRUE;

	//CSection cs(NULL,NULL);
	//MSectionLock CSCS;
	//if (gnRunMode == RM_SERVER)
	//CSCS.Lock(&gpSrv->cChangeSize);
	//cs.Enter(&gpSrv->csChangeSize, &gpSrv->ncsTChangeSize);

	if (gnRunMode == RM_SERVER && ghConEmuWnd && IsWindow(ghConEmuWnd))  // ComSpec ���� ������ �� ������!
	{
		// ���� ���� �������� ����� ������������, ����� ���������� ���� ������ - ������ �������� �� �����
		if (IsZoomed(ghConWnd))
		{
			SendMessage(ghConWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
			DWORD dwStartTick = GetTickCount();

			do
			{
				Sleep(20); // �������� ����, �� �� ������ �������
			}
			while(IsZoomed(ghConWnd) && (GetTickCount()-dwStartTick)<=1000);

			Sleep(20); // � ��� ����-����, ����� ������� �����������
			// ������ ����� ������� (������ �� ���������) ������ ������ �������
			// ���� ����� �� ������� - ������ ������� ������ ���������
			RECT rcConPos;
			GetWindowRect(ghConWnd, &rcConPos);
			MoveWindow(ghConWnd, rcConPos.left, rcConPos.top, 1, 1, 1);

			if (gnBufferHeight == 0)
			{
				//specified width and height cannot be less than the width and height of the console screen buffer's window
				lbRc = SetConsoleScreenBufferSize(ghConOut, gcrVisibleSize);
			}
			else
			{
				// ������� ������ ��� BufferHeight
				COORD crHeight = {gcrVisibleSize.X, gnBufferHeight};
				MoveWindow(ghConWnd, rcConPos.left, rcConPos.top, 1, 1, 1);
				lbRc = SetConsoleScreenBufferSize(ghConOut, crHeight); // � �� crNewSize - ��� "�������" �������
			}

			// � ������� ��� ������� �������������, ������� ��� ������� � ��������� ��� (�������� ���)
			SetConsoleWindowInfo(ghConOut, TRUE, &gpSrv->sbi.srWindow);
		}
	}

	CONSOLE_SCREEN_BUFFER_INFO csbi = {{0,0}};
	lbRc = GetConsoleScreenBufferInfo(ahConOut, &csbi);
	// Issue 373: wmic
	if (lbRc)
	{
		TODO("��� ���� � �� ConEmu ���������");
		if (csbi.dwSize.X && (gnBufferWidth != csbi.dwSize.X))
			gnBufferWidth = csbi.dwSize.X;
		else if (gnBufferWidth == (csbi.srWindow.Right - csbi.srWindow.Left + 1))
			gnBufferWidth = 0;
	}

	if (GetConsoleDisplayMode(&gpSrv->dwDisplayMode) && gpSrv->dwDisplayMode)
	{
		_ASSERTE(!csbi.srWindow.Left && !csbi.srWindow.Top);
		csbi.dwSize.X = csbi.srWindow.Right+1;
		csbi.dwSize.Y = csbi.srWindow.Bottom+1;
	}

	//
	_ASSERTE((csbi.srWindow.Bottom-csbi.srWindow.Top)<200);

	if (lbRc && (gnRunMode == RM_SERVER || gnRunMode == RM_ALTSERVER))  // ComSpec ���� ������ �� ������!
	{
		// ���������� � SetConsoleSize
		//     if (gnBufferHeight) {
		//// ���� �� ����� � ������ BufferHeight - ����� ����������������� ������ (����� ��� ���� �������?)
		//         if (gnBufferHeight <= (csbi.dwMaximumWindowSize.Y * 12 / 10))
		//             gnBufferHeight = max(300, (SHORT)(csbi.dwMaximumWindowSize.Y * 12 / 10));
		//     }
		// ���� ��������� ���� �� ������ - �� ����������� ������ ��, ����� ��� ������� FAR
		// ���������� ������ � ������� �������
		BOOL lbNeedCorrect = FALSE, lbNeedUpdateSrvMap = FALSE;

		SHORT nWidth = (csbi.srWindow.Right - csbi.srWindow.Left + 1);
		SHORT nHeight = (csbi.srWindow.Bottom - csbi.srWindow.Top + 1);

		// ����������� ��������� ������ ������ ������� �������.
		// ������ ������ - ����� ������, �� �� ����� ��� ������� ������� �������
		if (gpSrv && !gpSrv->nRequestChangeSize && (gpSrv->crReqSizeNewSize.X > 0) && (gpSrv->crReqSizeNewSize.Y > 0))
		{
			COORD crMax = GetLargestConsoleWindowSize(ghConOut);
			// ��� ����� ���������, ���� ������������ ����� �������� ���������� ������
			if (crMax.X > 0 && crMax.X < gpSrv->crReqSizeNewSize.X)
			{
				gpSrv->crReqSizeNewSize.X = crMax.X;
				TODO("�������� gcrVisibleSize");
				//lbNeedUpdateSrvMap = TRUE; 
			}
			if (crMax.Y > 0 && crMax.Y < gpSrv->crReqSizeNewSize.Y)
			{
				gpSrv->crReqSizeNewSize.Y = crMax.Y;
				TODO("�������� gcrVisibleSize");
				//lbNeedUpdateSrvMap = TRUE;
			}

			WARNING("���� ������ ��������� �����. ��� ����������� � gpSrv->nRequestChangeSize.");
			//������, ����� ���������� �� � gpSrv->crReqSizeNewSize, � � gcrVisibleSize
#if 0
			if (nWidth != gpSrv->crReqSizeNewSize.X && gpSrv->crReqSizeNewSize.X <= )
			{
				
				csbi.srWindow.Right = min(csbi.dwSize.X, gpSrv->crReqSizeNewSize.X)-1;
				csbi.srWindow.Left = max(0, (csbi.srWindow.Right-gpSrv->crReqSizeNewSize.X+1));
				lbNeedCorrect = TRUE;
			}
			
			if (nHeight != gpSrv->crReqSizeNewSize.Y)
			{
				csbi.srWindow.Bottom = min(csbi.dwSize.Y, gpSrv->crReqSizeNewSize.Y)-1;
				csbi.srWindow.Top = max(0, (csbi.srWindow.Bottom-gpSrv->crReqSizeNewSize.Y+1));
				lbNeedCorrect = TRUE;
			}

			if (lbNeedUpdateSrvMap)
				UpdateConsoleMapHeader();
#endif
		}

#if 0
		// ����� �������
		if (csbi.srWindow.Left > 0)
		{
			lbNeedCorrect = TRUE; csbi.srWindow.Left = 0;
		}

		// ������������ ������ �������
		if (csbi.dwSize.X > csbi.dwMaximumWindowSize.X)
		{
			// ��� ����� ���������, ���� ������������ ����� �������� ���������� ������
			// ��� ���������� ���������� ����������� ��������� ������ ��������������� ������ (Issue 373: ������ ��� ������� wmic)
			lbNeedCorrect = TRUE;
			if (gpSrv 
				&& (gpSrv->crReqSizeNewSize.X > 0)
				&& gpSrv->crReqSizeNewSize.X <= csbi.dwMaximumWindowSize.X)
			{
				csbi.dwSize.X = gpSrv->crReqSizeNewSize.X;
			}
			else
			{
				csbi.dwSize.X = csbi.dwMaximumWindowSize.X;
			}
			csbi.srWindow.Right = (csbi.dwSize.X - 1);
		}

		if ((csbi.srWindow.Right+1) < csbi.dwSize.X)
		{
			lbNeedCorrect = TRUE; csbi.srWindow.Right = (csbi.dwSize.X - 1);
		}

		BOOL lbBufferHeight = FALSE;
		SHORT nHeight = (csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
		// 2010-11-19 ������� "12 / 10" �� "2"
		// 2010-12-12 ������� (csbi.dwMaximumWindowSize.Y * 2) �� (nHeight + 1)
		WARNING("���������, �� ����� �� ������� MyGetConsoleScreenBufferInfo ���� ����� ��������� ������ ������");

		if (csbi.dwSize.Y >= (nHeight + 1))
		{
			lbBufferHeight = TRUE;
		}
		else if (csbi.dwSize.Y > csbi.dwMaximumWindowSize.Y)
		{
			// ��� ����� ���������, ���� ������������ ����� �������� ���������� ������
			lbNeedCorrect = TRUE; csbi.dwSize.Y = csbi.dwMaximumWindowSize.Y;
		}

		if (!lbBufferHeight)
		{
			_ASSERTE((csbi.srWindow.Bottom-csbi.srWindow.Top)<200);

			if (csbi.srWindow.Top > 0)
			{
				lbNeedCorrect = TRUE; csbi.srWindow.Top = 0;
			}

			if ((csbi.srWindow.Bottom+1) < csbi.dwSize.Y)
			{
				lbNeedCorrect = TRUE; csbi.srWindow.Bottom = (csbi.dwSize.Y - 1);
			}
		}
#endif

		WARNING("CorrectVisibleRect ���� �������������, ��� ��� ����� ������ �� ������");

		//if (CorrectVisibleRect(&csbi))
		//	lbNeedCorrect = TRUE;
		if (lbNeedCorrect)
		{
			lbSetRc = SetConsoleWindowInfo(ghConOut, TRUE, &csbi.srWindow);
			_ASSERTE(lbSetRc);
			lbRc = GetConsoleScreenBufferInfo(ahConOut, &csbi);
		}
	}

	// ���������� (��������) ����������������� ������
	*apsc = csbi;
	//cs.Leave();
	//CSCS.Unlock();
#ifdef _DEBUG
#endif
	return lbRc;
}



// BufferHeight  - ������ ������ (0 - ��� ���������)
// crNewSize     - ������ ���� (������ ���� == ������ ������)
// rNewRect      - ��� (BufferHeight!=0) ���������� new upper-left and lower-right corners of the window
BOOL SetConsoleSize(USHORT BufferHeight, COORD crNewSize, SMALL_RECT rNewRect, LPCSTR asLabel)
{
	_ASSERTE(ghConWnd);

	if (!ghConWnd) return FALSE;

//#ifdef _DEBUG
//	if (gnRunMode != RM_SERVER || !gpSrv->bDebuggerActive)
//	{
//		BOOL bFarInExecute = WaitForSingleObject(ghFarInExecuteEvent, 0) == WAIT_OBJECT_0;
//		if (BufferHeight) {
//			if (!bFarInExecute) {
//				_ASSERTE(BufferHeight && bFarInExecute);
//			}
//		}
//	}
//#endif
	DWORD dwCurThId = GetCurrentThreadId();
	DWORD dwWait = 0;

	if (gnRunMode == RM_SERVER)
	{
		// �������� ��, ��� ��������� ��� ��������� ������. ����������
		gpSrv->nReqSizeBufferHeight = BufferHeight;
		gpSrv->crReqSizeNewSize = crNewSize;
		_ASSERTE(gpSrv->crReqSizeNewSize.X!=0);
		gpSrv->rReqSizeNewRect = rNewRect;
		gpSrv->sReqSizeLabel = asLabel;

		// ������ ��������� ������ � ���� RefreshThread. ������� ���� ���� ������ - ����...
		if (gpSrv->dwRefreshThread && dwCurThId != gpSrv->dwRefreshThread)
		{
			ResetEvent(gpSrv->hReqSizeChanged);
			gpSrv->nRequestChangeSize++;
			//dwWait = WaitForSingleObject(gpSrv->hReqSizeChanged, REQSIZE_TIMEOUT);
			// ��������, ���� ��������� RefreshThread
			HANDLE hEvents[2] = {ghQuitEvent, gpSrv->hReqSizeChanged};
			DWORD nSizeTimeout = REQSIZE_TIMEOUT;

			#ifdef _DEBUG
			if (IsDebuggerPresent())
				nSizeTimeout = INFINITE;
			#endif

			dwWait = WaitForMultipleObjects(2, hEvents, FALSE, nSizeTimeout);

			if (gpSrv->nRequestChangeSize > 0)
			{
				gpSrv->nRequestChangeSize --;
			}
			else
			{
				_ASSERTE(gpSrv->nRequestChangeSize>0);
			}

			if (dwWait == WAIT_OBJECT_0)
			{
				// ghQuitEvent !!
				return FALSE;
			}

			if (dwWait == (WAIT_OBJECT_0+1))
			{
				return gpSrv->bRequestChangeSizeResult;
			}

			// ?? ����� ���� ����� ����� �����������?
			return FALSE;
		}
	}

	//CSection cs(NULL,NULL);
	//MSectionLock CSCS;
	//if (gnRunMode == RM_SERVER)
	//	CSCS.Lock(&gpSrv->cChangeSize, TRUE, 10000);
	//    //cs.Enter(&gpSrv->csChangeSize, &gpSrv->ncsTChangeSize);

	if (ghLogSize) LogSize(&crNewSize, asLabel);

	_ASSERTE(crNewSize.X>=MIN_CON_WIDTH && crNewSize.Y>=MIN_CON_HEIGHT);

	// �������� ������������ �������
	if (crNewSize.X</*4*/MIN_CON_WIDTH)
		crNewSize.X = /*4*/MIN_CON_WIDTH;

	if (crNewSize.Y</*3*/MIN_CON_HEIGHT)
		crNewSize.Y = /*3*/MIN_CON_HEIGHT;

	BOOL lbNeedChange = TRUE;
	CONSOLE_SCREEN_BUFFER_INFO csbi = {{0,0}};

	if (MyGetConsoleScreenBufferInfo(ghConOut, &csbi))
	{
		// ������������ ������ ��� (gnBufferHeight == 0)
		lbNeedChange = (csbi.dwSize.X != crNewSize.X) || (csbi.dwSize.Y != crNewSize.Y)
			|| ((csbi.srWindow.Right - csbi.srWindow.Left + 1) != crNewSize.X)
			|| ((csbi.srWindow.Bottom - csbi.srWindow.Top + 1) != crNewSize.Y);
#ifdef _DEBUG

		if (!lbNeedChange)
			BufferHeight = BufferHeight;

#endif
	}


	// Minimum console size
	int curSizeY, curSizeX;
	wchar_t sFontName[LF_FACESIZE];
	if (apiGetConsoleFontSize(ghConOut, curSizeY, curSizeX, sFontName) && curSizeY && curSizeX)
	{
		int nMinY = GetSystemMetrics(SM_CYMIN) - GetSystemMetrics(SM_CYSIZEFRAME) - GetSystemMetrics(SM_CYCAPTION);
		int nMinX = GetSystemMetrics(SM_CXMIN) - 2*GetSystemMetrics(SM_CXSIZEFRAME);
		if ((nMinX > 0) && (nMinY > 0))
		{
			// ������ ���������, ����� ������ ������ ��� �����
			int minSizeY = (nMinY / curSizeY);
			int minSizeX = (nMinX / curSizeX);
			if ((minSizeX > crNewSize.X) || (minSizeY > crNewSize.Y))
			{
				apiFixFontSizeForBufferSize(ghConOut, crNewSize);
				apiGetConsoleFontSize(ghConOut, curSizeY, curSizeX, sFontName);
			}
		}
	}


	BOOL lbRc = TRUE;
	RECT rcConPos = {0};
	COORD crMax = GetLargestConsoleWindowSize(ghConOut);

	// ���� ������ ��������� ���������� - ����� ������ �� ������,
	// ����� ���������� ���������� ������ ��� ������� AltEnter:
	// ������ ���� ���������� ������ ������ ��� ���, �� FullScreen �� ����������
	//if (crMax.X && crNewSize.X > crMax.X)
	//	crNewSize.X = crMax.X;
	//if (crMax.Y && crNewSize.Y > crMax.Y)
	//	crNewSize.Y = crMax.Y;
	if ((crMax.X && crNewSize.X > crMax.X)
		|| (crMax.Y && crNewSize.Y > crMax.Y))
	{
		if (apiFixFontSizeForBufferSize(ghConOut, crNewSize))
		{
			crMax = GetLargestConsoleWindowSize(ghConOut);
			if ((crMax.X && crNewSize.X > crMax.X)
				|| (crMax.Y && crNewSize.Y > crMax.Y))
			{
				lbRc = FALSE;
				goto wrap;
			}
		}
		else
		{
			lbRc = FALSE;
			goto wrap;
		}
	}

	// ������ ��� ����� MyGetConsoleScreenBufferInfo, �.�. ��������� ��������� ������� ����
	// ��� ������ ������������ �� gnBufferHeight
	gnBufferHeight = BufferHeight;
	gcrVisibleSize = crNewSize;
	_ASSERTE(gcrVisibleSize.Y<200);
	
	if (gnRunMode == RM_SERVER || gnRunMode == RM_ALTSERVER)
		UpdateConsoleMapHeader(); // �������� pConsoleMap.crLockedVisible

	if (gnBufferHeight)
	{
		// � ������ BufferHeight - ������ ������ ���� ������ ����������� ������� ���� �������
		// ����� �� ���������� ��� ��������� "�������� �� ��� �����"...
		if (gnBufferHeight <= (csbi.dwMaximumWindowSize.Y * 12 / 10))
			gnBufferHeight = max(300, (SHORT)(csbi.dwMaximumWindowSize.Y * 12 / 10));

		// � ������ cmd ����� �������� ������������ FPS
		gpSrv->dwLastUserTick = GetTickCount() - USER_IDLE_TIMEOUT - 1;
	}

	//RECT rcConPos = {0};
	GetWindowRect(ghConWnd, &rcConPos);
	//BOOL lbRc = TRUE;
	//DWORD nWait = 0;

	//if (gpSrv->hChangingSize) {
	//    nWait = WaitForSingleObject(gpSrv->hChangingSize, 300);
	//    _ASSERTE(nWait == WAIT_OBJECT_0);
	//    ResetEvent(gpSrv->hChangingSize);
	//}

	// case: simple mode
	if (BufferHeight == 0)
	{
		if (lbNeedChange)
		{
			DWORD dwErr = 0;

			// ���� ����� �� ������� - ������ ������� ������ ���������
			//if (crNewSize.X < csbi.dwSize.X || crNewSize.Y < csbi.dwSize.Y)
			if (crNewSize.X <= (csbi.srWindow.Right-csbi.srWindow.Left) || crNewSize.Y <= (csbi.srWindow.Bottom-csbi.srWindow.Top))
			{
				//MoveWindow(ghConWnd, rcConPos.left, rcConPos.top, 1, 1, 1);
				rNewRect.Left = 0; rNewRect.Top = 0;
				rNewRect.Right = min((crNewSize.X - 1),(csbi.srWindow.Right-csbi.srWindow.Left));
				rNewRect.Bottom = min((crNewSize.Y - 1),(csbi.srWindow.Bottom-csbi.srWindow.Top));

				if (!SetConsoleWindowInfo(ghConOut, TRUE, &rNewRect))
					MoveWindow(ghConWnd, rcConPos.left, rcConPos.top, 1, 1, 1);
			}

			//specified width and height cannot be less than the width and height of the console screen buffer's window
			lbRc = SetConsoleScreenBufferSize(ghConOut, crNewSize);

			if (!lbRc) dwErr = GetLastError();

			//TODO: � ���� ������ ������ ���� ������� �� ������� ������?
			//WARNING("�������� ��� �����");
			//MoveWindow(ghConWnd, rcConPos.left, rcConPos.top, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 1);
			rNewRect.Left = 0; rNewRect.Top = 0;
			rNewRect.Right = crNewSize.X - 1;
			rNewRect.Bottom = crNewSize.Y - 1;
			SetConsoleWindowInfo(ghConOut, TRUE, &rNewRect);
		}
	}
	else
	{
		// ������� ������ ��� BufferHeight
		COORD crHeight = {crNewSize.X, BufferHeight};
		SMALL_RECT rcTemp = {0};

		if (!rNewRect.Top && !rNewRect.Bottom && !rNewRect.Right)
			rNewRect = csbi.srWindow;

		// ��������� �������� ������� �������
		if (csbi.dwSize.Y == (csbi.srWindow.Bottom - csbi.srWindow.Top + 1))
		{
			// ��������� ������ ���, ��������� .Top ��� ���������!
		}
		// ��� ��������� ������ ������ (���� �� ��� ��� �������), ����� ��������������� ����� ������� �������
		else if (rNewRect.Bottom >= (csbi.dwSize.Y - (csbi.srWindow.Bottom - csbi.srWindow.Top)))
		{
			// �������, ��� ������� ������� ������� � ���� ������. ����� ��������� .Top
			int nBottomLines = (csbi.dwSize.Y - csbi.srWindow.Bottom - 1); // ������� ����� ������ ����� �� ������� �������?
			SHORT nTop = BufferHeight - crNewSize.Y - nBottomLines;
			rNewRect.Top = (nTop > 0) ? nTop : 0;
			// .Bottom ����������� ����, ����� ��������� SetConsoleWindowInfo
		}
		else
		{
			// �������, ��� ���� ������� ������� ����������, ��������� �� ���������
		}

		// ���� ����� �� ������� - ������ ������� ������ ���������
		if (crNewSize.X <= (csbi.srWindow.Right-csbi.srWindow.Left)
		        || crNewSize.Y <= (csbi.srWindow.Bottom-csbi.srWindow.Top))
		{
			//MoveWindow(ghConWnd, rcConPos.left, rcConPos.top, 1, 1, 1);
			rcTemp.Left = 0;
			WARNING("� ��� ��������� ������, ����� ������ ������� ���� �����, Top ������� �� �����?");
			rcTemp.Top = max(0,(csbi.srWindow.Bottom-crNewSize.Y+1));
			rcTemp.Right = min((crNewSize.X - 1),(csbi.srWindow.Right-csbi.srWindow.Left));
			rcTemp.Bottom = min((BufferHeight - 1),(rcTemp.Top+crNewSize.Y-1));//(csbi.srWindow.Bottom-csbi.srWindow.Top)); //-V592

			if (!SetConsoleWindowInfo(ghConOut, TRUE, &rcTemp))
				MoveWindow(ghConWnd, rcConPos.left, rcConPos.top, 1, 1, 1);
		}

		//MoveWindow(ghConWnd, rcConPos.left, rcConPos.top, 1, 1, 1);
		//WaitForSingleObject(ghConOut, 200);
		//Sleep(100);
		/*if (gOSVer.dwMajorVersion == 6 && gOSVer.dwMinorVersion == 1) {
			const SHORT nMaxBuf = 600;
			if (crHeight.Y >= nMaxBuf)
				crHeight.Y = nMaxBuf;
		}*/
		lbRc = SetConsoleScreenBufferSize(ghConOut, crHeight); // � �� crNewSize - ��� "�������" �������
		//}
		//������ ���������� ������ �� ������!
		//RECT rcCurConPos = {0};
		//GetWindowRect(ghConWnd, &rcCurConPos); //X-Y �����, �� ������ - ������
		//MoveWindow(ghConWnd, rcCurConPos.left, rcCurConPos.top, GetSystemMetrics(SM_CXSCREEN), rcConPos.bottom-rcConPos.top, 1);
		// ��������� ��������� ������� �������.
		// ����� ������� - ������ 0 (�������������� ��������� �� ������������)
		// ������������ ��������� - ������ �� rNewRect.Top
		rNewRect.Left = 0;
		rNewRect.Right = crHeight.X-1;
		rNewRect.Bottom = min((crHeight.Y-1), (rNewRect.Top+gcrVisibleSize.Y-1)); //-V592
		_ASSERTE((rNewRect.Bottom-rNewRect.Top)<200);
		SetConsoleWindowInfo(ghConOut, TRUE, &rNewRect);
	}

	//if (gpSrv->hChangingSize) { // �� ����� ������� ConEmuC
	//    SetEvent(gpSrv->hChangingSize);
	//}

wrap:
	gpSrv->bRequestChangeSizeResult = lbRc;

	if ((gnRunMode == RM_SERVER) && gpSrv->hRefreshEvent)
	{
		//gpSrv->bForceFullSend = TRUE;
		SetEvent(gpSrv->hRefreshEvent);
	}

	//cs.Leave();
	//CSCS.Unlock();
	return lbRc;
}

#if defined(__GNUC__)
extern "C" {
	BOOL WINAPI HandlerRoutine(DWORD dwCtrlType);
};
#endif

BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{
	//PRINT_COMSPEC(L"HandlerRoutine triggered. Event type=%i\n", dwCtrlType);
	if ((dwCtrlType == CTRL_CLOSE_EVENT)
		|| (dwCtrlType == CTRL_LOGOFF_EVENT)
		|| (dwCtrlType == CTRL_SHUTDOWN_EVENT))
	{
		PRINT_COMSPEC(L"Console about to be closed\n", 0);
		WARNING("��� �� ��������� �������, ���� ������ ���������� �������� ����������... � �� ��� ����������� ������ �������");
		gbInShutdown = TRUE;
		
		// ���������� ��������, ����� ������������ ������� ���� ����������
		if (gpSrv->bDebuggerActive)
		{
			if (pfnDebugActiveProcessStop) pfnDebugActiveProcessStop(gpSrv->dwRootProcess);

			gpSrv->bDebuggerActive = FALSE;
		}
		else
		{
			// trick to let ConsoleMain2() finish correctly
			ExitThread(1);
			//return TRUE;
		
			//#ifdef _DEBUG
			//wchar_t szTitle[128]; wsprintf(szTitle, L"ConEmuC, PID=%u", GetCurrentProcessId());
			////MessageBox(NULL, L"CTRL_CLOSE_EVENT in ConEmuC", szTitle, MB_SYSTEMMODAL);
			//#endif
			//DWORD nWait = WaitForSingleObject(ghExitQueryEvent, CLOSE_CONSOLE_TIMEOUT);
			//if (nWait == WAIT_OBJECT_0)
			//{
			//	#ifdef _DEBUG
			//	OutputDebugString(L"All console processes was terminated\n");
			//	#endif
			//}
			//else
			//{
			//	// ��������� �� (������) ������ ��������, �� ����� ��������
			//	// ���������� ����, ��� � ��� �������� �����-�� ��������
			//	EmergencyShow();
			//}
		}
	}
	else if ((dwCtrlType == CTRL_C_EVENT) || (dwCtrlType == CTRL_BREAK_EVENT))
	{
		if (gbTerminateOnCtrlBreak)
		{
			PRINT_COMSPEC(L"Ctrl+Break recieved, server will be terminated\n", 0);
			gbInShutdown = TRUE;
		}
		else if (gbDebugProcess)
		{
			DWORD nWait = WaitForSingleObject(gpSrv->hRootProcess, 0);
			#ifdef _DEBUG
			DWORD nErr = GetLastError();
			#endif
			if (nWait == WAIT_OBJECT_0)
			{
				_ASSERTE(gbTerminateOnCtrlBreak==FALSE);
				PRINT_COMSPEC(L"Ctrl+Break recieved, debgger will be stopped\n", 0);
				//if (pfnDebugActiveProcessStop) pfnDebugActiveProcessStop(gpSrv->dwRootProcess);
				//gpSrv->bDebuggerActive = FALSE;
				//gbInShutdown = TRUE;
				SetEvent(ghExitQueryEvent);
			}
			else
			{
				HMODULE hKernel = GetModuleHandle(L"kernel32.dll");
				typedef BOOL (WINAPI* DebugBreakProcess_t)(HANDLE Process);
				DebugBreakProcess_t DebugBreakProcess_f = (DebugBreakProcess_t)(hKernel ? GetProcAddress(hKernel, "DebugBreakProcess") : NULL);
				if (DebugBreakProcess_f)
				{
					_printf("ConEmuC: Sending DebugBreak event to process\n");
					gpSrv->bDebuggerRequestDump = TRUE;
					DWORD dwErr = 0;
					if (!DebugBreakProcess_f(gpSrv->hRootProcess))
					{
						dwErr = GetLastError();
						//_ASSERTE(FALSE && dwErr==0);
						_printf("ConEmuC: Sending DebugBreak event failed, Code=x%X, WriteMiniDump on the fly\n", dwErr);
						gpSrv->bDebuggerRequestDump = FALSE;
						WriteMiniDump(gpSrv->dwRootThread, NULL);
					}
				}
				else
				{
					_printf("ConEmuC: DebugBreakProcess not found in kernel32.dll\n");
				}
			}
		}
	}

	/*SafeCloseHandle(ghLogSize);
	if (wpszLogSizeFile) {
		DeleteFile(wpszLogSizeFile);
		free(wpszLogSizeFile); wpszLogSizeFile = NULL;
	}*/
	return TRUE;
}

int GetProcessCount(DWORD *rpdwPID, UINT nMaxCount)
{
	if (!rpdwPID || !nMaxCount)
	{
		_ASSERTE(rpdwPID && nMaxCount);
		return gpSrv->nProcessCount;
	}

	MSectionLock CS;
#ifdef _DEBUG

	if (!CS.Lock(gpSrv->csProc, FALSE, 200))
#else
	if (!CS.Lock(gpSrv->csProc, TRUE, 200))
#endif
	{
		// ���� �� ������� ������������� ���������� - ������ ������ ����
		*rpdwPID = gnSelfPID;
		return 1;
	}

	UINT nSize = gpSrv->nProcessCount;

	if (nSize > nMaxCount)
	{
		memset(rpdwPID, 0, sizeof(DWORD)*nMaxCount);
		rpdwPID[0] = gnSelfPID;

		for(int i1=0, i2=(nMaxCount-1); i1<(int)nSize && i2>0; i1++, i2--)
			rpdwPID[i2] = gpSrv->pnProcesses[i1]; //-V108

		nSize = nMaxCount;
	}
	else
	{
		memmove(rpdwPID, gpSrv->pnProcesses, sizeof(DWORD)*nSize);

		for(UINT i=nSize; i<nMaxCount; i++)
			rpdwPID[i] = 0; //-V108
	}

	_ASSERTE(rpdwPID[0]);
	return nSize;
	/*
	//DWORD dwErr = 0; BOOL lbRc = FALSE;
	DWORD *pdwPID = NULL; int nCount = 0, i;
	EnterCriticalSection(&gpSrv->csProc);
	nCount = gpSrv->nProcesses.size();
	if (nCount > 0 && rpdwPID) {
		pdwPID = (DWORD*)calloc(nCount, sizeof(DWORD));
		_ASSERTE(pdwPID!=NULL);
		if (pdwPID) {
			std::vector<DWORD>::iterator iter = gpSrv->nProcesses.begin();
			i = 0;
			while (iter != gpSrv->nProcesses.end()) {
				pdwPID[i++] = *iter;
				iter ++;
			}
		}
	}
	LeaveCriticalSection(&gpSrv->csProc);
	if (rpdwPID)
		*rpdwPID = pdwPID;
	return nCount;
	*/
}

#ifdef CRTPRINTF
WARNING("����� ���������... �������� �� wvsprintf");
//void _printf(LPCSTR asFormat, ...)
//{
//    va_list argList; va_start(argList, an_StrResId);
//    char szError[2000]; -- ������ ����� ������ ����� %s
//    wvsprintf(szError, asFormat, argList);
//}

void _printf(LPCSTR asFormat, DWORD dw1, DWORD dw2, LPCWSTR asAddLine)
{
	char szError[MAX_PATH];
	_wsprintfA(szError, SKIPLEN(countof(szError)) asFormat, dw1, dw2);
	_printf(szError);

	if (asAddLine)
	{
		_wprintf(asAddLine);
		_printf("\n");
	}
}

void _printf(LPCSTR asFormat, DWORD dwErr, LPCWSTR asAddLine)
{
	char szError[MAX_PATH];
	_wsprintfA(szError, SKIPLEN(countof(szError)) asFormat, dwErr);
	_printf(szError);
	_wprintf(asAddLine);
	_printf("\n");
}

void _printf(LPCSTR asFormat, DWORD dwErr)
{
	char szError[MAX_PATH];
	_wsprintfA(szError, SKIPLEN(countof(szError)) asFormat, dwErr);
	_printf(szError);
}

void _printf(LPCSTR asBuffer)
{
	if (!asBuffer) return;

	int nAllLen = lstrlenA(asBuffer);
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwWritten = 0;
	WriteConsoleA(hOut, asBuffer, nAllLen, &dwWritten, 0);
}

#endif

void _wprintf(LPCWSTR asBuffer)
{
	if (!asBuffer) return;

	int nAllLen = lstrlenW(asBuffer);
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwWritten = 0;
	WriteConsoleW(hOut, asBuffer, nAllLen, &dwWritten, 0);
	
	//UINT nOldCP = GetConsoleOutputCP();
	//char* pszOEM = (char*)malloc(nAllLen+1);
	//
	//if (pszOEM)
	//{
	//	WideCharToMultiByte(nOldCP,0, asBuffer, nAllLen, pszOEM, nAllLen, 0,0);
	//	pszOEM[nAllLen] = 0;
	//	WriteFile(hOut, pszOEM, nAllLen, &dwWritten, 0);
	//	free(pszOEM);
	//}
	//else
	//{
	//	WriteFile(hOut, asBuffer, nAllLen*2, &dwWritten, 0);
	//}
}

void DisableAutoConfirmExit(BOOL abFromFarPlugin)
{
	_ASSERTE(gnConfirmExitParm==0 || abFromFarPlugin);
	gbAutoDisableConfirmExit = FALSE; gbAlwaysConfirmExit = FALSE;
	// ������ nProcessStartTick �� �����. �������� ������ �� �������
	//gpSrv->nProcessStartTick = GetTickCount() - 2*CHECK_ROOTSTART_TIMEOUT;
}

//BOOL IsUserAdmin()
//{
//	OSVERSIONINFO osv = {sizeof(OSVERSIONINFO)};
//	GetVersionEx(&osv);
//	// ��������� ����� ������ ��� �����, ����� �� XP ������ "���" �� �����������
//	if (osv.dwMajorVersion < 6)
//		return FALSE;
//
//	BOOL b;
//	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
//	PSID AdministratorsGroup;
//
//	b = AllocateAndInitializeSid(
//		&NtAuthority,
//		2,
//		SECURITY_BUILTIN_DOMAIN_RID,
//		DOMAIN_ALIAS_RID_ADMINS,
//		0, 0, 0, 0, 0, 0,
//		&AdministratorsGroup);
//	if (b)
//	{
//		if (!CheckTokenMembership(NULL, AdministratorsGroup, &b))
//		{
//			b = FALSE;
//		}
//		FreeSid(AdministratorsGroup);
//	}
//	return(b);
//}

//WARNING("BUGBUG: x64 US-Dvorak"); - done
void CheckKeyboardLayout()
{
	if (pfnGetConsoleKeyboardLayoutName)
	{
		wchar_t szCurKeybLayout[32];

		//#ifdef _DEBUG
		//wchar_t szDbgKeybLayout[KL_NAMELENGTH/*==9*/];
		//BOOL lbGetRC = GetKeyboardLayoutName(szDbgKeybLayout); // -- �� ���� �������, ��������� "�� �������", � �� �� �������
		//#endif
		// ���������� ������ � ���� "00000419" -- ����� ��� 16 ����?
		if (pfnGetConsoleKeyboardLayoutName(szCurKeybLayout))
		{
			if (lstrcmpW(szCurKeybLayout, gpSrv->szKeybLayout))
			{
#ifdef _DEBUG
				wchar_t szDbg[128];
				_wsprintf(szDbg, SKIPLEN(countof(szDbg))
				          L"ConEmuC: InputLayoutChanged (GetConsoleKeyboardLayoutName returns) '%s'\n",
				          szCurKeybLayout);
				OutputDebugString(szDbg);
#endif

				if (ghLogSize)
				{
					char szInfo[128]; wchar_t szWide[128];
					_wsprintf(szWide, SKIPLEN(countof(szWide)) L"ConEmuC: ConsKeybLayout changed from %s to %s", gpSrv->szKeybLayout, szCurKeybLayout);
					WideCharToMultiByte(CP_ACP,0,szWide,-1,szInfo,128,0,0);
					LogString(szInfo);
				}

				// ��������
				wcscpy_c(gpSrv->szKeybLayout, szCurKeybLayout);
				// ������� � GUI
				wchar_t *pszEnd = szCurKeybLayout+8;
				//WARNING("BUGBUG: 16 ���� �� ������"); -- ��� ������ 8 ����. ��� LayoutNAME, � �� string(HKL)
				// LayoutName: "00000409", "00010409", ...
				// � HKL �� ���� ����������, ��� ��� �������� DWORD
				// HKL � x64 �������� ���: "0x0000000000020409", "0xFFFFFFFFF0010409"
				DWORD dwLayout = wcstoul(szCurKeybLayout, &pszEnd, 16);
				CESERVER_REQ* pIn = ExecuteNewCmd(CECMD_LANGCHANGE,sizeof(CESERVER_REQ_HDR)+sizeof(DWORD)); //-V119

				if (pIn)
				{
					//memmove(pIn->Data, &dwLayout, 4);
					pIn->dwData[0] = dwLayout;
					CESERVER_REQ* pOut = NULL;
					pOut = ExecuteGuiCmd(ghConWnd, pIn, ghConWnd);

					if (pOut) ExecuteFreeResult(pOut);

					ExecuteFreeResult(pIn);
				}
			}
		}
	}
}


/*
LPVOID calloc(size_t nCount, size_t nSize)
{
	#ifdef _DEBUG
	//HeapValidate(ghHeap, 0, NULL);
	#endif

	size_t nWhole = nCount * nSize;
	_ASSERTE(nWhole>0);
	LPVOID ptr = HeapAlloc ( ghHeap, HEAP_GENERATE_EXCEPTIONS|HEAP_ZERO_MEMORY, nWhole );

	#ifdef HEAP_LOGGING
	wchar_t szDbg[64];
	_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"%i: ALLOCATED   0x%08X..0x%08X   (%i bytes)\n", GetCurrentThreadId(), (DWORD)ptr, ((DWORD)ptr)+nWhole, nWhole);
	DEBUGSTR(szDbg);
	#endif

	#ifdef _DEBUG
	HeapValidate(ghHeap, 0, NULL);
	if (ptr)
	{
		gnHeapUsed += nWhole;
		if (gnHeapMax < gnHeapUsed)
			gnHeapMax = gnHeapUsed;
	}
	#endif

	return ptr;
}

void free(LPVOID ptr)
{
	if (ptr && ghHeap)
	{
		#ifdef _DEBUG
		//HeapValidate(ghHeap, 0, NULL);
		size_t nMemSize = HeapSize(ghHeap, 0, ptr);
		#endif
		#ifdef HEAP_LOGGING
		wchar_t szDbg[64];
		_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"%i: FREE BLOCK  0x%08X..0x%08X   (%i bytes)\n", GetCurrentThreadId(), (DWORD)ptr, ((DWORD)ptr)+nMemSize, nMemSize);
		DEBUGSTR(szDbg);
		#endif

		HeapFree ( ghHeap, 0, ptr );

		#ifdef _DEBUG
		HeapValidate(ghHeap, 0, NULL);
		if (gnHeapUsed > nMemSize)
			gnHeapUsed -= nMemSize;
		#endif
	}
}
*/

/* ������������ ��� extern � ConEmuCheck.cpp */
/*
LPVOID _calloc(size_t nCount,size_t nSize) {
	return calloc(nCount,nSize);
}
LPVOID _malloc(size_t nCount) {
	return calloc(nCount,1);
}
void   _free(LPVOID ptr) {
	free(ptr);
}
*/

/*
void * __cdecl operator new(size_t _Size)
{
	void * p = calloc(_Size,1);
	return p;
}

void __cdecl operator delete(void *p)
{
	free(p);
}
*/
