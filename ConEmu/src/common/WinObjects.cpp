
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
#include "MAssert.h"
#include "WinObjects.h"

#ifndef TTS_BALLOON
#define TTS_BALLOON             0x40
#define TTF_TRACK               0x0020
#define TTF_ABSOLUTE            0x0080
#define TTM_SETMAXTIPWIDTH      (WM_USER + 24)
#define TTM_TRACKPOSITION       (WM_USER + 18)  // lParam = dwPos
#define TTM_TRACKACTIVATE       (WM_USER + 17)  // wParam = TRUE/FALSE start end  lparam = LPTOOLINFO
#endif


#ifdef _DEBUG
void getWindowInfo(HWND ahWnd, wchar_t (&rsInfo)[1024])
{
//#ifdef CONEMU_MINIMAL
//	rsInfo[0] = 0;
//#else
	if (!ahWnd)
	{
		wcscpy_c(rsInfo, L"<NULL>");
	}
	else if (!isWindow(ahWnd))
	{
		msprintf(rsInfo, countof(rsInfo), L"0x%08X: Invalid window handle", (DWORD)ahWnd);
	}
	else
	{
		wchar_t szClass[256], szTitle[512];
		
		// �������� ����������� �������� ��� user32.dll
		typedef int (WINAPI* GetClassName_t)(HWND hWnd,LPWSTR lpClassName,int nMaxCount);
		static GetClassName_t GetClassName_f = NULL;
		typedef int (WINAPI* GetWindowText_t)(HWND hWnd, LPWSTR lpString, int nMaxCount);
		static GetWindowText_t GetWindowText_f = NULL;
		
		if (!GetClassName_f)
		{
			HMODULE hUser32 = GetModuleHandle(L"user32.dll");
			if (!hUser32)
			{
				// ������ �����, user32 ��� ������ ���� ��������, �� ���� ����� - ������
				// �����, ���� LoadLibrary ����� ���������� �� DllMain
				_ASSERTE(hUser32!=NULL);
			}
			else
			{
				GetClassName_f = (GetClassName_t)GetProcAddress(hUser32,"GetClassNameW");
				GetWindowText_f = (GetWindowText_t)GetProcAddress(hUser32,"GetWindowTextW");
			}
		}
		
		if (!GetClassName_f || !GetClassName_f(ahWnd, szClass, 256)) wcscpy_c(szClass, L"<GetClassName failed>");
		if (!GetWindowText_f || !GetWindowText_f(ahWnd, szTitle, 512)) szTitle[0] = 0;

		msprintf(rsInfo, countof(rsInfo), L"0x%08X: %s - '%s'", (DWORD)ahWnd, szClass, szTitle);
	}
//#endif
}
#endif

#ifndef CONEMU_MINIMAL
BOOL apiSetForegroundWindow(HWND ahWnd)
{
#ifdef _DEBUG
	wchar_t szLastFore[1024]; getWindowInfo(GetForegroundWindow(), szLastFore);
	wchar_t szWnd[1024]; getWindowInfo(ahWnd, szWnd);
#endif
	BOOL lbRc = ::SetForegroundWindow(ahWnd);
	return lbRc;
}
#endif

#ifndef CONEMU_MINIMAL
BOOL apiShowWindow(HWND ahWnd, int anCmdShow)
{
#ifdef _DEBUG
	wchar_t szLastFore[1024]; getWindowInfo(GetForegroundWindow(), szLastFore);
	wchar_t szWnd[1024]; getWindowInfo(ahWnd, szWnd);
#endif
	BOOL lbRc = ::ShowWindow(ahWnd, anCmdShow);
	return lbRc;
}
#endif

#ifndef CONEMU_MINIMAL
BOOL apiShowWindowAsync(HWND ahWnd, int anCmdShow)
{
#ifdef _DEBUG
	wchar_t szLastFore[1024]; getWindowInfo(GetForegroundWindow(), szLastFore);
	wchar_t szWnd[1024]; getWindowInfo(ahWnd, szWnd);
#endif
	BOOL lbRc = ::ShowWindowAsync(ahWnd, anCmdShow);
	return lbRc;
}
#endif


IsWindow_t Is_Window = NULL;
BOOL isWindow(HWND hWnd)
{
	if (!hWnd)
		return FALSE;
	if (!Is_Window)
	{
		HMODULE hUser = GetModuleHandle(L"User32.dll");
		if (hUser)
			Is_Window = (IsWindow_t)GetProcAddress(hUser, "IsWindow");
	}
	if (Is_Window)
		return Is_Window(hWnd);
	return TRUE;
}

// pnSize ����������� ������ � ��� ������, ���� ���� ������
BOOL FileExists(LPCWSTR asFilePath, DWORD* pnSize /*= NULL*/)
{
	WIN32_FIND_DATAW fnd = {0};
	HANDLE hFind = FindFirstFile(asFilePath, &fnd);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		BOOL lbFileFound = FALSE;

		// FindFirstFile ����� ���������� ��-�� ���������
		if (GetLastError() == ERROR_ACCESS_DENIED)
		{
			hFind = CreateFile(asFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

			if (hFind != NULL)
			{
				BY_HANDLE_FILE_INFORMATION fi = {0};

				if (GetFileInformationByHandle(hFind, &fi) && !(fi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					lbFileFound = TRUE;

					if (pnSize)
						*pnSize = fi.nFileSizeHigh ? 0xFFFFFFFF : fi.nFileSizeLow; //-V112
				}
			}

			CloseHandle(hFind);
		}

		return lbFileFound;
	}

	BOOL lbFound = FALSE;

	do
	{
		if ((fnd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			lbFound = TRUE;

			if (pnSize)
				*pnSize = fnd.nFileSizeHigh ? 0xFFFFFFFF : fnd.nFileSizeLow; //-V112

			break;
		}
	}
	while(FindNextFile(hFind, &fnd));

	FindClose(hFind);
	return lbFound;
}

// ��������� ��������, ����� �� asFilePath ���� �����
BOOL IsFilePath(LPCWSTR asFilePath)
{
	// ���� � ���� ����������� ������������ �������
	if (wcschr(asFilePath, L'"') ||
	        wcschr(asFilePath, L'>') ||
	        wcschr(asFilePath, L'<') ||
	        wcschr(asFilePath, L'|')
	  )
		return FALSE;

	// ������� UNC "\\?\"
	if (asFilePath[0] == L'\\' && asFilePath[1] == L'\\' && asFilePath[2] == L'?' && asFilePath[3] == L'\\')
		asFilePath += 4; //-V112

	// ���� asFilePath �������� ��� (� �����) ":\"
	LPCWSTR pszColon = wcschr(asFilePath, L':');

	if (pszColon)
	{
		// ���� ���� ":", �� ��� ������ ���� ���� ���� "X:\xxx", �.�. ":" - ������ ������
		if (pszColon != (asFilePath+1))
			return FALSE;

		if (wcschr(pszColon+1, L':'))
			return FALSE;
	}

	// May be file path
	return TRUE;
}

BOOL GetShortFileName(LPCWSTR asFullPath, int cchShortNameMax, wchar_t* rsShortName/*[MAX_PATH+1]-name only*/, BOOL abFavorLength=FALSE)
{
	WARNING("FindFirstFile ������������ ������ ��-�� ���������");
	WIN32_FIND_DATAW fnd; memset(&fnd, 0, sizeof(fnd));
	HANDLE hFind = FindFirstFile(asFullPath, &fnd);

	if (hFind == INVALID_HANDLE_VALUE)
		return FALSE;

	FindClose(hFind);

	if (fnd.cAlternateFileName[0])
	{
		if ((abFavorLength && (lstrlenW(fnd.cAlternateFileName) < lstrlenW(fnd.cFileName))) //-V303
		        || (wcschr(fnd.cFileName, L' ') != NULL))
		{
			if (lstrlen(fnd.cAlternateFileName) >= cchShortNameMax) //-V303
				return FALSE;
			_wcscpy_c(rsShortName, cchShortNameMax, fnd.cAlternateFileName); //-V106
			return TRUE;
		}
	}
	else if (wcschr(fnd.cFileName, L' ') != NULL)
	{
		return FALSE;
	}
	
	if (lstrlen(fnd.cFileName) >= cchShortNameMax) //-V303
		return FALSE;
	_wcscpy_c(rsShortName, cchShortNameMax, fnd.cFileName); //-V106
	return TRUE;
}

wchar_t* GetShortFileNameEx(LPCWSTR asLong, BOOL abFavorLength/*=TRUE*/)
{
	if (!asLong)
		return NULL;
	
	int nSrcLen = lstrlenW(asLong); //-V303
	wchar_t* pszLong = lstrdup(asLong);
	
	int nMaxLen = nSrcLen + MAX_PATH; // "��������" ��� ����� ����� MAX_PATH
	wchar_t* pszShort = (wchar_t*)calloc(nMaxLen, sizeof(wchar_t)); //-V106
	
	wchar_t* pszResult = NULL;
	wchar_t* pszSrc = pszLong;
	//wchar_t* pszDst = pszShort;
	wchar_t* pszSlash;
	wchar_t* szName = (wchar_t*)malloc((MAX_PATH+1)*sizeof(wchar_t));
	bool     lbNetwork = false;
	int      nLen, nCurLen = 0;
	
	// ���� ���� ������� (��� UNC?) ���������� ��������/�������
	if (pszSrc[0] == L'\\' && pszSrc[1] == '\\')
	{
		// ������� ������ ���� ������
		pszSrc += 2;
		// ������ "�����" �� ������������ \\.\Drive\...
		if (pszSrc[0] == L'.' && pszSrc[1] == L'\\')
			goto wrap;
		// UNC
		if (pszSrc[0] == L'?' && pszSrc[1] == L'\\')
		{
			pszSrc += 2;
			if (pszSrc[0] == L'U' && pszSrc[1] == L'N' && pszSrc[2] == L'C' && pszSrc[3] == L'\\')
			{
				// UNC\Server\share\...
				pszSrc += 4; //-V112
				lbNetwork = true;
			}
			// ����� - ��������� ����
		}
		// Network (\\Server\\Share\...)
		else
		{
			lbNetwork = true;
		}
	}
	
	if (pszSrc[0] == 0)
		goto wrap;
	
	if (lbNetwork)
	{
		pszSlash = wcschr(pszSrc, L'\\');
		if (!pszSlash)
			goto wrap;
		pszSlash = wcschr(pszSlash+1, L'\\');
		if (!pszSlash)
			goto wrap;
		pszShort[0] = L'\\'; pszShort[1] = L'\\'; pszShort[2] = 0;
		_wcscatn_c(pszShort, nMaxLen, pszSrc, (pszSlash-pszSrc+1)); // ������ ���������� calloc! //-V303 //-V104
	}
	else
	{
		// <Drive>:\path...
		if (pszSrc[1] != L':')
			goto wrap;
		if (pszSrc[2] != L'\\' && pszSrc[2] != 0)
			goto wrap;
		pszSlash = pszSrc + 2;
		_wcscatn_c(pszShort, nMaxLen, pszSrc, (pszSlash-pszSrc+1)); // ������ ���������� calloc!
	}
	
	nCurLen = lstrlenW(pszShort);
	
	while (pszSlash && (*pszSlash == L'\\'))
	{
		pszSrc = pszSlash;
		pszSlash = wcschr(pszSrc+1, L'\\');
		if (pszSlash)
			*pszSlash = 0;
		
		if (!GetShortFileName(pszLong, MAX_PATH+1, szName, abFavorLength))
			goto wrap;
		nLen = lstrlenW(szName);
		if ((nLen + nCurLen) >= nMaxLen)
			goto wrap;
		//pszShort[nCurLen++] = L'\\'; pszShort[nCurLen] = 0;
		_wcscpyn_c(pszShort+nCurLen, nMaxLen-nCurLen, szName, nLen);
		nCurLen += nLen;

		if (pszSlash)
		{
			*pszSlash = L'\\';
			pszShort[nCurLen++] = L'\\'; // ������ ���������� calloc!
		}
	}
	
	nLen = lstrlenW(pszShort);

	if ((nLen > 0) && (pszShort[nLen-1] == L'\\'))
		pszShort[--nLen] = 0;

	if (nLen <= MAX_PATH)
	{
		pszResult = pszShort;
		pszShort = NULL;
		goto wrap;
	}

wrap:
	if (szName)
		free(szName);
	if (pszShort)
		free(pszShort);
	if (pszLong)
		free(pszLong);
	return pszResult;
}

//wchar_t* GetShortFileNameEx(LPCWSTR asLong, BOOL abFavorLength=FALSE)
//{
//	TODO("������ �� � ������� ����� ������������");
//
//	if (!asLong) return NULL;
//
//	int nMaxLen = lstrlenW(asLong) + MAX_PATH; // "��������" ��� ����� ��������� ������� "��������"
//	wchar_t* pszShort = /*lstrdup(asLong);*/(wchar_t*)malloc(nMaxLen*2);
//	_wcscpy_c(pszShort, nMaxLen, asLong);
//	wchar_t* pszCur = wcschr(pszShort+3, L'\\');
//	wchar_t* pszSlash;
//	wchar_t  szName[MAX_PATH+1];
//	size_t nLen = 0;
//
//	while(pszCur)
//	{
//		*pszCur = 0;
//		{
//			if (GetShortFileName(pszShort, szName, abFavorLength))
//			{
//				if ((pszSlash = wcsrchr(pszShort, L'\\'))==0)
//					goto wrap;
//
//				_wcscpy_c(pszSlash+1, nMaxLen-(pszSlash-pszShort+1), szName);
//				pszSlash += 1+lstrlenW(szName);
//				_wcscpy_c(pszSlash+1, nMaxLen-(pszSlash-pszShort+1), pszCur+1);
//				pszCur = pszSlash;
//			}
//		}
//		*pszCur = L'\\';
//		pszCur = wcschr(pszCur+1, L'\\');
//	}
//
//	nLen = lstrlenW(pszShort);
//
//	if (nLen>0 && pszShort[nLen-1]==L'\\')
//		pszShort[--nLen] = 0;
//
//	if (nLen <= MAX_PATH)
//		return pszShort;
//
//wrap:
//	free(pszShort);
//	return NULL;
//}

#ifndef CONEMU_MINIMAL
BOOL IsUserAdmin()
{
	OSVERSIONINFO osv = {sizeof(OSVERSIONINFO)};
	GetVersionEx(&osv);

	// ��������� ����� ������ ��� �����, ����� �� XP ������ "���" �� �����������
	if (osv.dwMajorVersion < 6)
		return FALSE;

	BOOL b;
	SID_IDENTIFIER_AUTHORITY NtAuthority = {SECURITY_NT_AUTHORITY};
	PSID AdministratorsGroup;
	b = AllocateAndInitializeSid(
	        &NtAuthority,
	        2,
	        SECURITY_BUILTIN_DOMAIN_RID,
	        DOMAIN_ALIAS_RID_ADMINS,
	        0, 0, 0, 0, 0, 0,
	        &AdministratorsGroup);

	if (b)
	{
		if (!CheckTokenMembership(NULL, AdministratorsGroup, &b))
		{
			b = FALSE;
		}

		FreeSid(AdministratorsGroup);
	}

	return(b);
}


#include <Sddl.h> // ConvertSidToStringSid
// *ppszSID - must be LocalFree'd
BOOL GetLogonSID (HANDLE hToken, wchar_t **ppszSID)
{
	BOOL bSuccess = FALSE;
	//DWORD dwIndex;
	DWORD dwLength = 0;
	TOKEN_USER user;
	PTOKEN_USER ptu = &user;
	BOOL bFreeToken = FALSE;

	// Verify the parameter passed in is not NULL.
	if (NULL == ppszSID)
		goto Cleanup;
	*ppszSID = NULL;
		
	if (!hToken)
		bFreeToken = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken);

	// Get required buffer size and allocate the TOKEN_GROUPS buffer.
	if (!GetTokenInformation(
		hToken,         // handle to the access token
		TokenUser,      // get information about the token's user account
		(LPVOID) ptu,   // pointer to TOKEN_USER buffer
		0,              // size of buffer
		&dwLength       // receives required buffer size
	)) 
	{
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) 
			goto Cleanup;

		ptu = (PTOKEN_USER)calloc(dwLength,1);

		if (ptu == NULL)
			goto Cleanup;
	}

	// Get the token group information from the access token.

	if (!GetTokenInformation(
		hToken,         // handle to the access token
		TokenUser,      // get information about the token's user account
		(LPVOID) ptu,   // pointer to TOKEN_USER buffer
		dwLength,       // size of buffer
		&dwLength       // receives required buffer size
	)) 
	{
		goto Cleanup;
	}

	if (!ConvertSidToStringSid(ptu->User.Sid, ppszSID) || (*ppszSID == NULL))
		goto Cleanup;

	bSuccess = TRUE;

Cleanup: 

	// Free the buffer for the token groups.
	if ((ptu != NULL) && (ptu != &user))
		free(ptu);
	if (bFreeToken && hToken)
		CloseHandle(hToken);
	
	return bSuccess;
}
#endif



