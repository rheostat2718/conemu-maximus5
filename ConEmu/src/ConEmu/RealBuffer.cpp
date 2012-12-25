
//TODO: �������. 
//TODO: XXX       - ������� �������� ����������, �������� �������������� ��
//TODO: GetXXX    - ������� �������� ����������
//TODO: SetXXX    - ���������� �������� ����������, ������� � ������� �� ����������!
//TODO: ChangeXXX - ������� � ������� � ���������� �������� ����������

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
#define SHOWDEBUGSTR
//#define ALLOWUSEFARSYNCHRO

#include "Header.h"
#include <Tlhelp32.h>
#include "../common/ConEmuCheck.h"
#include "../common/RgnDetect.h"
#include "../common/Execute.h"
#include "RealBuffer.h"
#include "RealConsole.h"
#include "VirtualConsole.h"
#include "TabBar.h"
#include "ConEmu.h"
#include "ConEmuApp.h"
#include "VConChild.h"
#include "ConEmuPipe.h"
#include "Macro.h"
#include "Status.h"
#include "Menu.h"
#include "TrayIcon.h"

#define DEBUGSTRINPUT(s) //DEBUGSTR(s)
#define DEBUGSTRINPUTPIPE(s) //DEBUGSTR(s)
#define DEBUGSTRSIZE(s) //DEBUGSTR(s)
#define DEBUGSTRPROC(s) DEBUGSTR(s)
#define DEBUGSTRCMD(s) DEBUGSTR(s)
#define DEBUGSTRPKT(s) //DEBUGSTR(s)
#define DEBUGSTRCON(s) //DEBUGSTR(s)
#define DEBUGSTRLANG(s) //DEBUGSTR(s)// ; Sleep(2000)
#define DEBUGSTRLOG(s) //OutputDebugStringA(s)
#define DEBUGSTRALIVE(s) //DEBUGSTR(s)
#define DEBUGSTRTABS(s) DEBUGSTR(s)
#define DEBUGSTRMACRO(s) //DEBUGSTR(s)
#define DEBUGSTRCURSORPOS(s) //DEBUGSTR(s)

// ANSI, without "\r\n"
#define IFLOGCONSOLECHANGE gpSetCls->isAdvLogging>=2
#define LOGCONSOLECHANGE(s) if (IFLOGCONSOLECHANGE) mp_RCon->LogString(s, TRUE)

#ifndef CONSOLE_MOUSE_DOWN
#define CONSOLE_MOUSE_DOWN 8
#endif

#define Free SafeFree
#define Alloc calloc

#ifdef _DEBUG
#define HEAPVAL MCHKHEAP
#else
#define HEAPVAL
#endif

#define Assert(V) if ((V)==FALSE) { wchar_t szAMsg[MAX_PATH*2]; _wsprintf(szAMsg, SKIPLEN(countof(szAMsg)) L"Assertion (%s) at\n%s:%i", _T(#V), _T(__FILE__), __LINE__); CRealConsole::Box(szAMsg); }

const wchar_t gszAnalogues[32] =
{
	32, 9786, 9787, 9829, 9830, 9827, 9824, 8226, 9688, 9675, 9689, 9794, 9792, 9834, 9835, 9788,
	9658, 9668, 8597, 8252,  182,  167, 9632, 8616, 8593, 8595, 8594, 8592, 8735, 8596, 9650, 9660
};

const DWORD gnKeyBarFlags[] = {0,
	LEFT_CTRL_PRESSED, LEFT_ALT_PRESSED, SHIFT_PRESSED,
	LEFT_CTRL_PRESSED|LEFT_ALT_PRESSED, LEFT_CTRL_PRESSED|SHIFT_PRESSED,
	LEFT_ALT_PRESSED|SHIFT_PRESSED,
	RIGHT_CTRL_PRESSED, RIGHT_ALT_PRESSED,
	RIGHT_CTRL_PRESSED|RIGHT_ALT_PRESSED, RIGHT_CTRL_PRESSED|SHIFT_PRESSED,
	RIGHT_ALT_PRESSED|SHIFT_PRESSED
};


CRealBuffer::CRealBuffer(CRealConsole* apRCon, RealBufferType aType/*=rbt_Primary*/)
{
	mp_RCon = apRCon;
	m_Type = aType;
	mn_LastRgnFlags = -1;
	
	ZeroStruct(con);
	con.hInSetSize = CreateEvent(0,TRUE,TRUE,0);
	
	mb_BuferModeChangeLocked = FALSE;
	mcr_LastMousePos = MakeCoord(-1,-1);

	mr_LeftPanel = mr_LeftPanelFull = mr_RightPanel = mr_RightPanel = MakeRect(-1,-1);
	mb_LeftPanel = mb_RightPanel = FALSE;
	
	ZeroStruct(dump);

	InitializeCriticalSection(&m_TrueMode.csLock);
	m_TrueMode.mp_Cmp = NULL;
	m_TrueMode.nCmpMax = 0;
}

CRealBuffer::~CRealBuffer()
{
	if (con.pConChar)
		{ Free(con.pConChar); con.pConChar = NULL; }

	if (con.pConAttr)
		{ Free(con.pConAttr); con.pConAttr = NULL; }

	if (con.hInSetSize)
		{ CloseHandle(con.hInSetSize); con.hInSetSize = NULL; }

	dump.Close();

	DeleteCriticalSection(&m_TrueMode.csLock);
	SafeFree(m_TrueMode.mp_Cmp);
}

void CRealBuffer::ReleaseMem()
{
	TODO("���������� �� ������������ ������");
	m_Type = rbt_Undefined;
	dump.Close();
}

// ���������� ����� CVirtualConsole::Dump � CRealConsole::DumpConsole
// ��� ���������, ������ ���� � ����� �����
void CRealBuffer::DumpConsole(HANDLE ahFile)
{
	BOOL lbRc = FALSE;
	DWORD dw = 0;

	if (con.pConChar && con.pConAttr)
	{
		MSectionLock sc; sc.Lock(&csCON, FALSE);
		DWORD nSize = con.nTextWidth * con.nTextHeight * 2;
		lbRc = WriteFile(ahFile, con.pConChar, nSize, &dw, NULL);
		lbRc = WriteFile(ahFile, con.pConAttr, nSize, &dw, NULL); //-V519
	}
}


bool CRealBuffer::LoadDumpConsole(LPCWSTR asDumpFile)
{
	bool lbRc = false;
	HANDLE hFile = NULL;
	LARGE_INTEGER liSize;
	COORD cr = {};
	wchar_t* pszDumpTitle, *pszRN, *pszSize;
	uint nX = 0;
	uint nY = 0;
	DWORD dwConDataBufSize = 0;
	DWORD dwConDataBufSizeEx = 0;
	
	con.m_sel.dwFlags = 0;
	dump.Close();

	if (!asDumpFile || !*asDumpFile)
	{
		_ASSERTE(asDumpFile!=NULL && *asDumpFile);
		goto wrap;
	}
	
	hFile = CreateFile(asDumpFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (!hFile || hFile == INVALID_HANDLE_VALUE)
	{
		DisplayLastError(L"Can't open dump file for reading");
		goto wrap;
	}
	if (!GetFileSizeEx(hFile, &liSize) || !liSize.LowPart || liSize.HighPart)
	{
		DisplayLastError(L"Invalid dump file size", -1);
		goto wrap;
	}
	
	dump.ptrData = (LPBYTE)malloc(liSize.LowPart);
	if (!dump.ptrData)
	{
		_ASSERTE(dump.ptrData!=NULL);
		goto wrap;
	}
	dump.cbDataSize = liSize.LowPart;
	
	if (!ReadFile(hFile, dump.ptrData, liSize.LowPart, (LPDWORD)&liSize.HighPart, NULL) || (liSize.LowPart != (DWORD)liSize.HighPart))
	{
		DisplayLastError(L"Dump file reading failed");
		goto wrap;
	}
	CloseHandle(hFile); hFile = NULL;

	// �������
	pszDumpTitle = (wchar_t*)dump.ptrData;
	
	pszRN = wcschr(pszDumpTitle, L'\r');
	if (!pszRN)
	{
		DisplayLastError(L"Dump file invalid format (title)", -1);
		goto wrap;
	}
	*pszRN = 0;
	pszSize = pszRN + 2;

	if (wcsncmp(pszSize, L"Size: ", 6))
	{
		DisplayLastError(L"Dump file invalid format (Size start)", -1);
		goto wrap;
	}

	pszRN = wcschr(pszSize, L'\r');

	if (!pszRN)
	{
		DisplayLastError(L"Dump file invalid format (Size line end)", -1);
		goto wrap;
	}

	// ������ ����
	dump.pszBlock1 = pszRN + 2;
	
	pszSize += 6;
	dump.crSize.X = (SHORT)wcstol(pszSize, &pszRN, 10);

	if (!pszRN || *pszRN!=L'x')
	{
		DisplayLastError(L"Dump file invalid format (Size.X)", -1);
		goto wrap;
	}

	pszSize = pszRN + 1;
	dump.crSize.Y = (SHORT)wcstol(pszSize, &pszRN, 10);

	if (!pszRN || (*pszRN!=L' ' && *pszRN!=L'\r'))
	{
		DisplayLastError(L"Dump file invalid format (Size.Y)", -1);
		goto wrap;
	}

	pszSize = pszRN;
	dump.crCursor.X = 0; dump.crCursor.Y = dump.crSize.Y-1;

	if (*pszSize == L' ')
	{
		while(*pszSize == L' ') pszSize++;

		if (wcsncmp(pszSize, L"Cursor: ", 8)==0)
		{
			pszSize += 8;
			cr.X = (SHORT)wcstol(pszSize, &pszRN, 10);

			if (!pszRN || *pszRN!=L'x')
			{
				DisplayLastError(L"Dump file invalid format (Cursor)", -1);
				goto wrap;
			}

			pszSize = pszRN + 1;
			cr.Y = (SHORT)wcstol(pszSize, &pszRN, 10);

			if (cr.X>=0 && cr.Y>=0)
			{
				dump.crCursor.X = cr.X; dump.crCursor.Y = cr.Y;
			}
		}
	}

	dwConDataBufSize = dump.crSize.X * dump.crSize.Y;
	dwConDataBufSizeEx = dump.crSize.X * dump.crSize.Y * sizeof(CharAttr);
	
	dump.pcaBlock1 = (CharAttr*)(((WORD*)(dump.pszBlock1)) + dwConDataBufSize);
	dump.Block1 = (((LPBYTE)(((LPBYTE)(dump.pcaBlock1)) + dwConDataBufSizeEx)) - dump.ptrData) <= (INT_PTR)dump.cbDataSize;
	
	if (!dump.Block1)
	{
		DisplayLastError(L"Dump file invalid format (Block1)", -1);
		goto wrap;
	}
	
	m_Type = rbt_DumpScreen;

	
	// ��� ��������� ��������������� ������ ������ ��������� SyncWindow2Console.
	// �������� ������� ������� ������ � �������.
	nX = mp_RCon->TextWidth();
	nY = mp_RCon->TextHeight();

	
	// ����� ��� ������, �������� �������������
	ZeroStruct(con.m_sel);
	con.m_ci.dwSize = 15; con.m_ci.bVisible = TRUE;
	ZeroStruct(con.m_sbi);
	con.m_dwConsoleCP = con.m_dwConsoleOutputCP = 866; con.m_dwConsoleMode = 0;
	con.m_sbi.dwSize = dump.crSize;
	con.m_sbi.dwCursorPosition = dump.crCursor;
	con.m_sbi.wAttributes = 7;
	con.m_sbi.srWindow.Left = 0;
	con.m_sbi.srWindow.Top = 0;
	con.m_sbi.srWindow.Right = nX - 1;
	con.m_sbi.srWindow.Bottom = nY - 1;
	con.crMaxSize = mp_RCon->mp_RBuf->con.crMaxSize; //MakeCoord(max(dump.crSize.X,nX),max(dump.crSize.Y,nY));
	con.m_sbi.dwMaximumWindowSize = con.crMaxSize; //dump.crSize;
	con.nTopVisibleLine = 0;
	con.nTextWidth = nX/*dump.crSize.X*/;
	con.nTextHeight = nY/*dump.crSize.Y*/;
	con.nBufferHeight = dump.crSize.Y;
	con.bBufferHeight = TRUE;
	TODO("�������������� ���������");
	
	//dump.NeedApply = TRUE;
	
	// �������� �������
	if (!InitBuffers(0))
	{
		_ASSERTE(FALSE);
		goto wrap;
	}
	else
	// � �����������
	{
		wchar_t*  pszSrc = dump.pszBlock1;
		CharAttr* pcaSrc = dump.pcaBlock1;
		wchar_t*  pszDst = con.pConChar;
		TODO("������ �� ���� ����������� ����� ��� �������, � �� ������ CHAR_ATTR");
		WORD*     pnaDst = con.pConAttr;
		
		wmemmove(pszDst, pszSrc, dwConDataBufSize);

		WARNING("������, ��� ��� ������ �� ���������, ��� ����� ����� ����������");
		// ���������� ������ CharAttr �� ���������� ��������
		for (DWORD n = 0; n < dwConDataBufSize; n++, pcaSrc++, pnaDst++)
		{
			*pnaDst = (pcaSrc->nForeIdx & 0xF) | ((pcaSrc->nBackIdx & 0xF) << 4);
		}
	}

	con.bConsoleDataChanged = TRUE;

	lbRc = true; // OK
wrap:
	if (hFile && hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);
	if (!lbRc)
		dump.Close();
	return lbRc;
}

bool CRealBuffer::LoadDataFromDump(const CONSOLE_SCREEN_BUFFER_INFO& storedSbi, const CHAR_INFO* pData, DWORD cchMaxCellCount)
{
	bool lbRc = false;
	LARGE_INTEGER liSize;
	//COORD cr = {};
	uint nX = 0;
	uint nY = 0;

	dump.Close();

	dump.crSize = storedSbi.dwSize;
	dump.crCursor = storedSbi.dwCursorPosition;

	DWORD cchCellCount = storedSbi.dwSize.X * storedSbi.dwSize.Y;
	// ������ �������� "+1", ����� wchar_t* ��� ASCIIZ
	liSize.QuadPart = (cchCellCount + 1) * (sizeof(CharAttr) + sizeof(wchar_t));
	if (!cchCellCount || !liSize.LowPart || liSize.HighPart)
	{
		DisplayLastError(L"Invalid dump file size", -1);
		goto wrap;
	}

	if (!cchMaxCellCount || (cchMaxCellCount < cchCellCount))
	{
		DisplayLastError(L"Invalid max cell count", -1);
		goto wrap;
	}
	
	dump.ptrData = (LPBYTE)malloc(liSize.LowPart);
	if (!dump.ptrData)
	{
		_ASSERTE(dump.ptrData!=NULL);
		goto wrap;
	}
	dump.cbDataSize = liSize.LowPart;

	// �������

	// ������ (� ������������) ����
	dump.pszBlock1 = (wchar_t*)dump.ptrData;
	dump.pszBlock1[cchCellCount] = 0;
	dump.pcaBlock1 = (CharAttr*)(dump.pszBlock1 + cchCellCount + 1);
	dump.Block1 = TRUE;

	
	m_Type = rbt_Alternative;

	
	// ��� ��������� ��������������� ������ ������ ��������� SyncWindow2Console.
	// �������� ������� ������� ������ � �������.
	nX = mp_RCon->TextWidth();
	nY = mp_RCon->TextHeight();

	
	// ����� ��� ������, �������� �������������
	ZeroStruct(con.m_sel);
	con.m_ci.dwSize = 15; con.m_ci.bVisible = TRUE;
	ZeroStruct(con.m_sbi);
	con.m_dwConsoleCP = con.m_dwConsoleOutputCP = 866; con.m_dwConsoleMode = 0;
	con.m_sbi.dwSize = dump.crSize;
	con.m_sbi.dwCursorPosition = dump.crCursor;
	con.m_sbi.wAttributes = 7;

	con.m_sbi.srWindow.Right = nX - 1;
	con.m_sbi.srWindow.Left = 0;
	con.m_sbi.srWindow.Bottom = min((storedSbi.srWindow.Top + (int)nY - 1),(storedSbi.dwSize.Y - 1));
	con.nTopVisibleLine = con.m_sbi.srWindow.Top = max(0,con.m_sbi.srWindow.Bottom - nY + 1);
	
	con.crMaxSize = mp_RCon->mp_RBuf->con.crMaxSize; //MakeCoord(max(dump.crSize.X,nX),max(dump.crSize.Y,nY));
	con.m_sbi.dwMaximumWindowSize = con.crMaxSize; //dump.crSize;
	con.nTextWidth = nX/*dump.crSize.X*/;
	con.nTextHeight = nY/*dump.crSize.Y*/;
	con.bBufferHeight = (dump.crSize.Y > (int)nY);
	con.nBufferHeight = con.bBufferHeight ? dump.crSize.Y : 0;
	TODO("�������������� ���������");
	
	//dump.NeedApply = TRUE;
	
	// �������� �������
	if (!InitBuffers(0))
	{
		_ASSERTE(FALSE);
		goto wrap;
	}
	else
	// � �����������
	{
		TODO("������ �� ���� ����������� ����� ��� �������, � �� ������ CHAR_INFO");

		const CHAR_INFO* ptrSrc = pData;
		CharAttr* pcaDst = dump.pcaBlock1;
		wchar_t*  pszDst = dump.pszBlock1;
		WORD*     pnaDst = con.pConAttr;
		wchar_t   ch;

		CharAttr lcaTableExt[0x100], lcaTableOrg[0x100]; // crForeColor, crBackColor, nFontIndex, nForeIdx, nBackIdx, crOrigForeColor, crOrigBackColor
		PrepareColorTable(false/*bExtendFonts*/, lcaTableExt, lcaTableOrg);

		DWORD nMax = min(cchCellCount,cchMaxCellCount);
		// ���������� ������ �� ���������� ��������
		for (DWORD n = 0; n < nMax; n++, ptrSrc++, pszDst++, pcaDst++, pnaDst++)
		{
			ch = ptrSrc->Char.UnicodeChar;
			//2009-09-25. ��������� (������?) ��������� ���������� �������� � ������� ������� (ASC<32)
			//            �� ����� �������� �� ��������� �������
			*pszDst = (ch < 32) ? gszAnalogues[(WORD)ch] : ch;

			*pcaDst = lcaTableOrg[ptrSrc->Attributes & 0xFF];

			//WARNING("������, ��� ��� ������ �� ���������, ��� ����� ����� ����������");
			//*pnaDst = ptrSrc->Attributes;
		}

		if (cchCellCount > cchMaxCellCount)
		{
			_ASSERTE(cchCellCount <= cchMaxCellCount);
			for (DWORD n = nMax; n < cchCellCount; n++, pszDst++, pcaDst++)
			{
				*pszDst = L' ';
				*pcaDst = lcaTableOrg[0];
			}
		}
		
		//WARNING("������, ��� ��� ������ �� ���������, ��� ����� ����� ����������");
		//wmemmove(con.pConChar, dump.pszBlock1, cchCellCount);
	}

	con.bConsoleDataChanged = TRUE;

	lbRc = true; // OK
wrap:
	if (!lbRc)
		dump.Close();
	return lbRc;
}

// 1 - Last long console output, 2 - current console "image", 3 - copy of rbt_Primary
bool CRealBuffer::LoadAlternativeConsole(LoadAltMode iMode /*= lam_Default*/)
{
	bool lbRc = false;
	
	con.m_sel.dwFlags = 0;
	dump.Close();

	if (iMode == lam_Default)
	{
		iMode = (mp_RCon->isFar() && gpSet->AutoBufferHeight) ? lam_LastOutput : lam_FullBuffer;
	}

	if (iMode == lam_LastOutput)
	{
		MFileMapping<CESERVER_CONSAVE_MAPHDR> StoredOutputHdr;
		MFileMapping<CESERVER_CONSAVE_MAP> StoredOutputItem;

		CESERVER_CONSAVE_MAPHDR* pHdr = NULL;
		CESERVER_CONSAVE_MAP* pData = NULL;
		CONSOLE_SCREEN_BUFFER_INFO storedSbi = {};
		DWORD cchMaxBufferSize = 0;
		size_t nMaxSize = 0;

		StoredOutputHdr.InitName(CECONOUTPUTNAME, (DWORD)mp_RCon->hConWnd); //-V205
		if (!(pHdr = StoredOutputHdr.Open()) || !pHdr->sCurrentMap[0])
		{
			DisplayLastError(L"Stored output mapping was not created!");
			goto wrap;
		}

		cchMaxBufferSize = min(pHdr->MaxCellCount, (DWORD)(pHdr->info.dwSize.X * pHdr->info.dwSize.Y));

		StoredOutputItem.InitName(pHdr->sCurrentMap); //-V205
		nMaxSize = sizeof(*pData) + cchMaxBufferSize * sizeof(pData->Data[0]);
		if (!(pData = StoredOutputItem.Open(FALSE,nMaxSize)))
		{
			DisplayLastError(L"Stored output data mapping was not created!");
			goto wrap;
		}

		if ((pData->hdr.nVersion != CESERVER_REQ_VER) || (pData->hdr.cbSize <= sizeof(CESERVER_CONSAVE_MAP)))
		{
			DisplayLastError(L"Invalid data in mapping header", -1);
			goto wrap;
		}

		storedSbi = pData->info;

		lbRc = LoadDataFromDump(storedSbi, pData->Data, cchMaxBufferSize);
	}
	else if (iMode == lam_FullBuffer)
	{
		CESERVER_REQ *pIn = ExecuteNewCmd(CECMD_CONSOLEFULL, sizeof(CESERVER_REQ_HDR));
		if (pIn)
		{
			CESERVER_REQ *pOut = ExecuteSrvCmd(mp_RCon->GetServerPID(), pIn, ghWnd);
			if (pOut && (pOut->hdr.cbSize > sizeof(CESERVER_CONSAVE_MAP)))
			{
				const CESERVER_CONSAVE_MAP* pInfo = (const CESERVER_CONSAVE_MAP*)pOut;
				// Go
				lbRc = LoadDataFromDump(pInfo->info, pInfo->Data, pInfo->MaxCellCount);
			}
			ExecuteFreeResult(pOut);
			ExecuteFreeResult(pIn);
		}
	}
	else
	{
		// Unsupported yet...
		_ASSERTE(iMode==lam_LastOutput || iMode==lam_FullBuffer);
	}

wrap:
	if (!lbRc)
		dump.Close();
	return lbRc;
}

BOOL CRealBuffer::SetConsoleSizeSrv(USHORT sizeX, USHORT sizeY, USHORT sizeBuffer, DWORD anCmdID/*=CECMD_SETSIZESYNC*/)
{
	if (!this) return FALSE;

	_ASSERTE(m_Type == rbt_Primary);
	#ifdef _DEBUG
	int nVConNo = gpConEmu->isVConValid(mp_RCon->VCon());
	#endif

	if (!mp_RCon->hConWnd || mp_RCon->ms_ConEmuC_Pipe[0] == 0)
	{
		// 19.06.2009 Maks - ��� ������������� ����� ���� ��� �� �������
		//Box(_T("Console was not created (CRealConsole::SetConsoleSize)"));
		DEBUGSTRSIZE(L"SetConsoleSize skipped (!mp_RCon->hConWnd || !mp_RCon->ms_ConEmuC_Pipe)\n");

		if (gpSetCls->isAdvLogging) mp_RCon->LogString("SetConsoleSize skipped (!mp_RCon->hConWnd || !mp_RCon->ms_ConEmuC_Pipe)");

		return FALSE; // ������� ���� �� �������?
	}

	BOOL lbRc = FALSE;
	BOOL fSuccess = FALSE;
	DWORD dwRead = 0;
	DWORD dwTickStart = 0;
	DWORD nCallTimeout = 0;
	int nInSize = sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_SETSIZE);
	int nOutSize = sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_RETSIZE);
	CESERVER_REQ* pIn = ExecuteNewCmd(anCmdID, nInSize);
	CESERVER_REQ* pOut = ExecuteNewCmd(anCmdID, nOutSize);
	SMALL_RECT rect = {0};
	bool bLargestReached = false;
	bool bSecondTry = false;

	_ASSERTE(anCmdID==CECMD_SETSIZESYNC || anCmdID==CECMD_SETSIZENOSYNC || anCmdID==CECMD_CMDSTARTED || anCmdID==CECMD_CMDFINISHED);
	//ExecutePrepareCmd(&lIn.hdr, anCmdID, lIn.hdr.cbSize);
	if (!pIn || !pOut)
	{
		_ASSERTE(pIn && pOut);
		goto wrap;
	}

	// ��� ������ BufferHeight ����� �������� ��� � ������� ������������� (����� ������ ������� ����������?)
	if (con.bBufferHeight)
	{
		// case: buffer mode: change buffer
		CONSOLE_SCREEN_BUFFER_INFO sbi = con.m_sbi;
		sbi.dwSize.X = sizeX; // new
		sizeBuffer = BufferHeight(sizeBuffer); // ���� ����� - ���������������
		_ASSERTE(sizeBuffer > 0);
		sbi.dwSize.Y = sizeBuffer;
		rect.Top = sbi.srWindow.Top;
		rect.Left = sbi.srWindow.Left;
		rect.Right = rect.Left + sizeX - 1;
		rect.Bottom = rect.Top + sizeY - 1;

		if (rect.Right >= sbi.dwSize.X)
		{
			int shift = sbi.dwSize.X - 1 - rect.Right;
			rect.Left += shift;
			rect.Right += shift;
		}

		if (rect.Bottom >= sbi.dwSize.Y)
		{
			int shift = sbi.dwSize.Y - 1 - rect.Bottom;
			rect.Top += shift;
			rect.Bottom += shift;
		}

		// � size �������� ������� �������
		//sizeY = TextHeight(); -- sizeY ���(!) ������ ��������� ��������� ������ ������� �������!
	}
	else
	{
		_ASSERTE(sizeBuffer == 0);
	}

	_ASSERTE(sizeY>0 && sizeY<200);
	// ������������� ��������� ��� �������� � ConEmuC
	pIn->SetSize.nBufferHeight = sizeBuffer; //con.bBufferHeight ? gpSet->Default BufferHeight : 0;
	pIn->SetSize.size.X = sizeX;
	pIn->SetSize.size.Y = sizeY;
	TODO("nTopVisibleLine ������ ������������ ��� �������, � �� ��� �������!");
	#ifdef SHOW_AUTOSCROLL
	pIn->SetSize.nSendTopLine = (gpSetCls->AutoScroll || !con.bBufferHeight) ? -1 : con.nTopVisibleLine;
	#else
	pIn->SetSize.nSendTopLine = mp_RCon->InScroll() ? con.nTopVisibleLine : -1;
	#endif
	pIn->SetSize.rcWindow = rect;
	pIn->SetSize.dwFarPID = (con.bBufferHeight && !mp_RCon->isFarBufferSupported()) ? 0 : mp_RCon->GetFarPID(TRUE);

	// ���� ������ ������ �� ����� - �� � CallNamedPipe �� ������
	//if (mp_ConsoleInfo) {

	// ���� ������������� ������� ������� ������ - ��������� ������
	if (anCmdID != CECMD_CMDFINISHED && pIn->SetSize.nSendTopLine == -1)
	{
		// ����� - ��������� ������� ������������
		//CONSOLE_SCREEN_BUFFER_INFO sbi = mp_ConsoleInfo->sbi;
		bool lbSizeMatch = true;

		if (con.bBufferHeight)
		{
			if (con.m_sbi.dwSize.X != sizeX || con.m_sbi.dwSize.Y != sizeBuffer)
				lbSizeMatch = false;
			else if (con.m_sbi.srWindow.Top != rect.Top || con.m_sbi.srWindow.Bottom != rect.Bottom)
				lbSizeMatch = false;
		}
		else
		{
			if (con.m_sbi.dwSize.X != sizeX || con.m_sbi.dwSize.Y != sizeY)
				lbSizeMatch = false;
		}

		// fin
		if (lbSizeMatch && anCmdID != CECMD_CMDFINISHED)
		{
			lbRc = TRUE; // ������ ������ �� �����
			goto wrap;
		}

		// ��� ����� ������� ������ - �������� ��������� ������� �������
		ResetLastMousePos();
	}

	//}

	if (gpSetCls->isAdvLogging)
	{
		char szInfo[128];
		_wsprintfA(szInfo, SKIPLEN(countof(szInfo)) "%s(Cols=%i, Rows=%i, Buf=%i, Top=%i)",
		           (anCmdID==CECMD_SETSIZESYNC) ? "CECMD_SETSIZESYNC" :
		           (anCmdID==CECMD_CMDSTARTED) ? "CECMD_CMDSTARTED" :
		           (anCmdID==CECMD_CMDFINISHED) ? "CECMD_CMDFINISHED" :
		           "UnknownSizeCommand", sizeX, sizeY, sizeBuffer, pIn->SetSize.nSendTopLine);
		mp_RCon->LogString(szInfo, TRUE);
	}

	#ifdef _DEBUG
	wchar_t szDbgCmd[128]; _wsprintf(szDbgCmd, SKIPLEN(countof(szDbgCmd)) L"SetConsoleSize.CallNamedPipe(cx=%i, cy=%i, buf=%i, cmd=%i)\n",
	                                 sizeX, sizeY, sizeBuffer, anCmdID);
	DEBUGSTRSIZE(szDbgCmd);
	#endif

	dwTickStart = timeGetTime();
	// � ���������
	nCallTimeout = RELEASEDEBUGTEST(500,30000);

	fSuccess = CallNamedPipe(mp_RCon->ms_ConEmuC_Pipe, pIn, pIn->hdr.cbSize, pOut, pOut->hdr.cbSize, &dwRead, nCallTimeout);

	if (fSuccess && (dwRead == (DWORD)nOutSize))
	{
		int nSetWidth = sizeX, nSetHeight = sizeY;
		if (GetConWindowSize(pOut->SetSizeRet.SetSizeRet, &nSetWidth, &nSetHeight, NULL))
		{
			// If change-size (enlarging) was failed
			if ((sizeX > (UINT)nSetWidth) || (sizeY > (UINT)nSetHeight))
			{
				// Check width
				if ((pOut->SetSizeRet.crMaxSize.X > nSetWidth) && (sizeX > (UINT)nSetWidth))
				{
					bSecondTry = true;
					pIn->SetSize.size.X = pOut->SetSizeRet.crMaxSize.X;
					pIn->SetSize.rcWindow.Right = pIn->SetSize.rcWindow.Left + pIn->SetSize.size.X - 1;
				}
				// And height
				if ((pOut->SetSizeRet.crMaxSize.Y > nSetHeight) && (sizeY > (UINT)nSetHeight))
				{
					bSecondTry = true;
					pIn->SetSize.size.Y = pOut->SetSizeRet.crMaxSize.Y;
					pIn->SetSize.rcWindow.Bottom = pIn->SetSize.rcWindow.Top + pIn->SetSize.size.Y - 1;
				}
				// Try again with lesser size?
				if (bSecondTry)
				{
					fSuccess = CallNamedPipe(mp_RCon->ms_ConEmuC_Pipe, pIn, pIn->hdr.cbSize, pOut, pOut->hdr.cbSize, &dwRead, nCallTimeout);
				}
				// Inform user
				Icon.ShowTrayIcon(L"Maximum real console size was reached\nDecrease font size in the real console properties", tsa_Console_Size);
			}
		}
	}

	gpSetCls->debugLogCommand(pIn, FALSE, dwTickStart, timeGetTime()-dwTickStart, mp_RCon->ms_ConEmuC_Pipe, pOut);

	if (!fSuccess || dwRead<(DWORD)nOutSize)
	{
		if (gpSetCls->isAdvLogging)
		{
			char szInfo[128]; DWORD dwErr = GetLastError();
			_wsprintfA(szInfo, SKIPLEN(countof(szInfo)) "SetConsoleSizeSrv.CallNamedPipe FAILED!!! ErrCode=0x%08X, Bytes read=%i", dwErr, dwRead);
			mp_RCon->LogString(szInfo);
		}

		DEBUGSTRSIZE(L"SetConsoleSize.CallNamedPipe FAILED!!!\n");
	}
	else
	{
		_ASSERTE(mp_RCon->m_ConsoleMap.IsValid());
		bool bNeedApplyConsole = //mp_ConsoleInfo &&
		    mp_RCon->m_ConsoleMap.IsValid()
		    && (anCmdID == CECMD_SETSIZESYNC)
		    && (mp_RCon->mn_MonitorThreadID != GetCurrentThreadId());
		DEBUGSTRSIZE(L"SetConsoleSize.fSuccess == TRUE\n");

		if (pOut->hdr.nCmd != pIn->hdr.nCmd)
		{
			_ASSERTE(pOut->hdr.nCmd == pIn->hdr.nCmd);

			if (gpSetCls->isAdvLogging)
			{
				char szInfo[128]; _wsprintfA(szInfo, SKIPLEN(countof(szInfo)) "SetConsoleSizeSrv FAILED!!! OutCmd(%i)!=InCmd(%i)", pOut->hdr.nCmd, pIn->hdr.nCmd);
				mp_RCon->LogString(szInfo);
			}
		}
		else
		{
			//con.nPacketIdx = lOut.SetSizeRet.nNextPacketId;
			//mn_LastProcessedPkt = lOut.SetSizeRet.nNextPacketId;
			CONSOLE_SCREEN_BUFFER_INFO sbi = {{0,0}};
			int nBufHeight;

			//_ASSERTE(mp_ConsoleInfo);
			if (bNeedApplyConsole /*&& mp_ConsoleInfo->nCurDataMapIdx && (HWND)mp_ConsoleInfo->mp_RCon->hConWnd*/)
			{
				// ���� Apply ��� �� ������ - ����
				DWORD nWait = -1;

				if (con.m_sbi.dwSize.X != sizeX || con.m_sbi.dwSize.Y != (sizeBuffer ? sizeBuffer : sizeY))
				{
					//// �������� ��������� (��������) �����, ���� ��������� FileMapping � ����� ��������
					//_ASSERTE(mp_ConsoleInfo!=NULL);
					//COORD crCurSize = mp_ConsoleInfo->sbi.dwSize;
					//if (crCurSize.X != sizeX || crCurSize.Y != (sizeBuffer ? sizeBuffer : sizeY))
					//{
					//	DWORD dwStart = GetTickCount();
					//	do {
					//		Sleep(1);
					//		crCurSize = mp_ConsoleInfo->sbi.dwSize;
					//		if (crCurSize.X == sizeX && crCurSize.Y == (sizeBuffer ? sizeBuffer : sizeY))
					//			break;
					//	} while ((GetTickCount() - dwStart) < SETSYNCSIZEMAPPINGTIMEOUT);
					//}
					ResetEvent(mp_RCon->mh_ApplyFinished);
					mp_RCon->mn_LastConsolePacketIdx--;
					mp_RCon->SetMonitorThreadEvent();
					DWORD nWaitTimeout = SETSYNCSIZEAPPLYTIMEOUT;
					
					#ifdef _DEBUG
					nWaitTimeout = SETSYNCSIZEAPPLYTIMEOUT*4; //30000;
					nWait = WaitForSingleObject(mp_RCon->mh_ApplyFinished, nWaitTimeout);
					if (nWait == WAIT_TIMEOUT)
					{
						_ASSERTE(FALSE && "SETSYNCSIZEAPPLYTIMEOUT");
						//nWait = WaitForSingleObject(mp_RCon->mh_ApplyFinished, nWaitTimeout);
					}
					#else
					nWait = WaitForSingleObject(mp_RCon->mh_ApplyFinished, nWaitTimeout);
					#endif
					
					COORD crDebugCurSize = con.m_sbi.dwSize;

					if (crDebugCurSize.X != sizeX)
					{
						if (gpSetCls->isAdvLogging)
						{
							char szInfo[128]; _wsprintfA(szInfo, SKIPLEN(countof(szInfo)) "SetConsoleSize FAILED!!! ReqSize={%ix%i}, OutSize={%ix%i}", sizeX, (sizeBuffer ? sizeBuffer : sizeY), crDebugCurSize.X, crDebugCurSize.Y);
							mp_RCon->LogString(szInfo);
						}

#ifdef _DEBUG
						_ASSERTE(crDebugCurSize.X == sizeX);
#endif
					}
				}

				if (gpSetCls->isAdvLogging)
				{
					mp_RCon->LogString(
					    (nWait == (DWORD)-1) ?
					    "SetConsoleSizeSrv: does not need wait" :
					    (nWait != WAIT_OBJECT_0) ?
					    "SetConsoleSizeSrv.WaitForSingleObject(mp_RCon->mh_ApplyFinished) TIMEOUT!!!" :
					    "SetConsoleSizeSrv.WaitForSingleObject(mp_RCon->mh_ApplyFinished) succeded");
				}

				sbi = con.m_sbi;
				nBufHeight = con.nBufferHeight;
			}
			else
			{
				sbi = pOut->SetSizeRet.SetSizeRet;
				nBufHeight = pIn->SetSize.nBufferHeight;

				if (gpSetCls->isAdvLogging)
					mp_RCon->LogString("SetConsoleSizeSrv.Not waiting for ApplyFinished");
			}

			WARNING("!!! ����� ����� ��������� _ASSERTE'�. ������ ������ ������ �������� � ������ ���� � con.bBufferHeight ������ �� ��������?");

			if (con.bBufferHeight)
			{
				_ASSERTE((sbi.srWindow.Bottom-sbi.srWindow.Top)<200);

				if (gpSetCls->isAdvLogging)
				{
					char szInfo[128]; _wsprintfA(szInfo, SKIPLEN(countof(szInfo)) "Current size: Cols=%i, Buf=%i", sbi.dwSize.X, sbi.dwSize.Y);
					mp_RCon->LogString(szInfo, TRUE);
				}

				if (sbi.dwSize.X == sizeX && sbi.dwSize.Y == nBufHeight)
				{
					lbRc = TRUE;
				}
				else
				{
					if (gpSetCls->isAdvLogging)
					{
						char szInfo[128]; _wsprintfA(szInfo, SKIPLEN(countof(szInfo)) "SetConsoleSizeSrv FAILED! Ask={%ix%i}, Cur={%ix%i}, Ret={%ix%i}",
						                             sizeX, sizeY,
						                             sbi.dwSize.X, sbi.dwSize.Y,
						                             pOut->SetSizeRet.SetSizeRet.dwSize.X, pOut->SetSizeRet.SetSizeRet.dwSize.Y
						                            );
						mp_RCon->LogString(szInfo);
					}

					lbRc = FALSE;
				}
			}
			else
			{
				if (sbi.dwSize.Y > 200)
				{
					_ASSERTE(sbi.dwSize.Y<200);
				}

				if (gpSetCls->isAdvLogging)
				{
					char szInfo[128]; _wsprintfA(szInfo, SKIPLEN(countof(szInfo)) "Current size: Cols=%i, Rows=%i", sbi.dwSize.X, sbi.dwSize.Y);
					mp_RCon->LogString(szInfo, TRUE);
				}

				if (sbi.dwSize.X == sizeX && sbi.dwSize.Y == sizeY)
				{
					lbRc = TRUE;
				}
				else
				{
					if (gpSetCls->isAdvLogging)
					{
						char szInfo[128]; _wsprintfA(szInfo, SKIPLEN(countof(szInfo)) "SetConsoleSizeSrv FAILED! Ask={%ix%i}, Cur={%ix%i}, Ret={%ix%i}",
						                             sizeX, sizeY,
						                             sbi.dwSize.X, sbi.dwSize.Y,
						                             pOut->SetSizeRet.SetSizeRet.dwSize.X, pOut->SetSizeRet.SetSizeRet.dwSize.Y
						                            );
						mp_RCon->LogString(szInfo);
					}

					lbRc = FALSE;
				}
			}

			//if (sbi.dwSize.X == size.X && sbi.dwSize.Y == size.Y) {
			//    con.m_sbi = sbi;
			//    if (sbi.dwSize.X == con.m_sbi.dwSize.X && sbi.dwSize.Y == con.m_sbi.dwSize.Y) {
			//        SetEvent(mh_ConChanged);
			//    }
			//    lbRc = true;
			//}
			if (lbRc)  // ��������� ����� ������ nTextWidth/nTextHeight. ����� ������������� �������� ������� ������...
			{
				DEBUGSTRSIZE(L"SetConsoleSizeSrv.lbRc == TRUE\n");
				con.nChange2TextWidth = sbi.dwSize.X;
				con.nChange2TextHeight = con.bBufferHeight ? (sbi.srWindow.Bottom-sbi.srWindow.Top+1) : sbi.dwSize.Y;
#ifdef _DEBUG

				if (con.bBufferHeight)
					_ASSERTE(con.nBufferHeight == sbi.dwSize.Y);

#endif
				con.nBufferHeight = con.bBufferHeight ? sbi.dwSize.Y : 0;

				if (con.nChange2TextHeight > 200)
				{
					_ASSERTE(con.nChange2TextHeight<=200);
				}
			}
		}
	}

