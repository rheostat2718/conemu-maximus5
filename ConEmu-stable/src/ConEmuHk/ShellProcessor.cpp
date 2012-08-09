
/*
Copyright (c) 2011 Maximus5
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
#include <TCHAR.h>
#include <Tlhelp32.h>
#include <shlwapi.h>
#include "../common/common.hpp"
#include "../common/ConEmuCheck.h"
#include "SetHook.h"
#include "../common/execute.h"
#include "ConEmuHooks.h"
#include "ShellProcessor.h"
#include "Injects.h"
#include "GuiAttach.h"
#include "../common/WinObjects.h"
#include "../common/RConStartArgs.h"
#include "UserImp.h"

#ifndef SEE_MASK_NOZONECHECKS
#define SEE_MASK_NOZONECHECKS 0x800000
#endif
#ifndef SEE_MASK_NOASYNC
#define SEE_MASK_NOASYNC 0x00000100
#endif


#ifdef _DEBUG
#ifndef CONEMU_MINIMAL
void TestShellProcessor()
{
	for (int i = 0; i < 10; i++)
	{
		MCHKHEAP;
		CShellProc* sp = new CShellProc;
		LPCWSTR pszFile = NULL, pszParam = NULL;
		DWORD nCreateFlags = 0, nShowCmd = 0;
		SHELLEXECUTEINFOW sei = {sizeof(SHELLEXECUTEINFOW)};
		STARTUPINFOW si = {sizeof(STARTUPINFOW)};
		switch (i)
		{
		case 0:
			pszFile = L"C:\\GCC\\mingw\\bin\\mingw32-make.exe";
			pszParam = L"mingw32-make \"1.cpp\" ";
			sp->OnCreateProcessW(&pszFile, &pszParam, &nCreateFlags, &si);
			break;
		case 1:
			pszFile = L"C:\\GCC\\mingw\\bin\\mingw32-make.exe";
			pszParam = L"\"mingw32-make.exe\" \"1.cpp\" ";
			sp->OnCreateProcessW(&pszFile, &pszParam, &nCreateFlags, &si);
			break;
		case 2:
			pszFile = L"C:\\GCC\\mingw\\bin\\mingw32-make.exe";
			pszParam = L"\"C:\\GCC\\mingw\\bin\\mingw32-make.exe\" \"1.cpp\" ";
			sp->OnCreateProcessW(&pszFile, &pszParam, &nCreateFlags, &si);
			break;
		case 3:
			pszFile = L"F:\\VCProject\\FarPlugin\\ConEmu\\Bugs\\DOS\\Prince\\PRINCE.EXE";
			pszParam = L"prince megahit";
			sp->OnCreateProcessW(&pszFile, &pszParam, &nCreateFlags, &si);
			break;
		case 4:
			pszFile = NULL;
			pszParam = L" \"F:\\VCProject\\FarPlugin\\ConEmu\\Bugs\\DOS\\Prince\\PRINCE.EXE\"";
			sp->OnCreateProcessW(&pszFile, &pszParam, &nCreateFlags, &si);
			break;
		case 5:
			pszFile = L"C:\\GCC\\mingw\\bin\\mingw32-make.exe";
			pszParam = L" \"1.cpp\" ";
			sp->OnShellExecuteW(NULL, &pszFile, &pszParam, &nCreateFlags, &nShowCmd);
			break;
		default:
			break;
		}
		MCHKHEAP;
		delete sp;
	}
}
#endif
#endif


//int CShellProc::mn_InShellExecuteEx = 0;
int gnInShellExecuteEx = 0;

extern struct HookModeFar gFarMode;
extern DWORD  gnHookMainThreadId;

CShellProc::CShellProc()
{
	mn_CP = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
	mpwsz_TempAction = mpwsz_TempFile = mpwsz_TempParam = NULL;
	mpsz_TempRetFile = mpsz_TempRetParam = mpsz_TempRetDir = NULL;
	mpwsz_TempRetFile = mpwsz_TempRetParam = mpwsz_TempRetDir = NULL;
	mlp_ExecInfoA = NULL; mlp_ExecInfoW = NULL;
	mlp_SaveExecInfoA = NULL; mlp_SaveExecInfoW = NULL;
	mb_WasSuspended = FALSE;
	mb_NeedInjects = FALSE;
	// int CShellProc::mn_InShellExecuteEx = 0; <-- static
	mb_InShellExecuteEx = FALSE;
	//mb_DosBoxAllowed = FALSE;
	m_SrvMapping.cbSize = 0;
}

CShellProc::~CShellProc()
{
	if (mb_InShellExecuteEx && gnInShellExecuteEx)
	{
		gnInShellExecuteEx--;
		mb_InShellExecuteEx = FALSE;
	}

	if (mpwsz_TempAction)
		free(mpwsz_TempAction);
	if (mpwsz_TempFile)
		free(mpwsz_TempFile);
	if (mpwsz_TempParam)
		free(mpwsz_TempParam);
	// results
	if (mpsz_TempRetFile)
		free(mpsz_TempRetFile);
	if (mpsz_TempRetParam)
		free(mpsz_TempRetParam);
	if (mpsz_TempRetDir)
		free(mpsz_TempRetDir);
	if (mpwsz_TempRetFile)
		free(mpwsz_TempRetFile);
	if (mpwsz_TempRetParam)
		free(mpwsz_TempRetParam);
	if (mpwsz_TempRetDir)
		free(mpwsz_TempRetDir);
	// structures
	if (mlp_ExecInfoA)
		free(mlp_ExecInfoA);
	if (mlp_ExecInfoW)
		free(mlp_ExecInfoW);
}

wchar_t* CShellProc::str2wcs(const char* psz, UINT anCP)
{
	if (!psz)
		return NULL;
	int nLen = lstrlenA(psz);
	wchar_t* pwsz = (wchar_t*)calloc((nLen+1),sizeof(wchar_t));
	if (nLen > 0)
	{
		MultiByteToWideChar(anCP, 0, psz, nLen+1, pwsz, nLen+1);
	}
	else
	{
		pwsz[0] = 0;
	}
	return pwsz;
}
char* CShellProc::wcs2str(const wchar_t* pwsz, UINT anCP)
{
	int nLen = lstrlen(pwsz);
	char* psz = (char*)calloc((nLen+1),sizeof(char));
	if (nLen > 0)
	{
		WideCharToMultiByte(anCP, 0, pwsz, nLen+1, psz, nLen+1, 0, 0);
	}
	else
	{
		psz[0] = 0;
	}
	return psz;
}

DWORD CShellProc::GetUseInjects()
{
	if (m_SrvMapping.cbSize)
		return m_SrvMapping.bUseInjects;
	return 0;
}

BOOL CShellProc::LoadGuiMapping()
{
	_ASSERTEX(user);
	if (!m_SrvMapping.cbSize || (m_SrvMapping.hConEmuWnd && !user->isWindow(m_SrvMapping.hConEmuWnd)))
	{
		if (!::LoadSrvMapping(ghConWnd, m_SrvMapping))
			return FALSE;
	}

	if (!m_SrvMapping.hConEmuWnd || !user->isWindow(m_SrvMapping.hConEmuWnd))
		return FALSE;

	return TRUE;

	//// ��������, � ����� ��?
	//DWORD dwGuiProcessId = 0;
	//if (!ghConEmuWnd || !GetWindowThreadProcessId(ghConEmuWnd, &dwGuiProcessId))
	//	return FALSE;

	//MFileMapping<ConEmuGuiMapping> GuiInfoMapping;
	//GuiInfoMapping.InitName(CEGUIINFOMAPNAME, dwGuiProcessId);
	//const ConEmuGuiMapping* pInfo = GuiInfoMapping.Open();
	//if (!pInfo)
	//	return FALSE;
	//else if (pInfo->nProtocolVersion != CESERVER_REQ_VER)
	//	return FALSE;
	//else
	//{
	//	memmove(&m_GuiMapping, pInfo, min(pInfo->cbSize, sizeof(m_GuiMapping)));
	//	/*bDosBoxAllowed = pInfo->bDosBox;
	//	wcscpy_c(szBaseDir, pInfo->sConEmuBaseDir);
	//	wcscat_c(szBaseDir, L"\\");
	//	if (pInfo->nLoggingType != glt_Processes)
	//		return NULL;*/
	//}
	//GuiInfoMapping.CloseMap();

	//return (m_GuiMapping.cbSize != 0);
}

