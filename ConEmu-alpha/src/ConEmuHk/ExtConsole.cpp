
/*
Copyright (c) 2012 Maximus5
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
#include "../common/defines.h"
#include "../common/common.hpp"
#include "../common/ConEmuCheck.h"
#include "../common/UnicodeChars.h"
#include "../ConEmu/version.h"
#include "ExtConsole.h"
#include "../common/ConEmuColors3.h"
#include "../common/WinObjects.h"

#define MSG_TITLE "ConEmu writer"
#define MSG_INVALID_CONEMU_VER "Unsupported ConEmu version detected!\nRequired version: " CONEMUVERS "\nConsole writer'll works in 4bit mode"
#define MSG_TRUEMOD_DISABLED   "�Colorer TrueMod support� is not checked in the ConEmu settings\nConsole writer'll works in 4bit mode"
#define MSG_NO_TRUEMOD_BUFFER  "TrueMod support not enabled in the ConEmu settings\nConsole writer'll works in 4bit mode"

TODO("���������� �� ��������� ������� �������");

extern HMODULE ghOurModule; // Must be defined and initialized in DllMain

HWND ghExtConEmuWndDC = NULL; // VirtualCon. ���������������� � ExtCheckBuffers()

extern HWND ghConWnd;      // Console window
extern HWND ghConEmuWnd;   // Root! window
extern HWND ghConEmuWndDC; // ConEmu DC window

BOOL GetConsoleScreenBufferInfoCached(HANDLE hConsoleOutput, PCONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo, BOOL bForced = FALSE);
BOOL GetConsoleModeCached(HANDLE hConsoleHandle, LPDWORD lpMode, BOOL bForced = FALSE);

CESERVER_CONSOLE_MAPPING_HDR SrvMapping = {};

AnnotationHeader* gpTrueColor = NULL;
HANDLE ghTrueColor = NULL;

bool gbInitialized = false;
DWORD gnInitializeErrCode = 0;
//bool gbFarBufferMode = false; // true, ���� Far ������� � ������ "/w"

WORD defConForeIdx = 7;
WORD defConBackIdx = 0;

typedef BOOL (WINAPI* WriteConsoleW_t)(HANDLE hConsoleOutput, const VOID *lpBuffer, DWORD nNumberOfCharsToWrite, LPDWORD lpNumberOfCharsWritten, LPVOID lpReserved);

struct ExtCurrentAttr
{
	bool  WasSet;
	bool  Reserved1;

	WORD  CONColor;
	WORD  CONForeIdx;
	WORD  CONBackIdx;

	ConEmuColor CEColor;
	AnnotationInfo AIColor;
} gExtCurrentAttr;


#if 0
static bool isCharSpace(wchar_t inChar)
{
	// ���� ������ ��� �������, ������� ����� ���������� ������ ����� (��� ������� ������)
	bool isSpace = (inChar == ucSpace || inChar == ucNoBreakSpace || inChar == 0 
		/*|| (inChar>=0x2000 && inChar<=0x200F)
		|| inChar == 0x2060 || inChar == 0x3000 || inChar == 0xFEFF*/);
	return isSpace;
}
#endif


static BOOL ExtGetBufferInfo(HANDLE &h, CONSOLE_SCREEN_BUFFER_INFO &csbi, SMALL_RECT &srWork)
{
	_ASSERTE(gbInitialized);
	
	if (!h)
	{
		_ASSERTE(h!=NULL);
		h = GetStdHandle(STD_OUTPUT_HANDLE);
	}

	if (!GetConsoleScreenBufferInfoCached(h, &csbi))
		return FALSE;
	
	//if (gbFarBufferMode)
	{
		// ��� �������� ������ ����� �������. ��������� � ������ ����
		SHORT nWidth  = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		SHORT nHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
		srWork.Left = 0;
		srWork.Right = nWidth - 1;
		srWork.Top = csbi.dwSize.Y - nHeight;
		srWork.Bottom = csbi.dwSize.Y - 1;
	}
	/*else
	{
		// ��� �������� ��� ���� �������
		srWork.Left = srWork.Top = 0;
		srWork.Right = csbi.dwSize.X - 1;
		srWork.Bottom = csbi.dwSize.Y - 1;
	}*/
	
	if (srWork.Left < 0 || srWork.Top < 0 || srWork.Left > srWork.Right || srWork.Top > srWork.Bottom)
	{
		_ASSERTE(srWork.Left >= 0 && srWork.Top >= 0 && srWork.Left <= srWork.Right && srWork.Top <= srWork.Bottom);
		return FALSE;
	}
	
	return TRUE;
}

static BOOL ExtCheckBuffers(HANDLE h)
{
	if (!h)
	{
		_ASSERTE(h!=NULL);
		h = GetStdHandle(STD_OUTPUT_HANDLE);
	}

	if (!gbInitialized)
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi = {};
		if (!GetConsoleScreenBufferInfoCached(h, &csbi, TRUE))
		{
			gnInitializeErrCode = GetLastError();
		}
		else
		{
			#ifdef _DEBUG
			SHORT nWidth  = csbi.srWindow.Right - csbi.srWindow.Left + 1;
			SHORT nHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
			_ASSERTE((nWidth <= csbi.dwSize.X) && (nHeight <= csbi.dwSize.Y));
			//gbFarBufferMode = (nWidth < csbi.dwSize.X) || (nHeight < csbi.dwSize.Y);
			#endif
			gbInitialized = true;
		}
	}

	if (!ghConEmuWndDC)
	{
		ExtCloseBuffers();
		SetLastError(E_HANDLE);
		return FALSE;
	}

	if (ghConEmuWndDC != ghExtConEmuWndDC)
	{
		ghExtConEmuWndDC = ghConEmuWndDC;
		ExtCloseBuffers();

		//TODO: ���� �������� "��-�������", ����� ����� TrueColor. ����������, �� �� ���������
		wchar_t szMapName[128];
		wsprintf(szMapName, AnnotationShareName, (DWORD)sizeof(AnnotationInfo), (DWORD)ghExtConEmuWndDC); //-V205
		ghTrueColor = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, szMapName);
		if (!ghTrueColor)
		{
			return FALSE;
		}
		gpTrueColor = (AnnotationHeader*)MapViewOfFile(ghTrueColor, FILE_MAP_ALL_ACCESS,0,0,0);
		if (!gpTrueColor)
		{
			CloseHandle(ghTrueColor);
			ghTrueColor = NULL;
			return FALSE;
		}
	}
	if (gpTrueColor)
	{
		if (!gpTrueColor->locked)
		{
			gpTrueColor->locked = TRUE;
		}
	}
	
	return (gpTrueColor!=NULL);
}

void ExtCloseBuffers()
{
	if (gpTrueColor)
	{
		UnmapViewOfFile(gpTrueColor);
		gpTrueColor = NULL;
	}
	if (ghTrueColor)
	{
		CloseHandle(ghTrueColor);
		ghTrueColor = NULL;
	}
}