wrap:
	ExecuteFreeResult(pIn);
	ExecuteFreeResult(pOut);
	return lbRc;
}

BOOL CRealBuffer::SetConsoleSize(USHORT sizeX, USHORT sizeY, USHORT sizeBuffer, DWORD anCmdID/*=CECMD_SETSIZESYNC*/)
{
	if (!this) return FALSE;

	// ���� ���� ���������� DC - ������� ��
	mp_RCon->mp_VCon->LockDcRect(FALSE, NULL);

	if (m_Type != rbt_Primary)
	{
		// ��� �������������� ������ "��������" ����������
		con.nTextWidth = sizeX;
		con.nTextHeight = sizeY;
		con.nBufferHeight = sizeBuffer;
		con.bBufferHeight = (sizeBuffer != 0);
		ChangeScreenBufferSize(con.m_sbi, sizeX, sizeY, sizeX, sizeBuffer ? sizeBuffer : sizeY);

		if (mp_RCon->isActive())
		{
			gpConEmu->mp_Status->OnConsoleChanged(&con.m_sbi, &con.m_ci, true);
		}

		return TRUE;
	}

	//_ASSERTE(mp_RCon->hConWnd && mp_RCon->ms_ConEmuC_Pipe[0]);

	if (!mp_RCon->hConWnd || mp_RCon->ms_ConEmuC_Pipe[0] == 0)
	{
		// 19.06.2009 Maks - ��� ������������� ����� ���� ��� �� �������
		//Box(_T("Console was not created (CRealConsole::SetConsoleSize)"));
		if (gpSetCls->isAdvLogging) mp_RCon->LogString("SetConsoleSize skipped (!mp_RCon->hConWnd || !mp_RCon->ms_ConEmuC_Pipe)");

		DEBUGSTRSIZE(L"SetConsoleSize skipped (!mp_RCon->hConWnd || !mp_RCon->ms_ConEmuC_Pipe)\n");
		return false; // ������� ���� �� �������?
	}

	HEAPVAL
	_ASSERTE(sizeX>=MIN_CON_WIDTH && sizeY>=MIN_CON_HEIGHT);

	if (sizeX</*4*/MIN_CON_WIDTH)
		sizeX = /*4*/MIN_CON_WIDTH;

	if (sizeY</*3*/MIN_CON_HEIGHT)
		sizeY = /*3*/MIN_CON_HEIGHT;

	_ASSERTE(con.bBufferHeight || (!con.bBufferHeight && !sizeBuffer));

	if (con.bBufferHeight && !sizeBuffer)
		sizeBuffer = BufferHeight(sizeBuffer);

	_ASSERTE(!con.bBufferHeight || (sizeBuffer >= sizeY));
	BOOL lbRc = FALSE;
	#ifdef _DEBUG
	DWORD dwPID = mp_RCon->GetFarPID(TRUE);
	#endif
	//if (dwPID)
	//{
	//	// ���� ��� ������ FAR (��� Synchro) - ����� ���� �� ��������� ����� ������?
	//	// ���� ��� ���� � ���, ��� ���� ��������� � ���� ���� ���������, ��
	//	// ������� �� ������� ���������� ������ ����� ���������� �������
	//	if (!mb_PluginDetected || dwPID != mn_FarPID_PluginDetected)
	//		dwPID = 0;
	//}
	_ASSERTE(sizeY <= 300);
	/*if (!con.bBufferHeight && dwPID)
		lbRc = SetConsoleSizePlugin(sizeX, sizeY, sizeBuffer, anCmdID);
	else*/
	HEAPVAL;
	// ����� �� ����� ������� ������ �� ��������������
	ResetEvent(con.hInSetSize); con.bInSetSize = TRUE;
	lbRc = SetConsoleSizeSrv(sizeX, sizeY, sizeBuffer, anCmdID);
	con.bInSetSize = FALSE; SetEvent(con.hInSetSize);
	HEAPVAL;

#if 0
	if (lbRc && mp_RCon->isActive())
	{
		if (!mp_RCon->isNtvdm())
		{
			// update size info
			if (gpConEmu->isWindowNormal())
			{
				int nHeight = TextHeight();
				gpSetCls->UpdateSize(sizeX, nHeight);
			}
		}

		// -- ������ ����������� ��� ApplyData
		//gpConEmu->UpdateStatusBar();
	}
#endif

	HEAPVAL;
	DEBUGSTRSIZE(L"SetConsoleSize.finalizing\n");
	return lbRc;
}

void CRealBuffer::SyncConsole2Window(USHORT wndSizeX, USHORT wndSizeY)
{
	// �� ��������� ������ �������� �� � ������������...
	if (con.nTextWidth != wndSizeX || con.nTextHeight != wndSizeY)
	{
		if (IFLOGCONSOLECHANGE)
		{
			char szInfo[128]; _wsprintfA(szInfo, SKIPLEN(countof(szInfo)) "CRealBuffer::SyncConsole2Window(Cols=%i, Rows=%i, Current={%i,%i})", wndSizeX, wndSizeY, con.nTextWidth, con.nTextHeight);
			mp_RCon->LogString(szInfo);
		}

		#ifdef _DEBUG
		if (wndSizeX == 80)
		{
			int nDbg = wndSizeX;
		}
		#endif
		
		// ����� ��������, ����� � �������� ���� ��� ������������� ������ �� ������
		// ���������� ������������� RefreshThread, ����� �� ��������� ApplyConsoleInfo �� ���������� SetConsoleSize
		con.bLockChange2Text = TRUE;
		con.nChange2TextWidth = wndSizeX; con.nChange2TextHeight = wndSizeY;
		SetConsoleSize(wndSizeX, wndSizeY, 0/*Auto*/);
		con.bLockChange2Text = FALSE;

		if (mp_RCon->isActive() && gpConEmu->isMainThread())
		{
			// ����� �������� DC ����� ��������������� Width & Height
			mp_RCon->mp_VCon->OnConsoleSizeChanged();
		}
	}
}

BOOL CRealBuffer::isScroll(RealBufferScroll aiScroll/*=rbs_Any*/)
{
	TODO("�������������� ���������");
	return con.bBufferHeight;
}

// ���������� ��� ������ (CRealConsole::AttachConemuC)
void CRealBuffer::InitSBI(CONSOLE_SCREEN_BUFFER_INFO* ap_sbi)
{
	con.m_sbi = *ap_sbi;
}

void CRealBuffer::InitMaxSize(const COORD& crMaxSize)
{
	con.crMaxSize = crMaxSize;
}

COORD CRealBuffer::GetMaxSize()
{
	return con.crMaxSize;
}

bool CRealBuffer::isInitialized()
{
	if (!con.pConChar || !con.nTextWidth || con.nTextHeight<2)
		return false; // ������� �� ����������������, ������ ������
	return true;
}

bool CRealBuffer::isFarMenuOrMacro()
{
	BOOL lbMenuActive = false;
	MSectionLock cs; cs.Lock(&csCON);

	WARNING("CantActivateInfo: ������ �� ��� ����������� ����� 'Can't activate tab' ������� '������'");

	if (con.pConChar && con.pConAttr)
	{
		TODO("������ �� ������� � ���� ������, ��������� �� �� ������");
		if (((con.pConChar[0] == L'R') && ((con.pConAttr[0] & 0xFF) == 0x4F))
			|| ((con.pConChar[0] == L'P') && ((con.pConAttr[0] & 0xFF) == 0x2F)))
		{
			// ������ �������. �������� �������� �������������?
			lbMenuActive = true;
		}
		else if (con.pConChar[0] == L' ' && con.pConChar[con.nTextWidth] == ucBoxDblVert)
		{
			lbMenuActive = true;
		}
		else if (con.pConChar[0] == L' ' && (con.pConChar[con.nTextWidth] == ucBoxDblDownRight ||
		                                    (con.pConChar[con.nTextWidth] == '['
		                                     && (con.pConChar[con.nTextWidth+1] >= L'0' && con.pConChar[con.nTextWidth+1] <= L'9'))))
		{
			// ������ ���� ������ �����. ���������, ������� �� ����.
			for(int x=1; !lbMenuActive && x<con.nTextWidth; x++)
			{
				if (con.pConAttr[x] != con.pConAttr[0])  // ���������� ���� - �� ��������������
					lbMenuActive = true;
			}
		}
		else
		{
			// ���� ������ ���� ������ �� �����, � ������ ���������
			wchar_t* pszLine = con.pConChar + con.nTextWidth;

			for(int x=1; !lbMenuActive && x<(con.nTextWidth-10); x++)
			{
				if (pszLine[x] == ucBoxDblDownRight && pszLine[x+1] == ucBoxDblHorz)
					lbMenuActive = true;
			}
		}
	}

	cs.Unlock();
	
	return lbMenuActive;
}

BOOL CRealBuffer::isConsoleDataChanged()
{
	if (!this) return FALSE;
	return con.bConsoleDataChanged;
}

BOOL CRealBuffer::PreInit()
{
	MCHKHEAP;
	// ���������������� ���������� m_sbi, m_ci, m_sel
	//RECT rcWnd; Get ClientRect(ghWnd, &rcWnd);
	
	// mp_RCon->isBufferHeight ������������ ������, �.�. mp_RBuf.m_sbi ��� �� ���������������!
	bool bNeedBufHeight = (gpSetCls->bForceBufferHeight && gpSetCls->nForceBufferHeight>0)
	                      || (!gpSetCls->bForceBufferHeight && mp_RCon->mn_DefaultBufferHeight);

	if (bNeedBufHeight && !isScroll())
	{
		MCHKHEAP;
		SetBufferHeightMode(TRUE);
		MCHKHEAP;
		_ASSERTE(mp_RCon->mn_DefaultBufferHeight>0);
		BufferHeight(mp_RCon->mn_DefaultBufferHeight);
	}

	MCHKHEAP;
	RECT rcCon;

	if (gpConEmu->isIconic())
		rcCon = MakeRect(gpConEmu->wndWidth, gpConEmu->wndHeight);
	else
		rcCon = gpConEmu->CalcRect(CER_CONSOLE_CUR, mp_RCon->mp_VCon);

	_ASSERTE(rcCon.right!=0 && rcCon.bottom!=0);

	if (con.bBufferHeight)
	{
		_ASSERTE(mp_RCon->mn_DefaultBufferHeight>0);
		con.m_sbi.dwSize = MakeCoord(rcCon.right,mp_RCon->mn_DefaultBufferHeight);
	}
	else
	{
		con.m_sbi.dwSize = MakeCoord(rcCon.right,rcCon.bottom);
	}
	con.nTextWidth = rcCon.right;
	con.nTextHeight = rcCon.bottom;

	con.m_sbi.wAttributes = 7;
	con.m_sbi.srWindow.Right = rcCon.right-1; con.m_sbi.srWindow.Bottom = rcCon.bottom-1;
	con.m_sbi.dwMaximumWindowSize = con.m_sbi.dwSize;
	con.m_ci.dwSize = 15; con.m_ci.bVisible = TRUE;

	if (!InitBuffers(0))
		return FALSE;

	return TRUE;
}

BOOL CRealBuffer::InitBuffers(DWORD OneBufferSize)
{
	// ��� ������� ������ ���������� ������ � MonitorThread.
	// ����� ���������� ������ �� �����������
	
	#ifdef _DEBUG
	DWORD dwCurThId = GetCurrentThreadId();
	_ASSERTE(mp_RCon->mn_MonitorThreadID==0 || dwCurThId==mp_RCon->mn_MonitorThreadID
		|| ((m_Type==rbt_DumpScreen || m_Type==rbt_Alternative || m_Type==rbt_Selection || m_Type==rbt_Find) && gpConEmu->isMainThread()));
	#endif

	BOOL lbRc = FALSE;
	int nNewWidth = 0, nNewHeight = 0;
	DWORD nScroll = 0;
	MCHKHEAP;

	if (!GetConWindowSize(con.m_sbi, &nNewWidth, &nNewHeight, &nScroll))
		return FALSE;

	if (OneBufferSize)
	{
		if ((nNewWidth * nNewHeight * sizeof(*con.pConChar)) != OneBufferSize)
		{
			// ��� ����� ��������� �� ����� ������������ ������� (����� ��� �����)
			//// ��� ����� ��������� �� ����� �������
			//nNewWidth = nNewWidth;
			_ASSERTE((nNewWidth * nNewHeight * sizeof(*con.pConChar)) == OneBufferSize);
		}
		else if (con.nTextWidth == nNewWidth && con.nTextHeight == nNewHeight)
		{
			// �� ����� ��� ������������� ������ � ������
			if (con.pConChar!=NULL && con.pConAttr!=NULL && con.pDataCmp!=NULL)
			{
				return TRUE;
			}
		}

		//if ((nNewWidth * nNewHeight * sizeof(*con.pConChar)) != OneBufferSize)
		//    return FALSE;
	}

	// ���� ��������� ��������� ��� ������� (��������) ������
	if (!con.pConChar || (con.nTextWidth*con.nTextHeight) < (nNewWidth*nNewHeight))
	{
		MSectionLock sc; sc.Lock(&csCON, TRUE);
		MCHKHEAP;
		con.LastStartInitBuffersTick = GetTickCount();

		if (con.pConChar)
			{ Free(con.pConChar); con.pConChar = NULL; }

		if (con.pConAttr)
			{ Free(con.pConAttr); con.pConAttr = NULL; }

		if (con.pDataCmp)
			{ Free(con.pDataCmp); con.pDataCmp = NULL; }

		//if (con.pCmp)
		//	{ Free(con.pCmp); con.pCmp = NULL; }
		MCHKHEAP;
		int cchCharMax = nNewWidth * nNewHeight * 2;
		con.pConChar = (TCHAR*)Alloc(cchCharMax, sizeof(*con.pConChar));
		con.pConAttr = (WORD*)Alloc(cchCharMax, sizeof(*con.pConAttr));
		con.pDataCmp = (CHAR_INFO*)Alloc((nNewWidth * nNewHeight)*sizeof(CHAR_INFO),1);
		//con.pCmp = (CESERVER_REQ_CONINFO_DATA*)Alloc((nNewWidth * nNewHeight)*sizeof(CHAR_INFO)+sizeof(CESERVER_REQ_CONINFO_DATA),1);

		BYTE nDefTextAttr = (mp_RCon->GetDefaultBackColorIdx()<<4)|(mp_RCon->GetDefaultTextColorIdx());
		wmemset((wchar_t*)con.pConAttr, nDefTextAttr, cchCharMax);

		con.LastEndInitBuffersTick = GetTickCount();

		sc.Unlock();
		_ASSERTE(con.pConChar!=NULL);
		_ASSERTE(con.pConAttr!=NULL);
		_ASSERTE(con.pDataCmp!=NULL);
		//_ASSERTE(con.pCmp!=NULL);
		MCHKHEAP
		lbRc = con.pConChar!=NULL && con.pConAttr!=NULL && con.pDataCmp!=NULL;
	}
	else if (con.nTextWidth!=nNewWidth || con.nTextHeight!=nNewHeight)
	{
		MCHKHEAP
		MSectionLock sc; sc.Lock(&csCON);
		int cchCharMax = nNewWidth * nNewHeight * 2;
		memset(con.pConChar, 0, cchCharMax * sizeof(*con.pConChar));
		//memset(con.pConAttr, 0, cchCharMax * sizeof(*con.pConAttr));
		BYTE nDefTextAttr = (mp_RCon->GetDefaultBackColorIdx()<<4)|(mp_RCon->GetDefaultTextColorIdx());
		wmemset((wchar_t*)con.pConAttr, nDefTextAttr, cchCharMax);
		memset(con.pDataCmp, 0, (nNewWidth * nNewHeight) * sizeof(CHAR_INFO));
		//memset(con.pCmp->Buf, 0, (nNewWidth * nNewHeight) * sizeof(CHAR_INFO));
		sc.Unlock();
		MCHKHEAP
		lbRc = TRUE;
	}
	else
	{
		lbRc = TRUE;
	}

	MCHKHEAP
#ifdef _DEBUG

	if (nNewHeight == 158)
		nNewHeight = 158;

#endif
	con.nTextWidth = nNewWidth;
	con.nTextHeight = nNewHeight;
	// ����� ������������� ��������� ������� � ������ �����
	if (this == mp_RCon->mp_ABuf)
		mp_RCon->mb_DataChanged = TRUE;
	//else
	//{
	//	// ����� �� ������, ��� ����� ������������ ������ ���� ������ ���������
	//	_ASSERTE(this == mp_RCon->mp_ABuf);
	//}

	//InitDC(false,true);
	return lbRc;
}

void CRealBuffer::PreFillBuffers()
{
	if (con.pConChar && con.pConAttr)
	{
		MSectionLock sc; sc.Lock(&csCON, TRUE);

		size_t cchCharMax = (con.nTextWidth*con.nTextHeight) * 2;

		BYTE nDefTextAttr = (mp_RCon->GetDefaultBackColorIdx()<<4)|(mp_RCon->GetDefaultTextColorIdx());
		wmemset((wchar_t*)con.pConAttr, nDefTextAttr, cchCharMax);

		sc.Unlock();
	}
}

SHORT CRealBuffer::GetBufferWidth()
{
	return con.m_sbi.dwSize.X;
}

SHORT CRealBuffer::GetBufferHeight()
{
	return con.m_sbi.dwSize.Y;
}

SHORT CRealBuffer::GetBufferPosX()
{
	_ASSERTE(con.m_sbi.srWindow.Left==0);
	return con.m_sbi.srWindow.Left;
}

SHORT CRealBuffer::GetBufferPosY()
{
	#ifdef _DEBUG
	USHORT nTop = con.nTopVisibleLine;
	CONSOLE_SCREEN_BUFFER_INFO csbi = con.m_sbi;
	bool bInScroll = mp_RCon->InScroll();
	if (nTop != csbi.srWindow.Top)
	{
		TODO("���� �� ��������� ������ �� ���� - ������ ����� ��������� ������� � ��������������");
		_ASSERTE(nTop == csbi.srWindow.Top || bInScroll);
		bool bDbgShowConsole = false;
		if (bDbgShowConsole)
		{
			mp_RCon->ShowConsole(1);
			mp_RCon->ShowConsole(0);
		}
	}
	#endif

	return con.nTopVisibleLine;
}

int CRealBuffer::TextWidth()
{
	_ASSERTE(this!=NULL);

	if (!this) return MIN_CON_WIDTH;

	if (con.nChange2TextWidth!=-1 && con.nChange2TextWidth!=0)
		return con.nChange2TextWidth;

	_ASSERTE(con.nTextWidth>=MIN_CON_WIDTH);
	return con.nTextWidth;
}

int CRealBuffer::GetTextWidth()
{
	_ASSERTE(this!=NULL);

	if (!this) return MIN_CON_WIDTH;

	_ASSERTE(con.nTextWidth>=MIN_CON_WIDTH && con.nTextWidth<=400);
	return con.nTextWidth;
}

int CRealBuffer::TextHeight()
{
	_ASSERTE(this!=NULL);

	if (!this) return MIN_CON_HEIGHT;

	uint nRet = 0;
	#ifdef _DEBUG
	//struct RConInfo lcon = con;
	#endif

	if (con.nChange2TextHeight!=-1 && con.nChange2TextHeight!=0)
		nRet = con.nChange2TextHeight;
	else
		nRet = con.nTextHeight;

	#ifdef _DEBUG
	if (nRet <= MIN_CON_HEIGHT || nRet > 200)
	{
		_ASSERTE(nRet>=MIN_CON_HEIGHT && nRet<=200);
	}
	#endif

	return nRet;
}

int CRealBuffer::GetTextHeight()
{
	_ASSERTE(this!=NULL);

	if (!this) return MIN_CON_HEIGHT;
	
	_ASSERTE(con.nTextHeight>=MIN_CON_HEIGHT && con.nTextHeight<=200);
	return con.nTextHeight;
}

int CRealBuffer::GetWindowWidth()
{
	int nWidth = con.m_sbi.srWindow.Right - con.m_sbi.srWindow.Left + 1;
	_ASSERTE(nWidth>=MIN_CON_WIDTH && nWidth <= 400);
	return nWidth;
}

int CRealBuffer::GetWindowHeight()
{
	int nHeight = con.m_sbi.srWindow.Bottom - con.m_sbi.srWindow.Top + 1;
	_ASSERTE(nHeight>=MIN_CON_HEIGHT && nHeight <= 300);
	return nHeight;
}

