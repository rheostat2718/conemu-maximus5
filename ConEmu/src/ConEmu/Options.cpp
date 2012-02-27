
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


// cbFARuseASCIIsort, cbFixAltOnAltTab

#define SHOWDEBUGSTR

#include "Header.h"
//#include "../common/ConEmuCheck.h"
#include "Options.h"
#include "ConEmu.h"
#include "VConChild.h"
#include "VirtualConsole.h"
#include "RealConsole.h"
#include "TabBar.h"
#include "Background.h"
#include "TrayIcon.h"
#include "LoadImg.h"
#include "../ConEmuPlugin/FarDefaultMacros.h"
#include "OptionsFast.h"
#include "version.h"

//#define DEBUGSTRFONT(s) DEBUGSTR(s)

//#define COUNTER_REFRESH 5000
//#define BACKGROUND_FILE_POLL 5000

//#define RASTER_FONTS_NAME L"Raster Fonts"
//SIZE szRasterSizes[100] = {{0,0}}; // {{16,8},{6,9},{8,9},{5,12},{7,12},{8,12},{16,12},{12,16},{10,18}};
//const wchar_t szRasterAutoError[] = L"Font auto size is not allowed for a fixed raster font size. Select 'Terminal' instead of '[Raster Fonts ...]'";

//#define TEST_FONT_WIDTH_STRING_EN L"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
//#define TEST_FONT_WIDTH_STRING_RU L"��������������������������������"

//#define FAILED_FONT_TIMERID 101
//#define FAILED_FONT_TIMEOUT 3000
//#define FAILED_CONFONT_TIMEOUT 30000

//const u8 chSetsNums[] = {0, 178, 186, 136, 1, 238, 134, 161, 177, 129, 130, 77, 255, 204, 128, 2, 222, 162, 163};
//const char *ChSets[] = {"ANSI", "Arabic", "Baltic", "Chinese Big 5", "Default", "East Europe",
//                        "GB 2312", "Greek", "Hebrew", "Hangul", "Johab", "Mac", "OEM", "Russian", "Shiftjis",
//                        "Symbol", "Thai", "Turkish", "Vietnamese"
//                       };
//const WORD HostkeyCtrlIds[] = {cbHostWin, cbHostApps, cbHostLCtrl, cbHostRCtrl, cbHostLAlt, cbHostRAlt, cbHostLShift, cbHostRShift};
//int upToFontHeight=0;
//HWND ghOpWnd=NULL;

#ifdef _DEBUG
#define HEAPVAL //HeapValidate(GetProcessHeap(), 0, NULL);
#else
#define HEAPVAL
#endif

#define DEFAULT_FADE_LOW 0
#define DEFAULT_FADE_HIGH 0xD0


//struct CONEMUDEFCOLORS
//{
//	const wchar_t* pszTitle;
//	DWORD dwDefColors[0x10];
//};
//
//const CONEMUDEFCOLORS DefColors[] =
//{
//	{
//		L"Default color scheme (Windows standard)", {
//			0x00000000, 0x00800000, 0x00008000, 0x00808000, 0x00000080, 0x00800080, 0x00008080, 0x00c0c0c0,
//			0x00808080, 0x00ff0000, 0x0000ff00, 0x00ffff00, 0x000000ff, 0x00ff00ff, 0x0000ffff, 0x00ffffff
//		}
//	},
//	{
//		L"Gamma 1 (for use with dark monitors)", {
//			0x00000000, 0x00960000, 0x0000aa00, 0x00aaaa00, 0x000000aa, 0x00800080, 0x0000aaaa, 0x00c0c0c0,
//			0x00808080, 0x00ff0000, 0x0000ff00, 0x00ffff00, 0x000000ff, 0x00ff00ff, 0x0000ffff, 0x00ffffff
//		}
//	},
//	{
//		L"Murena scheme", {
//			0x00000000, 0x00644100, 0x00008000, 0x00808000, 0x00000080, 0x00800080, 0x00008080, 0x00c0c0c0,
//			0x00808080, 0x00ff0000, 0x0076c587, 0x00ffff00, 0x00004bff, 0x00d78ce6, 0x0000ffff, 0x00ffffff
//		}
//	},
//	{
//		L"tc-maxx", {
//			0x00000000, RGB(11,27,59), RGB(0,128,0), RGB(0,90,135), RGB(106,7,28), RGB(128,0,128), RGB(128,128,0), RGB(40,150,177),
//			RGB(128,128,128), RGB(0,0,255), RGB(0,255,0), RGB(0,215,243), RGB(190,7,23), RGB(255,0,255), RGB(255,255,0), RGB(255,255,255)
//		}
//	},
//
//};
extern const DWORD *dwDefColors; // = DefColors->dwDefColors;
//DWORD gdwLastColors[0x10] = {0};
//BOOL  gbLastColorsOk = FALSE;

//#define MAX_COLOR_EDT_ID c31




#define SetThumbColor(s,rgb,idx,us) { (s).RawColor = 0; (s).ColorRGB = rgb; (s).ColorIdx = idx; (s).UseIndex = us; }
#define SetThumbSize(s,sz,x1,y1,x2,y2,ls,lp,fn,fs) { \
		(s).nImgSize = sz; (s).nSpaceX1 = x1; (s).nSpaceY1 = y1; (s).nSpaceX2 = x2; (s).nSpaceY2 = y2; \
		(s).nLabelSpacing = ls; (s).nLabelPadding = lp; wcscpy_c((s).sFontName,fn); (s).nFontHeight=fs; }
#define ThumbLoadSet(s,n) { \
		reg->Load(L"PanView." s L".ImgSize", n.nImgSize); \
		reg->Load(L"PanView." s L".SpaceX1", n.nSpaceX1); \
		reg->Load(L"PanView." s L".SpaceY1", n.nSpaceY1); \
		reg->Load(L"PanView." s L".SpaceX2", n.nSpaceX2); \
		reg->Load(L"PanView." s L".SpaceY2", n.nSpaceY2); \
		reg->Load(L"PanView." s L".LabelSpacing", n.nLabelSpacing); \
		reg->Load(L"PanView." s L".LabelPadding", n.nLabelPadding); \
		reg->Load(L"PanView." s L".FontName", n.sFontName, countof(n.sFontName)); \
		reg->Load(L"PanView." s L".FontHeight", n.nFontHeight); }
#define ThumbSaveSet(s,n) { \
		reg->Save(L"PanView." s L".ImgSize", n.nImgSize); \
		reg->Save(L"PanView." s L".SpaceX1", n.nSpaceX1); \
		reg->Save(L"PanView." s L".SpaceY1", n.nSpaceY1); \
		reg->Save(L"PanView." s L".SpaceX2", n.nSpaceX2); \
		reg->Save(L"PanView." s L".SpaceY2", n.nSpaceY2); \
		reg->Save(L"PanView." s L".LabelSpacing", n.nLabelSpacing); \
		reg->Save(L"PanView." s L".LabelPadding", n.nLabelPadding); \
		reg->Save(L"PanView." s L".FontName", n.sFontName); \
		reg->Save(L"PanView." s L".FontHeight", n.nFontHeight); }


Settings::Settings()
{
	gpSet = this; // �����!
	
	// ����� ���������� (struct, ���������)
	memset(this, 0, sizeof(*this));

	// -- ��� memset
	//// ��������� ���� ����� ������ ��� InitSettings, �.�. ��� ����� ����
	//// ������� ����� �� ���������� ������� ��������
	//Type[0] = 0;
	//psCmd = psCurCmd = NULL;
	////wcscpy_c(szDefCmd, L"far");
	//psCmdHistory = NULL; nCmdHistorySize = 0;

	// ��������� ��������������� � CSettings::CSettings
	//-- // ������ ��������� ��������� ��������	
	//-- InitSettings();
}

void Settings::ReleasePointers()
{
	if (sTabCloseMacro) {free(sTabCloseMacro); sTabCloseMacro = NULL;}
	
	if (sSafeFarCloseMacro) {free(sSafeFarCloseMacro); sSafeFarCloseMacro = NULL;}

	if (sSaveAllMacro) {free(sSaveAllMacro); sSaveAllMacro = NULL;}

	if (sRClickMacro) {free(sRClickMacro); sRClickMacro = NULL;}
	
	if (psCmd) {free(psCmd); psCmd = NULL;}
	if (psCurCmd) {free(psCurCmd); psCurCmd = NULL;}
	if (psCmdHistory) {free(psCmdHistory); psCmdHistory = NULL;}
	
	UpdSet.FreePointers();
}

Settings::~Settings()
{
	ReleasePointers();
}

