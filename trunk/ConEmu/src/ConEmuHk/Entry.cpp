
/*
Copyright (c) 2009-2011 Maximus5
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
//  #define SHOW_STARTED_MSGBOX
//  #define SHOW_INJECT_MSGBOX
//  #define SHOW_MINGW_MSGBOX
#endif
//#define SHOW_INJECT_MSGBOX
//#define SHOW_STARTED_MSGBOX

#ifdef _DEBUG
	#define USE_PIPE_SERVER
#else
	#undef USE_PIPE_SERVER
#endif

#include <windows.h>

#ifndef TESTLINK
#include "../common/common.hpp"
#include "../common/ConEmuCheck.h"
#include "../common/execute.h"
#endif
#include "../common/PipeServer.h"

#ifdef _DEBUG
#include "DbgHooks.h"
#endif

#include "ConEmuHooks.h"
#include "RegHooks.h"
#include "ShellProcessor.h"


#if defined(__GNUC__)
extern "C" {
	BOOL WINAPI DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved);
	//LRESULT CALLBACK LLKeybHook(int nCode,WPARAM wParam,LPARAM lParam);
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

HMODULE ghOurModule = NULL; // ConEmu.dll - ��� ������
//UINT gnMsgActivateCon = 0; //RegisterWindowMessage(CONEMUMSG_LLKEYHOOK);
//SECURITY_ATTRIBUTES* gpLocalSecurity = NULL;

#define isPressed(inp) ((GetKeyState(inp) & 0x8000) == 0x8000)

extern HANDLE ghHeap;
extern HMODULE ghKernel32, ghUser32, ghShell32, ghAdvapi32, ghComdlg32;
extern const wchar_t *kernel32;// = L"kernel32.dll";
extern const wchar_t *user32  ;// = L"user32.dll";
extern const wchar_t *shell32 ;// = L"shell32.dll";
extern const wchar_t *advapi32;// = L"Advapi32.dll";
extern const wchar_t *comdlg32;// = L"comdlg32.dll";

//BOOL gbSkipInjects = FALSE;
BOOL gbHooksWasSet = FALSE;

UINT_PTR gfnLoadLibrary = NULL;
extern BOOL StartupHooks(HMODULE ahOurDll);
extern void ShutdownHooks();
extern void InitializeHookedModules();
extern void FinalizeHookedModules();
extern BOOL MyAllowSetForegroundWindow(DWORD dwProcessId);
extern DWORD MyGetWindowThreadProcessId(HWND hWnd, LPDWORD lpdwProcessId);
//HMODULE ghPsApi = NULL;
#ifdef _DEBUG
extern HHOOK ghGuiClientRetHook;
//extern bool gbAllowAssertThread;
#endif

DWORD   gnSelfPID = 0;
DWORD   gnServerPID = 0;
DWORD   gnGuiPID = 0;
HWND    ghConWnd = NULL; // Console window
HWND    ghConEmuWnd = NULL; // Root! window
HWND    ghConEmuWndDC = NULL; // ConEmu DC window
RECT    grcConEmuClient = {}; // ��� ������ ������ ����
BOOL    gbAttachGuiClient = FALSE; // ��� ������ ������ ����
BOOL    gbWasBufferHeight = FALSE;
BOOL    gbNonGuiMode = FALSE;
DWORD   gnImageSubsystem = 0;
DWORD   gnImageBits = WIN3264TEST(32,64); //-V112

//MSection *gpHookCS = NULL;


#ifdef _DEBUG
LPTOP_LEVEL_EXCEPTION_FILTER gfnPrevFilter = NULL;
LONG WINAPI HkExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo);
#endif

void SendStarted();
void SendStopped();

/*
void __stdcall _chkstk()
{
	return;
}
*/

#ifdef HOOK_USE_DLLTHREAD
HANDLE ghStartThread = NULL;
DWORD  gnStartThreadID = 0;
#endif


MFileMapping<CESERVER_CONSOLE_MAPPING_HDR> *gpConMap = NULL;
CESERVER_CONSOLE_MAPPING_HDR* gpConInfo = NULL;

