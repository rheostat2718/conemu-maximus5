
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


#define SHOWDEBUGSTR
//#define MCHKHEAP
#define DEBUGSTRMENU(s) //DEBUGSTR(s)
#define DEBUGSTRINPUT(s) //DEBUGSTR(s)
#define DEBUGSTRDLGEVT(s) //OutputDebugStringW(s)
#define DEBUGSTRCMD(s) DEBUGSTR(s)


//#include <stdio.h>
#include <windows.h>
//#include <windowsx.h>
//#include <string.h>
//#include <tchar.h>
#include "../common/common.hpp"
#include "../ConEmuHk/SetHook.h"
#ifdef _DEBUG
#pragma warning( disable : 4995 )
#endif
#include "../common/pluginW1761.hpp"
#ifdef _DEBUG
#pragma warning( default : 4995 )
#endif
#include "../common/ConsoleAnnotation.h"
#include "../common/WinObjects.h"
#include "../common/TerminalMode.h"
#include "..\ConEmu\version.h"
#include "PluginHeader.h"
#include "PluginBackground.h"
#include <Tlhelp32.h>

#ifndef __GNUC__
	#include <Dbghelp.h>
#else
#endif

#include "../common/ConEmuCheck.h"

#define Free free
#define Alloc calloc

//#define MAKEFARVERSION(major,minor,build) ( ((major)<<8) | (minor) | ((build)<<16))

//#define ConEmu_SysID 0x43454D55 // 'CEMU'
#define SETWND_CALLPLUGIN_SENDTABS 100
#define SETWND_CALLPLUGIN_BASE (SETWND_CALLPLUGIN_SENDTABS+1)
#define CHECK_RESOURCES_INTERVAL 5000
#define CHECK_FARINFO_INTERVAL 2000

