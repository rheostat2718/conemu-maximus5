
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

#include <windows.h>
#include "../common/common.hpp"
#include "../common/ConEmuCheck.h"
#include "../common/execute.h"
#include "../ConEmuCD/ExitCodes.h"
#include "ConEmuHooks.h"

#include "Conole2.h"

extern HMODULE ghOurModule;
extern UINT_PTR gfnLoadLibrary;
//extern HMODULE ghPsApi;

// The handle must have the PROCESS_CREATE_THREAD, PROCESS_QUERY_INFORMATION, PROCESS_VM_OPERATION, PROCESS_VM_WRITE, and PROCESS_VM_READ
int InjectHooks(PROCESS_INFORMATION pi, BOOL abForceGui, BOOL abLogProcess)
{
	int iRc = 0;
#ifndef CONEMUHK_EXPORTS
	_ASSERTE(FALSE)
#endif
	DWORD dwErr = 0; //, dwWait = 0;
	wchar_t szPluginPath[MAX_PATH*2], *pszSlash;
	//HANDLE hFile = NULL;
	//wchar_t* pszPathInProcess = NULL;
	//SIZE_T write = 0;
	//HANDLE hThread = NULL; DWORD nThreadID = 0;
	//LPTHREAD_START_ROUTINE ptrLoadLibrary = NULL;
	_ASSERTE(ghOurModule!=NULL);
	BOOL is64bitOs = FALSE, isWow64process = FALSE;
	int  ImageBits = 32; //-V112
	//DWORD ImageSubsystem = 0;
	isWow64process = FALSE;
#ifdef WIN64
	is64bitOs = TRUE;
#endif
	// ��� �������� IsWow64Process
	HMODULE hKernel = GetModuleHandle(L"kernel32.dll");
	int iFindAddress = 0;
	//bool lbInj = false;
	//UINT_PTR fnLoadLibrary = NULL;
	//DWORD fLoadLibrary = 0;
	DWORD nErrCode = 0, nWait = 0;
	int SelfImageBits = WIN3264TEST(32,64);

	// ������� �� ��� ���������, ��� ��� ����������
	nWait = WaitForSingleObject(pi.hProcess, 0);
	if (nWait == WAIT_OBJECT_0)
	{
		iRc = -506;
		goto wrap;
	}

	if (!GetModuleFileName(ghOurModule, szPluginPath, MAX_PATH))
	{
		dwErr = GetLastError();
		//_printf("GetModuleFileName failed! ErrCode=0x%08X\n", dwErr);
		iRc = -501;
		goto wrap;
	}

	pszSlash = wcsrchr(szPluginPath, L'\\');


	//if (pszSlash) pszSlash++; else pszSlash = szPluginPath;
	if (!pszSlash)
		pszSlash = szPluginPath;

	*pszSlash = 0;
	//TODO("x64 injects");
	//lstrcpy(pszSlash, L"ConEmuHk.dll");
	//
	//hFile = CreateFile(szPluginPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	//if (hFile == INVALID_HANDLE_VALUE)
	//{
	//	dwErr = GetLastError();
	//	_printf("\".\\ConEmuHk.dll\" not found! ErrCode=0x%08X\n", dwErr);
	//	goto wrap;
	//}
	//CloseHandle(hFile); hFile = NULL;
	//BOOL is64bitOs = FALSE, isWow64process = FALSE;
	//int  ImageBits = 32;
	//DWORD ImageSubsystem = 0;
	//isWow64process = FALSE;
	//#ifdef WIN64
	//is64bitOs = TRUE;
	//#endif
	//HMODULE hKernel = GetModuleHandle(L"kernel32.dll");

	if (hKernel)
	{
		typedef BOOL (WINAPI* IsWow64Process_t)(HANDLE, PBOOL);
		IsWow64Process_t IsWow64Process_f = (IsWow64Process_t)GetProcAddress(hKernel, "IsWow64Process");

		if (IsWow64Process_f)
		{
			BOOL bWow64 = FALSE;
#ifndef WIN64

			// ������� ������� - 32-������, (bWow64==TRUE) ����� �������� ��� OS - 64������
			if (IsWow64Process_f(GetCurrentProcess(), &bWow64) && bWow64)
				is64bitOs = TRUE;

#else
			_ASSERTE(is64bitOs);
#endif
			// ������ ��������� ���������� �������
			bWow64 = FALSE;

			if (is64bitOs && IsWow64Process_f(pi.hProcess, &bWow64) && !bWow64)
				ImageBits = 64;
		}
	}

	//int iFindAddress = 0;
	//bool lbInj = false;
	////UINT_PTR fnLoadLibrary = NULL;
	////DWORD fLoadLibrary = 0;
	//DWORD nErrCode = 0;
	//int SelfImageBits;
	//#ifdef WIN64
	//SelfImageBits = 64;
	//#else
	//SelfImageBits = 32;
	//#endif

	//#ifdef WIN64
	//	fnLoadLibrary = (UINT_PTR)::GetProcAddress(::GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");

	//	// 64������ conemuc ������ ����� ��������� � � 32������ ��������
	//	iFindAddress = FindKernelAddress(pi.hProcess, pi.dwProcessId, &fLoadLibrary);

	//#else
	// ���� �������� �� ��������� - ����� helper
	if (ImageBits != SelfImageBits)
	{
		DWORD dwPidWait = WaitForSingleObject(pi.hProcess, 0);
		if (dwPidWait == WAIT_OBJECT_0)
		{
			_ASSERTE(dwPidWait != WAIT_OBJECT_0);
		}
		// ��������� 64������(32������?) comspec ��� ��������� ����
		iFindAddress = -1;
		HANDLE hProcess = NULL, hThread = NULL;
		DuplicateHandle(GetCurrentProcess(), pi.hProcess, GetCurrentProcess(), &hProcess, 0, TRUE, DUPLICATE_SAME_ACCESS);
		DuplicateHandle(GetCurrentProcess(), pi.hThread, GetCurrentProcess(), &hThread, 0, TRUE, DUPLICATE_SAME_ACCESS);
		_ASSERTE(hProcess && hThread);
		#ifdef _WIN64
		// ���� ���������� DWORD � Handle - �� ����������� 32������ ���������. �� ���������� �� �� ������?
		if (((DWORD)hProcess != (DWORD_PTR)hProcess) || ((DWORD)hThread != (DWORD_PTR)hThread))
		{
			_ASSERTE(((DWORD)hProcess == (DWORD_PTR)hProcess) && ((DWORD)hThread == (DWORD_PTR)hThread));
			iRc = -509;
			goto wrap;
		}
		#endif
		wchar_t sz64helper[MAX_PATH*2];
		msprintf(sz64helper, countof(sz64helper),
		          L"\"%s\\ConEmuC%s.exe\" /SETHOOKS=%X,%u,%X,%u",
		          szPluginPath, (ImageBits==64) ? L"64" : L"",
		          (DWORD)hProcess, pi.dwProcessId, (DWORD)hThread, pi.dwThreadId);
		STARTUPINFO si = {sizeof(STARTUPINFO)};
		PROCESS_INFORMATION pi64 = {NULL};
		LPSECURITY_ATTRIBUTES lpSec = LocalSecurity();

		BOOL lbHelper = CreateProcessW(NULL, sz64helper, lpSec, lpSec, TRUE, HIGH_PRIORITY_CLASS, NULL, NULL, &si, &pi64);

		if (!lbHelper)
		{
			nErrCode = GetLastError();
			// ������ ���������� ���������� �������/�������
			iRc = -502;
			
			CloseHandle(hProcess); CloseHandle(hThread);
			goto wrap;
		}
		else
		{
			WaitForSingleObject(pi64.hProcess, INFINITE);

			if (!GetExitCodeProcess(pi64.hProcess, &nErrCode))
				nErrCode = -1;

			CloseHandle(pi64.hProcess); CloseHandle(pi64.hThread);
			CloseHandle(hProcess); CloseHandle(hThread);

			if ((int)nErrCode == CERR_HOOKS_WAS_SET)
			{
				iRc = 0;
				goto wrap;
			}
			else if ((int)nErrCode == CERR_HOOKS_FAILED)
			{
				iRc = -505;
				goto wrap;
			}

			// ������ ���������� ���������� �������/�������
		}
		
		// ��� ��� ����� ������ ���� ���� ����������!
		_ASSERTE(FALSE);
		iRc = -504;
		goto wrap;
		
	}
	else
	{
		//iFindAddress = FindKernelAddress(pi.hProcess, pi.dwProcessId, &fLoadLibrary);
		//fnLoadLibrary = (UINT_PTR)::GetProcAddress(::GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
		if (gfnLoadLibrary)
		{
			// -- �� ����� ������. ������� ��� "�� �������", ������� CreateToolhelp32Snapshot(TH32CS_SNAPMODULE) ������������
			//// ���������, � �� ��� �� ���?
			//BOOL lbIsGui = FALSE;
			//if (!abForceGui)
			//{
			//	DWORD nBits = 0;
			//	if (GetImageSubsystem(pi, ImageSubsystem, nBits))
			//		lbIsGui = (ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI);
			//	_ASSERTE(nBits == ImageBits);
			//	if (lbIsGui)
			//	{
			//		iRc = 0;
			//		goto wrap;
			//	}
			//}
		
			DWORD_PTR ptrAllocated = NULL; DWORD nAllocated = 0;
			iRc = InjectHookDLL(pi, gfnLoadLibrary, ImageBits, szPluginPath, &ptrAllocated, &nAllocated);

			if (abLogProcess || (iRc !=0 ))
			{
				int ImageSystem = 0;
				wchar_t szInfo[128];
				if (iRc != 0)
				{
					DWORD nErr = GetLastError();
					msprintf(szInfo, countof(szInfo), L"InjectHookDLL failed, code=%i:0x%08X", iRc, nErr);
				}
				#ifdef _WIN64
				_ASSERTE(SelfImageBits == 64);
				if (iRc == 0)
				{
					if ((DWORD)(ptrAllocated >> 32)) //-V112
					{
						msprintf(szInfo, countof(szInfo), L"Alloc: 0x%08X%08X, Size: %u",
							(DWORD)(ptrAllocated >> 32), (DWORD)(ptrAllocated & 0xFFFFFFFF), nAllocated); //-V112
					}
					else
					{
						msprintf(szInfo, countof(szInfo), L"Alloc: 0x%08X, Size: %u",
							(DWORD)(ptrAllocated & 0xFFFFFFFF), nAllocated); //-V112
					}
				}
				#else
				_ASSERTE(SelfImageBits == 32);
				if (iRc == 0)
				{
					msprintf(szInfo, countof(szInfo), L"Alloc: 0x" WIN3264TEST(L"%08X",L"%X%08X") L", Size: %u", WIN3264WSPRINT(ptrAllocated), nAllocated);
				}
				#endif
				
				CESERVER_REQ* pIn = ExecuteNewCmdOnCreate(eSrvLoaded,
					L"", szInfo, L"", NULL, NULL, NULL, NULL, 
					SelfImageBits, ImageSystem, NULL, NULL, NULL);
				if (pIn)
				{
					CESERVER_REQ* pOut = ExecuteGuiCmd(ghConWnd, pIn, ghConWnd);
					ExecuteFreeResult(pIn);
					if (pOut) ExecuteFreeResult(pOut);
				}
			}
		}
		else
		{
			_ASSERTE(gfnLoadLibrary!=NULL);
			iRc = -503;
			goto wrap;
		}	
	}

//#endif
	//if (iFindAddress != 0)
	//{
	//	iRc = -1;
	//	goto wrap;
	//}
	//fnLoadLibrary = (UINT_PTR)fLoadLibrary;
	//if (!lbInj)
	//{
	//	iRc = -1;
	//	goto wrap;
	//}
	//WARNING("The process handle must have the PROCESS_VM_OPERATION access right!");
	//size = (lstrlen(szPluginPath)+1)*2;
	//TODO("����� ����� �� DOS (16���) �����������");
	//TODO("���������, ������ �� ConEmu.x64 ����������� � 32������ ����������?");
	//pszPathInProcess = (wchar_t*)VirtualAllocEx(hProcess, 0, size, MEM_COMMIT, PAGE_READWRITE);
	//if (!pszPathInProcess)
	//{
	//	dwErr = GetLastError();
	//	_printf("VirtualAllocEx failed! ErrCode=0x%08X\n", dwErr);
	//	goto wrap;
	//}
	//if (!WriteProcessMemory(hProcess, pszPathInProcess, szPluginPath, size, &write ) || size != write)
	//{
	//	dwErr = GetLastError();
	//	_printf("WriteProcessMemory failed! ErrCode=0x%08X\n", dwErr);
	//	goto wrap;
	//}
	//
	//TODO("�������� ����� LoadLibraryW � �������� ������������ ����������� ��������!");
	//ptrLoadLibrary = (LPTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(L"Kernel32.dll"), "LoadLibraryW");
	//if (ptrLoadLibrary == NULL)
	//{
	//	dwErr = GetLastError();
	//	_printf("GetProcAddress(kernel32, LoadLibraryW) failed! ErrCode=0x%08X\n", dwErr);
	//	goto wrap;
	//}
	//
	//if (ptrLoadLibrary)
	//{
	//	// The handle must have the PROCESS_CREATE_THREAD, PROCESS_QUERY_INFORMATION, PROCESS_VM_OPERATION, PROCESS_VM_WRITE, and PROCESS_VM_READ
	//	hThread = CreateRemoteThread(hProcess, NULL, 0, ptrLoadLibrary, pszPathInProcess, 0, &nThreadID);
	//	if (!hThread)
	//	{
	//		dwErr = GetLastError();
	//		_printf("CreateRemoteThread failed! ErrCode=0x%08X\n", dwErr);
	//		goto wrap;
	//	}
	//	// ���������, ���� ����������
	//	dwWait = WaitForSingleObject(hThread,
	//		#ifdef _DEBUG
	//					INFINITE
	//		#else
	//					10000
	//		#endif
	//		);
	//	if (dwWait != WAIT_OBJECT_0) {
	//		dwErr = GetLastError();
	//		_printf("Inject tread timeout!");
	//		goto wrap;
	//	}
	//}
	//
wrap:
//#endif
	return iRc;
}
