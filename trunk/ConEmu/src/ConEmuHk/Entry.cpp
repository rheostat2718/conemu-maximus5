
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

#ifdef _DEBUG
//  �����������������, ����� ����� ����� �������� ������ �������� MessageBox, ����� ����������� ����������
//  #define SHOW_STARTED_MSGBOX
//  #define SHOW_INJECT_MSGBOX
#endif
//#define SHOW_INJECT_MSGBOX
//#define SHOW_STARTED_MSGBOX

#include <windows.h>

#ifndef TESTLINK
#include "../common/common.hpp"
#include "../common/ConEmuCheck.h"
#include "../common/execute.h"
#endif

#include "ConEmuC.h"


#if defined(__GNUC__)
extern "C" {
	BOOL WINAPI DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved);
	LRESULT CALLBACK LLKeybHook(int nCode,WPARAM wParam,LPARAM lParam);
#endif
	__declspec(dllexport) HHOOK ghKeyHook = 0;
	__declspec(dllexport) DWORD gnVkWinFix = 0xF0;
	__declspec(dllexport) HWND  ghKeyHookConEmuRoot = NULL;
#if defined(__GNUC__)
};
#endif

HMODULE ghOurModule = NULL; // ConEmu.dll - ��� ������
UINT gnMsgActivateCon = 0; //RegisterWindowMessage(CONEMUMSG_LLKEYHOOK);
//SECURITY_ATTRIBUTES* gpLocalSecurity = NULL;

#define isPressed(inp) ((GetKeyState(inp) & 0x8000) == 0x8000)

extern HANDLE ghHeap;

BOOL gbSkipInjects = FALSE;
BOOL gbHooksWasSet = FALSE;

UINT_PTR gfnLoadLibrary = NULL;
extern BOOL StartupHooks(HMODULE ahOurDll);
extern void ShutdownHooks();
//HMODULE ghPsApi = NULL;


