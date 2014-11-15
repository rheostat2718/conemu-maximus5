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
#include "ConEmu.h"
#include "MyClipboard.h"

static LONG gnMyClipboardOpened = 0;

bool MyOpenClipboard(LPCWSTR asAction)
{
	_ASSERTE(gnMyClipboardOpened==0 || gnMyClipboardOpened==1);

	if (gnMyClipboardOpened > 0)
	{
		InterlockedIncrement(&gnMyClipboardOpened);
		return true;
	}

	BOOL lbRc;

	// Открыть буфер обмена
	while (!(lbRc = OpenClipboard(ghWnd)))
	{
		DWORD dwErr = GetLastError();

		wchar_t szCode[32]; _wsprintf(szCode, SKIPCOUNT(szCode) L", Code=%u", dwErr);
		wchar_t* pszMsg = lstrmerge(L"OpenClipboard failed (", asAction, L")", szCode);
		LogString(pszMsg);
		int iBtn = DisplayLastError(pszMsg, dwErr, MB_RETRYCANCEL|MB_ICONSTOP);
		SafeFree(pszMsg);

		if (iBtn != IDRETRY)
			return false;
	}

	InterlockedIncrement(&gnMyClipboardOpened);
	_ASSERTE(gnMyClipboardOpened==1);

	LogString(L"OpenClipboard succeeded");
	return true;
}

void MyCloseClipboard()
{
	_ASSERTE(gnMyClipboardOpened==1 || gnMyClipboardOpened==2);

	if (InterlockedDecrement(&gnMyClipboardOpened) > 0)
	{
		return;
	}

	BOOL bRc = CloseClipboard();

	LogString(bRc ? L"CloseClipboard succeeded" : L"CloseClipboard failed");
}

HANDLE MySetClipboardData(UINT uFormat, HANDLE hMem)
{
	HANDLE h = SetClipboardData(uFormat, hMem);

	wchar_t szLog[100]; DWORD dwErr = (h == NULL) ? GetLastError() : 0;
	if (h != NULL)
		_wsprintf(szLog, SKIPCOUNT(szLog) L"SetClipboardData(x%04X, x%08X) succeeded", uFormat, (DWORD)(DWORD_PTR)hMem);
	else
		_wsprintf(szLog, SKIPCOUNT(szLog) L"SetClipboardData(x%04X, x%08X) failed, code=%u", uFormat, (DWORD)(DWORD_PTR)hMem, dwErr);
	LogString(szLog);

	return h;
}

bool CopyToClipboard(LPCWSTR asText)
{
	if (!asText)
		return false;

	bool bCopied = false;

	if (MyOpenClipboard(L"CopyToClipboard"))
	{
		DWORD cch = lstrlen(asText);
		HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (cch + 1) * sizeof(*asText));
		if (hglbCopy)
		{
			wchar_t* lptstrCopy = (wchar_t*)GlobalLock(hglbCopy);
			if (lptstrCopy)
			{
				_wcscpy_c(lptstrCopy, cch+1, asText);
				GlobalUnlock(hglbCopy);

				EmptyClipboard();
				bCopied = (MySetClipboardData(CF_UNICODETEXT, hglbCopy) != NULL);
			}
		}

		MyCloseClipboard();
	}

	return bCopied;
}

wchar_t* GetCliboardText(DWORD& rnErrCode, wchar_t* rsErrText, INT_PTR cchErrMax)
{
	if (!rsErrText || cchErrMax < 255)
	{
		_ASSERTE(FALSE && "Invalid arguments");
		return NULL;
	}

	HGLOBAL hglb;
	LPCWSTR lptstr;
	wchar_t* pszBuf = NULL;

	if ((hglb = GetClipboardData(CF_UNICODETEXT)) == NULL)
	{
		rnErrCode = GetLastError();
		_wsprintf(rsErrText, SKIPLEN(cchErrMax) L"Clipboard does not contain CF_UNICODETEXT, nothing to paste (code=%u)", rnErrCode);
		gpConEmu->LogString(rsErrText);
		_wcscpy_c(rsErrText, cchErrMax, L"Available formats:");
		int nLen = lstrlen(rsErrText);
		UINT fmt = 0;
		while (((nLen + 11) < cchErrMax) && ((fmt = EnumClipboardFormats(fmt)) != 0))
		{
			_wsprintf(rsErrText+nLen, SKIPLEN(cchErrMax-nLen) L" x%04X", fmt);
			nLen += lstrlen(rsErrText+nLen);
		}
		gpConEmu->LogString(rsErrText);
		rsErrText[0] = 0; // Don't call DisplayLastError
		TODO("Сделать статусное сообщение с таймаутом");
		//this->SetConStatus(L"Clipboard does not contains text. Nothing to paste.");
	}
	else if ((lptstr = (LPCWSTR)GlobalLock(hglb)) == NULL)
	{
		rnErrCode = GetLastError();
		_wsprintf(rsErrText, SKIPLEN(cchErrMax) L"Can't lock CF_UNICODETEXT, paste failed (code=%u)", rnErrCode);
		gpConEmu->LogString(rsErrText);
	}
	else if (*lptstr == 0)
	{
		rnErrCode = GetLastError();
		_wsprintf(rsErrText, SKIPLEN(cchErrMax) L"CF_UNICODETEXT is empty, nothing to paste (code=%u)", rnErrCode);
		gpConEmu->LogString(rsErrText);
		rsErrText[0] = 0; // Don't call DisplayLastError
		GlobalUnlock(hglb);
	}
	else
	{
		pszBuf = lstrdup(lptstr, 1); // Reserve memory for space-termination
		Assert(pszBuf!=NULL);
		GlobalUnlock(hglb);
	}

	return pszBuf;
}
