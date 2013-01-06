
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
//  �����������������, ����� ����� ����� �������� ������ �������� MessageBox, ����� ����������� ����������
//	#define SHOW_STARTED_MSGBOX
//	#define SHOW_INJECT_MSGBOX
	#define SHOW_EXE_MSGBOX // �������� ��������� ��� �������� � ������������ exe-���� (SHOW_EXE_MSGBOX_NAME)
	#define SHOW_EXE_MSGBOX_NAME L"mode.exe"
//	#define SHOW_EXE_TIMINGS
#endif
//#define SHOW_INJECT_MSGBOX
//#define SHOW_STARTED_MSGBOX


#undef SHOW_SHUTDOWN_STEPS
#ifdef _DEBUG
	#define SHOW_SHUTDOWN_STEPS
#endif


#ifdef _DEBUG
	//#define UseDebugExceptionFilter
	#undef UseDebugExceptionFilter
#else
	#undef UseDebugExceptionFilter
#endif

//#ifdef _DEBUG
#define USE_PIPE_SERVER
//#else
//	#undef USE_PIPE_SERVER
//#endif

#include <windows.h>

#ifndef TESTLINK
#include "../common/common.hpp"
#include "../common/ConEmuCheck.h"
#include "../common/execute.h"
#endif
#include "../common/PipeServer.h"
#include "../common/ConEmuInOut.h"

#ifdef _DEBUG
#include "DbgHooks.h"
#endif

#include "ConEmuHooks.h"
#include "RegHooks.h"
#include "ShellProcessor.h"
#include "UserImp.h"
#include "GuiAttach.h"
#include "Injects.h"
#include "../ConEmuCD/ExitCodes.h"
#include "../common/ConsoleAnnotation.h"

#undef FULL_STARTUP_ENV
#include "../common/StartupEnv.h"


#if defined(_DEBUG) || defined(SHOW_EXE_TIMINGS)
DWORD gnLastShowExeTick = 0;
#endif

#ifdef SHOW_EXE_TIMINGS
#define print_timings(s) if (gbShowExeMsgBox) { \
	DWORD w, nCurTick = GetTickCount(); \
	msprintf(szTimingMsg, countof(szTimingMsg), L">>> %s >>> %u >>> %s\n", SHOW_EXE_MSGBOX_NAME, (nCurTick - gnLastShowExeTick), s); \
	OnWriteConsoleW(hTimingHandle, szTimingMsg, lstrlen(szTimingMsg), &w, NULL); \
	gnLastShowExeTick = nCurTick; \
	}
#else
#define print_timings(s)
#endif


#if defined(__GNUC__)
extern "C" {
	BOOL WINAPI DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved);
#endif
	//__declspec(dllexport) HHOOK ghKeyHook = 0;
	//__declspec(dllexport) DWORD gnVkWinFix = 0xF0;
	//__declspec(dllexport) HWND  ghKeyHookConEmuRoot = NULL;
#if defined(__GNUC__)
};
#endif

//__declspec(dllexport) HHOOK ghKeyHook = 0;
//__declspec(dllexport) DWORD gnVkWinFix = 0xF0;
//__declspec(dllexport) HWND  ghKeyHookConEmuRoot = NULL;

//HHOOK ghKeyHook = 0;
//DWORD gnVkWinFix = 0xF0;
//HWND  ghKeyHookConEmuRoot = NULL;

extern HMODULE ghOurModule;
//HMODULE ghOurModule = NULL; // ConEmu.dll - ��� ������
//UINT gnMsgActivateCon = 0; //RegisterWindowMessage(CONEMUMSG_LLKEYHOOK);
//SECURITY_ATTRIBUTES* gpLocalSecurity = NULL;

#define isPressed(inp) ((GetKeyState(inp) & 0x8000) == 0x8000)

extern DWORD  gnHookMainThreadId;
extern HANDLE ghHeap;
//extern HMODULE ghKernel32, ghUser32, ghShell32, ghAdvapi32, ghComdlg32;
extern HMODULE ghUser32;
//extern const wchar_t *kernel32;// = L"kernel32.dll";
extern const wchar_t *user32  ;// = L"user32.dll";
//extern const wchar_t *shell32 ;// = L"shell32.dll";
//extern const wchar_t *advapi32;// = L"Advapi32.dll";
//extern const wchar_t *comdlg32;// = L"comdlg32.dll";
extern bool gbHookExecutableOnly;
extern DWORD gnAllowClinkUsage;

ConEmuHkDllState gnDllState = ds_Undefined;
int gnDllThreadCount = 0;
BOOL gbDllStopCalled = FALSE;

//BOOL gbSkipInjects = FALSE;
BOOL gbHooksWasSet = false;
BOOL gbDllDeinitialized = false; 

extern BOOL StartupHooks(HMODULE ahOurDll);
extern void ShutdownHooks();
extern void InitializeHookedModules();
extern void FinalizeHookedModules();
extern DWORD GetMainThreadId(bool bUseCurrentAsMain);
//HMODULE ghPsApi = NULL;
#ifdef _DEBUG
extern HHOOK ghGuiClientRetHook;
//extern bool gbAllowAssertThread;
#endif

CEStartupEnv* gpStartEnv = NULL;
DWORD   gnSelfPID = 0;
BOOL    gbSelfIsRootConsoleProcess = FALSE;
DWORD   gnServerPID = 0;
DWORD   gnPrevAltServerPID = 0;
BOOL    gbWasSucceededInRead = FALSE;
DWORD   gnGuiPID = 0;
HWND    ghConWnd = NULL; // Console window
HWND    ghConEmuWnd = NULL; // Root! window
HWND    ghConEmuWndDC = NULL; // ConEmu DC window
HWND    ghConEmuWndBack = NULL; // ConEmu Back window - holder for GUI client
void    SetConEmuHkWindows(HWND hDcWnd, HWND hBackWnd);
BOOL    gbWasBufferHeight = FALSE;
BOOL    gbNonGuiMode = FALSE;
DWORD   gnImageSubsystem = 0;
DWORD   gnImageBits = WIN3264TEST(32,64); //-V112
wchar_t gsInitConTitle[512] = {};

ConEmuInOutPipe *gpCEIO_In = NULL, *gpCEIO_Out = NULL, *gpCEIO_Err = NULL;
void StartPTY();
void StopPTY();

HMODULE ghSrvDll = NULL;
//typedef int (__stdcall* RequestLocalServer_t)(AnnotationHeader** ppAnnotation, HANDLE* ppOutBuffer);
RequestLocalServer_t gfRequestLocalServer = NULL;
TODO("AnnotationHeader* gpAnnotationHeader");
AnnotationHeader* gpAnnotationHeader = NULL;
HANDLE ghCurrentOutBuffer = NULL; // ��������������� ��� SetConsoleActiveScreenBuffer

bool IsAnsiCapable(HANDLE hFile, bool* bIsConsoleOutput = NULL);


#ifdef USEPIPELOG
namespace PipeServerLogger
{
    Event g_events[BUFFER_SIZE];
    LONG g_pos = -1;
}
#endif

#ifdef USEHOOKLOG
namespace HookLogger
{
	Event g_events[BUFFER_SIZE];
	LONG g_pos = -1;
	LARGE_INTEGER g_freq = {0};
	CritInfo g_crit[CRITICAL_BUFFER_SIZE];

	void RunAnalyzer()
	{
		ZeroStruct(g_crit);
		LONG iFrom = 0, iTo = min(BUFFER_SIZE,(ULONG)g_pos);
		for (LONG i = iFrom; i < iTo; ++i)
		{
			Event* e = g_events + i;
			if (!e->cntr1.QuadPart)
				continue;
			e->dur = (DWORD)(e->cntr1.QuadPart - e->cntr.QuadPart);

			LONG j = 0;
			while (j < CRITICAL_BUFFER_SIZE)
			{
				if (!g_crit[j].msg || g_crit[j].msg == e->msg)
					break;
				j++;
			}

			if (j < CRITICAL_BUFFER_SIZE)
			{
				g_crit[j].msg = e->msg;
				g_crit[j].count++;
				g_crit[j].total += e->dur;
			}
		}

		// Sort for clear analyzing
		for (LONG i = 0; i < (CRITICAL_BUFFER_SIZE-1); i++)
		{
			if (!g_crit[i].count) break;
			LONG m = i;
			for (LONG j = i+1; i < CRITICAL_BUFFER_SIZE; j++)
			{
				if (!g_crit[j].count) break;
				if (g_crit[j].total > g_crit[m].total)
					m = j;
			}
			if (m != i)
			{
				CritInfo c = g_crit[m];
				g_crit[m] = g_crit[i];
				g_crit[i] = c;
			}
		}
	}
}
#endif

#ifdef _DEBUG
	#ifdef UseDebugExceptionFilter
		LPTOP_LEVEL_EXCEPTION_FILTER gfnPrevFilter = NULL;
		LONG WINAPI HkExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo);
	#endif
#endif

void SendStarted();
void SendStopped();

/*
void __stdcall _chkstk()
{
	return;
}
*/

#ifdef SHOW_SHUTDOWN_STEPS
static int gnDbgPresent = 0;
void ShutdownStep(LPCWSTR asInfo, int nParm1 = 0, int nParm2 = 0, int nParm3 = 0, int nParm4 = 0)
{
	if (!gnDbgPresent)
		gnDbgPresent = IsDebuggerPresent() ? 1 : 2;
	if (gnDbgPresent != 1)
		return;
	wchar_t szFull[512];
	msprintf(szFull, countof(szFull), L"%u:ConEmuH:PID=%u:TID=%u: ",
		GetTickCount(), GetCurrentProcessId(), GetCurrentThreadId());
	if (asInfo)
	{
		int nLen = lstrlen(szFull);
		msprintf(szFull+nLen, countof(szFull)-nLen, asInfo, nParm1, nParm2, nParm3, nParm4);
	}
	lstrcat(szFull, L"\n");
	OutputDebugString(szFull);
}
#else
void ShutdownStep(LPCWSTR asInfo, int nParm1 = 0, int nParm2 = 0, int nParm3 = 0, int nParm4 = 0)
{
}
#endif


#ifdef HOOK_USE_DLLTHREAD
HANDLE ghStartThread = NULL;
DWORD  gnStartThreadID = 0;
#endif

