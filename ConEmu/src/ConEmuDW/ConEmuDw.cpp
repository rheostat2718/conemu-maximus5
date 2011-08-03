
#include <windows.h>
#include <crtdbg.h>

#pragma comment(lib, "Comdlg32.lib")

#define MASSERT_HEADER_DEFINED
#define MEMORY_HEADER_DEFINED

#include "../common/pluginW1900.hpp"
#include "../common/ConsoleAnnotation.h"
#include "../common/common.hpp"
#include "../ConEmu/version.h"
#include "ConEmuDw.h"
#include "resource.h"

#define MSG_TITLE "ConEmu writer"
#define MSG_INVALID_CONEMU_VER "Unsupported ConEmu version detected!\nRequired version: " CONEMUVERS "\nConsole writer'll works in 4bit mode"
#define MSG_TRUEMOD_DISABLED   "�Colorer TrueMod support� is not checked in the ConEmu settings\nConsole writer'll works in 4bit mode"
#define MSG_NO_TRUEMOD_BUFFER  "TrueMod support not enabled in the ConEmu settings\nConsole writer'll works in 4bit mode"

//TODO: RGB32???
#if 0
	#define IsTransparent(C) (((C) & 0xFF000000) != 0)
	#define SetTransparent(T) ((T) ? 0xFF000000 : 0)
#else
	#define IsTransparent(C) (((C) & 0xFF000000) == 0)
	#define SetTransparent(T) ((T) ? 0 : 0xFF000000)
#endif

#define MAX_READ_BUF 16384

#if defined(__GNUC__)
extern "C" {
	BOOL WINAPI DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved);
};
#endif


HMODULE ghOurModule = NULL; // ConEmuDw.dll
HWND    ghConWnd = NULL; // ���������������� � CheckBuffers()

AnnotationHeader* gpTrueColor = NULL;
HANDLE ghTrueColor = NULL;
BOOL CheckBuffers();
void CloseBuffers();


BOOL WINAPI DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch(ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			{
				//HeapInitialize();
				
				ghOurModule = (HMODULE)hModule;
				
			}
			break;
		
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		
		case DLL_PROCESS_DETACH:
			{
				CloseBuffers();
			}
			break;
	}

	return TRUE;
}

#if defined(CRTSTARTUP)
extern "C" {
	BOOL WINAPI _DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
};

BOOL WINAPI _DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
	DllMain(hDll, dwReason, lpReserved);
	return TRUE;
}
#endif




