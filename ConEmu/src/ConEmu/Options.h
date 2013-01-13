
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

template <class T>
T GetMinMax(T a, int v1, int v2)
{
	if (a < (T)v1)
		a = (T)v1;
	else if (a > (T)v2)
		a = (T)v2;
	return a;
}

template <class T>
void MinMax(T &a, int v1, int v2)
{
	if (a < (T)v1)
		a = (T)v1;
	else if (a > (T)v2)
		a = (T)v2;
}

template <class T>
void MinMax(T &a, int v2)
{
	if (a > (T)v2)
		a = (T)v2;
}

#define MIN_ALPHA_VALUE 40
#define MIN_INACTIVE_ALPHA_VALUE 0
#define MAX_FONT_STYLES 8  //normal/(bold|italic|underline)
#define MAX_FONT_GROUPS 20 // Main, Borders, Japan, Cyrillic, ...

#define LONGOUTPUTHEIGHT_MIN 300
#define LONGOUTPUTHEIGHT_MAX 9999

#define CURSORSIZE_MIN 5
#define CURSORSIZE_MAX 100
#define CURSORSIZEPIX_MIN 1
#define CURSORSIZEPIX_MAX 99

#define QUAKEANIMATION_DEF 300
#define QUAKEANIMATION_MAX 2000

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

enum FarMacroVersion
{
	fmv_Default = 0,
	fmv_Standard,
	fmv_Lua,
};

// -> Header.h
//enum BackgroundOp
//{
//	eUpLeft = 0,
//	eStretch = 1,
//	eTile = 2,
//	eUpRight = 3,
//	eDownLeft = 4,
//	eDownRight = 5,
//};

#define BgImageColorsDefaults (1|2)

#include "UpdateSet.h"

class CSettings;
class CVirtualConsole;

#define CmdFilePrefix     L'@'
#define DropLnkPrefix     L'?'
#define TaskBracketLeft   L'{'
#define TaskBracketRight  L'}'
#define AutoStartTaskName L"<Startup>"

#define SCROLLBAR_DELAY_MIN 100
#define SCROLLBAR_DELAY_MAX 15000

#define CENTERCONSOLEPAD_MIN 0
#define CENTERCONSOLEPAD_MAX 64

#define CEHOTKEY_MODMASK    0xFFFFFF00
#define CEHOTKEY_NUMHOSTKEY 0xFFFFFF00
#define CEHOTKEY_ARRHOSTKEY 0xFEFEFE00
#define CEHOTKEY_NOMOD      0x80808000

enum ConEmuHotKeyType
{
	chk_User = 0,  // ������� ������������� hotkey
	chk_Modifier,  // ��� �����, ��������
	chk_Modifier2, // ��� �����, �������� (����� ����� ������ ����� ������ ������������)
	chk_NumHost,   // system hotkey (<HostKey>-Number, � ��� ������ <HostKey>-Arrows)
	chk_ArrHost,   // system hotkey (<HostKey>-Number, � ��� ������ <HostKey>-Arrows)
	chk_System,    // predefined hotkeys, ��������������� (����?)
	chk_Global,    // globally registered hotkey
	chk_Macro,     // GUI Macro
};

// ����� � common.hpp
//enum ConEmuModifiers
//{
//	// ��� ��������, � ������� ����� VkMod �������� VK ������
//	cvk_VK_MASK  = 0x000000FF,
//
//	// ������������, ������� ���� ������ ���������, ������ ��� �����
//	cvk_LCtrl    = 0x00000100,
//	cvk_RCtrl    = 0x00000200,
//	cvk_LAlt     = 0x00000400,
//	cvk_RAlt     = 0x00000800,
//	cvk_LShift   = 0x00001000,
//	cvk_RShift   = 0x00002000,
//
//	// ���� ��� �������, ������ ��� �����
//	cvk_Ctrl     = 0x00010000,
//	cvk_Alt      = 0x00020000,
//	cvk_Shift    = 0x00040000,
//	cvk_Win      = 0x00080000,
//	cvk_Apps     = 0x00100000,
//
//	// ����� ���� � ������ ������/�����
//	cvk_DISTINCT = cvk_LCtrl|cvk_RCtrl|cvk_LAlt|cvk_RAlt|cvk_LShift|cvk_RShift|cvk_Win|cvk_Apps,
//	// ����� ������ ���� ���������� ������, �� ����������� ������ VK
//	cvk_ALLMASK  = 0xFFFFFF00,
//};

struct ConEmuHotKey
{
	int DescrLangID;
	
	ConEmuHotKeyType HkType; // 0 - hotkey, 1 - modifier (��� �����, ��������), 2 - system hotkey (��������� nMultiHotkeyModifier)

	bool   (*Enabled)();

	wchar_t Name[64];
	
	//// User. ���� NULL - ������ ���������, �� ����������
	//union
	//{
	//	DWORD* VkModPtr; // (HkType==chk_User, chk_Hostkey, chk_System, chk_Global)
	//	BYTE*  ModPtr;   // (HkType==chk_Modifier)
	//};
	
	DWORD VkMod;

    bool (WINAPI *fkey)(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon); // true-����������, false-���������� � �������
	bool OnKeyUp; // ��������� ���������� ����� ������������ "�� ����������" (����� ��������, ����, ...)

	wchar_t* GuiMacro;

	// Internal
	size_t cchGuiMacroMax;
	bool   NotChanged;

	bool CanChangeVK() const
	{
		//chk_System - ���� �� �������������
		if (HkType==chk_User || HkType==chk_Global || HkType==chk_Macro)
		{
			return true;
		}
		return false;
	};

	LPCWSTR GetDescription(wchar_t* pszDescr, int cchMaxLen, bool bAddMacroIndex = false) const
	{
		if (!pszDescr)
			return L"";

		LPCWSTR pszRc = pszDescr;
		bool lbColon = false;

		*pszDescr = 0;

		if (bAddMacroIndex && (HkType == chk_Macro))
		{
			_wsprintf(pszDescr, SKIPLEN(cchMaxLen) L"Macro %02i: ", DescrLangID-vkGuMacro01+1);
			int nLen = lstrlen(pszDescr);
			pszDescr += nLen;
			cchMaxLen -= nLen;
			lbColon = true;
		}

		if ((HkType != chk_Macro) && !LoadString(g_hInstance, DescrLangID, pszDescr, cchMaxLen))
		{
			if ((HkType == chk_User) && GuiMacro && *GuiMacro)
				lstrcpyn(pszDescr, GuiMacro, cchMaxLen);
			else
				_wsprintf(pszDescr, SKIPLEN(cchMaxLen) L"#%i", DescrLangID);
		}
		else if ((cchMaxLen >= 16) && GuiMacro && *GuiMacro)
		{
			size_t nLen = _tcslen(pszDescr);
			pszDescr += nLen;
			cchMaxLen -= nLen;

			if (!lbColon && (cchMaxLen > 2) && (pszDescr > pszRc))
			{
				lstrcpyn(pszDescr, L": ", cchMaxLen);
				pszDescr += 2;
				cchMaxLen -= 2;
			}
			lstrcpyn(pszDescr, GuiMacro, cchMaxLen);
		}

		return pszRc;
	};

	void Free()
	{
		SafeFree(GuiMacro);
	};
};

// ��������� ���������� ����� ������������ "�� ����������" �� ��������� ������ � �����������
extern const ConEmuHotKey* ConEmuSkipHotKey; // = ((ConEmuHotKey*)INVALID_HANDLE_VALUE)


struct FindTextOptions
{
	size_t   cchTextMax;
	wchar_t* pszText;
	bool     bMatchCase;
	bool     bMatchWholeWords;
	bool     bFreezeConsole;
	bool     bHighlightAll;
	bool     bTransparent;
};


enum CECursorStyle
{
	cur_Horz         = 0x00,
	cur_Vert         = 0x01,
	cur_Block        = 0x02,
	cur_Rect         = 0x03,

	// Used for Min/Max
	cur_First        = cur_Horz,
	cur_Last         = cur_Rect,
};

union CECursorType
{
	struct
	{
		CECursorStyle  CursorType   : 6;
		unsigned int   isBlinking   : 1;
		unsigned int   isColor      : 1;
		unsigned int   isFixedSize  : 1;
		unsigned int   FixedSize    : 7;
		unsigned int   MinSize      : 7;
		// set to true for use distinct settings for Inactive cursor
		unsigned int   Used         : 1;
	};

	DWORD Raw;
};

enum TabStyle
{
	ts_VS2008 = 0,
	ts_Win8   = 1,
};

struct Settings
{
	public:
		Settings();
		~Settings();
	protected:
		friend class CSettings;
	
		void ReleasePointers();		
	public:

		wchar_t Type[16]; // �������������: L"[reg]" ��� L"[xml]"

		bool IsConfigNew; // true, ���� ������������ �����

		//reg->Load(L"DefaultBufferHeight", DefaultBufferHeight);
		int DefaultBufferHeight;
		
		//bool AutoScroll;
		
		//reg->Load(L"AutoBufferHeight", AutoBufferHeight);
		bool AutoBufferHeight; // Long console output
		//reg->Load(L"CmdOutputCP", nCmdOutputCP);
		int nCmdOutputCP;

		ConEmuComspec ComSpec;