#define CMD__EXTERNAL_CALLBACK 0x80001
////typedef void (WINAPI* SyncExecuteCallback_t)(LONG_PTR lParam);
//struct SyncExecuteArg
//{
//	DWORD nCmd;
//	HMODULE hModule;
//	SyncExecuteCallback_t CallBack;
//	LONG_PTR lParam;
//};
//
//#ifdef _DEBUG
//wchar_t gszDbgModLabel[6] = {0};
//#endif
//
//#if defined(__GNUC__)
//extern "C" {
//	BOOL WINAPI DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved);
//	HWND WINAPI GetFarHWND();
//	HWND WINAPI GetFarHWND2(int anConEmuOnly);
//	void WINAPI GetFarVersion(FarVersion* pfv);
//	int  WINAPI ProcessEditorInputW(void* Rec);
//	void WINAPI SetStartupInfoW(void *aInfo);
//	//int  WINAPI ProcessSynchroEventW(int Event,void *Param);
//	BOOL WINAPI IsTerminalMode();
//	BOOL WINAPI IsConsoleActive();
//	int  WINAPI RegisterPanelView(PanelViewInit *ppvi);
//	int  WINAPI RegisterBackground(RegisterBackgroundArg *pbk);
//	int  WINAPI ActivateConsole();
//	int  WINAPI SyncExecute(HMODULE ahModule, SyncExecuteCallback_t CallBack, LONG_PTR lParam);
//	void WINAPI GetPluginInfoWcmn(void *piv);
//};
//#endif
//
//
//HMODULE ghPluginModule = NULL; // ConEmu.dll - ��� ������
//HWND ConEmuHwnd = NULL; // �������� ����� ���� ���������. ��� �������� ����.
//DWORD gdwPreDetachGuiPID = 0;
//DWORD gdwServerPID = 0;
//BOOL TerminalMode = FALSE;
//HWND FarHwnd = NULL;
////WARNING("������, �������� ghConIn �� GetStdHandle()"); // ����� � Win7 ����� ����� �����������
////HANDLE ghConIn = NULL;
//DWORD gnMainThreadId = 0, gnMainThreadIdInitial = 0;
////HANDLE hEventCmd[MAXCMDCOUNT], hEventAlive=NULL, hEventReady=NULL;
//HANDLE ghMonitorThread = NULL; DWORD gnMonitorThreadId = 0;
////HANDLE ghInputThread = NULL; DWORD gnInputThreadId = 0;
extern HANDLE ghSetWndSendTabsEvent;
//FarVersion gFarVersion = {};
//WCHAR gszDir1[CONEMUTABMAX], gszDir2[CONEMUTABMAX];
//// gszRootKey ������������ ������ ��� ������ �������� PanelTabs (SeparateTabs/ButtonColors)
//WCHAR gszRootKey[MAX_PATH*2]; // �� ������� "\\Plugins"
//int maxTabCount = 0, lastWindowCount = 0, gnCurTabCount = 0;
//CESERVER_REQ* gpTabs = NULL; //(ConEmuTab*) Alloc(maxTabCount, sizeof(ConEmuTab));
//BOOL gbIgnoreUpdateTabs = FALSE; // ������������ �� ����� CMD_SETWINDOW
//BOOL gbRequestUpdateTabs = FALSE; // ������������ ��� ��������� ������� FOCUS/KILLFOCUS
//BOOL gbClosingModalViewerEditor = FALSE; // ������������ ��� �������� ���������� ���������/�������
//MOUSE_EVENT_RECORD gLastMouseReadEvent = {{0,0}};
//BOOL gbUngetDummyMouseEvent = FALSE;
//LONG gnAllowDummyMouseEvent = 0;
//LONG gnDummyMouseEventFromMacro = 0;
//
//extern HMODULE ghHooksModule;
//extern BOOL gbHooksModuleLoaded; // TRUE, ���� ��� ����� LoadLibrary("ConEmuHk.dll"), ����� ��� ����� FreeLibrary ��� ������
//
//
////CRITICAL_SECTION csData;
//MSection *csData = NULL;
//// ��������� ���������� ������� (������� ��������� OutDataAlloc/OutDataWrite)
//CESERVER_REQ* gpCmdRet = NULL;
//// ���������������� ��� "gpData = gpCmdRet->Data;"
//LPBYTE gpData = NULL, gpCursor = NULL;
//DWORD  gnDataSize=0;
//
//int gnPluginOpenFrom = -1;
//DWORD gnReqCommand = -1;
//LPVOID gpReqCommandData = NULL;
//static HANDLE ghReqCommandEvent = NULL;
//static BOOL   gbReqCommandWaiting = FALSE;
//
//
//UINT gnMsgTabChanged = 0;
extern MSection *csTabs;
////WCHAR gcPlugKey=0;
//BOOL  gbPlugKeyChanged=FALSE;
//HKEY  ghRegMonitorKey=NULL; HANDLE ghRegMonitorEvt=NULL;
////HMODULE ghFarHintsFix = NULL;
WCHAR gszPluginServerPipe[MAX_PATH];
#define MAX_SERVER_THREADS 3
//HANDLE ghServerThreads[MAX_SERVER_THREADS] = {NULL,NULL,NULL};
//HANDLE ghActiveServerThread = NULL;
HANDLE ghPlugServerThread = NULL;
DWORD  gnPlugServerThreadId = 0;
////DWORD  gnServerThreadsId[MAX_SERVER_THREADS] = {0,0,0};
HANDLE ghServerTerminateEvent = NULL;
//HANDLE ghPluginSemaphore = NULL;
//wchar_t gsFarLang[64] = {0};
//BOOL FindServerCmd(DWORD nServerCmd, DWORD &dwServerPID);
//BOOL gbNeedPostTabSend = FALSE;
//BOOL gbNeedPostEditCheck = FALSE; // ���������, ����� � �������� ��������� ��������� ������
////BOOL gbNeedBgUpdate = FALSE; // ��������� ����������� Background
//int lastModifiedStateW = -1;
//BOOL gbNeedPostReloadFarInfo = FALSE;
//DWORD gnNeedPostTabSendTick = 0;
//#define NEEDPOSTTABSENDDELTA 100
////wchar_t gsMonitorEnvVar[0x1000];
////bool gbMonitorEnvVar = false;
//#define MONITORENVVARDELTA 1000
//void UpdateEnvVar(const wchar_t* pszList);
//BOOL StartupHooks();
////BOOL gbFARuseASCIIsort = FALSE; // ���������� ����������� ��������� ���������� � FAR
////HANDLE ghFileMapping = NULL;
////HANDLE ghColorMapping = NULL; // ��������� ��� ������ ������� ����� ����� AllocConsole
////BOOL gbHasColorMapping = FALSE; // ����� �����, ��� ����� True-Colorer ������
////int gnColorMappingMaxCells = 0;
////MFileMapping<AnnotationHeader>* gpColorMapping = NULL;
////#ifdef TRUE_COLORER_OLD_SUPPORT
////HANDLE ghColorMappingOld = NULL;
////BOOL gbHasColorMappingOld = FALSE;
////#endif
//MFileMapping<CESERVER_CONSOLE_MAPPING_HDR> *gpConMap;
//const CESERVER_CONSOLE_MAPPING_HDR *gpConMapInfo = NULL;
////AnnotationInfo *gpColorerInfo = NULL;
//BOOL gbStartedUnderConsole2 = FALSE;
////void CheckColorerHeader();
////int CreateColorerHeader();
////void CloseColorerHeader();
//BOOL ReloadFarInfo(BOOL abForce);
//DWORD gnSelfPID = 0; //GetCurrentProcessId();
////BOOL  gbNeedReloadFarInfo = FALSE;
//HANDLE ghFarInfoMapping = NULL;
//CEFAR_INFO_MAPPING *gpFarInfo = NULL, *gpFarInfoMapping = NULL;
//HANDLE ghFarAliveEvent = NULL;
//PanelViewRegInfo gPanelRegLeft = {NULL};
//PanelViewRegInfo gPanelRegRight = {NULL};
//// ��� �������� PicView & MMView ����� �����, ����� �� CtrlShift ��� F3
//HANDLE ghConEmuCtrlPressed = NULL, ghConEmuShiftPressed = NULL;
//BOOL gbWaitConsoleInputEmpty = FALSE, gbWaitConsoleWrite = FALSE; //, gbWaitConsoleInputPeek = FALSE;
//HANDLE ghConsoleInputEmpty = NULL, ghConsoleWrite = NULL; //, ghConsoleInputWasPeek = NULL;
//// SEE_MASK_NOZONECHECKS
////BOOL gbShellNoZoneCheck = FALSE;
//DWORD GetMainThreadId();
////wchar_t gsLogCreateProcess[MAX_PATH+1] = {0};
//int gnSynchroCount = 0;
//bool gbSynchroProhibited = false;
//bool gbInputSynchroPending = false;
//void SetConsoleFontSizeTo(HWND inConWnd, int inSizeY, int inSizeX, const wchar_t *asFontName);
//
extern struct HookModeFar gFarMode;
//extern SetFarHookMode_t SetFarHookMode;

