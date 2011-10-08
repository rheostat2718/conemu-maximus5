
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


/*

Spacing chars
0020	SPACE
	* sometimes considered a control code
	* other space characters: 2000-200A
	x (no-break space - 00A0)
	x (zero width space - 200B)
	x (word joiner - 2060)
	x (ideographic space - 3000)
	x (zero width no-break space - FEFF)

200C    ZERO WIDTH NON-JOINER
200C    ZERO WIDTH NON-JOINER
200E    LEFT-TO-RIGHT MARK
2060    WORD JOINER
FEFF    ZERO WIDTH NO-BREAK SPACE


*/


/*
200E    LEFT-TO-RIGHT MARK    
    * commonly abbreviated LRM
200F    RIGHT-TO-LEFT MARK    
    * commonly abbreviated RLM
*/


#define SHOWDEBUGSTR
#ifdef _DEBUG
//#define SHOWDEBUGSTEPS
#endif

#include "Header.h"
#include <Tlhelp32.h>
#include "ScreenDump.h"
#include "VirtualConsole.h"
#include "RealConsole.h"
#include "ConEmu.h"
#include "Options.h"
#include "Background.h"
#include "ConEmuPipe.h"
#include "TabBar.h"


#ifdef _DEBUG
#define DEBUGDRAW_RCONPOS VK_SCROLL // -- ��� ���������� ScrollLock ���������� �������������, ��������������� ��������� ���� RealConsole
#define DEBUGDRAW_DIALOGS VK_CAPITAL // -- ��� ���������� Caps ���������� ��������������, ��������������� ��������� ��������
#define DEBUGDRAW_VERTICALS VK_SCROLL // -- ��� ���������� ScrollLock ���������� ��������������, �������������� ������ �������� (������� ������ ������������� �� �������)
#endif

#define DEBUGSTRDRAW(s) //DEBUGSTR(s)
#define DEBUGSTRCOORD(s) //DEBUGSTR(s)

// WARNING("�� ���������� ���� �� ������ �������");
WARNING("�� ���������� ������ ������� ��� �������� ������� �����");
WARNING("��� ������� ������ ������ ������ ����� '���������' �� ����� �� ����, �� ���������� ��������� ������");

WARNING("���� � ��������� ���������� ������ ����� ���������� ��� Alt-F7, Enter. ��������, ���� ����� ���");
// ������ �� �������������� ������ ������ ��������� - ������ ����� ������� ����������� ����������.
// ������ ������ �����������, �� ����� �� ������ "..." �����������

//PRAGMA_ERROR("��� ������� ���������� F:\\VCProject\\FarPlugin\\PPCReg\\compile.cmd - Enter � ������� �� ������");

WARNING("� ������ VCon ������� ����� BYTE[256] ��� �������� ������������ ������ (Ctrl,...,Up,PgDn,Add,� ��.");
WARNING("�������������� ����� �������� � ����� {VKEY,wchar_t=0}, � ������� ��������� ��������� wchar_t �� WM_CHAR/WM_SYSCHAR");
WARNING("��� WM_(SYS)CHAR �������� wchar_t � ������, � ������ ��������� VKEY");
WARNING("��� ������������ WM_KEYUP - �����(� ������) wchar_t �� ����� ������, ������ � ������� UP");
TODO("� ������������ - ��������� �������� isKeyDown, � ������� �����");
WARNING("��� ������������ �� ������ ������� (�� �������� � � �������� ������ ������� - ����������� ����� ���� ������� � ����� ���������) ��������� �������� caps, scroll, num");
WARNING("� ����� ���������� �������/������� ��������� ����� �� �� ���������� Ctrl/Shift/Alt");

WARNING("����� ����� ��������������� ���������� ������ ������� ���������� (OK), �� ����������� ���������� ������� �� �������� � GUI - ��� ������� ���������� ������� � ������ ������");


//������, ��� ���������, ������ �������, ���������� �����, � ��...

#define VCURSORWIDTH 2
#define HCURSORHEIGHT 2

#define Assert(V) if ((V)==FALSE) { wchar_t szAMsg[MAX_PATH*2]; _wsprintf(szAMsg, SKIPLEN(countof(szAMsg)) L"Assertion (%s) at\n%s:%i", _T(#V), _T(__FILE__), __LINE__); Box(szAMsg); }

#ifdef _DEBUG
//#undef HEAPVAL
#define HEAPVAL //HeapValidate(mh_Heap, 0, NULL);
#define CURSOR_ALWAYS_VISIBLE
#else
#define HEAPVAL
#endif

#define ISBGIMGCOLOR(a) (nBgImageColors & (1 << a))

#ifndef CONSOLE_SELECTION_NOT_EMPTY
#define CONSOLE_SELECTION_NOT_EMPTY     0x0002   // non-null select rectangle
#endif

#ifdef _DEBUG
#define DUMPDC(f) if (mb_DebugDumpDC) DumpImage(hDC, Width, Height, f);
#else
#define DUMPDC(f)
#endif

//#define isCharSpace(c) (c == ucSpace || c == ucNoBreakSpace || c == 0 || (c>=0x2000 && c<=0x200F) || c == 0x2060 || c == 0x3000 || c == 0xFEFF)
//TODO("������ ��� acute � ������ ���������");
//#define isCharNonSpacing(c) (c == 0xFEFF || (c>=0x2000 && c<=0x200F) || c == 0x2060 || c == 0x3000)
//bool __inline isCharSpace(wchar_t c)
//{
//	switch (c)
//	{
//	case ucSpace:
//	case ucNoBreakSpace:
//	case 0x2000: case 0x2001: case 0x2002: case 0x2003: case 0x2004: case 0x2005:
//	case 0x2006: case 0x2007: case 0x2008: case 0x2009: case 0x200A:
//	200A
//	}
//}



CVirtualConsole::PARTBRUSHES CVirtualConsole::m_PartBrushes[MAX_COUNT_PART_BRUSHES] = {{0}};
char CVirtualConsole::mc_Uni2Oem[0x10000];
// MAX_SPACES == 0x400
wchar_t CVirtualConsole::ms_Spaces[MAX_SPACES];
wchar_t CVirtualConsole::ms_HorzDbl[MAX_SPACES];
wchar_t CVirtualConsole::ms_HorzSingl[MAX_SPACES];
HMENU CVirtualConsole::mh_PopupMenu = NULL;
HMENU CVirtualConsole::mh_DebugPopup = NULL;
HMENU CVirtualConsole::mh_EditPopup = NULL;

CVirtualConsole* CVirtualConsole::CreateVCon(RConStartArgs *args)
{
	CVirtualConsole* pCon = new CVirtualConsole();

	if (!pCon->mp_RCon->PreCreate(args))
	{
		delete pCon;
		return NULL;
	}

	return pCon;
}

CVirtualConsole::CVirtualConsole(/*HANDLE hConsoleOutput*/)
{
	mh_WndDC = NULL;
	//CreateView();
	mp_RCon = NULL; //new CRealConsole(this);
	gpConEmu->OnVConCreated(this);
	WARNING("��������������� ������ ����");
	SIZE_T nMinHeapSize = (1000 + (200 * 90 * 2) * 6 + MAX_PATH*2)*2 + 210*sizeof(*TextParts);
	mh_Heap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, nMinHeapSize, 0);
	cinf.dwSize = 15; cinf.bVisible = TRUE;
	ZeroStruct(winSize); ZeroStruct(coord);
	TextLen = 0;
	mb_RequiredForceUpdate = true;
	mb_LastFadeFlag = false;
	mn_LastBitsPixel = 0;
	mp_BkImgData = NULL; mn_BkImgDataMax = 0; mb_BkImgChanged = FALSE; mb_BkImgExist = /*mb_BkImgDelete =*/ FALSE;
	mp_BkEmfData = NULL; mn_BkEmfDataMax = 0; mb_BkEmfChanged = FALSE;
	mcs_BkImgData = NULL;
	mn_BkImgWidth = mn_BkImgHeight = 0;
	_ASSERTE(sizeof(mh_FontByIndex) == (sizeof(gpSet->mh_Font)+sizeof(mh_FontByIndex[0])));
	// mh_FontByIndex[MAX_FONT_STYLES] // ��������������� ��� 'Unicode CharMap'
	memmove(mh_FontByIndex, gpSet->mh_Font, MAX_FONT_STYLES*sizeof(mh_FontByIndex[0])); //-V512
	mh_UCharMapFont = NULL; ms_LastUCharMapFont[0] = 0;
	mh_FontByIndex[MAX_FONT_STYLES] = NULL; // ��������������� ��� 'Unicode CharMap'
	memset(&TransparentInfo, 0, sizeof(TransparentInfo));
	isFade = false; isForeground = true;
	mp_Colors = gpSet->GetColors();
	memset(&m_LeftPanelView, 0, sizeof(m_LeftPanelView));
	memset(&m_RightPanelView, 0, sizeof(m_RightPanelView));
	mn_ConEmuFadeMsg = /*mn_ConEmuSettingsMsg =*/ 0;
	// ��� ���������� ��������������� � TRUE, ���� ��� ��������� Redraw ����� �������� ������ �������
	mb_LeftPanelRedraw = mb_RightPanelRedraw = FALSE;
	mn_LastDialogsCount = 0;
	memset(mrc_LastDialogs, 0, sizeof(mrc_LastDialogs));
	//InitializeCriticalSection(&csDC); ncsTDC = 0;
	mb_PaintRequested = FALSE; mb_PaintLocked = FALSE;
	//InitializeCriticalSection(&csCON); ncsTCON = 0;
	mb_InPaintCall = FALSE;
	mb_InConsoleResize = FALSE;
	nFontHeight = gpSet->FontHeight();
	nFontWidth = gpSet->FontWidth();
	nFontCharSet = gpSet->FontCharSet();
	nLastNormalBack = 255;
	mb_ConDataChanged = FALSE;
	mh_TransparentRgn = NULL;
#ifdef _DEBUG
	mn_BackColorIdx = 2; // Green
#else
	mn_BackColorIdx = 0; // Black
#endif
	memset(&Cursor, 0, sizeof(Cursor));
	Cursor.nBlinkTime = GetCaretBlinkTime();
	TextWidth = TextHeight = Width = Height = nMaxTextWidth = nMaxTextHeight = 0;
	hDC = NULL; hBitmap = NULL; hBgDc = NULL; bgBmpSize.X = bgBmpSize.Y = 0;
	hSelectedFont = NULL; hOldFont = NULL;
	PointersInit();
	mb_IsForceUpdate = false;
	mb_InUpdate = FALSE;
	hBrush0 = NULL; hSelectedBrush = NULL; hOldBrush = NULL;
	isEditor = false;
	memset(&csbi, 0, sizeof(csbi)); mdw_LastError = 0;

	//nSpaceCount = 1000;
	//Spaces = (wchar_t*)Alloc(nSpaceCount,sizeof(wchar_t));
	//for (UINT i=0; i<nSpaceCount; i++) Spaces[i]=L' ';
	if (!ms_Spaces[0])
	{
		wmemset(ms_Spaces, L' ', MAX_SPACES);
		wmemset(ms_HorzDbl, ucBoxDblHorz, MAX_SPACES);
		wmemset(ms_HorzSingl, ucBoxSinglHorz, MAX_SPACES);
	}

	hOldBrush = NULL;
	hOldFont = NULL;

	if (gpSet->wndWidth)
		TextWidth = gpSet->wndWidth;

	if (gpSet->wndHeight)
		TextHeight = gpSet->wndHeight;

#ifdef _DEBUG
	mb_DebugDumpDC = FALSE;
#endif

	//if (gpSet->isShowBgImage)
	//    gpSet->LoadBackgroundFile(gpSet->sBgImage);

	if (gpSet->isAdvLogging != 3)
	{
		mpsz_LogScreen = NULL;
	}
	else
	{
		mn_LogScreenIdx = 0;
		//DWORD dwErr = 0;
		wchar_t szFile[MAX_PATH+64], *pszDot;
		wcscpy_c(szFile, gpConEmu->ms_ConEmuExe);

		if ((pszDot = wcsrchr(szFile, L'\\')) == NULL)
		{
			DisplayLastError(L"wcsrchr failed!");
			return; // ������
		}

		*pszDot = 0;
		mpsz_LogScreen = (wchar_t*)calloc(pszDot - szFile + 64, 2);
		wcscpy(mpsz_LogScreen, szFile);
		wcscpy(mpsz_LogScreen+_tcslen(mpsz_LogScreen), L"\\ConEmu-VCon-%i-%i.con"/*, RCon()->GetServerPID()*/);
	}

	CreateView();
	mp_RCon = new CRealConsole(this);

	// InitDC ����� ������������ - ������� ��� �� �������
}

CVirtualConsole::~CVirtualConsole()
{
	if (!gpConEmu->isMainThread())
	{
		//_ASSERTE(gpConEmu->isMainThread());
		MBoxA(L"~CVirtualConsole() called from background thread");
	}

	if (mp_RCon)
		mp_RCon->StopThread();

	HEAPVAL;

	if (hDC)
		{ DeleteDC(hDC); hDC = NULL; }

	if (hBitmap)
		{ DeleteObject(hBitmap); hBitmap = NULL; }

	PointersFree();

	// ���� ������ �� �����
	if (mh_Heap)
	{
		HeapDestroy(mh_Heap);
		mh_Heap = NULL;
	}

	if (mpsz_LogScreen)
	{
		//wchar_t szMask[MAX_PATH*2]; wcscpy(szMask, mpsz_LogScreen);
		//wchar_t *psz = wcsrchr(szMask, L'%');
		//if (psz) {
		//    wcscpy(psz, L"*.*");
		//    psz = wcsrchr(szMask, L'\\');
		//    if (psz) {
		//        psz++;
		//        WIN32_FIND_DATA fnd;
		//        HANDLE hFind = FindFirstFile(szMask, &fnd);
		//        if (hFind != INVALID_HANDLE_VALUE) {
		//            do {
		//                if ((fnd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0) {
		//                    wcscpy(psz, fnd.cFileName);
		//                    DeleteFile(szMask);
		//                }
		//            } while (FindNextFile(hFind, &fnd));
		//            FindClose(hFind);
		//        }
		//    }
		//}
		free(mpsz_LogScreen); mpsz_LogScreen = NULL;
	}

	//DeleteCriticalSection(&csDC);
	//DeleteCriticalSection(&csCON);

	if (mh_TransparentRgn)
	{
		DeleteObject(mh_TransparentRgn);
		mh_TransparentRgn = NULL;
	}

	if (mh_UCharMapFont)
	{
		DeleteObject(mh_UCharMapFont);
		mh_UCharMapFont = NULL;
	}

	if (mp_RCon)
	{
		delete mp_RCon;
		mp_RCon = NULL;
	}

	//if (mh_PopupMenu) { -- static �� ��� ����������
	//	DestroyMenu(mh_PopupMenu); mh_PopupMenu = NULL;
	//}
	MSectionLock SC;

	if (mcs_BkImgData)
		SC.Lock(mcs_BkImgData, TRUE);

	if (mp_BkImgData)
	{
		free(mp_BkImgData);
		mp_BkImgData = NULL;
		mn_BkImgDataMax = 0;
	}

	if (mp_BkEmfData)
	{
		free(mp_BkEmfData);
		mp_BkEmfData = NULL;
		mn_BkImgDataMax = 0;
	}
	
	if (mcs_BkImgData)
	{
		SC.Unlock();
		delete mcs_BkImgData;
		mcs_BkImgData = NULL;
	}

	//FreeBackgroundImage();
}

CRealConsole* CVirtualConsole::RCon()
{
	if (this)
		return mp_RCon;
	_ASSERTE(this!=NULL);
	return NULL;
}

HWND CVirtualConsole::GuiWnd()
{
	if (this && mp_RCon)
		return mp_RCon->GuiWnd();
	_ASSERTE(this!=NULL);
	return NULL;
}

bool CVirtualConsole::isVisible()
{
	return gpConEmu->isVisible(this);
}

int CVirtualConsole::GetTabCount()
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return 0;
	}
	if (!mp_RCon)
	{
		return 1;
	}
	return mp_RCon->GetTabCount();
}

int CVirtualConsole::GetActiveTab()
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return 0;
	}
	if (!mp_RCon)
	{
		return 0;
	}
	return mp_RCon->GetActiveTab();
}

BOOL CVirtualConsole::GetTab(int tabIdx, /*OUT*/ ConEmuTab* pTab)
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return FALSE;
	}
	if (!mp_RCon)
	{
		pTab->Pos = 0; pTab->Current = 1; pTab->Type = 1; pTab->Modified = 0;
		lstrcpyn(pTab->Name, gpConEmu->ms_ConEmuVer, countof(pTab->Name));
		return TRUE;
	}
	return mp_RCon->GetTab(tabIdx, pTab);
}

void CVirtualConsole::PointersInit()
{
	mb_PointersAllocated = false;
	mpsz_ConChar = mpsz_ConCharSave = NULL;
	mpn_ConAttrEx = mpn_ConAttrExSave = NULL;
	ConCharX = ConCharDX = NULL;
	tmpOem = NULL;
	TextParts = NULL;
	BgParts = NULL;
	PolyText = NULL;
	pbLineChanged = pbBackIsPic = NULL;
	pnBackRGB = NULL;
}

void CVirtualConsole::PointersFree()
{
#ifdef SafeFree
#undef SafeFree
#endif
#define SafeFree(f) if (f) { Free(f); f = NULL; }
	HEAPVAL;
	SafeFree(mpsz_ConChar);
	SafeFree(mpsz_ConCharSave);
	SafeFree(mpn_ConAttrEx);
	SafeFree(mpn_ConAttrExSave);
	SafeFree(ConCharX);
	SafeFree(ConCharDX);
	SafeFree(tmpOem);
	SafeFree(TextParts);
	SafeFree(BgParts);
	SafeFree(PolyText);
	SafeFree(pbLineChanged);
	SafeFree(pbBackIsPic);
	SafeFree(pnBackRGB);
	HEAPVAL;
	mb_PointersAllocated = false;
}

bool CVirtualConsole::PointersAlloc()
{
	mb_PointersAllocated = false;
#ifdef _DEBUG
	HeapValidate(mh_Heap, 0, NULL);
#endif
	uint nWidthHeight = (nMaxTextWidth * nMaxTextHeight);
#ifdef AllocArray
#undef AllocArray
#endif
#define AllocArray(p,t,c) p = (t*)Alloc(c,sizeof(t)); if (!p) return false;
	AllocArray(mpsz_ConChar, wchar_t, nWidthHeight+1); // ASCIIZ
	AllocArray(mpsz_ConCharSave, wchar_t, nWidthHeight+1); // ASCIIZ
	AllocArray(mpn_ConAttrEx, CharAttr, nWidthHeight);
	AllocArray(mpn_ConAttrExSave, CharAttr, nWidthHeight);
	AllocArray(ConCharX, DWORD, nWidthHeight);
	AllocArray(ConCharDX, DWORD, nMaxTextWidth); // ����� ��� TEXTPARTS
	AllocArray(tmpOem, char, nMaxTextWidth);
	AllocArray(TextParts, TEXTPARTS, (nMaxTextWidth + 1));
	AllocArray(BgParts, BGPARTS, nMaxTextWidth);
	AllocArray(PolyText, POLYTEXT, nMaxTextWidth);
	AllocArray(pbLineChanged, bool, nMaxTextHeight);
	AllocArray(pbBackIsPic, bool, nMaxTextHeight);
	AllocArray(pnBackRGB, COLORREF, nMaxTextHeight);
	HEAPVAL;
	return (mb_PointersAllocated = true);
}

void CVirtualConsole::PointersZero()
{
	uint nWidthHeight = (nMaxTextWidth * nMaxTextHeight);
	//100607: ���������� ��� �� �����. ��� ������� ����� �������������� � � ������ �����!
	HEAPVAL;
	//ZeroMemory(mpsz_ConChar, nWidthHeight*sizeof(*mpsz_ConChar));
	ZeroMemory(mpsz_ConCharSave, (nWidthHeight+1)*sizeof(*mpsz_ConChar)); // ASCIIZ
	HEAPVAL;
	//ZeroMemory(mpn_ConAttrEx, nWidthHeight*sizeof(*mpn_ConAttrEx));
	ZeroMemory(mpn_ConAttrExSave, nWidthHeight*sizeof(*mpn_ConAttrExSave));
	HEAPVAL;
	//ZeroMemory(ConCharX, nWidthHeight*sizeof(*ConCharX));
	ZeroMemory(ConCharDX, nMaxTextWidth*sizeof(*ConCharDX)); // ����� ��� TEXTPARTS
	HEAPVAL;
	ZeroMemory(tmpOem, nMaxTextWidth*sizeof(*tmpOem));
	HEAPVAL;
	ZeroMemory(TextParts, (nMaxTextWidth + 1)*sizeof(*TextParts));
	ZeroMemory(BgParts, nMaxTextWidth*sizeof(*BgParts));
	ZeroMemory(PolyText, nMaxTextWidth*sizeof(*PolyText));
	HEAPVAL;
	ZeroMemory(pbLineChanged, nMaxTextHeight*sizeof(*pbLineChanged));
	ZeroMemory(pbBackIsPic, nMaxTextHeight*sizeof(*pbBackIsPic));
	ZeroMemory(pnBackRGB, nMaxTextHeight*sizeof(*pnBackRGB));
	HEAPVAL;
}


// InitDC ���������� ������ ��� ����������� ���������� (�������, �����, � �.�.) ����� ����� ����������� DC � Bitmap
bool CVirtualConsole::InitDC(bool abNoDc, bool abNoWndResize)
{
	if (!this || !mp_RCon)
	{
		_ASSERTE(mp_RCon != NULL);
		return false;
	}

	MSectionLock SCON; SCON.Lock(&csCON);
	BOOL lbNeedCreateBuffers = FALSE;
	uint rTextWidth = mp_RCon->TextWidth();
	uint rTextHeight = mp_RCon->TextHeight();

	if (!rTextWidth || !rTextHeight)
	{
		WARNING("���� ��� ������ - ����� ������ DC Initialization failed, ��� �� �������...");
		Assert(mp_RCon->TextWidth() && mp_RCon->TextHeight());
		return false;
	}

#ifdef _DEBUG
	// ������ - ����������� � ������� ����
	//if (mp_RCon->IsConsoleThread())
	//{
	//    //_ASSERTE(!mp_RCon->IsConsoleThread());
	//}
	//if (TextHeight == 24)
	//    TextHeight = 24;
	_ASSERT(TextHeight >= 5);
#endif

	// ����� ����������� ������ ���� ��������� ��� ����������
	if (!mb_PointersAllocated ||
	        (nMaxTextWidth * nMaxTextHeight) < (rTextWidth * rTextHeight) ||
	        (nMaxTextWidth < rTextWidth) // � ��� ����� ��� TextParts & tmpOem
	  )
		lbNeedCreateBuffers = TRUE;

	if (lbNeedCreateBuffers && mb_PointersAllocated)
	{
		DEBUGSTRDRAW(L"Relocking SCON exclusively\n");
		SCON.RelockExclusive();
		DEBUGSTRDRAW(L"Relocking SCON exclusively (done)\n");
		PointersFree();
	}

	// InitDC ���������� ������ ��� ������ ������������� ��� ����� �������
	mb_IsForceUpdate = true; // ������� ���������� ������ Force
	TextWidth = rTextWidth;
	TextHeight = rTextHeight;

	if (lbNeedCreateBuffers)
	{
		// ���� ����������� ������ - �� � �������, ����� "��� ���� �� ������"...
		if (nMaxTextWidth < TextWidth)
			nMaxTextWidth = TextWidth+80;

		if (nMaxTextHeight < TextHeight)
			nMaxTextHeight = TextHeight+30;

		DEBUGSTRDRAW(L"Relocking SCON exclusively\n");
		SCON.RelockExclusive();
		DEBUGSTRDRAW(L"Relocking SCON exclusively (done)\n");

		if (!PointersAlloc())
		{
			WARNING("���� ��� ������ - ����� ������ DC Initialization failed, ��� �� �������...");
			return false;
		}
	}
	else
	{
		PointersZero();
	}

	SCON.Unlock();
	HEAPVAL
	hSelectedFont = NULL;

	// ��� ����� ����, ���� ��������� ����������� (debug) � ����� ���� ����� �� �����
	if (!abNoDc)
	{
		DEBUGSTRDRAW(L"*** Recreate DC\n");
		MSectionLock SDC;
		// ���� � ���� ���� ��� ������������ - ������ �� ���������
		SDC.Lock(&csDC, TRUE, 200); // �� �� ��������, ����� �� ������� ���������

		if (hDC)
			{ DeleteDC(hDC); hDC = NULL; }

		if (hBitmap)
			{ DeleteObject(hBitmap); hBitmap = NULL; }

		const HDC hScreenDC = GetDC(0);

		if ((hDC = CreateCompatibleDC(hScreenDC)) != NULL)
		{
			Assert(gpSet->FontWidth() && gpSet->FontHeight());
			nFontHeight = gpSet->FontHeight();
			nFontWidth = gpSet->FontWidth();
			nFontCharSet = gpSet->FontCharSet();
#ifdef _DEBUG
			BOOL lbWasInitialized = TextWidth && TextHeight;
#endif
			// ��������� ����� ������ � ��������
			Width = TextWidth * nFontWidth;
			Height = TextHeight * nFontHeight;
#ifdef _DEBUG

			if (Height > 1200)
			{
				_ASSERTE(Height <= 1200);
			}

#endif

			if (ghOpWnd)
				gpConEmu->UpdateSizes();

			//if (!lbWasInitialized) // ���� ������� InitDC ������ ������ ������� ���������
			if (!abNoWndResize)
			{
				if (gpConEmu->isVisible(this))
				{
					MSetter lInConsoleResize(&mb_InConsoleResize);
					gpConEmu->OnSize(-1);
				}
			}

			hBitmap = CreateCompatibleBitmap(hScreenDC, Width, Height);
			SelectObject(hDC, hBitmap);
		}

		ReleaseDC(0, hScreenDC);
		return hBitmap!=NULL;
	}

	return true;
}