typedef BOOL (WINAPI* IsWow64Process_t)(HANDLE hProcess, PBOOL Wow64Process);

// pbIsWow64Process <- TRUE, ���� 32������ ������� ������� � 64������ ��
BOOL IsWindows64(BOOL *pbIsWow64Process/* = NULL */)
{
	BOOL is64bitOs = FALSE, isWow64process = FALSE;
#ifdef WIN64
	is64bitOs = TRUE; isWow64process = FALSE;
#else
	// ���������, ��� �� ��������
	isWow64process = FALSE;
	HMODULE hKernel = GetModuleHandleW(L"kernel32.dll");

	if (hKernel)
	{
		IsWow64Process_t IsWow64Process_f = (IsWow64Process_t)GetProcAddress(hKernel, "IsWow64Process");

		if (IsWow64Process_f)
		{
			BOOL bWow64 = FALSE;

			if (IsWow64Process_f(GetCurrentProcess(), &bWow64) && bWow64)
			{
				isWow64process = TRUE;
			}
		}
	}

	is64bitOs = isWow64process;
#endif

	if (pbIsWow64Process)
		*pbIsWow64Process = isWow64process;

	return is64bitOs;
}

#ifndef CONEMU_MINIMAL
void RemoveOldComSpecC()
{
	wchar_t szComSpec[MAX_PATH], szComSpecC[MAX_PATH], szRealComSpec[MAX_PATH];
	//110202 - comspec ����� �� ����������������, ������� ������ "cmd", 
	// ���� ��� ����������� � ����������� �� ������ ������ conemu
	if (GetEnvironmentVariable(L"ComSpecC", szComSpecC, countof(szComSpecC)) && szComSpecC[0] != 0)
	{
		szRealComSpec[0] = 0;

		if (!GetEnvironmentVariable(L"ComSpec", szComSpec, countof(szComSpec)))
			szComSpec[0] = 0;

		#ifndef __GNUC__
		#pragma warning( push )
		#pragma warning(disable : 6400)
		#endif

		LPCWSTR pwszName = PointToName(szComSpec);

		if (lstrcmpiW(pwszName, L"ConEmuC.exe")==0 || lstrcmpiW(pwszName, L"ConEmuC64.exe")==0)
		{
			pwszName = PointToName(szComSpecC);
			if (lstrcmpiW(pwszName, L"ConEmuC.exe")!=0 && lstrcmpiW(pwszName, L"ConEmuC64.exe")!=0)
			{
				wcscpy_c(szRealComSpec, szComSpecC);
			}
		}
		#ifndef __GNUC__
		#pragma warning( pop )
		#endif

		if (szRealComSpec[0] == 0)
		{
			//\system32\cmd.exe
			if (GetWindowsDirectoryW(szRealComSpec, MAX_PATH-19))
			{
				int nLen = lstrlenW(szRealComSpec);
				if (szRealComSpec[nLen-1] != L'\\')
					wcscat_c(szRealComSpec, L"\\");
				wcscat_c(szRealComSpec, L"system32\\cmd.exe");
			}
			else
			{
				wcscpy_c(szRealComSpec, L"c:\\windows\\system32\\cmd.exe");
			}
		}

		SetEnvironmentVariable(L"ComSpec", szRealComSpec);
		SetEnvironmentVariable(L"ComSpecC", NULL);
	}
		//// ������ ���� ��� (��������) �� conemuc.exe
		//wchar_t* pwszCopy = (wchar_t*)PointToName(szComSpec); //wcsrchr(szComSpec, L'\\');
		////if (!pwszCopy) pwszCopy = szComSpec;

		//if (lstrcmpiW(pwszCopy, L"ConEmuC")==0 || lstrcmpiW(pwszCopy, L"ConEmuC.exe")==0
		//        /*|| lstrcmpiW(pwszCopy, L"ConEmuC64")==0 || lstrcmpiW(pwszCopy, L"ConEmuC64.exe")==0*/)
		//	szComSpec[0] = 0;
		//#pragma warning( pop )

		//if (szComSpec[0])
		//{
		//}

}
#endif

const wchar_t* PointToName(const wchar_t* asFileOrPath)
{
	if (!asFileOrPath)
	{
		_ASSERTE(asFileOrPath!=NULL);
		return NULL;
	}
	const wchar_t* pszFile = wcsrchr(asFileOrPath, L'\\');
	if (!pszFile) pszFile = asFileOrPath; else pszFile++;

	return pszFile;
}

const char* PointToName(const char* asFileOrPath)
{
	if (!asFileOrPath)
	{
		_ASSERTE(asFileOrPath!=NULL);
		return NULL;
	}

	const char* pszSlash = strrchr(asFileOrPath, '\\');

	if (pszSlash)
		return pszSlash+1;

	return asFileOrPath;
}

// ���������� ".ext" ��� NULL � ������ ������
const wchar_t* PointToExt(const wchar_t* asFullPath)
{
	const wchar_t* pszName = PointToName(asFullPath);
	if (!pszName)
		return NULL; // _ASSERTE ��� ���
	const wchar_t* pszExt = wcsrchr(pszName, L'.');
	return pszExt;
}