int CRealBuffer::BufferHeight(uint nNewBufferHeight/*=0*/)
{
	int nBufferHeight = 0;

	if (con.bBufferHeight)
	{
		if (nNewBufferHeight)
		{
			nBufferHeight = nNewBufferHeight;
			con.nBufferHeight = nNewBufferHeight;
		}
		else if (con.nBufferHeight)
		{
			nBufferHeight = con.nBufferHeight;
		}
		else if (mp_RCon->mn_DefaultBufferHeight)
		{
			nBufferHeight = mp_RCon->mn_DefaultBufferHeight;
		}
		else
		{
			nBufferHeight = gpSet->DefaultBufferHeight;
		}
	}
	else
	{
		// ����� ������ �� ��������� ������ ���������� ����������� ������, �����
		// � ��������� ��� ���������� ������ �� �������� (gpSet->DefaultBufferHeight)
		_ASSERTE(nNewBufferHeight == 0);
		con.nBufferHeight = 0;
	}

	return nBufferHeight;
}

BOOL CRealBuffer::GetConWindowSize(const CONSOLE_SCREEN_BUFFER_INFO& sbi, int* pnNewWidth, int* pnNewHeight, DWORD* pnScroll)
{
	TODO("�������� �� ����� ::GetConWindowSize �� WinObjects.cpp");
	DWORD nScroll = 0; // enum RealBufferScroll
	int nNewWidth = 0, nNewHeight = 0;
	
	// ������� ���������� ������ ���� (������� �������), �� ���� ����� ����� ���� ������
	
	int nCurWidth = TextWidth();
	if (sbi.dwSize.X == nCurWidth)
	{
		nNewWidth = sbi.dwSize.X;
	}
	else
	{
		TODO("�������� � con ������ �������������� ���������");
		TODO("� ������, �������� �� ����� ::GetConWindowSize �� WinObjects.cpp");
		if (sbi.dwSize.X <= EvalBufferTurnOnSize(max(nCurWidth,con.crMaxSize.X)))
		{
			nNewWidth = sbi.dwSize.X;
		}
		else
		{
			// 111125 ���� "nNewWidth  = sbi.dwSize.X;", �� ��� ������������ �������������� ���������
			nNewWidth = sbi.srWindow.Right - sbi.srWindow.Left + 1;
			_ASSERTE(nNewWidth <= sbi.dwSize.X);
		}
	}
	// �����
	if (/*(sbi.dwSize.X > sbi.dwMaximumWindowSize.X) ||*/ (nNewWidth < sbi.dwSize.X))
	{
		// ��� �������� �������
		//_ASSERTE((sbi.dwSize.X > sbi.dwMaximumWindowSize.X) && (nNewWidth < sbi.dwSize.X));
		nScroll |= rbs_Horz;
	}


	int nCurHeight = TextHeight();
	if (sbi.dwSize.Y == nCurHeight)
	{
		nNewHeight = sbi.dwSize.Y;
	}
	else
	{
		if ((con.bBufferHeight && (sbi.dwSize.Y > nCurHeight))
			|| (sbi.dwSize.Y > EvalBufferTurnOnSize(nCurHeight)))
		{
			nNewHeight = sbi.srWindow.Bottom - sbi.srWindow.Top + 1;
		}
		else
		{
			nNewHeight = sbi.dwSize.Y;
		}
	}
	// �����
	if (/*(sbi.dwSize.Y > sbi.dwMaximumWindowSize.Y) ||*/ (nNewHeight < sbi.dwSize.Y))
	{
		// ��� �������� �������
		//_ASSERTE((sbi.dwSize.Y >= sbi.dwMaximumWindowSize.Y) && (nNewHeight < sbi.dwSize.Y));
		nScroll |= rbs_Vert;
	}

	// Validation
	if ((nNewWidth <= 0) || (nNewHeight <= 0))
	{
		Assert(nNewWidth>0 && nNewHeight>0);
		return FALSE;
	}
	
	// Result
	if (pnNewWidth)
		*pnNewWidth = nNewWidth;
	if (pnNewHeight)
		*pnNewHeight = nNewHeight;
	if (pnScroll)
		*pnScroll = nScroll;
	
	return TRUE;
	
	//BOOL lbBufferHeight = this->isScroll();

	//// �������� ������� ���������
	//if (!lbBufferHeight)
	//{
	//	if (sbi.dwSize.Y > sbi.dwMaximumWindowSize.Y)
	//	{
	//		lbBufferHeight = TRUE; // ����������� ��������� ���������
	//	}
	//}

	//if (lbBufferHeight)
	//{
	//	if (sbi.srWindow.Top == 0
	//	        && sbi.dwSize.Y == (sbi.srWindow.Bottom + 1)
	//	  )
	//	{
	//		lbBufferHeight = FALSE; // ����������� ���������� ���������
	//	}
	//}

	//// ������ ���������� �������
	//if (!lbBufferHeight)
	//{
	//	nNewHeight =  sbi.dwSize.Y;
	//}
	//else
	//{
	//	// ��� ����� ������ �� ����� ����� �������
	//	if ((sbi.srWindow.Bottom - sbi.srWindow.Top + 1) < MIN_CON_HEIGHT)
	//		nNewHeight = con.nTextHeight;
	//	else
	//		nNewHeight = sbi.srWindow.Bottom - sbi.srWindow.Top + 1;
	//}

	//WARNING("����� ����� ��������� ���������, ���� nNewHeight ������ - �������� ����� BufferHeight");

	//if (pbBufferHeight)
	//	*pbBufferHeight = lbBufferHeight;

	//_ASSERTE(nNewWidth>=MIN_CON_WIDTH && nNewHeight>=MIN_CON_HEIGHT);

	//if (!nNewWidth || !nNewHeight)
	//{
	//	Assert(nNewWidth && nNewHeight);
	//	return FALSE;
	//}

	////if (nNewWidth < sbi.dwSize.X)
	////    nNewWidth = sbi.dwSize.X;
	//return TRUE;
}

// ��������� �������� ���������� (���� ����������� �������)
void CRealBuffer::SetBufferHeightMode(BOOL abBufferHeight, BOOL abIgnoreLock/*=FALSE*/)
{
	if (mb_BuferModeChangeLocked)
	{
		if (!abIgnoreLock)
		{
			//_ASSERTE(mb_BuferModeChangeLocked==FALSE || abIgnoreLock);
			return;
		}
	}

	con.bBufferHeight = abBufferHeight;

	if (mp_RCon->isActive())
		OnBufferHeight();
}

// ���������� �� TabBar->ConEmu
void CRealBuffer::ChangeBufferHeightMode(BOOL abBufferHeight)
{
	if (abBufferHeight && gOSVer.dwMajorVersion == 6 && gOSVer.dwMinorVersion == 1)
	{
		// Win7 BUGBUG: Issue 192: ������� Conhost ��� turn bufferheight ON
		// http://code.google.com/p/conemu-maximus5/issues/detail?id=192
		return;
		//const SHORT nMaxBuf = 600;
		//if (nNewBufHeightSize > nMaxBuf && mp_RCon->isFar())
		//	nNewBufHeightSize = nMaxBuf;
	}

	_ASSERTE(!mb_BuferModeChangeLocked);
	BOOL lb = mb_BuferModeChangeLocked; mb_BuferModeChangeLocked = TRUE;
	con.bBufferHeight = abBufferHeight;

	// ���� ��� ������� ���� "conemu.exe /bufferheight 0 ..."
	if (abBufferHeight /*&& !con.nBufferHeight*/)
	{
		// ���� ������������ ������ ������ ������ � ������� ��������
		con.nBufferHeight = gpSet->DefaultBufferHeight;

		if (con.nBufferHeight<300)
		{
			_ASSERTE(con.nBufferHeight>=300);
			con.nBufferHeight = max(300,con.nTextHeight*2);
		}
	}

	mcr_LastMousePos = MakeCoord(-1,-1);

	USHORT nNewBufHeightSize = abBufferHeight ? con.nBufferHeight : 0;
	SetConsoleSize(TextWidth(), TextHeight(), nNewBufHeightSize, CECMD_SETSIZESYNC);
	mb_BuferModeChangeLocked = lb;
}

void CRealBuffer::SetChange2Size(int anChange2TextWidth, int anChange2TextHeight)
{
	#ifdef _DEBUG
	if (anChange2TextHeight > 200)
	{
		_ASSERTE(anChange2TextHeight <= 200);
	}
	#endif

	con.nChange2TextWidth = anChange2TextWidth;
	con.nChange2TextHeight = anChange2TextHeight;
}

BOOL CRealBuffer::isBuferModeChangeLocked()
{
	return mb_BuferModeChangeLocked;
}

BOOL CRealBuffer::BuferModeChangeLock()
{
	BOOL lbNeedUnlock = !mb_BuferModeChangeLocked;
	mb_BuferModeChangeLocked = TRUE;
	return lbNeedUnlock;
}

void CRealBuffer::BuferModeChangeUnlock()
{
	mb_BuferModeChangeLocked = FALSE;
}

// �� con.m_sbi ���������, �������� �� ���������
BOOL CRealBuffer::CheckBufferSize()
{
	bool lbForceUpdate = false;

	if (!this)
		return false;

	if (mb_BuferModeChangeLocked)
		return false;

	//if (con.m_sbi.dwSize.X>(con.m_sbi.srWindow.Right-con.m_sbi.srWindow.Left+1)) {
	//  DEBUGLOGFILE("Wrong screen buffer width\n");
	//  // ������ ������� ������-�� ����������� �� �����������
	//  WARNING("���� ����� ��� �����");
	//  //MOVEWINDOW(mp_RCon->hConWnd, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 1);
	//} else {
	// BufferHeight ����� �������� � �� ������� (��������, DVDPanel), �� ����� ������ ����, ��� � ������ ���������� (wmic)
	BOOL lbTurnedOn = BufferHeightTurnedOn(&con.m_sbi);

	if (!lbTurnedOn && con.bBufferHeight)
	{
		// ����� ���� ���������� ��������� ��������� ����� ��������������?
		// TODO: ��������� ���������!!!
		SetBufferHeightMode(FALSE);
		//UpdateScrollInfo(); -- ������� � ApplyConsoleInfo()
		lbForceUpdate = true;
	}
	else if (lbTurnedOn && !con.bBufferHeight)
	{
		SetBufferHeightMode(TRUE);
		//UpdateScrollInfo(); -- ������� � ApplyConsoleInfo()
		lbForceUpdate = true;
	}

	//TODO: � ���� ������ ������ ����� ��������� �� ����� ���������� ���������?
	//if ((BufferHeight == 0) && (con.m_sbi.dwSize.Y>(con.m_sbi.srWindow.Bottom-con.m_sbi.srWindow.Top+1))) {
	//  TODO("��� ����� ���� ���������� ��������� ��������� ����� ��������������!")
	//      DEBUGLOGFILE("Wrong screen buffer height\n");
	//  // ������ ������� ������-�� ����������� �� ���������
	//  WARNING("���� ����� ��� �����");
	//  //MOVEWINDOW(mp_RCon->hConWnd, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 1);
	//}
	//TODO: ����� �� ��������� � ConEmuC, ���� ����� �����
	//// ��� ������ �� FAR -> CMD � BufferHeight - ����� QuickEdit ������
	//DWORD mode = 0;
	//BOOL lb = FALSE;
	//if (BufferHeight) {
	//  //TODO: ������, ��� ��� BufferHeight ��� ���������� ���������?
	//  //lb = GetConsoleMode(hConIn(), &mode);
	//  mode = GetConsoleMode();
	//  if (con.m_sbi.dwSize.Y>(con.m_sbi.srWindow.Bottom-con.m_sbi.srWindow.Top+1)) {
	//      // ����� ������ ������ ����
	//      mode |= ENABLE_QUICK_EDIT_MODE|ENABLE_INSERT_MODE|ENABLE_EXTENDED_FLAGS;
	//  } else {
	//      // ����� ����� ������ ���� (������ ��� ����������)
	//      mode &= ~(ENABLE_QUICK_EDIT_MODE|ENABLE_INSERT_MODE);
	//      mode |= ENABLE_EXTENDED_FLAGS;
	//  }
	//  TODO("SetConsoleMode");
	//  //lb = SetConsoleMode(hConIn(), mode);
	//}
	return lbForceUpdate;
}

BOOL CRealBuffer::LoadDataFromSrv(DWORD CharCount, CHAR_INFO* pData)
{
	if (m_Type != rbt_Primary)
	{
		_ASSERTE(m_Type == rbt_Primary);
		//if (dump.NeedApply)
		//{
		//	dump.NeedApply = FALSE;

		//	// �������� �������
		//	if (!InitBuffers(0))
		//	{
		//		_ASSERTE(FALSE);
		//	}
		//	else
		//	// � �����������
		//	{
		//		wchar_t*  pszSrc = dump.pszBlock1;
		//		CharAttr* pcaSrc = dump.pcaBlock1;
		//		wchar_t*  pszDst = con.pConChar;
		//		TODO("������ �� ���� ����������� ����� ��� �������, � �� ������ CHAR_ATTR");
		//		WORD*     pnaDst = con.pConAttr;
		//		
		//		DWORD dwConDataBufSize = dump.crSize.X * dump.crSize.Y;
		//		wmemmove(pszDst, pszSrc, dwConDataBufSize);

		//		// ���������� ������ CharAttr �� ���������� ��������
		//		for (DWORD n = 0; n < dwConDataBufSize; n++, pcaSrc++, pnaDst++)
		//		{
		//			*pnaDst = (pcaSrc->nForeIdx & 0xF0) | ((pcaSrc->nBackIdx & 0xF0) << 4);
		//		}
		//	}

		//	return TRUE;
		//}
		return FALSE; // ��������� ���
	}

	MCHKHEAP;
	BOOL lbScreenChanged = FALSE;
	wchar_t* lpChar = con.pConChar;
	WORD* lpAttr = con.pConAttr;
	CONSOLE_SELECTION_INFO sel;
	bool bSelectionPresent = GetConsoleSelectionInfo(&sel);
	UNREFERENCED_PARAMETER(bSelectionPresent);

	if (mp_RCon->mb_ResetStatusOnConsoleReady)
	{
		mp_RCon->mb_ResetStatusOnConsoleReady = false;
		mp_RCon->ms_ConStatus[0] = 0;
	}


	_ASSERTE(sizeof(*con.pDataCmp) == sizeof(*pData));

	lbScreenChanged = (memcmp(con.pDataCmp, pData, CharCount*sizeof(*pData)) != 0);

	if (lbScreenChanged)
	{
		if (IFLOGCONSOLECHANGE)
		{
			char sInfo[128];
			_wsprintfA(sInfo, SKIPLEN(countof(sInfo)) "DataCmp was changed, width=%u, height=%u, count=%u", con.nTextWidth, con.nTextHeight, CharCount);

			const CHAR_INFO* lp1 = con.pDataCmp;
			const CHAR_INFO* lp2 = pData;
			INT_PTR idx = -1;

			for (DWORD n = 0; n < CharCount; n++, lp1++, lp2++)
			{
				if (memcmp(lp1, lp2, sizeof(*lp2)) != 0)
				{
					idx = (lp1 - con.pDataCmp);
					int y = con.nTextWidth ? (idx / con.nTextWidth) : 0;
					int x = con.nTextWidth ? (idx - y * con.nTextWidth) : idx;

					_wsprintfA(sInfo+strlen(sInfo), SKIPLEN(32) ", posY=%i, posX=%i", y, x);

					break;
				}
			}
            
            if (idx == -1)
            {
            	lstrcatA(sInfo, ", failed to find change idx");
            }

			LOGCONSOLECHANGE(sInfo);
		}

		//con.pCopy->crBufSize = con.pCmp->crBufSize;
		//memmove(con.pCopy->Buf, con.pCmp->Buf, CharCount*sizeof(CHAR_INFO));
		memmove(con.pDataCmp, pData, CharCount*sizeof(CHAR_INFO));
		MCHKHEAP;
		CHAR_INFO* lpCur = con.pDataCmp;
		//// ����� �������� ����������� ��������� - ����� ����� ��������� ������ � ��������
		//_ASSERTE(!bSelectionPresent); -- �� �����. ��� ������� GetConsoleData
		wchar_t ch;

		// ���������� ������ CHAR_INFO �� ����� � ��������
		for (DWORD n = 0; n < CharCount; n++, lpCur++)
		{
			TODO("OPTIMIZE: *(lpAttr++) = lpCur->Attributes;");
			*(lpAttr++) = lpCur->Attributes;
			TODO("OPTIMIZE: ch = lpCur->Char.UnicodeChar;");
			ch = lpCur->Char.UnicodeChar;
			//2009-09-25. ��������� (������?) ��������� ���������� �������� � ������� ������� (ASC<32)
			//            �� ����� �������� �� ��������� �������
			*(lpChar++) = (ch < 32) ? gszAnalogues[(WORD)ch] : ch;
		}

		MCHKHEAP
	}

	return lbScreenChanged;
}

BOOL CRealBuffer::IsTrueColorerBufferChanged()
{
	BOOL lbChanged = FALSE;
	AnnotationHeader aHdr;
	int nCmp = 0;

	if (!gpSet->isTrueColorer || !mp_RCon->mp_TrueColorerData)
		goto wrap;

	// �������� ������ TrueColor
	if (!mp_RCon->m_TrueColorerMap.GetTo(&aHdr, sizeof(aHdr)))
		goto wrap;

	// Check counter
	if (mp_RCon->m_TrueColorerHeader.flushCounter == aHdr.flushCounter)
		goto wrap;

	// Compare data
	EnterCriticalSection(&m_TrueMode.csLock);
	{
		int nCurMax = max(con.nTextWidth*con.nTextHeight,aHdr.bufferSize);
		size_t cbCurSize = nCurMax*sizeof(*m_TrueMode.mp_Cmp);

		if (!m_TrueMode.mp_Cmp || (nCurMax > m_TrueMode.nCmpMax))
		{
			SafeFree(m_TrueMode.mp_Cmp);
			m_TrueMode.nCmpMax = nCurMax;
            m_TrueMode.mp_Cmp = (AnnotationInfo*)malloc(cbCurSize);
            nCmp = 2;
		}
		else
		{
			nCmp = memcmp(m_TrueMode.mp_Cmp, mp_RCon->mp_TrueColorerData, cbCurSize);
		}

		if ((nCmp != 0) && m_TrueMode.mp_Cmp)
		{
			memcpy(m_TrueMode.mp_Cmp, mp_RCon->mp_TrueColorerData, cbCurSize);
		}
	}
	LeaveCriticalSection(&m_TrueMode.csLock);


	if (nCmp != 0)
	{
		lbChanged = TRUE;

		if (IFLOGCONSOLECHANGE)
		{
			char szDbgSize[128]; _wsprintfA(szDbgSize, SKIPLEN(countof(szDbgSize)) "ApplyConsoleInfo: TrueColorer.flushCounter(%u) -> changed(%u)", mp_RCon->m_TrueColorerHeader.flushCounter, aHdr.flushCounter);
			LOGCONSOLECHANGE(szDbgSize);
		}
	}

	// Save counter to avoid redundant checks
	mp_RCon->m_TrueColorerHeader = aHdr;
wrap:
	return lbChanged;
}

BOOL CRealBuffer::ApplyConsoleInfo()
{
	BOOL bBufRecreated = FALSE;
	BOOL lbChanged = FALSE;
	
	#ifdef _DEBUG
	if (mp_RCon->mb_DebugLocked)
		return FALSE;
	int nConNo = gpConEmu->isVConValid(mp_RCon->mp_VCon);
	nConNo = nConNo;
	#endif

	if (!mp_RCon->isServerAvailable())
	{
		// ������ ��� �����������. ������� ������� ������ �� ������� ����� �������� � ���������!
		SetEvent(mp_RCon->mh_ApplyFinished);
		return FALSE;
	}

	ResetEvent(mp_RCon->mh_ApplyFinished);
	const CESERVER_REQ_CONINFO_INFO* pInfo = NULL;
	CESERVER_REQ_HDR cmd; ExecutePrepareCmd(&cmd, CECMD_CONSOLEDATA, sizeof(cmd));

	if (mp_RCon->mb_SwitchActiveServer)
	{
		// Skip this step. Waiting for new console server.
	}
	else if (!mp_RCon->m_ConsoleMap.IsValid())
	{
		_ASSERTE(mp_RCon->m_ConsoleMap.IsValid());
	}
	else if (!mp_RCon->m_GetDataPipe.Transact(&cmd, sizeof(cmd), (const CESERVER_REQ_HDR**)&pInfo) || !pInfo)
	{
		#ifdef _DEBUG
		if (mp_RCon->m_GetDataPipe.GetErrorCode() == ERROR_PIPE_NOT_CONNECTED)
		{
			int nDbg = ERROR_PIPE_NOT_CONNECTED;
		}
		else
		{
			MBoxA(mp_RCon->m_GetDataPipe.GetErrorText());
		}
		#endif
	}
	else if (pInfo->cmd.cbSize < sizeof(CESERVER_REQ_CONINFO_INFO))
	{
		_ASSERTE(pInfo->cmd.cbSize >= sizeof(CESERVER_REQ_CONINFO_INFO));
	}
	else
		//if (!mp_ConsoleInfo->mp_RCon->hConWnd || !mp_ConsoleInfo->nCurDataMapIdx) {
		//	_ASSERTE(mp_ConsoleInfo->mp_RCon->hConWnd && mp_ConsoleInfo->nCurDataMapIdx);
		//} else
	{
		//if (mn_LastConsoleDataIdx != mp_ConsoleInfo->nCurDataMapIdx) {
		//	ReopenMapData();
		//}
		DWORD nPID = GetCurrentProcessId();
		DWORD nMapGuiPID = mp_RCon->m_ConsoleMap.Ptr()->nGuiPID;

		if (nPID != nMapGuiPID)
		{
			// ���� ������� ����������� ��� "-new_console" �� nMapGuiPID ����� ���� ��� 0?
			// ����, ��� ����� ��������� ������ ���� ������ ������� �� ������� �� ������������ � GUI ConEmu.
			if (nMapGuiPID != 0)
			{
				_ASSERTE(nMapGuiPID == nPID);
			}
		}

		#ifdef _DEBUG
		HWND hWnd = pInfo->hConWnd;
		if (!hWnd || (hWnd != mp_RCon->hConWnd))
		{
			// Wine bug ? Incomplete packet?
			_ASSERTE(hWnd!=NULL);
			_ASSERTE(hWnd==mp_RCon->hConWnd);
		}
		#endif

		//if (mp_RCon->hConWnd != hWnd) {
		//    SetHwnd ( hWnd ); -- ����. Maps ��� �������!
		//}
		// 3
		// ����� � ��� �������� �������� �������, ���� ����������
		if (mp_RCon->ProcessUpdate(pInfo->nProcesses, countof(pInfo->nProcesses)))
		{
			//120325 - ��� ������ �������������� �������, ���� ������ � ��� �� ��������.
			//  ��� �������� 1) � ������� ���������; 2) ������ ��������� � ���������� ���������� �����������
			//lbChanged = TRUE; // ���� �������� ������ (Far/�� Far) - ������������ �� ������ ������
		}
		// ������ ����� ������� ������ - �������� ��������� ���������� ������
		MSectionLock sc;
		// 4
		DWORD dwCiSize = pInfo->dwCiSize;

		if (dwCiSize != 0)
		{
			_ASSERTE(dwCiSize == sizeof(con.m_ci));

			if (memcmp(&con.m_ci, &pInfo->ci, sizeof(con.m_ci))!=0)
			{
				LOGCONSOLECHANGE("ApplyConsoleInfo: CursorInfo changed");
				lbChanged = TRUE;
			}

			con.m_ci = pInfo->ci;
		}

		// 5, 6, 7
		con.m_dwConsoleCP = pInfo->dwConsoleCP;
		con.m_dwConsoleOutputCP = pInfo->dwConsoleOutputCP;

		if (con.m_dwConsoleMode != pInfo->dwConsoleMode)
		{
			if (ghOpWnd && mp_RCon->isActive())
				gpSetCls->UpdateConsoleMode(pInfo->dwConsoleMode);
		}

		con.m_dwConsoleMode = pInfo->dwConsoleMode;
		// 8
		DWORD dwSbiSize = pInfo->dwSbiSize;
		int nNewWidth = 0, nNewHeight = 0;

		if (dwSbiSize != 0)
		{
			MCHKHEAP
			_ASSERTE(dwSbiSize == sizeof(con.m_sbi));

			if (memcmp(&con.m_sbi, &pInfo->sbi, sizeof(con.m_sbi))!=0)
			{
				LOGCONSOLECHANGE("ApplyConsoleInfo: ScreenBufferInfo changed");
				lbChanged = TRUE;

				//if (mp_RCon->isActive())
				//	gpConEmu->UpdateCursorInfo(&pInfo->sbi, pInfo->sbi.dwCursorPosition, pInfo->ci);
			}

			#ifdef _DEBUG
			wchar_t szCursorDbg[255]; szCursorDbg[0] = 0;
			if (pInfo->sbi.dwCursorPosition.X != con.m_sbi.dwCursorPosition.X || pInfo->sbi.dwCursorPosition.Y != con.m_sbi.dwCursorPosition.Y)
				_wsprintf(szCursorDbg, SKIPLEN(countof(szCursorDbg)) L"CursorPos changed to %ux%u. ", pInfo->sbi.dwCursorPosition.X, pInfo->sbi.dwCursorPosition.Y);
			else
				_wsprintf(szCursorDbg, SKIPLEN(countof(szCursorDbg)) L"CursorPos is %ux%u. ", pInfo->sbi.dwCursorPosition.X, pInfo->sbi.dwCursorPosition.Y);
			#endif

			CONSOLE_SCREEN_BUFFER_INFO lsbi = pInfo->sbi;
			// ���� ������ ����� �������� ��������� - �� ������ TopVisible
			if (mp_RCon->InScroll())
			{
				UINT nY = lsbi.srWindow.Bottom - lsbi.srWindow.Top;
				lsbi.srWindow.Top = max(0,min(con.nTopVisibleLine,lsbi.dwSize.Y-nY-1));
				lsbi.srWindow.Bottom = lsbi.srWindow.Top + nY;
				#ifdef _DEBUG
				int l = lstrlen(szCursorDbg);
				_wsprintf(szCursorDbg+l, SKIPLEN(countof(szCursorDbg)-l) L"Visible rect locked to {%ux%u-%ux%u), Top=%u. ", lsbi.srWindow.Left, lsbi.srWindow.Top, lsbi.srWindow.Right, lsbi.srWindow.Bottom, con.nTopVisibleLine);
				#endif
			}
			#ifdef _DEBUG
			else if (memcmp(&pInfo->sbi.srWindow, &con.m_sbi.srWindow, sizeof(con.m_sbi.srWindow)))
			{
				if (pInfo->sbi.dwCursorPosition.X != con.m_sbi.dwCursorPosition.X || pInfo->sbi.dwCursorPosition.Y != con.m_sbi.dwCursorPosition.Y)
				{
					int l = lstrlen(szCursorDbg);
					_wsprintf(szCursorDbg+l, SKIPLEN(countof(szCursorDbg)-l) L"Visible rect changed to {%ux%u-%ux%u). ", pInfo->sbi.srWindow.Left, pInfo->sbi.srWindow.Top, pInfo->sbi.srWindow.Right, pInfo->sbi.srWindow.Bottom);
				}
			}

			if (szCursorDbg[0])
			{
				wcscat_c(szCursorDbg, L"\n");
				DEBUGSTRCURSORPOS(szCursorDbg);
			}
			#endif

			con.m_sbi = lsbi;

			DWORD nScroll;
			if (GetConWindowSize(con.m_sbi, &nNewWidth, &nNewHeight, &nScroll))
			{
				//if (con.bBufferHeight != (nNewHeight < con.m_sbi.dwSize.Y))
				BOOL lbTurnedOn = BufferHeightTurnedOn(&con.m_sbi);
				if (con.bBufferHeight != lbTurnedOn)
					SetBufferHeightMode(lbTurnedOn);

				//  TODO("�������� ���������? ��� ��� ����?");
				if (nNewWidth != con.nTextWidth || nNewHeight != con.nTextHeight)
				{
					if (IFLOGCONSOLECHANGE)
					{
						char szDbgSize[128]; _wsprintfA(szDbgSize, SKIPLEN(countof(szDbgSize)) "ApplyConsoleInfo: SizeWasChanged(cx=%i, cy=%i)", nNewWidth, nNewHeight);
						LOGCONSOLECHANGE(szDbgSize);
					}
					
					#ifdef _DEBUG
					wchar_t szDbgSize[128]; _wsprintf(szDbgSize, SKIPLEN(countof(szDbgSize)) L"ApplyConsoleInfo.SizeWasChanged(cx=%i, cy=%i)", nNewWidth, nNewHeight);
					DEBUGSTRSIZE(szDbgSize);
					#endif

					bBufRecreated = TRUE; // ����� �������, ����� �������������
					//sc.Lock(&csCON, TRUE);
					//WARNING("����� �� ���������������?");
					InitBuffers(nNewWidth*nNewHeight*2);
				}
			}


			#ifdef SHOW_AUTOSCROLL
			if (gpSetCls->AutoScroll)
				con.nTopVisibleLine = con.m_sbi.srWindow.Top;
			#else
			// ���� ������ ����� �������� ��������� - �� ������ TopVisible
			if (!mp_RCon->InScroll())
				con.nTopVisibleLine = con.m_sbi.srWindow.Top;
			#endif

			MCHKHEAP
		}

		//DWORD dwCharChanged = pInfo->ConInfo.RgnInfo.dwRgnInfoSize;
		//BOOL  lbDataRecv = FALSE;
		if (/*mp_ConsoleData &&*/ nNewWidth && nNewHeight)
		{
			// ��� ����� ��������� �� ����� ������������ ������� (����� ��� �����)
			// ��� ��� ��������� ���������� ������ (Aero->Standard)
			// ��� ��� �������� ���� (�� "���������������" ������ �������)
			_ASSERTE(nNewWidth == pInfo->crWindow.X && nNewHeight == pInfo->crWindow.Y);
			// 10
			//DWORD MaxBufferSize = pInfo->nCurDataMaxSize;
			//if (MaxBufferSize != 0) {

			//// �� ����� ������ ��� ������ �� �����, ���� ��������� ���
			//if (mp_RCon->mn_LastConsolePacketIdx != pInfo->nPacketId)

			// ���� ������ � ���������� ������ ���������� ������
			if (pInfo->nDataShift && pInfo->nDataCount)
			{
				LOGCONSOLECHANGE("ApplyConsoleInfo: Console contents received");

				mp_RCon->mn_LastConsolePacketIdx = pInfo->nPacketId;
				DWORD CharCount = pInfo->nDataCount;

				
				#ifdef _DEBUG
				if (CharCount != (nNewWidth * nNewHeight))
				{
					// ��� ����� ��������� �� ����� ������������ ������� (����� ��� �����)
					_ASSERTE(CharCount == (nNewWidth * nNewHeight));
				}
				#endif


				DWORD OneBufferSize = CharCount * sizeof(wchar_t);
				CHAR_INFO *pData = (CHAR_INFO*)(((LPBYTE)pInfo) + pInfo->nDataShift);
				// �������� �������!
				DWORD nCalcCount = (pInfo->cmd.cbSize - pInfo->nDataShift) / sizeof(CHAR_INFO);

				if (nCalcCount != CharCount)
				{
					_ASSERTE(nCalcCount == CharCount);

					if (nCalcCount < CharCount)
						CharCount = nCalcCount;
				}

				//MSectionLock sc2; sc2.Lock(&csCON);
				sc.Lock(&csCON);
				MCHKHEAP;

				if (InitBuffers(OneBufferSize))
				{
					if (LoadDataFromSrv(CharCount, pData))
					{
						LOGCONSOLECHANGE("ApplyConsoleInfo: InitBuffers&LoadDataFromSrv -> changed");
						lbChanged = TRUE;
					}
				}

				MCHKHEAP;
			}
		}

		TODO("�� ����� ������� ������� ����� ������������ - ������ �� �� ��� �����...");
		//_ASSERTE(*con.pConChar!=ucBoxDblVert);

		// ���� ���������� SetConsoleSizeSrv � ������ ���� ������ ���������� ��� ����������!
		if (!con.bLockChange2Text)
		{
			con.nChange2TextWidth = -1;
			con.nChange2TextHeight = -1;
		}

#ifdef _DEBUG
		wchar_t szCursorInfo[60];
		_wsprintf(szCursorInfo, SKIPLEN(countof(szCursorInfo)) L"Cursor (X=%i, Y=%i, Vis:%i, H:%i)\n",
		          con.m_sbi.dwCursorPosition.X, con.m_sbi.dwCursorPosition.Y,
		          con.m_ci.bVisible, con.m_ci.dwSize);
		DEBUGSTRPKT(szCursorInfo);

		// ������ ��� ������ ���� ���������, � ��� �� ������ ���� ����
		if (con.pConChar)
		{
			BOOL lbDataValid = TRUE; uint n = 0;
			_ASSERTE(con.nTextWidth == con.m_sbi.dwSize.X);
			uint TextLen = con.nTextWidth * con.nTextHeight;

			while(n<TextLen)
			{
				if (con.pConChar[n] == 0)
				{
					lbDataValid = FALSE; break;
				}
				else if (con.pConChar[n] != L' ')
				{
					// 0 - ����� ���� ������ ��� �������. ����� ������ ����� �������, ���� �� ����, ���� �� ������
					if (con.pConAttr[n] == 0)
					{
						lbDataValid = FALSE; break;
					}
				}

				n++;
			}
		}

		//_ASSERTE(lbDataValid);
		MCHKHEAP;
#endif

		// �������� ������ TrueColor
		if (!lbChanged && gpSet->isTrueColorer && mp_RCon->mp_TrueColorerData)
		{
			if (IsTrueColorerBufferChanged())
			{
				lbChanged = TRUE;
			}
		}

		if (lbChanged)
		{
			// �� con.m_sbi ���������, �������� �� ���������
			CheckBufferSize();
			MCHKHEAP;
		}

		sc.Unlock();
	}

	SetEvent(mp_RCon->mh_ApplyFinished);

	if (lbChanged)
	{
		mp_RCon->mb_DataChanged = TRUE; // ���������� ������������ ������ ������
		con.bConsoleDataChanged = TRUE; // � ��� - ��� ������� �� CVirtualConsole

		//if (mp_RCon->isActive()) -- mp_RCon->isActive() �������� ���� UpdateScrollInfo, � ��������� ����� ���� � � ������� �� �� �������� �������
		mp_RCon->UpdateScrollInfo();
	}

	return lbChanged;
}

