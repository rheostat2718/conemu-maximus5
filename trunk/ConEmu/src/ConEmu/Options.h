
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


#pragma once

#include "../Common/WinObjects.h"
#include <commctrl.h>

#define MIN_ALPHA_VALUE 40
#define MAX_FONT_STYLES 8 //normal/bold|italic|underline

#define CONEMU_ROOT_KEY L"Software\\ConEmu"

#include <pshpack1.h>
typedef struct tagMYRGB
{
	union
	{
		COLORREF color;
		struct
		{
			BYTE    rgbBlue;
			BYTE    rgbGreen;
			BYTE    rgbRed;
			BYTE    rgbReserved;
		};
	};
} MYRGB, MYCOLORREF;
#include <poppack.h>

class CBackground;

enum BackgroundOp
{
	eUpLeft = 0,
	eStretch = 1,
	eTile = 2,
};

class CSettings
{
	public:
		CSettings();
		~CSettings();

		wchar_t Type[16];
		private:
			wchar_t ConfigPath[MAX_PATH], ConfigName[240];
		public:
		void SetConfigName(LPCWSTR asConfigName);

		wchar_t szFontError[512];

		bool SingleInstanceArg;
		void ResetCmdArg();

		int DefaultBufferHeight;
		bool bForceBufferHeight; int nForceBufferHeight;
		bool AutoScroll; bool AutoBufferHeight;
		//bool FarSyncSize;
		int nCmdOutputCP;

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
		LOGFONT LogFont, LogFont2;
		LONG mn_AutoFontWidth, mn_AutoFontHeight; // ������� �������, ������� ���� ��������� ��� ����������� ������
		LONG mn_FontWidth, mn_FontHeight, mn_BorderFontWidth; // �������� ������� �������
		BYTE mn_LoadFontCharSet; // �� ��� ��������� ���������� (��� ��� ��������� � ������)
		TEXTMETRIC tm[MAX_FONT_STYLES];
		LPOUTLINETEXTMETRIC otm[MAX_FONT_STYLES];
		BOOL mb_Name1Ok, mb_Name2Ok;
		void ResetFontWidth();
		void SaveFontSizes(LOGFONT *pCreated, bool bAuto, bool bSendChanges);
		LPOUTLINETEXTMETRIC LoadOutline(HDC hDC, HFONT hFont);
		void DumpFontMetrics(LPCWSTR szType, HDC hDC, HFONT hFont, LPOUTLINETEXTMETRIC lpOutl = NULL);
	public:
		bool isFontAutoSize;
		bool isAutoRegisterFonts;
		//wchar_t FontFile[MAX_PATH];
		LOGFONT ConsoleFont;
		COLORREF* GetColors(BOOL abFade = FALSE);
		COLORREF GetFadeColor(COLORREF cr);
		bool isUserScreenTransparent;
		bool NeedDialogDetect();
		//COLORREF ColorKey;
		bool isExtendColors;
		char nExtendColor;
		bool isExtendFonts, isTrueColorer;
		char nFontNormalColor, nFontBoldColor, nFontItalicColor;

		/* Background image */
		WCHAR sBgImage[MAX_PATH];
		char isShowBgImage;
		bool isBackgroundImageValid;
		u8 bgImageDarker;
		DWORD nBgImageColors;
		char bgOperation; // BackgroundOp {eUpLeft = 0, eStretch = 1, eTile = 2}
		char isBgPluginAllowed;

		/* Transparency */
		u8 nTransparent;

		/* Command Line History (from start dialog) */
		LPWSTR psCmdHistory; DWORD nCmdHistorySize;

		/* Command Line (Registry) */
		LPTSTR psCmd;
		/* Command Line ("/cmd" arg) */
		LPTSTR psCurCmd;
	private:
		/* 'Default' command line (if nor Registry, nor /cmd specified) */
		WCHAR  szDefCmd[16];
	public:
		/* "Active" command line */
		LPCTSTR GetCmd();
		/* "Default" command line "far/cmd", based on /BufferHeight switch */
		LPCTSTR GetDefaultCmd();
		/* OUR(!) startup info */
		STARTUPINFOW ourSI;
		/* If Attach to PID requested */
		DWORD nAttachPID; HWND hAttachConWnd;