BOOL CheckBuffers()
{
	//TODO: ���������, �� ��������� �� HWND �������?
	HWND hCon = GetConsoleWindow();
	if (!hCon)
	{
		CloseBuffers();
		SetLastError(E_HANDLE);
		return FALSE;
	}
	if (hCon != ghConWnd)
	{
		ghConWnd = hCon;
		CloseBuffers();
		
		//TODO: ���� �������� "��-�������", ����� ����� TrueColor. ����������, �� �� ���������
		wchar_t szMapName[128];
		wsprintf(szMapName, AnnotationShareName, (DWORD)sizeof(AnnotationInfo), (DWORD)hCon);
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
	if (gpTrueColor && !gpTrueColor->locked)
	{
		//TODO: �������� ����� ���������� �����?
		gpTrueColor->locked = TRUE;
	}
	return (gpTrueColor!=NULL);
}

void CloseBuffers()
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

__inline int Max(int i1, int i2)
{
	return (i1 > i2) ? i1 : i2;
}

WORD Color2FgIndex(COLORREF Color)
{
	static int LastColor, LastIndex;
	if (LastColor == Color)
		return LastIndex;
	
	int B = (Color & 0xFF0000) >> 16;
	int G = (Color & 0xFF00) >> 8;
	int R = (Color & 0xFF);
	int nMax = Max(B,Max(R,G));
	
	int Index =
		(((B+32) > nMax) ? 1 : 0) |
		(((G+32) > nMax) ? 2 : 0) |
		(((R+32) > nMax) ? 4 : 0);
	LastColor = Color;
	LastIndex = Index;
	return Index;
}

WORD Color2BgIndex(COLORREF Color)
{
	static int LastColor, LastIndex;
	if (LastColor == Color)
		return LastIndex;
	
	int B = (Color & 0xFF0000) >> 16;
	int G = (Color & 0xFF00) >> 8;
	int R = (Color & 0xFF);
	int nMax = Max(B,Max(R,G));

	int Index =
		(((B+32) > nMax) ? 16 : 0) |
		(((G+32) > nMax) ? 32 : 0) |
		(((R+32) > nMax) ? 64 : 0);
	LastColor = Color;
	LastIndex = Index;
	return Index;
}

//TODO: ������ ����� �����������.
//TODO: ��� �� "����� �������� ������", ��� "���� �� ���������". ��, ��� ������������� 
//TODO: "Screen Text" � "Screen Background" � ��������� �������. ��, ��� ������� ��������
//TODO: color � cmd.exe. ��, ��� ����� ������������ ���� � ������� ������ ������ 
//TODO: �� printf/std::cout/WriteConsole, ��� ������ �������� �����.
BOOL WINAPI GetTextAttributes(FarColor* Attributes)
{
	BOOL lbTrueColor = CheckBuffers();
	
	CONSOLE_SCREEN_BUFFER_INFO csbi = {};
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	if (!GetConsoleScreenBufferInfo(h, &csbi))
		return FALSE;

	//TODO: ������-�� ����� �� ���������� ��, ��� ������ � SetTextAttributes?
	Attributes->Flags = FCF_FG_4BIT|FCF_BG_4BIT;
	Attributes->ForegroundColor = (csbi.wAttributes & 0xF);
	Attributes->BackgroundColor = (csbi.wAttributes & 0xF0) >> 4;
	
	return TRUE;
	
	//WORD nDefReadBuf[1024];
	//WORD *pnReadAttr = NULL;
	//int nBufWidth  = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	//int nBufHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	//int nBufCount  = nBufWidth*nBufHeight;
	//if (nBufWidth <= ARRAYSIZE(nDefReadBuf))
	//	pnReadAttr = nDefReadBuf;
	//else
	//	pnReadAttr = (WORD*)calloc(nBufCount,sizeof(*pnReadAttr));
	//if (!pnReadAttr)
	//{
	//	SetLastError(E_OUTOFMEMORY);
	//	return FALSE;
	//}
	//
	//BOOL lbRc = TRUE;
	//COORD cr = {csbi.srWindow.Left, csbi.srWindow.Top};
	//DWORD nRead;
//
	//AnnotationInfo* pTrueColor = (AnnotationInfo*)(gpTrueColor ? (((LPBYTE)gpTrueColor) + gpTrueColor->struct_size) : NULL);
	//AnnotationInfo* pTrueColorEnd = pTrueColor ? (pTrueColor + gpTrueColor->bufferSize) : NULL;
//
	//for (;cr.Y <= csbi.srWindow.Bottom;cr.Y++)
	//{
	//	BOOL lbRead = ReadConsoleOutputAttribute(h, pnReadAttr, nBufWidth, cr, &nRead) && (nRead == nBufWidth);
//
	//	FarColor clr = {};
	//	WORD* pn = pnReadAttr;
	//	for (int X = 0; X < nBufWidth; X++, pn++)
	//	{
	//		clr.Flags = 0;
//
	//		if (pTrueColor && pTrueColor >= pTrueColorEnd)
	//		{
	//			_ASSERTE(pTrueColor && pTrueColor < pTrueColorEnd);
	//			pTrueColor = NULL; // ���������� ����� �������� �������������
	//		}
	//		
	//		if (pTrueColor)
	//		{
	//			DWORD Style = pTrueColor->style;
	//			if (Style & AI_STYLE_BOLD)
	//				clr.Flags |= FCF_FG_BOLD;
	//			if (Style & AI_STYLE_ITALIC)
	//				clr.Flags |= FCF_FG_ITALIC;
	//			if (Style & AI_STYLE_UNDERLINE)
	//				clr.Flags |= FCF_FG_UNDERLINE;
	//		}
//
	//		if (pTrueColor && pTrueColor->fg_valid)
	//		{
	//			clr.ForegroundColor = pTrueColor->fg_color;
	//		}
	//		else
	//		{
	//			clr.Flags |= FCF_FG_4BIT;
	//			clr.ForegroundColor = lbRead ? ((*pn) & 0xF) : 7;
	//		}
//
	//		if (pTrueColor && pTrueColor->bk_valid)
	//		{
	//			clr.BackgroundColor = pTrueColor->bk_color;
	//		}
	//		else
	//		{
	//			clr.Flags |= FCF_BG_4BIT;
	//			clr.BackgroundColor = lbRead ? (((*pn) & 0xF0) >> 4) : 0;
	//		}
//
	//		//*(Attributes++) = clr; -- <G|S>etTextAttributes ������ ������� ��������� �� ���� FarColor.
	//		if (pTrueColor)
	//			pTrueColor++;
	//	}
	//}
	//
	//if (pnReadAttr != nDefReadBuf)
	//	free(pnReadAttr);
	
	//return lbRc;
}

//TODO: ������ ����� �����������.
//TODO: ��� �� "����� �������� ������", ��� "���� �� ���������". ��, ��� ������������� 
//TODO: "Screen Text" � "Screen Background" � ��������� �������. ��, ��� ������� ��������
//TODO: color � cmd.exe. ��, ��� ����� ������������ ���� � ������� ������ ������ 
//TODO: �� printf/std::cout/WriteConsole, ��� ������ �������� �����.
BOOL WINAPI SetTextAttributes(const FarColor* Attributes)
{
	BOOL lbTrueColor = CheckBuffers();

	CONSOLE_SCREEN_BUFFER_INFO csbi = {};
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	if (!GetConsoleScreenBufferInfo(h, &csbi))
		return FALSE;
	
	WORD nDefWriteBuf[1024];
	WORD *pnWriteAttr = NULL;
	int nBufWidth  = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	int nBufHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	int nBufCount  = nBufWidth*nBufHeight;
	if (nBufWidth <= ARRAYSIZE(nDefWriteBuf))
		pnWriteAttr = nDefWriteBuf;
	else
		pnWriteAttr = (WORD*)calloc(nBufCount,sizeof(*pnWriteAttr));
	if (!pnWriteAttr)
	{
		SetLastError(E_OUTOFMEMORY);
		return FALSE;
	}
	
	BOOL lbRc = TRUE;
	COORD cr = {csbi.srWindow.Left, csbi.srWindow.Top};
	DWORD nWritten;
	
	AnnotationInfo* pTrueColor = (AnnotationInfo*)(gpTrueColor ? (((LPBYTE)gpTrueColor) + gpTrueColor->struct_size) : NULL);
	AnnotationInfo* pTrueColorEnd = pTrueColor ? (pTrueColor + gpTrueColor->bufferSize) : NULL;

	// <G|S>etTextAttributes ������ ������� ��������� �� ���� FarColor.
	AnnotationInfo t = {};
	WORD n = 0, f = 0;
	unsigned __int64 Flags = Attributes->Flags;

	if (Flags & FCF_FG_BOLD)
		f |= AI_STYLE_BOLD;
	if (Flags & FCF_FG_ITALIC)
		f |= AI_STYLE_ITALIC;
	if (Flags & FCF_FG_UNDERLINE)
		f |= AI_STYLE_UNDERLINE;
	t.style = f;

	if (Flags & FCF_FG_4BIT)
	{
		n |= (WORD)(Attributes->ForegroundColor & 0xF);
		t.fg_valid = FALSE;
	}
	else
	{
		//n |= 0x07;
		n |= Color2FgIndex(Attributes->ForegroundColor & 0x00FFFFFF);
		t.fg_color = Attributes->ForegroundColor & 0x00FFFFFF;
		t.fg_valid = TRUE;
	}

	if (Flags & FCF_BG_4BIT)
	{
		n |= (WORD)(Attributes->BackgroundColor & 0xF)<<4;
		t.bk_valid = FALSE;
	}
	else
	{
		n |= Color2BgIndex(Attributes->BackgroundColor & 0x00FFFFFF);
		t.bk_color = Attributes->BackgroundColor & 0x00FFFFFF;
		t.bk_valid = TRUE;
	}


	for (;cr.Y <= csbi.srWindow.Bottom;cr.Y++)
	{
		WORD* pn = pnWriteAttr;
		for (int X = 0; X < nBufWidth; X++, pn++)
		{
			if (pTrueColor && pTrueColor >= pTrueColorEnd)
			{
				_ASSERTE(pTrueColor && pTrueColor < pTrueColorEnd);
				pTrueColor = NULL; // ���������� ����� �������� �������������
			}

			// ���� � RealConsole
			*pn = n;

			// ���� � ConEmu (GUI)
			if (pTrueColor)
			{
				*(pTrueColor++) = t;
			}
		}
	
		if (!WriteConsoleOutputAttribute(h, pnWriteAttr, nBufWidth, cr, &nWritten) || (nWritten != nBufWidth))
			lbRc = FALSE;
	}
	
	SetConsoleTextAttribute(h, n);
	
	if (pnWriteAttr != nDefWriteBuf)
		free(pnWriteAttr);
	
	return lbRc;
}

BOOL WINAPI ClearExtraRegions(const FarColor* Color)
{
	//TODO: ���� �������� ����� ������ Annotation buffer (���������� �����)
	//TODO: ������� �� ����������� ������������� ������� ������� ������
	return SetTextAttributes(Color);
}

BOOL WINAPI ReadOutput(FAR_CHAR_INFO* Buffer, COORD BufferSize, COORD BufferCoord, SMALL_RECT* ReadRegion)
{
	BOOL lbTrueColor = CheckBuffers();
	/*
	struct FAR_CHAR_INFO
	{
		WCHAR Char;
		struct FarColor Attributes;
	};
	typedef struct _CHAR_INFO {
	  union {
	    WCHAR UnicodeChar;
	    CHAR AsciiChar;
	  } Char;
	  WORD  Attributes;
	}CHAR_INFO, *PCHAR_INFO;
	BOOL WINAPI ReadConsoleOutput(
	  __in     HANDLE hConsoleOutput,
	  __out    PCHAR_INFO lpBuffer,
	  __in     COORD dwBufferSize,
	  __in     COORD dwBufferCoord,
	  __inout  PSMALL_RECT lpReadRegion
	);
	*/

	CONSOLE_SCREEN_BUFFER_INFO csbi = {};
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	if (!GetConsoleScreenBufferInfo(h, &csbi))
		return FALSE;

	CHAR_INFO cDefReadBuf[1024];
	CHAR_INFO *pcReadBuf = NULL;
	int nWindowWidth  = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	if (BufferSize.X <= ARRAYSIZE(cDefReadBuf))
		pcReadBuf = cDefReadBuf;
	else
		pcReadBuf = (CHAR_INFO*)calloc(BufferSize.X,sizeof(*pcReadBuf));
	if (!pcReadBuf)
	{
		SetLastError(E_OUTOFMEMORY);
		return FALSE;
	}

	BOOL lbRc = TRUE;

	AnnotationInfo* pTrueColorStart = (AnnotationInfo*)(gpTrueColor ? (((LPBYTE)gpTrueColor) + gpTrueColor->struct_size) : NULL);
	AnnotationInfo* pTrueColorEnd = pTrueColorStart ? (pTrueColorStart + gpTrueColor->bufferSize) : NULL;
	AnnotationInfo* pTrueColorLine = (AnnotationInfo*)(pTrueColorStart ? (pTrueColorStart + nWindowWidth * (ReadRegion->Top - csbi.srWindow.Top)) : NULL);

	FAR_CHAR_INFO* pFarEnd = Buffer + BufferSize.X*BufferSize.Y;

	SMALL_RECT rcRead = *ReadRegion;
	COORD MyBufferSize = {BufferSize.X, 1};
	COORD MyBufferCoord = {BufferCoord.X, 0};
	for (rcRead.Top = ReadRegion->Top; rcRead.Top <= ReadRegion->Bottom; rcRead.Top++)
	{
		rcRead.Bottom = rcRead.Top;
		BOOL lbRead = ReadConsoleOutputW(h, pcReadBuf, MyBufferSize, MyBufferCoord, &rcRead);

		CHAR_INFO* pc = pcReadBuf + BufferCoord.X;
		FAR_CHAR_INFO* pFar = Buffer + (rcRead.Top - ReadRegion->Top + BufferCoord.Y)*BufferSize.X + BufferCoord.X;
		AnnotationInfo* pTrueColor = (pTrueColorLine && (pTrueColorLine >= pTrueColorStart)) ? (pTrueColorLine + ReadRegion->Left) : NULL;

		for (int X = rcRead.Left; X <= rcRead.Right; X++, pc++, pFar++)
		{
			if (pFar >= pFarEnd)
			{
				_ASSERTE(pFar < pFarEnd);
				break;
			}

			FAR_CHAR_INFO chr = {lbRead ? pc->Char.UnicodeChar : L' '};

			if (pTrueColor && pTrueColor >= pTrueColorEnd)
			{
				_ASSERTE(pTrueColor && pTrueColor < pTrueColorEnd);
				pTrueColor = NULL; // ���������� ����� �������� �������������
			}

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
				chr.Attributes.ForegroundColor = lbRead ? (pc->Attributes & 0xF) : 7;
			}

			if (pTrueColor && pTrueColor->bk_valid)
			{
				chr.Attributes.BackgroundColor = pTrueColor->bk_color;
			}
			else
			{
				chr.Attributes.Flags |= FCF_BG_4BIT;
				chr.Attributes.BackgroundColor = lbRead ? (pc->Attributes & 0xF) : 0;
			}

			*pFar = chr;

			if (pTrueColor)
				pTrueColor++;
		}

		if (pTrueColorLine)
			pTrueColorLine += nWindowWidth;
	}

	if (pcReadBuf != cDefReadBuf)
		free(pcReadBuf);

	return lbRc;
}