void SetConEmuHkWindows(HWND hDcWnd, HWND hBackWnd)
{
	ghConEmuWndDC = hDcWnd;
	ghConEmuWndBack = hBackWnd;
}


MFileMapping<CESERVER_CONSOLE_MAPPING_HDR> *gpConMap = NULL;
CESERVER_CONSOLE_MAPPING_HDR* gpConInfo = NULL;

CESERVER_CONSOLE_MAPPING_HDR* GetConMap(BOOL abForceRecreate/*=FALSE*/)
{
	static bool bLastAnsi = false;
	bool bAnsi = false;

	if (gpConInfo && !abForceRecreate)
		goto wrap;
	
	if (!gpConMap || abForceRecreate)
	{
		if (!gpConMap)
			gpConMap = new MFileMapping<CESERVER_CONSOLE_MAPPING_HDR>;
		if (!gpConMap)
		{
			gpConInfo = NULL;
			goto wrap;
		}
		gpConMap->InitName(CECONMAPNAME, (DWORD)ghConWnd); //-V205
	}
	
	if (!gpConInfo || abForceRecreate)
	{
		gpConInfo = gpConMap->Open();
	}
	
	if (gpConInfo)
	{
		if (gpConInfo->cbSize >= sizeof(CESERVER_CONSOLE_MAPPING_HDR))
		{
			gnGuiPID = gpConInfo->nGuiPID;
			ghConEmuWnd = gpConInfo->hConEmuRoot;
			_ASSERTE(ghConEmuWnd==NULL || gnGuiPID!=0);
			SetConEmuHkWindows(gpConInfo->hConEmuWndDc, gpConInfo->hConEmuWndBack);
			_ASSERTE(ghConEmuWndDC && user->isWindow(ghConEmuWndDC));
			_ASSERTE(ghConEmuWndBack && user->isWindow(ghConEmuWndBack));
			gnServerPID = gpConInfo->nServerPID;
		}
		else
		{
			_ASSERTE(gpConInfo->cbSize == sizeof(CESERVER_CONSOLE_MAPPING_HDR));
			gpConMap->CloseMap();
			gpConInfo = NULL;
			delete gpConMap;
			gpConMap = NULL;
		}
	}
	else
	{
		delete gpConMap;
		gpConMap = NULL;
	}
	
wrap:
	bAnsi = ((gpConInfo != NULL) && (gpConInfo->bProcessAnsi != FALSE));
	if (abForceRecreate || (bLastAnsi != bAnsi))
	{
		bLastAnsi = bAnsi;
		SetEnvironmentVariable(ENV_CONEMUANSI_VAR_W, bAnsi ? L"ON" : L"OFF");
	}
	return gpConInfo;
}

void OnConWndChanged(HWND ahNewConWnd)
{
	//BOOL lbForceReopen = FALSE;

	if (ahNewConWnd)
	{
		#ifdef _DEBUG
		if (user)
		{
			wchar_t sClass[64]; user->getClassNameW(ahNewConWnd, sClass, countof(sClass));
			_ASSERTEX(isConsoleClass(sClass));
		}
		#endif

		if (ghConWnd != ahNewConWnd)
		{
			ghConWnd = ahNewConWnd;
			//lbForceReopen = TRUE;
		}
	}
	else
	{
		//lbForceReopen = TRUE;
	}

	GetConMap(TRUE);
}

#ifdef USE_PIPE_SERVER
BOOL WINAPI HookServerCommand(LPVOID pInst, CESERVER_REQ* pCmd, CESERVER_REQ* &ppReply, DWORD &pcbReplySize, DWORD &pcbMaxReplySize, LPARAM lParam);
BOOL WINAPI HookServerReady(LPVOID pInst, LPARAM lParam);
void WINAPI HookServerFree(CESERVER_REQ* pReply, LPARAM lParam);

PipeServer<CESERVER_REQ> *gpHookServer = NULL;
#endif

bool gbShowExeMsgBox = false;

void InitDefaultTerm()
{
	gpDefaultTermParm = (ConEmuGuiMapping*)calloc(sizeof(*gpDefaultTermParm),1);

	CShellProc sp;
	sp.LoadSrvMapping(TRUE);
}

