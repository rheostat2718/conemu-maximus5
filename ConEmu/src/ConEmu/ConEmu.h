
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

#define WM_TRAYNOTIFY WM_USER+1

#include "MenuIds.h"

#define IID_IShellLink IID_IShellLinkW

#define GET_X_LPARAM(inPx) ((int)(short)LOWORD(inPx))
#define GET_Y_LPARAM(inPy) ((int)(short)HIWORD(inPy))

//#define RCLICKAPPSTIMEOUT 600
//#define RCLICKAPPS_START 100 // ������ ��������� ������ ������ �������
//#define RCLICKAPPSTIMEOUT_MAX 10000
//#define RCLICKAPPSDELTA 3
#define DRAG_DELTA 5

//typedef DWORD (WINAPI* FGetModuleFileNameEx)(HANDLE hProcess,HMODULE hModule,LPWSTR lpFilename,DWORD nSize);

//typedef HRESULT(WINAPI* FDwmIsCompositionEnabled)(BOOL *pfEnabled);

class CConEmuChild;
class CConEmuBack;
class TabBarClass;
class CConEmuMacro;
class CAttachDlg;
class CRecreateDlg;
class CToolTip;
class CGestures;
class CVConGuard;
class CVConGroup;
class CStatus;
enum ConEmuWindowMode;
class CDefaultTerminal;
class CConEmuMenu;


struct MsgSrvStartedArg
{
	HWND  hConWnd;
	DWORD nSrcPID;
	DWORD timeStart;
	DWORD timeRecv;
	DWORD timeFin;
	HWND  hWndDc;
	HWND  hWndBack;
};

#include "DwmHelper.h"
#include "TaskBar.h"
#include "FrameHolder.h"
#include "GuiServer.h"
#include "GestureEngine.h"
#include "ConEmuCtrl.h"
#include "../common/MArray.h"

// IME support (WinXP or later)
typedef BOOL (WINAPI* ImmSetCompositionFontW_t)(HIMC hIMC, LPLOGFONT lplf);
typedef BOOL (WINAPI* ImmSetCompositionWindow_t)(HIMC hIMC, LPCOMPOSITIONFORM lpCompForm);
typedef HIMC (WINAPI* ImmGetContext_t)(HWND hWnd);


struct GuiShellExecuteExArg
{
	CVirtualConsole* pVCon;
	SHELLEXECUTEINFO* lpShellExecute;
	HANDLE hReadyEvent;
	BOOL bInProcess;
	BOOL bResult;
	DWORD dwErrCode;
};



