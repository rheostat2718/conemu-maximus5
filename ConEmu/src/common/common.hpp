
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


#ifndef _COMMON_HEADER_HPP_
#define _COMMON_HEADER_HPP_

// ������ ����������
#define CESERVER_REQ_VER    77

#include "defines.h"
#include "ConEmuColors.h"

#define MIN_CON_WIDTH 28
#define MIN_CON_HEIGHT 7
#define GUI_ATTACH_TIMEOUT 5000


#ifndef CONSOLE_NO_SELECTION

typedef struct _CONSOLE_SELECTION_INFO
{
	DWORD dwFlags;
	COORD dwSelectionAnchor;
	SMALL_RECT srSelection;
} CONSOLE_SELECTION_INFO, *PCONSOLE_SELECTION_INFO;

#endif



//#define MAXCONMAPCELLS      (600*400)
#define CES_NTVDM 0x10
#define CEC_INITTITLE       L"ConEmu"
//#define CE_CURSORUPDATE     L"ConEmuCursorUpdate%u" // ConEmuC_PID - ��������� ������ (������ ��� ���������). ��������� ������� ����������� GUI

// Pipe name formats
#define CESERVERPIPENAME    L"\\\\%s\\pipe\\ConEmuSrv%u"      // ConEmuC_PID
#define CESERVERINPUTNAME   L"\\\\%s\\pipe\\ConEmuSrvInput%u" // ConEmuC_PID
#define CESERVERQUERYNAME   L"\\\\%s\\pipe\\ConEmuSrvQuery%u" // ConEmuC_PID
#define CESERVERWRITENAME   L"\\\\%s\\pipe\\ConEmuSrvWrite%u" // ConEmuC_PID
#define CESERVERREADNAME    L"\\\\%s\\pipe\\ConEmuSrvRead%u"  // ConEmuC_PID
#define CEGUIPIPENAME       L"\\\\%s\\pipe\\ConEmuGui%u"      // GetConsoleWindow() // ����������, ����� ������ ��� �������� � GUI
															  // ghConEmuWndRoot --> CConEmuMain::GuiServerThreadCommand
#define CEPLUGINPIPENAME    L"\\\\%s\\pipe\\ConEmuPlugin%u"   // Far_PID
#define CEHOOKSPIPENAME     L"\\\\%s\\pipe\\ConEmuHk%u"       // PID ��������, � ������� �������� Pipe

#define CEINPUTSEMAPHORE    L"ConEmuInputSemaphore.%08X"      // GetConsoleWindow()

// Mapping name formats
#define CEGUIINFOMAPNAME    L"ConEmuGuiInfoMapping.%u" // --> ConEmuGuiMapping            ( % == dwGuiProcessId )
#define CECONMAPNAME        L"ConEmuFileMapping.%08X"  // --> CESERVER_CONSOLE_MAPPING_HDR ( % == (DWORD)ghConWnd )
#define CECONMAPNAME_A      "ConEmuFileMapping.%08X"   // --> CESERVER_CONSOLE_MAPPING_HDR ( % == (DWORD)ghConWnd ) simplifying ansi
#define CEFARMAPNAME        L"ConEmuFarMapping.%u"     // --> CEFAR_INFO_MAPPING               ( % == nFarPID )
#define CECONVIEWSETNAME    L"ConEmuViewSetMapping.%u" // --> PanelViewSetMapping
//#ifdef _DEBUG
#define CEPANELDLGMAPNAME   L"ConEmuPanelViewDlgsMapping.%u" // -> DetectedDialogs     ( % == nFarPID )
//#endif

#define CEDATAREADYEVENT    L"ConEmuSrvDataReady.%u"
#define CEFARALIVEEVENT     L"ConEmuFarAliveEvent.%u"
//#define CECONMAPNAMESIZE    (sizeof(CESERVER_REQ_CONINFO)+(MAXCONMAPCELLS*sizeof(CHAR_INFO)))
//#define CEGUIATTACHED       L"ConEmuGuiAttached.%u"
#define CEGUIRCONSTARTED    L"ConEmuGuiRConStarted.%u"
#define CEGUI_ALIVE_EVENT   L"ConEmuGuiStarted"
#define CEKEYEVENT_CTRL     L"ConEmuCtrlPressed.%u"
#define CEKEYEVENT_SHIFT    L"ConEmuShiftPressed.%u"
#define CEHOOKLOCKMUTEX     L"ConEmuHookMutex.%u"
//#define CEHOOKDISABLEEVENT  L"ConEmuSkipHooks.%u"
#define CEGHOSTSKIPACTIVATE L"ConEmuGhostActivate.%u"

#define CESECURITYNAME       "ConEmuLocalData"

//#define CONEMUMSG_ATTACH L"ConEmuMain::Attach"            // wParam == hConWnd, lParam == ConEmuC_PID
//WARNING("CONEMUMSG_SRVSTARTED ����� ���������� � ������� ����� ��� GUI");
//#define CONEMUMSG_SRVSTARTED L"ConEmuMain::SrvStarted"    // wParam == hConWnd, lParam == ConEmuC_PID
//#define CONEMUMSG_SETFOREGROUND L"ConEmuMain::SetForeground"            // wParam == hConWnd, lParam == ConEmuC_PID
#define CONEMUMSG_FLASHWINDOW L"ConEmuMain::FlashWindow"
//#define CONEMUCMDSTARTED L"ConEmuMain::CmdStarted"    // wParam == hConWnd, lParam == ConEmuC_PID (as ComSpec)
//#define CONEMUCMDSTOPPED L"ConEmuMain::CmdTerminated" // wParam == hConWnd, lParam == ConEmuC_PID (as ComSpec)
//#define CONEMUMSG_LLKEYHOOK L"ConEmuMain::LLKeyHook"    // wParam == hConWnd, lParam == ConEmuC_PID
#define CONEMUMSG_ACTIVATECON L"ConEmuMain::ActivateCon"  // wParam == ConNumber (1..12)
#define CONEMUMSG_SWITCHCON L"ConEmuMain::SwitchCon"
#define CONEMUMSG_HOOKEDKEY L"ConEmuMain::HookedKey"
#define CONEMUMSG_CONSOLEHOOKEDKEY L"ConEmuMain::ConsoleHookedKey"
#define CONEMUMSG_PNLVIEWFADE L"ConEmuTh::Fade"
#define CONEMUMSG_PNLVIEWSETTINGS L"ConEmuTh::Settings"

enum ConEmuTabCommand
{
	// 0: ��������/�������� ����, 1: ������� �� ���������, 2: ������� �� ����������, 3: commit switch
	ctc_ShowHide = 0,
	ctc_SwitchNext = 1,
	ctc_SwitchPrev = 2,
	ctc_SwitchCommit = 3,
};

enum CONSOLE_KEY_ID
{
	ID_ALTTAB,
	ID_ALTESC,
	ID_ALTSPACE,
	ID_ALTENTER,
	ID_ALTPRTSC,
	ID_PRTSC,
	ID_CTRLESC,
};

//#define CONEMUMAPPING    L"ConEmuPluginData%u"
//#define CONEMUDRAGFROM   L"ConEmuDragFrom%u"
//#define CONEMUDRAGTO     L"ConEmuDragTo%u"
//#define CONEMUREQTABS    L"ConEmuReqTabs%u"
//#define CONEMUSETWINDOW  L"ConEmuSetWindow%u"
//#define CONEMUPOSTMACRO  L"ConEmuPostMacro%u"
//#define CONEMUDEFFONT    L"ConEmuDefFont%u"
//#define CONEMULANGCHANGE L"ConEmuLangChange%u"
//#define CONEMUEXIT       L"ConEmuExit%u"
//#define CONEMUALIVE      L"ConEmuAlive%u"
//#define CONEMUREADY      L"ConEmuReady%u"
#define CONEMUTABCHANGED L"ConEmuTabsChanged"

#define VIRTUAL_REGISTRY_GUID   L"{16B56CA5-F8D2-4EEA-93DC-32403C7355E1}"
#define VIRTUAL_REGISTRY_GUID_A  "{16B56CA5-F8D2-4EEA-93DC-32403C7355E1}"
#define VIRTUAL_REGISTRY_ROOT   L"Software\\ConEmu Virtual Registry"

//#define CESIGNAL_C          L"ConEmuC_C_Signal.%u"
//#define CESIGNAL_BREAK      L"ConEmuC_Break_Signal.%u"