//std::vector<HANDLE> ghCommandThreads;
class CommandThreads
{
	public:
		HANDLE h[20];
	public:
		CommandThreads()
		{
			memset(h, 0, sizeof(h));
		};
		~CommandThreads()
		{
		};
		void CheckTerminated()
		{
			// ���������, ����� �����-�� ��������� ���� ��� �����������
			for(size_t i = 0; i < sizeof(h)/sizeof(h[0]); i++)
			{
				if (h[i] == NULL) continue;

				if (WaitForSingleObject(h[i], 0) == WAIT_OBJECT_0)
				{
					CloseHandle(h[i]);
					h[i] = NULL;
				}
			}
		};
		void push_back(HANDLE hThread)
		{
			for(size_t i = 0; i < sizeof(h)/sizeof(h[0]); i++)
			{
				if (h[i] == NULL)
				{
					h[i] = hThread;
					return;
				}
			}

			_ASSERTE(FALSE);
		};
		void KillAllThreads()
		{
			// ���������� ���� ���������� �����
			for(size_t i = 0; i < sizeof(h)/sizeof(h[0]); i++)
			{
				if (h[i] == NULL) continue;

				if (WaitForSingleObject(h[i], 100) != WAIT_OBJECT_0)
				{
					TerminateThread(h[i], 100);
				}

				CloseHandle(h[i]);
				h[i] = NULL;
			}
		};
};