void Settings::InitSettings()
{
	MCHKHEAP

	// ���������� ������, �.�. ������� ����� ���� ������� �� ���� ���������� ��������	
	ReleasePointers();
	
	// ����� ���������� (struct, ���������)
	memset(this, 0, sizeof(*this));
	
//------------------------------------------------------------------------
///| Moved from CVirtualConsole |/////////////////////////////////////////
//------------------------------------------------------------------------
	isAutoRegisterFonts = true;
	isMulti = true; icMultiNew = 'W'; icMultiNext = 'Q'; icMultiRecreate = 192/*VK_������*/; icMultiBuffer = 'A';
	icMinimizeRestore = 'C';
	icMultiClose = 0/*VK_DELETE*/; icMultiCmd = 'X'; isMultiAutoCreate = false; isMultiLeaveOnClose = false; isMultiIterate = true;
	isMultiNewConfirm = true; isUseWinNumber = true; isUseWinArrows = false; isUseWinTab = false; nMultiHotkeyModifier = VK_LWIN; TestHostkeyModifiers();
	m_isKeyboardHooks = 0;
	isFARuseASCIIsort = false; isFixAltOnAltTab = false; isShellNoZoneCheck = false;
	isFadeInactive = true; mn_FadeLow = DEFAULT_FADE_LOW; mn_FadeHigh = DEFAULT_FADE_HIGH; mb_FadeInitialized = false;
	mn_LastFadeSrc = mn_LastFadeDst = -1;
	nMainTimerElapse = 10;
	nMainTimerInactiveElapse = 1000;
	nAffinity = 0; // 0 - don't change default affinity
	isSkipFocusEvents = false;
	isSendAltEnter = isSendAltSpace = isSendAltTab = isSendAltEsc = isSendAltPrintScrn = isSendPrintScrn = isSendCtrlEsc = isSendAltF9 = false;
	isMonitorConsoleLang = 3;
	DefaultBufferHeight = 1000; AutoBufferHeight = true;
	nCmdOutputCP = 0;

	//WARNING("InitSettings() ����� ���������� �� ���������� ���������, �� ������������ � ��������");
	//// ������
	//memset(m_Fonts, 0, sizeof(m_Fonts));
	////TODO: OLD - �� ���������
	//memset(&LogFont, 0, sizeof(LogFont));
	//memset(&LogFont2, 0, sizeof(LogFont2));
	/*LogFont.lfHeight = mn_FontHeight =*/ FontSizeY = 16;
	//LogFont.lfWidth = mn_FontWidth = FontSizeX = mn_BorderFontWidth = 0;
	//FontSizeX2 = 0; FontSizeX3 = 0;
	//LogFont.lfEscapement = LogFont.lfOrientation = 0;
	/*LogFont.lfWeight = FW_NORMAL;*/ isBold = false;
	/*LogFont.lfItalic = LogFont.lfUnderline = LogFont.lfStrikeOut = FALSE;*/ isItalic = false;
	/*LogFont.lfCharSet*/ mn_LoadFontCharSet = DEFAULT_CHARSET;
	//LogFont.lfOutPrecision = OUT_TT_PRECIS;
	//LogFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	/*LogFont.lfQuality =*/ mn_AntiAlias = ANTIALIASED_QUALITY;
	//LogFont.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
	inFont[0] = inFont2[0] = 0;
	//wcscpy_c(inFont, L"Lucida Console");
	//wcscpy_c(inFont2, L"Lucida Console");
	//mb_Name1Ok = FALSE; mb_Name2Ok = FALSE;
	isTryToCenter = false;
	isAlwaysShowScrollbar = 2;
	isTabFrame = true;
	//isForceMonospace = false; isProportional = false;
	isMonospace = 1;
	isMinToTray = false; isAlwaysShowTrayIcon = false;
	memset(&rcTabMargins, 0, sizeof(rcTabMargins));
	//isFontAutoSize = false; mn_AutoFontWidth = mn_AutoFontHeight = -1;
	
	ConsoleFont.lfHeight = 5;
	ConsoleFont.lfWidth = 3;
	wcscpy_c(ConsoleFont.lfFaceName, L"Lucida Console");
	
	{
		SettingsRegistry RegConColors, RegConDef;

		if (RegConColors.OpenKey(L"Console", KEY_READ))
		{
			RegConDef.OpenKey(HKEY_USERS, L".DEFAULT\\Console", KEY_READ);
			TCHAR ColorName[] = L"ColorTable00";
			bool  lbBlackFound = false;

			for(uint i = 0x10; i--;)
			{
				// L"ColorTableNN"
				ColorName[10] = i/10 + '0';
				ColorName[11] = i%10 + '0';

				if (!RegConColors.Load(ColorName, (LPBYTE)&(Colors[i]), sizeof(Colors[0])))
					if (!RegConDef.Load(ColorName, (LPBYTE)&(Colors[i]), sizeof(Colors[0])))
						Colors[i] = dwDefColors[i]; //-V108

				if (Colors[i] == 0)
				{
					if (!lbBlackFound)
						lbBlackFound = true;
					else if (lbBlackFound)
						Colors[i] = dwDefColors[i]; //-V108
				}

				Colors[i+0x10] = Colors[i]; // ���������
			}

			RegConDef.CloseKey();
			RegConColors.CloseKey();
		}
	}
	isTrueColorer = true; // ������� �� ��������, ��� Far3
	isExtendColors = false;
	nExtendColor = 14;
	isExtendFonts = false;
#ifdef _DEBUG
	isExtendFonts = true;
#endif
	nFontNormalColor = 1; nFontBoldColor = 12; nFontItalicColor = 13;
	//CheckTheming(); -- ������ - ������. ����� ���������, ���� ������� ���� ����� �������
	//mb_ThemingEnabled = (gOSVer.dwMajorVersion >= 6 || (gOSVer.dwMajorVersion == 5 && gOSVer.dwMinorVersion >= 1));
//------------------------------------------------------------------------
///| Default settings |///////////////////////////////////////////////////
//------------------------------------------------------------------------
	isShowBgImage = 0;
	wcscpy_c(sBgImage, L"c:\\back.bmp");
	bgImageDarker = 255; // 0x46;
	//nBgImageColors = 1|2; �����,�������
	//nBgImageColors = 2; // ������ �����
	nBgImageColors = (DWORD)-1; // �������� ���� ������� �� ���� - ����� "1|2" == BgImageColorsDefaults.
	bgOperation = eUpLeft;
	isBgPluginAllowed = 1;
	nTransparent = 255;
	//isColorKey = false;
	//ColorKey = RGB(1,1,1);
	isUserScreenTransparent = false;
	isFixFarBorders = 1; isEnhanceGraphics = true; isEnhanceButtons = false;
	isPartBrush75 = 0xC8; isPartBrush50 = 0x96; isPartBrush25 = 0x5A;
	isPartBrushBlack = 32; //-V112
	isExtendUCharMap = true;
	isDownShowHiddenMessage = false;
	ParseCharRanges(L"2013-25C4", mpc_FixFarBorderValues);
	wndHeight = 25;
	ntvdmHeight = 0; // ��������� �������������
	wndWidth = 80;
	WindowMode = rNormal;
	//isFullScreen = false;
	isHideCaption = mb_HideCaptionAlways = false;
	nHideCaptionAlwaysFrame = 1; nHideCaptionAlwaysDelay = 2000; nHideCaptionAlwaysDisappear = 2000;
	isDesktopMode = false;
	isAlwaysOnTop = false;
	isSleepInBackground = false; // �� ��������� - �� �������� "��������� � ����".
	wndX = 0; wndY = 0; wndCascade = true;
	isAutoSaveSizePos = false; mb_SizePosAutoSaved = false;
	isConVisible = false; //isLockRealConsolePos = false;
	isUseInjects = true;
	#ifdef USEPORTABLEREGISTRY
	isPortableReg = true; // �������� �� ���������, DEBUG
	#else
	isPortableReg = false;
	#endif
	nConInMode = (DWORD)-1; // �� ���������, ��������� (ENABLE_QUICK_EDIT_MODE|ENABLE_EXTENDED_FLAGS|ENABLE_INSERT_MODE)
	nSlideShowElapse = 2500;
	nIconID = IDI_ICON1;
	isRClickSendKey = 2;
	sRClickMacro = NULL;
	wcscpy_c(szTabConsole, L"<%c> %s `%p`" /*L "%s [%c]" */);
	wcscpy_c(szTabEditor, L"<%c.%i>{%s} `%p`" /* L"[%s]" */);
	wcscpy_c(szTabEditorModified, L"<%c.%i>[%s] `%p` *" /* L"[%s] *" */);
	wcscpy_c(szTabViewer, L"<%c.%i>[%s] `%p`" /* L"{%s}" */);
	nTabLenMax = 20;
	isSafeFarClose = true;
	sSafeFarCloseMacro = NULL; // ���� NULL - �� ������������ ������ �� ���������
	isCursorV = true;
	isCursorBlink = true;
	isCursorColor = false;
	isCursorBlockInactive = true;
	isConsoleTextSelection = 1; // Always
	isCTSSelectBlock = true; isCTSVkBlock = VK_LMENU; // �� ��������� - ���� ���������� c LAlt
	isCTSSelectText = true; isCTSVkText = VK_LSHIFT; // � ����� - ��� ������� LShift
	isCTSVkBlockStart = 0; isCTSVkTextStart = 0; // ��� �������, ������������ ����� ��������� hotkey ������� ���������
	isCTSActMode = 2; // BufferOnly
	isCTSVkAct = 0; // �.�. �� ��������� - ������ BufferOnly, �� ������ ��� �������������
	isCTSRBtnAction = 3; // Auto (��������� ��� - Paste, ���� - Copy)
	isCTSMBtnAction = 0; // <None>
	isCTSColorIndex = 0xE0;
	isFarGotoEditor = true; isFarGotoEditorVk = VK_LCONTROL;
	isTabs = 1; isTabSelf = true; isTabRecent = true; isTabLazy = true;
	ilDragHeight = 10;
	m_isTabsOnTaskBar = 2;
	isTabsInCaption = false; //cbTabsInCaption
	wcscpy_c(sTabFontFace, L"Tahoma"); nTabFontCharSet = ANSI_CHARSET; nTabFontHeight = 16;
	sTabCloseMacro = sSaveAllMacro = NULL;
	nToolbarAddSpace = 0;
	wcscpy_c(szAdminTitleSuffix, L" (Admin)");
	bAdminShield = true;
	bHideInactiveConsoleTabs = false;
	bHideDisabledTabs = false;
	bShowFarWindows = true;
	isDisableMouse = false;
	isRSelFix = true; isMouseSkipActivation = true; isMouseSkipMoving = true;
	isFarHourglass = true; nFarHourglassDelay = 500;
	isDisableFarFlashing = false; isDisableAllFlashing = false;
	isDragEnabled = DRAG_L_ALLOWED; isDropEnabled = (BYTE)1; isDefCopy = true;
	nLDragKey = 0; nRDragKey = VK_LCONTROL;
	isDragOverlay = 1; isDragShowIcons = true;
	// ��������� ������� ������� ������
	isDragPanel = 2; // �� ��������� ������� ����� ��������� ��������� (����� � ����� �� Ctrl-Left/Right/Up/Down ������� �����... ��� �� ������� �� ����������)
	isDragPanelBothEdges = false; // ������� �� ��� ����� (������-����� ������ � �����-������ ������)
	isDebugSteps = true;
	MCHKHEAP
	// Thumbnails
	memset(&ThSet, 0, sizeof(ThSet));
	ThSet.cbSize = sizeof(ThSet);
	// ��� ���������: RGB ��� Index
	SetThumbColor(ThSet.crBackground, RGB(255,255,255), 16, TRUE); // �������� �� ��������� "Auto"
	// ����� ����� ������ ���������
	ThSet.nPreviewFrame = 1;
	SetThumbColor(ThSet.crPreviewFrame, RGB(128,128,128), 8, TRUE);
	// ����� ������ �������� ��������
	ThSet.nSelectFrame = 1;
	SetThumbColor(ThSet.crSelectFrame, RGB(192,192,192), 7, FALSE);
	/* ������ ������������� ������� */
	// ������� ��� preview
	SetThumbSize(ThSet.Thumbs,96,1,1,5,20,2,0,L"Tahoma",14);
	// ������� ��� tiles
	SetThumbSize(ThSet.Tiles,48,4,4,172,4,4,1,L"Tahoma",14); //-V112
	// ������ ��������� ��������
	ThSet.bLoadPreviews = 3;   // bitmask of {1=Thumbs, 2=Tiles}
	ThSet.bLoadFolders = true; // true - load infolder previews (only for Thumbs)
	ThSet.nLoadTimeout = 15;   // 15 sec
	ThSet.nMaxZoom = 600; // %%
	ThSet.bUsePicView2 = true;
	ThSet.bRestoreOnStartup = false;
	//// ���� �� ������������
	//DWORD nCacheFolderType; // ����/���������/temp/� �.�.
	//wchar_t sCacheFolder[MAX_PATH];

	/* *** AutoUpdate *** */
	_ASSERTE(UpdSet.szUpdateVerLocation==NULL); // ��� ������ ��� ���� ������ ReleasePointers
	UpdSet.ResetToDefaults();
}

