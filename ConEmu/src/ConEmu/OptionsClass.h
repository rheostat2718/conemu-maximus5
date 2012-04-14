
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

#include "../Common/WinObjects.h"
#include <commctrl.h>

#include "Options.h"
#include "CustomFonts.h"

class CBackground;
struct DebugLogShellActivity;

class CSettings
{
	public:
		CSettings();
		~CSettings();
	private:
		Settings m_Settings;
		void ReleaseHandles();
		void InitVars_Hotkeys();
		void InitVars_Pages();
	public:

		private:
			wchar_t ConfigPath[MAX_PATH], ConfigName[240];
		public:
		LPCWSTR GetConfigPath();
		LPCWSTR GetConfigName();
		void SetConfigName(LPCWSTR asConfigName);

		wchar_t szFontError[512];

		bool SingleInstanceArg;
		void ResetCmdArg();

		//int DefaultBufferHeight;
		bool bForceBufferHeight; int nForceBufferHeight;
		bool AutoScroll;
		//bool AutoBufferHeight;
		//bool FarSyncSize;
		//int nCmdOutputCP;

		LPCWSTR FontFaceName();
		LONG FontWidth();
		LONG FontHeight();
		LPCWSTR BorderFontFaceName();
		LONG BorderFontWidth();
		BYTE FontCharSet();
		BOOL FontBold();
		BOOL FontItalic();
		BOOL FontClearType();
		BYTE FontQuality();
		HFONT CreateOtherFont(const wchar_t* asFontName);
	private:
		//struct CEFontRange
		//{
		//public:
		//	bool    bUsed;              // ��� �������� ���������/����������
		//	wchar_t sTitle[64];         // "Title"="Borders and scrollbars"
		//	wchar_t sRange[1024];       // "CharRange"="2013-25C4;"
		//	wchar_t sFace[LF_FACESIZE]; // "FontFace"="Andale Mono"
		//	LONG    nHeight;            // "Height"=dword:00000012
		//	LONG    nWidth;             // "Width"=dword:00000000
		//	LONG    nLoadHeight;        // ��� ����������� ���������� ������������� �������� ������ ���
		//	LONG    nLoadWidth;         // ������� GUI, ��� ��������� �� ������� - ��������� ��������
		//	BYTE    nCharset;           // "Charset"=hex:00
		//	bool    bBold;              // "Bold"=hex:00
		//	bool    bItalic;            // "Italic"=hex:00
		//	BYTE    nQuality;           // "Anti-aliasing"=hex:03
		//	/*    */
		//private:
		//	LOGFONT LF;
		//public:
		//	LOGFONT& LogFont()
		//	{
		//		LF.lfHeight = nHeight;
		//		LF.lfWidth = nWidth;
		//		LF.lfEscapement = 0;
		//		LF.lfOrientation = 0;
		//		LF.lfWeight = bBold?FW_BOLD:FW_NORMAL;
		//		LF.lfItalic = bItalic ? 1 : 0;
		//		LF.lfUnderline = 0;
		//		LF.lfStrikeOut = 0;
		//		LF.lfQuality = nQuality;
		//		LF.lfCharSet = nCharset;
		//		LF.lfOutPrecision = OUT_TT_PRECIS;
		//		LF.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		//		LF.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
		//		lstrcpyn(LF.lfFaceName, sFace, countof(LF.lfFaceName));
		//		return LF;
		//	};
		//	void Scale(LONG anMainHeight, LONG anMainLoadHeight)
		//	{
		//		// ������
		//		if (nLoadHeight == 0 || !anMainLoadHeight)
		//		{
		//			nHeight = anMainHeight; // ������ ����� ������ � Main
		//		}
		//		else
		//		{
		//			nHeight = nLoadHeight * anMainHeight / anMainLoadHeight;
		//		}
		//		// ������
		//		if (nLoadWidth == 0 || !anMainLoadHeight)
		//		{
		//			nWidth = 0;
		//		}
		//		else
		//		{
		//			nWidth = nLoadWidth * anMainHeight / anMainLoadHeight;
		//		}
		//	};
		//	/*    */
		//	HFONT hFonts[MAX_FONT_STYLES]; //normal/(bold|italic|underline)
		//	/*    */
		//	BYTE RangeData[0x10000];
		//};
		//CEFontRange m_Fonts[MAX_FONT_GROUPS]; // 0-Main, 1-Borders, 2 � ����� - user defined
		//BOOL FontRangeLoad(SettingsBase* reg, int Idx);
		//BOOL FontRangeSave(SettingsBase* reg, int Idx);
		LOGFONT LogFont, LogFont2;
		LONG mn_AutoFontWidth, mn_AutoFontHeight; // ������� �������, ������� ���� ��������� ��� ����������� ������
		LONG mn_FontWidth, mn_FontHeight, mn_BorderFontWidth; // �������� ������� �������
		//BYTE mn_LoadFontCharSet; // �� ��� ��������� ���������� (��� ��� ��������� � ������)
		TEXTMETRIC m_tm[MAX_FONT_STYLES];
		LPOUTLINETEXTMETRIC m_otm[MAX_FONT_STYLES];
		BOOL mb_Name1Ok, mb_Name2Ok;
		void ResetFontWidth();
		void SaveFontSizes(LOGFONT *pCreated, bool bAuto, bool bSendChanges);
		LPOUTLINETEXTMETRIC LoadOutline(HDC hDC, HFONT hFont);
		void DumpFontMetrics(LPCWSTR szType, HDC hDC, HFONT hFont, LPOUTLINETEXTMETRIC lpOutl = NULL);
	public:
		//bool isFontAutoSize;
		//bool isAutoRegisterFonts;
		//wchar_t FontFile[MAX_PATH];
		//LOGFONT ConsoleFont;
		//COLORREF* GetColors(BOOL abFade = FALSE);
		//COLORREF GetFadeColor(COLORREF cr);
		//bool NeedDialogDetect();
		//COLORREF ColorKey;
		//bool isExtendColors;
		//char nExtendColor;
		//bool isExtendFonts, isTrueColorer;
		//char nFontNormalColor, nFontBoldColor, nFontItalicColor;

