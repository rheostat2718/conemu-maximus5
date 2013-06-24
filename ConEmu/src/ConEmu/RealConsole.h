
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
#include "../Common/ConsoleAnnotation.h"
#include "../Common/RgnDetect.h"
//#include "../Common/WinObjects.h"
#include "../Common/MArray.h"
#include "../Common/MMap.h"
#include "../Common/MPipe.h"

#define DEFINE_EXIT_DESC
#include "../ConEmuCD/ExitCodes.h"

#define CES_CMDACTIVE      0x01
#define CES_TELNETACTIVE   0x02
#define CES_FARACTIVE      0x04
#define CES_FARINSTACK     0x08
//#define CES_CONALTERNATIVE 0x08
//#define CES_PROGRAMS (0x0F)

//#define CES_NTVDM 0x10 -- common.hpp
//#define CES_PROGRAMS2 0xFF

#define CES_FILEPANEL      0x0001
#define CES_FARPANELS      0x000F // �� �������, ������ ��������� ��� ��������� ����� ��������� �������
//#define CES_TEMPPANEL      0x0002
//#define CES_PLUGINPANEL    0x0004
#define CES_EDITOR         0x0010
#define CES_VIEWER         0x0020
#define CES_COPYING        0x0040
#define CES_MOVING         0x0080
#define CES_NOTPANELFLAGS  0xFFF0
#define CES_FARFLAGS       0xFFFF
#define CES_MAYBEPANEL   0x010000
#define CES_WASPROGRESS  0x020000
#define CES_OPER_ERROR   0x040000
//... and so on

//// Undocumented console message
//#define WM_SETCONSOLEINFO           (WM_USER+201)
//// and others
//#define SC_RESTORE_SECRET 0x0000f122
//#define SC_MAXIMIZE_SECRET 0x0000f032
//#define SC_PROPERTIES_SECRET 0x0000fff7
//#define SC_MARK_SECRET 0x0000fff2
//#define SC_COPY_ENTER_SECRET 0x0000fff0
//#define SC_PASTE_SECRET 0x0000fff1
//#define SC_SELECTALL_SECRET 0x0000fff5
//#define SC_SCROLL_SECRET 0x0000fff3
//#define SC_FIND_SECRET 0x0000fff4

//#define MAX_TITLE_SIZE 0x400

#define FAR_ALIVE_TIMEOUT gpSet->nFarHourglassDelay //1000

#define CONSOLE_BLOCK_SELECTION 0x0100 // selecting text (standard mode)
#define CONSOLE_TEXT_SELECTION 0x0200 // selecting text (stream mode)
#define CONSOLE_DBLCLICK_SELECTION 0x0400 // ������� ������ �������� �����, ���������� WM_LBUTTONUP
#define CONSOLE_KEYMOD_MASK 0xFF000000 // ����� �������� �����������, ������� ������ ��������� ������

#define PROCESS_WAIT_START_TIME RELEASEDEBUGTEST(1000,1000)

#define SETSYNCSIZEAPPLYTIMEOUT 500
#define SETSYNCSIZEMAPPINGTIMEOUT 300
#define CONSOLEPROGRESSTIMEOUT 1500
#define CONSOLEPROGRESSWARNTIMEOUT 2000 // �������� 2�, �.�. ��� ����������� ������� ����������� ��� � �������
#define CONSOLEINACTIVERGNTIMEOUT 500
#define SERVERCLOSETIMEOUT 2000
#define UPDATESERVERACTIVETIMEOUT 2500

/*#pragma pack(push, 1)


//
//  Structure to send console via WM_SETCONSOLEINFO
//
typedef struct _CONSOLE_INFO
{
    ULONG       Length;
    COORD       ScreenBufferSize;
    COORD       WindowSize;
    ULONG       WindowPosX;
    ULONG       WindowPosY;

    COORD       FontSize;
    ULONG       FontFamily;
    ULONG       FontWeight;
    WCHAR       FaceName[32];

    ULONG       CursorSize;
    ULONG       FullScreen;
    ULONG       QuickEdit;
    ULONG       AutoPosition;
    ULONG       InsertMode;

    USHORT      ScreenColors;
    USHORT      PopupColors;
    ULONG       HistoryNoDup;
    ULONG       HistoryBufferSize;
    ULONG       NumberOfHistoryBuffers;

    COLORREF    ColorTable[16];

    ULONG       CodePage;
    HWND        Hwnd;

    WCHAR       ConsoleTitle[0x100];

} CONSOLE_INFO;

#pragma pack(pop)*/

struct ConProcess
{
	DWORD ProcessID, ParentPID; //, InputTID;
	bool  IsMainSrv; // Root ConEmuC
	bool  IsConHost; // conhost.exe (Win7 � ����)
	bool  IsFar, IsFarPlugin;
	bool  IsTelnet;  // ����� ���� ������� ������ � IsFar, ���� ������� ���������� � ���� ����� ������� ����
	bool  IsNtvdm;   // 16bit ����������
	bool  IsCmd;     // ������ ��� ��������� �������
	bool  NameChecked, RetryName;
	bool  Alive, inConsole;
	wchar_t Name[64]; // ����� ������ ���� �� ������ �������
};