void Settings::LoadSettings()
{
	MCHKHEAP
	mb_CharSetWasSet = FALSE;

	// -- ���������� � CheckOptionsFast()
	//;; Q. � Windows Vista �������� ������ ���������� ��������.
	//	;; A. "�������" ������� ConIme.exe. ����� �� �� ������ ��� ����� ����������
	//	;;    (����� � �.�.). ����� �� �����, ���� ���� ������ ���� � ����������� ����?
	//	;;    ����� ��������� ��� ���������� ��� ������ ������������� ���� ����, ��������
	//	;;    � 'ConIme.ex1' (������ ��� �������� ������ � ���������� ������).
	//	;;    ��������� ����������: ������� � ������ � ���������������
	//if (gOSVer.dwMajorVersion == 6 && gOSVer.dwMinorVersion == 0)
	//{
	//	CheckConIme();
	//}

//------------------------------------------------------------------------
///| Loading from registry |//////////////////////////////////////////////
//------------------------------------------------------------------------
	SettingsBase* reg = CreateSettings();
	wcscpy_c(Type, reg->Type);

	BOOL lbOpened = FALSE, lbNeedCreateVanilla = FALSE;

	lbOpened = reg->OpenKey(gpSetCls->GetConfigPath(), KEY_READ);
	// ��������� ������� ����� �������� (��������� ��� ����� ������� - ������ � ����� ����� Software\ConEmu)
	if (!lbOpened && (*gpSetCls->GetConfigName() == 0))
	{
		lbOpened = reg->OpenKey(CONEMU_ROOT_KEY, KEY_READ);
		lbNeedCreateVanilla = lbOpened;
	}

	if (lbOpened)
	{
		TCHAR ColorName[] = L"ColorTable00";

		for(uint i = countof(Colors)/*0x20*/; i--;)
		{
			ColorName[10] = i/10 + '0';
			ColorName[11] = i%10 + '0';
			reg->Load(ColorName, Colors[i]);
		}

		reg->Load(L"ExtendColors", isExtendColors);
		reg->Load(L"ExtendColorIdx", nExtendColor);

		if (nExtendColor<0 || nExtendColor>15) nExtendColor=14;

		reg->Load(L"TrueColorerSupport", isTrueColorer);
		reg->Load(L"FadeInactive", isFadeInactive);
		reg->Load(L"FadeInactiveLow", mn_FadeLow);
		reg->Load(L"FadeInactiveHigh", mn_FadeHigh);

		if (mn_FadeHigh <= mn_FadeLow) { mn_FadeLow = DEFAULT_FADE_LOW; mn_FadeHigh = DEFAULT_FADE_HIGH; }
		mn_LastFadeSrc = mn_LastFadeDst = -1;

		reg->Load(L"ExtendFonts", isExtendFonts);
		reg->Load(L"ExtendFontNormalIdx", nFontNormalColor);

		if (nFontNormalColor<0 || nFontNormalColor>15) nFontNormalColor=1;

		reg->Load(L"ExtendFontBoldIdx", nFontBoldColor);

		if (nFontBoldColor<0 || nFontBoldColor>15) nFontBoldColor=12;

		reg->Load(L"ExtendFontItalicIdx", nFontItalicColor);

		if (nFontItalicColor<0 || nFontItalicColor>15) nFontItalicColor=13;

		// Debugging
		reg->Load(L"ConVisible", isConVisible);
		reg->Load(L"ConInMode", nConInMode);
		
		
		reg->Load(L"UseInjects", isUseInjects);
		#ifdef USEPORTABLEREGISTRY
		reg->Load(L"PortableReg", isPortableReg);
		#endif
		
		// Don't move invisible real console. This affects GUI eMenu.
		//reg->Load(L"LockRealConsolePos", isLockRealConsolePos);
		//reg->Load(L"DumpPackets", szDumpPackets);
		reg->Load(L"AutoRegisterFonts", isAutoRegisterFonts);

		reg->Load(L"FontName", inFont, countof(inFont));

		reg->Load(L"FontName2", inFont2, countof(inFont2));

		if (!*inFont || !*inFont2)
			isAutoRegisterFonts = true;

		reg->Load(L"FontBold", isBold);
		reg->Load(L"FontItalic", isItalic);

		if (!reg->Load(L"Monospace", isMonospace))
		{
			// Compatibility. ��������� ��������, ��������� �� "������" ������ ����������
			bool bForceMonospace = false, bProportional = false;
			reg->Load(L"ForceMonospace", bForceMonospace);
			reg->Load(L"Proportional", bProportional);
			isMonospace = bForceMonospace ? 2 : bProportional ? 0 : 1;
		}
		if (isMonospace > 2) isMonospace = 2;

		//isMonospaceSelected = isMonospace ? isMonospace : 1; // ���������, ����� �������� �� ��� ����� ��� ����� ������
			
			
			
		reg->Load(L"CmdLine", &psCmd);
		reg->Load(L"CmdLineHistory", &psCmdHistory); nCmdHistorySize = 0; HistoryCheck();
		reg->Load(L"Multi", isMulti);
		reg->Load(L"Multi.Modifier", nMultiHotkeyModifier); TestHostkeyModifiers();
		reg->Load(L"Multi.NewConsole", icMultiNew);
		reg->Load(L"Multi.Next", icMultiNext);
		reg->Load(L"Multi.Recreate", icMultiRecreate);
		reg->Load(L"Multi.Close", icMultiClose);
		reg->Load(L"Multi.CmdKey", icMultiCmd);
		reg->Load(L"Multi.NewConfirm", isMultiNewConfirm);
		reg->Load(L"Multi.Buffer", icMultiBuffer);
		reg->Load(L"Multi.UseArrows", isUseWinArrows);
		reg->Load(L"Multi.UseNumbers", isUseWinNumber);
		reg->Load(L"Multi.UseWinTab", isUseWinTab);
		reg->Load(L"Multi.AutoCreate", isMultiAutoCreate);
		reg->Load(L"Multi.LeaveOnClose", isMultiLeaveOnClose);
		reg->Load(L"Multi.Iterate", isMultiIterate);
		reg->Load(L"MinimizeRestore", icMinimizeRestore);

		reg->Load(L"KeyboardHooks", m_isKeyboardHooks); if (m_isKeyboardHooks>2) m_isKeyboardHooks = 0;

		reg->Load(L"FontAutoSize", isFontAutoSize);
		reg->Load(L"FontSize", FontSizeY);
		reg->Load(L"FontSizeX", FontSizeX);
		reg->Load(L"FontSizeX3", FontSizeX3);
		reg->Load(L"FontSizeX2", FontSizeX2);
		reg->Load(L"FontCharSet", mn_LoadFontCharSet); mb_CharSetWasSet = FALSE;
		reg->Load(L"Anti-aliasing", mn_AntiAlias);
		
		reg->Load(L"ConsoleFontName", ConsoleFont.lfFaceName, countof(ConsoleFont.lfFaceName));
		reg->Load(L"ConsoleFontWidth", ConsoleFont.lfWidth);
		reg->Load(L"ConsoleFontHeight", ConsoleFont.lfHeight);
		
		reg->Load(L"WindowMode", WindowMode); if (WindowMode!=rFullScreen && WindowMode!=rMaximized && WindowMode!=rNormal) WindowMode = rNormal;
		reg->Load(L"HideCaption", isHideCaption);
		// ������ ������ � mb_HideCaptionAlways, �.�. ������������ ������� ���� � ���������, ������� ������� ���� ����� isHideCaptionAlways()
		reg->Load(L"HideCaptionAlways", mb_HideCaptionAlways);
		reg->Load(L"HideCaptionAlwaysFrame", nHideCaptionAlwaysFrame);

		if (nHideCaptionAlwaysFrame > 10) nHideCaptionAlwaysFrame = 10;

		reg->Load(L"HideCaptionAlwaysDelay", nHideCaptionAlwaysDelay);

		if (nHideCaptionAlwaysDelay > 30000) nHideCaptionAlwaysDelay = 30000;

		reg->Load(L"HideCaptionAlwaysDisappear", nHideCaptionAlwaysDisappear);

		if (nHideCaptionAlwaysDisappear > 30000) nHideCaptionAlwaysDisappear = 30000;

		reg->Load(L"DownShowHiddenMessage", isDownShowHiddenMessage);

		reg->Load(L"ConWnd X", wndX); /*if (wndX<-10) wndX = 0;*/
		reg->Load(L"ConWnd Y", wndY); /*if (wndY<-10) wndY = 0;*/
		reg->Load(L"AutoSaveSizePos", isAutoSaveSizePos);
		// ��� �� ������ �� szDefCmd. ������ ������ �������� ������ "/BufferHeight N"
		// ����� ������� (�������������) ������� ������� �� "cmd" ��� "far"
		reg->Load(L"Cascaded", wndCascade);
		reg->Load(L"ConWnd Width", wndWidth); if (!wndWidth) wndWidth = 80; else if (wndWidth>1000) wndWidth = 1000;
		reg->Load(L"ConWnd Height", wndHeight); if (!wndHeight) wndHeight = 25; else if (wndHeight>500) wndHeight = 500;

		//TODO: ��� ��� ��������� �� �����������
		reg->Load(L"16bit Height", ntvdmHeight);

		if (ntvdmHeight!=0 && ntvdmHeight!=25 && ntvdmHeight!=28 && ntvdmHeight!=43 && ntvdmHeight!=50) ntvdmHeight = 25;

		reg->Load(L"DefaultBufferHeight", DefaultBufferHeight);

		if (DefaultBufferHeight < LONGOUTPUTHEIGHT_MIN)
			DefaultBufferHeight = LONGOUTPUTHEIGHT_MIN;
		else if (DefaultBufferHeight > LONGOUTPUTHEIGHT_MAX)
			DefaultBufferHeight = LONGOUTPUTHEIGHT_MAX;

		reg->Load(L"AutoBufferHeight", AutoBufferHeight);
		//reg->Load(L"FarSyncSize", FarSyncSize);
		reg->Load(L"CmdOutputCP", nCmdOutputCP);
		
		reg->Load(L"CursorType", isCursorV);
		reg->Load(L"CursorColor", isCursorColor);
		reg->Load(L"CursorBlink", isCursorBlink);
		reg->Load(L"CursorBlockInactive", isCursorBlockInactive);

		reg->Load(L"ConsoleTextSelection", isConsoleTextSelection); if (isConsoleTextSelection>2) isConsoleTextSelection = 2;

		reg->Load(L"CTS.SelectBlock", isCTSSelectBlock);
		reg->Load(L"CTS.VkBlock", isCTSVkBlock);
		reg->Load(L"CTS.VkBlockStart", isCTSVkBlockStart);
		reg->Load(L"CTS.SelectText", isCTSSelectText);
		reg->Load(L"CTS.VkText", isCTSVkText);
		reg->Load(L"CTS.VkTextStart", isCTSVkTextStart);

		reg->Load(L"CTS.ActMode", isCTSActMode); if (!isCTSActMode || isCTSActMode>2) isCTSActMode = 2;

		reg->Load(L"CTS.VkAct", isCTSVkAct);

		reg->Load(L"CTS.RBtnAction", isCTSRBtnAction); if (isCTSRBtnAction>3) isCTSRBtnAction = 0;

		reg->Load(L"CTS.MBtnAction", isCTSMBtnAction); if (isCTSMBtnAction>3) isCTSMBtnAction = 0;

		reg->Load(L"CTS.ColorIndex", isCTSColorIndex); if ((isCTSColorIndex & 0xF) == ((isCTSColorIndex & 0xF0)>>4)) isCTSColorIndex = 0xE0;
		
		reg->Load(L"FarGotoEditor", isFarGotoEditor);
		reg->Load(L"FarGotoEditorVk", isFarGotoEditorVk);

		if (!reg->Load(L"FixFarBorders", isFixFarBorders))
			reg->Load(L"Experimental", isFixFarBorders); //����� ������ ��� ���������

		//mszCharRanges[0] = 0;

		TODO("��� ������ ����� ����� ���������, ��������� ������������ ���������� ����� �������");
		// max 100 ranges x 10 chars + a little ;)		
		wchar_t szCharRanges[1024] = {};		
		if (!reg->Load(L"FixFarBordersRanges", szCharRanges, countof(szCharRanges)))		
		{
			wcscpy_c(szCharRanges, L"2013-25C4"); // default
		}
		//{		
		//	int n = 0, nMax = countof(icFixFarBorderRanges);		
		//	wchar_t *pszRange = mszCharRanges, *pszNext = NULL;		
		//	wchar_t cBegin, cEnd;		
		//		
		//	while(*pszRange && n < nMax)		
		//	{		
		//		cBegin = (wchar_t)wcstol(pszRange, &pszNext, 16);		
		//		
		//		if (!cBegin || cBegin == 0xFFFF || *pszNext != L'-') break;		
		//		
		//		pszRange = pszNext + 1;		
		//		cEnd = (wchar_t)wcstol(pszRange, &pszNext, 16);		
		//		
		//		if (!cEnd || cEnd == 0xFFFF) break;		
		//		
		//		icFixFarBorderRanges[n].bUsed = true;		
		//		icFixFarBorderRanges[n].cBegin = cBegin;		
		//		icFixFarBorderRanges[n].cEnd = cEnd;		
		//		n ++;		
		//		
		//		if (*pszNext != L';') break;		
		//		
		//		pszRange = pszNext + 1;		
		//	}		
		//		
		//	for(; n < nMax; n++)		
		//		icFixFarBorderRanges[n].bUsed = false;		
		//}		
		ParseCharRanges(szCharRanges, mpc_FixFarBorderValues);

		reg->Load(L"ExtendUCharMap", isExtendUCharMap);
		reg->Load(L"PartBrush75", isPartBrush75); if (isPartBrush75<5) isPartBrush75=5; else if (isPartBrush75>250) isPartBrush75=250;
		reg->Load(L"PartBrush50", isPartBrush50); if (isPartBrush50<5) isPartBrush50=5; else if (isPartBrush50>250) isPartBrush50=250;
		reg->Load(L"PartBrush25", isPartBrush25); if (isPartBrush25<5) isPartBrush25=5; else if (isPartBrush25>250) isPartBrush25=250;
		reg->Load(L"PartBrushBlack", isPartBrushBlack);

		if (isPartBrush50>=isPartBrush75) isPartBrush50=isPartBrush75-10;

		if (isPartBrush25>=isPartBrush50) isPartBrush25=isPartBrush50-10;

		// ������� � ��������� ���������
		reg->Load(L"EnhanceGraphics", isEnhanceGraphics);
		
		reg->Load(L"EnhanceButtons", isEnhanceButtons);

		if (isFixFarBorders == 2 && !isEnhanceGraphics)
		{
			isFixFarBorders = 1;
			isEnhanceGraphics = true;
		}

		reg->Load(L"RightClick opens context menu", isRClickSendKey);
		if (!reg->Load(L"RightClickMacro2", &sRClickMacro) || (sRClickMacro && !*sRClickMacro)) { SafeFree(sRClickMacro); }
		
		reg->Load(L"AltEnter", isSendAltEnter);
		reg->Load(L"AltSpace", isSendAltSpace); //if (isSendAltSpace > 2) isSendAltSpace = 2; // �����-�� ��� 3state, ������ - bool
		reg->Load(L"SendAltTab", isSendAltTab);
		reg->Load(L"SendAltEsc", isSendAltEsc);
		reg->Load(L"SendAltPrintScrn", isSendAltPrintScrn);
		reg->Load(L"SendPrintScrn", isSendPrintScrn);
		reg->Load(L"SendCtrlEsc", isSendCtrlEsc);
		reg->Load(L"SendAltF9", isSendAltF9);
		
		reg->Load(L"Min2Tray", isMinToTray);
		reg->Load(L"AlwaysShowTrayIcon", isAlwaysShowTrayIcon);
		
		reg->Load(L"SafeFarClose", isSafeFarClose);
		if (!reg->Load(L"SafeFarCloseMacro", &sSafeFarCloseMacro) || (sSafeFarCloseMacro && !*sSafeFarCloseMacro)) { SafeFree(sSafeFarCloseMacro); }
		
		reg->Load(L"FARuseASCIIsort", isFARuseASCIIsort);
		reg->Load(L"ShellNoZoneCheck", isShellNoZoneCheck);
		reg->Load(L"FixAltOnAltTab", isFixAltOnAltTab);
		
		reg->Load(L"BackGround Image show", isShowBgImage);
		if (isShowBgImage!=0 && isShowBgImage!=1 && isShowBgImage!=2) isShowBgImage = 0;
		reg->Load(L"BackGround Image", sBgImage, countof(sBgImage));
		reg->Load(L"bgImageDarker", bgImageDarker);
		reg->Load(L"bgImageColors", nBgImageColors);
		if (!nBgImageColors) nBgImageColors = (DWORD)-1; //1|2 == BgImageColorsDefaults;
		reg->Load(L"bgOperation", bgOperation);
		if (bgOperation!=eUpLeft && bgOperation!=eStretch && bgOperation!=eTile) bgOperation = 0;
		reg->Load(L"bgPluginAllowed", isBgPluginAllowed);
		if (isBgPluginAllowed!=0 && isBgPluginAllowed!=1 && isBgPluginAllowed!=2) isBgPluginAllowed = 1;

		reg->Load(L"AlphaValue", nTransparent);
		if (nTransparent < MIN_ALPHA_VALUE) nTransparent = MIN_ALPHA_VALUE;

		//reg->Load(L"UseColorKey", isColorKey);
		//reg->Load(L"ColorKey", ColorKey);
		reg->Load(L"UserScreenTransparent", isUserScreenTransparent);
		
		//reg->Load(L"Update Console handle", isUpdConHandle);
		reg->Load(L"DisableMouse", isDisableMouse);
		reg->Load(L"RSelectionFix", isRSelFix);
		reg->Load(L"MouseSkipActivation", isMouseSkipActivation);
		reg->Load(L"MouseSkipMoving", isMouseSkipMoving);
		reg->Load(L"FarHourglass", isFarHourglass);
		reg->Load(L"FarHourglassDelay", nFarHourglassDelay);
		
		reg->Load(L"Dnd", isDragEnabled);
		isDropEnabled = (BYTE)(isDragEnabled ? 1 : 0); // ����� "DndDrop" �� ����, ������� ������ default
		reg->Load(L"DndLKey", nLDragKey);
		reg->Load(L"DndRKey", nRDragKey);
		reg->Load(L"DndDrop", isDropEnabled);
		reg->Load(L"DefCopy", isDefCopy);
		reg->Load(L"DragOverlay", isDragOverlay);
		reg->Load(L"DragShowIcons", isDragShowIcons);
		
		reg->Load(L"DragPanel", isDragPanel); if (isDragPanel > 2) isDragPanel = 1;
		reg->Load(L"DragPanelBothEdges", isDragPanelBothEdges);
		
		reg->Load(L"DebugSteps", isDebugSteps);				

		//reg->Load(L"GUIpb", isGUIpb);
		reg->Load(L"Tabs", isTabs);
		reg->Load(L"TabSelf", isTabSelf);
		reg->Load(L"TabLazy", isTabLazy);
		reg->Load(L"TabRecent", isTabRecent);
		reg->Load(L"TabsOnTaskBar", m_isTabsOnTaskBar);

		if (!reg->Load(L"TabCloseMacro", &sTabCloseMacro) || (sTabCloseMacro && !*sTabCloseMacro)) { SafeFree(sTabCloseMacro); }

		reg->Load(L"TabFontFace", sTabFontFace, countof(sTabFontFace));
		reg->Load(L"TabFontCharSet", nTabFontCharSet);
		reg->Load(L"TabFontHeight", nTabFontHeight);

		if (!reg->Load(L"SaveAllEditors", &sSaveAllMacro) || (sSaveAllMacro && !*sSaveAllMacro)) { SafeFree(sSaveAllMacro); }

		reg->Load(L"TabFrame", isTabFrame);
		reg->Load(L"TabMargins", rcTabMargins);
		
		reg->Load(L"ToolbarAddSpace", nToolbarAddSpace);
		if (nToolbarAddSpace<0 || nToolbarAddSpace>100) nToolbarAddSpace = 0;

		reg->Load(L"SlideShowElapse", nSlideShowElapse);
		
		reg->Load(L"IconID", nIconID);
		
		
		reg->Load(L"TabConsole", szTabConsole, countof(szTabConsole));
		//WCHAR* pszVert = wcschr(szTabPanels, L'|');
		//if (!pszVert) {
		//    if (_tcslen(szTabPanels)>54) szTabPanels[54] = 0;
		//    pszVert = szTabPanels + _tcslen(szTabPanels);
		//    wcscpy_c(pszVert+1, L"Console");
		//}
		//*pszVert = 0; pszTabConsole = pszVert+1;
		reg->Load(L"TabEditor", szTabEditor, countof(szTabEditor));
		reg->Load(L"TabEditorModified", szTabEditorModified, countof(szTabEditorModified));
		reg->Load(L"TabViewer", szTabViewer, countof(szTabViewer));
		reg->Load(L"TabLenMax", nTabLenMax); if (nTabLenMax < 10 || nTabLenMax >= CONEMUTABMAX) nTabLenMax = 20;

		/*reg->Load(L"ScrollTitle", isScrollTitle);
		reg->Load(L"ScrollTitleLen", ScrollTitleLen);*/
		reg->Load(L"AdminTitleSuffix", szAdminTitleSuffix, countof(szAdminTitleSuffix)); szAdminTitleSuffix[countof(szAdminTitleSuffix)-1] = 0;
		reg->Load(L"AdminShowShield", bAdminShield);
		reg->Load(L"HideInactiveConsoleTabs", bHideInactiveConsoleTabs);
		//reg->Load(L"HideDisabledTabs", bHideDisabledTabs);
		reg->Load(L"ShowFarWindows", bShowFarWindows);
		
		reg->Load(L"TryToCenter", isTryToCenter);

		reg->Load(L"ShowScrollbar", isAlwaysShowScrollbar); if (isAlwaysShowScrollbar > 2) isAlwaysShowScrollbar = 2;

		//reg->Load(L"CreateAppWindow", isCreateAppWindow);
		//reg->Load(L"AllowDetach", isAllowDetach);

		reg->Load(L"MainTimerElapse", nMainTimerElapse); if (nMainTimerElapse>1000) nMainTimerElapse = 1000;
		reg->Load(L"MainTimerInactiveElapse", nMainTimerInactiveElapse); if (nMainTimerInactiveElapse>10000) nMainTimerInactiveElapse = 10000;

		reg->Load(L"AffinityMask", nAffinity);
		
		//reg->Load(L"AdvLangChange", isAdvLangChange);
		
		reg->Load(L"SkipFocusEvents", isSkipFocusEvents);
		
		//reg->Load(L"LangChangeWsPlugin", isLangChangeWsPlugin);
		reg->Load(L"MonitorConsoleLang", isMonitorConsoleLang);
		reg->Load(L"DesktopMode", isDesktopMode);
		reg->Load(L"AlwaysOnTop", isAlwaysOnTop);
		reg->Load(L"SleepInBackground", isSleepInBackground);

		reg->Load(L"DisableFarFlashing", isDisableFarFlashing); if (isDisableFarFlashing>2) isDisableFarFlashing = 2;

		reg->Load(L"DisableAllFlashing", isDisableAllFlashing); if (isDisableAllFlashing>2) isDisableAllFlashing = 2;

		/* *********** Thumbnails and Tiles ************* */
		reg->Load(L"PanView.BackColor", ThSet.crBackground.RawColor);

		reg->Load(L"PanView.PFrame", ThSet.nPreviewFrame); if (ThSet.nPreviewFrame!=0 && ThSet.nPreviewFrame!=1) ThSet.nPreviewFrame = 1;

		reg->Load(L"PanView.PFrameColor", ThSet.crPreviewFrame.RawColor);

		reg->Load(L"PanView.SFrame", ThSet.nSelectFrame); if (ThSet.nSelectFrame!=0 && ThSet.nSelectFrame!=1) ThSet.nSelectFrame = 1;

		reg->Load(L"PanView.SFrameColor", ThSet.crSelectFrame.RawColor);
		/* ������ ������������� ������� */
		ThumbLoadSet(L"Thumbs", ThSet.Thumbs);
		ThumbLoadSet(L"Tiles", ThSet.Tiles);
		// ������ ��������� ��������
		reg->Load(L"PanView.LoadPreviews", ThSet.bLoadPreviews);
		reg->Load(L"PanView.LoadFolders", ThSet.bLoadFolders);
		reg->Load(L"PanView.LoadTimeout", ThSet.nLoadTimeout);
		reg->Load(L"PanView.MaxZoom", ThSet.nMaxZoom);
		reg->Load(L"PanView.UsePicView2", ThSet.bUsePicView2);
		reg->Load(L"PanView.RestoreOnStartup", ThSet.bRestoreOnStartup);

		/* *** AutoUpdate *** */
		reg->Load(L"Update.VerLocation", &UpdSet.szUpdateVerLocation);
		reg->Load(L"Update.CheckOnStartup", UpdSet.isUpdateCheckOnStartup);
		reg->Load(L"Update.CheckHourly", UpdSet.isUpdateCheckHourly);
		reg->Load(L"Update.ConfirmDownload", UpdSet.isUpdateConfirmDownload);
		reg->Load(L"Update.UseBuilds", UpdSet.isUpdateUseBuilds); if (UpdSet.isUpdateUseBuilds>2) UpdSet.isUpdateUseBuilds = 2; // 1-stable only, 2-latest
		reg->Load(L"Update.UseProxy", UpdSet.isUpdateUseProxy);
		reg->Load(L"Update.Proxy", &UpdSet.szUpdateProxy);
		reg->Load(L"Update.ProxyUser", &UpdSet.szUpdateProxyUser);
		reg->Load(L"Update.ProxyPassword", &UpdSet.szUpdateProxyPassword);
		reg->Load(L"Update.DownloadSetup", UpdSet.isUpdateDownloadSetup); // 1-Installer (ConEmuSetup.exe), 2-7z archieve (ConEmu.7z), WinRar or 7z required
		reg->Load(L"Update.ExeCmdLine", &UpdSet.szUpdateExeCmdLine);
		reg->Load(L"Update.ArcCmdLine", &UpdSet.szUpdateArcCmdLine);
		reg->Load(L"Update.DownloadPath", &UpdSet.szUpdateDownloadPath);
		reg->Load(L"Update.LeavePackages", UpdSet.isUpdateLeavePackages);
		reg->Load(L"Update.PostUpdateCmd", &UpdSet.szUpdatePostUpdateCmd);

		/* Done */
		reg->CloseKey();
	}

	// ������ ������ �� �����
	delete reg;
	reg = NULL;

	
	// ����� "FastConfiguration" (����� ��������� �����/������ ������������)
	{
		LPCWSTR pszDef = gpConEmu->GetDefaultTitle();
		wchar_t szType[8]; bool ReadOnly;
		GetSettingsType(szType, ReadOnly);
		LPCWSTR pszConfig = gpSetCls->GetConfigName();
		wchar_t szTitle[1024];
		if (pszConfig && *pszConfig)
			_wsprintf(szTitle, SKIPLEN(countof(szTitle)) L"%s fast configuration (%s) %s", pszDef, pszConfig, szType);
		else
			_wsprintf(szTitle, SKIPLEN(countof(szTitle)) L"%s fast configuration %s", pszDef, szType);
		
		CheckOptionsFast(szTitle, lbNeedCreateVanilla);
	}
	

	if (lbNeedCreateVanilla)
	{
		SaveSettings(TRUE/*abSilent*/);
	}

	// ����������� ������� ���������
	mb_FadeInitialized = false; GetColors(TRUE);
	
	

	// ��������� ������������� ��������� �����
	//-- isKeyboardHooks();
	// ��� ������ ������� - ���������, ����� �� �������� ��������������?
	//-- CheckUpdatesWanted();



	// ����� ����
	if (!WindowMode)
	{
		// ����� ���� ������ �� ������������
		_ASSERTE(WindowMode!=0);
		WindowMode = rNormal;
	}

	if (wndCascade && (ghWnd == NULL))
	{
		// ����� ��� �������
		int nShift = (GetSystemMetrics(SM_CYSIZEFRAME)+GetSystemMetrics(SM_CYCAPTION))*1.5;
		// ���������� � ������ ����������� ������� �������
		RECT rcScreen = MakeRect(800,600);
		int nMonitors = GetSystemMetrics(SM_CMONITORS);

		if (nMonitors > 1)
		{
			// ������ ������������ ������ �� ���� ���������
			rcScreen.left = GetSystemMetrics(SM_XVIRTUALSCREEN); // may be <0
			rcScreen.top  = GetSystemMetrics(SM_YVIRTUALSCREEN);
			rcScreen.right = rcScreen.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
			rcScreen.bottom = rcScreen.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);
			TODO("������ �� ��������� �� ������������ Taskbar...");
		}
		else
		{
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0);
		}

		HWND hPrev = FindWindow(VirtualConsoleClassMain, NULL);

		while(hPrev)
		{
			/*if (Is Iconic(hPrev) || Is Zoomed(hPrev)) {
				hPrev = FindWindowEx(NULL, hPrev, VirtualConsoleClassMain, NULL);
				continue;
			}*/
			WINDOWPLACEMENT wpl = {sizeof(WINDOWPLACEMENT)}; // Workspace coordinates!!!

			if (!GetWindowPlacement(hPrev, &wpl))
			{
				break;
			}

			// Screen coordinates!
			RECT rcWnd; GetWindowRect(hPrev, &rcWnd);

			if (wpl.showCmd == SW_HIDE || !IsWindowVisible(hPrev)
			        || wpl.showCmd == SW_SHOWMINIMIZED || wpl.showCmd == SW_SHOWMAXIMIZED
			        /* Max � ������ ������� ��������� */
			        || (wpl.rcNormalPosition.left<rcScreen.left || wpl.rcNormalPosition.top<rcScreen.top))
			{
				hPrev = FindWindowEx(NULL, hPrev, VirtualConsoleClassMain, NULL);
				continue;
			}

			wndX = rcWnd.left + nShift;
			wndY = rcWnd.top + nShift;
			break; // �����, ����������, �������
		}
	}

	if (rcTabMargins.top > 100) rcTabMargins.top = 100;

	_ASSERTE(!rcTabMargins.bottom && !rcTabMargins.left && !rcTabMargins.right);
	rcTabMargins.bottom = rcTabMargins.left = rcTabMargins.right = 0;

	if (!psCmdHistory)
	{
		psCmdHistory = (wchar_t*)calloc(2,2);
	}

	MCHKHEAP
	
	// ���������� ���������, ��������� �������������� �������� � ������ ��������
	gpSetCls->SettingsLoaded();
}

