
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

#include <windows.h>
#include <wchar.h>
#include "..\common\common.hpp"
#include "..\common\RgnDetect.h"
#pragma warning( disable : 4995 )
#include "..\common\pluginW1007.hpp"
#pragma warning( default : 4995 )
#include "PluginHeader.h"

// ����� �� �������� ��������� Up/Down ��� �������� ����� ��������
//  ����� "����" Esc, ������� RestoreScreen (�� ��� ������� �� �����)
//  � ������� ������ "Down Enter" - ��� ������� ���������
//
// ��������� '*' ��� ����������� ����� ����������� �����
//  = ���������� (������������ ������� �����, ��� ��� � �������)
//  = ������������ (������������ ������ ��������, �� ��� �����, � �� ��� ����)

extern struct PluginStartupInfo *InfoW995;
extern struct FarStandardFunctions *FSFW995;

CONSOLE_SCREEN_BUFFER_INFO csbi;
int gnPage = 0;
bool gbShowAttrsOnly = false;

BOOL CheckConInput(INPUT_RECORD* pr)
{
	DWORD dwCount = 0;
	memset(pr, 0, sizeof(INPUT_RECORD));
	BOOL lbRc = ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), pr, 1, &dwCount);
	if (!lbRc)
		return FALSE;

	if (pr->EventType == KEY_EVENT) {
		if (pr->Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE)
			return FALSE;
	}

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	return TRUE;
}

void ShowConPacket(CESERVER_REQ* pReq)
{
	// ���-�� ��� � ������������� ��������
	INPUT_RECORD *r = (INPUT_RECORD*)calloc(sizeof(INPUT_RECORD),2);
	HANDLE hO = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD cr;
	DWORD dw, dwLen;
	int nPage = 0;
	BOOL lbNeedRedraw = TRUE;
	wchar_t* pszText = (wchar_t*)calloc(200*100,2);
	wchar_t* psz = NULL, *pszEnd = NULL;
	CESERVER_CHAR *pceChar = NULL;
	wchar_t* pszConData = NULL;
	WORD* pnConData = NULL;
	DWORD dwConDataBufSize = 0;
	CONSOLE_SCREEN_BUFFER_INFO sbi = {{0,0}};

	SetWindowText(FarHwnd, L"ConEmu packet dump");

	psz = pszText;
	switch (pReq->hdr.nCmd) {
		//case CECMD_GETSHORTINFO: pszEnd = L"CECMD_GETSHORTINFO"; break;
		//case CECMD_GETFULLINFO: pszEnd = L"CECMD_GETFULLINFO"; break;
		case CECMD_SETSIZESYNC: pszEnd = L"CECMD_SETSIZESYNC"; break;
		case CECMD_CMDSTARTSTOP: pszEnd = L"CECMD_CMDSTARTSTOP"; break;
		case CECMD_GETGUIHWND: pszEnd = L"CECMD_GETGUIHWND"; break;
//		case CECMD_RECREATE: pszEnd = L"CECMD_RECREATE"; break;
		case CECMD_TABSCHANGED: pszEnd = L"CECMD_TABSCHANGED"; break;
		case CECMD_CMDSTARTED: pszEnd = L"CECMD_CMDSTARTED"; break;
		case CECMD_CMDFINISHED: pszEnd = L"CECMD_CMDFINISHED"; break;
		default: pszEnd = L"???";
	}
	wsprintf(psz, L"Packet size: %i;  Command: %i (%s);  Version: %i\n",
		pReq->hdr.cbSize, pReq->hdr.nCmd, pszEnd, pReq->hdr.nVersion);
	psz += lstrlenW(psz);

	r->EventType = WINDOW_BUFFER_SIZE_EVENT;

	do {
		if (r->EventType == WINDOW_BUFFER_SIZE_EVENT) {
			r->EventType = 0; cr.X = 0; cr.Y = 0;
			lbNeedRedraw = TRUE;
		} else if (r->EventType == KEY_EVENT && r->Event.KeyEvent.bKeyDown) {
			if (r->Event.KeyEvent.wVirtualKeyCode == VK_NEXT) {
				lbNeedRedraw = TRUE;
				gnPage++;
				FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
			} else if (r->Event.KeyEvent.wVirtualKeyCode == VK_PRIOR) {
				lbNeedRedraw = TRUE;
				gnPage--;
			}
		}
		if (gnPage<0) gnPage = 1; else if (gnPage>1) gnPage = 0;

		if (lbNeedRedraw) {
			lbNeedRedraw = FALSE;
			cr.X = 0; cr.Y = 0;
			if (gnPage == 0) {
				FillConsoleOutputAttribute(hO, 7, csbi.dwSize.X*csbi.dwSize.Y, cr, &dw);
				FillConsoleOutputCharacter(hO, L' ', csbi.dwSize.X*csbi.dwSize.Y, cr, &dw);
				cr.X = 0; cr.Y = 0; psz = pszText;
				while (*psz && cr.Y<csbi.dwSize.Y) {
					pszEnd = wcschr(psz, L'\n');
					dwLen = min(((int)(pszEnd-psz)),csbi.dwSize.X);
					SetConsoleCursorPosition(hO, cr);
					if (dwLen>0)
						WriteConsoleOutputCharacter(hO, psz, dwLen, cr, &dw);
					cr.Y ++; psz = pszEnd + 1;
				}
				SetConsoleCursorPosition(hO, cr);
			} else if (gnPage == 1) {
				FillConsoleOutputAttribute(hO, 0x10, csbi.dwSize.X*csbi.dwSize.Y, cr, &dw);
				FillConsoleOutputCharacter(hO, L' ', csbi.dwSize.X*csbi.dwSize.Y, cr, &dw);

				int nMaxX = min(sbi.dwSize.X, csbi.dwSize.X);
				int nMaxY = min(sbi.dwSize.Y, csbi.dwSize.Y);
				if (pszConData && dwConDataBufSize) {
					wchar_t* pszSrc = pszConData;
					wchar_t* pszEnd = pszConData + dwConDataBufSize;
					WORD* pnSrc = pnConData;
					cr.X = 0; cr.Y = 0;
					while (cr.Y < nMaxY && pszSrc < pszEnd) {
						WriteConsoleOutputCharacter(hO, pszSrc, nMaxX, cr, &dw);
						WriteConsoleOutputAttribute(hO, pnSrc, nMaxX, cr, &dw);

						pszSrc += sbi.dwSize.X; pnSrc += sbi.dwSize.X; cr.Y++;
					}
				}
				cr.Y = nMaxY-1;
				SetConsoleCursorPosition(hO, cr);
			}
		}

	} while (CheckConInput(r));

	free(pszText);
}