		/* Background image */
		//WCHAR sBgImage[MAX_PATH];
		//char isShowBgImage;
		bool isBackgroundImageValid;
		//u8 bgImageDarker;
		//DWORD nBgImageColors;
		//char bgOperation; // BackgroundOp {eUpLeft = 0, eStretch = 1, eTile = 2}
		//char isBgPluginAllowed;

		/* Transparency */
		//u8 nTransparent;
		//bool isUserScreenTransparent;

		/* Command Line History (from start dialog) */
		//LPWSTR psCmdHistory; DWORD nCmdHistorySize;

		/* Command Line (Registry) */
		//LPTSTR psCmd;
		/* Command Line ("/cmd" arg) */
		//LPTSTR psCurCmd;
	private:
		/* 'Default' command line (if nor Registry, nor /cmd specified) */
		WCHAR  szDefCmd[16];
	public:
		/* "Active" command line */
		//LPCTSTR GetCmd();
		/* "Default" command line "far/cmd", based on /BufferHeight switch */
		LPCTSTR GetDefaultCmd();
		/* OUR(!) startup info */
		STARTUPINFOW ourSI;
		/* If Attach to PID requested */
		DWORD nAttachPID; HWND hAttachConWnd;

		//DWORD FontSizeY;  // ������ ��������� ������ (����������� �� ��������!)
		//DWORD FontSizeX;  // ������ ��������� ������
		//DWORD FontSizeX2; // ������ ��� FixFarBorders (������ ������������ ������ ��� ��������� �����, �� ������ �� �����������)
		//DWORD FontSizeX3; // ������ ���������� ��� ���������� ������ (�� ������ � FontSizeX2)
		//bool isFullScreen, isHideCaption;
		//bool isHideCaptionAlways();
		//BYTE nHideCaptionAlwaysFrame;
		//DWORD nHideCaptionAlwaysDelay, nHideCaptionAlwaysDisappear;
		//bool isDownShowHiddenMessage;
		//bool isAlwaysOnTop, isDesktopMode;
		//BYTE isFixFarBorders;
		//bool isExtendUCharMap;
		//bool isDisableMouse;
		//bool isMouseSkipActivation, isMouseSkipMoving;
		//bool isFarHourglass; DWORD nFarHourglassDelay;
		//BYTE isDisableFarFlashing, isDisableAllFlashing;
		