CESERVER_REQ* CShellProc::NewCmdOnCreate(enum CmdOnCreateType aCmd,
				LPCWSTR asAction, LPCWSTR asFile, LPCWSTR asParam,
				DWORD* anShellFlags, DWORD* anCreateFlags, DWORD* anStartFlags, DWORD* anShowCmd,
				int mn_ImageBits, int mn_ImageSubsystem,
				HANDLE hStdIn, HANDLE hStdOut, HANDLE hStdErr
				/*wchar_t (&szBaseDir)[MAX_PATH+2], BOOL& bDosBoxAllowed*/)
{
	//szBaseDir[0] = 0;

	// ��������, � ���� ��?
	if (!LoadGuiMapping())
		return NULL;

	//	bDosBoxAllowed = pInfo->bDosBox;
	//	wcscpy_c(szBaseDir, pInfo->sConEmuBaseDir);
	//	wcscat_c(szBaseDir, L"\\");
	if (m_SrvMapping.nLoggingType != glt_Processes)
		return NULL;

	return ExecuteNewCmdOnCreate(&m_SrvMapping, ghConWnd, aCmd,
				asAction, asFile, asParam,
				anShellFlags, anCreateFlags, anStartFlags, anShowCmd,
				mn_ImageBits, mn_ImageSubsystem,
				hStdIn, hStdOut, hStdErr);

	////DWORD dwGuiProcessId = 0;
	////if (!ghConEmuWnd || !GetWindowThreadProcessId(ghConEmuWnd, &dwGuiProcessId))
	////	return NULL;
	//
	////MFileMapping<ConEmuGuiMapping> GuiInfoMapping;
	////GuiInfoMapping.InitName(CEGUIINFOMAPNAME, dwGuiProcessId);
	////const ConEmuGuiMapping* pInfo = GuiInfoMapping.Open();
	////if (!pInfo)
	////	return NULL;
	////else if (pInfo->nProtocolVersion != CESERVER_REQ_VER)
	////	return NULL;
	////else
	////{
	////	bDosBoxAllowed = pInfo->bDosBox;
	////	wcscpy_c(szBaseDir, pInfo->sConEmuBaseDir);
	////	wcscat_c(szBaseDir, L"\\");
	////	if (pInfo->nLoggingType != glt_Processes)
	////		return NULL;
	////}
	////GuiInfoMapping.CloseMap();
	//
	//
	//CESERVER_REQ *pIn = NULL;
	//
	//int nActionLen = (asAction ? lstrlen(asAction) : 0)+1;
	//int nFileLen = (asFile ? lstrlen(asFile) : 0)+1;
	//int nParamLen = (asParam ? lstrlen(asParam) : 0)+1;
	//
	//pIn = ExecuteNewCmd(CECMD_ONCREATEPROC, sizeof(CESERVER_REQ_HDR)
	//	+sizeof(CESERVER_REQ_ONCREATEPROCESS)+(nActionLen+nFileLen+nParamLen)*sizeof(wchar_t));
	//
	//#ifdef _WIN64
	//pIn->OnCreateProc.nSourceBits = 64;
	//#else
	//pIn->OnCreateProc.nSourceBits = 32;
	//#endif
	////pIn->OnCreateProc.bUnicode = TRUE;
	//pIn->OnCreateProc.nImageSubsystem = mn_ImageSubsystem;
	//pIn->OnCreateProc.nImageBits = mn_ImageBits;
	//pIn->OnCreateProc.hStdIn = (unsigned __int64)hStdIn;
	//pIn->OnCreateProc.hStdOut = (unsigned __int64)hStdOut;
	//pIn->OnCreateProc.hStdErr = (unsigned __int64)hStdErr;
	//
	//if (aCmd == eShellExecute)
	//	wcscpy_c(pIn->OnCreateProc.sFunction, L"Shell");
	//else if (aCmd == eCreateProcess)
	//	wcscpy_c(pIn->OnCreateProc.sFunction, L"Create");
	//else if (aCmd == eInjectingHooks)
	//	wcscpy_c(pIn->OnCreateProc.sFunction, L"Hooks");
	//else if (aCmd == eHooksLoaded)
	//	wcscpy_c(pIn->OnCreateProc.sFunction, L"Loaded");
	//else if (aCmd == eParmsChanged)
	//	wcscpy_c(pIn->OnCreateProc.sFunction, L"Changed");
	//else if (aCmd == eLoadLibrary)
	//	wcscpy_c(pIn->OnCreateProc.sFunction, L"LdLib");
	//else if (aCmd == eFreeLibrary)
	//	wcscpy_c(pIn->OnCreateProc.sFunction, L"FrLib");
	//else
	//	wcscpy_c(pIn->OnCreateProc.sFunction, L"Unknown");
	//
	//pIn->OnCreateProc.nShellFlags = anShellFlags ? *anShellFlags : 0;
	//pIn->OnCreateProc.nCreateFlags = anCreateFlags ? *anCreateFlags : 0;
	//pIn->OnCreateProc.nStartFlags = anStartFlags ? *anStartFlags : 0;
	//pIn->OnCreateProc.nShowCmd = anShowCmd ? *anShowCmd : 0;
	//pIn->OnCreateProc.nActionLen = nActionLen;
	//pIn->OnCreateProc.nFileLen = nFileLen;
	//pIn->OnCreateProc.nParamLen = nParamLen;
	//
	//wchar_t* psz = pIn->OnCreateProc.wsValue;
	//if (nActionLen > 1)
	//	_wcscpy_c(psz, nActionLen, asAction);
	//psz += nActionLen;
	//if (nFileLen > 1)
	//	_wcscpy_c(psz, nFileLen, asFile);
	//psz += nFileLen;
	//if (nParamLen > 1)
	//	_wcscpy_c(psz, nParamLen, asParam);
	//psz += nParamLen;
	//
	//return pIn;
}