typedef DWORD CECMD;
const CECMD
//	CECMD_CONSOLEINFO    = 1,
	CECMD_CONSOLEDATA    = 2,
//	CECMD_SETSIZE        = 3,
	CECMD_CMDSTARTSTOP   = 4, // CESERVER_REQ_STARTSTOP //-V112
	CECMD_GETGUIHWND     = 5,
//	CECMD_RECREATE       = 6,
	CECMD_TABSCHANGED    = 7,
	CECMD_CMDSTARTED     = 8, // == CECMD_SETSIZE + ������������ ���������� ������� (���������� comspec)
	CECMD_CMDFINISHED    = 9, // == CECMD_SETSIZE + ��������� ���������� ������� (���������� comspec)
	CECMD_GETOUTPUTFILE  = 10, // �������� ����� ��������� ���������� ��������� �� ��������� ���� � ������� ��� ���
	CECMD_GETOUTPUT      = 11,
	CECMD_LANGCHANGE     = 12,
	CECMD_NEWCMD         = 13, // ��������� � ���� ���������� ����� ������� � ���������� �������� (������������ ��� SingleInstance)
	CECMD_TABSCMD        = 14, // enum ConEmuTabCommand: 0: ��������/�������� ����, 1: ������� �� ���������, 2: ������� �� ����������, 3: commit switch
	CECMD_RESOURCES      = 15, // ���������� �������� ��� ������������� (��������� ��������)
	CECMD_GETNEWCONPARM  = 16, // ���.��������� ��� �������� ����� ������� (�����, ������,...)
	CECMD_SETSIZESYNC    = 17, // ���������, ���� (�� �������), ���� FAR ���������� ��������� ������� (�� ���� ����������)
	CECMD_ATTACH2GUI     = 18, // ��������� ����������� ������� (�����������) ������� � GUI. ��� ����������
	CECMD_FARLOADED      = 19, // ���������� �������� � ������
//	CECMD_SHOWCONSOLE    = 20, // � Win7 ������ ������ �������� ���� �������, ���������� � ������ �������������� -- �������� �� CECMD_POSTCONMSG & CECMD_SETWINDOWPOS
	CECMD_POSTCONMSG     = 21, // � Win7 ������ ������ �������� ��������� ���� �������, ���������� � ������ ��������������
//	CECMD_REQUESTCONSOLEINFO  = 22, // ���� CECMD_REQUESTFULLINFO
	CECMD_SETFOREGROUND  = 23,
	CECMD_FLASHWINDOW    = 24,
//	CECMD_SETCONSOLECP   = 25,
	CECMD_SAVEALIASES    = 26,
	CECMD_GETALIASES     = 27,
	CECMD_SETSIZENOSYNC  = 28, // ����� CECMD_SETSIZE. ���������� �� �������.
	CECMD_SETDONTCLOSE   = 29,
	CECMD_REGPANELVIEW   = 30,
	CECMD_ONACTIVATION   = 31, // ��� ��������� ������ ConsoleInfo->bConsoleActive
	CECMD_SETWINDOWPOS   = 32, // CESERVER_REQ_SETWINDOWPOS. //-V112
	CECMD_SETWINDOWRGN   = 33, // CESERVER_REQ_SETWINDOWRGN.
	CECMD_SETBACKGROUND  = 34, // CESERVER_REQ_SETBACKGROUND
	CECMD_ACTIVATECON    = 35, // CESERVER_REQ_ACTIVATECONSOLE
//	CECMD_ONSERVERCLOSE  = 35, // ���������� �� ConEmuC*.exe ����� ��������� � ������ �������
	CECMD_DETACHCON      = 36,
	CECMD_GUIMACRO       = 38, // CESERVER_REQ_GUIMACRO. ����� � ������ �������� ��������/������
	CECMD_ONCREATEPROC   = 39, // CESERVER_REQ_ONCREATEPROCESS
	CECMD_SRVSTARTSTOP   = 40, // {DWORD(1/101); DWORD(ghConWnd);}
	CECMD_SETFARPID      = 41, // ���������� � ������, ����� �� ������ (CESERVER_CONSOLE_MAPPING_HDR.nFarPID)
	CECMD_ASSERT         = 42, // ���������� Assert � ConEmu. In=wchar_t[], Out=DWORD.
	CECMD_GUICHANGED     = 43, // ���������� � ������, ����� �� ������� � ���� ConEmuGuiMapping->CESERVER_CONSOLE_MAPPING_HDR
	CECMD_PEEKREADINFO   = 44, // CESERVER_REQ_PEEKREADINFO: ���������� � GUI �� ������� Debug
	CECMD_TERMINATEPID   = 45,
	CECMD_ATTACHGUIAPP   = 46, // CESERVER_REQ_ATTACHGUIAPP
	CECMD_KEYSHORTCUTS   = 47, // BYTE Data[2]; SetConsoleKeyShortcuts from kernel32.dll
	CECMD_SETFOCUS       = 48, // CESERVER_REQ_SETFOCUS
	CECMD_SETPARENT      = 49, // CESERVER_REQ_SETPARENT
	CECMD_CTRLBREAK      = 50, // GenerateConsoleCtrlEvent(dwData[0], dwData[1])
	CECMD_GETCGUINFO     = 51, // ConEmuGuiMapping
	CECMD_SETGUIEXTERN   = 52, // dwData[0]==TRUE - ������� ���������� ������ �� ������� ConEmu, dwData[0]==FALSE - ������� �� �������
/** ������� FAR ������� **/
	CMD_FIRST_FAR_CMD    = 200,
	CMD_DRAGFROM         = 200,
	CMD_DRAGTO           = 201,
	CMD_REQTABS          = 202,
	CMD_SETWINDOW        = 203,
	CMD_POSTMACRO        = 204, // ���� ������ ������ ������� '@' � ����� ���� �� ������ - ������ ����������� � DisabledOutput
//	CMD_DEFFONT          = 205,
	CMD_LANGCHANGE       = 206,
	CMD_FARSETCHANGED    = 207, // ���������� ��������� ��� ���� (isFARuseASCIIsort, isShellNoZoneCheck, ...)
//	CMD_SETSIZE          = 208,
	CMD_EMENU            = 209,
	CMD_LEFTCLKSYNC      = 210,
	CMD_REDRAWFAR        = 211,
	CMD_FARPOST          = 212,
	CMD_CHKRESOURCES     = 213,
//	CMD_QUITFAR          = 214, // ������� ���������� ������� (����?)
	CMD_CLOSEQSEARCH     = 215,
//	CMD_LOG_SHELL        = 216,
	CMD_SET_CON_FONT     = 217, // CESERVER_REQ_SETFONT
	CMD_GUICHANGED       = 218, // CESERVER_REQ_GUICHANGED. ���������� ��������� GUI (�����), ������ ���� ConEmu, ��� ��� ���-��
	CMD_ACTIVEWNDTYPE    = 219, // ThreadSafe - �������� ���������� �� �������� ���� (��� ����) � Far
	CMD_OPENEDITORLINE   = 220, // CESERVER_REQ_FAREDITOR. ������� � ��������� ���� � ������� �� ������ (��������� ������ �����������)
	CMD_LAST_FAR_CMD     = CMD_OPENEDITORLINE;


#define PIPEBUFSIZE 4096
#define DATAPIPEBUFSIZE 40000

#define MOD_LALT         0x0010
#define MOD_RALT         0x0020
#define MOD_LCONTROL     0x0040
#define MOD_RCONTROL     0x0080


//// ������� FAR �������
//typedef DWORD FARCMD;
//const FARCMD
//	CMD_DRAGFROM       = 0,
//	CMD_DRAGTO         = 1,
//	CMD_REQTABS        = 2,
//	CMD_SETWINDOW      = 3,
//	CMD_POSTMACRO      = 4, // ���� ������ ������ ������� '@' � ����� ���� �� ������ - ������ ����������� � DisabledOutput
////	CMD_DEFFONT        = 5,
//	CMD_LANGCHANGE     = 6,
//	CMD_FARSETCHANGED  = 7, // ���������� ��������� ��� ���� (isFARuseASCIIsort, isShellNoZoneCheck, ...)
////	CMD_SETSIZE        = 8,
//	CMD_EMENU          = 9,
//	CMD_LEFTCLKSYNC    = 10,
//	CMD_REDRAWFAR      = 11,
//	CMD_FARPOST        = 12,
//	CMD_CHKRESOURCES   = 13,
////	CMD_QUITFAR        = 14, // ������� ���������� ������� (����?)
//	CMD_CLOSEQSEARCH   = 15,
////	CMD_LOG_SHELL      = 16,
//	CMD_SET_CON_FONT   = 17, // CESERVER_REQ_SETFONT
//	CMD_GUICHANGED     = 18, // CESERVER_REQ_GUICHANGED. ���������� ��������� GUI (�����), ������ ���� ConEmu, ��� ��� ���-��
//	CMD_ACTIVEWNDTYPE  = 19; // ThreadSafe - �������� ���������� �� �������� ���� (��� ����) � Far
//// +2
//	//MAXCMDCOUNT        = 21;
//    //CMD_EXIT           = MAXCMDCOUNT-1;