		// Text selection
		//BYTE isConsoleTextSelection;
		//bool isCTSSelectBlock, isCTSSelectText;
		//BYTE isCTSVkBlock, isCTSVkText; // ����������� ������� ��������� ������
		//BYTE isCTSActMode, isCTSVkAct; // ����� � ����������� ���������� �������� ������ � ������� ������ �����
		//BYTE isCTSRBtnAction, isCTSMBtnAction; // 0-off, 1-copy, 2-paste
		//BYTE isCTSColorIndex;
		//bool isFarGotoEditor; // ������������ � ���������� �� ����/������ (������ �����������)
		//BYTE isFarGotoEditorVk; // �������-����������� ��� isFarGotoEditor
		//bool isModifierPressed(DWORD vk);
		//bool isSelectionModifierPressed();
	protected:
		//bool mb_HideCaptionAlways;
		//typedef struct tag_CharRanges
		//{
		//	bool bUsed;
		//	wchar_t cBegin, cEnd;
		//} CharRanges;
		//wchar_t mszCharRanges[120];
		//CharRanges icFixFarBorderRanges[10];
		//int ParseCharRanges(LPCWSTR asRanges, BYTE (&Chars)[0x10000], BYTE abValue = TRUE); // ��������, L"2013-25C3,25C4"
		//wchar_t* CreateCharRanges(BYTE (&Chars)[0x10000]); // caller must free(result)
		//BYTE mpc_FixFarBorderValues[0x10000];
		//BYTE m_isKeyboardHooks;
		
		BYTE isMonospaceSelected; // 0 - proportional, 1 - monospace, 2 - forcemonospace
	public:
		//bool isKeyboardHooks();
		//bool isCharBorder(wchar_t inChar);
		//BYTE isPartBrush75, isPartBrush50, isPartBrush25, isPartBrushBlack;
		//bool isCursorV;
		//bool isCursorBlink;
		//bool isCursorColor;
		//bool isCursorBlockInactive;
		//char isRClickSendKey;
		//wchar_t *sRClickMacro;
		//bool isSafeFarClose;
		//wchar_t *sSafeFarCloseMacro;
		//bool isSendAltEnter;
		//bool isSendAltSpace;
		//bool isSendAltTab, isSendAltEsc, isSendAltPrintScrn, isSendPrintScrn, isSendCtrlEsc;
		//bool isMinToTray;
		//bool isAlwaysShowTrayIcon;
		//bool isForceMonospace, isProportional;
		//BYTE isMonospace, isMonospaceSelected; // 0 - proportional, 1 - monospace, 2 - forcemonospace
		//bool isUpdConHandle;
		//bool isRSelFix;
		//Drag
		//BYTE isDragEnabled;
		//BYTE isDropEnabled;
		//BYTE nLDragKey, nRDragKey; // ��� DWORD
		//bool isDefCopy;
		//BYTE isDragOverlay;
		//bool isDragShowIcons;
		//BYTE isDragPanel; // ��������� ������� ������� ������
		//bool
		//bool isDebugSteps;
		//bool isEnhanceGraphics; // Progressbars and scrollbars (pseudographics)
		//bool isFadeInactive;
		//DWORD nFadeInactiveMask;
		//char isTabs; bool isTabSelf, isTabRecent, isTabLazy, isTabsInCaption;

		// Tab theme properties
		//int ilDragHeight;

	//private:
	//	char m_isTabsOnTaskBar;
	public:
		//bool isTabsOnTaskBar();
		//wchar_t sTabFontFace[LF_FACESIZE]; DWORD nTabFontCharSet; int nTabFontHeight;
		//wchar_t *sTabCloseMacro;
		//wchar_t *sSaveAllMacro;
		//int nToolbarAddSpace;
		//DWORD wndWidth, wndHeight, ntvdmHeight; // � ��������
		//int wndX, wndY; // � ��������
		//bool wndCascade;
		//bool isAutoSaveSizePos;
	//private:
	//	// ��� �������� ���� ��������� - ��������� ������ ���� ���,
	//	// � �� ������ ����� � �������� �������� �������� ����������
	//	bool mb_SizePosAutoSaved;
	public:
		//DWORD nSlideShowElapse;
		//DWORD nIconID;
		//bool isTryToCenter;
		//BYTE isAlwaysShowScrollbar;
		//RECT rcTabMargins;
		//bool isTabFrame;
		//BYTE icMinimizeRestore;
		//bool isMulti;
		//BYTE icMultiNew, icMultiNext, icMultiRecreate, icMultiBuffer, icMultiClose, icMultiCmd;
		//bool isMultiAutoCreate, isMultiLeaveOnClose, isMultiIterate;
		//bool IsHostkey(WORD vk);
		//bool IsHostkeySingle(WORD vk);
		//bool IsHostkeyPressed();
		//WORD GetPressedHostkey();
		//UINT GetHostKeyMod(); // ����� ������ MOD_xxx ��� RegisterHotKey
		//bool isMultiNewConfirm, isUseWinNumber, isUseWinTab, isUseWinArrows;
		//bool isFARuseASCIIsort, isFixAltOnAltTab, isShellNoZoneCheck;