BOOL WINAPI WriteOutput(const FAR_CHAR_INFO* Buffer, COORD BufferSize, COORD BufferCoord, SMALL_RECT* WriteRegion)
{
	BOOL lbTrueColor = CheckBuffers();
	/*
	struct FAR_CHAR_INFO
	{
		WCHAR Char;
		struct FarColor Attributes;
	};
	typedef struct _CHAR_INFO {
	  union {
	    WCHAR UnicodeChar;
	    CHAR AsciiChar;
	  } Char;
	  WORD  Attributes;
	}CHAR_INFO, *PCHAR_INFO;
	BOOL WINAPI WriteConsoleOutput(
	  __in     HANDLE hConsoleOutput,
	  __in     const CHAR_INFO *lpBuffer,
	  __in     COORD dwBufferSize,
	  __in     COORD dwBufferCoord,
	  __inout  PSMALL_RECT lpWriteRegion
	);
	*/

	CONSOLE_SCREEN_BUFFER_INFO csbi = {};
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	if (!GetConsoleScreenBufferInfo(h, &csbi))
		return FALSE;

	CHAR_INFO cDefWriteBuf[1024];
	CHAR_INFO *pcWriteBuf = NULL;
	int nWindowWidth  = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	if (BufferSize.X <= ARRAYSIZE(cDefWriteBuf))
		pcWriteBuf = cDefWriteBuf;
	else
		pcWriteBuf = (CHAR_INFO*)calloc(BufferSize.X,sizeof(*pcWriteBuf));
	if (!pcWriteBuf)
	{
		SetLastError(E_OUTOFMEMORY);
		return FALSE;
	}

	BOOL lbRc = TRUE;

	AnnotationInfo* pTrueColorStart = (AnnotationInfo*)(gpTrueColor ? (((LPBYTE)gpTrueColor) + gpTrueColor->struct_size) : NULL);
	AnnotationInfo* pTrueColorEnd = pTrueColorStart ? (pTrueColorStart + gpTrueColor->bufferSize) : NULL;
	AnnotationInfo* pTrueColorLine = (AnnotationInfo*)(pTrueColorStart ? (pTrueColorStart + nWindowWidth * (WriteRegion->Top - csbi.srWindow.Top)) : NULL);

	SMALL_RECT rcWrite = *WriteRegion;
	COORD MyBufferSize = {BufferSize.X, 1};
	COORD MyBufferCoord = {BufferCoord.X, 0};
	for (rcWrite.Top = WriteRegion->Top; rcWrite.Top <= WriteRegion->Bottom; rcWrite.Top++)
	{
		rcWrite.Bottom = rcWrite.Top;

		CHAR_INFO* pc = pcWriteBuf + BufferCoord.X;
		const FAR_CHAR_INFO* pFar = Buffer + (rcWrite.Top - WriteRegion->Top + BufferCoord.Y)*BufferSize.X + BufferCoord.X;
		AnnotationInfo* pTrueColor = (pTrueColorLine && (pTrueColorLine >= pTrueColorStart)) ? (pTrueColorLine + WriteRegion->Left) : NULL;

		for (int X = rcWrite.Left; X <= rcWrite.Right; X++, pc++, pFar++)
		{
			pc->Char.UnicodeChar = pFar->Char;

			if (pTrueColor && pTrueColor >= pTrueColorEnd)
			{
				_ASSERTE(pTrueColor && pTrueColor < pTrueColorEnd);
				pTrueColor = NULL; // ���������� ����� �������� �������������
			}
			
			WORD n = 0, f = 0;
			unsigned __int64 Flags = pFar->Attributes.Flags;
			
			if (pTrueColor)
			{
				if (Flags & FCF_FG_BOLD)
					f |= AI_STYLE_BOLD;
				if (Flags & FCF_FG_ITALIC)
					f |= AI_STYLE_ITALIC;
				if (Flags & FCF_FG_UNDERLINE)
					f |= AI_STYLE_UNDERLINE;
				pTrueColor->style = f;
			}

			if (Flags & FCF_FG_4BIT)
			{
				n |= (WORD)(pFar->Attributes.ForegroundColor & 0xF);
				if (pTrueColor)
				{
					pTrueColor->fg_valid = FALSE;
				}
			}
			else
			{
				//n |= 0x07;
				n |= Color2FgIndex(pFar->Attributes.ForegroundColor & 0x00FFFFFF);
				if (pTrueColor)
				{
					pTrueColor->fg_color = pFar->Attributes.ForegroundColor & 0x00FFFFFF;
					pTrueColor->fg_valid = TRUE;
				}
			}
				
			if (Flags & FCF_BG_4BIT)
			{
				n |= (WORD)(pFar->Attributes.BackgroundColor & 0xF)<<4;
				if (pTrueColor)
				{
					pTrueColor->bk_valid = FALSE;
				}
			}
			else
			{
				n |= Color2BgIndex(pFar->Attributes.BackgroundColor & 0x00FFFFFF);
				if (pTrueColor)
				{
					pTrueColor->bk_color = pFar->Attributes.BackgroundColor & 0x00FFFFFF;
					pTrueColor->bk_valid = TRUE;
				}
			}
			
			
			
			pc->Attributes = n;

			if (pTrueColor)
				pTrueColor++;
		}
		
		if (!WriteConsoleOutputW(h, pcWriteBuf, MyBufferSize, MyBufferCoord, &rcWrite))
		{
			lbRc = FALSE;
		}

		if (pTrueColorLine)
			pTrueColorLine += nWindowWidth;
	}

	if (pcWriteBuf != cDefWriteBuf)
		free(pcWriteBuf);

	return lbRc;
}