		// Replace default terminal
		bool isSetDefaultTerminal;
		bool isRegisterOnOsStartup;
		bool isDefaultTerminalNoInjects;
		BYTE nDefaultTerminalConfirmClose; // "Press Enter to close console". 0 - Auto, 1 - Always, 2 - Never
		wchar_t* GetDefaultTerminalApps(); // "|" delimited
		const wchar_t* GetDefaultTerminalAppsMSZ(); // "\0" delimited
		void SetDefaultTerminalApps(const wchar_t* apszApps); // "|" delimited
	private:
		wchar_t* psDefaultTerminalApps; // MSZ

	public:
		struct ColorPalette
		{
			wchar_t* pszName;
			bool bPredefined;

			//reg->Load(L"ExtendColors", isExtendColors);
			bool isExtendColors;
			//reg->Load(L"ExtendColorIdx", nExtendColorIdx);
			BYTE nExtendColorIdx; // 0..15

			//reg->Load(L"TextColorIdx", nTextColorIdx);
			BYTE nTextColorIdx; // 0..15,16
			//reg->Load(L"BackColorIdx", nBackColorIdx);
			BYTE nBackColorIdx; // 0..15,16
			//reg->Load(L"PopTextColorIdx", nPopTextColorIdx);
			BYTE nPopTextColorIdx; // 0..15,16
			//reg->Load(L"PopBackColorIdx", nPopBackColorIdx);
			BYTE nPopBackColorIdx; // 0..15,16


			COLORREF Colors[0x20];

			void FreePtr()
			{
				SafeFree(pszName);
                ColorPalette* p = this;
                SafeFree(p);
			};
		};

		struct AppSettings
		{
			size_t   cchNameMax;
			wchar_t* AppNames; // "far.exe|far64.exe" � �.�.
			wchar_t* AppNamesLwr; // For internal use
			BYTE Elevated; // 00 - unimportant, 01 - elevated, 02 - nonelevated
			
			//const COLORREF* Palette/*[0x20]*/; // ������� ������� (Fade/�� Fade)

			bool OverridePalette; // Palette+Extend
			wchar_t szPaletteName[128];
			//reg->Load(L"ExtendColors", isExtendColors);
			bool isExtendColors;
			char ExtendColors() const { return (OverridePalette || !AppNames) ? isExtendColors : gpSet->AppStd.isExtendColors; };
			//reg->Load(L"ExtendColorIdx", nExtendColorIdx);
			BYTE nExtendColorIdx; // 0..15
			BYTE ExtendColorIdx() const { return (OverridePalette || !AppNames) ? nExtendColorIdx : gpSet->AppStd.nExtendColorIdx; };

			BYTE nTextColorIdx; // 0..15,16
			BYTE TextColorIdx() const { return (OverridePalette || !AppNames) ? nTextColorIdx : gpSet->AppStd.nTextColorIdx; };
			BYTE nBackColorIdx; // 0..15,16
			BYTE BackColorIdx() const { return (OverridePalette || !AppNames) ? nBackColorIdx : gpSet->AppStd.nBackColorIdx; };
			BYTE nPopTextColorIdx; // 0..15,16
			BYTE PopTextColorIdx() const { return (OverridePalette || !AppNames) ? nPopTextColorIdx : gpSet->AppStd.nPopTextColorIdx; };
			BYTE nPopBackColorIdx; // 0..15,16
			BYTE PopBackColorIdx() const { return (OverridePalette || !AppNames) ? nPopBackColorIdx : gpSet->AppStd.nPopBackColorIdx; };


			bool OverrideExtendFonts;
			//reg->Load(L"ExtendFonts", isExtendFonts);
			bool isExtendFonts;
			bool ExtendFonts() const { return (OverrideExtendFonts || !AppNames) ? isExtendFonts : gpSet->AppStd.isExtendFonts; };
			//reg->Load(L"ExtendFontNormalIdx", nFontNormalColor);
			BYTE nFontNormalColor; // 0..15
			BYTE FontNormalColor() const { return (OverrideExtendFonts || !AppNames) ? nFontNormalColor : gpSet->AppStd.nFontNormalColor; };
			//reg->Load(L"ExtendFontBoldIdx", nFontBoldColor);
			BYTE nFontBoldColor;   // 0..15
			BYTE FontBoldColor() const { return (OverrideExtendFonts || !AppNames) ? nFontBoldColor : gpSet->AppStd.nFontBoldColor; };
			//reg->Load(L"ExtendFontItalicIdx", nFontItalicColor);
			BYTE nFontItalicColor; // 0..15
			BYTE FontItalicColor() const { return (OverrideExtendFonts || !AppNames) ? nFontItalicColor : gpSet->AppStd.nFontItalicColor; };

			bool OverrideCursor;
			// *** Active ***
			////reg->Load(L"CursorType", isCursorType);
			////BYTE isCursorType; // 0 - Horz, 1 - Vert, 2 - Hollow-block
			//reg->Load(L"CursorTypeActive", CursorActive.Raw);
			//reg->Load(L"CursorTypeInactive", CursorActive.Raw);
			CECursorType CursorActive; // storage
			CECursorType CursorInactive; // storage
			CECursorStyle CursorStyle(bool bActive) const { return (OverrideCursor || !AppNames)
				? ((bActive || !CursorInactive.Used) ? CursorActive.CursorType : CursorInactive.CursorType)
				: ((bActive || !gpSet->AppStd.CursorInactive.Used) ? gpSet->AppStd.CursorActive.CursorType : gpSet->AppStd.CursorInactive.CursorType); };
			////reg->Load(L"CursorBlink", isCursorBlink);
			////bool isCursorBlink;
			bool CursorBlink(bool bActive) const { return (OverrideCursor || !AppNames)
				? ((bActive || !CursorInactive.Used) ? CursorActive.isBlinking : CursorInactive.isBlinking)
				: ((bActive || !gpSet->AppStd.CursorInactive.Used) ? gpSet->AppStd.CursorActive.isBlinking : gpSet->AppStd.CursorInactive.isBlinking); };
			////reg->Load(L"CursorColor", isCursorColor);
			////bool isCursorColor;
			bool CursorColor(bool bActive) const { return (OverrideCursor || !AppNames)
				? ((bActive || !CursorInactive.Used) ? CursorActive.isColor : CursorInactive.isColor)
				: ((bActive || !gpSet->AppStd.CursorInactive.Used) ? gpSet->AppStd.CursorActive.isColor : gpSet->AppStd.CursorInactive.isColor); };
			////reg->Load(L"CursorBlockInactive", isCursorBlockInactive);
			////bool isCursorBlockInactive;
			////bool CursorBlockInactive() const { return (OverrideCursor || !AppNames) ? isCursorBlockInactive : gpSet->AppStd.isCursorBlockInactive; };
			////reg->Load(L"CursorIgnoreSize", isCursorIgnoreSize);
			////bool isCursorIgnoreSize;
			bool CursorIgnoreSize(bool bActive) const { return (OverrideCursor || !AppNames)
				? ((bActive || !CursorInactive.Used) ? CursorActive.isFixedSize : CursorInactive.isFixedSize)
				: ((bActive || !gpSet->AppStd.CursorInactive.Used) ? gpSet->AppStd.CursorActive.isFixedSize : gpSet->AppStd.CursorInactive.isFixedSize); };
			////reg->Load(L"CursorFixedSize", nCursorFixedSize);
			////BYTE nCursorFixedSize; // � ���������
			BYTE CursorFixedSize(bool bActive) const { return (OverrideCursor || !AppNames)
				? ((bActive || !CursorInactive.Used) ? CursorActive.FixedSize : CursorInactive.FixedSize)
				: ((bActive || !gpSet->AppStd.CursorInactive.Used) ? gpSet->AppStd.CursorActive.FixedSize : gpSet->AppStd.CursorInactive.FixedSize); };
			//reg->Load(L"CursorMinSize", nCursorMinSize);
			//BYTE nCursorMinSize; // � ��������
			BYTE CursorMinSize(bool bActive) const { return (OverrideCursor || !AppNames)
				? ((bActive || !CursorInactive.Used) ? CursorActive.MinSize : CursorInactive.MinSize)
				: ((bActive || !gpSet->AppStd.CursorInactive.Used) ? gpSet->AppStd.CursorActive.MinSize : gpSet->AppStd.CursorInactive.MinSize); };