DWORD WINAPI DllStart(LPVOID /*apParm*/)
{
	//DLOG0("DllStart",0);

	wchar_t *szModule = (wchar_t*)calloc((MAX_PATH+1),sizeof(wchar_t));
	if (!GetModuleFileName(NULL, szModule, MAX_PATH+1))
		_wcscpy_c(szModule, MAX_PATH+1, L"GetModuleFileName failed");
	const wchar_t* pszName = PointToName(szModule);
	wchar_t szMsg[128];

	// For reporting purposes. Users may define env.var and run program.
	// When ConEmuHk.dll loaded in that process - it'll show msg box
	// Example (for cmd.exe prompt):
	// set ConEmuReportExe=sh.exe
	// sh.exe --login -i
	if (pszName && GetEnvironmentVariableW(ENV_CONEMUREPORTEXE_VAR_W, szMsg, countof(szMsg)) && *szMsg)
	{
		if (lstrcmpi(szMsg, pszName) == 0)
		{
			gbShowExeMsgBox = true;
		}
	}

	#if defined(SHOW_EXE_TIMINGS) || defined(SHOW_EXE_MSGBOX)
		wchar_t szTimingMsg[512]; UNREFERENCED_PARAMETER(szTimingMsg);
		HANDLE hTimingHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		if (!lstrcmpi(pszName, SHOW_EXE_MSGBOX_NAME))
		{
			gbShowExeMsgBox = true;
		}
	#endif


	// *******************  begin  *********************

	print_timings(L"DllStart: InitializeHookedModules");
	InitializeHookedModules();

	//HANDLE hStartedEvent = (HANDLE)apParm;


	if (gbShowExeMsgBox)
	{
		STARTUPINFO si = {sizeof(si)};
		GetStartupInfo(&si);
		LPCWSTR pszCmd = GetCommandLineW();
		// GuiMessageBox ��� �� ��������, ������ �� ����������������
		HMODULE hUser = LoadLibrary(user32);
		typedef int (WINAPI* MessageBoxW_t)(HWND hWnd,LPCTSTR lpText,LPCTSTR lpCaption,UINT uType);
		if (hUser)
		{
			MessageBoxW_t MsgBox = (MessageBoxW_t)GetProcAddress(hUser, "MessageBoxW");
			if (MsgBox)
			{
				lstrcpyn(szMsg, pszName, 96); lstrcat(szMsg, L" loaded!");
				wchar_t szTitle[64]; msprintf(szTitle, countof(szTitle), L"ConEmuHk, PID=%u, TID=%u", GetCurrentProcessId(), GetCurrentThreadId());
				MsgBox(NULL, szMsg, szTitle, MB_SYSTEMMODAL);
			}
			FreeLibrary(hUser);
		}
	}


	#ifdef _DEBUG
	{
		wchar_t szCpInfo[128];
		DWORD nCP = GetConsoleOutputCP();
		_wsprintf(szCpInfo, SKIPLEN(countof(szCpInfo)) L"Current Output CP = %u", nCP);
		print_timings(szCpInfo);
	}
	#endif

	if ((lstrcmpi(pszName, L"powershell.exe") == 0) || (lstrcmpi(pszName, L"powershell") == 0))
	{
		HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
		if (IsOutputHandle(hStdOut))
		{
			gbPowerShellMonitorProgress = true;
			MY_CONSOLE_SCREEN_BUFFER_INFOEX csbi = {sizeof(csbi)};
			if (apiGetConsoleScreenBufferInfoEx(hStdOut, &csbi))
			{
				gnConsolePopupColors = csbi.wPopupAttributes;
			}
			else
			{
				WARNING("�������� Popup �������� �� ��������");
				//gnConsolePopupColors = ...;
				gnConsolePopupColors = 0;
			}
		}
	}
	else if ((lstrcmpi(pszName, L"cmd.exe") == 0) || (lstrcmpi(pszName, L"cmd") == 0))
	{
		gnAllowClinkUsage = 1; // ���� �� ��������
	}
	else if ((lstrcmpi(pszName, L"sh.exe") == 0) || (lstrcmpi(pszName, L"sh") == 0)
		|| (lstrcmpi(pszName, L"bash.exe") == 0) || (lstrcmpi(pszName, L"bash") == 0)
		|| (lstrcmpi(pszName, L"isatty.exe") == 0)
		)
	{
		//_ASSERTEX(FALSE && "settings gbIsBashProcess");
		gbIsBashProcess = true;
		
		TODO("Start redirection of ConIn/ConOut to our pipes to achieve PTTY in bash");
		#if 0
		if (lstrcmpi(pszName, L"isatty.exe") == 0)
			StartPTY();
		#endif
	}
	else if ((lstrcmpi(pszName, L"hiew32.exe") == 0) || (lstrcmpi(pszName, L"hiew32") == 0))
	{
		gbIsHiewProcess = true;
	}
	else if ((lstrcmpi(pszName, L"dosbox.exe") == 0) || (lstrcmpi(pszName, L"dosbox") == 0))
	{
		gbDosBoxProcess = true;
	}

	// ��������� ��������� � �������� ����� ���� ���-�� �����������, ����� ������ �����
	// iFindAddress = FindKernelAddress(pi.hProcess, pi.dwProcessId, &fLoadLibrary);
	//HMODULE hKernel = ::GetModuleHandle(L"kernel32.dll");
	//if (hKernel)
	//{
	//	gfnLoadLibrary = (UINT_PTR)::GetProcAddress(hKernel, "LoadLibraryW");
	//	_ASSERTE(gfnLoadLibrary!=NULL);
	//}
	//else
	//{
	//	_ASSERTE(hKernel!=NULL);
	//	gfnLoadLibrary = NULL;
	//}
	if (!GetLoadLibraryAddress())
	{
		_ASSERTE(gfnLoadLibrary!=0);
	}
	
	ghUser32 = GetModuleHandle(user32);
	if (ghUser32) ghUser32 = LoadLibrary(user32); // ���� ����������� - ��������� �������

	WARNING("����������� �� ��������� LocalSecurity ��� ������");
	
	//#ifndef TESTLINK
	gpLocalSecurity = LocalSecurity();
	//gnMsgActivateCon = RegisterWindowMessage(CONEMUMSG_ACTIVATECON);
	//#endif
	//wchar_t szSkipEventName[128];
	//msprintf(szSkipEventName, SKIPLEN(countof(szSkipEventName)) CEHOOKDISABLEEVENT, GetCurrentProcessId());
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

	WARNING("����������� �� �������� � �������, � ����� ��� �� ���������� ConEmuData");
	// ������� ������� ������� � ���������� �������� HWND GUI, PID �������, � ��...
	if (ghConWnd)
	{
		print_timings(L"OnConWndChanged");
		OnConWndChanged(ghConWnd);
		//GetConMap();
	}

	if (ghConEmuWnd)
	{
#ifdef SHOW_INJECT_MSGBOX
		wchar_t* szDbgMsg = (wchar_t*)calloc(1024, sizeof(wchar_t));
		wchar_t* szTitle = (wchar_t*)calloc(128, sizeof(wchar_t));
		msprintf(szTitle, 1024, L"ConEmuHk, PID=%u", GetCurrentProcessId());
		msprintf(szDbgMsg, 128, L"SetAllHooks, ConEmuHk, PID=%u\n%s", GetCurrentProcessId(), szModule);
		GuiMessageBox(ghConEmuWnd, szDbgMsg, szTitle, MB_SYSTEMMODAL);
		free(szDbgMsg);
		free(szTitle);
#endif
	}

	//if (!gbSkipInjects && ghConWnd)
	//{
	//	InitializeConsoleInputSemaphore();
	//}


	print_timings(L"GetImageSubsystem");

	
	// ���������� ���������� �������� � ��� (CUI/GUI) ��������, � ������� ��� ���������
	gnImageBits = WIN3264TEST(32,64);
	gnImageSubsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
	// ��������� ��� (CUI/GUI)
	GetImageSubsystem(gnImageSubsystem, gnImageBits);
	// �������� ���� ����������
	_ASSERTE(gnImageBits==WIN3264TEST(32,64));
	_ASSERTE(gnImageSubsystem==IMAGE_SUBSYSTEM_WINDOWS_GUI || gnImageSubsystem==IMAGE_SUBSYSTEM_WINDOWS_CUI);
	
	
	//BOOL lbGuiWindowAttach = FALSE; // ��������� � ConEmu ������ ��������� (notepad, putty, ...)

	
#ifdef USE_PIPE_SERVER
	_ASSERTEX(gpHookServer==NULL);
	if (!gbPrepareDefaultTerminal)
	{
		print_timings(L"gpHookServer");
		gpHookServer = (PipeServer<CESERVER_REQ>*)calloc(1,sizeof(*gpHookServer));
		if (gpHookServer)
		{
			wchar_t szPipeName[128];
			msprintf(szPipeName, countof(szPipeName), CEHOOKSPIPENAME, L".", GetCurrentProcessId());
			
			gpHookServer->SetMaxCount(3);
			gpHookServer->SetOverlapped(true);
			gpHookServer->SetLoopCommands(false);
			gpHookServer->SetDummyAnswerSize(sizeof(CESERVER_REQ_HDR));
			
			if (!gpHookServer->StartPipeServer(szPipeName, (LPARAM)gpHookServer, LocalSecurity(), HookServerCommand, HookServerFree, NULL, NULL, HookServerReady))
			{
				_ASSERTEX(FALSE); // ������ ������� Pipes?
				gpHookServer->StopPipeServer(true);
				free(gpHookServer);
				gpHookServer = NULL;
			}
		}
		else
		{
			_ASSERTEX(gpHookServer!=NULL);
		}
	}
#endif

	
	if (gbPrepareDefaultTerminal)
	{
		TODO("�������������� �������������, ���� �����, ��� ��������� ���������� ��� DefaultTerminal");
		gbSelfIsRootConsoleProcess = true;
	}
	else if (ghConWnd)
	{
		WARNING("����������� �� �������� � �������, � ����� ��� �� ���������� ConEmuData");
		print_timings(L"CShellProc");
		CShellProc* sp = new CShellProc;
		if (sp)
		{
			if (sp->LoadSrvMapping())
			{
			
				wchar_t *szExeName = (wchar_t*)calloc((MAX_PATH+1),sizeof(wchar_t));
				//BOOL lbDosBoxAllowed = FALSE;
				if (!GetModuleFileName(NULL, szExeName, MAX_PATH+1)) szExeName[0] = 0;

				if (sp->GetUseInjects() == 2)
				{
					// ����� �� ������������ ����������� ������ ����� (������ ��� exe-�����)?
					if (!gbSelfIsRootConsoleProcess && !IsFarExe(szExeName))
					{
						gbHookExecutableOnly = true;
					}
				}

				CESERVER_REQ* pIn = sp->NewCmdOnCreate(eInjectingHooks, L"",
					szExeName, GetCommandLineW(),
					NULL, NULL, NULL, NULL, // flags
					gnImageBits, gnImageSubsystem,
					GetStdHandle(STD_INPUT_HANDLE), GetStdHandle(STD_OUTPUT_HANDLE), GetStdHandle(STD_ERROR_HANDLE));
				if (pIn)
				{
					//HWND hConWnd = GetConsoleWindow();
					CESERVER_REQ* pOut = ExecuteGuiCmd(ghConWnd, pIn, ghConWnd);
					ExecuteFreeResult(pIn);
					if (pOut) ExecuteFreeResult(pOut);
				}
				free(szExeName);
			}
			delete sp;
		}
	}
	else if (gnImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI)
	{
		print_timings(L"IMAGE_SUBSYSTEM_WINDOWS_GUI");
		DWORD dwConEmuHwnd = 0;
		BOOL  bAttachExistingWindow = FALSE;
		wchar_t szVar[64], *psz;
		ConEmuGuiMapping* GuiMapping = (ConEmuGuiMapping*)calloc(1,sizeof(*GuiMapping));
		// �� ��������� �� PID GUI ��������? ����� ���� ��� ������ ����� ����������� GUI ���������� ����� ���.
		if (GuiMapping && LoadGuiMapping(gnSelfPID, *GuiMapping))
		{
			gnGuiPID = GuiMapping->nGuiPID;
			ghConEmuWnd = GuiMapping->hGuiWnd;
			bAttachExistingWindow = gbAttachGuiClient = TRUE;
			//ghAttachGuiClient = 
		}
		else
		{
			_ASSERTEX((gbPrepareDefaultTerminal==false) && "LoadGuiMapping failed");
		}
		SafeFree(GuiMapping);

		// ���� ������� ������������ ���� - ��� � ConEmu ��� �� �����
		if (!bAttachExistingWindow)
		{
			if (!dwConEmuHwnd && GetEnvironmentVariable(ENV_CONEMUHWND_VAR_W, szVar, countof(szVar)))
			{
				if (szVar[0] == L'0' && szVar[1] == L'x')
				{
					dwConEmuHwnd = wcstoul(szVar+2, &psz, 16);
					if (!user->isWindow((HWND)dwConEmuHwnd))
						dwConEmuHwnd = 0;
					else if (!user->getClassNameW((HWND)dwConEmuHwnd, szVar, countof(szVar)))
						dwConEmuHwnd = 0;
					else if (lstrcmp(szVar, VirtualConsoleClassMain) != 0)
						dwConEmuHwnd = 0;
				}
			}
			
			if (dwConEmuHwnd)
			{
				// ��������������� ����������� ConEmu GUI, ��� �������� GUI ����������
				// � ��� ����� "�������� �� ������� ConEmu".
				DWORD nSize = sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_ATTACHGUIAPP);
				CESERVER_REQ *pIn = (CESERVER_REQ*)malloc(nSize);
				ExecutePrepareCmd(pIn, CECMD_ATTACHGUIAPP, nSize);
				pIn->AttachGuiApp.nPID = GetCurrentProcessId();
				GetModuleFileName(NULL, pIn->AttachGuiApp.sAppFileName, countof(pIn->AttachGuiApp.sAppFileName));
				pIn->AttachGuiApp.hkl = (DWORD)(LONG)(LONG_PTR)GetKeyboardLayout(0);

				wchar_t szGuiPipeName[128];
				msprintf(szGuiPipeName, countof(szGuiPipeName), CEGUIPIPENAME, L".", dwConEmuHwnd);
				
				CESERVER_REQ* pOut = ExecuteCmd(szGuiPipeName, pIn, 10000, NULL);

				free(pIn);

				if (pOut)
				{
					if (pOut->hdr.cbSize > sizeof(CESERVER_REQ_HDR))
					{
						if (pOut->AttachGuiApp.nFlags & agaf_Success)
						{
							user->allowSetForegroundWindow(pOut->hdr.nSrcPID); // PID ConEmu.
							_ASSERTEX(gnGuiPID==0 || gnGuiPID==pOut->hdr.nSrcPID);
							gnGuiPID = pOut->hdr.nSrcPID;
							//ghConEmuWnd = (HWND)dwConEmuHwnd;
							_ASSERTE(ghConEmuWnd==NULL || gnGuiPID!=0);
							_ASSERTE(pOut->AttachGuiApp.hConEmuWnd==(HWND)dwConEmuHwnd);
							ghConEmuWnd = pOut->AttachGuiApp.hConEmuWnd;
							SetConEmuHkWindows(pOut->AttachGuiApp.hConEmuDc, pOut->AttachGuiApp.hConEmuBack);
							ghConWnd = pOut->AttachGuiApp.hSrvConWnd;
							_ASSERTE(ghConEmuWndDC && user->isWindow(ghConEmuWndDC));
							grcConEmuClient = pOut->AttachGuiApp.rcWindow;
							gnServerPID = pOut->AttachGuiApp.nPID;
							//gbGuiClientHideCaption = pOut->AttachGuiApp.bHideCaption;
							gGuiClientStyles = pOut->AttachGuiApp.Styles;
							if (pOut->AttachGuiApp.hkl)
							{
								LONG_PTR hkl = (LONG_PTR)(LONG)pOut->AttachGuiApp.hkl;
								BOOL lbRc = ActivateKeyboardLayout((HKL)hkl, KLF_SETFORPROCESS) != NULL;
								UNREFERENCED_PARAMETER(lbRc);
							}
							OnConWndChanged(ghConWnd);
							gbAttachGuiClient = TRUE;
						}
					}
					ExecuteFreeResult(pOut);
				}
			}
		}
	}

	if (gbPrepareDefaultTerminal)
	{
		InitDefaultTerm();
	}

	//if (!gbSkipInjects)
	{
		//gnRunMode = RM_APPLICATION;

		#ifdef _DEBUG
		//wchar_t szModule[MAX_PATH+1]; szModule[0] = 0;
		//GetModuleFileName(NULL, szModule, countof(szModule));
		_ASSERTE((gnImageSubsystem==IMAGE_SUBSYSTEM_WINDOWS_CUI) || (lstrcmpi(pszName, L"DosBox.exe")==0) || gbAttachGuiClient || gbPrepareDefaultTerminal);
		//if (!lstrcmpi(pszName, L"far.exe") || !lstrcmpi(pszName, L"mingw32-make.exe"))
		//if (!lstrcmpi(pszName, L"as.exe"))
		//	MessageBoxW(NULL, L"as.exe loaded!", L"ConEmuHk", MB_SYSTEMMODAL);
		//else if (!lstrcmpi(pszName, L"cc1plus.exe"))
		//	MessageBoxW(NULL, L"cc1plus.exe loaded!", L"ConEmuHk", MB_SYSTEMMODAL);
		//else if (!lstrcmpi(pszName, L"mingw32-make.exe"))
		//	MessageBoxW(NULL, L"mingw32-make.exe loaded!", L"ConEmuHk", MB_SYSTEMMODAL);
		//if (!lstrcmpi(pszName, L"g++.exe"))
		//	MessageBoxW(NULL, L"g++.exe loaded!", L"ConEmuHk", MB_SYSTEMMODAL);
		//{
		#endif

		DLOG0("StartupHooks",0);
		print_timings(L"StartupHooks");
		gbHooksWasSet = StartupHooks(ghOurModule);
		print_timings(L"StartupHooks - done");
		DLOGEND();

		#ifdef _DEBUG
		//}
		#endif

		// ���� NULL - ������ ��� "Detached" ���������� �������, �������� "Started" � ������ ������ ���
		if (ghConWnd != NULL)
		{
			if (gbSelfIsRootConsoleProcess)
			{
				// To avoid cmd-execute lagging - send Start/Stop info only for root(!) process
				DLOG("SendStarted",0);
				print_timings(L"SendStarted");
				SendStarted();
				DLOGEND();
			}

			//#ifdef _DEBUG
			//// ����� ��� �������� � ������ _chkstk,
			//// ������ ��-�� ����, ��� dll-�� ��������� �� �� ��������� �������,
			//// � �� ���������� ��������������� ����� ������
			// -- � ����� �� �������, ��� ��������� ���������� ���������� ������� ����� ������
			// -- ��������� � malloc/free, ��� ����������
			//TestShellProcessor();
			//#endif
		}
	}
	//else
	//{
	//	gbHooksWasSet = FALSE;
	//}
	
	//delete sp;

	/*
	#ifdef _DEBUG
	if (!lstrcmpi(pszName, L"mingw32-make.exe"))
		GuiMessageBox(ghConEmuWnd, L"mingw32-make.exe DllMain finished", L"ConEmuHk", MB_SYSTEMMODAL);
	#endif
	*/

	SafeFree(szModule);
	
	//if (hStartedEvent)
	//	SetEvent(hStartedEvent);

	print_timings(L"DllStart - done");

	//DLOGEND();
	
	return 0;
}