void ShowConDump(wchar_t* pszText)
{
	INPUT_RECORD r[2];
	BOOL lbNeedRedraw = TRUE;
	HANDLE hO = GetStdHandle(STD_OUTPUT_HANDLE);
	//CONSOLE_SCREEN_BUFFER_INFO sbi = {{0,0}};
	COORD cr, crSize, crCursor;
	WCHAR* pszBuffers[3];
	void*  pnBuffers[3];
	WCHAR* pszDumpTitle, *pszRN, *pszSize, *pszTitle = NULL;

	SetWindowText(FarHwnd, L"ConEmu screen dump");

	pszDumpTitle = pszText;
	pszRN = wcschr(pszDumpTitle, L'\r');
	if (!pszRN) return;
	*pszRN = 0;
	pszSize = pszRN + 2;
	if (wcsncmp(pszSize, L"Size: ", 6)) return;

	pszRN = wcschr(pszSize, L'\r');
	if (!pszRN) return;
	pszBuffers[0] = pszRN + 2;

	pszSize += 6;
	//if ((pszRN = wcschr(pszSize, L'x'))==NULL) return;
	//*pszRN = 0;
	crSize.X = (SHORT)wcstol(pszSize, &pszRN, 10);
	if (!pszRN || *pszRN!=L'x') return;
	pszSize = pszRN + 1;
	//if ((pszRN = wcschr(pszSize, L'\r'))==NULL) return;
	//*pszRN = 0;
	crSize.Y = (SHORT)wcstol(pszSize, &pszRN, 10);
	if (!pszRN || (*pszRN!=L' ' && *pszRN!=L'\r')) return;
	pszSize = pszRN;
	crCursor.X = 0; crCursor.Y = crSize.Y-1;
	if (*pszSize == L' ')
	{
		while (*pszSize == L' ') pszSize++;
		if (wcsncmp(pszSize, L"Cursor: ", 8)==0)
		{
			pszSize += 8;
			cr.X = (SHORT)wcstol(pszSize, &pszRN, 10);
			if (!pszRN || *pszRN!=L'x') return;
			pszSize = pszRN + 1;
			cr.Y = (SHORT)wcstol(pszSize, &pszRN, 10);
			if (cr.X>=0 && cr.Y>=0)
			{
				crCursor.X = cr.X; crCursor.Y = cr.Y;
			}
		}
	}

	pszTitle = (WCHAR*)calloc(lstrlenW(pszDumpTitle)+200,2);

	DWORD dwConDataBufSize = crSize.X * crSize.Y;
	DWORD dwConDataBufSizeEx = crSize.X * crSize.Y * sizeof(CharAttr);
	pnBuffers[0] = (void*)(((WORD*)(pszBuffers[0])) + dwConDataBufSize);
	pszBuffers[1] = (WCHAR*)(((LPBYTE)(pnBuffers[0])) + dwConDataBufSizeEx);
	pnBuffers[1] = (void*)(((WORD*)(pszBuffers[1])) + dwConDataBufSize);
	pszBuffers[2] = (WCHAR*)(((LPBYTE)(pnBuffers[1])) + dwConDataBufSizeEx);
	pnBuffers[2] = (void*)(((WORD*)(pszBuffers[2])) + dwConDataBufSize);

	r->EventType = WINDOW_BUFFER_SIZE_EVENT;

	do {
		if (r->EventType == WINDOW_BUFFER_SIZE_EVENT) {
			r->EventType = 0;
			lbNeedRedraw = TRUE;
		} else if (r->EventType == KEY_EVENT && r->Event.KeyEvent.bKeyDown) {
			if (r->Event.KeyEvent.wVirtualKeyCode == VK_NEXT) {
				lbNeedRedraw = TRUE;
				gnPage++;
				FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
			} else if (r->Event.KeyEvent.wVirtualKeyCode == VK_PRIOR) {
				lbNeedRedraw = TRUE;
				gnPage--;
			} else if (r->Event.KeyEvent.uChar.UnicodeChar == L'*') {
				lbNeedRedraw = TRUE;
				gbShowAttrsOnly = !gbShowAttrsOnly;
			}
		}
		if (gnPage<0) gnPage = 3; else if (gnPage>3) gnPage = 0;

		if (lbNeedRedraw)
		{
			lbNeedRedraw = FALSE;
			cr.X = 0; cr.Y = 0;
			DWORD dw = 0;
			if (gnPage == 0)
			{
				FillConsoleOutputAttribute(hO, 7, csbi.dwSize.X*csbi.dwSize.Y, cr, &dw);
				FillConsoleOutputCharacter(hO, L' ', csbi.dwSize.X*csbi.dwSize.Y, cr, &dw);
				cr.X = 0; cr.Y = 0; SetConsoleCursorPosition(hO, cr);
				//wprintf(L"Console screen dump viewer\nTitle: %s\nSize: {%i x %i}\n",
				//	pszDumpTitle, crSize.X, crSize.Y);
				LPCWSTR psz = L"Console screen dump viewer";
				WriteConsoleOutputCharacter(hO, psz, lstrlenW(psz), cr, &dw); cr.Y++;
				psz = L"Title: ";
				WriteConsoleOutputCharacter(hO, psz, lstrlenW(psz), cr, &dw); cr.X += lstrlenW(psz);
				WriteConsoleOutputCharacter(hO, pszDumpTitle, lstrlenW(pszDumpTitle), cr, &dw); cr.X = 0; cr.Y++;
				wchar_t szSize[64]; wsprintf(szSize, L"Size: {%i x %i}", crSize.X, crSize.Y);
				WriteConsoleOutputCharacter(hO, szSize, lstrlenW(szSize), cr, &dw); cr.Y++;
				SetConsoleCursorPosition(hO, cr);
				
			}
			else if (gnPage >= 1 && gnPage <= 3)
			{
				FillConsoleOutputAttribute(hO, gbShowAttrsOnly ? 0xF : 0x10, csbi.dwSize.X*csbi.dwSize.Y, cr, &dw);
				FillConsoleOutputCharacter(hO, L' ', csbi.dwSize.X*csbi.dwSize.Y, cr, &dw);

				int nMaxX = min(crSize.X, csbi.dwSize.X);
				int nMaxY = min(crSize.Y, csbi.dwSize.Y);
				wchar_t* pszConData = pszBuffers[gnPage-1];
				void* pnConData = pnBuffers[gnPage-1];
				LPBYTE pnTemp = (LPBYTE)malloc(nMaxX*2);
				CharAttr *pSrcEx = (CharAttr*)pnConData;
				if (pszConData && dwConDataBufSize)
				{
					wchar_t* pszSrc = pszConData;
					wchar_t* pszEnd = pszConData + dwConDataBufSize;
					LPBYTE pnSrc;
					DWORD nAttrLineSize;
					if (gnPage == 3)
					{
						pnSrc = (LPBYTE)pnConData;
						nAttrLineSize = crSize.X * 2;
					}
					else
					{
						pnSrc = pnTemp;
						nAttrLineSize = 0;
					}
					cr.X = 0; cr.Y = 0;
					while (cr.Y < nMaxY && pszSrc < pszEnd)
					{
						if (!gbShowAttrsOnly)
						{
							WriteConsoleOutputCharacter(hO, pszSrc, nMaxX, cr, &dw);
							if (gnPage < 3)
							{
								for (int i = 0; i < nMaxX; i++)
								{
									((WORD*)pnSrc)[i] = pSrcEx[i].nForeIdx | (pSrcEx[i].nBackIdx << 4);
								}
								pSrcEx += crSize.X;
							}
							WriteConsoleOutputAttribute(hO, (WORD*)pnSrc, nMaxX, cr, &dw);
						}
						else
						{
							WriteConsoleOutputCharacter(hO, (wchar_t*)pnSrc, nMaxX, cr, &dw);
						}

						pszSrc += crSize.X;
						if (nAttrLineSize)
							pnSrc += nAttrLineSize;
						cr.Y++;
					}
				}
				free(pnTemp);
				//cr.Y = nMaxY-1;
				SetConsoleCursorPosition(hO, crCursor);
			}
		}

	} while (CheckConInput(r));

	free(pszTitle);
}