			bool OverrideClipboard;
			// *** Copying
			//reg->Load(L"ClipboardDetectLineEnd", isCTSDetectLineEnd);
			bool isCTSDetectLineEnd; // cbCTSDetectLineEnd
			bool CTSDetectLineEnd() const { return (OverrideClipboard || !AppNames) ? isCTSDetectLineEnd : gpSet->AppStd.isCTSDetectLineEnd; };
			//reg->Load(L"ClipboardBashMargin", isCTSBashMargin);
			bool isCTSBashMargin; // cbCTSBashMargin
			bool CTSBashMargin() const { return (OverrideClipboard || !AppNames) ? isCTSBashMargin : gpSet->AppStd.isCTSBashMargin; };
			//reg->Load(L"ClipboardTrimTrailing", isCTSTrimTrailing);
			BYTE isCTSTrimTrailing; // cbCTSTrimTrailing: 0 - ���, 1 - ��, 2 - ������ ��� stream-selection
			BYTE CTSTrimTrailing() const { return (OverrideClipboard || !AppNames) ? isCTSTrimTrailing : gpSet->AppStd.isCTSTrimTrailing; };
			//reg->Load(L"ClipboardEOL", isCTSEOL);
			BYTE isCTSEOL; // cbCTSEOL: 0="CR+LF", 1="LF", 2="CR"
			BYTE CTSEOL() const { return (OverrideClipboard || !AppNames) ? isCTSEOL : gpSet->AppStd.isCTSEOL; };
			//reg->Load(L"ClipboardArrowStart", pApp->isCTSShiftArrowStart);
			bool isCTSShiftArrowStart;
			bool CTSShiftArrowStart() const { return (OverrideClipboard || !AppNames) ? isCTSShiftArrowStart : gpSet->AppStd.isCTSShiftArrowStart; };
			// *** Pasting
			//reg->Load(L"ClipboardAllLines", isPasteAllLines);
			bool isPasteAllLines;
			bool PasteAllLines() const { return (OverrideClipboard || !AppNames) ? isPasteAllLines : gpSet->AppStd.isPasteAllLines; };
			//reg->Load(L"ClipboardFirstLine", isPasteFirstLine);
			bool isPasteFirstLine;
			bool PasteFirstLine() const { return (OverrideClipboard || !AppNames) ? isPasteFirstLine : gpSet->AppStd.isPasteFirstLine; };
			// *** Prompt
			// cbCTSClickPromptPosition
			//reg->Load(L"ClipboardClickPromptPosition", isCTSClickPromptPosition);
			//0 - off, 1 - force, 2 - try to detect "ReadConsole" (don't use 2 in bash)
			BYTE isCTSClickPromptPosition; // cbCTSClickPromptPosition
			BYTE CTSClickPromptPosition() const { return (OverrideClipboard || !AppNames) ? isCTSClickPromptPosition : gpSet->AppStd.isCTSClickPromptPosition; };
			BYTE isCTSDeleteLeftWord; // cbCTSDeleteLeftWord
			BYTE CTSDeleteLeftWord() const { return (OverrideClipboard || !AppNames) ? isCTSDeleteLeftWord : gpSet->AppStd.isCTSDeleteLeftWord; };

			bool OverrideBgImage;
			//reg->Load(L"BackGround Image show", isShowBgImage);
			char isShowBgImage; // cbBgImage
			char ShowBgImage() const { return (OverrideBgImage || !AppNames) ? isShowBgImage : gpSet->AppStd.isShowBgImage; };
			//reg->Load(L"BackGround Image", sBgImage, countof(sBgImage));
			WCHAR sBgImage[MAX_PATH]; // tBgImage
			LPCWSTR BgImage() const { return (OverrideBgImage || !AppNames) ? sBgImage : gpSet->AppStd.sBgImage; };
			//reg->Load(L"bgOperation", nBgOperation);
			char nBgOperation; // BackgroundOp {eUpLeft = 0, eStretch = 1, eTile = 2, ...}
			char BgOperation() const { return (OverrideBgImage || !AppNames) ? nBgOperation : gpSet->AppStd.nBgOperation; };


			void SetNames(LPCWSTR asAppNames)
			{
				size_t iLen = wcslen(asAppNames);

				if (!AppNames || !AppNamesLwr || (iLen >= cchNameMax))
				{
					SafeFree(AppNames);
					SafeFree(AppNamesLwr);

					cchNameMax = iLen+32;
					AppNames = (wchar_t*)malloc(cchNameMax*sizeof(wchar_t));
					AppNamesLwr = (wchar_t*)malloc(cchNameMax*sizeof(wchar_t));
					if (!AppNames || !AppNamesLwr)
					{
						_ASSERTE(AppNames!=NULL && AppNamesLwr!=NULL);
						return;
					}
				}

				_wcscpy_c(AppNames, iLen+1, asAppNames);
				_wcscpy_c(AppNamesLwr, iLen+1, asAppNames);
				CharLowerBuff(AppNamesLwr, iLen);
			};

			void FreeApps()
			{
				SafeFree(AppNames);
				SafeFree(AppNamesLwr);
				cchNameMax = 0;
			};
		};
		int GetAppSettingsId(LPCWSTR asExeAppName, bool abElevated);
		const AppSettings* GetAppSettings(int anAppId=-1);
		COLORREF* GetColors(int anAppId=-1, BOOL abFade = FALSE);
		COLORREF GetFadeColor(COLORREF cr);
		void ResetFadeColors();

		struct CommandTasks
		{
			size_t   cchNameMax;
			wchar_t* pszName;
			size_t   cchGuiArgMax;
			wchar_t* pszGuiArgs;
			size_t   cchCmdMax;
			wchar_t* pszCommands; // "\r\n" separated commands

			void FreePtr()
			{
				SafeFree(pszName);
				cchNameMax = 0;
				SafeFree(pszGuiArgs);
				cchGuiArgMax = 0;
				SafeFree(pszCommands);
				cchCmdMax = 0;
                CommandTasks* p = this;
                SafeFree(p);
			};

			void SetName(LPCWSTR asName, int anCmdIndex)
			{
				wchar_t szCmd[16];
				if (!asName || !*asName)
				{
					_wsprintf(szCmd, SKIPLEN(countof(szCmd)) L"Group%i", (anCmdIndex+1));
					asName = szCmd;
				}

				// ��� �������� ���������� ������ - ��� ������ ���� ��������� � ������� ������
				size_t iLen = wcslen(asName);

				if (!pszName || ((iLen+2) >= cchNameMax))
				{
					SafeFree(pszName);

					cchNameMax = iLen+16;
					pszName = (wchar_t*)malloc(cchNameMax*sizeof(wchar_t));
					if (!pszName)
					{
						_ASSERTE(pszName!=NULL);
						return;
					}
				}

				if (asName[0] == TaskBracketLeft)
				{
					_wcscpy_c(pszName, iLen+1, asName);
				}
				else
				{
					*pszName = TaskBracketLeft;
					_wcscpy_c(pszName+1, iLen+1, asName);
				}
				if (asName[iLen-1] != TaskBracketRight)
				{
					iLen = wcslen(pszName);
					pszName[iLen++] = TaskBracketRight; pszName[iLen] = 0;
				}
			};

			void SetGuiArg(LPCWSTR asGuiArg)
			{
				if (!asGuiArg)
					asGuiArg = L"";

				size_t iLen = wcslen(asGuiArg);

				if (!pszGuiArgs || (iLen >= cchGuiArgMax))
				{
					SafeFree(pszGuiArgs);

					cchGuiArgMax = iLen+256;
					pszGuiArgs = (wchar_t*)malloc(cchGuiArgMax*sizeof(wchar_t));
					if (!pszGuiArgs)
					{
						_ASSERTE(pszGuiArgs!=NULL);
						return;
					}
				}

				_wcscpy_c(pszGuiArgs, cchGuiArgMax, asGuiArg);
			};

			void SetCommands(LPCWSTR asCommands)
			{
				if (!asCommands)
					asCommands = L"";

				size_t iLen = wcslen(asCommands);

				if (!pszCommands || (iLen >= cchCmdMax))
				{
					SafeFree(pszCommands);

					cchCmdMax = iLen+1024;
					pszCommands = (wchar_t*)malloc(cchCmdMax*sizeof(wchar_t));
					if (!pszCommands)
					{
						_ASSERTE(pszCommands!=NULL);
						return;
					}
				}

				_wcscpy_c(pszCommands, cchCmdMax, asCommands);
			};

			void ParseGuiArgs(wchar_t** pszDir, wchar_t** pszIcon) const
			{
				if (!pszGuiArgs)
					return;
				LPCWSTR pszArgs = pszGuiArgs;
				wchar_t szArg[MAX_PATH+1];
				while (0 == NextArg(&pszArgs, szArg))
				{
					if (lstrcmpi(szArg, L"/DIR") == 0)
					{
						if (0 != NextArg(&pszArgs, szArg))
							break;
						if (*szArg && pszDir)
						{
							wchar_t* pszExpand = NULL;

							// ��������, "%USERPROFILE%"
							if (wcschr(szArg, L'%'))
							{
								pszExpand = ExpandEnvStr(szArg);
							}

							*pszDir = pszExpand ? pszExpand : lstrdup(szArg);
						}
					}
					else if (lstrcmpi(szArg, L"/ICON") == 0)
					{
						if (0 != NextArg(&pszArgs, szArg))
							break;
						if (*szArg && pszIcon)
						{
							wchar_t* pszExpand = NULL;

							// ��������, "%USERPROFILE%"
							if (wcschr(szArg, L'%'))
							{
								pszExpand = ExpandEnvStr(szArg);
							}

							*pszIcon = pszExpand ? pszExpand : lstrdup(szArg);
						}
					}
				}
			};
		};
		const CommandTasks* CmdTaskGet(int anIndex); // 0-based, index of CmdTasks. "-1" == autosaved task
		void CmdTaskSet(int anIndex, LPCWSTR asName, LPCWSTR asGuiArgs, LPCWSTR asCommands); // 0-based, index of CmdTasks
		bool CmdTaskXch(int anIndex1, int anIndex2); // 0-based, index of CmdTasks

		const ColorPalette* PaletteGet(int anIndex); // 0-based, index of Palettes
		void PaletteSaveAs(LPCWSTR asName); // Save active colors to named palette
		void PaletteDelete(LPCWSTR asName); // Delete named palette
		void PaletteSetStdIndexes();
		int PaletteSetActive(LPCWSTR asName);


