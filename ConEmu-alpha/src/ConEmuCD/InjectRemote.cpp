﻿
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

#include "ConEmuC.h"
#include "../common/MAssert.h"
#include "../common/WinFiles.h"
#include "../common/WinObjects.h"
#include "Infiltrate.h"
#include "../ConEmuHk/Injects.h"
#include "../ConEmu/version.h"
#pragma warning(disable: 4091)
#include <shlobj.h>
#pragma warning(default: 4091)

// 0 - OK, иначе - ошибка
// Здесь вызывается CreateRemoteThread
CINFILTRATE_EXIT_CODES InfiltrateDll(HANDLE hProcess, LPCWSTR asConEmuHk)
{
	CINFILTRATE_EXIT_CODES iRc = CIR_InfiltrateGeneral/*-150*/;

	//if (iRc != -150)
	//{
	//	InfiltrateProc(NULL); InfiltrateEnd();
	//}
	//const size_t cb = ((size_t)InfiltrateEnd) - ((size_t)InfiltrateProc);

	InfiltrateArg dat = {};
	HMODULE hKernel = NULL;
	HANDLE hThread = NULL;
	DWORD id = 0;
	LPTHREAD_START_ROUTINE pRemoteProc = NULL;
	PVOID pRemoteDat = NULL;
	CreateRemoteThread_t _CreateRemoteThread = NULL;
	char FuncName[20];
	void* ptrCode;
	size_t cbCode;

	//_ASSERTE("InfiltrateDll"==(void*)TRUE);

	cbCode = GetInfiltrateProc(&ptrCode);
	// Примерно, проверка размера кода созданного компилятором
	if (cbCode != WIN3264TEST(68,79))
	{
		_ASSERTE(cbCode == WIN3264TEST(68,79));
		iRc = CIR_WrongBitness/*-100*/;
		goto wrap;
	}

	if (lstrlen(asConEmuHk) >= (int)countof(dat.szConEmuHk))
	{
		iRc = CIR_TooLongHookPath/*-101*/;
		goto wrap;
	}

	// Исполняемый код загрузки библиотеки
	pRemoteProc = (LPTHREAD_START_ROUTINE) VirtualAllocEx(
		hProcess,	// Target process
		NULL,		// Let the VMM decide where
		cbCode,		// Size
		MEM_COMMIT,	// Commit the memory
		PAGE_EXECUTE_READWRITE); // Protections
	if (!pRemoteProc)
	{
		iRc = CIR_VirtualAllocEx/*-102*/;
		goto wrap;
	}
	if (!WriteProcessMemory(
		hProcess,		// Target process
		(void*)pRemoteProc,	// Source for code
		ptrCode,		// The code
		cbCode,			// Code length
		NULL))			// We don't care
	{
		iRc = CIR_WriteProcessMemory/*-103*/;
		goto wrap;
	}

	// Путь к нашей библиотеке
	lstrcpyn(dat.szConEmuHk, asConEmuHk, countof(dat.szConEmuHk));

	// Kernel-процедуры
	hKernel = LoadLibrary(L"Kernel32.dll");
	if (!hKernel)
	{
		iRc = CIR_LoadKernel/*-104*/;
		goto wrap;
	}

	// Избежать статической линковки и строки "CreateRemoteThread" в бинарнике
	FuncName[ 0] = 'C'; FuncName[ 2] = 'e'; FuncName[ 4] = 't'; FuncName[ 6] = 'R'; FuncName[ 8] = 'm';
	FuncName[ 1] = 'r'; FuncName[ 3] = 'a'; FuncName[ 5] = 'e'; FuncName[ 7] = 'e'; FuncName[ 9] = 'o';
	FuncName[10] = 't'; FuncName[12] = 'T'; FuncName[14] = 'r'; FuncName[16] = 'a';
	FuncName[11] = 'e'; FuncName[13] = 'h'; FuncName[15] = 'e'; FuncName[17] = 'd'; FuncName[18] = 0;
	_CreateRemoteThread = (CreateRemoteThread_t)GetProcAddress(hKernel, FuncName);

	// Functions for external process. MUST BE SAME ADDRESSES AS CURRENT PROCESS.
	// kernel32.dll компонуется таким образом, что всегда загружается по одному определенному адресу в памяти
	// Поэтому адреса процедур для приложений одинаковой битности совпадают (в разных процессах)
	dat._GetLastError = (GetLastError_t)GetProcAddress(hKernel, "GetLastError");
	dat._SetLastError = (SetLastError_t)GetProcAddress(hKernel, "SetLastError");
	dat._LoadLibraryW = (LoadLibraryW_t)GetLoadLibraryAddress(); // GetProcAddress(hKernel, "LoadLibraryW");
	if (!_CreateRemoteThread || !dat._LoadLibraryW || !dat._SetLastError || !dat._GetLastError)
	{
		iRc = CIR_NoKernelExport/*-105*/;
		goto wrap;
	}
	else
	{
		// Проверим, что адреса этих функций действительно лежат в модуле Kernel32.dll
		// и не были кем-то перехвачены до нас.
		FARPROC proc[] = {(FARPROC)dat._GetLastError, (FARPROC)dat._SetLastError, (FARPROC)dat._LoadLibraryW};
		if (!CheckCallbackPtr(hKernel, countof(proc), proc, TRUE, TRUE))
		{
			// Если функции перехвачены - попытка выполнить код по этим адресам
			// скорее всего приведет к ошибке доступа, что не есть гут.
			iRc = CIR_CheckKernelExportAddr/*-111*/;
			goto wrap;
		}
	}

	// Копируем параметры в процесс
	pRemoteDat = VirtualAllocEx(hProcess, NULL, sizeof(InfiltrateArg), MEM_COMMIT, PAGE_READWRITE);
	if (!pRemoteDat)
	{
		iRc = CIR_VirtualAllocEx2/*-106*/;
		goto wrap;
	}
	if (!WriteProcessMemory(hProcess, pRemoteDat, &dat, sizeof(InfiltrateArg), NULL))
	{
		iRc = CIR_WriteProcessMemory2/*-107*/;
		goto wrap;
	}

	// Запускаем поток в процессе hProcess
	// В принципе, на эту функцию могут ругаться антивирусы
	hThread = _CreateRemoteThread(
		hProcess,		// Target process
		NULL,			// No security
		4096 * 16,		// 16 pages of stack
		pRemoteProc,	// Thread routine address
		pRemoteDat,		// Data
		0,				// Run NOW
		&id);
	if (!hThread)
	{
		iRc = CIR_CreateRemoteThread/*-108*/;
		goto wrap;
	}

	// Дождаться пока поток завершится
	WaitForSingleObject(hThread, INFINITE);

	// И считать результат
	if (!ReadProcessMemory(
		hProcess,		// Target process
		pRemoteDat,		// Their data
		&dat,			// Our data
		sizeof(InfiltrateArg),	// Size
		NULL))			// We don't care
	{
		iRc = CIR_ReadProcessMemory/*-109*/;
		goto wrap;
	}

	// Вернуть результат загрузки
	SetLastError((dat.hInst != NULL) ? 0 : (DWORD)dat.ErrCode);
	iRc = (dat.hInst != NULL) ? CIR_OK/*0*/ : CIR_InInjectedCodeError/*-110*/;
wrap:
	if (hKernel)
		FreeLibrary(hKernel);
	if (hThread)
		CloseHandle(hThread);
	if (pRemoteProc)
		VirtualFreeEx(hProcess, (void*)pRemoteProc, cbCode, MEM_RELEASE);
	if (pRemoteDat)
		VirtualFreeEx(hProcess, pRemoteDat, sizeof(InfiltrateArg), MEM_RELEASE);
	return iRc;
}