//#pragma pack(push, 1)
#include <pshpack1.h>

typedef unsigned __int64 u64;

struct HWND2
{
	DWORD u;
	operator HWND() const
	{
		return (HWND)(DWORD_PTR)u; //-V204
	};
	operator DWORD() const
	{
		return (DWORD)u;
	};
	struct HWND2& operator=(HWND h)
	{
		u = (DWORD)(DWORD_PTR)h; //-V205
		return *this;
	};
};

// ��� ���������� x86/x64. ������� ����� �������� HKEY ������.
// ������������ ������ ��� "����������������" ������ ���� HKEY_USERS
struct HKEY2
{
	DWORD u;
	operator HKEY() const
	{
		return (HKEY)(DWORD_PTR)(LONG)u;
	};
	operator DWORD() const
	{
		return u;
	};
	struct HKEY2& operator=(HKEY h)
	{
		//_ASSERTE(((DWORD_PTR)h) < 0x100000000LL);
		u = (DWORD)(LONG)(LONG_PTR)h;
		return *this;
	};
};

struct HANDLE2
{
	u64 u;
	operator HANDLE() const
	{
		return (HANDLE)(DWORD_PTR)u;
	};
	operator DWORD_PTR() const
	{
		return (DWORD_PTR)u;
	};
	struct HANDLE2& operator=(HANDLE h)
	{
		u = (DWORD_PTR)h;
		return *this;
	};
};

struct MSG64
{
	UINT  message;
	HWND2 hwnd;
	u64   wParam;
	u64   lParam;
};

struct ThumbColor
{
	union
	{
		struct
		{
			unsigned int   ColorRGB : 24;
			unsigned int   ColorIdx : 5; // 5 bits, to support value '16' - automatic use of panel color
			unsigned int   UseIndex : 1; // TRUE - Use ColorRef, FALSE - ColorIdx
		};
		DWORD RawColor;
	};
};

struct ThumbSizes
{
	// ������� ��������� ��� ������
	int nImgSize; // Thumbs: 96, Tiles: 48
	// ����� ���������/������ ������&���� ������������ ������� ���� �����
	int nSpaceX1, nSpaceY1; // Thumbs: 1x1, Tiles: 4x4
	// ������ "�������" ������&���� ����� ���������/������. ����� �������� �����.
	int nSpaceX2, nSpaceY2; // Thumbs: 5x25, Tiles: 172x4
	// ���������� ����� ����������/������� � �������
	int nLabelSpacing; // Thumbs: 0, Tiles: 4
	// ������ ������ �� ����� �������������� �����
	int nLabelPadding; // Thumbs: 0, Tiles: 1
	// �����
	wchar_t sFontName[36]; // Tahoma
	int nFontHeight; // 14
};


enum PanelViewMode
{
	pvm_None = 0,
	pvm_Thumbnails = 1,
	pvm_Tiles = 2,
	// ��������� ����� (���� �� �����) ������ 4! (��� bitmask)
};

/* �������� ����� � GUI */
struct ConEmuMainFont
{
	wchar_t sFontName[32];
	DWORD nFontHeight, nFontWidth, nFontCellWidth;
	DWORD nFontQuality, nFontCharSet;
	BOOL Bold, Italic;
	wchar_t sBorderFontName[32];
	DWORD nBorderFontWidth;
};

struct PanelViewSetMapping
{
	DWORD cbSize; // Struct size, �� ������ ������

	/* ����� � ����� */
	ThumbColor crBackground; // ��� ���������: RGB ��� Index

	int nPreviewFrame; // 1 (����� ����� ������ ���������
	ThumbColor crPreviewFrame; // RGB ��� Index

	int nSelectFrame; // 1 (����� ������ �������� ��������)
	ThumbColor crSelectFrame; // RGB ��� Index

	/* ������������ */
	DWORD nFontQuality;

	/* ������ ������������� ������� */
	ThumbSizes Thumbs;
	ThumbSizes Tiles;

	/* �������� ����� � GUI */
	struct ConEmuMainFont MainFont;

	// ������ ��������� ��������
	BYTE  bLoadPreviews; // bitmask of PanelViewMode {1=Thumbs, 2=Tiles}
	bool  bLoadFolders;  // true - load infolder previews (only for Thumbs)
	DWORD nLoadTimeout;  // 15 sec

	DWORD nMaxZoom; // 600%
	bool  bUsePicView2; // true
	bool  bRestoreOnStartup;


	// ����� ������ ����� �����!
	COLORREF crPalette[16], crFadePalette[16];


	//// ���� �� ������������
	//DWORD nCacheFolderType; // ����/���������/temp/� �.�.
	//wchar_t sCacheFolder[MAX_PATH];
};


typedef BOOL (WINAPI* PanelViewInputCallback)(HANDLE hInput, PINPUT_RECORD lpBuffer, DWORD nBufSize, LPDWORD lpNumberOfEventsRead, BOOL* pbResult);
typedef union uPanelViewInputCallback
{
	u64 Reserved; // ���������� ��� ������������ �������� ��� x64 <--> x86
	PanelViewInputCallback f; //-V117
} PanelViewInputCallback_t;
typedef BOOL (WINAPI* PanelViewOutputCallback)(HANDLE hOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion);
typedef union uPanelViewOutputCallback
{
	u64 Reserved; // ���������� ��� ������������ �������� ��� x64 <--> x86
	PanelViewOutputCallback f; //-V117
} PanelViewOutputCallback_t;
struct PanelViewText
{
	// ���� ��������������, ������������ ������, � ��.
#define PVI_TEXT_NOTUSED 0
#define PVI_TEXT_LEFT    1
#define PVI_TEXT_CENTER  2
#define PVI_TEXT_RIGHT   4
#define PVI_TEXT_SKIPSORTMODE 0x100 // ������ ��� ����� ������� (�������� ��������� ������� ���������� � ^)
	DWORD nFlags;
	// ������������ ������ ������� ���� - ������� ���������� ������
	DWORD bConAttr;
	// ���������� �����
	wchar_t sText[128];
};
struct PanelViewInit
{
	DWORD cbSize;
	BOOL  bRegister, bVisible;
	HWND2 hWnd;
	BOOL  bLeftPanel;
	// Flags
#define PVI_COVER_NORMAL         0x000 // ��� ������, ����������� ���������������, � ������� ��� ������� �����
#define PVI_COVER_SCROLL_CLEAR   0x001 // ��� ��������� GUI ������� ������ ��������� (������� �� ������������-�������)
#define PVI_COVER_SCROLL_OVER    0x002 // PVI_COVER_NORMAL + ������ ����� (��� ������ ���������). ������ ������ ���������� ��������� ���
#define PVI_COVER_COLTITLE_OVER  0x004 // PVI_COVER_NORMAL + ������ � ����������� ������� (���� ��� ����)
#define PVI_COVER_FULL_OVER      0x008 // ��� ������� ������� ������ (�� ���� ������ - �����)
#define PVI_COVER_FRAME_OVER     0x010 // ��� ������� ������ (������� �����)
#define PVI_COVER_2PANEL_OVER    0x020 // ��� ������ (�������������)
#define PVI_COVER_CONSOLE_OVER   0x040 // ������� �������
	DWORD nCoverFlags;
	// FAR settings
	DWORD nFarInterfaceSettings;
	DWORD nFarPanelSettings;
	// ���������� ���� ������ (�����, ������, ��� fullscreen)
	RECT  PanelRect;
	// ������������� ��������� ���������
	PanelViewText tPanelTitle, tColumnTitle, tInfoLine[3];
	// ��� ���������� ��������������, � ������� ������� ������������� View
	// ���������� ������������ � GUI. ������� - ����������.
	RECT  WorkRect;
	// Callbacks, ������������ ������ � ��������
	PanelViewInputCallback_t pfnPeekPreCall, pfnPeekPostCall, pfnReadPreCall, pfnReadPostCall;
	PanelViewOutputCallback_t pfnWriteCall;
	/* out */
	//PanelViewSetMapping ThSet;
	BOOL bFadeColors;
};