		// 
		struct ConEmuProgressStore
		{
			// This two are stored in settings
			wchar_t* pszName;
			DWORD    nDuration;
			// Following are used in runtime
			DWORD    nStartTick;

			void FreePtr()
			{
				SafeFree(pszName);
			};
		};

		DWORD ProgressesGetDuration(LPCWSTR asName);
		void ProgressesSetDuration(LPCWSTR asName, DWORD anDuration);

	protected:
		AppSettings AppStd;
		int AppCount;
		AppSettings** Apps;
		// ��� CSettings
		AppSettings* GetAppSettingsPtr(int anAppId, BOOL abCreateNew = FALSE);
		void AppSettingsDelete(int anAppId);
		bool AppSettingsXch(int anIndex1, int anIndex2); // 0-based, index of Apps

		int CmdTaskCount;
		CommandTasks** CmdTasks;
		CommandTasks* StartupTask;
		bool LoadCmdTask(SettingsBase* reg, CommandTasks* &pTask, int iIndex);
		bool SaveCmdTask(SettingsBase* reg, CommandTasks* pTask);
		void FreeCmdTasks();

		int PaletteCount;
		ColorPalette** Palettes;
		int PaletteGetIndex(LPCWSTR asName);
		void SavePalettes(SettingsBase* reg);
		void SortPalettes();
		void FreePalettes();

		int ProgressesCount;
		ConEmuProgressStore** Progresses;
		bool LoadProgress(SettingsBase* reg, ConEmuProgressStore* &pProgress);
		bool SaveProgress(SettingsBase* reg, ConEmuProgressStore* pProgress);
		void FreeProgresses();

		void SaveStatusSettings(SettingsBase* reg);

	private:	
		// reg->Load(L"ColorTableNN", Colors[i]);
		COLORREF Colors[0x20];
		COLORREF ColorsFade[0x20];
		bool mb_FadeInitialized;

		struct CEAppColors
		{
			COLORREF Colors[0x20];
			COLORREF ColorsFade[0x20];
			bool FadeInitialized;
		} **AppColors; // [AppCount]

		void LoadCursorSettings(SettingsBase* reg, CECursorType* pActive, CECursorType* pInactive);

		void LoadAppSettings(SettingsBase* reg, bool abFromOpDlg = false);
		void LoadAppSettings(SettingsBase* reg, AppSettings* pApp, COLORREF* pColors);
		void SaveAppSettings(SettingsBase* reg, AppSettings* pApp, COLORREF* pColors);

		void FreeApps(int NewAppCount = 0, AppSettings** NewApps = NULL, Settings::CEAppColors** NewAppColors = NULL);

		DWORD mn_FadeMul;
		inline BYTE GetFadeColorItem(BYTE c);

	public:
		//reg->Load(L"FontAutoSize", isFontAutoSize);
		bool isFontAutoSize;
		//reg->Load(L"AutoRegisterFonts", isAutoRegisterFonts);
		bool isAutoRegisterFonts;
		
		//reg->Load(L"ConsoleFontName", ConsoleFont.lfFaceName, countof(ConsoleFont.lfFaceName));
		//reg->Load(L"ConsoleFontWidth", ConsoleFont.lfWidth);
		//reg->Load(L"ConsoleFontHeight", ConsoleFont.lfHeight);
		LOGFONT ConsoleFont;
		
		bool NeedDialogDetect();
		
		
		//reg->Load(L"TrueColorerSupport", isTrueColorer);
		bool isTrueColorer;
		

		/* *** Background image *** */
		//reg->Load(L"BackGround Image show", isShowBgImage);
		char isShowBgImage;
		//reg->Load(L"BackGround Image", sBgImage, countof(sBgImage));		
		WCHAR sBgImage[MAX_PATH];
		//reg->Load(L"bgImageDarker", bgImageDarker);
		u8 bgImageDarker;		
		//reg->Load(L"bgImageColors", nBgImageColors);
		DWORD nBgImageColors;
		//reg->Load(L"bgOperation", bgOperation);
		char bgOperation; // BackgroundOp {eUpLeft = 0, eStretch = 1, eTile = 2, ...}
		//reg->Load(L"bgPluginAllowed", isBgPluginAllowed);
		char isBgPluginAllowed;
		
		
		//bool isBackgroundImageValid;
		
		

		/* *** Transparency *** */
		bool isTransparentAllowed();
		//reg->Load(L"AlphaValue", nTransparent);
		u8 nTransparent;
		//reg->Load(L"AlphaValueSeparate", nTransparentSeparate);
		bool isTransparentSeparate;
		//reg->Load(L"AlphaValueInactive", nTransparentInactive);
		u8 nTransparentInactive;
		//reg->Load(L"UserScreenTransparent", isUserScreenTransparent);
		bool isUserScreenTransparent;
		//reg->Load(L"ColorKeyTransparent", isColorKeyTransparent);
		bool isColorKeyTransparent;
		//reg->Load(L"ColorKeyValue", nColorKeyValue);
		DWORD nColorKeyValue;

		/* *** Command Line History (from start dialog) *** */
		//reg->Load(L"CmdLineHistory", &psCmdHistory);
		LPWSTR psCmdHistory;
		//nCmdHistorySize = 0; HistoryCheck();
		DWORD nCmdHistorySize;

		/* *** Startup options *** */
		//reg->Load(L"StartType", nStartType);
		BYTE nStartType; // 0-cmd line, 1-cmd task file, 2-named task, 3-auto saved task
		//reg->Load(L"CmdLine", &psStartSingleApp);
		LPTSTR psStartSingleApp;
		//reg->Load(L"StartTasksFile", &psStartTasksFile);
		LPTSTR psStartTasksFile;
		//reg->Load(L"StartTasksName", &psStartTasksName);
		LPTSTR psStartTasksName;
		//reg->Load(L"StartFarFolders", isStartFarFolders);
		bool isStartFarFolders;
		//reg->Load(L"StartFarEditors", isStartFarEditors);
		bool isStartFarEditors;

		//reg->Load(L"StoreTaskbarkTasks", isStoreTaskbarkTasks);
		bool isStoreTaskbarkTasks;
		//reg->Load(L"StoreTaskbarCommands", isStoreTaskbarCommands);
		bool isStoreTaskbarCommands;

		
		/* Command Line ("/cmd" arg) */
		LPTSTR psCurCmd;
		bool isCurCmdList; // � ��� ���� ��� ������ /cmdlist

		/* 'Default' command line (if nor Registry, nor /cmd specified) */
		//WCHAR  szDefCmd[16];
	public:
		/* "Active" command line */
		LPCTSTR GetCmd(bool *pIsCmdList = NULL);

		RecreateActionParm GetDefaultCreateAction();

