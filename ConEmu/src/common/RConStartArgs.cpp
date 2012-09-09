
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

#define HIDE_USE_EXCEPTION_INFO
#include <windows.h>
#include "defines.h"
#include "MAssert.h"
#include "Memory.h"
#include "MStrSafe.h"
#include "RConStartArgs.h"
#include "common.hpp"
#include "WinObjects.h"

#ifdef __GNUC__
#define SecureZeroMemory(p,s) memset(p,0,s)
#endif

#define DefaultSplitValue 500

RConStartArgs::RConStartArgs()
{
	bDetached = bRunAsAdministrator = bRunAsRestricted = FALSE;
	bForceUserDialog = bBackgroundTab = bForceDosBox = FALSE;
	eSplit = eSplitNone; nSplitValue = DefaultSplitValue; nSplitPane = 0;
	aRecreate = cra_CreateTab;
	pszSpecialCmd = pszStartupDir = pszUserName = pszDomain = /*pszUserPassword =*/ NULL;
	bBufHeight = FALSE; nBufHeight = 0;
	eConfirmation = eConfDefault;
	szUserPassword[0] = 0;
	//hLogonToken = NULL;
}

RConStartArgs::~RConStartArgs()
{
	SafeFree(pszSpecialCmd); // ������ SafeFree
	SafeFree(pszStartupDir); // ������ SafeFree
	SafeFree(pszUserName);
	SafeFree(pszDomain);

	//SafeFree(pszUserPassword);
	if (szUserPassword[0]) SecureZeroMemory(szUserPassword, sizeof(szUserPassword));

	//if (hLogonToken) { CloseHandle(hLogonToken); hLogonToken = NULL; }
}

wchar_t* RConStartArgs::CreateCommandLine()
{
	wchar_t* pszFull = NULL;
	size_t cchMaxLen =
				 (pszSpecialCmd ? (lstrlen(pszSpecialCmd) + 3) : 0); // ������ �������
	cchMaxLen += (pszStartupDir ? (lstrlen(pszStartupDir) + 20) : 0); // "-new_console:d:..."
	cchMaxLen += (bRunAsAdministrator ? 15 : 0); // -new_console:a
	cchMaxLen += (bRunAsRestricted ? 15 : 0); // -new_console:r
	cchMaxLen += (pszUserName ? (lstrlen(pszUserName) + 32 // "-new_console:u:<user>:<pwd>"
						+ (pszDomain ? lstrlen(pszDomain) : 0)
						+ (szUserPassword ? lstrlen(szUserPassword) : 0)) : 0);
	cchMaxLen += (bForceUserDialog ? 15 : 0); // -new_console:u
	cchMaxLen += (bBackgroundTab ? 15 : 0); // -new_console:b
	cchMaxLen += (bBufHeight ? 32 : 0); // -new_console:h<lines>
	cchMaxLen += (eConfirmation ? 15 : 0); // -new_console:c / -new_console:n
	cchMaxLen += (bForceDosBox ? 15 : 0); // -new_console:x
	cchMaxLen += (eSplit ? 64 : 0); // -new_console:s[<SplitTab>T][<Percents>](H|V)

	pszFull = (wchar_t*)malloc(cchMaxLen*sizeof(*pszFull));
	if (!pszFull)
	{
		_ASSERTE(pszFull!=NULL);
		return NULL;
	}

	if (pszSpecialCmd)
	{
		// �� �����������. ���� ������ ����������� ������������
		_wcscpy_c(pszFull, cchMaxLen, pszSpecialCmd);
		_wcscat_c(pszFull, cchMaxLen, L" ");
	}
	else
	{
		*pszFull = 0;
	}

	wchar_t szAdd[128] = L"";
	if (bRunAsAdministrator)
		wcscat_c(szAdd, L"a");
	else if (bRunAsRestricted)
		wcscat_c(szAdd, L"r");
	
	if (bForceUserDialog)
		wcscat_c(szAdd, L"u");
	if (bBackgroundTab)
		wcscat_c(szAdd, L"b");
	if (bForceDosBox)
		wcscat_c(szAdd, L"x");

	if (bForceDosBox)
		wcscat_c(szAdd, L"x");
	
	if (eConfirmation == eConfAlways)
		wcscat_c(szAdd, L"c");
	else if (eConfirmation == eConfNever)
		wcscat_c(szAdd, L"n");

	if (bBufHeight)
	{
		if (nBufHeight)
			_wsprintf(szAdd+lstrlen(szAdd), SKIPLEN(16) L"h%u", nBufHeight);
		else
			wcscat_c(szAdd, L"h");
	}

	// -new_console:s[<SplitTab>T][<Percents>](H|V)
	if (eSplit)
	{
		wcscat_c(szAdd, L"s");
		if (nSplitPane)
			_wsprintf(szAdd+lstrlen(szAdd), SKIPLEN(16) L"%uT", nSplitPane);
		if ((int)(nSplitValue/10) != 0)
			_wsprintf(szAdd+lstrlen(szAdd), SKIPLEN(16) L"%u", (int)(nSplitValue/10));
		wcscat_c(szAdd, (eSplit == eSplitHorz) ? L"H" : L"V");
	}

	if (szAdd[0])
	{
		_wcscat_c(pszFull, cchMaxLen, L" -new_console:");
		_wcscat_c(pszFull, cchMaxLen, szAdd);
	}

	// "-new_console:d:..."
	if (pszStartupDir && *pszStartupDir)
	{
		bool bQuot = wcschr(pszStartupDir, L' ') != NULL;
		_wcscat_c(pszFull, cchMaxLen, bQuot ? L" \"-new_console:d:" : L" -new_console:d:");
		_wcscat_c(pszFull, cchMaxLen, pszStartupDir);
		if (bQuot)
			_wcscat_c(pszFull, cchMaxLen, L"\"");
	}

	// "-new_console:u:<user>:<pwd>"
	if (pszUserName && *pszUserName)
	{
		_wcscat_c(pszFull, cchMaxLen, L" \"-new_console:u:");
		if (pszDomain && *pszDomain)
		{
			_wcscat_c(pszFull, cchMaxLen, pszDomain);
			_wcscat_c(pszFull, cchMaxLen, L"\\");
		}
		_wcscat_c(pszFull, cchMaxLen, pszUserName);
		if (szUserPassword)
		{
			_wcscat_c(pszFull, cchMaxLen, L":");
			_wcscat_c(pszFull, cchMaxLen, szUserPassword);
		}
		_wcscat_c(pszFull, cchMaxLen, L"\"");
	}

	return pszFull;
}