CINFILTRATE_EXIT_CODES PrepareHookModule(wchar_t (&szModule)[MAX_PATH+16])
{
	CINFILTRATE_EXIT_CODES iRc = CIR_GeneralError/*-1*/;
	wchar_t szNewPath[MAX_PATH+16] = {}, szAddName[40] = {};
	wchar_t szMinor[8] = L""; lstrcpyn(szMinor, WSTRING(MVV_4a), countof(szMinor));
	INT_PTR nLen = 0;
	bool bAlreadyExists = false;

	// Copy szModule to CSIDL_LOCAL_APPDATA and return new path
	HRESULT hr = SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0/*SHGFP_TYPE_CURRENT*/, szNewPath);
	if ((hr != S_OK) || !*szNewPath)
	{
		iRc = CIR_SHGetFolderPath/*-251*/;
		goto wrap;
	}

	_wsprintf(szAddName, SKIPLEN(countof(szAddName))
		L"\\" CEDEFTERMDLLFORMAT /*L"ConEmuHk%s.%02u%02u%02u%s.dll"*/,
		WIN3264TEST(L"",L"64"), MVV_1, MVV_2, MVV_3, szMinor);

	nLen = lstrlen(szNewPath);
	if (szNewPath[nLen-1] != L'\\')
	{
		szNewPath[nLen++] = L'\\'; szNewPath[nLen] = 0;
	}

	if ((nLen + lstrlen(szAddName) + 8) >= countof(szNewPath))
	{
		iRc = CIR_TooLongTempPath/*-252*/;
		goto wrap;
	}

	wcscat_c(szNewPath, L"ConEmu");
	if (!DirectoryExists(szNewPath))
	{
		if (!CreateDirectory(szNewPath, NULL))
		{
			iRc = CIR_CreateTempDirectory/*-253*/;
			goto wrap;
		}
	}

	wcscat_c(szNewPath, szAddName);

	if ((bAlreadyExists = FileExists(szNewPath)) && FileCompare(szNewPath, szModule))
	{
		// OK, file exists and match the required
	}
	else
	{
		if (bAlreadyExists)
		{
			_ASSERTE(FALSE && "Continue to overwrite existing ConEmuHk in AppLocal");

			// Try to delete or rename old version
			if (!DeleteFile(szNewPath))
			{
				//SYSTEMTIME st; GetLocalTime(&st);
				wchar_t szBakPath[MAX_PATH+32]; wcscpy_c(szBakPath, szNewPath);
				wchar_t* pszExt = (wchar_t*)PointToExt(szBakPath);
				msprintf(pszExt, 16, L".%u.dll", GetTickCount());
				DeleteFile(szBakPath);
				MoveFile(szNewPath, szBakPath);
			}
		}

		if (!CopyFile(szModule, szNewPath, FALSE))
		{
			iRc = CIR_CopyHooksFile/*-254*/;
			goto wrap;
		}
	}

	wcscpy_c(szModule, szNewPath);
	iRc = CIR_OK/*0*/;