// ��� ��� "���� �� ���������".
// ��, ��� ������������� "Screen Text" � "Screen Background" � ��������� �������.
// ��, ��� ������� �������� color � cmd.exe.
// ��, ��� ����� ������������ ���� � ������� ������ ������ 
// �� printf/std::cout/WriteConsole, ��� ������ �������� �����.
BOOL ExtGetAttributes(ExtAttributesParm* Info)
{
	if (!Info)
	{
		_ASSERTE(Info!=NULL);
		SetLastError(E_INVALIDARG);
		return FALSE;
	}

	BOOL lbTrueColor = ExtCheckBuffers(Info->ConsoleOutput);
	UNREFERENCED_PARAMETER(lbTrueColor);
	
	HANDLE h = Info->ConsoleOutput;
	CONSOLE_SCREEN_BUFFER_INFO csbi = {};
	SMALL_RECT srWork = {};
	if (!ExtGetBufferInfo(h, csbi, srWork))
	{
		_ASSERTE(FALSE && "ExtGetBufferInfo failed in ExtGetAttributes");
		gExtCurrentAttr.WasSet = false;
		gExtCurrentAttr.CEColor.Flags = CECF_NONE;
		gExtCurrentAttr.CEColor.ForegroundColor = defConForeIdx;
		gExtCurrentAttr.CEColor.BackgroundColor = defConBackIdx;
		memset(&gExtCurrentAttr.AIColor, 0, sizeof(gExtCurrentAttr.AIColor));
		return FALSE;
	}

	// ���-�� � ��������� ������� ��������� ���� ������ ��� printf
	//_ASSERTE("GetTextAttributes" && ((csbi.wAttributes & 0xFF) == 0x07));

	TODO("�� ��������, gExtCurrentAttr ���� ������� �� ������ h");
	if (gExtCurrentAttr.WasSet && (csbi.wAttributes == gExtCurrentAttr.CONColor))
	{
		Info->Attributes = gExtCurrentAttr.CEColor;
	}
	else
	{
		gExtCurrentAttr.WasSet = false;
		memset(&gExtCurrentAttr.CEColor, 0, sizeof(gExtCurrentAttr.CEColor));
		memset(&gExtCurrentAttr.AIColor, 0, sizeof(gExtCurrentAttr.AIColor));

		Info->Attributes.Flags = CECF_NONE;
		Info->Attributes.ForegroundColor = CONFORECOLOR(csbi.wAttributes);
		Info->Attributes.BackgroundColor = CONBACKCOLOR(csbi.wAttributes);
	}

	return TRUE;
}

static void ExtPrepareColor(const ConEmuColor& Attributes, AnnotationInfo& t, WORD& n)
{
	//-- zeroing must be done by calling function
	//memset(&t, 0, sizeof(t)); n = 0;

	WORD f = 0;
	CECOLORFLAGS Flags = Attributes.Flags;

	if (Flags & CECF_FG_BOLD)
		f |= AI_STYLE_BOLD;
	if (Flags & CECF_FG_ITALIC)
		f |= AI_STYLE_ITALIC;
	if (Flags & CECF_FG_UNDERLINE)
		f |= AI_STYLE_UNDERLINE;
	t.style = f;

	DWORD nForeColor, nBackColor;
	if (Flags & CECF_FG_24BIT)
	{
		//n |= 0x07;
		nForeColor = Attributes.ForegroundColor & 0x00FFFFFF;
		Far3Color::Color2FgIndex(nForeColor, n);
		t.fg_color = nForeColor;
		t.fg_valid = TRUE;
	}
	else
	{
		nForeColor = -1;
		n |= (WORD)INDEXVALUE(Attributes.ForegroundColor);
		t.fg_valid = FALSE;
	}

	if (Flags & CECF_BG_24BIT)
	{
		nBackColor = Attributes.BackgroundColor & 0x00FFFFFF;
		Far3Color::Color2BgIndex(nBackColor, nBackColor==nForeColor, n);
		t.bk_color = nBackColor;
		t.bk_valid = TRUE;
	}
	else
	{
		n |= (WORD)(INDEXVALUE(Attributes.BackgroundColor)<<4);
		t.bk_valid = FALSE;
	}
}

// ��� ��� "���� �� ���������".
// ��, ��� ������������� "Screen Text" � "Screen Background" � ��������� �������.
// ��, ��� ������� �������� color � cmd.exe.
// ��, ��� ����� ������������ ���� � ������� ������ ������ 
// �� printf/std::cout/WriteConsole, ��� ������ �������� �����.
BOOL ExtSetAttributes(const ExtAttributesParm* Info)
{
	if (!Info)
	{
		// ConEmu internals: �������� ����������� "�������"
		gExtCurrentAttr.WasSet = false;
		return TRUE;
	}

	BOOL lbTrueColor = ExtCheckBuffers(Info->ConsoleOutput);
	UNREFERENCED_PARAMETER(lbTrueColor);

	HANDLE h = Info->ConsoleOutput;
	CONSOLE_SCREEN_BUFFER_INFO csbi = {};
	SMALL_RECT srWork = {};
	if (!ExtGetBufferInfo(h, csbi, srWork))
		return FALSE;
	
	BOOL lbRc = TRUE;
	
	// <G|S>etTextAttributes ������ ������� ��������� �� ���� ConEmuColor.
	AnnotationInfo t = {};
	WORD n = 0;
	ExtPrepareColor(Info->Attributes, t, n);
	
	SetConsoleTextAttribute(h, n);
	
	TODO("�� ��������, gExtCurrentAttr ���� ������� �� ������ h");
	// ���������, ��� WriteConsole ������ ������ ��������� "t"
	gExtCurrentAttr.WasSet = true;
	gExtCurrentAttr.CONColor = n;
	gExtCurrentAttr.CONForeIdx = CONFORECOLOR(n);
	gExtCurrentAttr.CONBackIdx = CONBACKCOLOR(n);
	gExtCurrentAttr.CEColor = Info->Attributes;
	gExtCurrentAttr.AIColor = t;
	
	return lbRc;
}

void ConEmuCharBuffer::Inc(size_t Cells /*= 1*/)
{
	switch (BufferType)
	{
	case (int)ewtf_FarClr:
		FARBuffer += Cells;
		break;
	case (int)ewtf_CeClr:
		CEBuffer += Cells;
		CEColor += Cells;
		break;
	case (int)ewtf_AiClr:
		CEBuffer += Cells;
		AIColor += Cells;
		break;

	#ifdef _DEBUG
	default:
		_ASSERTE(BufferType==(int)ewtf_FarClr && "Invalid type specified!");
	#endif
	};
};


