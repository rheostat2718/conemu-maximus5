
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


#define DROP_SETCP_ON_WIN2K3R2
//#define EXTERNAL_HOOK_LIBRARY

// ����� �� ����������� GetConsoleAliases (���� �� ������ ���� �������� � Win2k)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <TCHAR.h>
#include <Tlhelp32.h>
#include <shlwapi.h>
#include "..\common\common.hpp"
#include "..\common\ConEmuCheck.h"
#include "..\common\SetHook.h"

//#ifdef _DEBUG
//	#include <crtdbg.h>
//#else
//	#ifndef _ASSERTE
//		#define _ASSERTE(x)
//	#endif
//#endif


extern HMODULE ghPluginModule;


static TCHAR kernel32[] = _T("kernel32.dll");
//static TCHAR user32[]   = _T("user32.dll");
static TCHAR shell32[]  = _T("shell32.dll");
static TCHAR wininet[]  = _T("wininet.dll");

//static BOOL bHooksWin2k3R2Only = FALSE;
//static HookItem HooksWin2k3R2Only[] = {
//	{OnSetConsoleCP, "SetConsoleCP", kernel32, 0},
//	{OnSetConsoleOutputCP, "SetConsoleOutputCP", kernel32, 0},
//	/* ************************ */
//	{0, 0, 0}
//};


#ifndef EXTERNAL_HOOK_LIBRARY
extern int WINAPI OnCompareStringW(LCID Locale, DWORD dwCmpFlags, LPCWSTR lpString1, int cchCount1, LPCWSTR lpString2, int cchCount2);
//
extern BOOL WINAPI OnHttpSendRequestA(LPVOID hRequest, LPCSTR lpszHeaders, DWORD dwHeadersLength, LPVOID lpOptional, DWORD dwOptionalLength);
extern BOOL WINAPI OnHttpSendRequestW(LPVOID hRequest, LPCWSTR lpszHeaders, DWORD dwHeadersLength, LPVOID lpOptional, DWORD dwOptionalLength);

static HookItem HooksFarOnly[] = {
//	{OnlstrcmpiA,      "lstrcmpiA",      kernel32, 0},
	{(void*)OnCompareStringW, "CompareStringW", kernel32, 0},
	/* ************************ */
	{(void*)OnHttpSendRequestA, "HttpSendRequestA", wininet, 0},
	{(void*)OnHttpSendRequestW, "HttpSendRequestW", wininet, 0},
	/* ************************ */
	{0, 0, 0}
};
#endif // EXTERNAL_HOOK_LIBRARY


//void UnsetAllHooks()
//{
//	UnsetHook( HooksFarOnly, NULL, TRUE );
//	
//	/*if (bHooksWin2k3R2Only) {
//		bHooksWin2k3R2Only = FALSE;
//		UnsetHook( HooksWin2k3R2Only, NULL, TRUE );
//	}*/
//	
//    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);
//    if(snapshot != INVALID_HANDLE_VALUE)
//    {
//        MODULEENTRY32 module = {sizeof(module)};
//		BOOL lbExecutable = TRUE;
//
//        for(BOOL res = Module32First(snapshot, &module); res; res = Module32Next(snapshot, &module))
//        {
//            if(module.hModule != ghPluginModule && !IsModuleExcluded(module.hModule))
//            {
//                DebugString( module.szModule );
//                UnsetHook( Hooks, module.hModule, lbExecutable );
//				if (lbExecutable) lbExecutable = FALSE;
//            }
//        }
//
//        CloseHandle(snapshot);
//    }
//}