		DWORD FontSizeY;  // ������ ��������� ������ (����������� �� ��������!)
		DWORD FontSizeX;  // ������ ��������� ������
		DWORD FontSizeX2; // ������ ��� FixFarBorders (������ ������������ ������ ��� ��������� �����, �� ������ �� �����������)
		DWORD FontSizeX3; // ������ ���������� ��� ���������� ������ (�� ������ � FontSizeX2)
		bool isFullScreen, isHideCaption;
		bool isHideCaptionAlways();
		BYTE nHideCaptionAlwaysFrame;
		DWORD nHideCaptionAlwaysDelay, nHideCaptionAlwaysDisappear;
		bool isAlwaysOnTop, isDesktopMode;
		BYTE isFixFarBorders;
		bool isExtendUCharMap;
		bool isMouseSkipActivation, isMouseSkipMoving;
		bool isFarHourglass; DWORD nFarHourglassDelay;
		BYTE isDisableFarFlashing, isDisableAllFlashing;
		// Text selection
		BYTE isConsoleTextSelection;
		bool isCTSSelectBlock, isCTSSelectText;
		BYTE isCTSVkBlock, isCTSVkText; // ����������� ������� ��������� ������
		BYTE isCTSActMode, isCTSVkAct; // ����� � ����������� ���������� �������� ������ � ������� ������ �����
		BYTE isCTSRBtnAction, isCTSMBtnAction; // 0-off, 1-copy, 2-paste
		BYTE isCTSColorIndex;
		bool isModifierPressed(DWORD vk);
		//bool isSelectionModifierPressed();
	protected:
		bool mb_HideCaptionAlways;
		typedef struct tag_CharRanges
		{
			bool bUsed;
			wchar_t cBegin, cEnd;
		} CharRanges;
		wchar_t mszCharRanges[120];
		CharRanges icFixFarBorderRanges[10];
		bool *mpc_FixFarBorderValues;
		BYTE m_isKeyboardHooks;
	public:
		bool isKeyboardHooks();
		bool isCharBorder(wchar_t inChar);
		BYTE isPartBrush75, isPartBrush50, isPartBrush25, isPartBrushBlack;
		bool isCursorV;
		bool isCursorBlink;
		bool isCursorColor;
		bool isCursorBlockInactive;
		char isRClickSendKey;
		wchar_t *sRClickMacro;
		bool isSafeFarClose;
		wchar_t *sSafeFarCloseMacro;
		bool isSendAltEnter;
		BYTE isSendAltSpace;
		bool isMinToTray;
		bool isAlwaysShowTrayIcon;
		//bool isForceMonospace, isProportional;
		BYTE isMonospace, isMonospaceSelected; // 0 - proportional, 1 - monospace, 2 - forcemonospace
		//bool isUpdConHandle;
		bool isRSelFix;
		//Drag
		BYTE isDragEnabled;
		BYTE isDropEnabled;
		DWORD nLDragKey, nRDragKey;
		bool isDefCopy;
		BYTE isDragOverlay;
		bool isDragShowIcons;
		BYTE isDragPanel; // ��������� ������� ������� ������
		//bool
		bool isDebugSteps;
		bool isEnhanceGraphics; // Progressbars and scrollbars (pseudographics)
		bool isFadeInactive;
		//DWORD nFadeInactiveMask;
		char isTabs; bool isTabSelf, isTabRecent, isTabLazy, isTabsInCaption;
		wchar_t sTabFontFace[LF_FACESIZE]; DWORD nTabFontCharSet; int nTabFontHeight;
		wchar_t *sTabCloseMacro;
		wchar_t *sSaveAllMacro;
		int nToolbarAddSpace;
		DWORD wndWidth, wndHeight, ntvdmHeight; // � ��������
		int wndX, wndY; // � ��������
		bool wndCascade, isAutoSaveSizePos;
		DWORD nSlideShowElapse;
		DWORD nIconID;
		bool isTryToCenter;
		BYTE isAlwaysShowScrollbar;
		RECT rcTabMargins;
		bool isTabFrame;
		BYTE icMinimizeRestore;
		bool isMulti; BYTE icMultiNew, icMultiNext, icMultiRecreate, icMultiBuffer, icMultiClose, icMultiCmd;
		bool isMultiAutoCreate, isMultiLeaveOnClose, isMultiIterate;
		bool IsHostkey(WORD vk);
		bool IsHostkeySingle(WORD vk);
		bool IsHostkeyPressed();
		WORD GetPressedHostkey();
		UINT GetHostKeyMod(); // ����� ������ MOD_xxx ��� RegisterHotKey
		bool isMultiNewConfirm, isUseWinNumber;
		bool isFARuseASCIIsort, isFixAltOnAltTab, isShellNoZoneCheck;