// �������, ��� ������� ���������� ������� ���������� ��� ����������� ����
enum PaintBackgroundEvents
{
	pbe_Common = 0,              // ��� ��������� ������� ���� ConEmu/������� ������/������� ConEmu/...
	pbe_PanelDirectory = 1,      // ��� ����� �����
	//TODO, by request
	//bue_LeftDirChanged = 1,      // �� ����� ������ �������� ������� �����
	//bue_RightDirChanged = 2,     // �� ������ ������ �������� ������� �����
	//bue_FocusChanged = 4,        // ����� ������������ ����� �����/������ �������
};

enum PaintBackgroundPlaces
{
	// ������������ ��� ����������� � ���������
	pbp_Panels = 1,
	pbp_Editor = 2,
	pbp_Viewer = 4,
	// ������������ ������ ��� [OUT] ��� ��������� // Reserved
	pbp_UserScreen  = 8,
	pbp_CommandLine = 16,
	pbp_KeyBar      = 32,
	pbp_MenuBar     = 64, // ������� ������ - �������� ����
	pbp_StatusLine  = 128, // ��������� ������ - ��������/������
	// ���� ��� ������������ ��� ���������� ���� ���������
	pbp_Finalize    = 0x80000000,
};

#define BkPanelInfo_CurDirMax 32768
#define BkPanelInfo_FormatMax MAX_PATH
#define BkPanelInfo_HostFileMax 32768

struct PaintBackgroundArg
{
	DWORD cbSize;

	// ������ ��� ������ RegisterBackground(rbc_Register)
	LPARAM lParam;

	// ������/��������/������
	enum PaintBackgroundPlaces Place;

	// ����, � ������� ������ ������. �� ����, ��� ���������� �����, 0-based
	// ���� (dwLevel > 0) ������ �� ������ �������� ��� �������.
	DWORD dwLevel;
	// [Reserved] ���������� �� enum PaintBackgroundEvents
	DWORD dwEventFlags;

	// �������� ����� � ConEmu GUI
	struct ConEmuMainFont MainFont;
	// ������� � ConEmu GUI
	COLORREF crPalette[16];
	// Reserved
	DWORD dwReserved[20];


	// DC ��� ��������� ����. ���������� (��� dwLevel==0) DC ����� ������ ���� crPalette[0]
	HDC hdc;
	// ������ DC � ��������
	int dcSizeX, dcSizeY;
	// ���������� ������� � DC (base 0x0)
	RECT rcDcLeft, rcDcRight;


	// ��� ���������� ����� �������� - ������� ��������� FAR
	RECT rcConWorkspace; // ��������� ������� ������� FAR. � FAR 2 � ������ /w ���� ����� ���� != {0,0}
	COORD conCursor; // ��������� �������, ��� {-1,-1} ���� �� �� �����
	DWORD nFarInterfaceSettings; // ACTL_GETINTERFACESETTINGS
	DWORD nFarPanelSettings; // ACTL_GETPANELSETTINGS
	BYTE nFarColors[col_LastIndex]; // ������ ������ ����

	// ��������� � �������
	BOOL bPanelsAllowed;
	typedef struct tag_BkPanelInfo
	{
		BOOL bVisible;   // ������� ������
		BOOL bFocused;   // � ������
		BOOL bPlugin;    // ���������� ������
		int  nPanelType; // enum PANELINFOTYPE
		wchar_t *szCurDir/*[32768]*/;    // ������� ����� �� ������
		wchar_t *szFormat/*[MAX_PATH]*/; // �������� ������ � FAR2, � FAR3 ��� ����� ���� �������, ���� "������" �������� �� ��������
		wchar_t *szHostFile/*[32768]*/;  // �������� ������ � FAR2
		RECT rcPanelRect; // ���������� ��������� ������. � FAR 2 � ������ /w ���� ����� ���� != {0,0}
	} BkPanelInfo;
	BkPanelInfo LeftPanel;
	BkPanelInfo RightPanel;

	// [OUT] ������ ������ �������, ����� ����� ������� �� "���������" - enum PaintBackgroundPlaces
	DWORD dwDrawnPlaces;
};

typedef int (WINAPI* PaintConEmuBackground_t)(struct PaintBackgroundArg* pBk);

// ���� ������� ������ 0 - ���������� ���� ���� �� ���������
typedef int (WINAPI* BackgroundTimerProc_t)(LPARAM lParam);

enum RegisterBackgroundCmd
{
	rbc_Register   = 1, // ��������� ����������� "��������" �������
	rbc_Unregister = 2, // ������ ������ �� ������ "�������"
	rbc_Redraw     = 3, // ���� ������� ��������� �������� ��� - �� ����� ��� �������
};

// �������� ��� �������: int WINAPI RegisterBackground(RegisterBackgroundArg *pbk)
struct RegisterBackgroundArg
{ //-V802
	DWORD cbSize;
	enum RegisterBackgroundCmd Cmd; // DWORD
	HMODULE hPlugin; // Instance �������, ����������� PaintConEmuBackground

	// ��� ������������� ���� �������� ���������� ������� {sizeof(RegisterBackgroundArg), rbc_Unregister, hPlugin}

	// ��� �������� ��� ���������� ����.
	// ��������� ��������� ������ ��� Cmd==rbc_Register,
	// � ��������� ������� ������� ������������

	// ������ ����� ���������������� ��������� ��������� ��� {PaintConEmuBackground,lParam}
	PaintConEmuBackground_t PaintConEmuBackground; // ���������� ������
	LPARAM lParam; // lParam ����� ������� � PaintConEmuBackground

	DWORD  dwPlaces;     // bitmask of PaintBackgroundPlaces
	DWORD  dwEventFlags; // bitmask of PaintBackgroundEvents

	// 0 - ������ ������������ ������������ ��� ������. ��� ������ nSuggestedLevel
	// ��� ������� ����� ���� ������ ������ ��� ���������� ����
	DWORD  dwSuggestedLevel;

	// �������������� ������ �������
	BackgroundTimerProc_t BackgroundTimerProc;
	// ��������������� ������� ������ (ms)
	DWORD nBackgroundTimerElapse;
};

typedef int (WINAPI* RegisterBackground_t)(RegisterBackgroundArg *pbk);

typedef void (WINAPI* SyncExecuteCallback_t)(LONG_PTR lParam);


struct FarVersion
{
	union
	{
		DWORD dwVer;
		struct
		{
			WORD dwVerMinor;
			WORD dwVerMajor;
		};
	};
	DWORD dwBuild;
};


// �������� ��� ������� OnConEmuLoaded
struct ConEmuLoadedArg
{
	DWORD cbSize;    // ������ ��������� � ������
	DWORD nBuildNo;  // {����� ������ ConEmu*10} (i.e. 1012070). ��� ������ �������. � ��������, ������ GUI ����� ����������
	                 // YYMMDDX (YY - ��� ����� ����, MM - �����, DD - ����, X - 0 � ����-����� ���������)
	HMODULE hConEmu; // conemu.dll / conemu.x64.dll
	HMODULE hPlugin; // ��� ���������� - Instance ����� �������, � ������� ���������� OnConEmuLoaded
	BOOL bLoaded;    // TRUE - ��� �������� conemu.dll, FALSE - ��� ��������
	BOOL bGuiActive; // TRUE - ������� �������� ��-��� ConEmu, FALSE - standalone

	/* ***************** */
	/* ��������� ������� */
	/* ***************** */
	HWND (WINAPI *GetFarHWND)();
	HWND (WINAPI *GetFarHWND2)(BOOL abConEmuOnly);
	void (WINAPI *GetFarVersion)(FarVersion* pfv);
	BOOL (WINAPI *IsTerminalMode)();
	BOOL (WINAPI *IsConsoleActive)();
	int (WINAPI *RegisterPanelView)(PanelViewInit *ppvi);
	int (WINAPI *RegisterBackground)(RegisterBackgroundArg *pbk);
	// Activate console tab in ConEmu
	int (WINAPI *ActivateConsole)();
	// Synchronous execute CallBack in Far main thread (FAR2 use ACTL_SYNCHRO).
	// SyncExecute does not returns, until CallBack finished.
	// ahModule must be module handle, wich contains CallBack
	int (WINAPI *SyncExecute)(HMODULE ahModule, SyncExecuteCallback_t CallBack, LONG_PTR lParam);
};