BOOL WINAPI Commit()
{
	// ���� ����� �� ��� ������ - �� � ������������� ������
	if (gpTrueColor != NULL)
	{
		if (gpTrueColor->locked)
		{
			gpTrueColor->flushCounter++;
			gpTrueColor->locked = FALSE;
		}
	}
	return TRUE; //TODO: � ���� ����������-��?
}

struct ColorParam
{
	FarColor Color;
	BOOL b4bitmode;
	BOOL bCentered;
	BOOL bAddTransparent;
	COLORREF crCustom[16];
	HWND hConsole, hGUI;
	RECT rcParent;
	SMALL_RECT rcBuffer; // ������� ����� � �������
	HWND hDialog;

	BOOL bBold, bItalic, bUnderline;
	BOOL bBackTransparent, bForeTransparent;
	COLORREF crBackColor, crForeColor;
	HBRUSH hbrBackground;
	
	void CreateBrush()
	{
		if (hbrBackground)
			DeleteObject(hbrBackground);
		hbrBackground = CreateSolidBrush(bBackTransparent ? GetSysColor(COLOR_BTNFACE) : crBackColor);
	}

	void Far2Ref(const FarColor* p, BOOL Foreground, COLORREF* cr, BOOL* Transparent)
	{
		*cr = 0; *Transparent = FALSE;
		if (Foreground)
		{
			*Transparent = IsTransparent(p->ForegroundColor);
			UINT nClr = (p->ForegroundColor & 0x00FFFFFF);
			if (p->Flags & FCF_FG_4BIT)
			{
				if (nClr < 16)
				{
					*cr = crCustom[nClr];
				}
			}
			else //TODO: ��������� �����?
			{
				*cr = nClr;
			}
			bBold = (p->Flags & FCF_FG_BOLD) == FCF_FG_BOLD;
			bItalic = (p->Flags & FCF_FG_ITALIC) == FCF_FG_ITALIC;
			bUnderline = (p->Flags & FCF_FG_UNDERLINE) == FCF_FG_UNDERLINE;
		}
		else // Background
		{
			*Transparent = IsTransparent(p->BackgroundColor);
			UINT nClr = (p->BackgroundColor & 0x00FFFFFF);
			if (p->Flags & FCF_BG_4BIT)
			{
				if (nClr < 16)
				{
					*cr = crCustom[nClr];
				}
			}
			else //TODO: ��������� �����?
			{
				*cr = nClr;
			}
		}
	};
	void Ref2Far(BOOL Transparent, COLORREF cr, BOOL Foreground, FarColor* p)
	{
		int Color = (cr & 0x00FFFFFF);
		
		if (b4bitmode)
		{
			int Change = Foreground ? 7 : 0;
			for (int i = 0; i < ARRAYSIZE(crCustom); i++)
			{
				if (crCustom[i] == Color)
				{
					Change = i;
					break;
				}
			}
			Color = Change;
		}
		
		if (Foreground)
		{
			if (b4bitmode)
				p->Flags |= FCF_FG_4BIT;
			else
				p->Flags &= ~FCF_FG_4BIT; //TODO: ��������� �����?
			p->ForegroundColor = Color | SetTransparent(Transparent);

			if (bBold)
				p->Flags |= FCF_FG_BOLD;
			else
				p->Flags &= ~FCF_FG_BOLD;
			if (bItalic)
				p->Flags |= FCF_FG_ITALIC;
			else
				p->Flags &= ~FCF_FG_ITALIC;
			if (bUnderline)
				p->Flags |= FCF_FG_UNDERLINE;
			else
				p->Flags &= ~FCF_FG_UNDERLINE;
		}
		else // Background
		{
			if (b4bitmode)
				p->Flags |= FCF_BG_4BIT;
			else
				p->Flags &= ~FCF_BG_4BIT; //TODO: ��������� �����?
			p->BackgroundColor = Color | SetTransparent(Transparent);
		}
	};
};



