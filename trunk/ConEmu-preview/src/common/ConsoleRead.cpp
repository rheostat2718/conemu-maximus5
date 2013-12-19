
/*
Copyright (c) 2009-2013 Maximus5
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
#include "ConsoleRead.h"

//#define DUMP_TEST_READS
#undef DUMP_TEST_READS

BOOL AreCpInfoLeads(DWORD nCP, UINT* pnMaxCharSize)
{
	BOOL bLeads = FALSE;
	static CPINFOEX cpinfo = {};
	static BOOL sbLeads = FALSE;

	if (cpinfo.CodePage != nCP)
	{
		if (!GetCPInfoEx(nCP, 0, &cpinfo) || (cpinfo.MaxCharSize <= 1))
		{
			sbLeads = bLeads = FALSE;
		}
		else
		{
			sbLeads = bLeads = (cpinfo.LeadByte[0] && cpinfo.LeadByte[1]); // At lease one lead-bytes range
		}
	}
	else if (cpinfo.MaxCharSize > 1)
	{
		bLeads = sbLeads;
	}

	if (pnMaxCharSize)
		*pnMaxCharSize = cpinfo.MaxCharSize;

	return bLeads;
}


// ������� ����� �������, ��� �������� ������� ����� ������������� ���������� �����.
// MAX_CONREAD_SIZE ��������� ����������������
BOOL ReadConsoleOutputEx(HANDLE hOut, CHAR_INFO *pData, COORD bufSize, SMALL_RECT rgn)
{
	BOOL lbRc = FALSE;

	DWORD nTick1 = GetTickCount(), nTick2 = 0, nTick3 = 0, nTick4 = 0, nTick5 = 0;

	static bool bDBCS = false, bDBCS_Checked = false;
	if (!bDBCS_Checked)
	{
		bDBCS = (GetSystemMetrics(SM_DBCSENABLED) != 0);
		bDBCS_Checked = true;
	}

	bool   bDBCS_CP = bDBCS;
	UINT   MaxCharSize = 0;
	DWORD  nCP, nCP1, nMode;

	if (bDBCS)
	{
		nCP = GetConsoleOutputCP();
		nCP1 = GetConsoleCP();
		GetConsoleMode(hOut, &nMode);

		if (!AreCpInfoLeads(nCP, &MaxCharSize) || MaxCharSize < 2)
		{
			bDBCS_CP = false;
		}
	}

	size_t nBufWidth = bufSize.X;
	int nWidth = (rgn.Right - rgn.Left + 1);
	int nHeight = (rgn.Bottom - rgn.Top + 1);
	int nCurSize = nWidth * nHeight;

	_ASSERTE(bufSize.X >= nWidth);
	_ASSERTE(bufSize.Y >= nHeight);
	_ASSERTE(rgn.Top>=0 && rgn.Bottom>=rgn.Top);
	_ASSERTE(rgn.Left>=0 && rgn.Right>=rgn.Left);

	//MSectionLock RCS;
	//if (gpSrv->pReqSizeSection && !RCS.Lock(gpSrv->pReqSizeSection, TRUE, 30000))
	//{
	//	_ASSERTE(FALSE);
	//	SetLastError(ERROR_INVALID_PARAMETER);
	//	return FALSE;

	//}

	COORD bufCoord = {0,0};
	DWORD dwErrCode = 0;
	CONSOLE_SCREEN_BUFFER_INFO sbi_tmp = {}; BOOL bSbiTmp = (BOOL)-1;

	nTick2 = GetTickCount();

	if (!bDBCS_CP && (nCurSize <= MAX_CONREAD_SIZE))
	{
		if (ReadConsoleOutputW(hOut, pData, bufSize, bufCoord, &rgn))
			lbRc = TRUE;
		nTick3 = GetTickCount();
	}

	if (!lbRc)
	{
		// �������� ������ ���������
		
		// ������������ - ����� � �������, �� ������� ����� ���������, � �����
		// ���� ����������, ���� ������� "������". ������� ���������...

		//bufSize.X = TextWidth;
		bufSize.Y = 1;
		bufCoord.X = 0; bufCoord.Y = 0;
		//rgn = gpSrv->sbi.srWindow;

		int Y1 = rgn.Top;
		int Y2 = rgn.Bottom;

		CHAR_INFO* pLine = pData;
		if (!bDBCS_CP)
		{
			// Simple processing (no DBCS) - one cell == one wchar_t
			for (int y = Y1; y <= Y2; y++, rgn.Top++, pLine+=nBufWidth)
			{
				nTick3 = GetTickCount();
				rgn.Bottom = rgn.Top;

				#ifdef DUMP_TEST_READS
				bSbiTmp = GetConsoleScreenBufferInfo(hOut, &sbi_tmp);
				#endif

				lbRc = ReadConsoleOutputW(hOut, pLine, bufSize, bufCoord, &rgn);

				#ifdef DUMP_TEST_READS
				UNREFERENCED_PARAMETER(sbi_tmp.dwSize.Y);
				#endif

				if (!lbRc)
				{
					dwErrCode = GetLastError();
					_ASSERTE(FALSE && "ReadConsoleOutputW failed in MyReadConsoleOutput");
					break;
				}
				nTick4 = GetTickCount();
			}
		}
		else // Process on DBCS-capable systems
		{
			for (int y = Y1; y <= Y2; y++, rgn.Top++, pLine+=nBufWidth)
			{
				nTick3 = GetTickCount();
				rgn.Bottom = rgn.Top;

				#ifdef DUMP_TEST_READS
				bSbiTmp = GetConsoleScreenBufferInfo(hOut, &sbi_tmp);
				#endif

				lbRc = ReadConsoleOutputW(hOut, pLine, bufSize, bufCoord, &rgn);

				#ifdef DUMP_TEST_READS
				UNREFERENCED_PARAMETER(sbi_tmp.dwSize.Y);
				#endif

				if (!lbRc)
				{
					dwErrCode = GetLastError();
					_ASSERTE(FALSE && "ReadConsoleOutputW failed in MyReadConsoleOutput");
					break;
				}

				// DBCS corrections (we need only glyph per hieroglyph for drawing)
				const CHAR_INFO* pSrc = pLine;
				const CHAR_INFO* pEnd = pLine + nBufWidth;
				CHAR_INFO* pDst = pLine;
				// Check first, line has hieroglyphs?
				while (pSrc < pEnd)
				{
					if (pSrc->Attributes & COMMON_LVB_LEADING_BYTE)
						break;
					*(pDst++) = *(pSrc++);
				}
				// If line was not fully processed (LVB was found)
				if (pSrc < pEnd)
				{
					// Yes, Process this line with substitutions
					while (pSrc < pEnd)
					{
						*pDst = *pSrc;
						wchar_t wc = pSrc->Char.UnicodeChar;
						if (pSrc->Attributes & COMMON_LVB_LEADING_BYTE)
						{
							while (((++pSrc) < pEnd) && !(pSrc->Attributes & COMMON_LVB_TRAILING_BYTE))
							{
								// May be 2 or 4 cells
								if ((pDst->Char.UnicodeChar != wc) && ((pDst+1) < pEnd))
								{
									wc = pSrc->Char.UnicodeChar;
									*(++pDst) = *pSrc;
								}
							}
							pDst->Attributes |= COMMON_LVB_TRAILING_BYTE;
						}
						pSrc++; pDst++;
					}
					// Clean rest of line
					WORD nLastAttr = (pEnd-1)->Attributes;
					while (pDst < pEnd)
					{
						pDst->Attributes = nLastAttr;
						pDst->Char.UnicodeChar = L' ';
						pDst++;
					}
				}

				// Line is done
				nTick4 = GetTickCount();
			}
		} // End of DBCS-capable processing

		nTick5 = GetTickCount();
	}

	UNREFERENCED_PARAMETER(nTick1);
	UNREFERENCED_PARAMETER(nTick2);
	UNREFERENCED_PARAMETER(nTick3);
	UNREFERENCED_PARAMETER(nTick4);
	UNREFERENCED_PARAMETER(nTick5);
	UNREFERENCED_PARAMETER(sbi_tmp.dwSize.Y);
	UNREFERENCED_PARAMETER(bSbiTmp);
	return lbRc;
}