void CVirtualConsole::DumpConsole()
{
	if (!this) return;

	OPENFILENAME ofn; memset(&ofn,0,sizeof(ofn));
	wchar_t temp[MAX_PATH+5];
	ofn.lStructSize=sizeof(ofn);
	ofn.hwndOwner = ghWnd;
	ofn.lpstrFilter = _T("ConEmu dumps (*.con)\0*.con\0\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = temp; temp[0] = 0;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = L"Dump console...";
	ofn.lpstrDefExt = L"con";
	ofn.Flags = OFN_ENABLESIZING|OFN_NOCHANGEDIR
	            | OFN_PATHMUSTEXIST|OFN_EXPLORER|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;

	if (!GetSaveFileName(&ofn))
		return;

	Dump(temp);
}

BOOL CVirtualConsole::Dump(LPCWSTR asFile)
{
	// ��� ������� ������ ������ ������ (hDC) � png ����
	DumpImage(hDC, Width, Height, asFile);
	HANDLE hFile = CreateFile(asFile, GENERIC_WRITE, FILE_SHARE_READ,
	                          NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		DisplayLastError(_T("Can't create file!"));
		return FALSE;
	}

	DWORD dw;
	LPCTSTR pszTitle = gpConEmu->GetLastTitle();
	WriteFile(hFile, pszTitle, _tcslen(pszTitle)*sizeof(*pszTitle), &dw, NULL);
	wchar_t temp[100];
	swprintf(temp, _T("\r\nSize: %ix%i   Cursor: %ix%i\r\n"), TextWidth, TextHeight, Cursor.x, Cursor.y);
	WriteFile(hFile, temp, _tcslen(temp)*sizeof(wchar_t), &dw, NULL);
	WriteFile(hFile, mpsz_ConChar, TextWidth * TextHeight * sizeof(*mpsz_ConChar), &dw, NULL);
	WriteFile(hFile, mpn_ConAttrEx, TextWidth * TextHeight * sizeof(*mpn_ConAttrEx), &dw, NULL);
	WriteFile(hFile, mpsz_ConCharSave, TextWidth * TextHeight * sizeof(*mpsz_ConCharSave), &dw, NULL);
	WriteFile(hFile, mpn_ConAttrExSave, TextWidth * TextHeight * sizeof(*mpn_ConAttrExSave), &dw, NULL);

	if (mp_RCon)
	{
		mp_RCon->DumpConsole(hFile);
	}

	CloseHandle(hFile);
	return TRUE;
}

// ��� ������� ����� � ��. ����. �������
//#define isCharBorder(inChar) (inChar>=0x2013 && inChar<=0x266B)
bool CVirtualConsole::isCharBorder(wchar_t inChar)
{
	// ���� - ����� ��������� gpSet->isFixFarBorders ��� ��� ��� ���������, ����� ��������� gpSet->isEnhanceGraphics
	//if (!gpSet->isFixFarBorders)
	//	return false;
	return gpSet->isCharBorder(inChar);
	//CSettings::CharRanges *pcr = gpSet->icFixFarBorderRanges;
	//for (int i = 10; --i && pcr->bUsed; pcr++) {
	//	CSettings::CharRanges cr = *pcr;
	//	if (inChar>=cr.cBegin && inChar<=cr.cEnd)
	//		return true;
	//}
	//return false;
	//if (inChar>=0x2013 && inChar<=0x266B)
	//    return true;
	//else
	//    return false;
	//if (gpSet->isFixFarBorders)
	//{
	//  //if (! (inChar > 0x2500 && inChar < 0x251F))
	//  if ( !(inChar > 0x2013/*En dash*/ && inChar < 0x266B/*Beamed Eighth Notes*/) /*&& inChar!=L'}'*/ )
	//      /*if (inChar != 0x2550 && inChar != 0x2502 && inChar != 0x2551 && inChar != 0x007D &&
	//      inChar != 0x25BC && inChar != 0x2593 && inChar != 0x2591 && inChar != 0x25B2 &&
	//      inChar != 0x2562 && inChar != 0x255F && inChar != 0x255A && inChar != 0x255D &&
	//      inChar != 0x2554 && inChar != 0x2557 && inChar != 0x2500 && inChar != 0x2534 && inChar != 0x2564) // 0x2520*/
	//      return false;
	//  else
	//      return true;
	//}
	//else
	//{
	//  if (inChar < 0x01F1 || inChar > 0x0400 && inChar < 0x045F || inChar > 0x2012 && inChar < 0x203D || /*? - not sure that optimal*/ inChar > 0x2019 && inChar < 0x2303 || inChar > 0x24FF && inChar < 0x266C)
	//      return false;
	//  else
	//      return true;
	//}
}

// � ��� ������ "��������" �������, � ������� ���� ����� (���� �� ���������) ������������ ����� + �������/���������
bool CVirtualConsole::isCharBorderVertical(wchar_t inChar)
{
	//if (inChar>=0x2502 && inChar<=0x25C4 && inChar!=0x2550)
	//2009-07-12 ��� ��������� - ������� �������� ����� �� ����������� �������������� �����
	//if (inChar==ucBoxSinglVert || inChar==0x2503 || inChar==0x2506 || inChar==0x2507
	//    || (inChar>=0x250A && inChar<=0x254B) || inChar==0x254E || inChar==0x254F
	//    || (inChar>=0x2551 && inChar<=0x25C5)) // �� ������ �������� Arial Unicode MS
	TODO("ucBoxSinglHorz �������� �� �����?");

	if (inChar != ucBoxDblHorz && (inChar >= ucBoxSinglVert && inChar <= ucBoxDblVertHorz))
		return true;
	else
		return false;
}

bool CVirtualConsole::isCharProgress(wchar_t inChar)
{
	bool isProgress = (inChar == ucBox25 || inChar == ucBox50 || inChar == ucBox75 || inChar == ucBox100);
	return isProgress;
}

bool CVirtualConsole::isCharScroll(wchar_t inChar)
{
	bool isScrollbar = (inChar == ucBox25 || inChar == ucBox50 || inChar == ucBox75 || inChar == ucBox100
	                    || inChar == ucUpScroll || inChar == ucDnScroll);
	return isScrollbar;
}

bool CVirtualConsole::isCharNonSpacing(wchar_t inChar)
{
	// ����� ���������� �� �������, ������� ������ �������� ������ � �������� �������.
	// ��������, 0xFEFF �� ��������� ������� ������ ����������� GDI �� �����-�� ����� ����� O_O
	// �� � ��, ��� ��� "�� �������� �����" � ������� �����������. ���� ���� ��������, �� ����,
	// ������ ������������� ��� ������, �������� ��� ���� �� ����� �� ���, �.�. �� ��������
	// ���������� � �������!
	switch (inChar)
	{
		case 0x135F:
		case 0x2060:
		case 0x3000:
		case 0x3099:
		case 0x309A:
		case 0xA66F:
		case 0xA670:
		case 0xA671:
		case 0xA672:
		case 0xA67C:
		case 0xA67D:
		case 0xFEFF:
			return true;

		default:
			if (inChar>=0x0300)
			{
				if (inChar<=0x2DFF)
				{
					if ((inChar<=0x036F) // Combining/Accent/Acute/NonSpacing
						|| (inChar>=0x2000 && inChar<=0x200F)
						|| (inChar>=0x202A && inChar<=0x202E)
						|| (inChar>=0x0483 && inChar<=0x0489)
						|| (inChar>=0x07EB && inChar<=0x07F3)
						|| (inChar>=0x1B6B && inChar<=0x1B73)
						|| (inChar>=0x1DC0 && inChar<=0x1DFF)
						|| (inChar>=0x20D0 && inChar<=0x20F0)
						|| (inChar>=0x2DE0))
					{
						return true;
					}
				}
				else if (inChar>=0xFE20 && inChar<=0xFE26)
				{
					return true;
				}
			}
	}
	return false;
	
	/*
	wchar_t CharList[] = {0x135F, 0xFEFF, 0};
	__asm {
		MOV  ECX, ARRAYSIZE(CharList)
		REPZ SCAS CharList
	}
	*/
}

bool CVirtualConsole::isCharSpace(wchar_t inChar)
{
	// ���� ������ ��� �������, ������� ����� ���������� ������ ����� (��� ������� ������)
	bool isSpace = (inChar == ucSpace || inChar == ucNoBreakSpace || inChar == 0 
		/*|| (inChar>=0x2000 && inChar<=0x200F)
		|| inChar == 0x2060 || inChar == 0x3000 || inChar == 0xFEFF*/);
	return isSpace;
}

void CVirtualConsole::BlitPictureTo(int inX, int inY, int inWidth, int inHeight)
{
#ifdef _DEBUG
	BOOL lbDump = FALSE;

	if (lbDump) DumpImage(hBgDc, bgBmpSize.X, bgBmpSize.Y, L"F:\\bgtemp.png");

#endif

	if (bgBmpSize.X>inX && bgBmpSize.Y>inY)
		BitBlt(hDC, inX, inY, inWidth, inHeight, hBgDc, inX, inY, SRCCOPY);

	if (bgBmpSize.X<(inX+inWidth) || bgBmpSize.Y<(inY+inHeight))
	{
		if (hBrush0 == NULL)
		{
			hBrush0 = CreateSolidBrush(mp_Colors[0]);
			SelectBrush(hBrush0);
		}

		RECT rect = {max(inX,bgBmpSize.X), inY, inX+inWidth, inY+inHeight};
#ifndef SKIP_ALL_FILLRECT

		if (!IsRectEmpty(&rect))
			FillRect(hDC, &rect, hBrush0);

#endif

		if (bgBmpSize.X>inX)
		{
			rect.left = inX; rect.top = max(inY,bgBmpSize.Y); rect.right = bgBmpSize.X;
#ifndef SKIP_ALL_FILLRECT

			if (!IsRectEmpty(&rect))
				FillRect(hDC, &rect, hBrush0);

#endif
		}

		//DeleteObject(hBrush);
	}
}

void CVirtualConsole::SelectFont(HFONT hNew)
{
	if (hNew == NULL)
	{
		if (hOldFont)
			SelectObject(hDC, hOldFont);

		hOldFont = NULL;
		hSelectedFont = NULL;
	}
	else if (hSelectedFont != hNew)
	{
#ifdef _DEBUG

		if (hNew == mh_UCharMapFont)
			hNew = mh_UCharMapFont;

#endif
		hSelectedFont = (HFONT)SelectObject(hDC, hNew);

		if (!hOldFont)
			hOldFont = hSelectedFont;

		hSelectedFont = hNew;
	}
}

void CVirtualConsole::SelectBrush(HBRUSH hNew)
{
	if (!hNew)
	{
		if (hOldBrush)
			SelectObject(hDC, hOldBrush);

		hOldBrush = NULL;
	}
	else if (hSelectedBrush != hNew)
	{
		hSelectedBrush = (HBRUSH)SelectObject(hDC, hNew);

		if (!hOldBrush)
			hOldBrush = hSelectedBrush;

		hSelectedBrush = hNew;
	}
}

bool CVirtualConsole::CheckSelection(const CONSOLE_SELECTION_INFO& select, SHORT row, SHORT col)
{
	TODO("��������� ���� �� �����");
	return false;
	//if ((select.dwFlags & CONSOLE_SELECTION_NOT_EMPTY) == 0)
	//    return false;
	//if (row < select.srSelection.Top || row > select.srSelection.Bottom)
	//    return false;
	//if (col < select.srSelection.Left || col > select.srSelection.Right)
	//    return false;
	//return true;
}

#ifdef MSGLOGGER
class DcDebug
{
	public:
		DcDebug(HDC* ahDcVar, HDC* ahPaintDC)
		{
			mb_Attached=FALSE; mh_OrigDc=NULL; mh_DcVar=NULL; mh_Dc=NULL;

			if (!ahDcVar || !ahPaintDC) return;

			mh_DcVar = ahDcVar;
			mh_OrigDc = *ahDcVar;
			mh_Dc = *ahPaintDC;
			*mh_DcVar = mh_Dc;
		};
		~DcDebug()
		{
			if (mb_Attached && mh_DcVar)
			{
				mb_Attached = FALSE;
				*mh_DcVar = mh_OrigDc;
			}
		};
	protected:
		BOOL mb_Attached;
		HDC mh_OrigDc, mh_Dc;
		HDC* mh_DcVar;
};
#endif

// ������������� ���������� �������� ������� (���+�����) � ����� ���� � ������
// (!) ���������� ������ ������ ����� ���� ��������� �� ���� ������ ����� ����
// atr ������������� � BYTE, �.�. ������� ����� ��� �� ����������
//void CVirtualConsole::GetCharAttr(WORD atr, BYTE& foreColorNum, BYTE& backColorNum, HFONT* pFont)
//{
//	// ��� ���������� ������ ������ ����
//    atr &= 0xFF;
//
//    // ������� ����� ������� ������� ������ � �������, � �������� ������ �� �������
//    foreColorNum = m_ForegroundColors[atr];
//    backColorNum = m_BackgroundColors[atr];
//    if (pFont) *pFont = mh_FontByIndex[atr];
//
//    if (bExtendColors) {
//        if (backColorNum == nExtendColor) {
//            backColorNum = attrBackLast; // ��� ����� �������� �� ������� ���� �� �������� ������
//            foreColorNum += 0x10;
//        } else {
//            attrBackLast = backColorNum; // �������� ������� ���� �������� ������
//        }
//    }
//}

void CVirtualConsole::CharABC(wchar_t ch, ABC *abc)
{
	BOOL lbCharABCOk;

	if (!gpSet->CharABC[ch].abcB)
	{
		if (gpSet->mh_Font2 && gpSet->isFixFarBorders && isCharBorder(ch))
		{
			SelectFont(gpSet->mh_Font2);
		}
		else
		{
			TODO("��� ���� �� ������� �� ������ �������");
			SelectFont(gpSet->mh_Font[0]);
		}

		//This function succeeds only with TrueType fonts
		lbCharABCOk = GetCharABCWidths(hDC, ch, ch, &gpSet->CharABC[ch]);

		if (!lbCharABCOk)
		{
			// ������ ����� �� TTF/OTF
			gpSet->CharABC[ch].abcB = CharWidth(ch);
			_ASSERTE(gpSet->CharABC[ch].abcB);

			if (!gpSet->CharABC[ch].abcB) gpSet->CharABC[ch].abcB = 1;

			gpSet->CharABC[ch].abcA = gpSet->CharABC[ch].abcC = 0;
		}
	}

	*abc = gpSet->CharABC[ch];
}

// ���������� ������ �������, ��������� FixBorders
WORD CVirtualConsole::CharWidth(wchar_t ch)
{
	// ��������� �����, ����� �� �������� �� ������
	WORD nWidth = gpSet->CharWidth[ch];

	if (nWidth)
		return nWidth;

	if (gpSet->isMonospace
	        || (gpSet->isFixFarBorders && isCharBorder(ch))
	        || (gpSet->isEnhanceGraphics && isCharProgress(ch))
	  )
	{
		//2009-09-09 ��� �����������. ������ ������ ����� ����� ���� ������ ����������
		//return gpSet->BorderFontWidth();
		gpSet->CharWidth[ch] = nFontWidth;
		return nFontWidth;
	}

	//nWidth = nFontWidth;
	//bool isBorder = false; //, isVBorder = false;

	// �������� ��� �� ����� ������� ������ � ��� ������, ������� ����� ���� �����������
	if (gpSet->mh_Font2 && gpSet->isFixFarBorders && isCharBorder(ch))
	{
		SelectFont(gpSet->mh_Font2);
	}
	else
	{
		TODO("��� ���� �� ������� �� ������ �������");
		SelectFont(gpSet->mh_Font[0]);
	}

	//SelectFont(gpSet->mh_Font[0]);
	SIZE sz;
	//This function succeeds only with TrueType fonts
	//#ifdef _DEBUG
	//ABC abc;
	//BOOL lb1 = GetCharABCWidths(hDC, ch, ch, &abc);
	//#endif

	if (GetTextExtentPoint32(hDC, &ch, 1, &sz) && sz.cx)
		nWidth = sz.cx;
	else
		nWidth = nFontWidth;

	if (!nWidth)
		nWidth = 1; // �� ������ ������, ����� ������� �� 0 �� ��������

	gpSet->CharWidth[ch] = nWidth;
	return nWidth;
}

char CVirtualConsole::Uni2Oem(wchar_t ch)
{
	if (mc_Uni2Oem[ch])
		return mc_Uni2Oem[ch];

	char c = '?';
	WideCharToMultiByte(CP_OEMCP, 0, &ch, 1, &c, 1, 0, 0);
	mc_Uni2Oem[ch] = c;
	return c;
}

bool CVirtualConsole::CheckChangedTextAttr()
{
	textChanged = 0!=memcmp(mpsz_ConChar, mpsz_ConCharSave, TextLen * sizeof(*mpsz_ConChar));
	attrChanged = 0!=memcmp(mpn_ConAttrEx, mpn_ConAttrExSave, TextLen * sizeof(*mpn_ConAttrEx));
//#ifdef MSGLOGGER
//    COORD ch;
//    if (textChanged) {
//        for (UINT i=0; i<TextLen; i++) {
//            if (mpsz_ConChar[i] != mpsz_ConCharSave[i]) {
//                ch.Y = i % TextWidth;
//                ch.X = i - TextWidth * ch.Y;
//                break;
//            }
//        }
//    }
//    if (attrChanged) {
//        for (UINT i=0; i<TextLen; i++) {
//            if (mpn_ConAttr[i] != mpn_ConAttrSave[i]) {
//                ch.Y = i % TextWidth;
//                ch.X = i - TextWidth * ch.Y;
//                break;
//            }
//        }
//    }
//#endif
	return textChanged || attrChanged;
}

bool CVirtualConsole::Update(bool abForce, HDC *ahDc)
{
	if (!this || !mp_RCon || !mp_RCon->ConWnd())
		return false;

	isForce = abForce;

	if (!gpConEmu->isMainThread())
	{
		if (isForce)
			mb_RequiredForceUpdate = true;

		//if (mb_RequiredForceUpdate || updateText || updateCursor)
		{
			if (gpConEmu->isVisible(this))
			{
				if (gpSet->isAdvLogging>=3) mp_RCon->LogString("Invalidating from CVirtualConsole::Update.1");

				gpConEmu->Invalidate(this);
			}

			return true;
		}
		return false;
	}

	/* -- 2009-06-20 ���������. ������� ������ �������� ������ � MainThread
	if (!mp_RCon->IsConsoleThread()) {
	    if (!ahDc) {
	        mp_RCon->SetForceRead();
	        mp_RCon->WaitEndUpdate(1000);
	        //gpConEmu->InvalidateAll(); -- ����� �� All??? Update � ��� Invalidate �����
	        return false;
	    }
	}
	*/

	if (mb_InUpdate)
	{
		// �� ������ ���������� �� ��������
		if (!mb_InConsoleResize)
		{
			_ASSERTE(!mb_InUpdate);
		}

		return false;
	}

	MSetter inUpdate(&mb_InUpdate);
	// ������ ���������� ���������� �� CRealConsole. ����� ������� � ������� �� ��������.
	//RetrieveConsoleInfo();
#ifdef MSGLOGGER
	DcDebug dbg(&hDC, ahDc); // ��� ������� - ������ ����� �� ������� ����
#endif
	HEAPVAL
	bool lRes = false;
	MSectionLock SCON; SCON.Lock(&csCON);
	MSectionLock SDC; //&csDC, &ncsTDC);

	//if (mb_PaintRequested) -- �� ������ ����. ��� ������� �������� ������ � ���������� ����
	if (mb_PaintLocked)  // ������ ���� ����������� Paint (BitBlt) - ��� ����� ���� �� ����� �������, ��� ��� ������� ���-�� ���������
		SDC.Lock(&csDC, TRUE, 200); // �� �� ��������, ����� �� ������� ���������

	mp_RCon->GetConsoleScreenBufferInfo(&csbi);
	// start timer before "Read Console Output*" calls, they do take time
	//gpSet->Performance(tPerfRead, FALSE);
	//if (gbNoDblBuffer) isForce = TRUE; // Debug, dblbuffer
	isForeground = gpConEmu->isMeForeground(false);

	if (isFade == isForeground && gpSet->isFadeInactive)
		isForce = true;

	//------------------------------------------------------------------------
	///| Read console output and cursor info... |/////////////////////////////
	//------------------------------------------------------------------------
	if (!UpdatePrepare(ahDc, &SDC))
	{
		gpConEmu->DebugStep(_T("DC initialization failed!"));
		return false;
	}

	//gpSet->Performance(tPerfRead, TRUE);
	gpSet->Performance(tPerfRender, FALSE);
	//------------------------------------------------------------------------
	///| Drawing text (if there were changes in console) |////////////////////
	//------------------------------------------------------------------------
	bool updateText, updateCursor;

	if (isForce)
	{
		updateText = updateCursor = attrChanged = textChanged = true;
	}
	else
	{
		// Do we have to update changed text?
		updateText = CheckChangedTextAttr();

		// Do we have to update text under changed cursor?
		// Important: check both 'cinf.bVisible' and 'Cursor.isVisible',
		// because console may have cursor hidden and its position changed -
		// in this case last visible cursor remains shown at its old place.
		// Also, don't check foreground window here for the same reasons.
		// If position is the same then check the cursor becomes hidden.
		if (Cursor.x != csbi.dwCursorPosition.X || Cursor.y != csbi.dwCursorPosition.Y)
			// ��������� �������. ��������� ���� ������ �����
			updateCursor = cinf.bVisible || Cursor.isVisible || Cursor.isVisiblePrevFromInfo;
		else
			updateCursor = Cursor.isVisiblePrevFromInfo && !cinf.bVisible;
	}

	mb_DialogsChanged = CheckDialogsChanged();
	// ���� �� ������� ������ ConEmuTh - ��������� ��������� �
	// ������ ����������� ���.����� (��� �� ������ ���������� �������� ���������)
	// � �������� ������� ������� (������ �����, ������� ��������� "���" ���������)
	PolishPanelViews();

	//gpSet->Performance(tPerfRender, FALSE);

	if (updateText /*|| updateCursor*/)
	{
		lRes = true;
		DEBUGSTRDRAW(L" +++ updateText detected in VCon\n");
		//gpSet->Performance(tPerfRender, FALSE);
		//------------------------------------------------------------------------
		///| Drawing modified text |//////////////////////////////////////////////
		//------------------------------------------------------------------------
		UpdateText();
		//gpSet->Performance(tPerfRender, TRUE);
		//HEAPVAL
		//------------------------------------------------------------------------
		///| Now, store data for further comparison |/////////////////////////////
		//------------------------------------------------------------------------
		memcpy(mpsz_ConCharSave, mpsz_ConChar, TextLen * sizeof(*mpsz_ConChar));
		memcpy(mpn_ConAttrExSave, mpn_ConAttrEx, TextLen * sizeof(*mpn_ConAttrEx));
	}

	//-- ���������� � Paint
	//// ���� ���������������� ������ (ConEmuTh) - �������� ������� �������
	//// ������ ��� ����� ����� UpdateText, ������ ��� ����� ConCharX ����� ���� �������
	//if (m_LeftPanelView.bRegister || m_RightPanelView.bRegister) {
	//	if (mb_DialogsChanged) {
	//		UpdatePanelRgn(TRUE);
	//		UpdatePanelRgn(FALSE);
	//	}
	//}
	//HEAPVAL
	//------------------------------------------------------------------------
	///| Checking cursor |////////////////////////////////////////////////////
	//------------------------------------------------------------------------
	UpdateCursor(lRes);
	HEAPVAL
	//SDC.Leave();
	SCON.Unlock();

	// ����� ��������� ���������� ���������� ������� (�������/��������)
	//
	if (lRes && gpConEmu->isVisible(this))
	{
		if (mpsz_LogScreen && mp_RCon && mp_RCon->GetServerPID())
		{
			// ������� ������ � ���
			mn_LogScreenIdx++;
			wchar_t szLogPath[MAX_PATH]; _wsprintf(szLogPath, SKIPLEN(countof(szLogPath)) mpsz_LogScreen, mp_RCon->GetServerPID(), mn_LogScreenIdx);
			Dump(szLogPath);
		}

		// ���� ��������� - ����������� ���������� viewport'�
		if (!mb_InPaintCall)
		{
			// ������ ���������� � �������� ����
			_ASSERTE(gpConEmu->isMainThread());
			mb_PaintRequested = TRUE;
			gpConEmu->Invalidate(this);

			if (gpSet->isAdvLogging>=3) mp_RCon->LogString("Invalidating from CVirtualConsole::Update.2");

			//09.06.13 � ���� ���? ������� ��������� �� ������ �� ��������?
			//UpdateWindow('ghWnd DC'); // ��� �������� ��������� � ����, � ���� ��������� ���������
#ifdef _DEBUG
			//_ASSERTE(!gpConEmu->m_Child->mb_Invalidated);
#endif
			mb_PaintRequested = FALSE;
		}
	}

	gpSet->Performance(tPerfRender, TRUE);
	/* ***************************************** */
	/*       Finalization, release objects       */
	/* ***************************************** */
	SelectBrush(NULL);

	if (hBrush0)    // ��������� � BlitPictureTo
	{
		DeleteObject(hBrush0); hBrush0 = NULL;
	}

	/*
	for (UINT br=0; br<m_PartBrushes.size(); br++) {
	    DeleteObject(m_PartBrushes[br].hBrush);
	}
	m_PartBrushes.clear();
	*/
	SelectFont(NULL);
	HEAPVAL
	return lRes;
}

BOOL CVirtualConsole::CheckTransparentRgn()
{
	BOOL lbRgnChanged = FALSE;

	if (mb_ConDataChanged)
	{
		mb_ConDataChanged = FALSE;

		// (����)������� ������
		if (mpsz_ConChar && mpn_ConAttrEx && TextWidth && TextHeight)
		{
			MSectionLock SCON; SCON.Lock(&csCON);
			CharAttr* pnAttr = mpn_ConAttrEx;
			int nFontHeight = gpSet->FontHeight();
			int    nMaxRects = TextHeight*5;
#ifdef _DEBUG
			nMaxRects = 5;
#endif
			POINT *lpAllPoints = (POINT*)Alloc(nMaxRects*4,sizeof(POINT)); // 4 ����
			//INT   *lpAllCounts = (INT*)Alloc(nMaxRects,sizeof(INT));
			HEAPVAL;
			int    nRectCount = 0;

			if (lpAllPoints /*&& lpAllCounts*/)
			{
				POINT *lpPoints = lpAllPoints;
				//INT   *lpCounts = lpAllCounts;

				for(uint nY = 0; nY < TextHeight; nY++)
				{
					uint nX = 0;
					int nYPix1 = nY * nFontHeight;

					while(nX < TextWidth)
					{
						// ����� ������ ���������� cell
						#if 0
						while (nX < TextWidth && !pnAttr[nX].bTransparent)
							nX++;
						#else
						WARNING("CharAttr_Transparent");
						while (nX < TextWidth && !(pnAttr[nX].Flags & CharAttr_Transparent))
							nX++;
						#endif

						if (nX >= TextWidth)
							break;

						// ����� ����� ����������� �����
						uint nTranStart = nX;

						#if 0
						while (++nX < TextWidth && pnAttr[nX].bTransparent)
							;
						#else
						WARNING("CharAttr_Transparent");
						while (++nX < TextWidth && (pnAttr[nX].Flags & CharAttr_Transparent))
							;
						#endif

						// ������������ ��������������� Rect
						int nXPix = 0;

						if (nRectCount>=nMaxRects)
						{
							nMaxRects += TextHeight;
							HEAPVAL;
							POINT *lpTmpPoints = (POINT*)Alloc(nMaxRects*4,sizeof(POINT)); // 4 ����
							//INT   *lpTmpCounts = (INT*)Alloc(nMaxRects,sizeof(INT));
							HEAPVAL;

							if (!lpTmpPoints /*|| !lpTmpCounts*/)
							{
								_ASSERTE(/*lpTmpCounts &&*/ lpTmpPoints);
								//Free(lpAllCounts);
								Free(lpAllPoints);
								return FALSE;
							}

							memmove(lpTmpPoints, lpAllPoints, nRectCount*4*sizeof(POINT));
							//memmove(lpTmpCounts, lpAllCounts, nRectCount*sizeof(INT));
							HEAPVAL;
							lpPoints = lpTmpPoints + (lpPoints - lpAllPoints);
							//Free(lpAllCounts);
							Free(lpAllPoints);
							lpAllPoints = lpTmpPoints;
							//lpAllCounts = lpTmpCounts;
							HEAPVAL;
						}

						nRectCount++;
#ifdef _DEBUG

						if (nRectCount>=nMaxRects)
						{
							nRectCount = nRectCount;
						}

#endif
						//*(lpCounts++) = 4;
						lpPoints[0] = ConsoleToClient(nTranStart, nY);
						lpPoints[1] = ConsoleToClient(nX, nY);
						// x - 1?
						//if (lpPoints[1].x > lpPoints[0].x) lpPoints[1].x--;
						lpPoints[2] = lpPoints[1]; lpPoints[2].y += nFontHeight;
						lpPoints[3] = lpPoints[0]; lpPoints[3].y = lpPoints[2].y;
						lpPoints += 4;
						nX++;
						HEAPVAL;
					}

					pnAttr += TextWidth;
				}

				HEAPVAL;
				lbRgnChanged = (nRectCount != TransparentInfo.nRectCount);

				if (!lbRgnChanged && TransparentInfo.nRectCount)
				{
					_ASSERTE(TransparentInfo.pAllPoints && TransparentInfo.pAllCounts);
					lbRgnChanged = memcmp(TransparentInfo.pAllPoints, lpAllPoints, sizeof(POINT)*4*nRectCount)!=0;
				}

				HEAPVAL;

				if (lbRgnChanged)
				{
					if (mh_TransparentRgn) { DeleteObject(mh_TransparentRgn); mh_TransparentRgn = NULL; }

					INT   *lpAllCounts = NULL;

					if (nRectCount > 0)
					{
						lpAllCounts = (INT*)Alloc(nRectCount,sizeof(INT));
						INT   *lpCounts = lpAllCounts;

						for(int n = 0; n < nRectCount; n++)
							*(lpCounts++) = 4;

						mh_TransparentRgn = CreatePolyPolygonRgn(lpAllPoints, lpAllCounts, nRectCount, WINDING);
					}

					HEAPVAL;

					if (TransparentInfo.pAllCounts)
						Free(TransparentInfo.pAllCounts);

					HEAPVAL;
					TransparentInfo.pAllCounts = lpAllCounts; lpAllCounts = NULL;

					if (TransparentInfo.pAllPoints)
						Free(TransparentInfo.pAllPoints);

					HEAPVAL;
					TransparentInfo.pAllPoints = lpAllPoints; lpAllPoints = NULL;
					HEAPVAL;
					TransparentInfo.nRectCount = nRectCount;
				}

				HEAPVAL;

				//if (lpAllCounts) Free(lpAllCounts);
				if (lpAllPoints) Free(lpAllPoints);

				HEAPVAL;
			}
		}
	}

	return lbRgnChanged;
}

bool CVirtualConsole::LoadConsoleData()
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return false;
	}

	// ����� ����, ���� ������� ���������, �.�. CVirtualConsole::Update �� ���������� � ������� �� ����������. � ��� ���������.
	if (!mpsz_ConChar || !mpn_ConAttrEx)
	{
		// ���� ��� ������ ConEmu ������� ��������� �������� ����� '@'
		// �� ��� ����� �������� - �� ���������������� (InitDC �� ���������),
		// ��� ����� ������ � ������� ����, LoadConsoleData �� ���� �����������
		gpConEmu->InitInactiveDC(this);
		return false;
	}

	gpSet->Performance(tPerfData, FALSE);
	{
		#ifdef SHOWDEBUGSTEPS
		gpConEmu->DebugStep(L"mp_RCon->GetConsoleData");
		#endif

		mp_RCon->GetConsoleData(mpsz_ConChar, mpn_ConAttrEx, TextWidth, TextHeight); //TextLen*2);

		#ifdef SHOWDEBUGSTEPS
		gpConEmu->DebugStep(NULL);
		#endif
	}
	SMALL_RECT rcFull, rcGlyph = {0,0,-1,-1};

	if (gpSet->isExtendUCharMap)
	{
		const CRgnDetect* pRgn = mp_RCon->GetDetector();

		if (pRgn && (pRgn->GetFlags() & (FR_UCHARMAP|FR_UCHARMAPGLYPH)) == (FR_UCHARMAP|FR_UCHARMAPGLYPH))
		{
			if (pRgn->GetDetectedDialogs(1, &rcFull, NULL, FR_UCHARMAP, FR_UCHARMAP)
			        && pRgn->GetDetectedDialogs(1, &rcGlyph, NULL, FR_UCHARMAPGLYPH, FR_UCHARMAPGLYPH))
			{
				wchar_t szFontName[32], *pszStart, *pszEnd;
				pszStart = mpsz_ConChar + TextWidth*(rcFull.Top+1) + rcFull.Left + 1;
				pszEnd = pszStart + 31;

				while(*pszEnd == L' ' && pszEnd >= pszStart) pszEnd--;

				if (pszEnd > pszStart)
				{
					wmemcpy(szFontName, pszStart, pszEnd-pszStart+1);
					szFontName[pszEnd-pszStart+1] = 0;

					if (!mh_UCharMapFont || lstrcmp(ms_LastUCharMapFont, szFontName))
					{
						wcscpy_c(ms_LastUCharMapFont, szFontName);

						if (mh_UCharMapFont) DeleteObject(mh_UCharMapFont);

						mh_UCharMapFont = gpSet->CreateOtherFont(ms_LastUCharMapFont);
					}
				}

				// ����� �������, ������ - �������� �������������� ������ ��� ������������� ����� ������
				if (mh_UCharMapFont)
				{
					for(int Y = rcGlyph.Top; Y <= rcGlyph.Bottom; Y++)
					{
						CharAttr *pAtr = mpn_ConAttrEx + (TextWidth*Y + rcGlyph.Left);

						for(int X = rcGlyph.Left; X <= rcGlyph.Right; X++, pAtr++)
						{
							pAtr->nFontIndex = MAX_FONT_STYLES;
						}
					}
				}
			}
		}
	}

	mrc_UCharMap = rcGlyph;
	gpSet->Performance(tPerfData, TRUE);
	return true;
}