BOOL RConStartArgs::CheckUserToken(HWND hPwd)
{
	//if (hLogonToken) { CloseHandle(hLogonToken); hLogonToken = NULL; }
	if (!pszUserName || !*pszUserName)
		return FALSE;

	//wchar_t szPwd[MAX_PATH]; szPwd[0] = 0;
	szUserPassword[0] = 0;

	if (!GetWindowText(hPwd, szUserPassword, MAX_PATH-1))
		return FALSE;

	SafeFree(pszDomain);
	wchar_t* pszSlash = wcschr(pszUserName, L'\\');
	if (pszSlash)
	{
		pszDomain = pszUserName;
		*pszSlash = 0;
		pszUserName = lstrdup(pszSlash+1);
	}

	HANDLE hLogonToken = NULL;
	BOOL lbRc = LogonUser(pszUserName, pszDomain, szUserPassword, LOGON32_LOGON_INTERACTIVE,
	                      LOGON32_PROVIDER_DEFAULT, &hLogonToken);
	//if (szUserPassword[0]) SecureZeroMemory(szUserPassword, sizeof(szUserPassword));

	if (!lbRc || !hLogonToken)
	{
		MessageBox(GetParent(hPwd), L"Invalid user name or password specified!", L"ConEmu", MB_OK|MB_ICONSTOP);
		return FALSE;
	}

	CloseHandle(hLogonToken);
	//hLogonToken may be used for CreateProcessAsUser
	return TRUE;
}