class CConEmuMain :
	public CDwmHelper,
	public CTaskBar,
	public CFrameHolder,
	public CGestures,
	public CConEmuCtrl
{
	public:
		//HMODULE mh_Psapi;
		//FGetModuleFileNameEx GetModuleFileNameEx;
		wchar_t ms_ConEmuDefTitle[32];          // �������� � �������, �������� "ConEmu 110117 (32)"
		wchar_t ms_ConEmuBuild[16];             // ����� ������, �������� "110117"
		wchar_t ms_ConEmuExe[MAX_PATH+1];       // ������ ���� � ConEmu.exe (GUI)
		wchar_t ms_ConEmuExeDir[MAX_PATH+1];    // ��� ������������ �����. ����� �������� ConEmu.exe
		wchar_t ms_ConEmuBaseDir[MAX_PATH+1];   // ��� ������������ �����. ����� �������� ConEmuC.exe, ConEmuHk.dll, ConEmu.xml
		wchar_t ms_ComSpecInitial[MAX_PATH];
		wchar_t *mps_IconPath;
		BOOL mb_DosBoxExists;
		BOOL mb_MingwMode;   // ConEmu ���������� ��� ����� MinGW
		BOOL mb_MSysStartup; // ������ ��� �� msys: "%ConEmuDir%\..\msys\1.0\bin\sh.exe" (MinGW mode)
		// Portable Far Registry
		BOOL mb_PortableRegExist;
		wchar_t ms_PortableRegHive[MAX_PATH]; // ������ ���� � "Portable.S-x-x-..."
		wchar_t ms_PortableRegHiveOrg[MAX_PATH]; // ���� � "Portable.S-x-x-..." (� ConEmu). ���� ���� ��� ���� ���������� � ms_PortableRegHive
		wchar_t ms_PortableReg[MAX_PATH]; // "Portable.reg"
		HKEY mh_PortableMountRoot; // ��� HKEY_CURRENT_USER ��� HKEY_USERS
		wchar_t ms_PortableMountKey[MAX_PATH];
		BOOL mb_PortableKeyMounted;
		wchar_t ms_PortableTempDir[MAX_PATH];
		HKEY mh_PortableRoot; // ��� �������� ����
		bool PreparePortableReg();
		bool mb_UpdateJumpListOnStartup;
	public:
		// ����� ����������. ����������� ��� �������� ����, ��������, � ������� ����������.
		enum {
			ii_None = 0,
			ii_Auto,
			ii_Explorer,
			ii_Simple,
		} m_InsideIntegration;
		bool  mb_InsideIntegrationShift;
		bool  mb_InsideSynchronizeCurDir;
		wchar_t* ms_InsideSynchronizeCurDir; // \ecd /d %1 - \e - ESC, \b - BS, \n - ENTER, %1 - "dir", %2 - "bash dir"
		bool  mb_InsidePaneWasForced;
		DWORD mn_InsideParentPID;  // PID "�������������" �������� ������ ����������
		HWND  mh_InsideParentWND; // ��� ���� ������������ ��� ������������ � ������ ����������
		HWND  mh_InsideParentRoot; // �������� ���� ������ ���������� (��� �������� isMeForeground)
		HWND  InsideFindParent();
	private:
		HWND  mh_InsideParentRel;  // ����� ���� NULL (ii_Simple). HWND ������������ �������� ����� �����������������
		HWND  mh_InsideParentPath; // Win7 Text = "Address: D:\MYDOC"
		HWND  mh_InsideParentCD;   // Edit ��� ����� ������� �����, �������� -> "C:\USERS"
		RECT  mrc_InsideParent, mrc_InsideParentRel; // ��� ���������, ���� �����, ��� ����������� ����
		wchar_t ms_InsideParentPath[MAX_PATH+1];
		static BOOL CALLBACK EnumInsideFindParent(HWND hwnd, LPARAM lParam);
		HWND  InsideFindConEmu(HWND hFrom);
		bool  InsideFindShellView(HWND hFrom);
		void  InsideParentMonitor();
		void  InsideUpdateDir();
		void  InsideUpdatePlacement();
	private:
		bool CheckBaseDir();
		BOOL CheckDosBoxExists();
		void CheckPortableReg();
		void FinalizePortableReg();
		wchar_t ms_ConEmuXml[MAX_PATH+1];       // ������ ���� � ������������ ����������
		wchar_t ms_ConEmuIni[MAX_PATH+1];       // ������ ���� � ������������ ����������
	public:
		bool SetConfigFile(LPCWSTR asFilePath, bool abWriteReq = false);
		LPWSTR ConEmuXml();
		LPWSTR ConEmuIni();
		wchar_t ms_ConEmuChm[MAX_PATH+1];       // ������ ���� � chm-����� (help)
		wchar_t ms_ConEmuC32Full[MAX_PATH+12];  // ������ ���� � ������� (ConEmuC.exe) � �������� �������
		wchar_t ms_ConEmuC64Full[MAX_PATH+12];  // ������ ���� � ������� (ConEmuC64.exe) � �������� �������
		LPCWSTR ConEmuCExeFull(LPCWSTR asCmdLine=NULL);
		//wchar_t ms_ConEmuCExeName[32];        // ��� ������� (ConEmuC.exe ��� ConEmuC64.exe) -- �� ��������
		wchar_t ms_ConEmuCurDir[MAX_PATH+1];    // ��� ������������ �����. ����� ������� ConEmu.exe (GetCurrentDirectory)
		void RefreshConEmuCurDir();
		wchar_t *mpsz_ConEmuArgs;    // ���������
		void GetComSpecCopy(ConEmuComspec& ComSpec);
		void CreateGuiAttachMapping(DWORD nGuiAppPID);
	private:
		ConEmuGuiMapping m_GuiInfo;
		MFileMapping<ConEmuGuiMapping> m_GuiInfoMapping;
		MFileMapping<ConEmuGuiMapping> m_GuiAttachMapping;
		void FillConEmuMainFont(ConEmuMainFont* pFont);
		void UpdateGuiInfoMapping();
		void UpdateGuiInfoMappingActive(bool bActive);
		int mn_QuakePercent; // 0 - ��������, ����� (>0 && <=100) - ���� �������� Quake
		bool mb_InCreateWindow;
		HMONITOR mh_MinFromMonitor;
	public:
		bool InCreateWindow();
		bool InQuakeAnimation();
	public:
		//CConEmuChild *m_Child;
		//CConEmuBack  *m_Back;
		//CConEmuMacro *m_Macro;
		CConEmuMenu *mp_Menu;
		TabBarClass *mp_TabBar;
		CStatus *mp_Status;
		CToolTip *mp_Tip;
		MFileLog *mp_Log;
		CDefaultTerminal *mp_DefTrm;
		void CreateLog();
		void LogString(LPCWSTR asInfo, bool abWriteTime = true, bool abWriteLine = true);
		void LogString(LPCSTR asInfo, bool abWriteTime = true, bool abWriteLine = true);
		void LogMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		void LogWindowPos(LPCWSTR asPrefix);
		//POINT cwShift; // difference between window size and client area size for main ConEmu window
		POINT ptFullScreenSize; // size for GetMinMaxInfo in Fullscreen mode

		ConEmuWindowMode WindowMode;           // wmNormal/wmMaximized/wmFullScreen
		ConEmuWindowMode changeFromWindowMode; // wmNotChanging/rmNormal/rmMaximized/rmFullScreen
		bool isRestoreFromMinimized;
		bool isWndNotFSMaximized; // �������� � true, ���� ��� �������� � FullScreen - ��� Maximized
		bool isQuakeMinimized;    // ������, ��� ������ ����� "Quake" ������ ������������ �� ��������
		HMONITOR GetNearestMonitor(MONITORINFO* pmi = NULL, LPRECT prcWnd = NULL);
		HMONITOR GetPrimaryMonitor(MONITORINFO* pmi = NULL);
		void StorePreMinimizeMonitor();

		DWORD wndWidth, wndHeight;  // � ��������
		int   wndX, wndY;           // � ��������

		bool  WindowStartMinimized; // ������ "/min" ��� "��������" � ��������� ������
		bool  ForceMinimizeToTray;  // ������� "/tsa" ��� "/tray"
		bool  DisableAutoUpdate;    // ������ "/noupdate"
		bool  DisableKeybHooks;     // ������ "/nokeyhook"
		bool  DisableSetDefTerm;    // ������ "/nodeftrm"
		bool  DisableRegisterFonts; // ������ "/noregfont"

		BOOL  mb_ExternalHidden;
		
		BOOL  mb_StartDetached;

		enum StartupStage {
			ss_Starting,
			ss_PostCreate1Called,
			ss_PostCreate1Finished,
			ss_PostCreate2Called,
			ss_PostCreate2Finished,
			ss_Started = ss_PostCreate2Finished
		} mn_StartupFinished;

		struct
		{
			WORD  state;
			bool  bSkipRDblClk;
			bool  bIgnoreMouseMove;
			bool  bCheckNormalRect;

			COORD LClkDC, LClkCon;
			POINT LDblClkDC; // ����������� � PatchMouseEvent
			DWORD LDblClkTick;
			COORD RClkDC, RClkCon;
			DWORD RClkTick;

			// ����� �� ����� � ������� ����������� WM_MOUSEMOVE
			UINT   lastMsg;
			WPARAM lastMMW;
			LPARAM lastMML;

			// ���������� ���� ������ (���� ���� ���������)
			UINT nSkipEvents[2]; UINT nReplaceDblClk;
			// �� ���������� ��������� ���� � �������!
			BOOL bForceSkipActivation;

			// �������� ������ �� ���������� �������
			POINT ptWndDragStart;
			RECT  rcWndDragStart;

			// ��������� ������� ������ (������� �����/�������� "�� ����")
			UINT nWheelScrollChars, nWheelScrollLines;
			void  ReloadWheelScroll()
			{
				#ifndef SPI_GETWHEELSCROLLCHARS
				#define SPI_GETWHEELSCROLLCHARS   0x006C
				#endif
				if (!SystemParametersInfo(SPI_GETWHEELSCROLLCHARS, 0, &nWheelScrollChars, 0) || !nWheelScrollChars)
					nWheelScrollChars = 3;
				if (!SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &nWheelScrollLines, 0) || !nWheelScrollLines)
					nWheelScrollLines = 3;
			};
			UINT GetWheelScrollChars()
			{
				if (!nWheelScrollChars)
					ReloadWheelScroll();
				return nWheelScrollChars;
			};
			UINT GetWheelScrollLines()
			{
				if (!nWheelScrollLines)
					ReloadWheelScroll();
				return nWheelScrollLines;
			};
		} mouse;
		bool isPiewUpdate;
		bool gbPostUpdateWindowSize;
		HWND hPictureView; bool bPicViewSlideShow; DWORD dwLastSlideShowTick; RECT mrc_WndPosOnPicView;
		HWND mh_ShellWindow; // ���� Progman ��� Desktop ������
		DWORD mn_ShellWindowPID;
		BOOL mb_FocusOnDesktop;
		//bool gb_ConsoleSelectMode;
		//bool setParent, setParent2;
		//BOOL mb_InClose;
		//int RBDownNewX, RBDownNewY;
		POINT cursor, Rcursor;
		//WPARAM lastMMW;
		//LPARAM lastMML;
		//COORD m_LastConSize; // console size after last resize (in columns and lines)
		bool mb_IgnoreSizeChange;
		bool mb_InCaptionChange;
		DWORD m_FixPosAfterStyle;
		RECT mrc_FixPosAfterStyle;
		//bool mb_IgnoreStoreNormalRect;
		//TCHAR szConEmuVersion[32];
		DWORD m_ProcCount;
		//DWORD mn_ActiveStatus;
		//TCHAR ms_EditorRus[16], ms_ViewerRus[16], ms_TempPanel[32], ms_TempPanelRus[32];
		//OSVERSIONINFO m_osv;
		BOOL mb_IsUacAdmin; // ConEmu itself is started elevated
		bool IsActiveConAdmin();
		HCURSOR mh_CursorWait, mh_CursorArrow, mh_CursorAppStarting, mh_CursorMove;
		HCURSOR mh_SplitV, mh_SplitH;
		HCURSOR mh_DragCursor;
		CDragDrop *mp_DragDrop;
		bool mb_SkipOnFocus;
		bool mb_LastConEmuFocusState;
		//DWORD mn_SysMenuOpenTick, mn_SysMenuCloseTick;
	protected:
		friend class CVConGroup;

		friend class CGuiServer;
		CGuiServer m_GuiServer;

		//CProgressBars *ProgressBars;
		//HMENU mh_DebugPopup, mh_EditPopup, mh_ActiveVConPopup, mh_TerminateVConPopup, mh_VConListPopup, mh_HelpPopup; // Popup's ��� SystemMenu
		//HMENU mh_InsideSysMenu;
		TCHAR Title[MAX_TITLE_SIZE+192]; //, TitleCmp[MAX_TITLE_SIZE+192]; //, MultiTitle[MAX_TITLE_SIZE+30];
		TCHAR TitleTemplate[128];
		short mn_Progress;
		//LPTSTR GetTitleStart();
		BOOL mb_InTimer;
		BOOL mb_ProcessCreated, mb_WorkspaceErasedOnClose; //DWORD mn_StartTick;
		bool mb_CloseGuiConfirmed;
		#ifndef _WIN64
		HWINEVENTHOOK mh_WinHook; //, mh_PopupHook;
		static VOID CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD anEvent, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
		#endif
		//CVirtualConsole *mp_VCon[MAX_CONSOLE_COUNT];
		//CVirtualConsole *mp_VActive, *mp_VCon1, *mp_VCon2;
		CAttachDlg *mp_AttachDlg;
		CRecreateDlg *mp_RecreateDlg;
		bool mb_SkipSyncSize;
		BOOL mb_WaitCursor;
		//BOOL mb_InTrackSysMenu; -> mn_TrackMenuPlace
		//TrackMenuPlace mn_TrackMenuPlace;
		BOOL mb_LastRgnWasNull;
		BOOL mb_LockWindowRgn;
		enum {
			fsf_Hide = 0,     // ����� � ��������� ��������
			fsf_WaitShow = 1, // ������� ������ ������ �����
			fsf_Show = 2,     // ����� ��������
		} m_ForceShowFrame;
		void StartForceShowFrame();
		void StopForceShowFrame();
		//wchar_t *mpsz_RecreateCmd;
		//ITaskbarList3 *mp_TaskBar3;
		//ITaskbarList2 *mp_TaskBar2;
		typedef BOOL (WINAPI* FRegisterShellHookWindow)(HWND);
		struct IdealRectInfo
		{
			// Current Ideal rect
			RECT  rcIdeal;
			// TODO: Reserved fields
			RECT  rcClientMargins; // (TabBar + StatusBar) at storing moment
			COORD crConsole;       // Console size in cells at storing moment
			SIZE  csFont;          // VCon Font size (Width, Height) at storing moment
		} mr_Ideal;
	public:
		void StoreIdealRect();
		RECT GetIdealRect();
	protected:
		BOOL mn_InResize;
		//bool mb_InScMinimize;
		RECT mrc_StoredNormalRect;
		void StoreNormalRect(RECT* prcWnd);
		//BOOL mb_MaximizedHideCaption; // � ������ HideCaption
		//BOOL mb_InRestore; // �� ����� �������������� �� Maximized
		BOOL mb_MouseCaptured;
		//BYTE m_KeybStates[256];
		DWORD_PTR m_ActiveKeybLayout;
		struct
		{
			BOOL      bUsed;
			//wchar_t   klName[KL_NAMELENGTH/*==9*/];
			DWORD     klName;
			DWORD_PTR hkl;
		} m_LayoutNames[20];
		struct
		{
			wchar_t szTranslatedChars[16];
		} m_TranslatedChars[256];
		BYTE mn_LastPressedVK;
		bool mb_InImeComposition, mb_ImeMethodChanged;
		wchar_t ms_ConEmuAliveEvent[MAX_PATH];
		BOOL mb_AliveInitialized;
		HANDLE mh_ConEmuAliveEvent; bool mb_ConEmuAliveOwned; DWORD mn_ConEmuAliveEventErr;
		HANDLE mh_ConEmuAliveEventNoDir; bool mb_ConEmuAliveOwnedNoDir; DWORD mn_ConEmuAliveEventErrNoDir;
		//
		BOOL mb_HotKeyRegistered;
		HHOOK mh_LLKeyHook;
		HMODULE mh_LLKeyHookDll;
		HWND* mph_HookedGhostWnd;
		HMODULE LoadConEmuCD();
		void RegisterHotKeys();
		void UnRegisterHotKeys(BOOL abFinal=FALSE);
		//int mn_MinRestoreRegistered; UINT mn_MinRestore_VK, mn_MinRestore_MOD;
		//HMODULE mh_DwmApi;
		//FDwmIsCompositionEnabled DwmIsCompositionEnabled;
		HBITMAP mh_RightClickingBmp; HDC mh_RightClickingDC;
		POINT m_RightClickingSize; // {384 x 16} 24 ������, �������, ��� �������� ����������� ������� ������ �� ������ ������
		int m_RightClickingFrames, m_RightClickingCurrent;
		BOOL mb_RightClickingPaint, mb_RightClickingLSent, mb_RightClickingRegistered;
		void StartRightClickingPaint();
		void StopRightClickingPaint();
		static LRESULT CALLBACK RightClickingProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
		HWND mh_RightClickingWnd;
		bool PatchMouseEvent(UINT messg, POINT& ptCurClient, POINT& ptCurScreen, WPARAM wParam, bool& isPrivate);
		void CheckTopMostState();
	public:
		wchar_t* LoadConsoleBatch(LPCWSTR asSource, wchar_t** ppszStartupDir = NULL);
	private:
		wchar_t* LoadConsoleBatch_File(LPCWSTR asSource);
		wchar_t* LoadConsoleBatch_Drops(LPCWSTR asSource);
		wchar_t* LoadConsoleBatch_Task(LPCWSTR asSource, wchar_t** ppszStartupDir = NULL);
	public:
		void RightClickingPaint(HDC hdcIntVCon, CVirtualConsole* apVCon);
		void RegisterMinRestore(bool abRegister);
		void RegisterHoooks();
		void UnRegisterHoooks(BOOL abFinal=FALSE);
		void OnWmHotkey(WPARAM wParam);
		void UpdateWinHookSettings();
		void CtrlWinAltSpace();
	protected:
		friend class CConEmuCtrl;
		//BOOL LowLevelKeyHook(UINT nMsg, UINT nVkKeyCode);
		//DWORD_PTR mn_CurrentKeybLayout;
		// Registered messages
		DWORD mn_MainThreadId;
		UINT mn_MsgPostCreate;
		UINT mn_MsgPostCopy;
		UINT mn_MsgMyDestroy;
		UINT mn_MsgUpdateSizes;
		UINT mn_MsgUpdateCursorInfo;
		UINT mn_MsgSetWindowMode;
		UINT mn_MsgUpdateTitle;
		//UINT mn_MsgAttach;
		UINT mn_MsgSrvStarted;
		UINT mn_MsgVConTerminated;
		UINT mn_MsgUpdateScrollInfo;
		UINT mn_MsgUpdateTabs; // = RegisterWindowMessage(CONEMUMSG_UPDATETABS);
		UINT mn_MsgOldCmdVer; BOOL mb_InShowOldCmdVersion;
		UINT mn_MsgTabCommand;
		UINT mn_MsgTabSwitchFromHook; /*BOOL mb_InWinTabSwitch;*/ // = RegisterWindowMessage(CONEMUMSG_SWITCHCON);
		//WARNING!!! mb_InWinTabSwitch - ��������� � Keys!
		UINT mn_MsgWinKeyFromHook;
		//UINT mn_MsgConsoleHookedKey;
		UINT mn_MsgSheelHook;
		UINT mn_ShellExecuteEx;
		UINT mn_PostConsoleResize;
		UINT mn_ConsoleLangChanged;
		UINT mn_MsgPostOnBufferHeight;
		UINT mn_MsgPostAltF9;
		//UINT mn_MsgPostSetBackground;
		UINT mn_MsgInitInactiveDC;
		//UINT mn_MsgSetForeground;
		UINT mn_MsgFlashWindow; // = RegisterWindowMessage(CONEMUMSG_FLASHWINDOW);
		//UINT mn_MsgActivateCon; // = RegisterWindowMessage(CONEMUMSG_ACTIVATECON);
		UINT mn_MsgUpdateProcDisplay;
		//UINT wmInputLangChange;
		UINT mn_MsgAutoSizeFont;
		UINT mn_MsgDisplayRConError;
		UINT mn_MsgMacroFontSetName;
		UINT mn_MsgCreateViewWindow;
		UINT mn_MsgPostTaskbarActivate; BOOL mb_PostTaskbarActivate;
		UINT mn_MsgInitVConGhost;
		UINT mn_MsgCreateCon;
		UINT mn_MsgRequestUpdate;
		UINT mn_MsgTaskBarCreated;
		UINT mn_MsgPanelViewMapCoord;
		UINT mn_MsgTaskBarBtnCreated;

		//
		virtual void OnUseGlass(bool abEnableGlass);
		virtual void OnUseTheming(bool abEnableTheming);
		virtual void OnUseDwm(bool abEnableDwm);

		//
		void InitCommCtrls();
		bool mb_CommCtrlsInitialized;
		HWND mh_AboutDlg;
		static INT_PTR CALLBACK aboutProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam);

		//
		CRITICAL_SECTION mcs_ShellExecuteEx;
		MArray<GuiShellExecuteExArg*> m_ShellExecuteQueue;
		void GuiShellExecuteExQueue();
		bool mb_InShellExecuteQueue;

	public:
		DWORD CheckProcesses();
		DWORD GetFarPID(BOOL abPluginRequired=FALSE);

	public:
		LPCWSTR GetDefaultTitle(); // ������� ms_ConEmuVer
		LPCTSTR GetLastTitle(bool abUseDefault=true);
		LPCTSTR GetVConTitle(int nIdx);
		void SetTitleTemplate(LPCWSTR asTemplate);
		int GetActiveVCon(CVConGuard* pVCon = NULL, int* pAllCount = NULL);
		CVirtualConsole* GetVCon(int nIdx, bool bFromCycle = false);
		int isVConValid(CVirtualConsole* apVCon);
		CVirtualConsole* GetVConFromPoint(POINT ptScreen);
		void UpdateCursorInfo(const CONSOLE_SCREEN_BUFFER_INFO* psbi, COORD crCursor, CONSOLE_CURSOR_INFO cInfo);
		void UpdateProcessDisplay(BOOL abForce);
		void UpdateSizes();

	public:
		CConEmuMain();
		virtual ~CConEmuMain();

	public:
		//CVirtualConsole* ActiveCon();
		BOOL Activate(CVirtualConsole* apVCon);
		int ActiveConNum(); // 0-based
		int GetConCount(); // ���������� �������� ��������
		static void AddMargins(RECT& rc, RECT& rcAddShift, BOOL abExpand=FALSE);
		void AskChangeBufferHeight();
		void AskChangeAlternative();
		void AttachToDialog();
		BOOL AttachRequested(HWND ahConWnd, const CESERVER_REQ_STARTSTOP* pStartStop, CESERVER_REQ_STARTSTOPRET* pRet);
		CRealConsole* AttachRequestedGui(LPCWSTR asAppFileName, DWORD anAppPID);
		void AutoSizeFont(RECT arFrom, enum ConEmuRect tFrom);
		RECT CalcMargins(DWORD/*enum ConEmuMargins*/ mg, ConEmuWindowMode wmNewMode = wmCurrent);
		RECT CalcRect(enum ConEmuRect tWhat, CVirtualConsole* pVCon=NULL);
		RECT CalcRect(enum ConEmuRect tWhat, const RECT &rFrom, enum ConEmuRect tFrom, CVirtualConsole* pVCon=NULL, enum ConEmuMargins tTabAction=CEM_TAB);
		bool FixWindowRect(RECT& rcWnd, DWORD nBorders /* enum of ConEmuBorders */);
		//POINT CalcTabMenuPos(CVirtualConsole* apVCon);
		void CheckFocus(LPCWSTR asFrom);
		bool CheckRequiredFiles();
		void CheckUpdates(BOOL abShowMessages);
		enum DragPanelBorder CheckPanelDrag(COORD crCon);
		bool ConActivate(int nCon);
		bool ConActivateNext(BOOL abNext);
		//bool CorrectWindowPos(WINDOWPOS *wp);
		//void CheckGuiBarsCreated();
		bool CreateWnd(RConStartArgs *args);
		CVirtualConsole* CreateCon(RConStartArgs *args, bool abAllowScripts = false, bool abForceCurConsole = false);
		CVirtualConsole* CreateConGroup(LPCWSTR apszScript, bool abForceAsAdmin = false, LPCWSTR asStartupDir = NULL);
		LPCWSTR ParseScriptLineOptions(LPCWSTR apszLine, bool* rpbAsAdmin, bool* rpbSetActive, size_t cchNameMax=0, wchar_t* rsName/*[MAX_RENAME_TAB_LEN]*/=NULL);
		void CreateGhostVCon(CVirtualConsole* apVCon);
		BOOL CreateMainWindow();
		BOOL CreateWorkWindow();
		HRGN CreateWindowRgn(bool abTestOnly=false);
		HRGN CreateWindowRgn(bool abTestOnly,bool abRoundTitle,int anX, int anY, int anWndWidth, int anWndHeight);
		void Destroy();
		void DebugStep(LPCWSTR asMsg, BOOL abErrorSeverity=FALSE);
		void ForceShowTabs(BOOL abShow);
		DWORD_PTR GetActiveKeyboardLayout();
		RECT GetDefaultRect();
		RECT GetGuiClientRect();
		//HMENU GetSysMenu(BOOL abInitial = FALSE);
	//protected:
	//	void UpdateSysMenu(HMENU hSysMenu);
	public:
		RECT GetVirtualScreenRect(BOOL abFullScreen);
		DWORD GetWindowStyle();
		DWORD FixWindowStyle(DWORD dwExStyle, ConEmuWindowMode wmNewMode = wmCurrent);
		void SetWindowStyle(DWORD anStyle);
		void SetWindowStyle(HWND ahWnd, DWORD anStyle);
		DWORD GetWindowStyleEx();
		void SetWindowStyleEx(DWORD anStyleEx);
		void SetWindowStyleEx(HWND ahWnd, DWORD anStyleEx);
		DWORD GetWorkWindowStyle();
		DWORD GetWorkWindowStyleEx();
		LRESULT GuiShellExecuteEx(SHELLEXECUTEINFO* lpShellExecute, CVirtualConsole* apVCon);
		BOOL Init();
		void InitInactiveDC(CVirtualConsole* apVCon);
		void Invalidate(CVirtualConsole* apVCon);
		void InvalidateAll();
		void UpdateWindowChild(CVirtualConsole* apVCon);
		bool isActive(CVirtualConsole* apVCon, bool abAllowGroup = true);
		bool isChildWindow();
		bool isCloseConfirmed();
		bool isConSelectMode();
		bool isConsolePID(DWORD nPID);
		bool isDragging();
		bool isEditor();
		bool isFar(bool abPluginRequired=false);
		bool isFarExist(CEFarWindowType anWindowType=fwt_Any, LPWSTR asName=NULL, CVConGuard* rpVCon=NULL);
		bool isFilePanel(bool abPluginAllowed=false);
		bool isFirstInstance(bool bFolderIgnore = false);
		bool isFullScreen();
		//bool IsGlass();		
		bool isIconic(bool abRaw = false);
		bool isInImeComposition();		
		bool isLBDown();
		bool isMainThread();
		bool isMeForeground(bool abRealAlso=false, bool abDialogsAlso=true);
		bool isMouseOverFrame(bool abReal=false);		
		bool isNtvdm(BOOL abCheckAllConsoles=FALSE);		
		bool isOurConsoleWindow(HWND hCon);		
		bool isPictureView();		
		bool isProcessCreated();		
		bool isRightClickingPaint();		
		bool isSizing();
		bool isValid(CRealConsole* apRCon);
		bool isValid(CVirtualConsole* apVCon);
		bool isVConExists(int nIdx);
		bool isVConHWND(HWND hChild, CVConGuard* pVCon = NULL);
		bool isViewer();
		bool isVisible(CVirtualConsole* apVCon);
		bool isWindowNormal();
		bool isZoomed();
		void LoadIcons();
		//bool LoadVersionInfo(wchar_t* pFullPath);
		//RECT MapRect(RECT rFrom, BOOL bFrame2Client);
		void MoveActiveTab(CVirtualConsole* apVCon, bool bLeftward);
		//void PaintCon(HDC hPaintDC);
		void InvalidateGaps();
		//void PaintGaps(HDC hDC);
		void PostAutoSizeFont(int nRelative/*0/1*/, int nValue/*��� nRelative==0 - ������, ��� ==1 - +-1, +-2,...*/);
		void PostDragCopy(BOOL abMove, BOOL abRecieved=FALSE);
		void PostCreate(BOOL abRecieved=FALSE);
		void PostCreateCon(RConStartArgs *pArgs);
		HWND PostCreateView(CConEmuChild* pChild);
		void PostMacro(LPCWSTR asMacro);
		void PostMacroFontSetName(wchar_t* pszFontName, WORD anHeight /*= 0*/, WORD anWidth /*= 0*/, BOOL abPosted);
		void PostDisplayRConError(CRealConsole* apRCon, wchar_t* pszErrMsg);
		//void PostSetBackground(CVirtualConsole* apVCon, CESERVER_REQ_SETBACKGROUND* apImgData);
		bool PtDiffTest(POINT C, int aX, int aY, UINT D); //(((abs(C.x-LOWORD(lParam)))<D) && ((abs(C.y-HIWORD(lParam)))<D))
		void RecreateAction(RecreateActionParm aRecreate, BOOL abConfirm, BOOL abRunAs = FALSE);
		int RecreateDlg(RConStartArgs* apArg);
		//void RePaint();
		bool ReportUpdateConfirmation();
		void ReportUpdateError();
		void RequestExitUpdate();
		void RequestPostUpdateTabs();
		void ReSize(BOOL abCorrect2Ideal = FALSE);
		//void ResizeChildren();
		BOOL RunSingleInstance(HWND hConEmuWnd = NULL, LPCWSTR apszCmd = NULL);
		bool ScreenToVCon(LPPOINT pt, CVirtualConsole** ppVCon);
		//void SetAllConsoleWindowsSize(const COORD& size, /*bool updateInfo,*/ bool bSetRedraw = false);
		void SetDragCursor(HCURSOR hCur);
		void SetSkipOnFocus(bool abSkipOnFocus);
		void SetWaitCursor(BOOL abWait);
		ConEmuWindowMode GetWindowMode();
		bool SetWindowMode(ConEmuWindowMode inMode, BOOL abForce = FALSE, BOOL abFirstShow = FALSE);
		bool SetQuakeMode(BYTE NewQuakeMode, ConEmuWindowMode nNewWindowMode = wmNotChanging, bool bFromDlg = false);
	private:
		struct {
			bool bWasSaved;
			bool bWaitReposition; // ��������� ����� ������� ��� OnHideCaption
			DWORD wndWidth, wndHeight; // �������
			DWORD wndX, wndY; // GUI
			DWORD nFrame;
			ConEmuWindowMode WindowMode;
			IdealRectInfo rcIdealInfo;
		} m_QuakePrevSize;
	public:
		//void ShowMenuHint(HMENU hMenu, WORD nID, WORD nFlags);
		//void ShowKeyBarHint(HMENU hMenu, WORD nID, WORD nFlags);
		BOOL ShowWindow(int anCmdShow, DWORD nAnimationMS = 0);
		void ReportOldCmdVersion(DWORD nCmd, DWORD nVersion, int bFromServer, DWORD nFromProcess, u64 hFromModule, DWORD nBits);
		//virtual void ShowSysmenu(int x=-32000, int y=-32000, bool bAlignUp = false);
		bool SetParent(HWND hNewParent);
		//HMENU CreateDebugMenuPopup();
		//HMENU CreateEditMenuPopup(CVirtualConsole* apVCon, HMENU ahExist = NULL);
		//HMENU CreateHelpMenuPopup();
		//HMENU CreateVConListPopupMenu(HMENU ahExist, BOOL abFirstTabOnly);
		//HMENU CreateVConPopupMenu(CVirtualConsole* apVCon, HMENU ahExist, BOOL abAddNew, HMENU& hTerminate);
		void setFocus();
		bool StartDebugLogConsole();
		bool StartDebugActiveProcess();
		bool MemoryDumpActiveProcess();
		//void StartLogCreateProcess();
		//void StopLogCreateProcess();
		//void UpdateLogCreateProcess();
		//wchar_t ms_LogCreateProcess[MAX_PATH]; bool mb_CreateProcessLogged;
		void SyncConsoleToWindow(LPRECT prcNewWnd=NULL, bool bSync=false);
		void SyncNtvdm();
		void SyncWindowToConsole(); // -- ������� ������, ������������
		void SwitchKeyboardLayout(DWORD_PTR dwNewKeybLayout);
		//void TabCommand(UINT nTabCmd);
		BOOL TrackMouse();
		//int trackPopupMenu(TrackMenuPlace place, HMENU hMenu, UINT uFlags, int x, int y, HWND hWnd, RECT *prcRect = NULL);
		void Update(bool isForce = false);
		void UpdateActiveGhost(CVirtualConsole* apVCon);
		void UpdateFarSettings();
		//void UpdateIdealRect(BOOL abAllowUseConSize=FALSE);
	protected:
		void UpdateIdealRect(RECT rcNewIdeal);
		void UpdateImeComposition();
	public:
		void UpdateTextColorSettings(BOOL ChangeTextAttr = TRUE, BOOL ChangePopupAttr = TRUE);
		void UpdateTitle(/*LPCTSTR asNewTitle*/);
		void UpdateProgress(/*BOOL abUpdateTitle*/);
		void UpdateWindowRgn(int anX=-1, int anY=-1, int anWndWidth=-1, int anWndHeight=-1);
		static LRESULT CALLBACK MainWndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK WorkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		BOOL isDialogMessage(MSG &Msg);
		//LPCWSTR MenuAccel(int DescrID, LPCWSTR asText);
		LRESULT WndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
	public:
		void OnAltEnter();
		void OnAltF9(BOOL abPosted=FALSE);
		void OnMinimizeRestore(SingleInstanceShowHideType ShowHideType = sih_None);
		void OnForcedFullScreen(bool bSet = true);
		void OnAlwaysOnTop();
		void OnAlwaysShowScrollbar(bool abSync = true);
		void OnBufferHeight();
		//bool DoClose();
		//BOOL mb_InConsoleResize;
		void OnConsoleKey(WORD vk, LPARAM Mods);
		void OnConsoleResize(BOOL abPosted=FALSE);
		void OnCopyingState();
		LRESULT OnCreate(HWND hWnd, LPCREATESTRUCT lpCreate);
		void OnDesktopMode();
		LRESULT OnDestroy(HWND hWnd);
		LRESULT OnFlashWindow(DWORD nFlags, DWORD nCount, HWND hCon);
		LRESULT OnFocus(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam, LPCWSTR asMsgFrom = NULL);
		LRESULT OnGetMinMaxInfo(LPMINMAXINFO pInfo);
		void OnHideCaption();
		void OnInfo_About(LPCWSTR asPageName = NULL);
		void OnInfo_Help();
		void OnInfo_HomePage();
		void OnInfo_ReportBug();
		//LRESULT OnInitMenuPopup(HWND hWnd, HMENU hMenu, LPARAM lParam);
		LRESULT OnKeyboard(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
		LRESULT OnKeyboardHook(DWORD VkMod);
		LRESULT OnKeyboardIme(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
		LRESULT OnLangChange(UINT messg, WPARAM wParam, LPARAM lParam);
		LRESULT OnLangChangeConsole(CVirtualConsole *apVCon, DWORD dwLayoutName);
		LRESULT OnMouse(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
		LRESULT OnMouse_Move(CVirtualConsole* pVCon, HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam, POINT ptCur, COORD cr);
		LRESULT OnMouse_LBtnDown(CVirtualConsole* pVCon, HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam, POINT ptCur, COORD cr);
		LRESULT OnMouse_LBtnUp(CVirtualConsole* pVCon, HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam, POINT ptCur, COORD cr);
		LRESULT OnMouse_LBtnDblClk(CVirtualConsole* pVCon, HWND hWnd, UINT& messg, WPARAM wParam, LPARAM lParam, POINT ptCur, COORD cr);
		LRESULT OnMouse_RBtnDown(CVirtualConsole* pVCon, HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam, POINT ptCur, COORD cr);
		LRESULT OnMouse_RBtnUp(CVirtualConsole* pVCon, HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam, POINT ptCur, COORD cr);
		LRESULT OnMouse_RBtnDblClk(CVirtualConsole* pVCon, HWND hWnd, UINT& messg, WPARAM wParam, LPARAM lParam, POINT ptCur, COORD cr);
		BOOL OnMouse_NCBtnDblClk(HWND hWnd, UINT& messg, WPARAM wParam, LPARAM lParam);
		//virtual void OnPaintClient(HDC hdc/*, int width, int height*/);
		LRESULT OnSetCursor(WPARAM wParam=-1, LPARAM lParam=-1);
		LRESULT OnSize(bool bResizeRCon=true, WPARAM wParam=0, WORD newClientWidth=(WORD)-1, WORD newClientHeight=(WORD)-1);
		LRESULT OnSizing(WPARAM wParam, LPARAM lParam);
		LRESULT OnMoving(LPRECT prcWnd = NULL, bool bWmMove = false);
		virtual LRESULT OnWindowPosChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT OnWindowPosChanging(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		void OnSizePanels(COORD cr);
		LRESULT OnShellHook(WPARAM wParam, LPARAM lParam);
		//LRESULT OnSysCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
		LRESULT OnTimer(WPARAM wParam, LPARAM lParam);
		void OnTimer_Main(CVirtualConsole* pVCon);
		void OnTimer_ConRedraw(CVirtualConsole* pVCon);
		void OnTimer_FrameAppearDisappear(WPARAM wParam);
		void OnTimer_RClickPaint();
		void OnTimer_AdmShield();
		void OnTransparent(bool abFromFocus = false, bool bSetFocus = true);
		void OnVConCreated(CVirtualConsole* apVCon, const RConStartArgs *args);
		LRESULT OnVConClosed(CVirtualConsole* apVCon, BOOL abPosted = FALSE);
		void OnAllVConClosed();
		void OnAllGhostClosed();
		void OnGhostCreated(CVirtualConsole* apVCon, HWND ahGhost);
		void OnRConStartedSuccess(CRealConsole* apRCon);
		LRESULT OnUpdateScrollInfo(BOOL abPosted = FALSE);
		void OnPanelViewSettingsChanged(BOOL abSendChanges=TRUE);
		void OnGlobalSettingsChanged();
		void OnTaskbarCreated();
		void OnTaskbarButtonCreated();
		void OnTaskbarSettingsChanged();
		#ifdef __GNUC__
		AlphaBlend_t GdiAlphaBlend;
		#endif
		void OnActiveConWndStore(HWND hConWnd);

		// return true - when state was changes
		bool SetTransparent(HWND ahWnd, UINT anAlpha/*0..255*/, bool abColorKey = false, COLORREF acrColorKey = 0, bool abForceLayered = false);
		GetLayeredWindowAttributes_t _GetLayeredWindowAttributes;
		#ifdef __GNUC__
		SetLayeredWindowAttributes_t SetLayeredWindowAttributes;
		#endif

		// IME support (WinXP or later)
		HMODULE mh_Imm32;
		ImmSetCompositionFontW_t _ImmSetCompositionFont;
		ImmSetCompositionWindow_t _ImmSetCompositionWindow;
		ImmGetContext_t _ImmGetContext;
};

#ifndef __GNUC__
#include <intrin.h>
#else
#define _InterlockedIncrement InterlockedIncrement
#endif

// Message Logger
// Originally from http://preshing.com/20120522/lightweight-in-memory-logging
namespace ConEmuMsgLogger
{
	enum Source {
		msgCommon,
		msgMain,
		msgWork,
		msgApp,
		msgCanvas,
		msgBack,
		msgGhost,
	};
	static const int BUFFER_SIZE = RELEASEDEBUGTEST(0x1000,0x1000);   // Must be a power of 2
	extern MSG g_msg[BUFFER_SIZE];
	extern LONG g_msgidx;

	static const int BUFFER_POS_SIZE = RELEASEDEBUGTEST(0x100,0x100);   // Must be a power of 2
	struct Event {
		short x, y, w, h;
		enum {
			Empty,
			WindowPosChanging,
			WindowPosChanged,
			Sizing,
			Size,
			Moving,
			Move,
		} msg;
		DWORD time;
		HWND hwnd;
	};
	extern Event g_pos[BUFFER_POS_SIZE];
	extern LONG g_posidx;
	extern void LogPos(const MSG& msg, Source from);
 
	inline void Log(const MSG& msg, Source from)
	{
		// Get next message index
		LONG i = _InterlockedIncrement(&g_msgidx);
		// Write a message at this index
		g_msg[i & (BUFFER_SIZE - 1)] = msg; // Wrap to buffer size
		switch (msg.message)
		{
		case WM_WINDOWPOSCHANGING:
		case WM_WINDOWPOSCHANGED:
		case WM_MOVE:
		case WM_SIZE:
		case WM_SIZING:
		case WM_MOVING:
			LogPos(msg, from);
			break;
		}
	}
}

extern CConEmuMain *gpConEmu;