#if 0
// ConEmuCharBuffer Buffer, BufferSize, BufferCoord, Region
BOOL WINAPI ExtReadOutput(ExtReadWriteOutputParm* Info)
{
	if (!Info || (Info->StructSize < sizeof(*Info)))
	{
		_ASSERTE(Info!=NULL && (Info->StructSize >= sizeof(*Info)));
		SetLastError(E_INVALIDARG);
		return FALSE;
	}

	BOOL lbTrueColor = ExtCheckBuffers();
	UNREFERENCED_PARAMETER(lbTrueColor);

	HANDLE h = Info->ConsoleOutput;
	CONSOLE_SCREEN_BUFFER_INFO csbi = {};
	SMALL_RECT srWork = {};
	if (!ExtGetBufferInfo(h, csbi, srWork))
		return FALSE;

	CHAR_INFO cDefReadBuf[1024];
	CHAR_INFO *pcReadBuf = NULL;
	int nWindowWidth  = srWork.Right - srWork.Left + 1;
	if (Info->BufferSize.X <= (int)countof(cDefReadBuf))
		pcReadBuf = cDefReadBuf;
	else
		pcReadBuf = (CHAR_INFO*)calloc(Info->BufferSize.X,sizeof(*pcReadBuf));
	if (!pcReadBuf)
	{
		SetLastError(E_OUTOFMEMORY);
		return FALSE;
	}

	BOOL lbRc = TRUE;

	AnnotationInfo* pTrueColorStart = (AnnotationInfo*)(gpTrueColor ? (((LPBYTE)gpTrueColor) + gpTrueColor->struct_size) : NULL);
	AnnotationInfo* pTrueColorEnd = pTrueColorStart ? (pTrueColorStart + gpTrueColor->bufferSize) : NULL;
	PRAGMA _ERROR("�������� �� INT_PTR nLinePosition");
	//AnnotationInfo* pTrueColorLine = (AnnotationInfo*)(pTrueColorStart ? (pTrueColorStart + nWindowWidth * (Info->Region.Top /*- srWork.Top*/)) : NULL);

	int nBufferType = (int)(Info->Flags & ewtf_Mask);

	ConEmuCharBuffer BufEnd = {};
	BufEnd.BufferType = nBufferType;
	switch (nBufferType)
	{
	case (int)ewtf_FarClr:
		BufEnd.FARBuffer = Info->Buffer.FARBuffer + (Info->BufferSize.X * Info->BufferSize.Y); //-V104;
		break;
	case (int)ewtf_CeClr:
	case (int)ewtf_AiClr:
		BufEnd.CEBuffer = BufEnd.CEBuffer = Info->Buffer.CEBuffer + (Info->BufferSize.X * Info->BufferSize.Y); //-V104;
		break;
	default:
		_ASSERTE(nBufferType==(int)ewtf_FarClr && "Invalid type specified!");
		SetLastError(E_INVALIDARG);
		return FALSE;
	};
	

	SMALL_RECT rcRead = Info->Region;
	COORD MyBufferSize = {Info->BufferSize.X, 1};
	COORD MyBufferCoord = {Info->BufferCoord.X, 0};
	SHORT YShift = /*gbFarBufferMode ?*/ (csbi.dwSize.Y - (srWork.Bottom - srWork.Top + 1)) /*: 0*/;
	SHORT Y1 = Info->Region.Top + YShift;
	SHORT Y2 = Info->Region.Bottom + YShift;
	SHORT BufferShift = Info->Region.Top + YShift;

	if (Y2 >= csbi.dwSize.Y)
	{
		_ASSERTE((Y2 >= 0) && (Y2 < csbi.dwSize.Y));
		Y2 = csbi.dwSize.Y - 1;
		lbRc = FALSE; // �� ���������, �������, ������� ������
	}

	if ((Y1 < 0) || (Y1 >= csbi.dwSize.Y))
	{
		_ASSERTE((Y1 >= 0) && (Y1 < csbi.dwSize.Y));
		if (Y1 >= csbi.dwSize.Y)
			Y1 = csbi.dwSize.Y - 1;
		if (Y1 < 0)
			Y1 = 0;
		lbRc = FALSE;
	}
	
	for (rcRead.Top = Y1; rcRead.Top <= Y2; rcRead.Top++)
	{
		rcRead.Bottom = rcRead.Top;
		BOOL lbRead = MyReadConsoleOutputW(h, pcReadBuf, MyBufferSize, MyBufferCoord, &rcRead);

		CHAR_INFO* pc = pcReadBuf + Info->BufferCoord.X;
		ConEmuCharBuffer BufPtr = Info->Buffer;
		BufPtr.Inc((rcRead.Top - BufferShift + Info->BufferCoord.Y)*Info->BufferSize.X + Info->BufferCoord.X); //-V104
		//FAR_CHAR_INFO* pFar = Buffer + (rcRead.Top - BufferShift + Info->BufferCoord.Y)*Info->BufferSize.X + Info->BufferCoord.X;
		AnnotationInfo* pTrueColor = (pTrueColorLine && (pTrueColorLine >= pTrueColorStart)) ? (pTrueColorLine + Info->Region.Left) : NULL;

		for (int X = rcRead.Left; X <= rcRead.Right; X++, pc++, BufPtr.Inc())
		{
			if (BufPtr.RAW >= BufEnd.RAW)
			{
				_ASSERTE(BufPtr.RAW < BufEnd.RAW);
				break;
			}

			if (pTrueColor && pTrueColor >= pTrueColorEnd)
			{
				_ASSERTE(pTrueColor && pTrueColor < pTrueColorEnd);
				pTrueColor = NULL; // ���������� ����� �������� �������������
			}

			switch (nBufferType)
			{
			case (int)ewtf_FarClr:
				{
					FAR_CHAR_INFO chr = {lbRead ? pc->Char.UnicodeChar : L' '};

					if (pTrueColor)
					{
						DWORD Style = pTrueColor->style;
						if (Style & AI_STYLE_BOLD)
							chr.Attributes.Flags |= FCF_FG_BOLD;
						if (Style & AI_STYLE_ITALIC)
							chr.Attributes.Flags |= FCF_FG_ITALIC;
						if (Style & AI_STYLE_UNDERLINE)
							chr.Attributes.Flags |= FCF_FG_UNDERLINE;
					}
					
					if (pTrueColor && pTrueColor->fg_valid)
					{
						chr.Attributes.ForegroundColor = pTrueColor->fg_color;
					}
					else
					{
						chr.Attributes.Flags |= FCF_FG_4BIT;
						chr.Attributes.ForegroundColor = lbRead ? CONFORECOLOR(pc->Attributes) : defConForeIdx;
					}

					if (pTrueColor && pTrueColor->bk_valid)
					{
						chr.Attributes.BackgroundColor = pTrueColor->bk_color;
					}
					else
					{
						chr.Attributes.Flags |= FCF_BG_4BIT;
						chr.Attributes.BackgroundColor = lbRead ? CONBACKCOLOR(pc->Attributes) : defConBackIdx;
					}

					*BufPtr.FARBuffer = chr;

				} // ewtf_FarClr
				break;

			case (int)ewtf_CeClr:
				{
					*BufPtr.CEBuffer = *pc;

					ConEmuColor clr = {};

					if (pTrueColor)
					{
						DWORD Style = pTrueColor->style;
						if (Style & AI_STYLE_BOLD)
							clr.Flags |= CECF_FG_BOLD;
						if (Style & AI_STYLE_ITALIC)
							clr.Flags |= CECF_FG_ITALIC;
						if (Style & AI_STYLE_UNDERLINE)
							clr.Flags |= CECF_FG_UNDERLINE;
					}
					
					if (pTrueColor && pTrueColor->fg_valid)
					{
						clr.Flags |= CECF_FG_24BIT;
						clr.ForegroundColor = pTrueColor->fg_color;
					}
					else
					{
						clr.ForegroundColor = lbRead ? CONFORECOLOR(pc->Attributes) : defConForeIdx;
					}

					if (pTrueColor && pTrueColor->bk_valid)
					{
						clr.Flags |= CECF_BG_24BIT;
						clr.BackgroundColor = pTrueColor->bk_color;
					}
					else
					{
						clr.BackgroundColor = lbRead ? CONBACKCOLOR(pc->Attributes) : defConBackIdx;
					}

					*BufPtr.CEColor = clr;

				} // ewtf_CeClr
				break;

			case (int)ewtf_AiClr:
				{
					*BufPtr.CEBuffer = *pc;

					if (pTrueColor)
						*BufPtr.AIColor = *pTrueColor;
					else
						memset(BufPtr.AIColor, 0, sizeof(*BufPtr.AIColor));

				} // ewtf_AiClr
				break;
			};

			if (pTrueColor)
				pTrueColor++;
		}

		if (pTrueColorLine)
			pTrueColorLine += nWindowWidth; //-V102
	}

	if (pcReadBuf != cDefReadBuf)
		free(pcReadBuf);

	return lbRc;
}