CESERVER_CONSOLE_MAPPING_HDR* GetConMap()
{
	if (gpConInfo)
		return gpConInfo;
	
	if (!gpConMap)
	{
		gpConMap = new MFileMapping<CESERVER_CONSOLE_MAPPING_HDR>;
		gpConMap->InitName(CECONMAPNAME, (DWORD)ghConWnd); //-V205
	}
	
	if (!gpConInfo)
	{
		gpConInfo = gpConMap->Open();
	}
	
	if (gpConInfo)
	{
		if (gpConInfo->cbSize >= sizeof(CESERVER_CONSOLE_MAPPING_HDR))
		{
			gnGuiPID = gpConInfo->nGuiPID;
			ghConEmuWnd = gpConInfo->hConEmuRoot;
			ghConEmuWndDC = gpConInfo->hConEmuWnd;
			_ASSERTE(ghConEmuWndDC && IsWindow(ghConEmuWndDC));
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
	
	return gpConInfo;
}

BOOL WINAPI HookServerCommand(CESERVER_REQ* pCmd, CESERVER_REQ** ppReply, DWORD* pcbReplySize, DWORD* pcbMaxReplySize, LPARAM lParam);
BOOL WINAPI HookServerReady(LPARAM lParam);
void WINAPI HookServerFree(CESERVER_REQ* pReply, LPARAM lParam);

#ifdef USE_PIPE_SERVER
PipeServer<CESERVER_REQ,1> *gpHookServer = NULL;
#endif

DWORD WINAPI DllStart(LPVOID /*apParm*/)
{
	InitializeHookedModules();

	//HANDLE hStartedEvent = (HANDLE)apParm;
	#ifdef _DEBUG
	wchar_t *szModule = (wchar_t*)calloc((MAX_PATH+1),sizeof(wchar_t));
	if (!GetModuleFileName(NULL, szModule, MAX_PATH+1))
		_wcscpy_c(szModule, MAX_PATH+1, L"GetModuleFileName failed");
	const wchar_t* pszName = PointToName(szModule);
	if (!lstrcmpi(pszName, L"mingw32-make.exe"))
	{
		#ifdef SHOW_MINGW_MSGBOX
		// GuiMessageBox ��� �� ��������, ������ �� ����������������
		HMODULE hUser = LoadLibrary(L"user32.dll");
		typedef int (WINAPI* MessageBoxW_t)(HWND hWnd,LPCTSTR lpText,LPCTSTR lpCaption,UINT uType);
		if (hUser)
		{
			MessageBoxW_t MsgBox = (MessageBoxW_t)GetProcAddress(hUser, "MessageBoxW");
			if (MsgBox)
			{
				MsgBox(NULL, L"mingw32-make.exe loaded!", L"ConEmuHk", MB_SYSTEMMODAL);
			}
			FreeLibrary(hUser);
		}
		#endif
	}
	#endif
	
	// ��������� ��������� � �������� ����� ���� ���-�� �����������, ����� ������ �����
	// iFindAddress = FindKernelAddress(pi.hProcess, pi.dwProcessId, &fLoadLibrary);
	HMODULE hKernel = ::GetModuleHandle(L"kernel32.dll");
	if (hKernel)
	{
		gfnLoadLibrary = (UINT_PTR)::GetProcAddress(hKernel, "LoadLibraryW");
		_ASSERTE(gfnLoadLibrary!=NULL);
	}
	else
	{
		_ASSERTE(hKernel!=NULL);
		gfnLoadLibrary = NULL;
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
		GetConMap();

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

	
	// ���������� ���������� �������� � ��� (CUI/GUI) ��������, � ������� ��� ���������
	gnImageBits = WIN3264TEST(32,64);
	gnImageSubsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
	// ��������� ��� (CUI/GUI)
	GetImageSubsystem(gnImageSubsystem, gnImageBits);
	// �������� ���� ����������
	_ASSERTE(gnImageBits==WIN3264TEST(32,64));
	_ASSERTE(gnImageSubsystem==IMAGE_SUBSYSTEM_WINDOWS_GUI || gnImageSubsystem==IMAGE_SUBSYSTEM_WINDOWS_CUI);
	
	
	BOOL lbGuiWindowAttach = FALSE; // ��������� � ConEmu ������ ��������� (notepad, putty, ...)

	
#ifdef USE_PIPE_SERVER
	_ASSERTEX(gpHookServer==NULL);
	gpHookServer = (PipeServer<CESERVER_REQ,1>*)calloc(1,sizeof(*gpHookServer));
	if (gpHookServer)
	{
		wchar_t szPipeName[128];
		_wsprintf(szPipeName, SKIPLEN(countof(szPipeName)) CEHOOKSPIPENAME, GetCurrentProcessId());
		if (!gpHookServer->StartPipeServer(szPipeName, HookServerCommand, HookServerReady, HookServerFree, (LPARAM)gpHookServer))
		{
			_ASSERTEX(FALSE); // ������ ������� Pipes?
			gpHookServer->StopPipeServer();
			free(gpHookServer);
			gpHookServer = NULL;
		}
	}
	else
	{
		_ASSERTEX(gpHookServer!=NULL);
	}
#endif

	//#ifndef SKIP_GETIMAGESUBSYSTEM_ONLOAD
	//GetImageSubsystem(nImageSubsystem,nImageBits);
	//#else
	//PRAGMA_ERROR("error: ����������� ������� ���������� ��� ������� � ConEmu �������� �� �����");
	//#endif
	

	WARNING("����������� �� �������� � �������, � ����� ��� �� ���������� ConEmuData");
	if (ghConWnd)
	{
		CShellProc* sp = new CShellProc;
		if (sp)
		{
			if (sp->LoadGuiMapping())
			{
			
				wchar_t *szExeName = (wchar_t*)calloc((MAX_PATH+1),sizeof(wchar_t));
				//BOOL lbDosBoxAllowed = FALSE;
				if (!GetModuleFileName(NULL, szExeName, MAX_PATH+1)) szExeName[0] = 0;
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
		DWORD dwConEmuHwnd = 0;
		wchar_t szVar[64], *psz;
		if (GetEnvironmentVariable(L"ConEmuHWND", szVar, countof(szVar)))
		{
			if (szVar[0] == L'0' && szVar[1] == L'x')
			{
				dwConEmuHwnd = wcstoul(szVar+2, &psz, 16);
				if (!IsWindow((HWND)dwConEmuHwnd))
					dwConEmuHwnd = 0;
				else if (!GetClassName((HWND)dwConEmuHwnd, szVar, countof(szVar)))
					dwConEmuHwnd = 0;
				else if (lstrcmp(szVar, VirtualConsoleClassMain) != 0)
					dwConEmuHwnd = 0;
			}
		}
		
		if (dwConEmuHwnd)
		{
			DWORD nSize = sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_ATTACHGUIAPP);
			CESERVER_REQ *pIn = (CESERVER_REQ*)calloc(nSize,1);
			ExecutePrepareCmd(pIn, CECMD_ATTACHGUIAPP, nSize);
			pIn->AttachGuiApp.nPID = GetCurrentProcessId();
			GetModuleFileName(NULL, pIn->AttachGuiApp.sAppFileName, countof(pIn->AttachGuiApp.sAppFileName));

			wchar_t szGuiPipeName[128];
			msprintf(szGuiPipeName, countof(szGuiPipeName), CEGUIPIPENAME, L".", dwConEmuHwnd);
			
			CESERVER_REQ* pOut = ExecuteCmd(szGuiPipeName, pIn, 1000, NULL);

			free(pIn);

			if (pOut)
			{
				if (pOut->hdr.cbSize > sizeof(CESERVER_REQ_HDR))
				{
					if (pOut->AttachGuiApp.bOk)
					{
						MyAllowSetForegroundWindow(pOut->hdr.nSrcPID); // PID ConEmu.
						ghConEmuWnd = (HWND)dwConEmuHwnd;
						ghConEmuWndDC = pOut->AttachGuiApp.hWindow;
						_ASSERTE(ghConEmuWndDC && IsWindow(ghConEmuWndDC));
						grcConEmuClient = pOut->AttachGuiApp.rcWindow;
						gnServerPID = pOut->AttachGuiApp.nPID;
						gbAttachGuiClient = TRUE;
					}
				}
				ExecuteFreeResult(pOut);
			}
		}
	}

	//if (!gbSkipInjects)
	{
		//gnRunMode = RM_APPLICATION;

		#ifdef _DEBUG
		//wchar_t szModule[MAX_PATH+1]; szModule[0] = 0;
		//GetModuleFileName(NULL, szModule, countof(szModule));
		_ASSERTE((gnImageSubsystem==IMAGE_SUBSYSTEM_WINDOWS_CUI) || (lstrcmpi(pszName, L"DosBox.exe")==0) || gbAttachGuiClient);
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

		gbHooksWasSet = StartupHooks(ghOurModule);

		#ifdef _DEBUG
		//}
		#endif

		// ���� NULL - ������ ��� "Detached" ���������� �������, �������� "Started" � ������ ������ ���
		if (ghConWnd != NULL)
		{
			SendStarted();

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
	free(szModule);
	#endif
	*/
	
	//if (hStartedEvent)
	//	SetEvent(hStartedEvent);
	
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

void DllStop()
{
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

#ifdef USE_PIPE_SERVER
	if (gpHookServer)
	{
		gpHookServer->StopPipeServer();
		free(gpHookServer);
		gpHookServer = NULL;
	}
#endif
	
	#ifdef _DEBUG
	if (ghGuiClientRetHook)
		UnhookWindowsHookEx(ghGuiClientRetHook);
	#endif

	if (/*!gbSkipInjects &&*/ gbHooksWasSet)
	{
		gbHooksWasSet = FALSE;
		// ��������� ������ � ��������
		DoneHooksReg();
		// "�������" ����
		ShutdownHooks();
	}

	//if (gnRunMode == RM_APPLICATION)
	//{
	SendStopped();
	//}

	if (gpConMap)
	{
		gpConMap->CloseMap();
		gpConInfo = NULL;
		delete gpConMap;
		gpConMap = NULL;
	}
	
	//#ifndef TESTLINK
	CommonShutdown();

	//if (gpHookCS)
	//{
	//	MSection *p = gpHookCS;
	//	gpHookCS = NULL;
	//	delete p;
	//}

	FinalizeHookedModules();

	//ReleaseConsoleInputSemaphore();

	//#endif
	//if (ghHeap)
	//{
	//	HeapDestroy(ghHeap);
	//	ghHeap = NULL;
	//}
	HeapDeinitialize();
	
	#ifdef _DEBUG
	// ?gfnPrevFilter?
	// �������. A value of NULL for this parameter specifies default handling within UnhandledExceptionFilter.
	SetUnhandledExceptionFilter(NULL);
	#endif
}

BOOL WINAPI DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	BOOL lbAllow = TRUE;

	switch(ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			#ifdef _DEBUG
			HANDLE hProcHeap = GetProcessHeap();
			#endif
			HeapInitialize();
			
			ghOurModule = (HMODULE)hModule;
			ghConWnd = GetConsoleWindow();
			gnSelfPID = GetCurrentProcessId();
			ghWorkingModule = (u64)hModule;
			gfGetRealConsoleWindow = GetConsoleWindow;

			#ifdef _DEBUG
			gAllowAssertThread = am_Pipe;
			#endif
			
			#ifdef _DEBUG
			gfnPrevFilter = SetUnhandledExceptionFilter(HkExceptionFilter);
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

			//_ASSERTE(ghHeap == NULL);
			//ghHeap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 200000, 0);
			
			#ifdef HOOK_USE_DLLTHREAD
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
		}
		break;
		
		case DLL_THREAD_ATTACH:
		{
			if (gbHooksWasSet)
				InitHooksRegThread();
		}
		break;
		case DLL_THREAD_DETACH:
		{
			if (gbHooksWasSet)
				DoneHooksRegThread();
		}
		break;
		
		case DLL_PROCESS_DETACH:
		{
			if (gbHooksWasSet)
				lbAllow = FALSE; // ����� ��������, �.�. FreeLibrary �����������
			DllStop();
		}
		break;
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
//		//	wsprintf(szKH+lstrlen(szKH)-1, L" - WinDelta=%i\n", (pKB->time - gnWinPressTick));
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
	//static bool bSent = false;
	//
	//if (bSent)
	//	return; // �������� ������ ���� ���
	//
	////crNewSize = gpSrv->sbi.dwSize;
	////_ASSERTE(crNewSize.X>=MIN_CON_WIDTH && crNewSize.Y>=MIN_CON_HEIGHT);
	//HWND hConWnd = GetConsoleWindow();
	//
	//if (!gnSelfPID)
	//{
	//	_ASSERTE(gnSelfPID!=0);
	//	gnSelfPID = GetCurrentProcessId();
	//}

	//if (!hConWnd)
	//{
	//	// ��� Detached �������. ������ ����� ������� ������ COMSPEC
	//	_ASSERTE(gnRunMode == RM_COMSPEC);
	//	gbNonGuiMode = TRUE; // �� �������� ExecuteGuiCmd ��� ������. ��� �� ���� �������
	//	return;
	//}
	//
	//_ASSERTE(hConWnd == ghConWnd);
	//ghConWnd = hConWnd;

	//DWORD nServerPID = 0, nGuiPID = 0;

	// ��� ComSpec-� ����� ����� ���������, � ����-�� ������ � ���� �������...
	//if (gnRunMode /*== RM_COMSPEC*/ > RM_SERVER)
	//{

	//MFileMapping<CESERVER_CONSOLE_MAPPING_HDR> ConsoleMap;
	//ConsoleMap.InitName(CECONMAPNAME, (DWORD)ghConWnd);
	//const CESERVER_CONSOLE_MAPPING_HDR* pConsoleInfo = ConsoleMap.Open();

	////WCHAR sHeaderMapName[64];
	////StringCchPrintf(sHeaderMapName, countof(sHeaderMapName), CECONMAPNAME, (DWORD)hConWnd);
	////HANDLE hFileMapping = OpenFileMapping(FILE_MAP_READ/*|FILE_MAP_WRITE*/, FALSE, sHeaderMapName);
	////if (hFileMapping) {
	////	const CESERVER_CONSOLE_MAPPING_HDR* pConsoleInfo
	////		= (CESERVER_CONSOLE_MAPPING_HDR*)MapViewOfFile(hFileMapping, FILE_MAP_READ/*|FILE_MAP_WRITE*/,0,0,0);
	//if (pConsoleInfo)
	//{
	//	gnServerPID = pConsoleInfo->nServerPID;
	//	gnGuiPID = pConsoleInfo->nGuiPID;

	//	//if (pConsoleInfo->cbSize >= sizeof(CESERVER_CONSOLE_MAPPING_HDR))
	//	//{
	//	//	if (pConsoleInfo->nLogLevel)
	//	//		CreateLogSizeFile(pConsoleInfo->nLogLevel);
	//	//}

	//	//UnmapViewOfFile(pConsoleInfo);
	//	ConsoleMap.CloseMap();
	//}

	//	CloseHandle(hFileMapping);
	//}

	if (gnServerPID == 0)
	{
		gbNonGuiMode = TRUE; // �� �������� ExecuteGuiCmd ��� ������. ��� �� ���� �������
		return; // ����� ComSpec, �� ������� ���, ��������������, � GUI ������ �������� �� �����
	}

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

		//// Cmd/Srv ����� �����
		//switch (gnRunMode)
		//{
		//case RM_SERVER:
		//	pIn->StartStop.nStarted = sst_ServerStart; break;
		//case RM_COMSPEC:
		//	pIn->StartStop.nStarted = sst_ComspecStart; break;
		//default:
		pIn->StartStop.nStarted = sst_AppStart;
		//}
		pIn->StartStop.hWnd = ghConWnd;
		pIn->StartStop.dwPID = gnSelfPID;
		pIn->StartStop.nImageBits = WIN3264TEST(32,64);
		//TODO("Ntvdm/DosBox -> 16");

		////pIn->StartStop.dwInputTID = (gnRunMode == RM_SERVER) ? gpSrv->dwInputThreadId : 0;
		//if (gnRunMode == RM_SERVER)
		//	pIn->StartStop.bUserIsAdmin = IsUserAdmin();

		//// ����� �������� 16��� ���������� ����� ������������ �������...
		//gnImageSubsystem = 0;
		//LPCWSTR pszTemp = gpszRunCmd;
		//wchar_t lsRoot[MAX_PATH+1] = {0};
		//
		//if (gnRunMode == RM_SERVER && gpSrv->bDebuggerActive)
		//{
		//	// "��������"
		//	gnImageSubsystem = 0x101;
		//	gbRootIsCmdExe = TRUE; // ����� ����� ��������
		//}
		//else if (/*!gpszRunCmd &&*/ gbAttachFromFar)
		//{
		//	// ����� �� ���-�������
		//	gnImageSubsystem = 0x100;
		//}
		//else if (gpszRunCmd && ((0 == NextArg(&pszTemp, lsRoot))))
		//{
		//	PRINT_COMSPEC(L"Starting: <%s>", lsRoot);
		//
		//	DWORD nImageFileAttr = 0;
		//	if (!GetImageSubsystem(lsRoot, gnImageSubsystem, gnImageBits, nImageFileAttr))
		//		gnImageSubsystem = 0;
		//
		//	PRINT_COMSPEC(L", Subsystem: <%i>\n", gnImageSubsystem);
		//	PRINT_COMSPEC(L"  Args: %s\n", pszTemp);
		//}
		//else
		//{

		// -- ���, � DllStart
		//// ���������� ���������� �������� � ��� (CUI/GUI) ��������, � ������� ��� ���������
		//GetImageSubsystem(gnImageSubsystem, gnImageBits);

		pIn->StartStop.nSubSystem = gnImageSubsystem;
		//pIn->StartStop.bRootIsCmdExe = gbRootIsCmdExe; //2009-09-14
		// �� MyGet..., � �� ����� ���������������...
		HANDLE hOut = NULL;

		//if (gnRunMode == RM_SERVER)
		//	hOut = (HANDLE)ghConOut;
		//else
		hOut = GetStdHandle(STD_OUTPUT_HANDLE);

		//DWORD dwErr1 = 0;
		//BOOL lbRc1 =
		GetConsoleScreenBufferInfo(hOut, &pIn->StartStop.sbi);

		//if (!lbRc1) dwErr1 = GetLastError();
		//
		//PRINT_COMSPEC(L"Starting %s mode (ExecuteGuiCmd started)\n",(RunMode==RM_SERVER) ? L"Server" : L"ComSpec");
		//// CECMD_CMDSTARTSTOP
		//if (gnRunMode == RM_APPLICATION)
		pOut = ExecuteSrvCmd(gnServerPID, pIn, ghConWnd);
		//else
		//	pOut = ExecuteGuiCmd(ghConWnd, pIn, ghConWnd);

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
		//PRINT_COMSPEC(L"Starting %s mode (ExecuteGuiCmd finished)\n",(RunMode==RM_SERVER) ? L"Server" : L"ComSpec");

		if (pOut)
		{
			//bSent = true;
			gbWasBufferHeight = pOut->StartStopRet.bWasBufferHeight;
			gnGuiPID = pOut->StartStopRet.dwPID;
			ghConEmuWnd = pOut->StartStopRet.hWnd;
			ghConEmuWndDC = pOut->StartStopRet.hWndDC;
			_ASSERTE(ghConEmuWndDC && IsWindow(ghConEmuWndDC));
			//if (gnRunMode == RM_SERVER)
			//{
			//	if (gpSrv)
			//	{
			//		gpSrv->bWasDetached = FALSE;
			//	}
			//	else
			//	{
			//		_ASSERTE(gpSrv!=NULL);
			//	}
			//}

			//UpdateConsoleMapHeader();

			gnServerPID = pOut->StartStopRet.dwSrvPID;

			
			// -- �� ����� ����� ���� ������, ������ ������� ��� ��� ��� �����������
			//AllowSetForegroundWindow(gnGuiPID);


			//gnBufferHeight  = (SHORT)pOut->StartStopRet.nBufferHeight;
			//gcrBufferSize.X = (SHORT)pOut->StartStopRet.nWidth;
			//gcrBufferSize.Y = (SHORT)pOut->StartStopRet.nHeight;
			//gbParmBufferSize = TRUE;

			//if (gnRunMode == RM_SERVER)
			//{
			//	if (gpSrv->bDebuggerActive && !gnBufferHeight) gnBufferHeight = 1000;
			//
			//	SMALL_RECT rcNil = {0};
			//	SetConsoleSize(gnBufferHeight, gcrBufferSize, rcNil, "::SendStarted");
			//
			//	// ����� ��������� ����������
			//	if (pOut->StartStopRet.bNeedLangChange)
			//	{
			//#ifndef INPUTLANGCHANGE_SYSCHARSET
			//#define INPUTLANGCHANGE_SYSCHARSET 0x0001
			//#endif
			//		WPARAM wParam = INPUTLANGCHANGE_SYSCHARSET;
			//		TODO("��������� �� x64, �� ����� �� ������� � 0xFFFFFFFFFFFFFFFFFFFFF");
			//		LPARAM lParam = (LPARAM)(DWORD_PTR)pOut->StartStopRet.NewConsoleLang;
			//		SendMessage(ghConWnd, WM_INPUTLANGCHANGEREQUEST, wParam, lParam);
			//	}
			//}
			//else
			//{
			//	// ����� ��� ����������, ��� ���� COMSPEC ������� �� �������.
			//	// 100628 - �����������. COMSPEC ������������ � cmd.exe
			//	//if (bAlreadyBufferHeight)
			//	//	gpSrv->bNonGuiMode = TRUE; // �� �������� ExecuteGuiCmd ��� ������ - ��������� ������ ��������
			//gbWasBufferHeight = bAlreadyBufferHeight;
			//}

			//nNewBufferHeight = ((DWORD*)(pOut->Data))[0];
			//crNewSize.X = (SHORT)((DWORD*)(pOut->Data))[1];
			//crNewSize.Y = (SHORT)((DWORD*)(pOut->Data))[2];
			//TODO("���� �� ������� ��� COMSPEC - �� � GUI �������� ��������� ����� �� ������");
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

void SendStopped()
{
	if (gbNonGuiMode || !gnServerPID)
		return;
	
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

		// �� MyGet..., � �� ����� ���������������...
		// ghConOut ����� ���� NULL, ���� ������ ��������� �� ����� ������� ����������
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &pIn->StartStop.sbi);

		pOut = ExecuteSrvCmd(gnServerPID, pIn, ghConWnd);

		ExecuteFreeResult(pIn); pIn = NULL;
		
		if (pOut)
		{
			ExecuteFreeResult(pOut);
			pOut = NULL;
		}
	}
}

// GetConsoleWindow ��������, �������, ��� ��������� ��������� ����������� ����
// ����� ������� ��� �������������� �������
HWND WINAPI GetRealConsoleWindow()
{
	_ASSERTE(gfGetRealConsoleWindow);
	HWND hConWnd = gfGetRealConsoleWindow ? gfGetRealConsoleWindow() : NULL; //GetConsoleWindow();
#ifdef _DEBUG
	wchar_t sClass[64]; GetClassName(hConWnd, sClass, countof(sClass));
	_ASSERTEX(hConWnd==NULL || lstrcmp(sClass, L"ConsoleWindowClass")==0);
#endif
	return hConWnd;
}


// ��� ���������� ����� - ������ �������� ������, ������ ����� ������������ �� �� ������ (*pcbMaxReplySize)
BOOL WINAPI HookServerCommand(CESERVER_REQ* pCmd, CESERVER_REQ** ppReply, DWORD* pcbReplySize, DWORD* pcbMaxReplySize, LPARAM lParam)
{
	TODO("����������, ���������� ������!");
	
	switch (pCmd->hdr.nCmd)
	{
	case CECMD_ATTACHGUIAPP:
		break;
	case CECMD_SETFOCUS:
		break;
	case CECMD_SETPARENT:
		break;
	}
	
	// ���� ��� ��� - ������ "��������"
	if (*ppReply && (*pcbMaxReplySize < sizeof(CESERVER_REQ_HDR)))
	{
		ExecuteFreeResult(*ppReply);
		*ppReply = NULL;
	}
	if (!*ppReply)
	{
		*pcbMaxReplySize = sizeof(CESERVER_REQ_HDR);
		*ppReply = ExecuteNewCmd(pCmd->hdr.nCmd, sizeof(CESERVER_REQ_HDR));
	}
	else
	{
		ExecutePrepareCmd(*ppReply, pCmd->hdr.nCmd, sizeof(CESERVER_REQ_HDR));
	}
	
	return TRUE;
}

// ���������� ����� ����, ��� ������ Pipe Instance
BOOL WINAPI HookServerReady(LPARAM lParam)
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