		//reg->Load(L"FontName", inFont, countof(inFont))
		wchar_t inFont[LF_FACESIZE];
		//reg->Load(L"FontName2", inFont2, countof(inFont2))
		wchar_t inFont2[LF_FACESIZE];
		//reg->Load(L"FontBold", isBold);
		bool isBold;
		//reg->Load(L"FontItalic", isItalic);
		bool isItalic;
		//reg->Load(L"Anti-aliasing", Quality);
		DWORD mn_AntiAlias; //���������� ��� Quality
		//reg->Load(L"FontCharSet", mn_LoadFontCharSet); mb_CharSetWasSet = FALSE;
		BYTE mn_LoadFontCharSet; // �� ��� ��������� ���������� (��� ��� ��������� � ������)
		//reg->Load(L"FontCharSet", mn_LoadFontCharSet); mb_CharSetWasSet = FALSE;
		BOOL mb_CharSetWasSet;
		//reg->Load(L"FontSize", FontSizeY);
		DWORD FontSizeY;  // ������ ��������� ������ (����������� �� ��������!)
		//reg->Load(L"FontSizeX", FontSizeX);
		DWORD FontSizeX;  // ������ ��������� ������
		//reg->Load(L"FontSizeX2", FontSizeX2);
		DWORD FontSizeX2; // ������ ��� FixFarBorders (������ ������������ ������ ��� ��������� �����, �� ������ �� �����������)
		//reg->Load(L"FontSizeX3", FontSizeX3);
		DWORD FontSizeX3; // ������ ���������� ��� ���������� ������ (�� ������ � FontSizeX2)
		
		
		//reg->Load(L"HideCaption", isHideCaption);
		bool isHideCaption; // Hide caption when maximized
		//reg->Load(L"HideChildCaption", isHideChildCaption);
		bool isHideChildCaption; // Hide caption of child GUI applications, started in ConEmu tabs (PuTTY, Notepad, etc.)
		//reg->Load(L"FocusInChildWindows", isFocusInChildWindows);
		bool isFocusInChildWindows;
		//reg->Load(L"QuakeStyle", isQuakeStyle);
		BYTE isQuakeStyle; // 0 - NoQuake, 1 - Quake, 2 - Quake+HideOnLoseFocus
		DWORD nQuakeAnimation;
		protected:
		//reg->Load(L"HideCaptionAlways", mb_HideCaptionAlways);
		bool mb_HideCaptionAlways;
		public:
		void SetHideCaptionAlways(bool bHideCaptionAlways);
		void SwitchHideCaptionAlways();
		bool isHideCaptionAlways(); //<<mb_HideCaptionAlways
		bool isMinimizeOnLoseFocus();
		bool isForcedHideCaptionAlways(); // true, ���� mb_HideCaptionAlways ��������� ������
		bool isCaptionHidden(ConEmuWindowMode wmNewMode = wmCurrent);
		bool isFrameHidden();
		//reg->Load(L"HideCaptionAlwaysFrame", nHideCaptionAlwaysFrame);
		BYTE nHideCaptionAlwaysFrame;
		int HideCaptionAlwaysFrame() { return (gpSet->nHideCaptionAlwaysFrame > 0x7F) ? -1 : gpSet->nHideCaptionAlwaysFrame; };
		//reg->Load(L"HideCaptionAlwaysDelay", nHideCaptionAlwaysDelay);
		DWORD nHideCaptionAlwaysDelay;
		//reg->Load(L"HideCaptionAlwaysDisappear", nHideCaptionAlwaysDisappear);
		DWORD nHideCaptionAlwaysDisappear;
		//reg->Load(L"DownShowHiddenMessage", isDownShowHiddenMessage);
		bool isDownShowHiddenMessage;
		//reg->Load(L"AlwaysOnTop", isAlwaysOnTop);
		bool isAlwaysOnTop;
		//reg->Load(L"DesktopMode", isDesktopMode);
		bool isDesktopMode;
		//reg->Load(L"SnapToDesktopEdges", isSnapToDesktopEdges);
		bool isSnapToDesktopEdges;
		//reg->Load(L"FixFarBorders", isFixFarBorders)
		BYTE isFixFarBorders;
		//reg->Load(L"ExtendUCharMap", isExtendUCharMap);
		bool isExtendUCharMap;
		//reg->Load(L"DisableMouse", isDisableMouse);
		bool isDisableMouse;
		//reg->Load(L"MouseSkipActivation", isMouseSkipActivation);
		bool isMouseSkipActivation;
		//reg->Load(L"MouseSkipMoving", isMouseSkipMoving);
		bool isMouseSkipMoving;
		//reg->Load(L"FarHourglass", isFarHourglass);
		bool isFarHourglass;
		//reg->Load(L"FarHourglassDelay", nFarHourglassDelay);
		DWORD nFarHourglassDelay;
		//reg->Load(L"DisableFarFlashing", isDisableFarFlashing); if (isDisableFarFlashing>2) isDisableFarFlashing = 2;
		BYTE isDisableFarFlashing;
		//reg->Load(L"DisableAllFlashing", isDisableAllFlashing); if (isDisableAllFlashing>2) isDisableAllFlashing = 2;
		BYTE isDisableAllFlashing;
		/* *** Text selection *** */
		//reg->Load(L"ConsoleTextSelection", isConsoleTextSelection);
		BYTE isConsoleTextSelection;
		//reg->Load(L"CTS.AutoCopy", isCTSAutoCopy);
		bool isCTSAutoCopy;
		//reg->Load(L"CTS.EndOnTyping", isCTSEndOnTyping);
		BYTE isCTSEndOnTyping;
		//reg->Load(L"CTS.EndOnKeyPress", isCTSEndOnKeyPress);
		bool isCTSEndOnKeyPress; // +isCTSEndOnTyping. +���, ��� �� ������� WM_CHAR (������� � ��.)
		//reg->Load(L"CTS.Freeze", isCTSFreezeBeforeSelect);
		bool isCTSFreezeBeforeSelect;
		//reg->Load(L"CTS.SelectBlock", isCTSSelectBlock);
		bool isCTSSelectBlock;
		//reg->Load(L"CTS.SelectText", isCTSSelectText);
		bool isCTSSelectText;
		////reg->Load(L"CTS.ClickPromptPosition", isCTSClickPromptPosition);
		//BYTE isCTSClickPromptPosition; // & vkCTSVkPromptClk
		//reg->Load(L"CTS.VkBlock", isCTSVkBlock);
		//BYTE isCTSVkBlock; // ����������� ������� ��������� ������
		//reg->Load(L"CTS.VkBlockStart", vmCTSVkBlockStart);
		//DWORD vmCTSVkBlockStart; // ������ ������ ��������� ������������� �����
		//reg->Load(L"CTS.VkText", isCTSVkText);
		//BYTE isCTSVkText; // ����������� ������� ��������� ������
		//reg->Load(L"CTS.VkTextStart", vmCTSVkTextStart);
		//DWORD vmCTSVkTextStart; // ������ ������ ��������� ���������� �����
		//reg->Load(L"CTS.ActMode", isCTSActMode);
		BYTE isCTSActMode; // ����� � ����������� ���������� �������� ������ � ������� ������ �����
		//reg->Load(L"CTS.VkAct", isCTSVkAct);
		//BYTE isCTSVkAct; // ����� � ����������� ���������� �������� ������ � ������� ������ �����
		//reg->Load(L"CTS.RBtnAction", isCTSRBtnAction);
		BYTE isCTSRBtnAction; // 0-off, 1-copy, 2-paste, 3-auto
		//reg->Load(L"CTS.MBtnAction", isCTSMBtnAction);
		BYTE isCTSMBtnAction; // 0-off, 1-copy, 2-paste, 3-auto
		//reg->Load(L"CTS.ColorIndex", isCTSColorIndex);
		BYTE isCTSColorIndex;
		//reg->Load(L"ClipboardConfirmEnter", isPasteConfirmEnter);
		bool isPasteConfirmEnter;
		//reg->Load(L"ClipboardConfirmLonger", nPasteConfirmLonger);
		DWORD nPasteConfirmLonger;
		//reg->Load(L"FarGotoEditorOpt", isFarGotoEditor);
		bool isFarGotoEditor; // ������������ � ���������� �� ����/������ (������ �����������)
		//reg->Load(L"FarGotoEditorVk", isFarGotoEditorVk);
		//BYTE isFarGotoEditorVk; // �������-����������� ��� isFarGotoEditor
		//reg->Load(L"FarGotoEditorPath", sFarGotoEditor);
		wchar_t* sFarGotoEditor; // ������� ������� ���������
		
		bool IsModifierPressed(int nDescrID, bool bAllowEmpty);
		//bool isSelectionModifierPressed();
		//typedef struct tag_CharRanges
		//{
		//	bool bUsed;
		//	wchar_t cBegin, cEnd;
		//} CharRanges;
		//wchar_t mszCharRanges[120];
		//CharRanges icFixFarBorderRanges[10];
		
		// !!! ������� �� �������� Init/Load... !!!
		int ParseCharRanges(LPCWSTR asRanges, BYTE (&Chars)[0x10000], BYTE abValue = TRUE); // ��������, L"2013-25C3,25C4"
		wchar_t* CreateCharRanges(BYTE (&Chars)[0x10000]); // caller must free(result)
		BYTE mpc_FixFarBorderValues[0x10000];
		
		//reg->Load(L"KeyboardHooks", m_isKeyboardHooks); if (m_isKeyboardHooks>2) m_isKeyboardHooks = 0;
		BYTE m_isKeyboardHooks;
	public:
		bool isKeyboardHooks(bool abNoDisable = false);

		//bool CheckUpdatesWanted();

		bool isCharBorder(wchar_t inChar);
		
		//reg->Load(L"PartBrush75", isPartBrush75); if (isPartBrush75<5) isPartBrush75=5; else if (isPartBrush75>250) isPartBrush75=250;
		BYTE isPartBrush75;
		//reg->Load(L"PartBrush50", isPartBrush50); if (isPartBrush50<5) isPartBrush50=5; else if (isPartBrush50>250) isPartBrush50=250;
		BYTE isPartBrush50;
		//reg->Load(L"PartBrush25", isPartBrush25); if (isPartBrush25<5) isPartBrush25=5; else if (isPartBrush25>250) isPartBrush25=250;
		BYTE isPartBrush25;
		//reg->Load(L"PartBrushBlack", isPartBrushBlack);
		BYTE isPartBrushBlack;
		
		//reg->Load(L"RightClick opens context menu", isRClickSendKey);
		// 0 - �� ����� EMenu, 1 - ����� ������, 2 - ����� �� �������� �����
		char isRClickSendKey;
		//��� ���������� - ������� �� �������� ���� ���������� ����,
		// � �� �������� (Press and tap) ��������� ��������� ������
		// �������, ���� isRClickTouch, �� "�������"/"��������" ���� �������������
		// --> isRClickSendKey==1 - ����� ������ (isRClickTouchInvert �� ������)
		// --> isRClickSendKey==2 - ����� �� �������� ���� (������ �������� RClick)
		// ��� ����, PressAndTap ������ �������� RClick � ������� (��� ��������� ������).
		bool isRClickTouchInvert();
		//reg->Load(L"RightClickMacro2", &sRClickMacro);
		wchar_t *sRClickMacro;
		LPCWSTR RClickMacro(FarMacroVersion fmv);
		LPCWSTR RClickMacroDefault(FarMacroVersion fmv);
		
		//reg->Load(L"SafeFarClose", isSafeFarClose);
		bool isSafeFarClose;
		//reg->Load(L"SafeFarCloseMacro", &sSafeFarCloseMacro);
		wchar_t *sSafeFarCloseMacro;
		LPCWSTR SafeFarCloseMacro(FarMacroVersion fmv);
		LPCWSTR SafeFarCloseMacroDefault(FarMacroVersion fmv);