/*
[HKEY_CURRENT_USER\Software\ConEmu\.Vanilla\Fonts\Font3]
"Title"="CJK"
"Used"=hex:01 ; ��� �������� ���������/���������� ������
"CharRange"="2E80-2EFF;3000-303F;3200-4DB5;4E00-9FC3;F900-FAFF;FE30-FE4F;"
"FontFace"="Arial Unicode MS"
"Width"=dword:00000000
"Height"=dword:00000009
"Charset"=hex:00
"Bold"=hex:00
"Italic"=hex:00
"Anti-aliasing"=hex:04
*/
BOOL Settings::FontRangeLoad(SettingsBase* reg, int Idx)
{
	_ASSERTE(FALSE); // �� �����������

	BOOL lbOpened = FALSE; //, lbNeedCreateVanilla = FALSE;
	wchar_t szFontKey[MAX_PATH+20];
	_wsprintf(szFontKey, SKIPLEN(countof(szFontKey)) L"%s\\Font%i", gpSetCls->GetConfigPath(), (Idx+1));
	
	
	
	lbOpened = reg->OpenKey(szFontKey, KEY_READ);
	if (lbOpened)
	{
		TODO("��������� ���� m_Fonts[Idx], ������� hFonts, �� ������ ��� nLoadHeight/nLoadWidth");

		reg->CloseKey();
	}
	
	return lbOpened;
}
BOOL Settings::FontRangeSave(SettingsBase* reg, int Idx)
{
	_ASSERTE(FALSE); // �� �����������
	return FALSE;
}

void Settings::SaveSizePosOnExit()
{
	if (!this || !isAutoSaveSizePos)
		return;

	// ��� �������� ���� ��������� - ��������� ������ ���� ���,
	// � �� ������ ����� � �������� �������� �������� ����������
	if (mb_SizePosAutoSaved)
		return;
	mb_SizePosAutoSaved = true;
		
	SettingsBase* reg = CreateSettings();

	if (reg->OpenKey(gpSetCls->GetConfigPath(), KEY_WRITE))
	{
		reg->Save(L"ConWnd Width", wndWidth);
		reg->Save(L"ConWnd Height", wndHeight);
		reg->Save(L"16bit Height", ntvdmHeight);
		reg->Save(L"ConWnd X", wndX);
		reg->Save(L"ConWnd Y", wndY);
		reg->Save(L"Cascaded", wndCascade);
		reg->Save(L"AutoSaveSizePos", isAutoSaveSizePos);
		reg->CloseKey();
	}

	delete reg;
}

void Settings::SaveConsoleFont()
{
	if (!this)
		return;

	SettingsBase* reg = CreateSettings();

	if (reg->OpenKey(gpSetCls->GetConfigPath(), KEY_WRITE))
	{
		reg->Save(L"ConsoleFontName", ConsoleFont.lfFaceName);
		reg->Save(L"ConsoleFontWidth", ConsoleFont.lfWidth);
		reg->Save(L"ConsoleFontHeight", ConsoleFont.lfHeight);
		reg->CloseKey();
	}

	delete reg;
}