// ������ ������� ����� �������������� ������� "OnConEmuLoaded"
// ��� ��������� ������� ConEmu (��� ������� ���� ��-��� ConEmu)
// ��� ������� ("OnConEmuLoaded") ����� �������, � ��� ������ �����
// ���������������� ������� ��� ���������� Background-��
// ��������!!! ��� �� ������� ���������� ��� �������� conemu.dll,
// ��� �������� hConEmu � ��� ������� == NULL
typedef void (WINAPI* OnConEmuLoaded_t)(struct ConEmuLoadedArg* pConEmuInfo);

enum GuiLoggingType
{
	glt_None = 0,
	glt_Processes = 1,
	glt_Input = 2,
	// glt_Keyboard, glt_Files, ...
};

struct ConEmuGuiMapping
{
	DWORD    cbSize;
	DWORD    nProtocolVersion; // == CESERVER_REQ_VER
	HWND2    hGuiWnd; // �������� (��������) ���� ConEmu
	
	DWORD    nLoggingType; // enum GuiLoggingType
	
	wchar_t  sConEmuExe[MAX_PATH+1]; // ������ ���� � ConEmu.exe (GUI)
	wchar_t  sConEmuDir[MAX_PATH+1]; // ��� ������������ �����. ����� �������� ConEmu.exe
	wchar_t  sConEmuBaseDir[MAX_PATH+1]; // ��� ������������ �����. ����� �������� ConEmuC.exe, ConEmuHk.dll, ConEmu.xml
	wchar_t  sConEmuArgs[MAX_PATH*2];

	DWORD    bUseInjects; // 0-off, 1-on. ����� ����� ���� ���.����� (�������)? chcp, Hook HKCU\FAR[2] & HKLM\FAR and translate them to hive, ...
	BOOL     bUseTrueColor; // ������� ������ "TrueMod support"

	/* �������� ����� � GUI */
	struct ConEmuMainFont MainFont;
	
	// �������� �������
	DWORD   isHookRegistry; // bitmask. 1 - supported, 2 - current
	wchar_t sHiveFileName[MAX_PATH];
	HKEY2   hMountRoot;  // NULL ��� Vista+, ��� Win2k&XP ����� �������� �������� ���� (HKEY_USERS), � ������� �������� hive
	wchar_t sMountKey[MAX_PATH]; // ��� Win2k&XP ����� �������� ��� �����, � ������� �������� hive
	
	// DosBox
	BOOL     bDosBox; // ������� DosBox
	//wchar_t  sDosBoxExe[MAX_PATH+1]; // ������ ���� � DosBox.exe
	//wchar_t  sDosBoxEnv[8192]; // ������� �������� (mount, � ��.)

	//DWORD    bUseInjects; // 0-off, 1-on. ����� ����� ���� ���.����� (�������)? chcp, Hook HKCU\FAR[2] & HKLM\FAR and translate them to hive, ...
	//wchar_t  sInjectsDir[MAX_PATH+1]; // path to "conemu.dll" & "conemu.x64.dll"
	// ��� ���������� Inject-�� �������� ����� ����� ���� � ���������� ������ ���������.
	//wchar_t  sInjects32[MAX_PATH+16], sInjects64[MAX_PATH+16]; // path to "ConEmuHk.dll" & "ConEmuHk64.dll"
	// Kernel32 ����������� �� �������������� ������, ��
	// ��� 64-������ ��������� �� ����, � ��� 32-������ ���-�� ������.
	// ������� � 64-������ �������� ���������� ������������ 64-������ ������� ConEmu.exe
	// ������� ������ ��������� ���������� ����� ��� 64-������� kernel,
	// � ����� 32-������� kernel ������ �������� ����� ��� ��������.
	//ULONGLONG ptrLoadLib32, ptrLoadLib64;
	//// ����������� CreateProcess, ShellExecute, � ������ ����������� �������
	//// ���� ����� - �� ����������
	//wchar_t  sLogCreateProcess[MAX_PATH];
};


//TODO("Restrict CONEMUTABMAX to 128 chars. Only filename, and may be ellipsed...");
#define CONEMUTABMAX 0x400
struct ConEmuTab
{
	int  Pos;
	int  Current;
	int  Type; // (Panels=1, Viewer=2, Editor=3) | (Elevated=0x100)
	int  Modified;
	int  EditViewId;
	wchar_t Name[CONEMUTABMAX];
	//  int  Modified;
	//  int isEditor;
};

struct CESERVER_REQ_CONEMUTAB
{
	DWORD nTabCount;
	BOOL  bMacroActive;
	BOOL  bMainThread;
	int   CurrentType; // WTYPE_PANELS / WTYPE_VIEWER / WTYPE_EDITOR
	int   CurrentIndex; // ��� ��������, ������ �������� ���� � tabs
	ConEmuTab tabs[1];
};

struct CESERVER_REQ_CONEMUTAB_RET
{
	BOOL  bNeedPostTabSend;
	BOOL  bNeedResize;
	COORD crNewSize;
};

struct ForwardedPanelInfo
{
	RECT ActiveRect;
	RECT PassiveRect;
	int ActivePathShift; // ����� � ���� ��������� � ������
	int PassivePathShift; // ����� � ���� ��������� � ������
	BOOL NoFarConsole;
	union //x64 ready
	{
		WCHAR* pszActivePath/*[MAX_PATH+1]*/; //-V117
		u64 Reserved1;
	};
	union //x64 ready
	{
		WCHAR* pszPassivePath/*[MAX_PATH+1]*/; //-V117
		u64 Reserved2;
	};
};

struct ForwardedFileInfo
{
	WCHAR Path[MAX_PATH+1];
};


struct CESERVER_REQ_HDR
{
	DWORD   cbSize;     // �� size_t(!), � ������ DWORD, �.�. �������� ������������ � 32<->64 ��� ����� �����.
	CECMD   nCmd;       // DWORD
	DWORD   nVersion;
	DWORD   nSrcThreadId;
	DWORD   nSrcPID;
	DWORD   nCreateTick;
	DWORD   nBits;      // �������� ����������� ��������
	DWORD   nLastError; // ��������� GetLastError() ��� �������� ������
	u64     hModule;
};

#define CHECK_CMD_SIZE(pCmd,data_size) ((pCmd)->hdr.cbSize >= (sizeof(CESERVER_REQ_HDR) + data_size))


struct CESERVER_CHAR_HDR
{
	int   nSize;    // ������ ��������� ������������. ���� 0 - ������ ������������� is NULL
	COORD cr1, cr2; // WARNING: ��� ���������� ���������� (��� ����� ���������), � �� ��������.
};

struct CESERVER_CHAR
{
	CESERVER_CHAR_HDR hdr; // ������������� �����
	WORD  data[2];  // variable(!) length
};

struct CESERVER_CONSAVE_HDR
{
	CESERVER_REQ_HDR hdr; // ����������� ������ ����� �������� � ������
	CONSOLE_SCREEN_BUFFER_INFO sbi;
	DWORD cbMaxOneBufferSize;
};

struct CESERVER_CONSAVE
{
	CESERVER_CONSAVE_HDR hdr;
	wchar_t Data[1];
};



struct CESERVER_REQ_RGNINFO
{
	DWORD dwRgnInfoSize;
	CESERVER_CHAR RgnInfo;
};

struct CESERVER_REQ_FULLCONDATA
{
	DWORD dwRgnInfoSize_MustBe0; // must be 0
	DWORD dwOneBufferSize; // may be 0
	wchar_t Data[300]; // Variable length!!!
};

struct CEFAR_SHORT_PANEL_INFO
{
	int   PanelType;
	int   Plugin;
	RECT  PanelRect;
	//int   ItemsNumber;
	//int   SelectedItemsNumber;
	//int   CurrentItem;
	//int   TopPanelItem;
	int   Visible;
	int   Focus;
	//int   ViewMode;
	//int   ShortNames;
	//int   SortMode;
	int   ColumnNames;
	int   StatusLines;
	unsigned __int64 Flags;
};

struct CEFAR_PANELTABS
{
	int   SeparateTabs; // ���� -1 - �� ���������
	int   ButtonColor;  // ���� -1 - �� ���������
};

