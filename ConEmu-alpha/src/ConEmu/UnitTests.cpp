﻿
/*
Copyright (c) 2014 Maximus5
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


#define HIDE_USE_EXCEPTION_INFO
#define SHOWDEBUGSTR

#include "Header.h"

#pragma warning(disable: 4091)
#include <ShlObj.h>

#include "ConEmu.h"
#include "Hotkeys.h"
#include "Macro.h"
#include "Match.h"
#include "../common/WFiles.h"
#include "../common/WUser.h"

#include "UnitTests.h"

#ifdef _DEBUG
void UnitMaskTests()
{
	struct {
		LPCWSTR asFileName, asMask;
		bool Result;
	} Tests[] = {
		{L"FileName.txt", L"FileName.txt", true},
		{L"FileName.txt", L"FileName", false},
		{L"FileName.txt", L"FileName.doc", false},
		{L"FileName.txt", L"*", true},
		{L"FileName.txt", L"FileName*", true},
		{L"FileName.txt", L"FileName*txt", true},
		{L"FileName.txt", L"FileName*.txt", true},
		{L"FileName.txt", L"FileName*e.txt", false},
		{L"FileName.txt", L"File*qqq", false},
		{L"FileName.txt", L"File*txt", true},
		{L"FileName.txt", L"Name*txt", false},
		{NULL}
	};
	bool bCheck;
	for (size_t i = 0; Tests[i].asFileName; i++)
	{
		bCheck = CompareFileMask(Tests[i].asFileName, Tests[i].asMask);
		_ASSERTE(bCheck == Tests[i].Result);
	}
	bCheck = true;
}

void UnitDriveTests()
{
	struct {
		LPCWSTR asPath, asResult;
	} Tests[] = {
		{L"C:", L"C:"},
		{L"C:\\Dir1\\Dir2\\File.txt", L"C:"},
		{L"\\\\Server\\Share", L"\\\\Server\\Share"},
		{L"\\\\Server\\Share\\Dir1\\Dir2\\File.txt", L"\\\\Server\\Share"},
		{L"\\\\?\\UNC\\Server\\Share", L"\\\\?\\UNC\\Server\\Share"},
		{L"\\\\?\\UNC\\Server\\Share\\Dir1\\Dir2\\File.txt", L"\\\\?\\UNC\\Server\\Share"},
		{L"\\\\?\\C:", L"\\\\?\\C:"},
		{L"\\\\?\\C:\\Dir1\\Dir2\\File.txt", L"\\\\?\\C:"},
		{NULL}
	};
	bool bCheck;
	wchar_t szDrive[MAX_PATH];
	for (size_t i = 0; Tests[i].asPath; i++)
	{
		wmemset(szDrive, L'z', countof(szDrive));
		bCheck = (wcscmp(GetDrive(Tests[i].asPath, szDrive, countof(szDrive)), Tests[i].asResult) == 0);
		_ASSERTE(bCheck);
	}
	bCheck = true;
}

void UnitFileNamesTest()
{
	_ASSERTE(IsDotsName(L"."));
	_ASSERTE(IsDotsName(L".."));
	_ASSERTE(!IsDotsName(L"..."));
	_ASSERTE(!IsDotsName(L""));

	struct {
		LPCWSTR asPath, asPart1, asPart2, asResult;
	} Tests[] = {
		{L"C:", L"Dir", L"File.txt", L"C:\\Dir\\File.txt"},
		{L"C:\\", L"\\Dir\\", L"\\File.txt", L"C:\\Dir\\File.txt"},
		{L"C:\\", L"\\File.txt", NULL, L"C:\\File.txt"},
		{L"C:", L"\\File.txt", NULL, L"C:\\File.txt"},
		{L"C:\\", L"File.txt", NULL, L"C:\\File.txt"},
		{NULL}
	};
	bool bCheck;
	wchar_t* pszJoin;
	for (size_t i = 0; Tests[i].asPath; i++)
	{
		pszJoin = JoinPath(Tests[i].asPath, Tests[i].asPart1, Tests[i].asPart2);
		bCheck = (pszJoin && (lstrcmp(pszJoin, Tests[i].asResult) == 0));
		_ASSERTE(bCheck);
		SafeFree(pszJoin);
	}
	bCheck = true;
}

void UnitExpandTest()
{
	CmdArg szExe;
	wchar_t szChoc[MAX_PATH] = L"powershell -NoProfile -ExecutionPolicy unrestricted -Command \"iex ((new-object net.webclient).DownloadString('https://chocolatey.org/install.ps1'))\" && SET PATH=%PATH%;%systemdrive%\\chocolatey\\bin";
	wchar_t* pszExpanded = ExpandEnvStr(szChoc);
	int nLen = pszExpanded ? lstrlen(pszExpanded) : 0;
	BOOL bFound = FileExistsSearch(szChoc, szExe, false);
	wcscpy_c(szChoc, gpConEmu->ms_ConEmuExeDir);
	wcscat_c(szChoc, L"\\Tests\\Executables\\abcd");
	bFound = FileExistsSearch(szChoc, szExe, false);
	// TakeCommand
	ConEmuComspec tcc = {cst_AutoTccCmd};
	FindComspec(&tcc, false);
}

void UnitModuleTest()
{
	wchar_t* pszConEmuCD = lstrmerge(gpConEmu->ms_ConEmuBaseDir, WIN3264TEST(L"\\ConEmuCD.dll",L"\\ConEmuCD64.dll"));
	HMODULE hMod, hGetMod;
	bool bTest;

	_ASSERTE(!IsModuleValid((HMODULE)NULL));
	_ASSERTE(!IsModuleValid((HMODULE)INVALID_HANDLE_VALUE));

	hMod = GetModuleHandle(L"kernel32.dll");
	if (hMod)
	{
		bTest = IsModuleValid(hMod);
		_ASSERTE(bTest);
	}
	else
	{
		_ASSERTE(FALSE && "GetModuleHandle(kernel32) failed");
	}

	hMod = LoadLibrary(pszConEmuCD);
	if (hMod)
	{
		bTest = IsModuleValid(hMod);
		_ASSERTE(bTest);

		FreeLibrary(hMod);
		bTest = IsModuleValid(hMod);
		// Due to unknown reason (KIS?) FreeLibrary was not able to release hMod sometimes
		hGetMod = GetModuleHandle(pszConEmuCD);
		if (!hGetMod)
			bTest = IsModuleValid(hMod);
		_ASSERTE(!bTest || (hGetMod!=NULL));
	}
	else
	{
		_ASSERTE(FALSE && "LoadLibrary(pszConEmuCD) failed");
	}
}

void DebugUnitMprintfTest()
{
	wchar_t szTest[80];
	msprintf(szTest, countof(szTest), L"%u,%02u,%02u,%02u,%02x,%08x", 12345, 1, 23, 4567, 0xFE, 0xABCD1234);
	int nDbg = lstrcmp(szTest, L"12345,01,23,4567,fe,abcd1234");
	_ASSERTE(nDbg==0);
}

void DebugVersionTest()
{
	DWORDLONG const dwlConditionMask = VerSetConditionMask(VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL), VER_MINORVERSION, VER_GREATER_EQUAL);

	_ASSERTE(_WIN32_WINNT_WIN7==0x601);
	OSVERSIONINFOEXW osvi7 = {sizeof(osvi7), HIBYTE(_WIN32_WINNT_WIN7), LOBYTE(_WIN32_WINNT_WIN7)};
	bool bWin7 = VerifyVersionInfoW(&osvi7, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask) != 0;

	_ASSERTE(_WIN32_WINNT_VISTA==0x600);
	OSVERSIONINFOEXW osvi6 = {sizeof(osvi6), HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA)};
	bool bWin6 = VerifyVersionInfoW(&osvi6, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask) != 0;

	OSVERSIONINFOW osv = {sizeof(OSVERSIONINFOW)};
	GetVersionExW(&osv);
	bool bVerWin7 = ((osv.dwMajorVersion > 6) || ((osv.dwMajorVersion == 6) && (osv.dwMinorVersion >= 1)));
	bool bVerWin6 = (osv.dwMajorVersion >= 6);

	_ASSERTE(bWin7 == bVerWin7);
	_ASSERTE(bWin6 == bVerWin6);
}

void DebugFileExistTests()
{
	bool b;
	b = DirectoryExists(L"C:\\Documents and Settings\\Maks\\.ipython\\");
	b = DirectoryExists(L"C:\\Documents and Settings\\Maks\\.ipython\\.");
	b = DirectoryExists(L"C:\\Documents and Settings\\Maks\\.ipython");
	b = DirectoryExists(L"C:\\Documents and Settings\\Maks\\.ipython-qq");
	b = FileExists(L"C:\\Documents and Settings\\Maks\\.ipython");
	b = FileExists(L"C:\\Documents and Settings\\Maks\\.ipython\\README");
	b = FileExists(L"C:\\Documents and Settings\\Maks\\.ipython\\.");
}

void DebugNeedCmdUnitTests()
{
	BOOL b;
	struct strTests { LPCWSTR pszCmd; BOOL bNeed; }
	Tests[] = {
		{L"\"C:\\windows\\notepad.exe -f \"makefile\" COMMON=\"../../../plugins/common\"\"", FALSE},
		{L"\"\"C:\\windows\\notepad.exe  -new_console\"\"", FALSE},
		{L"\"\"cmd\"\"", FALSE},
		{L"cmd /c \"\"C:\\Program Files\\Windows NT\\Accessories\\wordpad.exe\" -?\"", FALSE},
		{L"cmd /c \"dir c:\\\"", FALSE},
		{L"abc.cmd", TRUE},
		// Do not do too many heuristic. If user really needs redirection (for 'root'!)
		// he must explicitly call "cmd /c ...". With only exception if first exe not found.
		{L"notepad text & start explorer", FALSE},
	};
	LPCWSTR psArgs;
	BOOL bNeedCut, bRootIsCmd, bAlwaysConfirm, bAutoDisable;
	CmdArg szExe;
	for (INT_PTR i = 0; i < countof(Tests); i++)
	{
		szExe.Empty();
		RConStartArgs rcs; rcs.pszSpecialCmd = lstrdup(Tests[i].pszCmd);
		rcs.ProcessNewConArg();
		b = IsNeedCmd(TRUE, rcs.pszSpecialCmd, szExe, &psArgs, &bNeedCut, &bRootIsCmd, &bAlwaysConfirm, &bAutoDisable);
		_ASSERTE(b == Tests[i].bNeed);
	}
}

void DebugStrUnitTest()
{
	struct strTests { wchar_t szTest[100], szCmp[100]; }
	Tests[] = {
		{L"Line1\n#Comment1\nLine2\r\n#Comment2\r\nEnd of file", L"Line1\nLine2\r\nEnd of file"},
		{L"Line1\n#Comment1\r\n", L"Line1\n"}
	};
	int iCmp;
	for (INT_PTR i = 0; i < countof(Tests); i++)
	{
		StripLines(Tests[i].szTest, L"#");
		iCmp = wcscmp(Tests[i].szTest, Tests[i].szCmp);
		_ASSERTE(iCmp == 0);
		StripLines(Tests[i].szTest, L"#");
		iCmp = wcscmp(Tests[i].szTest, Tests[i].szCmp);
		_ASSERTE(iCmp == 0);
	}

	// Some compilation and operators check

	CEStr str1;
	LPCWSTR pszTest = Tests[0].szTest;
	{
		str1 = lstrdup(pszTest);
	}
	iCmp = lstrcmp(str1, pszTest);
	_ASSERTE(iCmp == 0);

	{
		CEStr str2(lstrdup(pszTest));
		wchar_t* pszDup = lstrdup(pszTest);
		iCmp = lstrcmp(str2, pszTest);
		_ASSERTE(iCmp == 0 && str2.ms_Arg && str2.ms_Arg != pszTest);
	}

	// The following block must to be able to compile
	#if 0
	{
		CEStr str3(pszDup);
		str3 = pszTest;
	}

	{
		CEStr str4;
		str4 = pszTest;
		iCmp = lstrcmp(str4, pszTest);
		_ASSERTE(iCmp == 0 && str4.ms_Arg && str4.ms_Arg != pszTest);
	}

	{
		CEStr str5;
		str5 = str1;
		iCmp = lstrcmp(str5, pszTest);
		_ASSERTE(iCmp == 0 && str5.ms_Arg && str3.ms_Arg != str1.ms_Arg);
	}
	#endif

	iCmp = lstrcmp(str1, pszTest);
	_ASSERTE(iCmp == 0);
}

void DebugUnitTests()
{
	RConStartArgs::RunArgTests();
	DebugNeedCmdUnitTests();
	UnitMaskTests();
	UnitDriveTests();
	UnitFileNamesTest();
	UnitExpandTest();
	UnitModuleTest();
	DebugUnitMprintfTest();
	DebugVersionTest();
	DebugFileExistTests();
	ConEmuMacro::UnitTests();
	DebugStrUnitTest();
	CMatch::UnitTests();
	ConEmuChord::ChordUnitTests();
	ConEmuHotKey::HotkeyNameUnitTests();
}
#endif

#if defined(__GNUC__) || defined(_DEBUG)
void GnuUnitTests()
{
	int nRegW = RegisterClipboardFormatW(CFSTR_FILENAMEW/*L"FileNameW"*/);
	int nRegA = RegisterClipboardFormatA("FileNameW");
	Assert(nRegW && nRegA && nRegW==nRegA);

	wchar_t szHex[] = L"\x2018"/*�*/ L"\x2019"/*�*/;
	wchar_t szNum[] = {0x2018, 0x2019, 0};
	int iDbg = lstrcmp(szHex, szNum);
	Assert(iDbg==0);
}
#endif