// !!! ������ asPath !!!
const wchar_t* Unquote(wchar_t* asPath)
{
	if (!asPath)
		return NULL;
	if (*asPath != L'"')
		return asPath;
	wchar_t* pszEndQ = wcsrchr(asPath, L'"');
	if (!pszEndQ || (pszEndQ == asPath))
	{
		*asPath = 0;
		return asPath;
	}
	*pszEndQ = 0;
	return (asPath+1);
}


#ifndef __GNUC__
#pragma warning( push )
#pragma warning(disable : 6400)
#endif
BOOL IsExecutable(LPCWSTR aszFilePathName)
{
#ifndef __GNUC__
#pragma warning( push )
#pragma warning(disable : 6400)
#endif
	LPCWSTR pwszDot = wcsrchr(aszFilePathName, L'.');

	if (pwszDot)  // ���� ������ .exe ��� .com ����
	{
		if (lstrcmpiW(pwszDot, L".exe")==0 || lstrcmpiW(pwszDot, L".com")==0)
		{
			if (FileExists(aszFilePathName))
				return TRUE;
		}
	}

	return FALSE;
}
#ifndef __GNUC__
#pragma warning( pop )
#endif

// �������, ������� �� ����� �������� ���������� � exe?
// ����� ����, ���� ������� �������� "?" ��� "*" - ���� �� ��������.
const wchar_t* gsInternalCommands = L"ACTIVATE\0ALIAS\0ASSOC\0ATTRIB\0BEEP\0BREAK\0CALL\0CDD\0CHCP\0COLOR\0COPY\0DATE\0DEFAULT\0DEL\0DELAY\0DESCRIBE\0DETACH\0DIR\0DIRHISTORY\0DIRS\0DRAWBOX\0DRAWHLINE\0DRAWVLINE\0ECHO\0ECHOERR\0ECHOS\0ECHOSERR\0ENDLOCAL\0ERASE\0ERRORLEVEL\0ESET\0EXCEPT\0EXIST\0EXIT\0FFIND\0FOR\0FREE\0FTYPE\0GLOBAL\0GOTO\0HELP\0HISTORY\0IF\0IFF\0INKEY\0INPUT\0KEYBD\0KEYS\0LABEL\0LIST\0LOG\0MD\0MEMORY\0MKDIR\0MOVE\0MSGBOX\0NOT\0ON\0OPTION\0PATH\0PAUSE\0POPD\0PROMPT\0PUSHD\0RD\0REBOOT\0REN\0RENAME\0RMDIR\0SCREEN\0SCRPUT\0SELECT\0SET\0SETDOS\0SETLOCAL\0SHIFT\0SHRALIAS\0START\0TEE\0TIME\0TIMER\0TITLE\0TOUCH\0TREE\0TRUENAME\0TYPE\0UNALIAS\0UNSET\0VER\0VERIFY\0VOL\0VSCRPUT\0WINDOW\0Y\0\0";

BOOL IsNeedCmd(LPCWSTR asCmdLine, BOOL *rbNeedCutStartEndQuot, wchar_t (&szExe)[MAX_PATH+1],
			   BOOL& rbRootIsCmdExe, BOOL& rbAlwaysConfirmExit, BOOL& rbAutoDisableConfirmExit)
{
	_ASSERTE(asCmdLine && *asCmdLine);
	rbRootIsCmdExe = TRUE;

	memset(szExe, 0, sizeof(szExe));

	if (!asCmdLine || *asCmdLine == 0)
		return TRUE;
		
	//110202 ������� ����, �.�. ��� ��� ����� ���� cmd.exe, � ����� � ���� ������ �����
	//// ���� ���� ���� �� ������ ���������������, ��� ������� - ����� CMD.EXE
	//if (wcschr(asCmdLine, L'&') ||
	//        wcschr(asCmdLine, L'>') ||
	//        wcschr(asCmdLine, L'<') ||
	//        wcschr(asCmdLine, L'|') ||
	//        wcschr(asCmdLine, L'^') // ��� �������������
	//  )
	//{
	//	return TRUE;
	//}

	//wchar_t szArg[MAX_PATH+10] = {0};
	int iRc = 0;
	BOOL lbFirstWasGot = FALSE;
	LPCWSTR pwszCopy = asCmdLine;
	// cmd /c ""c:\program files\arc\7z.exe" -?"   // �� ��� � ������ ����� ���� ��������...
	// cmd /c "dir c:\"
	int nLastChar = lstrlenW(pwszCopy) - 1;

	if (pwszCopy[0] == L'"' && pwszCopy[nLastChar] == L'"')
	{
		if (pwszCopy[1] == L'"' && pwszCopy[2])
		{
			pwszCopy ++; // ��������� ������ ������� � �������� ����: ""c:\program files\arc\7z.exe" -?"

			if (rbNeedCutStartEndQuot) *rbNeedCutStartEndQuot = TRUE;
		}
		else
			// ������� �� ""F:\VCProject\FarPlugin\#FAR180\far.exe  -new_console""
			//if (wcschr(pwszCopy+1, L'"') == (pwszCopy+nLastChar)) {
			//	LPCWSTR pwszTemp = pwszCopy;
			//	// ������� ������ ������� (����������� ����?)
			//	if ((iRc = NextArg(&pwszTemp, szArg)) != 0) {
			//		//Parsing command line failed
			//		return TRUE;
			//	}
			//	pwszCopy ++; // ��������� ������ ������� � �������� ����: "c:\arc\7z.exe -?"
			//	lbFirstWasGot = TRUE;
			//	if (rbNeedCutStartEndQuot) *rbNeedCutStartEndQuot = TRUE;
			//} else
		{
			// ��������� ������ ������� �: "C:\GCC\msys\bin\make.EXE -f "makefile" COMMON="../../../plugins/common""
			LPCWSTR pwszTemp = pwszCopy + 1;

			// ������� ������ ������� (����������� ����?)
			if ((iRc = NextArg(&pwszTemp, szExe)) != 0)
			{
				//Parsing command line failed
				return TRUE;
			}
			
			if (lstrcmpiW(szExe, L"start") == 0)
			{
				// ������� start ������������ ������ ���������
				return TRUE;
			}

			LPCWSTR pwszQ = pwszCopy + 1 + lstrlen(szExe);

			if (*pwszQ != L'"' && IsExecutable(szExe))
			{
				pwszCopy ++; // �����������
				lbFirstWasGot = TRUE;

				if (rbNeedCutStartEndQuot) *rbNeedCutStartEndQuot = TRUE;
			}
		}
	}

	// ������� ������ ������� (����������� ����?)
	if (!lbFirstWasGot)
	{
		szExe[0] = 0;
		// 17.10.2010 - ��������� ����������� ������������ ����� ��� ����������, �� � ��������� � ����
		LPCWSTR pchEnd = pwszCopy + lstrlenW(pwszCopy);

		while(pchEnd > pwszCopy && *(pchEnd-1) == L' ') pchEnd--;

		if ((pchEnd - pwszCopy) < MAX_PATH)
		{
			memcpy(szExe, pwszCopy, (pchEnd - pwszCopy)*sizeof(wchar_t));
			szExe[(pchEnd - pwszCopy)] = 0;

			if (!FileExists(szExe))
				szExe[0] = 0;
		}

		if (szExe[0] == 0)
		{
			if ((iRc = NextArg(&pwszCopy, szExe)) != 0)
			{
				//Parsing command line failed
				return TRUE;
			}
		}
	}
	
	if (!*szExe)
	{
		_ASSERTE(szExe[0] != 0);
	}
	else
	{
		// "�����" ������� � ����� �����
		if (wcspbrk(szExe, L"?*<>|"))
		{
			rbRootIsCmdExe = TRUE; // ������ ����� "���������"
			return TRUE; // �������� "cmd.exe"
		}
		
		// ���� "����" �� ������
		if (wcschr(szExe, L'\\') == NULL) 
		{
			bool bHasExt = (wcschr(szExe, L'.') != NULL);
			// ��������, ����� ��� ������� ���������� (���� "DIR")?
			if (!bHasExt)
			{
				bool bIsCommand = false;
				wchar_t* pszUppr = lstrdup(szExe);
				if (pszUppr)
				{
					// �������� �������� �� user32.dll
					//CharUpperBuff(pszUppr, lstrlen(pszUppr));
					for (wchar_t* p = pszUppr; *p; p++)
					{
						if (*p >= L'a' && *p <= 'z')
							*p -= 0x20;
					}
					
					const wchar_t* pszFind = gsInternalCommands;
					while (*pszFind)
					{
						if (lstrcmp(pszUppr, pszFind) == 0)
						{
							bIsCommand = true;
							break;
						}
						pszFind += lstrlen(pszFind)+1;
					}
					free(pszUppr);
				}
				if (bIsCommand)
				{
					rbRootIsCmdExe = TRUE; // ������ ����� "���������"
					return TRUE; // �������� "cmd.exe"
				}
			}
			
			// ������� ����� "�� �����" ��������������� exe-����.
			DWORD nCchMax = countof(szExe); // �������� ������, ������ ��� szExe ������� �� ������
			wchar_t* pszSearch = (wchar_t*)malloc(nCchMax*sizeof(wchar_t));
			if (pszSearch)
			{
				#ifndef CONEMU_MINIMAL
				MWow64Disable wow; wow.Disable(); // ��������� ����������!
				#endif
				wchar_t *pszName = NULL;
				DWORD nRc = SearchPath(NULL, szExe, bHasExt ? NULL : L".exe", nCchMax, pszSearch, &pszName);
				if (nRc && (nRc < nCchMax))
				{
					// �����, ���������� ��� �����
					wcscpy_c(szExe, pszSearch);
				}
				free(pszSearch);
			}
		} // end: if (wcschr(szExe, L'\\') == NULL) 
	}

	// ���� szExe �� �������� ���� � ����� - ��������� ����� cmd
	// "start "" C:\Utils\Files\Hiew32\hiew32.exe C:\00\Far.exe"
	if (!IsFilePath(szExe))
	{
		rbRootIsCmdExe = TRUE; // ������ ����� "���������"
		return TRUE; // �������� "cmd.exe"
	}

	//pwszCopy = wcsrchr(szArg, L'\\'); if (!pwszCopy) pwszCopy = szArg; else pwszCopy ++;
	pwszCopy = PointToName(szExe);
	//2009-08-27
	wchar_t *pwszEndSpace = szExe + lstrlenW(szExe) - 1;

	while((*pwszEndSpace == L' ') && (pwszEndSpace > szExe))
		*(pwszEndSpace--) = 0;

#ifndef __GNUC__
#pragma warning( push )
#pragma warning(disable : 6400)
#endif

	if (lstrcmpiW(pwszCopy, L"cmd")==0 || lstrcmpiW(pwszCopy, L"cmd.exe")==0)
	{
		rbRootIsCmdExe = TRUE; // ��� ������ ���� ���������, �� ��������
		rbAlwaysConfirmExit = TRUE; rbAutoDisableConfirmExit = FALSE;
		return FALSE; // ��� ������ ��������� ���������, cmd.exe � ������ ��������� �� �����
	}


	// ���� ���� ���� �� ������ ���������������, ��� ������� - ����� CMD.EXE
	if (wcschr(asCmdLine, L'&') ||
		wcschr(asCmdLine, L'>') ||
		wcschr(asCmdLine, L'<') ||
		wcschr(asCmdLine, L'|') ||
		wcschr(asCmdLine, L'^') // ��� �������������
		)
	{
		return TRUE;
	}


	if (lstrcmpiW(pwszCopy, L"far")==0 || lstrcmpiW(pwszCopy, L"far.exe")==0)
	{
		rbAutoDisableConfirmExit = TRUE;
		rbRootIsCmdExe = FALSE; // FAR!
		return FALSE; // ��� ������ ��������� ���������, cmd.exe � ������ ��������� �� �����
	}

	if (IsExecutable(szExe))
	{
		rbRootIsCmdExe = FALSE; // ��� ������ �������� - ����� �� ��������
		return FALSE; // ����������� ���������� ���������� ���������. cmd.exe �� ���������
	}

	//����� ��� �������� ������ �: SearchPath, GetFullPathName, ������� ���������� .exe & .com
	//���� ��� ��� ��������� ������ ���� � ��������, ��� ��� ����� �� ��������������
	rbRootIsCmdExe = TRUE;
#ifndef __GNUC__
#pragma warning( pop )
#endif
	return TRUE;
}