typedef unsigned int CEFAR_MACRO_AREA;
static const CEFAR_MACRO_AREA
	fma_Unknown  = 0x00000000,
	fma_Panels   = 0x00000001,
	fma_Viewer   = 0x00000002,
	fma_Editor   = 0x00000003,
	fma_Dialog   = 0x00000004 //-V112
	;


WARNING("CEFAR_INFO_MAPPING.nFarColors - subject to change!!!");
struct CEFAR_INFO_MAPPING
{
	DWORD cbSize;
	DWORD nFarInfoIdx;
	FarVersion FarVer;
	DWORD nProtocolVersion; // == CESERVER_REQ_VER
	DWORD nFarPID, nFarTID;
	BYTE nFarColors[col_LastIndex]; // ������ ������ ����
	DWORD nFarInterfaceSettings;
	DWORD nFarPanelSettings;
	DWORD nFarConfirmationSettings;
	BOOL  bFarPanelAllowed; // FCTL_CHECKPANELSEXIST
	// ������� ������� � ����
	CEFAR_MACRO_AREA nMacroArea;
	BOOL bMacroActive;
	// ���������� ���������� � ������� �������������� �� �������
	BOOL bFarPanelInfoFilled;
	BOOL bFarLeftPanel, bFarRightPanel;
	CEFAR_SHORT_PANEL_INFO FarLeftPanel, FarRightPanel; // FCTL_GETPANELSHORTINFO,...
	BOOL bViewerOrEditor; // ��� ���������� ����� RgnDetect
	DWORD nFarConsoleMode;
	BOOL bBufferSupport; // FAR2 � ������ /w ?
	CEFAR_PANELTABS PanelTabs; // ��������� ������� PanelTabs
	//DWORD nFarReadIdx;    // index, +1, ����� ��� � ��������� ��� ������ (Read|Peek)ConsoleInput ��� GetConsoleInputCount
	// ����� ���� ��������� �������, �� ������� � ��������� ������� ������������� GUI
	wchar_t sLngEdit[64]; // "edit"
	wchar_t sLngView[64]; // "view"
	wchar_t sLngTemp[64]; // "{Temporary panel"
	//wchar_t sLngName[64]; // "Name"
	wchar_t sReserved[MAX_PATH]; // ����� �������� GetMsg �� ����������� ��������� �� �����
};


// CECONMAPNAME
struct CESERVER_CONSOLE_MAPPING_HDR
{
	DWORD cbSize;
	DWORD nLogLevel;
	COORD crMaxConSize;
	DWORD bDataReady;  // TRUE, ����� ���� ��� ������ ������� ���������� � ����� �������� ������
	HWND2 hConWnd;     // !! ��������� hConWnd � nGuiPID �� ������ !!
	DWORD nServerPID;  //
	DWORD nGuiPID;     // !! �� ��� ������������� PicViewWrapper   !!
	//
	DWORD bConsoleActive;
	DWORD nProtocolVersion; // == CESERVER_REQ_VER
	DWORD bThawRefreshThread; // FALSE - ����������� �������� ������ ������� (GUI ������ �����)
	//
	DWORD nActiveFarPID; // PID ���������� ��������� ����
	//
	DWORD nServerInShutdown; // GetTickCount() ������ �������� �������
	//
	wchar_t  sConEmuExe[MAX_PATH+1]; // ������ ���� � ConEmu.exe (GUI)
	wchar_t  sConEmuBaseDir[MAX_PATH+1]; // ��� ������������ �����. ����� �������� ConEmuC.exe, ConEmuHk.dll, ConEmu.xml
	//
	//DWORD bUseInjects; // 0-off, 1-on. ����� ����� ���� ���.����� (�������)? chcp, Hook HKCU\FAR[2] & HKLM\FAR and translate them to hive, ...
	//wchar_t  sConEmuDir[MAX_PATH+1];  // ����� ����� ������ ���������� hive
	//wchar_t  sInjectsDir[MAX_PATH+1]; // path to "ConEmuHk.dll" & "ConEmuHk64.dll"
	DWORD nAltServerPID;  //

	// Root(!) ConEmu window
	HWND2 hConEmuRoot;
	// DC ConEmu window
	HWND2 hConEmuWnd;

	DWORD nLoggingType; // enum GuiLoggingType
	BOOL  bDosBox; // DosBox ����������, ����� ������������
	DWORD bUseInjects; // 0-off, 1-on. ����� ����� ���� ���.����� (�������)? chcp, Hook HKCU\FAR[2] & HKLM\FAR and translate them to hive, ...
	BOOL  bUseTrueColor; // ������� ������ "TrueMod support"
	
	// �������� �������
	DWORD   isHookRegistry; // bitmask. 1 - supported, 2 - current
	wchar_t sHiveFileName[MAX_PATH];
	HKEY2   hMountRoot;  // NULL ��� Vista+, ��� Win2k&XP ����� �������� �������� ���� (HKEY_USERS), � ������� �������� hive
	wchar_t sMountKey[MAX_PATH]; // ��� Win2k&XP ����� �������� ��� �����, � ������� �������� hive

	//// ����������� CreateProcess, ShellExecute, � ������ ����������� �������
	//// ���� ����� - �� ����������
	//wchar_t  sLogCreateProcess[MAX_PATH];
};

struct CESERVER_REQ_CONINFO_INFO
{
	CESERVER_REQ_HDR cmd;
	//DWORD cbSize;
	HWND2 hConWnd;
	//DWORD nGuiPID;
	//DWORD nCurDataMapIdx; // ������� ��� �������� MAP ����� � �������
	//DWORD nCurDataMaxSize; // ������������ ������ ������ nCurDataMapIdx
	DWORD nPacketId;
	//DWORD nFarReadTick; // GetTickCount(), ����� ��� � ��������� ��� �������� ������� �� �������
	//DWORD nFarUpdateTick0;// GetTickCount(), ��������������� � ������ ���������� ������� �� ���� (����� ��� ��������...)
	//DWORD nFarUpdateTick; // GetTickCount(), ����� ������� ���� ��������� � ��������� ��� �� ����
	//DWORD nFarReadIdx;    // index, +1, ����� ��� � ��������� ��� ������ (Read|Peek)ConsoleInput ��� GetConsoleInputCount
	DWORD nSrvUpdateTick; // GetTickCount(), ����� ������� ���� ������� � ��������� ��� � �������
	DWORD nReserved0; //DWORD nInputTID;
	DWORD nProcesses[20];
	DWORD dwCiSize;
	CONSOLE_CURSOR_INFO ci;
	DWORD dwConsoleCP;
	DWORD dwConsoleOutputCP;
	DWORD dwConsoleMode;
	DWORD dwSbiSize;
	CONSOLE_SCREEN_BUFFER_INFO sbi;
	COORD crWindow; // ��� �������� - ������ ���� (�� ������) ��� ��������� ReadConsoleData
	COORD crMaxSize; // ������������ ������ ������� � �������� (��� �������� ���������� ������)
	DWORD nDataShift; // ��� �������� - ����� ������ ������ (data) �� ������ info
	DWORD nDataCount; // ��� �������� - ���������� ����� (data)
	//// ���������� � ������� FAR
	//DWORD nFarInfoIdx; // ������� �� ��������� CEFAR_INFO_MAPPING, �.�. �� ����� �������� � �������
	//CEFAR_INFO_MAPPING FarInfo;
};

//typedef struct tag_CESERVER_REQ_CONINFO_DATA {
//	CESERVER_REQ_HDR cmd;
//	COORD      crMaxSize;
//	CHAR_INFO  Buf[1];
//} CESERVER_REQ_CONINFO_DATA;

struct CESERVER_REQ_CONINFO_FULL
{
	DWORD cbMaxSize;    // ������ ����� ������ CESERVER_REQ_CONINFO_FULL (������ ����� ����� ������ �������� ������)
	//DWORD cbActiveSize; // ������ �������� ������ CESERVER_REQ_CONINFO_FULL, � �� ����� ������ data
	//BOOL  bChanged;     // ���� ����, ��� ������ ���������� � ��������� �������� � GUI
	BOOL  bDataChanged; // ������������ � TRUE, ��� ���������� ����������� ������� (� �� ������ ��������� �������...)
	CESERVER_CONSOLE_MAPPING_HDR  hdr;
	CESERVER_REQ_CONINFO_INFO info;
	//CESERVER_REQ_CONINFO_DATA data;
	CHAR_INFO  data[1];
};

