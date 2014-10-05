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
#include <windows.h>
#include "defines.h"
#include "Memory.h"
#include "MAssert.h"
#include "WinRegistry.h"

int RegEnumKeys(HKEY hkRoot, LPCWSTR pszParentPath, RegEnumKeysCallback fn, LPARAM lParam)
{
	int iRc = -1;
	HKEY hk = NULL, hkChild = NULL;
	bool bContinue = true;
	LONG lrc;

	if (0 == (lrc = RegOpenKeyEx(hkRoot, pszParentPath, 0, KEY_READ, &hk)))
	{
		iRc = 0;
		UINT n = 0;
		wchar_t szSubKey[MAX_PATH] = L""; DWORD cchMax = countof(szSubKey) - 1;

		while (0 == (lrc = RegEnumKeyEx(hk, n++, szSubKey, &cchMax, NULL, NULL, NULL, NULL)))
		{
			if (0 == (lrc = RegOpenKeyEx(hk, szSubKey, 0, KEY_READ, &hkChild)))
			{
				if (fn != NULL)
				{
					if (!fn(hkChild, szSubKey, lParam))
						break;
				}
				iRc++;
				RegCloseKey(hkChild);
			}
			cchMax = countof(szSubKey) - 1;
		}

		RegCloseKey(hk);
	}

	return iRc;
}

int RegGetStringValue(HKEY hk, LPCWSTR pszSubKey, LPCWSTR pszValueName, CEStr& rszData)
{
	int iLen = -1;
	HKEY hkChild = hk;
	DWORD cbSize = 0;
	LONG lrc;

	rszData.Empty();

	if (pszSubKey && *pszSubKey)
	{
		if (0 != (lrc = RegOpenKeyEx(hk, pszSubKey, 0, KEY_READ, &hkChild)))
			hkChild = NULL;
	}

	if (hkChild && (0 == (lrc = RegQueryValueEx(hkChild, pszValueName, NULL, NULL, NULL, &cbSize))))
	{
		wchar_t* pszData = rszData.GetBuffer((cbSize>>1)+2); // +wchar_t+1byte (на возможные ошибки хранения данных в реестре)
		if (pszData)
		{
			pszData[cbSize>>1] = 0; // Make sure it will be 0-terminated
			if (0 == (lrc = RegQueryValueEx(hkChild, pszValueName, NULL, NULL, (LPBYTE)pszData, &cbSize)))
			{
				iLen = lstrlen(pszData);
			}
			else
			{
				rszData.Empty();
			}
		}
	}

	if (hkChild != hk)
		RegCloseKey(hkChild);

	return iLen;
}