// �� ����������� CONSOLE_SCREEN_BUFFER_INFO ����������, �������� �� ���������
// static
BOOL CRealBuffer::BufferHeightTurnedOn(CONSOLE_SCREEN_BUFFER_INFO* psbi)
{
	BOOL lbTurnedOn = FALSE;
	TODO("!!! ���������������");

	if (psbi->dwSize.Y <= (psbi->srWindow.Bottom - psbi->srWindow.Top + 1))
	{
		_ASSERTE(psbi->dwSize.Y == (psbi->srWindow.Bottom - psbi->srWindow.Top + 1))
		// ������ ���� == ������ ������,
		lbTurnedOn = FALSE;
	}
	else
	{
		//Issue 509: ����� ���� ��������� ��������, ����� �������� ������� �������,
		//           �� �� �������� ������ ������. � ���� ������, ����� �������� ������!
		TODO("��� ����� �� ���������� �� � TextHeight(), � � ������� ������. �� ��� ���������������� ���� ������ ��� ������ � ����������.");
		int nHeight = TextHeight();
		// ������ ������ '�������' ������ ������ ������ ����
		if (con.m_sbi.dwSize.Y > EvalBufferTurnOnSize(nHeight))
			lbTurnedOn = TRUE;
	}
	//else if (con.m_sbi.dwSize.Y < (con.m_sbi.srWindow.Bottom-con.m_sbi.srWindow.Top+10))
	//{
	//	// ������ ���� �������� ����� ������ ������
	//	lbTurnedOn = FALSE;
	//}
	//else if (con.m_sbi.dwSize.Y>(con.m_sbi.srWindow.Bottom-con.m_sbi.srWindow.Top+10))
	//{
	//	// ������ ������ '�������' ������ ������ ����
	//	lbTurnedOn = TRUE;
	//}

	// ������, ���� ������ ������� ������ ��� ����������� � GUI ���� - ����� �������� BufferHeight
	if (!lbTurnedOn)
	{
		//TODO: ������, ���� ������ ������� ������ ��� ����������� � GUI ���� - ����� �������� BufferHeight
	}

	return lbTurnedOn;
}

void CRealBuffer::OnBufferHeight()
{
	if (!this)
	{
		_ASSERTE(this);
		return;
	}

	// ��� ����� ������ ������ - �������� ��������� ������� �������
	ResetLastMousePos();

	gpConEmu->OnBufferHeight();
}

// ���� �������� ��������� - ��������������� ������ ������ �� �������� � ��������
COORD CRealBuffer::ScreenToBuffer(COORD crMouse)
{
	if (!this)
		return crMouse;

	if (isScroll())
	{
		crMouse.X += con.m_sbi.srWindow.Left;
		crMouse.Y += con.m_sbi.srWindow.Top;
	}

	return crMouse;
}

COORD CRealBuffer::BufferToScreen(COORD crMouse, bool bVertOnly /*= false*/)
{
	if (!this)
		return crMouse;

	if (isScroll())
	{
		if (!bVertOnly)
			crMouse.X = max(0,crMouse.X-con.m_sbi.srWindow.Left);
		crMouse.Y = max(0,crMouse.Y-con.m_sbi.srWindow.Top);
	}

	return crMouse;
}

ExpandTextRangeType CRealBuffer::GetLastTextRangeType()
{
	if (!this)
		return etr_None;
	return con.etrLast;
}

void CRealBuffer::ResetLastMousePos()
{
	mcr_LastMousePos = MakeCoord(-1,-1);
}

bool CRealBuffer::ProcessFarHyperlink(UINT messg/*=WM_USER*/)
{
	if (mcr_LastMousePos.X == -1)
	{
		if (con.etrLast != etr_None)
		{
			StoreLastTextRange(etr_None);
		}
	}

	return ProcessFarHyperlink(messg, mcr_LastMousePos);
}

bool CRealBuffer::LookupFilePath(LPCWSTR asFileOrPath, wchar_t* pszPath, size_t cchPathMax)
{
	_ASSERTE(pszPath!=NULL && asFileOrPath!=NULL && cchPathMax>=MAX_PATH);

	lstrcpyn(pszPath, asFileOrPath, (int)cchPathMax);

	TODO("�������� ������� ���������� ������� ����");
	TODO("������� ������� � ������� ����� � ����������");
	TODO("������� ������ ����� � ��������� �������� ����");
	TODO("��������� �������� ��������, ����� ������ � ������� ���� �� ��������");

	if (FileExists(pszPath))
		return true;

	return false;
}

bool CRealBuffer::ProcessFarHyperlink(UINT messg, COORD crFrom)
{
	if (!mp_RCon->IsFarHyperlinkAllowed(false))
		return false;
		
	bool lbProcessed = false;
	
	//if (messg == WM_MOUSEMOVE || messg == WM_LBUTTONDOWN || messg == WM_LBUTTONUP || messg == WM_LBUTTONDBLCLK)
	//{
	COORD crStart = MakeCoord(crFrom.X - con.m_sbi.srWindow.Left, crFrom.Y - con.m_sbi.srWindow.Top);
	// �� ����� ������� ���������� ����� ������ �������
	if ((crStart.Y < 0 || crStart.Y >= con.nTextHeight)
		|| (crStart.X < 0 || crStart.X >= con.nTextWidth))
	{
		bool bChanged = false;
		ResetLastMousePos();
		if (con.etrLast != etr_None)
		{
			StoreLastTextRange(etr_None);
			bChanged = true;
		}
		return bChanged;
	}


	COORD crEnd = crStart;
	wchar_t szText[MAX_PATH+10];
	ExpandTextRangeType rc = mp_RCon->isActive()
		? ExpandTextRange(crStart, crEnd, etr_FileAndLine, szText, countof(szText))
		: etr_None;
	if (memcmp(&crStart, &con.mcr_FileLineStart, sizeof(crStart)) != 0
		|| memcmp(&crEnd, &con.mcr_FileLineEnd, sizeof(crStart)) != 0)
	{
		con.mcr_FileLineStart = crStart;
		con.mcr_FileLineEnd = crEnd;
		// WM_USER ���������� ���� ����� ���� �� GetConsoleData ��� ��������� ���������� ���������
		if (messg != WM_USER)
		{
			UpdateSelection(); // �������� �� ������
		}
	}
	
	if ((rc == etr_FileAndLine) || (rc == etr_Url))
	{
		if ((messg == WM_LBUTTONDOWN) && *szText)
		{
			if (rc == etr_Url)
			{
				int iRc = (int)ShellExecute(ghWnd, L"open", szText, NULL, NULL, SW_SHOWNORMAL);
				if (iRc <= 32)
				{
					DisplayLastError(szText, iRc, MB_ICONSTOP, L"URL open failed");
				}
			}
			else if (rc == etr_FileAndLine)
			{
				// ����� ����� ������
				CESERVER_REQ_FAREDITOR cmd = {sizeof(cmd)};
				int nLen = lstrlen(szText)-1;
				if (szText[nLen] == L')')
				{
					szText[nLen] = 0;
					nLen--;
				}
				while ((nLen > 0)
					&& (((szText[nLen-1] >= L'0') && (szText[nLen-1] <= L'9'))
						|| ((szText[nLen-1] == L',') && ((szText[nLen] >= L'0') && (szText[nLen] <= L'9')))))
				{
					nLen--;
				}
				if (nLen < 3)
				{
					_ASSERTE(nLen >= 3);
				}
				else
				{ // 1.c:3: 
					wchar_t* pszEnd;
					cmd.nLine = wcstol(szText+nLen, &pszEnd, 10);
					if (pszEnd && (*pszEnd == L',') && isDigit(*(pszEnd+1)))
						cmd.nColon = wcstol(pszEnd+1, &pszEnd, 10);
					if (cmd.nColon < 1)
						cmd.nColon = 1;
					szText[nLen-1] = 0;
					while ((pszEnd = wcschr(szText, L'/')) != NULL)
						*pszEnd = L'\\'; // �������� ������ ����� �� ��������

					//lstrcpyn(cmd.szFile, szText, countof(cmd.szFile));
					LookupFilePath(szText/*name from console*/, cmd.szFile/*full path*/, countof(cmd.szFile));
					
					TODO("��� ��������, ���� ��� ������ � ���������, ��� ���������� ����, ��� ������� ������");
					// ������ ����� ������, ��� ������� ��� ����� ���� ����������, �� ������ UserScreen (CtrlO)
					
					// ���������, ����� ��� ������ ��� � ���� ������?
					LPCWSTR pszFileName = wcsrchr(cmd.szFile, L'\\');
					if (!pszFileName) pszFileName = cmd.szFile; else pszFileName++;
					CVirtualConsole* pVCon = NULL;

					//// ����� ��������, � �� ��� �������� � �������,
					//// ����� ����������� ��� �������, ������ ��������...
					//StoreLastTextRange(etr_None);
					//UpdateSelection();

					int liActivated = gpConEmu->mp_TabBar->ActiveTabByName(fwt_Editor|fwt_ActivateFound|fwt_PluginRequired, pszFileName, &pVCon);
					
					if (liActivated == -2)
					{
						// �����, �� ������������ ������, TabBar ������ ��� �������� ����������� ��������� � �������
						_ASSERTE(FALSE);
					}
					else
					{
						if (liActivated >= 0)
						{
							// �����, ������������, ����� ������ �� ������ �������
							if (cmd.nLine > 0)
							{
								wchar_t szMacro[96];
								if (mp_RCon->m_FarInfo.FarVer.dwVerMajor == 1)
									_wsprintf(szMacro, SKIPLEN(countof(szMacro)) L"@$if(Editor) AltF8 \"%i:%i\" Enter $end", cmd.nLine, cmd.nColon);
								else
									_wsprintf(szMacro, SKIPLEN(countof(szMacro)) L"@$if(Editor) AltF8 print(\"%i:%i\") Enter $end", cmd.nLine, cmd.nColon);
								_ASSERTE(pVCon!=NULL);

								// -- ������� ���-������ � �������, ����� ��� ���� �� UserScreen ��������� ����� ��������?
								//PostMouseEvent(WM_LBUTTONUP, 0, crFrom);

								// ��, ������� �� ������ (������)
								pVCon->RCon()->PostMacro(szMacro, TRUE);
							}
						}
						else
						{
							CVConGuard VCon;
							if (!gpConEmu->isFarExist(fwt_NonModal|fwt_PluginRequired, NULL, &VCon))
							{
								if (gpSet->sFarGotoEditor && *gpSet->sFarGotoEditor)
								{
									wchar_t szRow[32], szCol[32];
									_wsprintf(szRow, SKIPLEN(countof(szRow)) L"%u", cmd.nLine);
									_wsprintf(szCol, SKIPLEN(countof(szCol)) L"%u", cmd.nColon);
									//LPCWSTR pszVar[] = {L"%1", L"%2", L"%3"};
									LPCWSTR pszVal[] = {szRow, szCol, cmd.szFile};
									//_ASSERTE(countof(pszVar)==countof(pszVal));
									wchar_t* pszCmd = ExpandMacroValues(gpSet->sFarGotoEditor, pszVal, countof(pszVal));
									if (!pszCmd)
									{
										DisplayLastError(L"Invalid command specified in \"External editor\"", -1);
									}
									else
									{
										RConStartArgs args;
										args.pszSpecialCmd = pszCmd; pszCmd = NULL;
										args.pszStartupDir = mp_RCon->m_Args.pszStartupDir ? lstrdup(mp_RCon->m_Args.pszStartupDir) : NULL;
										args.bRunAsAdministrator = mp_RCon->m_Args.bRunAsAdministrator;
										args.bForceUserDialog = mp_RCon->m_Args.bRunAsRestricted || (mp_RCon->m_Args.pszUserName != NULL);
										args.bBufHeight = TRUE;
										//args.eConfirmation = RConStartArgs::eConfNever;

										gpConEmu->CreateCon(&args);
									}
								}
								else
								{
									DisplayLastError(L"Available Far Manager was not found in open tabs", -1);
								}
							}
							else
							{
								// -- ������� ���-������ � �������, ����� ��� ���� �� UserScreen ��������� ����� ��������?
								//PostMouseEvent(WM_LBUTTONUP, 0, crFrom);

								// Prepared, ����� ����� ������
								VCon->RCon()->PostCommand(CMD_OPENEDITORLINE, sizeof(cmd), &cmd);
								gpConEmu->Activate(VCon.VCon());
							}
						}
					}
				}
			}
		}
		lbProcessed = true;
	}

	return lbProcessed;
}

void CRealBuffer::ShowKeyBarHint(WORD nID)
{
	if ((nID > 0) && (nID <= countof(gnKeyBarFlags)))
	{
		// ����� �����-�� ���������� ������ "��������" ������, �� ���,
		// ����� �� ��������� �������, ����������� �� ��������� ������������!
		//INPUT_RECORD r = {KEY_EVENT};
		//r.Event.MouseEvent.dwMousePosition.X = nID;
		//r.Event.MouseEvent.dwControlKeyState = gnKeyBarFlags[nID - 1];
		//r.Event.MouseEvent.dwEventFlags = MOUSE_MOVED;
		//mp_RCon->PostConsoleEvent(&r);
		mp_RCon->PostKeyPress(VK_RWIN, gnKeyBarFlags[nID - 1], 0);
	}
}

// x,y - �������� ����������
// crMouse - ScreenToBuffer
// ���������� true, ���� ����� ��������� "��� �����"
bool CRealBuffer::OnMouse(UINT messg, WPARAM wParam, int x, int y, COORD crMouse, bool abFromTouch /*= false*/)
{
#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL                  0x020E
#endif

	mcr_LastMousePos = crMouse;

	if (mp_RCon->isSelectionAllowed())
	{
		if (messg == WM_LBUTTONDOWN)
		{
			// ������ ��������� ���������
			if (OnMouseSelection(messg, wParam, x, y))
				return true;
		}

		// ���� ��������� ��� �� ������, �� ������������ ����������� - ������������ WM_MOUSEMOVE
		if (messg == WM_MOUSEMOVE && !con.m_sel.dwFlags)
		{
			if ((gpSet->isCTSSelectBlock && gpSet->IsModifierPressed(vkCTSVkBlock, false))
				|| (gpSet->isCTSSelectText && gpSet->IsModifierPressed(vkCTSVkText, false)))
			{
				// ����������, ������������ ���������� ������ ���������, �� �������� �������� ���� � �������
				return true;
			}
		}

		if (((gpSet->isCTSRBtnAction == 2) || ((gpSet->isCTSRBtnAction == 3) && !isSelectionPresent()))
				&& (messg == WM_RBUTTONDOWN || messg == WM_RBUTTONUP)
		        && ((gpSet->isCTSActMode == 2 && mp_RCon->isBufferHeight() && !mp_RCon->isFarBufferSupported())
		            || (gpSet->isCTSActMode == 1 && gpSet->IsModifierPressed(vkCTSVkAct, true))))
		{
			if (messg == WM_RBUTTONUP) mp_RCon->Paste();

			return true;
		}

		if (((gpSet->isCTSMBtnAction == 2) || ((gpSet->isCTSMBtnAction == 3) && !isSelectionPresent()))
				&& (messg == WM_MBUTTONDOWN || messg == WM_MBUTTONUP)
		        && ((gpSet->isCTSActMode == 2 && mp_RCon->isBufferHeight() && !mp_RCon->isFarBufferSupported())
		            || (gpSet->isCTSActMode == 1 && gpSet->IsModifierPressed(vkCTSVkAct, true))))
		{
			if (messg == WM_MBUTTONUP) mp_RCon->Paste();

			return true;
		}
	}

	// ����� � ��������� ������ � �������� ����
	// .\realconsole.cpp(8104) : error ...
	if ((con.m_sel.dwFlags == 0) && mp_RCon->IsFarHyperlinkAllowed(false))
	{
		if (messg == WM_MOUSEMOVE || messg == WM_LBUTTONDOWN || messg == WM_LBUTTONUP || messg == WM_LBUTTONDBLCLK)
		{
			if (ProcessFarHyperlink(messg, crMouse))
			{
				// ������� ��� ��� ������� ���� � �������?
				// ����� �������� �� �������, � �� ������ ����� ��������� �� ���������, ��������
				return true;
			}
		}
	}

	BOOL lbFarBufferSupported = mp_RCon->isFarBufferSupported();
	BOOL lbMouseOverScroll = FALSE;
	// ��������� ����� ����� ����� ������ ���� ��� ������������ � ���, � �� �������� �� ���������
	if ((messg == WM_MOUSEWHEEL) || (messg == WM_MOUSEHWHEEL))
	{
		if (con.bBufferHeight && (m_Type == rbt_Primary) && lbFarBufferSupported)
		{
			lbMouseOverScroll = mp_RCon->mp_VCon->CheckMouseOverScroll(true);
		}
	}

	if (con.bBufferHeight && ((m_Type != rbt_Primary) || !lbFarBufferSupported || lbMouseOverScroll))
	{
		if (messg == WM_MOUSEWHEEL)
		{
			SHORT nDir = (SHORT)HIWORD(wParam);
			BOOL lbCtrl = isPressed(VK_CONTROL);

			UINT nCount = abFromTouch ? 1 : gpConEmu->mouse.GetWheelScrollLines();

			if (nDir > 0)
			{
				OnScroll(lbCtrl ? SB_PAGEUP : SB_LINEUP, -1, nCount);
			}
			else if (nDir < 0)
			{
				OnScroll(lbCtrl ? SB_PAGEDOWN : SB_LINEDOWN, -1, nCount);
			}

			return true; // ��� ����������
		}
		else if (messg == WM_MOUSEHWHEEL)
		{
			TODO("WM_MOUSEHWHEEL - �������������� ���������");
			_ASSERTE(FALSE && "Horz scrolling! WM_MOUSEHWHEEL");
			//return true; -- ����� ����� ������ - return true;
		}

		if (!isConSelectMode())
		{
			// ���������� � �������, ���� ��� �� Far
			return (m_Type != rbt_Primary);
		}
	}

	//if (isConSelectMode()) -- ��� �����������. ��� ��������� � �� ��������� ������� (����� D&D �� �������)
	if (con.m_sel.dwFlags != 0)
	{
		// ������ ��������� ���������, �� ������� ���������� �� �������...
		OnMouseSelection(messg, wParam, x, y);
		return true;
	}

	// ��� ������ ����� �� KeyBar'� - �������� PopupMenu � ���������� ������������� F-������
	TODO("���� ������ ��� Far Manager?");
	if ((m_Type == rbt_Primary) && (gpSet->isKeyBarRClick)
		&& ((messg == WM_RBUTTONDOWN && (crMouse.Y == (GetTextHeight() - 1)) && mp_RCon->isFarKeyBarShown())
			|| ((messg == WM_MOUSEMOVE || messg == WM_RBUTTONUP) && con.bRClick4KeyBar)))
	{
		if (messg == WM_RBUTTONDOWN)
		{
			MSectionLock csData;
			wchar_t* pChar = NULL;
			int nLen = 0;
			if (GetConsoleLine(crMouse.Y, &pChar, &nLen, &csData) && (*pChar == L'1'))
			{
				// �.�. ������ ����� ����������, ����
				int x, k, px = 0, vk = 0;
				for (x = 1, k = 2; x < nLen; x++)
				{
					if (pChar[x] < L'0' || pChar[x] > L'9')
						continue;
					if (k <= 9)
					{
						if ((((int)pChar[x] - L'0') == k) && (pChar[x-1] == L' ')
							&& (pChar[x+1] < L'0' || pChar[x+1] > L'9'))
						{
							if ((crMouse.X <= (x - 1)) && (crMouse.X >= px))
							{
								vk = VK_F1 + (k - 2);
								break;
							}
							px = x - 1;
							k++;
						}
					}
					else if (k <= 12)
					{
						if ((pChar[x] == L'1') && (((int)pChar[x+1] - L'0') == (k-10)) && (pChar[x-1] == L' ')
							&& (pChar[x+2] < L'0' || pChar[x+2] > L'9'))
						{
							if ((crMouse.X <= (x - 1)) && (crMouse.X >= px))
							{
								px++;
								vk = VK_F1 + (k - 2);
								break;
							}
							px = x - 1;
							k++;
						}
					}
					else
					{
						px++;
						vk = VK_F12;
						break;
					}
				}

				if (vk)
				{
					con.bRClick4KeyBar = TRUE;
					con.crRClick4KeyBar = crMouse;
					con.ptRClick4KeyBar = mp_RCon->mp_VCon->ConsoleToClient((vk==VK_F1)?(px+1):(px+2), crMouse.Y);
					ClientToScreen(mp_RCon->GetView(), &con.ptRClick4KeyBar);
					con.nRClickVK = vk;
					return true;
				}
			}
		}
		else if (messg == WM_RBUTTONUP)
		{
			_ASSERTE(con.bRClick4KeyBar);
			//Run!
			HMENU h = CreatePopupMenu();
			wchar_t szText[128];
			for (size_t i = 0; i < countof(gnKeyBarFlags); i++)
			{
				*szText = 0;
				if (gnKeyBarFlags[i] & LEFT_CTRL_PRESSED)
					wcscat_c(szText, L"Ctrl-");
				if (gnKeyBarFlags[i] & LEFT_ALT_PRESSED)
					wcscat_c(szText, L"Alt-");
				if (gnKeyBarFlags[i] & RIGHT_CTRL_PRESSED)
					wcscat_c(szText, L"RCtrl-");
				if (gnKeyBarFlags[i] & RIGHT_ALT_PRESSED)
					wcscat_c(szText, L"RAlt-");
				if (gnKeyBarFlags[i] & SHIFT_PRESSED)
					wcscat_c(szText, L"Shift-");
				_wsprintf(szText+lstrlen(szText), SKIPLEN(8) L"F%i", (con.nRClickVK - VK_F1 + 1));

				AppendMenu(h, MF_STRING|((!(i % 4)) ? MF_MENUBREAK : 0), i+1, szText);
			}

			RECT rcExcl = {con.ptRClick4KeyBar.x-1,con.ptRClick4KeyBar.y-1,con.ptRClick4KeyBar.x+1,con.ptRClick4KeyBar.y+1};

			int i = gpConEmu->mp_Menu->trackPopupMenu(tmp_KeyBar, h, TPM_LEFTALIGN|TPM_BOTTOMALIGN|/*TPM_NONOTIFY|*/TPM_RETURNCMD|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
						con.ptRClick4KeyBar.x, con.ptRClick4KeyBar.y, ghWnd, &rcExcl);
			DestroyMenu(h);

			if ((i > 0) && (i <= (int)countof(gnKeyBarFlags)))
			{
				i--;
				mp_RCon->PostKeyPress(con.nRClickVK, gnKeyBarFlags[i], 0);
			}

			//Done
			con.bRClick4KeyBar = FALSE;
			return true;
		}
		else if (messg == WM_MOUSEMOVE)
		{
			_ASSERTE(con.bRClick4KeyBar);
			TODO("����������� ���� ��� �����?");
			return true; // �� ���������� � �������
		}
	}
	else if (con.bRClick4KeyBar)
	{
		con.bRClick4KeyBar = FALSE;
	}
	
	// ���������� ���� � ������� ������ ���� ����� ��������
	return (m_Type != rbt_Primary);
}

BOOL CRealBuffer::GetRBtnDrag(COORD* pcrMouse)
{
	if (pcrMouse)
		*pcrMouse = con.crRBtnDrag;
	return con.bRBtnDrag;
}

void CRealBuffer::SetRBtnDrag(BOOL abRBtnDrag, const COORD* pcrMouse)
{
	con.bRBtnDrag = abRBtnDrag;

	if (pcrMouse)
		con.crRBtnDrag = *pcrMouse;
}