#ifdef HOOK_USE_DLLTHREAD
void DllThreadClose()
{
	if (ghStartThread)
	{
		DWORD nWait = WaitForSingleObject(ghStartThread, 5000);
		if (nWait == WAIT_TIMEOUT)
		{
			// ��������, �� ����, ���� ������������� ���������� ����������
			TerminateThread(ghStartThread, 100);
		}
		CloseHandle(ghStartThread);
		ghStartThread = NULL;
	}
}
#endif

void FlushMouseEvents()
{
	if (ghConWnd)
	{
		HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
		DWORD nTotal = 0;
		if (GetNumberOfConsoleInputEvents(h, &nTotal) && nTotal)
		{
			INPUT_RECORD *pr = (INPUT_RECORD*)calloc(nTotal, sizeof(*pr));
			if (pr && PeekConsoleInput(h, pr, nTotal, &nTotal) && nTotal)
			{
				bool bHasMouse = false;
				DWORD j = 0;
				for (DWORD i = 0; i < nTotal; i++)
				{
					if (pr[i].EventType == MOUSE_EVENT)
					{
						bHasMouse = true;
						continue;
					}
					else
					{
						if (i > j)
							pr[j] = pr[i];
						j++;
					}
				}

				// ���� ���� ������� ������� - �������� ��
				if (bHasMouse)
				{
					if (FlushConsoleInputBuffer(h))
					{
						// �� ���� ���� �� ������� - ������� �� � �����
						if (j > 0)
						{
							WriteConsoleInput(h, pr, j, &nTotal);
						}
					}
				}
			}
		}
	}
}

void DllStop()
{
	//DLOG0("DllStop",0);

	#if defined(SHOW_EXE_TIMINGS) || defined(SHOW_EXE_MSGBOX)
		wchar_t szTimingMsg[512]; UNREFERENCED_PARAMETER(szTimingMsg);
		HANDLE hTimingHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	#endif

	TODO("Stop redirection of ConIn/ConOut to our pipes to achieve PTTY in bash");
	#ifdef _DEBUG
	StopPTY();
	#endif

	print_timings(L"DllStop");
	//gbDllStopCalled = TRUE; -- � �����


	// Issue 689: Progress stuck at 100%
	if (gbPowerShellMonitorProgress && (gnPowerShellProgressValue != -1))
	{
		DLOG0("GuiSetProgress(0,0)",0);
		gnPowerShellProgressValue = -1;
		GuiSetProgress(0,0);
		DLOGEND();
	}


	#ifdef HOOK_USE_DLLTHREAD
	DllThreadClose();
	#endif
	
	#ifdef _DEBUG
	wchar_t *szModule = (wchar_t*)calloc((MAX_PATH+1),sizeof(wchar_t));
	if (!GetModuleFileName(NULL, szModule, MAX_PATH+1))
		_wcscpy_c(szModule, MAX_PATH+1, L"GetModuleFileName failed");
	const wchar_t* pszName = PointToName(szModule);
	//if (!lstrcmpi(pszName, L"mingw32-make.exe"))
	//	GuiMessageBox(ghConEmuWnd, L"mingw32-make.exe terminating", L"ConEmuHk", MB_SYSTEMMODAL);
	free(szModule);
	#endif

	// 120528 - �������� ����� �� ������� �������, ����� ���������� ������.
	// ���� �� ����� ���������� ������� (�������� "dir c: /s")
	// ������ ������� ������ - �� ��� �������� � ��� ����� ������ ��������� ����
	if (ghConWnd)
	{
		DLOG0("FlushMouseEvents",0);
		print_timings(L"FlushMouseEvents");
		FlushMouseEvents();
		DLOGEND();
	}


#ifdef USE_PIPE_SERVER
	if (gpHookServer)
	{
		DLOG0("StopPipeServer",0);
		print_timings(L"StopPipeServer");
		gpHookServer->StopPipeServer(true);
		free(gpHookServer);
		gpHookServer = NULL;
		DLOGEND();
	}
#endif
	
	#ifdef _DEBUG
	if (ghGuiClientRetHook)
	{
		DLOG0("unhookWindowsHookEx",0);
		print_timings(L"unhookWindowsHookEx");
		user->unhookWindowsHookEx(ghGuiClientRetHook);
		DLOGEND();
	}
	#endif

	if (/*!gbSkipInjects &&*/ gbHooksWasSet)
	{
		DLOG0("ShutdownHooks",0);
		print_timings(L"ShutdownHooks");
		gbHooksWasSet = FALSE;
		// ��������� ������ � ��������
		DoneHooksReg();
		// "�������" ����
		ShutdownHooks();
		DLOGEND();
	}

	if (gbSelfIsRootConsoleProcess)
	{
		// To avoid cmd-execute lagging - send Start/Stop info only for root(!) process
		DLOG0("SendStopped",0);
		print_timings(L"SendStopped");
		SendStopped();
		DLOGEND();
	}

	if (gpConMap)
	{
		DLOG0("gpConMap->CloseMap",0);
		print_timings(L"gpConMap->CloseMap");
		gpConMap->CloseMap();
		gpConInfo = NULL;
		delete gpConMap;
		gpConMap = NULL;
		DLOGEND();
	}
	
	// CommonShutdown
	{
		DLOG0("CommonShutdown",0);
		//#ifndef TESTLINK
		print_timings(L"CommonShutdown");
		CommonShutdown();
		DLOGEND();
	}

	
	// FinalizeHookedModules
	{
		DLOG0("FinalizeHookedModules",0);
		print_timings(L"FinalizeHookedModules");
		FinalizeHookedModules();
		DLOGEND();
	}

#ifndef _DEBUG
	HeapDeinitialize();
#endif
	
	#ifdef _DEBUG
		#ifdef UseDebugExceptionFilter
			// ?gfnPrevFilter?
			// �������. A value of NULL for this parameter specifies default handling within UnhandledExceptionFilter.
			SetUnhandledExceptionFilter(NULL);
		#endif
	#endif

	gbDllStopCalled = TRUE;
	print_timings(L"DllStop - Done");

	//DLOGEND();
}

BOOL WINAPI DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	BOOL lbAllow = TRUE;

#if defined(_DEBUG) && !defined(_WIN64)
	// pThreadInfo[9] -> GetCurrentThreadId();
	DWORD* pThreadInfo = ((DWORD*) __readfsdword(24));