void Settings::UpdateMargins(RECT arcMargins)
{
	if (memcmp(&arcMargins, &rcTabMargins, sizeof(rcTabMargins))==0)
		return;

	rcTabMargins = arcMargins;
	SettingsBase* reg = CreateSettings();

	if (reg->OpenKey(gpSetCls->GetConfigPath(), KEY_WRITE))
	{
		reg->Save(L"TabMargins", rcTabMargins);
		reg->CloseKey();
	}

	delete reg;
}

BOOL Settings::SaveSettings(BOOL abSilent /*= FALSE*/)
{
	gpSetCls->SettingsPreSave();

	SettingsBase* reg = CreateSettings();

	if (reg->OpenKey(gpSetCls->GetConfigPath(), KEY_WRITE, abSilent))
	{
		wcscpy_c(Type, reg->Type);
		TCHAR ColorName[] = L"ColorTable00";

		for(uint i = 0; i<countof(Colors)/*0x20*/; i++)
		{
			ColorName[10] = i/10 + '0';
			ColorName[11] = i%10 + '0';
			reg->Save(ColorName, (DWORD)Colors[i]);
		}

		reg->Save(L"ExtendColors", isExtendColors);
		reg->Save(L"ExtendColorIdx", nExtendColor);
		reg->Save(L"TrueColorerSupport", isTrueColorer);
		reg->Save(L"FadeInactive", isFadeInactive);
		reg->Save(L"FadeInactiveLow", mn_FadeLow);
		reg->Save(L"FadeInactiveHigh", mn_FadeHigh);
		/* ���� ��������, ����� ��������� ���������� ����� ���� */
		reg->Save(L"ExtendFonts", isExtendFonts);
		reg->Save(L"ExtendFontNormalIdx", nFontNormalColor);
		reg->Save(L"ExtendFontBoldIdx", nFontBoldColor);
		reg->Save(L"ExtendFontItalicIdx", nFontItalicColor);

		/*if (!isFullScreen && !gpConEmu->isZoomed() && !gpConEmu->isIconic())
		{
		    RECT rcPos; GetWindowRect(ghWnd, &rcPos);
		    wndX = rcPos.left;
		    wndY = rcPos.top;
		}*/
		reg->Save(L"ConVisible", isConVisible);
		reg->Save(L"ConInMode", nConInMode);
		
		reg->Save(L"UseInjects", isUseInjects);

		#ifdef USEPORTABLEREGISTRY
		reg->Save(L"PortableReg", isPortableReg);
		#endif
		
		//reg->Save(L"LockRealConsolePos", isLockRealConsolePos);
		reg->Save(L"CmdLine", psCmd);

		if (psCmdHistory)
			reg->SaveMSZ(L"CmdLineHistory", psCmdHistory, nCmdHistorySize);

		reg->Save(L"Multi", isMulti);
		reg->Save(L"Multi.Modifier", nMultiHotkeyModifier);
		reg->Save(L"Multi.NewConsole", icMultiNew);
		reg->Save(L"Multi.Next", icMultiNext);
		reg->Save(L"Multi.Recreate", icMultiRecreate);
		reg->Save(L"Multi.Close", icMultiClose);
		reg->Save(L"Multi.CmdKey", icMultiCmd);
		reg->Save(L"Multi.NewConfirm", isMultiNewConfirm);
		reg->Save(L"Multi.Buffer", icMultiBuffer);
		reg->Save(L"Multi.UseArrows", isUseWinArrows);
		reg->Save(L"Multi.UseNumbers", isUseWinNumber);
		reg->Save(L"Multi.UseWinTab", isUseWinTab);
		reg->Save(L"Multi.AutoCreate", isMultiAutoCreate);
		reg->Save(L"Multi.LeaveOnClose", isMultiLeaveOnClose);
		reg->Save(L"Multi.Iterate", isMultiIterate);
		reg->Save(L"MinimizeRestore", icMinimizeRestore);
		_ASSERTE(m_isKeyboardHooks!=0);
		reg->Save(L"KeyboardHooks", m_isKeyboardHooks);
		reg->Save(L"FontName", inFont);
		reg->Save(L"FontName2", inFont2);
		bool lbTest = isAutoRegisterFonts; // ���� � ������� ��������� ����, ��� ���������� ��������

		if (reg->Load(L"AutoRegisterFonts", lbTest) || isAutoRegisterFonts != lbTest)
			reg->Save(L"AutoRegisterFonts", isAutoRegisterFonts);
			
		reg->Save(L"FontAutoSize", isFontAutoSize);
		reg->Save(L"FontSize", FontSizeY);
		reg->Save(L"FontSizeX", FontSizeX);
		reg->Save(L"FontSizeX2", FontSizeX2);
		reg->Save(L"FontSizeX3", FontSizeX3);
		reg->Save(L"FontCharSet", mn_LoadFontCharSet);
		if (ghOpWnd != NULL)			
			mb_CharSetWasSet = FALSE;
		reg->Save(L"Anti-aliasing", mn_AntiAlias);
		reg->Save(L"FontBold", isBold);
		reg->Save(L"FontItalic", isItalic);
		
		reg->Save(L"Monospace", isMonospace);
		
		reg->Save(L"BackGround Image show", isShowBgImage);		
		reg->Save(L"BackGround Image", sBgImage);		
		reg->Save(L"bgImageDarker", bgImageDarker);		
		reg->Save(L"bgImageColors", nBgImageColors);		
		reg->Save(L"bgOperation", bgOperation);		
		reg->Save(L"bgPluginAllowed", isBgPluginAllowed);		
		reg->Save(L"AlphaValue", nTransparent);		
		//reg->Save(L"UseColorKey", isColorKey);		
		//reg->Save(L"ColorKey", ColorKey);		
		reg->Save(L"UserScreenTransparent", isUserScreenTransparent);		
		DWORD saveMode = (ghWnd == NULL) 
					? gpConEmu->WindowMode
					: (gpConEmu->isFullScreen() ? rFullScreen : gpConEmu->isZoomed() ? rMaximized : rNormal);
		reg->Save(L"WindowMode", saveMode);
		reg->Save(L"HideCaption", isHideCaption);
		reg->Save(L"HideCaptionAlways", mb_HideCaptionAlways);
		reg->Save(L"HideCaptionAlwaysFrame", nHideCaptionAlwaysFrame);
		reg->Save(L"HideCaptionAlwaysDelay", nHideCaptionAlwaysDelay);
		reg->Save(L"HideCaptionAlwaysDisappear", nHideCaptionAlwaysDisappear);
		reg->Save(L"DownShowHiddenMessage", isDownShowHiddenMessage);
		reg->Save(L"ConsoleFontName", ConsoleFont.lfFaceName);
		reg->Save(L"ConsoleFontWidth", ConsoleFont.lfWidth);
		reg->Save(L"ConsoleFontHeight", ConsoleFont.lfHeight);
		reg->Save(L"DefaultBufferHeight", DefaultBufferHeight);
		reg->Save(L"AutoBufferHeight", AutoBufferHeight);
		reg->Save(L"CmdOutputCP", nCmdOutputCP);
		reg->Save(L"CursorType", isCursorV);
		reg->Save(L"CursorColor", isCursorColor);
		reg->Save(L"CursorBlink", isCursorBlink);
		reg->Save(L"CursorBlockInactive", isCursorBlockInactive);
		reg->Save(L"ConsoleTextSelection", isConsoleTextSelection);
		reg->Save(L"CTS.SelectBlock", isCTSSelectBlock);
		reg->Save(L"CTS.VkBlock", isCTSVkBlock);
		reg->Save(L"CTS.VkBlockStart", isCTSVkBlockStart);
		reg->Save(L"CTS.SelectText", isCTSSelectText);
		reg->Save(L"CTS.VkText", isCTSVkText);
		reg->Save(L"CTS.VkTextStart", isCTSVkTextStart);
		reg->Save(L"CTS.ActMode", isCTSActMode);
		reg->Save(L"CTS.VkAct", isCTSVkAct);
		reg->Save(L"CTS.RBtnAction", isCTSRBtnAction);
		reg->Save(L"CTS.MBtnAction", isCTSMBtnAction);
		reg->Save(L"CTS.ColorIndex", isCTSColorIndex);
		
		reg->Save(L"FarGotoEditor", isFarGotoEditor);
		reg->Save(L"FarGotoEditorVk", isFarGotoEditorVk);
		
		reg->Save(L"FixFarBorders", isFixFarBorders);
		{
		wchar_t* pszCharRanges = CreateCharRanges(mpc_FixFarBorderValues);
		reg->Save(L"FixFarBordersRanges", pszCharRanges ? pszCharRanges : L"2013-25C4");
		if (pszCharRanges) free(pszCharRanges);
		}
		reg->Save(L"ExtendUCharMap", isExtendUCharMap);
		reg->Save(L"EnhanceGraphics", isEnhanceGraphics);
		reg->Save(L"EnhanceButtons", isEnhanceButtons);
		reg->Save(L"PartBrush75", isPartBrush75);
		reg->Save(L"PartBrush50", isPartBrush50);
		reg->Save(L"PartBrush25", isPartBrush25);
		reg->Save(L"PartBrushBlack", isPartBrushBlack);
		reg->Save(L"RightClick opens context menu", isRClickSendKey);
		reg->Save(L"RightClickMacro2", sRClickMacro);
		reg->Save(L"AltEnter", isSendAltEnter);
		reg->Save(L"AltSpace", isSendAltSpace);
		reg->Save(L"SendAltTab", isSendAltTab);
		reg->Save(L"SendAltEsc", isSendAltEsc);
		reg->Save(L"SendAltPrintScrn", isSendAltPrintScrn);
		reg->Save(L"SendPrintScrn", isSendPrintScrn);
		reg->Save(L"SendCtrlEsc", isSendCtrlEsc);
		reg->Save(L"SendAltF9", isSendAltF9);
		reg->Save(L"Min2Tray", isMinToTray);
		reg->Save(L"AlwaysShowTrayIcon", isAlwaysShowTrayIcon);
		reg->Save(L"SafeFarClose", isSafeFarClose);
		reg->Save(L"SafeFarCloseMacro", sSafeFarCloseMacro);
		reg->Save(L"FARuseASCIIsort", isFARuseASCIIsort);
		reg->Save(L"ShellNoZoneCheck", isShellNoZoneCheck);
		reg->Save(L"FixAltOnAltTab", isFixAltOnAltTab);
		reg->Save(L"DisableMouse", isDisableMouse);
		reg->Save(L"RSelectionFix", isRSelFix);
		reg->Save(L"MouseSkipActivation", isMouseSkipActivation);
		reg->Save(L"MouseSkipMoving", isMouseSkipMoving);
		reg->Save(L"FarHourglass", isFarHourglass);
		reg->Save(L"FarHourglassDelay", nFarHourglassDelay);
		reg->Save(L"Dnd", isDragEnabled);
		reg->Save(L"DndLKey", nLDragKey);
		reg->Save(L"DndRKey", nRDragKey);
		reg->Save(L"DndDrop", isDropEnabled);
		reg->Save(L"DefCopy", isDefCopy);
		reg->Save(L"DragOverlay", isDragOverlay);
		reg->Save(L"DragShowIcons", isDragShowIcons);
		reg->Save(L"DebugSteps", isDebugSteps);
		reg->Save(L"DragPanel", isDragPanel);
		reg->Save(L"DragPanelBothEdges", isDragPanelBothEdges);
		//reg->Save(L"GUIpb", isGUIpb);
		reg->Save(L"Tabs", isTabs);
		reg->Save(L"TabSelf", isTabSelf);
		reg->Save(L"TabLazy", isTabLazy);
		reg->Save(L"TabRecent", isTabRecent);
		reg->Save(L"TabsOnTaskBar", m_isTabsOnTaskBar);
		reg->Save(L"TabCloseMacro", sTabCloseMacro);
		reg->Save(L"TabFontFace", sTabFontFace);
		reg->Save(L"TabFontCharSet", nTabFontCharSet);
		reg->Save(L"TabFontHeight", nTabFontHeight);
		reg->Save(L"SaveAllEditors", sSaveAllMacro);
		reg->Save(L"TabFrame", isTabFrame);
		reg->Save(L"TabMargins", rcTabMargins);
		reg->Save(L"ToolbarAddSpace", nToolbarAddSpace);
		reg->Save(L"TabConsole", szTabConsole);
		reg->Save(L"TabEditor", szTabEditor);
		reg->Save(L"TabEditorModified", szTabEditorModified);
		reg->Save(L"TabViewer", szTabViewer);
		reg->Save(L"TabLenMax", nTabLenMax);
		reg->Save(L"AdminTitleSuffix", szAdminTitleSuffix);
		reg->Save(L"AdminShowShield", bAdminShield);
		reg->Save(L"HideInactiveConsoleTabs", bHideInactiveConsoleTabs);
		//reg->Save(L"HideDisabledTabs", bHideDisabledTabs);
		reg->Save(L"ShowFarWindows", bShowFarWindows);
		reg->Save(L"TryToCenter", isTryToCenter);
		reg->Save(L"ShowScrollbar", isAlwaysShowScrollbar);
		reg->Save(L"IconID", nIconID);
		//reg->Save(L"Update Console handle", isUpdConHandle);
		reg->Save(L"ConWnd Width", wndWidth);
		reg->Save(L"ConWnd Height", wndHeight);
		reg->Save(L"16bit Height", ntvdmHeight);
		reg->Save(L"ConWnd X", wndX);
		reg->Save(L"ConWnd Y", wndY);
		reg->Save(L"Cascaded", wndCascade);
		reg->Save(L"AutoSaveSizePos", isAutoSaveSizePos);
		mb_SizePosAutoSaved = false; // ��� ���� �������������� ������������� ���������� �������� - ������� ������
		/*reg->Save(L"ScrollTitle", isScrollTitle);
		reg->Save(L"ScrollTitleLen", ScrollTitleLen);*/
		reg->Save(L"MainTimerElapse", nMainTimerElapse);
		reg->Save(L"MainTimerInactiveElapse", nMainTimerInactiveElapse);
		reg->Save(L"AffinityMask", nAffinity);
		reg->Save(L"SkipFocusEvents", isSkipFocusEvents);
		reg->Save(L"MonitorConsoleLang", isMonitorConsoleLang);
		reg->Save(L"DesktopMode", isDesktopMode);
		reg->Save(L"AlwaysOnTop", isAlwaysOnTop);
		reg->Save(L"SleepInBackground", isSleepInBackground);
		reg->Save(L"DisableFarFlashing", isDisableFarFlashing);
		reg->Save(L"DisableAllFlashing", isDisableAllFlashing);
		/* *********** Thumbnails and Tiles ************* */
		reg->Save(L"PanView.BackColor", ThSet.crBackground.RawColor);
		reg->Save(L"PanView.PFrame", ThSet.nPreviewFrame);
		reg->Save(L"PanView.PFrameColor", ThSet.crPreviewFrame.RawColor);
		reg->Save(L"PanView.SFrame", ThSet.nSelectFrame);
		reg->Save(L"PanView.SFrameColor", ThSet.crSelectFrame.RawColor);
		/* ������ ������������� ������� */
		ThumbSaveSet(L"Thumbs", ThSet.Thumbs);
		ThumbSaveSet(L"Tiles", ThSet.Tiles);
		// ������ ��������� ��������
		reg->Save(L"PanView.LoadPreviews", ThSet.bLoadPreviews);
		reg->Save(L"PanView.LoadFolders", ThSet.bLoadFolders);
		reg->Save(L"PanView.LoadTimeout", ThSet.nLoadTimeout);
		reg->Save(L"PanView.MaxZoom", ThSet.nMaxZoom);
		reg->Save(L"PanView.UsePicView2", ThSet.bUsePicView2);
		reg->Save(L"PanView.RestoreOnStartup", ThSet.bRestoreOnStartup);

		/* *** AutoUpdate *** */
		reg->Save(L"Update.VerLocation", UpdSet.szUpdateVerLocation);
		reg->Save(L"Update.CheckOnStartup", UpdSet.isUpdateCheckOnStartup);
		reg->Save(L"Update.CheckHourly", UpdSet.isUpdateCheckHourly);
		reg->Save(L"Update.ConfirmDownload", UpdSet.isUpdateConfirmDownload);
		reg->Save(L"Update.UseBuilds", UpdSet.isUpdateUseBuilds);
		reg->Save(L"Update.UseProxy", UpdSet.isUpdateUseProxy);
		reg->Save(L"Update.Proxy", UpdSet.szUpdateProxy);
		reg->Save(L"Update.ProxyUser", UpdSet.szUpdateProxyUser);
		reg->Save(L"Update.ProxyPassword", UpdSet.szUpdateProxyPassword);
		reg->Save(L"Update.DownloadSetup", UpdSet.isUpdateDownloadSetup); // 1-Installer (ConEmuSetup.exe), 2-7z archieve (ConEmu.7z), WinRar or 7z required
		reg->Save(L"Update.ExeCmdLine", UpdSet.szUpdateExeCmdLine);
		reg->Save(L"Update.ArcCmdLine", UpdSet.szUpdateArcCmdLine);
		reg->Save(L"Update.DownloadPath", UpdSet.szUpdateDownloadPath);
		reg->Save(L"Update.LeavePackages", UpdSet.isUpdateLeavePackages);
		reg->Save(L"Update.PostUpdateCmd", UpdSet.szUpdatePostUpdateCmd);

		/* Done */
		reg->CloseKey();
		delete reg;
		//if (isTabs==1) ForceShowTabs();
		//MessageBoxA(ghOpWnd, "Saved.", "Information", MB_ICONINFORMATION);
		return TRUE;
	}

	//}
	delete reg;

	// ����� � ���������� �� �����. ������ ��� ��� ��������
	//MessageBoxA(ghOpWnd, "Failed", "Information", MB_ICONERROR);
	return FALSE;
}