		////reg->Load(L"AltEnter", isSendAltEnter);
		//bool isSendAltEnter;
		////reg->Load(L"AltSpace", isSendAltSpace);
		//bool isSendAltSpace;
		//reg->Load(L"SendAltTab", isSendAltTab);
		bool isSendAltTab;
		//reg->Load(L"SendAltEsc", isSendAltEsc);
		bool isSendAltEsc;
		//reg->Load(L"SendAltPrintScrn", isSendAltPrintScrn);
		bool isSendAltPrintScrn;
		//reg->Load(L"SendPrintScrn", isSendPrintScrn);
		bool isSendPrintScrn;
		//reg->Load(L"SendCtrlEsc", isSendCtrlEsc);
		bool isSendCtrlEsc;
		////reg->Load(L"SendAltF9", isSendAltF9);
		//bool isSendAltF9;
		
		//reg->Load(L"Min2Tray", mb_MinToTray);
		public:
		bool mb_MinToTray;
		bool isMinToTray(bool bRawOnly = false);
		void SetMinToTray(bool bMinToTray);
		//reg->Load(L"AlwaysShowTrayIcon", isAlwaysShowTrayIcon);
		bool isAlwaysShowTrayIcon;
		//bool isForceMonospace, isProportional;
		//reg->Load(L"Monospace", isMonospace)
		BYTE isMonospace; // 0 - proportional, 1 - monospace, 2 - forcemonospace
		//bool isUpdConHandle;
		//reg->Load(L"RSelectionFix", isRSelFix);
		bool isRSelFix;
		
		/* *** Drag *** */
		//reg->Load(L"Dnd", isDragEnabled);
		BYTE isDragEnabled;
		//reg->Load(L"DndDrop", isDropEnabled);
		BYTE isDropEnabled;
		//reg->Load(L"DndLKey", nLDragKey);
		//BYTE nLDragKey;
		//reg->Load(L"DndRKey", nRDragKey);
		//BYTE nRDragKey; // ��� DWORD
		//reg->Load(L"DefCopy", isDefCopy);
		bool isDefCopy;
		//reg->Load(L"DropUseMenu", isDropUseMenu);
		BYTE isDropUseMenu;
		//reg->Load(L"DragOverlay", isDragOverlay);
		bool isDragOverlay;
		//reg->Load(L"DragShowIcons", isDragShowIcons);
		bool isDragShowIcons;
		//reg->Load(L"DragPanel", isDragPanel); if (isDragPanel > 2) isDragPanel = 1;
		BYTE isDragPanel; // ��������� ������� ������� ������
		//reg->Load(L"DragPanelBothEdges", isDragPanelBothEdges);
		bool isDragPanelBothEdges; // ������� �� ��� ����� (������-����� ������ � �����-������ ������)

		//reg->Load(L"KeyBarRClick", isKeyBarRClick);
		bool isKeyBarRClick; // ������ ���� �� ������� - �������� PopupMenu
		
		//reg->Load(L"DebugSteps", isDebugSteps);
		bool isDebugSteps;
		
		//reg->Load(L"EnhanceGraphics", isEnhanceGraphics);
		bool isEnhanceGraphics; // Progressbars and scrollbars (pseudographics)
		//reg->Load(L"EnhanceButtons", isEnhanceButtons);
		bool isEnhanceButtons; // Buttons, CheckBoxes and RadioButtons (pseudographics)
		
		//reg->Load(L"FadeInactive", isFadeInactive);
		bool isFadeInactive;
		protected:
		//reg->Load(L"FadeInactiveLow", mn_FadeLow);
		BYTE mn_FadeLow;
		//reg->Load(L"FadeInactiveHigh", mn_FadeHigh);		
		BYTE mn_FadeHigh;
		//mn_LastFadeSrc = mn_LastFadeDst = -1;
		COLORREF mn_LastFadeSrc;
		//mn_LastFadeSrc = mn_LastFadeDst = -1;		
		COLORREF mn_LastFadeDst;
		public:

		//reg->Load(L"StatusBar.Show", isStatusBarShow);
		bool isStatusBarShow;
		//reg->Load(L"StatusBar.Flags", isStatusBarFlags);
		DWORD isStatusBarFlags; // set of CEStatusFlags
		//reg->Load(L"StatusFontFace", sStatusFontFace, countof(sStatusFontFace));
		wchar_t sStatusFontFace[LF_FACESIZE];
		//reg->Load(L"StatusFontCharSet", nStatusFontCharSet);
		DWORD nStatusFontCharSet;
		//reg->Load(L"StatusFontHeight", nStatusFontHeight);
		int nStatusFontHeight;
		int StatusBarFontHeight() { return max(4,nStatusFontHeight); };
		int StatusBarHeight() { return StatusBarFontHeight()+2; };
		//reg->Load(L"StatusBar.Color.Back", nStatusBarBack);
		DWORD nStatusBarBack;
		//reg->Load(L"StatusBar.Color.Light", nStatusBarLight);
		DWORD nStatusBarLight;
		//reg->Load(L"StatusBar.Color.Dark", nStatusBarDark);
		DWORD nStatusBarDark;
		//reg->Load(L"StatusBar.HideColumns", nHideStatusColumns);
		bool isStatusColumnHidden[64]; // _ASSERTE(countof(isStatusColumnHidden)>csi_Last);
		//��� ����������, ����� ��������� ��������� ��� ������
		bool mb_StatusSettingsWasChanged;
		
		//reg->Load(L"Tabs", isTabs);
		char isTabs;
		//reg->Load(L"TabsLocation", nTabsLocation);
		BYTE nTabsLocation; // 0 - top, 1 - bottom
		//reg->Load(L"TabSelf", isTabSelf);
		bool isTabSelf;
		//reg->Load(L"TabRecent", isTabRecent);
		bool isTabRecent;
		//reg->Load(L"TabLazy", isTabLazy);
		bool isTabLazy;

		//reg->Load(L"TabDblClick", nTabDblClickAction);
		DWORD nTabDblClickAction; // 0-None, 1-Auto, 2-Maximize/Restore, 3-NewTab.
		
		//TODO:
		bool isTabsInCaption;

		// Tab theme properties
		int ilDragHeight;

		protected:
		//reg->Load(L"TabsOnTaskBar", m_isTabsOnTaskBar);
		BYTE m_isTabsOnTaskBar; // 0 - ConEmu only, 1 - all tabs & all OS, 2 - all tabs & Win 7, 3 - DON'T SHOW
		public:
		bool isTabsOnTaskBar();
		bool isWindowOnTaskBar(bool bStrictOnly = false);
		//void SetTabsOnTaskBar(BYTE nTabsOnTaskBar);
		bool isTaskbarShield;
		
		//reg->Load(L"TabFontFace", sTabFontFace, countof(sTabFontFace));
		wchar_t sTabFontFace[LF_FACESIZE];
		//reg->Load(L"TabFontCharSet", nTabFontCharSet);
		DWORD nTabFontCharSet;
		//reg->Load(L"TabFontHeight", nTabFontHeight);
		int nTabFontHeight;
		
		//if (!reg->Load(L"TabCloseMacro", &sTabCloseMacro) || (sTabCloseMacro && !*sTabCloseMacro)) { if (sTabCloseMacro) { free(sTabCloseMacro); sTabCloseMacro = NULL; } }
		wchar_t *sTabCloseMacro;
		LPCWSTR TabCloseMacro(FarMacroVersion fmv);
		LPCWSTR TabCloseMacroDefault(FarMacroVersion fmv);
		
		//if (!reg->Load(L"SaveAllEditors", &sSaveAllMacro)) { sSaveAllMacro = lstrdup(L"...
		wchar_t *sSaveAllMacro;
		LPCWSTR SaveAllMacro(FarMacroVersion fmv);
		LPCWSTR SaveAllMacroDefault(FarMacroVersion fmv);
		
		//reg->Load(L"ToolbarAddSpace", nToolbarAddSpace);
		int nToolbarAddSpace;
		//reg->Load(L"ConWnd Width", wndWidth);
		DWORD _wndWidth;
		//reg->Load(L"ConWnd Height", wndHeight);
		DWORD _wndHeight;
		//reg->Load(L"16bit Height", ntvdmHeight);
		DWORD ntvdmHeight; // � ��������
		//reg->Load(L"ConWnd X", wndX);
		int _wndX; // � ��������
		//reg->Load(L"ConWnd Y", wndY);
		int _wndY; // � ��������
		//reg->Load(L"WindowMode", WindowMode); if (WindowMode!=rFullScreen && WindowMode!=rMaximized && WindowMode!=rNormal) WindowMode = rNormal;
		DWORD _WindowMode;
		//reg->Load(L"Cascaded", wndCascade);
		bool wndCascade;
		//reg->Load(L"AutoSaveSizePos", isAutoSaveSizePos);
		bool isAutoSaveSizePos;
		//reg->Load(L"UseCurrentSizePos", isUseCurrentSizePos);
		bool isUseCurrentSizePos; // Show in settings dialog and save current window size/pos

		bool isIntegralSize();