#endif

	switch(ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			DLOG0("DllMain.DLL_PROCESS_ATTACH",ul_reason_for_call);

			#ifdef USEHOOKLOG
			QueryPerformanceFrequency(&HookLogger::g_freq);
			#endif

			gnDllState = ds_DllProcessAttach;
			#ifdef _DEBUG
			HANDLE hProcHeap = GetProcessHeap();
			#endif
			HeapInitialize();
			
			DLOG1("DllMain.LoadStartupEnv",ul_reason_for_call);
			/* *** DEBUG PURPOSES */
			gpStartEnv = LoadStartupEnv();
			DLOGEND1();
			//if (gpStartEnv && gpStartEnv->hIn.hStd && !(gpStartEnv->hIn.nMode & 0x80000000))
			//{
			//	if ((gpStartEnv->hIn.nMode & 0xF0) == 0xE0)
			//	{
			//		_ASSERTE(FALSE && "ENABLE_MOUSE_INPUT was disabled! Enabling...");
			//		SetConsoleMode(gpStartEnv->hIn.hStd, gpStartEnv->hIn.nMode|ENABLE_MOUSE_INPUT);
			//	}
			//}
			/* *** DEBUG PURPOSES */

			DLOG1_("DllMain.Console",ul_reason_for_call);
			ghOurModule = (HMODULE)hModule;
			ghConWnd = GetConsoleWindow();
			if (ghConWnd)
				GetConsoleTitle(gsInitConTitle, countof(gsInitConTitle));
			gnSelfPID = GetCurrentProcessId();
			ghWorkingModule = (u64)hModule;
			gfGetRealConsoleWindow = GetConsoleWindow;
			user = (UserImp*)calloc(1, sizeof(*user));
			DLOGEND1();



			DLOG1_("DllMain.RootEvents",ul_reason_for_call);
			bool bCurrentThreadIsMain = false;

			wchar_t szEvtName[64];
			msprintf(szEvtName, countof(szEvtName), CECONEMUROOTPROCESS, gnSelfPID);
			HANDLE hRootProcessFlag = OpenEvent(SYNCHRONIZE|EVENT_MODIFY_STATE, FALSE, szEvtName);
			DWORD nWaitRoot = -1;
			if (hRootProcessFlag)
			{
				nWaitRoot = WaitForSingleObject(hRootProcessFlag, 0);
				gbSelfIsRootConsoleProcess = (nWaitRoot == WAIT_OBJECT_0);
			}
			SafeCloseHandle(hRootProcessFlag);

			msprintf(szEvtName, countof(szEvtName), CECONEMUROOTTHREAD, gnSelfPID);
			hRootProcessFlag = OpenEvent(SYNCHRONIZE|EVENT_MODIFY_STATE, FALSE, szEvtName);
			if (hRootProcessFlag)
			{
				nWaitRoot = WaitForSingleObject(hRootProcessFlag, 0);
				bCurrentThreadIsMain = (nWaitRoot == WAIT_OBJECT_0);
			}
			SafeCloseHandle(hRootProcessFlag);

			if (!gbSelfIsRootConsoleProcess)
			{
				msprintf(szEvtName, countof(szEvtName), CEDEFAULTTERMHOOK, gnSelfPID);
				hRootProcessFlag = OpenEvent(SYNCHRONIZE|EVENT_MODIFY_STATE, FALSE, szEvtName);
				if (hRootProcessFlag)
				{
					nWaitRoot = WaitForSingleObject(hRootProcessFlag, 0);
					gbPrepareDefaultTerminal = (nWaitRoot == WAIT_OBJECT_0);
				}
				SafeCloseHandle(hRootProcessFlag);
			}
			DLOGEND1();



			DLOG1_("DllMain.MainThreadId",ul_reason_for_call);
			GetMainThreadId(bCurrentThreadIsMain); // ���������������� gnHookMainThreadId
			DLOGEND1();

			DLOG1_("DllMain.InQueue",ul_reason_for_call);
			gcchLastWriteConsoleMax = 4096;
			gpszLastWriteConsole = (wchar_t*)calloc(gcchLastWriteConsoleMax,sizeof(*gpszLastWriteConsole));
			gInQueue.Initialize(512, NULL);
			DLOGEND1();

			DLOG1_("DllMain.Misc",ul_reason_for_call);
			#ifdef _DEBUG
			gAllowAssertThread = am_Pipe;
			#endif
			
			#ifdef _DEBUG
				#ifdef UseDebugExceptionFilter
					gfnPrevFilter = SetUnhandledExceptionFilter(HkExceptionFilter);
				#endif
			#endif

			#ifdef SHOW_STARTED_MSGBOX
			if (!IsDebuggerPresent())
			{
				::MessageBox(ghConEmuWnd, L"ConEmuHk*.dll loaded", L"ConEmu hooks", MB_SYSTEMMODAL);
			}
			#endif
			#ifdef _DEBUG
			DWORD dwConMode = -1;
			GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &dwConMode);
			#endif
			DLOGEND1();

			//_ASSERTE(ghHeap == NULL);
			//ghHeap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 200000, 0);

			if (gbPrepareDefaultTerminal)
				user->setAllowLoadLibrary();

			
			DLOG1_("DllMain.DllStart",ul_reason_for_call);
			#ifdef HOOK_USE_DLLTHREAD
			_ASSERTEX(FALSE && "Hooks starting in background thread?");
			//HANDLE hEvents[2];
			//hEvents[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
			//hEvents[1] = 
			ghStartThread = CreateThread(NULL, 0, DllStart, NULL/*(LPVOID)(hEvents[0])*/, 0, &gnStartThreadID);
			if (ghStartThread == NULL)
			{
				//_ASSERTE(ghStartThread!=NULL);
				wchar_t szMsg[128]; DWORD nErrCode = GetLastError();
				msprintf(szMsg, countof(szMsg),
					L"Failed to start DllStart thread!\nErrCode=0x%08X\nPID=%u",
					nErrCode, GetCurrentProcessId());
				GuiMessageBox(ghConEmuWnd, szMsg, L"ConEmu hooks", 0);
			}
			else
			{
				DWORD nThreadWait = WaitForSingleObject(ghStartThread, 5000);
				DllThreadClose();
			}
			//DWORD nThreadWait = WaitForMultipleObjects(hEvents, countof(hEvents), FALSE, INFINITE);
			//CloseHandle(hEvents[0]);
			#else
			DllStart(NULL);
			#endif
			DLOGEND1();
			
			user->setAllowLoadLibrary();

			DLOGEND();
		}
		break; // DLL_PROCESS_ATTACH
		
		case DLL_THREAD_ATTACH:
		{
			DLOG0("DllMain.DLL_THREAD_ATTACH",ul_reason_for_call);
			gnDllThreadCount++;
			if (gbHooksWasSet)
				InitHooksRegThread();
			DLOGEND();
		}
		break; // DLL_THREAD_ATTACH

		case DLL_THREAD_DETACH:
		{
			DLOG0("DllMain.DLL_THREAD_DETACH",ul_reason_for_call);

			#ifdef SHOW_SHUTDOWN_STEPS
			gnDbgPresent = 0;
			ShutdownStep(L"DLL_THREAD_DETACH");
			#endif

			if (gbHooksWasSet)
				DoneHooksRegThread();
			// DLL_PROCESS_DETACH ������� ��� ���������� �� ������
			if (gnHookMainThreadId && (GetCurrentThreadId() == gnHookMainThreadId) && !gbDllDeinitialized)
			{
				gbDllDeinitialized = true;
				DLOG1("DllMain.DllStop",ul_reason_for_call);
				//WARNING!!! OutputDebugString must NOT be used from ConEmuHk::DllMain(DLL_PROCESS_DETACH). See Issue 465
				DllStop();
				DLOGEND1();
			}
			gnDllThreadCount--;
			ShutdownStep(L"DLL_THREAD_DETACH done, left=%i", gnDllThreadCount);

			DLOGEND();
		}
		break; // DLL_THREAD_DETACH
		
		case DLL_PROCESS_DETACH:
		{
			DLOG0("DllMain.DLL_PROCESS_DETACH",ul_reason_for_call);

			ShutdownStep(L"DLL_PROCESS_DETACH");
			gnDllState = ds_DllProcessDetach;
			if (gbHooksWasSet)
				lbAllow = FALSE; // ����� ��������, �.�. FreeLibrary �����������
			// ��� ����� ������� � DLL_THREAD_DETACH
			if (!gbDllDeinitialized)
			{
				gbDllDeinitialized = true;
				DLOG1("DllMain.DllStop",ul_reason_for_call);
				//WARNING!!! OutputDebugString must NOT be used from ConEmuHk::DllMain(DLL_PROCESS_DETACH). See Issue 465
				DllStop();
				DLOGEND1();
			}
			// -- free �� �����, �.�. ��� ������ HeapDeinitialize()
			//free(user);
			ShutdownStep(L"DLL_PROCESS_DETACH done");

			#ifdef USEHOOKLOG
			DLOGEND();
			//HookLogger::RunAnalyzer();
			//_ASSERTEX(FALSE && "Hooks terminated");
			#endif
		}
		break; // DLL_PROCESS_DETACH
	}

	return lbAllow;
}

#if defined(CRTSTARTUP)
extern "C" {
	BOOL WINAPI _DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
};

BOOL WINAPI _DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
	DllMain(hDll, dwReason, lpReserved);
	return TRUE;
}
#endif

///* ������������ ��� extern � ConEmuCheck.cpp */
//LPVOID _calloc(size_t nCount,size_t nSize) {
//	return calloc(nCount,nSize);
//}
//LPVOID _malloc(size_t nCount) {
//	return malloc(nCount);
//}
//void   _free(LPVOID ptr) {
//	free(ptr);
//}