// Returns ">0" - when changes was made
//  0 - no changes
// -1 - error
int RConStartArgs::ProcessNewConArg()
{
	if (!pszSpecialCmd || !*pszSpecialCmd)
	{
		_ASSERTE(pszSpecialCmd && *pszSpecialCmd);
		return -1;
	}

	int nChanges = 0;
	
	// 120115 - ���� ������ �������� - "ConEmu.exe" ��� "ConEmu64.exe" - �� ������������ "-cur_console" � "-new_console"
	{
		LPCWSTR pszTemp = pszSpecialCmd;
		wchar_t szExe[MAX_PATH+1];
		if (0 == NextArg(&pszTemp, szExe))
		{
			pszTemp = PointToName(szExe);
			if (lstrcmpi(pszTemp, L"ConEmu.exe") == 0
				|| lstrcmpi(pszTemp, L"ConEmu") == 0
				|| lstrcmpi(pszTemp, L"ConEmu64.exe") == 0
				|| lstrcmpi(pszTemp, L"ConEmu64") == 0)
			{
				return 0;
			}
		}
	}
	

	// 111211 - ����� ����� ���� ������� "-new_console:..."
	LPCWSTR pszNewCon = L"-new_console";
	// 120108 - ��� "-cur_console:..." ��� ��������� ���������� ������� ������ (�� ���� ��������)
	LPCWSTR pszCurCon = L"-cur_console";
	int nNewConLen = lstrlen(pszNewCon);
	_ASSERTE(lstrlen(pszCurCon)==nNewConLen);
	wchar_t* pszFind;
	bool bStop = false;

	while (!bStop
		&& ((pszFind = wcsstr(pszSpecialCmd, pszNewCon)) != NULL || (pszFind = wcsstr(pszSpecialCmd, pszCurCon)) != NULL)
		)
	{
		// �������� ����������
		_ASSERTE(pszFind > pszSpecialCmd);
		if (((pszFind == pszSpecialCmd) || (*(pszFind-1) == L'"') || (*(pszFind-1) == L' ')) // ������ ���������
			&& (pszFind[nNewConLen] == L' ' || pszFind[nNewConLen] == L':' 
				|| pszFind[nNewConLen] == L'"' || pszFind[nNewConLen] == 0))
		{
			// �� ���������, ������������� �������� "Press Enter or Esc to close console"
			eConfirmation = eConfAlways;
		
			bool lbQuot = (*(pszFind-1) == L'"');
			const wchar_t* pszEnd = pszFind+nNewConLen;
			//wchar_t szNewConArg[MAX_PATH+1];
			if (lbQuot)
				pszFind--;

			if (*pszEnd == L'"')
			{
				pszEnd++;
			}
			else if (*pszEnd != L':')
			{
				// �����
				_ASSERTE(*pszEnd == L' ' || *pszEnd == 0);
			}
			else
			{
				if (*pszEnd == L':')
				{
					pszEnd++;
				}
				else
				{
					_ASSERTE(*pszEnd == L':');
				}

				// ��������� ���.���������� -new_console:xxx
				bool lbReady = false;
				while (!lbReady && *pszEnd)
				{
					switch (*(pszEnd++))
					{
					//case L'-':
					//	bStop = true; // ��������� "-new_console" - �� �������!
					//	break;
					case L'"':
					case L' ':
					case 0:
						lbReady = true;
						break;
						
					case L'b':
						// b - background, �� ������������ ���
						bBackgroundTab = TRUE;
						break;
						
					case L'a':
						// a - RunAs shell verb (as admin on Vista+, login/password in WinXP-)
						bRunAsAdministrator = TRUE;
						break;
						
					case L'r':
						// r - run as restricted user
						bRunAsRestricted = TRUE;
						break;
						
					case L'h':
						// "h0" - ��������� �����, "h9999" - �������� ����� � 9999 �����
						{
							bBufHeight = TRUE;
							if (isDigit(*pszEnd))
							{
								wchar_t* pszDigits = NULL;
								nBufHeight = wcstoul(pszEnd, &pszDigits, 10);
								if (pszDigits)
									pszEnd = pszDigits;
							}
							else
							{
								nBufHeight = 0;
							}
						} // L'h':
						break;
						
					case L'n':
						// n - ��������� "Press Enter or Esc to close console"
						eConfirmation = eConfNever;
						break;
						
					case L'c':
						// c - ������������� �������� "Press Enter or Esc to close console"
						eConfirmation = eConfAlways;
						break;
						
					case L'x':
						// x - Force using dosbox for .bat files
						bForceDosBox = TRUE;
						break;
						
					// "Long" code blocks below: 'd', 'u', 's' and so on (in future)
					case L'd':
						// d:<StartupDir>. MUST be last options
						{
							if (*pszEnd == L':')
								pszEnd++;
							const wchar_t* pszDir = pszEnd;
							while ((*pszEnd) && (lbQuot || *pszEnd != L' ') && (*pszEnd != L'"'))
								pszEnd++;
							if (pszEnd > pszDir)
							{
								size_t cchLen = pszEnd - pszDir;
								SafeFree(pszStartupDir);
								pszStartupDir = (wchar_t*)malloc((cchLen+1)*sizeof(*pszStartupDir));
								if (pszStartupDir)
								{
									wmemmove(pszStartupDir, pszDir, cchLen);
									pszStartupDir[cchLen] = 0;
									// ��������, "%USERPROFILE%"
									if (wcschr(pszStartupDir, L'%'))
									{
										wchar_t* pszExpand = NULL;
										if (((pszExpand = ExpandEnvStr(pszStartupDir)) != NULL))
										{
											SafeFree(pszStartupDir);
											pszStartupDir = pszExpand;
										}
									}
								}
							}
						} // L'd':
						break;
						
					case L's':
						// s[<SplitTab>T][<Percents>](H|V)
						// ������: "s3T30H" - ������� 3-�� ���. ����� ������ ����� Pane ������, ������� 30% �� 3-�� ����.
						{
							UINT nTab = 0 /*active*/, nValue = /*�������*/DefaultSplitValue/10;
							while (*pszEnd)
							{
								if (isDigit(*pszEnd))
								{
									wchar_t* pszDigits = NULL;
									UINT n = wcstoul(pszEnd, &pszDigits, 10);
									if (!pszDigits)
										break;
									pszEnd = pszDigits;
									if (*pszDigits == L'T')
									{
                                    	nTab = n;
                                	}
                                    else if ((*pszDigits == L'H') || (*pszDigits == L'V'))
                                    {
                                    	nValue = n;
                                    	eSplit = (*pszDigits == L'H') ? eSplitHorz : eSplitVert;
                                    }
                                    else
                                    {
                                    	break;
                                    }
                                    pszEnd++;
								}
								else if (*pszEnd == L'T')
								{
									nTab = 0;
									pszEnd++;
								}
								else if ((*pszEnd == L'H') || (*pszEnd == L'V'))
								{
	                            	nValue = DefaultSplitValue/10;
	                            	eSplit = (*pszEnd == L'H') ? eSplitHorz : eSplitVert;
	                            	pszEnd++;
								}
								else
								{
									break;
								}
							}
							if (!eSplit)
								eSplit = eSplitHorz;
							// ��� ��������, ������������ ������ ������ ����� �����
							nSplitValue = 1000-max(1,min(nValue*10,999)); // ��������
							_ASSERTE(nSplitValue>=1 && nSplitValue<1000);
							nSplitPane = nTab;
						} // L's'
						break;
						
					case L'u':
						{
							// u - ConEmu choose user dialog
							// u:<user>:<pwd> - specify user/pwd in args. MUST be last option
							
							lbReady = true; // ��������� �����

							SafeFree(pszUserName);
							SafeFree(pszDomain);
							if (szUserPassword[0]) SecureZeroMemory(szUserPassword, sizeof(szUserPassword));
							
							if (*pszEnd == L':')
							{
								pszEnd++;
								
								wchar_t szUser[MAX_PATH], *p = szUser, *p2 = szUser+countof(szUser)-1;
								while (*pszEnd && (p < p2))
								{
									if ((*pszEnd == 0) || (*pszEnd == L':') || (*pszEnd == L'"'))
									{
										break;
									}
									//else if (*pszEnd == L'"' && *(pszEnd+1) == L'"')
									//{
									//	*(p++) = L'"'; pszEnd += 2;
									//}
									else if (*pszEnd == L'^')
									{
										pszEnd++;
										*(p++) = *(pszEnd++);
									}
									else
									{
										*(p++) = *(pszEnd++);
									}
								}
								*p = 0;

								wchar_t* pszSlash = wcschr(szUser, L'\\');
								if (pszSlash)
								{
									*pszSlash = 0;
									pszDomain = lstrdup(szUser);
									pszUserName = lstrdup(pszSlash+1);
								}
								else
								{
									pszUserName = lstrdup(szUser);
								}
								
								if (*pszEnd == L':')
								{
									pszEnd++;
									//lstrcpyn(szUserPassword, pszPwd, countof(szUserPassword));

									p = szUserPassword; p2 = szUserPassword+countof(szUserPassword)-1;
									while (*pszEnd && (p < p2))
									{
										if ((*pszEnd == 0) || (*pszEnd == L':') || (*pszEnd == L'"'))
										{
											break;
										}
										else if (*pszEnd == L'^')
										{
											pszEnd++;
											*(p++) = *(pszEnd++);
										}
										else
										{
											*(p++) = *(pszEnd++);
										}
									}
									*p = 0;

								}
							}
							else
							{
								bForceUserDialog = TRUE;
							}
						} // L'u'
						break;
					}
				}
			}

			if (pszEnd > pszFind)
			{
				if (lbQuot)
				{
					if (*pszEnd == L'"' && *(pszEnd-1) != L'"')
						pszEnd++;
				}
				else
				{
					while (*(pszEnd-1) == L'"')
						pszEnd--;
				}

				while (((pszFind - 1) > pszSpecialCmd) && (*(pszFind-1) == L' ') && (*(pszFind-2) == L' '))
					pszFind--;
				//wmemset(pszFind, L' ', pszEnd - pszFind);
				wmemmove(pszFind, pszEnd, (lstrlen(pszEnd)+1));
				nChanges++;
			}
			else
			{
				_ASSERTE(pszEnd > pszFind);
				*pszFind = 0;
				nChanges++;
			}
		}
	}

	return nChanges;
}