CommandThreads *ghCommandThreads = NULL;



DWORD WINAPI PlugServerThread(LPVOID lpvParam);
DWORD WINAPI PlugServerThreadCommand(LPVOID ahPipe);


void PlugServerInit()
{
	ghCommandThreads = new CommandThreads();
}

bool PlugServerStart()
{
	bool lbStarted = false;
	DWORD dwCurProcId = GetCurrentProcessId();
	
	_wsprintf(gszPluginServerPipe, SKIPLEN(countof(gszPluginServerPipe)) CEPLUGINPIPENAME, L".", dwCurProcId);

	ghServerTerminateEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	_ASSERTE(ghServerTerminateEvent!=NULL);

	if (ghServerTerminateEvent) ResetEvent(ghServerTerminateEvent);

	gnPlugServerThreadId = 0;
	ghPlugServerThread = CreateThread(NULL, 0, PlugServerThread, (LPVOID)NULL, 0, &gnPlugServerThreadId);
	_ASSERTE(ghPlugServerThread!=NULL);
	lbStarted = (ghPlugServerThread != NULL);
	
	return lbStarted;
}

void PlugServerStop(bool abDelete)
{
	if (ghPlugServerThread)
	{
		HANDLE hPipe = INVALID_HANDLE_VALUE;
		DWORD dwWait = 0;
		// ����������� ����, ����� ���� ������� �����������
		OutputDebugString(L"Plugin: Touching our server pipe\n");
		hPipe = CreateFile(gszPluginServerPipe,GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);

		if (hPipe == INVALID_HANDLE_VALUE)
		{
			OutputDebugString(L"Plugin: All pipe instances closed?\n");
		}
		else
		{
			OutputDebugString(L"Plugin: Waiting server pipe thread\n");
			dwWait = WaitForSingleObject(ghPlugServerThread, 300); // �������� ���������, ���� ���� ����������
			// ������ ������� ���� - ��� ����� ���� �����������
			CloseHandle(hPipe);
			hPipe = INVALID_HANDLE_VALUE;
		}

		dwWait = WaitForSingleObject(ghPlugServerThread, 0);

		if (dwWait != WAIT_OBJECT_0)
		{
			#if !defined(__GNUC__)
			#pragma warning (disable : 6258)
			#endif
			TerminateThread(ghPlugServerThread, 100);
		}

		SafeCloseHandle(ghPlugServerThread);
	}

	if (abDelete)
	{
		if (ghCommandThreads)
		{
			delete ghCommandThreads;
			ghCommandThreads = NULL;
		}
	}
}

