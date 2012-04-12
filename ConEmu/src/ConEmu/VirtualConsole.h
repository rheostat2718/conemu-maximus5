
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

#pragma once
//#include "kl_parts.h"
//#include "../Common/common.hpp"
#include "../Common/ConEmuCheck.h"
#include "VConRelease.h"

#include "Options.h"
#include "RealConsole.h"
#include "VConChild.h"

#define MAX_COUNT_PART_BRUSHES 16*16*4
#define MAX_SPACES 0x400

class CBackground;
class CTaskBarGhost;

class CVirtualConsole :
	public CVConRelease,
	public CConEmuChild
{
	private:
		// RealConsole
		CRealConsole  *mp_RCon;
		CTaskBarGhost *mp_Ghost;
	public:
		CVirtualConsole(const RConStartArgs *args);
		static CVirtualConsole* CreateVCon(RConStartArgs *args);
		void InitGhost();
	protected:
		virtual ~CVirtualConsole();
		friend class CVConRelease;
	public:
		CRealConsole *RCon();
		HWND GuiWnd();
		HWND GhostWnd();
		bool isVisible();
		int GetTabCount();
		int GetActiveTab();
		bool GetTab(int tabIdx, /*OUT*/ ConEmuTab* pTab);
	public:
		WARNING("������� protected!");
		uint TextWidth, TextHeight; // ������ � ��������
		uint Width, Height; // ������ � ��������
		bool LoadConsoleData();
	private:
		uint nMaxTextWidth, nMaxTextHeight; // ������ � ��������
	private:
		struct
		{
			bool isVisible;
			bool isVisiblePrev;
			bool isVisiblePrevFromInfo;
			bool isPrevBackground;
			short x;
			short y;
			COLORREF foreColor;
			COLORREF bgColor;
			BYTE foreColorNum, bgColorNum;
			wchar_t ch;
			DWORD nBlinkTime, nLastBlink;
			RECT lastRect;
			UINT lastSize; // ���������� ������ ������� (� ���������)
		} Cursor;
		//
		bool    mb_IsForceUpdate; // ��� ��������������� � InitDC, ����� �������� isForce �� ���������
		bool    mb_RequiredForceUpdate; // �������� �����, ��������...
		bool    isForce; // � ��� - ������ (��������������� �� ��������� � Update)
		DWORD   mn_LastBitsPixel;
	private:
		bool    mb_InUpdate;
		CEDC    hDC;
		HBITMAP hBitmap;
		HBRUSH  hBrush0, hOldBrush, hSelectedBrush;
		CEFONT  hSelectedFont, hOldFont;
		CEFONT  mh_FontByIndex[MAX_FONT_STYLES+1]; // ������ �� Normal/Bold/Italic/Bold&Italic/...Underline
		HFONT   mh_UCharMapFont; SMALL_RECT mrc_UCharMap;
		wchar_t ms_LastUCharMapFont[32];
		
		#ifdef _DEBUG
		bool    mb_DebugDumpDC;
		#endif
		
		bool    mb_ConDataChanged;
		HRGN    mh_TransparentRgn;
		//
		bool	mb_ChildWindowWasFound;
	public:
		bool InitDC(bool abNoDc, bool abNoWndResize, MSectionLock *pSDC, MSectionLock *pSCON);
	private:
		enum _PartType
		{
			pNull=0,     // ����� ������/���������, �������������� �������
			pSpace,      // ��� ������� ������ ����� ��������, ���� ����� pText,pSpace,pText �� pSpace,pText �������� � ������ pText
			pBorder,     // �������, ������� ����� �������� ������� hFont2
			pFills,      // Progressbars & Scrollbars
			pText,       // ���� ����� �� OEM  (����� ����� TextOutW)
			pOemText,    // ���� ����� OEM-��� (����� ����� ������ ����� TextOutA)
			pDummy  // �������������� "�������", ������� ����� ���������� ����� ����� ������
			//pUnderscore, // '_' �������. �� ���� ����� ������ � ����� ������
		};
		enum _PartType GetCharType(wchar_t ch);
		typedef struct _TextParts
		{
			enum _PartType partType;
			int  nFontIdx;   // ������ ������������� ������
			COLORREF crFore; // ���� ������
			COLORREF crBack; // !!! ������������ ������ ��� ��������� ������� �������� (���������� � ��.)
			uint i;     // ������ � ������� ������ (0-based)
			uint n;     // ���������� �������� � �����
			int  x;     // ���������� ������ ������ (may be >0)
			uint width; // ������ �������� ����� � ��������

			DWORD *pDX; // ������ ��� ��������� (���-�� ������ ���������). ��� ��������� �� ����� ConCharDX
		} TEXTPARTS;
		typedef struct _BgParts
		{
			// ������ ������
			uint i; // i ������ ������ 0 (0 - �������� ��� � �� ��� �����)
			// � ���������� �����
			uint n;
			// �������� ��� ���?
			BOOL bBackIsPic;
			COLORREF nBackRGB;
		} BGPARTS;

		// Working pointers
		bool mb_PointersAllocated;
		wchar_t  *mpsz_ConChar, *mpsz_ConCharSave;   // nMaxTextWidth * nMaxTextHeight
		// CharAttr ��������� � RealConsole.h
		CharAttr *mpn_ConAttrEx, *mpn_ConAttrExSave; // nMaxTextWidth * nMaxTextHeight
		DWORD *ConCharX;      // nMaxTextWidth * nMaxTextHeight
		DWORD *ConCharDX;     // nMaxTextWidth
		char  *tmpOem;        // nMaxTextWidth
		TEXTPARTS *TextParts; // nMaxTextWidth + 1
		BGPARTS *BgParts;     // nMaxTextWidth
		POLYTEXT  *PolyText;  // nMaxTextWidth
		bool *pbLineChanged;  // nMaxTextHeight
		bool *pbBackIsPic;    // nMaxTextHeight :: ����������� ���� *pbLineChanged
		COLORREF* pnBackRGB;  // nMaxTextHeight :: ����������� ���� *pbLineChanged � �� *pbBackIsPic

		// ������� ��������� ������
		void PointersInit();
		void PointersFree();
		bool PointersAlloc();
		void PointersZero();

		// *** ������ ����� ***
		// ������� ���������� ����� �������� ����� � ���������� pbLineChanged, pbBackIsPic, pnBackRGB
		void Update_CheckAndFill();
		// ������ ������ �� ������������ (���������� true, ���� ���� ������ � �� �������� �����)
		// ������� ����� ���������� ������������� (���������� ��������� � DX)
		bool Update_ParseTextParts(uint row, const wchar_t* ConCharLine, const CharAttr* ConAttrLine);
		// ������� ����� � �� �������� �����, ��������� ���������� � �����
		void Update_FillAlternate(uint row, uint nY);
		// ����� ���������� ������ (��� ������������� Clipped)
		void Update_DrawText(uint row, uint nY);

		// ������������� �����
		void DistributeSpaces(wchar_t* ConCharLine, CharAttr* ConAttrLine, DWORD* ConCharXLine, const int j, const int j2, const int end);

		// PanelViews
		PanelViewInit m_LeftPanelView, m_RightPanelView;
		bool mb_LeftPanelRedraw, mb_RightPanelRedraw;
		SMALL_RECT mrc_LastDialogs[MAX_DETECTED_DIALOGS]; int mn_LastDialogsCount; DWORD mn_LastDialogFlags[MAX_DETECTED_DIALOGS];
		SMALL_RECT mrc_Dialogs[MAX_DETECTED_DIALOGS]; int mn_DialogsCount; DWORD mn_DialogAllFlags, mn_DialogFlags[MAX_DETECTED_DIALOGS];
		bool UpdatePanelView(bool abLeftPanel, bool abOnRegister=false);
		CRgnRects m_RgnTest, m_RgnLeftPanel, m_RgnRightPanel;
		bool UpdatePanelRgn(bool abLeftPanel, bool abTestOnly=FALSE, bool abOnRegister=FALSE);
		void PolishPanelViews();
		bool CheckDialogsChanged();
		bool mb_DialogsChanged;
		UINT mn_ConEmuFadeMsg;
		UINT mn_ConEmuSettingsMsg;

		void CharAttrFromConAttr(WORD conAttr, CharAttr* pAttr);
	public:
		const PanelViewInit* GetPanelView(bool abLeftPanel);

	public:
		// ������ � ���� ����� ���������� ���� "��������" ��� ������� (��������, ���������� � ���� ����� �����)
		bool PutBackgroundImage(CBackground* pBack, LONG X, LONG Y, LONG Width, LONG Height); // �������� � pBack ���� ��������
		//void FreeBackgroundImage(); // ���������� (���� ������) HBITMAP ��� mp_BkImgData
		SetBackgroundResult SetBackgroundImageData(CESERVER_REQ_SETBACKGROUND* apImgData); // ���������� ��� ��������� ������ Background
		bool HasBackgroundImage(LONG* pnBgWidth, LONG* pnBgHeight);
		void NeedBackgroundUpdate();
	protected:
		bool mb_NeedBgUpdate;
		bool mb_BgLastFade;
		bool PrepareBackground(HDC* phBgDc, COORD* pbgBmpSize);
		CBackground* mp_Bg;
		MSection *mcs_BkImgData;
		size_t mn_BkImgDataMax;
		CESERVER_REQ_SETBACKGROUND* mp_BkImgData; // followed by image data
		bool mb_BkImgChanged; // ������ � mp_BkImgData ���� �������� ��������, ��������� ���������
		bool mb_BkImgExist; //, mb_BkImgDelete;
		LONG mn_BkImgWidth, mn_BkImgHeight;
		// ��������� EMF
		size_t mn_BkEmfDataMax;
		CESERVER_REQ_SETBACKGROUND* mp_BkEmfData; // followed by EMF data
		bool mb_BkEmfChanged; // ������ � mp_BkEmfData ���� �������� ��������, ��������� ���������
		//// ��� ��������, ��� ��������� � �������� ���� �������� �������� ����������
		//const CESERVER_REQ_SETBACKGROUND* mp_LastImgData;
		UINT IsBackgroundValid(const CESERVER_REQ_SETBACKGROUND* apImgData, bool* rpIsEmf) const; // ���������� ������ ������, ��� 0 ��� ������
//public:
		//MSection csBkImgData;

		const Settings::AppSettings* mp_Set;

	public:
		bool isEditor, isViewer, isFilePanel, isFade, isForeground;
		BYTE attrBackLast;
		COLORREF *mp_Colors;

		//wchar_t *Spaces; WORD nSpaceCount;
		static wchar_t ms_Spaces[MAX_SPACES], ms_HorzDbl[MAX_SPACES], ms_HorzSingl[MAX_SPACES];
		// ��� ��������� ��������� �������� �����
		//BYTE  m_ForegroundColors[0x100], m_BackgroundColors[0x100];
		//HFONT mh_FontByIndex[0x100]; // �������� ������ (�� �����) �� ������ normal/bold/italic

		bool  mb_LastFadeFlag;

		void DumpConsole();
		bool LoadDumpConsole();
		bool Dump(LPCWSTR asFile);
		bool Update(bool abForce = false, HDC *ahDc=NULL);
		void UpdateCursor(bool& lRes);
		void UpdateThumbnail(bool abNoSnapshoot = FALSE);
		void SelectFont(CEFONT hNew);
		void SelectBrush(HBRUSH hNew);
		inline bool isCharBorder(wchar_t inChar);
		static bool isCharBorderVertical(wchar_t inChar);
		static bool isCharProgress(wchar_t inChar);
		static bool isCharScroll(wchar_t inChar);
		static bool isCharNonSpacing(wchar_t inChar);
		static bool isCharSpace(wchar_t inChar);
		void BlitPictureTo(int inX, int inY, int inWidth, int inHeight);
		bool CheckSelection(const CONSOLE_SELECTION_INFO& select, SHORT row, SHORT col);
		//bool GetCharAttr(wchar_t ch, WORD atr, wchar_t& rch, BYTE& foreColorNum, BYTE& backColorNum, FONT* pFont);
		void Paint(HDC hPaintDc, RECT rcClient);
		void StretchPaint(HDC hPaintDC, int anX, int anY, int anShowWidth, int anShowHeight);
		void UpdateInfo();
		//void GetConsoleCursorInfo(CONSOLE_CURSOR_INFO *ci) { mp_RCon->GetConsoleCursorInfo(ci); };
		//DWORD GetConsoleCP() { return mp_RCon->GetConsoleCP(); };
		//DWORD GetConsoleOutputCP() { return mp_RCon->GetConsoleOutputCP(); };
		//DWORD GetConsoleMode() { return mp_RCon->GetConsoleMode(); };
		//void GetConsoleScreenBufferInfo(CONSOLE_SCREEN_BUFFER_INFO *sbi) { mp_RCon->GetConsoleScreenBufferInfo(sbi); };
		RECT GetRect();
		RECT GetDcClientRect();
		void OnFontChanged();
		COORD ClientToConsole(LONG x, LONG y, bool StrictMonospace=false);
		POINT ConsoleToClient(LONG x, LONG y);
		void OnConsoleSizeChanged();
		static void ClearPartBrushes();
		HRGN GetExclusionRgn(bool abTestOnly=false);
		COORD FindOpaqueCell();
		void ShowPopupMenu(POINT ptCur);
		void ExecPopupMenuCmd(int nCmd);
		bool RegisterPanelView(PanelViewInit* ppvi);
		void OnPanelViewSettingsChanged();
		bool IsPanelViews();
		bool CheckTransparent();
		void OnTitleChanged();
		void SavePaneSnapshoot();
		void OnTaskbarSettingsChanged();
		void OnTaskbarFocus();

	protected:
		//inline void GetCharAttr(WORD atr, BYTE& foreColorNum, BYTE& backColorNum, HFONT* pFont);
		wchar_t* mpsz_LogScreen; DWORD mn_LogScreenIdx;
		CONSOLE_SCREEN_BUFFER_INFO csbi; DWORD mdw_LastError;
		CONSOLE_CURSOR_INFO	cinf;
		COORD winSize, coord;
		//CONSOLE_SELECTION_INFO select1, select2;
		uint TextLen;
		bool isCursorValid, drawImage, textChanged, attrChanged;
		DWORD nBgImageColors;
		COORD bgBmpSize; HDC hBgDc;
		void UpdateCursorDraw(HDC hPaintDC, RECT rcClient, COORD pos, UINT dwSize);
		bool UpdatePrepare(HDC *ahDc, MSectionLock *pSDC, MSectionLock *pSCON);
		void UpdateText(); //, bool updateText, bool updateCursor);
		WORD CharWidth(wchar_t ch);
		void CharABC(wchar_t ch, ABC *abc);
		bool CheckChangedTextAttr();
		bool CheckTransparentRgn(bool abHasChildWindows);
		//void ParseLine(int row, wchar_t *ConCharLine, WORD *ConAttrLine);
		HANDLE mh_Heap;
		LPVOID Alloc(size_t nCount, size_t nSize);
		void Free(LPVOID ptr);
		MSection csCON;
		int mn_BackColorIdx; //==0
		void Box(LPCTSTR szText);
		static char mc_Uni2Oem[0x10000];
		char Uni2Oem(wchar_t ch);
		typedef struct tag_PARTBRUSHES
		{
			wchar_t ch; // 0x2591 0x2592 0x2593 0x2588 - �� ���������� ���������
			COLORREF nBackCol;
			COLORREF nForeCol;
			HBRUSH  hBrush;
		} PARTBRUSHES;
		//std::vector<PARTBRUSHES> m_PartBrushes;
		static PARTBRUSHES m_PartBrushes[MAX_COUNT_PART_BRUSHES];
		//static HBRUSH PartBrush(wchar_t ch, SHORT nBackIdx, SHORT nForeIdx);
		static HBRUSH PartBrush(wchar_t ch, COLORREF nBackCol, COLORREF nForeCol);
		bool mb_InPaintCall;
		bool mb_InConsoleResize;
		//
		bool FindChanges(int row, int &j, int &end, const wchar_t* ConCharLine, const CharAttr* ConAttrLine, const wchar_t* ConCharLine2, const CharAttr* ConAttrLine2);
		LONG nFontHeight, nFontWidth;
		BYTE nFontCharSet;
		BYTE nLastNormalBack;
		//bool bExtendFonts, bExtendColors;
		//BYTE nFontNormalColor, nFontBoldColor, nFontItalicColor, nExtendColor;
		struct _TransparentInfo
		{
			INT    nRectCount;
			POINT *pAllPoints;
			INT   *pAllCounts;
		} TransparentInfo;
		static HMENU mh_PopupMenu, mh_TerminatePopup, mh_DebugPopup, mh_EditPopup;
	protected:
		virtual void OnDestroy();
};