//BYTE gnOtherWin = 0;
//DWORD gnSkipVkModCode = 0;
////DWORD gnSkipScanModCode = 0;
//DWORD gnSkipVkKeyCode = 0;
////DWORD gnWinPressTick = 0;
////int gnMouseTouch = 0;
//
//LRESULT CALLBACK LLKeybHook(int nCode,WPARAM wParam,LPARAM lParam)
//{
//	if (nCode >= 0)
//	{
//		KBDLLHOOKSTRUCT *pKB = (KBDLLHOOKSTRUCT*)lParam;
//#ifdef _DEBUG
//		wchar_t szKH[128];
//		DWORD dwTick = GetTickCount();
//		msprintf(szKH, SKIPLEN(countof(szKH)) L"[hook] %s(vk=%i, flags=0x%08X, time=%i, tick=%i, delta=%i)\n",
//		          (wParam==WM_KEYDOWN) ? L"WM_KEYDOWN" :
//		          (wParam==WM_KEYUP) ? L"WM_KEYUP" :
//		          (wParam==WM_SYSKEYDOWN) ? L"WM_SYSKEYDOWN" :
//		          (wParam==WM_SYSKEYUP) ? L"WM_SYSKEYUP" : L"UnknownMessage",
//		          pKB->vkCode, pKB->flags, pKB->time, dwTick, (dwTick-pKB->time));
//		//if (wParam == WM_KEYUP && gnSkipVkModCode && pKB->vkCode == gnSkipVkModCode) {
//		//	msprintf(szKH+lstrlen(szKH)-1, L" - WinDelta=%i\n", (pKB->time - gnWinPressTick));
//		//}
//		OutputDebugString(szKH);
//#endif
//
//		if (wParam == WM_KEYDOWN && ghKeyHookConEmuRoot)
//		{
//			if ((pKB->vkCode >= (UINT)'0' && pKB->vkCode <= (UINT)'9') /*|| pKB->vkCode == (int)' '*/)
//			{
//				BOOL lbLeftWin = isPressed(VK_LWIN);
//				BOOL lbRightWin = isPressed(VK_RWIN);
//
//				if ((lbLeftWin || lbRightWin) && IsWindow(ghKeyHookConEmuRoot))
//				{
//					DWORD nConNumber = (pKB->vkCode == (UINT)'0') ? 10 : (pKB->vkCode - (UINT)'0');
//					PostMessage(ghKeyHookConEmuRoot, gnMsgActivateCon, nConNumber, 0);
//					gnSkipVkModCode = lbLeftWin ? VK_LWIN : VK_RWIN;
//					gnSkipVkKeyCode = pKB->vkCode;
//					// ������ ��������� ��������
//					return 1; // ����� ���������� 1, ����� ������� �� ���� � Win7 Taskbar
//					////gnWinPressTick = pKB->time;
//					//HWND hConEmu = GetForegroundWindow();
//					//// �� ����, ������ ���� ConEmu, �� ���������� ��������� (����� ��� �� ������?)
//					//if (hConEmu)
//					//{
//					//	wchar_t szClass[64];
//					//	if (GetClassNameW(hConEmu, szClass, 63) && lstrcmpW(szClass, VirtualConsoleClass)==0)
//					//	{
//					//		//if (!gnMsgActivateCon) --> DllMain
//					//		//	gnMsgActivateCon = RegisterWindowMessage(CONEMUMSG_LLKEYHOOK);
//					//		WORD nConNumber = (pKB->vkCode == (UINT)'0') ? 10 : (pKB->vkCode - (UINT)'0');
//					//		if (SendMessage(hConEmu, gnMsgActivateCon, wParam, pKB->vkCode) == 1)
//					//		{
//					//			gnSkipVkModCode = lbLeftWin ? VK_LWIN : VK_RWIN;
//					//			gnSkipVkKeyCode = pKB->vkCode;
//					//			// ������ ��������� ��������
//					//			return 1; // ����� ���������� 1, ����� ������� �� ���� � Win7 Taskbar
//					//		}
//					//	}
//					//}
//				}
//			}
//
//			// �� ������ ������� �� �������� - ������ ��� ���������
//			//if (pKB->vkCode == VK_LWIN || pKB->vkCode == VK_RWIN) {
//			//	gnWinPressTick = pKB->time;
//			//}
//
//			if (gnSkipVkKeyCode && !gnOtherWin)
//			{
//				// ��������� �� ���������
//				gnSkipVkModCode = 0;
//				gnSkipVkKeyCode = 0;
//			}
//		}
//		else if (wParam == WM_KEYUP)
//		{
//			if (gnSkipVkModCode && pKB->vkCode == gnSkipVkModCode)
//			{
//				if (gnSkipVkKeyCode)
//				{
//#ifdef _DEBUG
//					OutputDebugString(L"*** Win released before key ***\n");
//#endif
//					// ��� ������� ������� Win+<������> ����� ���������� ��� ��� Win ����������� ������ <������>.
//					gnOtherWin = (BYTE)gnVkWinFix;
//					keybd_event(gnOtherWin, gnOtherWin, 0, 0);
//				}
//				else
//				{
//					gnOtherWin = 0;
//				}
//
//				gnSkipVkModCode = 0;
//				return 0; // ��������� ��������� ��������, �� �� ���������� � ������ ����
//			}
//
//			if (gnSkipVkKeyCode && pKB->vkCode == gnSkipVkKeyCode)
//			{
//				gnSkipVkKeyCode = 0;
//
//				if (gnOtherWin)
//				{
//					keybd_event(gnOtherWin, gnOtherWin, KEYEVENTF_KEYUP, 0);
//					gnOtherWin = 0;
//				}
//
//				return 0; // ��������� ��������� ��������, �� �� ���������� � ������ ����
//			}
//		}
//	}
//
//	return CallNextHookEx(ghKeyHook, nCode, wParam, lParam);
//}



WARNING("����������� SendStarted ��������� �� �� DllMain, � ��������� ������� ����");

void SendStarted()
{
	if (gnServerPID == 0)
	{
		gbNonGuiMode = TRUE; // �� �������� ExecuteGuiCmd ��� ������. ��� �� ���� �������
		return; // ����� ComSpec, �� ������� ���, ��������������, � GUI ������ �������� �� �����
	}

	// To avoid cmd-execute lagging - send Start/Stop info only for root(!) process
	_ASSERTEX(gbSelfIsRootConsoleProcess);

	//_ASSERTE(FALSE && "Continue to SendStarted");

	CESERVER_REQ *pIn = NULL, *pOut = NULL;
	size_t nSize = sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_STARTSTOP); //-V119
	pIn = ExecuteNewCmd(CECMD_CMDSTARTSTOP, nSize);

	if (pIn)
	{
		if (!GetModuleFileName(NULL, pIn->StartStop.sModuleName, countof(pIn->StartStop.sModuleName)))
			pIn->StartStop.sModuleName[0] = 0;
		#ifdef _DEBUG
		LPCWSTR pszFileName = wcsrchr(pIn->StartStop.sModuleName, L'\\');
		#endif

		pIn->StartStop.nStarted = sst_AppStart;
		pIn->StartStop.hWnd = ghConWnd;
		pIn->StartStop.dwPID = gnSelfPID;
		pIn->StartStop.nImageBits = WIN3264TEST(32,64);

		pIn->StartStop.nSubSystem = gnImageSubsystem;
		//pIn->StartStop.bRootIsCmdExe = gbRootIsCmdExe; //2009-09-14
		// �� MyGet..., � �� ����� ���������������...
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

		GetConsoleScreenBufferInfo(hOut, &pIn->StartStop.sbi);
		gbWasBufferHeight = (pIn->StartStop.sbi.dwSize.Y > (pIn->StartStop.sbi.srWindow.Bottom - pIn->StartStop.sbi.srWindow.Top + 100));

		pIn->StartStop.crMaxSize = MyGetLargestConsoleWindowSize(hOut);


		BOOL bAsync = FALSE;
		if (ghConWnd && (gnGuiPID != 0) && (gnImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI))
			bAsync = TRUE;

		pOut = ExecuteSrvCmd(gnServerPID, pIn, ghConWnd, bAsync);


		if (bAsync || pOut)
		{
			if (pOut)
			{
				gbWasBufferHeight = pOut->StartStopRet.bWasBufferHeight;
				gnGuiPID = pOut->StartStopRet.dwPID;
				ghConEmuWnd = pOut->StartStopRet.hWnd;
				_ASSERTE(ghConEmuWnd==NULL || gnGuiPID!=0);
				SetConEmuHkWindows(pOut->StartStopRet.hWndDc, pOut->StartStopRet.hWndBack);
				_ASSERTE(ghConEmuWndDC && user->isWindow(ghConEmuWndDC));
				_ASSERTE(ghConEmuWndBack && user->isWindow(ghConEmuWndBack));

				gnServerPID = pOut->StartStopRet.dwMainSrvPID;
				ExecuteFreeResult(pOut); pOut = NULL;
			}
		}
		else
		{
			gbNonGuiMode = TRUE; // �� �������� ExecuteGuiCmd ��� ������. ��� �� ���� �������
		}

		ExecuteFreeResult(pIn); pIn = NULL;
	}
}

void SendStopped()
{
	if (gbNonGuiMode || !gnServerPID)
		return;
	
	// To avoid cmd-execute lagging - send Start/Stop info only for root(!) process
	_ASSERTEX(gbSelfIsRootConsoleProcess);

	//_ASSERTE(FALSE && "Continue to SendStopped");

	CESERVER_REQ *pIn = NULL, *pOut = NULL;
	size_t nSize = sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_STARTSTOP);
	pIn = ExecuteNewCmd(CECMD_CMDSTARTSTOP,nSize);

	if (pIn)
	{
		pIn->StartStop.nStarted = sst_AppStop;

		if (!GetModuleFileName(NULL, pIn->StartStop.sModuleName, countof(pIn->StartStop.sModuleName)))
			pIn->StartStop.sModuleName[0] = 0;

		pIn->StartStop.hWnd = ghConWnd;
		pIn->StartStop.dwPID = gnSelfPID;
		pIn->StartStop.nSubSystem = gnImageSubsystem;
		pIn->StartStop.bWasBufferHeight = gbWasBufferHeight;
		pIn->StartStop.nOtherPID = gnPrevAltServerPID;
		pIn->StartStop.bWasSucceededInRead = gbWasSucceededInRead;

		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

		// �� MyGet..., � �� ����� ���������������...
		// ghConOut ����� ���� NULL, ���� ������ ��������� �� ����� ������� ����������
		GetConsoleScreenBufferInfo(hOut, &pIn->StartStop.sbi);

		pIn->StartStop.crMaxSize = MyGetLargestConsoleWindowSize(hOut);

		pOut = ExecuteSrvCmd(gnServerPID, pIn, ghConWnd, TRUE/*bAsyncNoResult*/);

		ExecuteFreeResult(pIn); pIn = NULL;
		
		if (pOut)
		{
			ExecuteFreeResult(pOut);
			pOut = NULL;
		}
	}
}