DWORD WINAPI PlugServerThread(LPVOID lpvParam)
{
	BOOL fConnected = FALSE;
	DWORD dwErr = 0;
	HANDLE hPipe = NULL;
	//HANDLE hWait[2] = {NULL,NULL};
	//DWORD dwTID = GetCurrentThreadId();
	//std::vector<HANDLE>::iterator iter;
	_ASSERTE(gszPluginServerPipe[0]!=0);
	//_ASSERTE(ghServerSemaphore!=NULL);

	// The main loop creates an instance of the named pipe and
	// then waits for a client to connect to it. When the client
	// connects, a thread is created to handle communications
	// with that client, and the loop is repeated.

	//hWait[0] = ghServerTerminateEvent;
	//hWait[1] = ghServerSemaphore;

	// ���� �� ����������� ���������� �������
	do
	{
		fConnected = FALSE; // ����� ����

		while(!fConnected)
		{
			_ASSERTE(hPipe == NULL);
			// ���������, ����� �����-�� ��������� ���� ��� �����������
			ghCommandThreads->CheckTerminated();
			//iter = ghCommandThreads.begin();
			//while (iter != ghCommandThreads.end()) {
			//	HANDLE hThread = *iter; dwErr = 0;
			//	if (WaitForSingleObject(hThread, 0) == WAIT_OBJECT_0) {
			//		CloseHandle ( hThread );
			//		iter = ghCommandThreads.erase(iter);
			//	} else {
			//		iter++;
			//	}
			//}
			//// ��������� ���������� ��������, ��� �������� �������
			//dwErr = WaitForMultipleObjects ( 2, hWait, FALSE, INFINITE );
			//if (dwErr == WAIT_OBJECT_0) {
			//    return 0; // ������� �����������
			//}
			//for (int i=0; i<MAX_SERVER_THREADS; i++) {
			//	if (gnServerThreadsId[i] == dwTID) {
			//		ghActiveServerThread = ghServerThreads[i]; break;
			//	}
			//}
			hPipe = CreateNamedPipe(gszPluginServerPipe, PIPE_ACCESS_DUPLEX,
			                        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES,
			                        PIPEBUFSIZE, PIPEBUFSIZE, 0, gpLocalSecurity);
			_ASSERTE(hPipe != INVALID_HANDLE_VALUE);

			if (hPipe == INVALID_HANDLE_VALUE)
			{
				//DisplayLastError(L"CreateNamedPipe failed");
				hPipe = NULL;
				Sleep(50);
				continue;
			}

			// Wait for the client to connect; if it succeeds,
			// the function returns a nonzero value. If the function
			// returns zero, GetLastError returns ERROR_PIPE_CONNECTED.
			fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : ((dwErr = GetLastError()) == ERROR_PIPE_CONNECTED);

			// ������� �����������!
			if (WaitForSingleObject(ghServerTerminateEvent, 0) == WAIT_OBJECT_0)
			{
				//FlushFileBuffers(hPipe); -- ��� �� �����, �� ������ �� ����������
				//DisconnectNamedPipe(hPipe);
				//ReleaseSemaphore(ghServerSemaphore, 1, NULL);
				SafeCloseHandle(hPipe);
				goto wrap;
			}

			if (!fConnected)
				SafeCloseHandle(hPipe);
		}

		if (fConnected)
		{
			// ����� �������, ����� �� ������
			//fConnected = FALSE;
			// ��������� ������ ���� ������� �����
			//ReleaseSemaphore(ghServerSemaphore, 1, NULL);
			//ServerThreadCommand ( hPipe ); // ��� ������������� - ���������� � ���� ��������� ����
			DWORD dwThread = 0;
			HANDLE hThread = CreateThread(NULL, 0, PlugServerThreadCommand, (LPVOID)hPipe, 0, &dwThread);
			_ASSERTE(hThread!=NULL);

			if (hThread==NULL)
			{
				// ��� �� ������� ��������� ���� - ����� ����������� ��� ����������...
				PlugServerThreadCommand((LPVOID)hPipe);
			}
			else
			{
				ghCommandThreads->push_back(hThread);
			}

			hPipe = NULL; // ��������� ServerThreadCommand
		}

		if (hPipe)
		{
			if (hPipe != INVALID_HANDLE_VALUE)
			{
				FlushFileBuffers(hPipe);
				//DisconnectNamedPipe(hPipe);
				CloseHandle(hPipe);
			}

			hPipe = NULL;
		}
	} // ������� � �������� ������ instance �����

	while(WaitForSingleObject(ghServerTerminateEvent, 0) != WAIT_OBJECT_0);

wrap:
	// ���������� ���� ���������� �����
	ghCommandThreads->KillAllThreads();
	//iter = ghCommandThreads.begin();
	//while (iter != ghCommandThreads.end()) {
	//	HANDLE hThread = *iter; dwErr = 0;
	//	if (WaitForSingleObject(hThread, 100) != WAIT_OBJECT_0) {
	//		TerminateThread(hThread, 100);
	//	}
	//	CloseHandle ( hThread );
	//	iter = ghCommandThreads.erase(iter);
	//}
	return 0;
}