bool CVirtualConsole::UpdatePrepare(HDC *ahDc, MSectionLock *pSDC)
{
	MSectionLock SCON; SCON.Lock(&csCON);
	attrBackLast = 0;
	isEditor = gpConEmu->isEditor();
	isViewer = gpConEmu->isViewer();
	isFilePanel = gpConEmu->isFilePanel(true);
	isFade = !isForeground && gpSet->isFadeInactive;
	mp_Colors = gpSet->GetColors(isFade);
	nFontHeight = gpSet->FontHeight();
	nFontWidth = gpSet->FontWidth();
	nFontCharSet = gpSet->FontCharSet();
	//bExtendColors = gpSet->isExtendColors;
	//nExtendColor = gpSet->nExtendColor;
	//bExtendFonts = gpSet->isExtendFonts;
	//nFontNormalColor = gpSet->nFontNormalColor;
	//nFontBoldColor = gpSet->nFontBoldColor;
	//nFontItalicColor = gpSet->nFontItalicColor;
	//m_ForegroundColors[0x100], m_BackgroundColors[0x100];
	//TODO("� ��������, ��� ����� ������ �� ������, � ������ ��� ����������");
	//int nColorIndex = 0;
	//for (int nBack = 0; nBack <= 0xF; nBack++) {
	//	for (int nFore = 0; nFore <= 0xF; nFore++, nColorIndex++) {
	//		m_ForegroundColors[nColorIndex] = nFore;
	//		m_BackgroundColors[nColorIndex] = nBack;
	//		mh_FontByIndex[nColorIndex] = gpSet->mh_Font;
	//		if (bExtendFonts) {
	//			if (nBack == nFontBoldColor) { // nFontBoldColor may be -1, ����� �� ���� �� ��������
	//				if (nFontNormalColor != 0xFF)
	//					m_BackgroundColors[nColorIndex] = nFontNormalColor;
	//				mh_FontByIndex[nColorIndex] = gpSet->mh_FontB;
	//			} else if (nBack == nFontItalicColor) { // nFontItalicColor may be -1, ����� �� ���� �� ��������
	//				if (nFontNormalColor != 0xFF)
	//					m_BackgroundColors[nColorIndex] = nFontNormalColor;
	//				mh_FontByIndex[nColorIndex] = gpSet->mh_FontI;
	//			}
	//		}
	//	}
	//}
	//winSize.X = csbi.srWindow.Right - csbi.srWindow.Left + 1; winSize.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	//if (winSize.X < csbi.dwSize.X)
	//    winSize.X = csbi.dwSize.X;
	winSize = MakeCoord(mp_RCon->TextWidth(),mp_RCon->TextHeight());
	//csbi.dwCursorPosition.X -= csbi.srWindow.Left; -- �������������� ��������� ������������!
	csbi.dwCursorPosition.Y -= csbi.srWindow.Top;
	isCursorValid =
	    csbi.dwCursorPosition.X >= 0 && csbi.dwCursorPosition.Y >= 0 &&
	    csbi.dwCursorPosition.X < winSize.X && csbi.dwCursorPosition.Y < winSize.Y;

	if (mb_RequiredForceUpdate || mb_IsForceUpdate)
	{
		isForce = true;
		mb_RequiredForceUpdate = false;
	}

	// ������ �������������, ��� ����� �������
	BOOL lbSizeChanged = (TextWidth != (uint)winSize.X || TextHeight != (uint)winSize.Y);
#ifdef _DEBUG
	COORD dbgWinSize = winSize;
	COORD dbgTxtSize = {TextWidth,TextHeight};
#endif

	if (isForce || !mb_PointersAllocated || lbSizeChanged)
	{
		// 100627 ������� ����� InitDC, �.�. ���������
		//if (lbSizeChanged)
		//	gpConEmu->OnConsoleResize(TRUE);
		if (pSDC && !pSDC->isLocked())  // ���� ������ ��� �� ������������� (��������� - ���������� �������)
			pSDC->Lock(&csDC, TRUE, 200); // �� �� ��������, ����� �� ������� ���������

		if (!InitDC(ahDc!=NULL && !isForce/*abNoDc*/ , false/*abNoWndResize*/))
			return false;

		if (lbSizeChanged)
		{
			MSetter lInConsoleResize(&mb_InConsoleResize);
			gpConEmu->OnConsoleResize(TRUE);
		}

		isForce = true; // ����� ������ ������� � DC - ��������� ������ refresh...
#ifdef _DEBUG

		if (TextWidth == 80 && !mp_RCon->isNtvdm())
		{
			TextWidth = TextWidth;
		}

#endif
		//if (lbSizeChanged) -- ��������� ����� ���������
		//	gpConEmu->OnConsoleResize();
	}

	// ��������� ������ �����������!
	if (mb_IsForceUpdate)
	{
		isForce = true;
		mb_IsForceUpdate = false;
	}

	// ���������� �����. ������� ����������� ���������, ��������� ���� ����� �� ahDc
	if (ahDc)
		isForce = true;

	//drawImage = (gpSet->isShowBgImage == 1 || (gpSet->isShowBgImage == 2 && !(isEditor || isViewer)) )
	//	&& gpSet->isBackgroundImageValid;
	drawImage = gpSet->IsBackgroundEnabled(this);
	TextLen = TextWidth * TextHeight;
	coord.X = csbi.srWindow.Left; coord.Y = csbi.srWindow.Top;

	DWORD nIndexes = gpSet->nBgImageColors;
	if (nIndexes == (DWORD)-1 && mp_RCon)
	{
		// �������� �� ����
		const CEFAR_INFO_MAPPING* pFar = mp_RCon->GetFarInfo();
		if (pFar)
		{
			// ���� ���� �������
			BYTE bgIndex = (pFar->nFarColors[col_PanelText] & 0xF0) >> 4;
			nIndexes = 1 << (DWORD)bgIndex;
		}
	}
	if (nIndexes == (DWORD)-1)
		nIndexes = BgImageColorsDefaults; // ���������
	nBgImageColors = nIndexes;

	if (drawImage)
	{
		// ��� ������ ��������, �� ��������� �� ����?
		if (gpSet->PrepareBackground(&hBgDc, &bgBmpSize))
		{
			isForce = true;
		}
		else
		{
			drawImage = (hBgDc!=NULL);
		}
	}

	// ����������� ������ �� ��������� ������� � mpn_ConAttrEx/mpsz_ConChar
	BOOL bConDataChanged = isForce || mp_RCon->IsConsoleDataChanged();

	if (bConDataChanged)
	{
		mb_ConDataChanged = TRUE; // � FALSE - �� ����������
		// ����� � �������, �.�. ������ ��� RealConsole ����� ������� ���������� ��������� ��������
		LoadConsoleData();
	}

	HEAPVAL
	// ���������� ���������� ������� (��������� � CRealConsole)
	// get cursor info
	mp_RCon->GetConsoleCursorInfo(/*hConOut(),*/ &cinf);
	HEAPVAL
	return true;
}

//enum CVirtualConsole::_PartType CVirtualConsole::GetCharType(wchar_t ch)
//{
//    enum _PartType cType = pNull;
//
//    if (ch == L' ')
//        cType = pSpace;
//    //else if (ch == L'_')
//    //  cType = pUnderscore;
//    else if (isCharBorder(ch)) {
//        if (isCharBorderVertical(ch))
//            cType = pVBorder;
//        else
//            cType = pBorder;
//    }
//    else if (isFilePanel && ch == L'}')
//        cType = pRBracket;
//    else
//        cType = pText;
//
//    return cType;
//}



// row - 0-based
//void CVirtualConsole::ParseLine(int row, wchar_t *ConCharLine, WORD *ConAttrLine)
//{
//    //UINT idx = 0;
//    struct _TextParts *pStart=TextParts, *pEnd=TextParts;
//    enum _PartType cType1, cType2;
//    UINT i1=0, i2=0;
//
//    pEnd->partType = pNull; // ����� ��������� ������
//
//    wchar_t ch1, ch2;
//    BYTE af1, ab1, af2, ab2;
//    DWORD pixels;
//    while (i1<TextWidth)
//    {
//        //GetCharAttr(ConCharLine[i1], ConAttrLine[i1], ch1, af1, ab1);
//		ch1 = ConCharLine[i1];
//		GetCharAttr(ConAttrLine[i1], af1, ab1, NULL);
//        cType1 = GetCharType(ch1);
//        if (cType1 == pRBracket) {
//            if (!(row>=2 && isCharBorderVertical(mpsz_ConChar[TextWidth+i1]))
//                && (((UINT)row)<=(TextHeight-4)))
//                cType1 = pText;
//        }
//        pixels = CharWidth(ch1);
//
//        i2 = i1+1;
//        // � ������ Force Monospace ��������� ���� �� ������ �������
//        if (!gpSet->isForceMonospace && i2 < TextWidth &&
//            (cType1 != pVBorder && cType1 != pRBracket))
//        {
//            //GetCharAttr(ConCharLine[i2], ConAttrLine[i2], ch2, af2, ab2);
//			ch2 = ConCharLine[i2];
//			GetCharAttr(ConAttrLine[i2], af2, ab2, NULL);
//            // �������� ���� �������� � ������������ �������
//            while (i2 < TextWidth && af2 == af1 && ab2 == ab1) {
//                // ���� ������ ���������� �� �������
//
//                cType2 = GetCharType(ch2);
//                if ((ch2 = ConCharLine[i2]) != ch1) {
//                    if (cType2 == pRBracket) {
//                        if (!(row>=2 && isCharBorderVertical(mpsz_ConChar[TextWidth+i2]))
//                            && (((UINT)row)<=(TextHeight-4)))
//                            cType2 = pText;
//                    }
//
//                    // � �� ������ �� ������ ������
//                    if (cType2 != cType1)
//                        break; // �� ��������� �����
//                }
//                pixels += CharWidth(ch2); // �������� ������ ������� � ��������
//                i2++; // ��������� ������
//                //GetCharAttr(ConCharLine[i2], ConAttrLine[i2], ch2, af2, ab2);
//				ch2 = ConCharLine[i2];
//				GetCharAttr(ConAttrLine[i2], af2, ab2, NULL);
//                if (cType2 == pRBracket) {
//                    if (!(row>=2 && isCharBorderVertical(mpsz_ConChar[TextWidth+i2]))
//                        && (((UINT)row)<=(TextHeight-4)))
//                        cType2 = pText;
//                }
//            }
//        }
//
//        // ��� ������� ������ ����� ��������, ���� ����� pText,pSpace,pText �� pSpace,pText �������� � ������ pText
//        if (cType1 == pText && (pEnd - pStart) >= 2) {
//            if (pEnd[-1].partType == pSpace && pEnd[-2].partType == pText &&
//                pEnd[-1].attrBack == ab1 && pEnd[-1].attrFore == af1 &&
//                pEnd[-2].attrBack == ab1 && pEnd[-2].attrFore == af1
//                )
//            {
//                pEnd -= 2;
//                pEnd->i2 = i2 - 1;
//                pEnd->iwidth = i2 - pEnd->i1;
//                pEnd->width += pEnd[1].width + pixels;
//                pEnd ++;
//                i1 = i2;
//                continue;
//            }
//        }
//        pEnd->i1 = i1; pEnd->i2 = i2 - 1; // ����� "�������"
//        pEnd->partType = cType1;
//        pEnd->attrBack = ab1; pEnd->attrFore = af1;
//        pEnd->iwidth = i2 - i1;
//        pEnd->width = pixels;
//        if (gpSet->isForceMonospace ||
//            (gpSet->isProportional && (cType1 == pVBorder || cType1 == pRBracket)))
//        {
//            pEnd->x1 = i1 * gpSet->FontWidth();
//        } else {
//            pEnd->x1 = -1;
//        }
//
//        pEnd ++; // ������ �� ����� ���� ������ ���������� �������� � ������, ��� ��� � ������������ ��� ��
//        i1 = i2;
//    }
//    // ���� �������� ����� ������, �����, ���� ������ �� ������ - ������� pDummy
//    pEnd->partType = pNull;
//}

void CVirtualConsole::Update_CheckAndFill()
{
	// pointers
	wchar_t* ConCharLine = mpsz_ConChar;
	CharAttr* ConAttrLine = mpn_ConAttrEx;
	// previous
	const CharAttr* ConAttrLine2 = mpn_ConAttrExSave;
	const wchar_t* ConCharLine2 = mpsz_ConCharSave;
	// counters
	int pos = 0;
	int row = 0;
	int nMaxPos = Height - nFontHeight;
	// locals
	bool bRowChanged;

	// rows
	//BUGBUG: ������ �� ������������ ��������� ������, ���� ���� ��� ���� �� ������
	for(; pos <= nMaxPos;
	        ConCharLine += TextWidth, ConAttrLine += TextWidth,
	        ConCharLine2 += TextWidth, ConAttrLine2 += TextWidth,
	        pbLineChanged++, pbBackIsPic++, pnBackRGB++,
	        pos += nFontHeight, row++)
	{
		if (row >= (int)TextHeight)
		{
			_ASSERTE(row<(int)TextHeight);
			return;
		}

		bRowChanged = isForce
		              || (wmemcmp(ConCharLine, ConCharLine2, TextWidth) != 0)
		              || (memcmp(ConAttrLine, ConAttrLine2, TextWidth*sizeof(*ConAttrLine)) != 0);
		//// skip not changed symbols except the old cursor or selection
		//int j = 0, end = TextWidth;
		//// � ������ ���������������� ������� ��� isForce==true - ������������ ����� �������
		//// ��� ���������������� ��� ����������� ���, ��� ��� ����������� ���������� �����
		//// ����� ��������� ������ ����������� ��������, � ���������� �������� ���������
		//if (skipNotChanged)
		//{
		//    // Skip not changed head and tail symbols
		//	if (!FindChanges(j, end, ConCharLine, ConAttrLine, ConCharLine2, ConAttrLine2))
		//		continue;
		//} // if (skipNotChanged)
		*pbLineChanged = bRowChanged;

		if (bRowChanged)
		{
			if (drawImage && ISBGIMGCOLOR(ConAttrLine->nBackIdx))
			{
				*pbBackIsPic = true;
				BlitPictureTo(0, pos, Width, nFontHeight);
			}
			else
			{
				*pbBackIsPic = false;
				COLORREF cr = ConAttrLine->crBackColor;
				*pnBackRGB = cr;
				// ������ ����������, ����� �� �������������� � ���������� ��������� ��������
				HBRUSH hbr = PartBrush(L' ', cr, 0);
				RECT rect = MakeRect(0, pos, Width, pos + nFontHeight);
				FillRect(hDC, &rect, hbr);
			}
		}
	}
}