// ConEmuCharBuffer Buffer, BufferSize, BufferCoord, Region
BOOL WINAPI ExtWriteOutput(const ExtReadWriteOutputParm* Info)
{
	if (!Info || (Info->StructSize < sizeof(*Info)))
	{
		_ASSERTE(Info!=NULL && (Info->StructSize >= sizeof(*Info)));
		SetLastError(E_INVALIDARG);
		return FALSE;
	}

	BOOL lbTrueColor = ExtCheckBuffers();
	UNREFERENCED_PARAMETER(lbTrueColor);

	HANDLE h = Info->ConsoleOutput;
	CONSOLE_SCREEN_BUFFER_INFO csbi = {};
	SMALL_RECT srWork = {};
	if (!ExtGetBufferInfo(h, csbi, srWork))
		return FALSE;

	CHAR_INFO cDefWriteBuf[1024];
	CHAR_INFO *pcWriteBuf = NULL;
	int nWindowWidth  = srWork.Right - srWork.Left + 1;
	if (Info->BufferSize.X <= (int)countof(cDefWriteBuf))
		pcWriteBuf = cDefWriteBuf;
	else
		pcWriteBuf = (CHAR_INFO*)calloc(Info->BufferSize.X,sizeof(*pcWriteBuf));
	if (!pcWriteBuf)
	{
		SetLastError(E_OUTOFMEMORY);
		return FALSE;
	}

	BOOL lbRc = TRUE;

	AnnotationInfo* pTrueColorStart = (AnnotationInfo*)(gpTrueColor ? (((LPBYTE)gpTrueColor) + gpTrueColor->struct_size) : NULL); //-V104
	AnnotationInfo* pTrueColorEnd = pTrueColorStart ? (pTrueColorStart + gpTrueColor->bufferSize) : NULL; //-V104
	PRAGMA _ERROR("�������� �� INT_PTR nLinePosition");
	//AnnotationInfo* pTrueColorLine = (AnnotationInfo*)(pTrueColorStart ? (pTrueColorStart + nWindowWidth * (Info->Region.Top /*- srWork.Top*/)) : NULL); //-V104

	int nBufferType = (int)(Info->Flags & ewtf_Mask);

	SMALL_RECT rcWrite = Info->Region;
	COORD MyBufferSize = {Info->BufferSize.X, 1};
	COORD MyBufferCoord = {Info->BufferCoord.X, 0};
	SHORT YShift = /*gbFarBufferMode ?*/ (csbi.dwSize.Y - (srWork.Bottom - srWork.Top + 1)) /*: 0*/;
	SHORT Y1 = Info->Region.Top + YShift;
	SHORT Y2 = Info->Region.Bottom + YShift;
	SHORT BufferShift = Info->Region.Top + YShift;

	if (Y2 >= csbi.dwSize.Y)
	{
		_ASSERTE((Y2 >= 0) && (Y2 < csbi.dwSize.Y));
		Y2 = csbi.dwSize.Y - 1;
		lbRc = FALSE; // �� ���������, �������, ������� ������
	}

	if ((Y1 < 0) || (Y1 >= csbi.dwSize.Y))
	{
		_ASSERTE((Y1 >= 0) && (Y1 < csbi.dwSize.Y));
		if (Y1 >= csbi.dwSize.Y)
			Y1 = csbi.dwSize.Y - 1;
		if (Y1 < 0)
			Y1 = 0;
		lbRc = FALSE;
	}
	
	for (rcWrite.Top = Y1; rcWrite.Top <= Y2; rcWrite.Top++)
	{
		rcWrite.Bottom = rcWrite.Top;

		CHAR_INFO* pc = pcWriteBuf + Info->BufferCoord.X;
		ConEmuCharBuffer BufPtr = Info->Buffer;
		BufPtr.Inc((rcWrite.Top - BufferShift + Info->BufferCoord.Y)*Info->BufferSize.X + Info->BufferCoord.X); //-V104
		//const FAR_CHAR_INFO* pFar = Buffer + (rcWrite.Top - BufferShift + Info->BufferCoord.Y)*Info->BufferSize.X + Info->BufferCoord.X; //-V104
		AnnotationInfo* pTrueColor = (pTrueColorLine && (pTrueColorLine >= pTrueColorStart)) ? (pTrueColorLine + Info->Region.Left) : NULL;


		for (int X = rcWrite.Left; X <= rcWrite.Right; X++, pc++, BufPtr.Inc())
		{
			if (pTrueColor && pTrueColor >= pTrueColorEnd)
			{
				#ifdef _DEBUG
				static bool bBufferAsserted = false;
				if (!bBufferAsserted)
				{
					bBufferAsserted = true;
					_ASSERTE(pTrueColor && pTrueColor < pTrueColorEnd && "Allocated buffer was not enough");
				}
				#endif
				pTrueColor = NULL; // ���������� ����� �������� �������������
			}

			bool Bold = false, Italic = false, Underline = false;
			bool Fore24bit = false, Back24bit = false;
			DWORD ForegroundColor = 0, BackgroundColor = 0;

			// go
			switch (nBufferType)
			{
			case (int)ewtf_FarClr:
				{
					pc->Char.UnicodeChar = BufPtr.FARBuffer->Char;

					unsigned __int64 Flags = BufPtr.FARBuffer->Attributes.Flags;
					
					Bold = (Flags & FCF_FG_BOLD) != 0;
					Italic = (Flags & FCF_FG_ITALIC) != 0;
					Underline = (Flags & FCF_FG_UNDERLINE) != 0;

					Fore24bit = (Flags & FCF_FG_4BIT) == 0;
					Back24bit = (Flags & FCF_BG_4BIT) == 0;

					ForegroundColor = BufPtr.FARBuffer->Attributes.ForegroundColor & (Fore24bit ? COLORMASK : INDEXMASK);
					BackgroundColor = BufPtr.FARBuffer->Attributes.BackgroundColor & (Back24bit ? COLORMASK : INDEXMASK);
				}
				break;

			case (int)ewtf_CeClr:
				{
					pc->Char.UnicodeChar = BufPtr.CEBuffer->Char.UnicodeChar;

					unsigned __int64 Flags = BufPtr.CEColor->Flags;
					
					Bold = (Flags & CECF_FG_BOLD) != 0;
					Italic = (Flags & CECF_FG_ITALIC) != 0;
					Underline = (Flags & CECF_FG_UNDERLINE) != 0;

					Fore24bit = (Flags & CECF_FG_24BIT) != 0;
					Back24bit = (Flags & CECF_BG_24BIT) != 0;

					ForegroundColor = BufPtr.CEColor->ForegroundColor & (Fore24bit ? COLORMASK : INDEXMASK);
					BackgroundColor = BufPtr.CEColor->BackgroundColor & (Back24bit ? COLORMASK : INDEXMASK);
				}
				break;

			case (int)ewtf_AiClr:
				{
					pc->Char.UnicodeChar = BufPtr.CEBuffer->Char.UnicodeChar;

					unsigned int Flags = BufPtr.AIColor->style;

					Bold = (Flags & AI_STYLE_BOLD) != 0;
					Italic = (Flags & AI_STYLE_ITALIC) != 0;
					Underline = (Flags & AI_STYLE_UNDERLINE) != 0;

					Fore24bit = BufPtr.AIColor->fg_valid != 0;
					Back24bit = BufPtr.AIColor->bk_valid != 0;

					ForegroundColor = Fore24bit ? BufPtr.AIColor->fg_color : CONFORECOLOR(BufPtr.CEBuffer->Attributes);
					BackgroundColor = Back24bit ? BufPtr.AIColor->bk_color : CONBACKCOLOR(BufPtr.CEBuffer->Attributes);
				}
				break;
			}

			WORD n = 0, f = 0;
			
			if (pTrueColor)
			{
				if (Bold)
					f |= AI_STYLE_BOLD;
				if (Italic)
					f |= AI_STYLE_ITALIC;
				if (Underline)
					f |= AI_STYLE_UNDERLINE;
				pTrueColor->style = f;
			}

			DWORD nForeColor, nBackColor;
			if (!Fore24bit)
			{
				nForeColor = -1;
				_ASSERTE(ForegroundColor<=INDEXMASK);
				n |= (WORD)ForegroundColor;
				if (pTrueColor)
				{
					pTrueColor->fg_valid = FALSE;
				}
			}
			else
			{
				//n |= 0x07;
				nForeColor = ForegroundColor;
				Far3Color::Color2FgIndex(nForeColor, n);
				if (pTrueColor)
				{
					pTrueColor->fg_color = nForeColor;
					pTrueColor->fg_valid = TRUE;
				}
			}
			
			if (!Back24bit)
			{
				nBackColor = -1;
				_ASSERTE(BackgroundColor<=INDEXMASK);
				WORD bk = (WORD)BackgroundColor;
				// ��������� �������, ���� ����������� ������� �������
				if (n == bk && Fore24bit && !isCharSpace(BufPtr.FARBuffer->Char))
				{
					if (n & 8)
						bk ^= 8;
					else
						n |= 8;
				}
				n |= bk<<4;

				if (pTrueColor)
				{
					pTrueColor->bk_valid = FALSE;
				}
			}
			else
			{
				nBackColor = BackgroundColor;
				Far3Color::Color2BgIndex(nBackColor, nForeColor==nBackColor, n);
				if (pTrueColor)
				{
					pTrueColor->bk_color = nBackColor;
					pTrueColor->bk_valid = TRUE;
				}
			}

			// Apply color (console, 4bit)
			pc->Attributes = n;

			// Done
			if (pTrueColor)
				pTrueColor++;
		}
		
		if (!WriteConsoleOutputW(h, pcWriteBuf, MyBufferSize, MyBufferCoord, &rcWrite))
		{
			lbRc = FALSE;
		}

		if (pTrueColorLine)
			pTrueColorLine += nWindowWidth; //-V102
	}

	PRAGMA _ERROR("�� ������ ��������� ������ � AnnotationInfo");

	// Commit �� �����
	if (Info->Flags & ewtf_Commit)
	{
		ExtCommitParm c = {sizeof(c), Info->ConsoleOutput};
		ExtCommit(&c);
	}

	if (pcWriteBuf != cDefWriteBuf)
		free(pcWriteBuf);

	return lbRc;
}
#endif