const wchar_t* SkipNonPrintable(const wchar_t* asParams)
{
	if (!asParams)
		return NULL;
	const wchar_t* psz = asParams;
	while (*psz == L' ' || *psz == L'\t' || *psz == L'\r' || *psz == L'\n') psz++;
	return psz;
}


//// ������� ���� � �����, ���������� ConEmuC.exe
//BOOL FindConEmuBaseDir(wchar_t (&rsConEmuBaseDir)[MAX_PATH+1], wchar_t (&rsConEmuExe)[MAX_PATH+1])
//{
//	// ������� ������� Mapping ������� (����� ����?)
//	{
//		MFileMapping<CESERVER_CONSOLE_MAPPING_HDR> ConMap;
//		ConMap.InitName(CECONMAPNAME, (DWORD)GetConEmuHWND(2));
//		CESERVER_CONSOLE_MAPPING_HDR* p = ConMap.Open();
//		if (p && p->sConEmuBaseDir[0])
//		{
//			// �������
//			wcscpy_c(rsConEmuBaseDir, p->sConEmuBaseDir);
//			wcscpy_c(rsConEmuExe, p->sConEmuExe);
//			return TRUE;
//		}
//	}
//
//	// ������ - ������� ����� ������������ ���� ConEmu
//	HWND hConEmu = FindWindow(VirtualConsoleClassMain, NULL);
//	DWORD dwGuiPID = 0;
//	if (hConEmu)
//	{
//		if (GetWindowThreadProcessId(hConEmu, &dwGuiPID) && dwGuiPID)
//		{
//			MFileMapping<ConEmuGuiMapping> GuiMap;
//			GuiMap.InitName(CEGUIINFOMAPNAME, dwGuiPID);
//			ConEmuGuiMapping* p = GuiMap.Open();
//			if (p && p->sConEmuBaseDir[0])
//			{
//				wcscpy_c(rsConEmuBaseDir, p->sConEmuBaseDir);
//				wcscpy_c(rsConEmuExe, p->sConEmuExe);
//				return TRUE;
//			}
//		}
//	}
//
//	
//	wchar_t szExePath[MAX_PATH+1];
//	HKEY hkRoot[] = {NULL,HKEY_CURRENT_USER,HKEY_LOCAL_MACHINE,HKEY_LOCAL_MACHINE};
//	DWORD samDesired = KEY_QUERY_VALUE;
//	DWORD RedirectionFlag = 0;
//	BOOL isWin64 = FALSE;
//	#ifdef _WIN64
//	isWin64 = TRUE;
//	RedirectionFlag = KEY_WOW64_32KEY;
//	#else
//	isWin64 = IsWindows64();
//	RedirectionFlag = isWin64 ? KEY_WOW64_64KEY : 0;
//	#endif
//	for (size_t i = 0; i < countof(hkRoot); i++)
//	{
//		szExePath[0] = 0;
//
//		if (i == 0)
//		{
//			// ����������� ConEmu.exe ���, ����� �������� � �������� �������� ����������
//
//			if (!GetModuleFileName(NULL, szExePath, countof(szExePath)-20))
//				continue;
//			wchar_t* pszName = wcsrchr(szExePath, L'\\');
//			if (!pszName)
//				continue;
//			*(pszName+1) = 0;
//		}
//		else
//		{
//			// ������� ��������� ���� - ���� ConEmu ���������� ����� MSI, �� ���� ������ � �������
//			// [HKEY_LOCAL_MACHINE\SOFTWARE\ConEmu]
//			// "InstallDir"="C:\\Utils\\Far180\\"
//
//			if (i == (countof(hkRoot)-1))
//			{
//				if (RedirectionFlag)
//					samDesired |= RedirectionFlag;
//				else
//					break;
//			}
//
//			HKEY hKey;
//			if (RegOpenKeyEx(hkRoot[i], L"Software\\ConEmu", 0, samDesired, &hKey) != ERROR_SUCCESS)
//				continue;
//			memset(szExePath, 0, countof(szExePath));
//			DWORD nType = 0, nSize = sizeof(szExePath)-20*sizeof(wchar_t);
//			int RegResult = RegQueryValueEx(hKey, L"", NULL, &nType, (LPBYTE)szExePath, &nSize);
//			RegCloseKey(hKey);
//			if (RegResult != ERROR_SUCCESS)
//				continue;
//		}
//
//		if (szExePath[0])
//		{
//			// ���� � ������ � ������� - ������ ����� �� ����. ���������
//			if (szExePath[lstrlen(szExePath)-1] != L'\\')
//				wcscat_c(szExePath, L"\\");
//			wcscpy_c(rsConEmuExe, szExePath);
//			BOOL lbExeFound = FALSE;
//			wchar_t* pszName = rsConEmuExe+lstrlen(rsConEmuExe);
//			LPCWSTR szGuiExe[2] = {L"ConEmu64.exe", L"ConEmu.exe"};
//			for (int i = 0; !lbExeFound && (i < countof(szGuiExe)); i++)
//			{
//				if (!i && !isWin64) continue;
//				wcscpy_add(pszName, rsConEmuExe, szGuiExe[i]);
//				lbExeFound = FileExists(rsConEmuExe);
//			}
//
//			// ���� GUI-exe ������ - ���� "base"
//			if (lbExeFound)
//			{
//				wchar_t* pszName = szExePath+lstrlen(szExePath);
//				LPCWSTR szSrvExe[4] = {L"ConEmuC64.exe", L"ConEmu\\ConEmuC64.exe", L"ConEmuC.exe", L"ConEmu\\ConEmuC.exe"};
//				for (int i = 0; (i < countof(szSrvExe)); i++)
//				{
//					if ((i <=1) && !isWin64) continue;
//					wcscpy_add(pszName, szExePath, szSrvExe[i]);
//					if (FileExists(szExePath))
//					{
//						pszName = wcsrchr(szExePath, L'\\');
//						if (pszName)
//						{
//							*pszName = 0; // ��� ����� �� �����!
//							wcscpy_c(rsConEmuBaseDir, szExePath);
//							return TRUE;
//						}
//					}
//				}
//			}
//		}
//	}
//
//	// �� �������
//	return FALSE;
//}