// ������ ������ �� ������������ (���������� true, ���� ���� ������ � �� �������� �����)
// ������� ����� ���������� ������������� (���������� ��������� � DX)
bool CVirtualConsole::Update_ParseTextParts(uint row, const wchar_t* ConCharLine, const CharAttr* ConAttrLine)
{
	bool bHasAlternateBg = false;
	int j = 0, j0 = 0;
	TEXTPARTS *pTP = TextParts;
	bool bEnhanceGraphics = gpSet->isEnhanceGraphics;
	//bool bProportional = gpSet->isMonospace == 0;
	//bool bForceMonospace = gpSet->isMonospace == 2;
	wchar_t c = 0;
	bool isUnicode = false, isProgress = false, isSpace = false, isUnicodeOrProgress = false;
	uint nPrevForeFont = 0, nForeFont;
	// ������� - ������ ����
	BGPARTS *pBG = BgParts;
	uint nColorIdx = 0;
	uint nPrevColorIdx = ConAttrLine[0].nBackIdx;
	j = j0 = 0;

	while(j < (int)TextWidth)
	{
		while(++j < (int)TextWidth)
		{
			if (nPrevColorIdx != ConAttrLine[j].nBackIdx)
			{
				nColorIdx = ConAttrLine[j].nBackIdx;
				break;
			}
		}

		pBG->i = j0;
		pBG->n = j - j0;

		if (!(pBG->bBackIsPic = (drawImage && ISBGIMGCOLOR(nPrevColorIdx))))
			pBG->nBackRGB = ConAttrLine[j0].crBackColor;

		pBG++;
		nPrevColorIdx = nColorIdx;
		j0 = j;
	}

	// *** ������ - Foreground (text) ***
	// ����� ������ ��� �� ��������� - ��������
	j = 0;
	nPrevForeFont = ConAttrLine[0].ForeFont;

	while(j < (int)TextWidth)
	{
		j0 = j;
		const CharAttr* attr = ConAttrLine+j;
		nForeFont = attr->ForeFont;
		c = ConCharLine[j];
		TODO("������ gpSet->isFixFarBorders && gpSet->isEnhanceGraphics");
		isUnicode = isCharBorder(c);
		bool isProgress = false, isSpace = false, isUnicodeOrProgress = false;

		if (isUnicode || bEnhanceGraphics)
			isProgress = isCharProgress(c); // ucBox25 / ucBox50 / ucBox75 / ucBox100

		isUnicodeOrProgress = isUnicode || isProgress;

		if (!isUnicodeOrProgress)
			isSpace = isCharSpace(c);

		TODO("���� '����������'");

		if (isProgress)
		{
			while(++j < (int)TextWidth)
			{
				if (ConCharLine[j] != c || attr->All != ConAttrLine[j].All)
					break; // �� ������ ����� ������� ��� ����� ����-������
			}
		}
		else if (isSpace)
		{
			while(++j < (int)TextWidth)
			{
				if (!isCharSpace(ConCharLine[j]))
					break; // �� ������� ������������� �������
			}
		}
		else if (isUnicode)
		{
			COLORREF crFore = attr->crForeColor;

			while(++j < (int)TextWidth)
			{
				if (!isCharBorder(ConCharLine[j]) || crFore != ConAttrLine[j].crForeColor)
					break; // �� ������� ����������� ������� ��� ����� ����� ������
			}
		}
		else
		{
			//PRAGMA_ERROR("�������� �����");
			while(++j < (int)TextWidth)
			{
				attr = ConAttrLine+j;
				nForeFont = attr->ForeFont;
				c = ConCharLine[j];

				// ���� ����� ����� ���� ��� ������
				if (nForeFont != nPrevForeFont)
					break;

				isUnicode = isCharBorder(c);
				bool isProgress = false, isSpace = false, isUnicodeOrProgress = false;

				if (isUnicode || bEnhanceGraphics)
				{
					isProgress = isCharProgress(c); // ucBox25 / ucBox50 / ucBox75 / ucBox100
				}

				isUnicodeOrProgress = isUnicode || isProgress;

				if (!isUnicodeOrProgress)
					isSpace = isCharSpace(c);
			}
		}

		TODO("������� ���� j0..j");
	}

	return bHasAlternateBg;
}

// ������� ����� � �� �������� �����, ��������� ���������� � �����
void CVirtualConsole::Update_FillAlternate(uint row, uint nY)
{
}

// ����� ���������� ������ (��� ������������� Clipped)
void CVirtualConsole::Update_DrawText(uint row, uint nY)
{
}