extern BOOL WINAPI OnConsoleDetaching(HookCallbackArg* pArgs);
extern VOID WINAPI OnConsoleWasAttached(HookCallbackArg* pArgs);
extern BOOL WINAPI OnConsolePeekInput(HookCallbackArg* pArgs);
extern VOID WINAPI OnConsolePeekInputPost(HookCallbackArg* pArgs);
extern BOOL WINAPI OnConsoleReadInput(HookCallbackArg* pArgs);
extern VOID WINAPI OnConsoleReadInputPost(HookCallbackArg* pArgs);
extern BOOL WINAPI OnWriteConsoleOutput(HookCallbackArg* pArgs);
//extern VOID WINAPI OnWasWriteConsoleOutputA(HookCallbackArg* pArgs);
//extern VOID WINAPI OnWasWriteConsoleOutputW(HookCallbackArg* pArgs);
extern VOID WINAPI OnGetNumberOfConsoleInputEventsPost(HookCallbackArg* pArgs);
extern VOID WINAPI OnShellExecuteExW_Except(HookCallbackArg* pArgs);
extern VOID WINAPI OnLibraryLoaded(HMODULE ahModule);

#ifdef EXTERNAL_HOOK_LIBRARY
	HMODULE ghHooksModule = NULL;
	SetHookCallbacks_t SetHookCallbacks = NULL;
	SetLoadLibraryCallback_t SetLoadLibraryCallback = NULL;
#endif

// ��� ������� ����� ������� �� DllMain �������
BOOL StartupHooks(HMODULE ahOurDll)
{
	WARNING("�������� � ��������� ��������� �������� - ���� �� ������");
	
	#ifndef EXTERNAL_HOOK_LIBRARY
		InitHooks( HooksFarOnly );
	#else
		if (ghHooksModule == NULL)
		{
			ghHooksModule = GetModuleHandle(
				#ifdef WIN64
					L"ConEmuHk64.dll"
				#else
					L"ConEmuHk.dll"
				#endif
				);
			
			if (ghHooksModule == NULL)
			{
				TODO("����������� ��������� LoadLibrary, � ��������� ������� GetModuleHandle ����� ������������");
				_ASSERTE(ghHooksModule!=NULL);
				WARNING("�������� ������!");
				MessageBox(NULL, L"ConEmuHk was not loaded!", L"ConEmu plugin", MB_ICONSTOP|MB_SYSTEMMODAL);
				return FALSE;
			}
		}
		if (!SetHookCallbacks || !SetLoadLibraryCallback)
		{
			SetHookCallbacks = (SetHookCallbacks_t)GetProcAddress(ghHooksModule, "SetHookCallbacks");
			SetLoadLibraryCallback = (SetLoadLibraryCallback_t)GetProcAddress(ghHooksModule, "SetLoadLibraryCallback");
			if (!SetHookCallbacks || !SetLoadLibraryCallback)
			{
				WARNING("�������� ������!");
				MessageBox(NULL, L"ConEmuHk is broken, exports not found!", L"ConEmu plugin", MB_ICONSTOP|MB_SYSTEMMODAL);
				return FALSE;
			}
		}
	#endif
	
	
	SetLoadLibraryCallback(ghPluginModule, OnLibraryLoaded, NULL/*OnLibraryUnLoaded*/);
	
	
	SetHookCallbacks( "FreeConsole",  kernel32, ghPluginModule, OnConsoleDetaching, NULL, NULL );
	SetHookCallbacks( "AllocConsole", kernel32, ghPluginModule, NULL, OnConsoleWasAttached, NULL );

	SetHookCallbacks( "PeekConsoleInputA", kernel32, ghPluginModule, OnConsolePeekInput, OnConsolePeekInputPost, NULL );
	SetHookCallbacks( "PeekConsoleInputW", kernel32, ghPluginModule, OnConsolePeekInput, OnConsolePeekInputPost, NULL );
	
	SetHookCallbacks( "ReadConsoleInputA", kernel32, ghPluginModule, OnConsoleReadInput, OnConsoleReadInputPost, NULL );
	SetHookCallbacks( "ReadConsoleInputW", kernel32, ghPluginModule, OnConsoleReadInput, OnConsoleReadInputPost, NULL );

	SetHookCallbacks( "WriteConsoleOutputA", kernel32, ghPluginModule, OnWriteConsoleOutput, NULL, NULL );
	SetHookCallbacks( "WriteConsoleOutputW", kernel32, ghPluginModule, OnWriteConsoleOutput, NULL, NULL );
	
	SetHookCallbacks( "GetNumberOfConsoleInputEvents", kernel32, ghPluginModule, NULL, OnGetNumberOfConsoleInputEventsPost, NULL );

	SetHookCallbacks( "ShellExecuteExW", shell32, ghPluginModule, NULL, NULL, OnShellExecuteExW_Except );
	
	
	bool lbRc = false;
	
	#ifndef EXTERNAL_HOOK_LIBRARY
		lbRc = SetAllHooks(ahOurDll);
	#else
		lbRc = true;
	#endif
	
	return lbRc;

	//HMODULE hKernel = GetModuleHandle( kernel32 );
	//HMODULE hUser   = GetModuleHandle( user32 );
	//HMODULE hShell  = GetModuleHandle( shell32 );
	//if (!hShell) hShell = LoadLibrary ( shell32 );
	//_ASSERTE(hKernel && hUser && hShell);
	//if (!hKernel || !hUser || !hShell)
	//	return FALSE; // ������ ������ ���� ��������� �� conemu.dll
	//
	//OSVERSIONINFOEX osv = {sizeof(OSVERSIONINFOEX)};
	//GetVersionEx((LPOSVERSIONINFO)&osv);
	//
	//// ��������� ���� HookItem.OldAddress (�������� ��������� �� ������� ���������)
	//InitHooks( Hooks );
	//InitHooks( HooksFarOnly );
	////InitHooks( HooksWin2k3R2Only );
	//
	////  ��������� ������������� ������� � Far.exe (���� ��� ������ ����������� �����)
	//SetHook( HooksFarOnly, NULL, TRUE );
	//
	//// Windows Server 2003 R2
	//
	///*if (osv.dwMajorVersion==5 && osv.dwMinorVersion==2 && osv.wServicePackMajor>=2)
	//{
	//	//DWORD dwBuild = GetSystemMetrics(SM_SERVERR2); // ������� ��� �� ����������. 0 ��� :(
	//	bHooksWin2k3R2Only = TRUE;
	//	SetHook( HooksWin2k3R2Only, NULL );
	//}*/
	//
	//// ��������� ������������� ������� �� ���� ������� ��������, ����������� �� conemu.dll
	//SetHookEx( Hooks, ghPluginModule );
	//
	//// �������� � ������ Module �������������� ������� �� ����������� �������� �������
	//// �� ���������, �.�. � Win32 ���������� shell32 ����� ���� ��������� ����� conemu.dll
	////   ��� ������� ������������ �������� �������,
	//// � � Win64 �������� ������ ������ ���� 64�������, � ��������� ������ ������ ������ 32������ ��������
	//
	//return TRUE;
}