//typedef struct tag_CESERVER_REQ_CONINFO {
//	CESERVER_CONSOLE_MAPPING_HDR inf;
//    union {
//	/* 9*/DWORD dwRgnInfoSize;
//	      CESERVER_REQ_RGNINFO RgnInfo;
//    /*10*/CESERVER_REQ_FULLCONDATA FullData;
//	};
//} CESERVER_REQ_CONINFO;

struct CESERVER_REQ_SETSIZE
{
	USHORT nBufferHeight; // 0 ��� ������ ������ (����� � ����������)
	COORD  size;
	SHORT  nSendTopLine;  // -1 ��� 0based ����� ������ ��������������� � GUI (������ ��� ������ � ����������)
	SMALL_RECT rcWindow;  // ���������� ������� ������� ��� ������ � ����������
	DWORD  dwFarPID;      // ���� �������� - ������ ������ ��� ����������� �� FAR'� � �������� ��� ������ ����� ������ ����� ���������
};

struct CESERVER_REQ_OUTPUTFILE
{
	BOOL  bUnicode;
	WCHAR szFilePathName[MAX_PATH+1];
};

struct CESERVER_REQ_RETSIZE
{
	DWORD nNextPacketId;
	CONSOLE_SCREEN_BUFFER_INFO SetSizeRet;
};

struct CESERVER_REQ_NEWCMD
{
	wchar_t szCurDir[MAX_PATH];
	wchar_t szCommand[MAX_PATH]; // �� ����� ���� - variable_size !!!
};

struct CESERVER_REQ_SETFONT
{
	DWORD cbSize; // ���������
	int inSizeY;
	int inSizeX;
	wchar_t sFontName[LF_FACESIZE];
};

// ���������� ��������� GUI (�����), ������ ���� ConEmu, ��� ��� ���-��
struct CESERVER_REQ_GUICHANGED
{
	DWORD cbSize; // ���������
	DWORD nGuiPID;
	HWND2 hLeftView, hRightView;
};

// CMD_OPENEDITORLINE. ������� � ��������� ���� � ������� �� ������ (��������� ������ �����������)
struct CESERVER_REQ_FAREDITOR
{
	DWORD cbSize; // ���������
	int nLine;
	wchar_t szFile[MAX_PATH+1];
};

enum StartStopType
{
	sst_ServerStart  = 0,
	sst_ServerStop   = 1,
	sst_ComspecStart = 2,
	sst_ComspecStop  = 3,
	sst_AppStart     = 4,
	sst_AppStop      = 5,
	sst_App16Start   = 6,
	sst_App16Stop    = 7,
};
struct CESERVER_REQ_STARTSTOP
{
	DWORD nStarted; // StartStopType
	HWND2 hWnd; // ��� �������� � GUI - �������, ��� �������� � ������� - GUI
	DWORD dwPID; //, dwInputTID;
	DWORD nSubSystem; // 255 ��� DOS ��������, 0x100 - ����� �� FAR �������
	DWORD nImageBits; // 16/32/64
	BOOL  bRootIsCmdExe;
	BOOL  bUserIsAdmin;
	// � ��� �������� �� �������, ������ ���������� ��������� ������ �������� ������ ������
	CONSOLE_SCREEN_BUFFER_INFO sbi;
	// ������������ ������ ������� �� ������� ������
	COORD crMaxSize;
	// ������ ComSpec
	BOOL  bWasBufferHeight;
	// ������ ��� ������. ����� ���� NULL-��
	u64   hServerProcessHandle;
	// ��� ���������� � �������� (GetModuleFileName(0))
	wchar_t sModuleName[MAX_PATH+1];
	// Reserved
	DWORD nReserved0[20];
	// CmdLine
	wchar_t sCmdLine[1]; // variable length
};

// CECMD_ONCREATEPROC
struct CESERVER_REQ_ONCREATEPROCESS
{
	//BOOL    bUnicode;
	int     nSourceBits;
	wchar_t sFunction[32];
	int     nImageSubsystem;
	int     nImageBits;
	DWORD   nShellFlags; // ����� ShellExecuteEx
	DWORD   nCreateFlags, nStartFlags; // ����� CreateProcess
	DWORD   nShowCmd;
	int     nActionLen;
	int     nFileLen;
	int     nParamLen;
	unsigned __int64 hStdIn, hStdOut, hStdErr;
	// Variable length tail
	wchar_t wsValue[1];
};
struct CESERVER_REQ_ONCREATEPROCESSRET
{
	BOOL  bContinue;
	//BOOL  bUnicode;
	BOOL  bForceBufferHeight;
	BOOL  bAllowDosbox;
	//int   nFileLen;
	//int   nBaseLen;
	//union
	//{
	//	wchar_t wsValue[1];
	//	char sValue[1];
	//};
};

// _ASSERTE(sizeof(CESERVER_REQ_STARTSTOPRET) <= sizeof(CESERVER_REQ_STARTSTOP));
struct CESERVER_REQ_STARTSTOPRET
{
	BOOL  bWasBufferHeight;
	HWND2 hWnd; // ��� �������� � ������� - GUI (������� ����)
	HWND2 hWndDC;
	DWORD dwPID; // ��� �������� � ������� - PID ConEmu.exe
	DWORD nBufferHeight, nWidth, nHeight;
	DWORD dwSrvPID;
	BOOL  bNeedLangChange;
	u64   NewConsoleLang;
	// ������������ ��� CECMD_ATTACH2GUI
	CESERVER_REQ_SETFONT Font;
};

struct CESERVER_REQ_POSTMSG
{
	BOOL    bPost;
	HWND2   hWnd;
	UINT    nMsg;
	// ��������� �� ���������� x86 & x64
	u64     wParam, lParam;
};

struct CESERVER_REQ_FLASHWINFO
{
	BOOL  bSimple;
	HWND2 hWnd;
	BOOL  bInvert; // ������ ���� bSimple == TRUE
	DWORD dwFlags; // � ��� � �����, ���� bSimple == FALSE
	UINT  uCount;
	DWORD dwTimeout;
};

// CMD_FARCHANGED - FAR plugin
struct FAR_REQ_FARSETCHANGED
{
	BOOL    bFARuseASCIIsort;
	BOOL    bShellNoZoneCheck; // ������� ��� SEE_MASK_NOZONECHECKS
	BOOL    bMonitorConsoleInput; // ��� (Read/Peek)ConsoleInput(A/W) ������� ���� � GUI/Settings/Debug
	//wchar_t szEnv[1]; // Variable length: <Name>\0<Value>\0<Name2>\0<Value2>\0\0
};

struct CESERVER_REQ_SETCONCP
{
	BOOL    bSetOutputCP; // [IN], [Out]=result
	DWORD   nCP;          // [IN], [Out]=LastError
};

struct CESERVER_REQ_SETWINDOWPOS
{
	HWND2 hWnd;
	HWND2 hWndInsertAfter;
	int X;
	int Y;
	int cx;
	int cy;
	UINT uFlags;
};

struct CESERVER_REQ_SETWINDOWRGN
{
	HWND2 hWnd;
	int   nRectCount;  // ���� 0 - �������� WindowRgn, ����� - ���������.
	BOOL  bRedraw;
	RECT  rcRects[20]; // [0] - �������� ����, [
};

// CECMD_SETBACKGROUND - �������� �� ������� "PanelColorer.dll"
// Warning! Structure has variable length. "bmp" field must be followed by bitmap data (same as in *.bmp files)
struct CESERVER_REQ_SETBACKGROUND
{
	int               nType;    // Reserved for future use. Must be 1
	BOOL              bEnabled; // TRUE - ConEmu use this image, FALSE - ConEmu use self background settings

	// ����� ����� ������� ���������� - enum PaintBackgroundPlaces
	DWORD dwDrawnPlaces;

	//int               nReserved1; // Must by 0. reserved for alpha
	//DWORD             nReserved2; // Must by 0. reserved for replaced colors
	//int               nReserved3; // Must by 0. reserved for background op
	//DWORD nReserved4; // Must by 0. reserved for flags (BmpIsTransparent, RectangleSpecified)
	//DWORD nReserved5; // Must by 0. reserved for level (Some plugins may want to draw small parts over common background)
	//RECT  rcReserved5; // Must by 0. reserved for filled rect (plugin may cover only one panel, or part of it)