	private:
		// ��� �������� ���� ��������� - ��������� ������ ���� ���,
		// � �� ������ ����� � �������� �������� �������� ����������
		bool mb_SizePosAutoSaved;
	public:
		//reg->Load(L"SlideShowElapse", nSlideShowElapse);
		DWORD nSlideShowElapse;
		//reg->Load(L"IconID", nIconID);
		DWORD nIconID;
		//reg->Load(L"TryToCenter", isTryToCenter);
		bool isTryToCenter;
		//reg->Load(L"CenterConsolePad", nCenterConsolePad);
		DWORD nCenterConsolePad;
		//reg->Load(L"ShowScrollbar", isAlwaysShowScrollbar); if (isAlwaysShowScrollbar > 2) isAlwaysShowScrollbar = 2;
		BYTE isAlwaysShowScrollbar; // 0-�� ����������, 1-������, 2-������������� (�� ���������� ����� �� �������)
		//reg->Load(L"ScrollBarAppearDelay", nScrollBarAppearDelay);
		DWORD nScrollBarAppearDelay;
		//reg->Load(L"ScrollBarDisappearDelay", nScrollBarDisappearDelay);
		DWORD nScrollBarDisappearDelay;
		
		////reg->Load(L"TabMargins", rcTabMargins);
		//RECT rcTabMargins;
		////reg->Load(L"TabFrame", isTabFrame);
		//bool isTabFrame;

		//reg->Load(L"MinimizeRestore", vmMinimizeRestore);
		//DWORD vmMinimizeRestore;
		//reg->Load(L"Multi", isMulti);
		bool isMulti;
		//reg->Load(L"Multi.ShowButtons", isMultiShowButtons);
		bool isMultiShowButtons;
		//reg->Load(L"NumberInCaption", isMulti);
		bool isNumberInCaption;
		private:
		//reg->Load(L"Multi.Modifier", nHostkeyModifier); TestHostkeyModifiers();
		DWORD nHostkeyNumberModifier; // ������������ ��� 0..9, WinSize
		//reg->Load(L"Multi.ArrowsModifier", nHostkeyArrowModifier); TestHostkeyModifiers();
		DWORD nHostkeyArrowModifier; // ������������ ��� WinSize
		//
		public:
		//reg->Load(L"Multi.NewConsole", vmMultiNew);
		//DWORD vmMultiNew;
		//reg->Load(L"Multi.NewConsoleShift", vmMultiNewShift);
		//DWORD vmMultiNewShift; // Default - vmMultiNew+Shift
		//reg->Load(L"Multi.Next", vmMultiNext);
		//DWORD vmMultiNext;
		//reg->Load(L"Multi.NextShift", vmMultiNextShift);
		//DWORD vmMultiNextShift;
		//reg->Load(L"Multi.Recreate", vmMultiRecreate);
		//DWORD vmMultiRecreate;
		//reg->Load(L"Multi.Buffer", vmMultiBuffer);
		//DWORD vmMultiBuffer;
		//reg->Load(L"Multi.Close", vmMultiClose);
		//DWORD vmMultiClose;
		//reg->Load(L"Multi.CloseConfirm", isCloseConsoleConfirm);
		bool isCloseConsoleConfirm;
		bool isCloseEditViewConfirm;
		//reg->Load(L"Multi.CmdKey", vmMultiCmd);
		//DWORD vmMultiCmd;
		//reg->Load(L"Multi.AutoCreate", isMultiAutoCreate);
		bool isMultiAutoCreate;
		//reg->Load(L"Multi.LeaveOnClose", isMultiLeaveOnClose);
		bool isMultiLeaveOnClose;
		//reg->Load(L"Multi.HideOnClose", isMultiHideOnClose);
		bool isMultiHideOnClose;
		//reg->Load(L"Multi.MinByEsc", isMultiMinByEsc);
		BYTE isMultiMinByEsc; // 0 - Never, 1 - Always, 2 - NoConsoles
		//reg->Load(L"MapShiftEscToEsc", isMapShiftEscToEsc);
		bool isMapShiftEscToEsc; // used only when isMultiMinByEsc==1 and only for console apps
		//reg->Load(L"Multi.Iterate", isMultiIterate);
		bool isMultiIterate;
		//reg->Load(L"Multi.NewConfirm", isMultiNewConfirm);
		bool isMultiNewConfirm;
		//reg->Load(L"Multi.UseNumbers", isUseWinNumber);
		bool isUseWinNumber;
		//reg->Load(L"Multi.UseWinTab", isUseWinTab);
		bool isUseWinTab;
		//reg->Load(L"Multi.UseArrows", isUseWinArrows);
		bool isUseWinArrows;
		//reg->Load(L"Multi.SplitWidth", nSplitWidth);
		BYTE nSplitWidth;
		//reg->Load(L"Multi.SplitHeight", nSplitHeight);
		BYTE nSplitHeight;
		////reg->Load(L"Multi.SplitClr1", nSplitClr1);
		//DWORD nSplitClr1;
		////reg->Load(L"Multi.SplitClr2", nSplitClr2);
		//DWORD nSplitClr2;
		//reg->Load(L"FARuseASCIIsort", isFARuseASCIIsort);
		bool isFARuseASCIIsort;
		//reg->Load(L"FixAltOnAltTab", isFixAltOnAltTab);
		bool isFixAltOnAltTab;
		//reg->Load(L"ShellNoZoneCheck", isShellNoZoneCheck);
		bool isShellNoZoneCheck;

		// FindText: bMatchCase, bMatchWholeWords, bFreezeConsole, bHighlightAll;
		FindTextOptions FindOptions;
		

	public:
		/* ************************ */
		/* ************************ */
		/* *** Hotkeys/Hostkeys *** */
		/* ************************ */
		/* ************************ */

		// VkMod = LOBYTE - VK, ������� ��� ����� - ������������ (���� VK)

		// ������ ��� �������� ����������� � VkMod
		DWORD SetModifier(DWORD VkMod, BYTE Mod/*VK*/, bool Xor=true);
		// ��������� ������� ��� �������������. ��������� ������� VkMod
		DWORD MakeHotKey(BYTE Vk, BYTE vkMod1=0, BYTE vkMod2=0, BYTE vkMod3=0);
		// ������� ����������� ������������ (idx = 1..3). ���������� 0 (����) ��� VK
		DWORD GetModifier(DWORD VkMod, int idx/*1..3*/);
		// ������� ��� ������������ (���� "Apps+Space")
		LPCWSTR GetHotkeyName(const ConEmuHotKey* ppHK, wchar_t (&szFull)[128]);
		void GetVkKeyName(BYTE vk, wchar_t (&szName)[32]);
		// ������� ��� VK
		DWORD GetHotkey(DWORD VkMod);
		// ����� ������ MOD_xxx ��� RegisterHotKey
		DWORD GetHotKeyMod(DWORD VkMod);
		// ���� �� � ���� (VkMod) ������ - ����������� Mod (VK)
		bool  HasModifier(DWORD VkMod, BYTE Mod/*VK*/);
		// ������� �������� VkMod, ��� 0 ���� �� �����. nDescrID = vkXXX (e.g. vkMinimizeRestore)
		DWORD GetHotkeyById(int nDescrID, const ConEmuHotKey** ppHK = NULL);
		// ���������, ����� �� ���� hotkey. nDescrID = vkXXX (e.g. vkMinimizeRestore)
		bool IsHotkey(int nDescrID);
		// ���������� ����� hotkey. nDescrID = vkXXX (e.g. vkMinimizeRestore).
		void SetHotkeyById(int nDescrID, DWORD VkMod);
		// ���������, ���� �� ������ � ����������� ��������� �������������
		bool isModifierExist(BYTE Mod/*VK*/, bool abStrictSingle = false);
		// ���� �� ����� ������ ��� ����������� (��������� ��� VK_APPS)
		bool isKeyOrModifierExist(BYTE Mod/*VK*/);
		// ��������� �� �����
		void CheckHotkeyUnique();
	private:
		void LoadHotkeys(SettingsBase* reg);
		void SaveHotkeys(SettingsBase* reg);
		// nHostMod � ������� 3-� ������ ����� ��������� VK (������������).
		// ������� ���������, ����� ��� �� �������������
		void TestHostkeyModifiers(DWORD& nHostMod);
	public:

		/* *** ��������� ����� *** */
		//reg->Load(L"TabConsole", szTabConsole, countof(szTabConsole));
		WCHAR szTabConsole[32];
		//reg->Load(L"TabSkipWords", szTabSkipWords, countof(szTabSkipWords));
		WCHAR szTabSkipWords[255];
		//reg->Load(L"TabPanels", szTabPanels, countof(szTabPanels));
		WCHAR szTabPanels[32];
		//reg->Load(L"TabEditor", szTabEditor, countof(szTabEditor));
		WCHAR szTabEditor[32];
		//reg->Load(L"TabEditorModified", szTabEditorModified, countof(szTabEditorModified));
		WCHAR szTabEditorModified[32];
		//reg->Load(L"TabViewer", szTabViewer, countof(szTabViewer));
		WCHAR szTabViewer[32];
		//reg->Load(L"TabLenMax", nTabLenMax); if (nTabLenMax < 10 || nTabLenMax >= CONEMUTABMAX) nTabLenMax = 20;
		DWORD nTabLenMax;
		//todo
		DWORD nTabWidthMax;
		TabStyle nTabStyle; // enum