BOOL WINAPI DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch(ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			ghOurModule = (HMODULE)hModule;
			ghConWnd = GetConsoleWindow();
			gnSelfPID = GetCurrentProcessId();
#ifdef SHOW_STARTED_MSGBOX

			if (!IsDebuggerPresent()) MessageBoxA(NULL, "ConEmuHk*.dll loaded", "ConEmu hooks", 0);

#endif
			//_ASSERTE(ghHeap == NULL);
			//ghHeap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 200000, 0);
			HeapInitialize();
			
			// ��������� ��������� � �������� ����� ���� ���-�� �����������, ����� ������ �����
			// iFindAddress = FindKernelAddress(pi.hProcess, pi.dwProcessId, &fLoadLibrary);
			gfnLoadLibrary = (UINT_PTR)::GetProcAddress(::GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
			
			//#ifndef TESTLINK
			gpLocalSecurity = LocalSecurity();
			gnMsgActivateCon = RegisterWindowMessage(CONEMUMSG_ACTIVATECON);
			//#endif
			wchar_t szSkipEventName[128];
			_wsprintf(szSkipEventName, SKIPLEN(countof(szSkipEventName)) CEHOOKDISABLEEVENT, GetCurrentProcessId());
			HANDLE hSkipEvent = OpenEvent(EVENT_ALL_ACCESS , FALSE, szSkipEventName);
			BOOL lbSkipInjects = FALSE;

			if (hSkipEvent)
			{
				gbSkipInjects = (WaitForSingleObject(hSkipEvent, 0) == WAIT_OBJECT_0);
				CloseHandle(hSkipEvent);
			}
			else
			{
				gbSkipInjects = FALSE;
			}

			// ������� ������� ������� � ���������� �������� HWND GUI, PID �������, � ��...
			if (!gbSkipInjects && ghConWnd)
			{
				MFileMapping<CESERVER_CONSOLE_MAPPING_HDR> ConInfo;
				ConInfo.InitName(CECONMAPNAME, (DWORD)ghConWnd);
				CESERVER_CONSOLE_MAPPING_HDR *pInfo = ConInfo.Open();
				if (pInfo 
					&& (pInfo->cbSize >= sizeof(CESERVER_CONSOLE_MAPPING_HDR))
					//&& (pInfo->nProtocolVersion == CESERVER_REQ_VER)
					)
				{
					if (pInfo->hConEmuRoot && IsWindow(pInfo->hConEmuRoot))
					{
						ghConEmuWnd = pInfo->hConEmuRoot;
						if (pInfo->hConEmuWnd && IsWindow(pInfo->hConEmuWnd))
							ghConEmuWndDC = pInfo->hConEmuWnd;
					}
					if (pInfo->nServerPID && pInfo->nServerPID != gnSelfPID)
						gnServerPID = pInfo->nServerPID;
					ConInfo.CloseMap();
				}

				InitializeConsoleInputSemaphore();
			}
			
			
			#ifdef _WIN64
			DWORD nImageBits = 64, nImageSubsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
			#else
			DWORD nImageBits = 32, nImageSubsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
			#endif
			GetImageSubsystem(nImageSubsystem,nImageBits);

			wchar_t szExeName[MAX_PATH+1];
			if (!GetModuleFileName(NULL, szExeName, countof(szExeName))) szExeName[0] = 0;
			CESERVER_REQ* pIn = NewCmdOnCreateW(
				gbSkipInjects ? eHooksLoaded : eInjectingHooks, L"", 0, 
				szExeName, GetCommandLineW(), nImageBits, nImageSubsystem,
				GetStdHandle(STD_INPUT_HANDLE), GetStdHandle(STD_OUTPUT_HANDLE), GetStdHandle(STD_ERROR_HANDLE));
			if (pIn)
			{
				//HWND hConWnd = GetConsoleWindow();
				CESERVER_REQ* pOut = ExecuteGuiCmd(ghConWnd, pIn, ghConWnd);
				ExecuteFreeResult(pIn);
				if (pOut) ExecuteFreeResult(pOut);
			}
			

			if (!gbSkipInjects)
			{
#ifdef SHOW_INJECT_MSGBOX
				wchar_t szDbgMsg[128], szTitle[128];
				_wsprintf(szTitle, SKIPLEN(countof(szTitle)) L"ConEmuHk, PID=%u", GetCurrentProcessId());
				_wsprintf(szDbgMsg, SKIPLEN(countof(szDbgMsg)) L"SetAllHooks, ConEmuHk, PID=%u", GetCurrentProcessId());
				MessageBoxW(NULL, szDbgMsg, szTitle, MB_SYSTEMMODAL);
#endif
				gnRunMode = RM_APPLICATION;
				_ASSERTE(nImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI);
				gbHooksWasSet = StartupHooks(ghOurModule);
				// ���� NULL - ������ ��� "Detached" ���������� �������, �������� "Started" � ������ ������ ���
				if (ghConWnd != NULL)
				{
					SendStarted();
				}
			}
			else
			{
				gbHooksWasSet = FALSE;
			}
		}
		break;
		case DLL_PROCESS_DETACH:
		{
			if (!gbSkipInjects && gbHooksWasSet)
			{
				gbHooksWasSet = FALSE;
				ShutdownHooks();
			}

			if (gnRunMode == RM_APPLICATION)
			{
				SendStopped();
			}

			//#ifndef TESTLINK
			CommonShutdown();

			ReleaseConsoleInputSemaphore();

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


BYTE gnOtherWin = 0;
DWORD gnSkipVkModCode = 0;
//DWORD gnSkipScanModCode = 0;
DWORD gnSkipVkKeyCode = 0;
//DWORD gnWinPressTick = 0;
//int gnMouseTouch = 0;

LRESULT CALLBACK LLKeybHook(int nCode,WPARAM wParam,LPARAM lParam)
{
	if (nCode >= 0)
	{
		KBDLLHOOKSTRUCT *pKB = (KBDLLHOOKSTRUCT*)lParam;
#ifdef _DEBUG
		wchar_t szKH[128];
		DWORD dwTick = GetTickCount();
		_wsprintf(szKH, SKIPLEN(countof(szKH)) L"[hook] %s(vk=%i, flags=0x%08X, time=%i, tick=%i, delta=%i)\n",
		          (wParam==WM_KEYDOWN) ? L"WM_KEYDOWN" :
		          (wParam==WM_KEYUP) ? L"WM_KEYUP" :
		          (wParam==WM_SYSKEYDOWN) ? L"WM_SYSKEYDOWN" :
		          (wParam==WM_SYSKEYUP) ? L"WM_SYSKEYUP" : L"UnknownMessage",
		          pKB->vkCode, pKB->flags, pKB->time, dwTick, (dwTick-pKB->time));
		//if (wParam == WM_KEYUP && gnSkipVkModCode && pKB->vkCode == gnSkipVkModCode) {
		//	wsprintf(szKH+lstrlen(szKH)-1, L" - WinDelta=%i\n", (pKB->time - gnWinPressTick));
		//}
		OutputDebugString(szKH);
#endif

		if (wParam == WM_KEYDOWN && ghKeyHookConEmuRoot)
		{
			if ((pKB->vkCode >= (UINT)'0' && pKB->vkCode <= (UINT)'9') /*|| pKB->vkCode == (int)' '*/)
			{
				BOOL lbLeftWin = isPressed(VK_LWIN);
				BOOL lbRightWin = isPressed(VK_RWIN);

				if ((lbLeftWin || lbRightWin) && IsWindow(ghKeyHookConEmuRoot))
				{
					DWORD nConNumber = (pKB->vkCode == (UINT)'0') ? 10 : (pKB->vkCode - (UINT)'0');
					PostMessage(ghKeyHookConEmuRoot, gnMsgActivateCon, nConNumber, 0);
					gnSkipVkModCode = lbLeftWin ? VK_LWIN : VK_RWIN;
					gnSkipVkKeyCode = pKB->vkCode;
					// ������ ��������� ��������
					return 1; // ����� ���������� 1, ����� ������� �� ���� � Win7 Taskbar
					////gnWinPressTick = pKB->time;
					//HWND hConEmu = GetForegroundWindow();
					//// �� ����, ������ ���� ConEmu, �� ���������� ��������� (����� ��� �� ������?)
					//if (hConEmu)
					//{
					//	wchar_t szClass[64];
					//	if (GetClassNameW(hConEmu, szClass, 63) && lstrcmpW(szClass, VirtualConsoleClass)==0)
					//	{
					//		//if (!gnMsgActivateCon) --> DllMain
					//		//	gnMsgActivateCon = RegisterWindowMessage(CONEMUMSG_LLKEYHOOK);
					//		WORD nConNumber = (pKB->vkCode == (UINT)'0') ? 10 : (pKB->vkCode - (UINT)'0');
					//		if (SendMessage(hConEmu, gnMsgActivateCon, wParam, pKB->vkCode) == 1)
					//		{
					//			gnSkipVkModCode = lbLeftWin ? VK_LWIN : VK_RWIN;
					//			gnSkipVkKeyCode = pKB->vkCode;
					//			// ������ ��������� ��������
					//			return 1; // ����� ���������� 1, ����� ������� �� ���� � Win7 Taskbar
					//		}
					//	}
					//}
				}
			}

			// �� ������ ������� �� �������� - ������ ��� ���������
			//if (pKB->vkCode == VK_LWIN || pKB->vkCode == VK_RWIN) {
			//	gnWinPressTick = pKB->time;
			//}

			if (gnSkipVkKeyCode && !gnOtherWin)
			{
				// ��������� �� ���������
				gnSkipVkModCode = 0;
				gnSkipVkKeyCode = 0;
			}
		}
		else if (wParam == WM_KEYUP)
		{
			if (gnSkipVkModCode && pKB->vkCode == gnSkipVkModCode)
			{
				if (gnSkipVkKeyCode)
				{
#ifdef _DEBUG
					OutputDebugString(L"*** Win released before key ***\n");
#endif
					// ��� ������� ������� Win+<������> ����� ���������� ��� ��� Win ����������� ������ <������>.
					gnOtherWin = (BYTE)gnVkWinFix;
					keybd_event(gnOtherWin, gnOtherWin, 0, 0);
				}
				else
				{
					gnOtherWin = 0;
				}

				gnSkipVkModCode = 0;
				return 0; // ��������� ��������� ��������, �� �� ���������� � ������ ����
			}

			if (gnSkipVkKeyCode && pKB->vkCode == gnSkipVkKeyCode)
			{
				gnSkipVkKeyCode = 0;

				if (gnOtherWin)
				{
					keybd_event(gnOtherWin, gnOtherWin, KEYEVENTF_KEYUP, 0);
					gnOtherWin = 0;
				}

				return 0; // ��������� ��������� ��������, �� �� ���������� � ������ ����
			}
		}
	}

	return CallNextHookEx(ghKeyHook, nCode, wParam, lParam);
}