//// Undocumented console message
//#define WM_SETCONSOLEINFO			(WM_USER+201)
//
//#pragma pack(push, 1)
//
////
////	Structure to send console via WM_SETCONSOLEINFO
////
//typedef struct _CONSOLE_INFO
//{
//	ULONG		Length;
//	COORD		ScreenBufferSize;
//	COORD		WindowSize;
//	ULONG		WindowPosX;
//	ULONG		WindowPosY;
//
//	COORD		FontSize;
//	ULONG		FontFamily;
//	ULONG		FontWeight;
//	WCHAR		FaceName[32];
//
//	ULONG		CursorSize;
//	ULONG		FullScreen;
//	ULONG		QuickEdit;
//	ULONG		AutoPosition;
//	ULONG		InsertMode;
//
//	USHORT		ScreenColors;
//	USHORT		PopupColors;
//	ULONG		HistoryNoDup;
//	ULONG		HistoryBufferSize;
//	ULONG		NumberOfHistoryBuffers;
//
//	COLORREF	ColorTable[16];
//
//	ULONG		CodePage;
//	HWND		Hwnd;
//
//	WCHAR		ConsoleTitle[0x100];
//
//} CONSOLE_INFO;
//
//CONSOLE_INFO* gpConsoleInfoStr = NULL;
//HANDLE ghConsoleSection = NULL;
//const UINT gnConsoleSectionSize = sizeof(CONSOLE_INFO)+1024;
//
//#pragma pack(pop)
//
//
//
//
//
//
//
//
//
//
//BOOL GetShortFileName(LPCWSTR asFullPath, wchar_t* rsShortName/*name only, MAX_PATH required*/)
//{
//	WIN32_FIND_DATAW fnd; memset(&fnd, 0, sizeof(fnd));
//	HANDLE hFind = FindFirstFile(asFullPath, &fnd);
//	if (hFind == INVALID_HANDLE_VALUE)
//		return FALSE;
//	FindClose(hFind);
//	if (fnd.cAlternateFileName[0]) {
//		if (lstrlenW(fnd.cAlternateFileName) < lstrlenW(fnd.cFileName)) {
//			lstrcpyW(rsShortName, fnd.cAlternateFileName);
//			return TRUE;
//		}
//	}
//
//	return FALSE;
//}
//
//wchar_t* GetShortFileNameEx(LPCWSTR asLong)
//{
//	TODO("������ �� � ������� ����� ������������");
//	if (!asLong) return NULL;
//
//	wchar_t* pszShort = /*lstrdup(asLong);*/(wchar_t*)malloc((lstrlenW(asLong)+1)*2);
//	lstrcpyW(pszShort, asLong);
//	wchar_t* pszCur = wcschr(pszShort+3, L'\\');
//	wchar_t* pszSlash;
//	wchar_t  szName[MAX_PATH+1];
//	size_t nLen = 0;
//
//	while (pszCur) {
//		*pszCur = 0;
//		{
//			if (GetShortFileName(pszShort, szName)) {
//				if ((pszSlash = wcsrchr(pszShort, L'\\'))==0)
//					goto wrap;
//				lstrcpyW(pszSlash+1, szName);
//				pszSlash += 1+lstrlenW(szName);
//				lstrcpyW(pszSlash+1, pszCur+1);
//				pszCur = pszSlash;
//			}
//		}
//		*pszCur = L'\\';
//		pszCur = wcschr(pszCur+1, L'\\');
//	}
//	nLen = lstrlenW(pszShort);
//	if (nLen>0 && pszShort[nLen-1]==L'\\')
//		pszShort[--nLen] = 0;
//	if (nLen <= MAX_PATH)
//		return pszShort;
//
//wrap:
//	free(pszShort);
//	return NULL;
//}
//
//int NextArg(const wchar_t** asCmdLine, wchar_t* rsArg/*[MAX_PATH+1]*/, const wchar_t** rsArgStart/*=NULL*/)
//{
//    LPCWSTR psCmdLine = *asCmdLine, pch = NULL;
//    wchar_t ch = *psCmdLine;
//    size_t nArgLen = 0;
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
//        //if (!pch) pch = psCmdLine + lstrlenW(psCmdLine); // �� ����� ������
//    }
//
//    nArgLen = pch - psCmdLine;
//    if (nArgLen > MAX_PATH) return CERR_CMDLINE;
//
//    // ������� ��������
//    memcpy(rsArg, psCmdLine, nArgLen*sizeof(wchar_t));
//    if (rsArgStart) *rsArgStart = psCmdLine;
//    rsArg[nArgLen] = 0;
//
//    psCmdLine = pch;
//
//    // Finalize
//    ch = *psCmdLine; // ����� ��������� �� ����������� �������
//    if (ch == L'"') ch = *(++psCmdLine);
//    while (ch == L' ' || ch == L'\t' || ch == L'\r' || ch == L'\n') ch = *(++psCmdLine);
//    *asCmdLine = psCmdLine;
//
//    return 0;
//}
//
//
//BOOL PackInputRecord(const INPUT_RECORD* piRec, MSG* pMsg)
//{
//	_ASSERTE(pMsg!=NULL && piRec!=NULL);
//	memset(pMsg, 0, sizeof(MSG));
//
//    UINT nMsg = 0; WPARAM wParam = 0; LPARAM lParam = 0;
//    if (piRec->EventType == KEY_EVENT) {
//    	nMsg = piRec->Event.KeyEvent.bKeyDown ? WM_KEYDOWN : WM_KEYUP;
//
//		lParam |= (WORD)piRec->Event.KeyEvent.uChar.UnicodeChar;
//		lParam |= ((BYTE)piRec->Event.KeyEvent.wVirtualKeyCode) << 16;
//		lParam |= ((BYTE)piRec->Event.KeyEvent.wVirtualScanCode) << 24;
//
//        wParam |= (WORD)piRec->Event.KeyEvent.dwControlKeyState;
//        wParam |= ((DWORD)piRec->Event.KeyEvent.wRepeatCount & 0xFF) << 16;
//
//    } else if (piRec->EventType == MOUSE_EVENT) {
//		switch (piRec->Event.MouseEvent.dwEventFlags) {
//			case MOUSE_MOVED:
//				nMsg = MOUSE_EVENT_MOVE;
//				break;
//			case 0:
//				nMsg = MOUSE_EVENT_CLICK;
//				break;
//			case DOUBLE_CLICK:
//				nMsg = MOUSE_EVENT_DBLCLICK;
//				break;
//			case MOUSE_WHEELED:
//				nMsg = MOUSE_EVENT_WHEELED;
//				break;
//			case /*MOUSE_HWHEELED*/ 0x0008:
//				nMsg = MOUSE_EVENT_HWHEELED;
//				break;
//			default:
//				_ASSERT(FALSE);
//		}
//
//    	lParam = ((WORD)piRec->Event.MouseEvent.dwMousePosition.X)
//    	       | (((DWORD)(WORD)piRec->Event.MouseEvent.dwMousePosition.Y) << 16);
//
//		// max 0x0010/*FROM_LEFT_4ND_BUTTON_PRESSED*/
//		wParam |= ((DWORD)piRec->Event.MouseEvent.dwButtonState) & 0xFF;
//
//		// max - ENHANCED_KEY == 0x0100
//		wParam |= (((DWORD)piRec->Event.MouseEvent.dwControlKeyState) & 0xFFFF) << 8;
//
//		if (nMsg == MOUSE_EVENT_WHEELED || nMsg == MOUSE_EVENT_HWHEELED) {
//    		// HIWORD() - short (direction[1/-1])*count*120
//    		short nWheel = (short)((((DWORD)piRec->Event.MouseEvent.dwButtonState) & 0xFFFF0000) >> 16);
//    		char  nCount = nWheel / 120;
//    		wParam |= ((DWORD)(BYTE)nCount) << 24;
//		}
//
//
//    } else if (piRec->EventType == FOCUS_EVENT) {
//    	nMsg = piRec->Event.FocusEvent.bSetFocus ? WM_SETFOCUS : WM_KILLFOCUS;
//
//    } else {
//    	_ASSERT(FALSE);
//    	return FALSE;
//    }
//    _ASSERTE(nMsg!=0);
//
//
//    pMsg->message = nMsg;
//    pMsg->wParam = wParam;
//    pMsg->lParam = lParam;
//
//    return TRUE;
//}
//
//BOOL UnpackInputRecord(const MSG* piMsg, INPUT_RECORD* pRec)
//{
//	_ASSERTE(piMsg!=NULL && pRec!=NULL);
//	memset(pRec, 0, sizeof(INPUT_RECORD));
//
//	if (piMsg->message == 0)
//		return FALSE;
//
//	if (piMsg->message == WM_KEYDOWN || piMsg->message == WM_KEYUP) {
//		pRec->EventType = KEY_EVENT;
//
//		// lParam
//        pRec->Event.KeyEvent.bKeyDown = (piMsg->message == WM_KEYDOWN);
//        pRec->Event.KeyEvent.uChar.UnicodeChar = (WCHAR)(piMsg->lParam & 0xFFFF);
//        pRec->Event.KeyEvent.wVirtualKeyCode   = (((DWORD)piMsg->lParam) & 0xFF0000) >> 16;
//        pRec->Event.KeyEvent.wVirtualScanCode  = (((DWORD)piMsg->lParam) & 0xFF000000) >> 24;
//
//        // wParam. ���� ��� ��� ����� ���� max(ENHANCED_KEY==0x0100)
//        pRec->Event.KeyEvent.dwControlKeyState = ((DWORD)piMsg->wParam & 0xFFFF);
//
//        pRec->Event.KeyEvent.wRepeatCount = ((DWORD)piMsg->wParam & 0xFF0000) >> 16;
//
//	} else if (piMsg->message >= MOUSE_EVENT_FIRST && piMsg->message <= MOUSE_EVENT_LAST) {
//		pRec->EventType = MOUSE_EVENT;
//
//		switch (piMsg->message) {
//			case MOUSE_EVENT_MOVE:
//				pRec->Event.MouseEvent.dwEventFlags = MOUSE_MOVED;
//				break;
//			case MOUSE_EVENT_CLICK:
//				pRec->Event.MouseEvent.dwEventFlags = 0;
//				break;
//			case MOUSE_EVENT_DBLCLICK:
//				pRec->Event.MouseEvent.dwEventFlags = DOUBLE_CLICK;
//				break;
//			case MOUSE_EVENT_WHEELED:
//				pRec->Event.MouseEvent.dwEventFlags = MOUSE_WHEELED;
//				break;
//			case MOUSE_EVENT_HWHEELED:
//				pRec->Event.MouseEvent.dwEventFlags = /*MOUSE_HWHEELED*/ 0x0008;
//				break;
//		}
//
//		pRec->Event.MouseEvent.dwMousePosition.X = LOWORD(piMsg->lParam);
//		pRec->Event.MouseEvent.dwMousePosition.Y = HIWORD(piMsg->lParam);
//
//		// max 0x0010/*FROM_LEFT_4ND_BUTTON_PRESSED*/
//		pRec->Event.MouseEvent.dwButtonState = ((DWORD)piMsg->wParam) & 0xFF;
//
//		// max - ENHANCED_KEY == 0x0100
//		pRec->Event.MouseEvent.dwControlKeyState = (((DWORD)piMsg->wParam) & 0xFFFF00) >> 8;
//
//		if (piMsg->message == MOUSE_EVENT_WHEELED || piMsg->message == MOUSE_EVENT_HWHEELED) {
//    		// HIWORD() - short (direction[1/-1])*count*120
//    		signed char nDir = (signed char)((((DWORD)piMsg->wParam) & 0xFF000000) >> 24);
//    		WORD wDir = nDir*120;
//    		pRec->Event.MouseEvent.dwButtonState |= wDir << 16;
//		}
//
//	} else if (piMsg->message == WM_SETFOCUS || piMsg->message == WM_KILLFOCUS) {
//        pRec->EventType = FOCUS_EVENT;
//
//        pRec->Event.FocusEvent.bSetFocus = (piMsg->message == WM_SETFOCUS);
//
//	} else {
//		return FALSE;
//	}
//
//	return TRUE;
//}
//
//class MNullDesc {
//protected:
//	PSECURITY_DESCRIPTOR mp_NullDesc;
//	SECURITY_ATTRIBUTES  m_NullSecurity;
//public:
//	DWORD mn_LastError;
//public:
//	MNullDesc() {
//		memset(&m_NullSecurity, 0, sizeof(m_NullSecurity));
//		mp_NullDesc = NULL;
//		mn_LastError = 0;
//	};
//	~MNullDesc() {
//		memset(&m_NullSecurity, 0, sizeof(m_NullSecurity));
//		LocalFree(mp_NullDesc); mp_NullDesc = NULL;
//	};
//public:
//	SECURITY_ATTRIBUTES* NullSecurity() {
//		mn_LastError = 0;
//
//		if (mp_NullDesc) {
//			_ASSERTE(m_NullSecurity.lpSecurityDescriptor==mp_NullDesc);
//			return (&m_NullSecurity);
//		}
//		mp_NullDesc = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR,
//		      SECURITY_DESCRIPTOR_MIN_LENGTH);
//
//		if (mp_NullDesc == NULL) {
//			mn_LastError = GetLastError();
//			return NULL;
//		}
//
//		if (!InitializeSecurityDescriptor(mp_NullDesc, SECURITY_DESCRIPTOR_REVISION)) {
//			mn_LastError = GetLastError();
//			LocalFree(mp_NullDesc); mp_NullDesc = NULL;
//			return NULL;
//		}
//
//		// Add a null DACL to the security descriptor.
//		if (!SetSecurityDescriptorDacl(mp_NullDesc, TRUE, (PACL) NULL, FALSE)) {
//			mn_LastError = GetLastError();
//			LocalFree(mp_NullDesc); mp_NullDesc = NULL;
//			return NULL;
//		}
//
//		m_NullSecurity.nLength = sizeof(m_NullSecurity);
//		m_NullSecurity.lpSecurityDescriptor = mp_NullDesc;
//		m_NullSecurity.bInheritHandle = TRUE;
//
//		return (&m_NullSecurity);
//	};
//};
//MNullDesc *gNullDesc = NULL;
//
//SECURITY_ATTRIBUTES* NullSecurity()
//{
//	if (!gNullDesc) gNullDesc = new MNullDesc();
//	return gNullDesc->NullSecurity();
//}
//
//void CommonShutdown()
//{
//	if (gNullDesc) {
//		delete gNullDesc;
//		gNullDesc = NULL;
//	}
//
//	// Clean memory
//	if (ghConsoleSection) {
//		CloseHandle(ghConsoleSection);
//		ghConsoleSection = NULL;
//	}
//	if (gpConsoleInfoStr) {
//		free(gpConsoleInfoStr);
//		gpConsoleInfoStr = NULL;
//	}
//}
//
//BOOL IsUserAdmin()
//{
//	OSVERSIONINFO osv = {sizeof(OSVERSIONINFO)};
//	GetVersionEx(&osv);
//	// ��������� ����� ������ ��� �����, ����� �� XP ������ "���" �� �����������
//	if (osv.dwMajorVersion < 6)
//		return FALSE;
//
//	BOOL b;
//	SID_IDENTIFIER_AUTHORITY NtAuthority = {SECURITY_NT_AUTHORITY};
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
//


