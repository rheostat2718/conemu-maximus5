
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
//  #define SHOW_STARTED_MSGBOX
//  #define SHOW_INJECT_MSGBOX
  #define SHOW_EXE_MSGBOX // �������� ��������� ��� �������� � ������������ exe-���� (SHOW_EXE_MSGBOX_NAME)
  #define SHOW_EXE_MSGBOX_NAME L"vlc.exe"
#endif
//#define SHOW_INJECT_MSGBOX
//#define SHOW_STARTED_MSGBOX

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

#ifdef _DEBUG
#include "DbgHooks.h"
#endif

#include "ConEmuHooks.h"
#include "RegHooks.h"
#include "ShellProcessor.h"
#include "UserImp.h"
#include "GuiAttach.h"
#include "Injects.h"


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

extern DWORD  gnHookMainThreadId;
extern HANDLE ghHeap;
extern HMODULE ghKernel32, ghUser32, ghShell32, ghAdvapi32, ghComdlg32;
extern const wchar_t *kernel32;// = L"kernel32.dll";
extern const wchar_t *user32  ;// = L"user32.dll";
extern const wchar_t *shell32 ;// = L"shell32.dll";
extern const wchar_t *advapi32;// = L"Advapi32.dll";
extern const wchar_t *comdlg32;// = L"comdlg32.dll";

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
extern DWORD GetMainThreadId();
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
BOOL    gbWasBufferHeight = FALSE;
BOOL    gbNonGuiMode = FALSE;
DWORD   gnImageSubsystem = 0;
DWORD   gnImageBits = WIN3264TEST(32,64); //-V112

//MSection *gpHookCS = NULL;


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

#ifdef HOOK_USE_DLLTHREAD
HANDLE ghStartThread = NULL;
DWORD  gnStartThreadID = 0;
#endif


MFileMapping<CESERVER_CONSOLE_MAPPING_HDR> *gpConMap = NULL;
CESERVER_CONSOLE_MAPPING_HDR* gpConInfo = NULL;