BOOL WINAPI apiWriteConsoleW(HANDLE hConsoleOutput, const VOID *lpBuffer, DWORD nNumberOfCharsToWrite, LPDWORD lpNumberOfCharsWritten, LPVOID lpReserved, WriteConsoleW_t lpfn = NULL)
{
	BOOL lbRc = FALSE;

	static WriteConsoleW_t fnWriteConsoleW = NULL;

	if (!fnWriteConsoleW)
	{
		if (lpfn)
		{
			fnWriteConsoleW = lpfn;
		}
		else
		{
			HMODULE hHooks = GetModuleHandle(WIN3264TEST(L"ConEmuHk.dll",L"ConEmuHk64.dll"));
			if (hHooks && (hHooks != ghOurModule))
			{
				typedef FARPROC (WINAPI* GetWriteConsoleW_t)();
				GetWriteConsoleW_t getf = (GetWriteConsoleW_t)GetProcAddress(hHooks, "GetWriteConsoleW");
				if (!getf)
				{
					_ASSERTE(getf!=NULL);
					return FALSE;
				}

				fnWriteConsoleW = (WriteConsoleW_t)getf();
				if (!fnWriteConsoleW)
				{
					_ASSERTE(fnWriteConsoleW!=NULL);
					return FALSE;
				}
			}

			if (!fnWriteConsoleW)
			{
				fnWriteConsoleW = WriteConsoleW;
			}
		}
	}
	
	lbRc = fnWriteConsoleW(hConsoleOutput, lpBuffer, nNumberOfCharsToWrite, lpNumberOfCharsWritten, lpReserved);
	
	return lbRc;
}


static BOOL IntWriteText(HANDLE h, SHORT x, SHORT ForceDumpX,
						 const wchar_t *pFrom, DWORD cchWrite,
						 AnnotationInfo* pTrueColorLine, AnnotationInfo* pTrueColorEnd,
						 const AnnotationInfo& AIColor,
						 WriteConsoleW_t lpfn = NULL)
{
	// ����� ����������� ��������� (���� ����� � �����)
	if (pTrueColorLine)
	{
		AnnotationInfo* pTrueColor = pTrueColorLine + x;

		for (SHORT x1 = x; x1 <= ForceDumpX; x1++)
		{
			if (pTrueColor >= pTrueColorEnd)
			{
				#ifdef _DEBUG
				static bool bBufferAsserted = false;
				if (!bBufferAsserted)
				{
					bBufferAsserted = true;
					_ASSERTE(pTrueColor && pTrueColor < pTrueColorEnd && "Allocated buffer was not enough");
				}
				#endif
				pTrueColor = NULL; // ���������� ����� �������� �������������
				break;
			}
			
			*(pTrueColor++) = AIColor;
		}
	}

	// ����� � ������� "������"
	DWORD nWritten = 0;
	BOOL lbRc = apiWriteConsoleW(h, pFrom, cchWrite, &nWritten, NULL, lpfn);

	return lbRc;
}