#ifndef CONEMU_MINIMAL
MConHandle::MConHandle(LPCWSTR asName)
{
	mn_StdMode = 0;
	mb_OpenFailed = FALSE; mn_LastError = 0;
	mh_Handle = INVALID_HANDLE_VALUE;
	lstrcpynW(ms_Name, asName, 9);
	/*
	FAR2 ���������
	Conemu ���������

	��� ������ ����� ����

	������� ���� ����������

	[HKEY_CURRENT_USER\Software\Far2\Associations\Type0]
	"Mask"="*.ini"
	"Description"=""
	"Execute"="@"
	"AltExec"=""
	"View"=""
	"AltView"=""
	"Edit"=""
	"AltEdit"=""
	"State"=dword:0000003f

	��� ������� �� �������� ������ �� INI �����. �� Enter - �� �������.


	1:31:11.647 Mouse: {10x15} Btns:{L} KeyState: 0x00000000 |DOUBLE_CLICK
	CECMD_CMDSTARTED(Cols=80, Rows=25, Buf=1000, Top=-1)
	SetConsoleSizeSrv.Not waiting for ApplyFinished
	SyncWindowToConsole(Cols=80, Rows=22)
	Current size: Cols=80, Buf=1000
	CECMD_CMDFINISHED(Cols=80, Rows=22, Buf=0, Top=-1)
	SetConsoleSizeSrv.Not waiting for ApplyFinished
	1:31:11.878 Mouse: {10x15} Btns:{} KeyState: 0x00000000
	Current size: Cols=80, Rows=22
	1:31:11.906 Mouse: {10x15} Btns:{} KeyState: 0x00000000 |MOUSE_MOVED
	1:31:11.949 Mouse: {10x15} Btns:{L} KeyState: 0x00000000 |DOUBLE_CLICK
	CECMD_CMDSTARTED(Cols=80, Rows=22, Buf=1000, Top=-1)
	SetConsoleSizeSrv.Not waiting for ApplyFinished
	Current size: Cols=80, Buf=1000
	CECMD_CMDFINISHED(Cols=80, Rows=22, Buf=0, Top=-1)
	SetConsoleSizeSrv.Not waiting for ApplyFinished
	1:31:12.163 Mouse: {10x15} Btns:{} KeyState: 0x00000000
	1:31:12.196 Mouse: {10x15} Btns:{} KeyState: 0x00000000 |MOUSE_MOVED
	Current size: Cols=80, Rows=22
	1:31:12.532 Mouse: {11x15} Btns:{} KeyState: 0x00000000 |MOUSE_MOVED
	1:31:12.545 Mouse: {13x15} Btns:{} KeyState: 0x00000000 |MOUSE_MOVED
	1:31:12.573 Mouse: {14x15} Btns:{} KeyState: 0x00000000 |MOUSE_MOVED
	1:31:12.686 Mouse: {15x15} Btns:{} KeyState: 0x00000000 |MOUSE_MOVED
	1:31:12.779 Mouse: {16x15} Btns:{} KeyState: 0x00000000 |MOUSE_MOVED
	1:31:12.859 Mouse: {16x15} Btns:{L} KeyState: 0x00000000
	1:31:12.876 Mouse: {16x15} Btns:{L} KeyState: 0x00000000 |MOUSE_MOVED
	1:31:12.944 Mouse: {16x15} Btns:{} KeyState: 0x00000000
	1:31:12.956 Mouse: {16x15} Btns:{} KeyState: 0x00000000 |MOUSE_MOVED
	1:31:13.010 Mouse: {16x15} Btns:{L} KeyState: 0x00000000 |DOUBLE_CLICK
	CECMD_CMDSTARTED(Cols=80, Rows=22, Buf=1000, Top=-1)
	SetConsoleSizeSrv.Not waiting for ApplyFinished
	SyncWindowToConsole(Cols=80, Rows=19)
	Current size: Cols=80, Buf=1000
	CECMD_CMDFINISHED(Cols=80, Rows=19, Buf=0, Top=-1)
	1:31:13.150 Mouse: {16x15} Btns:{} KeyState: 0x00000000 |MOUSE_MOVED
	1:31:13.175 Mouse: {16x15} Btns:{L} KeyState: 0x00000000
	1:31:13.206 Mouse: {16x15} Btns:{L} KeyState: 0x00000000 |MOUSE_MOVED
	SetConsoleSizeSrv.Not waiting for ApplyFinished
	1:31:13.240 Mouse: {16x15} Btns:{} KeyState: 0x00000000
	Current size: Cols=80, Rows=191:31:13.317 Mouse: {16x15} Btns:{} KeyState: 0x00000000 |MOUSE_MOVED
	1:31:13.357 Mouse: {16x15} Btns:{L} KeyState: 0x00000000 |DOUBLE_CLICK


	1:31:11 CurSize={80x25} ChangeTo={80x25} RefreshThread :CECMD_SETSIZESYNC
	1:31:11 CurSize={80x1000} ChangeTo={80x25} RefreshThread :CECMD_SETSIZESYNC
	1:31:11 CurSize={80x25} ChangeTo={80x25} RefreshThread :CECMD_SETSIZESYNC
	1:31:11 CurSize={80x1000} ChangeTo={80x22} RefreshThread :CECMD_SETSIZESYNC
	1:31:12 CurSize={80x22} ChangeTo={80x22} RefreshThread :CECMD_SETSIZESYNC
	1:31:12 CurSize={80x1000} ChangeTo={80x22} RefreshThread :CECMD_SETSIZESYNC
	1:31:13 CurSize={80x22} ChangeTo={80x22} RefreshThread :CECMD_SETSIZESYNC
	1:31:13 CurSize={80x1000} ChangeTo={80x19} RefreshThread :CECMD_SETSIZESYNC
	1:31:15 CurSize={80x19} ChangeTo={80x19} RefreshThread :CECMD_SETSIZESYNC
	1:31:15 CurSize={80x1000} ChangeTo={80x19} RefreshThread :CECMD_SETSIZESYNC
	1:31:16 CurSize={80x19} ChangeTo={80x19} RefreshThread :CECMD_SETSIZESYNC
	1:31:16 CurSize={80x1000} ChangeTo={80x19} RefreshThread :CECMD_SETSIZESYNC
	1:31:16 CurSize={80x19} ChangeTo={80x19} RefreshThread :CECMD_SETSIZESYNC
	1:31:16 CurSize={80x19} ChangeTo={80x19} RefreshThread :CECMD_SETSIZESYNC
	1:31:16 CurSize={80x1000} ChangeTo={80x16} RefreshThread :CECMD_SETSIZESYNC
	1:31:25 CurSize={80x16} ChangeTo={80x16} RefreshThread :CECMD_SETSIZESYNC
	1:31:25 CurSize={80x1000} ChangeTo={80x16} RefreshThread :CECMD_SETSIZESYNC




	*/
	//OSVERSIONINFO osv = {sizeof(OSVERSIONINFO)}; GetVersionEx(&osv);
	//if (osv.dwMajorVersion == 6 && osv.dwMinorVersion == 1) {
	//	if (!lstrcmpW(ms_Name, L"CONOUT$"))
	//		mn_StdMode = STD_OUTPUT_HANDLE;
	//	else if (!lstrcmpW(ms_Name, L"CONIN$"))
	//		mn_StdMode = STD_INPUT_HANDLE;
	//}
};

MConHandle::~MConHandle()
{
	Close();
};