//#include <pshpack1.h>
//typedef struct tag_CharAttr
//{
//	TODO("OPTIMIZE: �������� �� ������� ���� �� ���� DWORD, � ������� ������� ����� ����� �� �����, ����������� ��� ������������ ������");
//	union {
//		// ���������� �����/������
//		struct {
//			unsigned int crForeColor : 24; // ����� � ui64 ���������� � nFontIndex
//			unsigned int nFontIndex : 8; // 0 - normal, 1 - bold, 2 - italic
//			unsigned int crBackColor : 32; // ������� ���� �������������, ����� ��� ������������ �����������
//			unsigned int nForeIdx : 8;
//			unsigned int nBackIdx : 8; // ����� ������������ ��� ExtendColors
//			unsigned int crOrigForeColor : 32;
//			unsigned int crOrigBackColor : 32; // �������� ����� � �������, crForeColor � crBackColor ����� ���� �������� ���������
//			// ��������������� �����
//			unsigned int bDialog : 1;
//			unsigned int bDialogVBorder : 1;
//			unsigned int bDialogCorner : 1;
//			unsigned int bSomeFilled : 1;
//			unsigned int bTransparent : 1; // UserScreen
//		};
//		// � ��� ��� ��������� (����� ���������)
//		unsigned __int64 All;
//		// ��� ���������, ����� ��� �� �����
//		unsigned int ForeFont;
//	};
//	//
//	//DWORD dwAttrubutes; // ����� ����� ����������� �������������� �����...
//	//
//    ///**
//    // * Used exclusively by ConsoleView to append annotations to each character
//    // */
//    //AnnotationInfo annotationInfo;
//} CharAttr;
//#include <poppack.h>
//
//inline bool operator==(const CharAttr& s1, const CharAttr& s2)
//{
//    return s1.All == s2.All;
//}
//

#define DBGMSG_LOG_ID (WM_APP+100)
#define DBGMSG_LOG_SHELL_MAGIC 0xD73A34
#define DBGMSG_LOG_INPUT_MAGIC 0xD73A35
#define DBGMSG_LOG_CMD_MAGIC   0xD73A36
struct DebugLogShellActivity
{
	DWORD   nParentPID, nParentBits, nChildPID;
	wchar_t szFunction[32];
	wchar_t* pszAction;
	wchar_t* pszFile;
	wchar_t* pszParam;
	int     nImageSubsystem;
	int     nImageBits;
	DWORD   nShellFlags, nCreateFlags, nStartFlags, nShowCmd;
	BOOL    bDos;
	DWORD   hStdIn, hStdOut, hStdErr;
};

//#define MAX_SERVER_THREADS 3
//#define MAX_THREAD_PACKETS 100

class CVirtualConsole;
class CRgnDetect;
class CRealBuffer;

enum RealBufferType
{
	rbt_Undefined = 0,
	rbt_Primary,
	rbt_Alternative,
	rbt_Selection,
	rbt_Find,
	rbt_DumpScreen,
};

enum LoadAltMode
{
	lam_Default = 0,    // ����� ���������� �������������
	lam_LastOutput = 1, // ����� ����� ������ ��� ���� � "Long console output"
	lam_FullBuffer = 2, // ������ ������ - ������ ����� � ����������
	lam_Primary = 3,    // TODO: ��� �������� ������ ��������� - ����� ������ rbt_Primary
};

enum ExpandTextRangeType;

#include "RealServer.h"

class CRealConsole
{
#ifdef _DEBUG
		friend class CVirtualConsole;
#endif
	public:

		uint TextWidth();
		uint TextHeight();
		uint BufferHeight(uint nNewBufferHeight=0);
		uint BufferWidth();
		void OnBufferHeight();

	private:
		HWND    hConWnd;
		HWND    hGuiWnd; // ���� �������� � Gui-������ (Notepad, Putty, ...)
		RECT    mrc_LastGuiWnd; // Screen coordinates!
		HWND    mh_GuiWndFocusStore;
		DWORD   mn_GuiAttachInputTID;
		static  BOOL CALLBACK FindChildGuiWindowProc(HWND hwnd, LPARAM lParam);
		DWORD   mn_GuiAttachFlags; // ������������ � SetGuiMode
		BOOL    mb_GuiExternMode; // FALSE ���� �������� �������� GUI ���������� ��� ������� ConEmu (Ctrl-Win-Alt-Space)
		RECT    rcPreGuiWndRect; // ��������� ���� �� ������
		BOOL    mb_InGuiAttaching;
		BOOL    mb_InSetFocus;
		DWORD   mn_GuiWndStyle, mn_GuiWndStylEx; // �������� ����� ���� �� ����������� � ConEmu
		DWORD   mn_GuiWndPID;
		wchar_t ms_GuiWndProcess[MAX_PATH];
		BYTE    m_ConsoleKeyShortcuts;
		BYTE    mn_TextColorIdx, mn_BackColorIdx, mn_PopTextColorIdx, mn_PopBackColorIdx;
		void    PrepareDefaultColors(BYTE& nTextColorIdx, BYTE& nBackColorIdx, BYTE& nPopTextColorIdx, BYTE& nPopBackColorIdx, bool bUpdateRegistry = false, HKEY hkConsole = NULL);
		//HANDLE  hFileMapping;
		//CESERVER_REQ_CONINFO* pConsoleData;
		//void CloseMapping();
		void setGuiWndPID(DWORD anPID, LPCWSTR asProcessName);

	public:
		HWND    ConWnd();  // HWND RealConsole
		HWND    GetView(); // HWND ���������

		// ���� �������� � Gui-������ (Notepad, Putty, ...)
		HWND    GuiWnd();  // HWND Gui ����������
		DWORD   GuiWndPID();  // HWND Gui ����������
		void    GuiNotifyChildWindow();
		void    GuiWndFocusStore();
		void    GuiWndFocusRestore(bool bForce = false);
	private:
		void    GuiWndFocusThread(HWND hSetFocus, BOOL& bAttached, BOOL& bAttachCalled, DWORD& nErr);
	public:
		BOOL    isGuiVisible();
		BOOL    isGuiOverCon();
		void    StoreGuiChildRect(LPRECT prcNewPos);
		void    SetGuiMode(DWORD anFlags, HWND ahGuiWnd, DWORD anStyle, DWORD anStyleEx, LPCWSTR asAppFileName, DWORD anAppPID, RECT arcPrev);
		static void CorrectGuiChildRect(DWORD anStyle, DWORD anStyleEx, RECT& rcGui);