void ShutdownHooks()
{
#ifndef EXTERNAL_HOOK_LIBRARY
	UnsetAllHooks();
#else
	if (SetLoadLibraryCallback)
	{
		SetLoadLibraryCallback(ghPluginModule, NULL, NULL);
	}
	
	if (SetHookCallbacks)
	{
		SetHookCallbacks( "FreeConsole",  kernel32, ghPluginModule, NULL, NULL, NULL );
		SetHookCallbacks( "AllocConsole", kernel32, ghPluginModule, NULL, NULL, NULL );

		SetHookCallbacks( "PeekConsoleInputA", kernel32, ghPluginModule, NULL, NULL, NULL );
		SetHookCallbacks( "PeekConsoleInputW", kernel32, ghPluginModule, NULL, NULL, NULL );
		
		SetHookCallbacks( "ReadConsoleInputA", kernel32, ghPluginModule, NULL, NULL, NULL );
		SetHookCallbacks( "ReadConsoleInputW", kernel32, ghPluginModule, NULL, NULL, NULL );

		SetHookCallbacks( "WriteConsoleOutputA", kernel32, ghPluginModule, NULL, NULL, NULL );
		SetHookCallbacks( "WriteConsoleOutputW", kernel32, ghPluginModule, NULL, NULL, NULL );
		
		SetHookCallbacks( "GetNumberOfConsoleInputEvents", kernel32, ghPluginModule, NULL, NULL, NULL );

		SetHookCallbacks( "ShellExecuteExW", shell32, ghPluginModule, NULL, NULL, NULL );
	}
#endif
}