MConHandle::operator const HANDLE()
{
	if (mh_Handle == INVALID_HANDLE_VALUE)
	{
		if (mn_StdMode)
		{
			mh_Handle = GetStdHandle(mn_StdMode);
			return mh_Handle;
		}

		// ����� �������� �� ������� ����� ��������� ��� � ������ �������
		MSectionLock CS; CS.Lock(&mcs_Handle, TRUE);

		// �� ����� �������� ����� ��� ��� ������ � ������ ������
		if (mh_Handle == INVALID_HANDLE_VALUE)
		{
			mh_Handle = CreateFileW(ms_Name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
			                        0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

			if (mh_Handle != INVALID_HANDLE_VALUE)
			{
				mb_OpenFailed = FALSE;
			}
			else
			{
				mn_LastError = GetLastError();

				if (!mb_OpenFailed)
				{
					mb_OpenFailed = TRUE; // ����� ������ ������������ ������ ���� ���!
					char szErrMsg[512], szNameA[10], szSelfFull[MAX_PATH];
					const char *pszSelf;
					char *pszDot;

					if (!GetModuleFileNameA(0,szSelfFull,MAX_PATH))
					{
						pszSelf = "???";
					}
					else
					{
						pszSelf = strrchr(szSelfFull, '\\');
						if (pszSelf) pszSelf++; else pszSelf = szSelfFull;

						pszDot = strrchr((char*)pszSelf, '.');

						if (pszDot) *pszDot = 0;
					}

					WideCharToMultiByte(CP_OEMCP, 0, ms_Name, -1, szNameA, sizeof(szNameA), 0,0);
					_wsprintfA(szErrMsg, SKIPLEN(countof(szErrMsg)) "%s: CreateFile(%s) failed, ErrCode=0x%08X\n", pszSelf, szNameA, mn_LastError);
					HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

					if (h && h!=INVALID_HANDLE_VALUE)
					{
						DWORD dwWritten = 0;
						WriteFile(h, szErrMsg, lstrlenA(szErrMsg), &dwWritten, 0);
					}
				}
			}
		}
	}

	return mh_Handle;
};

void MConHandle::Close()
{
	if (mh_Handle != INVALID_HANDLE_VALUE)
	{
		if (mn_StdMode)
		{
			mh_Handle = INVALID_HANDLE_VALUE;
		}
		else
		{
			HANDLE h = mh_Handle;
			mh_Handle = INVALID_HANDLE_VALUE;
			mb_OpenFailed = FALSE;
			CloseHandle(h);
		}
	}
};
#endif



#ifndef CONEMU_MINIMAL
MEvent::MEvent()
{
	ms_EventName[0] = 0;
	mh_Event = NULL;
	mn_LastError = 0;
}

MEvent::~MEvent()
{
	if (mh_Event)
		Close();
}

void MEvent::Close()
{
	mn_LastError = 0;
	ms_EventName[0] = 0;

	if (mh_Event)
	{
		CloseHandle(mh_Event);
		mh_Event = NULL;
	}
}

void MEvent::InitName(const wchar_t *aszTemplate, DWORD Parm1)
{
	if (mh_Event)
		Close();

	_wsprintf(ms_EventName, SKIPLEN(countof(ms_EventName)) aszTemplate, Parm1);
	mn_LastError = 0;
}

HANDLE MEvent::Open()
{
	if (mh_Event)  // ���� ��� ������� - ����� �������!
		return mh_Event;

	if (ms_EventName[0] == 0)
	{
		_ASSERTE(ms_EventName[0]!=0);
		return NULL;
	}

	mn_LastError = 0;
	mh_Event = OpenEvent(EVENT_MODIFY_STATE|SYNCHRONIZE, FALSE, ms_EventName);

	if (!mh_Event)
		mn_LastError = GetLastError();

	return mh_Event;
}

DWORD MEvent::Wait(DWORD dwMilliseconds, BOOL abAutoOpen/*=TRUE*/)
{
	if (!mh_Event && abAutoOpen)
		Open();

	if (!mh_Event)
		return WAIT_ABANDONED;

	DWORD dwWait = WaitForSingleObject(mh_Event, dwMilliseconds);
#ifdef _DEBUG
	mn_LastError = GetLastError();
#endif
	return dwWait;
}

HANDLE MEvent::GetHandle()
{
	return mh_Event;
}
#endif















#ifndef CONEMU_MINIMAL
MSetter::MSetter(BOOL* st) :
	mp_BoolVal(NULL), mdw_DwordVal(NULL)
{
	type = st_BOOL;
	mp_BoolVal = st;

	if (mp_BoolVal) *mp_BoolVal = TRUE;
}
MSetter::MSetter(DWORD* st, DWORD setValue) :
	mp_BoolVal(NULL), mdw_DwordVal(NULL)
{
	type = st_DWORD; mdw_OldDwordValue = *st; *st = setValue;
	mdw_DwordVal = st;
}
MSetter::~MSetter()
{
	Unlock();
}
void MSetter::Unlock()
{
	if (type==st_BOOL)
	{
		if (mp_BoolVal) *mp_BoolVal = FALSE;
	}
	else if (type==st_DWORD)
	{
		if (mdw_DwordVal) *mdw_DwordVal = mdw_OldDwordValue;
	}
}
#endif



MSection::MSection()
{
	mn_TID = 0; mn_Locked = 0; mb_Exclusive = FALSE;
#ifdef _DEBUG
	mn_UnlockedExclusiveTID = 0;
#endif
	ZeroStruct(mn_LockedTID); ZeroStruct(mn_LockedCount);
	InitializeCriticalSection(&m_cs);
	mh_ReleaseEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	_ASSERTE(mh_ReleaseEvent!=NULL);

	if (mh_ReleaseEvent) ResetEvent(mh_ReleaseEvent);
};
MSection::~MSection()
{
	DeleteCriticalSection(&m_cs);

	if (mh_ReleaseEvent)
	{
		CloseHandle(mh_ReleaseEvent); mh_ReleaseEvent = NULL;
	}
};
void MSection::ThreadTerminated(DWORD dwTID)
{
	for(int i=1; i<10; i++)
	{
		if (mn_LockedTID[i] == dwTID)
		{
			mn_LockedTID[i] = 0;

			if (mn_LockedCount[i] != 0)
			{
				_ASSERTE(mn_LockedCount[i] == 0);
			}

			break;
		}
	}
};
void MSection::AddRef(DWORD dwTID)
{
	mn_Locked ++; // ����������� ������� nonexclusive locks
	_ASSERTE(mn_Locked>0);
	ResetEvent(mh_ReleaseEvent); // �� ������ ������ ������� Event
	int j = -1; // ����� -2, ���� ++ �� ������������, ����� - +1 �� ������

	for(int i=1; i<10; i++)
	{
		if (mn_LockedTID[i] == dwTID)
		{
			mn_LockedCount[i] ++;
			j = -2;
			break;
		}
		else if (j == -1 && mn_LockedTID[i] == 0)
		{
			mn_LockedTID[i] = dwTID;
			mn_LockedCount[i] ++;
			j = i;
			break;
		}
	}

	if (j == -1)  // ����� ���� �� ������
	{
		_ASSERTE(j != -1);
	}
};
int MSection::ReleaseRef(DWORD dwTID)
{
	_ASSERTE(mn_Locked>0);
	int nInThreadLeft = 0;

	if (mn_Locked > 0) mn_Locked --;

	if (mn_Locked == 0)
		SetEvent(mh_ReleaseEvent); // ������ nonexclusive locks �� ��������

	for(int i=1; i<10; i++)
	{
		if (mn_LockedTID[i] == dwTID)
		{
			mn_LockedCount[i] --;

			if ((nInThreadLeft = mn_LockedCount[i]) == 0)
				mn_LockedTID[i] = 0; // ����� ��� ����������� ����������� ����� - 10 ����� � ������ ������������

			break;
		}
	}

	return nInThreadLeft;
};
void MSection::WaitUnlocked(DWORD dwTID, DWORD anTimeout)
{
	DWORD dwStartTick = GetTickCount();
	int nSelfCount = 0;

	for(int i=1; i<10; i++)
	{
		if (mn_LockedTID[i] == dwTID)
		{
			nSelfCount = mn_LockedCount[i];
			break;
		}
	}

	while(mn_Locked > nSelfCount)
	{
#ifdef _DEBUG
		DEBUGSTR(L"!!! Waiting for exclusive access\n");
		DWORD nWait =
#endif
		    WaitForSingleObject(mh_ReleaseEvent, 10);
		DWORD dwCurTick = GetTickCount();
		DWORD dwDelta = dwCurTick - dwStartTick;

		if (anTimeout != (DWORD)-1)
		{
			if (dwDelta > anTimeout)
			{
#ifndef CSECTION_NON_RAISE
				_ASSERTE(dwDelta<=anTimeout);
#endif
				break;
			}
		}

#ifdef _DEBUG
		else if (dwDelta > 3000)
		{
#ifndef CSECTION_NON_RAISE
			_ASSERTE(dwDelta <= 3000);
#endif
			break;
		}

#endif
	}
};
bool MSection::MyEnterCriticalSection(DWORD anTimeout)
{
	//EnterCriticalSection(&m_cs);
	// ��������� ���� ������ ��������
	// ����. �.�. ����� ���� ����� nTimeout (��� DC)
	DWORD dwTryLockSectionStart = GetTickCount(), dwCurrentTick;

	if (!TryEnterCriticalSection(&m_cs))
	{
		Sleep(10);

		while(!TryEnterCriticalSection(&m_cs))
		{
			Sleep(10);
			DEBUGSTR(L"TryEnterCriticalSection failed!!!\n");
			dwCurrentTick = GetTickCount();

			if (anTimeout != (DWORD)-1)
			{
				if (((dwCurrentTick - dwTryLockSectionStart) > anTimeout))
				{
#ifndef CSECTION_NON_RAISE
					_ASSERTE((dwCurrentTick - dwTryLockSectionStart) <= anTimeout);
#endif
					return false;
				}
			}

#ifdef _DEBUG
			else if ((dwCurrentTick - dwTryLockSectionStart) > 3000)
			{
#ifndef CSECTION_NON_RAISE
				_ASSERTE((dwCurrentTick - dwTryLockSectionStart) <= 3000);
#endif
				dwTryLockSectionStart = GetTickCount();
			}

#endif
		}
	}

	return true;
}
BOOL MSection::Lock(BOOL abExclusive, DWORD anTimeout/*=-1*/)
{
	DWORD dwTID = GetCurrentThreadId();

	// ����� ��� ���� ��� ��������� �������������?
	if (mb_Exclusive && dwTID == mn_TID)
	{
		return FALSE; // ���, �� Unlock ������ �� �����!
	}

	if (!abExclusive)
	{
		// ������� ����������, �� ����������� ������ ������ �����.
		// ��������� ������ ��������� (������������ ������ ��������)
		AddRef(dwTID);

		// ���� ������ ���� ��� ��������� exclusive
		if (mb_Exclusive)
		{
			int nLeft = ReleaseRef(dwTID); // ����� ����� ������� �� �������� ����������

			if (nLeft > 0)
			{
				// ����� �������� �����. ������ ���� �� ����� � ���� ����
				// ����� ������ ���� ��� �������� non exclusive lock
				_ASSERTE(nLeft == 0);
			}

			DEBUGSTR(L"!!! Failed non exclusive lock, trying to use CriticalSection\n");
			bool lbEntered = MyEnterCriticalSection(anTimeout); // ��������� ���� ������ ��������
			// mb_Exclusive ����� ���� ���������, ���� ������ ������ ���� �������� ��������� exclusive lock
			_ASSERTE(!mb_Exclusive); // ����� LeaveCriticalSection mb_Exclusive ��� ������ ���� �������
			AddRef(dwTID); // ���������� ����������

			// �� ��������� ��� ����� ������ nonexclusive lock
			if (lbEntered)
				LeaveCriticalSection(&m_cs);
		}
	}
	else
	{
		// ��������� Exclusive Lock
#ifdef _DEBUG
		if (mb_Exclusive)
		{
			// ����� ���� ��������� ��������
			DEBUGSTR(L"!!! Exclusive lock found in other thread\n");
		}

#endif
		// ���� ���� ExclusiveLock (� ������ ����) - �������� ���� EnterCriticalSection
#ifdef _DEBUG
		BOOL lbPrev = mb_Exclusive;
		DWORD nPrevTID = mn_TID;
#endif
		// ����� ��������� mb_Exclusive, ����� � ������ ����� �������� �� ������ nonexclusive lock
		// ����� ����� ����������, ��� nonexclusive lock ������ ��������� exclusive lock (���� ���� �����)
		mb_Exclusive = TRUE;
		TODO("Need to check, if MyEnterCriticalSection failed on timeout!\n");

		if (!MyEnterCriticalSection(anTimeout))
		{
			// ���� �������� _ASSERTE, ����� ����������, ��������� �� Timeout-� ��� ����������
			_ASSERTE(FALSE);

			if (mn_TID == 0)  // ��������� ������������� �� ������� - ������� ������
				mb_Exclusive = FALSE;

			return FALSE;
		}

		_ASSERTE(!(lbPrev && mb_Exclusive)); // ����� LeaveCriticalSection mb_Exclusive ��� ������ ���� �������
		mn_TID = dwTID; // � ��������, � ����� ���� ��� ���������
		mb_Exclusive = TRUE; // ���� ����� �������� ������ ����, ����������� Leave
		_ASSERTE(mn_LockedTID[0] == 0 && mn_LockedCount[0] == 0);
		mn_LockedTID[0] = dwTID;
		mn_LockedCount[0] ++;

		/*if (abRelockExclusive) {
			ReleaseRef(dwTID); // ���� �� ����� ��� nonexclusive lock
		}*/

		// B ���� ���� nonexclusive locks - ��������� �� ����������
		if (mn_Locked)
		{
			//WARNING: ��� ���� ���� ����������, ���� ������� ��� NonExclusive, � ����� � ���� �� ���� - Exclusive
			// � ����� ������� ����� �������� � ���������� abRelockExclusive
			WaitUnlocked(dwTID, anTimeout);
		}
	}

	return TRUE;
};
void MSection::Unlock(BOOL abExclusive)
{
	DWORD dwTID = GetCurrentThreadId();

	if (abExclusive)
	{
		_ASSERTE(mn_TID == dwTID && mb_Exclusive);
		_ASSERTE(mn_LockedTID[0] == dwTID);
#ifdef _DEBUG
		mn_UnlockedExclusiveTID = dwTID;
#endif
		mb_Exclusive = FALSE; mn_TID = 0;
		mn_LockedTID[0] = 0; mn_LockedCount[0] --;
		LeaveCriticalSection(&m_cs);
	}
	else
	{
		ReleaseRef(dwTID);
	}
};


#ifndef CONEMU_MINIMAL
MSectionThread::MSectionThread(MSection* apS)
{
	mp_S = apS; mn_TID = GetCurrentThreadId();
};
MSectionThread::~MSectionThread()
{
	if (mp_S && mn_TID)
		mp_S->ThreadTerminated(mn_TID);
};
#endif



MSectionLock::MSectionLock()
{
	mp_S = NULL; mb_Locked = FALSE; mb_Exclusive = FALSE;
};
MSectionLock::~MSectionLock()
{
	if (mb_Locked) Unlock();
};
BOOL MSectionLock::Lock(MSection* apS, BOOL abExclusive/*=FALSE*/, DWORD anTimeout/*=-1*/)
{
	if (!apS)
		return FALSE;
	if (mb_Locked && apS == mp_S && (abExclusive == mb_Exclusive || mb_Exclusive))
		return FALSE; // ��� ������������

	_ASSERTE(!mb_Locked);
	mb_Exclusive = abExclusive;
	mp_S = apS;
	mb_Locked = mp_S->Lock(mb_Exclusive, anTimeout);
	return mb_Locked;
};
BOOL MSectionLock::RelockExclusive(DWORD anTimeout/*=-1*/)
{
	if (!mp_S)
		return FALSE;
	if (mb_Locked && mb_Exclusive)
		return FALSE;  // ���

	// ������ ReLock ������ ������. ������ ������ ����, ������� ���� ��������� ReLock
	Unlock();
	mb_Exclusive = TRUE;
	mb_Locked = mp_S->Lock(mb_Exclusive, anTimeout);
	return mb_Locked;
};
void MSectionLock::Unlock()
{
	if (mp_S && mb_Locked)
	{
		mp_S->Unlock(mb_Exclusive);
		mb_Locked = FALSE;
	}
};
BOOL MSectionLock::isLocked()
{
	return (mp_S!=NULL) && mb_Locked;
};



#ifndef CONEMU_MINIMAL
MFileLog::MFileLog(LPCWSTR asName, LPCWSTR asDir /*= NULL*/, DWORD anPID /*= 0*/)
{
	mh_LogFile = NULL;

	if (!anPID) anPID = GetCurrentProcessId();

	wchar_t szTemp[MAX_PATH]; szTemp[0] = 0;
	GetTempPath(MAX_PATH-16, szTemp);

	if (!asDir || !*asDir)
	{
		wcscat_c(szTemp, L"ConEmuLog");
		CreateDirectoryW(szTemp, NULL);
		wcscat_c(szTemp, L"\\");
		asDir = szTemp;
	}

	int nDirLen = lstrlenW(asDir);
	wchar_t szFile[MAX_PATH*2];
	_wsprintf(szFile, SKIPLEN(countof(szFile)) L"%s-%u.log", asName ? asName : L"LogFile", anPID);
	int nFileLen = lstrlenW(szFile);
	int nCchMax = nDirLen+nFileLen+3;
	ms_FilePathName = (wchar_t*)calloc(nCchMax,2);
	_wcscpy_c(ms_FilePathName, nCchMax, asDir);

	if (nDirLen > 0 && ms_FilePathName[nDirLen-1] != L'\\')
		_wcscat_c(ms_FilePathName, nCchMax, L"\\");

	_wcscat_c(ms_FilePathName, nCchMax, szFile);
}
MFileLog::~MFileLog()
{
	if (mh_LogFile && mh_LogFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(mh_LogFile);
		mh_LogFile = NULL;
	}

	if (ms_FilePathName)
	{
		free(ms_FilePathName);
		ms_FilePathName = NULL;
	}
}
// Returns 0 if succeeded, otherwise - GetLastError() code
HRESULT MFileLog::CreateLogFile()
{
	if (!this)
		return -1;

	if (mh_LogFile && mh_LogFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(mh_LogFile);
	}

	if (!ms_FilePathName || !*ms_FilePathName)
		return -1;

	mh_LogFile = CreateFileW(ms_FilePathName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (mh_LogFile == INVALID_HANDLE_VALUE)
	{
		mh_LogFile = NULL;
		DWORD dwErr = GetLastError();
		return (dwErr ? dwErr : -1);
	}

	return 0; // OK
}
LPCWSTR MFileLog::GetLogFileName()
{
	if (!this)
		return L"<NULL>";

	return (ms_FilePathName ? ms_FilePathName : L"<NullFileName>");
}
void MFileLog::LogString(LPCSTR asText, BOOL abWriteTime /*= TRUE*/, LPCSTR asThreadName /*= NULL*/)
{
	if (!this)
		return;

	if (mh_LogFile == INVALID_HANDLE_VALUE || mh_LogFile == NULL)
		return;

	wchar_t szInfo[460]; szInfo[0] = 0;
	wchar_t szThread[32]; szThread[0] = 0;

	if (asText)
	{
		MultiByteToWideChar(CP_OEMCP, 0, asText, -1, szInfo, countof(szInfo));
		szInfo[countof(szInfo)-1] = 0;
	}

	if (asThreadName)
	{
		MultiByteToWideChar(CP_OEMCP, 0, asThreadName, -1, szThread, countof(szThread));
		szThread[countof(szThread)-1] = 0;
	}

	LogString(szInfo, abWriteTime, szThread);
}
void MFileLog::LogString(LPCWSTR asText, BOOL abWriteTime /*= TRUE*/, LPCWSTR asThreadName /*= NULL*/)
{
	if (!this)
		return;

	if (mh_LogFile == INVALID_HANDLE_VALUE || mh_LogFile == NULL)
		return;

	wchar_t szInfo[512]; szInfo[0] = 0;
	SYSTEMTIME st; GetLocalTime(&st);
	_wsprintf(szInfo, SKIPLEN(countof(szInfo)) L"%i:%02i:%02i.%03i ",
	          st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	int nCur = lstrlenW(szInfo);

	if (asThreadName && *asThreadName)
	{
		lstrcpynW(szInfo+nCur, asThreadName, 32); //-V112
		nCur += lstrlenW(szInfo+nCur);
	}

	if (asText && *asText)
	{
		lstrcpynW(szInfo+nCur, asText, 508-nCur);
		nCur += lstrlenW(szInfo+nCur);
	}

	wcscpy_add(nCur, szInfo, L"\r\n");
	nCur += 2;
	DWORD dwLen = 0;
	WriteFile(mh_LogFile, szInfo, nCur*2, &dwLen, 0);
	FlushFileBuffers(mh_LogFile);
}
#endif


#ifndef CONEMU_MINIMAL
/* *********************************** */
CToolTip::CToolTip()
{
	mh_Tip = mh_Ball = NULL;
	mpsz_LastTip = NULL;
	mn_LastTipCchMax = 0;
	memset(&mti_Tip,  0, sizeof(mti_Tip));
	memset(&mti_Ball, 0, sizeof(mti_Ball));
	mb_LastTipBalloon = -1;
}
CToolTip::~CToolTip()
{
	if (mh_Tip)
	{
		if (IsWindow(mh_Tip))
			DestroyWindow(mh_Tip);
		mh_Tip = NULL;
	}
	if (mh_Ball)
	{
		if (IsWindow(mh_Ball))
			DestroyWindow(mh_Ball);
		mh_Ball = NULL;
	}
	if (mpsz_LastTip)
	{
		free(mpsz_LastTip);
		mpsz_LastTip = NULL;
	}
}
void CToolTip::ShowTip(HWND ahOwner, HWND ahControl, LPCWSTR asText, BOOL abBalloon, POINT pt, HINSTANCE hInstance)
{
	HideTip();

	if (!asText || !*asText)
		return;

	
	int nTipLen = lstrlen(asText);
	if (!mpsz_LastTip || (nTipLen >= mn_LastTipCchMax))
	{
		if (mpsz_LastTip)
			free(mpsz_LastTip);
		mn_LastTipCchMax = nTipLen + 1;
		mpsz_LastTip = (wchar_t*)malloc(mn_LastTipCchMax*sizeof(wchar_t));
	}
	_wcscpy_c(mpsz_LastTip, mn_LastTipCchMax, asText);
	
	TOOLINFO *pti = abBalloon ? (&mti_Ball) : (&mti_Tip);
	
	if (abBalloon)
	{
		if (!mh_Ball || !IsWindow(mh_Ball))
		{
			mh_Ball = CreateWindowEx ( WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
			                           WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX,
			                           CW_USEDEFAULT, CW_USEDEFAULT,
			                           CW_USEDEFAULT, CW_USEDEFAULT,
			                           ahOwner, NULL,
			                           hInstance/*g_hInstance*/, NULL);
			SetWindowPos(mh_Ball, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
			// Set up tool information.
			// In this case, the "tool" is the entire parent window.
			pti->cbSize = 44; // ��� sizeof(TOOLINFO);
			pti->uFlags = TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE;
			pti->hwnd = ahControl;
			pti->hinst = hInstance;
			static wchar_t szAsterisk[] = L"*"; // eliminate GCC warning
			pti->lpszText = szAsterisk;
			pti->uId = (UINT_PTR)ahControl;
			GetClientRect(ahControl, &(pti->rect));
			// Associate the ToolTip with the tool window.
			SendMessage(mh_Ball, TTM_ADDTOOL, 0, (LPARAM)pti);
			// Allow multiline
			SendMessage(mh_Ball, TTM_SETMAXTIPWIDTH, 0, (LPARAM)300);
		}
	}
	else
	{
		if (!mh_Tip || !IsWindow(mh_Tip))
		{
			mh_Tip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
			                         WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX,
			                         CW_USEDEFAULT, CW_USEDEFAULT,
			                         CW_USEDEFAULT, CW_USEDEFAULT,
			                         ahOwner, NULL,
			                         hInstance, NULL);
			SetWindowPos(mh_Tip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
			SendMessage(mh_Tip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 30000);
		}
	}
	
	mb_LastTipBalloon = abBalloon;
	
	HWND hTip = abBalloon ? (mh_Ball) : (mh_Tip);
	if (!hTip)
	{
		_ASSERTE(hTip != NULL);
		return;
	}
	
	pti->lpszText = mpsz_LastTip;

	SendMessage(hTip, TTM_UPDATETIPTEXT, 0, (LPARAM)pti);
	//RECT rcControl; GetWindowRect(GetDlgItem(hDlg, nCtrlID), &rcControl);
	//int ptx = rcControl.right - 10;
	//int pty = (rcControl.top + rcControl.bottom) / 2;
	SendMessage(hTip, TTM_TRACKPOSITION, 0, MAKELONG(pt.x,pt.y));
	SendMessage(hTip, TTM_TRACKACTIVATE, TRUE, (LPARAM)pti);
	
	//SetTimer(hDlg, FAILED_FONT_TIMERID, nTimeout/*FAILED_FONT_TIMEOUT*/, 0);
}
void CToolTip::HideTip()
{
	HWND hTip = (mb_LastTipBalloon == 0) ? mh_Tip : mh_Ball;
	TOOLINFO *pti = (mb_LastTipBalloon == 0) ? (&mti_Tip) : (&mti_Ball);
	
	if (hTip)
		SendMessage(hTip, TTM_TRACKACTIVATE, FALSE, (LPARAM)pti);
}
#endif