HANDLE WINAPI OpenFilePluginW(const wchar_t *Name,const unsigned char *Data,int DataSize,int OpMode)
{
	//Name==NULL, ����� Shift-F1
	if (!InfoW995) return INVALID_HANDLE_VALUE;
	if (OpMode || Name == NULL) return INVALID_HANDLE_VALUE; // ������ �� ������� � ������� ������
	const wchar_t* pszDot = wcsrchr(Name, L'.');
	if (!pszDot) return INVALID_HANDLE_VALUE;
	if (lstrcmpi(pszDot, L".con")) return INVALID_HANDLE_VALUE;
	if (DataSize < sizeof(CESERVER_REQ_HDR)) return INVALID_HANDLE_VALUE;

	HANDLE hFile = CreateFile(Name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (hFile == INVALID_HANDLE_VALUE) {
		InfoW995->Message(InfoW995->ModuleNumber, FMSG_ALLINONE|FMSG_MB_OK|FMSG_WARNING,
			NULL, (const wchar_t* const*)L"ConEmu plugin\nCan't open file!", 0,0);
		return INVALID_HANDLE_VALUE;
	}

	DWORD dwSizeLow, dwSizeHigh = 0;
	dwSizeLow = GetFileSize(hFile, &dwSizeHigh);
	if (dwSizeHigh) {
		InfoW995->Message(InfoW995->ModuleNumber, FMSG_ALLINONE|FMSG_MB_OK|FMSG_WARNING,
			NULL, (const wchar_t* const*)L"ConEmu plugin\nFile too large!", 0,0);
		CloseHandle(hFile);
		return INVALID_HANDLE_VALUE;
	}

	wchar_t* pszData = (wchar_t*)calloc(dwSizeLow+2,1);
	if (!pszData) return INVALID_HANDLE_VALUE;
	if (!ReadFile(hFile, pszData, dwSizeLow, &dwSizeHigh, 0) || dwSizeHigh != dwSizeLow) {
		InfoW995->Message(InfoW995->ModuleNumber, FMSG_ALLINONE|FMSG_MB_OK|FMSG_WARNING,
			NULL, (const wchar_t* const*)L"ConEmu plugin\nCan't read file!", 0,0);
		return INVALID_HANDLE_VALUE;
	}
	CloseHandle(hFile);

	memset(&csbi, 0, sizeof(csbi));
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);

	HANDLE h = InfoW995->SaveScreen(0,0,-1,-1);

	CESERVER_REQ* pReq = (CESERVER_REQ*)pszData;
	if (pReq->hdr.cbSize == dwSizeLow)
	{
		if (pReq->hdr.nVersion != CESERVER_REQ_VER && pReq->hdr.nVersion < 40) {
			InfoW995->Message(InfoW995->ModuleNumber, FMSG_ALLINONE|FMSG_MB_OK|FMSG_WARNING,
				NULL, (const wchar_t* const*)L"ConEmu plugin\nUnknown version of packet", 0,0);
		} else {
			ShowConPacket(pReq);
		}
	} else if ((*(DWORD*)pszData) >= 0x200020) {
		ShowConDump(pszData);
	}

	InfoW995->RestoreScreen(NULL);
	InfoW995->RestoreScreen(h);
	InfoW995->Text(0,0,0,0);

	//InfoW995->Control(PANEL_ACTIVE, FCTL_REDRAWPANEL, 0, 0); 
	//InfoW995->Control(PANEL_PASSIVE, FCTL_REDRAWPANEL, 0, 0); 
	//INPUT_RECORD r = {WINDOW_BUFFER_SIZE_EVENT};
	//r.Event.WindowBufferSizeEvent.dwSize = csbi.dwSize;
	//WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &r, 1, &dwSizeLow);

	free(pszData);
	return INVALID_HANDLE_VALUE;
}