		// ��������� �����
		WCHAR szTabConsole[32];
		WCHAR szTabEditor[32];
		WCHAR szTabEditorModified[32];
		WCHAR szTabViewer[32];
		DWORD nTabLenMax;

		//bool isVisualizer;
		//char nVizNormal, nVizFore, nVizTab, nVizEOL, nVizEOF;
		//wchar_t cVizTab, cVizEOL, cVizEOF;

		char isAllowDetach;
		bool isCreateAppWindow;
		/*bool isScrollTitle;
		DWORD ScrollTitleLen;*/
		wchar_t szAdminTitleSuffix[64]; //" (Admin)"
		bool bAdminShield, bHideInactiveConsoleTabs;

		DWORD nMainTimerElapse; // �������������, � ������� �� ������� ����������� �����
		//bool isAdvLangChange; // � ����� ��� ConIme � ����� ������� �� �������� ����, ���� �� ������� WM_SETFOCUS. �� ��� ���� �������� ������ �������� ������
		bool isSkipFocusEvents;
		//bool isLangChangeWsPlugin;
		char isMonitorConsoleLang;
		bool isSleepInBackground;

		DWORD nAffinity;

		// Debugging - "c:\\temp\\ConEmuVCon-%i-%i.dat"
		BYTE isAdvLogging;
		//wchar_t szDumpPackets[MAX_PATH];
		// Debugging
		bool isConVisible; // isLockRealConsolePos;
		DWORD nConInMode;
		
		//
		enum GuiLoggingType m_RealConLoggingType;

		// Thumbnails and Tiles
		PanelViewSetMapping ThSet;
		MFileMapping<PanelViewSetMapping> m_ThSetMap;

		// Working variables...
	private:
		//HBITMAP  hBgBitmap;
		//COORD    bgBmp;
		//HDC      hBgDc;
		CBackground* mp_Bg;
		//MSection mcs_BgImgData;
		BITMAPFILEHEADER* mp_BgImgData;
		BOOL mb_NeedBgUpdate, mb_WasVConBgImage;
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
		HFONT   mh_Font[MAX_FONT_STYLES], mh_Font2;
		TODO("�� ��������, CharWidth & CharABC ����� ��������� �� ������� - � Bold ������ ����� ���� ������");
		WORD    CharWidth[0x10000]; //, Font2Width[0x10000];
		ABC     CharABC[0x10000];

		HWND hMain, hExt, hTabs, hColors, hViews, hInfo, hDebug;