bool Settings::isKeyboardHooks()
{
	//// ����� � ��� WinXP, �� ������ � "���������" ������
	//if (gOSVer.dwMajorVersion < 6)
	//{
	//	return true;
	//}


	//if (m_isKeyboardHooks == 0)
	//{
	//	// ������ ������������ ��� �� �������� (��� �� ������, ���� ��� � �� �������)
	//	int nBtn = MessageBox(NULL,
	//	                      L"Do You want to use Win-Number combination for \n"
	//	                      L"switching between consoles (Multi Console feature)? \n\n"
	//	                      L"If You choose 'Yes' - ConEmu will install keyboard hook. \n"
	//	                      L"So, You must allow that in antiviral software (such as AVP). \n\n"
	//	                      L"You can change behavior later via Settings->Features->\n"
	//	                      L"'Install keyboard hooks (Vista & Win7)' check box, or\n"
	//	                      L"'KeyboardHooks' value in ConEmu settings (registry or xml)."
	//	                      , gpConEmu->GetDefaultTitle(), MB_YESNOCANCEL|MB_ICONQUESTION);

	//	if (nBtn == IDCANCEL)
	//	{
	//		m_isKeyboardHooks = 2; // NO
	//	}
	//	else
	//	{
	//		m_isKeyboardHooks = (nBtn == IDYES) ? 1 : 2;
	//		SettingsBase* reg = CreateSettings();

	//		if (!reg)
	//		{
	//			_ASSERTE(reg!=NULL);
	//		}
	//		else
	//		{
	//			if (reg->OpenKey(gpSetCls->GetConfigPath(), KEY_WRITE))
	//			{
	//				reg->Save(L"KeyboardHooks", m_isKeyboardHooks);
	//				reg->CloseKey();
	//			}

	//			delete reg;
	//		}
	//	}
	//}

	return (m_isKeyboardHooks == 1);
}

//bool Settings::CheckUpdatesWanted()
//{
//	if (UpdSet.isUpdateUseBuilds == 0)
//	{
//		int nBtn = MessageBox(NULL,
//			L"Do you want to enable automatic updates? \n\n"
//			L"You may change settings on 'Update' page of Settings dialog later"
//			, gpConEmu->GetDefaultTitle(), MB_YESNOCANCEL|MB_ICONQUESTION);
//
//		if (nBtn == IDCANCEL)
//		{
//			return false;
//		}
//
//		UpdSet.isUpdateCheckOnStartup = (nBtn == IDYES);
//		UpdSet.isUpdateCheckHourly = false;
//		UpdSet.isUpdateConfirmDownload = true;
//		UpdSet.isUpdateUseBuilds = 2;
//
//		SettingsBase* reg = CreateSettings();
//		if (!reg)
//		{
//			_ASSERTE(reg!=NULL);
//		}
//		else
//		{
//			if (reg->OpenKey(gpSetCls->GetConfigPath(), KEY_WRITE))
//			{
//				reg->Save(L"Update.CheckOnStartup", UpdSet.isUpdateCheckOnStartup);
//				reg->Save(L"Update.CheckHourly", UpdSet.isUpdateCheckHourly);
//				reg->Save(L"Update.ConfirmDownload", UpdSet.isUpdateConfirmDownload);
//				reg->Save(L"Update.UseBuilds", UpdSet.isUpdateUseBuilds);
//				reg->CloseKey();
//			}
//
//			delete reg;
//		}
//	}
//
//	return (UpdSet.isUpdateUseBuilds != 0);
//}

bool Settings::IsHostkey(WORD vk)
{
	for(int i=0; i < 15 && mn_HostModOk[i]; i++)
		if (mn_HostModOk[i] == vk)
			return true;

	return false;
}

// ���� ���� vk - �������� �� vkNew
void Settings::ReplaceHostkey(BYTE vk, BYTE vkNew)
{
	for(int i = 0; i < 15; i++)
	{
		if (gpSet->mn_HostModOk[i] == vk)
		{
			gpSet->mn_HostModOk[i] = vkNew;
			return;
		}
	}
}

void Settings::AddHostkey(BYTE vk)
{
	for(int i = 0; i < 15; i++)
	{
		if (gpSet->mn_HostModOk[i] == vk)
			break; // ��� ����

		if (!gpSet->mn_HostModOk[i])
		{
			gpSet->mn_HostModOk[i] = vk; // ��������
			break;
		}
	}
}

BYTE Settings::CheckHostkeyModifier(BYTE vk)
{
	// ���� ������� VK_NULL
	if (!vk)
		return 0;

	switch(vk)
	{
		case VK_LWIN: case VK_RWIN:

			if (gpSet->IsHostkey(VK_RWIN))
				ReplaceHostkey(VK_RWIN, VK_LWIN);

			vk = VK_LWIN; // ��������� ������ �����-Win
			break;
		case VK_APPS:
			break; // ��� - ��
		case VK_LSHIFT:

			if (gpSet->IsHostkey(VK_RSHIFT) || gpSet->IsHostkey(VK_SHIFT))
			{
				vk = VK_SHIFT;
				ReplaceHostkey(VK_RSHIFT, VK_SHIFT);
			}

			break;
		case VK_RSHIFT:

			if (gpSet->IsHostkey(VK_LSHIFT) || gpSet->IsHostkey(VK_SHIFT))
			{
				vk = VK_SHIFT;
				ReplaceHostkey(VK_LSHIFT, VK_SHIFT);
			}

			break;
		case VK_SHIFT:

			if (gpSet->IsHostkey(VK_LSHIFT))
				ReplaceHostkey(VK_LSHIFT, VK_SHIFT);
			else if (gpSet->IsHostkey(VK_RSHIFT))
				ReplaceHostkey(VK_RSHIFT, VK_SHIFT);

			break;
		case VK_LMENU:

			if (gpSet->IsHostkey(VK_RMENU) || gpSet->IsHostkey(VK_MENU))
			{
				vk = VK_MENU;
				ReplaceHostkey(VK_RMENU, VK_MENU);
			}

			break;
		case VK_RMENU:

			if (gpSet->IsHostkey(VK_LMENU) || gpSet->IsHostkey(VK_MENU))
			{
				vk = VK_MENU;
				ReplaceHostkey(VK_LMENU, VK_MENU);
			}

			break;
		case VK_MENU:

			if (gpSet->IsHostkey(VK_LMENU))
				ReplaceHostkey(VK_LMENU, VK_MENU);
			else if (gpSet->IsHostkey(VK_RMENU))
				ReplaceHostkey(VK_RMENU, VK_MENU);

			break;
		case VK_LCONTROL:

			if (gpSet->IsHostkey(VK_RCONTROL) || gpSet->IsHostkey(VK_CONTROL))
			{
				vk = VK_CONTROL;
				ReplaceHostkey(VK_RCONTROL, VK_CONTROL);
			}

			break;
		case VK_RCONTROL:

			if (gpSet->IsHostkey(VK_LCONTROL) || gpSet->IsHostkey(VK_CONTROL))
			{
				vk = VK_CONTROL;
				ReplaceHostkey(VK_LCONTROL, VK_CONTROL);
			}

			break;
		case VK_CONTROL:

			if (gpSet->IsHostkey(VK_LCONTROL))
				ReplaceHostkey(VK_LCONTROL, VK_CONTROL);
			else if (gpSet->IsHostkey(VK_RCONTROL))
				ReplaceHostkey(VK_RCONTROL, VK_CONTROL);

			break;
	}

	// �������� � ������ �������� � Host
	AddHostkey(vk);
	// ������� (�������� ����������) VK
	return vk;
}

bool Settings::TestHostkeyModifiers()
{
	memset(mn_HostModOk, 0, sizeof(mn_HostModOk));
	memset(mn_HostModSkip, 0, sizeof(mn_HostModSkip));

	if (!nMultiHotkeyModifier)
		nMultiHotkeyModifier = VK_LWIN;

	BYTE vk;
	vk = (nMultiHotkeyModifier & 0xFF);
	CheckHostkeyModifier(vk);
	vk = (nMultiHotkeyModifier & 0xFF00) >> 8;
	CheckHostkeyModifier(vk);
	vk = (nMultiHotkeyModifier & 0xFF0000) >> 16;
	CheckHostkeyModifier(vk);
	vk = (nMultiHotkeyModifier & 0xFF000000) >> 24;
	CheckHostkeyModifier(vk);
	// ������, ��������� �� ����� 3-� ������ (������ ������ �� �����)
	TrimHostkeys();
	// ������������ (�������� �����������������) ����� HostKey
	bool lbChanged = MakeHostkeyModifier();
	return lbChanged;
}

bool Settings::MakeHostkeyModifier()
{
	bool lbChanged = false;
	// ������������ (�������� �����������������) ����� HostKey
	DWORD nNew = 0;

	if (gpSet->mn_HostModOk[0])
		nNew |= gpSet->mn_HostModOk[0];

	if (gpSet->mn_HostModOk[1])
		nNew |= ((DWORD)(gpSet->mn_HostModOk[1])) << 8;

	if (gpSet->mn_HostModOk[2])
		nNew |= ((DWORD)(gpSet->mn_HostModOk[2])) << 16;

	if (gpSet->mn_HostModOk[3])
		nNew |= ((DWORD)(gpSet->mn_HostModOk[3])) << 24;

	TODO("!!! �������� � mn_HostModSkip �� VK, ������� ����������� � mn_HostModOk");

	if (gpSet->nMultiHotkeyModifier != nNew)
	{
		gpSet->nMultiHotkeyModifier = nNew;
		lbChanged = true;
	}

	return lbChanged;
}

// �������� � mn_HostModOk ������ 3 VK
void Settings::TrimHostkeys()
{
	if (gpSet->mn_HostModOk[0] == 0)
		return;

	int i = 0;

	while (++i < 15 && gpSet->mn_HostModOk[i])
		;

	// ���� ����� ������ ����� 3-� ������������� - ��������, ������� 3 ���������
	if (i > 3)
	{
		if (i >= (int)countof(gpSet->mn_HostModOk))
		{
			_ASSERTE(i < countof(gpSet->mn_HostModOk));
			i = countof(gpSet->mn_HostModOk) - 1;
		}
		memmove(gpSet->mn_HostModOk, gpSet->mn_HostModOk+i-3, 3); //-V112
	}

	memset(gpSet->mn_HostModOk+3, 0, sizeof(gpSet->mn_HostModOk)-3);
}