DWORD WINAPI PlugServerThreadCommand(LPVOID ahPipe)
{
	HANDLE hPipe = (HANDLE)ahPipe;
	CESERVER_REQ *pIn=NULL;
	BYTE cbBuffer[64]; // ��� ������� ����� ������ ��� ������
	DWORD cbRead = 0, cbWritten = 0, dwErr = 0;
	BOOL fSuccess = FALSE;
	MSectionThread SCT(csTabs);
	// Send a message to the pipe server and read the response.
	fSuccess = ReadFile(hPipe, cbBuffer, sizeof(cbBuffer), &cbRead, NULL);
	dwErr = GetLastError();

	if (!fSuccess && (dwErr != ERROR_MORE_DATA))
	{
		_ASSERTE("ReadFile(pipe) failed"==NULL);
		CloseHandle(hPipe);
		return 0;
	}

	pIn = (CESERVER_REQ*)cbBuffer; // ���� cast, ���� ����� ������ - ������� ������
	_ASSERTE(pIn->hdr.cbSize>=sizeof(CESERVER_REQ_HDR) && cbRead>=sizeof(CESERVER_REQ_HDR));
	_ASSERTE(pIn->hdr.nVersion == CESERVER_REQ_VER);

	if (cbRead < sizeof(CESERVER_REQ_HDR) || /*in.nSize < cbRead ||*/ pIn->hdr.nVersion != CESERVER_REQ_VER)
	{
		CloseHandle(hPipe);
		return 0;
	}

	int nAllSize = pIn->hdr.cbSize;
	pIn = (CESERVER_REQ*)Alloc(nAllSize,1);
	_ASSERTE(pIn!=NULL);

	if (!pIn)
	{
		CloseHandle(hPipe);
		return 0;
	}

	memmove(pIn, cbBuffer, cbRead);
	_ASSERTE(pIn->hdr.nVersion==CESERVER_REQ_VER);
	LPBYTE ptrData = ((LPBYTE)pIn)+cbRead;
	nAllSize -= cbRead;

	while(nAllSize>0)
	{
		//_tprintf(TEXT("%s\n"), chReadBuf);

		// Break if TransactNamedPipe or ReadFile is successful
		if (fSuccess)
			break;

		// Read from the pipe if there is more data in the message.
		fSuccess = ReadFile(
		               hPipe,      // pipe handle
		               ptrData,    // buffer to receive reply
		               nAllSize,   // size of buffer
		               &cbRead,    // number of bytes read
		               NULL);      // not overlapped

		// Exit if an error other than ERROR_MORE_DATA occurs.
		if (!fSuccess && ((dwErr = GetLastError()) != ERROR_MORE_DATA))
			break;

		ptrData += cbRead;
		nAllSize -= cbRead;
	}

	TODO("����� ���������� ASSERT, ���� ������� ���� ������� � �������� ������");
	_ASSERTE(nAllSize==0);

	if (nAllSize>0)
	{
		if (((LPVOID)cbBuffer) != ((LPVOID)pIn))
			Free(pIn);

		CloseHandle(hPipe);
		return 0; // ������� ������� �� ��� ������
	}
	#ifdef _DEBUG
	UINT nDataSize = pIn->hdr.cbSize - sizeof(CESERVER_REQ_HDR);
	#endif
	// ��� ������ �� ����� ��������, ������������ ������� � ���������� (���� �����) ���������
	//fSuccess = WriteFile( hPipe, pOut, pOut->nSize, &cbWritten, NULL);

	if (pIn->hdr.nCmd == CMD_LANGCHANGE)
	{
		_ASSERTE(nDataSize>=4); //-V112
		// LayoutName: "00000409", "00010409", ...
		// � HKL �� ���� ����������, ��� ��� �������� DWORD
		// HKL � x64 �������� ���: "0x0000000000020409", "0xFFFFFFFFF0010409"
		DWORD hkl = pIn->dwData[0];
		DWORD dwLastError = 0;
		HKL hkl1 = NULL, hkl2 = NULL;

		if (hkl)
		{
			WCHAR szLoc[10]; _wsprintf(szLoc, SKIPLEN(countof(szLoc)) L"%08x", hkl);
			hkl1 = LoadKeyboardLayout(szLoc, KLF_ACTIVATE|KLF_REORDER|KLF_SUBSTITUTE_OK|KLF_SETFORPROCESS);
			hkl2 = ActivateKeyboardLayout(hkl1, KLF_SETFORPROCESS|KLF_REORDER);

			if (!hkl2)
				dwLastError = GetLastError();
			else
				fSuccess = TRUE;
		}

	}
	//} else if (pIn->hdr.nCmd == CMD_DEFFONT) {
	//	// ���������� - �����������, ��������� �� ���������
	//	SetConsoleFontSizeTo(FarHwnd, 4, 6);
	//	MoveWindow(FarHwnd, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 1); // ����� ������ ��������� ������ ���������...
	else if (pIn->hdr.nCmd == CMD_REQTABS || pIn->hdr.nCmd == CMD_SETWINDOW)
	{
		MSectionLock SC; SC.Lock(csTabs, FALSE, 1000);

		if (pIn->hdr.nCmd == CMD_SETWINDOW)
		{
			ResetEvent(ghSetWndSendTabsEvent);

			// ��� FAR2 - ����� QSearch ����������� � ��� �� �����, � ������� ������������ ����
			if (gFarVersion.dwVerMajor == 1 && pIn->dwData[1])
			{
				// � ��� ��� FAR1 - ����� ��������
				ProcessCommand(CMD_CLOSEQSEARCH, TRUE/*bReqMainThread*/, pIn->dwData/*���� � �� �����?*/);
			}

			// ������������ 2 DWORD
			ProcessCommand(pIn->hdr.nCmd, TRUE/*bReqMainThread*/, pIn->dwData);

			DEBUGSTRCMD(L"Plugin: PlugServerThreadCommand: CMD_SETWINDOW waiting...\n");

			WARNING("������ ��� FAR1 �� ����? ���� ����������� ��������������� � 1.7 ��� ���?");
			if (gFarVersion.dwVerMajor >= 2)
			{
				DWORD nTimeout = 2000;
				#ifdef _DEBUG
				if (IsDebuggerPresent()) nTimeout = 120000;
				#endif
				
				WaitForSingleObject(ghSetWndSendTabsEvent, nTimeout);
			}

			DEBUGSTRCMD(L"Plugin: PlugServerThreadCommand: CMD_SETWINDOW finished\n");
		}

		if (gpTabs)
		{
			fSuccess = WriteFile(hPipe, gpTabs, gpTabs->hdr.cbSize, &cbWritten, NULL);
		}

		SC.Unlock();
	}
	else if (pIn->hdr.nCmd == CMD_FARSETCHANGED)
	{
		// ���������� ���������� ���������
		// ������ ��� �������� � ����� �� CECMD_RESOURCES, ��������� � GUI ��� �������� �������
		_ASSERTE(nDataSize>=8);
		//wchar_t *pszName  = (wchar_t*)pIn->Data;
		FAR_REQ_FARSETCHANGED *pFarSet = (FAR_REQ_FARSETCHANGED*)pIn->Data;
		//wchar_t *pszName = pSetEnvVar->szEnv;
		gFarMode.bFARuseASCIIsort = pFarSet->bFARuseASCIIsort;
		gFarMode.bShellNoZoneCheck = pFarSet->bShellNoZoneCheck;
		gFarMode.bMonitorConsoleInput = pFarSet->bMonitorConsoleInput;

		if (SetFarHookMode)
		{
			// ��������� �� ���������� ���������� ����� (ConEmuHk.dll)
			SetFarHookMode(&gFarMode);
		}

		//_ASSERTE(nDataSize<sizeof(gsMonitorEnvVar));
		//gbMonitorEnvVar = false;
		//// ������ FarCall "��������" COMSPEC (�������� ���������� ������������ ��������)
		//bool lbOk = false;

		//if (nDataSize<sizeof(gsMonitorEnvVar))
		//{
		//	memcpy(gsMonitorEnvVar, pszName, nDataSize);
		//	lbOk = true;
		//}

		//UpdateEnvVar(pszName);
		////while (*pszName && *pszValue) {
		////	const wchar_t* pszChanged = pszValue;
		////	// ��� ConEmuOutput == AUTO ���������� �� ������ ����
		////	if (!lstrcmpi(pszName, L"ConEmuOutput") && !lstrcmp(pszChanged, L"AUTO")) {
		////		if (gFarVersion.dwVerMajor==1)
		////			pszChanged = L"ANSI";
		////		else
		////			pszChanged = L"UNICODE";
		////	}
		////	// ���� � pszValue ������ ������ - �������� ����������
		////	SetEnvironmentVariableW(pszName, (*pszChanged != 0) ? pszChanged : NULL);
		////	//
		////	pszName = pszValue + lstrlenW(pszValue) + 1;
		////	if (*pszName == 0) break;
		////	pszValue = pszName + lstrlenW(pszName) + 1;
		////}
		//gbMonitorEnvVar = lbOk;
	}
	else if (pIn->hdr.nCmd == CMD_DRAGFROM)
	{
		#ifdef _DEBUG
		BOOL  *pbClickNeed = (BOOL*)pIn->Data;
		COORD *crMouse = (COORD *)(pbClickNeed+1);
		#endif

		ProcessCommand(CMD_LEFTCLKSYNC, TRUE/*bReqMainThread*/, pIn->Data);
		CESERVER_REQ* pCmdRet = NULL;
		ProcessCommand(pIn->hdr.nCmd, TRUE/*bReqMainThread*/, pIn->Data, &pCmdRet);

		if (pCmdRet)
		{
			fSuccess = WriteFile(hPipe, pCmdRet, pCmdRet->hdr.cbSize, &cbWritten, NULL);
			Free(pCmdRet);
		}

		//if (gpCmdRet && gpCmdRet == pCmdRet)
		//{
		//	Free(gpCmdRet);
		//	gpCmdRet = NULL; gpData = NULL; gpCursor = NULL;
		//}
	}
	else if (pIn->hdr.nCmd == CMD_EMENU)
	{
		COORD *crMouse = (COORD *)pIn->Data;
#ifdef _DEBUG
		const wchar_t *pszUserMacro = (wchar_t*)(crMouse+1);
#endif
		DWORD ClickArg[2] = {TRUE, MAKELONG(crMouse->X, crMouse->Y)};
		
		// �������� ���� ��� ��������
		DEBUGSTRMENU(L"\n*** ServerThreadCommand->ProcessCommand(CMD_LEFTCLKSYNC) begin\n");
		BOOL lb1 = ProcessCommand(CMD_LEFTCLKSYNC, TRUE/*bReqMainThread*/, ClickArg/*pIn->Data*/);
		DEBUGSTRMENU(L"\n*** ServerThreadCommand->ProcessCommand(CMD_LEFTCLKSYNC) done\n");

		// � ������, ���������� ������� ����
		DEBUGSTRMENU(L"\n*** ServerThreadCommand->ProcessCommand(CMD_EMENU) begin\n");
		BOOL lb2 = ProcessCommand(pIn->hdr.nCmd, TRUE/*bReqMainThread*/, pIn->Data);
		DEBUGSTRMENU(L"\n*** ServerThreadCommand->ProcessCommand(CMD_EMENU) done\n");
	}
	else if (pIn->hdr.nCmd == CMD_ACTIVEWNDTYPE)
	{
		CESERVER_REQ Out;
		ExecutePrepareCmd(&Out, CMD_ACTIVEWNDTYPE, sizeof(CESERVER_REQ_HDR)+sizeof(DWORD));
		if (gFarVersion.dwVerMajor==1)
			Out.dwData[0] = -1;
		else
			Out.dwData[0] = GetActiveWindowType();
		fSuccess = WriteFile(hPipe, &Out, Out.hdr.cbSize, &cbWritten, NULL);
		
	}
	else
	{
		CESERVER_REQ* pCmdRet = NULL;
		BOOL lbCmd = ProcessCommand(pIn->hdr.nCmd, TRUE/*bReqMainThread*/, pIn->Data, &pCmdRet);

		if (pCmdRet)
		{
			fSuccess = WriteFile(hPipe, pCmdRet, pCmdRet->hdr.cbSize, &cbWritten, NULL);
			Free(pCmdRet);
		}

		//if (gpCmdRet && gpCmdRet == pCmdRet) {
		//	Free(gpCmdRet);
		//	gpCmdRet = NULL; gpData = NULL; gpCursor = NULL;
		//}
	}

	// ���������� ������
	if (((LPVOID)cbBuffer) != ((LPVOID)pIn))
		Free(pIn);

	CloseHandle(hPipe);
	return 0;
}