CESERVER_CONSOLE_MAPPING_HDR* GetConMap(BOOL abForceRecreate/*=FALSE*/)
{
	if (gpConInfo && !abForceRecreate)
		return gpConInfo;
	
	if (!gpConMap || abForceRecreate)
	{
		if (!gpConMap)
			gpConMap = new MFileMapping<CESERVER_CONSOLE_MAPPING_HDR>;
		if (!gpConMap)
		{
			gpConInfo = NULL;
			return NULL;
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
			ghConEmuWndDC = gpConInfo->hConEmuWnd;
			_ASSERTE(ghConEmuWndDC && user->isWindow(ghConEmuWndDC));
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

void OnConWndChanged(HWND ahNewConWnd)
{
	//BOOL lbForceReopen = FALSE;

	if (ahNewConWnd)
	{
		#ifdef _DEBUG
		if (user)
		{
			wchar_t sClass[64]; user->getClassNameW(ahNewConWnd, sClass, countof(sClass));
			_ASSERTEX(lstrcmp(sClass, L"ConsoleWindowClass")==0);
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
	#endif

	#ifdef SHOW_EXE_MSGBOX
		if (!lstrcmpi(pszName, SHOW_EXE_MSGBOX_NAME))
		{
			// GuiMessageBox ��� �� ��������, ������ �� ����������������
			HMODULE hUser = LoadLibrary(L"user32.dll");
			typedef int (WINAPI* MessageBoxW_t)(HWND hWnd,LPCTSTR lpText,LPCTSTR lpCaption,UINT uType);
			if (hUser)
			{
				MessageBoxW_t MsgBox = (MessageBoxW_t)GetProcAddress(hUser, "MessageBoxW");
				if (MsgBox)
				{
					wchar_t szMsg[128]; lstrcpyn(szMsg, pszName, 96); lstrcat(szMsg, L" loaded!");
					wchar_t szTitle[64]; msprintf(szTitle, countof(szTitle), L"ConEmuHk, PID=%u, TID=%u", GetCurrentProcessId(), GetCurrentThreadId());
					MsgBox(NULL, szMsg, szTitle, MB_SYSTEMMODAL);
				}
				FreeLibrary(hUser);
			}
		}
	#endif
	
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
	gpHookServer = (PipeServer<CESERVER_REQ,1>*)calloc(1,sizeof(*gpHookServer));
	if (gpHookServer)
	{
		wchar_t szPipeName[128];
		msprintf(szPipeName, countof(szPipeName), CEHOOKSPIPENAME, L".", GetCurrentProcessId());
		bool lbOverlapped = true;
		if (!gpHookServer->StartPipeServer(szPipeName, (LPARAM)gpHookServer, LocalSecurity(), HookServerCommand, HookServerFree, NULL, NULL, HookServerReady, lbOverlapped))
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
			
			CESERVER_REQ* pOut = ExecuteCmd(szGuiPipeName, pIn, 1000, NULL);

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
						ghConEmuWnd = (HWND)dwConEmuHwnd;
						_ASSERTE(ghConEmuWnd==NULL || gnGuiPID!=0);
						ghConEmuWndDC = pOut->AttachGuiApp.hWindow;
						ghConWnd = pOut->AttachGuiApp.hSrvConWnd;
						_ASSERTE(ghConEmuWndDC && user->isWindow(ghConEmuWndDC));
						grcConEmuClient = pOut->AttachGuiApp.rcWindow;
						gnServerPID = pOut->AttachGuiApp.nPID;
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
	//gbDllStopCalled = TRUE; -- � �����

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
		user->unhookWindowsHookEx(ghGuiClientRetHook);
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
}

BOOL WINAPI DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	BOOL lbAllow = TRUE;

	switch(ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			gnDllState = ds_DllProcessAttach;
			#ifdef _DEBUG
			HANDLE hProcHeap = GetProcessHeap();
			#endif
			HeapInitialize();
			
			ghOurModule = (HMODULE)hModule;
			ghConWnd = GetConsoleWindow();
			gnSelfPID = GetCurrentProcessId();
			ghWorkingModule = (u64)hModule;
			gfGetRealConsoleWindow = GetConsoleWindow;
			user = (UserImp*)calloc(1, sizeof(*user));
			GetMainThreadId(); // ���������������� gnHookMainThreadId

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
			
			user->setAllowLoadLibrary();
		}
		break;
		
		case DLL_THREAD_ATTACH:
		{
			gnDllThreadCount++;
			if (gbHooksWasSet)
				InitHooksRegThread();
		}
		break;
		case DLL_THREAD_DETACH:
		{
			if (gbHooksWasSet)
				DoneHooksRegThread();
			// DLL_PROCESS_DETACH ������� ��� ���������� �� ������
			if (gnHookMainThreadId && (GetCurrentThreadId() == gnHookMainThreadId) && !gbDllDeinitialized)
			{
				gbDllDeinitialized = true;
				//WARNING!!! OutputDebugString must NOT be used from ConEmuHk::DllMain(DLL_PROCESS_DETACH). See Issue 465
				DllStop();
			}
			gnDllThreadCount--;
		}
		break;
		
		case DLL_PROCESS_DETACH:
		{
			gnDllState = ds_DllProcessDetach;
			if (gbHooksWasSet)
				lbAllow = FALSE; // ����� ��������, �.�. FreeLibrary �����������
			// ��� ����� ������� � DLL_THREAD_DETACH
			if (!gbDllDeinitialized)
			{
				gbDllDeinitialized = true;
				//WARNING!!! OutputDebugString must NOT be used from ConEmuHk::DllMain(DLL_PROCESS_DETACH). See Issue 465
				DllStop();
			}
			// -- free �� �����, �.�. ��� ������ HeapDeinitialize()
			//free(user);
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

		pIn->StartStop.crMaxSize = GetLargestConsoleWindowSize(hOut);

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
			_ASSERTE(ghConEmuWnd==NULL || gnGuiPID!=0);
			ghConEmuWndDC = pOut->StartStopRet.hWndDC;
			_ASSERTE(ghConEmuWndDC && user->isWindow(ghConEmuWndDC));
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

		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

		// �� MyGet..., � �� ����� ���������������...
		// ghConOut ����� ���� NULL, ���� ������ ��������� �� ����� ������� ����������
		GetConsoleScreenBufferInfo(hOut, &pIn->StartStop.sbi);

		pIn->StartStop.crMaxSize = GetLargestConsoleWindowSize(hOut);

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
	wchar_t sClass[64]; user->getClassNameW(hConWnd, sClass, countof(sClass));
	_ASSERTEX(hConWnd==NULL || lstrcmp(sClass, L"ConsoleWindowClass")==0);
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
		TODO("��� '�������' ������ �������������� ������ �� ConEmu");
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
		}
		break;
	case CECMD_SETGUIEXTERN:
		if (ghAttachGuiClient)
		{
			SetGuiExternMode(pCmd->dwData[0] != 0);
			pcbReplySize = sizeof(CESERVER_REQ_HDR)+sizeof(DWORD);
			lbRc = ExecuteNewCmd(ppReply, pcbMaxReplySize, pCmd->hdr.nCmd, pcbReplySize);
			if (lbRc)
				ppReply->dwData[0] = gbGuiClientExternMode;
		}
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
		}
		break;
	case CECMD_STARTSERVER:
		{
			int nErrCode = -1;
			wchar_t szSelf[MAX_PATH+16], *pszNamePtr, szArgs[64];
			PROCESS_INFORMATION pi = {};
			STARTUPINFO si = {sizeof(si)};

			if (GetModuleFileName(ghOurModule, szSelf, MAX_PATH) && ((pszNamePtr = (wchar_t*)PointToName(szSelf)) != NULL))
			{
				// ��������� ������ ��� �� ��������, ��� � ������� �������
				_wcscpy_c(pszNamePtr, 16, WIN3264TEST(L"ConEmuC.exe",L"ConEmuC64.exe"));
				_wsprintf(szArgs, SKIPLEN(countof(szArgs)) L" /GID=%u /ATTACH /PID=%u", pCmd->NewServer.nPID, GetCurrentProcessId());
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
		}
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