INT_PTR CALLBACK ColorDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ColorParam* P = NULL;

	if (uMsg == WM_INITDIALOG)
	{
		P = (ColorParam*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		// Transparent
		CheckDlgButton(hwndDlg, IDC_FORE_TRANS, P->bForeTransparent ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_BACK_TRANS, P->bBackTransparent ? BST_CHECKED : BST_UNCHECKED);
		// Bold/Italic/Underline
		CheckDlgButton(hwndDlg, IDC_BOLD, P->bBold ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ITALIC, P->bItalic ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_UNDERLINE, P->bUnderline ? BST_CHECKED : BST_UNCHECKED);
		// ��������� ���� ���������
		SetDlgItemText(hwndDlg, IDC_TEXT, L"Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text Text");

		if (!P->bAddTransparent)
		{
			ShowWindow(GetDlgItem(hwndDlg, IDC_FORE_TRANS), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_BACK_TRANS), SW_HIDE);
			RECT rcText, rcCheck;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_TEXT), &rcText);
			GetWindowRect(GetDlgItem(hwndDlg, IDC_FORE_TRANS), &rcCheck);
			SetWindowPos(GetDlgItem(hwndDlg, IDC_TEXT), 0, 0,0,
				rcText.right-rcText.left, rcCheck.bottom-rcText.top-10, SWP_NOMOVE|SWP_NOZORDER);
		}
		

		SetFocus(GetDlgItem(hwndDlg, IDOK));
		
		RECT rcDlg; GetWindowRect(hwndDlg, &rcDlg);
		//TODO: ���� ��������� � ��������...
		//TODO: ��������� P->Center
		HWND hTop = HWND_TOP;
		if (GetWindowLong(P->hConsole, GWL_EXSTYLE) & WS_EX_TOPMOST)
			hTop = HWND_TOPMOST;
		if (P->bCentered)
		{
			SetWindowPos(hwndDlg, hTop,
				(P->rcParent.right+P->rcParent.left-(rcDlg.right-rcDlg.left))>>1,
				(P->rcParent.bottom+P->rcParent.top-(rcDlg.bottom-rcDlg.top))>>1,
				0, 0, SWP_NOSIZE|SWP_SHOWWINDOW);
		}
		else
		{
			// ��������� � ���������� {X=37,Y=2} (0based)
			int x = max(0,(38 - P->rcBuffer.Left));
			int xShift = (P->rcParent.right - P->rcParent.left + 1) / (P->rcBuffer.Right - P->rcBuffer.Left + 1) * x;
			if ((xShift + (rcDlg.right-rcDlg.left)) > P->rcParent.right)
				xShift = max(P->rcParent.left, (P->rcParent.right - (rcDlg.right-rcDlg.left)));
			int y = max(0,(2 - P->rcBuffer.Top));
			int yShift = (P->rcParent.bottom - P->rcParent.top + 1) / (P->rcBuffer.Bottom - P->rcBuffer.Top + 1) * y;
			if ((yShift + (rcDlg.bottom-rcDlg.top)) > P->rcParent.bottom)
				yShift = max(P->rcParent.top, (P->rcParent.bottom - (rcDlg.bottom-rcDlg.top)));
			yShift += 32; //TODO: ����, ���� ���
			SetWindowPos(hwndDlg, hTop,
				P->rcParent.left+xShift, P->rcParent.top+yShift,
				0, 0, SWP_NOSIZE|SWP_SHOWWINDOW);
		}
		P->hDialog = hwndDlg;
		return FALSE;
	}

	P = (ColorParam*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	if (!P)
	{
		// WM_SETFONT, ��������, ����� ������ ����� WM_INITDIALOG O_O
		return FALSE;
	}

	switch (uMsg)
	{
	case WM_CTLCOLORSTATIC:
		if (GetDlgCtrlID((HWND)lParam) == IDC_TEXT)
		{
			SetTextColor((HDC)wParam, P->bForeTransparent ? GetSysColor(COLOR_BTNFACE) : P->crForeColor);
			SetBkMode((HDC)wParam, TRANSPARENT);
			return (INT_PTR)P->hbrBackground;
		}
		break;
	case WM_COMMAND:
		switch (wParam)
		{
		case IDOK:
			EndDialog(hwndDlg, 1);
			return TRUE;
		case IDCANCEL:
			EndDialog(hwndDlg, 2);
			return TRUE;
		case IDC_FORE:
		case IDC_BACK:
			{
				CHOOSECOLOR clr = {sizeof(clr),
					hwndDlg, NULL,
					(wParam == IDC_FORE) ? P->crForeColor : P->crBackColor,
					P->crCustom,
					/*CC_FULLOPEN|*/CC_RGBINIT|CC_SOLIDCOLOR, //TODO: �����
				};
				if (ChooseColor(&clr))
				{
					if (wParam == IDC_FORE)
					{
						//P->bForeTransparent = FALSE; -- ����/���?
						//CheckDlgButton(hwndDlg, IDC_FORE_TRANS, BST_UNCHECKED);
						P->crForeColor = clr.rgbResult;
					}
					else
					{
						//P->bBackTransparent = FALSE; -- ����/���?
						//CheckDlgButton(hwndDlg, IDC_BACK_TRANS, BST_UNCHECKED);
						if (P->crBackColor != clr.rgbResult)
						{
							P->crBackColor = clr.rgbResult;
							P->CreateBrush();
						}
					}
					InvalidateRect(GetDlgItem(hwndDlg, IDC_TEXT), NULL, TRUE);
				}
			}
			break;
		case IDC_FORE_TRANS:
			{
				P->bForeTransparent = IsDlgButtonChecked(hwndDlg, IDC_FORE_TRANS)!=BST_UNCHECKED;
				InvalidateRect(GetDlgItem(hwndDlg, IDC_TEXT), NULL, TRUE);
			}
			break;
		case IDC_BACK_TRANS:
			{
				P->bBackTransparent = IsDlgButtonChecked(hwndDlg, IDC_BACK_TRANS)!=BST_UNCHECKED;
				P->CreateBrush();
				InvalidateRect(GetDlgItem(hwndDlg, IDC_TEXT), NULL, TRUE);
			}
			break;
		case IDC_BOLD:
			{
				P->bBold = IsDlgButtonChecked(hwndDlg, IDC_BOLD)!=BST_UNCHECKED;
				//TODO: ������� ����� ���������
			}
			break;
		case IDC_ITALIC:
			{
				P->bItalic = IsDlgButtonChecked(hwndDlg, IDC_ITALIC)!=BST_UNCHECKED;
				//TODO: ������� ����� ���������
			}
			break;
		case IDC_UNDERLINE:
			{
				P->bUnderline = IsDlgButtonChecked(hwndDlg, IDC_UNDERLINE)!=BST_UNCHECKED;
				//TODO: ������� ����� ���������
			}
			break;
		}
		break;
	}
	return FALSE;
}