		//static void CenterDialog(HWND hWnd2);
		void OnClose();
		static INT_PTR CALLBACK wndOpProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		static INT_PTR CALLBACK mainOpProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		static INT_PTR CALLBACK extOpProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		static INT_PTR CALLBACK tabsOpProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		static INT_PTR CALLBACK colorOpProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		static INT_PTR CALLBACK viewsOpProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		static INT_PTR CALLBACK infoOpProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		static INT_PTR CALLBACK debugOpProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		static INT_PTR CALLBACK hideOpProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		//static INT_PTR CALLBACK multiOpProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		static INT_PTR CALLBACK selectionOpProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		void LoadSettings();
		void InitSettings();
		BOOL SaveSettings(BOOL abSilent = FALSE);
		void SaveSizePosOnExit();
		void SaveConsoleFont();
		bool ShowColorDialog(HWND HWndOwner, COLORREF *inColor);
		static int CALLBACK EnumFamCallBack(LPLOGFONT lplf, LPNEWTEXTMETRIC lpntm, DWORD FontType, LPVOID aFontCount);
		static int CALLBACK EnumFontCallBackEx(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, DWORD FontType, LPARAM lParam);
		void UpdateMargins(RECT arcMargins);
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
		BOOL GetFontNameFromFile(LPCTSTR lpszFilePath, wchar_t (&rsFontName)[32]);
		void HistoryCheck();
		void HistoryAdd(LPCWSTR asCmd);
		LPCWSTR HistoryGet();
		void UpdateConsoleMode(DWORD nMode);
		BOOL CheckConIme();
		void CheckConsoleSettings();
		SettingsBase* CreateSettings();
		bool AutoRecreateFont(int nFontW, int nFontH);
		bool AutoSizeFont(int nRelative/*0/1*/, int nValue/*1,2,...*/);
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
		LRESULT OnInitDialog();
		LRESULT OnInitDialog_Main();
		LRESULT OnInitDialog_Ext();
		LRESULT OnInitDialog_Tabs();
		LRESULT OnInitDialog_Color();
		LRESULT OnInitDialog_Views();
		LRESULT OnInitDialog_ViewsFonts();
		LRESULT OnInitDialog_Info();
		LRESULT OnInitDialog_Debug();
		LRESULT OnButtonClicked(WPARAM wParam, LPARAM lParam);
		LRESULT OnColorButtonClicked(WPARAM wParam, LPARAM lParam);
		LRESULT OnEditChanged(WPARAM wParam, LPARAM lParam);
		LRESULT OnColorEditChanged(WPARAM wParam, LPARAM lParam);
		LRESULT OnComboBox(WPARAM wParam, LPARAM lParam);
		LRESULT OnColorComboBox(WPARAM wParam, LPARAM lParam);
		LRESULT OnTab(LPNMHDR phdr);
		INT_PTR OnMeasureFontItem(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		INT_PTR OnDrawFontItem(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);
		void OnSaveActivityLogFile(HWND hListView);
	private:
		bool GetColorById(WORD nID, COLORREF* color);
		bool SetColorById(WORD nID, COLORREF color);
		void ColorSetEdit(HWND hWnd2, WORD c);
		bool ColorEditDialog(HWND hWnd2, WORD c);
		void FillBgImageColors();
		HBRUSH mh_CtlColorBrush;
		INT_PTR ColorCtlStatic(HWND hWnd2, WORD c, HWND hItem);
		COLORREF acrCustClr[16]; // array of custom colors
		BOOL mb_IgnoreEditChanged, mb_IgnoreTtfChange, mb_CharSetWasSet;
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
		HFONT CreateFontIndirectMy(LOGFONT *inFont);
		void RecreateFont(WORD wFromID);
		// Theming
		HMODULE mh_Uxtheme;
		typedef HRESULT(STDAPICALLTYPE *SetWindowThemeT)(HWND hwnd,LPCWSTR pszSubAppName,LPCWSTR pszSubIdList);
		SetWindowThemeT SetWindowThemeF;
		typedef HRESULT(STDAPICALLTYPE *EnableThemeDialogTextureT)(HWND hwnd,DWORD dwFlags);
		EnableThemeDialogTextureT EnableThemeDialogTextureF;
		UINT mn_MsgUpdateCounter;
		//wchar_t temp[MAX_PATH];
		UINT mn_MsgRecreateFont;
		UINT mn_MsgLoadFontFromMain;
		static int IsChecked(HWND hParent, WORD nCtrlId);
		static int GetNumber(HWND hParent, WORD nCtrlId);
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
			wchar_t szFontFile[MAX_PATH]; // ������ ����
			wchar_t szFontName[32];       // Font Family
			BOOL    bUnicode;             // ���������?
			BOOL    bHasBorders;          // ����� �� ������ ����� ������� �����
			BOOL    bAlreadyInSystem;     // ����� � ����� ������ ��� ��� ��������������� � �������
		} RegFont;
		std::vector<RegFont> m_RegFonts;
		BOOL mb_StopRegisterFonts;
		//
		COLORREF Colors[0x20];
		bool mb_FadeInitialized;
		BYTE mn_FadeLow, mn_FadeHigh;
		DWORD mn_FadeMul;
		COLORREF ColorsFade[0x20];
		BOOL GetColorRef(HWND hDlg, WORD TB, COLORREF* pCR);
		inline BYTE GetFadeColorItem(BYTE c);
		//
		bool mb_ThemingEnabled;
		//
		bool TestHostkeyModifiers();
		static BYTE CheckHostkeyModifier(BYTE vk);
		static void ReplaceHostkey(BYTE vk, BYTE vkNew);
		static void AddHostkey(BYTE vk);
		static void TrimHostkeys();
		static void SetupHotkeyChecks(HWND hWnd2);
		static bool MakeHostkeyModifier();
		static BYTE HostkeyCtrlId2Vk(WORD nID);
		DWORD nMultiHotkeyModifier;
		BYTE mn_HostModOk[15], mn_HostModSkip[15];
		bool isHostkeySingleLR(WORD vk, WORD vkC, WORD vkL, WORD vkR);
		static void FillListBoxItems(HWND hList, uint nItems, const WCHAR** pszItems, const DWORD* pnValues, DWORD& nValue);
		static void GetListBoxItem(HWND hList, uint nItems, const WCHAR** pszItems, const DWORD* pnValues, DWORD& nValue);
		static void CenterMoreDlg(HWND hWnd2);
		static bool IsAlmostMonospace(LPCWSTR asFaceName, int tmMaxCharWidth, int tmAveCharWidth, int tmHeight);
};