void CVirtualConsole::UpdateText()
{
	//if (!updateText) {
	//    _ASSERTE(updateText);
	//    return;
	//}
#ifdef _DEBUG
	if (mp_RCon->IsConsoleThread())
	{
		//_ASSERTE(!mp_RCon->IsConsoleThread());
	}

	//// ������ ��� ������ ���� ���������, � ��� �� ������ ���� ����
	//BOOL lbDataValid = TRUE; uint n = 0;
	//while (n<TextLen) {
	//    if (mpsz_ConChar[n] == 0) {
	//        lbDataValid = FALSE; //break;
	//        mpsz_ConChar[n] = L'�';
	//        mpn_ConAttr[n] = 12;
	//    } else if (mpsz_ConChar[n] != L' ') {
	//        // 0 - ����� ���� ������ ��� �������. ����� ������ ����� �������, ���� �� ����, ���� �� ������
	//        if (mpn_ConAttr[n] == 0) {
	//            lbDataValid = FALSE; //break;
	//            mpn_ConAttr[n] = 12;
	//        }
	//    }
	//    n++;
	//}
	////_ASSERTE(lbDataValid);
#endif
	memmove(mh_FontByIndex, gpSet->mh_Font, MAX_FONT_STYLES*sizeof(mh_FontByIndex[0]));
	mh_FontByIndex[MAX_FONT_STYLES] = mh_UCharMapFont ? mh_UCharMapFont : mh_FontByIndex[0];
	SelectFont(mh_FontByIndex[0]);
	// pointers
	wchar_t* ConCharLine = NULL;
	CharAttr* ConAttrLine = NULL;
	DWORD* ConCharXLine = NULL;
	// counters
	int pos, row;
	{
		int i;
		i = 0; //TextLen - TextWidth; // TextLen = TextWidth/*80*/ * TextHeight/*25*/;
		pos = 0; //Height - nFontHeight; // Height = TextHeight * nFontHeight;
		row = 0; //TextHeight - 1;
		ConCharLine = mpsz_ConChar + i;
		ConAttrLine = mpn_ConAttrEx + i;
		ConCharXLine = ConCharX + i;
	}
	int nMaxPos = Height - nFontHeight;

	if (!ConCharLine || !ConAttrLine || !ConCharXLine)
	{
		MBoxAssert(ConCharLine && ConAttrLine && ConCharXLine);
		return;
	}


	if (/*gpSet->isForceMonospace ||*/ !drawImage)
		SetBkMode(hDC, OPAQUE);
	else
		SetBkMode(hDC, TRANSPARENT);

	int *nDX = (int*)malloc((TextWidth+1)*sizeof(int));
	// rows
	// ����� � isForceMonospace ������������� �������������� ���?
	// const bool skipNotChanged = !isForce /*&& !gpSet->isForceMonospace*/;
	const bool skipNotChanged = !isForce; // && !((gpSet->FontItalic() || gpSet->FontClearType()));
	bool bEnhanceGraphics = gpSet->isEnhanceGraphics;
	bool bProportional = gpSet->isMonospace == 0;
	bool bForceMonospace = gpSet->isMonospace == 2;
	bool bFixFarBorders = gpSet->isFixFarBorders;
	//mh_FontByIndex[0] = gpSet->mh_Font; mh_FontByIndex[1] = gpSet->mh_FontB; mh_FontByIndex[2] = gpSet->mh_FontI; mh_FontByIndex[3] = gpSet->mh_FontBI;
	HFONT hFont = gpSet->mh_Font[0];
	HFONT hFont2 = gpSet->mh_Font2;

	//BUGBUG: ������ �� ������������ ��������� ������, ���� ���� ��� ���� �� ������
	for(; pos <= nMaxPos;
	        ConCharLine += TextWidth, ConAttrLine += TextWidth, ConCharXLine += TextWidth,
	        pos += nFontHeight, row++)
	{
		//2009-09-25. ��������� (������?) ��������� ���������� �������� � ������� ������� (ASC<32)
		//            �� ����� �������� �� ��������� �������
		//{
		//	wchar_t* pszEnd = ConCharLine + TextWidth;
		//	for (wchar_t* pch = ConCharLine; pch < pszEnd; pch++) {
		//		if (((WORD)*pch) < 32) *pch = gszAnalogues[(WORD)*pch];
		//	}
		//}
		// the line
		const CharAttr* const ConAttrLine2 = mpn_ConAttrExSave + (ConAttrLine - mpn_ConAttrEx);
		const wchar_t* const ConCharLine2 = mpsz_ConCharSave + (ConCharLine - mpsz_ConChar);
		// skip not changed symbols except the old cursor or selection
		int j = 0, end = TextWidth;

		// � ������ ���������������� ������� ��� isForce==true - ������������ ����� �������
		// ��� ���������������� ��� ����������� ���, ��� ��� ����������� ���������� �����
		// ����� ��������� ������ ����������� ��������, � ���������� �������� ���������
		if (skipNotChanged)
		{
			// Skip not changed head and tail symbols
			if (!FindChanges(row, j, end, ConCharLine, ConAttrLine, ConCharLine2, ConAttrLine2))
				continue;
		} // if (skipNotChanged)

		if (Cursor.isVisiblePrev && row == Cursor.y
		        && (j <= Cursor.x && Cursor.x <= end))
			Cursor.isVisiblePrev = false;

		// *) Now draw as much as possible in a row even if some symbols are not changed.
		// More calls for the sake of fewer symbols is slower, e.g. in panel status lines.
		int j2=j+1;

		for(; j < end; j = j2)
		{
			TODO("OPTIMIZE: ������� ���������� �� �����");
			TODO("OPTIMIZE: ���� ������/������� ��������� � ���������� (j>0 !) �� �� ����� �������� CharWidth, isChar..., isUnicode, ...");
			const CharAttr attr = ConAttrLine[j];
			wchar_t c = ConCharLine[j];
			//BYTE attrForeIdx, attrBackIdx;
			//COLORREF attrForeClr, attrBackClr;
			bool isUnicode = gpSet->isFixFarBorders && isCharBorder(c/*ConCharLine[j]*/);
			bool isProgress = false, isSpace = false, isUnicodeOrProgress = false, isNonSpacing = false;
			bool lbS1 = false, lbS2 = false;
			int nS11 = 0, nS12 = 0, nS21 = 0, nS22 = 0;
			//GetCharAttr(c, attr, c, attrFore, attrBack);
			//GetCharAttr(attr, attrFore, attrBack, &hFont);
			//if (GetCharAttr(c, attr, c, attrFore, attrBack))
			//    isUnicode = true;
			hFont = mh_FontByIndex[attr.nFontIndex];

			if (isUnicode || bEnhanceGraphics)
				isProgress = isCharProgress(c); // ucBox25 / ucBox50 / ucBox75 / ucBox100

			isUnicodeOrProgress = isUnicode || isProgress;

			if (!isUnicodeOrProgress)
			{ 
				isNonSpacing = isCharNonSpacing(c);
				isSpace = isCharSpace(c);
			}

			TODO("��� ���������������� (�������� � DistrubuteSpaces) �������� ��������������� �� ����������� ���������� ������");
			// ����� ��������� �������� �������� �� ���������������� �������
			HEAPVAL

			// ������������� ���������� �������� � �����
			if (bProportional && (c==ucBoxDblHorz || c==ucBoxSinglHorz))
			{
				lbS1 = true; nS11 = nS12 = j;

				while((nS12 < end) && (ConCharLine[nS12+1] == c))
					nS12 ++;

				// ��������� ������� ���� �� �������� ����� ������
				if (nS12<end)
				{
					nS21 = nS12+1; // ��� ������ ���� �� c

					// ���� ������ "��������" ������
					while((nS21<end) && (ConCharLine[nS21] != c) && !isCharBorder(ConCharLine[nS21]))
						nS21 ++;

					if (nS21<end && ConCharLine[nS21]==c)
					{
						lbS2 = true; nS22 = nS21;

						while((nS22 < end) && (ConCharLine[nS22+1] == c))
							nS22 ++;
					}
				}
			} HEAPVAL

			// � ��� ��� �������� - ����� �� ����� ������
			/*else if (c==L' ' && j<end && ConCharLine[j+1]==L' ')
			{
			    lbS1 = true; nS11 = nS12 = j;
			    while ((nS12 < end) && (ConCharLine[nS12+1] == c))
			        nS12 ++;
			}*/
			//SetTextColor(hDC, pColors[attrFore]);
			SetTextColor(hDC, attr.crForeColor);

			// ������������� ��������� ������������ ����� (Coord.X>0)
			if (bProportional && j)
			{
				HEAPVAL;
				DWORD nPrevX = ConCharXLine[j-1];
				// ����� (�� ������������ �����) � ������ ���������;
				// �������� } ��� �������� ����� ������ �������� �� ������� �������...
				// ���� ������ ��� ����� �� ��� �� ������� ����� (��� ��� �� '}')
				// ������������ �������
				bool bBord = isCharBorderVertical(c);
				TODO("���� �� ����� ������, ��� ������ ����� ����� ���� ������ ������!");
				bool bFrame = false; // mpn_ConAttrEx[row*TextWidth+j].bDialogVBorder;

				if (bFrame || bBord || isCharScroll(c) || (isFilePanel && c==L'}'))
				{
					//2009-04-21 ���� (j-1) * nFontWidth;
					//ConCharXLine[j-1] = j * nFontWidth;
					bool bMayBeFrame = true;

					if (!bBord && !bFrame)
					{
						int R;
						wchar_t prevC;

						for(R = (row-1); bMayBeFrame && !bBord && R>=0; R--)
						{
							prevC = mpsz_ConChar[R*TextWidth+j];
							bBord = isCharBorderVertical(prevC);
							bMayBeFrame = (bBord || isCharScroll(prevC) || (isFilePanel && prevC==L'}'));
						}

						for(R = (row+1); bMayBeFrame && !bBord && R < (int)TextHeight; R++)
						{
							prevC = mpsz_ConChar[R*TextWidth+j];
							bBord = isCharBorderVertical(prevC);
							bMayBeFrame = (bBord || isCharScroll(prevC) || (isFilePanel && prevC==L'}'));
						}
					}

					if (bMayBeFrame)
						ConCharXLine[j-1] = j * nFontWidth;
				}

				//else if (isFilePanel && c==L'}') {
				//    if ((row>=2 && isCharBorderVertical(mpsz_ConChar[TextWidth+j]))
				//        && (((UINT)row)<=(TextHeight-4)))
				//        //2009-04-21 ���� (j-1) * nFontWidth;
				//        ConCharXLine[j-1] = j * nFontWidth;
				//    //row = TextHeight - 1;
				//}
				HEAPVAL;

				if (nPrevX < ConCharXLine[j-1])
				{
					// ��������� ���������� ����������� �������. ��������� ���-��?
					RECT rect;
					rect.left = nPrevX;
					rect.top = pos;
					rect.right = ConCharXLine[j-1];
					rect.bottom = rect.top + nFontHeight;

					if (gbNoDblBuffer) GdiFlush();

					// ���� �� ��������� ���� ���������
					if (!(drawImage && ISBGIMGCOLOR(attr.nBackIdx)))
					{
						//BYTE PrevAttrFore = attrFore, PrevAttrBack = attrBack;
						const CharAttr PrevAttr = ConAttrLine[j-1];
						wchar_t PrevC = ConCharLine[j-1];

						//GetCharAttr(PrevC, PrevAttr, PrevC, PrevAttrFore, PrevAttrBack);
						//GetCharAttr(PrevAttr, PrevAttrFore, PrevAttrBack, NULL);
						//if (GetCharAttr(PrevC, PrevAttr, PrevC, PrevAttrFore, PrevAttrBack))
						//	isUnicode = true;

						// ���� ������� ������ - ������������ �����, � ���������� ������ - �����
						// ����� �������� ����� �� �������� �������
						if (isCharBorderVertical(c) && isCharBorder(PrevC))
						{
							//SetBkColor(hDC, pColors[attrBack]);
							SetBkColor(hDC, attr.crBackColor);
							wchar_t *pchBorder = (c == ucBoxDblDownLeft || c == ucBoxDblUpLeft
							                      || c == ucBoxSinglDownDblHorz || c == ucBoxSinglUpDblHorz
							                      || c == ucBoxDblDownDblHorz || c == ucBoxDblUpDblHorz
												  || c == ucBoxDblVertLeft
							                      || c == ucBoxDblVertHorz) ? ms_HorzDbl : ms_HorzSingl;
							int nCnt = ((rect.right - rect.left) / CharWidth(pchBorder[0]))+1;

							if (nCnt > MAX_SPACES)
							{
								_ASSERTE(nCnt<=MAX_SPACES);
								nCnt = MAX_SPACES;
							}

							//UINT nFlags = ETO_CLIPPED; // || ETO_OPAQUE;
							//ExtTextOut(hDC, rect.left, rect.top, nFlags, &rect, Spaces, min(nSpaceCount, nCnt), 0);
							//if (! (drawImage && ISBGIMGCOLOR(attr.nBackIdx)))
							//	SetBkColor(hDC, pColors[attrBack]);
							//else if (drawImage)
							//	BlitPictureTo(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
							UINT nFlags = ETO_CLIPPED | ETO_OPAQUE;

							//wmemset(Spaces, chBorder, nCnt);
							if (bFixFarBorders)
								SelectFont(hFont2);

							SetTextColor(hDC, PrevAttr.crForeColor);
							ExtTextOut(hDC, rect.left, rect.top, nFlags, &rect, pchBorder, nCnt, 0);
							SetTextColor(hDC, attr.crForeColor); // �������
						}
						else
						{
							HBRUSH hbr = PartBrush(L' ', attr.crBackColor, attr.crForeColor);
							FillRect(hDC, &rect, hbr);
						}
					}
					else if (drawImage)
					{
						BlitPictureTo(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
					} HEAPVAL

					if (gbNoDblBuffer) GdiFlush();
				}
			}

			ConCharXLine[j] = (j ? ConCharXLine[j-1] : 0)+CharWidth(c);
			HEAPVAL

			if (bForceMonospace)
			{
				HEAPVAL
				//SetBkColor(hDC, pColors[attrBack]);
				SetBkColor(hDC, attr.crBackColor);
				j2 = j + 1;

				if (bFixFarBorders && isUnicode)
					SelectFont(hFont2);
				else
					SelectFont(hFont);

				RECT rect;

				if (!bProportional)
					rect = MakeRect(j * nFontWidth, pos, j2 * nFontWidth, pos + nFontHeight);
				else
				{
					rect.left = j ? ConCharXLine[j-1] : 0;
					rect.top = pos;
					rect.right = (TextWidth>(UINT)j2) ? ConCharXLine[j2-1] : Width;
					rect.bottom = rect.top + nFontHeight;
				}

				UINT nFlags = ETO_CLIPPED | ((drawImage && ISBGIMGCOLOR(attr.nBackIdx)) ? 0 : ETO_OPAQUE);
				int nShift = 0;
				HEAPVAL

				if (!isSpace && !isUnicodeOrProgress)
				{
					ABC abc;
					//��� �� TrueType ���������� wrapper (CharWidth)
					CharABC(c, &abc);

					if (abc.abcA<0)
					{
						// ����� ������ �������� ������� �� ����������?
						nShift = -abc.abcA;
					}
					else if (abc.abcB < (UINT)nFontWidth)
					{
						int nEdge = (nFontWidth - abc.abcB - 1) >> 1;

						if (abc.abcA < nEdge)
						{
							// ������ I, i, � ��. ����� ������ - ������ ����������
							nShift = nEdge - abc.abcA;
						}
					}
				}

				HEAPVAL
				BOOL lbImgDrawn = FALSE;

				if (!(drawImage && ISBGIMGCOLOR(attr.nBackIdx)))
				{
					//SetBkColor(hDC, pColors[attrBack]);
					SetBkColor(hDC, attr.crBackColor);

					// � ������ ForceMonospace ������� �������� �������� �� ������ (���� ��� ��� ����������)
					// ����� �� ���������� ������ �� ���������� ��������� - ����� ������ ���������� �����
					if (nShift>0 && !isSpace && !isProgress)
					{
						HBRUSH hbr = PartBrush(L' ', attr.crBackColor, attr.crForeColor);
						FillRect(hDC, &rect, hbr);
					}
				}
				else if (drawImage)
				{
					BlitPictureTo(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
					lbImgDrawn = TRUE;
				}

				if (nShift>0)
				{
					rect.left += nShift;
					// ���� ������� ��� - ��� �������� �� ������ ������� (nShift > 0)
					//rect.right += nShift;
				}

				if (gbNoDblBuffer) GdiFlush();

				if (/*isSpace ||*/ (isProgress && bEnhanceGraphics))
				{
					HBRUSH hbr = PartBrush(c, attr.crBackColor, attr.crForeColor);
					FillRect(hDC, &rect, hbr);
				}
				else if (/*gpSet->isProportional &&*/ isSpace/*c == ' '*/)
				{
					//int nCnt = ((rect.right - rect.left) / CharWidth(L' '))+1;
					//Ext Text Out(hDC, rect.left, rect.top, nFlags, &rect, Spaces, nCnt, 0);
					TODO("���������, ��� ����� ���� �������� ������ �� ������ ��� ������� ���������");

					if (!lbImgDrawn)
					{
						HBRUSH hbr = PartBrush(L' ', attr.crBackColor, attr.crForeColor);
						FillRect(hDC, &rect, hbr);
					}
				}
				else
				{
					// ��� ����� Force monospace
					if (nFontCharSet == OEM_CHARSET && !isUnicode)
					{
						char cOem = Uni2Oem(c);
						ExtTextOutA(hDC, rect.left, rect.top, nFlags, &rect, &cOem, 1, 0);
					}
					else
					{
						ExtTextOut(hDC, rect.left, rect.top, nFlags, &rect, &c, 1, 0);
					}
				}

				if (gbNoDblBuffer) GdiFlush();

				HEAPVAL
			} // end  - if (gpSet->isForceMonospace)
			else
			{
				wchar_t* pszDraw = NULL;
				int      nDrawLen = -1;
				bool     bDrawReplaced = false;
				RECT rect;

				//else if (/*gpSet->isProportional &&*/ (c==ucSpace || c==ucNoBreakSpace || c==0))
				if (isSpace)
				{
					j2 = j + 1; HEAPVAL;

					while(j2 < end && ConAttrLine[j2] == attr && isCharSpace(ConCharLine[j2]))
						j2++;

					DistributeSpaces(ConCharLine, ConAttrLine, ConCharXLine, j, j2, end);
				}
				else if (isNonSpacing)
				{
					j2 = j + 1; HEAPVAL
					wchar_t ch;
					int nLastNonSpace = -1;
					while(j2 < end && ConAttrLine[j2] == attr
					        && isCharNonSpacing(ch = ConCharLine[j2]))
					{
						ConCharXLine[j2] = (j2 ? ConCharXLine[j2-1] : 0)+CharWidth(ch);
						j2++;
					}
					SelectFont(hFont);
					HEAPVAL
				}
				else if (!isUnicodeOrProgress)
				{
					j2 = j + 1; HEAPVAL
#ifndef DRAWEACHCHAR
					// ���� ����� �� ������ - � ���������������� ������� ����� ����� �������� ���� �� ������
					wchar_t ch;
					int nLastNonSpace = -1;
					WARNING("*** �������� � ��������� �������: (!gpSet->isProportional || !isFilePanel || (ch != L'}' && ch!=L' '))");
					TODO("��� ������ �� ������ - ��������� nLastNonSpace");

					while(j2 < end && ConAttrLine[j2] == attr
					        && !isCharBorder(ch = ConCharLine[j2])
							&& !isCharNonSpacing(ch)
					        && (!bProportional || !isFilePanel || (ch != L'}' && ch!=L' '))) // ������������� ���� � ��������
					{
						ConCharXLine[j2] = (j2 ? ConCharXLine[j2-1] : 0)+CharWidth(ch);
						j2++;
					}

#endif
					TODO("�� ������ - ��������� ������� (���� ���� ���� nLastNonSpace)");
					SelectFont(hFont);
					HEAPVAL
				}
				else
				{
					// ���� �� �������� ��� ��������, ������� �������������� "��������" �������
					// ������, ��� ����� ���� �� ������ �����, �� � ����� ���� ���, ��� ��������
					// ������������ (��������� ���������� -> gpSet->icFixFarBorderRanges)
					j2 = j + 1; HEAPVAL

					// ����� 25%, 50%, 75%, 100%
					if (bEnhanceGraphics && isProgress)
					{
						wchar_t ch;
						ch = c; // ����������� ��������� ��������� � ���������

						while(j2 < end && ConAttrLine[j2] == attr && ch == ConCharLine[j2+1])
						{
							ConCharXLine[j2] = (j2 ? ConCharXLine[j2-1] : 0)+CharWidth(ch);
							j2++;
						}
					}
					else if (!bFixFarBorders)
					{
						// ���� ����� �������� �� ������ - ����� isUnicodeOrProgress ������ ���� false
						// �.�. � ��������� ��������� gpSet->isFixFarBorders
						_ASSERTE(bFixFarBorders);
						_ASSERTE(!isUnicodeOrProgress);
						wchar_t ch;

						while(j2 < end && ConAttrLine[j2] == attr && isCharBorder(ch = ConCharLine[j2]))
						{
							ConCharXLine[j2] = (j2 ? ConCharXLine[j2-1] : 0)+CharWidth(ch);
							j2++;
						}
					}
					else
					{
						//wchar_t ch;
						//WARNING("��� ������������ �� ucBoxDblVert � ������ �������. �� ������ ������ � ...");
						// ��� ��������� ����� (ucBoxDblHorz) �� ���� ������
						bool bIsHorzBorder = (c == ucBoxDblHorz || c == ucBoxSinglHorz);

						if (bIsHorzBorder)
						{
							while(j2 < end && ConAttrLine[j2] == attr && c == ConCharLine[j2])
								j2++;
						}

						DistributeSpaces(ConCharLine, ConAttrLine, ConCharXLine, j, j2, end);
						int nBorderWidth = CharWidth(c);
						rect.left = j ? ConCharXLine[j-1] : 0;
						rect.right = (TextWidth>(UINT)j2) ? ConCharXLine[j2-1] : Width;
						int nCnt = (rect.right - rect.left + (nBorderWidth>>1)) / nBorderWidth;

						if (nCnt > (j2 - j))
						{
							if (c==ucBoxDblHorz || c==ucBoxSinglHorz)
							{
								_ASSERTE(nCnt <= MAX_SPACES);

								if (nCnt > MAX_SPACES) nCnt = MAX_SPACES;

								bDrawReplaced = true;
								nDrawLen = nCnt;
								pszDraw = (c==ucBoxDblHorz) ? ms_HorzDbl : ms_HorzSingl;
							}

#ifdef _DEBUG
							else
							{
								//static bool bShowAssert = true;
								//if (bShowAssert) {
								_ASSERTE(!bIsHorzBorder || (c==ucBoxDblHorz || c==ucBoxSinglHorz));
								//}
							}

#endif
						}

						//while(j2 < end && ConAttrLine[j2] == attr &&
						//    isCharBorder(ch = ConCharLine[j2]) && ch == ConCharLine[j2+1])
						//{
						//    ConCharXLine[j2] = (j2 ? ConCharXLine[j2-1] : 0)+CharWidth(ch);
						//    j2++;
						//}
					}

					SelectFont(bFixFarBorders ? hFont2 : hFont);
					HEAPVAL
				}

				if (!pszDraw)
				{
					pszDraw = ConCharLine + j;
					nDrawLen = j2 - j;
				}

				//if (!gpSet->isProportional) {
				//	TODO("���-�� ���-��... ���� ��������� ��� ����� ���������?");
				//  rect = MakeRect(j * nFontWidth, pos, j2 * nFontWidth, pos + nFontHeight);
				//} else {
				rect.left = j ? ConCharXLine[j-1] : 0;
				rect.top = pos;
				rect.right = (TextWidth>(UINT)j2) ? ConCharXLine[j2-1] : Width;
				rect.bottom = rect.top + nFontHeight;
				//}
				HEAPVAL
				BOOL lbImgDrawn = FALSE;

				if (!(drawImage && ISBGIMGCOLOR(attr.nBackIdx)))
				{
					//SetBkColor(hDC, pColors[attrBack]);
					SetBkColor(hDC, attr.crBackColor);
				}
				else if (drawImage)
				{
					BlitPictureTo(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
					lbImgDrawn = TRUE;
				}

				WARNING("ETO_CLIPPED ����� ������ �� �������� ������, ���� ��� ����� �� �������?");
				UINT nFlags = ETO_CLIPPED | ((drawImage && ISBGIMGCOLOR(attr.nBackIdx)) ? 0 : ETO_OPAQUE);
				HEAPVAL

				if (gbNoDblBuffer) GdiFlush();

				if (isProgress && bEnhanceGraphics)
				{
					HBRUSH hbr = PartBrush(c, attr.crBackColor, attr.crForeColor);
					FillRect(hDC, &rect, hbr);
				}
				else if (/*gpSet->isProportional &&*/ isSpace/*c == ' '*/)
				{
					//int nCnt = ((rect.right - rect.left) / CharWidth(L' '))+1;
					//Ext Text Out(hDC, rect.left, rect.top, nFlags, &rect, Spaces, nCnt, 0);
					TODO("���������, ��� ����� ���� �������� ������ �� ������ ��� ������� ���������");

					if (!lbImgDrawn)
					{
						HBRUSH hbr = PartBrush(L' ', attr.crBackColor, attr.crForeColor);
						FillRect(hDC, &rect, hbr);
					}
				}
				else
				{
					// � ����� ����������� ������� �������� bProportional �� ������� �����
					/*if (bProportional) {

						if (!isSpace && !isUnicodeOrProgress) {
							ABC abc;
							//��� �� TrueType ���������� wrapper (CharWidth)
							CharABC(c, &abc);

							if (abc.abcA<0) {
								// ����� ������ �������� ������� �� ����������?
								nShift = -abc.abcA;

							} else if (abc.abcB < (UINT)nFontWidth)  {
								int nEdge = (nFontWidth - abc.abcB - 1) >> 1;
								if (abc.abcA < nEdge) {
									// ������ I, i, � ��. ����� ������ - ������ ����������
									nShift = nEdge - abc.abcA;
								}
							}
						}

					} else {
					}*/
					if (nFontCharSet == OEM_CHARSET && !isUnicode)
					{
						if (nDrawLen > (int)TextWidth)
						{
							_ASSERTE(nDrawLen <= (int)TextWidth);
							nDrawLen = TextWidth;
						}

						WideCharToMultiByte(CP_OEMCP, 0, pszDraw, nDrawLen, tmpOem, TextWidth, 0, 0);

						if (!bProportional)
							for(int idx = 0; idx < nDrawLen; idx++)
							{
								WARNING("BUGBUG: ��� ������ ����� ���������� ��� ��������� ������ OEM �������?");
								nDX[idx] = CharWidth(tmpOem[idx]);
							}

						ExtTextOutA(hDC, rect.left, rect.top, nFlags,
						            &rect, tmpOem, nDrawLen, bProportional ? 0 : nDX);
					}
					else
					{
						if (nDrawLen == 1)  // support visualizer change
							ExtTextOut(hDC, rect.left, rect.top, nFlags, &rect,
							           &c/*ConCharLine + j*/, 1, 0);
						else
						{
							if (!bProportional)
								for(int idx = 0, n = nDrawLen; n; idx++, n--)
									nDX[idx] = CharWidth(pszDraw[idx]);

							// nDX ��� ������ �� ������ ���������� �������, � ������ �����������
							ExtTextOut(hDC, rect.left, rect.top, nFlags, &rect,
							           pszDraw, nDrawLen, bProportional ? 0 : nDX);
						}
					}
				}

				if (gbNoDblBuffer) GdiFlush();

				HEAPVAL
			}

			DUMPDC(L"F:\\vcon.png");
			//-- �� ��������� ������� � ����������������� �������� � ClearType
			//-- ������ ���������� ������ �������
			//// skip next not changed symbols again
			//if (skipNotChanged)
			//{
			//    HEAPVAL
			//	wchar_t ch;
			//    // skip the same except spaces
			//    while(j2 < end && (ch = ConCharLine[j2]) == ConCharLine2[j2] && ConAttrLine[j2] == ConAttrLine2[j2]
			//		&& (ch != ucSpace && ch != ucNoBreakSpace && ch != 0))
			//    {
			//        ++j2;
			//    }
			//}
		}
	}

	free(nDX);

	return;
}

void CVirtualConsole::ClearPartBrushes()
{
	_ASSERTE(gpConEmu->isMainThread());

	for(UINT br=0; br<MAX_COUNT_PART_BRUSHES; br++)
	{
		DeleteObject(m_PartBrushes[br].hBrush);
	}

	memset(m_PartBrushes, 0, sizeof(m_PartBrushes));
}

HBRUSH CVirtualConsole::PartBrush(wchar_t ch, COLORREF nBackCol, COLORREF nForeCol)
{
	_ASSERTE(gpConEmu->isMainThread());
	//std::vector<PARTBRUSHES>::iterator iter = m_PartBrushes.begin();
	//while (iter != m_PartBrushes.end()) {
	PARTBRUSHES *pbr = m_PartBrushes;

	for(UINT br=0; pbr->ch && br<MAX_COUNT_PART_BRUSHES; br++, pbr++)
	{
		if (pbr->ch == ch && pbr->nBackCol == nBackCol && pbr->nForeCol == nForeCol)
		{
			_ASSERTE(pbr->hBrush);
			return pbr->hBrush;
		}
	}

	MYRGB clrBack, clrFore, clrMy;
	clrBack.color = nBackCol;
	clrFore.color = nForeCol;
	clrMy.color = clrFore.color; // 100 %
	////#define   PART_75(f,b) ((((int)f) + ((int)b)*3) / 4)
	////#define   PART_50(f,b) ((((int)f) + ((int)b)) / 2)
	////#define   PART_25(f,b) (((3*(int)f) + ((int)b)) / 4)
	////#define   PART_75(f,b) (b + 0.80*(f-b))
	////#define   PART_50(f,b) (b + 0.75*(f-b))
	////#define   PART_25(f,b) (b + 0.50*(f-b))
	// (gpSet->isPartBrushXX >> 8) ���� ��� �� ����� (0 .. 1), ��������, 0.80
#define PART_XX(f,b,p) ((f==b) ? f : ( (b + ((p*(f-b))>>8))))
#define PART_75(f,b) ((f>b) ? PART_XX(f,b,gpSet->isPartBrush75) : PART_XX(b,f,gpSet->isPartBrush25))
#define PART_50(f,b) ((f>b) ? PART_XX(f,b,gpSet->isPartBrush50) : PART_XX(b,f,gpSet->isPartBrush50))
#define PART_25(f,b) ((f>b) ? PART_XX(f,b,gpSet->isPartBrush25) : PART_XX(b,f,gpSet->isPartBrush75))

	if (ch == ucBox75 || ch == ucBox50 || ch == ucBox25)
	{
		// ��� ������ ���������. ���� 25% ������ �� ���������� �� ������������ 100% (������� ������)
		if (clrBack.color == 0)
		{
			if (gpSet->isPartBrushBlack
			        && clrFore.rgbRed > gpSet->isPartBrushBlack
			        && clrFore.rgbGreen > gpSet->isPartBrushBlack
			        && clrFore.rgbBlue > gpSet->isPartBrushBlack)
			{
				clrBack.rgbBlue = clrBack.rgbGreen = clrBack.rgbRed = gpSet->isPartBrushBlack;
			}
		}
		else if (clrFore.color == 0)
		{
			if (gpSet->isPartBrushBlack
			        && clrBack.rgbRed > gpSet->isPartBrushBlack
			        && clrBack.rgbGreen > gpSet->isPartBrushBlack
			        && clrBack.rgbBlue > gpSet->isPartBrushBlack)
			{
				clrFore.rgbBlue = clrFore.rgbGreen = clrFore.rgbRed = gpSet->isPartBrushBlack;
			}
		}
	}

	if (ch == ucBox75 /* 75% */)
	{
		clrMy.rgbRed = PART_75(clrFore.rgbRed,clrBack.rgbRed);
		clrMy.rgbGreen = PART_75(clrFore.rgbGreen,clrBack.rgbGreen);
		clrMy.rgbBlue = PART_75(clrFore.rgbBlue,clrBack.rgbBlue);
		clrMy.rgbReserved = 0;
	}
	else if (ch == ucBox50 /* 50% */)
	{
		clrMy.rgbRed = PART_50(clrFore.rgbRed,clrBack.rgbRed);
		clrMy.rgbGreen = PART_50(clrFore.rgbGreen,clrBack.rgbGreen);
		clrMy.rgbBlue = PART_50(clrFore.rgbBlue,clrBack.rgbBlue);
		clrMy.rgbReserved = 0;
	}
	else if (ch == ucBox25 /* 25% */)
	{
		clrMy.rgbRed = PART_25(clrFore.rgbRed,clrBack.rgbRed);
		clrMy.rgbGreen = PART_25(clrFore.rgbGreen,clrBack.rgbGreen);
		clrMy.rgbBlue = PART_25(clrFore.rgbBlue,clrBack.rgbBlue);
		clrMy.rgbReserved = 0;
	}
	else if (ch == ucSpace || ch == ucNoBreakSpace /* Non breaking space */ || !ch)
	{
		clrMy.color = clrBack.color;
	}

	PARTBRUSHES pb;
	pb.ch = ch; pb.nBackCol = nBackCol; pb.nForeCol = nForeCol;
	pb.hBrush = CreateSolidBrush(clrMy.color);
	//m_PartBrushes.push_back(pb);
	*pbr = pb;
	return pb.hBrush;
}

void CVirtualConsole::UpdateCursorDraw(HDC hPaintDC, RECT rcClient, COORD pos, UINT dwSize)
{
	Cursor.x = csbi.dwCursorPosition.X;
	Cursor.y = csbi.dwCursorPosition.Y;
	int CurChar = pos.Y * TextWidth + pos.X;

	if (CurChar < 0 || CurChar>=(int)(TextWidth * TextHeight))
	{
		return; // ����� ���� ��� ���� - ��� ������ ������� ��� ����� �������� � ���������� ������� ������� ������� � ������
	}

	if (!ConCharX)
	{
		return; // ������
	}

	COORD pix;
	pix.X = pos.X * nFontWidth;
	pix.Y = pos.Y * nFontHeight;

	if (pos.X && ConCharX[CurChar-1])
		pix.X = ConCharX[CurChar-1];

	RECT rect;
	bool bForeground = gpConEmu->isMeForeground();

	if (!bForeground && gpSet->isCursorBlockInactive)
	{
		dwSize = 100;
		rect.left = pix.X; /*Cursor.x * nFontWidth;*/
		rect.right = pix.X + nFontWidth; /*(Cursor.x+1) * nFontWidth;*/ //TODO: � ���� ������� ���������� ������� ��������!
		rect.bottom = (pos.Y+1) * nFontHeight;
		rect.top = (pos.Y * nFontHeight) /*+ 1*/;
	}
	else if (!gpSet->isCursorV)
	{
		if (!gpSet->isMonospace)
		{
			rect.left = pix.X; /*Cursor.x * nFontWidth;*/
			rect.right = pix.X + nFontWidth; /*(Cursor.x+1) * nFontWidth;*/ //TODO: � ���� ������� ���������� ������� ��������!
		}
		else
		{
			rect.left = pos.X * nFontWidth;
			rect.right = (pos.X+1) * nFontWidth;
		}

		//rect.top = (Cursor.y+1) * nFontHeight - MulDiv(nFontHeight, cinf.dwSize, 100);
		rect.bottom = (pos.Y+1) * nFontHeight;
		rect.top = (pos.Y * nFontHeight) /*+ 1*/;
		//if (cinf.dwSize<50)
		int nHeight = 0;

		if (dwSize)
		{
			nHeight = MulDiv(nFontHeight, dwSize, 100);

			if (nHeight < HCURSORHEIGHT) nHeight = HCURSORHEIGHT;
		}

		//if (nHeight < HCURSORHEIGHT) nHeight = HCURSORHEIGHT;
		rect.top = max(rect.top, (rect.bottom-nHeight));
	}
	else
	{
		if (!gpSet->isMonospace)
		{
			rect.left = pix.X; /*Cursor.x * nFontWidth;*/
			//rect.right = rect.left/*Cursor.x * nFontWidth*/ //TODO: � ���� ������� ���������� ������� ��������!
			//  + klMax(1, MulDiv(nFontWidth, cinf.dwSize, 100)
			//  + (cinf.dwSize > 10 ? 1 : 0));
		}
		else
		{
			rect.left = pos.X * nFontWidth;
			//rect.right = Cursor.x * nFontWidth
			//  + klMax(1, MulDiv(nFontWidth, cinf.dwSize, 100)
			//  + (cinf.dwSize > 10 ? 1 : 0));
		}

		rect.top = pos.Y * nFontHeight;
		int nR = (!gpSet->isMonospace && ConCharX[CurChar]) // ������ �������
		         ? ConCharX[CurChar] : ((pos.X+1) * nFontWidth);
		//if (cinf.dwSize>=50)
		//  rect.right = nR;
		//else
		//  rect.right = min(nR, (rect.left+VCURSORWIDTH));
		int nWidth = 0;

		if (dwSize)
		{
			nWidth = MulDiv((nR - rect.left), dwSize, 100);

			if (nWidth < VCURSORWIDTH) nWidth = VCURSORWIDTH;
		}

		rect.right = min(nR, (rect.left+nWidth));
		//rect.right = rect.left/*Cursor.x * nFontWidth*/ //TODO: � ���� ������� ���������� ������� ��������!
		//      + klMax(1, MulDiv(nFontWidth, cinf.dwSize, 100)
		//      + (cinf.dwSize > 10 ? 1 : 0));
		rect.bottom = (pos.Y+1) * nFontHeight;
	}

	//if (!gpSet->isCursorBlink) {
	//	gpConEmu->m_Child->SetCaret ( 1, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top );
	//	return;
	//} else {
	//	gpConEmu->m_Child->SetCaret ( -1 ); // ���� ��� ������ ��������� ������ - �� ����������
	//}
	rect.left += rcClient.left;
	rect.right += rcClient.left;
	rect.top += rcClient.top;
	rect.bottom += rcClient.top;
	// ���� ������ �������� ����� 40% ������� - ������������� �������
	// XOR �����, ����� (��� ����� ��� ����������) ������ ���������
	// ���� ������ � ��� �� �����
	// 110131 ���� ������ �������� - �������� ��������� ������ ��� AltIns � ����
	bool bCursorColor = gpSet->isCursorColor || (dwSize >= 40 && !gpSet->isCursorBlink);

	// ������ � rect ����� ���������� ������ (XOR'�� ���������?)
	if (bCursorColor)
	{
		HBRUSH hBr = CreateSolidBrush(0xC0C0C0);
		HBRUSH hOld = (HBRUSH)SelectObject(hPaintDC, hBr);

		if (bForeground || !gpSet->isCursorBlockInactive)
		{
			BitBlt(hPaintDC, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, hDC, 0,0,
			       PATINVERT);
		}
		else
		{
			// ����� CE �� � ������ - ����� ������ ������������� ��������
			BitBlt(hPaintDC, rect.left, rect.top, 1, rect.bottom-rect.top, hDC, 0,0,
			       PATINVERT);
			BitBlt(hPaintDC, rect.left, rect.top, rect.right-rect.left, 1, hDC, 0,0,
			       PATINVERT);
			BitBlt(hPaintDC, rect.right-1, rect.top, 1, rect.bottom-rect.top, hDC, 0,0,
			       PATINVERT);
			BitBlt(hPaintDC, rect.left, rect.bottom-1, rect.right-rect.left, 1, hDC, 0,0,
			       PATINVERT);
		}

		SelectObject(hPaintDC, hOld);
		DeleteObject(hBr);
		return;
	}

	//lbDark = Cursor.foreColorNum < 5; // ���� ������
	//BOOL lbIsProgr = isCharProgress(Cursor.ch[0]);
	BOOL lbDark = FALSE;
	DWORD clr = (Cursor.ch != ucBox100 && Cursor.ch != ucBox75) ? Cursor.bgColor : Cursor.foreColor;
	BYTE R = (clr & 0xFF);
	BYTE G = (clr & 0xFF00) >> 8;
	BYTE B = (clr & 0xFF0000) >> 16;
	lbDark = (R <= 0xC0) && (G <= 0xC0) && (B <= 0xC0);
	clr = lbDark ? mp_Colors[15] : mp_Colors[0];
	HBRUSH hBr = CreateSolidBrush(clr);
	FillRect(hPaintDC, &rect, hBr);
	DeleteObject(hBr);
}

void CVirtualConsole::UpdateCursor(bool& lRes)
{
	//------------------------------------------------------------------------
	///| Drawing cursor |/////////////////////////////////////////////////////
	//------------------------------------------------------------------------
	Cursor.isVisiblePrevFromInfo = cinf.bVisible;
	BOOL lbUpdateTick = FALSE;
	bool bForeground = gpConEmu->isMeForeground();
	bool bConActive = gpConEmu->isActive(this); // � ��� ������ Active, �.�. ������ ������ ������ � �������� �������
	//if (bConActive) {
	//	bForeground = gpConEmu->isMeForeground();
	//}

	// ���� ������ (� �������) �����, � ��������� � ������� ������� (��� ���������)
	if (cinf.bVisible && isCursorValid)
	{
		if (!gpSet->isCursorBlink || !bForeground)
		{
			Cursor.isVisible = true; // ����� ������ (���� � ���������� �������), �� ������

			if (Cursor.isPrevBackground == bForeground)
			{
				lRes = true;
			}
		}
		else
		{
			// ����� ������� ������� - ��� ����� ��������, ���� ���� �������
			if ((Cursor.x != csbi.dwCursorPosition.X) || (Cursor.y != csbi.dwCursorPosition.Y))
			{
				Cursor.isVisible = bConActive;

				if (Cursor.isVisible) lRes = true;  //force, pos changed

				lbUpdateTick = TRUE;
			}
			else

				// ������� ����� �������
				if ((GetTickCount() - Cursor.nLastBlink) > Cursor.nBlinkTime)
				{
					if (gpConEmu->isRightClickingPaint())
					{
						// ������ ������ ������ �����, ������ ��������� ��������
						Cursor.isVisible = false;
					}
					else if (Cursor.isPrevBackground && bForeground)
					{
						// ����� "�����������" ������� - ����� �������� ��������, ������ ����� - ������
						Cursor.isVisible = true;
						lRes = true;
					}
					else
					{
						Cursor.isVisible = bConActive && !Cursor.isVisible;
					}

					lbUpdateTick = TRUE;
				}
		}
	}
	else
	{
		// ����� - ��� ����� �������� (������ ����� � �������, ��� ���� �� ������� ������)
		Cursor.isVisible = false;
	}

	// ����� ��������� ����� � ConEmu
	if (Cursor.isVisible != Cursor.isVisiblePrev && !gpConEmu->isRightClickingPaint())
	{
		lRes = true;
		lbUpdateTick = TRUE;
	}

	if (Cursor.isVisible)
		Cursor.isPrevBackground = !bForeground;

	// �������� ����� ���������
	//Cursor.x = csbi.dwCursorPosition.X;
	//Cursor.y = csbi.dwCursorPosition.Y;
	Cursor.isVisiblePrev = Cursor.isVisible;

	if (lbUpdateTick)
		Cursor.nLastBlink = GetTickCount();
}


LPVOID CVirtualConsole::Alloc(size_t nCount, size_t nSize)
{
#ifdef _DEBUG
	//HeapValidate(mh_Heap, 0, NULL);
#endif
	size_t nWhole = nCount * nSize;
	LPVOID ptr = HeapAlloc(mh_Heap, HEAP_GENERATE_EXCEPTIONS|HEAP_ZERO_MEMORY, nWhole);
#ifdef _DEBUG
	//HeapValidate(mh_Heap, 0, NULL);
#endif
	return ptr;
}

void CVirtualConsole::Free(LPVOID ptr)
{
	if (ptr && mh_Heap)
	{
#ifdef _DEBUG
		//HeapValidate(mh_Heap, 0, NULL);
#endif
		HeapFree(mh_Heap, 0, ptr);
#ifdef _DEBUG
		//HeapValidate(mh_Heap, 0, NULL);
#endif
	}
}


// hdc - ��� DC ������������� ���� (ghWnd)
// rcClient - ��� �����, ���� ����� "��������" ������. ����� ���� ������������!
void CVirtualConsole::Paint(HDC hPaintDc, RECT rcClient)
{
	// ���� "�����" PostUpdate
	if (gpConEmu->mp_TabBar->NeedPostUpdate())
		gpConEmu->mp_TabBar->Update();

	if (!mh_WndDC)
	{
		_ASSERTE(mh_WndDC!=NULL);
		return;
	}

	_ASSERTE(hPaintDc);
	_ASSERTE(rcClient.left!=rcClient.right && rcClient.top!=rcClient.bottom);
//#ifdef _DEBUG
//    if (this) {
//        if (!mp_RCon || !mp_RCon->isPackets()) {
//        	DEBUGSTRDRAW(L"*** Painting ***\n");
//        } else {
//            DEBUGSTRDRAW(L"*** Painting (!!! Non processed packets are queued !!!) ***\n");
//        }
//    }
//#endif
	BOOL lbSimpleBlack = FALSE;

	if (!this)
		lbSimpleBlack = TRUE;
	else if (!mp_RCon)
		lbSimpleBlack = TRUE;
	else if (!mp_RCon->ConWnd())
		lbSimpleBlack = TRUE;

	//else if (!mpsz_ConChar || !mpn_ConAttrEx)
	//	lbSimpleBlack = TRUE;
	if (lbSimpleBlack)
	{
		//RECT rcWndClient; GetClientRect(ghWnd, &rcWndClient);
		//RECT rcCalcCon = gpConEmu->CalcRect(CER_BACK, rcWndClient, CER_MAINCLIENT);
		//RECT rcCon = gpConEmu->CalcRect(CER_CONSOLE, rcCalcCon, CER_BACK);
		//rcClient = gpConEmu->CalcRect(CER_BACK, rcCon, CER_CONSOLE);
		GetClientRect(mh_WndDC, &rcClient);
		// ������ ������ 0
#ifdef _DEBUG
		int nBackColorIdx = 2; // Green
#else
		int nBackColorIdx = 0; // Black
#endif
		COLORREF *pColors = gpSet->GetColors();
		HBRUSH hBr = CreateSolidBrush(pColors[nBackColorIdx]);
		//RECT rcClient; GetClientRect('ghWnd DC', &rcClient);
		//PAINTSTRUCT ps;
		//HDC hPaintDc = BeginPaint('ghWnd DC', &ps);
#ifndef SKIP_ALL_FILLRECT
		FillRect(hPaintDc, &rcClient, hBr);
#endif
		HFONT hOldF = (HFONT)SelectObject(hPaintDc, gpSet->mh_Font[0]);
		LPCWSTR pszStarting = L"Initializing ConEmu.";
		
		if (gpConEmu->isProcessCreated())
		{
			pszStarting = L"No consoles";
		}
		else if (CRealConsole::ms_LastRConStatus[0])
		{
			pszStarting = CRealConsole::ms_LastRConStatus;
		}

		if (this)
		{
			if (mp_RCon)
				pszStarting = mp_RCon->GetConStatus();
		}

		if (pszStarting != NULL)
		{
			UINT nFlags = ETO_CLIPPED;
			SetTextColor(hPaintDc, pColors[7]);
			SetBkColor(hPaintDc, pColors[0]);
			ExtTextOut(hPaintDc, rcClient.left, rcClient.top, nFlags, &rcClient,
			           pszStarting, _tcslen(pszStarting), 0);
			SelectObject(hPaintDc, hOldF);
			DeleteObject(hBr);
		}
		//EndPaint('ghWnd DC', &ps);
		return;
	}

	if (gpConEmu->isActive(this))
		gpSet->Performance(tPerfFPS, TRUE); // ��������� �� ������

	// ���������, �� ��������� �� �������� � ���������� ����
	DWORD nBits = GetDeviceCaps(hPaintDc, BITSPIXEL);

	if (mn_LastBitsPixel != nBits)
	{
		mb_RequiredForceUpdate = true;
	}

	mb_InPaintCall = TRUE;

	if (gbNoDblBuffer)
		Update(true, &hPaintDc);
	else
		Update(mb_RequiredForceUpdate);

	mb_InPaintCall = FALSE;
	BOOL lbExcept = FALSE;
	RECT client = rcClient;
	//PAINTSTRUCT ps;
	//HDC hPaintDc = NULL;

	//GetClientRect('ghWnd DC', &client);

	if (!gpConEmu->isNtvdm())
	{
		// ����� ������ � �������� ����� ���� �������� � �������� ���� DC
		if ((client.right-client.left) < (LONG)Width || (client.bottom-client.top) < (LONG)Height)
		{
			WARNING("�������������. �������� Paint, ������� �������� OnSize. � ����� - 100% �������� ����.");
			gpConEmu->OnSize(-1); // ������ ������ �������� ����
		}
	}

	MSectionLock S; //&csDC, &ncsTDC);
	//RECT rcUpdateRect = {0};
	//BOOL lbInval = GetUpdateRect('ghWnd DC', &rcUpdateRect, FALSE);
	//if (lbInval)
	//  hPaintDc = BeginPaint('ghWnd DC', &ps);
	//else
	//	hPaintDc = GetDC('ghWnd DC');
	// ���� ���� ������ �������� DC - ������ ���� (������/�����) ������� ������
	HBRUSH hBr = NULL;

	if (((ULONG)(client.right-client.left)) > Width)
	{
		if (!hBr) hBr = CreateSolidBrush(mp_Colors[mn_BackColorIdx]);

		RECT rcFill = MakeRect(client.left+Width, client.top, client.right, client.bottom);
#ifndef SKIP_ALL_FILLRECT
		FillRect(hPaintDc, &rcFill, hBr);
#endif
		client.right = client.left+Width;
	}

	if (((ULONG)(client.bottom-client.top)) > Height)
	{
		if (!hBr) hBr = CreateSolidBrush(mp_Colors[mn_BackColorIdx]);

		RECT rcFill = MakeRect(client.left, client.top+Height, client.right, client.bottom);
#ifndef SKIP_ALL_FILLRECT
		FillRect(hPaintDc, &rcFill, hBr);
#endif
		client.bottom = client.top+Height;
	}

	if (hBr) { DeleteObject(hBr); hBr = NULL; }

	BOOL lbPaintLocked = FALSE;

	if (!mb_PaintRequested)    // ���� Paint ������� �� �� Update (� �������� ������, ��� ��� ����� ������� ���-�� ���������).
	{
		if (S.Lock(&csDC, 200))  // �� �� ��������, ����� �� ������� ���������
			mb_PaintLocked = lbPaintLocked = TRUE;
	}

	//bool bFading = false;
	bool lbLeftExists = (m_LeftPanelView.hWnd && IsWindowVisible(m_LeftPanelView.hWnd));

	if (lbLeftExists && !mb_LeftPanelRedraw)
	{
		DWORD n = GetWindowLong(m_LeftPanelView.hWnd, 16*4); //-V112

		if (n != (isFade ? 2 : 1)) mb_LeftPanelRedraw = TRUE;
	}

	bool lbRightExists = (m_RightPanelView.hWnd && IsWindowVisible(m_RightPanelView.hWnd));

	if (lbRightExists && !mb_RightPanelRedraw)
	{
		DWORD n = GetWindowLong(m_RightPanelView.hWnd, 16*4); //-V112

		if (n != (isFade ? 2 : 1)) mb_RightPanelRedraw = TRUE;
	}

	if (mb_LeftPanelRedraw)
	{
		UpdatePanelView(TRUE); mb_LeftPanelRedraw = FALSE;

		if (lbLeftExists)
		{
			InvalidateRect(m_LeftPanelView.hWnd, NULL, FALSE);
		}
	}
	else if (mb_DialogsChanged)
	{
		UpdatePanelRgn(TRUE, FALSE, FALSE);
	}

	if (mb_RightPanelRedraw)
	{
		UpdatePanelView(FALSE); mb_RightPanelRedraw = FALSE;

		if (lbRightExists)
		{
			InvalidateRect(m_RightPanelView.hWnd, NULL, FALSE);
		}
	}
	else if (mb_DialogsChanged)
	{
		UpdatePanelRgn(FALSE, FALSE, FALSE);
	}

	// ����������, ����������� �������� bitmap
	if (!gbNoDblBuffer)
	{
		// ������� �����
		if (gpSet->isAdvLogging>=3) mp_RCon->LogString("Blitting to Display");

		/*if (gpSet->isFadeInactive && !gpConEmu->isMeForeground()) {
			// Fade-effect ����� CE �� � ������
			DWORD dwFadeColor = gpSet->nFadeInactiveMask; //0xC0C0C0;
			// �������� ������� ����� Colors[7] & Colors[8] - ���������

			bFading = true;

			HBRUSH hBr = CreateSolidBrush(dwFadeColor);
			HBRUSH hOld = (HBRUSH)SelectObject ( hPaintDc, hBr );
			DWORD dwEffect = MERGECOPY;

			BitBlt(hPaintDc, client.left, client.top, client.right-client.left, client.bottom-client.top, hDC, 0, 0,
				dwEffect);

			SelectObject ( hPaintDc, hOld );
			DeleteObject ( hBr );

		} else {*/

		BOOL lbBltRc = BitBlt(
				hPaintDc, client.left, client.top, client.right-client.left, client.bottom-client.top,
				hDC, 0, 0,
				SRCCOPY);

		//#ifdef _DEBUG
		//MoveToEx(hPaintDc, client.left, client.top, NULL);
		//LineTo(hPaintDc, client.right-client.left, client.bottom-client.top);
		//FillRect(hPaintDc, &rcClient, (HBRUSH)GetStockObject(WHITE_BRUSH));
		//#endif

		/*}*/

		if (gpSet->isUserScreenTransparent)
		{
			if (CheckTransparentRgn())
				gpConEmu->UpdateWindowRgn();
		}
	}
	else
	{
		GdiSetBatchLimit(1); // ��������� ����������� ������ ��� ������� ����
		GdiFlush();
		// ������ ����� �� �������, ��� �����������
		// ��� ��� ������� ���������, ������� ����������� ���� � ������
		_ASSERTE(gpSet->isTabs == 0);
		Update(true, &hPaintDc);
	}

	// ��������� ��������� �������� �������
	mn_LastBitsPixel = nBits;
	//mb_LastFadeFlag = bFading;
	S.Unlock();

	if (lbPaintLocked)
		mb_PaintLocked = FALSE;

	// ����� ������� ������ ������ ������ � ����
	//UpdateCursor(hPaintDc);

	if (gbNoDblBuffer) GdiSetBatchLimit(0);  // ������� ����������� �����

	if (Cursor.isVisible && cinf.bVisible && isCursorValid)
	{
		if (mpsz_ConChar && mpn_ConAttrEx)
		{
			HFONT hOldFont = (HFONT)SelectObject(hPaintDc, gpSet->mh_Font[0]);
			MSectionLock SCON; SCON.Lock(&csCON);

			int CurChar = csbi.dwCursorPosition.Y * TextWidth + csbi.dwCursorPosition.X;
			//Cursor.ch[1] = 0;
			//CVirtualConsole* p = this;
			//GetCharAttr(mpsz_ConChar[CurChar], mpn_ConAttr[CurChar], Cursor.ch[0], Cursor.foreColorNum, Cursor.bgColorNum);
			Cursor.ch = mpsz_ConChar[CurChar];
			//GetCharAttr(mpn_ConAttr[CurChar], Cursor.foreColorNum, Cursor.bgColorNum, NULL);
			//Cursor.foreColor = pColors[Cursor.foreColorNum];
			//Cursor.bgColor = pColors[Cursor.bgColorNum];
			Cursor.foreColor = mpn_ConAttrEx[CurChar].crForeColor;
			Cursor.bgColor = mpn_ConAttrEx[CurChar].crBackColor;

			UpdateCursorDraw(hPaintDc, rcClient, csbi.dwCursorPosition, cinf.dwSize);
			Cursor.isVisiblePrev = Cursor.isVisible;
			SelectObject(hPaintDc, hOldFont);
			SCON.Unlock();
		}
	}

	//#ifdef _DEBUG
	if (mp_RCon)
	{
		#ifdef DEBUGDRAW_RCONPOS
		if (GetKeyState(DEBUGDRAW_RCONPOS) & 1)
		{
			// �������������, ������������� ��������� ���� RealConsole
			HWND hConWnd = mp_RCon->hConWnd;
			RECT rcCon; GetWindowRect(hConWnd, &rcCon);
			MapWindowPoints(NULL, GetView(), (LPPOINT)&rcCon, 2);
			SelectObject(hPaintDc, GetStockObject(WHITE_PEN));
			SelectObject(hPaintDc, GetStockObject(HOLLOW_BRUSH));
			Rectangle(hPaintDc, rcCon.left, rcCon.top, rcCon.right, rcCon.bottom);
		}
		#endif

		if (gbDebugShowRects
				#ifdef DEBUGDRAW_DIALOGS
		        || (GetKeyState(DEBUGDRAW_DIALOGS) & 1)
				#endif
		  )
		{
			// �������������� ��������� ��������
			//SMALL_RECT rcFound[MAX_DETECTED_DIALOGS]; DWORD nDlgFlags[MAX_DETECTED_DIALOGS];
			//int nFound = mp_RCon->GetDetectedDialogs(MAX_DETECTED_DIALOGS, rcFound, nDlgFlags);
			const DetectedDialogs* pDlg = NULL;
			// ���� �������� PanelView - ����������� ����� ������� � ����
			//#ifdef _DEBUG
			MFileMapping<DetectedDialogs> pvMap;

			if (m_LeftPanelView.bRegister || m_RightPanelView.bRegister)
			{
				pvMap.InitName(CEPANELDLGMAPNAME, mp_RCon->GetFarPID(TRUE));
				pDlg = pvMap.Open();
			}
			//#endif

			if (!pDlg) pDlg = mp_RCon->GetDetector()->GetDetectedDialogsPtr();

			// �������
			HPEN hFrame = CreatePen(PS_SOLID, 1, RGB(255,0,255));
			HPEN hPanel = CreatePen(PS_SOLID, 2, RGB(255,255,0));
			HPEN hQSearch = CreatePen(PS_SOLID, 1, RGB(255,255,0));
			HPEN hUCharMap = CreatePen(PS_SOLID, 1, RGB(255,255,0));
			HPEN hActiveMenu = (HPEN)GetStockObject(WHITE_PEN);
			HPEN hOldPen = (HPEN)SelectObject(hPaintDc, hFrame);
			HBRUSH hOldBr = (HBRUSH)SelectObject(hPaintDc, GetStockObject(HOLLOW_BRUSH));
			HPEN hRed = CreatePen(PS_SOLID, 1, 255);
			HPEN hDash = CreatePen(PS_DOT, 1, 0xFFFFFF);
			HFONT hSmall = CreateFont(-7,0,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,DEFAULT_PITCH,L"Small fonts");
			HFONT hOldFont = (HFONT)SelectObject(hPaintDc, hSmall);
			SetTextColor(hPaintDc, 0xFFFFFF);
			SetBkColor(hPaintDc, 0);

			for(int i = 0; i < pDlg->Count; i++)
			{
				int n = 1;
				DWORD nFlags = pDlg->DlgFlags[i];

				if (i == (MAX_DETECTED_DIALOGS-1))
					SelectObject(hPaintDc, hRed);
				else if ((nFlags & FR_ACTIVEMENUBAR) == FR_ACTIVEMENUBAR)
					SelectObject(hPaintDc, hActiveMenu);
				else if ((nFlags & FR_QSEARCH))
					SelectObject(hPaintDc, hQSearch);
				else if ((nFlags & (FR_UCHARMAP|FR_UCHARMAPGLYPH)))
					SelectObject(hPaintDc, hUCharMap);
				else if ((nFlags & (FR_LEFTPANEL|FR_RIGHTPANEL|FR_FULLPANEL)))
					SelectObject(hPaintDc, hPanel);
				else if ((nFlags & FR_HASBORDER))
					SelectObject(hPaintDc, hFrame);
				else
				{
					SelectObject(hPaintDc, hDash);
					n = 0;
				}

				//
				POINT pt[2];
				pt[0] = ConsoleToClient(pDlg->Rects[i].Left, pDlg->Rects[i].Top);
				pt[1] = ConsoleToClient(pDlg->Rects[i].Right+1, pDlg->Rects[i].Bottom+1);
				//MapWindowPoints(GetView(), ghWnd, pt, 2);
				Rectangle(hPaintDc, pt[0].x+n, pt[0].y+n, pt[1].x-n, pt[1].y-n);
				wchar_t szCoord[32]; _wsprintf(szCoord, SKIPLEN(countof(szCoord)) L"%ix%i", pDlg->Rects[i].Left, pDlg->Rects[i].Top);
				TextOut(hPaintDc, pt[0].x+1, pt[0].y+1, szCoord, _tcslen(szCoord));
			}

			SelectObject(hPaintDc, hOldBr);
			SelectObject(hPaintDc, hOldPen);
			SelectObject(hPaintDc, hOldFont);
			DeleteObject(hRed);
			DeleteObject(hPanel);
			DeleteObject(hQSearch);
			DeleteObject(hUCharMap);
			DeleteObject(hDash);
			DeleteObject(hFrame);
			DeleteObject(hSmall);
		}
	}

	//#endif

	if (lbExcept)
		Box(_T("Exception triggered in CVirtualConsole::Paint"));

	//  if (hPaintDc && 'ghWnd DC') {
	//if (lbInval)
	//	EndPaint('ghWnd DC', &ps);
	//else
	//	ReleaseDC('ghWnd DC', hPaintDc);
	//  }
	//gpSet->Performance(tPerfFPS, FALSE); // ��������
	mb_DialogsChanged = FALSE; // �������, ������ ������ �� �����

	#ifdef _DEBUG
	if ((GetKeyState(VK_SCROLL) & 1) && (GetKeyState(VK_CAPITAL) & 1))
	{
		gpConEmu->DebugStep(L"ConEmu: Sleeping in CVirtualConsole::Paint for 1s");
		Sleep(1000);
		gpConEmu->DebugStep(NULL);
	}
	#endif
}

void CVirtualConsole::UpdateInfo()
{
	if (!ghOpWnd || !this)
		return;

	if (!gpConEmu->isMainThread())
	{
		return;
	}

	wchar_t szSize[128];

	if (!mp_RCon)
	{
		SetDlgItemText(gpSet->hInfo, tConSizeChr, L"(None)");
		SetDlgItemText(gpSet->hInfo, tConSizePix, L"(None)");
		SetDlgItemText(gpSet->hInfo, tPanelLeft, L"(None)");
		SetDlgItemText(gpSet->hInfo, tPanelRight, L"(None)");
	}
	else
	{
		_wsprintf(szSize, SKIPLEN(countof(szSize)) _T("%ix%i"), mp_RCon->TextWidth(), mp_RCon->TextHeight());
		SetDlgItemText(gpSet->hInfo, tConSizeChr, szSize);
		_wsprintf(szSize, SKIPLEN(countof(szSize)) _T("%ix%i"), Width, Height);
		SetDlgItemText(gpSet->hInfo, tConSizePix, szSize);
		RECT rcPanel;
		RCon()->GetPanelRect(FALSE, &rcPanel);

		if (rcPanel.right>rcPanel.left)
			_wsprintf(szSize, SKIPLEN(countof(szSize)) L"(%i, %i)-(%i, %i), %ix%i", rcPanel.left+1, rcPanel.top+1, rcPanel.right+1, rcPanel.bottom+1, rcPanel.right-rcPanel.left+1, rcPanel.bottom-rcPanel.top+1);
		else
			wcscpy_c(szSize, L"<Absent>");

		SetDlgItemText(gpSet->hInfo, tPanelLeft, szSize);
		RCon()->GetPanelRect(TRUE, &rcPanel);

		if (rcPanel.right>rcPanel.left)
			_wsprintf(szSize, SKIPLEN(countof(szSize)) L"(%i, %i)-(%i, %i), %ix%i", rcPanel.left+1, rcPanel.top+1, rcPanel.right+1, rcPanel.bottom+1, rcPanel.right-rcPanel.left+1, rcPanel.bottom-rcPanel.top+1);
		else
			wcscpy_c(szSize, L"<Absent>");

		SetDlgItemText(gpSet->hInfo, tPanelRight, szSize);
	}
}

void CVirtualConsole::Box(LPCTSTR szText)
{
#ifdef _DEBUG
	_ASSERT(FALSE);
#endif
	MessageBox(NULL, szText, gpConEmu->ms_ConEmuVer, MB_ICONSTOP);
}

RECT CVirtualConsole::GetRect()
{
	RECT rc;

	if (Width == 0 || Height == 0)    // ���� ������� ��� �� ����������� - ����������� �� ������� GUI ����
	{
		//rc = MakeRect(winSize.X, winSize.Y);
		RECT rcWnd; GetClientRect(ghWnd, &rcWnd);
		RECT rcCon = gpConEmu->CalcRect(CER_CONSOLE, rcWnd, CER_MAINCLIENT);
		RECT rcDC = gpConEmu->CalcRect(CER_DC, rcCon, CER_CONSOLE);
		rc = MakeRect(rcDC.right, rcDC.bottom);
	}
	else
	{
		rc = MakeRect(Width, Height);
	}

	return rc;
}

void CVirtualConsole::OnFontChanged()
{
	if (!this) return;

	mb_RequiredForceUpdate = true;
	//ClearPartBrushes();
}

void CVirtualConsole::OnConsoleSizeChanged()
{
	// �� ���� ��� ������� ���������� ��� ������� ���� GUI ConEmu
	BOOL lbLast = mb_InPaintCall;
	mb_InPaintCall = TRUE; // ����� Invalidate ������ ��� �� ��������
	Update(mb_RequiredForceUpdate);
	mb_InPaintCall = lbLast;
}

POINT CVirtualConsole::ConsoleToClient(LONG x, LONG y)
{
	POINT pt = {0,0};

	if (!this)
	{
		pt.y = y*gpSet->FontHeight();
		pt.x = x*gpSet->FontWidth();
		return pt;
	}

	pt.y = y*nFontHeight;

	if (x>0)
	{
		if (ConCharX && y >= 0 && y < (int)TextHeight && x < (int)TextWidth)
		{
			pt.x = ConCharX[y*TextWidth + x-1];

			if (x && !pt.x)
			{
				TODO("��� ������ �� �� �������� ���� - ConCharX ����� ���� �������");
				//2010-06-07
				// 	CRealConsole::RConServerThread -->
				//  CRealConsole::ServerThreadCommand -->
				//  CVirtualConsole::RegisterPanelView -->
				//  CVirtualConsole::UpdatePanelView(int abLeftPanel=1, int abOnRegister=1) -->
				//  CVirtualConsole::UpdatePanelRgn(int abLeftPanel=1, int abTestOnly=0, int abOnRegister=1) -->
				//  � ��� ��� ��� ����������. ConCharX �������� �������?
				Sleep(1);
				pt.x = ConCharX[y*TextWidth + x-1];
				_ASSERTE(pt.x || x==0);
			}
		}
		else
		{
			pt.x = x*nFontWidth;
		}
	}

	return pt;
}

// ������� ����� �����, � �� � gpSet, �.�. ����� �� ����� ����� ����� ����������� ����������
COORD CVirtualConsole::ClientToConsole(LONG x, LONG y)
{
	COORD cr = {0,0};

	if (!this)
	{
		cr.Y = y/gpSet->FontHeight();
		cr.X = x/gpSet->FontWidth();
		return cr;
	}

	_ASSERTE(nFontWidth!=0 && nFontHeight!=0);

	// ������� ��������������� �������� �� �������� ������
	if (nFontHeight)
		cr.Y = y/nFontHeight;

	if (nFontWidth)
		cr.X = x/nFontWidth;

	// � ������, ���� ��������, ������� X ����������
	if (x > 0)
	{
		x++; // ����� ��������� �� ���� ������ �����

		if (ConCharX && cr.Y >= 0 && cr.Y < (int)TextHeight)
		{
			DWORD* ConCharXLine = ConCharX + cr.Y * TextWidth;

			for(uint i = 0; i < TextWidth; i++, ConCharXLine++)
			{
				if (((int)*ConCharXLine) >= x)
				{
					if (cr.X != (int)i)
					{
#ifdef _DEBUG
						wchar_t szDbg[120]; _wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"Coord corrected from {%i-%i} to {%i-%i}",
						                              cr.X, cr.Y, i, cr.Y);
						DEBUGSTRCOORD(szDbg);
#endif
						cr.X = i;
					}

					break;
				}
			}
		}
	}

	return cr;
}

// �� ����� ���� �� ������ �������, �� � ucBoxSinglHorz/ucBoxDblVertHorz
void CVirtualConsole::DistributeSpaces(wchar_t* ConCharLine, CharAttr* ConAttrLine, DWORD* ConCharXLine, const int j, const int j2, const int end)
{
	//WORD attr = ConAttrLine[j];
	//wchar_t ch, c;
	//
	//if ((c=ConCharLine[j]) == ucSpace || c == ucNoBreakSpace || c == 0)
	//{
	//	while(j2 < end && ConAttrLine[j2] == attr
	//		// ����� � ��� &nbsp; � 0x00
	//		&& ((ch=ConCharLine[j2]) == ucSpace || ch == ucNoBreakSpace || ch == 0))
	//		j2++;
	//} else
	//if ((c=ConCharLine[j]) == ucBoxSinglHorz || c == ucBoxDblVertHorz)
	//{
	//	while(j2 < end && ConAttrLine[j2] == attr
	//		&& ((ch=ConCharLine[j2]) == ucBoxSinglHorz || ch == ucBoxDblVertHorz))
	//		j2++;
	//} else
	//if (isCharProgress(c=ConCharLine[j]))
	//{
	//	while(j2 < end && ConAttrLine[j2] == attr && (ch=ConCharLine[j2]) == c)
	//		j2++;
	//}
	_ASSERTE(j2 > j && j >= 0);

	// ������ ������� "�������" (��� ������� �����) ����� ������� ��������������� ����������� ��� �����

	if (j2>=(int)TextWidth)    // ����� ������
	{
		ConCharXLine[j2-1] = Width;
	}
	else
	{
		//2009-09-09 ��� �����������. ������ ������ ����� ����� ���� ������ ����������
		//int nBordWidth = gpSet->BorderFontWidth(); if (!nBordWidth) nBordWidth = nFontWidth;
		int nBordWidth = nFontWidth;

		// ���������� ���������� ����� ������������������
		if (isCharBorderVertical(ConCharLine[j2]))
		{
			//2009-09-09 � ��� �������������� �� ����� (���� nFontWidth == nBordWidth)
			//ConCharXLine[j2-1] = (j2-1) * nFontWidth + nBordWidth; // ��� ��� [j] ������ ����?
			ConCharXLine[j2-1] = j2 * nBordWidth;
		}
		else
		{
			TODO("��� ���������������� ������� ���� ������ ���-�� �� �������!");

			//2009-09-09 � ��� �������������� �� ����� (���� nFontWidth == nBordWidth)
			//ConCharXLine[j2-1] = (j2-1) * nFontWidth + nBordWidth; // ��� ��� [j] ������ ����?
			if (!gpSet->isMonospace && j > 1)
			{
				//2009-12-31 ����� ������� �� ����������� �������!
				ConCharXLine[j2-1] = ConCharXLine[j-1] + (j2 - j) * nBordWidth;
			}
			else
			{
				ConCharXLine[j2-1] = j2 * nBordWidth;
			}
		}
	}

	if (j2 > (j + 1))
	{
		HEAPVAL
		DWORD n1 = (j ? ConCharXLine[j-1] : 0); // j? ���� j==0 �� ��� � ��� 10 (������ ������� 0�� ������� � ������)
		DWORD n3 = j2-j; // ���-�� ��������
		_ASSERTE(n3>0);
		DWORD n2 = ConCharXLine[j2-1] - n1; // ���������� �� ������� ������
		HEAPVAL

		for(int k=j, l=1; k<(j2-1); k++, l++)
		{
#ifdef _DEBUG
			DWORD nn = n1 + (n3 ? klMulDivU32(l, n2, n3) : 0);

			if (nn != ConCharXLine[k])
				ConCharXLine[k] = nn;

#endif
			ConCharXLine[k] = n1 + (n3 ? klMulDivU32(l, n2, n3) : 0);
			//n1 + (n3 ? klMulDivU32(k-j, n2, n3) : 0);
			HEAPVAL
		}
	}
}

BOOL CVirtualConsole::FindChanges(int row, int &j, int &end, const wchar_t* ConCharLine, const CharAttr* ConAttrLine, const wchar_t* ConCharLine2, const CharAttr* ConAttrLine2)
{
	// UCharMap - ������������ ���������, ��� ����� ����� ����������
	if (gpSet->isExtendUCharMap && mrc_UCharMap.Right > mrc_UCharMap.Left)
	{
		if (row >= mrc_UCharMap.Top && row <= mrc_UCharMap.Bottom)
			return TRUE; // ���� ������ ������� ����� ��������
	}

	// ���� ��������� � ������ ������ ���
	if (wmemcmp(ConCharLine, ConCharLine2, end) == 0
	        && memcmp(ConAttrLine, ConAttrLine2, end*sizeof(*ConAttrLine)) == 0)
		return FALSE;

	// Default: j = 0, end = TextWidth;
	return TRUE; // ���� ������ ������� ����� ��������
	// ���� �������� ����� � ������� ���������, ��
	// �� ��������� ��������� ������ ����� ����� ����� �������� ������ �������
	TODO("�������� ��� ���������� ClearType �� ����� ����� �������� ������ �������, ���� ��������� �� Transparent ��������� ������");

	if (gpSet->FontItalic() || gpSet->FontClearType())
		return TRUE;

	// *) Skip not changed tail symbols. �� ������ ���� ����� ����������
	if (gpSet->isMonospace)
	{
		TODO("OPTIMIZE:");

		while(--end >= 0 && ConCharLine[end] == ConCharLine2[end] && ConAttrLine[end] == ConAttrLine2[end])
		{
			// ���� ���� ����� ������ (������/������), ��
			// �� ��������� ��������� ������ ����� ����� ����� �������� ������ �������
			if (ConAttrLine[end].nFontIndex)
			{
				end = TextWidth;
				return TRUE;
			}
		}

		// [%] ClearType, TTF fonts (isProportional ����� ���� � ��������)
		if (end >= 0   // ���� ���� ���� �����-�� ���������
		        && end < (int)(TextWidth - 1) // �� ����� ������
		        && (ConCharLine[end+1] == ucSpace || ConCharLine[end+1] == ucNoBreakSpace || isCharProgress(ConCharLine[end+1])) // ����� ������������ ��� �������
		  )
		{
			int n = TextWidth - 1;

			while((end < n)
			        && (ConCharLine[end+1] == ucSpace || ConCharLine[end+1] == ucNoBreakSpace || isCharProgress(ConCharLine[end+1])))
				end++;
		}

		if (end < j)
			return FALSE;

		++end;
	}

	// *) Skip not changed head symbols.
	while(j < end && ConCharLine[j] == ConCharLine2[j] && ConAttrLine[j] == ConAttrLine2[j])
	{
		// ���� ���� ����� ������ (������/������) ��
		// �� ��������� ��������� ������ ����� ����� ����� �������� ������ �������
		if (ConAttrLine[j].nFontIndex)
		{
			j = 0;
			end = TextWidth;
			return TRUE;
		}

		++j;
	}

	// [%] ClearType, proportional fonts
	if (j > 0  // ���� � ������ ������
	        && (ConCharLine[j-1] == ucSpace || ConCharLine[j-1] == ucNoBreakSpace || isCharProgress(ConCharLine[j-1]) // ���� �������
	            || (gpSet->FontItalic() || ConAttrLine[j-1].nFontIndex))  // ��� ������ ������/������?
	  )
	{
		if (gpSet->FontItalic() || ConAttrLine[j-1].nFontIndex)
		{
			j = 0; // � ��������� ���������� �������������� �� �����, � �� ��� ClearType �������...
		}
		else
		{
			while((j > 0)
			        && (ConCharLine[j-1] == ucSpace || ConCharLine[j-1] == ucNoBreakSpace || isCharProgress(ConCharLine[j-1])))
			{
				j--;
			}
		}
	}

	if (j >= end)
	{
		// ��� ���������������� ������� ��������� ����������� ������ � ������ ������,
		// ������� ���� �� ������ ����� ������� - ������ ������ �� ��������
		return FALSE;
	}

	return TRUE;
}

HRGN CVirtualConsole::GetExclusionRgn(bool abTestOnly/*=false*/)
{
	if (!gpSet->isUserScreenTransparent)
		return NULL;

	if (mp_RCon->GetFarPID(TRUE) == 0)
		return NULL;

	// ���������� mh_TransparentRgn
	// ��� mh_TransparentRgn ����������� � CheckTransparentRgn,
	// ������� ������ ���������� � CVirtualConsole::Paint
	if (abTestOnly && mh_TransparentRgn)
		return (HRGN)1;

	return mh_TransparentRgn;
}

COORD CVirtualConsole::FindOpaqueCell()
{
	COORD cr = {0,0};

	if (this)
	{
		// �� ��������� - �������
		cr.X = cr.Y = -1;

		if (mpsz_ConChar && mpn_ConAttrEx && TextWidth && TextHeight)
		{
			MSectionLock SCON; SCON.Lock(&csCON);
			CharAttr* pnAttr = mpn_ConAttrEx;

			// ����� ������� �������������
			for (uint y = 0; y < TextHeight; y++)
			{
				for (uint x = 0; x < TextWidth; x++, pnAttr++)
				{
					#if 0
					if (!pnAttr[x].bTransparent)
					#else
					if (!(pnAttr->Flags & CharAttr_Transparent))
					#endif
					{
						cr.X = x; cr.Y = y;
						return cr;
					}
				}

				//pnAttr += TextWidth;
			}
		}
	}

	return cr;
}

// �������� ����������� ���� ��� ������� �������� �������
// ptCur - �������� ����������
void CVirtualConsole::ShowPopupMenu(POINT ptCur)
{
	BOOL lbNeedCreate = (mh_PopupMenu == NULL);

	// ������� ��� �������� enable/disable
	mh_PopupMenu = gpConEmu->CreateVConPopupMenu(this, mh_PopupMenu, TRUE);
	if (!mh_PopupMenu)
	{
		MBoxAssert(mh_PopupMenu!=NULL);
		return;
	}

	if (lbNeedCreate)
	{
		AppendMenu(mh_PopupMenu, MF_BYPOSITION, MF_SEPARATOR, 0);

		mh_EditPopup = gpConEmu->CreateEditMenuPopup(this);
		AppendMenu(mh_PopupMenu, MF_BYPOSITION | MF_POPUP | MF_ENABLED, (UINT_PTR)mh_EditPopup, _T("Ed&it"));

		mh_DebugPopup = gpConEmu->CreateDebugMenuPopup();
		AppendMenu(mh_PopupMenu, MF_BYPOSITION | MF_POPUP | MF_ENABLED, (UINT_PTR)mh_DebugPopup, _T("&Debug"));
	}
	else
	{
		// �������� enable/disable ������� ����
		gpConEmu->CreateEditMenuPopup(this, mh_EditPopup);
	}

	// ��������. ����� ������ ������ ��� ����
	//ptCur.x++; ptCur.y++; // ����� ���� ����� ���� ����� ������� ����� ������.
	
	
	// -- ���������� � CreateVConPopupMenu
	//bool lbIsFar = mp_RCon->isFar(TRUE/* abPluginRequired */)!=FALSE;
	//bool lbIsPanels = lbIsFar && mp_RCon->isFilePanel(false/* abPluginAllowed */)!=FALSE;
	//bool lbIsEditorModified = lbIsFar && mp_RCon->isEditorModified()!=FALSE;
	//bool lbHaveModified = lbIsFar && mp_RCon->GetModifiedEditors()!=FALSE;
	//bool lbCanCloseTab = mp_RCon->CanCloseTab();
	//
	//if (lbHaveModified)
	//{
	//	if (!gpSet->sSaveAllMacro || !*gpSet->sSaveAllMacro)
	//		lbHaveModified = false;
	//}
	//
	//EnableMenuItem(mh_PopupMenu, IDM_CLOSE, MF_BYCOMMAND | (lbCanCloseTab ? MF_ENABLED : MF_GRAYED));
	//EnableMenuItem(mh_PopupMenu, IDM_ADMIN_DUPLICATE, MF_BYCOMMAND | (lbIsPanels ? MF_ENABLED : MF_GRAYED));
	//EnableMenuItem(mh_PopupMenu, IDM_SAVE, MF_BYCOMMAND | (lbIsEditorModified ? MF_ENABLED : MF_GRAYED));
	//EnableMenuItem(mh_PopupMenu, IDM_SAVEALL, MF_BYCOMMAND | (lbHaveModified ? MF_ENABLED : MF_GRAYED));
	
	int nCmd = TrackPopupMenu(mh_PopupMenu,
	                          TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD,
	                          ptCur.x, ptCur.y, 0, ghWnd, NULL);

	if (!nCmd)
		return; // ������

	ExecPopupMenuCmd(nCmd);
}

void CVirtualConsole::ExecPopupMenuCmd(int nCmd)
{
	if (!this)
		return;

	switch(nCmd)
	{
		case IDM_CLOSE:
			//mp_RCon->PostMacro(gpSet->sTabCloseMacro ? gpSet->sTabCloseMacro : L"F10");
			mp_RCon->CloseTab();
			break;
		case IDM_DETACH:
			mp_RCon->Detach();
			gpConEmu->OnVConTerminated(this);
			break;
		case IDM_TERMINATE:
			mp_RCon->CloseConsole(TRUE);
			break;
		case IDM_RESTART:
		case IDM_RESTARTAS:

			if (gpConEmu->isActive(this))
			{
				gpConEmu->Recreate(TRUE, FALSE, (nCmd==IDM_RESTARTAS));
			}
			else
			{
				MBoxAssert(gpConEmu->isActive(this));
			}

			break;
		case IDM_NEW:
			gpConEmu->Recreate(FALSE, gpSet->isMultiNewConfirm);
			break;
		case IDM_ADMIN_DUPLICATE:
			mp_RCon->AdminDuplicate();
			break;
		case IDM_SAVE:
			mp_RCon->PostMacro(L"F2");
			break;
		case IDM_SAVEALL:
			mp_RCon->PostMacro(gpSet->sSaveAllMacro);
			break;
		default:
			if (nCmd >= 0xAB00)
			{
				// "���������" �������, �������������� � CConEmu
				gpConEmu->OnSysCommand(ghWnd, nCmd, 0);
			}
	}
}

void CVirtualConsole::OnPanelViewSettingsChanged()
{
	if (!this) return;

	if (!mp_RCon) return;

	DWORD nPID = 0;

	if ((nPID = mp_RCon->GetFarPID(TRUE)) != 0)
	{
		CConEmuPipe pipe(nPID, 1000);

		if (pipe.Init(_T("CVirtualConsole::OnPanelViewSettingsChanged"), TRUE))
		{
			CESERVER_REQ_GUICHANGED lWindows = {sizeof(CESERVER_REQ_GUICHANGED)};
			lWindows.nGuiPID = GetCurrentProcessId();
			lWindows.hLeftView = m_LeftPanelView.hWnd;
			lWindows.hRightView = m_RightPanelView.hWnd;
			pipe.Execute(CMD_GUICHANGED, &lWindows, sizeof(lWindows));
			gpConEmu->DebugStep(NULL);
		}
	}

	//if (!mn_ConEmuSettingsMsg)
	//	mn_ConEmuSettingsMsg = RegisterWindowMessage(CONEMUMSG_PNLVIEWSETTINGS);
	//DWORD nPID = GetCurrentProcessId();
	//if (m_LeftPanelView.hWnd && IsWindow(m_LeftPanelView.hWnd))
	//{
	//	WARNING("������ �� ����� ������������, ���� ������� �������� ��� �������");
	//	PostMessage(m_LeftPanelView.hWnd, mn_ConEmuSettingsMsg, nPID, 0);
	//}
	//if (m_RightPanelView.hWnd && IsWindow(m_RightPanelView.hWnd))
	//{
	//	PostMessage(m_RightPanelView.hWnd, mn_ConEmuSettingsMsg, nPID, 0);
	//}
}

BOOL CVirtualConsole::IsPanelViews()
{
	if (!this) return FALSE;

	if (m_LeftPanelView.hWnd && IsWindow(m_LeftPanelView.hWnd))
		return TRUE;

	if (m_RightPanelView.hWnd && IsWindow(m_RightPanelView.hWnd))
		return TRUE;

	return FALSE;
}

BOOL CVirtualConsole::RegisterPanelView(PanelViewInit* ppvi)
{
	_ASSERTE(ppvi && ppvi->cbSize == sizeof(PanelViewInit));
	BOOL lbRc = FALSE;
	PanelViewInit* pp = (ppvi->bLeftPanel) ? &m_LeftPanelView : &m_RightPanelView;
	BOOL lbPrevRegistered = pp->bRegister;
	*pp = *ppvi;
	// ������� ������� ������� GUI
	gpConEmu->OnPanelViewSettingsChanged(FALSE/*abSendChanges*/);
	//COLORREF *pcrNormal = gpSet->GetColors(FALSE);
	//COLORREF *pcrFade = gpSet->GetColors(TRUE);
	//for (int i=0; i<16; i++) {
	//	// ����� FOR ����� � BitMask �� ����������
	//	ppvi->crPalette[i] = (pcrNormal[i]) & 0xFFFFFF;
	//	ppvi->crFadePalette[i] = (pcrFade[i]) & 0xFFFFFF;
	//}
	ppvi->bFadeColors = isFade;
	//memmove(&(ppvi->ThSet), &(gpSet->ThSet), sizeof(gpSet->ThSet));
	DWORD dwSetParentErr = 0;

	if (ppvi->bRegister)
	{
		_ASSERTE(ppvi->bVisible);

		// ��� ��������� ����������� - �� ���������
		if (!lbPrevRegistered)
		{
			//for (int i=0; i<20; i++) // �� ������ ������ - ������� ��� �������
			//	SetWindowLong(pp->hWnd, i*4, 0);
			HWND hViewParent = GetParent(ppvi->hWnd);

			if (hViewParent != ghWnd)
			{
				MBoxAssert(hViewParent==ghWnd);
				// -- ������ �������� ������ �������� - �������, � Win7 ��� � �� ���������
				// -- ���� ���������� ������� ������� "Run as administrator"
				// --- �� ���-���� ���������?
				//DWORD dwSetParentErr = 0;
				HWND hRc = SetParent((HWND)ppvi->hWnd, ghWnd);
				dwSetParentErr = GetLastError();
			}
			else
			{
				lbRc = TRUE;
			}
		}
		else
		{
			lbRc = TRUE;

			if (ppvi->bLeftPanel)
				mb_LeftPanelRedraw = TRUE;
			else
				mb_RightPanelRedraw = TRUE;
		}

		if (lbRc)
		{
			// ��������� � ������� (�� �������� ��������� ����)
			lbRc = UpdatePanelView(ppvi->bLeftPanel, TRUE);
			// �������� ���� ���������
			ppvi->bVisible = pp->bVisible;
			ppvi->WorkRect = pp->WorkRect;
			// pp->WorkRect ��� �������� ������������� "�� +1".
			ppvi->WorkRect.bottom--; ppvi->WorkRect.right--;
			// �� ������� ����� "��������" ������ ����� �����
			Update(true);

			// ���� ������� ���� �������� - �������� � ��� ������ "Apply"
			if (ghOpWnd) gpSet->OnPanelViewAppeared(TRUE);
		}
	}
	else
	{
		lbRc = TRUE;
	}

	return lbRc;
}

//HRGN CVirtualConsole::CreateConsoleRgn(int x1, int y1, int x2, int y2, BOOL abTestOnly)
//{
//	POINT pt[2];
//	if (abTestOnly) {
//		// ���������� �������������� ����������� ���������������, ��� ��� ���������� �� �����
//		pt[0].x = x1 << 3; pt[0].y = y1 << 3;
//		pt[1].x = x2 << 3; pt[1].y = y2 << 3;
//	} else {
//		pt[0] = ConsoleToClient(x1, y1);
//		pt[1] = ConsoleToClient(x2+1, y2+1);
//	}
//	HRGN hRgn = CreateRectRgn(pt[0].x, pt[0].y, pt[1].x, pt[1].y);
//	return hRgn;
//}

BOOL CVirtualConsole::CheckDialogsChanged()
{
	BOOL lbChanged = FALSE;
	SMALL_RECT rcDlg[32];
	_ASSERTE(sizeof(mrc_LastDialogs) == sizeof(rcDlg));
	int nDlgCount = mp_RCon->GetDetectedDialogs(countof(rcDlg), rcDlg, NULL);

	if (mn_LastDialogsCount != nDlgCount)
	{
		lbChanged = TRUE;
	}
	else if (memcmp(rcDlg, mrc_LastDialogs, nDlgCount*sizeof(rcDlg[0]))!=0)
	{
		lbChanged = TRUE;
	}

	if (lbChanged)
	{
		memset(mrc_LastDialogs, 0, sizeof(mrc_LastDialogs));
		memmove(mrc_LastDialogs, rcDlg, nDlgCount*sizeof(rcDlg[0]));
		mn_LastDialogsCount = nDlgCount;
	}

	return lbChanged;
}

const PanelViewInit* CVirtualConsole::GetPanelView(BOOL abLeftPanel)
{
	if (!this) return NULL;

	PanelViewInit* pp = abLeftPanel ? &m_LeftPanelView : &m_RightPanelView;

	if (!pp->hWnd || !IsWindow(pp->hWnd))
	{
		if (pp->hWnd)
			pp->hWnd = NULL;

		pp->bVisible = FALSE;
		return NULL;
	}

	return pp;
}

// ���������� TRUE, ���� ������ (��� ���� �� ����� �� ������)
BOOL CVirtualConsole::UpdatePanelRgn(BOOL abLeftPanel, BOOL abTestOnly, BOOL abOnRegister)
{
	PanelViewInit* pp = abLeftPanel ? &m_LeftPanelView : &m_RightPanelView;

	if (!pp->hWnd || !IsWindow(pp->hWnd))
	{
		if (pp->hWnd)
			pp->hWnd = NULL;

		pp->bVisible = FALSE;
		return FALSE;
	}

	CRgnRects* pRgn = abLeftPanel ? &m_RgnLeftPanel : &m_RgnRightPanel;
	BOOL lbPartHidden = FALSE;
	BOOL lbPanelVisible = FALSE;
	SMALL_RECT rcDlg[32]; DWORD rnDlgFlags[32];
	_ASSERTE(sizeof(mrc_LastDialogs) == sizeof(rcDlg));
	const CRgnDetect* pDetect = mp_RCon->GetDetector();
	int nDlgCount = pDetect->GetDetectedDialogs(countof(rcDlg), rcDlg, rnDlgFlags);
	DWORD nDlgFlags = pDetect->GetFlags();

	if (!nDlgCount)
	{
		lbPartHidden = TRUE;
		pp->bVisible = FALSE;
		//if (!abTestOnly && !abOnRegister) {
		//	if (IsWindowVisible(pp->hWnd))
		//		mp_RCon->ShowOtherWindow(pp->hWnd, SW_HIDE);
		//}
	}
	else
	{
		//HRGN hRgn = NULL, hSubRgn = NULL, hCombine = NULL;
		m_RgnTest.Reset();
		SMALL_RECT rcPanel; DWORD nGetRc;

		if ((nDlgFlags & FR_FULLPANEL) == FR_FULLPANEL)
			nGetRc = pDetect->GetDialog(FR_FULLPANEL, &rcPanel);
		else if (pp->PanelRect.left == 0)
			nGetRc = pDetect->GetDialog(FR_LEFTPANEL, &rcPanel);
		else
			nGetRc = pDetect->GetDialog(FR_RIGHTPANEL, &rcPanel);

		if (!nGetRc)
		{
			lbPanelVisible = FALSE;
		}
		else
		{
			lbPanelVisible = TRUE;
			TODO("��� ������ �� �������� WorkRect? ��� �� ���������� ������ ����� �� ���� ������� RegisterPanel �� �������?");
			m_RgnTest.Init(&(pp->WorkRect));

			for(int i = 0; i < nDlgCount; i++)
			{
				// ���������� ������������� �������������� ����� ������
				if (rcDlg[i].Left == rcPanel.Left &&
				        rcDlg[i].Bottom == rcPanel.Bottom &&
				        rcDlg[i].Right == rcPanel.Right)
				{
					continue;
				}

				if (!IntersectSmallRect(pp->WorkRect, rcDlg[i]))
					continue;

				// ��� ��������� �������������� �������� �� hRgn
				int nCRC = m_RgnTest.DiffSmall(rcDlg+i);

				// ��������, ����� ������ ��� ����?
				if (nCRC == NULLREGION)
				{
					lbPanelVisible = FALSE; break;
				}
			}
		}

		if (abTestOnly)
		{
			// ������ ��������
		}
		else
		{
			bool lbChanged = pRgn->LoadFrom(&m_RgnTest);

			if (lbPanelVisible)
			{
				lbPartHidden = (pRgn->nRgnState == COMPLEXREGION);
				pp->bVisible = TRUE;
				_ASSERTE(pRgn->nRectCount > 0);

				if (pRgn->nRgnState == SIMPLEREGION)
				{
					//NULL ������ ����� ������ ������ (���� ����� ���� ReUsed)
					//if (!abOnRegister) {
					mp_RCon->SetOtherWindowRgn(pp->hWnd, 0, NULL, TRUE);
					//}
				}
				else if (lbChanged)
				{
					// � ��� ��� ��� ����� ������ ��������� � ������, ������� ����� �������, � �� ��������...
					MSectionLock SCON; SCON.Lock(&csCON);
					POINT ptShift = ConsoleToClient(pp->WorkRect.left, pp->WorkRect.top);
					POINT pt[2];
					RECT  rc[MAX_DETECTED_DIALOGS];
					_ASSERTE(pRgn->nRectCount<MAX_DETECTED_DIALOGS);

					for(int i = 0; i < pRgn->nRectCount; i++)
					{
						pt[0] = ConsoleToClient(pRgn->rcRect[i].left, pRgn->rcRect[i].top);
						pt[1] = ConsoleToClient(pRgn->rcRect[i].right+1, pRgn->rcRect[i].bottom+1);
						rc[i].left  = pt[0].x - ptShift.x;
						rc[i].top   = pt[0].y - ptShift.y;
						rc[i].right = pt[1].x - ptShift.x;
						rc[i].bottom= pt[1].y - ptShift.y;
					}

					mp_RCon->SetOtherWindowRgn(pp->hWnd, pRgn->nRectCount, rc, TRUE);
				}

				//if (!abOnRegister) {
				//	if (!IsWindowVisible(pp->hWnd))
				//		mp_RCon->ShowOtherWindow(pp->hWnd, SW_SHOWNA);
				//}
			}
			else
			{
				lbPartHidden = TRUE;
				pp->bVisible = FALSE;
				//if (IsWindowVisible(pp->hWnd)) {
				//	// ������� ������ � ������(!) ����
				//	mp_RCon->SetOtherWindowRgn(pp->hWnd, -1, NULL, FALSE);
				//}
			}
		}
	}

	if (!abTestOnly && !abOnRegister)
	{
		if (pp->bVisible && !IsWindowVisible(pp->hWnd))
		{
			mp_RCon->ShowOtherWindow(pp->hWnd, SW_SHOWNA);
		}
		else if (!pp->bVisible && IsWindowVisible(pp->hWnd))
		{
			// ������� ������ � ������(!) ����
			mp_RCon->SetOtherWindowRgn(pp->hWnd, -1, NULL, FALSE);
			//mp_RCon->ShowOtherWindow(pp->hWnd, SW_HIDE);
		}
	}

	return lbPanelVisible;
}

// ������ - Redraw �� �����, ������ Invalidate!
BOOL CVirtualConsole::UpdatePanelView(BOOL abLeftPanel, BOOL abOnRegister/*=FALSE*/)
{
	PanelViewInit* pp = abLeftPanel ? &m_LeftPanelView : &m_RightPanelView;

	// ����� ������ ����, ��� ���������� ������� (��� ��� Fade, ��� �������� ������������� ������).
	//for (int i=0; i<16; i++)
	//	SetWindowLong(pp->hWnd, i*4, mp_Colors[i]);
	if (!mn_ConEmuFadeMsg)
		mn_ConEmuFadeMsg = RegisterWindowMessage(CONEMUMSG_PNLVIEWFADE);

	DWORD nNewFadeValue = isFade ? 2 : 1;
	WARNING("������ �� ����� �������� �� Win7 RunAsAdmin");

	if (GetWindowLong(pp->hWnd, 16*4) != nNewFadeValue)
		PostMessage(pp->hWnd, mn_ConEmuFadeMsg, 100, nNewFadeValue);

	// ����������� �������
	POINT pt[2];
	int nTopShift = 1 + ((pp->nFarPanelSettings & 0x20/*FPS_SHOWCOLUMNTITLES*/) ? 1 : 0); //-V112
	int nBottomShift = 0;
	if (mp_RCon && (pp->nFarPanelSettings & 0x40/*FPS_SHOWSTATUSLINE*/))
	{
		nBottomShift = mp_RCon->GetStatusLineCount(pp->PanelRect.left) + 1;
		if (nBottomShift < 2)
			nBottomShift = 2;
	}
	pp->WorkRect = MakeRect(
	                   pp->PanelRect.left+1, pp->PanelRect.top+nTopShift,
	                   pp->PanelRect.right, pp->PanelRect.bottom-nBottomShift);
	//if (mb_DialogsChanged)
	// ������� ����, �.�. ��� ����������� ������ ���������� GUI
	//UpdatePanelRgn(abLeftPanel, FALSE, abOnRegister);
	// ����� �� �������� �� ConCharX - �� ����� ��������� �� ������������������!
	pt[0] = MakePoint(pp->WorkRect.left*gpSet->FontWidth(), pp->WorkRect.top*gpSet->FontHeight());
	pt[1] = MakePoint(pp->WorkRect.right*gpSet->FontWidth(), pp->WorkRect.bottom*gpSet->FontHeight());
	//pt[0] = ConsoleToClient(pp->WorkRect.left, pp->WorkRect.top);
	//pt[1] = ConsoleToClient(pp->WorkRect.right, pp->WorkRect.bottom);
	TODO("����������� ��������� ��� DoubleView");
	MapWindowPoints(GetView(), ghWnd, pt, 2);
	//MoveWindow(ahView, pt[0].x,pt[0].y, pt[1].x-pt[0].x,pt[1].y-pt[0].y, FALSE);
	// �� ���������, ���� ������ ������ �� �����
	DWORD dwErr = 0; BOOL lbRc = TRUE;
	RECT rcCur; GetWindowRect(pp->hWnd, &rcCur);
	MapWindowPoints(NULL, ghWnd, (LPPOINT)&rcCur, 2);

	if (rcCur.left != pt[0].x || rcCur.top != pt[0].y
	        || rcCur.right != pt[1].x || rcCur.bottom != pt[1].y)
	{
		lbRc = mp_RCon->SetOtherWindowPos(pp->hWnd, HWND_TOP,
		                                  pt[0].x,pt[0].y, pt[1].x-pt[0].x,pt[1].y-pt[0].y,
		                                  SWP_ASYNCWINDOWPOS|SWP_DEFERERASE|SWP_NOREDRAW);

		if (!lbRc)
		{
			dwErr = GetLastError();
			DisplayLastError(L"Can't update position of PanelView window!", dwErr);
		}
	}

	// � ���������� -- �� �����. ���� ��� �� ��������? ����� ����� �������� - ����� � ����������
	//InvalidateRect(pp->hWnd, NULL, FALSE); -- �� �����, ��� ���������� ������� WM_PAINT
	// ������� � ��������� (��������� ��� ����������� �� ��������)
	UpdatePanelRgn(abLeftPanel, FALSE, abOnRegister);
	return TRUE;
}

void CVirtualConsole::PolishPanelViews()
{
	if (!this) return;

	//mpsz_ConChar, mpn_ConAttrEx, TextWidth, TextHeight

	if (!m_LeftPanelView.bRegister && !m_RightPanelView.bRegister)
		return;

	for(int i=0; i<=1; i++)
	{
		PanelViewInit *pp = i ? &m_RightPanelView : &m_LeftPanelView;

		if (!pp->bRegister || !pp->hWnd)
			continue; // ������ �� ����������������

		if (mb_DialogsChanged)
		{
			// ���������, ������� ������ ���� �� ��������
			if (!UpdatePanelRgn(i==0, TRUE))
				continue; // ������ ����� ���������
		}

		const CEFAR_INFO_MAPPING* pFarInfo = mp_RCon->GetFarInfo();

		if (!pFarInfo || !IsWindowVisible(pp->hWnd))
			continue; // ������ ��������� �������

		/* ���, ������ ������, ����� "���������" ��������� � ����������� ����� �������� */
		RECT rc = pp->PanelRect;

		if (rc.right >= (LONG)TextWidth || rc.bottom >= (LONG)TextHeight)
		{
			if (rc.left >= (LONG)TextWidth || rc.top >= (LONG)TextHeight)
			{
				_ASSERTE(rc.right<(LONG)TextWidth && rc.bottom<(LONG)TextHeight);
				continue;
			}

			if (pp->PanelRect.right >= (LONG)TextWidth) pp->PanelRect.right = (LONG)TextWidth-1;

			if (pp->PanelRect.bottom >= (LONG)TextHeight) pp->PanelRect.bottom = (LONG)TextHeight-1;

			MBoxAssert(rc.right<(LONG)TextWidth && rc.bottom<(LONG)TextHeight);
			rc = pp->PanelRect;
		}

		// ����� ����
		BYTE btNamesColor = pFarInfo->nFarColors[col_PanelColumnTitle];
		BYTE btPanelColor = pFarInfo->nFarColors[col_PanelBox];
		// 1. ��������� ������
		int x;
		wchar_t *pszLine = mpsz_ConChar;
		CharAttr *pAttrs = mpn_ConAttrEx;
		int nFore = btPanelColor & 0xF;
		int nBack = (btPanelColor & 0xF0) >> 4;

		for(x = rc.left+1; x < rc.right && pszLine[x] != L' '; x++)
		{
			if ((pszLine[x] == ucBoxSinglDownDblHorz || pszLine[x] == ucBoxDblDownDblHorz) && pAttrs[x].nForeIdx == nFore && pAttrs[x].nBackIdx == nBack)
				pszLine[x] = ucBoxDblHorz;
		}

		for(x = rc.right-1; x > rc.left && pszLine[x] != L' '; x--)
		{
			if ((pszLine[x] == ucBoxSinglDownDblHorz || pszLine[x] == ucBoxDblDownDblHorz) && pAttrs[x].nForeIdx == nFore && pAttrs[x].nBackIdx == nBack)
				pszLine[x] = ucBoxDblHorz;
		}

		// 2. ������ � ������� �������
		pszLine = mpsz_ConChar+TextWidth;
		pAttrs = mpn_ConAttrEx+TextWidth;
		int nNFore = btNamesColor & 0xF;
		int nNBack = (btNamesColor & 0xF0) >> 4;

		if ((pp->nFarPanelSettings & 0x20/*FPS_SHOWCOLUMNTITLES*/) && pp->tColumnTitle.nFlags) //-V112
		{
			LPCWSTR pszNameTitle = pp->tColumnTitle.sText;
			CharAttr ca; CharAttrFromConAttr(pp->tColumnTitle.bConAttr, &ca);
			int nNameLen = _tcslen(pszNameTitle);
			int nX1 = rc.left + 1;

			if ((pp->tColumnTitle.nFlags & PVI_TEXT_SKIPSORTMODE)
			        && (pp->nFarPanelSettings & 0x800/*FPS_SHOWSORTMODELETTER*/))
				nX1 += 2;

			int nLineLen = rc.right - nX1;

			//wmemset(pszLine+nX1, L' ', nLineLen);
			if (nNameLen > nLineLen) nNameLen = nLineLen;

			int nX3 = nX1 + ((nLineLen - nNameLen) >> 1);
			_ASSERTE(nX3<=rc.right);

			//wmemcpy(pszLine+nX3, mp_RCon->ms_NameTitle, nNameLen);
			//TODO("�������� ����� ����� � �������� (����) ��������");
			for(x = nX1; x < nX3; x++)
				if ((pszLine[x] != L' ' && pAttrs[x].nForeIdx == nNFore && pAttrs[x].nBackIdx == nNBack)
				        || ((pszLine[x] == ucBoxSinglVert || pszLine[x] == ucBoxDblVert) && pAttrs[x].nForeIdx == nFore && pAttrs[x].nBackIdx == nBack)
				  ) pszLine[x] = L' ';

			int nX4 = min(rc.right,(nX3+nNameLen));

			for(x = nX3; x < nX4; x++, pszNameTitle++)
				if ((pAttrs[x].nForeIdx == nNFore && pAttrs[x].nBackIdx == nNBack)
				        || ((pszLine[x] == ucBoxSinglVert || pszLine[x] == ucBoxDblVert) && pAttrs[x].nForeIdx == nFore && pAttrs[x].nBackIdx == nBack)
				  )
				{
					pszLine[x] = *pszNameTitle;
					pAttrs[x] = ca;
				}

			for(x = nX4; x < rc.right; x++)
				if ((pszLine[x] != L' ' && pAttrs[x].nForeIdx == nNFore && pAttrs[x].nBackIdx == nNBack)
				        || ((pszLine[x] == ucBoxSinglVert || pszLine[x] == ucBoxDblVert) && pAttrs[x].nForeIdx == nFore && pAttrs[x].nBackIdx == nBack)
				  ) pszLine[x] = L' ';
		}

		// 3. �����������
		rc = pp->WorkRect;
		pszLine = mpsz_ConChar+TextWidth*(rc.bottom);
		pAttrs = mpn_ConAttrEx+TextWidth*(rc.bottom);

		if ((pp->nFarPanelSettings & 0x40/*FPS_SHOWSTATUSLINE*/))
		{
			for(x = rc.left+1; x < rc.right; x++)
			{
				if ((pszLine[x] == ucBoxSinglUpHorz || pszLine[x] == ucBoxDblUpSinglHorz || pszLine[x] == ucBoxDblUpSinglHorz)
					&& pAttrs[x].nForeIdx == nFore && pAttrs[x].nBackIdx == nBack)
				{
					pszLine[x] = ucBoxSinglHorz;
				}
			}
		}
	}
}

void CVirtualConsole::CharAttrFromConAttr(WORD conAttr, CharAttr* pAttr)
{
	memset(pAttr, 0, sizeof(CharAttr));
	pAttr->nForeIdx = (conAttr & 0xF);
	pAttr->nBackIdx = (conAttr & 0xF0) >> 4;
	pAttr->crForeColor = pAttr->crOrigForeColor = mp_Colors[pAttr->nForeIdx];
	pAttr->crBackColor = pAttr->crOrigBackColor = mp_Colors[pAttr->nBackIdx];
}


// ������� (��� ���������� ��� ���������) HDC (CompatibleDC) ��� mp_BkImgData
bool CVirtualConsole::PutBackgroundImage(CBackground* pBack, LONG X, LONG Y, LONG Width, LONG Height)
{
	if (!this) return NULL;

	_ASSERTE(gpConEmu->isMainThread());

	// �����
	mb_BkImgChanged = FALSE;

	/*if (mb_BkImgDelete && mp_BkImgData)
	{
		free(mp_BkImgData); mp_BkImgData = NULL;
		mb_BkImgExist = FALSE;
		return false;
	}*/
	if (!mb_BkImgExist)
		return false;
		
	MSectionLock SC;
	SC.Lock(mcs_BkImgData, FALSE);

	if (mb_BkEmfChanged)
	{
		// ����� �����
		mb_BkEmfChanged = FALSE;
		
		if (!mp_BkEmfData)
		{
			_ASSERTE(mp_BkEmfData!=NULL);
			return false;
		}
		
		// ����� ���������� EMF � mp_BkImgData
		BITMAPINFOHEADER bi = mp_BkEmfData->bi;
		size_t nBitSize = bi.biWidth*bi.biHeight*sizeof(COLORREF);
		size_t nWholeSize = sizeof(CESERVER_REQ_SETBACKGROUND)+nBitSize; //-V103 //-V119
		if (!mp_BkImgData || (mn_BkImgDataMax < nWholeSize))
		{
			if (mp_BkImgData)
				free(mp_BkImgData);
			mp_BkImgData = (CESERVER_REQ_SETBACKGROUND*)malloc(nWholeSize);
			if (!mp_BkImgData)
			{
				_ASSERTE(mp_BkImgData!=NULL);
				return false;
			}
		}
		
		*mp_BkImgData = *mp_BkEmfData;
		mp_BkImgData->bmp.bfType = 0x4D42/*BM*/;
		mp_BkImgData->bmp.bfSize = nBitSize+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER); //-V119
		
		// ������ ����� ������������ DIB � ���������� � ��� EMF
		HDC hScreen = GetDC(NULL);
		//RECT rcMeta = {0,0, mn_BkImgWidth, mn_BkImgHeight}; // (in pixels)
		//RECT rcMetaMM = {0,0, mn_BkImgWidth*10, mn_BkImgHeight*10}; // (in .01-millimeter units)
		//HDC hdcEmf = CreateEnhMetaFile(NULL, NULL, &rcMetaMM, L"ConEmu\0Far Background\0\0");
		//if (!hdcEmf)
		//{
		//	_ASSERTE(hdcEmf!=NULL);
		//	return;
		//}

		HDC hdcDib = CreateCompatibleDC(hScreen);
		if (!hdcDib)
		{
			_ASSERTE(hdcDib!=NULL);
			//DeleteEnhMetaFile(hdcEmf);
			return false;
		}
		COLORREF* pBits = NULL;
		HBITMAP hDib = CreateDIBSection(hScreen, (BITMAPINFO*)&bi, DIB_RGB_COLORS, (void**)&pBits, NULL, 0);
		ReleaseDC(NULL, hScreen); hScreen = NULL;
		
		HBITMAP hOld = (HBITMAP)SelectObject(hdcDib, hDib);
	
		size_t nBitsSize = bi.biWidth*bi.biHeight*sizeof(COLORREF);
		// ������ ������ - �� ���������
		#ifdef _DEBUG
			memset(pBits, 128, nBitSize);
		#else
			memset(pBits, 0, nBitsSize);
		#endif
		
		DWORD nEmfBits = mp_BkEmfData->bmp.bfSize - sizeof(BITMAPFILEHEADER) - sizeof(BITMAPINFOHEADER);
		LPBYTE pEmfBits = (LPBYTE)(mp_BkEmfData+1);
		HENHMETAFILE hdcEmf = SetEnhMetaFileBits(nEmfBits, pEmfBits);
		RECT rcPlay = {0,0, bi.biWidth, bi.biHeight};
		DWORD nPlayErr = 0;
		if (hdcEmf)
		{
			#ifdef _DEBUG
			ENHMETAHEADER	emh = {0};
			DWORD			PixelsX, PixelsY, MMX, MMY, cx, cy;
			emh.nSize = sizeof(ENHMETAHEADER);
			if( GetEnhMetaFileHeader( hdcEmf, sizeof( ENHMETAHEADER ), &emh ) )
			{
				// Get the characteristics of the output device
				HDC hDC = GetDC(NULL);
				PixelsX = GetDeviceCaps( hDC, HORZRES );
				PixelsY = GetDeviceCaps( hDC, VERTRES );
				MMX = GetDeviceCaps( hDC, HORZSIZE ) * 100;
				MMY = GetDeviceCaps( hDC, VERTSIZE ) * 100;
				ReleaseDC(NULL, hDC);

				// Calculate the rect in which to draw the metafile based on the
				// intended size and the current output device resolution
				// Remember that the intended size is given in 0.01mm units, so
				// convert those to device units on the target device
				cx = (int)((float)(emh.rclFrame.right - emh.rclFrame.left) * PixelsX / (MMX));
				cy = (int)((float)(emh.rclFrame.bottom - emh.rclFrame.top) * PixelsY / (MMY));
				//pw->PreferredSize.cx = ip.MulDivI32((emh.rclFrame.right - emh.rclFrame.left), PixelsX, MMX);
				//pw->PreferredSize.cy = ip.MulDivI32((emh.rclFrame.bottom - emh.rclFrame.top), PixelsY, MMY);
				_ASSERTE(cx>0 && cy>0);
				//if (pw->PreferredSize.cx < 0) pw->PreferredSize.cx = -pw->PreferredSize.cx;
				//if (pw->PreferredSize.cy < 0) pw->PreferredSize.cy = -pw->PreferredSize.cy;
				//rcPlay = MakeRect(emh.rclBounds.left,emh.rclBounds.top,emh.rclBounds.right,emh.rclBounds.bottom);
			}
			#endif

			if (!PlayEnhMetaFile(hdcDib, hdcEmf, &rcPlay))
			{
				nPlayErr = GetLastError();
				_ASSERTE(FALSE && (nPlayErr == 0));
			}

			GdiFlush();
			memmove(mp_BkImgData+1, pBits, nBitSize);
		}
		
		SelectObject(hdcDib, hOld);
		DeleteObject(hDib);
		DeleteDC(hdcDib);
		if (hdcEmf)
		{
			DeleteEnhMetaFile(hdcEmf);
		}
		else
		{
			return false;
		}
	}

	if (!mp_BkImgData)
	{
		// ����� �� ���? ��� ���������� ��������?
		_ASSERTE(mp_BkImgData!=NULL);
		return false;
	}

	bool lbFade = false;

	if (gpSet->isFadeInactive && !gpConEmu->isMeForeground(false))
		lbFade = true;

	bool lbRc = pBack->FillBackground(&mp_BkImgData->bmp, X, Y, Width, Height, eUpLeft, lbFade);
	//mb_BkImgChanged = FALSE;
	return lbRc;
}

//// ���������� (���� ������) HBITMAP ��� mp_BkImgData
//void CVirtualConsole::FreeBackgroundImage()
//{
//	if (!this) return;
//    if (mp_BkImage)
//    {
//    	delete mp_BkImage;
//    	mp_BkImage = NULL;
//    }
//}

UINT CVirtualConsole::IsBackgroundValid(const CESERVER_REQ_SETBACKGROUND* apImgData, BOOL* rpIsEmf) const
{
	if (rpIsEmf)
		*rpIsEmf = FALSE;
		
	if (!apImgData)
		return 0;

	//if (IsBadReadPtr(apImgData, sizeof(CESERVER_REQ_SETBACKGROUND)))
	//	return 0;
	if (!apImgData->bEnabled)
		return (UINT)sizeof(CESERVER_REQ_SETBACKGROUND);

	if (apImgData->bmp.bfType == 0x4D42/*BM*/ && apImgData->bmp.bfSize)
	{
		#ifdef _DEBUG
		if (IsBadReadPtr(&apImgData->bmp, apImgData->bmp.bfSize))
		{
			_ASSERTE(!IsBadReadPtr(&apImgData->bmp, apImgData->bmp.bfSize));
			return 0;
		}
		#endif
		
		LPBYTE pBuf = (LPBYTE)&apImgData->bmp;

		if (*(u32*)(pBuf + 0x0A) >= 0x36 && *(u32*)(pBuf + 0x0A) <= 0x436 && *(u32*)(pBuf + 0x0E) == 0x28 && !pBuf[0x1D] && !*(u32*)(pBuf + 0x1E))
		{
			//UINT nSize = (UINT)sizeof(CESERVER_REQ_SETBACKGROUND) - (UINT)sizeof(apImgData->bmp)
			//           + apImgData->bmp.bfSize;
			UINT nSize = apImgData->bmp.bfSize
			             + (((LPBYTE)(void*)&(apImgData->bmp)) - ((LPBYTE)apImgData));
			return nSize;
		}
	}
	else if (apImgData->bmp.bfType == 0x4645/*EF*/ && apImgData->bmp.bfSize)
	{
		#ifdef _DEBUG
		if (IsBadReadPtr(&apImgData->bmp, apImgData->bmp.bfSize))
		{
			_ASSERTE(!IsBadReadPtr(&apImgData->bmp, apImgData->bmp.bfSize));
			return 0;
		}
		#endif
		
		// ��� EMF, �� ������� ��� BITMAPINFOHEADER
		if (rpIsEmf)
			*rpIsEmf = TRUE;
		
		LPBYTE pBuf = (LPBYTE)&apImgData->bmp;
		
		UINT nSize = apImgData->bmp.bfSize
		             + (((LPBYTE)(void*)&(apImgData->bmp)) - ((LPBYTE)apImgData));
		return nSize;
	}

	return 0;
}

// ���������� ��� ��������� ������ Background
SetBackgroundResult CVirtualConsole::SetBackgroundImageData(CESERVER_REQ_SETBACKGROUND* apImgData)
{
	if (!this) return esbr_Unexpected;

	//if (!gpConEmu->isMainThread())
	//{

	// ��� ������ �� ��������� ���� (������ ��� ������ �� �������)
	if (mp_RCon->isConsoleClosing())
		return esbr_ConEmuInShutdown;

	BOOL bIsEmf = FALSE;
	UINT nSize = IsBackgroundValid(apImgData, &bIsEmf);

	if (!nSize)
	{
		_ASSERTE(IsBackgroundValid(apImgData, NULL) != 0);
		return esbr_InvalidArg;
	}

	if (!apImgData->bEnabled)
	{
		//mb_BkImgDelete = TRUE;
		mb_BkImgExist = FALSE;
		gpSet->NeedBackgroundUpdate();
		Update(true/*bForce*/);
		return gpSet->isBgPluginAllowed ? esbr_OK : esbr_PluginForbidden;
	}

#ifdef _DEBUG

	if ((GetKeyState(VK_SCROLL) & 1))
	{
		static UINT nBackIdx = 0;
		wchar_t szFileName[32];
		_wsprintf(szFileName, SKIPLEN(countof(szFileName)) L"PluginBack_%04u.bmp", nBackIdx++);
		char szAdvInfo[512];
		BITMAPINFOHEADER* pBmp = (BITMAPINFOHEADER*)((&apImgData->bmp)+1);
		_wsprintfA(szAdvInfo, SKIPLEN(countof(szAdvInfo)) "\r\nnType=%i, bEnabled=%i,\r\nWidth=%i, Height=%i, Bits=%i, Encoding=%i\r\n",
		           apImgData->nType, apImgData->bEnabled,
		           pBmp->biWidth, pBmp->biHeight, pBmp->biBitCount, pBmp->biCompression);
		HANDLE hFile = CreateFile(szFileName, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD cbWrite;
			WriteFile(hFile, &apImgData->bmp, apImgData->bmp.bfSize, &cbWrite, 0);
			WriteFile(hFile, szAdvInfo, lstrlenA(szAdvInfo), &cbWrite, 0);
			CloseHandle(hFile);
		}
	}

#endif

	//	// ��������� ����� ����������� (����� ���������� � ������), �� ����� ������� ����� ������
	//	CESERVER_REQ_SETBACKGROUND* pCopy = (CESERVER_REQ_SETBACKGROUND*)malloc(nSize);
	//	if (!pCopy)
	//		return esbr_Unexpected;
	//	memmove(pCopy, apImgData, nSize);
	//	// ��������� ��������� ����������, � ������� � ������� ����
	//	mp_LastImgData = pCopy;
	//	mb_BkImgDelete = FALSE;
	//	gpConEmu->PostSetBackground(this, pCopy);
	//	return gpSet->isBgPluginAllowed ? esbr_OK : esbr_PluginForbidden;
	//}

	//// ���� ����� ������ �� ����� �������� ������� - ������������
	//if (mp_RCon->isConsoleClosing())
	////// ���� apImgData ��� �� ��������. �� ����� ��������� ��������� ������ ����� Background.
	////	|| (mp_LastImgData && mp_LastImgData != apImgData))
	//{
	//	free(apImgData);
	//	return esbr_Unexpected;
	//}

	// ������ �� ���������� - �� ����������. ��� ������ ��������������, � ���� ����������� ���������� � ����������������
	//mp_LastImgData = NULL;

	//UINT nSize = IsBackgroundValid(apImgData);
	//if (!nSize)
	//{
	//	// �� ���������� apImgData. ����� ������ ���� �� ������ - ��� ��� ���������
	//	_ASSERTE(IsBackgroundValid(apImgData) != 0);
	//	//free(apImgData);
	//	return esbr_InvalidArg;
	//}

	//MSectionLock SBK; SBK.Lock(&csBkImgData);
	//_ASSERTE(gpConEmu->isMainThread());

	if (!mcs_BkImgData)
		mcs_BkImgData = new MSection();

	MSectionLock SC;
	SC.Lock(mcs_BkImgData, TRUE);

	if (bIsEmf)
	{
		if (!mp_BkEmfData || mn_BkEmfDataMax < nSize)
		{
			if (mp_BkEmfData)
			{
				free(mp_BkEmfData); mp_BkEmfData = NULL;
				mb_BkImgChanged = mb_BkEmfChanged = TRUE;
				mb_BkImgExist = FALSE;
				mn_BkImgWidth = mn_BkImgHeight = 0;
			}

			mn_BkEmfDataMax = nSize+8192;
			mp_BkEmfData = (CESERVER_REQ_SETBACKGROUND*)malloc(mn_BkEmfDataMax);
		}
	}
	else
	{
		if (!mp_BkImgData || mn_BkImgDataMax < nSize)
		{
			if (mp_BkImgData)
			{
				free(mp_BkImgData); mp_BkImgData = NULL;
				mb_BkImgChanged = TRUE;
				mb_BkImgExist = FALSE;
				mn_BkImgWidth = mn_BkImgHeight = 0;
			}

			mp_BkImgData = (CESERVER_REQ_SETBACKGROUND*)malloc(nSize);
		}
	}
	
	SetBackgroundResult rc;

	if (!(bIsEmf ? mp_BkEmfData : mp_BkImgData))
	{
		_ASSERTE((bIsEmf ? mp_BkEmfData : mp_BkImgData)!=NULL);
		rc = esbr_Unexpected;
	}
	else
	{
		if (bIsEmf)
			memmove(mp_BkEmfData, apImgData, nSize);
		else
			memmove(mp_BkImgData, apImgData, nSize);
		mb_BkImgChanged = TRUE;
		mb_BkEmfChanged = bIsEmf;
		mb_BkImgExist = TRUE;
		BITMAPINFOHEADER* pBmp = bIsEmf ? (&mp_BkEmfData->bi) : (&mp_BkImgData->bi);
		mn_BkImgWidth = pBmp->biWidth;
		mn_BkImgHeight = pBmp->biHeight;
		gpSet->NeedBackgroundUpdate();

		//// ��� ���� ����� ������ - ����� ����������
		//free(apImgData); apImgData = NULL;

		if (gpConEmu->isVisible(this) && gpSet->isBgPluginAllowed)
		{
			Update(true/*bForce*/);
		}
		
		rc = esbr_OK;
	}
	
	return rc;
}

bool CVirtualConsole::HasBackgroundImage(LONG* pnBgWidth, LONG* pnBgHeight)
{
	if (!this) return false;

	if (!mp_RCon || !mp_RCon->isFar()) return false;

	if (!mb_BkImgExist || !(mp_BkImgData || (mp_BkEmfData && mb_BkEmfChanged))) return false;

	// ���������� mn_BkImgXXX ����� �� ������������ �� ��������� mp_BkImgData

	if (pnBgWidth)
		*pnBgWidth = mn_BkImgWidth;

	if (pnBgHeight)
		*pnBgHeight = mn_BkImgHeight;

	return (mn_BkImgWidth != 0 && mn_BkImgHeight != 0);
	//MSectionLock SBK; SBK.Lock(&csBkImgData);
	//if (mp_BkImgData)
	//{
	//	BITMAPINFOHEADER* pBmp = (BITMAPINFOHEADER*)(mp_BkImgData+1);
	//	if (pnBgWidth)
	//		*pnBgWidth = pBmp->biWidth;
	//	if (pnBgHeight)
	//		*pnBgHeight = pBmp->biHeight;
	//}
	//return mp_BkImgData;
}