DWORD WINAPI ColorDialogThread(LPVOID lpParameter)
{
	ColorParam *P = (ColorParam*)lpParameter;
	if (!P)
	{
		_ASSERTE(P!=NULL);
		return 100;
	}
	P->CreateBrush();

	// NULL � �������� ParentWindow ������ ��� ����������� ������������� � GUI,
	// � GUI ������������ ��������� ����� ����� ������� (�������), ����� �������,
	// �������� ������� �� �������� ���� GUI ����������� ��� ��������� �������, ��� �����.
	LRESULT lRc = DialogBoxParam(ghOurModule, MAKEINTRESOURCE(IDD_COLORS), NULL, ColorDialogProc, (LPARAM)P);

	if (P->hbrBackground)
		DeleteObject(P->hbrBackground);

	return (DWORD)lRc;
}

int  WINAPI GetColorDialog(FarColor* Color, BOOL Centered, BOOL AddTransparent)
{
	//TODO: �������� ������ (����) � ������� ������ ���� ���� � �������,
	//TODO: ������������ ��������� ������� (����/������)
	//TODO: + ��� (������������) ������ "Transparent" (� ��� ���� � ��� ������)
	//TODO: ChooseColor ������� �� ������� "&Background color" "&Text color"

	ColorParam Parm = {*Color, TRUE/*b4bitmode*/, Centered, AddTransparent,
		{0x00000000, 0x00800000, 0x00008000, 0x00808000, 0x00000080, 0x00800080, 0x00008080, 0x00c0c0c0,
		0x00808080, 0x00ff0000, 0x0000ff00, 0x00ffff00, 0x000000ff, 0x00ff00ff, 0x0000ffff, 0x00ffffff},
	};
	////TODO: BUGBUG. ��� ��������� ������ ����� ��� �������� Color ����������� 0-��
	//if (!Parm.Color.Flags && !Parm.Color.ForegroundColor && !Parm.Color.BackgroundColor && !Parm.Color.Reserved)
	//{
	//	Parm.Color.Flags = FCF_FG_4BIT|FCF_BG_4BIT;
	//	Parm.Color.ForegroundColor = 7 | SetTransparent(FALSE);
	//	Parm.Color.BackgroundColor = SetTransparent(FALSE);
	//}
	Parm.Far2Ref(&Parm.Color, TRUE, &Parm.crForeColor, &Parm.bForeTransparent);
	Parm.Far2Ref(&Parm.Color, FALSE, &Parm.crBackColor, &Parm.bBackTransparent);
	if (!AddTransparent)
	{
		Parm.bForeTransparent = Parm.bBackTransparent = FALSE;
	}
	
	//TODO: �������� �� ���������� ���� GUI
	Parm.hConsole = GetConsoleWindow();
	
	// ����� HWND GUI
	wchar_t szMapName[128];
	wsprintf(szMapName, CECONMAPNAME, (DWORD)Parm.hConsole);
	HANDLE hMap = OpenFileMapping(FILE_MAP_READ, FALSE, szMapName);
	if (hMap != NULL)
	{
		CESERVER_CONSOLE_MAPPING_HDR *pHdr = (CESERVER_CONSOLE_MAPPING_HDR*)MapViewOfFile(hMap, FILE_MAP_READ,0,0,0);
		if (pHdr)
		{
			if ((pHdr->cbSize < sizeof(*pHdr))
				|| (pHdr->nProtocolVersion != CESERVER_REQ_VER) 
				|| !IsWindow(pHdr->hConEmuRoot))
			{
				if ((pHdr->cbSize >= sizeof(*pHdr)) 
					&& pHdr->hConEmuRoot && IsWindow(pHdr->hConEmuRoot)
					&& IsWindowVisible(pHdr->hConEmuRoot))
				{
					// ��� ���-���� ����� ���� ���� ConEmu
					Parm.hGUI = pHdr->hConEmuRoot; //TODO: �������� �� DC?
				}
				MessageBoxA(NULL, MSG_INVALID_CONEMU_VER, MSG_TITLE, MB_ICONSTOP|MB_SYSTEMMODAL);
			}
			else
			{
				Parm.hGUI = pHdr->hConEmuRoot; //TODO: �������� �� DC?
				if (!pHdr->bUseTrueColor)
				{
					MessageBoxA(NULL, MSG_TRUEMOD_DISABLED, MSG_TITLE, MB_ICONSTOP|MB_SYSTEMMODAL);
				}
				else if (!CheckBuffers())
				{
					MessageBoxA(NULL, MSG_NO_TRUEMOD_BUFFER, MSG_TITLE, MB_ICONSTOP|MB_SYSTEMMODAL);
				}
				else
				{
					Parm.b4bitmode = FALSE;
				}
			}
			UnmapViewOfFile(pHdr);
		}
		CloseHandle(hMap);
	}
	
	if (!Parm.hGUI && !IsWindowVisible(Parm.hConsole))
	{
		SystemParametersInfo(SPI_GETWORKAREA, 0, &Parm.rcParent, 0);
	}
	else
	{
		GetClientRect(Parm.hGUI ? Parm.hGUI : Parm.hConsole, &Parm.rcParent);
		MapWindowPoints(Parm.hGUI ? Parm.hGUI : Parm.hConsole, NULL, (POINT*)&Parm.rcParent, 2);
	}
	
	CONSOLE_SCREEN_BUFFER_INFO csbi = {};
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(h, &csbi);
	Parm.rcBuffer = csbi.srWindow;

	// ��������� ����, ����� 1) �� ������� � ��������; 2) �� ����� ������� � ������������ ��������� � ������������� GDI
	DWORD dwThreadID = 0;
	HANDLE hThread = CreateThread(NULL, 0, ColorDialogThread, &Parm, 0, &dwThreadID);
	if (!hThread)
		return -1;

	while (WaitForSingleObject(hThread, 100) == WAIT_TIMEOUT)
	{
		if (Parm.hDialog)
		{
			SetForegroundWindow(Parm.hDialog);
			break;
		}
	}

	//TODO: �������� ����� ����� �������� �����-�� ����������, 
	//TODO: ����� �� ������� ����� ���� ���������� GUI ������
	WaitForSingleObject(hThread, INFINITE);
	
	DWORD nRc = 0;
	GetExitCodeThread(hThread, &nRc);
	
	if (nRc == 1)
	{
		Parm.Ref2Far(Parm.bForeTransparent, Parm.crForeColor, TRUE, Color);
		Parm.Ref2Far(Parm.bBackTransparent, Parm.crBackColor, FALSE, Color);
	}
	
	return (nRc == 1);
}