		CRealConsole();
		bool Construct(CVirtualConsole* apVCon, RConStartArgs *args);
		~CRealConsole();

		CVirtualConsole* VCon();

		BYTE GetConsoleKeyShortcuts() { return this ? m_ConsoleKeyShortcuts : 0; };
		BYTE GetDefaultTextColorIdx() { return this ? (mn_TextColorIdx & 0xF) : 7; };
		BYTE GetDefaultBackColorIdx() { return this ? (mn_BackColorIdx & 0xF) : 0; };

		BOOL PreInit();
		void DumpConsole(HANDLE ahFile);
		bool LoadDumpConsole(LPCWSTR asDumpFile);
		
		RealBufferType GetActiveBufferType();
		bool SetActiveBuffer(RealBufferType aBufferType);

		BOOL SetConsoleSize(USHORT sizeX, USHORT sizeY, USHORT sizeBuffer=0, DWORD anCmdID=CECMD_SETSIZESYNC);
	private:
		bool SetActiveBuffer(CRealBuffer* aBuffer, bool abTouchMonitorEvent = true);
		bool LoadAlternativeConsole(LoadAltMode iMode = lam_Default);
		BOOL SetConsoleSizeSrv(USHORT sizeX, USHORT sizeY, USHORT sizeBuffer, DWORD anCmdID=CECMD_SETSIZESYNC);
	private:
		//void SendConsoleEvent(INPUT_RECORD* piRec);
		DWORD mn_FlushIn, mn_FlushOut;
	public:
		COORD ScreenToBuffer(COORD crMouse);
		COORD BufferToScreen(COORD crMouse, bool bVertOnly = false);
		bool PostConsoleEvent(INPUT_RECORD* piRec, bool bFromIME = false);
		bool PostKeyPress(WORD vkKey, DWORD dwControlState, wchar_t wch, int ScanCode = -1);
		bool DeleteWordKeyPress(bool bTestOnly = false);
		bool PostKeyUp(WORD vkKey, DWORD dwControlState, wchar_t wch, int ScanCode = -1);
		bool PostLeftClickSync(COORD crDC);
		bool PostConsoleEventPipe(MSG64 *pMsg, size_t cchCount = 1);
		void ShowKeyBarHint(WORD nID);
		bool PostPromptCmd(bool CD, LPCWSTR asCmd);
	private:
		bool PostString(wchar_t* pszChars, size_t cchCount);
		//void TranslateKeyPress(WORD vkKey, DWORD dwControlState, wchar_t wch, int ScanCode, INPUT_RECORD& rDown, INPUT_RECORD& rUp);
		void PostMouseEvent(UINT messg, WPARAM wParam, COORD crMouse, bool abForceSend = false);
	public:
		BOOL OpenConsoleEventPipe();
		bool PostConsoleMessage(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
		bool SetFullScreen();
		BOOL ShowOtherWindow(HWND hWnd, int swShow, BOOL abAsync=TRUE);
		BOOL SetOtherWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);
		BOOL SetOtherWindowFocus(HWND hWnd, BOOL abSetForeground);
		HWND SetOtherWindowParent(HWND hWnd, HWND hParent);
		BOOL SetOtherWindowRgn(HWND hWnd, int nRects, LPRECT prcRects, BOOL bRedraw);
		void PostDragCopy(BOOL abMove);
		void PostMacro(LPCWSTR asMacro, BOOL abAsync = FALSE);
		bool GetFarVersion(FarVersion* pfv);
		bool IsFarLua();
	private:
		struct PostMacroAnyncArg
		{
			CRealConsole* pRCon;
			BOOL    bPipeCommand;
			DWORD   nCmdSize;
			DWORD   nCmdID;
			union
			{
				wchar_t szMacro[1];
				BYTE    Data[1];
			};
		};
		static DWORD WINAPI PostMacroThread(LPVOID lpParameter);
		HANDLE mh_PostMacroThread; DWORD mn_PostMacroThreadID;
		void PostCommand(DWORD anCmdID, DWORD anCmdSize, LPCVOID ptrData);
		DWORD mn_InPostDeadChar;
	public:
		//BOOL FlushInputQueue(DWORD nTimeout = 500);
		void OnKeyboard(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam, const wchar_t *pszChars, const MSG* pDeadCharMsg);
		void ProcessKeyboard(UINT messg, WPARAM wParam, LPARAM lParam, const wchar_t *pszChars);
		void OnKeyboardIme(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
		void OnMouse(UINT messg, WPARAM wParam, int x, int y, bool abForceSend = false, bool abFromTouch = false);
		void OnFocus(BOOL abFocused);

		void StopSignal();
		void StopThread(BOOL abRecreating=FALSE);
		bool InScroll();
		BOOL isBufferHeight();
		BOOL isAlternative();
		HWND isPictureView(BOOL abIgnoreNonModal=FALSE);
		BOOL isWindowVisible();
		LPCTSTR GetTitle(bool abGetRenamed=false);
		void GetConsoleScreenBufferInfo(CONSOLE_SCREEN_BUFFER_INFO* sbi);
		//void GetConsoleCursorPos(COORD *pcr);
		void GetConsoleCursorInfo(CONSOLE_CURSOR_INFO *ci, COORD *cr = NULL);
		DWORD GetConsoleCP();
		DWORD GetConsoleOutputCP();
		DWORD GetConsoleMode();
		void SyncConsole2Window(BOOL abNtvdmOff=FALSE, LPRECT prcNewWnd=NULL);
		void SyncGui2Window(RECT* prcClient=NULL);
		//void OnWinEvent(DWORD anEvent, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
		void OnServerStarted(const HWND ahConWnd, const DWORD anServerPID, const DWORD dwKeybLayout);
		void OnDosAppStartStop(enum StartStopType sst, DWORD anPID);
		int  GetProcesses(ConProcess** ppPrc, bool ClientOnly = false);
		DWORD GetFarPID(bool abPluginRequired=false);
		void SetFarPID(DWORD nFarPID);
		void SetFarPluginPID(DWORD nFarPluginPID);
		void SetProgramStatus(DWORD nNewProgramStatus);
		void SetFarStatus(DWORD nNewFarStatus);
		DWORD GetActivePID();
		LPCWSTR GetActiveProcessName();
		int GetActiveAppSettingsId(LPCWSTR* ppProcessName=NULL);
	private:
		int GetDefaultAppSettingsId();
	public:
		void ResetActiveAppSettingsId();
		DWORD GetProgramStatus();
		DWORD GetFarStatus();
		bool isServerAlive();
		DWORD GetServerPID(bool bMainOnly = false);
		DWORD GetMonitorThreadID();
		bool isServerCreated(bool bFullRequired = false);
		bool isServerAvailable();
		bool isServerClosing();
		LRESULT OnScroll(int nDirection);
		LRESULT OnSetScrollPos(WPARAM wParam);
		bool GetConsoleSelectionInfo(CONSOLE_SELECTION_INFO *sel);
		bool isConSelectMode();
		bool isFar(bool abPluginRequired=false);
		bool isFarBufferSupported();
		bool isFarInStack();
		bool isFarKeyBarShown();
		bool isSelectionAllowed();
		bool isSelectionPresent();
		bool isMouseSelectionPresent();
		void AutoCopyTimer(); // ����� ��������� "Auto Copy" & "Double click - select word"
		void StartSelection(BOOL abTextMode, SHORT anX=-1, SHORT anY=-1, BOOL abByMouse=FALSE);
		void ExpandSelection(SHORT anX=-1, SHORT anY=-1);
		bool DoSelectionCopy(bool bCopyAll = false);
		void DoSelectionStop();
		void DoFindText(int nDirection);
		void DoEndFindText();
		void CtrlWinAltSpace();
		void ShowConsoleOrGuiClient(int nMode); // -1 Toggle, 0 - Hide, 1 - Show
		void ShowConsole(int nMode); // -1 Toggle, 0 - Hide, 1 - Show
		void ShowGuiClientExt(int nMode, BOOL bDetach = FALSE); // -1 Toggle, 0 - Hide, 1 - Show
		void ShowGuiClientInt(bool bShow);
		void ChildSystemMenu();
		bool isDetached();
		BOOL AttachConemuC(HWND ahConWnd, DWORD anConemuC_PID, const CESERVER_REQ_STARTSTOP* rStartStop, CESERVER_REQ_STARTSTOPRET* pRet);
		BOOL RecreateProcess(RConStartArgs *args);
		void GetConsoleData(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight);
		ExpandTextRangeType GetLastTextRangeType();
	private:
		bool PreCreate(RConStartArgs *args);

		BOOL GetConsoleLine(int nLine, wchar_t** pChar, /*CharAttr** pAttr,*/ int* pLen, MSectionLock* pcsData);
		//enum ExpandTextRangeType
		//{
		//	etr_None = 0,
		//	etr_Word = 1,
		//	etr_FileAndLine = 2,
		//};
		//ExpandTextRangeType ExpandTextRange(COORD& crFrom/*[In/Out]*/, COORD& crTo/*[Out]*/, ExpandTextRangeType etr, wchar_t* pszText = NULL, size_t cchTextMax = 0);
		bool IsFarHyperlinkAllowed(bool abFarRequired);
		bool ProcessFarHyperlink(UINT messg, COORD crFrom);
		void UpdateTabFlags(/*IN|OUT*/ ConEmuTab* pTab);
		static INT_PTR CALLBACK renameProc(HWND hDlg, UINT messg, WPARAM wParam, LPARAM lParam);
	public:
		BOOL IsConsoleDataChanged();
		void OnActivate(int nNewNum, int nOldNum);
		void OnDeactivate(int nNewNum);
		void ShowHideViews(BOOL abShow);
		void OnGuiFocused(BOOL abFocus, BOOL abForceChild = FALSE);
		void UpdateServerActive(BOOL abImmediate = FALSE);
		BOOL CheckBufferSize();
		//LRESULT OnConEmuCmd(BOOL abStarted, DWORD anConEmuC_PID);
		BOOL BufferHeightTurnedOn(CONSOLE_SCREEN_BUFFER_INFO* psbi);
		void UpdateScrollInfo();
		void SetTabs(ConEmuTab* tabs, int tabsCount);
		void DoRenameTab();
		bool DuplicateRoot(bool bSkipMsg = false, LPCWSTR asAddArgs = NULL, bool bRunAsAdmin = false);
		void RenameTab(LPCWSTR asNewTabText = NULL);
		void RenameWindow(LPCWSTR asNewWindowText = NULL);
		int GetTabCount(BOOL abVisibleOnly = FALSE);
		int GetActiveTab();
		CEFarWindowType GetActiveTabType();
		bool GetTab(int tabIdx, /*OUT*/ ConEmuTab* pTab);
		int GetModifiedEditors();
		BOOL ActivateFarWindow(int anWndIndex);
		DWORD CanActivateFarWindow(int anWndIndex);
		void OnConsoleKeyboardLayout(DWORD dwNewLayout);
		void SwitchKeyboardLayout(WPARAM wParam,DWORD_PTR dwNewKeybLayout);
		void CloseConsole(bool abForceTerminate, bool abConfirm, bool abAllowMacro = true);
		void CloseConsoleWindow(bool abConfirm);
		bool isCloseConfirmed(LPCWSTR asConfirmation, bool bForceAsk = false);
		void CloseConfirmReset();
		BOOL CanCloseTab(BOOL abPluginRequired = FALSE);
		void CloseTab();
		bool isConsoleClosing();
		void OnServerClosing(DWORD anSrvPID);
		void Paste(bool abFirstLineOnly = false, LPCWSTR asText = NULL, bool abNoConfirm = false, bool abCygWin = false);
		void LogString(LPCSTR asText, BOOL abShowTime = FALSE);
		void LogString(LPCWSTR asText, BOOL abShowTime = FALSE);
		bool isActive(bool abAllowGroup = false);
		bool isInFocus();
		bool isFilePanel(bool abPluginAllowed = false, bool abSkipEditViewCheck = false);
		bool isEditor();
		bool isEditorModified();
		bool isViewer();
		bool isVisible();
		bool isNtvdm();
		//bool isPackets();
		LPCWSTR GetCmd();
		LPCWSTR GetDir();
		wchar_t* CreateCommandLine(bool abForTasks = false);
		BOOL GetUserPwd(const wchar_t** ppszUser, const wchar_t** ppszDomain, BOOL* pbRestricted);
		short GetProgress(int* rpnState/*1-error,2-ind*/, BOOL* rpbNotFromTitle = NULL);
		bool SetProgress(short nState, short nValue, LPCWSTR pszName = NULL);
		void UpdateGuiInfoMapping(const ConEmuGuiMapping* apGuiInfo);
		void UpdateFarSettings(DWORD anFarPID=0);
		void UpdateTextColorSettings(BOOL ChangeTextAttr = TRUE, BOOL ChangePopupAttr = TRUE);
		int CoordInPanel(COORD cr, BOOL abIncludeEdges = FALSE);
		BOOL GetPanelRect(BOOL abRight, RECT* prc, BOOL abFull = FALSE, BOOL abIncludeEdges = FALSE);
		bool isAdministrator();
		BOOL isMouseButtonDown();
		void OnConsoleLangChange(DWORD_PTR dwNewKeybLayout);
		DWORD GetConsoleStates();
		void ChangeBufferHeightMode(BOOL abBufferHeight); // ���������� �� TabBar->ConEmu
		//void RemoveFromCursor(); // -- �������� �� �������� ������� ScreenToClient
		bool isAlive();
		bool GetMaxConSize(COORD* pcrMaxConSize);
		int GetDetectedDialogs(int anMaxCount, SMALL_RECT* rc, DWORD* rf);
		const CRgnDetect* GetDetector();
		// ����������� Shell �������
		//void LogShellStartStop();
		bool IsLogShellStarted();
		wchar_t ms_LogShellActivity[MAX_PATH]; bool mb_ShellActivityLogged;
		int GetStatusLineCount(int nLeftPanelEdge);

	public:
		BOOL IsConsoleThread();
		void SetForceRead();
		//DWORD WaitEndUpdate(DWORD dwTimeout=1000);
		LPCWSTR GetConStatus();
		void SetConStatus(LPCWSTR asStatus, bool abResetOnConsoleReady = false, bool abDontUpdate = false);
		static wchar_t ms_LastRConStatus[80];
		void UpdateCursorInfo();
		bool isNeedCursorDraw();
		void Detach(bool bPosted = false, bool bSendCloseConsole = false);
		void AdminDuplicate();
		const CEFAR_INFO_MAPPING *GetFarInfo(); // FarVer � ������
		bool InCreateRoot();
		bool InRecreate(); 
		BOOL GuiAppAttachAllowed(LPCWSTR asAppFileName, DWORD anAppPID);
		//LPCWSTR GetLngNameTime();
		void ShowPropertiesDialog();
		void LogInput(UINT uMsg, WPARAM wParam, LPARAM lParam, LPCWSTR pszTranslatedChars = NULL);

		//static void Box(LPCTSTR szText, DWORD nBtns = 0);

		void OnStartProcessAllowed();
		void OnTimerCheck();

	public:
		void MonitorAssertTrap();
	private:
		bool mb_MonitorAssertTrap;

	protected:
		CVirtualConsole* mp_VCon; // ��������������� ����������� �������

		void SetMainSrvPID(DWORD anMainSrvPID, HANDLE ahMainSrv);
		void SetAltSrvPID(DWORD anAltSrvPID/*, HANDLE ahAltSrv*/);
		// ������ � �������������� ������
		DWORD mn_MainSrv_PID; HANDLE mh_MainSrv;
		DWORD mn_ConHost_PID;
		MMap<DWORD,BOOL>* mp_ConHostSearch;
		void ConHostSearchPrepare();
		DWORD ConHostSearch(bool bFinal);
		void ConHostSetPID(DWORD nConHostPID);
		bool  mb_MainSrv_Ready; // ������ ����� ��������� �������?
		DWORD mn_ActiveLayout;
		DWORD mn_AltSrv_PID;  //HANDLE mh_AltSrv;
		HANDLE mh_SwitchActiveServer, mh_ActiveServerSwitched;
		bool mb_SwitchActiveServer;
		bool InitAltServer(DWORD nAltServerPID/*, HANDLE hAltServer*/);
		bool ReopenServerPipes();

		// ���� ����������� �����
		wchar_t ms_ConEmuCInput_Pipe[MAX_PATH];
		HANDLE mh_ConInputPipe; // wsprintf(ms_ConEmuCInput_Pipe, CESERVERINPUTNAME, L".", mn_ConEmuC_PID)
		
		BOOL mb_InCreateRoot;
		BOOL mb_UseOnlyPipeInput;
		TCHAR ms_ConEmuC_Pipe[MAX_PATH], ms_MainSrv_Pipe[MAX_PATH], ms_VConServer_Pipe[MAX_PATH];
		//TCHAR ms_ConEmuC_DataReady[64]; HANDLE mh_ConEmuC_DataReady;
		void InitNames();
		// ������� ��������� ������� � ��� �������� ��� ��������� (��� ����������� ���������)
		WCHAR Title[MAX_TITLE_SIZE+1], TitleCmp[MAX_TITLE_SIZE+1];
		// � ����� ���������� ��, ��� ������������ � ConEmu (����� ���� ��������� " (Admin)")
		WCHAR TitleFull[MAX_TITLE_SIZE+96], TitleAdmin[MAX_TITLE_SIZE+192];
		// ������������� ������� OnTitleChanged, ��������, ��� ��������� ��������� � �������
		BOOL mb_ForceTitleChanged;
		// ����� ����������� ��������� ���� (� ��������), ����� FAR ����� � ������� ������ (��������� � ��������...).
		WCHAR ms_PanelTitle[CONEMUTABMAX];
		// ����������
		short mn_Progress, mn_LastShownProgress;
		short mn_PreWarningProgress; DWORD mn_LastWarnCheckTick;
		short mn_ConsoleProgress, mn_LastConsoleProgress; DWORD mn_LastConProgrTick;
		short mn_AppProgressState, mn_AppProgress; // ����� ���� ����� �� ������� (Ansi codes, Macro)
		short CheckProgressInTitle();
		//short CheckProgressInConsole(const wchar_t* pszCurLine);
		//void SetProgress(short anProgress); // ���������� ���������� mn_Progress � mn_LastProgressTick

#if 0
		BOOL AttachPID(DWORD dwPID); //120714 - ����������� ��������� �������� � ConEmuC.exe, � � GUI ��� � �� ��������. ����� ����
#endif
		BOOL StartProcess();
		BOOL StartMonitorThread();
		void SetMonitorThreadEvent();
		BOOL mb_NeedStartProcess;

		// ���� ���������� �� ��������
		static DWORD WINAPI MonitorThread(LPVOID lpParameter);
		DWORD MonitorThreadWorker(BOOL bDetached, BOOL& rbChildProcessCreated);
		static int WorkerExFilter(unsigned int code, struct _EXCEPTION_POINTERS *ep, LPCTSTR szFile, UINT nLine);
		HANDLE mh_MonitorThread; DWORD mn_MonitorThreadID;
		HANDLE mh_MonitorThreadEvent;
		HANDLE mh_UpdateServerActiveEvent;
		DWORD mn_ServerActiveTick1, mn_ServerActiveTick2;
		//BOOL mb_UpdateServerActive;
		DWORD mn_LastUpdateServerActive;
		// ��� ��������� ������� ����� � �������
		//static DWORD WINAPI InputThread(LPVOID lpParameter);
		//HANDLE mh_InputThread; DWORD mn_InputThreadID;

		DWORD mn_TermEventTick;
		HANDLE mh_TermEvent, mh_ApplyFinished;
		HANDLE mh_StartExecuted;
		BOOL mb_StartResult, mb_WaitingRootStartup;
		BOOL mb_FullRetrieveNeeded; //, mb_Detached;
		RConStartArgs m_Args;
		BOOL mb_WasStartDetached;
		wchar_t ms_RootProcessName[MAX_PATH];
		// Replace in asCmd some env.vars (!ConEmuBackHWND! and so on)
		wchar_t* ParseConEmuSubst(LPCWSTR asCmd);
		wchar_t* mpsz_CmdBuffer;
		//BOOL mb_AdminShieldChecked;
		//wchar_t* ms_SpecialCmd;
		//BOOL mb_RunAsAdministrator;

		//BOOL RetrieveConsoleInfo(/*BOOL bShortOnly,*/ UINT anWaitSize);
		BOOL WaitConsoleSize(int anWaitSize, DWORD nTimeout);
		BOOL InitBuffers(DWORD OneBufferSize);
		BOOL LoadDataFromSrv(DWORD CharCount, CHAR_INFO* pData);
	private:
		friend class CRealBuffer;
		CRealBuffer* mp_RBuf; // �������� ����� �������
		CRealBuffer* mp_EBuf; // ���������� ������ ����� ����������� ������� � Far
		CRealBuffer* mp_SBuf; // ��������� ����� (������) ��� ������������ ����������� (���������/���������/�����)
		CRealBuffer* mp_ABuf; // �������� ����� ������� -- ������ �� ���� �� mp_RBuf/mp_EBuf/mp_SBuf
		bool mb_ABufChaged; // �������� �������� �����, �������� �������
		
		int mn_DefaultBufferHeight;
		DWORD mn_LastInactiveRgnCheck;
		#ifdef _DEBUG
		BOOL mb_DebugLocked; // ��� ������� - ���������� ��� ����, ����� �� ������ ���������, �������� �� ����������� ����
		#endif
		
		//// ��� ���������� ���������������� � RetrieveConsoleInfo()
		//MSection csCON; //DWORD ncsT;
		//struct RConInfo
		//{
		//	CONSOLE_SELECTION_INFO m_sel;
		//	CONSOLE_CURSOR_INFO m_ci;
		//	DWORD m_dwConsoleCP, m_dwConsoleOutputCP, m_dwConsoleMode;
		//	CONSOLE_SCREEN_BUFFER_INFO m_sbi;
		//	COORD crMaxSize; // ������������ ������ ������� �� ������� ������
		//	USHORT nTopVisibleLine; // ����� ���������� �� m_sbi.srWindow.Top, ���� ��������� �������������
		//	wchar_t *pConChar;
		//	WORD  *pConAttr;
		//	COORD mcr_FileLineStart, mcr_FileLineEnd; // ��������� ����� ������ ������������
		//	//CESERVER_REQ_CONINFO_DATA *pCopy, *pCmp;
		//	CHAR_INFO *pDataCmp;
		//	int nTextWidth, nTextHeight, nBufferHeight;
		//	BOOL bLockChange2Text;
		//	int nChange2TextWidth, nChange2TextHeight;
		//	BOOL bBufferHeight; // TRUE, ���� ���� ���������
		//	//DWORD nPacketIdx;
		//	DWORD_PTR dwKeybLayout;
		//	BOOL bRBtnDrag; // � ������� ���������� ���� ������ ������� (��������� � FAR)
		//	COORD crRBtnDrag;
		//	BOOL bInSetSize; HANDLE hInSetSize;
		//	int DefaultBufferHeight;
		//	BOOL bConsoleDataChanged;
		//	DWORD nLastInactiveRgnCheck;
		//	#ifdef _DEBUG
		//	BOOL bDebugLocked; // ��� ������� - ���������� ��� ����, ����� �� ������ ���������, �������� �� ����������� ����
		//	#endif
		//} con;
		
		//BOOL mb_ThawRefreshThread;
		struct ServerClosing
		{
			DWORD  nServerPID;     // PID �������������� �������
			DWORD  nRecieveTick;   // Tick, ����� �������� ��������� � ��������
			HANDLE hServerProcess; // Handle �������� �������
		} m_ServerClosing;
		//
		MSection csPRC; //DWORD ncsTPRC;
		MArray<ConProcess> m_Processes;
		int mn_ProcessCount, mn_ProcessClientCount;
		DWORD m_FarPlugPIDs[128];
		UINT mn_FarPlugPIDsCount;
		BOOL mb_SkipFarPidChange;
		DWORD m_TerminatedPIDs[128]; UINT mn_TerminatedIdx;
		//
		DWORD mn_FarPID;
		DWORD mn_ActivePID;
		void SetActivePID(DWORD anNewPID);
		DWORD mn_LastSetForegroundPID; // PID ��������, �������� � ��������� ��� ���� ��������� AllowSetForegroundWindow
		DWORD mn_LastProcessNamePID;
		wchar_t ms_LastProcessName[MAX_PATH];
		int mn_LastAppSettingsId;
		//
		ConEmuTab* mp_tabs;
		wchar_t    ms_RenameFirstTab[MAX_RENAME_TAB_LEN/*128*/];
		MSection   msc_Tabs;
		int mn_tabsCount, mn_MaxTabs, mn_ActiveTab;
		BOOL mb_TabsWasChanged;
		void CheckPanelTitle();
		//
		//void ProcessAdd(DWORD addPID);
		//void ProcessDelete(DWORD addPID);
		BOOL ProcessUpdate(const DWORD *apPID, UINT anCount);
		BOOL ProcessUpdateFlags(BOOL abProcessChanged);
		void ProcessCheckName(struct ConProcess &ConPrc, LPWSTR asFullFileName);
		DWORD mn_ProgramStatus, mn_FarStatus;
		DWORD mn_Comspec4Ntvdm;
		BOOL mb_IgnoreCmdStop; // ��� ������� 16bit ���������� �� ���������� ������ �������! ��� ������� OnWinEvent
		BOOL isShowConsole;
		//BOOL mb_FarGrabberActive; // ������ mb_ConsoleSelectMode
		WORD mn_SelectModeSkipVk; // ���������� "����������" ������� Esc/Enter ��� ��������� ������
		//bool OnMouseSelection(UINT messg, WPARAM wParam, int x, int y);
		//void UpdateSelection(); // �������� �� ������

		friend class CRealServer;
		CRealServer m_RConServer;
		
		//void ApplyConsoleInfo(CESERVER_REQ* pInfo);
		void SetHwnd(HWND ahConWnd, BOOL abForceApprove = FALSE);
		WORD mn_LastVKeyPressed;
		BOOL GetConWindowSize(const CONSOLE_SCREEN_BUFFER_INFO& sbi, int& nNewWidth, int& nNewHeight, BOOL* pbBufferHeight=NULL);
		int mn_Focused; //-1 ����� �������, 1 - � ������, 0 - �� � ������
		DWORD mn_InRecreate; // Tick, ����� ������ ������������
		BOOL mb_ProcessRestarted;
		BOOL mb_InCloseConsole;
		DWORD mn_CloseConfirmedTick;
		bool mb_CloseFarMacroPosted;
		bool mb_WasSendClickToReadCon;
		// ����
		BYTE m_UseLogs;
		//HANDLE mh_LogInput; wchar_t *mpsz_LogInputFile/*, *mpsz_LogPackets*/; //UINT mn_LogPackets;
		MFileLog *mp_Log;
		void CreateLogFiles();
		void CloseLogFiles();
		void LogInput(INPUT_RECORD* pRec);
		//void LogPacket(CESERVER_REQ* pInfo);
		BOOL RecreateProcessStart();
		// ����� � ��������� �������
		//MSection csPKT; //DWORD ncsTPKT;
		//DWORD mn_LastProcessedPkt; //HANDLE mh_PacketArrived;
		//std::vector<CESERVER_REQ*> m_Packets;
		//CESERVER_REQ* m_PacketQueue[(MAX_SERVER_THREADS+1)*MAX_THREAD_PACKETS];
		//void PushPacket(CESERVER_REQ* pPkt);
		//CESERVER_REQ* PopPacket();
		//HANDLE mh_FileMapping, mh_FileMappingData,
		//HANDLE mh_FarFileMapping,
		//HANDLE mh_FarAliveEvent;
		MEvent m_FarAliveEvent;
		MPipe<CESERVER_REQ_HDR,CESERVER_REQ_HDR> m_GetDataPipe;
		MEvent m_ConDataChanged;
		//wchar_t ms_HeaderMapName[64], ms_DataMapName[64];
		//const CESERVER_CONSOLE_MAPPING_HDR *mp_ConsoleInfo;
		//const CESERVER_REQ_CONINFO_DATA *mp_ConsoleData; // Mapping
		MFileMapping<CESERVER_CONSOLE_MAPPING_HDR> m_ConsoleMap;
		//const CEFAR_INFO_MAPPING *mp_FarInfo;
		CEFAR_INFO_MAPPING m_FarInfo; // FarVer � ������
		MFileMapping<const CEFAR_INFO_MAPPING> m__FarInfo; // � ���� �������� �� ������������, ������ ����� ������!
		MSection ms_FarInfoCS;
		// Colorer Mapping
		//HANDLE mh_ColorMapping;
		//AnnotationHeader *mp_ColorHdr;
		MFileMapping<AnnotationHeader> m_TrueColorerMap;
		AnnotationHeader m_TrueColorerHeader;
		const AnnotationInfo *mp_TrueColorerData;
		DWORD mn_LastColorFarID;
		void CreateColorMapping(); // ������� ������� �������� (�� HWND)
		//void CheckColorMapping(DWORD dwPID); // ��������� ���������� ������ - todo
		void CloseColorMapping();
		//
		DWORD mn_LastConsoleDataIdx, mn_LastConsolePacketIdx; //, mn_LastFarReadIdx;
		DWORD mn_LastFarReadTick;
		BOOL OpenFarMapData();
		void CloseFarMapData(MSectionLock* pCS = NULL);
		BOOL OpenMapHeader(BOOL abFromAttach=FALSE);
		//void CloseMapData();
		//BOOL ReopenMapData();
		void CloseMapHeader();
		BOOL ApplyConsoleInfo();
		BOOL mb_DataChanged;
		void OnServerStarted(DWORD anServerPID, HANDLE ahServerHandle, DWORD dwKeybLayout);
		void OnStartedSuccess();
		BOOL mb_RConStartedSuccess;
		//
		BOOL PrepareOutputFile(BOOL abUnicodeText, wchar_t* pszFilePathName);
		HANDLE PrepareOutputFileCreate(wchar_t* pszFilePathName);
		// ���� ��� dblclick � ���������
		MOUSE_EVENT_RECORD m_LastMouse;
		POINT m_LastMouseGuiPos; // � �������� ��������
		BOOL mb_BtnClicked; COORD mrc_BtnClickPos;
		//
		wchar_t ms_Editor[32], ms_EditorRus[32], ms_Viewer[32], ms_ViewerRus[32];
		wchar_t ms_TempPanel[32], ms_TempPanelRus[32];
		//wchar_t ms_NameTitle[32];
		//
		//BOOL mb_PluginDetected;
		DWORD mn_FarPID_PluginDetected; //, mn_Far_PluginInputThreadId;
		void CheckFarStates();
		void OnTitleChanged();
		DWORD mn_LastInvalidateTick;
		//
		HWND hPictureView; BOOL mb_PicViewWasHidden;
		//
		BOOL mb_MouseButtonDown;
		COORD mcr_LastMouseEventPos;
		// ��� Far Manager: ��������� "���������" ������
		BOOL mb_MouseTapChanged;
		COORD mcr_MouseTapReal, mcr_MouseTapChanged;
		//
		SHELLEXECUTEINFO *mp_sei;
		//
		HWND FindPicViewFrom(HWND hFrom);
		//
		wchar_t ms_ConStatus[80];
		bool mb_ResetStatusOnConsoleReady;
		//
		bool isCharBorderVertical(WCHAR inChar);
		bool isCharBorderLeftVertical(WCHAR inChar);
		bool isCharBorderHorizontal(WCHAR inChar);
		bool ConsoleRect2ScreenRect(const RECT &rcCon, RECT *prcScr);
		/* ****************************************** */
		/* ����� �������� � ������� "����������" ���� */
		/* ****************************************** */
		//CRgnDetect* mp_Rgn; DWORD mn_LastRgnFlags;
		//int mn_DetectCallCount;
		//void PrepareTransparent(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight);
		//void DetectDialog(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int nFromX, int nFromY, int *pnMostRight=NULL, int *pnMostBottom=NULL);
		//bool FindDialog_TopLeft(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight, int &nMostBottom, BOOL &bMarkBorder);
		//bool FindDialog_TopRight(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight, int &nMostBottom, BOOL &bMarkBorder);
		//bool FindDialog_Left(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight, int &nMostBottom, BOOL &bMarkBorder);
		//bool FindDialog_Right(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight, int &nMostBottom, BOOL &bMarkBorder);
		//bool FindDialog_Any(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight, int &nMostBottom, BOOL &bMarkBorder);
		//bool FindDialog_Inner(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY);
		//bool FindFrame_TopLeft(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nFrameX, int &nFrameY);
		//bool FindFrameTop_ByRight(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostTop);
		//bool FindFrameTop_ByLeft(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostTop);
		//bool FindFrameBottom_ByRight(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostBottom);
		//bool FindFrameBottom_ByLeft(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostBottom);
		//bool FindFrameRight_ByTop(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight);
		//bool FindFrameRight_ByBottom(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight);
		//bool FindFrameLeft_ByTop(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostLeft);
		//bool FindFrameLeft_ByBottom(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostLeft);
		//// ��������� ����
		//bool FindByBackground(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight, int &nMostBottom, BOOL &bMarkBorder);
		//// ���������
		//bool ExpandDialogFrame(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int nFrameX, int nFrameY, int &nMostRight, int &nMostBottom);
		//void MarkDialog(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int nX1, int nY1, int nX2, int nY2, BOOL bMarkBorder = FALSE, BOOL bFindExterior = TRUE);
		//#define MAX_DETECTED_DIALOGS 20
		//	struct {
		//		int Count;
		//		SMALL_RECT Rects[MAX_DETECTED_DIALOGS];
		//		bool bWasFrame[MAX_DETECTED_DIALOGS];
		//	} m_DetectedDialogs;
};

//#define Assert(V) if ((V)==FALSE) { wchar_t szAMsg[MAX_PATH*2]; _wsprintf(szAMsg, SKIPLEN(countof(szAMsg)) L"Assertion (%s) at\n%s:%i\n\nPress <Retry> to report a bug (web page)", _T(#V), _T(__FILE__), __LINE__); CRealConsole::Box(szAMsg, MB_RETRYCANCEL); }