void StartPTY()
{
	if (gpCEIO_In)
	{
		_ASSERTEX(gpCEIO_In==NULL);
		return;
	}

	gpCEIO_In = (ConEmuInOutPipe*)calloc(sizeof(ConEmuInOutPipe),1); 
	gpCEIO_Out = (ConEmuInOutPipe*)calloc(sizeof(ConEmuInOutPipe),1); 
	gpCEIO_Err = (ConEmuInOutPipe*)calloc(sizeof(ConEmuInOutPipe),1);

	HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);

	if (!hIn || !gpCEIO_In
		|| !gpCEIO_In->CEIO_Initialize(hIn, false)
		|| !gpCEIO_In->CEIO_RunThread())
	{
		SafeFree(gpCEIO_In);
	}
	else
	{
		SetStdHandle(STD_INPUT_HANDLE, gpCEIO_In->hRead);
	}

	if (!hOut || !gpCEIO_Out
		|| !gpCEIO_Out->CEIO_Initialize(hOut, true)
		|| !gpCEIO_Out->CEIO_RunThread())
	{
		SafeFree(gpCEIO_Out);
	}
	else
	{
		SetStdHandle(STD_OUTPUT_HANDLE, gpCEIO_Out->hWrite);
	}

	if (!hErr || (hErr == hOut) || !gpCEIO_Err
		|| !gpCEIO_Err->CEIO_Initialize(hIn, true)
		|| !gpCEIO_Err->CEIO_RunThread())
	{
		SafeFree(gpCEIO_Err);
		if (gpCEIO_In)
		{
			SetStdHandle(STD_ERROR_HANDLE, gpCEIO_Out->hWrite);
		}
	}
	else
	{
		SetStdHandle(STD_ERROR_HANDLE, gpCEIO_Err->hWrite);
	}
}

void StopPTY()
{
	if (gpCEIO_In)
	{
		gpCEIO_In->CEIO_Terminate();
		
		SetStdHandle(STD_INPUT_HANDLE, gpCEIO_In->hStd);
		
		SafeFree(gpCEIO_In);
	}

	if (gpCEIO_Out || gpCEIO_Err)
	{
		if (gpCEIO_Out)
		{
			gpCEIO_Out->CEIO_Terminate();
			SetStdHandle(STD_OUTPUT_HANDLE, gpCEIO_Out->hStd);
		}

		if (gpCEIO_Err)
		{
			gpCEIO_Err->CEIO_Terminate();
			SetStdHandle(STD_ERROR_HANDLE, gpCEIO_Err->hStd);
		}
		else if (gpCEIO_Out)
		{
			SetStdHandle(STD_ERROR_HANDLE, gpCEIO_Out->hStd);
		}

		SafeFree(gpCEIO_Out);
		SafeFree(gpCEIO_Err);
	}
}

int DuplicateRoot(CESERVER_REQ_DUPLICATE* Duplicate)
{
	if (!gpStartEnv)
		return -1;
	if (!gpStartEnv->pszCmdLine || !*gpStartEnv->pszCmdLine)
		return -2;
	if (!Duplicate->hGuiWnd || !Duplicate->nGuiPID || !Duplicate->nAID)
		return -3;

	ConEmuGuiMapping* GuiMapping = (ConEmuGuiMapping*)calloc(1,sizeof(*GuiMapping));
	if (!GuiMapping || !LoadGuiMapping(Duplicate->nGuiPID, *GuiMapping))
	{
		SafeFree(GuiMapping);
		return -4;
	}

	int iRc = -10;
	// go
	STARTUPINFO si = {sizeof(si)};
	PROCESS_INFORMATION pi = {};
	size_t cchCmdLine = 300 + lstrlen(GuiMapping->sConEmuBaseDir) + lstrlen(gpStartEnv->pszCmdLine);
	wchar_t *pszCmd, *pszName;
	BOOL bSrvFound;

	pszCmd = (wchar_t*)malloc(cchCmdLine*sizeof(*pszCmd));
	if (!pszCmd)
	{
		iRc = -11;
		goto wrap;
	}

	*pszCmd = L'"';
	lstrcpy(pszCmd+1, GuiMapping->sConEmuBaseDir);
	pszName = pszCmd + lstrlen(pszCmd);
	lstrcpy(pszName, WIN3264TEST(L"\\ConEmuC.exe",L"\\ConEmuC64.exe"));
	bSrvFound = FileExists(pszCmd+1);
	#ifdef _WIN64
	if (!bSrvFound)
	{
		// On 64-bit OS may be "ConEmuC64.exe" was not installed? (weird, but possible)
		lstrcpy(pszName, L"\\ConEmuC.exe");
		bSrvFound = FileExists(pszCmd+1);
	}
	#endif
	if (!bSrvFound)
	{
		iRc = -12;
		goto wrap;
	}

	pszName = pszCmd + lstrlen(pszCmd);

	if (!Duplicate->bRunAs)
	{
		*(pszName++) = L'"';
		*pszName = 0;
	}
	
	msprintf(pszName, cchCmdLine-(pszName-pszCmd),
		L" /ATTACH /GID=%u /GHWND=%08X /AID=%u /TA=%08X /BW=%i /BH=%i /BZ=%i /HIDE /ROOT %s",
		Duplicate->nGuiPID, (DWORD)Duplicate->hGuiWnd, Duplicate->nAID,
		Duplicate->nColors, Duplicate->nWidth, Duplicate->nHeight, Duplicate->nBufferHeight,
		gpStartEnv->pszCmdLine);

	si.dwFlags = STARTF_USESHOWWINDOW;

	if (!CreateProcess(NULL, pszCmd, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
	{
		iRc = GetLastError();
		goto wrap;
	}

	iRc = 0;
wrap:
	SafeFree(GuiMapping);
	SafeFree(pszCmd);
	SafeCloseHandle(pi.hProcess);
	SafeCloseHandle(pi.hThread);
	return iRc;
}

// GetConsoleWindow ��������, �������, ��� ��������� ��������� ����������� ����
// ����� ������� ��� �������������� �������
HWND WINAPI GetRealConsoleWindow()
{
	_ASSERTE(gfGetRealConsoleWindow);
	HWND hConWnd = gfGetRealConsoleWindow ? gfGetRealConsoleWindow() : NULL; //GetConsoleWindow();
#ifdef _DEBUG
	wchar_t sClass[64]; user->getClassNameW(hConWnd, sClass, countof(sClass));
	_ASSERTEX(hConWnd==NULL || isConsoleClass(sClass));
#endif
	return hConWnd;
}


// ��� ���������� ����� - ������ �������� ������, ������ ����� ������������ �� �� ������ (*pcbMaxReplySize)
BOOL WINAPI HookServerCommand(LPVOID pInst, CESERVER_REQ* pCmd, CESERVER_REQ* &ppReply, DWORD &pcbReplySize, DWORD &pcbMaxReplySize, LPARAM lParam)
{
	WARNING("����������, ���������� ������!");
	
	BOOL lbRc = FALSE, lbFRc;
	
	switch (pCmd->hdr.nCmd)
	{
	case CECMD_ATTACHGUIAPP:
		{
			// ��� '�������' ������ �������������� ������ �� ConEmu
			_ASSERTEX(pCmd->AttachGuiApp.hConEmuWnd && ghConEmuWnd==pCmd->AttachGuiApp.hConEmuWnd);
			//gbGuiClientHideCaption = pCmd->AttachGuiApp.bHideCaption;
			gGuiClientStyles = pCmd->AttachGuiApp.Styles;
			//ghConEmuWndDC -- ��� ����
			AttachGuiWindow(pCmd->AttachGuiApp.hAppWindow);
			// ���������
			pcbReplySize = sizeof(CESERVER_REQ_HDR)+sizeof(DWORD);
			lbRc = ExecuteNewCmd(ppReply, pcbMaxReplySize, pCmd->hdr.nCmd, pcbReplySize);
			if (lbRc)
				ppReply->dwData[0] = (DWORD)ghAttachGuiClient;
		} // CECMD_ATTACHGUIAPP
		break;
	case CECMD_SETFOCUS:
		break;
	case CECMD_SETPARENT:
		break;
	case CECMD_CTRLBREAK:
		if (CHECK_CMD_SIZE(pCmd,2*sizeof(DWORD)))
		{
			lbFRc = GenerateConsoleCtrlEvent(pCmd->dwData[0], pCmd->dwData[1]);
			pcbReplySize = sizeof(CESERVER_REQ_HDR)+sizeof(DWORD);
			lbRc = ExecuteNewCmd(ppReply, pcbMaxReplySize, pCmd->hdr.nCmd, pcbReplySize);
			if (lbRc)
				ppReply->dwData[0] = lbFRc;
		} // CECMD_CTRLBREAK
		break;
	case CECMD_SETGUIEXTERN:
		if (ghAttachGuiClient && (pCmd->DataSize() >= sizeof(CESERVER_REQ_SETGUIEXTERN)))
		{
			SetGuiExternMode(pCmd->SetGuiExtern.bExtern, NULL/*pCmd->SetGuiExtern.bDetach ? &pCmd->SetGuiExtern.rcOldPos : NULL*/);
			pcbReplySize = sizeof(CESERVER_REQ_HDR)+sizeof(DWORD);
			lbRc = ExecuteNewCmd(ppReply, pcbMaxReplySize, pCmd->hdr.nCmd, pcbReplySize);
			if (lbRc)
				ppReply->dwData[0] = gbGuiClientExternMode;

			if (pCmd->SetGuiExtern.bExtern && pCmd->SetGuiExtern.bDetach)
			{
				gbAttachGuiClient = gbGuiClientAttached = FALSE;
				ghAttachGuiClient = NULL;
				ghConEmuWnd = NULL;
				SetConEmuHkWindows(NULL, NULL);
				gnServerPID = 0;
			}

		} // CECMD_SETGUIEXTERN
		break;
	case CECMD_LANGCHANGE:
		{
			LONG_PTR hkl = (LONG_PTR)(LONG)pCmd->dwData[0];
			BOOL lbRc = ActivateKeyboardLayout((HKL)hkl, KLF_SETFORPROCESS) != NULL;
			DWORD nErrCode = lbRc ? 0 : GetLastError();
			pcbReplySize = sizeof(CESERVER_REQ_HDR)+2*sizeof(DWORD);
			if (ExecuteNewCmd(ppReply, pcbMaxReplySize, pCmd->hdr.nCmd, pcbReplySize))
			{
				ppReply->dwData[0] = lbRc;
				ppReply->dwData[1] = nErrCode;
			}
		} // CECMD_LANGCHANGE
		break;
	case CECMD_MOUSECLICK:
	case CECMD_BSDELETEWORD:
		{
			BOOL bProcessed = FALSE;
			if ((gReadConsoleInfo.InReadConsoleTID || gReadConsoleInfo.LastReadConsoleInputTID)
				&& (pCmd->DataSize() >= sizeof(CESERVER_REQ_PROMPTACTION)))
			{
				switch (pCmd->hdr.nCmd)
				{
				case CECMD_MOUSECLICK:
					bProcessed = OnReadConsoleClick(pCmd->Prompt.xPos, pCmd->Prompt.yPos, (pCmd->Prompt.Force != 0), (pCmd->Prompt.BashMargin != 0));
					break;
				case CECMD_BSDELETEWORD:
					bProcessed = OnPromptBsDeleteWord((pCmd->Prompt.Force != 0), (pCmd->Prompt.BashMargin != 0));
					break;
				default:
					_ASSERTEX(FALSE && "Undefined action");
				}
			}

			lbRc = TRUE;
			pcbReplySize = sizeof(CESERVER_REQ_HDR)+sizeof(DWORD);
			if (ExecuteNewCmd(ppReply, pcbMaxReplySize, pCmd->hdr.nCmd, pcbReplySize))
			{
				ppReply->dwData[0] = bProcessed;
			}
		} // CECMD_MOUSECLICK, CECMD_BSDELETEWORD
		break;
	case CECMD_PROMPTCMD:
		{
			BOOL bProcessed = FALSE;
			if ((gReadConsoleInfo.InReadConsoleTID || gReadConsoleInfo.LastReadConsoleInputTID)
				&& (pCmd->DataSize() >= 2*sizeof(wchar_t)))
			{
				bProcessed = OnExecutePromptCmd((LPCWSTR)pCmd->wData);
			}

			lbRc = TRUE;
			pcbReplySize = sizeof(CESERVER_REQ_HDR)+sizeof(DWORD);
			if (ExecuteNewCmd(ppReply, pcbMaxReplySize, pCmd->hdr.nCmd, pcbReplySize))
			{
				ppReply->dwData[0] = bProcessed;
			}
		}
		break;
	case CECMD_STARTSERVER:
		{
			int nErrCode = -1;
			wchar_t szSelf[MAX_PATH+16], *pszNamePtr, szArgs[128];
			PROCESS_INFORMATION pi = {};
			STARTUPINFO si = {sizeof(si)};

			if (GetModuleFileName(ghOurModule, szSelf, MAX_PATH) && ((pszNamePtr = (wchar_t*)PointToName(szSelf)) != NULL))
			{
				// ��������� ������ ��� �� ��������, ��� � ������� �������
				_wcscpy_c(pszNamePtr, 16, WIN3264TEST(L"ConEmuC.exe",L"ConEmuC64.exe"));
				if (gnImageSubsystem==IMAGE_SUBSYSTEM_WINDOWS_GUI)
				{
					_ASSERTEX(pCmd->NewServer.hAppWnd!=0);
					_wsprintf(szArgs, SKIPLEN(countof(szArgs)) L" /GID=%u /GHWND=%08X /GUIATTACH=%08X /PID=%u",
							pCmd->NewServer.nGuiPID, (DWORD)pCmd->NewServer.hGuiWnd, (DWORD)pCmd->NewServer.hAppWnd, GetCurrentProcessId());
					gbAttachGuiClient = TRUE;
				}
				else
				{
					_ASSERTEX(pCmd->NewServer.hAppWnd==0);
					_wsprintf(szArgs, SKIPLEN(countof(szArgs)) L" /GID=%u /GHWND=%08X /ATTACH /PID=%u",
						pCmd->NewServer.nGuiPID, (DWORD)pCmd->NewServer.hGuiWnd, GetCurrentProcessId());
				}
				lbRc = CreateProcess(szSelf, szArgs, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);
				if (lbRc)
				{
					CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
					nErrCode = 0;
				}
				else
				{
					nErrCode = HRESULT_FROM_WIN32(GetLastError());
				}
			}

			lbRc = true; // ������� ��������� ����������

			pcbReplySize = sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_START);
			if (ExecuteNewCmd(ppReply, pcbMaxReplySize, pCmd->hdr.nCmd, pcbReplySize))
			{
				ppReply->dwData[0] = pi.dwProcessId;
				ppReply->dwData[1] = (DWORD)nErrCode;
			}
		} // CECMD_STARTSERVER
		break;
	case CECMD_EXPORTVARS:
		{
			LPCWSTR pszSrc = (LPCWSTR)pCmd->wData;
			while (*pszSrc)
			{
				LPCWSTR pszName = pszSrc;
				LPCWSTR pszVal = pszName + lstrlen(pszName) + 1;
				LPCWSTR pszNext = pszVal + lstrlen(pszVal) + 1;
				// Skip ConEmu's internals!
				if (lstrcmpni(pszName, L"ConEmu", 6) != 0)
				{
					SetEnvironmentVariableW(pszName, pszVal);
				}
				pszSrc = pszNext;
			}

			lbRc = true; // ������� ��������� ����������

			pcbReplySize = sizeof(CESERVER_REQ_HDR)+sizeof(DWORD);
			if (ExecuteNewCmd(ppReply, pcbMaxReplySize, pCmd->hdr.nCmd, pcbReplySize))
			{
				ppReply->dwData[0] = TRUE;
			}
		} // CECMD_EXPORTVARS
		break;
	case CECMD_DUPLICATE:
		{
			int nFRc = DuplicateRoot(&pCmd->Duplicate);

			lbRc = true; // ������� ���������
			pcbReplySize = sizeof(CESERVER_REQ_HDR)+sizeof(DWORD);
			if (ExecuteNewCmd(ppReply, pcbMaxReplySize, pCmd->hdr.nCmd, pcbReplySize))
			{
				ppReply->dwData[0] = nFRc;
			}
		} // CECMD_DUPLICATE
		break;
	}
	
	// ���� (lbRc == FALSE) - � ���� ����� ������ "��������" ((DWORD)0)
	return lbRc;
}

// ���������� ����� ����, ��� ������ Pipe Instance
BOOL WINAPI HookServerReady(LPVOID pInst, LPARAM lParam)
{
	return TRUE;
}

// ���������� ������, ���������� ��� ���������
void WINAPI HookServerFree(CESERVER_REQ* pReply, LPARAM lParam)
{
	ExecuteFreeResult(pReply);
}


#ifdef _DEBUG
LONG WINAPI HkExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo)
{
	wchar_t szTitle[128], szText[MAX_PATH*2]; szText[0] = 0;
	msprintf(szTitle, countof(szTitle), L"Exception, PID=%u", GetCurrentProcessId(), GetCurrentThreadId());
	GetModuleFileName(NULL, szText, countof(szText));
	int nBtn = GuiMessageBox(ghConEmuWnd, szText, szTitle, MB_RETRYCANCEL|MB_SYSTEMMODAL);
	return (nBtn == IDRETRY) ? EXCEPTION_CONTINUE_SEARCH : EXCEPTION_EXECUTE_HANDLER;
}