		//reg->Load(L"AdminTitleSuffix", szAdminTitleSuffix, countof(szAdminTitleSuffix)); szAdminTitleSuffix[countof(szAdminTitleSuffix)-1] = 0;
		wchar_t szAdminTitleSuffix[64]; //" (Admin)"
		//reg->Load(L"AdminShowShield", bAdminShield);
		bool bAdminShield;
		//reg->Load(L"HideInactiveConsoleTabs", bHideInactiveConsoleTabs);
		bool bHideInactiveConsoleTabs;

		TODO("��������/���������� bHideDisabledTabs");
		bool bHideDisabledTabs;
		
		//reg->Load(L"ShowFarWindows", bShowFarWindows);
		bool bShowFarWindows;

		bool NeedCreateAppWindow();
		
		//reg->Load(L"MainTimerElapse", nMainTimerElapse); if (nMainTimerElapse>1000) nMainTimerElapse = 1000;
		DWORD nMainTimerElapse; // �������������, � ������� �� ������� ����������� �����
		//reg->Load(L"MainTimerInactiveElapse", nMainTimerInactiveElapse); if (nMainTimerInactiveElapse>10000) nMainTimerInactiveElapse = 10000;
		DWORD nMainTimerInactiveElapse; // ������������� ��� ������������
		
		//bool isAdvLangChange; // � ����� ��� ConIme � ����� ������� �� �������� ����, ���� �� ������� WM_SETFOCUS. �� ��� ���� �������� ������ �������� ������
		
		//reg->Load(L"SkipFocusEvents", isSkipFocusEvents);
		bool isSkipFocusEvents;
		
		//bool isLangChangeWsPlugin;
		
		//reg->Load(L"MonitorConsoleLang", isMonitorConsoleLang);
		BYTE isMonitorConsoleLang;
		
		//reg->Load(L"SleepInBackground", isSleepInBackground);
		bool isSleepInBackground;

		//reg->Load(L"MinimizeOnLoseFocus", mb_MinimizeOnLoseFocus);
		bool mb_MinimizeOnLoseFocus;

		//reg->Load(L"AffinityMask", nAffinity);
		DWORD nAffinity;

		//reg->Load(L"UseInjects", isUseInjects);
		bool isUseInjects; // 0 - off, 1 - always /*, 2 - only executable*/. Note, Root process is infiltrated always.
		//reg->Load(L"ProcessAnsi", isProcessAnsi);
		bool isProcessAnsi; // ANSI X3.64 & XTerm-256-colors Support
		//reg->Load(L"UseClink", mb_UseClink);
		bool mb_UseClink; // ������������ ���������� ��������� ������ (ReadConsole)
		DWORD isUseClink(bool abCheckVersion = false);
		//reg->Load(L"PortableReg", isPortableReg);
		bool isPortableReg;

		/* *** Debugging *** */
		//reg->Load(L"ConVisible", isConVisible);
		bool isConVisible;
		//reg->Load(L"ConInMode", nConInMode);
		DWORD nConInMode;

		/* *** Thumbnails and Tiles *** */
		//reg->Load(L"PanView.BackColor", ThSet.crBackground.RawColor);
		//reg->Load(L"PanView.PFrame", ThSet.nPreviewFrame); if (ThSet.nPreviewFrame!=0 && ThSet.nPreviewFrame!=1) ThSet.nPreviewFrame = 1;
		//� �.�...
		PanelViewSetMapping ThSet;
		
		/* *** AutoUpdate *** */
		ConEmuUpdateSettings UpdSet;
		//wchar_t *szUpdateVerLocation; // ConEmu latest version location info
		//bool isUpdateCheckOnStartup;
		//bool isUpdateCheckHourly;
		//bool isUpdateConfirmDownload;
		//BYTE isUpdateUseBuilds; // 1-stable only, 2-latest
		//bool isUpdateUseProxy;
		//wchar_t *szUpdateProxy; // "Server:port"
		//wchar_t *szUpdateProxyUser;
		//wchar_t *szUpdateProxyPassword;
		//BYTE isUpdateDownloadSetup; // 1-Installer (ConEmuSetup.exe), 2-7z archieve (ConEmu.7z), WinRar or 7z required
		//wchar_t *szUpdateArcCmdLine; // "%1"-archive file, "%2"-ConEmu base dir
		//wchar_t *szUpdateDownloadPath; // "%TEMP%"
		//bool isUpdateLeavePackages;
		//wchar_t *szUpdatePostUpdateCmd; // ���� ����� ����-�� ���� ������ � �������������� �������


		/* *** HotKeys & GuiMacros *** */
		//reg->Load(L"GuiMacro<N>.Key", &Macros.vk);
		//reg->Load(L"GuiMacro<N>.Macro", &Macros.szGuiMacro);
		struct HotGuiMacro
		{
			union {
				BYTE vk;
				LPVOID dummy;
			};
			wchar_t* szGuiMacro;
		};
		HotGuiMacro Macros[24];
		
	public:
		ConEmuHotKey* AllocateHotkeys();
		void LoadSettings(bool *rbNeedCreateVanilla);
		void InitSettings();
		void LoadCmdTasks(SettingsBase* reg, bool abFromOpDlg = false);
		void LoadPalettes(SettingsBase* reg);
		void LoadProgresses(SettingsBase* reg);
		BOOL SaveSettings(BOOL abSilent = FALSE, const SettingsStorage* apStorage = NULL);
		void SaveAppSettings(SettingsBase* reg);
		bool SaveCmdTasks(SettingsBase* reg);
		bool SaveProgresses(SettingsBase* reg);
		void SaveSizePosOnExit();
		void SaveConsoleFont();
		void SaveFindOptions(SettingsBase* reg = NULL);
		//void UpdateMargins(RECT arcMargins);
	public:
		void HistoryCheck();
		void HistoryAdd(LPCWSTR asCmd);
		void HistoryReset();
		LPCWSTR HistoryGet();
		//void UpdateConsoleMode(DWORD nMode);
		//BOOL CheckConIme();
		void CheckConsoleSettings();
		
		SettingsBase* CreateSettings(const SettingsStorage* apStorage);
		
		void GetSettingsType(SettingsStorage& Storage, bool& ReadOnly);
		
	private:
		struct CEFontRange
		{
		public:
			bool    bUsed;              // ��� �������� ���������/����������
			wchar_t sTitle[64];         // "Title"="Borders and scrollbars"
			wchar_t sRange[1024];       // "CharRange"="2013-25C4;"
			wchar_t sFace[LF_FACESIZE]; // "FontFace"="Andale Mono"
			LONG    nHeight;            // "Height"=dword:00000012
			LONG    nWidth;             // "Width"=dword:00000000
			LONG    nLoadHeight;        // ��� ����������� ���������� ������������� �������� ������ ���
			LONG    nLoadWidth;         // ������� GUI, ��� ��������� �� ������� - ��������� ��������
			BYTE    nCharset;           // "Charset"=hex:00
			bool    bBold;              // "Bold"=hex:00
			bool    bItalic;            // "Italic"=hex:00
			BYTE    nQuality;           // "Anti-aliasing"=hex:03
			/*    */
		private:
			LOGFONT LF;
		public:
			LOGFONT& LogFont()
			{
				LF.lfHeight = nHeight;
				LF.lfWidth = nWidth;
				LF.lfEscapement = 0;
				LF.lfOrientation = 0;
				LF.lfWeight = bBold?FW_BOLD:FW_NORMAL;
				LF.lfItalic = bItalic ? 1 : 0;
				LF.lfUnderline = 0;
				LF.lfStrikeOut = 0;
				LF.lfQuality = nQuality;
				LF.lfCharSet = nCharset;
				LF.lfOutPrecision = OUT_TT_PRECIS;
				LF.lfClipPrecision = CLIP_DEFAULT_PRECIS;
				LF.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
				lstrcpyn(LF.lfFaceName, sFace, countof(LF.lfFaceName));
				return LF;
			};
			void Scale(LONG anMainHeight, LONG anMainLoadHeight)
			{
				// ������
				if (nLoadHeight == 0 || !anMainLoadHeight)
				{
					nHeight = anMainHeight; // ������ ����� ������ � Main
				}
				else
				{
					nHeight = nLoadHeight * anMainHeight / anMainLoadHeight;
				}
				// ������
				if (nLoadWidth == 0 || !anMainLoadHeight)
				{
					nWidth = 0;
				}
				else
				{
					nWidth = nLoadWidth * anMainHeight / anMainLoadHeight;
				}
			};
			/*    */
			HFONT hFonts[MAX_FONT_STYLES]; //normal/(bold|italic|underline)
			/*    */
			BYTE RangeData[0x10000];
		};
		CEFontRange m_Fonts[MAX_FONT_GROUPS]; // 0-Main, 1-Borders, 2 � ����� - user defined
		BOOL FontRangeLoad(SettingsBase* reg, int Idx);
		BOOL FontRangeSave(SettingsBase* reg, int Idx);

	protected:
		static bool UseWinNumber() { return gpSet->isMulti && gpSet->isUseWinNumber; };
		static bool UseWinArrows() { return gpSet->isUseWinArrows; };
		static bool UseCTSShiftArrow(); // { return gpSet->isUseWinArrows; }; // { return (OverrideClipboard || !AppNames) ? isCTSShiftArrowStart : gpSet->AppStd.isCTSShiftArrowStart; };
};

#include "OptionsClass.h"