// wchar_t* Buffer, NumberOfCharsToWrite
BOOL ExtWriteText(ExtWriteTextParm* Info)
{
	if (!Info || (Info->StructSize < sizeof(*Info)))
	{
		_ASSERTE(Info!=NULL && (Info->StructSize >= sizeof(*Info)));
		SetLastError(E_INVALIDARG);
		return FALSE;
	}

#ifdef _DEBUG
	static HMODULE hKernel = GetModuleHandle(L"kernel32.dll");
	// � x64 ����������, ��� Info->Private ��������� �� ���������� �������,
	// � WriteConsoleW - �� ��������/��������
	_ASSERTE((Info->Private==NULL) || (Info->Private==(void*)WriteConsoleW || CheckCallbackPtr(hKernel, 1, (FARPROC*)&Info->Private, TRUE)));
#endif

	// �������� ����������
	if (!Info->ConsoleOutput || !Info->Buffer || !Info->NumberOfCharsToWrite)
	{
		_ASSERTE(Info->ConsoleOutput && Info->Buffer && Info->NumberOfCharsToWrite);
		SetLastError(E_INVALIDARG);
		return FALSE;
	}

	BOOL lbTrueColor = ExtCheckBuffers(Info->ConsoleOutput);
	UNREFERENCED_PARAMETER(lbTrueColor);

	HANDLE h = Info->ConsoleOutput;
	CONSOLE_SCREEN_BUFFER_INFO csbi = {};
	SMALL_RECT srWork = {};
	if (!ExtGetBufferInfo(h, csbi, srWork))
		return FALSE;

	bool  bWrap = true, bVirtualWrap = false;;
	SHORT WrapAtCol = csbi.dwSize.X; // 1-based
	DWORD Mode = 0;

	// "Working lines" may be defined (Vim and others)
	LONG  ScrollTop, ScrollBottom;
	bool  ScrollRegion = ((Info->Flags & ewtf_Region) != 0);
	COORD crScrollCursor;
	if (ScrollRegion)
	{
		_ASSERTEX(Info->Region.left==0 && Info->Region.right==0); // Not used yet
		_ASSERTEX(Info->Region.top>=0 && Info->Region.bottom>=Info->Region.top);
		ScrollTop = Info->Region.top;
		ScrollBottom = Info->Region.bottom+1;
	}
	else
	{
		ScrollTop = 0;
		ScrollBottom = csbi.dwSize.Y;
	}

	if ((Info->Flags & ewtf_WrapAt))
	{
		if ((Info->WrapAtCol > 0) && (Info->WrapAtCol < WrapAtCol))
		{
			WrapAtCol = Info->WrapAtCol;
			bVirtualWrap = true;
		}
	}
	else
	{
		GetConsoleModeCached(h, &Mode);
		bWrap = (Mode & ENABLE_WRAP_AT_EOL_OUTPUT) != 0;
	}

	_ASSERTE(srWork.Bottom == (csbi.dwSize.Y - 1));
	// ������ TrueColor ������
	SHORT nWindowWidth  = srWork.Right - srWork.Left + 1;
	DEBUGTEST(SHORT nWindowHeight = srWork.Bottom - srWork.Top + 1);
	SHORT YShift = csbi.dwCursorPosition.Y - srWork.Top;

	BOOL lbRc = TRUE;


	AnnotationInfo* pTrueColorStart = (AnnotationInfo*)(gpTrueColor ? (((LPBYTE)gpTrueColor) + gpTrueColor->struct_size) : NULL); //-V104
	AnnotationInfo* pTrueColorEnd = pTrueColorStart ? (pTrueColorStart + gpTrueColor->bufferSize) : NULL; //-V104
	//AnnotationInfo* pTrueColorLine = (AnnotationInfo*)(pTrueColorStart ? (pTrueColorStart + nWindowWidth * YShift) : NULL); //-V104
	INT_PTR nLinePosition = nWindowWidth * YShift;

	// ���� ������ "�������" ��������� ������
	_ASSERTE((Info->Flags & ewtf_Current) == ewtf_Current);

	// Current attributes
	AnnotationInfo AIColor = {};
	ExtAttributesParm DefClr = {sizeof(DefClr), h};
	ExtGetAttributes(&DefClr);
	WORD DefConColor = 0;
	ExtPrepareColor(DefClr.Attributes, AIColor, DefConColor);

	// go
	const wchar_t *pFrom = Info->Buffer;
	const wchar_t *pEnd = pFrom + Info->NumberOfCharsToWrite;


	const wchar_t *pCur = pFrom;
	SHORT x = csbi.dwCursorPosition.X, y = csbi.dwCursorPosition.Y; // 0-based
	SHORT x2 = x, y2 = y;

	Info->ScrolledRowsUp = 0;

	TODO("��� ����� ���� ������ - ���� ������ ���������� � ��� �� ����� ������� � ������� - ����� �����...");
	for (; pCur < pEnd; pCur++)
	{
		// ���������
		// bWrap - ��� ������ � ��������� ������� ������� - ���������� ������, ���� ��������� ������ - ������ ������

		/*
		if (csbi.dwCursorPosition.X > nMaxCursorPos)
		{
			// �.�. ��� ������� ������� ������� �������� �� ����� (��������?)
			// ��� ����� ���� ������ � ��� ������, ���� ������ ����� "�� ������ �������"
			// ��������, ������� ������ ������� ������, � ����� ���������� ������ ����
			csbi.dwCursorPosition.X++;
			PadAndScroll(hConsoleOutput, csbi);
		}
		*/

		SHORT ForceDumpX = 0;
		bool BS = false;

		// ��������� ��������, ������� ������� ������
		switch (*pCur)
		{
		case L'\t':
			x2 = ((x2 + 7) >> 3) << 3;
			// like normal char...
			if (x2 >= WrapAtCol)
			{
				ForceDumpX = min(x2, WrapAtCol)-1;
			}
			break;
		case L'\r':
			ForceDumpX = x2;
			x2 = 0;
			break;
		case L'\n':
			ForceDumpX = x2;
			x2 = 0; y2++;
			_ASSERTE(bWrap);
			break;
		case 7:
			//Beep (no spacing)
			break;
		case 8:
			//BS
			if (x2>x)
				ForceDumpX = x2;
			if (x2>0)
				x2--;
			BS = true;
			break;
		default:
			// ������ �����.
			// � real-������� ����� �� �� ����� ���� non-spacing ��������, ����� ����-������������
			x2++;
			// like '\t'
			if (x2 >= WrapAtCol)
			{
				ForceDumpX = min(x2, WrapAtCol)-1;
			}
		}

		
		if (ForceDumpX)
		{
			// ����� ����������� ��������� (���� ����� � �����) � ���������� ������
			// ������� ������ pCur
			lbRc = IntWriteText(h, x, ForceDumpX, pFrom, (DWORD)(pCur - pFrom + 1),
						(pTrueColorStart && (nLinePosition >= 0)) ? (pTrueColorStart + nLinePosition) : NULL,
						(pTrueColorEnd), AIColor, (WriteConsoleW_t)Info->Private);

			// Update processed pos
			pFrom = pCur + 1;
		}

		// ����� BS - ������� "���������" ����������
		if (BS)
			x = x2;

		// ��� ����� ������
		if (y2 > y)
		{
			_ASSERTE(bWrap && "��� !Wrap - ��������");
			if (y2 >= ScrollBottom/*csbi.dwSize.Y*/)
			{
				// ����� ����������� �� ���� ������ �����
				
				// ����������� ����� - ����������
				_ASSERTE((y-y2) == -1); // ������ ���� ����� �� ���� ������
				ExtScrollScreenParm Shift = {sizeof(Shift), essf_None, h, y-y2, DefClr.Attributes, L' '};
				if (ScrollRegion)
				{
					Shift.Flags |= essf_Region;
					Shift.Region = Info->Region;
					//Shift.Region.top += (y2-y);
					if (ScrollBottom >= csbi.dwSize.Y)
						Shift.Flags |= essf_ExtOnly;
				}
				else
				{
					Shift.Flags |= essf_ExtOnly;
				}
				ExtScrollScreen(&Shift);

				Info->ScrolledRowsUp++;

				// ���������� - "��������" (��� ��� �� �� ����������)
				if (ScrollRegion && (ScrollBottom < csbi.dwSize.Y))
				{
					y2 = ScrollBottom - 1;
					_ASSERTEX((int)y2 == (int)(ScrollBottom - 1));
					
					crScrollCursor.X = x2;
					crScrollCursor.Y = y2;
					
					SetConsoleCursorPosition(Info->ConsoleOutput, crScrollCursor);
				}
				else
				{
					y2 = csbi.dwSize.Y - 1;
				}
			}
			else //if (pTrueColorLine)
			{
				// ������� �� ��������� ������ � ����������� ������
				nLinePosition += nWindowWidth;
				//pTrueColorLine += nWindowWidth; //-V102
			}
			y = y2;
		}
	}

	if (pCur > pFrom)
	{
		// �� ������� ������ pCur
		SHORT ForceDumpX = (x2 > x) ? (min(x2, WrapAtCol)-1) : -1;
		lbRc = IntWriteText(h, x, ForceDumpX, pFrom, (DWORD)(pCur - pFrom),
					 (pTrueColorStart && (nLinePosition >= 0)) ? (pTrueColorStart + nLinePosition) : NULL,
					 (pTrueColorEnd), AIColor, (WriteConsoleW_t)Info->Private);
	}


	// ��������� ������ � AnnotationInfo
	if (gpTrueColor)
	{
		gpTrueColor->flushCounter++;
	}

	// Commit �� �����
	if (Info->Flags & ewtf_Commit)
	{
		ExtCommitParm c = {sizeof(c), Info->ConsoleOutput};
		ExtCommit(&c);
	}

	if (lbRc)
		Info->NumberOfCharsWritten = Info->NumberOfCharsToWrite;

	return lbRc;
}