int main()
{
	_CrtDbgBreak();
	return 0;
}
#endif

int WINAPI RequestLocalServer(/*[IN/OUT]*/RequestLocalServerParm* Parm)
{
	//_ASSERTE(FALSE && "ConEmuHk. Continue to RequestLocalServer");

	int iRc = CERR_SRVLOADFAILED;
	if (!Parm || (Parm->StructSize != sizeof(*Parm)))
	{
		iRc = CERR_CARGUMENT;
		goto wrap;
	}
	//RequestLocalServerParm Parm = {(DWORD)sizeof(Parm)};

	if (Parm->Flags & slsf_ReinitWindows)
	{
		if (!GetConMap(TRUE))
		{
			SetConEmuHkWindows(NULL, NULL);
		}

		if ((Parm->Flags & slsf_ReinitWindows) == Parm->Flags)
		{
			iRc = 0;
			goto wrap;
		}
	}

	if (Parm->Flags & slsf_AltServerStopped)
	{
		iRc = 0;
		// SendStopped ���������� �� DllStop!
		goto wrap;
	}

	if (!ghSrvDll || !gfRequestLocalServer)
	{
		LPCWSTR pszSrvName = WIN3264TEST(L"ConEmuCD.dll",L"ConEmuCD64.dll");

		if (!ghSrvDll)
		{
			gfRequestLocalServer = NULL;
			ghSrvDll = GetModuleHandle(pszSrvName);
		}

		if (!ghSrvDll)
		{
			wchar_t *pszSlash, szFile[MAX_PATH+1] = {};

			GetModuleFileName(ghOurModule, szFile, MAX_PATH);
			pszSlash = wcsrchr(szFile, L'\\');
			if (!pszSlash)
				goto wrap;
			pszSlash[1] = 0;
			wcscat_c(szFile, pszSrvName);

			ghSrvDll = LoadLibrary(szFile);
			if (!ghSrvDll)
				goto wrap;
		}

		gfRequestLocalServer = (RequestLocalServer_t)GetProcAddress(ghSrvDll, "PrivateEntry");
	}

	if (!gfRequestLocalServer)
		goto wrap;

	_ASSERTE(CheckCallbackPtr(ghSrvDll, 1, (FARPROC*)&gfRequestLocalServer, TRUE));

	//iRc = gfRequestLocalServer(&gpAnnotationHeader, &ghCurrentOutBuffer);
	iRc = gfRequestLocalServer(Parm);

	if  ((iRc == 0) && (Parm->Flags & slsf_PrevAltServerPID))
	{
		gnPrevAltServerPID = Parm->nPrevAltServerPID;
	}
wrap:
	return iRc;
}

// When _st_ is 0: remove progress.
// When _st_ is 1: set progress value to _pr_ (number, 0-100).
// When _st_ is 2: set error state in progress on Windows 7 taskbar
void GuiSetProgress(WORD st, WORD pr, LPCWSTR pszName /*= NULL*/)
{
	int nLen = pszName ? (lstrlen(pszName) + 1) : 1;
	CESERVER_REQ* pIn = ExecuteNewCmd(CECMD_SETPROGRESS, sizeof(CESERVER_REQ_HDR)+sizeof(WORD)*(2+nLen));
	if (pIn)
	{
		pIn->wData[0] = st;
		pIn->wData[1] = pr;
		if (pszName)
		{
			lstrcpy((wchar_t*)(pIn->wData+2), pszName);
		}

		CESERVER_REQ* pOut = ExecuteGuiCmd(ghConWnd, pIn, ghConWnd);
		ExecuteFreeResult(pIn);
		ExecuteFreeResult(pOut);
	}
}