// x,y - �������� ����������
bool CRealBuffer::OnMouseSelection(UINT messg, WPARAM wParam, int x, int y)
{
	if (TextWidth()<=1 || TextHeight()<=1)
	{
		_ASSERTE(TextWidth()>1 && TextHeight()>1);
		return false;
	}

	// �������� ��������� ���������� ��������
	COORD cr = mp_RCon->mp_VCon->ClientToConsole(x,y);
	if (cr.X<0) cr.X = 0; else if (cr.X >= (int)TextWidth()) cr.X = TextWidth()-1;
	if (cr.Y<0) cr.Y = 0; else if (cr.Y >= (int)TextHeight()) cr.Y = TextHeight()-1;

	if (messg == WM_LBUTTONDOWN)
	{
		BOOL lbStreamSelection = FALSE;
		BYTE vkMod = 0; // ���� ������������ ����������� - ��� ����� "���������" � �������
		bool bTripleClick = (con.m_sel.dwFlags & CONSOLE_DBLCLICK_SELECTION) && ((GetTickCount() - con.m_SelDblClickTick) <= GetDoubleClickTime());

		if (con.m_sel.dwFlags & (CONSOLE_TEXT_SELECTION|CONSOLE_BLOCK_SELECTION))
		{
			// ��������� �������� �� ����
			lbStreamSelection = (con.m_sel.dwFlags & (CONSOLE_TEXT_SELECTION)) == CONSOLE_TEXT_SELECTION;
		}
		else
		{
			if (gpSet->isCTSSelectBlock && gpSet->IsModifierPressed(vkCTSVkBlock, true))
			{
				lbStreamSelection = FALSE; vkMod = gpSet->GetHotkeyById(vkCTSVkBlock); // OK
			}
			else if (gpSet->isCTSSelectText && gpSet->IsModifierPressed(vkCTSVkText, true))
			{
				lbStreamSelection = TRUE; vkMod = gpSet->GetHotkeyById(vkCTSVkText); // OK
			}
			else
			{
				return false; // ����������� �� ��������
			}
		}

		con.m_sel.dwFlags &= ~CONSOLE_KEYMOD_MASK;
		con.m_sel.dwFlags |= ((DWORD)vkMod) << 24;

		COORD crTo = cr;
		if (bTripleClick)
		{
			cr.X = 0;
			crTo.X = GetBufferWidth()-1;
		}

		// ���� ����� ���� - ������ ��� ����������� �����, ��� �� ���� ��������� ���������
		StartSelection(lbStreamSelection, cr.X, cr.Y, TRUE, bTripleClick ? WM_LBUTTONDBLCLK : WM_LBUTTONDOWN, bTripleClick ? &crTo : NULL);

		//WARNING!!! ����� StartSelection - ������ �� ������! ��� ��������� �����!

		return true;
	}
	else if ((messg == WM_LBUTTONDBLCLK) && (con.m_sel.dwFlags & (CONSOLE_TEXT_SELECTION|CONSOLE_BLOCK_SELECTION)))
	{
		// �������� ����� ��� �������� (��� � ������� �������)
		BOOL lbStreamSelection = (con.m_sel.dwFlags & CONSOLE_TEXT_SELECTION) == CONSOLE_TEXT_SELECTION;
		
		// ����� �������� ���������� �����
		COORD crFrom = cr, crTo = cr;
		ExpandTextRange(crFrom/*[In/Out]*/, crTo/*[Out]*/, etr_Word);
		
		// ��������� ���������
		StartSelection(lbStreamSelection, crFrom.X, crFrom.Y, TRUE, WM_LBUTTONDBLCLK, &crTo);

		// ������ ������ ����� ��������, �����
		con.m_sel.dwFlags &= ~CONSOLE_MOUSE_DOWN;

		//WARNING!!! ����� StartSelection - ������ �� ������! ��� ��������� �����!
		return true;
	}
	else if (
		((messg == WM_MOUSEMOVE) && (con.m_sel.dwFlags & CONSOLE_MOUSE_DOWN))
		|| ((messg == WM_LBUTTONUP) && (con.m_sel.dwFlags & CONSOLE_MOUSE_SELECTION))
		)
	{
		// ��� LBtnUp ����� ���� ��������� ���������
		// 1. Cick. ����� WM_LBUTTONUP ����� ��������� ������� � ������� DoSelectionCopy.
		// 2. DblClick. ��������� WM_LBUTTONUP WM_LBUTTONDBLCLK. ��������� "�����".
		// 3. TripleCLick. ��������� WM_LBUTTONUP WM_LBUTTONDBLCLK WM_LBUTTONDOWN WM_LBUTTONUP. ��������� "������".

		TODO("�������������� ���������?");
		// ���� ���� �� ��������� ���� ������� - ��������������� ���������� (MinMax)
		if (cr.X<0 || cr.X>=(int)TextWidth())
			cr.X = GetMinMax(cr.X, 0, TextWidth());
		if (cr.Y<0 || cr.Y>=(int)TextHeight())
			cr.Y = GetMinMax(cr.Y, 0, TextHeight());

		// ������ �������� Double/Triple.
		if ((messg == WM_LBUTTONUP)
			&& ((((con.m_sel.dwFlags & CONSOLE_MOUSE_SELECTION)
					&& ((GetTickCount() - con.m_SelClickTick) <= GetDoubleClickTime()))
				|| ((con.m_sel.dwFlags & CONSOLE_DBLCLICK_SELECTION)
					&& ((GetTickCount() - con.m_SelDblClickTick) <= GetDoubleClickTime())))
				)
			)
			//&& ((messg == WM_LBUTTONUP)
			//	|| ((messg == WM_MOUSEMOVE)
			//		&& (memcmp(&cr,&con.m_sel.srSelection.Left,sizeof(cr)) || memcmp(&cr,&con.m_sel.srSelection.Right,sizeof(cr)))
			//		)
			//	)
			//)
		{
			// Ignoring due DoubleClickTime
			int nDbg = 0; UNREFERENCED_PARAMETER(nDbg);
		}
		else
		{
			if (con.m_sel.dwFlags & CONSOLE_DBLCLICK_SELECTION)
			{
				con.m_sel.dwFlags &= ~CONSOLE_DBLCLICK_SELECTION;
			}
			else
			{
				ExpandSelection(cr.X, cr.Y);
			}

		}

		if (messg == WM_LBUTTONUP)
		{
			con.m_sel.dwFlags &= ~CONSOLE_MOUSE_DOWN;

			if (gpSet->isCTSAutoCopy)
			{
				//if ((con.m_sel.srSelection.Left != con.m_sel.srSelection.Right) || (con.m_sel.srSelection.Top != con.m_sel.srSelection.Bottom))
				DWORD nPrevTick = (con.m_sel.dwFlags & CONSOLE_DBLCLICK_SELECTION) ? con.m_SelDblClickTick : con.m_SelClickTick;
				if ((GetTickCount() - nPrevTick) > GetDoubleClickTime())
				{
					// ���� ������������ ��������� ������ ����� ��������� DblClickTime
					// �� ����� (� �����) ����� ��������� �����������
					_ASSERTE(nPrevTick!=0);
					DoSelectionFinalize(true, 0);
				}
				else
				{
					// ����� - ������, ����� �� ��������� ����������� DblClick/TripleClick
					// ��������������� ������� ������� �� ����������
					mp_RCon->VCon()->SetAutoCopyTimer(true);
				}
			}
		}

		return true;
	}
	else if ((messg == WM_RBUTTONUP || messg == WM_MBUTTONUP) && (con.m_sel.dwFlags & (CONSOLE_TEXT_SELECTION|CONSOLE_BLOCK_SELECTION)))
	{
		BYTE bAction = (messg == WM_RBUTTONUP) ? gpSet->isCTSRBtnAction : gpSet->isCTSMBtnAction; // 0-off, 1-copy, 2-paste, 3-auto

		// ������ Copy. ������ Paste ��� ������� ��������� - �����. ��� ��� ������ Copy.
		BOOL bDoCopy = (bAction == 1) || (bAction == 3);
		DoSelectionFinalize(bDoCopy);

		return true;
	}

	return false;
}

void CRealBuffer::MarkFindText(int nDirection, LPCWSTR asText, bool abCaseSensitive, bool abWholeWords)
{
	bool bFound = false;
	COORD crStart = {}, crEnd = {};
	LPCWSTR pszFrom = NULL, pszDataStart = NULL, pszEnd = NULL;
	LPCWSTR pszFrom1 = NULL, pszEnd1 = NULL;
	size_t nWidth = 0, nHeight = 0;

	if (m_Type == rbt_Primary)
	{
		pszDataStart = pszFrom = pszFrom1 = con.pConChar;
		nWidth = this->GetTextWidth();
		nHeight = this->GetTextHeight();
		_ASSERTE(pszFrom[nWidth*nHeight] == 0); // ������ ���� ASCIIZ
		pszEnd = pszEnd1 = pszFrom + (nWidth * nHeight);
	}
	else if (dump.pszBlock1)
	{
		//WARNING("���������� ��� ������ � ����������");
		nWidth = dump.crSize.X;
		nHeight = dump.crSize.Y;
		_ASSERTE(dump.pszBlock1[nWidth*nHeight] == 0); // ������ ���� ASCIIZ

		pszDataStart = dump.pszBlock1;
		pszEnd = pszDataStart + (nWidth * nHeight);

		pszFrom = pszFrom1 = dump.pszBlock1 + con.m_sbi.srWindow.Top * nWidth;

		// Search in whole buffer
		//nHeight = min((dump.crSize.Y-con.m_sbi.srWindow.Top),(con.m_sbi.srWindow.Bottom - con.m_sbi.srWindow.Top + 1));
		// But remember visible area
		pszEnd1 = pszFrom + (nWidth * min((dump.crSize.Y-con.m_sbi.srWindow.Top),(con.m_sbi.srWindow.Bottom - con.m_sbi.srWindow.Top + 1)));
	}

	WARNING("TODO: bFindNext");
	
	if (pszFrom && asText && *asText && nWidth && nHeight)
	{
		int nFindLen = lstrlen(asText);
		const wchar_t* szWordDelim = L"~!%^&*()+|{}:\"<>?`-=\\[];',./";
		LPCWSTR pszFound = NULL;
		int nStepMax = 0;
		INT_PTR nFrom = -1;

		if (nDirection > 1)
			nDirection = 1;
		else if (nDirection < -1)
			nDirection = -1;

		if (isSelectionPresent())
		{
			nFrom = con.m_sel.srSelection.Left + (con.m_sel.srSelection.Top * nWidth);
			_ASSERTE(nFrom>=0);

			if (nDirection >= 0)
			{
				if ((nFrom + nDirection) >= (INT_PTR)(nWidth * nHeight))
					goto done; // �������, ��� �� �����
				pszFrom += (nFrom + nDirection);
			}
			else if (nDirection < 0)
			{
				pszEnd = pszFrom + nFrom;
			}
			nStepMax = ((m_Type == rbt_Primary) || (nDirection >= 0)) ? 1 : 2;
		}

		for (int i = 0; i <= nStepMax; i++)
		{
			while (pszFrom && (pszFrom < pszEnd) && *pszFrom)
			{
				if (abCaseSensitive)
					pszFrom = StrStr(pszFrom, asText);
				else
					pszFrom = StrStrI(pszFrom, asText);

				if (pszFrom)
				{
					if (abWholeWords)
					{
						#define isWordDelim(ch) (!ch || (wcschr(szWordDelim,ch)!=NULL) || (ch>=0x2100 && ch<0x2800) || (ch<=32))
						if (pszFrom > con.pConChar)
						{
							if (!isWordDelim(*(pszFrom-1)))
							{
								pszFrom++;
								continue;
							}
						}
						if (!isWordDelim(pszFrom[nFindLen]))
						{
							pszFrom++;
							continue;
						}
					}

					if (nDirection < 0)
					{
						if (pszFrom < pszEnd)
						{
							pszFound = pszFrom;
							pszFrom++;
							bFound = true;
							continue;
						}
						else
						{
							pszFrom = NULL;
						}
					}
					bFound = true;
					break; // OK, ��������
				}
			}

			if ((nDirection < 0) && bFound && pszFound)
			{
				pszFrom = pszFound;
			}
			if (pszFrom && bFound)
				break;

			if (!nStepMax)
				break;


			pszFrom = pszDataStart;
			pszEnd = pszDataStart + (nWidth * nHeight);
			if ((nDirection < 0) && (nFrom >= 0))
			{
				if (i == 0)
				{
					pszEnd = pszFrom1 + nFrom;
				}
				else
				{
					pszFrom = pszFrom1 + nFrom + 1;
				}
			}
		}


		if (pszFrom && bFound)
		{
			// �����
			size_t nCharIdx = (pszFrom - pszDataStart);

			WARNING("���������� ��������� �� ��������� ���������, � �� ������ ������� �������");

			// � ���� - ��������� ��������� ������� �������, ���� ����� "�� ���������"
			if (m_Type != rbt_Primary)
			{
				int nRows = con.m_sbi.srWindow.Bottom - con.m_sbi.srWindow.Top; // ��� ��������

				if (pszFrom < pszFrom1)
				{
					// ���������� ����� �����
					con.nTopVisibleLine = con.m_sbi.srWindow.Top = max(0,((int)(nCharIdx / nWidth))-1);
					con.m_sbi.srWindow.Bottom = min((con.m_sbi.srWindow.Top + nRows), con.m_sbi.dwSize.Y-1);
				}
				else if (pszFrom >= pszEnd1)
				{
					// ���������� ����� ����
					con.m_sbi.srWindow.Bottom = min((nCharIdx / nWidth)+1, (UINT)con.m_sbi.dwSize.Y-1);
					con.nTopVisibleLine = con.m_sbi.srWindow.Top = max(0, con.m_sbi.srWindow.Bottom-nRows);
				}

				if (nCharIdx >= (size_t)(con.m_sbi.srWindow.Top*nWidth))
				{
					nCharIdx -= con.m_sbi.srWindow.Top*nWidth;
				}
			}
			else
			{
				// ��� rbt_Primary - ��������� ���� �� ������ (�� ���������� ����� �� ���)
				_ASSERTE(pszDataStart == pszFrom1);
			}

			WARNING("��� �� �� ������ ������ ���������������, � �� ������� �������...");
			if (nCharIdx < (nWidth*nHeight))
			{
				bFound = true;
				crStart.Y = nCharIdx / nWidth;
				crStart.X = nCharIdx - (nWidth * crStart.Y);
				crEnd.Y = (nCharIdx + nFindLen - 1) / nWidth;
				crEnd.X = (nCharIdx + nFindLen - 1) - (nWidth * crEnd.Y);
			}
		}

	} // if (pszFrom && asText && *asText && nWidth && nHeight)

done:
	if (!bFound)
	{
		DoSelectionStop();
	}
	else
	{
		con.m_sel.dwFlags = CONSOLE_SELECTION_IN_PROGRESS | CONSOLE_TEXT_SELECTION;
		con.m_sel.dwSelectionAnchor = crStart;
		con.m_sel.srSelection.Left = crStart.X;
		con.m_sel.srSelection.Top = crStart.Y;
		con.m_sel.srSelection.Right = crEnd.X;
		con.m_sel.srSelection.Bottom = crEnd.Y;
	}

	UpdateSelection();
}

void CRealBuffer::StartSelection(BOOL abTextMode, SHORT anX/*=-1*/, SHORT anY/*=-1*/, BOOL abByMouse/*=FALSE*/, UINT anFromMsg/*=0*/, COORD *pcrTo/*=NULL*/)
{
	if (!(con.m_sel.dwFlags & (CONSOLE_BLOCK_SELECTION|CONSOLE_TEXT_SELECTION)) && gpSet->isCTSFreezeBeforeSelect)
	{
		if (m_Type == rbt_Primary)
		{
			if (mp_RCon->LoadAlternativeConsole(lam_FullBuffer) && (mp_RCon->mp_ABuf != this))
			{
				// �������� ����� (������������� �� �������������� �������)
				// ������� ���������� �������� - � ���� ������

				DoSelectionStop();

				_ASSERTE(mp_RCon->mp_ABuf->m_Type==rbt_Alternative);
				mp_RCon->mp_ABuf->m_Type = rbt_Selection; // ��������, ����� �� ���������� ��������� - ����� �������

				mp_RCon->mp_ABuf->StartSelection(abTextMode, anX, anY, abByMouse, anFromMsg);
				return;
			}
		}
	}

	mp_RCon->VCon()->SetAutoCopyTimer(false);

	WARNING("���������� ��� ������ � ���������� - ��������� ���������, ��� � ������� �������");
	if (anX == -1 && anY == -1)
	{
		anX = con.m_sbi.dwCursorPosition.X - con.m_sbi.srWindow.Left;
		anY = con.m_sbi.dwCursorPosition.Y - con.m_sbi.srWindow.Top;
	}
	// ���� ������ ��������� �� ������ - ������ ��������� ����
	if (anX < 0)
		anX = 0;
	else if (anX >= TextWidth())
		anX = TextWidth()-1;
	if (anY < 0)
		anY = anX = 0;
	else if (anY >= TextHeight())
	{
		anX = 0;
		anY = TextHeight()-1;
	}

	COORD cr = {anX,anY};

	if (cr.X<0 || cr.X>=TextWidth() || cr.Y<0 || cr.Y>=TextHeight())
	{
		_ASSERTE(cr.X>=0 && cr.X<TextWidth());
		_ASSERTE(cr.Y>=0 && cr.Y<TextHeight());
		return; // ������ � �����������
	}

	DWORD vkMod = con.m_sel.dwFlags & CONSOLE_KEYMOD_MASK;
	if (vkMod && !abByMouse)
	{
		DoSelectionStop(); // ����� ��� �� �����, ��� ��� ��� ����� �����������
	}

	con.m_sel.dwFlags = CONSOLE_SELECTION_IN_PROGRESS
	                    | (abByMouse ? (CONSOLE_MOUSE_SELECTION|CONSOLE_MOUSE_DOWN) : 0)
	                    | (abTextMode ? CONSOLE_TEXT_SELECTION : CONSOLE_BLOCK_SELECTION)
						| (abByMouse ? vkMod : 0);
	con.m_sel.dwSelectionAnchor = cr;
	con.m_sel.srSelection.Left = con.m_sel.srSelection.Right = cr.X;
	con.m_sel.srSelection.Top = con.m_sel.srSelection.Bottom = cr.Y;
	UpdateSelection();

	if ((anFromMsg == WM_LBUTTONDBLCLK) || (pcrTo && (con.m_sel.dwFlags & CONSOLE_DBLCLICK_SELECTION)))
	{
		if (pcrTo)
			ExpandSelection(pcrTo->X, pcrTo->Y);
		con.m_sel.dwFlags |= CONSOLE_DBLCLICK_SELECTION;

		_ASSERTE(anFromMsg == WM_LBUTTONDBLCLK);
		//if (anFromMsg == WM_LBUTTONDBLCLK)
		con.m_SelDblClickTick = GetTickCount();

		if (gpSet->isCTSAutoCopy)
		{
			//DoSelectionFinalize(true);
			mp_RCon->VCon()->SetAutoCopyTimer(true);
		}
	}
	else if (abByMouse)
	{
		con.m_SelClickTick = GetTickCount();

		if (gpSet->isCTSAutoCopy)
		{
			mp_RCon->VCon()->SetAutoCopyTimer(true);
		}
	}
}

void CRealBuffer::ExpandSelection(SHORT anX/*=-1*/, SHORT anY/*=-1*/)
{
	_ASSERTE(con.m_sel.dwFlags!=0);
	COORD cr = {anX,anY};

	if (cr.X<0 || cr.X>=(int)TextWidth() || cr.Y<0 || cr.Y>=(int)TextHeight())
	{
		_ASSERTE(cr.X>=0 && cr.X<(int)TextWidth());
		_ASSERTE(cr.Y>=0 && cr.Y<(int)TextHeight());
		return; // ������ � �����������
	}

	BOOL lbStreamSelection = (con.m_sel.dwFlags & (CONSOLE_TEXT_SELECTION)) == CONSOLE_TEXT_SELECTION;

	if (!lbStreamSelection)
	{
		if (cr.X < con.m_sel.dwSelectionAnchor.X)
		{
			con.m_sel.srSelection.Left = cr.X;
			con.m_sel.srSelection.Right = con.m_sel.dwSelectionAnchor.X;
		}
		else
		{
			con.m_sel.srSelection.Left = con.m_sel.dwSelectionAnchor.X;
			con.m_sel.srSelection.Right = cr.X;
		}
	}
	else
	{
		if ((cr.Y > con.m_sel.dwSelectionAnchor.Y)
		        || ((cr.Y == con.m_sel.dwSelectionAnchor.Y) && (cr.X > con.m_sel.dwSelectionAnchor.X)))
		{
			con.m_sel.srSelection.Left = con.m_sel.dwSelectionAnchor.X;
			con.m_sel.srSelection.Right = cr.X;
		}
		else
		{
			con.m_sel.srSelection.Left = cr.X;
			con.m_sel.srSelection.Right = con.m_sel.dwSelectionAnchor.X;
		}
	}

	if (cr.Y < con.m_sel.dwSelectionAnchor.Y)
	{
		con.m_sel.srSelection.Top = cr.Y;
		con.m_sel.srSelection.Bottom = con.m_sel.dwSelectionAnchor.Y;
	}
	else
	{
		con.m_sel.srSelection.Top = con.m_sel.dwSelectionAnchor.Y;
		con.m_sel.srSelection.Bottom = cr.Y;
	}

	UpdateSelection();
}

void CRealBuffer::DoSelectionStop()
{
	BYTE vkMod = HIBYTE(HIWORD(con.m_sel.dwFlags));

	if (vkMod)
	{
		// �� ����� ��� �� �������� ������ (���� ���� ����� �� RAlt ��������...)
		if (vkMod == VK_CONTROL || vkMod == VK_LCONTROL || vkMod == VK_RCONTROL)
			mp_RCon->PostKeyPress(VK_SHIFT, LEFT_CTRL_PRESSED, 0);
		else if (vkMod == VK_MENU || vkMod == VK_LMENU || vkMod == VK_RMENU)
			mp_RCon->PostKeyPress(VK_SHIFT, LEFT_ALT_PRESSED, 0);
		else
			mp_RCon->PostKeyPress(VK_CONTROL, SHIFT_PRESSED, 0);

		// "���������" � ������� �����������
		mp_RCon->PostKeyUp(vkMod, 0, 0);
	}

	con.m_sel.dwFlags = 0;
}

bool CRealBuffer::DoSelectionCopy()
{
	if (!con.m_sel.dwFlags)
	{
		MBoxAssert(con.m_sel.dwFlags != 0);
		return false;
	}

	LPCWSTR pszDataStart = NULL;
	int nTextWidth = 0, nTextHeight = 0;

	if (m_Type == rbt_Primary)
	{
		pszDataStart = con.pConChar;
		nTextWidth = this->GetTextWidth();
		nTextHeight = this->GetTextHeight();
		_ASSERTE(pszDataStart[nTextWidth*nTextHeight] == 0); // ������ ���� ASCIIZ
	}
	else if (dump.pszBlock1)
	{
		WARNING("���������� ��� ������ � ����������");
		nTextWidth = dump.crSize.X;
		nTextHeight = dump.crSize.Y;
		//nTextWidth = this->GetTextWidth();
		//nTextHeight = this->GetTextHeight();
		_ASSERTE(dump.pszBlock1[nTextWidth*nTextHeight] == 0); // ������ ���� ASCIIZ

		pszDataStart = dump.pszBlock1 + con.m_sbi.srWindow.Top * nTextWidth;
		nTextHeight = min((dump.crSize.Y-con.m_sbi.srWindow.Top),(con.m_sbi.srWindow.Bottom - con.m_sbi.srWindow.Top + 1));
	}

	//if (!con.pConChar)
	if (!pszDataStart || !nTextWidth || !nTextHeight)
	{
		MBoxAssert(pszDataStart != NULL);
		return false;
	}

	const Settings::AppSettings* pApp = gpSet->GetAppSettings(mp_RCon->GetActiveAppSettingsId());
	if (pApp == NULL)
	{
		MBoxAssert(pApp!=NULL);
		return false;
	}
	BYTE nEOL = pApp->CTSEOL();
	BYTE nTrimTailing = pApp->CTSTrimTrailing();
	bool bDetectLines = pApp->CTSDetectLineEnd();
	bool bBash = pApp->CTSBashMargin();

	wchar_t sPreLineBreak[] =
		{
			ucBox25,ucBox50,ucBox75,ucBox100,ucUpScroll,ucDnScroll,ucLeftScroll,ucRightScroll,ucArrowUp,ucArrowDown,
			ucNoBreakSpace,
			ucBoxDblVert,ucBoxSinglVert,ucBoxDblDownRight,ucBoxDblDownLeft,ucBoxDblUpRight,ucBoxDblUpLeft,ucBoxSinglDownRight,
			ucBoxSinglDownLeft,ucBoxSinglUpRight,ucBoxSinglUpLeft,ucBoxSinglDownDblHorz,ucBoxSinglUpDblHorz,ucBoxDblDownDblHorz,
			ucBoxDblUpDblHorz,ucBoxSinglDownHorz,ucBoxSinglUpHorz,ucBoxDblDownSinglHorz,ucBoxDblUpSinglHorz,ucBoxDblVertRight,
			ucBoxDblVertLeft,ucBoxDblVertSinglRight,ucBoxDblVertSinglLeft,ucBoxSinglVertRight,ucBoxSinglVertLeft,
			ucBoxDblHorz,ucBoxSinglHorz,ucBoxDblVertHorz,
			// End
			0 /*ASCIIZ!!!*/
		};


	DWORD dwErr = 0;
	BOOL  lbRc = FALSE;
	bool  Result = false;
	BOOL lbStreamMode = (con.m_sel.dwFlags & CONSOLE_TEXT_SELECTION) == CONSOLE_TEXT_SELECTION;
	int nSelWidth = con.m_sel.srSelection.Right - con.m_sel.srSelection.Left;
	int nSelHeight = con.m_sel.srSelection.Bottom - con.m_sel.srSelection.Top;
	//int nTextWidth = con.nTextWidth;

	//if (nSelWidth<0 || nSelHeight<0)
	if (con.m_sel.srSelection.Left > (con.m_sel.srSelection.Right+(con.m_sel.srSelection.Bottom-con.m_sel.srSelection.Top)*nTextWidth))
	{
		MBoxAssert(con.m_sel.srSelection.Left <= (con.m_sel.srSelection.Right+(con.m_sel.srSelection.Bottom-con.m_sel.srSelection.Top)*nTextWidth));
		return false;
	}

	nSelWidth++; nSelHeight++;
	int nCharCount = 0;

	if (!lbStreamMode)
	{
		nCharCount = ((nSelWidth+2/* "\r\n" */) * nSelHeight) - 2; // ����� ��������� ������ "\r\n" �� ��������
	}
	else
	{
		if (nSelHeight == 1)
		{
			nCharCount = nSelWidth;
		}
		else if (nSelHeight == 2)
		{
			// �� ������ ������ - �� ����� ������, ������ ������ - �� ��������� �����, + "\r\n"
			nCharCount = (con.nTextWidth - con.m_sel.srSelection.Left) + (con.m_sel.srSelection.Right + 1) + 2;
		}
		else
		{
			_ASSERTE(nSelHeight>2);
			// �� ������ ������ - �� ����� ������, ��������� ������ - �� ��������� �����, + "\r\n"
			nCharCount = (con.nTextWidth - con.m_sel.srSelection.Left) + (con.m_sel.srSelection.Right + 1) + 2
			             + ((nSelHeight - 2) * (con.nTextWidth + 2)); // + ��������� * (����� ������� + "\r\n")
		}
	}

	HGLOBAL hUnicode = NULL;
	hUnicode = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, (nCharCount+1)*sizeof(wchar_t));

	if (hUnicode == NULL)
	{
		MBoxAssert(hUnicode != NULL);
		return false;
	}

	wchar_t *pch = (wchar_t*)GlobalLock(hUnicode);

	if (!pch)
	{
		MBoxAssert(pch != NULL);
		GlobalFree(hUnicode);
	}

	// ��������� �������
	if ((con.m_sel.srSelection.Left + nSelWidth) > con.nTextWidth)
	{
		_ASSERTE((con.m_sel.srSelection.Left + nSelWidth) <= con.nTextWidth);
		nSelWidth = con.nTextWidth - con.m_sel.srSelection.Left;
	}

	if ((con.m_sel.srSelection.Top + nSelHeight) > con.nTextHeight)
	{
		_ASSERTE((con.m_sel.srSelection.Top + nSelHeight) <= con.nTextHeight);
		nSelHeight = con.nTextHeight - con.m_sel.srSelection.Top;
	}

	nSelHeight--;

	if (!lbStreamMode)
	{
		// �������� ���������
		for (int Y = 0; Y <= nSelHeight; Y++)
		{
			LPCWSTR pszCon = NULL;

			if (m_Type == rbt_Primary)
			{
				pszCon = con.pConChar + con.nTextWidth*(Y+con.m_sel.srSelection.Top) + con.m_sel.srSelection.Left;
			}
			else if (pszDataStart && (Y < nTextHeight))
			{
				WARNING("��������� ��� ������ � ����������!");
				pszCon = pszDataStart + dump.crSize.X*(Y+con.m_sel.srSelection.Top) + con.m_sel.srSelection.Left;
			}

			int nMaxX = nSelWidth - 1;

			if (pszCon)
			{
				wchar_t* pchStart = pch;

				for (int X = 0; X <= nMaxX; X++)
				{
					*(pch++) = *(pszCon++);
				}

				if (nTrimTailing == 1)
				{
					while ((pch > pchStart) && (*(pch-1) == L' '))
						*(--pch) = 0;
				}
			}
			else if (nTrimTailing != 1)
			{
				wmemset(pch, L' ', nSelWidth);
				pch += nSelWidth;
			}

			// �������� ������� ������
			if (Y < nSelHeight)
			{
				switch (nEOL)
				{
				case 1:
					*(pch++) = L'\n'; break;
				case 2:
					*(pch++) = L'\r'; break;
				default:
					*(pch++) = L'\r'; *(pch++) = L'\n';
				}
			}
		}
	}
	else
	{
		// ��������� (���������) ���������
		int nX1, nX2;
		//for (nY = rc.Top; nY <= rc.Bottom; nY++) {
		//	pnDst = pAttr + nWidth*nY;

		//	nX1 = (nY == rc.Top) ? rc.Left : 0;
		//	nX2 = (nY == rc.Bottom) ? rc.Right : (nWidth-1);

		//	for (nX = nX1; nX <= nX2; nX++) {
		//		pnDst[nX] = lcaSel;
		//	}
		//}
		for (int Y = 0; Y <= nSelHeight; Y++)
		{
			nX1 = (Y == 0) ? con.m_sel.srSelection.Left : 0;
			nX2 = (Y == nSelHeight) ? con.m_sel.srSelection.Right : (con.nTextWidth-1);
			LPCWSTR pszCon = NULL;
			LPCWSTR pszNextLine = NULL;
			
			if (m_Type == rbt_Primary)
			{
				pszCon = con.pConChar + con.nTextWidth*(Y+con.m_sel.srSelection.Top) + nX1;
				pszNextLine = ((Y + 1) <= nSelHeight) ? (con.pConChar + con.nTextWidth*(Y+1+con.m_sel.srSelection.Top)) : NULL;
			}
			else if (pszDataStart && (Y < nTextHeight))
			{
				WARNING("��������� ��� ������ � ����������!");
				pszCon = pszDataStart + dump.crSize.X*(Y+con.m_sel.srSelection.Top) + nX1;
				pszNextLine = ((Y + 1) <= nSelHeight) ? (pszDataStart + dump.crSize.X*(Y+1+con.m_sel.srSelection.Top)) : NULL;
			}

			wchar_t* pchStart = pch;

			if (pszCon)
			{
				for (int X = nX1; X <= nX2; X++)
				{
					*(pch++) = *(pszCon++);
				}
			}
			else
			{
				wmemset(pch, L' ', nSelWidth);
				pch += nSelWidth;
			}

			// �������� ������� ������
			if (Y < nSelHeight)
			{
				bool bContinue = false;

				if (bDetectLines && pszNextLine && (*pszNextLine != L' ')
					&& !wcschr(sPreLineBreak, *(pch - 1))
					&& !wcschr(sPreLineBreak, *pszNextLine))
				{
					// �������� ���������, ����� ��� ������ ��� ������ ������� � Prompt?
					if ((*(pch - 1) != L' ')
						|| (((pch - 1) > pchStart) && (*(pch - 2) != L' '))
						// sh.exe - one cell space pad on right edge
						|| (bBash && ((pch - 2) > pchStart) && (*(pch - 2) == L' ') && (*(pch - 3) != L' ')))
					{
						bContinue = true;
					}

					if (bBash && (*(pch - 1) == L' '))
					{
						*(--pch) = 0;
					}
				}

				if (!bContinue)
				{
					if (nTrimTailing)
					{
						while ((pch > pchStart) && (*(pch-1) == L' '))
							*(--pch) = 0;
					}

					switch (nEOL)
					{
					case 1:
						*(pch++) = L'\n'; break;
					case 2:
						*(pch++) = L'\r'; break;
					default:
						*(pch++) = L'\r'; *(pch++) = L'\n';
					}
				}
			}
			else
			{
				if (nTrimTailing)
				{
					while ((pch > pchStart) && (*(pch-1) == L' '))
						*(--pch) = 0;
				}
			}
		}
	}

	// Ready
	GlobalUnlock(hUnicode);

	// ������� ����� ������
	while(!(lbRc = OpenClipboard(ghWnd)))
	{
		dwErr = GetLastError();

		if (IDRETRY != DisplayLastError(L"OpenClipboard failed!", dwErr, MB_RETRYCANCEL|MB_ICONSTOP))
			return false;
	}

	lbRc = EmptyClipboard();
	// ���������� ������
	Result = SetClipboardData(CF_UNICODETEXT, hUnicode);

	while(!Result)
	{
		dwErr = GetLastError();

		if (IDRETRY != DisplayLastError(L"SetClipboardData(CF_UNICODETEXT, ...) failed!", dwErr, MB_RETRYCANCEL|MB_ICONSTOP))
		{
			GlobalFree(hUnicode); hUnicode = NULL;
			break;
		}

		Result = SetClipboardData(CF_UNICODETEXT, hUnicode);
	}

	lbRc = CloseClipboard();

	// Fin, ����������
	if (Result)
	{
		DoSelectionStop(); // con.m_sel.dwFlags = 0;
		
		if (m_Type == rbt_Selection)
		{
			mp_RCon->SetActiveBuffer(rbt_Primary);
			// ����� �� �����!
		}
		else
		{
			UpdateSelection(); // �������� �� ������
		}
	}

	return Result;
}