BOOL ExtFillOutput(ExtFillOutputParm* Info)
{
	if (!Info || !Info->Flags || !Info->Count)
	{
		_ASSERTE(Info && Info->Flags && Info->Count);
		return FALSE;
	}

	BOOL lbRc = TRUE, b;
	DWORD nWritten;

	BOOL lbTrueColor = ExtCheckBuffers(Info->ConsoleOutput);
	UNREFERENCED_PARAMETER(lbTrueColor);

	HANDLE h = Info->ConsoleOutput;
	CONSOLE_SCREEN_BUFFER_INFO csbi = {};
	SMALL_RECT srWork = {};
	if (!ExtGetBufferInfo(h, csbi, srWork))
		return FALSE;

	_ASSERTE(srWork.Bottom == (csbi.dwSize.Y - 1));
	// ������ TrueColor ������
	SHORT nWindowWidth  = srWork.Right - srWork.Left + 1;
	SHORT nWindowHeight = srWork.Bottom - srWork.Top + 1; UNREFERENCED_PARAMETER(nWindowHeight);
	//SHORT YShift = csbi.dwCursorPosition.Y - srWork.Top;

	AnnotationInfo* pTrueColorStart = (AnnotationInfo*)(gpTrueColor ? (((LPBYTE)gpTrueColor) + gpTrueColor->struct_size) : NULL); //-V104
	AnnotationInfo* pTrueColorEnd = pTrueColorStart ? (pTrueColorStart + gpTrueColor->bufferSize) : NULL; //-V104
	//AnnotationInfo* pTrueColorLine = (AnnotationInfo*)(pTrueColorStart ? (pTrueColorStart + nWindowWidth * YShift) : NULL); //-V104
	UNREFERENCED_PARAMETER(pTrueColorEnd);


	if (Info->Flags & efof_Attribute)
	{
		if (Info->Flags & efof_Current)
		{
			ExtAttributesParm DefClr = {sizeof(DefClr), h};
			ExtGetAttributes(&DefClr);
			Info->FillAttr = DefClr.Attributes;
		}
		else if (Info->Flags & efof_ResetExt)
		{
			Info->FillAttr.Flags = CECF_NONE;
			// ���� - ��� �������. ����� �������� ������ ����������� ��������,
			// �������� ���� � ������� ��������� ��� ���������
			Info->FillAttr.ForegroundColor = 7;
			Info->FillAttr.BackgroundColor = 0;
		}

		AnnotationInfo t = {};
		WORD n = 0;
		ExtPrepareColor(Info->FillAttr, t, n);

		if (gpTrueColor)
		{
			SHORT Y1 = Info->Coord.Y;
			DWORD nEndCell = Info->Coord.X + (Info->Coord.Y * csbi.dwSize.X) + Info->Count;
			SHORT Y2 = (SHORT)(nEndCell / csbi.dwSize.X);
			SHORT X2 = (SHORT)(nEndCell % csbi.dwSize.X);
			TODO("��������� ����� ������");
			DWORD nMaxLines = gpTrueColor->bufferSize / nWindowWidth;
			if (Y2 >= (int)(srWork.Top + nMaxLines))
			{
				Y2 = (SHORT)(srWork.Top + nMaxLines - 1);
				X2 = nWindowWidth;
			}

			for (SHORT y = max(Y1, srWork.Top); y <= Y2; y++)
			{
				SHORT xMax = (y == Y2) ? X2 : nWindowWidth;
				SHORT xMin = (y == Y1) ? Info->Coord.X : 0;
				TODO("��������� ������ �������, � �� ������ ������� �����...");
				INT_PTR nShift = nWindowWidth * (y - srWork.Top) + xMin;
				if (nShift < 0)
				{
					_ASSERTE((nWindowWidth * (y - srWork.Top) + xMin) > 0);
					continue;
				}
				AnnotationInfo* pTrueColor = pTrueColorStart + nShift;
				for (SHORT x = xMin; x < xMax; x++)
				{
					*(pTrueColor++) = t;
				}
				_ASSERTE(pTrueColor <= pTrueColorEnd);
			}
		}

		if (!(Info->Flags & efof_ResetExt))
		{
			b = FillConsoleOutputAttribute(h, n, Info->Count, Info->Coord, &nWritten);
			lbRc &= b;
		}
	}

	if (Info->Flags & efof_Character)
	{
		b = FillConsoleOutputCharacter(h, Info->FillChar ? Info->FillChar : L' ', Info->Count, Info->Coord, &nWritten);
		lbRc &= b;
	}

	// Commit �� �����
	if (Info->Flags & ewtf_Commit)
	{
		ExtCommitParm c = {sizeof(c), h};
		ExtCommit(&c);
	}

	return lbRc;
}