wrap:
	return iRc;
}

// CIR_OK=0 - OK, CIR_AlreadyInjected=1 - Already injected, иначе - ошибка
// Здесь вызывается CreateRemoteThread
CINFILTRATE_EXIT_CODES InjectRemote(DWORD nRemotePID, bool abDefTermOnly /*= false */)
{
	CINFILTRATE_EXIT_CODES iRc = CIR_GeneralError/*-1*/;
	bool lbWin64 = WIN3264TEST((IsWindows64()!=0),true);
	bool is32bit;
	int  nBits;
	DWORD nWrapperWait = (DWORD)-1, nWrapperResult = (DWORD)-1;
	HANDLE hProc = NULL;
	wchar_t szSelf[MAX_PATH+16], szHooks[MAX_PATH+16];
	wchar_t *pszNamePtr, szArgs[32];
	wchar_t szName[64];
	HANDLE hEvent = NULL;
	HANDLE hDefTermReady = NULL;
	bool bAlreadyHooked = false;
	HANDLE hSnap = NULL;
	MODULEENTRY32 mi = {sizeof(mi)};

	if (!GetModuleFileName(NULL, szSelf, MAX_PATH))
	{
		iRc = CIR_GetModuleFileName/*-200*/;
		goto wrap;
	}
	wcscpy_c(szHooks, szSelf);
	pszNamePtr = (wchar_t*)PointToName(szHooks);
	if (!pszNamePtr)
	{
		iRc = CIR_GetModuleFileName/*-200*/;
		goto wrap;
	}


	// Hey, may be ConEmuHk.dll is already loaded?
	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, nRemotePID);
	if (hSnap && Module32First(hSnap, &mi))
	{
		// 130829 - Let load newer(!) ConEmuHk.dll into target process.

		LPCWSTR pszConEmuHk = WIN3264TEST(L"conemuhk.", L"conemuhk64.");
		size_t nDllNameLen = lstrlen(pszConEmuHk);
		// Out preferred module name
		wchar_t szOurName[40] = {};
		wchar_t szMinor[8] = L""; lstrcpyn(szMinor, _CRT_WIDE(MVV_4a), countof(szMinor));
		_wsprintf(szOurName, SKIPLEN(countof(szOurName))
			CEDEFTERMDLLFORMAT /*L"ConEmuHk%s.%02u%02u%02u%s.dll"*/,
			WIN3264TEST(L"",L"64"), MVV_1, MVV_2, MVV_3, szMinor);
		CharLowerBuff(szOurName, lstrlen(szOurName));

		// Go to enumeration
		wchar_t szName[64];
		do {
			LPCWSTR pszName = PointToName(mi.szModule);
			// Name of hooked module may be changed (copied to %APPDATA%)
			if (pszName && *pszName)
			{
				lstrcpyn(szName, pszName, countof(szName));
				CharLowerBuff(szName, lstrlen(szName));
				// ConEmuHk*.*.dll?
				if (wmemcmp(szName, pszConEmuHk, nDllNameLen) == 0
					&& wmemcmp(szName+lstrlen(szName)-4, L".dll", 4) == 0)
				{
					// Yes! ConEmuHk.dll already loaded into nRemotePID!
					// But what is the version? Let don't downgrade loaded version!
					if (lstrcmp(szName, szOurName) >= 0)
					{
						// OK, szName is newer or equal to our build
						bAlreadyHooked = true;
					}
					// Stop enumeration
					break;
				}
			}
		} while (Module32Next(hSnap, &mi));

		// Check done
	}
	SafeCloseHandle(hSnap);


	// Already hooked?
	if (bAlreadyHooked)
	{
		iRc = CIR_AlreadyInjected/*1*/;
		goto wrap;
	}


	// Check, if we can access that process
	hProc = OpenProcess(PROCESS_CREATE_THREAD|PROCESS_QUERY_INFORMATION|PROCESS_VM_OPERATION|PROCESS_VM_WRITE|PROCESS_VM_READ, FALSE, nRemotePID);
	if (hProc == NULL)
	{
		iRc = CIR_OpenProcess/*-201*/;
		goto wrap;
	}


	// Go to hook

	// Preparing Events
	_wsprintf(szName, SKIPLEN(countof(szName)) CEDEFAULTTERMHOOK, nRemotePID);
	if (!abDefTermOnly)
	{
		// When running in normal mode (NOT set up as default terminal)
		// we need full initialization procedure, not a light one when hooking explorer.exe
		hEvent = OpenEvent(EVENT_MODIFY_STATE|SYNCHRONIZE, FALSE, szName);
		if (hEvent)
		{
			ResetEvent(hEvent);
			CloseHandle(hEvent);
		}
	}
	else
	{
		hEvent = CreateEvent(LocalSecurity(), FALSE, FALSE, szName);
		SetEvent(hEvent);

		_wsprintf(szName, SKIPLEN(countof(szName)) CEDEFAULTTERMHOOKOK, nRemotePID);
		hDefTermReady = CreateEvent(LocalSecurity(), FALSE, FALSE, szName);
		ResetEvent(hDefTermReady);
	}

	// Creating as remote thread.
	// Resetting this event notify ConEmuHk about
	// 1) need to determine MainThreadId
	// 2) need to start pipe server
	_wsprintf(szName, SKIPLEN(countof(szName)) CECONEMUROOTTHREAD, nRemotePID);
	hEvent = OpenEvent(EVENT_MODIFY_STATE|SYNCHRONIZE, FALSE, szName);
	if (hEvent)
	{
		ResetEvent(hEvent);
		CloseHandle(hEvent);
	}


	// Определить битность процесса, Если он 32битный, а текущий - ConEmuC64.exe
	// Перезапустить 32битную версию ConEmuC.exe
	nBits = GetProcessBits(nRemotePID, hProc);
	if (nBits == 0)
	{
		// Do not even expected, ConEmu GUI must run ConEmuC elevated if required.
		iRc = CIR_GetProcessBits/*-204*/;
		goto wrap;
	}

	is32bit = (nBits == 32);

	if (is32bit != WIN3264TEST(true,false))
	{
		// По идее, такого быть не должно. ConEmu должен был запустить соответствующий conemuC*.exe
		_ASSERTE(is32bit == WIN3264TEST(true,false));
		PROCESS_INFORMATION pi = {};
		STARTUPINFO si = {sizeof(si)};

		_wcscpy_c(pszNamePtr, 16, is32bit ? L"ConEmuC.exe" : L"ConEmuC64.exe");
		_wsprintf(szArgs, SKIPLEN(countof(szArgs)) L" /INJECT=%u", nRemotePID);

		if (!CreateProcess(szHooks, szArgs, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi))
		{
			iRc = CIR_CreateProcess/*-202*/;
			goto wrap;
		}
		nWrapperWait = WaitForSingleObject(pi.hProcess, INFINITE);
		GetExitCodeProcess(pi.hProcess, &nWrapperResult);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		if ((nWrapperResult != CERR_HOOKS_WAS_SET) && (nWrapperResult != CERR_HOOKS_WAS_ALREADY_SET))
		{
			iRc = CIR_WrapperResult/*-203*/;
			SetLastError(nWrapperResult);
			goto wrap;
		}
		// Значит всю работу сделал враппер
		iRc = CIR_OK/*0*/;
		goto wrap;
	}

	// Поехали
	_wcscpy_c(pszNamePtr, 16, is32bit ? L"ConEmuHk.dll" : L"ConEmuHk64.dll");
	if (!FileExists(szHooks))
	{
		iRc = CIR_ConEmuHkNotFound/*-250*/;
		goto wrap;
	}

	if (abDefTermOnly)
	{
		CINFILTRATE_EXIT_CODES iFRc = PrepareHookModule(szHooks);
		if (iFRc != 0)
		{
			iRc = iFRc;
			goto wrap;
		}
	}

	iRc = InfiltrateDll(hProc, szHooks);

	// Если создавали временную копию - запланировать ее удаление
	if (abDefTermOnly && (lstrcmpi(szHooks, szSelf) != 0))
	{
		MoveFileEx(szHooks, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
	}
wrap:
	if (hProc != NULL)
		CloseHandle(hProc);
	// But check the result of the operation

	//_ASSERTE(FALSE && "WaitForSingleObject(hDefTermReady)");

	if ((iRc == 0) && hDefTermReady)
	{
		_ASSERTE(abDefTermOnly);
		DWORD nWaitReady = WaitForSingleObject(hDefTermReady, CEDEFAULTTERMHOOKWAIT/*==0*/);
		if (nWaitReady == WAIT_TIMEOUT)
		{
			iRc = CIR_DefTermWaitingFailed/*-300*/; // Failed to start hooking thread in remote process
		}
	}
	return iRc;
}