// �������� �� ������
void CRealBuffer::UpdateSelection()
{
	TODO("��� ���������? ����� �������� VCon");
	con.bConsoleDataChanged = TRUE; // � ��� - ��� ������� �� CVirtualConsole
	mp_RCon->mp_VCon->Update(true);
	mp_RCon->mp_VCon->Redraw();
}

BOOL CRealBuffer::isConSelectMode()
{
	if (!this) return false;

	if (con.m_sel.dwFlags != 0)
		return true;

	if ((this == mp_RCon->mp_RBuf) && mp_RCon->isFar())
	{
		// � ���� ����� ���� ������� ������� (AltIns)
		if (con.m_ci.dwSize >= 40)  // ��������� ��� ���������� �� ������ �������.
			return true;
	}

	return false;
}

BOOL CRealBuffer::isSelfSelectMode()
{
	if (!this) return false;
	
	return (con.m_sel.dwFlags != 0);
}

BOOL CRealBuffer::isStreamSelection()
{
	if (!this) return false;
	
	return ((con.m_sel.dwFlags & CONSOLE_TEXT_SELECTION) == CONSOLE_TEXT_SELECTION);
}

// true/false - true-�������� ����� (������� rbt_Primary)
bool CRealBuffer::DoSelectionFinalize(bool abCopy, WPARAM wParam)
{
	if (abCopy)
	{
		DoSelectionCopy();
	}

	mp_RCon->mn_SelectModeSkipVk = wParam;
	DoSelectionStop(); // con.m_sel.dwFlags = 0;

	if (m_Type == rbt_Selection)
	{
		mp_RCon->SetActiveBuffer(rbt_Primary);
		// ����� �� �����!
		return true;
	}
	else
	{
		//mb_ConsoleSelectMode = false;
		UpdateSelection(); // �������� �� ������
	}

	return false;
}

// pszChars may be NULL
bool CRealBuffer::OnKeyboard(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam, const wchar_t *pszChars)
{
	// ��������� Left/Right/Up/Down ��� ���������

	if (con.m_sel.dwFlags && messg == WM_KEYDOWN
	        && ((wParam == VK_ESCAPE) || (wParam == VK_RETURN)
				|| ((wParam == 'C' || wParam == VK_INSERT) && isPressed(VK_CONTROL))
	            || (wParam == VK_LEFT) || (wParam == VK_RIGHT) || (wParam == VK_UP) || (wParam == VK_DOWN))
	  )
	{
		if ((wParam == VK_ESCAPE) || (wParam == VK_RETURN) || (wParam == 'C' || wParam == VK_INSERT))
		{
			if (DoSelectionFinalize(wParam != VK_ESCAPE, wParam))
				return true;
		}
		else
		{
			COORD cr; ConsoleCursorPos(&cr);
			// ���������
			cr.Y -= con.nTopVisibleLine;

			if (wParam == VK_LEFT)  { if (cr.X>0) cr.X--; }
			else if (wParam == VK_RIGHT) { if (cr.X<(con.nTextWidth-1)) cr.X++; }
			else if (wParam == VK_UP)    { if (cr.Y>0) cr.Y--; }
			else if (wParam == VK_DOWN)  { if (cr.Y<(con.nTextHeight-1)) cr.Y++; }

			// ������ - �������
			BOOL bShift = isPressed(VK_SHIFT);

			if (!bShift)
			{
				BOOL lbStreamSelection = (con.m_sel.dwFlags & (CONSOLE_TEXT_SELECTION)) == CONSOLE_TEXT_SELECTION;
				// ��������� ��� ���� ������. ������ �������� � ������ ������.
				StartSelection(lbStreamSelection, cr.X,cr.Y);
			}
			else
			{
				ExpandSelection(cr.X,cr.Y);
			}
		}

		return true;
	}

	if (messg == WM_KEYUP)
	{
		if (wParam == mp_RCon->mn_SelectModeSkipVk)
		{
			mp_RCon->mn_SelectModeSkipVk = 0; // ���������� ����������, ��������� ������� ���� �� �����������/������
			return true;
		}
		else if (gpSet->isFarGotoEditor && isKey(wParam, gpSet->GetHotkeyById(vkFarGotoEditorVk)))
		{
			if (GetLastTextRangeType() != etr_None)
			{
				StoreLastTextRange(etr_None);
				UpdateSelection();
			}
		}
	}

	if (messg == WM_KEYDOWN)
	{
		if (gpSet->isFarGotoEditor && isKey(wParam, gpSet->GetHotkeyById(vkFarGotoEditorVk)))
		{
			if (ProcessFarHyperlink(WM_MOUSEMOVE))
				UpdateSelection();
		}

		if (mp_RCon->mn_SelectModeSkipVk)
			mp_RCon->mn_SelectModeSkipVk = 0; // ��� _�������_ ����� ������ ������� - �������� ������ (�� ���������)
	}

	switch (m_Type)
	{
	case rbt_DumpScreen:
	case rbt_Alternative:
	case rbt_Selection:
	case rbt_Find:
		if (messg == WM_KEYUP)
		{
			if (wParam == VK_ESCAPE)
			{
				if ((m_Type == rbt_Find) && gpSetCls->mh_FindDlg && IsWindow(gpSetCls->mh_FindDlg))
				{
					break; // ���� ����� ������ ������ - ����� �� ���������!
				}
				mp_RCon->SetActiveBuffer(rbt_Primary);
				return true;
			}
		}
		else if (messg == WM_KEYDOWN)
		{
			if ((wParam == VK_NEXT) || (wParam == VK_PRIOR))
			{
				gpConEmu->key_BufferScroll(false, wParam, mp_RCon);
				return true;
			}
			else if (((m_Type != rbt_Selection) && (m_Type != rbt_Find)) // � ������ ��������� ������� ������� �� �����
					&& ((wParam == VK_UP) || (wParam == VK_DOWN)))
			{
				gpConEmu->key_BufferScroll(false, wParam, mp_RCon);
				return true;
			}
		}
		break;
	default:
		;
	}

	// ��������� ���������� ��������� (������ � ������)
	return false;
}

COORD CRealBuffer::GetDefaultNtvdmHeight()
{
	// 100627 - con.m_sbi.dwSize.Y ����� ������������ ����������� ����� "far/w"
	COORD cr16bit = {80,con.m_sbi.srWindow.Bottom-con.m_sbi.srWindow.Top+1};

	if (gpSet->ntvdmHeight && cr16bit.Y >= (int)gpSet->ntvdmHeight) cr16bit.Y = gpSet->ntvdmHeight;
	else if (cr16bit.Y>=50) cr16bit.Y = 50;
	else if (cr16bit.Y>=43) cr16bit.Y = 43;
	else if (cr16bit.Y>=28) cr16bit.Y = 28;
	else if (cr16bit.Y>=25) cr16bit.Y = 25;
	
	return cr16bit;
}

const CONSOLE_SCREEN_BUFFER_INFO* CRealBuffer::GetSBI()
{
	return &con.m_sbi;
}

//BOOL CRealBuffer::IsConsoleDataChanged()
//{
//	if (!this) return FALSE;
//
//	return con.bConsoleDataChanged;
//}

BOOL CRealBuffer::GetConsoleLine(int nLine, wchar_t** pChar, /*CharAttr** pAttr,*/ int* pLen, MSectionLock* pcsData)
{
	// ����� ���� ��� �������������
	MSectionLock csData;
	if (pcsData)
	{
		if (!pcsData->isLocked())
			pcsData->Lock(&csCON);
	}
	else
	{
		csData.Lock(&csCON);
	}
	
	_ASSERTE(nLine>=0 && nLine<GetWindowHeight());
	if (nLine < 0 || nLine >= con.nTextHeight)
	{
		return FALSE;
	}

	if ((m_Type == rbt_DumpScreen) || (m_Type == rbt_Alternative) || (m_Type == rbt_Selection) || (m_Type == rbt_Find))
	{
		if (!dump.pszBlock1)
			return FALSE;

		if (pChar)
			*pChar = dump.pszBlock1 + ((con.m_sbi.srWindow.Top + nLine) * dump.crSize.X) + con.m_sbi.srWindow.Left;
		if (pLen)
			*pLen = dump.crSize.X;
	}
	else
	{
		// ������� ������
		if (!con.pConChar || !con.pConAttr)
			return FALSE;
		
		if (pChar)
			*pChar = con.pConChar + (nLine * con.nTextWidth);
		//if (pAttr)
		//	*pAttr = con.pConAttr + (nLine * con.nTextWidth);
		if (pLen)
			*pLen = con.nTextWidth;
	}

	return TRUE;
}

void CRealBuffer::PrepareColorTable(bool bExtendFonts, CharAttr (&lcaTableExt)[0x100], CharAttr (&lcaTableOrg)[0x100], const Settings::AppSettings* pApp /*= NULL*/)
{
	CharAttr lca; // crForeColor, crBackColor, nFontIndex, nForeIdx, nBackIdx, crOrigForeColor, crOrigBackColor
	//COLORREF lcrForegroundColors[0x100], lcrBackgroundColors[0x100];
	//BYTE lnForegroundColors[0x100], lnBackgroundColors[0x100], lnFontByIndex[0x100];
	int  nColorIndex = 0;
	if (!pApp)
		pApp = gpSet->GetAppSettings(mp_RCon->GetActiveAppSettingsId());
	BYTE nFontNormalColor = pApp->FontNormalColor();
	BYTE nFontBoldColor = pApp->FontBoldColor();
	BYTE nFontItalicColor = pApp->FontItalicColor();

	for (int nBack = 0; nBack <= 0xF; nBack++)
	{
		for (int nFore = 0; nFore <= 0xF; nFore++, nColorIndex++)
		{
			memset(&lca, 0, sizeof(lca));
			lca.nForeIdx = nFore;
			lca.nBackIdx = nBack;
			lca.crForeColor = lca.crOrigForeColor = mp_RCon->mp_VCon->mp_Colors[lca.nForeIdx];
			lca.crBackColor = lca.crOrigBackColor = mp_RCon->mp_VCon->mp_Colors[lca.nBackIdx];
			lcaTableOrg[nColorIndex] = lca;

			if (bExtendFonts)
			{
				if (nBack == nFontBoldColor)  // nFontBoldColor may be -1, ����� �� ���� �� ��������
				{
					if (nFontNormalColor != 0xFF)
						lca.nBackIdx = nFontNormalColor;

					lca.nFontIndex = 1; //  Bold
					lca.crBackColor = lca.crOrigBackColor = mp_RCon->mp_VCon->mp_Colors[lca.nBackIdx];
				}
				else if (nBack == nFontItalicColor)  // nFontItalicColor may be -1, ����� �� ���� �� ��������
				{
					if (nFontNormalColor != 0xFF)
						lca.nBackIdx = nFontNormalColor;

					lca.nFontIndex = 2; // Italic
					lca.crBackColor = lca.crOrigBackColor = mp_RCon->mp_VCon->mp_Colors[lca.nBackIdx];
				}
			}

			lcaTableExt[nColorIndex] = lca;
		}
	}
}

// nWidth � nHeight ��� �������, ������� ����� �������� VCon (��� ����� ��� �� ������������ �� ���������?
void CRealBuffer::GetConsoleData(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight)
{
	if (!this) return;

	//DWORD cbDstBufSize = nWidth * nHeight * 2;
	DWORD cwDstBufSize = nWidth * nHeight;
	_ASSERTE(nWidth != 0 && nHeight != 0);
	bool bDataValid = false;

	if (nWidth == 0 || nHeight == 0)
		return;

	#ifdef _DEBUG
	if (mp_RCon->mb_DebugLocked)
	{
		return;
	}
	#endif
	
	con.bConsoleDataChanged = FALSE;

	// ������������ ������������� ������, �� ��������� �������
	//TODO("� ��������, ��� ����� ������ �� ������, � ������ ��� ����������");
	bool lbIsFar = (mp_RCon->GetFarPID() != 0);
	bool lbAllowHilightFileLine = mp_RCon->IsFarHyperlinkAllowed(false);
	if (!lbAllowHilightFileLine && (con.etrLast != etr_None))
		StoreLastTextRange(etr_None);
	WARNING("lbIsFar - ������ �� �������� �� �������� � ���������� �����������?");
	const Settings::AppSettings* pApp = gpSet->GetAppSettings(mp_RCon->GetActiveAppSettingsId());
	_ASSERTE(pApp!=NULL);
	// 120331 - ����� ������������ ��������� ���.������?
	bool bExtendColors = /*lbIsFar &&*/ pApp->ExtendColors();
	BYTE nExtendColorIdx = pApp->ExtendColorIdx();
	bool bExtendFonts = lbIsFar && pApp->ExtendFonts();
	bool lbFade = mp_RCon->mp_VCon->isFade;
	//BOOL bUseColorKey = gpSet->isColorKey  // ������ ���� ������� � ���������
	//	&& mp_RCon->isFar(TRUE/*abPluginRequired*/) // � ���� �������� ������ (����� � ������� �� �����������)
	//	&& (mp_tabs && mn_tabsCount>0 && mp_tabs->Current) // ������� ���� - ������
	//	&& !(mb_LeftPanel && mb_RightPanel) // � ���� �� ���� ������ ��������
	//	&& (!con.m_ci.bVisible || con.m_ci.dwSize<30) // � ������ �� ������� ����� ��������
	//	;
	CharAttr lca, lcaTableExt[0x100], lcaTableOrg[0x100], *lcaTable; // crForeColor, crBackColor, nFontIndex, nForeIdx, nBackIdx, crOrigForeColor, crOrigBackColor
	//COLORREF lcrForegroundColors[0x100], lcrBackgroundColors[0x100];
	//BYTE lnForegroundColors[0x100], lnBackgroundColors[0x100], lnFontByIndex[0x100];

	TODO("OPTIMIZE: � ��������, ��� ����� ������ �� ������, � ������ ��� ����������");
	PrepareColorTable(bExtendFonts, lcaTableExt, lcaTableOrg, pApp);
	
	lcaTable = lcaTableOrg;

	MSectionLock csData; csData.Lock(&csCON);
	con.LastStartReadBufferTick = GetTickCount();
	HEAPVAL
	wchar_t wSetChar = L' ';
	CharAttr lcaDef;
	BYTE nDefTextAttr = (mp_RCon->GetDefaultBackColorIdx()<<4)|(mp_RCon->GetDefaultTextColorIdx());
	_ASSERTE(nDefTextAttr<countof(lcaTableOrg));
	lcaDef = lcaTable[nDefTextAttr]; // LtGray on Black
	//WORD    wSetAttr = 7;
	#ifdef _DEBUG
	wSetChar = (wchar_t)8776; //wSetAttr = 12;
	lcaDef = lcaTable[12]; // Red on Black
	#endif

	int nXMax = 0, nYMax = 0;
	int nX = 0, nY = 0;
	wchar_t  *pszDst = pChar;
	CharAttr *pcaDst = pAttr;

	if ((m_Type == rbt_DumpScreen) || (m_Type == rbt_Alternative) || (m_Type == rbt_Selection) || (m_Type == rbt_Find))
	{
		bDataValid = true;
		nXMax = min(nWidth, dump.crSize.X);
		int nFirstLine = con.m_sbi.srWindow.Top;
		nYMax = min(nHeight, (dump.crSize.Y - nFirstLine));
		wchar_t* pszSrc = dump.pszBlock1 + (nFirstLine * dump.crSize.X) + con.m_sbi.srWindow.Left;
		CharAttr* pcaSrc = dump.pcaBlock1 + (nFirstLine * dump.crSize.X) + con.m_sbi.srWindow.Left;
		for (int Y = 0; Y < nYMax; Y++)
		{
			wmemmove(pszDst, pszSrc, nXMax);
			memmove(pcaDst, pcaSrc, nXMax*sizeof(*pcaDst));

			// ����� �������� ������
			TODO("���������� �� ���� Flags. ������.");
			for (int i = 0; i < nXMax; i++)
				pcaDst[i].Flags = 0;

			if (nWidth > nXMax)
			{
				wmemset(pszDst+nXMax, wSetChar, nWidth-nXMax);
				for (int i = nXMax; i < nWidth; i++)
					pcaDst[i] = lcaDef;
			}

			pszDst += nWidth;
			pcaDst += nWidth;
			pszSrc += dump.crSize.X;
			pcaSrc += dump.crSize.X;
		}
		#ifdef _DEBUG
		*pszDst = 0; // ��� ���������
		#endif
		// ������� ������ �� ��������� ���� ����
	}
	else
	{
		if (!con.pConChar || !con.pConAttr)
		{
			nYMax = nHeight; // ����� ������� �� �����, ��� ������ "reset"

			wmemset(pChar, wSetChar, cwDstBufSize);

			for (DWORD i = 0; i < cwDstBufSize; i++)
				pAttr[i] = lcaDef;

			//wmemset((wchar_t*)pAttr, wSetAttr, cbDstBufSize);
			//} else if (nWidth == con.nTextWidth && nHeight == con.nTextHeight) {
			//    TODO("�� ����� ������� ������� ����� ������������ - ������ �� �� ��� �����...");
			//    //_ASSERTE(*con.pConChar!=ucBoxDblVert);
			//    memmove(pChar, con.pConChar, cbDstBufSize);
			//    WARNING("��� �������� �� for");
			//    memmove(pAttr, con.pConAttr, cbDstBufSize);
		}
		else
		{
			bDataValid = true;

			TODO("�� ����� ������� ������� ����� ������������ - ������ �� �� ��� �����...");
			//_ASSERTE(*con.pConChar!=ucBoxDblVert);
			nYMax = min(nHeight,con.nTextHeight);
			wchar_t  *pszSrc = con.pConChar;
			WORD     *pnSrc = con.pConAttr;
			const AnnotationInfo *pcolSrc = NULL;
			const AnnotationInfo *pcolEnd = NULL;
			BOOL bUseColorData = FALSE, bStartUseColorData = FALSE;

			if (lbAllowHilightFileLine)
			{
				// ���� ���� ����������� - ����� ���������
				// ���� ���� ���� �� ��������� - ����� ��� ����������.
				/*if ((con.mcr_FileLineStart.X == con.mcr_FileLineEnd.X)
					|| (con.mcr_FileLineStart.Y != mcr_LastMousePos.Y)
					|| (con.mcr_FileLineStart.X > mcr_LastMousePos.X || con.mcr_FileLineEnd.X < mcr_LastMousePos.X))*/
				if ((mp_RCon->mp_ABuf == this) && gpConEmu->isMeForeground())
				{
					ProcessFarHyperlink();
				}
			}

			if (gpSet->isTrueColorer
				&& mp_RCon->m_TrueColorerMap.IsValid()
				&& mp_RCon->mp_TrueColorerData
				/*&& mp_RCon->isFar()*/)
			{
				pcolSrc = mp_RCon->mp_TrueColorerData;
				pcolEnd = mp_RCon->mp_TrueColorerData + mp_RCon->m_TrueColorerMap.Ptr()->bufferSize;
				bUseColorData = TRUE;
				WARNING("���� far/w - pcolSrc ����� ������� �����, bStartUseColorData=TRUE, bUseColorData=FALSE");
				if (con.bBufferHeight)
				{
					if (mp_RCon->isFar() && !mp_RCon->isFarBufferSupported())
					{
						bUseColorData = FALSE;
					}
					else
					{
						int nShiftRows = (con.m_sbi.dwSize.Y - nHeight) - con.m_sbi.srWindow.Top;
						_ASSERTE(nShiftRows>=0);
						if (nShiftRows > 0)
						{
							_ASSERTE(con.nTextWidth == (con.m_sbi.srWindow.Right - con.m_sbi.srWindow.Left + 1));
							pcolSrc -= nShiftRows*con.nTextWidth;
							bUseColorData = FALSE;
							bStartUseColorData = TRUE;
						}
					}
				}
			}

			DWORD cbDstLineSize = nWidth * 2;
			DWORD cnSrcLineLen = con.nTextWidth;
			DWORD cbSrcLineSize = cnSrcLineLen * 2;

			#ifdef _DEBUG
			if (con.nTextWidth != con.m_sbi.dwSize.X)
			{
				_ASSERTE(con.nTextWidth == con.m_sbi.dwSize.X);
			}
			#endif

			DWORD cbLineSize = min(cbDstLineSize,cbSrcLineSize);
			int nCharsLeft = max(0, (nWidth-con.nTextWidth));
			//int nY, nX;
			//120331 - �������� �������� �� "������" � ������ ������
			BYTE attrBackLast = 0;
			int nExtendStartsY = 0;
			//int nPrevDlgBorder = -1;
			
			bool lbStoreBackLast = false;
			if (bExtendColors)
			{
				BYTE FirstBackAttr = lcaTable[(*pnSrc) & 0xFF].nBackIdx;
				if (FirstBackAttr != nExtendColorIdx)
					attrBackLast = FirstBackAttr;

				const CEFAR_INFO_MAPPING* pFarInfo = lbIsFar ? mp_RCon->GetFarInfo() : NULL;
				if (pFarInfo)
				{
					// ���� � �������� ����� "����������" ������ ���� ������� - ������
					// ������������ ������ �������� "������" ������� ��� ������� ����.
					// � ���������, ����� ������� ������ �������� ������ ����� ��� �������� ��� ��������.
					if (((pFarInfo->nFarColors[col_PanelText] & 0xF0) >> 4) != nExtendColorIdx)
						lbStoreBackLast = true;
					else
						attrBackLast = FirstBackAttr;

					if (pFarInfo->FarInterfaceSettings.AlwaysShowMenuBar || mp_RCon->isEditor() || mp_RCon->isViewer())
						nExtendStartsY = 1; // ���������� ��������� ������ ���� 
				}
				else
				{
					lbStoreBackLast = true;
				}
			}

			// ���������� ������
			for (nY = 0; nY < nYMax; nY++)
			{
				if (nY == 1) lcaTable = lcaTableExt;

				// �����
				memmove(pszDst, pszSrc, cbLineSize);

				if (nCharsLeft > 0)
					wmemset(pszDst+cnSrcLineLen, wSetChar, nCharsLeft);

				// ��������
				DWORD atr = 0;

				if (mp_RCon->mn_InRecreate)
				{
					lca = lcaTable[7];

					for (nX = 0; nX < (int)cnSrcLineLen; nX++, pnSrc++, pcolSrc++)
					{
						pcaDst[nX] = lca;
					}
				}
				else
				{
					bool lbHilightFileLine = lbAllowHilightFileLine 
							&& (con.m_sel.dwFlags == 0)
							&& (nY == con.mcr_FileLineStart.Y)
							&& (con.mcr_FileLineStart.X < con.mcr_FileLineEnd.X);
					for (nX = 0; nX < (int)cnSrcLineLen; nX++, pnSrc++, pcolSrc++)
					{
						atr = (*pnSrc) & 0xFF; // ��������� ������ ������ ���� - ��� ������� ������
						TODO("OPTIMIZE: lca = lcaTable[atr];");
						lca = lcaTable[atr];
						TODO("OPTIMIZE: ������� �������� bExtendColors �� �����");

						if (bExtendColors && (nY >= nExtendStartsY))
						{
							if (lca.nBackIdx == nExtendColorIdx)
							{
								lca.nBackIdx = attrBackLast; // ��� ����� �������� �� ������� ���� �� �������� ������
								lca.nForeIdx += 0x10;
								lca.crForeColor = lca.crOrigForeColor = mp_RCon->mp_VCon->mp_Colors[lca.nForeIdx];
								lca.crBackColor = lca.crOrigBackColor = mp_RCon->mp_VCon->mp_Colors[lca.nBackIdx];
							}
							else if (lbStoreBackLast)
							{
								attrBackLast = lca.nBackIdx; // �������� ������� ���� ���������� ������
							}
						}

						// Colorer & Far - TrueMod
						TODO("OPTIMIZE: ������� �������� bUseColorData �� �����");

						if (bStartUseColorData)
						{
							// � ������ "far /w" ����� ����� ����� �������� ���� ������� ������� �������,
							// ���� ����� ������� ��������� �����
							if (pcolSrc >= mp_RCon->mp_TrueColorerData)
								bUseColorData = TRUE;
						}
						
						if (bUseColorData)
						{
							if (pcolSrc >= pcolEnd)
							{
								bUseColorData = FALSE;
							}
							else
							{
								TODO("OPTIMIZE: ������ � ������� ����� ������ ����...");

								if (pcolSrc->fg_valid)
								{
									lca.nFontIndex = 0; //bold/italic/underline, ������������ ����
									lca.crForeColor = lbFade ? gpSet->GetFadeColor(pcolSrc->fg_color) : pcolSrc->fg_color;

									if (pcolSrc->bk_valid)
										lca.crBackColor = lbFade ? gpSet->GetFadeColor(pcolSrc->bk_color) : pcolSrc->bk_color;
								}
								else if (pcolSrc->bk_valid)
								{
									lca.nFontIndex = 0; //bold/italic/underline, ������������ ����
									lca.crBackColor = lbFade ? gpSet->GetFadeColor(pcolSrc->bk_color) : pcolSrc->bk_color;
								}

								// nFontIndex: 0 - normal, 1 - bold, 2 - italic, 3 - bold&italic,..., 4 - underline, ...
								if (pcolSrc->style)
									lca.nFontIndex = pcolSrc->style & 7;
							}
						}

						//if (lbHilightFileLine && (nX >= con.mcr_FileLineStart.X) && (nX <= con.mcr_FileLineEnd.X))
						//	lca.nFontIndex |= 4; // ���������� ��� ��� Underline

						TODO("OPTIMIZE: lca = lcaTable[atr];");
						pcaDst[nX] = lca;
						//memmove(pcaDst, pnSrc, cbLineSize);
					}

					if (lbHilightFileLine)
					{
						int nFrom = con.mcr_FileLineStart.X;
						int nTo = min(con.mcr_FileLineEnd.X,(int)cnSrcLineLen);
						for (nX = nFrom; nX <= nTo; nX++)
						{
							pcaDst[nX].nFontIndex |= 4; // ���������� ��� ��� Underline
						}
					}
				}

				// ������ ������� (���� �������� ������� �������, ��� ���� �������
				for (nX = cnSrcLineLen; nX < nWidth; nX++)
				{
					pcaDst[nX] = lcaDef;
				}

				// Far2 ���������� ������� 'A' � ������ ������ ���� �������
				// ���� ���� ������� ���� ���� ����� ������� � Extend Font Colors
				if (bExtendFonts && ((nY+1) == nYMax) && mp_RCon->isFar()
						&& (pszDst[nWidth-1] == L'A') && (atr == 0xCF))
				{
					// ������� "������" ���� � �����
					pcaDst[nWidth-1] = lcaTable[atr];
				}

				// Next line
				pszDst += nWidth; pszSrc += cnSrcLineLen;
				pcaDst += nWidth; //pnSrc += con.nTextWidth;
			}
		}
	} // rbt_Primary

	// ���� ����� ��������� ������� ������, ��� ������� � ������� - ��������� ���
	for (nY = nYMax; nY < nHeight; nY++)
	{
		wmemset(pszDst, wSetChar, nWidth);
		pszDst += nWidth;

		//wmemset((wchar_t*)pcaDst, wSetAttr, nWidth);
		for (nX = 0; nX < nWidth; nX++)
		{
			pcaDst[nX] = lcaDef;
		}

		pcaDst += nWidth;
	}

	// ����� ��������� ������������ ��������� ������� - �������������� ������ ASCIIZ. ���� pChars ����� � \0 ���������?
	*pszDst = 0;

	if (bDataValid)
	{
		// ����������� ������ ��� Transparent
		// ����������� �������� ����� ������ ��� ���������� ������������,
		// ��� ��� ���������������� ������
		// ���� ���� �� (gpSet->NeedDialogDetect()) - ����� �������� ���������� ���������������.
		PrepareTransparent(pChar, pAttr, nWidth, nHeight);

		if (mn_LastRgnFlags != m_Rgn.GetFlags())
		{
			// ���������� ����� ������ � �������� �����
			FindPanels();

			WARNING("�� �����, ��� ��� ������� ����� ��� ���������� �������� �������");
			// �������� ������� ������
			if (mp_RCon->isActive() && this == mp_RCon->mp_ABuf)
			{
				PostMessage(mp_RCon->GetView(), WM_SETCURSOR, -1, -1);
			}

			mn_LastRgnFlags = m_Rgn.GetFlags();
		}

		if (con.m_sel.dwFlags)
		{
			BOOL lbStreamMode = (con.m_sel.dwFlags & CONSOLE_TEXT_SELECTION) == CONSOLE_TEXT_SELECTION;
			SMALL_RECT rc = con.m_sel.srSelection;
			if (rc.Left<0) rc.Left = 0; else if (rc.Left>=nWidth) rc.Left = nWidth-1;
			if (rc.Top<0) rc.Top = 0; else if (rc.Top>=nHeight) rc.Top = nHeight-1;
			if (rc.Right<0) rc.Right = 0; else if (rc.Right>=nWidth) rc.Right = nWidth-1;
			if (rc.Bottom<0) rc.Bottom = 0; else if (rc.Bottom>=nHeight) rc.Bottom = nHeight-1;

			// ��� �������������� ��������� ���������� ������������ � ������ ����������� ���� ��������� (lcaSel)
			//CharAttr lcaSel = lcaTable[gpSet->isCTSColorIndex]; // Black on LtGray
			BYTE nForeIdx = (gpSet->isCTSColorIndex & 0xF);
			COLORREF crForeColor = mp_RCon->mp_VCon->mp_Colors[nForeIdx];
			BYTE nBackIdx = (gpSet->isCTSColorIndex & 0xF0) >> 4;
			COLORREF crBackColor = mp_RCon->mp_VCon->mp_Colors[nBackIdx];
			int nX1, nX2;

			// Block mode
			for (nY = rc.Top; nY <= rc.Bottom; nY++)
			{
				if (!lbStreamMode)
				{
					nX1 = rc.Left; nX2 = rc.Right;
				}
				else
				{
					nX1 = (nY == rc.Top) ? rc.Left : 0;
					nX2 = (nY == rc.Bottom) ? rc.Right : (nWidth-1);
				}

				pcaDst = pAttr + nWidth*nY + nX1;

				for(nX = nX1; nX <= nX2; nX++, pcaDst++)
				{
					//pcaDst[nX] = lcaSel; -- ����� �� ���������� ����� ����� � �������� - ������ �� �����
					pcaDst->crForeColor = pcaDst->crOrigForeColor = crForeColor;
					pcaDst->crBackColor = pcaDst->crOrigBackColor = crBackColor;
					pcaDst->nForeIdx = nForeIdx;
					pcaDst->nBackIdx = nBackIdx;
					#if 0
					pcaDst->bTransparent = FALSE;
					#else
					pcaDst->Flags &= ~CharAttr_Transparent;
					#endif
					pcaDst->ForeFont = 0;
				}
			}

			//} else {
			//	int nX1, nX2;
			//	for (nY = rc.Top; nY <= rc.Bottom; nY++) {
			//		pnDst = pAttr + nWidth*nY;
			//		nX1 = (nY == rc.Top) ? rc.Left : 0;
			//		nX2 = (nY == rc.Bottom) ? rc.Right : (nWidth-1);
			//		for (nX = nX1; nX <= nX2; nX++) {
			//			pnDst[nX] = lcaSel;
			//		}
			//	}
			//}
		}
	}

	// ���� ��������� �������� "������" - ������������� ���������� ������ ������� ������ ������������� ������
	if (!gpSet->isStatusBarShow && mp_RCon->ms_ConStatus[0])
	{
		int nLen = _tcslen(mp_RCon->ms_ConStatus);
		wmemcpy(pChar, mp_RCon->ms_ConStatus, nLen);

		if (nWidth>nLen)
			wmemset(pChar+nLen, L' ', nWidth-nLen);

		//wmemset((wchar_t*)pAttr, 0x47, nWidth);
		lca = lcaTableExt[7];

		for(int i = 0; i < nWidth; i++)
			pAttr[i] = lca;
	}

	con.LastEndReadBufferTick = GetTickCount();

	//FIN
	HEAPVAL
	csData.Unlock();
	return;
}