		// ��������� �����
		//WCHAR szTabConsole[32];
		//WCHAR szTabEditor[32];
		//WCHAR szTabEditorModified[32];
		//WCHAR szTabViewer[32];
		//DWORD nTabLenMax;

		//bool isVisualizer;
		//char nVizNormal, nVizFore, nVizTab, nVizEOL, nVizEOF;
		//wchar_t cVizTab, cVizEOL, cVizEOF;

		char isAllowDetach;
		//bool isCreateAppWindow;
		//bool NeedCreateAppWindow();
		/*bool isScrollTitle;
		DWORD ScrollTitleLen;*/
		//wchar_t szAdminTitleSuffix[64]; //" (Admin)"
		//bool bAdminShield, bHideInactiveConsoleTabs;

		//DWORD nMainTimerElapse; // �������������, � ������� �� ������� ����������� �����
		//DWORD nMainTimerInactiveElapse; // ������������� ��� ������������
		//bool isAdvLangChange; // � ����� ��� ConIme � ����� ������� �� �������� ����, ���� �� ������� WM_SETFOCUS. �� ��� ���� �������� ������ �������� ������
		//bool isSkipFocusEvents;
		//bool isLangChangeWsPlugin;
		//char isMonitorConsoleLang;
		//bool isSleepInBackground;

		//DWORD nAffinity;

		//bool isUseInjects;
		//bool isPortableReg;

		// Debugging - "c:\\temp\\ConEmuVCon-%i-%i.dat"
		BYTE isAdvLogging;
		//wchar_t szDumpPackets[MAX_PATH];
		// Debugging
		//bool isConVisible; // isLockRealConsolePos;
		//DWORD nConInMode;
		
		//
		enum GuiLoggingType m_ActivityLoggingType;
		DWORD mn_ActivityCmdStartTick;

		// Thumbnails and Tiles
		//PanelViewSetMapping ThSet;
		MFileMapping<PanelViewSetMapping> m_ThSetMap;

		// Working variables...
	private:
		//HBITMAP  hBgBitmap;
		//COORD    bgBmp;
		//HDC      hBgDc;
		CBackground* mp_Bg;
		//MSection mcs_BgImgData;
		BITMAPFILEHEADER* mp_BgImgData;
		BOOL mb_NeedBgUpdate; //, mb_WasVConBgImage;
		bool mb_BgLastFade;
		FILETIME ftBgModified;
		DWORD nBgModifiedTick;
	public:
		bool PrepareBackground(HDC* phBgDc, COORD* pbgBmpSize);
		bool PollBackgroundFile(); // true, ���� ���� �������
		bool /*LoadImageFrom*/LoadBackgroundFile(TCHAR *inPath, bool abShowErrors=false);
		bool IsBackgroundEnabled(CVirtualConsole* apVCon);
		void NeedBackgroundUpdate();
		//CBackground* CreateBackgroundImage(const BITMAPFILEHEADER* apBkImgData);
	public:
		CEFONT  mh_Font[MAX_FONT_STYLES], mh_Font2;
		TODO("�� ��������, CharWidth & CharABC ����� ��������� �� ������� - � Bold ������ ����� ���� ������");
		WORD    CharWidth[0x10000]; //, Font2Width[0x10000];
		ABC     CharABC[0x10000];

		//HWND hMain, hExt, hFar, hKeys, hTabs, hColors, hCmdTasks, hViews, hInfo, hDebug, hUpdate, hSelection;
		enum TabHwndIndex {
			thi_Main = 0,
			thi_Ext,
			thi_Selection,
			thi_Far,
			thi_Keys,
			thi_Tabs,
			thi_Colors,
			thi_Tasks,
			thi_Apps,
			thi_Views,
			thi_Debug,
			thi_Update,
			thi_Info,
			//
			thi_Last
		};
		HWND mh_Tabs[thi_Last];