BOOL CShellProc::ChangeExecuteParms(enum CmdOnCreateType aCmd, BOOL abNewConsole,
				LPCWSTR asFile, LPCWSTR asParam, /*LPCWSTR asBaseDir,*/
				LPCWSTR asExeFile, DWORD& ImageBits, DWORD& ImageSubsystem,
				LPWSTR* psFile, LPWSTR* psParam)
{
	if (!LoadGuiMapping())
		return FALSE;

	BOOL lbRc = FALSE;
	//size_t cchComspec = MAX_PATH+20;
	wchar_t *szComspec = NULL;
	size_t cchConEmuC = MAX_PATH+16;
	wchar_t *szConEmuC = NULL; // ConEmuC64.exe
	BOOL lbUseDosBox = FALSE;
	size_t cchDosBoxExe = MAX_PATH+16, cchDosBoxCfg = MAX_PATH+16;
	wchar_t *szDosBoxExe = NULL, *szDosBoxCfg = NULL;
	BOOL lbComSpec = FALSE; // TRUE - ���� %COMSPEC% �������������
	int nCchSize = 0;
	BOOL lbEndQuote = FALSE;
	bool lbNewGuiConsole = false, lbNewConsoleFromGui = false;
	BOOL lbComSpecK = FALSE; // TRUE - ���� ����� ��������� /K, � �� /C

	szConEmuC = (wchar_t*)malloc(cchConEmuC*sizeof(*szConEmuC)); // ConEmuC64.exe
	if (!szConEmuC)
	{
		_ASSERTE(szConEmuC!=NULL);
		goto wrap;
	}
	_wcscpy_c(szConEmuC, cchConEmuC, m_SrvMapping.sConEmuBaseDir);
	_wcscat_c(szConEmuC, cchConEmuC, L"\\");
	
	_ASSERTE(aCmd==eShellExecute || aCmd==eCreateProcess);


	if (aCmd == eCreateProcess)
	{
		if (asFile && !*asFile)
			asFile = NULL;

		// ���-�� ����� ���������� ��������� asFile
		wchar_t* pszFileUnquote = NULL;
		if (asFile && (*asFile == L'"'))
		{
			pszFileUnquote = lstrdup(asFile+1);
			asFile = pszFileUnquote;
			pszFileUnquote = wcschr(pszFileUnquote, L'"');
			if (pszFileUnquote)
				*pszFileUnquote = 0;
		}

		// ��� �������� - ����� ������� asFile ���� �� ��������� � ������ ���������� � asParam
		if (asFile && *asFile && asParam && *asParam)
		{
			LPCWSTR pszParam = SkipNonPrintable(asParam);
			if (pszParam && ((*pszParam != L'"') || (pszParam[0] == L'"' && pszParam[1] != L'"')))
			{
				//BOOL lbSkipEndQuote = FALSE;
				//if (*pszParam == L'"')
				//{
				//	pszParam++;
				//	lbSkipEndQuote = TRUE;
				//}
				int nLen = lstrlen(asFile)+1;
				int cchMax = max(nLen,(MAX_PATH+1));
				wchar_t* pszTest = (wchar_t*)malloc(cchMax*sizeof(wchar_t));
				_ASSERTE(pszTest);
				if (pszTest)
				{
					// ������� ������� �� ������ ������������
					if (asFile)
					{
						_wcscpyn_c(pszTest, nLen, (*pszParam == L'"') ? (pszParam+1) : pszParam, nLen); //-V501
						pszTest[nLen-1] = 0;
						// �������� asFile � ������ ���������� � asParam
						if (lstrcmpi(pszTest, asFile) == 0)
						{
							// exe-���� ��� ������ � asParam, ��������� ������������� �� �����
							asFile = NULL;
						}
					}
					// ���� �� ������� - � asParam ����� ���� ������ ��� ��� ����� ���� ������������ �����
					// �������� CreateProcess(L"C:\\GCC\\mingw32-make.exe", L"mingw32-make.exe makefile",...)
					// ���      CreateProcess(L"C:\\GCC\\mingw32-make.exe", L"mingw32-make makefile",...)
					// ���      CreateProcess(L"C:\\GCC\\mingw32-make.exe", L"\\GCC\\mingw32-make.exe makefile",...)
					// ��� ����� ����� �������� �� asParam
					if (asFile)
					{
						LPCWSTR pszFileOnly = PointToName(asFile);
						
						if (pszFileOnly)
						{
							LPCWSTR pszCopy = pszParam;
							wchar_t szFirst[MAX_PATH+1];
							if (NextArg(&pszCopy, szFirst) != 0)
							{
								_ASSERTE(FALSE && "NextArg failed?");
							}
							else
							{
								LPCWSTR pszFirstName = PointToName(szFirst);
								// �������� asFile � ������ ���������� � asParam
								if (lstrcmpi(pszFirstName, pszFileOnly) == 0)
								{
									// exe-���� ��� ������ � asParam, ��������� ������������� �� �����
									// -- asFile = NULL; -- ������� ������, ������ �� �������� ������ ����!
									asParam = pszCopy;
									pszFileOnly = NULL;
								}
								else
								{
									// ������� asFile ��� ����������
									wchar_t szTmpFileOnly[MAX_PATH+1]; szTmpFileOnly[0] = 0;
									_wcscpyn_c(szTmpFileOnly, countof(szTmpFileOnly), pszFileOnly, countof(szTmpFileOnly)); //-V501
									wchar_t* pszExt = wcsrchr(szTmpFileOnly, L'.');
									if (pszExt)
									{
										*pszExt = 0;
										if (lstrcmpi(pszFirstName, szTmpFileOnly) == 0)
										{
											// exe-���� ��� ������ � asParam, ��������� ������������� �� �����
											// -- asFile = NULL; -- ������� ������, ������ �� �������� ������ ����!
											asParam = pszCopy;
											pszFileOnly = NULL;
										}
										else if ((pszExt = wcsrchr(szFirst, L'.')) != NULL)
										{
											if (lstrcmpi(pszFirstName, szTmpFileOnly) == 0)
											{
												// exe-���� ��� ������ � asParam, ��������� ������������� �� �����
												// -- asFile = NULL; -- ������� ������, ������ �� �������� ������ ����!
												asParam = pszCopy;
												pszFileOnly = NULL;
											}
										}
									}
								}
							}
						}

						//// asFile ��� ��������, ��� ��� ��� ����� ����������� ������ ���� � asFile ��� ������ ����
						//if (pszFileOnly && (pszFileOnly != asFile))
						//{
						//	// ������� � ������������ ����
						//	nLen = lstrlen(pszFileOnly)+1;
						//	_wcscpyn_c(pszTest, nLen, pszParam, nLen); //-V501
						//	pszTest[nLen-1] = 0;
						//	// �������� asFile � ������ ���������� � asParam
						//	if (lstrcmpi(pszTest, pszFileOnly) == 0)
						//	{
						//		// exe-���� ��� ������ � asParam, ��������� ������������� �� �����
						//		// -- asFile = NULL; -- ������� ������, ������ �� �������� ������ ����!
						//		asParam = pszParam+nLen-(lbSkipEndQuote?0:1);
						//		pszFileOnly = NULL;
						//	}
						//}

						//// Last chance
						//if (pszFileOnly)
						//{
						//	// � ������, � ������������ ����������
						//	wchar_t szTmpFileOnly[MAX_PATH+1]; szTmpFileOnly[0] = 0;
						//	_wcscpyn_c(szTmpFileOnly, countof(szTmpFileOnly), pszFileOnly, countof(szTmpFileOnly)); //-V501
						//	wchar_t* pszExt = wcsrchr(szTmpFileOnly, L'.');
						//	if (pszExt)
						//	{
						//		*pszExt = 0;
						//		nLen = lstrlen(szTmpFileOnly);
						//		int nParmLen = lstrlen(pszParam);
						//		if ((nParmLen >= nLen) && ((pszParam[nLen] == 0) || wcschr(L" /\"", pszParam[nLen])))
						//		{
						//			_wcscpyn_c(pszTest, nLen+1, pszParam, nLen+1); //-V501
						//			pszTest[nLen] = 0;
						//			// �������� asFile � ������ ���������� � asParam
						//			if (lstrcmpi(pszTest, szTmpFileOnly) == 0)
						//			{
						//				// exe-���� ��� ������ � asParam, ��������� ������������� �� �����
						//				// -- asFile = NULL; -- ������� ������, ������ �� �������� ������ ����!
						//				asParam = pszParam+nLen-(lbSkipEndQuote?0:1);
						//				pszFileOnly =  NULL;
						//			}
						//		}
						//	}
						//}
					}

					free(pszTest);
				}
			}
		}

		SafeFree(pszFileUnquote);
	}

	//szComspec = (wchar_t*)calloc(cchComspec,sizeof(*szComspec));
	szComspec = GetComspec(&m_SrvMapping.ComSpec);
	if (!szComspec || !*szComspec)
	{
		_ASSERTE(szComspec && *szComspec);
		goto wrap;
	}

	//if (GetEnvironmentVariable(L"ComSpec", szComspec, (DWORD)cchComspec))
	//{
	//	// �� ������ ���� (���� ��������) ConEmuC.exe
	//	const wchar_t* pszName = PointToName(szComspec);
	//	if (!pszName || !lstrcmpi(pszName, L"ConEmuC.exe") || !lstrcmpi(pszName, L"ConEmuC64.exe"))
	//		szComspec[0] = 0;
	//}
	//// ���� �� ������� ���������� ����� ���������� ��������� - ������� ������� "cmd.exe" �� System32
	//if (szComspec[0] == 0)
	//{
	//	int n = GetWindowsDirectory(szComspec, MAX_PATH);
	//	if (n > 0 && n < MAX_PATH)
	//	{
	//		// �������� \System32\cmd.exe
	//		WARNING("TCC/ComSpec");
	//		wcscat_c(szComspec, (szComspec[n-1] == L'\\') ? L"System32\\cmd.exe" : L"\\System32\\cmd.exe");
	//	}
	//}

	// ���� ������� ���������� "ComSpec"
	if (asParam)
	{
		BOOL lbNewCmdCheck = NULL;
		const wchar_t* psz = SkipNonPrintable(asParam);
		// ���� ��������� cmd.exe ��� ���������� - �� ����������� ���!
		if (psz && *psz && szComspec[0])
		{
			// asFile ����� ���� � ��� ShellExecute � ��� CreateProcess, �������� � ����� �����
			if (asFile && (lstrcmpi(szComspec, asFile) == 0))
			{
				if (psz[0] == L'/' && wcschr(L"CcKk", psz[1]))
				{
					if (abNewConsole)
					{
						// 111211 - "-new_console" ���������� � GUI
						_ASSERTEX(psz[1] == L'C' || psz[1] == L'c');
						lbComSpecK = FALSE;
					}
					else
					{
						// �� ��������� � ���������� ������� asFile (��� ������������� cmd.exe)
						lbComSpecK = (psz[1] == L'K' || psz[1] == L'k');
						asFile = NULL;
						asParam = SkipNonPrintable(psz+2); // /C ��� /K ����������� � ConEmuC.exe
						lbNewCmdCheck = TRUE;
					}

					//BOOL lbRootIsCmdExe = FALSE, lbAlwaysConfirmExit = FALSE, lbAutoDisableConfirmExit = FALSE;
					//BOOL lbNeedCutStartEndQuot = FALSE;
					//DWORD nFileAttrs = (DWORD)-1;
					//ms_ExeTmp[0] = 0;
					//IsNeedCmd(asParam, &lbNeedCutStartEndQuot, ms_ExeTmp, lbRootIsCmdExe, lbAlwaysConfirmExit, lbAutoDisableConfirmExit);
					//// ��� ����� ���� ������� ���.����������!
					//// �������, ��������, ������ � ��������� �������� ����� ������ ���
					//// ������ � ��������� �����������.
					//// cmd.exe /c echo -> �� ������
					//// cmd.exe /c echo.exe -> ����� � ��������
					//if (ms_ExeTmp[0] && (wcschr(ms_ExeTmp, L'.') || wcschr(ms_ExeTmp, L'\\')))
					//{
					//	DWORD nCheckSybsystem = 0, nCheckBits = 0;
					//	if (FindImageSubsystem(ms_ExeTmp, nCheckSybsystem, nCheckBits, nFileAttrs))
					//	{
					//		ImageSubsystem = nCheckSybsystem;
					//		ImageBits = nCheckBits;
					//	}
					//}

					lbComSpec = TRUE;
				}
			}
			else if ((aCmd == eCreateProcess) && !asFile)
			{
				// ������ ��������� ��� CreateProcess.
				// asFile ��� ������� � NULL, ���� �� ��������� � ������ ���������� � asParam
				// �������� ��� �������� asParam (��� �������)
				// "c:\windows\system32\cmd.exe" /c dir
				// "c:\windows\system32\cmd.exe /c dir"
				// ������ - ���� �������������, ��� �������������
				INT_PTR nLen = lstrlen(szComspec)+1;
				wchar_t* pszTest = (wchar_t*)malloc(nLen*sizeof(wchar_t));
				_ASSERTE(pszTest);
				if (pszTest)
				{
					_wcscpyn_c(pszTest, nLen, (*psz == L'"') ? (psz+1) : psz, nLen); //-V501
					pszTest[nLen-1] = 0;
					// �������� ������ �������� � asParam � %COMSPEC%
					const wchar_t* pszCmdLeft = NULL;
					if (lstrcmpi(pszTest, szComspec) == 0)
					{
						if (*psz == L'"')
						{
							if (psz[nLen] == L'"')
							{
								pszCmdLeft = psz+nLen+1;
							}
						}
						else
						{
							pszCmdLeft = psz+nLen-1;
						}

						// ������ ����� ���������, ��� ��� � ������ ������� (���� ������� ��� - �������� cmd.exe)
						pszCmdLeft = SkipNonPrintable(pszCmdLeft);
						if (pszCmdLeft && pszCmdLeft[0] == L'/' && wcschr(L"CcKk", pszCmdLeft[1]))
						{
							// �� ��������� � ���������� ������� asFile (��� ������������� cmd.exe)
							lbComSpecK = (psz[1] == L'K' || psz[1] == L'k');
							_ASSERTE(asFile == NULL); // ��� ������ ���� NULL
							asParam = pszCmdLeft+2; // /C ��� /K ����������� � ConEmuC.exe
							lbNewCmdCheck = TRUE;

							lbComSpec = TRUE;
						}
					}
					free(pszTest);
				}
			}
		}

		if (lbNewCmdCheck)
		{
			BOOL lbRootIsCmdExe = FALSE, lbAlwaysConfirmExit = FALSE, lbAutoDisableConfirmExit = FALSE;
			BOOL lbNeedCutStartEndQuot = FALSE;
			DWORD nFileAttrs = (DWORD)-1;
			ms_ExeTmp[0] = 0;
			IsNeedCmd(asParam, NULL, &lbNeedCutStartEndQuot, ms_ExeTmp, lbRootIsCmdExe, lbAlwaysConfirmExit, lbAutoDisableConfirmExit);
			// ��� ����� ���� ������� ���.����������!
			// �������, ��������, ������ � ��������� �������� ����� ������ ���
			// ������ � ��������� �����������.
			// cmd.exe /c echo -> �� ������
			// cmd.exe /c echo.exe -> ����� � ��������
			if (ms_ExeTmp[0] && (wcschr(ms_ExeTmp, L'.') || wcschr(ms_ExeTmp, L'\\')))
			{
				bool bSkip = false;
				LPCWSTR pszExeName = PointToName(ms_ExeTmp);
				// �� ��������, ����� � ������ ����� ���������,
				// �� ���� ���-�� ������� ������ cmd.exe - ����
				if (lstrcmpi(pszExeName, L"cmd.exe") == 0)
				{
					ImageSubsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
					switch (m_SrvMapping.ComSpec.csBits)
					{
					case csb_SameOS:
						ImageBits = IsWindows64() ? 64 : 32;
						break;
					case csb_x32:
						ImageBits = 32;
						break;
					default:
					//case csb_SameApp:
						ImageBits = WIN3264TEST(32,64);
						break;
					}
					bSkip = true;
				}
				
				if (!bSkip)
				{
					DWORD nCheckSybsystem = 0, nCheckBits = 0;
					if (FindImageSubsystem(ms_ExeTmp, nCheckSybsystem, nCheckBits, nFileAttrs))
					{
						ImageSubsystem = nCheckSybsystem;
						ImageBits = nCheckBits;
					}
				}
			}
		}
	}

	//wchar_t szNewExe[MAX_PATH+1];
	//const wchar_t* pszExeExt = NULL;
	//// ���������, ����� ����������� ������?
	//if (/*lbComSpec &&*/ !asFile && asParam)
	//{
	//	const wchar_t* pszTemp = asParam;
	//	if (0 == NextArg(&pszTemp, szNewExe))
	//	{
	//		pszExeExt = PointToExt(szNewExe);
	//	}
	//}
	// ���� �� ����� ������ ������, ����� �������� "��� � ����"
	//if (pszExeExt && (!lstrcmpi(pszExeExt, L".cmd") || !lstrcmpi(pszExeExt, L".bat")))
	//{
	//	ImageSubsystem = IMAGE_SUBSYSTEM_BATCH_FILE;
	//	// ����� ��������� "�������" � �������� ��������
	//	#ifdef _WIN64
	//	ImageBits = 64;
	//	#else
	//	ImageBits = 32;
	//	#endif
	//}
	
//#ifdef _DEBUG
//	// ���� ����������� ������ - ����������� "��������" ��������?
//	if (ImageSubsystem == IMAGE_SUBSYSTEM_BATCH_FILE)
//	{
//		#ifdef _WIN64
//		ImageBits = 64;
//		#else
//		ImageBits = 32;
//		#endif
//	}
//#endif



	lbUseDosBox = FALSE;
	szDosBoxExe = (wchar_t*)calloc(cchDosBoxExe, sizeof(*szDosBoxExe));
	szDosBoxCfg = (wchar_t*)calloc(cchDosBoxCfg, sizeof(*szDosBoxCfg));
	if (!szDosBoxExe || !szDosBoxCfg)
	{
		_ASSERTE(szDosBoxExe && szDosBoxCfg);
		goto wrap;
	}

	if (ImageBits == 32) //-V112
	{
		wcscat_c(szConEmuC, L"ConEmuC.exe");
	}
	else if (ImageBits == 64)
	{
		wcscat_c(szConEmuC, L"ConEmuC64.exe");
	}
	else if (ImageBits == 16)
	{
		if (m_SrvMapping.cbSize && m_SrvMapping.bDosBox)
		{
			wcscpy_c(szDosBoxExe, m_SrvMapping.sConEmuBaseDir);
			wcscat_c(szDosBoxExe, L"\\DosBox\\DosBox.exe");
			wcscpy_c(szDosBoxCfg, m_SrvMapping.sConEmuBaseDir);
			wcscat_c(szDosBoxCfg, L"\\DosBox\\DosBox.conf");

			if (!FileExists(szDosBoxExe) || !FileExists(szDosBoxCfg))
			{
				// DoxBox �� ����������!
				lbRc = FALSE;
				//return FALSE;
				goto wrap;
			}

			wcscat_c(szConEmuC, L"ConEmuC.exe");
			lbUseDosBox = TRUE;
		}
		else
		{
			// � ����� ���� ����� ��������� ����� ConEmuC.exe, ����� GUI ��� ����� �����, ����� 16��� ���������� ����������
			// �� ��� � 64������ OS - ntvdm �����������, ��� ��� (���� DosBox-� ���), ��������� �� �����
			if (IsWindows64())
			{
				lbRc = FALSE;
				//return FALSE;
				goto wrap;
			}
			else
			{
				wcscat_c(szConEmuC, L"ConEmuC.exe");
			}
		}
	}
	else
	{
		// ���� �� ������ ���������� ��� ��� � ��� ����������� - ����� �� �������
		_ASSERTE(ImageBits==16||ImageBits==32||ImageBits==64);
		//wcscat_c(szConEmuC, L"ConEmuC.exe");
		lbRc = FALSE;
		//return FALSE;
		goto wrap;
	}
	
	nCchSize = (asFile ? lstrlen(asFile) : 0) + (asParam ? lstrlen(asParam) : 0) + 64;
	if (lbUseDosBox)
	{
		// ����� ���� ����� ������������� ������� ��� ��� ����, ������������� �����
		// �� � ���� ��������� ��� DosBox
		nCchSize = (nCchSize*2) + lstrlen(szDosBoxExe) + lstrlen(szDosBoxCfg) + 128 + MAX_PATH*2/*�� cd � ������ �����*/;
	}

	if (!FileExists(szConEmuC))
	{
		wchar_t szInfo[MAX_PATH+128], szTitle[64];
		wcscpy_c(szInfo, L"ConEmu executable not found!\n");
		wcscat_c(szInfo, szConEmuC);
		msprintf(szTitle, countof(szTitle), L"ConEmuHk, PID=%i", GetCurrentProcessId());
		GuiMessageBox(ghConEmuWnd, szInfo, szTitle, MB_ICONSTOP);
	}

	if (aCmd == eShellExecute)
	{
		*psFile = lstrdup(szConEmuC);
	}
	else
	{
		nCchSize += lstrlen(szConEmuC)+1;
		*psFile = NULL;
	}

	// ���� ����������� ����� GUI ��� �������, ��� ���������� ���������� �� GUI ��� �������
	lbNewGuiConsole = (ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI) || (ghAttachGuiClient != NULL);
	lbNewConsoleFromGui = (ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI) && (ghAttachGuiClient != NULL);

	#if 0
	if (lbNewGuiConsole)
	{
		// ����� ��� �������� /ATTACH /GID=%i,  � �.�.
		nCchSize += 128;
	}
	#endif
	if (lbNewConsoleFromGui)
	{
		// ����� ��� �������� /ATTACH /GID=%i,  � �.�.
		nCchSize += 128;
	}

	if (gFarMode.cbSize && gFarMode.bFarHookMode)
	{
		// �������� /PARENTFARPID=%u
		nCchSize += 32;
	}
	
	// � ShellExecute ���������� "ConEmuC.exe" ������� � psFile, � ��� CreatePocess - � psParam
	// /C ��� /K � ����� ������� ����� ������ � psParam
	lbEndQuote = FALSE;
	*psParam = (wchar_t*)malloc(nCchSize*sizeof(wchar_t));
	(*psParam)[0] = 0;
	if (aCmd == eCreateProcess)
	{
		(*psParam)[0] = L'"';
		_wcscpy_c((*psParam)+1, nCchSize-1, szConEmuC);
		_wcscat_c((*psParam), nCchSize, L"\"");
	}


	if (aCmd == eShellExecute)
	{
		// C:\Windows\system32\cmd.exe /C ""F:\Batches\!!Save&SetNewCFG.cmd" "
		lbEndQuote = (asFile && *asFile); // ����� ��������� ����� �������������� �����������
	}
	else if (aCmd == eCreateProcess)
	{
		// as_Param: "C:\test.cmd" "c:\my documents\test.txt"
		// ���� ��� �� ��������� - cmd.exe ������� ������ � ���������, � ����������
		lbEndQuote = (asFile && *asFile == L'"') || (!asFile && asParam && *asParam == L'"');
	}

	if (lbUseDosBox)
		_wcscat_c((*psParam), nCchSize, L" /DOSBOX");

	if (gFarMode.cbSize && gFarMode.bFarHookMode)
	{
		// �������� /PARENTFAR=%u
		wchar_t szParentFar[64];
		msprintf(szParentFar, countof(szParentFar), L" /PARENTFARPID=%u", GetCurrentProcessId());
		_wcscat_c((*psParam), nCchSize, szParentFar);
	}

	// 111211 - "-new_console" ���������� � GUI
	if (lbNewConsoleFromGui)
	{
		// ����� ��� �������� /ATTACH /GID=%i,  � �.�.
		int nCurLen = lstrlen(*psParam);
		msprintf((*psParam) + nCurLen, nCchSize - nCurLen, L" /ATTACH /GID=%u /GHWND=%08X /ROOT ",
			m_SrvMapping.nGuiPID, (DWORD)m_SrvMapping.hConEmuRoot);
		TODO("��������, ������ �� ���������� /K|/C? ���� ���������� ����������� �� GUI");
	}
	else
	{
		_wcscat_c((*psParam), nCchSize, lbComSpecK ? L" /K " : L" /C ");
	}
	if (asParam && *asParam == L' ')
		asParam++;

	WARNING("###: ��������� ��������� ���������� DosBox � ConEmuC!");
	if (lbUseDosBox)
	{
		lbEndQuote = TRUE;
		_wcscat_c((*psParam), nCchSize, L"\"\"");
		_wcscat_c((*psParam), nCchSize, szDosBoxExe);
		_wcscat_c((*psParam), nCchSize, L"\" -noconsole ");
		_wcscat_c((*psParam), nCchSize, L" -conf \"");
		_wcscat_c((*psParam), nCchSize, szDosBoxCfg);
		_wcscat_c((*psParam), nCchSize, L"\" ");
		_wcscat_c((*psParam), nCchSize, L" -c \"");
		//_wcscat_c((*psParam), nCchSize, L" \"");
		// ����������� ���� (���� ����, ����� ���� ������ � asParam)
		if (asFile && *asFile)
		{
			LPCWSTR pszRunFile = asFile;
			wchar_t* pszShort = GetShortFileNameEx(asFile);
			if (pszShort)
				pszRunFile = pszShort;
			LPCWSTR pszSlash = wcsrchr(pszRunFile, L'\\');
			if (pszSlash)
			{
				if (pszRunFile[1] == L':')
				{
					_wcscatn_c((*psParam), nCchSize, pszRunFile, 2);
					_wcscat_c((*psParam), nCchSize, L"\" -c \"");
				}
				_wcscat_c((*psParam), nCchSize, L"cd ");
				_wcscatn_c((*psParam), nCchSize, pszRunFile, (pszSlash-pszRunFile));
				_wcscat_c((*psParam), nCchSize, L"\" -c \"");
			}
			_wcscat_c((*psParam), nCchSize, pszRunFile);

			if (pszShort)
				free(pszShort);

			if (asParam && *asParam)
				_wcscat_c((*psParam), nCchSize, L" ");
		}
		// ���������, ������� ����� ������������!
		if (asParam && *asParam)
		{
			LPWSTR pszParam = NULL;
			if (!asFile || !*asFile)
			{
				// exe-����� � asFile ������� �� ����, ������ �� � asParam, ����� ��� ��������, � ������������ ������� DosBox
				BOOL lbRootIsCmdExe = FALSE, lbAlwaysConfirmExit = FALSE, lbAutoDisableConfirmExit = FALSE;
				BOOL lbNeedCutStartEndQuot = FALSE;
				ms_ExeTmp[0] = 0;
				IsNeedCmd(asParam, NULL, &lbNeedCutStartEndQuot, ms_ExeTmp, lbRootIsCmdExe, lbAlwaysConfirmExit, lbAutoDisableConfirmExit);

				if (ms_ExeTmp[0])
				{
					LPCWSTR pszQuot = SkipNonPrintable(asParam);
					if (lbNeedCutStartEndQuot)
					{
						while (*pszQuot == L'"') pszQuot++;
						pszQuot += lstrlen(ms_ExeTmp);
						if (*pszQuot == L'"') pszQuot++;
					}
					else
					{
						pszQuot += lstrlen(ms_ExeTmp);
					}
					if (pszQuot && *pszQuot)
					{
						pszParam = lstrdup(pszQuot);
						INT_PTR nLen = lstrlen(pszParam);
						if (pszParam[nLen-1] == L'"')
							pszParam[nLen-1] = 0;
					}
					asParam = pszParam;

					LPCWSTR pszRunFile = ms_ExeTmp;
					wchar_t* pszShort = GetShortFileNameEx(ms_ExeTmp);
					if (pszShort)
						pszRunFile = pszShort;
					LPCWSTR pszSlash = wcsrchr(pszRunFile, L'\\');
					if (pszSlash)
					{
						if (pszRunFile[1] == L':')
						{
							_wcscatn_c((*psParam), nCchSize, pszRunFile, 2);
							_wcscat_c((*psParam), nCchSize, L"\" -c \"");
						}
						_wcscat_c((*psParam), nCchSize, L"cd ");
						_wcscatn_c((*psParam), nCchSize, pszRunFile, (pszSlash-pszRunFile));
						_wcscat_c((*psParam), nCchSize, L"\" -c \"");
					}
					_wcscat_c((*psParam), nCchSize, pszRunFile);

					if (pszShort)
						free(pszShort);

					if (asParam && *asParam)
						_wcscat_c((*psParam), nCchSize, L" ");
				}
			}

			wchar_t* pszDst = (*psParam)+lstrlen((*psParam));
			const wchar_t* pszSrc = SkipNonPrintable(asParam);
			while (pszSrc && *pszSrc)
			{
				if (*pszSrc == L'"')
					*(pszDst++) = L'\\';
				*(pszDst++) = *(pszSrc++);
			}
			*pszDst = 0;

			if (pszParam)
				free(pszParam);
		}

		_wcscat_c((*psParam), nCchSize, L"\" -c \"exit\" ");

		//_wcscat_c((*psParam), nCchSize, L" -noconsole ");
		//_wcscat_c((*psParam), nCchSize, L" -conf \"");
		//_wcscat_c((*psParam), nCchSize, szDosBoxCfg);
		//_wcscat_c((*psParam), nCchSize, L"\" ");

	}
	else //NOT lbUseDosBox
	{
		if (lbEndQuote)
			_wcscat_c((*psParam), nCchSize, L"\"");

		if (asFile && *asFile)
		{
			if (*asFile != L'"')
				_wcscat_c((*psParam), nCchSize, L"\"");
			_wcscat_c((*psParam), nCchSize, asFile);
			if (*asFile != L'"')
				_wcscat_c((*psParam), nCchSize, L"\"");

			if (asParam && *asParam)
				_wcscat_c((*psParam), nCchSize, L" ");
		}

		if (asParam && *asParam)
		{
			// 111211 - "-new_console" ���������� � GUI
			#if 0
			const wchar_t* sNewConsole = L"-new_console";
			int nNewConsoleLen = lstrlen(sNewConsole);
			const wchar_t* pszNewPtr = wcsstr(asParam, sNewConsole);
			if (pszNewPtr)
			{
				if (!(((pszNewPtr == asParam) || (*(pszNewPtr-1) == L' ')) && ((pszNewPtr[nNewConsoleLen] == 0) || (pszNewPtr[nNewConsoleLen] == L' '))))
					pszNewPtr = NULL;
			}

			if ((lbNewGuiConsole) && pszNewPtr)
			{
				// �������� "-new_console" �� ����������
				int nCurLen = lstrlen((*psParam));
				wchar_t* pszDst = (*psParam)+nCurLen;
				INT_PTR nCchLeft = nCchSize - nCurLen;
				if (pszNewPtr > asParam)
				{
					_wcscpyn_c(pszDst, nCchLeft, asParam, (pszNewPtr-asParam)+1);
					pszDst += pszNewPtr-asParam;
					*pszDst = 0;
					nCchLeft -= pszNewPtr-asParam;
				}
				_wcscpy_c(pszDst, nCchLeft, pszNewPtr+lstrlen(sNewConsole));
			}
			else
			#endif
			{
				_wcscat_c((*psParam), nCchSize, asParam);
			}
		}
	}

	if (lbEndQuote)
		_wcscat_c((*psParam), nCchSize, L" \"");


#ifdef _DEBUG
	{
		int cchLen = (*psFile ? lstrlen(*psFile) : 0) + (*psParam ? lstrlen(*psParam) : 0) + 128;
		wchar_t* pszDbgMsg = (wchar_t*)calloc(cchLen, sizeof(wchar_t));
		if (pszDbgMsg)
		{
			msprintf(pszDbgMsg, cchLen, L"RunChanged(ParentPID=%u): %s <%s> <%s>\n",
				GetCurrentProcessId(),
				(aCmd == eShellExecute) ? L"Shell" : (aCmd == eCreateProcess) ? L"Create" : L"???",
				*psFile ? *psFile : L"", *psParam ? *psParam : L"");
			OutputDebugString(pszDbgMsg);
			free(pszDbgMsg);
		}
	}
#endif
		
	//if (aCmd == eShellExecute)
	//{
	//	if (asFile && *asFile)
	//	{
	//		lbEndQuote = TRUE;
	//		if (lbUseDosBox)
	//			_wcscat_c((*psParam), nCchSize, L" /DOSBOX");
	//		_wcscat_c((*psParam), nCchSize, lbComSpecK ? L" /K " : L" /C ");
	//		_wcscat_c((*psParam), nCchSize, L"\"\"");
	//		_wcscat_c((*psParam), nCchSize, asFile ? asFile : L"");
	//		_wcscat_c((*psParam), nCchSize, L"\"");
	//	}
	//	else
	//	{
	//		if (lbUseDosBox)
	//			_wcscat_c((*psParam), nCchSize, L" /DOSBOX");
	//		_wcscat_c((*psParam), nCchSize, lbComSpecK ? L" /K " : L" /C ");
	//	}
	//}
	//else
	//{
	//	//(*psParam)[0] = L'"';
	//	//_wcscpy_c((*psParam)+1, nCchSize-1, szConEmuC);
	//	//_wcscat_c((*psParam), nCchSize, L"\"");

	//	if (lbUseDosBox)
	//		_wcscat_c((*psParam), nCchSize, L" /DOSBOX");
	//	_wcscat_c((*psParam), nCchSize, lbComSpecK ? L" /K " : L" /C ");
	//	// ��� CreateProcess. ����������� ���� ����� ���� ��� ������ � asParam
	//	if (asFile && *asFile)
	//	{
	//		_wcscat_c((*psParam), nCchSize, L"\"");
	//		_ASSERTE(asFile!=NULL);
	//		_wcscat_c((*psParam), nCchSize, asFile);
	//		_wcscat_c((*psParam), nCchSize, L"\"");
	//	}
	//}
	//if (asParam && *asParam)
	//{
	//	_wcscat_c((*psParam), nCchSize, L" ");
	//	_wcscat_c((*psParam), nCchSize, asParam);
	//}
	//if (lbEndQuote)
	//	_wcscat_c((*psParam), nCchSize, L" \"");

	lbRc = TRUE;
wrap:
	if (szComspec)
		free(szComspec);
	if (szConEmuC)
		free(szConEmuC);
	if (szDosBoxExe)
		free(szDosBoxExe);
	if (szDosBoxCfg)
		free(szDosBoxCfg);
	return TRUE;
}

