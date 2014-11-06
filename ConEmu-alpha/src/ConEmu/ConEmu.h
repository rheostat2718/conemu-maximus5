﻿
/*
Copyright (c) 2009-2014 Maximus5
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

#if !defined(IID_IShellLink)
#define IID_IShellLink IID_IShellLinkW
#endif

#define GET_X_LPARAM(inPx) ((int)(short)LOWORD(inPx))
#define GET_Y_LPARAM(inPy) ((int)(short)HIWORD(inPy))

#ifndef __GNUC__
#include <intrin.h>
#else
#define _InterlockedIncrement InterlockedIncrement
#endif

//#define RCLICKAPPSTIMEOUT 600
//#define RCLICKAPPS_START 100 // начало отрисовки кружка вокруг курсора
//#define RCLICKAPPSTIMEOUT_MAX 10000
//#define RCLICKAPPSDELTA 3
//#define DRAG_DELTA 5

//typedef DWORD (WINAPI* FGetModuleFileNameEx)(HANDLE hProcess,HMODULE hModule,LPWSTR lpFilename,DWORD nSize);

//typedef HRESULT(WINAPI* FDwmIsCompositionEnabled)(BOOL *pfEnabled);

class CAttachDlg;
class CConEmuBack;
class CConEmuChild;
class CConEmuInside;
class CConEmuMacro;
class CConEmuMenu;
class CDefaultTerminal;
class CGestures;
class CRecreateDlg;
class CRunQueue;
class CStatus;
class CTabBarClass;
class CToolTip;
class CVConGroup;
class CVConGuard;
class MFileLog;
struct MSectionSimple;
struct MSectionLockSimple;
enum ConEmuWindowMode;
struct CEFindDlg;
union CESize;

struct MsgSrvStartedArg
{
	HWND  hConWnd;
	DWORD nSrcPID;
	DWORD dwKeybLayout;
	DWORD timeStart;
	DWORD timeRecv;
	DWORD timeFin;
	HWND  hWndDc;
	HWND  hWndBack;
};

struct ConsoleInfoArg
{
	CONSOLE_SCREEN_BUFFER_INFO sbi;
	SMALL_RECT srRealWindow;
	COORD crCursor;
	CONSOLE_CURSOR_INFO cInfo;
	TOPLEFTCOORD TopLeft;
};

#include "DwmHelper.h"
#include "TaskBar.h"
#include "FrameHolder.h"
#include "GuiServer.h"
#include "GestureEngine.h"
#include "ConEmuCtrl.h"
#include "ConEmuSize.h"
#include "../common/MArray.h"
#include "../common/MMap.h"
#include "../common/MFileMapping.h"

// IME support (WinXP or later)
typedef BOOL (WINAPI* ImmSetCompositionFontW_t)(HIMC hIMC, LPLOGFONT lplf);
typedef BOOL (WINAPI* ImmSetCompositionWindow_t)(HIMC hIMC, LPCOMPOSITIONFORM lpCompForm);
typedef HIMC (WINAPI* ImmGetContext_t)(HWND hWnd);


//struct GuiShellExecuteExArg
//{
//	CVirtualConsole* pVCon;
//	SHELLEXECUTEINFO* lpShellExecute;
//	HANDLE hReadyEvent;
//	BOOL bInProcess;
//	BOOL bResult;
//	DWORD dwErrCode;
//};

typedef LRESULT (*CallMainThreadFn)(LPARAM lParam);


typedef DWORD ConEmuInstallMode;
const ConEmuInstallMode
	cm_Normal       = 0x0000,
	cm_MinGW        = 0x0001, // ConEmu установлен как пакет MinGW
	cm_PortableApps = 0x0002, // ConEmu установлен как пакет PortableApps.com
	cm_MSysStartup  = 0x1000  // найден баш из msys: "%ConEmuDir%\..\msys\1.0\bin\sh.exe" (MinGW mode)
	;


class CConEmuMain
	: public CDwmHelper
	, public CTaskBar
	, public CFrameHolder
	, public CGestures
	, public CConEmuCtrl
	, public CConEmuSize
{
	public:
		wchar_t ms_ConEmuDefTitle[32];          // Название с версией, например "ConEmu 110117 (32)"
		wchar_t ms_ConEmuBuild[16];             // номер сборки, например "110117" или "131129dbg"
		wchar_t ms_ConEmuExe[MAX_PATH+1];       // полный путь к ConEmu.exe (GUI)
		wchar_t ms_ConEmuExeDir[MAX_PATH+1];    // БЕЗ завершающего слеша. Папка содержит ConEmu.exe
		wchar_t ms_ConEmuBaseDir[MAX_PATH+1];   // БЕЗ завершающего слеша. Папка содержит ConEmuC.exe, ConEmuHk.dll, ConEmu.xml
		wchar_t ms_ConEmuWorkDir[MAX_PATH+1];   // БЕЗ завершающего слеша. Папка запуска ConEmu.exe (GetCurrentDirectory)
		bool mb_ConEmuWorkDirArg;               // Was started as "ConEmu /Dir C:\abc ..." this must override "/dir" switch in task parameter
		void StoreWorkDir(LPCWSTR asNewCurDir = NULL);
		LPCWSTR WorkDir(LPCWSTR asOverrideCurDir = NULL);
		bool ChangeWorkDir(LPCWSTR asTempCurDir);
		wchar_t ms_ComSpecInitial[MAX_PATH];
		wchar_t *mps_IconPath;
		void SetWindowIcon(LPCWSTR asNewIcon);
		BOOL mb_DosBoxExists;
		ConEmuInstallMode m_InstallMode;
		bool isMingwMode();
		bool isMSysStartup();
		bool isUpdateAllowed();
		// Portable Far Registry
		BOOL mb_PortableRegExist;
		wchar_t ms_PortableRegHive[MAX_PATH]; // полный путь к "Portable.S-x-x-..."
		wchar_t ms_PortableRegHiveOrg[MAX_PATH]; // путь к "Portable.S-x-x-..." (в ConEmu). этот файл мог быть скопирован в ms_PortableRegHive
		wchar_t ms_PortableReg[MAX_PATH]; // "Portable.reg"
		HKEY mh_PortableMountRoot; // Это HKEY_CURRENT_USER или HKEY_USERS
		wchar_t ms_PortableMountKey[MAX_PATH];
		BOOL mb_PortableKeyMounted;
		wchar_t ms_PortableTempDir[MAX_PATH];
		HKEY mh_PortableRoot; // Это открытый ключ
		bool PreparePortableReg();
		bool mb_UpdateJumpListOnStartup;
		bool mb_FindBugMode;
		UINT mn_LastTransparentValue;
	private:
		struct
		{
			bool  bBlockChildrenDebuggers;
		} m_DbgInfo;
	private:
		bool CheckBaseDir();
		BOOL CheckDosBoxExists();
		void CheckPortableReg();
		void FinalizePortableReg();
		bool mb_ForceUseRegistry;
		wchar_t ms_ConEmuXml[MAX_PATH+1];       // полный путь к портабельным настройкам
		wchar_t ms_ConEmuIni[MAX_PATH+1];       // полный путь к портабельным настройкам
	public:
		bool SetConfigFile(LPCWSTR asFilePath, bool abWriteReq = false);
		void SetForceUseRegistry();
		LPWSTR ConEmuXml();
		LPWSTR ConEmuIni();
		wchar_t ms_ConEmuChm[MAX_PATH+1];       // полный путь к chm-файлу (help)
		wchar_t ms_ConEmuC32Full[MAX_PATH+12];  // полный путь к серверу (ConEmuC.exe) с длинными именами
		wchar_t ms_ConEmuC64Full[MAX_PATH+12];  // полный путь к серверу (ConEmuC64.exe) с длинными именами
		LPCWSTR ConEmuCExeFull(LPCWSTR asCmdLine=NULL);
		wchar_t *mpsz_ConEmuArgs;    // Аргументы
		void GetComSpecCopy(ConEmuComspec& ComSpec);
		void CreateGuiAttachMapping(DWORD nGuiAppPID);
		void InitComSpecStr(ConEmuComspec& ComSpec);
		bool IsResetBasicSettings();
		bool IsFastSetupDisabled();
		bool IsAllowSaveSettingsOnExit();
	private:
		ConEmuGuiMapping m_GuiInfo;
		MFileMapping<ConEmuGuiMapping> m_GuiInfoMapping;
		MFileMapping<ConEmuGuiMapping> m_GuiAttachMapping;
	public:
		void GetGuiInfo(ConEmuGuiMapping& GuiInfo);
		void GetAnsiLogInfo(ConEmuAnsiLog &AnsiLog);
	private:
		void FillConEmuMainFont(ConEmuMainFont* pFont);
		void UpdateGuiInfoMapping();
		static bool UpdateGuiInfoMappingFill(CVirtualConsole* pVCon, LPARAM lParam);
		void UpdateGuiInfoMappingActive(bool bActive, bool bUpdatePtr = true);
		bool mb_InCreateWindow;
		bool mb_LastTransparentFocused; // нужно для проверки gpSet->isTransparentSeparate
	public:
		bool InCreateWindow();
		bool InQuakeAnimation();
		UINT IsQuakeVisible();
	protected:
		struct WindowsOverQuake
		{
			RECT rcWnd;
			HRGN hRgn;
			int  iRc;
		};
		static BOOL CALLBACK EnumWindowsOverQuake(HWND hWnd, LPARAM lpData);
	public:
		//CConEmuChild *m_Child;
		//CConEmuBack  *m_Back;
		//CConEmuMacro *m_Macro;
		CConEmuMenu *mp_Menu;
		CTabBarClass *mp_TabBar;
		CConEmuInside *mp_Inside;
		CStatus *mp_Status;
		CToolTip *mp_Tip;
		MFileLog *mp_Log; MSectionSimple* mpcs_Log; // mcs_Log - для создания
		CDefaultTerminal *mp_DefTrm;
		CEFindDlg *mp_Find;
		CRunQueue *mp_RunQueue;

		bool CreateLog();
		void LogString(LPCWSTR asInfo, bool abWriteTime = true, bool abWriteLine = true);
		void LogString(LPCSTR asInfo, bool abWriteTime = true, bool abWriteLine = true);
		void LogMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		void LogWindowPos(LPCWSTR asPrefix);

	public:
		bool  WindowStartMinimized; // ключик "/min" или "Свернуть" в свойствах ярлыка
		bool  WindowStartTsa;       // ключики "/StartTSA" или "/MinTSA"
		bool  WindowStartNoClose;   // ключик "/MinTSA"
		bool  ForceMinimizeToTray;  // ключики "/tsa" или "/tray"
		bool  DisableAutoUpdate;    // ключик "/noupdate"
		bool  DisableKeybHooks;     // ключик "/nokeyhook"
		bool  DisableAllMacro;      // ключик "/nomacro"
		bool  DisableAllHotkeys;    // ключик "/nohotkey"
		bool  DisableSetDefTerm;    // ключик "/nodeftrm"
		bool  DisableRegisterFonts; // ключик "/noregfont"
		bool  DisableCloseConfirm;  // ключик "/nocloseconfirm"

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
			POINT LDblClkDC; // заполняется в PatchMouseEvent
			DWORD LDblClkTick;
			COORD RClkDC, RClkCon;
			DWORD RClkTick;

			// Для обработки gpSet->isActivateSplitMouseOver
			POINT  ptLastSplitOverCheck;

			// Чтобы не слать в консоль бесконечные WM_MOUSEMOVE
			UINT   lastMsg;
			WPARAM lastMMW;
			LPARAM lastMML;

			// Пропустить клик мышкой (окно было неактивно)
			UINT nSkipEvents[2]; UINT nReplaceDblClk;
			// не пропускать следующий клик в консоль!
			BOOL bForceSkipActivation;

			// таскание окошка за клиентскую область
			POINT ptWndDragStart;
			RECT  rcWndDragStart;

			// настройки скролла мышкой (сколько линий/символов "за клик")
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
				_ASSERTE(nWheelScrollChars<=3);
				return nWheelScrollChars;
			};
			UINT GetWheelScrollLines()
			{
				if (!nWheelScrollLines)
					ReloadWheelScroll();
				return nWheelScrollLines;
			};
		} mouse;
		struct SessionInfo
		{
			HMODULE hWtsApi;
			WPARAM  wState;     // session state change event
			LPARAM  lSessionID; // session ID

			typedef BOOL (WINAPI* WTSRegisterSessionNotification_t)(HWND hWnd, DWORD dwFlags);
			WTSRegisterSessionNotification_t pfnRegister;
			typedef BOOL (WINAPI* WTSUnRegisterSessionNotification_t)(HWND hWnd);
			WTSUnRegisterSessionNotification_t pfnUnregister;

			#define SESSION_LOG_SIZE 128
			struct EvtLog {
				DWORD  nTick;
				DWORD  wState;     // session state change event
				LPARAM lSessionID; // session ID
			} g_evt[SESSION_LOG_SIZE];
			LONG g_evtidx;
			void Log(WPARAM State, LPARAM SessionID);

			bool Connected();

			void SessionChanged(WPARAM State, LPARAM SessionID);

			void SetSessionNotification(bool bSwitch);
		} session;
		struct DpiInfo
		{
			// Win 8.1: shcore.dll
			enum Monitor_DPI_Type { MDT_Effective_DPI = 0, MDT_Angular_DPI = 1, MDT_Raw_DPI = 2, MDT_Default = MDT_Effective_DPI };
			typedef HRESULT (WINAPI* GetDPIForMonitor_t)(/*_In_*/HMONITOR hmonitor, /*_In_*/Monitor_DPI_Type dpiType, /*_Out_*/UINT *dpiX, /*_Out_*/UINT *dpiY);
			enum Process_DPI_Awareness { Process_DPI_Unaware = 0, Process_System_DPI_Aware = 1, Process_Per_Monitor_DPI_Aware = 2 };
			typedef HRESULT (WINAPI* SetProcessDPIAwareness_t)(/*_In_*/Process_DPI_Awareness value);
		} dpi;
		bool isPiewUpdate;
		HWND hPictureView; bool bPicViewSlideShow; DWORD dwLastSlideShowTick; RECT mrc_WndPosOnPicView;
		HWND mh_ShellWindow; // Окно Progman для Desktop режима
		DWORD mn_ShellWindowPID;
		BOOL mb_FocusOnDesktop;
		POINT cursor, Rcursor;
		bool mb_InCaptionChange;
		DWORD m_FixPosAfterStyle;
		RECT mrc_FixPosAfterStyle;
		DWORD m_ProcCount;
		BOOL mb_IsUacAdmin; // ConEmu itself is started elevated
		bool IsActiveConAdmin();
		HCURSOR mh_CursorWait, mh_CursorArrow, mh_CursorAppStarting, mh_CursorMove, mh_CursorIBeam;
		HCURSOR mh_SplitV, mh_SplitH;
		HCURSOR mh_DragCursor;
		CDragDrop *mp_DragDrop;
		bool mb_SkipOnFocus;
		bool mb_LastConEmuFocusState;
		DWORD mn_ForceTimerCheckLoseFocus; // GetTickCount()
		bool mb_AllowAutoChildFocus;
	public:
		void OnOurDialogOpened();
		void OnOurDialogClosed();
		void CheckAllowAutoChildFocus(DWORD nDeactivatedTID);
		bool isMenuActive();
		bool CanSetChildFocus();
		void SetScClosePending(bool bFlag);
		void PostScClose();
		bool OnScClose();
		bool isScClosing();
	protected:
		bool mb_ScClosePending; // Устанавливается в TRUE в CVConGroup::CloseQuery
	protected:
		friend class CVConGroup;

		friend class CGuiServer;
		CGuiServer m_GuiServer;

		TCHAR Title[MAX_TITLE_SIZE+192];
		TCHAR TitleTemplate[128];
		short mn_Progress;
		bool mb_InTimer;
		bool mb_ProcessCreated;
		bool mb_WorkspaceErasedOnClose;
		#ifndef _WIN64
		HWINEVENTHOOK mh_WinHook;
		static VOID CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD anEvent, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
		#endif
		CAttachDlg *mp_AttachDlg;
		CRecreateDlg *mp_RecreateDlg;
		bool mb_SkipSyncSize;
		BOOL mb_WaitCursor;
		typedef BOOL (WINAPI* FRegisterShellHookWindow)(HWND);
		struct
		{
			BOOL  bChecked;
			DWORD nReadyToSelNoEmpty;
			DWORD nReadyToSel;
		} m_Pressed;
	public:
		void OnTabbarActivated(bool bTabbarVisible);
	protected:
		BOOL mb_MouseCaptured;
		void CheckActiveLayoutName();
		void AppendHKL(wchar_t* szInfo, size_t cchInfoMax, HKL* hKeyb, int nCount);
		void AppendRegisteredLayouts(wchar_t* szInfo, size_t cchInfoMax);
		void StoreLayoutName(int iIdx, DWORD dwLayout, HKL hkl);
		DWORD_PTR m_ActiveKeybLayout;
		struct LayoutNames
		{
			BOOL      bUsed;
			DWORD     klName;
			DWORD_PTR hkl;
		} m_LayoutNames[20];
		struct TranslatedCharacters
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
		bool mb_HotKeyRegistered;
		HHOOK mh_LLKeyHook;
		HMODULE mh_LLKeyHookDll;
		HWND* mph_HookedGhostWnd;
		HMODULE LoadConEmuCD();
		void RegisterHotKeys();
		void RegisterGlobalHotKeys(bool bRegister);
	public:
		void GlobalHotKeyChanged();
	protected:
		void UnRegisterHotKeys(BOOL abFinal=FALSE);
		HBITMAP mh_RightClickingBmp; HDC mh_RightClickingDC;
		POINT m_RightClickingSize; // {384 x 16} 24 фрейма, считаем, что четверть отведенного времени прошла до начала показа
		int m_RightClickingFrames, m_RightClickingCurrent;
		BOOL mb_RightClickingPaint, mb_RightClickingLSent, mb_RightClickingRegistered;
		void StartRightClickingPaint();
		void StopRightClickingPaint();
		static LRESULT CALLBACK RightClickingProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
		HWND mh_RightClickingWnd;
		bool PatchMouseEvent(UINT messg, POINT& ptCurClient, POINT& ptCurScreen, WPARAM wParam, bool& isPrivate);
	public:
		wchar_t* LoadConsoleBatch(LPCWSTR asSource, RConStartArgs* pArgs = NULL);
	private:
		wchar_t* LoadConsoleBatch_File(LPCWSTR asSource);
		wchar_t* LoadConsoleBatch_Drops(LPCWSTR asSource);
		wchar_t* LoadConsoleBatch_Task(LPCWSTR asSource, RConStartArgs* pArgs = NULL);
	public:
		void RightClickingPaint(HDC hdcIntVCon, CVirtualConsole* apVCon);
		void RegisterMinRestore(bool abRegister);
		bool IsKeyboardHookRegistered();
		void RegisterHooks();
		void UnRegisterHooks(bool abFinal=false);
		void OnWmHotkey(WPARAM wParam);
		void UpdateWinHookSettings();
		void CtrlWinAltSpace();
		void DeleteVConMainThread(CVirtualConsole* apVCon);
		UINT GetRegisteredMessage(LPCSTR asLocal, LPCWSTR asGlobal = NULL);
		LRESULT CallMainThread(bool bSync, CallMainThreadFn fn, LPARAM lParam);
	private:
		UINT RegisterMessage(LPCSTR asLocal, LPCWSTR asGlobal = NULL);
		void RegisterMessages();
	protected:
		friend class CConEmuCtrl;
		friend class CRunQueue;
		friend class CConEmuSize;
		DWORD mn_MainThreadId;
		// Registered messages
		UINT mn__FirstAppMsg;
		MMap<UINT,LPCSTR> m__AppMsgs;
		UINT mn_MsgPostCreate;
		UINT mn_MsgPostCopy;
		UINT mn_MsgMyDestroy;
		UINT mn_MsgUpdateSizes;
		UINT mn_MsgUpdateCursorInfo;
		UINT mn_MsgSetWindowMode;
		UINT mn_MsgUpdateTitle;
		//UINT mn_MsgAttach;
		UINT mn_MsgSrvStarted;
		//UINT mn_MsgVConTerminated;
		UINT mn_MsgUpdateScrollInfo;
		UINT mn_MsgUpdateTabs; // = RegisterWindowMessage(CONEMUMSG_UPDATETABS);
		UINT mn_MsgOldCmdVer; BOOL mb_InShowOldCmdVersion;
		UINT mn_MsgTabCommand;
		UINT mn_MsgTabSwitchFromHook; /*BOOL mb_InWinTabSwitch;*/ // = RegisterWindowMessage(CONEMUMSG_SWITCHCON);
		//WARNING!!! mb_InWinTabSwitch - перенести в Keys!
		UINT mn_MsgWinKeyFromHook;
		//UINT mn_MsgConsoleHookedKey;
		UINT mn_MsgSheelHook;
		//UINT mn_ShellExecuteEx;
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
		UINT mn_MsgFontSetSize;
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
		UINT mn_MsgDeleteVConMainThread;
		UINT mn_MsgReqChangeCurPalette;
		UINT mn_MsgMacroExecSync;
		UINT mn_MsgActivateVCon;
		UINT mn_MsgPostScClose;
		UINT mn_MsgOurSysCommand;
		UINT mn_MsgCallMainThread;

		//
		virtual void OnUseGlass(bool abEnableGlass) override;
		virtual void OnUseTheming(bool abEnableTheming) override;
		virtual void OnUseDwm(bool abEnableDwm) override;

		bool ExecuteProcessPrepare();
		void ExecuteProcessFinished(bool bOpt);

	public:
		DWORD CheckProcesses();
		DWORD GetFarPID(BOOL abPluginRequired=FALSE);

	public:
		LPCWSTR GetDefaultTitle(); // вернуть ms_ConEmuDefTitle
		LPCWSTR GetDefaultTabLabel(); // L"ConEmu"
		LPCTSTR GetLastTitle(bool abUseDefault=true);
		LPCTSTR GetVConTitle(int nIdx);
		void SetTitleTemplate(LPCWSTR asTemplate);
		int GetActiveVCon(CVConGuard* pVCon = NULL, int* pAllCount = NULL);
		int isVConValid(CVirtualConsole* apVCon);
		void UpdateCursorInfo(const ConsoleInfoArg* pInfo);
		void UpdateProcessDisplay(BOOL abForce);
		void UpdateSizes();

	public:
		CConEmuMain();
		virtual ~CConEmuMain();

	public:
		BOOL Activate(CVirtualConsole* apVCon);
		int ActiveConNum(); // 0-based
		int GetConCount(); // количество открытых консолей
		void AskChangeBufferHeight();
		void AskChangeAlternative();
		void AttachToDialog();
		void CheckFocus(LPCWSTR asFrom);
		bool CheckRequiredFiles();
		void CheckUpdates(BOOL abShowMessages);
		DWORD isSelectionModifierPressed(bool bAllowEmpty);
		void ForceSelectionModifierPressed(DWORD nValue);
		enum DragPanelBorder CheckPanelDrag(COORD crCon);
		bool ConActivate(int nCon);
		bool ConActivateNext(BOOL abNext);
		bool CreateWnd(RConStartArgs *args);
		CVirtualConsole* CreateCon(RConStartArgs *args, bool abAllowScripts = false, bool abForceCurConsole = false);
		CVirtualConsole* CreateConGroup(LPCWSTR apszScript, bool abForceAsAdmin = false, LPCWSTR asStartupDir = NULL, const RConStartArgs *apDefArgs = NULL);
		LPCWSTR ParseScriptLineOptions(LPCWSTR apszLine, bool* rpbAsAdmin, bool* rpbSetActive, size_t cchNameMax=0, wchar_t* rsName/*[MAX_RENAME_TAB_LEN]*/=NULL);
		void CreateGhostVCon(CVirtualConsole* apVCon);
		BOOL CreateMainWindow();
		BOOL CreateWorkWindow();
		void DebugStep(LPCWSTR asMsg, BOOL abErrorSeverity=FALSE);
		void ForceShowTabs(BOOL abShow);
		DWORD_PTR GetActiveKeyboardLayout();
		bool isTabsShown();

	public:
		void Destroy();
	private:
		#ifdef _DEBUG
		bool mb_DestroySkippedInAssert;
		#endif

	public:
		DWORD GetWindowStyle();
		DWORD FixWindowStyle(DWORD dwExStyle, ConEmuWindowMode wmNewMode = wmCurrent);
		void SetWindowStyle(DWORD anStyle);
		void SetWindowStyle(HWND ahWnd, DWORD anStyle);
		DWORD GetWindowStyleEx();
		void SetWindowStyleEx(DWORD anStyleEx);
		void SetWindowStyleEx(HWND ahWnd, DWORD anStyleEx);
		DWORD GetWorkWindowStyle();
		DWORD GetWorkWindowStyleEx();
		BOOL Init();
		void InitInactiveDC(CVirtualConsole* apVCon);
		void Invalidate(LPRECT lpRect, BOOL bErase = TRUE);
		void InvalidateAll();
		void UpdateWindowChild(CVirtualConsole* apVCon);
		bool isCloseConfirmed();
		bool isDestroyOnClose(bool ScCloseOnEmpty = false);
		bool isConSelectMode();
		bool isConsolePID(DWORD nPID);
		bool isDragging();
		bool isFirstInstance(bool bFolderIgnore = false);
		bool isInImeComposition();
		bool isLBDown();
		bool isMeForeground(bool abRealAlso=false, bool abDialogsAlso=true, HWND* phFore=NULL);
		bool isMouseOverFrame(bool abReal=false);
		bool isNtvdm(BOOL abCheckAllConsoles=FALSE);
		bool isOurConsoleWindow(HWND hCon);
		bool isPictureView();
		bool isProcessCreated();
		bool isRightClickingPaint();
		bool isValid(CRealConsole* apRCon);
		bool isValid(CVirtualConsole* apVCon);
		bool isVConExists(int nIdx);
		bool isVConHWND(HWND hChild, CVConGuard* pVCon = NULL);
		bool isViewer();
		void LoadIcons();
		void MoveActiveTab(CVirtualConsole* apVCon, bool bLeftward);
		void InvalidateGaps();
		void PostDragCopy(BOOL abMove, BOOL abReceived=FALSE);
		void PostCreate(BOOL abReceived=FALSE);
		void PostCreateCon(RConStartArgs *pArgs);
		HWND PostCreateView(CConEmuChild* pChild);
		void PostFontSetSize(int nRelative/*0/1/2*/, int nValue/*для nRelative==0 - высота, для ==1 - +-1, +-2,... | 100%*/);
		void PostMacro(LPCWSTR asMacro);
		void PostMacroFontSetName(wchar_t* pszFontName, WORD anHeight /*= 0*/, WORD anWidth /*= 0*/, BOOL abPosted);
		void PostDisplayRConError(CRealConsole* apRCon, wchar_t* pszErrMsg);
		void PostChangeCurPalette(LPCWSTR pszPalette, bool bChangeDropDown, bool abPosted);
		LRESULT SyncExecMacro(WPARAM wParam, LPARAM lParam);
		bool PtDiffTest(POINT C, int aX, int aY, UINT D); //(((abs(C.x-LOWORD(lParam)))<D) && ((abs(C.y-HIWORD(lParam)))<D))
		void RecreateAction(RecreateActionParm aRecreate, BOOL abConfirm, RConBoolArg bRunAs = crb_Undefined);
		int RecreateDlg(RConStartArgs* apArg);
		bool ReportUpdateConfirmation();
		void ReportUpdateError();
		void RequestExitUpdate();
		void RequestPostUpdateTabs();
		BOOL RunSingleInstance(HWND hConEmuWnd = NULL, LPCWSTR apszCmd = NULL);
		void SetDragCursor(HCURSOR hCur);
		bool SetSkipOnFocus(bool abSkipOnFocus);
		void SetWaitCursor(BOOL abWait);
	public:
		void ReportOldCmdVersion(DWORD nCmd, DWORD nVersion, int bFromServer, DWORD nFromProcess, u64 hFromModule, DWORD nBits);
		bool SetParent(HWND hNewParent);
		void setFocus();
		bool StartDebugLogConsole();
		bool StartDebugActiveProcess();
		bool MemoryDumpActiveProcess();
		void SyncNtvdm();
		void SwitchKeyboardLayout(DWORD_PTR dwNewKeybLayout);
		BOOL TrackMouse();
		void SetSkipMouseEvent(UINT nMsg1, UINT nMsg2, UINT nReplaceDblClk);
		void Update(bool isForce = false);
		void UpdateActiveGhost(CVirtualConsole* apVCon);
	protected:
		void UpdateImeComposition();
	public:
		void CheckNeedUpdateTitle(LPCWSTR asRConTitle);
		void UpdateTitle();
		void UpdateProgress(/*BOOL abUpdateTitle*/);
		static LRESULT CALLBACK MainWndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK WorkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		BOOL isDialogMessage(MSG &Msg);
		bool isSkipNcMessage(const MSG& Msg);
		void PreWndProc(UINT messg);
		LRESULT WndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
	public:
		void OnSwitchGuiFocus(SwitchGuiFocusOp FocusOp);
		void OnAlwaysShowScrollbar(bool abSync = true);
		void OnBufferHeight();
		void OnConsoleKey(WORD vk, LPARAM Mods);
		LRESULT OnCreate(HWND hWnd, LPCREATESTRUCT lpCreate);
		LRESULT OnDestroy(HWND hWnd);
		LRESULT OnFlashWindow(WPARAM wParam, LPARAM lParam);
		void DoFlashWindow(CESERVER_REQ_FLASHWINFO* pFlash, bool bFromMacro);
		LRESULT OnFocus(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam, LPCWSTR asMsgFrom = NULL, BOOL abForceChild = FALSE);
		bool IsChildFocusAllowed(HWND hChild);
		void OnHideCaption();
		LRESULT OnKeyboard(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
		LRESULT OnKeyboardHook(DWORD VkMod);
		LRESULT OnKeyboardIme(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
		LRESULT OnLangChange(UINT messg, WPARAM wParam, LPARAM lParam);
		LRESULT OnLangChangeConsole(CVirtualConsole *apVCon, const DWORD adwLayoutName);
		LRESULT OnMouse(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
		LRESULT OnActivateByMouse(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
		LRESULT OnMouse_Move(CVirtualConsole* pVCon, HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam, POINT ptCur, COORD cr);
		LRESULT OnMouse_LBtnDown(CVirtualConsole* pVCon, HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam, POINT ptCur, COORD cr);
		LRESULT OnMouse_LBtnUp(CVirtualConsole* pVCon, HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam, POINT ptCur, COORD cr);
		LRESULT OnMouse_LBtnDblClk(CVirtualConsole* pVCon, HWND hWnd, UINT& messg, WPARAM wParam, LPARAM lParam, POINT ptCur, COORD cr);
		LRESULT OnMouse_RBtnDown(CVirtualConsole* pVCon, HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam, POINT ptCur, COORD cr);
		LRESULT OnMouse_RBtnUp(CVirtualConsole* pVCon, HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam, POINT ptCur, COORD cr);
		LRESULT OnMouse_RBtnDblClk(CVirtualConsole* pVCon, HWND hWnd, UINT& messg, WPARAM wParam, LPARAM lParam, POINT ptCur, COORD cr);
		BOOL OnMouse_NCBtnDblClk(HWND hWnd, UINT& messg, WPARAM wParam, LPARAM lParam);
		LRESULT OnSetCursor(WPARAM wParam=-1, LPARAM lParam=-1);
		LRESULT OnQueryEndSession(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT OnSessionChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT OnShellHook(WPARAM wParam, LPARAM lParam);
		UINT_PTR SetKillTimer(bool bEnable, UINT nTimerID, UINT nTimerElapse);
		LRESULT OnTimer(WPARAM wParam, LPARAM lParam);
		void OnTimer_Main(CVirtualConsole* pVCon);
		void OnTimer_ActivateSplit();
		void OnTimer_ConRedraw(CVirtualConsole* pVCon);
		void OnTimer_FrameAppearDisappear(WPARAM wParam);
		void OnTimer_RClickPaint();
		void OnTimer_AdmShield();
		void OnTimer_QuakeFocus();
		void OnActivateSplitChanged();
		void OnTransparent();
		void OnTransparent(bool abFromFocus/* = false*/, bool bSetFocus/* = false*/);
		void OnTransparentSeparate(bool bSetFocus);
		void OnVConCreated(CVirtualConsole* apVCon, const RConStartArgs *args);
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
		void OnDefaultTermChanged();
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

	public:
		// Windows7 - lock creating new consoles (ConHost search related)
		bool LockConhostStart();
		void UnlockConhostStart();
		void ReleaseConhostDelay();
	protected:
		struct LockConhostStart {
			MSectionSimple* pcs;
			MSectionLockSimple* pcsLock;
			bool wait;
		} m_LockConhostStart;
};

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