DWORD_PTR CRealBuffer::GetKeybLayout()
{
	return con.dwKeybLayout;
}

void CRealBuffer::SetKeybLayout(DWORD_PTR anNewKeyboardLayout)
{
	con.dwKeybLayout = anNewKeyboardLayout;
}

DWORD CRealBuffer::GetConMode()
{
	return con.m_dwConsoleMode;
}

int CRealBuffer::GetStatusLineCount(int nLeftPanelEdge)
{
	if (!this)
		return 0;

	if (!mp_RCon->isFar() || !con.pConChar || !con.nTextWidth)
		return 0;
	
	int nBottom, nLeft;
	if (nLeftPanelEdge > mr_LeftPanelFull.left)
	{
		nBottom = mr_RightPanelFull.bottom;
		nLeft = mr_RightPanelFull.left;
	}
	else
	{
		nBottom = mr_LeftPanelFull.bottom;
		nLeft = mr_LeftPanelFull.left;
	}
	if (nBottom < 5)
		return 0; // ����������� ������ ������

	for (int i = 2; i <= 11 && i < nBottom; i++)
	{
		if (con.pConChar[con.nTextWidth*(nBottom-i)+nLeft] == ucBoxDblVertSinglRight)
		{
			return (i - 1);
		}
	}
	
	return 0;
}

void CRealBuffer::PrepareTransparent(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight)
{
	//if (!mp_ConsoleInfo)
	if (!pChar || !pAttr)
		return;

	TODO("������ �� m_FarInfo ���� � ���� ���������.");
	CEFAR_INFO_MAPPING FI;
	
	if (m_Type == rbt_Primary)
	{
		FI = mp_RCon->m_FarInfo;

		// �� (mp_RCon->isViewer() || mp_RCon->isEditor()) ���������������
		// ������, �.�. CheckFarStates ��� �� ��� ������
		BOOL bViewerOrEditor = FALSE;
		if (mp_RCon->GetFarPID(TRUE))
		{
			int nTabType = mp_RCon->GetActiveTabType();
			bViewerOrEditor = ((nTabType & 0xFF) == 2 || (nTabType & 0xFF) == 3);
		}

		if (!bViewerOrEditor)
		{
			WARNING("����������� ���, ���� FindPanels ��� �� ��� ������");

			if (mb_LeftPanel)
			{
				FI.bFarLeftPanel = true;
				FI.FarLeftPanel.PanelRect = mr_LeftPanelFull;
			}
			else
			{
				FI.bFarLeftPanel = false;
			}

			if (mb_RightPanel)
			{
				FI.bFarRightPanel = true;
				FI.FarRightPanel.PanelRect = mr_RightPanelFull;
			}
			else
			{
				FI.bFarRightPanel = false;
			}

			FI.bViewerOrEditor = FALSE;
		}
		else
		{
			FI.bViewerOrEditor = TRUE;
			FI.bFarLeftPanel = false;
			FI.bFarRightPanel = false;
		}

		//if (!FI.bFarLeftPanel && !FI.bFarRightPanel)
		//{
		//	// ���� ��� ������� - ��� ����� ���� ������/��������
		//	FI.bViewerOrEditor = (mp_RCon->isViewer() || mp_RCon->isEditor());
		//}
	}
	else
	{
		ZeroStruct(FI);
		TODO("��������� CEFAR_INFO_MAPPING �� �����");
	}

	m_Rgn.SetNeedTransparency(gpSet->isUserScreenTransparent);
	TODO("��� �������� ����� ������ �� �� ���� � ������� ���� ���������/��������");
	m_Rgn.PrepareTransparent(&FI, mp_RCon->mp_VCon->mp_Colors, GetSBI(), pChar, pAttr, nWidth, nHeight);
	
	#ifdef _DEBUG
	int nCount = m_Rgn.GetDetectedDialogs(0,NULL,NULL);

	if (nCount == 1)
	{
		nCount = 1;
	}
	#endif
}

// ����� ������, �������� mp_RCon->mn_ConsoleProgress
void CRealBuffer::FindPanels()
{
	TODO("��������� ������� ����� �� �������� �� �������");
	WARNING("!!! ����� ��� ��������� ������ '���� ������ ��������'");
	
	#ifdef _DEBUG
	if (mp_RCon->mb_DebugLocked)
		return;
	WARNING("this==mp_RCon->mp_RBuf ?");
	#endif

	RECT rLeftPanel = MakeRect(-1,-1);
	RECT rLeftPanelFull = rLeftPanel;
	RECT rRightPanel = rLeftPanel;
	RECT rRightPanelFull = rLeftPanel;
	BOOL bLeftPanel = FALSE;
	BOOL bRightPanel = FALSE;
	BOOL bMayBePanels = FALSE;
	BOOL lbNeedUpdateSizes = FALSE;
	BOOL lbPanelsChanged = FALSE;
	short nLastProgress = mp_RCon->mn_ConsoleProgress;
	short nNewProgress = -1;
	/*
	����� �����. ��� ������� ������� CtrlLeft/CtrlRight ������ ������ ���������
	���������� ������� �� ��������� ������ � ��� ����� ����������. � ����� �������
	����� ���������� �� ������������� ������, ��� ������ ������
	����������� ������� � CRealConsole::FindPanels - �������� �������, ���
	����� ������ - ������������� :(
	*/
	WARNING("�������� �������� �� ���� ������ �������, ����� �� ��� �� ���� ������������ ��������");
	// �� ���� �� ����� ������ �� ���� ������ �������� (������������/������� �������� ������ �������������� ����� ������)

	// ������� ��������� (mn_ProgramStatus & CES_FARACTIVE) � �.�.
	if (mp_RCon->isFar())
	{
		if (mp_RCon->isFar(TRUE) && mp_RCon->m_FarInfo.cbSize)
		{
			if (mp_RCon->m__FarInfo.Ptr() && mp_RCon->m__FarInfo.Ptr()->nFarInfoIdx != mp_RCon->m_FarInfo.nFarInfoIdx)
			{
				mp_RCon->m__FarInfo.GetTo(&mp_RCon->m_FarInfo, sizeof(mp_RCon->m_FarInfo));
			}
		}

		// ���� ������� �������� ��� ������ (��� �������, �����������, � �.�.) - ������ ������������
		if ((mp_RCon->mn_FarStatus & CES_NOTPANELFLAGS) == 0)
			bMayBePanels = TRUE; // ������ ���� ���
	}

	if (bMayBePanels && con.nTextHeight >= MIN_CON_HEIGHT && con.nTextWidth >= MIN_CON_WIDTH)
	{
		uint nY = 0;
		BOOL lbIsMenu = FALSE;

		if (con.pConChar[0] == L' ')
		{
			lbIsMenu = TRUE;

			for(int i=0; i<con.nTextWidth; i++)
			{
				if (con.pConChar[i]==ucBoxDblHorz || con.pConChar[i]==ucBoxDblDownRight || con.pConChar[i]==ucBoxDblDownLeft)
				{
					lbIsMenu = FALSE; break;
				}
			}

			if (lbIsMenu)
				nY ++; // ������ �����, ������ ������ - ����
		}
		else if ((((con.pConChar[0] == L'R') && ((con.pConAttr[0] & 0xFF) == 0x4F))
					|| ((con.pConChar[0] == L'P') && ((con.pConAttr[0] & 0xFF) == 0x2F)))
			&& con.pConChar[1] == L' ')
		{
			for(int i=1; i<con.nTextWidth; i++)
			{
				if (con.pConChar[i]==ucBoxDblHorz || con.pConChar[i]==ucBoxDblDownRight || con.pConChar[i]==ucBoxDblDownLeft)
				{
					lbIsMenu = FALSE; break;
				}
			}

			if (lbIsMenu)
				nY ++; // ������ �����, ������ ������ - ����, ��� ���������� ������ �������
		}

		uint nIdx = nY*con.nTextWidth;
		// ����� ������
		BOOL bFirstCharOk = (nY == 0)
		                    && (
		                        (con.pConChar[0] == L'R' && (con.pConAttr[0] & 0xFF) == 0x4F) // ������ ������ �������
		                        || (con.pConChar[0] == L'P' && (con.pConAttr[0] & 0xFF) == 0x2F) // ������ ��������������� �������
		                    );

		BOOL bFarShowColNames = TRUE;
		BOOL bFarShowStatus = TRUE;
		const CEFAR_INFO_MAPPING *pFar = NULL;
		if (mp_RCon->m_FarInfo.cbSize)
		{
			pFar = &mp_RCon->m_FarInfo;
			if (pFar)
			{
				if ((pFar->FarPanelSettings.ShowColumnTitles) == 0) //-V112
					bFarShowColNames = FALSE;
				if ((pFar->FarPanelSettings.ShowStatusLine) == 0)
					bFarShowStatus = FALSE;
			}
		}

		// ��-�� ������ ��������� FAR2 ���� ������ '[' - ����� ������
		//if (( ((bFirstCharOk || con.pConChar[nIdx] == L'[') && (con.pConChar[nIdx+1]>=L'0' && con.pConChar[nIdx+1]<=L'9')) // ������� ��������� ����������/��������
		if ((
		            ((bFirstCharOk || con.pConChar[nIdx] != ucBoxDblDownRight)
		             && (con.pConChar[nIdx+1]>=L'0' && con.pConChar[nIdx+1]<=L'9')) // ������� ��������� ����������/��������
		            ||
		            ((bFirstCharOk || con.pConChar[nIdx] == ucBoxDblDownRight)
		             && ((con.pConChar[nIdx+1] == ucBoxDblHorz && bFarShowColNames)
		                 || con.pConChar[nIdx+1] == ucBoxSinglDownDblHorz // ���.���� ���, ������ �����
						 || con.pConChar[nIdx+1] == ucBoxDblDownDblHorz
		                 || (con.pConChar[nIdx+1] == L'[' && con.pConChar[nIdx+2] == ucLeftScroll) // ScreenGadgets, default
						 || (!bFarShowColNames && con.pConChar[nIdx+1] != ucBoxDblHorz
							&& con.pConChar[nIdx+1] != ucBoxSinglDownDblHorz && con.pConChar[nIdx+1] != ucBoxDblDownDblHorz)
		                ))
		        )
		        && con.pConChar[nIdx+con.nTextWidth] == ucBoxDblVert) // ������� ����� ������������ ����
		{
			for(int i=2; !bLeftPanel && i<con.nTextWidth; i++)
			{
				// ����� ������ ���� ����� ������
				if (con.pConChar[nIdx+i] == ucBoxDblDownLeft
				        && ((con.pConChar[nIdx+i-1] == ucBoxDblHorz)
				            || con.pConChar[nIdx+i-1] == ucBoxSinglDownDblHorz // ������ ���� ������
							|| con.pConChar[nIdx+i-1] == ucBoxDblDownDblHorz
				            || (con.pConChar[nIdx+i-1] == L']' && con.pConChar[nIdx+i-2] == L'\\') // ScreenGadgets, default
							)
				        // ����� ���� ������� AltHistory
				        /*&& con.pConChar[nIdx+i+con.nTextWidth] == ucBoxDblVert*/)
				{
					uint nBottom = con.nTextHeight - 1;

					while(nBottom > 4) //-V112
					{
						if (con.pConChar[con.nTextWidth*nBottom] == ucBoxDblUpRight
						        /*&& con.pConChar[con.nTextWidth*nBottom+i] == ucBoxDblUpLeft*/)
						{
							rLeftPanel.left = 1;
							rLeftPanel.top = nY + (bFarShowColNames ? 2 : 1);
							rLeftPanel.right = i-1;
							if (bFarShowStatus)
							{
								rLeftPanel.bottom = nBottom - 3;
								for (int j = (nBottom - 3); j > (int)(nBottom - 13) && j > rLeftPanel.top; j--)
								{
									if (con.pConChar[con.nTextWidth*j] == ucBoxDblVertSinglRight)
									{
										rLeftPanel.bottom = j - 1; break;
									}
								}
							}
							else
							{
								rLeftPanel.bottom = nBottom - 1;
							}
							rLeftPanelFull.left = 0;
							rLeftPanelFull.top = nY;
							rLeftPanelFull.right = i;
							rLeftPanelFull.bottom = nBottom;
							bLeftPanel = TRUE;
							break;
						}

						nBottom --;
					}
				}
			}
		}

		// (���� ���� ����� ������ � ��� �� FullScreen) ��� ����� ������ ��� ������
		if ((bLeftPanel && rLeftPanelFull.right < con.nTextWidth) || !bLeftPanel)
		{
			if (bLeftPanel)
			{
				// ��������� ��������, ����� ������ ��������� �������
				if (con.pConChar[nIdx+rLeftPanelFull.right+1] == ucBoxDblDownRight
				        /*&& con.pConChar[nIdx+rLeftPanelFull.right+1+con.nTextWidth] == ucBoxDblVert*/
				        /*&& con.pConChar[nIdx+con.nTextWidth*2] == ucBoxDblVert*/
				        /*&& con.pConChar[(rLeftPanelFull.bottom+3)*con.nTextWidth+rLeftPanelFull.right+1] == ucBoxDblUpRight*/
				        && con.pConChar[(rLeftPanelFull.bottom+1)*con.nTextWidth-1] == ucBoxDblUpLeft
				  )
				{
					rRightPanel = rLeftPanel; // bottom & top ����� �� rLeftPanel
					rRightPanel.left = rLeftPanelFull.right+2;
					rRightPanel.right = con.nTextWidth-2;
					rRightPanelFull = rLeftPanelFull;
					rRightPanelFull.left = rLeftPanelFull.right+1;
					rRightPanelFull.right = con.nTextWidth-1;
					bRightPanel = TRUE;
				}
			}

			// ������� � FAR2 build 1295 ������ ����� ���� ������ ������
			// ��� ����� ������ ���
			// ��� �������� ������ � FullScreen ������
			if (!bRightPanel)
			{
				// ����� ���������� ��������� ������
				if (((con.pConChar[nIdx+con.nTextWidth-1]>=L'0' && con.pConChar[nIdx+con.nTextWidth-1]<=L'9')  // ������ ����
				        || con.pConChar[nIdx+con.nTextWidth-1] == ucBoxDblDownLeft) // ��� �����
				        && (con.pConChar[nIdx+con.nTextWidth*2-1] == ucBoxDblVert // �� � ������ ������� ������
							|| con.pConChar[nIdx+con.nTextWidth*2-1] == ucUpScroll) // ��� ������� ����������
						)
				{
					int iMinFindX = bLeftPanel ? (rLeftPanelFull.right+1) : 0;
					for(int i=con.nTextWidth-3; !bRightPanel && i>=iMinFindX; i--)
					{
						// ���� ����� ������� ������ ������
						if (con.pConChar[nIdx+i] == ucBoxDblDownRight
						        && ((con.pConChar[nIdx+i+1] == ucBoxDblHorz && bFarShowColNames)
						            || con.pConChar[nIdx+i+1] == ucBoxSinglDownDblHorz // ������ ���� ������
									|| con.pConChar[nIdx+i+1] == ucBoxDblDownDblHorz
						            || (con.pConChar[nIdx+i-1] == L']' && con.pConChar[nIdx+i-2] == L'\\') // ScreenGadgets, default
									|| (!bFarShowColNames && con.pConChar[nIdx+i+1] != ucBoxDblHorz 
										&& con.pConChar[nIdx+i+1] != ucBoxSinglDownDblHorz && con.pConChar[nIdx+i+1] != ucBoxDblDownDblHorz)
									)
						        // ����� ���� ������� AltHistory
						        /*&& con.pConChar[nIdx+i+con.nTextWidth] == ucBoxDblVert*/)
						{
							uint nBottom = con.nTextHeight - 1;

							while(nBottom > 4) //-V112
							{
								if (/*con.pConChar[con.nTextWidth*nBottom+i] == ucBoxDblUpRight
									&&*/ con.pConChar[con.nTextWidth*(nBottom+1)-1] == ucBoxDblUpLeft)
								{
									rRightPanel.left = i+1;
									rRightPanel.top = nY + (bFarShowColNames ? 2 : 1);
									rRightPanel.right = con.nTextWidth-2;
									//rRightPanel.bottom = nBottom - 3;
									if (bFarShowStatus)
									{
										rRightPanel.bottom = nBottom - 3;
										for (int j = (nBottom - 3); j > (int)(nBottom - 13) && j > rRightPanel.top; j--)
										{
											if (con.pConChar[con.nTextWidth*j+i] == ucBoxDblVertSinglRight)
											{
												rRightPanel.bottom = j - 1; break;
											}
										}
									}
									else
									{
										rRightPanel.bottom = nBottom - 1;
									}
									rRightPanelFull.left = i;
									rRightPanelFull.top = nY;
									rRightPanelFull.right = con.nTextWidth-1;
									rRightPanelFull.bottom = nBottom;
									bRightPanel = TRUE;
									break;
								}

								nBottom --;
							}
						}
					}
				}
			}
		}
	}

#ifdef _DEBUG

	if (bLeftPanel && !bRightPanel && rLeftPanelFull.right > 120)
	{
		bLeftPanel = bLeftPanel;
	}

#endif

	if (mp_RCon->isActive())
		lbNeedUpdateSizes = (memcmp(&mr_LeftPanel,&rLeftPanel,sizeof(mr_LeftPanel)) || memcmp(&mr_RightPanel,&rRightPanel,sizeof(mr_RightPanel)));

	lbPanelsChanged = lbNeedUpdateSizes || (mb_LeftPanel != bLeftPanel) || (mb_RightPanel != bRightPanel)
	                  || ((bLeftPanel || bRightPanel) && ((mp_RCon->mn_FarStatus & CES_FILEPANEL) == 0));
	mr_LeftPanel = rLeftPanel; mr_LeftPanelFull = rLeftPanelFull; mb_LeftPanel = bLeftPanel;
	mr_RightPanel = rRightPanel; mr_RightPanelFull = rRightPanelFull; mb_RightPanel = bRightPanel;

	if (bRightPanel || bLeftPanel)
		bMayBePanels = TRUE;
	else
		bMayBePanels = FALSE;

	nNewProgress = -1;

	// mn_ProgramStatus �� �����. ������� ������������ �������� �� ������� Update, � ��� ��� ��� CES_FARACTIVE
	//if (!abResetOnly && (mn_ProgramStatus & CES_FARACTIVE) == 0) {
	if (!bMayBePanels && (mp_RCon->mn_FarStatus & CES_NOTPANELFLAGS) == 0)
	{
		// ��������� ��������� NeroCMD � ��. ���������� ��������
		// ���� ������ ��������� � ������� �������
		int nY = con.m_sbi.dwCursorPosition.Y - con.m_sbi.srWindow.Top;

		if (/*con.m_ci.bVisible && con.m_ci.dwSize -- Update ������ ������?
			&&*/ con.m_sbi.dwCursorPosition.X >= 0 && con.m_sbi.dwCursorPosition.X < con.nTextWidth
		    && nY >= 0 && nY < con.nTextHeight)
		{
			int nIdx = nY * con.nTextWidth;
			// ��������� ��������� NeroCMD � ��. ���������� �������� (���� ������ ��������� � ������� �������)
			nNewProgress = CheckProgressInConsole(con.pConChar+nIdx);
		}
	}

	if (mp_RCon->mn_ConsoleProgress != nNewProgress || nLastProgress != nNewProgress)
	{
		// ���������, ��� �������� �� �������
		mp_RCon->mn_ConsoleProgress = nNewProgress;
		// �������� �������� � ���������
		mp_RCon->mb_ForceTitleChanged = TRUE;
	}

	if (lbPanelsChanged)
	{
		// ����� ������� (�������� ��������), ����� �������� ����� ���������
		mp_RCon->CheckFarStates();
	}

	if (lbNeedUpdateSizes)
		gpConEmu->UpdateSizes();
}

bool CRealBuffer::isSelectionAllowed()
{
	if (!this)
		return false;

	if (!con.pConChar || !con.pConAttr)
		return false; // ���� ������ ������� ��� ���

	if (con.m_sel.dwFlags != 0)
		return true; // ���� ��������� ���� �������� ����� ����

	if (!gpSet->isConsoleTextSelection)
		return false; // ��������� ������ ��������� � ����������
	else if (gpSet->isConsoleTextSelection == 1)
		return true; // ��������� ������
	else if (mp_RCon->isBufferHeight())
	{
		// ����� - ������ � ������ � ����������
		//DWORD nFarPid = 0;

		// �� � FAR2 �������� ����� ������ /w
		if (!mp_RCon->isFarBufferSupported())
			return true;
	}

	//if ((con.m_dwConsoleMode & ENABLE_QUICK_EDIT_MODE) == ENABLE_QUICK_EDIT_MODE)
	//	return true;
	//if (mp_ConsoleInfo && mp_RCon->isFar(TRUE)) {
	//	if ((mp_ConsoleInfo->FarInfo.nFarInterfaceSettings & 4/*FIS_MOUSE*/) == 0)
	//		return true;
	//}
	return false;
}

bool CRealBuffer::isSelectionPresent()
{
	if (!this)
		return false;

	return (con.m_sel.dwFlags != 0);
}

bool CRealBuffer::GetConsoleSelectionInfo(CONSOLE_SELECTION_INFO *sel)
{
	if (!this)
		return false;

	if (!mp_RCon->isSelectionAllowed())
		return false;

	if (sel)
	{
		*sel = con.m_sel;
	}

	return (con.m_sel.dwFlags != 0);
	//return (con.m_sel.dwFlags & CONSOLE_SELECTION_NOT_EMPTY) == CONSOLE_SELECTION_NOT_EMPTY;
}

void CRealBuffer::ConsoleCursorInfo(CONSOLE_CURSOR_INFO *ci)
{
	if (!this) return;

	*ci = con.m_ci;

	// ���� ������ ���������� ����� ������ (ConEmu internal)
	// �� ������ ����� ����������� � �������� ����������!
	if (isSelectionPresent())
	{
		TODO("� ����� �� ���, ��� �������� ����������? ��� �� ���������?");
		if (ci->dwSize < 50)
			ci->dwSize = 50;
	}
	else
	{
		const Settings::AppSettings* pApp = gpSet->GetAppSettings(mp_RCon->GetActiveAppSettingsId());
		bool bActive = mp_RCon->isInFocus();
		if (pApp->CursorIgnoreSize(bActive))
			ci->dwSize = pApp->CursorFixedSize(bActive);
	}
}

void CRealBuffer::GetCursorInfo(COORD* pcr, CONSOLE_CURSOR_INFO* pci)
{
	if (pcr)
	{
		*pcr = con.m_sbi.dwCursorPosition;
	}

	if (pci)
	{
		//*pci = con.m_ci;
		ConsoleCursorInfo(pci);
	}
}

void CRealBuffer::ConsoleScreenBufferInfo(CONSOLE_SCREEN_BUFFER_INFO* sbi)
{
	if (!this) return;

	*sbi = con.m_sbi;

	if (con.m_sel.dwFlags)
	{
		// � ������ ��������� - ��������� ������� ������ ����
		ConsoleCursorPos(&(sbi->dwCursorPosition));
		//if (con.m_sel.dwSelectionAnchor.X == con.m_sel.srSelection.Left)
		//	sbi->dwCursorPosition.X = con.m_sel.srSelection.Right;
		//else
		//	sbi->dwCursorPosition.X = con.m_sel.srSelection.Left;
		//
		//if (con.m_sel.dwSelectionAnchor.Y == con.m_sel.srSelection.Top)
		//	sbi->dwCursorPosition.Y = con.m_sel.srSelection.Bottom;
		//else
		//	sbi->dwCursorPosition.Y = con.m_sel.srSelection.Top;
	}
}

void CRealBuffer::ConsoleCursorPos(COORD *pcr)
{
	if (con.m_sel.dwFlags)
	{
		// � ������ ��������� - ��������� ������� ������ ����
		if (con.m_sel.dwSelectionAnchor.X == con.m_sel.srSelection.Left)
			pcr->X = con.m_sel.srSelection.Right;
		else
			pcr->X = con.m_sel.srSelection.Left;

		if (con.m_sel.dwSelectionAnchor.Y == con.m_sel.srSelection.Top)
			pcr->Y = con.m_sel.srSelection.Bottom + con.nTopVisibleLine;
		else
			pcr->Y = con.m_sel.srSelection.Top + con.nTopVisibleLine;
	}
	else
	{
		*pcr = con.m_sbi.dwCursorPosition;
	}
}

void CRealBuffer::ResetBuffer()
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return;
	}
	con.m_ci.bVisible = TRUE;
	con.m_ci.dwSize = 15;
	con.m_sbi.dwCursorPosition = MakeCoord(0,con.m_sbi.srWindow.Top);

	if (con.pConChar && con.pConAttr)
	{
		MSectionLock sc; sc.Lock(&csCON);
		DWORD OneBufferSize = con.nTextWidth*con.nTextHeight*sizeof(wchar_t);
		memset(con.pConChar,0,OneBufferSize);
		memset(con.pConAttr,0,OneBufferSize);
	}
}

// ������������� ���������� ���������� ������� � ���������� ������ ������
// (������� ����� ������� ������� ������ � ��������������� ������� ������)
bool CRealBuffer::ConsoleRect2ScreenRect(const RECT &rcCon, RECT *prcScr)
{
	if (!this) return false;

	*prcScr = rcCon;

	if (con.bBufferHeight && con.nTopVisibleLine)
	{
		prcScr->top -= con.nTopVisibleLine;
		prcScr->bottom -= con.nTopVisibleLine;
	}

	bool lbRectOK = true;

	if (prcScr->left == 0 && prcScr->right >= con.nTextWidth)
		prcScr->right = con.nTextWidth - 1;

	if (prcScr->left)
	{
		if (prcScr->left >= con.nTextWidth)
			return false;

		if (prcScr->right >= con.nTextWidth)
			prcScr->right = con.nTextWidth - 1;
	}

	if (prcScr->bottom < 0)
	{
		lbRectOK = false; // ��������� ����� �� ������� �����
	}
	else if (prcScr->top >= con.nTextHeight)
	{
		lbRectOK = false; // ��������� ����� �� ������� ����
	}
	else
	{
		// ��������������� �� �������� ��������������
		if (prcScr->top < 0)
			prcScr->top = 0;

		if (prcScr->bottom >= con.nTextHeight)
			prcScr->bottom = con.nTextHeight - 1;

		lbRectOK = (prcScr->bottom > prcScr->top);
	}

	return lbRectOK;
}

DWORD CRealBuffer::GetConsoleCP()
{
	return con.m_dwConsoleCP;
}

DWORD CRealBuffer::GetConsoleOutputCP()
{
	return con.m_dwConsoleOutputCP;
}

bool CRealBuffer::FindRangeStart(COORD& crFrom/*[In/Out]*/, COORD& crTo/*[In/Out]*/, bool& bUrlMode, LPCWSTR pszBreak, LPCWSTR pszUrlDelim, LPCWSTR pszSpacing, LPCWSTR pszUrl, LPCWSTR pszProtocol, LPCWSTR pChar, int nLen)
{
	bool lbRc = false;

	// ������ ��� ������������?
	// ��������� ����� ������ ����� �����
	while ((crFrom.X) > 0 && !wcschr(bUrlMode ? pszUrlDelim : pszBreak, pChar[crFrom.X-1]))
	{
		if (!bUrlMode && pChar[crFrom.X] == L'/')
		{
			if ((crFrom.X >= 2) && ((crFrom.X + 1) < nLen)
				&& ((pChar[crFrom.X+1] == L'/') && (pChar[crFrom.X-1] == L':')
					&& wcschr(pszUrl, pChar[crFrom.X-2]))) // ��� ������� ���� ����� �� ��������
			{
				crFrom.X++;
			}

			if ((crFrom.X >= 3)
				&& ((pChar[crFrom.X-1] == L'/') && (pChar[crFrom.X-2] == L':')
					&& wcschr(pszUrl, pChar[crFrom.X-3]))) // ��� ������� ���� ����� �� ��������
			{
				bUrlMode = true;
				crTo.X = crFrom.X-2;
				crFrom.X -= 3;
				while ((crFrom.X > 0) && wcschr(pszProtocol, pChar[crFrom.X-1]))
					crFrom.X--;
				break;
			}
			else if ((pChar[crFrom.X] == L'/') && (crFrom.X >= 1) && (pChar[crFrom.X-1] == L'/'))
			{	
				crFrom.X++;
				break; // ����������� � ������?
			}
		}
		crFrom.X--;
		if (pChar[crFrom.X] == L':')
		{
			if (pChar[crFrom.X+1] == L' ')
			{
				// ASM - ������������ ����� "test.asasm(1,1)"
				// object.Exception@assembler.d(1239): test.asasm(1,1):
				crFrom.X += 2;
				break;
			}
			else if (pChar[crFrom.X+1] != L'\\' && pChar[crFrom.X+1] != L'/')
			{
				goto wrap; // �� ���
			}
		}
	}
	while (((crFrom.X+1) < nLen) && wcschr(pszSpacing, pChar[crFrom.X]))
		crFrom.X++;
	if (crFrom.X > crTo.X)
	{
		goto wrap; // Fail?
	}

	lbRc = true;
wrap:
	return lbRc;
}