bool Settings::isHostkeySingleLR(WORD vk, WORD vkC, WORD vkL, WORD vkR)
{
	if (vk == vkC)
	{
		bool bLeft  = isPressed(vkL);
		bool bRight = isPressed(vkR);

		if (bLeft && !bRight)
			return (nMultiHotkeyModifier == vkL);

		if (bRight && !bLeft)
			return (nMultiHotkeyModifier == vkR);

		// ������� ����� ������ - ����������
		return false;
	}

	if (vk == vkL)
		return (nMultiHotkeyModifier == vkL);

	if (vk == vkR)
		return (nMultiHotkeyModifier == vkR);

	return false;
}

bool Settings::IsHostkeySingle(WORD vk)
{
	if (nMultiHotkeyModifier > 0xFF)
		return false; // � Host-���������� ������ ����� �������

	if (vk == VK_LWIN || vk == VK_RWIN)
		return (nMultiHotkeyModifier == VK_LWIN);

	if (vk == VK_APPS)
		return (nMultiHotkeyModifier == VK_APPS);

	if (vk == VK_SHIFT || vk == VK_LSHIFT || vk == VK_RSHIFT)
		return isHostkeySingleLR(vk, VK_SHIFT, VK_LSHIFT, VK_RSHIFT);

	if (vk == VK_CONTROL || vk == VK_LCONTROL || vk == VK_RCONTROL)
		return isHostkeySingleLR(vk, VK_CONTROL, VK_LCONTROL, VK_RCONTROL);

	if (vk == VK_MENU || vk == VK_LMENU || vk == VK_RMENU)
		return isHostkeySingleLR(vk, VK_MENU, VK_LMENU, VK_RMENU);

	return false;
}

// ����� ������ MOD_xxx ��� RegisterHotKey
UINT Settings::GetHostKeyMod()
{
	UINT nMOD = 0;

	for(int i=0; i < 15 && mn_HostModOk[i]; i++)
	{
		switch(mn_HostModOk[i])
		{
			case VK_LWIN: case VK_RWIN:
				nMOD |= MOD_WIN;
				break;
			case VK_CONTROL: case VK_LCONTROL: case VK_RCONTROL:
				nMOD |= MOD_CONTROL;
				break;
			case VK_SHIFT: case VK_LSHIFT: case VK_RSHIFT:
				nMOD |= MOD_SHIFT;
				break;
			case VK_MENU: case VK_LMENU: case VK_RMENU:
				nMOD |= MOD_ALT;
				break;
		}
	}

	if (!nMOD)
		nMOD = MOD_WIN;

	return nMOD;
}

WORD Settings::GetPressedHostkey()
{
	_ASSERTE(mn_HostModOk[0]!=0);

	if (mn_HostModOk[0] == VK_LWIN)
	{
		if (isPressed(VK_LWIN))
			return VK_LWIN;

		if (isPressed(VK_RWIN))
			return VK_RWIN;
	}

	if (!isPressed(mn_HostModOk[0]))
	{
		_ASSERT(FALSE);
		return 0;
	}

	// ��� ������-����� - ���������� �����, �.�. ������ �� �������� � WM_KEYUP
	if (mn_HostModOk[0] == VK_LSHIFT || mn_HostModOk[0] == VK_RSHIFT)
		return VK_SHIFT;

	if (mn_HostModOk[0] == VK_LMENU || mn_HostModOk[0] == VK_RMENU)
		return VK_MENU;

	if (mn_HostModOk[0] == VK_LCONTROL || mn_HostModOk[0] == VK_RCONTROL)
		return VK_CONTROL;

	return mn_HostModOk[0];
}

bool Settings::IsHostkeyPressed()
{
	if (mn_HostModOk[0] == 0)
	{
		_ASSERTE(mn_HostModOk[0]!=0);
		mn_HostModOk[0] = VK_LWIN;
		return isPressed(VK_LWIN) || isPressed(VK_RWIN);
	}

	// �� ����� 3-� ������������� + ������
	_ASSERTE(mn_HostModOk[4] == 0);
	for(int i = 0; i < 4 && mn_HostModOk[i]; i++) //-V112
	{
		if (mn_HostModOk[i] == VK_LWIN)
		{
			if (!(isPressed(VK_LWIN) || isPressed(VK_RWIN)))
				return false;
		}
		else if (!isPressed(mn_HostModOk[i]))
		{
			return false;
		}
	}

	// �� ����� 3-� ������������� + ������
	for(int j = 0; j < 4 && mn_HostModSkip[j]; j++) //-V112
	{
		if (isPressed(mn_HostModSkip[j]))
			return false;
	}

	return true;
}

LPCTSTR Settings::GetCmd()
{
	if (psCurCmd && *psCurCmd)
		return psCurCmd;

	if (psCmd && *psCmd)
		return psCmd;

	SafeFree(psCurCmd); // ���������, ��� ������ ������ ����� �� �����, �� �� ������ ������...
	// ������ �� ����� ��������� ���������� ������ ����, �� ��� �� ������ ������
	// �������� x64 ���� ������ ���������� � x86 ConEmu.
	DWORD nFarSize = 0;

	if (lstrcmpi(gpSetCls->GetDefaultCmd(), L"far") == 0)
	{
		// ���� ���. (1) � ����� ConEmu, (2) � ������� ����������, (2) �� ������� ����� �� ����� ConEmu
		wchar_t szFar[MAX_PATH*2], *pszSlash;
		szFar[0] = L'"';
		wcscpy_add(1, szFar, gpConEmu->ms_ConEmuExeDir); // ������ szFar �������� ���� ������� ���������
		pszSlash = szFar + _tcslen(szFar);
		_ASSERTE(pszSlash > szFar);
		BOOL lbFound = FALSE;

		// (1) � ����� ConEmu
		if (!lbFound)
		{
			wcscpy_add(pszSlash, szFar, L"\\Far.exe");

			if (FileExists(szFar+1, &nFarSize))
				lbFound = TRUE;
		}

		// (2) � ������� ����������
		if (!lbFound && lstrcmpi(gpConEmu->ms_ConEmuCurDir, gpConEmu->ms_ConEmuExeDir))
		{
			szFar[0] = L'"';
			wcscpy_add(1, szFar, gpConEmu->ms_ConEmuCurDir);
			wcscat_add(1, szFar, L"\\Far.exe");

			if (FileExists(szFar+1, &nFarSize))
				lbFound = TRUE;
		}

		// (3) �� ������� �����
		if (!lbFound)
		{
			szFar[0] = L'"';
			wcscpy_add(1, szFar, gpConEmu->ms_ConEmuExeDir);
			pszSlash = szFar + _tcslen(szFar);
			*pszSlash = 0;
			pszSlash = wcsrchr(szFar, L'\\');

			if (pszSlash)
			{
				wcscpy_add(pszSlash+1, szFar, L"Far.exe");

				if (FileExists(szFar+1, &nFarSize))
					lbFound = TRUE;
			}
		}

		if (lbFound)
		{
			// 110124 - �����, ���� ������������ ���� - ��� ��� �������� ��������, ��� ������
			//// far ���� ����� ����� ���������� � "Program Files", ������� ��� ��������� ������� - �����������
			//// ���� ���� - ���� far.exe > 1200K - �������, ��� ��� Far2
			//wcscat_c(szFar, (nFarSize>1228800) ? L"\" /w" : L"\"");
			wcscat_c(szFar, L"\"");
		}
		else
		{
			// ���� Far.exe �� ������ ����� � ConEmu - ��������� cmd.exe
			wcscpy_c(szFar, L"cmd");
		}

		// Finally - Result
		psCurCmd = lstrdup(szFar);
	}
	else
	{
		// Simple Copy
		psCurCmd = lstrdup(gpSetCls->GetDefaultCmd());
	}

	return psCurCmd;
}

void Settings::HistoryCheck()
{
	if (!psCmdHistory || !*psCmdHistory)
	{
		nCmdHistorySize = 0;
	}
	else
	{
		const wchar_t* psz = psCmdHistory;

		while(*psz)
			psz += _tcslen(psz)+1;

		if (psz == psCmdHistory)
			nCmdHistorySize = 0;
		else
			nCmdHistorySize = (psz - psCmdHistory + 1)*sizeof(wchar_t);
	}
}

void Settings::HistoryAdd(LPCWSTR asCmd)
{
	if (!asCmd || !*asCmd)
		return;

	if (psCmd && lstrcmp(psCmd, asCmd)==0)
		return;

	if (psCurCmd && lstrcmp(psCurCmd, asCmd)==0)
		return;

	HEAPVAL
	wchar_t *pszNewHistory, *psz;
	int nCount = 0;
	DWORD nCchNewSize = (nCmdHistorySize>>1) + _tcslen(asCmd) + 2;
	DWORD nNewSize = nCchNewSize*2;
	pszNewHistory = (wchar_t*)malloc(nNewSize);

	//wchar_t* pszEnd = pszNewHistory + nNewSize/sizeof(wchar_t);
	if (!pszNewHistory) return;

	_wcscpy_c(pszNewHistory, nCchNewSize, asCmd);
	psz = pszNewHistory + _tcslen(pszNewHistory) + 1;
	nCount++;

	if (psCmdHistory)
	{
		wchar_t* pszOld = psCmdHistory;
		int nLen;
		HEAPVAL;

		while(nCount < MAX_CMD_HISTORY && *pszOld /*&& psz < pszEnd*/)
		{
			const wchar_t *pszCur = pszOld;
			pszOld += _tcslen(pszOld) + 1;

			if (lstrcmp(pszCur, asCmd) == 0)
				continue;

			_wcscpy_c(psz, nCchNewSize-(psz-pszNewHistory), pszCur);
			psz += (nLen = (_tcslen(psz)+1));
			nCount ++;
		}
	}

	*psz = 0;
	HEAPVAL;
	free(psCmdHistory);
	psCmdHistory = pszNewHistory;
	nCmdHistorySize = (psz - pszNewHistory + 1)*sizeof(wchar_t);
	HEAPVAL;
	// � ����� ��������� � ����������
	SettingsBase* reg = CreateSettings();

	if (reg->OpenKey(gpSetCls->GetConfigPath(), KEY_WRITE))
	{
		HEAPVAL;
		reg->SaveMSZ(L"CmdLineHistory", psCmdHistory, nCmdHistorySize);
		HEAPVAL;
		reg->CloseKey();
	}

	delete reg;
}

LPCWSTR Settings::HistoryGet()
{
	if (psCmdHistory && *psCmdHistory)
		return psCmdHistory;

	return NULL;
}

// ��������, L"2013-25C3,25C4"
// ���������� 0 - � ������ ������,
// ��� ������ - ������ (1-based) ���������� ������� � asRanges
// -1 - ������ ��������� ������
int Settings::ParseCharRanges(LPCWSTR asRanges, BYTE (&Chars)[0x10000], BYTE abValue /*= TRUE*/)
{
	if (!asRanges)
	{
		_ASSERTE(asRanges!=NULL);
		return -1;
	}
	
	int iRc = 0;
	int n = 0, nMax = _tcslen(asRanges);
	wchar_t *pszCopy = lstrdup(asRanges);
	if (!pszCopy)
	{
		_ASSERTE(pszCopy!=NULL);
		return -1;
	}
	wchar_t *pszRange = pszCopy;
	wchar_t *pszNext = NULL;
	UINT cBegin, cEnd;
	
	memset(Chars, 0, sizeof(Chars));

	while(*pszRange && n < nMax)
	{
		cBegin = (UINT)wcstol(pszRange, &pszNext, 16);
		if (!cBegin || (cBegin > 0xFFFF))
		{
			iRc = (int)(pszRange - asRanges);
			goto wrap;
		}
			
		switch (*pszNext)
		{
		case L';':
		case 0:
			cEnd = cBegin;
			break;
		case L'-':
		case L' ':
			pszRange = pszNext + 1;
			cEnd = (UINT)wcstol(pszRange, &pszNext, 16);
			if ((cEnd < cBegin) || (cEnd > 0xFFFF))
			{
				iRc = (int)(pszRange - asRanges);
				goto wrap;
			}
			break;
		default:
			iRc = (int)(pszNext - asRanges);
			goto wrap;
		}

		for (UINT i = cBegin; i <= cEnd; i++)
			Chars[i] = abValue;
		
		if (*pszNext != L';') break;
		pszRange = pszNext + 1;
	}
	
	iRc = 0; // ok
wrap:
	if (pszCopy)
		free(pszCopy);
	return iRc;
}

// caller must free(result)
wchar_t* Settings::CreateCharRanges(BYTE (&Chars)[0x10000])
{
	size_t nMax = 1024;
	wchar_t* pszRanges = (wchar_t*)calloc(nMax,sizeof(*pszRanges));
	if (!pszRanges)
	{
		_ASSERTE(pszRanges!=NULL);
		return NULL;
	}
	
	wchar_t* psz = pszRanges;
	wchar_t* pszEnd = pszRanges + nMax;
	UINT c = 0;
	_ASSERTE((countof(Chars)-1) == 0xFFFF);
	while (c < countof(Chars))
	{
		if (Chars[c])
		{
			if ((psz + 10) >= pszEnd)
			{
				// ������� ������� ����
				_ASSERTE((psz + 10) < pszEnd);
				break;
			}
			
			UINT cBegin = (c++);
			UINT cEnd = cBegin;
			
			while (c < countof(Chars) && Chars[c])
			{
				cEnd = (c++);
			}
			
			if (cBegin == cEnd)
			{
				wsprintf(psz, L"%04X;", cBegin);
			}
			else
			{
				wsprintf(psz, L"%04X-%04X;", cBegin, cEnd);
			}
			psz += _tcslen(psz);
		}
		else
		{
			c++;
		}
	}
	
	return pszRanges;
}