	BITMAPFILEHEADER  bmp;
	BITMAPINFOHEADER  bi;
};

// ConEmu respond for CESERVER_REQ_SETBACKGROUND
typedef int SetBackgroundResult;
const SetBackgroundResult
	esbr_OK = 0,               // All OK
	esbr_InvalidArg = 1,       // Invalid *RegisterBackgroundArg
	esbr_PluginForbidden = 2,  // "Allow plugins" unchecked in ConEmu settings ("Main" page)
	esbr_ConEmuInShutdown = 3, // Console is closing. This is not an error, just information
	esbr_Unexpected = 4,       // Unexpected error in ConEmu //-V112
	esbr_InvalidArgSize = 5,   // Invalid RegisterBackgroundArg.cbSize
	esbr_InvalidArgProc = 6,   // Invalid RegisterBackgroundArg.PaintConEmuBackground
	esbr_LastErrorNo = esbr_InvalidArgProc;

struct CESERVER_REQ_SETBACKGROUNDRET
{
	SetBackgroundResult  nResult;
};

// CECMD_ACTIVATECON.
struct CESERVER_REQ_ACTIVATECONSOLE
{
	HWND2 hConWnd;
};

// CECMD_GUIMACRO
#define CEGUIMACRORETENVVAR L"ConEmuMacroResult"
struct CESERVER_REQ_GUIMACRO
{
	DWORD   nSucceeded;
	wchar_t sMacro[1];    // Variable length
};

// CECMD_PEEKREADINFO: ���������� � GUI �� ������� Debug
struct CESERVER_REQ_PEEKREADINFO
{
	WORD         nCount;
	BYTE         bMainThread;
	BYTE         bReserved;
	wchar_t      cPeekRead/*'P'/'R' ��� 'W' � GUI*/;
	wchar_t      cUnicode/*'A'/'W'*/;
	HANDLE2      h;
	DWORD        nPID;
	DWORD        nTID;
	INPUT_RECORD Buffer[1];
};

enum ATTACHGUIAPP_FLAGS
{
	agaf_Fail     = 0,
	agaf_Success  = 1,
	agaf_DotNet   = 2,
	agaf_NoMenu   = 4,
	agaf_WS_CHILD = 8,
};

struct CESERVER_REQ_ATTACHGUIAPP
{
	DWORD nFlags;
	DWORD nPID;
	DWORD hkl;
	HWND2 hWindow;  // NULL - �������� ����� ��, HWND - ����� ������
	HWND2 hSrvConWnd;
	RECT  rcWindow; // ����������
	DWORD nStyle, nStyleEx;
	wchar_t sAppFileName[MAX_PATH*2];
};

struct CESERVER_REQ_SETFOCUS
{
	BOOL  bSetForeground;
	HWND2 hWindow;
};

struct CESERVER_REQ_SETPARENT
{
	HWND2 hWnd;
	HWND2 hParent;
};

struct MyAssertInfo
{
	int nBtns;
	BOOL bNoPipe;
	wchar_t szTitle[255];
	wchar_t szDebugInfo[4096];
};

struct CESERVER_REQ
{
	CESERVER_REQ_HDR hdr;
	union
	{
		BYTE    Data[1]; // variable(!) length
		WORD    wData[1];
		DWORD   dwData[1];
		u64     qwData[1];
		ConEmuGuiMapping GuiInfo;
		CESERVER_CONSOLE_MAPPING_HDR ConInfo;
		CESERVER_REQ_SETSIZE SetSize;
		CESERVER_REQ_RETSIZE SetSizeRet;
		CESERVER_REQ_OUTPUTFILE OutputFile;
		CESERVER_REQ_NEWCMD NewCmd;
		CESERVER_REQ_STARTSTOP StartStop;
		CESERVER_REQ_STARTSTOPRET StartStopRet;
		CESERVER_REQ_CONEMUTAB Tabs;
		CESERVER_REQ_CONEMUTAB_RET TabsRet;
		CESERVER_REQ_POSTMSG Msg;
		CESERVER_REQ_FLASHWINFO Flash;
		FAR_REQ_FARSETCHANGED FarSetChanged;
		CESERVER_REQ_SETCONCP SetConCP;
		CESERVER_REQ_SETWINDOWPOS SetWndPos;
		CESERVER_REQ_SETWINDOWRGN SetWndRgn;
		CESERVER_REQ_SETBACKGROUND Background;
		CESERVER_REQ_SETBACKGROUNDRET BackgroundRet;
		CESERVER_REQ_ACTIVATECONSOLE ActivateCon;
		PanelViewInit PVI;
		CESERVER_REQ_SETFONT Font;
		CESERVER_REQ_GUIMACRO GuiMacro;
		CESERVER_REQ_ONCREATEPROCESS OnCreateProc;
		CESERVER_REQ_ONCREATEPROCESSRET OnCreateProcRet;
		CESERVER_REQ_PEEKREADINFO PeekReadInfo;
		MyAssertInfo AssertInfo;
		CESERVER_REQ_ATTACHGUIAPP AttachGuiApp;
		CESERVER_REQ_SETFOCUS setFocus;
		CESERVER_REQ_SETPARENT setParent;
	};
};


//#pragma pack(pop)
#include <poppack.h>



//#define GWL_TABINDEX     0
//#define GWL_LANGCHANGE   4

#ifdef _DEBUG
#define CONEMUALIVETIMEOUT INFINITE
#define CONEMUREADYTIMEOUT INFINITE
#define CONEMUFARTIMEOUT   120000 // ������� �������, ���� ��� ���������� �� ����� �������
#else
#define CONEMUALIVETIMEOUT 1000  // ������� ������� ���� �������
#define CONEMUREADYTIMEOUT 10000 // � �� ���������� ������� - 10s max
#define CONEMUFARTIMEOUT   10000 // ������� �������, ���� ��� ���������� �� ����� �������
#endif

#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020E
#endif



/*enum PipeCmd
{
    SetTabs=0,
    DragFrom,
    DragTo
};*/

// ConEmu.dll ������������ ��������� �������
//HWND WINAPI GetFarHWND();
//void WINAPI _export GetFarVersion ( FarVersion* pfv );

//#if defined(__GNUC__)
////typedef DWORD   HWINEVENTHOOK;
//#define WINEVENT_OUTOFCONTEXT   0x0000  // Events are ASYNC
//// User32.dll
//typedef HWINEVENTHOOK (WINAPI* FSetWinEventHook)(DWORD eventMin, DWORD eventMax, HMODULE hmodWinEventProc, WINEVENTPROC pfnWinEventProc, DWORD idProcess, DWORD idThread, DWORD dwFlags);
//typedef BOOL (WINAPI* FUnhookWinEvent)(HWINEVENTHOOK hWinEventHook);
//#endif


#ifdef __cplusplus


#define CERR_CMDLINEEMPTY 200
#define CERR_CMDLINE      201

#define MOUSE_EVENT_MOVE      (WM_APP+10)
#define MOUSE_EVENT_CLICK     (WM_APP+11)
#define MOUSE_EVENT_DBLCLICK  (WM_APP+12)
#define MOUSE_EVENT_WHEELED   (WM_APP+13)
#define MOUSE_EVENT_HWHEELED  (WM_APP+14)
#define MOUSE_EVENT_FIRST MOUSE_EVENT_MOVE
#define MOUSE_EVENT_LAST MOUSE_EVENT_HWHEELED

//#define INPUT_THREAD_ALIVE_MSG (WM_APP+100)

//#define MAX_INPUT_QUEUE_EMPTY_WAIT 1000

int NextArg(const wchar_t** asCmdLine, wchar_t (&rsArg)[MAX_PATH+1], const wchar_t** rsArgStart=NULL);
int NextArg(const char** asCmdLine, char (&rsArg)[MAX_PATH+1], const char** rsArgStart=NULL);
const wchar_t* SkipNonPrintable(const wchar_t* asParams);
BOOL PackInputRecord(const INPUT_RECORD* piRec, MSG64* pMsg);
BOOL UnpackInputRecord(const MSG64* piMsg, INPUT_RECORD* pRec);
void CommonShutdown();


#ifndef _CRT_WIDE
#define __CRT_WIDE(_String) L ## _String
#define _CRT_WIDE(_String) __CRT_WIDE(_String)
#endif

#include "MAssert.h"
#include "MSecurity.h"

#endif // __cplusplus



// This must be the end line
#endif // _COMMON_HEADER_HPP_