BOOL ExtScrollScreen(ExtScrollScreenParm* Info)
{
	TODO("!!!scrolling region!!!");

	BOOL lbTrueColor = ExtCheckBuffers(Info->ConsoleOutput);
	UNREFERENCED_PARAMETER(lbTrueColor);

	HANDLE h = Info->ConsoleOutput;
	CONSOLE_SCREEN_BUFFER_INFO csbi = {};
	SMALL_RECT srWork = {};
	if (!ExtGetBufferInfo(h, csbi, srWork))
		return FALSE;

	BOOL lbRc = TRUE;
	CHAR_INFO Buf[200];
	CHAR_INFO* pBuf = (csbi.dwSize.X <= (int)countof(Buf)) ? Buf : (CHAR_INFO*)malloc(csbi.dwSize.X*sizeof(*pBuf));
	COORD crSize = {csbi.dwSize.X,1};
	COORD cr0 = {};
	SMALL_RECT rcRgn = {0,0,csbi.dwSize.X-1,0};
	int SrcLineTop = 0, SrcLineBottom = 0;
	int nDir = (int)Info->Dir;

	if (Info->Flags & essf_Region)
	{
		_ASSERTEX(Info->Region.left==0 && Info->Region.right==0); // Not used yet!
		if ((Info->Region.top < 0) || (Info->Region.bottom < Info->Region.top))
		{
			_ASSERTEX(Info->Region.top>=0 && Info->Region.bottom>=Info->Region.top); // Must not be empty!
			SetLastError(E_INVALIDARG);
			return FALSE;
		}

		if (nDir < 0)
		{
			 SrcLineTop = Info->Region.top + (-nDir); SrcLineBottom = Info->Region.bottom;
		}
		else if (nDir > 0)
		{
			SrcLineTop = Info->Region.top; SrcLineBottom = Info->Region.bottom - nDir;
		}
	}
	else
	{
		if (nDir < 0)
		{
			 SrcLineTop = srWork.Top + (-nDir); SrcLineBottom = csbi.dwSize.Y - 1;
		}
		else if (nDir > 0)
		{
			SrcLineTop = srWork.Top; SrcLineBottom = csbi.dwSize.Y - nDir - 1;
		}
	}

	if (!pBuf)
	{
		return FALSE;
	}

	if (SrcLineBottom < SrcLineTop)
	{
		_ASSERTEX(SrcLineBottom >= SrcLineTop);
		return FALSE;
	}


	// ������ TrueColor ������
	SHORT nWindowWidth  = srWork.Right - srWork.Left + 1;
	SHORT nWindowHeight = srWork.Bottom - srWork.Top + 1;

	AnnotationInfo* pTrueColorStart = (AnnotationInfo*)(gpTrueColor ? (((LPBYTE)gpTrueColor) + gpTrueColor->struct_size) : NULL); //-V104
	DEBUGTEST(AnnotationInfo* pTrueColorEnd = pTrueColorStart ? (pTrueColorStart + gpTrueColor->bufferSize) : NULL); //-V104

	if (Info->Flags & essf_Current)
	{
		ExtAttributesParm DefClr = {sizeof(DefClr), h};
		ExtGetAttributes(&DefClr);
		Info->FillAttr = DefClr.Attributes;
	}

	if (Info->Flags & essf_Pad)
	{
		_ASSERTE(Info->Dir == -1);
		nDir = -1;

		// ��������� ��������? ���� ����� � �� �����. ��� ��� "�����������" ����� �� �������� ������� �������.

		/*
		COORD crFrom = {csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y};
		nCount = csbi.dwSize.X - csbi.dwCursorPosition.X;

		ExtFillOutputParm f = {sizeof(f), efof_Attribute|efof_Character, Info->ConsoleOutput,
			Info->FillAttr, Info->FillChar, crFrom, nCount};
		lbRc = ExtFillOutput(&f);

		if (lbRc)
			nDir = -1;
		*/
	}


	// Go
	if (nDir < 0)
	{
		// Scroll whole page up by n (default 1) lines. New lines are added at the bottom.
		if (gpTrueColor)
		{
			int nMaxCell = min(nWindowWidth * nWindowHeight,gpTrueColor->bufferSize);
			int nMaxRows = nMaxCell / nWindowWidth;
			_ASSERTEX(SrcLineTop >= srWork.Top);
			int nY1 = min(max((SrcLineTop - srWork.Top),0),nMaxRows);
			int nY2 = min(max((SrcLineBottom - srWork.Top),0),nMaxRows);
			int nRows = nY2 - nY1 + 1;

			if (nRows > (-nDir))
			{
				if (nY1 > (-nDir))
				{
					pTrueColorStart += ((nY1+nDir) * nWindowWidth);
					_ASSERTEX((pTrueColorStart+(-nDir*nWindowWidth)+(nRows*nWindowWidth))<pTrueColorEnd);
				}

				WARNING("OPTIMIZE: �� ����� ������� ����������� ������ ������. ��� ��������� � ������ - � �� ���������.");
				memmove(pTrueColorStart, pTrueColorStart+(-nDir*nWindowWidth),
					(nRows * nWindowWidth * sizeof(*pTrueColorStart)));

				if (Info->Flags & essf_ExtOnly)
				{
					TODO("��� ��������");
					AnnotationInfo AIInfo = {};
					for (int i = (nWindowHeight+nDir)*nWindowWidth; i < nMaxCell; i++)
					{
						pTrueColorStart[i] = AIInfo;
					}
				}
			}
			else
			{
				_ASSERTEX(nRows > (-nDir));
			}
		}

		if (!(Info->Flags & essf_ExtOnly))
		{
			SMALL_RECT rcSrc = {0, SrcLineTop, csbi.dwSize.X-1, SrcLineBottom};
			COORD crDst = {0, SrcLineTop + nDir/*<0*/};
			AnnotationInfo t = {};
			CHAR_INFO cFill = {{Info->FillChar}};
			ExtPrepareColor(Info->FillAttr, t, cFill.Attributes);
			
			ScrollConsoleScreenBuffer(Info->ConsoleOutput, &rcSrc, NULL, crDst, &cFill);

			//for (SHORT y = 0, y1 = -nDir; y1 < csbi.dwSize.Y; y++, y1++)
			//{
			//	rcRgn.Top = rcRgn.Bottom = y1;
			//	if (ReadConsoleOutputW(Info->ConsoleOutput, pBuf, crSize, cr0, &rcRgn))
			//	{
			//		rcRgn.Top = rcRgn.Bottom = y;
			//		WriteConsoleOutputW(Info->ConsoleOutput, pBuf, crSize, cr0, &rcRgn);
			//	}
			//}

			if (nDir < 0)
			{
				cr0.Y = max(0,(csbi.dwSize.Y - 1 + nDir));
				int nLines = csbi.dwSize.Y - cr0.Y;
				ExtFillOutputParm f = {sizeof(f), efof_Attribute|efof_Character, Info->ConsoleOutput,
					Info->FillAttr, Info->FillChar, cr0, csbi.dwSize.X * nLines};
				ExtFillOutput(&f);
				//FillConsoleOutputAttribute(Info->ConsoleOutput, GetDefaultTextAttr(), csbi.dwSize.X, cr0, &nCount);
				//FillConsoleOutputCharacter(Info->ConsoleOutput, L' ', csbi.dwSize.X, cr0, &nCount);
				//++nDir;
			}
		}
	}
	else if (nDir > 0)
	{
		// Scroll whole page down by n (default 1) lines. New lines are added at the top.
		if (gpTrueColor)
		{
			int nMaxCell = min(nWindowWidth * nWindowHeight,gpTrueColor->bufferSize);
			int nMaxRows = nMaxCell / nWindowWidth;
			_ASSERTEX(SrcLineTop >= srWork.Top);
			int nY1 = min(max((SrcLineTop - srWork.Top),0),nMaxRows);
			int nY2 = min(max((SrcLineBottom - srWork.Top),0),nMaxRows);
			int nRows = nY2 - nY1 + 1;

			if (nRows > nDir)
			{
				if (nY1 > nDir)
				{
					pTrueColorStart += ((nY1 - nDir) * nWindowWidth);
					_ASSERTEX((pTrueColorStart+(nDir*nWindowWidth)+(nRows*nWindowWidth))<pTrueColorEnd);
				}

				WARNING("OPTIMIZE: �� ����� ������� ����������� ������ ������. ��� ��������� � ������ - � �� ���������.");
				memmove(pTrueColorStart+(nDir*nWindowWidth), pTrueColorStart,
					(nRows * nWindowWidth * sizeof(*pTrueColorStart)));
			
				if (Info->Flags & essf_ExtOnly)
				{
					TODO("��� ��������");
					AnnotationInfo AIInfo = {};
					for (int i = (nDir*nWindowWidth)-1; i > 0; i--)
					{
						pTrueColorStart[i] = AIInfo;
					}
				}
			}
		}

		if (!(Info->Flags & essf_ExtOnly))
		{
			SMALL_RECT rcSrc = {0, SrcLineTop, csbi.dwSize.X-1, SrcLineBottom};
			COORD crDst = {0, SrcLineTop + nDir/*>0*/};
			AnnotationInfo t = {};
			CHAR_INFO cFill = {{Info->FillChar}};
			ExtPrepareColor(Info->FillAttr, t, cFill.Attributes);
			
			ScrollConsoleScreenBuffer(Info->ConsoleOutput, &rcSrc, NULL, crDst, &cFill);

			//for (SHORT y = csbi.dwSize.Y - nDir - 1, y1 = csbi.dwSize.Y - 1; y >= 0; y--, y1--)
			//{
			//	rcRgn.Top = rcRgn.Bottom = y;
			//	if (ReadConsoleOutputW(Info->ConsoleOutput, pBuf, crSize, cr0, &rcRgn))
			//	{
			//		rcRgn.Top = rcRgn.Bottom = y1;
			//		WriteConsoleOutputW(Info->ConsoleOutput, pBuf, crSize, cr0, &rcRgn);
			//	}
			//}

			if (nDir > 0)
			{
				cr0.Y = 0;
				ExtFillOutputParm f = {sizeof(f), efof_Attribute|efof_Character, Info->ConsoleOutput,
					Info->FillAttr, Info->FillChar, cr0, csbi.dwSize.X * nDir};
				ExtFillOutput(&f);
				//FillConsoleOutputAttribute(Info->ConsoleOutput, GetDefaultTextAttr(), csbi.dwSize.X, cr0, &nCount);
				//FillConsoleOutputCharacter(Info->ConsoleOutput, L' ', csbi.dwSize.X, cr0, &nCount);
				//--nDir;
			}
		}
	}

	// ��������� ������ � AnnotationInfo
	if (gpTrueColor)
	{
		gpTrueColor->flushCounter++;
	}

	// Commit �� �����
	if (Info->Flags & essf_Commit)
	{
		ExtCommitParm c = {sizeof(c), Info->ConsoleOutput};
		ExtCommit(&c);
	}

	if (pBuf != Buf)
		free(pBuf);

	return lbRc;
}



BOOL ExtCommit(ExtCommitParm* Info)
{
	if (!Info)
	{
		_ASSERTE(Info!=NULL);
		SetLastError(E_INVALIDARG);
		return FALSE;
	}

	TODO("ExtCommit: Info->ConsoleOutput");

	// ���� ����� �� ��� ������ - �� � ������������� ������
	if (gpTrueColor != NULL)
	{
		if (gpTrueColor->locked)
		{
			gpTrueColor->flushCounter++;
			gpTrueColor->locked = FALSE;
		}
	}

	return TRUE;
}