bool CRealBuffer::CheckValidUrl(COORD& crFrom/*[In/Out]*/, COORD& crTo/*[In/Out]*/, bool& bUrlMode, LPCWSTR pszUrlDelim, LPCWSTR pszUrl, LPCWSTR pszProtocol, LPCWSTR pChar, int nLen)
{
	bool lbRc = false;

	// URL? (������ ��� ������ ��� ����������)
	while ((crTo.X < nLen) && wcschr(pszProtocol, pChar[crTo.X]))
		crTo.X++;
	if (((crTo.X+1) < nLen) && (pChar[crTo.X] == L':'))
	{
		if (((crTo.X+4) < nLen) && (pChar[crTo.X+1] == L'/') && (pChar[crTo.X+2] == L'/'))
		{
			bUrlMode = true;
			if (wcschr(pszUrl+2 /*���������� ":/"*/, pChar[crTo.X+3]))
			{
				if (((crTo.X+4) < nLen) // ���� file://c:\xxx ?
					&& ((pChar[crTo.X+3] >= L'a' && pChar[crTo.X+3] <= L'z')
						|| (pChar[crTo.X+3] >= L'A' && pChar[crTo.X+3] <= L'Z'))
					&& (pChar[crTo.X+4] == L':'))
				{
					if (((crTo.X+5) < nLen) && (pChar[crTo.X+5] == L'\\'))
					{
						_ASSERTE(*pszUrlDelim == L'\\');
						pszUrlDelim++;
					}
					crTo.X += 3;
					crFrom = crTo;
					bUrlMode = false;
				}

				if (bUrlMode)
				{
					crFrom = crTo;
					while ((crFrom.X > 0) && wcschr(pszProtocol, pChar[crFrom.X-1]))
						crFrom.X--;
				}
			}
			else
			{
				goto wrap; // Fail
			}
		}
	}

	lbRc = true;
wrap:
	return lbRc;
}

ExpandTextRangeType CRealBuffer::ExpandTextRange(COORD& crFrom/*[In/Out]*/, COORD& crTo/*[Out]*/, ExpandTextRangeType etr, wchar_t* pszText /*= NULL*/, size_t cchTextMax /*= 0*/)
{
	ExpandTextRangeType result = etr_None;

	crTo = crFrom; // Initialize

	COORD crMouse = crFrom; // Save

	// ����� �������� ������
	MSectionLock csData; csData.Lock(&csCON);
	wchar_t* pChar = NULL;
	int nLen = 0;

	if (mp_RCon->mp_VCon && GetConsoleLine(crFrom.Y, &pChar, /*NULL,*/ &nLen, &csData) && pChar)
	{
		TODO("��������� �� ������ ����� ���������� �������������� ���������");
		if (etr == etr_Word)
		{
			while ((crFrom.X) > 0 && !(mp_RCon->mp_VCon->isCharSpace(pChar[crFrom.X-1]) || mp_RCon->mp_VCon->isCharNonSpacing(pChar[crFrom.X-1])))
				crFrom.X--;
			while (((crTo.X+1) < nLen) && !(mp_RCon->mp_VCon->isCharSpace(pChar[crTo.X]) || mp_RCon->mp_VCon->isCharNonSpacing(pChar[crTo.X])))
				crTo.X++;
			result = etr; // OK
		}
		else if (etr == etr_FileAndLine)
		{
			// � ������ ������ �����������: "/\:|*?<>~t~r~n
			const wchar_t  pszBreak[] = {
								/*������������ � FS*/
								L'\"', '|', '*', '?', '<', '>', '\t', '\r', '\n', 
								/*��� �������� - ��������� � �����*/
								ucArrowUp, ucArrowDown, ucDnScroll, ucUpScroll,
								ucBox100, ucBox75, ucBox50, ucBox25,
								ucBoxDblVert, ucBoxSinglVert, ucBoxDblVertSinglRight, ucBoxDblVertSinglLeft,
								ucBoxDblDownRight, ucBoxDblDownLeft, ucBoxDblUpRight,
								ucBoxDblUpLeft, ucBoxSinglDownRight, ucBoxSinglDownLeft, ucBoxSinglUpRight,
								ucBoxSinglUpLeft, ucBoxSinglDownDblHorz, ucBoxSinglUpDblHorz, ucBoxDblDownDblHorz,
								ucBoxDblUpDblHorz, ucBoxSinglDownHorz, ucBoxSinglUpHorz, ucBoxDblDownSinglHorz,
								ucBoxDblUpSinglHorz, ucBoxDblVertRight, ucBoxDblVertLeft, 
								ucBoxSinglVertRight, ucBoxSinglVertLeft, ucBoxDblVertHorz,
								0};
			const wchar_t* pszSpacing = L" \t\xB7\x2192"; //B7 - ����� "Show white spaces", 2192 - ������ ��������� ��� ��
			const wchar_t* pszSeparat = L" \t:(";
			const wchar_t* pszTermint = L":),";
			const wchar_t* pszDigits  = L"0123456789";
			const wchar_t* pszSlashes = L"/\\";
			const wchar_t* pszUrl = L":/%#ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz;?@&=+$,-_.!~*'()0123456789";
			const wchar_t* pszProtocol = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_.";
			const wchar_t* pszEMail = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-.";
			const wchar_t* pszUrlDelim = L"\\\"<>{}[]^` \t\r\n";
			int nColons = 0;
			bool bUrlMode = false, bMaybeMail = false;
			SHORT MailX = -1;

			// ������ ��� ������������?
			// ��������� ����� ������ ����� �����
			if (!FindRangeStart(crFrom, crTo, bUrlMode, pszBreak, pszUrlDelim, pszSpacing, pszUrl, pszProtocol, pChar, nLen))
				goto wrap;

			// URL? (������ ��� ������ ��� ����������)
			if (!CheckValidUrl(crFrom, crTo, bUrlMode, pszUrlDelim, pszUrl, pszProtocol, pChar, nLen))
				goto wrap;


			// ����� ��������� ����� ������������ (���� ������� ���������� � �.�.)
			crTo.X = crFrom.X;

			// ������ - ����� �����.
			// �������, ��� ��� ������ ����� ��� ���������, ����� �������� ���� �������� ������
			// ��� ���������� (http/...) - ������ ������������ ������

			TODO("����� �� � ������ �������� ������ ����������, ��� ���������� '������ � �������'");

			// -- VC
			// 1>t:\vcproject\conemu\realconsole.cpp(8104) : error C2065: 'qqq' : undeclared identifier
			// DefResolve.cpp(18) : error C2065: 'sdgagasdhsahd' : undeclared identifier
			// DefResolve.cpp(18): warning: note xxx
			// -- GCC
			// ConEmuC.cpp:49: error: 'qqq' does not name a type
			// 1.c:3: some message
			// file.cpp:29:29: error
			// Delphi
			// T:\VCProject\FarPlugin\$FarPlugins\MaxRusov\far-plugins-read-only\FarLib\FarCtrl.pas(1002) Error: Undeclared identifier: 'PCTL_GETPLUGININFO'
			// FPC
			// FarCtrl.pas(1002,49) Error: Identifier not found "PCTL_GETPLUGININFO"
			// -- Possible?
			// abc.py (3): some message
			// ASM - ������������ ����� "test.asasm(1,1)"
			// object.Exception@assembler.d(1239): test.asasm(1,1):
			// -- URL's
			// file://c:\temp\qqq.html
			// http://www.farmanager.com
			// $ http://www.KKK.ru - ����� ����� - �� �����������
			// C:\ConEmu>http://www.KKK.ru - ...
			// -- False detects
			// 29.11.2011 18:31:47
			// C:\VC\unicode_far\macro.cpp  1251 Ln 5951/8291 Col 51 Ch 39 0043h 13:54
			// InfoW1900->SettingsControl(sc.Handle, SCTL_FREE, 0, 0);

			bool bDigits = false, bLineNumberFound = false, bWasSeparator = false;
			// ��� �� ���������� ������ ���� "11.05.2010 10:20:35"
			// � ����� ����� ������ ���� ���� �� ���� ����� (����������), ������ ����������
			int iExtFound = 0, iBracket = 0;
			// �������
			if (bUrlMode)
			{
				while (((crTo.X+1) < nLen) && !wcschr(pszUrlDelim, pChar[crTo.X+1]))
					crTo.X++;
			}
			else while ((crTo.X+1) < nLen)
				//&& ((pChar[crTo.X] != L':') || (pChar[crTo.X] == L':' && wcschr(pszDigits, pChar[crTo.X+1]))))
			{
				if ((pChar[crTo.X] == L'/') && ((crTo.X+1) < nLen) && (pChar[crTo.X+1] == L'/')
					&& !((crTo.X > 1) && (pChar[crTo.X] == L':'))) // � �� URL �����
				{
					goto wrap; // �� ��� (����������� � ������)
				}

				if (bWasSeparator 
					&& ((pChar[crTo.X] >= L'0' && pChar[crTo.X] <= L'9')
						|| (bDigits && (pChar[crTo.X] == L',')))) // FarCtrl.pas(1002,49) Error: 
				{
					if (bLineNumberFound)
					{
						// gcc ����� ������ ���� ����� ��������
						// file.cpp:29:29: error
						crTo.X--;
						break;
					}
					if (!bDigits && (crFrom.X < crTo.X) /*&& (pChar[crTo.X-1] == L':')*/)
					{
						bDigits = true;
					}
				}
				else
				{
					if (iExtFound != 2)
					{
						if (!iExtFound)
						{
							if (pChar[crTo.X] == L'.')
								iExtFound = 1;
						}
						else
						{
							// �� ����� ������������� � ������� � ������. ������ ��������� ��������� �� ������ ������������...
							if ((pChar[crTo.X] >= L'a' && pChar[crTo.X] <= L'z') || (pChar[crTo.X] >= L'A' && pChar[crTo.X] <= L'Z'))
							{
								iExtFound = 2;
								iBracket = 0;
							}
						}
					}

					if (iExtFound == 2)
					{
						if (pChar[crTo.X] == L'.')
						{
							iExtFound = 1;
							iBracket = 0;
						}
						else if (wcschr(pszSlashes, pChar[crTo.X]) != NULL)
						{
							// ��� ����, ������ ���������� - ��� ���
							iExtFound = 0;
							iBracket = 0;
							bWasSeparator = false;
						}
						else if (wcschr(pszSpacing, pChar[crTo.X]) && wcschr(pszSpacing, pChar[crTo.X+1]))
						{
							// ������� ����� ��������
							iExtFound = 0;
							iBracket = 0;
							bWasSeparator = false;
						}
						else
							bWasSeparator = (wcschr(pszSeparat, pChar[crTo.X]) != NULL);
					}

					// ��������� �� ����������� : ��� ) ��� ,
					_ASSERTE(pszTermint[0]==L':' && pszTermint[1]==L')' && pszTermint[2]==L',' && pszTermint[3]==0);
					if (bDigits && wcschr(pszTermint, pChar[crTo.X]) /*pChar[crTo.X] == L':'*/)
					{
						// ���� ����� ������ �������� �������� - ������ ������ ���� ��������������
						if (((pChar[crTo.X] == L':')
								&& (wcschr(pszSpacing, pChar[crTo.X+1])
									|| wcschr(pszDigits, pChar[crTo.X+1])))
						|| ((pChar[crTo.X] == L')') && (iBracket == 1)
								&& ((pChar[crTo.X+1] == L':')
									|| wcschr(pszSpacing, pChar[crTo.X+1])
									|| wcschr(pszDigits, pChar[crTo.X+1])))
							)
						{
							_ASSERTE(bLineNumberFound==false);
							bLineNumberFound = true;
							break; // found?
						}
					}
					bDigits = false;

					switch (pChar[crTo.X])
					{
					case L'(': iBracket++; break;
					case L')': iBracket--; break;
					case L'/': case L'\\': iBracket = 0; break;
					case L'@':
						if (MailX != -1)
						{
							bMaybeMail = false;
						}
						else if (((crTo.X > 0) && wcschr(pszEMail, pChar[crTo.X-1]))
							&& (((crTo.X+1) < nLen) && wcschr(pszEMail, pChar[crTo.X+1])))
						{
							bMaybeMail = true;
							MailX = crTo.X;
						}
						break;
					}

					if (pChar[crTo.X] == L':')
						nColons++;
					else if (pChar[crTo.X] == L'\\' || pChar[crTo.X] == L'/')
						nColons = 0;
				}

				if (nColons >= 2)
					break;

				crTo.X++;
				if (wcschr(bUrlMode ? pszUrlDelim : pszBreak, pChar[crTo.X]))
				{
					if (bMaybeMail)
						break;
					goto wrap; // �� ���
				}
			}

			if (bUrlMode)
			{
				// �������, ��� OK
				bMaybeMail = false;
			}
			else
			{
				if (!bLineNumberFound && bDigits)
					bLineNumberFound = true;

				if (bLineNumberFound)
					bMaybeMail = false;

				if ((pChar[crTo.X] != L':' && pChar[crTo.X] != L' ' && !(pChar[crTo.X] == L')' && iBracket == 1)) || !bLineNumberFound || (nColons > 2))
				{
					if (!bMaybeMail)
						goto wrap;
				}
				if (bMaybeMail || (!bMaybeMail && pChar[crTo.X] != L')'))
					crTo.X--;
				// �������� �������� �������
				while ((crFrom.X < crTo.X) && wcschr(pszSpacing, pChar[crFrom.X]))
					crFrom.X++;
				while ((crTo.X > crFrom.X) && wcschr(pszSpacing, pChar[crTo.X]))
					crTo.X--;
				if ((crFrom.X + 4) > crTo.X) // 1.c:1: //-V112
				{
					// ������� �������, ������� ��� �� ���
					goto wrap;
				}
				if (!bMaybeMail)
				{
					// ���������, ����� ��� � ������� ����� ������
					if (!(pChar[crTo.X] >= L'0' && pChar[crTo.X] <= L'9') // ConEmuC.cpp:49:
						&& !(pChar[crTo.X] == L')' && (pChar[crTo.X-1] >= L'0' && pChar[crTo.X-1] <= L'9'))) // ConEmuC.cpp(49) :
					{
						goto wrap; // ������ ������ ���
					}
					// ����� ���� �������� �� ������������:
					// 29.11.2011 18:31:47
					{
						bool bNoDigits = false;
						for (int i = crFrom.X; i <= crTo.X; i++)
						{
							if (pChar[i] < L'0' || pChar[i] > L'9')
							{
								bNoDigits = true;
							}
						}
						if (!bNoDigits)
							goto wrap;
					}
					// -- ��� �������� // ��� ���������� � VC �������� ������
					//if ((pChar[crTo.X] == L')') && (pChar[crTo.X+1] == L':'))
					//	crTo.X++;
				}
				else // bMaybeMail
				{
					// ��� ������ - ��������� ���������� ������� (����� �������� �� ���� � ������� ������)
					int x = MailX - 1; _ASSERTE(x>=0);
					while ((x > 0) && wcschr(pszEMail, pChar[x-1]))
						x--;
					crFrom.X = x;
					x = MailX + 1; _ASSERTE(x<nLen);
					while (((x+1) < nLen) && wcschr(pszEMail, pChar[x+1]))
						x++;
					crTo.X = x;
				}
			} // end "else / if (bUrlMode)"

			// Check mouse pos, it must be inside region
			if ((crMouse.X < crFrom.X) || (crMouse.X > crTo.X))
			{
				goto wrap;
			}

			// Ok
			if (pszText && cchTextMax)
			{
				_ASSERTE(!bMaybeMail || !bUrlMode); // ������������ - ����� �� ����� ���� ����������!
				int iMailTo = (bMaybeMail && !bUrlMode) ? lstrlen(L"mailto:") : 0;
				if ((crTo.X - crFrom.X + 1 + iMailTo) >= (INT_PTR)cchTextMax)
					goto wrap; // ������������ ����� ��� �����
				if (iMailTo)
				{
					// �������� ������� ���������
					lstrcpyn(pszText, L"mailto:", iMailTo+1);
					pszText += iMailTo;
					cchTextMax -= iMailTo;
					bUrlMode = true;
				}
				memmove(pszText, pChar+crFrom.X, (crTo.X - crFrom.X + 1)*sizeof(*pszText));
				pszText[crTo.X - crFrom.X + 1] = 0;

				#ifdef _DEBUG
				if (!bUrlMode && wcsstr(pszText, L"//")!=NULL)
				{
					_ASSERTE(FALSE);
				}
				#endif
			}
			result = bUrlMode ? etr_Url : etr;
		}
	}
wrap:
	// Fail?
	if (result == etr_None)
	{
		crFrom = crTo = MakeCoord(0,0);
	}
	StoreLastTextRange(result);
	return result;
}

void CRealBuffer::StoreLastTextRange(ExpandTextRangeType etr)
{
	if (con.etrLast != etr)
	{
		con.etrLast = etr;
		//if (etr == etr_None)
		//{
		//	con.mcr_FileLineStart = con.mcr_FileLineEnd = MakeCoord(0,0);
		//}

		if ((mp_RCon->mp_ABuf == this) && mp_RCon->isVisible())
			gpConEmu->OnSetCursor();
	}
}

BOOL CRealBuffer::GetPanelRect(BOOL abRight, RECT* prc, BOOL abFull /*= FALSE*/, BOOL abIncludeEdges /*= FALSE*/)
{
	if (!this)
	{
		if (prc)
			*prc = MakeRect(-1,-1);

		return FALSE;
	}

	if (abRight)
	{
		if (prc)
			*prc = abFull ? mr_RightPanelFull : mr_RightPanel;

		if (mr_RightPanel.right <= mr_RightPanel.left)
			return FALSE;
	}
	else
	{
		if (prc)
			*prc = abFull ? mr_LeftPanelFull : mr_LeftPanel;

		if (mr_LeftPanel.right <= mr_LeftPanel.left)
			return FALSE;
	}
	
	if (prc && !abFull && abIncludeEdges)
	{
		prc->left  = abRight ? mr_RightPanelFull.left  : mr_LeftPanelFull.left;
		prc->right = abRight ? mr_RightPanelFull.right : mr_LeftPanelFull.right;
	}

	return TRUE;
}

BOOL CRealBuffer::isLeftPanel()
{
	return mb_LeftPanel;
}

BOOL CRealBuffer::isRightPanel()
{
	return mb_RightPanel;
}

short CRealBuffer::CheckProgressInConsole(const wchar_t* pszCurLine)
{
	// ��������� ��������� NeroCMD � ��. ���������� �������� (���� ������ ��������� � ������� �������)
	//������ Update
	//"Downloading Far                                               99%"
	//NeroCMD
	//"012% ########.................................................................."
	//ChkDsk
	//"Completed: 25%"
	//Rar
	// ...       Vista x86\Vista x86.7z         6%
	int nIdx = 0;
	bool bAllowDot = false;
	short nProgress = -1;

	const wchar_t *szPercentEng = L" percent";
	const wchar_t *szComplEng  = L"Completed:";
	static wchar_t szPercentRus[16] = {}, szComplRus[16] = {};
	static int nPercentEngLen = lstrlen(szPercentEng), nComplEngLen = lstrlen(szComplEng);
	static int nPercentRusLen, nComplRusLen;
	
	if (szPercentRus[0] == 0)
	{
		szPercentRus[0] = L' ';
		TODO("������ �� � ������ ������������ �������� ������������, ����� �� ���������");
		MultiByteToWideChar(CP_ACP,0,"�������",-1,szPercentRus+1,countof(szPercentRus)-1);
		MultiByteToWideChar(CP_ACP,0,"���������:",-1,szComplRus,countof(szComplRus));
		
		nPercentRusLen = lstrlen(szPercentRus);
		nComplRusLen = lstrlen(szComplEng);
	}

	// ������� ��������, ����� ����� ���� � ������ ������ (���������� �������)?
	if (pszCurLine[nIdx] == L' ' && isDigit(pszCurLine[nIdx+1]))
		nIdx++; // ���� ���������� ������ ����� ������
	else if (pszCurLine[nIdx] == L' ' && pszCurLine[nIdx+1] == L' ' && isDigit(pszCurLine[nIdx+2]))
		nIdx += 2; // ��� ���������� ������� ����� ������
	else if (!isDigit(pszCurLine[nIdx]))
	{
		// ������ ���������� �� � �����. ����� ���������� ����� �� ��������� ��������� (ChkDsk)?

		if (!wcsncmp(pszCurLine, szComplRus, nComplRusLen))
		{
			nIdx += nComplRusLen;

			if (pszCurLine[nIdx] == L' ') nIdx++;

			if (pszCurLine[nIdx] == L' ') nIdx++;

			bAllowDot = true;
		}
		else if (!wcsncmp(pszCurLine, szComplEng, nComplEngLen))
		{
			nIdx += nComplEngLen;

			if (pszCurLine[nIdx] == L' ') nIdx++;

			if (pszCurLine[nIdx] == L' ') nIdx++;

			bAllowDot = true;
		}

		// ��������� ��������� �� �������, ���������, ����� ������� ���� � ����� ������?
		if (!nIdx)
		{
			//TODO("�� �������� � ����� ������");
			// Creating archive T:\From_Work\VMWare\VMWare.part006.rar
			// ...       Vista x86\Vista x86.7z         6%
			int i = GetTextWidth() - 1;

			// �������� trailing spaces
			while(i>3 && pszCurLine[i] == L' ')
				i--;

			// ������, ���� ����� �� '%' � ����� ��� - �����
			if (i >= 3 && pszCurLine[i] == L'%' && isDigit(pszCurLine[i-1]))
			{
				//i -= 2;
				i--;

				int j = i, k = -1;
				while (j > 0 && isDigit(pszCurLine[j-1]))
					j--;

				// ����� ���� ���-�� ���� "Progress 25.15%"
				if (((i - j) <= 2) && (j >= 2) && ((i - j) <= 2) && (pszCurLine[j-1] == L'.'))
				{
					k = j - 1;
					while (k > 0 && isDigit(pszCurLine[k-1]))
						k--;
				}

				if (k >= 0)
				{
					if (((j - k) <= 3) // 2 ����� + �����
						|| (((j - k) <= 4) && (pszCurLine[k] == L'1'))) // "100.0%"
					{
						nIdx = i = k;
						bAllowDot = true;
					}
				}
				else
				{
					if (((j - i) <= 2) // 2 ����� + �����
						|| (((j - i) <= 3) && (pszCurLine[j] == L'1'))) // "100%"
					{
						nIdx = i = j;
					}
				}

				#if 0
				// ��� ����� ����� '%'?
				if (isDigit(pszCurLine[i-1]))
					i--;

				// ��� ����� ����������� ������ ��� '100%'
				if (pszCurLine[i-1] == L'1' && !isDigit(pszCurLine[i-2]))
				{
					nIdx = i - 1;
				}
				// final check
				else if (!isDigit(pszCurLine[i-1]))
				{
					nIdx = i;
				}
				#endif

				// ����� �������� ������������� ��������, ���� ��� ������ � prompt
				// ��������, ��� ���� � ������ ���� ������ '>' - �� ��� �� ��������
				while (i>=0)
				{
					if (pszCurLine[i] == L'>')
					{
						nIdx = 0;
						break;
					}

					i--;
				}
			}
		}
	}

	// ������ nProgress ������ ���� ����� �������� � ������ � ��������
	if (isDigit(pszCurLine[nIdx]))
	{
		if (isDigit(pszCurLine[nIdx+1]) && isDigit(pszCurLine[nIdx+2])
			&& (pszCurLine[nIdx+3]==L'%' || (bAllowDot && pszCurLine[nIdx+3]==L'.')
			|| !wcsncmp(pszCurLine+nIdx+3,szPercentEng,nPercentEngLen)
			|| !wcsncmp(pszCurLine+nIdx+3,szPercentRus,nPercentRusLen)))
		{
			nProgress = 100*(pszCurLine[nIdx] - L'0') + 10*(pszCurLine[nIdx+1] - L'0') + (pszCurLine[nIdx+2] - L'0');
		}
		else if (isDigit(pszCurLine[nIdx+1])
			&& (pszCurLine[nIdx+2]==L'%' || (bAllowDot && pszCurLine[nIdx+2]==L'.')
			|| !wcsncmp(pszCurLine+nIdx+2,szPercentEng,nPercentEngLen)
			|| !wcsncmp(pszCurLine+nIdx+2,szPercentRus,nPercentRusLen)))
		{
			nProgress = 10*(pszCurLine[nIdx] - L'0') + (pszCurLine[nIdx+1] - L'0');
		}
		else if (pszCurLine[nIdx+1]==L'%' || (bAllowDot && pszCurLine[nIdx+1]==L'.')
			|| !wcsncmp(pszCurLine+nIdx+1,szPercentEng,nPercentEngLen)
			|| !wcsncmp(pszCurLine+nIdx+1,szPercentRus,nPercentRusLen))
		{
			nProgress = (pszCurLine[nIdx] - L'0');
		}
	}

	if (nProgress != -1)
	{
		mp_RCon->mn_LastConProgrTick = GetTickCount();
		mp_RCon->mn_LastConsoleProgress = nProgress; // ��� ��������� ������
	}
	else
	{
		DWORD nDelta = GetTickCount() - mp_RCon->mn_LastConProgrTick;
		if (nDelta < CONSOLEPROGRESSTIMEOUT) // ���� ������� ����������� �������� ��� �� ��������
			nProgress = mp_RCon->mn_ConsoleProgress; // ������� ���������� ��������
		mp_RCon->mn_LastConsoleProgress = -1; // ��� ��������� ������
	}

	return nProgress;
}

LRESULT CRealBuffer::OnScroll(int nDirection, short nTrackPos /*= -1*/, UINT nCount /*= 1*/)
{
	if (!this) return 0;

	int nVisible = GetTextHeight();

	if (!nCount)
	{
		_ASSERTE(nCount >= 1);
		nCount = 1;
	}

	// SB_LINEDOWN / SB_LINEUP / SB_PAGEDOWN / SB_PAGEUP
	if (m_Type == rbt_Primary)
	{
		if ((nDirection != SB_THUMBTRACK) && (nDirection != SB_THUMBPOSITION))
			nTrackPos = 0;
		if (nTrackPos < 0)
			nTrackPos = con.m_sbi.srWindow.Top;

		WPARAM wParm = MAKELONG(nDirection,nTrackPos);
		while (true)
		{
			WARNING("���������� � ������� �����");
			mp_RCon->PostConsoleMessage(mp_RCon->hConWnd, WM_VSCROLL, wParm, NULL);

			if ((nCount <= 1) || (nDirection != SB_LINEUP && nDirection != SB_LINEDOWN) /*|| mp_RCon->isFar()*/)
				break;
			nCount--;
		}

		if ((nDirection == SB_THUMBTRACK) || (nDirection == SB_THUMBPOSITION))
		{
			_ASSERTE(nTrackPos>=0);
			int nVisible = GetTextHeight();

			if (nTrackPos < 0)
				nTrackPos = 0;
			else if ((nTrackPos + nVisible) >= con.m_sbi.dwSize.Y)
				nTrackPos = con.m_sbi.dwSize.Y - nVisible;

			// ������� Top, ����� ������ �������������� � ������������ �����
			con.m_sbi.srWindow.Top = nTrackPos;
			con.m_sbi.srWindow.Bottom = nTrackPos + nVisible - 1;
			con.nTopVisibleLine = nTrackPos;

			////mp_RCon->mp_VCon->Invalidate();
			//mp_RCon->mb_DataChanged = TRUE; // ���������� ������������ ������ ������
			//con.bConsoleDataChanged = TRUE; // � ��� - ��� ������� �� CVirtualConsole
		}
	}
	else
	{
		SHORT nNewTop = con.m_sbi.srWindow.Top;

		switch (nDirection)
		{
		case SB_LINEDOWN:
			if ((nNewTop + nVisible) < con.m_sbi.dwSize.Y)
				nNewTop = min((nNewTop+(SHORT)nCount),(con.m_sbi.dwSize.Y-nVisible));
			break;
		case SB_LINEUP:
			if (nNewTop > 0)
				nNewTop = max(0,(nNewTop-(SHORT)nCount));
			break;
		case SB_PAGEDOWN:
			nNewTop = min((con.m_sbi.dwSize.Y - nVisible),(con.m_sbi.srWindow.Top + nVisible - 1));
			break;
		case SB_PAGEUP:
			nNewTop = max(0, (con.m_sbi.srWindow.Top - nVisible + 1));
			break;
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			{
				_ASSERTE(nTrackPos>=0);

				if (nTrackPos < 0)
					nTrackPos = 0;
				else if ((nTrackPos + nVisible) >= con.m_sbi.dwSize.Y)
					nTrackPos = con.m_sbi.dwSize.Y - nVisible;

				nNewTop = nTrackPos;
			}
			break;
		case SB_ENDSCROLL:
			break;
		default:
			// ������������ ���
			_ASSERTE(nDirection==SB_LINEUP);
		}


		if (nNewTop != con.m_sbi.srWindow.Top)
		{
			con.m_sbi.srWindow.Top = nNewTop;
			con.m_sbi.srWindow.Bottom = nNewTop + nVisible - 1;

			con.nTopVisibleLine = con.m_sbi.srWindow.Top;

			//mp_RCon->mp_VCon->Invalidate();
			mp_RCon->mb_DataChanged = TRUE; // ���������� ������������ ������ ������
			con.bConsoleDataChanged = TRUE; // � ��� - ��� ������� �� CVirtualConsole

			//if (mp_RCon->isActive()) -- mp_RCon->isActive() �������� ���� UpdateScrollInfo, � ��������� ����� ���� � � ������� �� �� �������� �������
			//mp_RCon->UpdateScrollInfo();
		}
	}
	return 0;
}

LRESULT CRealBuffer::OnSetScrollPos(WPARAM wParam)
{
	if (!this) return 0;

	// SB_LINEDOWN / SB_LINEUP / SB_PAGEDOWN / SB_PAGEUP
	OnScroll(LOWORD(wParam),HIWORD(wParam));
	return 0;
}

const CRgnDetect* CRealBuffer::GetDetector()
{
	if (this)
		return &m_Rgn;
	return NULL;
}