BOOL CShellProc::PrepareExecuteParms(
			enum CmdOnCreateType aCmd,
			LPCWSTR asAction, LPCWSTR asFile, LPCWSTR asParam,
			DWORD* anShellFlags, DWORD* anCreateFlags, DWORD* anStartFlags, DWORD* anShowCmd,
			HANDLE* lphStdIn, HANDLE* lphStdOut, HANDLE* lphStdErr,
			LPWSTR* psFile, LPWSTR* psParam, LPWSTR* psStartDir)
{
	// !!! anFlags ����� ���� NULL;
	// !!! asAction ����� ���� NULL;
	_ASSERTE(*psFile==NULL && *psParam==NULL);
	if (!ghConEmuWndDC)
		return FALSE; // ������������� ������ ��� ConEmu

	// ��� ����������� - �������� �����
	HANDLE hIn  = lphStdIn  ? *lphStdIn  : NULL;
	HANDLE hOut = lphStdOut ? *lphStdOut : NULL;
	HANDLE hErr = lphStdErr ? *lphStdErr : NULL;
	BOOL bLongConsoleOutput = gFarMode.bFarHookMode && gFarMode.bLongConsoleOutput;
	
	bool bNewConsoleArg = false, bForceNewConsole = false, bCurConsoleArg = false;
	// Service object
	RConStartArgs args;
		
	// Issue 351: ����� �������� ����������� ���� �� ShellExecuteEx ������-�� ���� ���� ���������
	//            ����� ����� (hStdOutput = 0x00010001), ������ ���������� 0x00060265
	//            � ������������������� ���� 0x400 � lpStartupInfo->dwFlags
	if ((aCmd == eCreateProcess) && (gnInShellExecuteEx > 0)
		&& lphStdOut && lphStdErr && anStartFlags && (*anStartFlags) == 0x401)
	{
		OSVERSIONINFO osv = {sizeof(OSVERSIONINFO)};

		if (GetVersionEx(&osv))
		{
			// ������� Win2k, Minor ����� �� ���������
			if (osv.dwMajorVersion == 5) // && (osv.dwMinorVersion == 1/*WinXP*/ || osv.dwMinorVersion == 2/*Win2k3*/))
			{
				if (//(*lphStdOut == (HANDLE)0x00010001)
					(((DWORD_PTR)*lphStdOut) >= 0x00010000)
					&& (*lphStdErr == NULL))
				{
					*lphStdOut = NULL;
					*anStartFlags &= ~0x400;
				}
			}
		}
	}
	
	// ��������� ��������� ConEmuGuiMapping.bUseInjects
	if (!LoadGuiMapping() || !(m_SrvMapping.bUseInjects & 1))
	{
		// -- �������� ������ �� ������ "Use Injects", ����� ������ ��������� ����������� "ConEmuC.exe" �� ConEmu --
		// ��������� � ConEmu ConEmuGuiMapping.bUseInjects, ��� gFarMode.bFarHookMode. ����� - ����� �������
		if (!(gFarMode.bFarHookMode && gFarMode.bLongConsoleOutput))
		{
			return FALSE;
		}
	}

		
	//wchar_t *szTest = (wchar_t*)malloc(MAX_PATH*2*sizeof(wchar_t)); //[MAX_PATH*2]
	//wchar_t *szExe = (wchar_t*)malloc((MAX_PATH+1)*sizeof(wchar_t)); //[MAX_PATH+1];
	DWORD /*mn_ImageSubsystem = 0, mn_ImageBits = 0,*/ nFileAttrs = (DWORD)-1;
	bool lbGuiApp = false;
	//int nActionLen = (asAction ? lstrlen(asAction) : 0)+1;
	//int nFileLen = (asFile ? lstrlen(asFile) : 0)+1;
	//int nParamLen = (asParam ? lstrlen(asParam) : 0)+1;
	BOOL lbNeedCutStartEndQuot = FALSE;

	mn_ImageSubsystem = mn_ImageBits = 0;
	
	if (/*(aCmd == eShellExecute) &&*/ asFile && *asFile)
	{
		if (*asFile == L'"')
		{
			LPCWSTR pszEnd = wcschr(asFile+1, L'"');
			if (pszEnd)
			{
				size_t cchLen = (pszEnd - asFile) - 1;
				_wcscpyn_c(ms_ExeTmp, countof(ms_ExeTmp), asFile+1, cchLen);
			}
			else
			{
				_wcscpyn_c(ms_ExeTmp, countof(ms_ExeTmp), asFile+1, countof(ms_ExeTmp));
			}
		}
		else
		{
			_wcscpyn_c(ms_ExeTmp, countof(ms_ExeTmp), asFile, countof(ms_ExeTmp));
		}
	}
	else
	{
		BOOL lbRootIsCmdExe = FALSE, lbAlwaysConfirmExit = FALSE, lbAutoDisableConfirmExit = FALSE;
		IsNeedCmd(asParam, NULL, &lbNeedCutStartEndQuot, ms_ExeTmp, lbRootIsCmdExe, lbAlwaysConfirmExit, lbAutoDisableConfirmExit);
	}
	
	if (ms_ExeTmp[0])
	{
		//wchar_t *pszNamePart = NULL;
		int nLen = lstrlen(ms_ExeTmp);
		// ����� ������ 0 � �� ������������� ������
		BOOL lbMayBeFile = (nLen > 0) && (ms_ExeTmp[nLen-1] != L'\\') && (ms_ExeTmp[nLen-1] != L'/');
		//const wchar_t* pszExt = PointToExt(ms_ExeTmp);

		BOOL lbSubsystemOk = FALSE;
		mn_ImageBits = 0;
		mn_ImageSubsystem = IMAGE_SUBSYSTEM_UNKNOWN;

		if (!lbMayBeFile)
		{
			mn_ImageBits = 0;
			mn_ImageSubsystem = IMAGE_SUBSYSTEM_UNKNOWN;
		}
		//else if (pszExt && (!lstrcmpi(pszExt, L".cmd") || !lstrcmpi(pszExt, L".bat")))
		//{
		//	lbGuiApp = FALSE;
		//	mn_ImageSubsystem = IMAGE_SUBSYSTEM_BATCH_FILE;
		//	mn_ImageBits = IsWindows64() ? 64 : 32;
		//	lbSubsystemOk = TRUE;
		//}
		else if (FindImageSubsystem(ms_ExeTmp, mn_ImageSubsystem, mn_ImageBits, nFileAttrs))
		{
			lbGuiApp = (mn_ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI);
			lbSubsystemOk = TRUE;
		}
		//else if (GetFullPathName(ms_ExeTmp, countof(szTest), szTest, &pszNamePart)
		//	&& GetImageSubsystem(szTest, mn_ImageSubsystem, mn_ImageBits, nFileAttrs))
		//{
		//	lbGuiApp = (mn_ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI);
		//	lbSubsystemOk = TRUE;
		//}
		//else if (SearchPath(NULL, ms_ExeTmp, NULL, countof(szTest), szTest, &pszNamePart)
		//	&& GetImageSubsystem(szTest, mn_ImageSubsystem, mn_ImageBits, nFileAttrs))
		//{
		//	lbGuiApp = (mn_ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI);
		//	lbSubsystemOk = TRUE;
		//}
		//else
		//{
		//	szTest[0] = 0;
		//}
		

		//if (!lbSubsystemOk)
		//{
		//	mn_ImageBits = 0;
		//	mn_ImageSubsystem = IMAGE_SUBSYSTEM_UNKNOWN;

		//	if ((nFileAttrs != (DWORD)-1) && !(nFileAttrs & FILE_ATTRIBUTE_DIRECTORY))
		//	{
		//		LPCWSTR pszExt = wcsrchr(szTest, L'.');
		//		if (pszExt)
		//		{
		//			if ((lstrcmpiW(pszExt, L".cmd") == 0) || (lstrcmpiW(pszExt, L".bat") == 0))
		//			{
		//				#ifdef _WIN64
		//				mn_ImageBits = 64;
		//				#else
		//				mn_ImageBits = IsWindows64() ? 64 : 32;
		//				#endif
		//				mn_ImageSubsystem = IMAGE_SUBSYSTEM_BATCH_FILE;
		//			}
		//		}
		//	}
		//}
	}
	
	BOOL lbChanged = FALSE;
	mb_NeedInjects = FALSE;
	//wchar_t szBaseDir[MAX_PATH+2]; szBaseDir[0] = 0;
	CESERVER_REQ *pIn = NULL;
	pIn = NewCmdOnCreate(aCmd, 
			asAction, asFile, asParam, 
			anShellFlags, anCreateFlags, anStartFlags, anShowCmd, 
			mn_ImageBits, mn_ImageSubsystem, 
			hIn, hOut, hErr/*, szBaseDir, mb_DosBoxAllowed*/);
	if (pIn)
	{
		HWND hConWnd = GetConsoleWindow();
		CESERVER_REQ *pOut = NULL;
		pOut = ExecuteGuiCmd(hConWnd, pIn, hConWnd);
		ExecuteFreeResult(pIn); pIn = NULL;
		if (!pOut)
			goto wrap;
		if (!pOut->OnCreateProcRet.bContinue)
			goto wrap;
		ExecuteFreeResult(pOut);
	}

#ifdef _DEBUG
	{
		int cchLen = (asFile ? lstrlen(asFile) : 0) + (asParam ? lstrlen(asParam) : 0) + 128;
		wchar_t* pszDbgMsg = (wchar_t*)calloc(cchLen, sizeof(wchar_t));
		if (pszDbgMsg)
		{
			msprintf(pszDbgMsg, cchLen, L"Run(ParentPID=%u): %s <%s> <%s>\n",
				GetCurrentProcessId(),
				(aCmd == eShellExecute) ? L"Shell" : (aCmd == eCreateProcess) ? L"Create" : L"???",
				asFile ? asFile : L"", asParam ? asParam : L"");
			OutputDebugString(pszDbgMsg);
			free(pszDbgMsg);
		}
	}
#endif

	//wchar_t* pszExecFile = (wchar_t*)pOut->OnCreateProcRet.wsValue;
	//wchar_t* pszBaseDir = (wchar_t*)(pOut->OnCreateProcRet.wsValue); // + pOut->OnCreateProcRet.nFileLen);
	
	if (asParam)
	{
		const wchar_t* sNewConsole = L"-new_console";
		const wchar_t* sCurConsole = L"-cur_console";
		int nNewConsoleLen = lstrlen(sNewConsole);
		const wchar_t* pszFind = wcsstr(asParam, sNewConsole);
		// 111211 - ����� "-new_console:" ������ ����������� ���������
		if (pszFind && ((pszFind == asParam) || (*(pszFind-1) == L' ') || (*(pszFind-1) == L'"'))
			&& ((pszFind[nNewConsoleLen] == 0) || (pszFind[nNewConsoleLen] == L' ') || (pszFind[nNewConsoleLen] == L':') || (pszFind[nNewConsoleLen] == L'"')))
		{
			bNewConsoleArg = true;
		}
		else if (wcsstr(asParam, sCurConsole) != NULL)
		{
			// � ��� "-cur_console" ����� ������������ _�����_
			args.pszSpecialCmd = lstrdup(asParam);
			if (args.ProcessNewConArg() > 0)
			{
				bCurConsoleArg = true;
				if (args.bForceDosBox && m_SrvMapping.cbSize && m_SrvMapping.bDosBox)
				{
					mn_ImageSubsystem = IMAGE_SUBSYSTEM_DOS_EXECUTABLE;
					mn_ImageSubsystem = 16;
					bLongConsoleOutput = FALSE;
					lbGuiApp = FALSE;
				}
			}
		}
	}
	// ���� GUI ���������� �������� �� ������� ConEmu - ��������� ���������� ���������� � ����� ������� ConEmu
	if (!bNewConsoleArg 
		&& ghAttachGuiClient && (mn_ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI)
		&& ((anShowCmd == NULL) || (*anShowCmd != SW_HIDE)))
	{
		WARNING("�� ������, ��� ������� �� ������� ����� ������ ���� ������� ����������� �������");
		bForceNewConsole = true;
	}
	if (ghAttachGuiClient && (bNewConsoleArg || bForceNewConsole) && !lbGuiApp)
	{
		lbGuiApp = true;
	}
	
	if (aCmd == eShellExecute)
	{
		WARNING("�������� ������� ��� ������ ShellExecute!");
		// !!! anFlags ����� ���� NULL;
		DWORD nFlagsMask = (SEE_MASK_FLAG_NO_UI|SEE_MASK_NOASYNC|SEE_MASK_NOCLOSEPROCESS|SEE_MASK_NO_CONSOLE);
		DWORD nFlags = (anShellFlags ? *anShellFlags : 0) & nFlagsMask;
		// ���� bNewConsoleArg - �� ���������� ���������� � ����� ������� ConEmu (GUI ������ ���� ������� �����)
		if (bNewConsoleArg || bForceNewConsole)
		{
			if (anShellFlags)
			{
				// 111211 - "-new_console" ����������� � GUI
				// ����� ��������������� �� ConEmuC, � ��� ����� ��������� � ���� �������
				//--WARNING("����, �������� ����� �� ���, ����� �� �� ����� � �������, ��� ���� ����� �� �����, �� ������ ��� �������� ���");
				//--*anShellFlags = (SEE_MASK_FLAG_NO_UI|SEE_MASK_NOASYNC|SEE_MASK_NOCLOSEPROCESS);
				if (anShowCmd && !(*anShellFlags & SEE_MASK_NO_CONSOLE))
				{
					*anShowCmd = SW_HIDE;
				}
			}
			else
			{
				_ASSERTE(anShellFlags!=NULL);
			}
		}
		else if (nFlags != nFlagsMask)
		{
			goto wrap; // ���� ��� - ��� ��� ��������� ���������� �������
		}
		if (asAction && (lstrcmpiW(asAction, L"open") != 0))
		{
			goto wrap; // runas, print, � ������ ��� �� ����������
		}
	}
	else
	{
		// ����������, ����� ��� ������� ����� ������� ��� CreateProcess?
		DWORD nFlags = anCreateFlags ? *anCreateFlags : 0;
		if ((nFlags & (CREATE_NO_WINDOW|DETACHED_PROCESS)) != 0)
			goto wrap; // ����������� �� ������ (��� ����������� ����), ����������

		// ��� ��� ����� ���� ConEmuC.exe
		const wchar_t* pszExeName = PointToName(ms_ExeTmp);
		if (pszExeName && (!lstrcmpi(pszExeName, L"ConEmuC.exe") || !lstrcmpi(pszExeName, L"ConEmuC64.exe")))
		{
			mb_NeedInjects = FALSE;
			goto wrap;
		}
	}
	
	//bool lbGuiApp = false;
	//DWORD ImageSubsystem = 0, ImageBits = 0;

	//if (!pszExecFile || !*pszExecFile)
	if (ms_ExeTmp[0] == 0)
	{
		_ASSERTE(ms_ExeTmp[0] != 0);
		goto wrap; // ������?
	}
	//if (GetImageSubsystem(pszExecFile,ImageSubsystem,ImageBits))
	//lbGuiApp = (ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI);

	if (lbGuiApp && !(bNewConsoleArg || bForceNewConsole))
		goto wrap; // ��� - �� ������������� (���� ������ �� ������ "-new_console")

	// ����������� ConEmuC.exe ����� ������ ��� ����, ����� 
	//	1. � ���� �������� ������� ����� � ��������� ������� ��������� � ������� (�� � ConsoleAlias ����������)
	//	2. ��� ������� ShellExecute/ShellExecuteEx, �.�. �� ����,
	//     ��� ���� ShellExecute ������� CreateProcess �� kernel32 (������� ����������).
	//     � Win7 ��� ����� ���� ����� ������ ��������� ������� (App-.....dll)

	#ifdef _DEBUG
	// ��� �������������� ������� ConEmuC.exe - ��������� true. ������ ��� �������!
	bool lbAlwaysAddConEmuC = false;
	#endif

	if ((mn_ImageBits == 0) && (mn_ImageSubsystem == 0)
		#ifdef _DEBUG
		&& !lbAlwaysAddConEmuC
		#endif
		)
	{
		// ��� ����� ���� ����������� ��������, �������� .doc, ��� .sln ����
		goto wrap;
	}

	if (bLongConsoleOutput)
	{
		// MultiArc issue. ��� ������ ����� �������� ������� �����. ��� ������?
		// ���� �� ������� �� �� �������� ������.
		if (GetCurrentThreadId() != gnHookMainThreadId)
			bLongConsoleOutput = FALSE;
	}

	_ASSERTE(mn_ImageBits!=0);
	// ���� ��� ��� - ���������� ��������� ConEmuC.exe
	// -- bFarHookMode ������� �� bLongConsoleOutput --
	if ((bLongConsoleOutput)
		|| (lbGuiApp && (bNewConsoleArg || bForceNewConsole)) // ����� GUI ��������� � ����� ������� � ConEmu, ��� ����� ������� �� GUI
		// eCreateProcess ������������� �� ����� (���� ������� InjectHooks ����� CreateProcess)
		|| ((mn_ImageBits != 16) && (m_SrvMapping.bUseInjects & 1) 
			&& (bNewConsoleArg || (aCmd == eShellExecute) 
				#ifdef _DEBUG
				|| lbAlwaysAddConEmuC
				#endif
				))
		// ���� ��� ���-���������� - �� ���� ������� DosBox, ��������� ConEmuC.exe /DOSBOX
		|| ((mn_ImageBits == 16) && (mn_ImageSubsystem == IMAGE_SUBSYSTEM_DOS_EXECUTABLE)
		    && m_SrvMapping.cbSize && m_SrvMapping.bDosBox))
	{
		lbChanged = ChangeExecuteParms(aCmd, bNewConsoleArg, asFile, asParam, /*szBaseDir, */
						ms_ExeTmp, mn_ImageBits, mn_ImageSubsystem, psFile, psParam);
		if (!lbChanged)
		{
			// ���� ������ ������� � 16������ ���������� - ����� �����, ntvdm.exe ������������!
			mb_NeedInjects = (mn_ImageBits != 16);
		}
		else
		{
			// ������ �� "ConEmuC.exe ...", ��� "-new_console" / "-cur_console" ����� ���������� � ���
			bCurConsoleArg = false;

			HWND hConWnd = GetConsoleWindow();

			if (lbGuiApp && (bNewConsoleArg || bForceNewConsole))
			{
				if (anShowCmd)
					*anShowCmd = SW_HIDE;

				if (anShellFlags && hConWnd)
				{
					*anShellFlags |= SEE_MASK_NO_CONSOLE;
				}

				#if 0
				// ����� ����������� ��� ������� �������!
				if (aCmd == eCreateProcess)
				{
					if (anCreateFlags)
					{
						*anCreateFlags |= CREATE_NEW_CONSOLE;
						*anCreateFlags &= ~(DETACHED_PROCESS|CREATE_NO_WINDOW);
					}
				}
				else if (aCmd == eShellExecute)
				{
					if (anShellFlags)
						*anShellFlags |= SEE_MASK_NO_CONSOLE;
				}
				#endif
			}
			pIn = NewCmdOnCreate(eParmsChanged, 
					asAction, *psFile, *psParam, 
					anShellFlags, anCreateFlags, anStartFlags, anShowCmd, 
					mn_ImageBits, mn_ImageSubsystem, 
					hIn, hOut, hErr/*, szBaseDir, mb_DosBoxAllowed*/);
			if (pIn)
			{
				CESERVER_REQ *pOut = NULL;
				pOut = ExecuteGuiCmd(hConWnd, pIn, hConWnd);
				ExecuteFreeResult(pIn); pIn = NULL;
				if (pOut)
					ExecuteFreeResult(pOut);
			}
		}
	}
	else
	{
		//lbChanged = ChangeExecuteParms(aCmd, asFile, asParam, pszBaseDir, 
		//				ms_ExeTmp, mn_ImageBits, mn_ImageSubsystem, psFile, psParam);
		// ���� ������ ������� � 16������ ���������� - ����� �����, ntvdm.exe ������������!
		mb_NeedInjects = (aCmd == eCreateProcess) && (mn_ImageBits != 16);
	}

wrap:
	if (bCurConsoleArg)
	{
		if (lbChanged)
		{
			_ASSERTE(lbChanged==FALSE && "bCurConsoleArg must be reset?");
		}
		else
		{
			// ���� �� ���� ����������
			if (args.pszStartupDir)
			{
				*psStartDir = args.pszStartupDir;
				args.pszStartupDir = NULL;
			}
			*psParam = args.pszSpecialCmd;
			args.pszSpecialCmd = NULL;
			// ������ ������!
			if (args.bBufHeight && gnServerPID)
			{
				//CESERVER_REQ *pIn = ;
				//ExecutePrepareCmd(&In, CECMD_SETSIZESYNC, sizeof(CESERVER_REQ_HDR));
				//CESERVER_REQ* pOut = ExecuteSrvCmd(nServerPID/*gdwServerPID*/, (CESERVER_REQ*)&In, hConWnd);
				HANDLE hConOut = GetStdHandle(STD_OUTPUT_HANDLE);
				CONSOLE_SCREEN_BUFFER_INFO csbi = {};
				if (GetConsoleScreenBufferInfo(hConOut, &csbi))
				{
					bool bNeedChange = false;
					BOOL bBufChanged = FALSE;
					if (args.nBufHeight)
					{
						WARNING("������ �� �� ������� ������� ��� ���������");
						//SHORT nNewHeight = max((csbi.srWindow.Bottom - csbi.srWindow.Top + 1),(SHORT)args.nBufHeight);
						//if (nNewHeight != csbi.dwSize.Y)
						//{
						//	csbi.dwSize.Y = nNewHeight;
						//	bNeedChange = true;
						//}
					}
					else if (csbi.dwSize.Y > (csbi.srWindow.Bottom - csbi.srWindow.Top + 1))
					{
						bNeedChange = true;
						csbi.dwSize.Y = (csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
					}

					if (bNeedChange)
					{
						bBufChanged = SetConsoleScreenBufferSize(hConOut, csbi.dwSize);
					}
					UNREFERENCED_PARAMETER(bBufChanged);
				}
			}
		}
	}
	return lbChanged;
}

BOOL CShellProc::OnShellExecuteA(LPCSTR* asAction, LPCSTR* asFile, LPCSTR* asParam, LPCSTR* asDir, DWORD* anFlags, DWORD* anShowCmd)
{
	if (!ghConEmuWndDC || !isWindow(ghConEmuWndDC))
		return FALSE; // ������������� ������ ��� ConEmu
		
	mb_InShellExecuteEx = TRUE;
	gnInShellExecuteEx ++;

	mpwsz_TempAction = str2wcs(asAction ? *asAction : NULL, mn_CP);
	mpwsz_TempFile = str2wcs(asFile ? *asFile : NULL, mn_CP);
	mpwsz_TempParam = str2wcs(asParam ? *asParam : NULL, mn_CP);

	_ASSERTEX(!mpwsz_TempRetFile && !mpwsz_TempRetParam && !mpwsz_TempRetDir);

	BOOL lbRc = PrepareExecuteParms(eShellExecute,
					mpwsz_TempAction, mpwsz_TempFile, mpwsz_TempParam,
					anFlags, NULL, NULL, anShowCmd,
					NULL, NULL, NULL, // *StdHandles
					&mpwsz_TempRetFile, &mpwsz_TempRetParam, &mpwsz_TempRetDir);
	if (lbRc)
	{
		if (mpwsz_TempRetFile && *mpwsz_TempRetFile)
		{
			mpsz_TempRetFile = wcs2str(mpwsz_TempRetFile, mn_CP);
			*asFile = mpsz_TempRetFile;
		}
		else
		{
			*asFile = NULL;
		}
	}

	if (lbRc || mpwsz_TempRetParam)
	{
		mpsz_TempRetParam = wcs2str(mpwsz_TempRetParam, mn_CP);
		*asParam = mpsz_TempRetParam;
	}

	if (mpwsz_TempRetDir)
	{
		mpsz_TempRetDir = wcs2str(mpwsz_TempRetDir, mn_CP);
		*asDir = mpsz_TempRetDir;
	}

	return lbRc;
}
BOOL CShellProc::OnShellExecuteW(LPCWSTR* asAction, LPCWSTR* asFile, LPCWSTR* asParam, LPCWSTR* asDir, DWORD* anFlags, DWORD* anShowCmd)
{
	if (!ghConEmuWndDC || !isWindow(ghConEmuWndDC))
		return FALSE; // ������������� ������ ��� ConEmu
	
	mb_InShellExecuteEx = TRUE;
	gnInShellExecuteEx ++;

	_ASSERTEX(!mpwsz_TempRetFile && !mpwsz_TempRetParam && !mpwsz_TempRetDir);

	BOOL lbRc = PrepareExecuteParms(eShellExecute,
					asAction ? *asAction : NULL,
					asFile ? *asFile : NULL,
					asParam ? *asParam : NULL,
					anFlags, NULL, NULL, anShowCmd,
					NULL, NULL, NULL, // *StdHandles
					&mpwsz_TempRetFile, &mpwsz_TempRetParam, &mpwsz_TempRetDir);
	if (lbRc)
	{
		*asFile = mpwsz_TempRetFile;
	}

	if (lbRc || mpwsz_TempRetParam)
	{
		*asParam = mpwsz_TempRetParam;
	}

	if (mpwsz_TempRetDir)
	{
		*asDir = mpwsz_TempRetDir;
	}

	return lbRc;
}
BOOL CShellProc::FixShellArgs(DWORD afMask, HWND ahWnd, DWORD* pfMask, HWND* phWnd)
{
	BOOL lbRc = FALSE;

	// �������� ������, ����� Shell �� ������� ������� ������� "������ �� �� ��������� ���� ����"...
	if (!(afMask & SEE_MASK_NOZONECHECKS) && gFarMode.bFarHookMode && gFarMode.bShellNoZoneCheck)
	{
		OSVERSIONINFOEX osv = {sizeof(OSVERSIONINFOEX)};
		if (GetVersionEx((LPOSVERSIONINFO)&osv))
		{
			if ((osv.dwMajorVersion >= 6)
				|| (osv.dwMajorVersion == 5 
				&& (osv.dwMinorVersion > 1 
				|| (osv.dwMinorVersion == 1 && osv.wServicePackMajor >= 1))))
			{
				(*pfMask) |= SEE_MASK_NOZONECHECKS;
				lbRc = TRUE;
			}
		}
	}

	// ����� ������� UAC ��� ��� ����� GUI ������� ��������� ��� ��� ����, � �� ��� ConEmu
	if ((!ahWnd || (ahWnd == ghConWnd)) && ghConEmuWnd)
	{
		*phWnd = ghConEmuWnd;
		lbRc = TRUE;
	}

	return lbRc;
}
BOOL CShellProc::OnShellExecuteExA(LPSHELLEXECUTEINFOA* lpExecInfo)
{
	if (!ghConEmuWndDC || !isWindow(ghConEmuWndDC) || !lpExecInfo)
		return FALSE; // ������������� ������ ��� ConEmu

	mlp_SaveExecInfoA = *lpExecInfo;
	mlp_ExecInfoA = (LPSHELLEXECUTEINFOA)malloc((*lpExecInfo)->cbSize);
	if (!mlp_ExecInfoA)
	{
		_ASSERTE(mlp_ExecInfoA!=NULL);
		return FALSE;
	}
	memmove(mlp_ExecInfoA, (*lpExecInfo), (*lpExecInfo)->cbSize);
	
	BOOL lbRc = FALSE;

	if (FixShellArgs((*lpExecInfo)->fMask, (*lpExecInfo)->hwnd, &(mlp_ExecInfoA->fMask), &(mlp_ExecInfoA->hwnd)))
		lbRc = TRUE;

	if (OnShellExecuteA(&mlp_ExecInfoA->lpVerb, &mlp_ExecInfoA->lpFile, &mlp_ExecInfoA->lpParameters, &mlp_ExecInfoA->lpDirectory, &mlp_ExecInfoA->fMask, (DWORD*)&mlp_ExecInfoA->nShow))
		lbRc = TRUE;

	if (lbRc)
		*lpExecInfo = mlp_ExecInfoA;
	return lbRc;
}
BOOL CShellProc::OnShellExecuteExW(LPSHELLEXECUTEINFOW* lpExecInfo)
{
	if (!ghConEmuWndDC || !isWindow(ghConEmuWndDC) || !lpExecInfo)
		return FALSE; // ������������� ������ ��� ConEmu

	mlp_SaveExecInfoW = *lpExecInfo;
	mlp_ExecInfoW = (LPSHELLEXECUTEINFOW)malloc((*lpExecInfo)->cbSize);
	if (!mlp_ExecInfoW)
	{
		_ASSERTE(mlp_ExecInfoW!=NULL);
		return FALSE;
	}
	memmove(mlp_ExecInfoW, (*lpExecInfo), (*lpExecInfo)->cbSize);
	
	BOOL lbRc = FALSE;

	if (FixShellArgs((*lpExecInfo)->fMask, (*lpExecInfo)->hwnd, &(mlp_ExecInfoW->fMask), &(mlp_ExecInfoW->hwnd)))
		lbRc = TRUE;

	if (OnShellExecuteW(&mlp_ExecInfoW->lpVerb, &mlp_ExecInfoW->lpFile, &mlp_ExecInfoW->lpParameters, &mlp_ExecInfoW->lpDirectory, &mlp_ExecInfoW->fMask, (DWORD*)&mlp_ExecInfoW->nShow))
		lbRc = TRUE;

	if (lbRc)
		*lpExecInfo = mlp_ExecInfoW;
	return lbRc;
}
void CShellProc::OnCreateProcessA(LPCSTR* asFile, LPCSTR* asCmdLine, LPCSTR* asDir, DWORD* anCreationFlags, LPSTARTUPINFOA lpSI)
{
	if (!ghConEmuWndDC || !isWindow(ghConEmuWndDC))
		return; // ������������� ������ ��� ConEmu

	mpwsz_TempFile = str2wcs(asFile ? *asFile : NULL, mn_CP);
	mpwsz_TempParam = str2wcs(asCmdLine ? *asCmdLine : NULL, mn_CP);
	DWORD nShowCmd = lpSI->wShowWindow;
	mb_WasSuspended = ((*anCreationFlags) & CREATE_SUSPENDED) == CREATE_SUSPENDED;

	_ASSERTEX(!mpwsz_TempRetFile && !mpwsz_TempRetParam && !mpwsz_TempRetDir);

	BOOL lbRc = PrepareExecuteParms(eCreateProcess,
					NULL, mpwsz_TempFile, mpwsz_TempParam,
					NULL, anCreationFlags, &lpSI->dwFlags, &nShowCmd,
					&lpSI->hStdInput, &lpSI->hStdOutput, &lpSI->hStdError,
					&mpwsz_TempRetFile, &mpwsz_TempRetParam, &mpwsz_TempRetDir);
	// ���������� TRUE ������ ���� ���� �������� ������,
	// � ���� ��������� mb_NeedInjects - ������ �������� _Suspended
	if (mb_NeedInjects)
		(*anCreationFlags) |= CREATE_SUSPENDED;
	if (lbRc)
	{
		if (lpSI->wShowWindow != nShowCmd)
			lpSI->wShowWindow = (WORD)nShowCmd;
		if (mpwsz_TempRetFile)
		{
			mpsz_TempRetFile = wcs2str(mpwsz_TempRetFile, mn_CP);
			*asFile = mpsz_TempRetFile;
		}
		else
		{
			*asFile = NULL;
		}
	}
	if (lbRc || mpwsz_TempRetParam)
	{
		if (mpwsz_TempRetParam)
		{
			mpsz_TempRetParam = wcs2str(mpwsz_TempRetParam, mn_CP);
			*asCmdLine = mpsz_TempRetParam;
		}
		else
		{
			*asCmdLine = NULL;
		}
	}
	if (mpwsz_TempRetDir)
	{
		mpsz_TempRetDir = wcs2str(mpwsz_TempRetDir, mn_CP);
		*asDir = mpsz_TempRetDir;
	}
}
void CShellProc::OnCreateProcessW(LPCWSTR* asFile, LPCWSTR* asCmdLine, LPCWSTR* asDir, DWORD* anCreationFlags, LPSTARTUPINFOW lpSI)
{
	if (!ghConEmuWndDC || !isWindow(ghConEmuWndDC))
		return; // ������������� ������ ��� ConEmu
		
	DWORD nShowCmd = (lpSI->dwFlags & STARTF_USESHOWWINDOW) ? lpSI->wShowWindow : SW_SHOWNORMAL;
	mb_WasSuspended = ((*anCreationFlags) & CREATE_SUSPENDED) == CREATE_SUSPENDED;

	_ASSERTEX(!mpwsz_TempRetFile && !mpwsz_TempRetParam && !mpwsz_TempRetDir);

	BOOL lbRc = PrepareExecuteParms(eCreateProcess,
					NULL,
					asFile ? *asFile : NULL,
					asCmdLine ? *asCmdLine : NULL,
					NULL, anCreationFlags, &lpSI->dwFlags, &nShowCmd,
					&lpSI->hStdInput, &lpSI->hStdOutput, &lpSI->hStdError,
					&mpwsz_TempRetFile, &mpwsz_TempRetParam, &mpwsz_TempRetDir);
	// ���������� TRUE ������ ���� ���� �������� ������,
	// � ���� ��������� mb_NeedInjects - ������ �������� _Suspended
	if (mb_NeedInjects)
		(*anCreationFlags) |= CREATE_SUSPENDED;
	if (lbRc)
	{
		if (lpSI->wShowWindow != nShowCmd)
		{
			lpSI->wShowWindow = (WORD)nShowCmd;
			if (!(lpSI->dwFlags & STARTF_USESHOWWINDOW))
				lpSI->dwFlags |= STARTF_USESHOWWINDOW;
		}
		*asFile = mpwsz_TempRetFile;
	}
	if (lbRc || mpwsz_TempRetParam)
	{
		*asCmdLine = mpwsz_TempRetParam;
	}
	if (mpwsz_TempRetDir)
	{
		*asDir = mpwsz_TempRetDir;
	}
}

void CShellProc::OnCreateProcessFinished(BOOL abSucceeded, PROCESS_INFORMATION *lpPI)
{
#ifdef _DEBUG
	{
		int cchLen = 255;
		wchar_t* pszDbgMsg = (wchar_t*)calloc(cchLen, sizeof(wchar_t));
		if (pszDbgMsg)
		{
			if (!abSucceeded)
			{
				msprintf(pszDbgMsg, cchLen, L"Create(ParentPID=%u): Failed, ErrCode=0x%08X", 
					GetCurrentProcessId(), GetLastError());
			}
			else
			{
				msprintf(pszDbgMsg, cchLen, L"Create(ParentPID=%u): Ok, PID=%u", 
					GetCurrentProcessId(), lpPI->dwProcessId);
				if (WaitForSingleObject(lpPI->hProcess, 0) == WAIT_OBJECT_0)
				{
					DWORD dwExitCode = 0;
					GetExitCodeProcess(lpPI->hProcess, &dwExitCode);
					msprintf(pszDbgMsg+lstrlen(pszDbgMsg), cchLen-lstrlen(pszDbgMsg),
						L", Terminated!!! Code=%u", dwExitCode);
				}
			}
			_wcscat_c(pszDbgMsg, cchLen, L"\n");
			OutputDebugString(pszDbgMsg);
			free(pszDbgMsg);
		}
	}
#endif

	if (abSucceeded)
	{
		if (mb_NeedInjects)
		{
			wchar_t szDbgMsg[255];
			#ifdef _DEBUG
			msprintf(szDbgMsg, countof(szDbgMsg), L"InjectHooks(x%u), ParentPID=%u, ChildPID=%u\n",
					#ifdef _WIN64
						64
					#else
						86
					#endif
						, GetCurrentProcessId(), lpPI->dwProcessId);
			OutputDebugString(szDbgMsg);
			#endif

			int iHookRc = InjectHooks(*lpPI, FALSE, (m_SrvMapping.cbSize && (m_SrvMapping.nLoggingType == glt_Processes)));

			if (iHookRc != 0)
			{
				DWORD nErrCode = GetLastError();
				// ���� �� ��������� ���������� ��� ��������� ��������� ��������� ���� ntvdm.exe,
				// �� ��� ������� dos ���������� �� ���� ����� �� ������
				_ASSERTE(iHookRc == 0);
				wchar_t szTitle[128];
				msprintf(szTitle, countof(szTitle), L"ConEmuC, PID=%u", GetCurrentProcessId());
				msprintf(szDbgMsg, countof(szDbgMsg), L"ConEmuC.W, PID=%u\nInjecting hooks into PID=%u\nFAILED, code=%i:0x%08X",
					GetCurrentProcessId(), lpPI->dwProcessId, iHookRc, nErrCode);
				GuiMessageBox(NULL, szDbgMsg, szTitle, MB_SYSTEMMODAL);
			}

			// ��������� �������
			if (!mb_WasSuspended)
				ResumeThread(lpPI->hThread);
		}
	}
}

void CShellProc::OnShellFinished(BOOL abSucceeded, HINSTANCE ahInstApp, HANDLE ahProcess)
{
	DWORD dwProcessID = 0;
	if (abSucceeded && gfGetProcessId && ahProcess)
	{
		dwProcessID = gfGetProcessId(ahProcess);
	}

	// InjectHooks & ResumeThread ��� ������ �� �����, ������ ������� ���������, ���� ���� ���������������
	if (mlp_SaveExecInfoW)
	{
		mlp_SaveExecInfoW->hInstApp = ahInstApp;
		mlp_SaveExecInfoW->hProcess = ahProcess;
	}
	else if (mlp_SaveExecInfoA)
	{
		mlp_SaveExecInfoA->hInstApp = ahInstApp;
		mlp_SaveExecInfoA->hProcess = ahProcess;
	}

	if (mb_InShellExecuteEx)
	{
		if (gnInShellExecuteEx > 0)
			gnInShellExecuteEx--;
		mb_InShellExecuteEx = FALSE;
	}
}