bool Settings::isCharBorder(wchar_t inChar)
{
	return mpc_FixFarBorderValues[(WORD)inChar];
}

//BOOL Settings::CheckConIme()
//{
//	if (!(gOSVer.dwMajorVersion == 6 && gOSVer.dwMinorVersion == 0))
//		return FALSE; // ��������� ������ � Vista
//
//	long  lbStopWarning = FALSE;
//	DWORD dwValue=1;
//	SettingsBase* reg = CreateSettings();
//
//	// ��� ����� ������������!
//	if (reg->OpenKey(CONEMU_ROOT_KEY, KEY_READ))
//	{
//		if (!reg->Load(_T("StopWarningConIme"), lbStopWarning))
//			lbStopWarning = FALSE;
//
//		reg->CloseKey();
//	}
//
//	if (!lbStopWarning)
//	{
//		HKEY hk = NULL;
//
//		if (0 == RegOpenKeyEx(HKEY_CURRENT_USER, L"Console", 0, KEY_READ, &hk))
//		{
//			DWORD dwType = REG_DWORD, nSize = sizeof(DWORD);
//
//			if (0 != RegQueryValueEx(hk, L"LoadConIme", 0, &dwType, (LPBYTE)&dwValue, &nSize))
//				dwValue = 1;
//
//			RegCloseKey(hk);
//
//			if (dwValue!=0)
//			{
//				if (IDCANCEL==MessageBox(0,
//				                        L"Unwanted value of 'LoadConIme' registry parameter!\r\n"
//				                        L"Press 'Cancel' to stop this message.\r\n"
//				                        L"Take a look at 'FAQ-ConEmu.txt'.\r\n"
//				                        L"You may simply import file 'Disable_ConIme.reg'\r\n"
//				                        L"located in 'ConEmu.Addons' folder.",
//				                        gpConEmu->GetDefaultTitle(),MB_OKCANCEL|MB_ICONEXCLAMATION))
//					lbStopWarning = TRUE;
//			}
//		}
//		else
//		{
//			if (IDCANCEL==MessageBox(0,
//			                        L"Can't determine a value of 'LoadConIme' registry parameter!\r\n"
//			                        L"Press 'Cancel' to stop this message.\r\n"
//			                        L"Take a look at 'FAQ-ConEmu.txt'",
//			                        gpConEmu->GetDefaultTitle(),MB_OKCANCEL|MB_ICONEXCLAMATION))
//				lbStopWarning = TRUE;
//		}
//
//		if (lbStopWarning)
//		{
//			// ��� ����� ������������!
//			if (reg->OpenKey(CONEMU_ROOT_KEY, KEY_WRITE))
//			{
//				reg->Save(_T("StopWarningConIme"), lbStopWarning);
//				reg->CloseKey();
//			}
//		}
//	}
//
//	delete reg;
//	return TRUE;
//}

void Settings::CheckConsoleSettings()
{
	// ���� � ����� [HKEY_CURRENT_USER\Console] ����� ����� �������� - �� � Win7 �����
	// �������� �������� ����� :-)
	// ��������, ���������� ���� ����� "�������" - ����� ����, � ����������� - ��� :-P
	DWORD nFullScr = 0, nFullScrEmu = 0, nSize;
	HKEY hkCon, hkEmu;
	if (!RegOpenKeyEx(HKEY_CURRENT_USER, L"Console", 0, KEY_ALL_ACCESS, &hkCon))
	{
		if (RegQueryValueEx(hkCon, L"FullScreen", NULL, NULL, (LPBYTE)&nFullScr, &(nSize=sizeof(nFullScr))))
			nFullScr = 0;
		if (!RegOpenKeyEx(hkCon, CEC_INITTITLE, 0, KEY_ALL_ACCESS, &hkEmu))
		{
			if (RegQueryValueEx(hkEmu, L"FullScreen", NULL, NULL, (LPBYTE)&nFullScrEmu, &(nSize=sizeof(nFullScrEmu))))
				nFullScrEmu = 0;
			RegCloseKey(hkEmu);
		}
		else
		{
			nFullScrEmu = nFullScr;
		}
		RegCloseKey(hkCon);
	}
	
	if (nFullScr || nFullScrEmu)
	{
		if (gOSVer.dwMajorVersion >= 6)
		{
			wchar_t szWarning[512];
			_wsprintf(szWarning, SKIPLEN(countof(szWarning)) 
				L"Warning!\n"
				L"Dangerous values detected in yours registry\n\n"
				L"Please check [HKEY_CURRENT_USER\\Console] and [HKEY_CURRENT_USER\\Console\\ConEmu] keys\n\n"
				L"\"FullScreen\" recommended value is dword:00000000\n\n"
				L"Current value is dword:%08X",
				nFullScr ? nFullScr : nFullScrEmu);
			MBoxA(szWarning);
		}
	}
}

SettingsBase* Settings::CreateSettings()
{
#ifndef __GNUC__
	SettingsBase* pReg = NULL;
	BOOL lbXml = FALSE;
	DWORD dwAttr = -1;
	LPWSTR pszXmlFile = gpConEmu->ConEmuXml();

	if (pszXmlFile && *pszXmlFile)
	{
		dwAttr = GetFileAttributes(pszXmlFile);

		if (dwAttr != (DWORD)-1 && !(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
			lbXml = TRUE;
	}

	if (lbXml)
	{
		pReg = new SettingsXML();

		if (!((SettingsXML*)pReg)->IsXmlAllowed())
		{
			// ���� MSXml.DomDocument �� ���������������
			pszXmlFile[0] = 0;
			lbXml = FALSE;
		}
	}

	if (!lbXml)
		pReg = new SettingsRegistry();

	return pReg;
#else
	return new SettingsRegistry();
#endif
}

void Settings::GetSettingsType(wchar_t (&szType)[8], bool& ReadOnly)
{
	const wchar_t* pszType = L"[reg]";
	ReadOnly = false;

#ifndef __GNUC__
	HANDLE hFile = NULL;
	LPWSTR pszXmlFile = gpConEmu->ConEmuXml();

	if (pszXmlFile && *pszXmlFile)
	{
		hFile = CreateFile(pszXmlFile, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
		                   NULL, OPEN_EXISTING, 0, 0);

		// XML-���� ����
		if (hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile); hFile = NULL;
			pszType = L"[xml]";
			// ��������, ������ �� �� � ���� ��������
			hFile = CreateFile(pszXmlFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,
			                   NULL, OPEN_EXISTING, 0, 0);

			if (hFile != INVALID_HANDLE_VALUE)
			{
				CloseHandle(hFile); hFile = NULL; // OK
			}
			else
			{
				//EnableWindow(GetDlgItem(ghOpWnd, bSaveSettings), FALSE); // ���������� ���������
				ReadOnly = true;
			}
		}
	}
#endif

	wcscpy_c(szType, pszType);
}

bool Settings::isHideCaptionAlways()
{
	return mb_HideCaptionAlways || (!mb_HideCaptionAlways && isUserScreenTransparent);
}

COLORREF* Settings::GetColors(BOOL abFade)
{
	if (!abFade || !isFadeInactive)
		return Colors;

	if (!mb_FadeInitialized)
	{
		mn_LastFadeSrc = mn_LastFadeDst = -1;
		if (((int)mn_FadeHigh - (int)mn_FadeLow) < 64)
		{
			mn_FadeLow = DEFAULT_FADE_LOW; mn_FadeHigh = DEFAULT_FADE_HIGH;
		}

		mn_FadeMul = mn_FadeHigh - mn_FadeLow;
		mb_FadeInitialized = true;

		for (size_t i=0; i<countof(ColorsFade); i++)
		{
			ColorsFade[i] = GetFadeColor(Colors[i]);
		}
	}

	return ColorsFade;
}

COLORREF Settings::GetFadeColor(COLORREF cr)
{
	if (!isFadeInactive)
		return cr;
		
	if (cr == mn_LastFadeSrc)
		return mn_LastFadeDst;

	MYCOLORREF mcr, mcrFade = {}; mcr.color = cr;

	if (!mb_FadeInitialized)
	{
		GetColors(TRUE);
	}

	mcrFade.rgbRed = GetFadeColorItem(mcr.rgbRed);
	mcrFade.rgbGreen = GetFadeColorItem(mcr.rgbGreen);
	mcrFade.rgbBlue = GetFadeColorItem(mcr.rgbBlue);

	mn_LastFadeSrc = cr;
	mn_LastFadeDst = mcrFade.color;
	
	return mcrFade.color;
}

BYTE Settings::GetFadeColorItem(BYTE c)
{
	DWORD nRc;

	switch(c)
	{
		case 0:
			return mn_FadeLow;
		case 255:
			return mn_FadeHigh;
		default:
			nRc = ((((DWORD)c) + mn_FadeLow) * mn_FadeHigh) >> 8;

			if (nRc >= 255)
			{
				//_ASSERTE(nRc <= 255); -- ����� (mn_FadeLow&mn_FadeHigh) ������������ � ��������� ��� ������
				return 255;
			}

			return (BYTE)nRc;
	}
}

bool Settings::NeedDialogDetect()
{
	// 100331 ������ ��� ����� � ��� PanelView
	return true;
	//return (isUserScreenTransparent || !isMonospace);
}

bool Settings::isModifierPressed(DWORD vk)
{
	// ���� �� 0 - ������ ���� �����
	if (vk)
	{
		if (!isPressed(vk))
			return false;
	}

	// �� ������ ������������ ������ ���� �� ������!
	if (vk != VK_SHIFT && vk != VK_LSHIFT && vk != VK_RSHIFT)
	{
		if (isPressed(VK_SHIFT))
			return false;
	}

	if (vk != VK_MENU && vk != VK_LMENU && vk != VK_RMENU)
	{
		if (isPressed(VK_MENU))
			return false;
	}

	if (vk != VK_CONTROL && vk != VK_LCONTROL && vk != VK_RCONTROL)
	{
		if (isPressed(VK_CONTROL))
			return false;
	}

	// �����
	return true;
}

bool Settings::NeedCreateAppWindow()
{
	// ���� ���, ���� ��� Application ����� ��������� ������ ��� XP � ����
	// � ��� ������, ���� �� �������� ������������ ������ ���������� ��������
	// ��� ��� ����, ����� ��� Alt-Tab �� ��������� "������" ������ �������� ����
	if (!IsWindows7 && isTabsOnTaskBar())
		return TRUE;
	return FALSE;
}

bool Settings::isTabsOnTaskBar()
{
	if (isDesktopMode)
		return false;
	if ((m_isTabsOnTaskBar == 1) || (((BYTE)m_isTabsOnTaskBar > 1) && IsWindows7))
		return true;
	return false;
}

bool Settings::isRClickTouchInvert()
{
	return gpConEmu->IsGesturesEnabled() && (isRClickSendKey == 2);
}

LPCWSTR Settings::RClickMacro()
{
	if (sRClickMacro && *sRClickMacro)
		return sRClickMacro;
	return RClickMacroDefault();
}

LPCWSTR Settings::RClickMacroDefault()
{
	// L"@$If (!CmdLine.Empty) %Flg_Cmd=1; %CmdCurPos=CmdLine.ItemCount-CmdLine.CurPos+1; %CmdVal=CmdLine.Value; Esc $Else %Flg_Cmd=0; $End $Text \"rclk_gui:\" Enter $If (%Flg_Cmd==1) $Text %CmdVal %Flg_Cmd=0; %Num=%CmdCurPos; $While (%Num!=0) %Num=%Num-1; CtrlS $End $End"
	static LPCWSTR pszDefaultMacro = FarRClickMacroDefault;
	return pszDefaultMacro;
}

LPCWSTR Settings::SafeFarCloseMacro()
{
	if (sSafeFarCloseMacro && *sSafeFarCloseMacro)
		return sSafeFarCloseMacro;
	return SafeFarCloseMacroDefault();
}

LPCWSTR Settings::SafeFarCloseMacroDefault()
{
	// L"@$while (Dialog||Editor||Viewer||Menu||Disks||MainMenu||UserMenu||Other||Help) $if (Editor) ShiftF10 $else Esc $end $end  Esc  $if (Shell) F10 $if (Dialog) Enter $end $Exit $end  F10"
	static LPCWSTR pszDefaultMacro = FarSafeCloseMacroDefault;
	return pszDefaultMacro;
}

LPCWSTR Settings::TabCloseMacro()
{
	if (sTabCloseMacro && *sTabCloseMacro)
		return sTabCloseMacro;
	return TabCloseMacroDefault();
}

LPCWSTR Settings::TabCloseMacroDefault()
{
	// L"@$if (Shell) F10 $if (Dialog) Enter $end $else F10 $end";
	static LPCWSTR pszDefaultMacro = FarTabCloseMacroDefault;
	return pszDefaultMacro;
}

LPCWSTR Settings::SaveAllMacro()
{
	if (sSaveAllMacro && *sSaveAllMacro)
		return sSaveAllMacro;
	return SaveAllMacroDefault();
}

LPCWSTR Settings::SaveAllMacroDefault()
{
	// L"@F2 $If (!Editor) $Exit $End %i0=-1; F12 %cur = CurPos; Home Down %s = Menu.Select(\" * \",3,2); $While (%s > 0) $If (%s == %i0) MsgBox(\"FAR SaveAll\",\"Asterisk in menuitem for already processed window\",0x10001) $Exit $End Enter $If (Editor) F2 $If (!Editor) $Exit $End $Else $If (!Viewer) $Exit $End $End %i0 = %s; F12 %s = Menu.Select(\" * \",3,2); $End $If (Menu && Title==\"Screens\") Home $Rep (%cur-1) Down $End Enter $End $Exit"
	static LPCWSTR pszDefaultMacro = FarSaveAllMacroDefault;
	return pszDefaultMacro;
}