		//static void CenterDialog(HWND hWnd2);
		void OnClose();
		// IDD_SETTINGS
		static INT_PTR CALLBACK wndOpProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		// ������� ��������: IDD_SPG_MAIN, IDD_SPG_FEATURE, � �.�.
		static INT_PTR CALLBACK pageOpProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		////static INT_PTR CALLBACK extOpProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		////static INT_PTR CALLBACK keysOpProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		////static INT_PTR CALLBACK tabsOpProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		////static INT_PTR CALLBACK colorOpProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		////static INT_PTR CALLBACK viewsOpProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		////static INT_PTR CALLBACK infoOpProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		////static INT_PTR CALLBACK debugOpProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		// IDD_MORE_HIDE
		//static INT_PTR CALLBACK hideOpProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		////static INT_PTR CALLBACK multiOpProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		// IDD_SPG_SELECTION
		//static INT_PTR CALLBACK selectionOpProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		//
		void debugLogShell(HWND hWnd2, DebugLogShellActivity *pShl);
		void debugLogShellText(wchar_t* &pszParamEx, LPCWSTR asFile);
		void debugLogInfo(HWND hWnd2, CESERVER_REQ_PEEKREADINFO* pInfo);
		void debugLogCommand(CESERVER_REQ* pInfo, BOOL abInput, DWORD anTick, DWORD anDur, LPCWSTR asPipe, CESERVER_REQ* pResult = NULL);
		//
		void SettingsLoaded();
		void SettingsPreSave();
		//void InitSettings();
		//BOOL SaveSettings(BOOL abSilent = FALSE);
		//void SaveSizePosOnExit();
		//void SaveConsoleFont();
		bool ShowColorDialog(HWND HWndOwner, COLORREF *inColor);
		static int CALLBACK EnumFamCallBack(LPLOGFONT lplf, LPNEWTEXTMETRIC lpntm, DWORD FontType, LPVOID aFontCount);
		static int CALLBACK EnumFontCallBackEx(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, DWORD FontType, LPARAM lParam);
		//void UpdateMargins(RECT arcMargins);
		static void Dialog();
		void UpdatePos(int x, int y);
		void UpdateSize(UINT w, UINT h);
		void UpdateTTF(BOOL bNewTTF);
		void UpdateFontInfo();
		void Performance(UINT nID, BOOL bEnd);
		void SetArgBufferHeight(int anBufferHeight);
		void InitFont(LPCWSTR asFontName=NULL, int anFontHeight=-1, int anQuality=-1);
		BOOL RegisterFont(LPCWSTR asFontFile, BOOL abDefault);
		void RegisterFonts();
	private:
		void RegisterFontsInt(LPCWSTR asFromDir);
	public:
		void UnregisterFonts();
		BOOL GetFontNameFromFile(LPCTSTR lpszFilePath, wchar_t (&rsFontName)[LF_FACESIZE], wchar_t (&rsFullFontName)[LF_FACESIZE]);
		BOOL GetFontNameFromFile_TTF(LPCTSTR lpszFilePath, wchar_t (&rsFontName)[LF_FACESIZE], wchar_t (&rsFullFontName)[LF_FACESIZE]);
		BOOL GetFontNameFromFile_OTF(LPCTSTR lpszFilePath, wchar_t (&rsFontName)[LF_FACESIZE], wchar_t (&rsFullFontName)[LF_FACESIZE]);
		BOOL GetFontNameFromFile_BDF(LPCTSTR lpszFilePath, wchar_t (&rsFontName)[LF_FACESIZE], wchar_t (&rsFullFontName)[LF_FACESIZE]);
		void UpdateConsoleMode(DWORD nMode);
		bool AutoRecreateFont(int nFontW, int nFontH);
		bool MacroFontSetSize(int nRelative/*0/1*/, int nValue/*1,2,...*/);
		void MacroFontSetName(LPCWSTR pszFontName, WORD anHeight /*= 0*/, WORD anWidth /*= 0*/);
		bool CheckTheming();
		void OnPanelViewAppeared(BOOL abAppear);
		bool EditConsoleFont(HWND hParent);
		static INT_PTR CALLBACK EditConsoleFontProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		static int CALLBACK EnumConFamCallBack(LPLOGFONT lplf, LPNEWTEXTMETRIC lpntm, DWORD FontType, LPVOID aFontCount);
		bool CheckConsoleFontFast();
		enum
		{
			ConFontErr_NonSystem   = 0x01,
			ConFontErr_NonRegistry = 0x02,
			ConFontErr_InvalidName = 0x04,
		};
	protected:
		BOOL bShowConFontError, bConsoleFontChecked;
		wchar_t sConFontError[512];
		wchar_t sDefaultConFontName[32]; // "��������� ����", ���� ���� ��������� ������� ���������� �����
		HWND hConFontDlg;
		DWORD nConFontError; // 0x01 - ����� �� ��������������� � �������, 0x02 - �� ������ � ������� ��� �������
		HWND hwndConFontBalloon;
		static bool CheckConsoleFontRegistry(LPCWSTR asFaceName);
		static bool CheckConsoleFont(HWND ahDlg);
		static void ShowConFontErrorTip(LPCTSTR asInfo);
		LPCWSTR CreateConFontError(LPCWSTR asReqFont=NULL, LPCWSTR asGotFont=NULL);
		TOOLINFO tiConFontBalloon;
	private:
		static void ShowErrorTip(LPCTSTR asInfo, HWND hDlg, int nCtrlID, wchar_t* pszBuffer, int nBufferSize, HWND hBall, TOOLINFO *pti, HWND hTip, DWORD nTimeout);
	protected:
		void OnResetOrReload(BOOL abResetSettings);
		// IDD_SETTINGS
		LRESULT OnInitDialog();
		// OnInitDialogPage_t: IDD_SPG_MAIN, � �.�.
		LRESULT OnInitDialog_Main(HWND hWnd2);
		LRESULT OnInitDialog_Ext(HWND hWnd2);
		LRESULT OnInitDialog_Selection(HWND hWnd2);
		LRESULT OnInitDialog_Far(HWND hWnd2, BOOL abInitial);
		LRESULT OnInitDialog_Keys(HWND hWnd2, BOOL abInitial);
		LRESULT OnInitDialog_Tabs(HWND hWnd2);
		LRESULT OnInitDialog_Color(HWND hWnd2);
		LRESULT OnInitDialog_Tasks(HWND hWnd2, bool abForceReload);
		LRESULT OnInitDialog_Apps(HWND hWnd2, bool abForceReload);
		LRESULT OnInitDialog_Views(HWND hWnd2);
		void OnInitDialog_CopyFonts(HWND hWnd2, int nList1, ...); // ����������� ������ ������� � ������� hMain
		LRESULT OnInitDialog_Debug(HWND hWnd2);
		LRESULT OnInitDialog_Update(HWND hWnd2);
		LRESULT OnInitDialog_Info(HWND hWnd2);
		//
		LRESULT OnButtonClicked(HWND hWnd2, WPARAM wParam, LPARAM lParam);
		LRESULT OnButtonClicked_Tasks(HWND hWnd2, WPARAM wParam, LPARAM lParam);
		bool mb_IgnoreCmdGroupEdit, mb_IgnoreCmdGroupList;
		//LRESULT OnButtonClicked_Apps(HWND hWnd2, WPARAM wParam, LPARAM lParam);
		//UINT mn_AppsEnableControlsMsg;
		INT_PTR pageOpProc_Apps(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		bool mb_IgnoreAppsEdit, mb_IgnoreAppsList;
		//LRESULT OnColorButtonClicked(HWND hWnd2, WPARAM wParam, LPARAM lParam);
		//LRESULT OnColorComboBox(HWND hWnd2, WPARAM wParam, LPARAM lParam);		
		//LRESULT OnColorEditChanged(HWND hWnd2, WPARAM wParam, LPARAM lParam);				
		LRESULT OnEditChanged(HWND hWnd2, WPARAM wParam, LPARAM lParam);				
		LRESULT OnComboBox(HWND hWnd2, WPARAM wParam, LPARAM lParam);
		LRESULT OnPage(LPNMHDR phdr);
		INT_PTR OnMeasureFontItem(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		INT_PTR OnDrawFontItem(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		void OnSaveActivityLogFile(HWND hListView);
		LRESULT OnActivityLogNotify(HWND hWnd2, WPARAM wParam, LPARAM lParam);
		void FillHotKeysList(HWND hWnd2, BOOL abInitial);
		LRESULT OnHotkeysNotify(HWND hWnd2, WPARAM wParam, LPARAM lParam);
		UINT mn_ActivateTabMsg;
		bool mb_IgnoreSelPage;
	private:
		bool GetColorById(WORD nID, COLORREF* color);
		bool SetColorById(WORD nID, COLORREF color);
		void ColorSetEdit(HWND hWnd2, WORD c);
		bool ColorEditDialog(HWND hWnd2, WORD c);
		void FillBgImageColors();
		HBRUSH mh_CtlColorBrush;
		INT_PTR ColorCtlStatic(HWND hWnd2, WORD c, HWND hItem);
		COLORREF acrCustClr[16]; // array of custom colors, ������������ � ChooseColor(...)
		BOOL mb_IgnoreEditChanged;
		BOOL mb_IgnoreTtfChange;
		//BOOL mb_CharSetWasSet;
		i64 mn_Freq;
		i64 mn_FPS[20]; int mn_FPS_CUR_FRAME;
		i64 mn_RFPS[20]; int mn_RFPS_CUR_FRAME;
		i64 mn_Counter[tPerfInterval-gbPerformance];
		i64 mn_CounterMax[tPerfInterval-gbPerformance];
		DWORD mn_CounterTick[tPerfInterval-gbPerformance];
		HWND hwndTip, hwndBalloon;
		static void ShowFontErrorTip(LPCTSTR asInfo);
		TOOLINFO tiBalloon;
		void RegisterTipsFor(HWND hChildDlg);
		CEFONT CreateFontIndirectMy(LOGFONT *inFont);
		void RecreateFont(WORD wFromID);
#if 0
		// Theming
		HMODULE mh_Uxtheme;
		typedef HRESULT(STDAPICALLTYPE *SetWindowThemeT)(HWND hwnd,LPCWSTR pszSubAppName,LPCWSTR pszSubIdList);
		SetWindowThemeT SetWindowThemeF;
		typedef HRESULT(STDAPICALLTYPE *EnableThemeDialogTextureT)(HWND hwnd,DWORD dwFlags);
		EnableThemeDialogTextureT EnableThemeDialogTextureF;
#endif
		UINT mn_MsgUpdateCounter;
		//wchar_t temp[MAX_PATH];
		UINT mn_MsgRecreateFont;
		UINT mn_MsgLoadFontFromMain;
		static int IsChecked(HWND hParent, WORD nCtrlId);
		static int GetNumber(HWND hParent, WORD nCtrlId);
		static INT_PTR GetString(HWND hParent, WORD nCtrlId, wchar_t** ppszStr, LPCWSTR asNoDefault = NULL);
		static int SelectString(HWND hParent, WORD nCtrlId, LPCWSTR asText);
		static int SelectStringExact(HWND hParent, WORD nCtrlId, LPCWSTR asText);
		BOOL mb_TabHotKeyRegistered;
		void RegisterTabs();
		void UnregisterTabs();
		static DWORD CALLBACK EnumFontsThread(LPVOID apArg);
		HANDLE mh_EnumThread;
		WORD mn_LastChangingFontCtrlId;
		// �������� �������������� ������
		typedef struct tag_RegFont
		{
			BOOL    bDefault;             // ���� ����� ������������ ������ ����� /fontfile
			CustomFontFamily* pCustom;    // ��� �������, ���������� ����
			wchar_t szFontFile[MAX_PATH]; // ������ ����
			wchar_t szFontName[32];       // Font Family
			BOOL    bUnicode;             // ���������?
			BOOL    bHasBorders;          // ����� �� ������ ����� ������� �����
			BOOL    bAlreadyInSystem;     // ����� � ����� ������ ��� ��� ��������������� � �������
		} RegFont;
		std::vector<RegFont> m_RegFonts;
		BOOL mb_StopRegisterFonts;
		////
		//COLORREF Colors[0x20];
		//bool mb_FadeInitialized;
		//BYTE mn_FadeLow, mn_FadeHigh;
		//DWORD mn_FadeMul;
		//COLORREF mn_LastFadeSrc, mn_LastFadeDst;
		//COLORREF ColorsFade[0x20];
		BOOL GetColorRef(HWND hDlg, WORD TB, COLORREF* pCR);
		//inline BYTE GetFadeColorItem(BYTE c);
		//
		bool mb_ThemingEnabled;
		//
		//bool TestHostkeyModifiers();
		//static BYTE CheckHostkeyModifier(BYTE vk);
		//static void ReplaceHostkey(BYTE vk, BYTE vkNew);
		//static void AddHostkey(BYTE vk);
		//static void TrimHostkeys();
		void SetupHotkeyChecks(HWND hWnd2);
		//static bool MakeHostkeyModifier();
		static BYTE HostkeyCtrlId2Vk(WORD nID);
		//DWORD nMultiHotkeyModifier;
		//BYTE mn_HostModOk[15], mn_HostModSkip[15];
		//bool isHostkeySingleLR(WORD vk, WORD vkC, WORD vkL, WORD vkR);
		static void FillListBoxItems(HWND hList, uint nItems, const WCHAR** pszItems, const DWORD* pnValues, DWORD& nValue, BOOL abExact = FALSE);
		static void GetListBoxItem(HWND hList, uint nItems, const WCHAR** pszItems, const DWORD* pnValues, DWORD& nValue);
		static void CenterMoreDlg(HWND hWnd2);
		static bool IsAlmostMonospace(LPCWSTR asFaceName, int tmMaxCharWidth, int tmAveCharWidth, int tmHeight);
	private:
		struct ConEmuHotKeys
		{
			int  DescrLangID;
			
			int  Type; // 0 - hotkey, 1 - modifier (��� �����, ��������)
			
			// User
			BYTE* VkPtr; // ���� NULL - ������ ���������, �� ����������
			
			// System or default
			BYTE Vk;
			DWORD Modifier; // System only, ��� "User" - ������������ "HostKey"
			
			TODO("���� ����� �� ��� �������� ���� �� ����� �������� � ��� ��� �����������");

			// Internal

		};
		#define MAKEMODIFIER2(vk1,vk2) ((DWORD)vk1&0xFF)|(((DWORD)vk2&0xFF)<<8)
		#define MAKEMODIFIER3(vk1,vk2,vk3) ((DWORD)vk1&0xFF)|(((DWORD)vk2&0xFF)<<8)|(((DWORD)vk3&0xFF)<<16)
		ConEmuHotKeys *m_HotKeys;
		ConEmuHotKeys *mp_ActiveHotKey;

		enum KeyListColumns
		{
			klc_Type = 0,
			klc_Hotkey,
			klc_Desc
		};
		enum LogProcessColumns
		{
			lpc_Time = 0,
			lpc_PPID,
			lpc_Func,
			lpc_Oper,
			lpc_Bits,
			lpc_System,
			lpc_App,
			lpc_Params,
			lpc_Flags,
			lpc_StdIn,
			lpc_StdOut,
			lpc_StdErr,
		};
		enum LogInputColumns
		{
			lic_Time = 0,
			lic_Type,
			lic_Dup,
			lic_Event,
		};
		enum LogCommandsColumns
		{
			lcc_InOut = 0,
			lcc_Time,
			lcc_Duration,
			lcc_Command,
			lcc_Size,
			lcc_PID,
			lcc_Pipe,
			lcc_Extra,
		};
		struct LogCommandsData
		{
			BOOL  bInput, bMainThread;
			DWORD nTick, nDur, nCmd, nSize, nPID;
			wchar_t szPipe[64];
			wchar_t szExtra[128];
		};
		void debugLogCommand(HWND hWnd2, LogCommandsData* apData);
		
		static void GetVkKeyName(BYTE vk, wchar_t (&szName)[128]);

		enum ConEmuSetupItemType
		{
			sit_Bool      = 1,
			sit_Byte      = 2,
			sit_Char      = 3,
			sit_Long      = 4,
			sit_ULong     = 5,
			sit_Rect      = 6,
			sit_FixString = 7,
			sit_VarString = 8,
			sit_MSZString = 9,
			sit_FixData   = 10,
			sit_Fonts     = 11,
			sit_FontsAndRaster = 12,
		};
		struct ConEmuSetupItem
		{
			//DWORD nDlgID; // ID �������
			DWORD nItemID; // ID �������� � �������, 0 - ��������� ������� � ������
			ConEmuSetupItemType nDataType; // ��� ������

			void* pData; // ������ �� ������� � gpSet
			size_t nDataSize; // ������ ��� maxlen, ���� pData �������������

			ConEmuSetupItemType nListType; // ��� ������ � pListData
			const void* pListData; // ��� DDLB - ����� ������ ������
			size_t nListItems; // ���������� ��������� � ������
			
			#ifdef _DEBUG
			BOOL bFound; // ��� ������� ������������ ���������
			#endif

			//wchar_t sItemName[32]; // ��� �������� � ��������� (reg/xml)
		};
		struct ConEmuSetupPages
		{
			int              PageID;    // Dialog ID
			wchar_t          PageName[64];
			//HWND     *hPage;
			TabHwndIndex     PageIndex; // mh_Tabs[...]
			HTREEITEM        hTI;
			ConEmuSetupItem* pItems;
		};
		ConEmuSetupPages *m_Pages;
};
