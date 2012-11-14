
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

#define HIDE_USE_EXCEPTION_INFO
#ifdef _DEBUG
//  #define SHOW_GUIATTACH_START
#endif


#define SHOWDEBUGSTR

#define CHILD_DESK_MODE
#define REGPREPARE_EXTERNAL
//#define CATCH_TOPMOST_SET

#include "Header.h"
#include "About.h"
#include <Tlhelp32.h>
#include <Shlobj.h>
//#include <lm.h>
//#include "../common/ConEmuCheck.h"
#include "VirtualConsole.h"
#include "RealBuffer.h"
#include "options.h"
#include "DragDrop.h"
#include "TrayIcon.h"
#include "VConChild.h"
#include "VConGroup.h"
#include "GestureEngine.h"
#include "ConEmu.h"
#include "ConEmuApp.h"
#include "TabBar.h"
#include "ConEmuPipe.h"
#include "version.h"
#include "Macro.h"
#include "Attach.h"
#include "Recreate.h"
#include "Update.h"
#include "LoadImg.h"
#include "Status.h"
#include "../ConEmuCD/RegPrepare.h"
#include "../ConEmuCD/GuiHooks.h"
#include "../common/execute.h"
//#ifdef __GNUC__
#include "ShObjIdl_Part.h"
//#endif

#define DEBUGSTRSYS(s) //DEBUGSTR(s)
#define DEBUGSTRSIZE(s) //DEBUGSTR(s)
#define DEBUGSTRCONS(s) //DEBUGSTR(s)
#define DEBUGSTRTABS(s) //DEBUGSTR(s)
#define DEBUGSTRLANG(s) //DEBUGSTR(s)// ; Sleep(2000)
#define DEBUGSTRMOUSE(s) //DEBUGSTR(s)
#define DEBUGSTRRCLICK(s) //DEBUGSTR(s)
#define DEBUGSTRKEY(s) //DEBUGSTR(s)
#define DEBUGSTRIME(s) //DEBUGSTR(s)
#define DEBUGSTRCHAR(s) //DEBUGSTR(s)
#define DEBUGSTRSETCURSOR(s) //OutputDebugString(s)
#define DEBUGSTRCONEVENT(s) //DEBUGSTR(s)
#define DEBUGSTRMACRO(s) //DEBUGSTR(s)
#define DEBUGSTRPANEL(s) //DEBUGSTR(s)
#define DEBUGSTRPANEL2(s) //DEBUGSTR(s)
#define DEBUGSTRFOCUS(s) LogFocusInfo(s)
#define DEBUGSTRFOCUS2(s) //LogFocusInfo(s)
#define DEBUGSTRFOREGROUND(s) //DEBUGSTR(s)
#define DEBUGSTRLLKB(s) //DEBUGSTR(s)
#define DEBUGSTRTIMER(s) //DEBUGSTR(s)
#define DEBUGSTRMSG(s) //DEBUGSTR(s)
#define DEBUGSTRANIMATE(s) //DEBUGSTR(s)
#ifdef _DEBUG
//#define DEBUGSHOWFOCUS(s) DEBUGSTR(s)
#endif

//#define PROCESS_WAIT_START_TIME 1000

#define PTDIFFTEST(C,D) PtDiffTest(C, ptCur.x, ptCur.y, D)
//(((abs(C.x-(short)LOWORD(lParam)))<D) && ((abs(C.y-(short)HIWORD(lParam)))<D))



#if defined(__GNUC__)
#define EXT_GNUC_LOG
#endif


#define TIMER_MAIN_ID 0
#define TIMER_CONREDRAW_ID 1
#define TIMER_CAPTION_APPEAR_ID 3
#define TIMER_CAPTION_DISAPPEAR_ID 4
#define TIMER_RCLICKPAINT 5
#define TIMER_RCLICKPAINT_ELAPSE 20
#define TIMER_ADMSHIELD_ID 7
#define TIMER_ADMSHIELD_ELAPSE 1000

#define HOTKEY_CTRLWINALTSPACE_ID 0x0201 // this is wParam for WM_HOTKEY
#define HOTKEY_GLOBAL_START      0x1001 // this is wParam for WM_HOTKEY

#define RCLICKAPPSTIMEOUT 600
#define RCLICKAPPS_START 200 // ������ ��������� ������ ������ �������
#define RCLICKAPPSTIMEOUT_MAX 10000
#define RCLICKAPPSDELTA 3

#define TOUCH_DBLCLICK_DELTA 1000 // 1sec

#define FIXPOSAFTERSTYLE_DELTA 2500

#define CONEMU_ANIMATE_DURATION 200
#define CONEMU_ANIMATE_DURATION_MAX 5000

const wchar_t* gsHomePage = L"http://conemu-maximus5.googlecode.com";
const wchar_t* gsReportBug = L"http://code.google.com/p/conemu-maximus5/issues/entry";

static struct RegisteredHotKeys
{
	int DescrID;
	int RegisteredID; // wParam ��� WM_HOTKEY
	UINT VK, MOD;     // ����� �� ��������� �����������
}
gRegisteredHotKeys[] = {
	{vkMinimizeRestore},
	{vkMinimizeRestor2},
	{vkGlobalRestore},
	{vkForceFullScreen},
};

namespace ConEmuMsgLogger
{
	MSG g_msg[BUFFER_SIZE];
	LONG g_msgidx = -1;

	Event g_pos[BUFFER_POS_SIZE];
	LONG g_posidx = -1;

	void LogPos(const MSG& msg, Source from)
	{
		#ifdef _DEBUG
		if (from != msgCanvas)
			return;
		#endif

		// Get next message index
		LONG i = _InterlockedIncrement(&g_posidx);
		// Write a message at this index
		Event& e = g_pos[i & (BUFFER_POS_SIZE - 1)]; // Wrap to buffer size

		e.hwnd = msg.hwnd;
		e.time = GetTickCount(); // msg.time == 0 ������ �����

		switch (msg.message)
		{
		case WM_WINDOWPOSCHANGING:
			e.msg = Event::WindowPosChanging;
			break;
		case WM_WINDOWPOSCHANGED:
			e.msg = Event::WindowPosChanged;
			break;
		case WM_MOVE:
			e.msg = Event::Move;
			break;
		case WM_SIZE:
			e.msg = Event::Size;
			break;
		case WM_MOVING:
			e.msg = Event::Moving;
			break;
		case WM_SIZING:
			e.msg = Event::Sizing;
			break;
		}

		switch (msg.message)
		{
		case WM_WINDOWPOSCHANGING:
		case WM_WINDOWPOSCHANGED:
			if (((WINDOWPOS*)msg.lParam)->flags & SWP_NOMOVE)
			{
				e.x = e.y = 0;
			}
			else
			{
				e.x = ((WINDOWPOS*)msg.lParam)->x;
				e.y = ((WINDOWPOS*)msg.lParam)->y;
			}
			if (((WINDOWPOS*)msg.lParam)->flags & SWP_NOSIZE)
			{
				e.w = e.h = 0;
			}
			else
			{
				e.w = ((WINDOWPOS*)msg.lParam)->cx;
				e.h = ((WINDOWPOS*)msg.lParam)->cy;
			}
			break;
		case WM_MOVE:
			e.x = (int)(short)LOWORD(msg.lParam);
			e.y = (int)(short)HIWORD(msg.lParam);
			e.w = e.h = 0;
			break;
		case WM_SIZE:
			e.x = e.y = 0;
			e.w = LOWORD(msg.lParam);
			e.h = HIWORD(msg.lParam);
			break;
		case WM_MOVING:
		case WM_SIZING:
			e.msg = Event::Sizing;
			e.x = ((LPRECT)msg.lParam)->left;
			e.y = ((LPRECT)msg.lParam)->top;
			e.w = ((LPRECT)msg.lParam)->right - e.x;
			e.h = ((LPRECT)msg.lParam)->bottom - e.y;
			break;
		}
	} // void LogPos(const MSG& msg)
}

CConEmuMain::CConEmuMain()
{
	gpConEmu = this; // �����!

	CVConGroup::Initialize();

	getForegroundWindow();

	_ASSERTE(gOSVer.dwMajorVersion>=5);
	//memset(&gOSVer,0,sizeof(gOSVer));
	//gOSVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	//GetVersionEx(&gOSVer);

	mp_Log = NULL;

	mb_CommCtrlsInitialized = false;
	mh_AboutDlg = NULL;

	//HeapInitialize(); - ���
	//#define D(N) (1##N-100)

	_wsprintf(ms_ConEmuBuild, SKIPLEN(countof(ms_ConEmuBuild)) L"%02u%02u%02u%s", (MVV_1%100),MVV_2,MVV_3,_T(MVV_4a));
	wcscpy_c(ms_ConEmuDefTitle, L"ConEmu ");
	wcscat_c(ms_ConEmuDefTitle, ms_ConEmuBuild);
	wcscat_c(ms_ConEmuDefTitle, WIN3264TEST(L" [32]",L" [64]"));

	mp_TabBar = NULL; /*m_Macro = NULL;*/ mp_Tip = NULL;
	mp_Status = new CStatus;
	ms_ConEmuAliveEvent[0] = 0;	mb_AliveInitialized = FALSE; mh_ConEmuAliveEvent = NULL; mb_ConEmuAliveOwned = FALSE;
	mn_MainThreadId = GetCurrentThreadId();
	//wcscpy_c(szConEmuVersion, L"?.?.?.?");
	
	mn_StartupFinished = ss_Starting;
	WindowMode = wmNormal;
	changeFromWindowMode = wmNotChanging;
	isRestoreFromMinimized = false;
	WindowStartMinimized = false;
	ForceMinimizeToTray = false;
	mb_InCreateWindow = true;
	mh_MinFromMonitor = NULL;
	
	wndX = gpSet->_wndX; wndY = gpSet->_wndY;
	wndWidth = gpSet->_wndWidth; wndHeight = gpSet->_wndHeight;
	
	mn_QuakePercent = 0; // 0 - ��������
	DisableAutoUpdate = false;
	DisableKeybHooks = false;
	mn_SysMenuOpenTick = mn_SysMenuCloseTick = 0;
	m_InsideIntegration = ii_None; mb_InsideIntegrationShift = false; mn_InsideParentPID = 0;
	mb_InsideSynchronizeCurDir = false;
	ms_InsideSynchronizeCurDir = NULL;
	mb_InsidePaneWasForced = false;
	mh_InsideParentRoot = mh_InsideParentWND = mh_InsideParentRel = NULL;
	mh_InsideParentPath = mh_InsideParentCD = NULL; ms_InsideParentPath[0] = 0;
	mh_InsideSysMenu = NULL;
	mb_PassSysCommand = false;
	mb_ExternalHidden = FALSE;
	memset(&mrc_StoredNormalRect, 0, sizeof(mrc_StoredNormalRect));
	isWndNotFSMaximized = false;
	isQuakeMinimized = false;
	mb_StartDetached = FALSE;
	//mb_SkipSyncSize = false;
	isPiewUpdate = false; //true; --Maximus5
	gbPostUpdateWindowSize = false;
	hPictureView = NULL;  mrc_WndPosOnPicView = MakeRect(0,0);
	bPicViewSlideShow = false;
	dwLastSlideShowTick = 0;
	mh_ShellWindow = NULL; mn_ShellWindowPID = 0;
	mb_FocusOnDesktop = TRUE;
	cursor.x=0; cursor.y=0; Rcursor=cursor;
	//m_LastConSize = MakeCoord(0,0);
	mp_DragDrop = NULL;
	//mb_InConsoleResize = FALSE;
	//ProgressBars = NULL;
	//cBlinkShift=0;
	mh_DebugPopup = mh_EditPopup = mh_ActiveVConPopup = mh_TerminateVConPopup = mh_VConListPopup = mh_HelpPopup = NULL;
	Title[0] = 0; //TitleCmp[0] = 0; /*MultiTitle[0] = 0;*/ mn_Progress = -1;
	TitleTemplate[0] = 0;
	mb_InTimer = FALSE;
	//mb_InClose = FALSE;
	//memset(m_ProcList, 0, 1000*sizeof(DWORD));
	m_ProcCount=0;
	mb_ProcessCreated = FALSE; /*mn_StartTick = 0;*/ mb_WorkspaceErasedOnClose = FALSE;
	mb_IgnoreSizeChange = false;
	mb_InCaptionChange = false;
	m_FixPosAfterStyle = 0;
	ZeroStruct(mrc_FixPosAfterStyle);
	mb_InImeComposition = false; mb_ImeMethodChanged = false;
	ZeroStruct(mr_Ideal);
	mn_InResize = 0;
	mb_InScMinimize = false;
	mb_MouseCaptured = FALSE;
	mb_HotKeyRegistered = FALSE;
	mh_LLKeyHookDll = NULL;
	mph_HookedGhostWnd = NULL;
	mh_LLKeyHook = NULL;
	mh_RightClickingBmp = NULL; mh_RightClickingDC = NULL; mb_RightClickingPaint = mb_RightClickingLSent = FALSE;
	m_RightClickingSize.x = m_RightClickingSize.y = m_RightClickingFrames = 0; m_RightClickingCurrent = -1;
	mh_RightClickingWnd = NULL; mb_RightClickingRegistered = FALSE;
	mb_WaitCursor = FALSE;
	//mb_InTrackSysMenu = FALSE;
	mn_TrackMenuPlace = tmp_None;
	mb_LastRgnWasNull = TRUE;
	mb_LockWindowRgn = FALSE;
	m_ForceShowFrame = fsf_Hide;
	mh_CursorWait = LoadCursor(NULL, IDC_WAIT);
	mh_CursorArrow = LoadCursor(NULL, IDC_ARROW);
	mh_CursorMove = LoadCursor(NULL, IDC_SIZEALL);
	mh_DragCursor = NULL;
	mh_CursorAppStarting = LoadCursor(NULL, IDC_APPSTARTING);
	// g_hInstance ��� �� ���������������
	mh_SplitV = LoadCursor(GetModuleHandle(0), MAKEINTRESOURCE(IDC_SPLITV));
	mh_SplitH = LoadCursor(GetModuleHandle(0), MAKEINTRESOURCE(IDC_SPLITH));
	//ms_LogCreateProcess[0] = 0; mb_CreateProcessLogged = false;
	memset(&mouse, 0, sizeof(mouse));
	mouse.lastMMW=-1;
	mouse.lastMML=-1;
	memset(m_TranslatedChars, 0, sizeof(m_TranslatedChars));
	//GetKeyboardState(m_KeybStates);
	//memset(m_KeybStates, 0, sizeof(m_KeybStates));
	m_ActiveKeybLayout = GetActiveKeyboardLayout();
	memset(m_LayoutNames, 0, sizeof(m_LayoutNames));
	//wchar_t szTranslatedChars[16];
	//HKL hkl = (HKL)GetActiveKeyboardLayout();
	//int nTranslatedChars = ToUnicodeEx(0, 0, m_KeybStates, szTranslatedChars, 15, 0, hkl);
	mn_LastPressedVK = 0;
	memset(&m_GuiInfo, 0, sizeof(m_GuiInfo));
	m_GuiInfo.cbSize = sizeof(m_GuiInfo);
	//mh_RecreatePasswFont = NULL;
	mb_SkipOnFocus = false;
	mb_LastConEmuFocusState = false;
	mb_CloseGuiConfirmed = false;
	mb_UpdateJumpListOnStartup = false;
	ZeroStruct(m_QuakePrevSize);

	mps_IconPath = NULL;

	// ��������� "������" �� �������?
	HKEY hk;
	ms_ComSpecInitial[0] = 0;
	if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment", 0, KEY_READ, &hk))
	{
		DWORD nSize = sizeof(ms_ComSpecInitial)-sizeof(wchar_t);
		if (RegQueryValueEx(hk, L"ComSpec", NULL, NULL, (LPBYTE)ms_ComSpecInitial, &nSize))
			ms_ComSpecInitial[0] = 0;
		RegCloseKey(hk);
	}
	if (!*ms_ComSpecInitial)
	{
		GetComspecFromEnvVar(ms_ComSpecInitial, countof(ms_ComSpecInitial));
	}
	_ASSERTE(*ms_ComSpecInitial);

	mpsz_ConEmuArgs = NULL;
	ms_ConEmuExe[0] = ms_ConEmuExeDir[0] = ms_ConEmuBaseDir[0] = 0;
	//ms_ConEmuCExe[0] = 
	ms_ConEmuC32Full[0] = ms_ConEmuC64Full[0] = 0;
	ms_ConEmuXml[0] = ms_ConEmuIni[0] = ms_ConEmuChm[0] = 0;
	//ms_ConEmuCExeName[0] = 0;
	wchar_t *pszSlash = NULL;

	if (!GetModuleFileName(NULL, ms_ConEmuExe, MAX_PATH) || !(pszSlash = wcsrchr(ms_ConEmuExe, L'\\')))
	{
		DisplayLastError(L"GetModuleFileName failed");
		TerminateProcess(GetCurrentProcess(), 100);
		return;
	}

	#ifdef __GNUC__
	HMODULE hGdi32 = GetModuleHandle(L"gdi32.dll");
	GdiAlphaBlend = (AlphaBlend_t)(hGdi32 ? GetProcAddress(hGdi32, "GdiAlphaBlend") : NULL);
	#endif
	
	HMODULE hUser32 = GetModuleHandle(L"user32.dll");
	// GetLayeredWindowAttributes �������� ������ � XP
	_GetLayeredWindowAttributes = (GetLayeredWindowAttributes_t)(hUser32 ? GetProcAddress(hUser32, "GetLayeredWindowAttributes") : NULL);
	#ifdef __GNUC__
	// SetLayeredWindowAttributes ���� � Win2k, ������ ��� ���� �� ����� GCC
	SetLayeredWindowAttributes = (SetLayeredWindowAttributes_t)(hUser32 ? GetProcAddress(hUser32, "SetLayeredWindowAttributes") : NULL);
	#endif

	// IME support (WinXP or later)
	mh_Imm32 = NULL;
	_ImmSetCompositionFont = NULL;
	_ImmSetCompositionWindow = NULL;
	_ImmGetContext = NULL;
	if (gnOsVer >= 0x501)
	{
		mh_Imm32 = LoadLibrary(L"Imm32.dll");
		if (mh_Imm32)
		{
			_ImmSetCompositionFont = (ImmSetCompositionFontW_t)GetProcAddress(mh_Imm32, "ImmSetCompositionFontW");
			_ImmSetCompositionWindow = (ImmSetCompositionWindow_t)GetProcAddress(mh_Imm32, "ImmSetCompositionWindow");
			_ImmGetContext = (ImmGetContext_t)GetProcAddress(mh_Imm32, "ImmGetContext");
			if (!_ImmSetCompositionFont || !_ImmSetCompositionWindow || !_ImmGetContext)
			{
				_ASSERTE(_ImmSetCompositionFont && _ImmSetCompositionWindow && _ImmGetContext);
				_ImmSetCompositionFont = NULL;
				_ImmSetCompositionWindow = NULL;
				_ImmGetContext = NULL;
				FreeLibrary(mh_Imm32);
				mh_Imm32 = NULL;
			}
		}
	}

	//LoadVersionInfo(ms_ConEmuExe);
	// ����� ���������
	wcscpy_c(ms_ConEmuExeDir, ms_ConEmuExe);
	pszSlash = wcsrchr(ms_ConEmuExeDir, L'\\');
	*pszSlash = 0;

	bool lbBaseFound = false;

	// Mingw/msys installation?
	mb_MingwMode = FALSE; // �������� ����
	wchar_t szBashFile[MAX_PATH+64];
	wcscpy_c(szBashFile, ms_ConEmuExeDir);
	pszSlash = wcsrchr(szBashFile, L'\\');
	if (pszSlash) *pszSlash = 0;
	pszSlash = szBashFile + _tcslen(szBashFile);
	wcscat_c(szBashFile, L"\\msys\\1.0\\bin\\sh.exe");
	mb_MSysStartup = FileExists(szBashFile);
	if (!mb_MSysStartup)
	{
		// Git-bash mode
		*pszSlash = 0;
		wcscat_c(szBashFile, L"\\bin\\sh.exe");
		mb_MSysStartup = FileExists(szBashFile);
	}

	// ������� ��������� ��������
	wcscpy_c(ms_ConEmuBaseDir, ms_ConEmuExeDir);
	wcscat_c(ms_ConEmuBaseDir, L"\\ConEmu");
	lbBaseFound = CheckBaseDir();


	if (!lbBaseFound)
	{
		// ���� �� ����� - �� � ��� �� �����, ��� � GUI
		wcscpy_c(ms_ConEmuBaseDir, ms_ConEmuExeDir);
		lbBaseFound = CheckBaseDir();
	}

	if (!lbBaseFound)
	{
		// ���� ���-����� �� ����� - ���������, ����� ��� mingw?
		pszSlash = wcsrchr(ms_ConEmuExeDir, L'\\');
		if (pszSlash && (lstrcmpi(pszSlash, L"\\bin") == 0))
		{
			// -> $MINGW_ROOT/libexec/ConEmu
			size_t cch = pszSlash - ms_ConEmuExeDir;
			wmemmove(ms_ConEmuBaseDir, ms_ConEmuExeDir, cch);
			ms_ConEmuBaseDir[cch] = 0;
			wcscat_c(ms_ConEmuBaseDir, L"\\libexec\\ConEmu");
			// Last chance
			lbBaseFound = CheckBaseDir();
			if (lbBaseFound)
			{
				mb_MingwMode = lbBaseFound;
			}
			else
			{
				// Fail, revert to ExeDir
				wcscpy_c(ms_ConEmuBaseDir, ms_ConEmuExeDir);
			}
		}
	}

	if (mb_MingwMode && mb_MSysStartup)
	{
		// This is example. Will be replaced with full path.
		gpSetCls->SetDefaultCmd(L"sh.exe --login -i");
	}


	// �������� � ��������� ���������� � ������ � ConEmu.exe
	SetEnvironmentVariable(ENV_CONEMUDIR_VAR_W, ms_ConEmuExeDir);
	SetEnvironmentVariable(ENV_CONEMUBASEDIR_VAR_W, ms_ConEmuBaseDir);
	SetEnvironmentVariable(ENV_CONEMUANSI_BUILD_W, ms_ConEmuBuild);
	SetEnvironmentVariable(ENV_CONEMUANSI_CONFIG_W, L"");
	// ���������� "ConEmuArgs" ����������� � ConEmuApp.cpp:PrepareCommandLine

	// ���� ���� ������������ ��������. ������� ������� � BaseDir
	ConEmuXml();
	ConEmuIni();

	// Help-����. ������� ��������� � BaseDir
	wcscpy_c(ms_ConEmuChm, ms_ConEmuBaseDir); lstrcat(ms_ConEmuChm, L"\\ConEmu.chm");

	if (!FileExists(ms_ConEmuChm))
	{
		if (lstrcmp(ms_ConEmuBaseDir, ms_ConEmuExeDir))
		{
			wcscpy_c(ms_ConEmuChm, ms_ConEmuExeDir); lstrcat(ms_ConEmuChm, L"\\ConEmu.chm");

			if (!FileExists(ms_ConEmuChm))
				ms_ConEmuChm[0] = 0;
		}
		else
		{
			ms_ConEmuChm[0] = 0;
		}
	}

	wcscpy_c(ms_ConEmuC32Full, ms_ConEmuBaseDir);
	wcscat_c(ms_ConEmuC32Full, L"\\ConEmuC.exe");
	if (IsWindows64())
	{
		wcscpy_c(ms_ConEmuC64Full, ms_ConEmuBaseDir);
		wcscat_c(ms_ConEmuC64Full, L"\\ConEmuC64.exe");
		// ��������� �������
		if (!FileExists(ms_ConEmuC64Full))
			wcscpy_c(ms_ConEmuC64Full, ms_ConEmuC32Full);
		else if (!FileExists(ms_ConEmuC32Full))
			wcscpy_c(ms_ConEmuC32Full, ms_ConEmuC64Full);
	}
	else
	{
		wcscpy_c(ms_ConEmuC64Full, ms_ConEmuC32Full);
	}

	//// ���� ConEmu.exe ������� � �������� ������� -  ������� ���� �� ������
	//if (ms_ConEmuExe[0] == L'\\' /*|| wcschr(ms_ConEmuExe, L' ') == NULL*/)
	//{
	//	wcscpy_c(ms_ConEmuCExe, ms_ConEmuCExeFull);
	//}
	//else
	//{
	//	wchar_t* pszShort = GetShortFileNameEx(ms_ConEmuCExeFull);

	//	if (pszShort)
	//	{
	//		wcscpy_c(ms_ConEmuCExe, pszShort);
	//		free(pszShort);
	//	}
	//	else
	//	{
	//		wcscpy_c(ms_ConEmuCExe, ms_ConEmuCExeFull);
	//	}

	//	//pszSlash = ms_ConEmuCExe + _tcslen(ms_ConEmuCExe);
	//	//if (*(pszSlash-1) != L'\\')
	//	//{
	//	//	*(pszSlash++) = L'\\'; *pszSlash = 0;
	//	//}
	//}

	//WARNING("������ ConEmuC64.exe - ��������� �������� � ������������");
	//if (IsWindows64())
	//{
	//	wcscpy(pszSlash, L"ConEmuC64.exe");
	//	if (!FileExists(ms_ConEmuCExe)) // ����������� "ConEmuC.exe"
	//	{
	//		wcscpy(pszSlash, L"ConEmuC.exe");
	//		if (!FileExists(ms_ConEmuCExe)) // ������� 64 ����, ��� 32 �� �����
	//			wcscpy(pszSlash, L"ConEmuC64.exe");
	//	}
	//}
	//else
	//{
	//	wcscpy(pszSlash, L"ConEmuC.exe");
	//}
	//wcscpy(ms_ConEmuCExeName, pszSlash);
	// ��������� ������� ����� (�� ������ �������)
	RefreshConEmuCurDir();

	
	// DosBox (������������ ������ ������� Dos-���������� � 64-������ OS)
	mb_DosBoxExists = CheckDosBoxExists();
	
	// ������������ ������ ��� ����?
	mb_PortableRegExist = FALSE;
	ms_PortableReg[0] = ms_PortableRegHive[0] = ms_PortableRegHiveOrg[0] = ms_PortableMountKey[0] = 0;
	mb_PortableKeyMounted = FALSE;
	ms_PortableTempDir[0] = 0;
	mh_PortableMountRoot = mh_PortableRoot = NULL;
	CheckPortableReg();
	

	if (gOSVer.dwMajorVersion >= 6)
	{
		mb_IsUacAdmin = IsUserAdmin(); // ����� �����, ����� �� ��� �������� ��� UAC �������?
	}
	else
	{
		mb_IsUacAdmin = FALSE;
	}
	
	//mh_Psapi = NULL;
	//GetModuleFileNameEx = (FGetModuleFileNameEx)GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "GetModuleFileNameExW");
	//if (GetModuleFileNameEx == NULL)
	//{
	//	mh_Psapi = LoadLibrary(_T("psapi.dll"));
	//	if (mh_Psapi)
	//	    GetModuleFileNameEx = (FGetModuleFileNameEx)GetProcAddress(mh_Psapi, "GetModuleFileNameExW");
	//}

	#ifndef _WIN64
	mh_WinHook = NULL;
	#endif
	//mh_PopupHook = NULL;
	//mp_TaskBar2 = NULL;
	//mp_TaskBar3 = NULL;
	//mp_ VActive = NULL; mp_VCon1 = NULL; mp_VCon2 = NULL;
	//mb_CreatingActive = false;
	//memset(mp_VCon, 0, sizeof(mp_VCon));
	mp_AttachDlg = NULL;
	mp_RecreateDlg = NULL;
	UINT nAppMsg = WM_APP+10;
	mn_MsgPostCreate = ++nAppMsg;
	mn_MsgPostCopy = ++nAppMsg;
	mn_MsgMyDestroy = ++nAppMsg;
	mn_MsgUpdateSizes = ++nAppMsg;
	mn_MsgUpdateCursorInfo = ++nAppMsg;
	mn_MsgSetWindowMode = ++nAppMsg;
	mn_MsgUpdateTitle = ++nAppMsg;
	//mn_MsgAttach = RegisterWindowMessage(CONEMUMSG_ATTACH);
	mn_MsgSrvStarted = ++nAppMsg; //RegisterWindowMessage(CONEMUMSG_SRVSTARTED);
	mn_MsgVConTerminated = ++nAppMsg;
	mn_MsgUpdateScrollInfo = ++nAppMsg;
	mn_MsgUpdateTabs = RegisterWindowMessage(CONEMUMSG_UPDATETABS);
	mn_MsgOldCmdVer = ++nAppMsg; mb_InShowOldCmdVersion = FALSE;
	mn_MsgTabCommand = ++nAppMsg;
	mn_MsgTabSwitchFromHook = RegisterWindowMessage(CONEMUMSG_SWITCHCON); //mb_InWinTabSwitch = FALSE;
	mn_MsgWinKeyFromHook = RegisterWindowMessage(CONEMUMSG_HOOKEDKEY);
	//mn_MsgConsoleHookedKey = RegisterWindowMessage(CONEMUMSG_CONSOLEHOOKEDKEY);
	mn_ShellExecuteEx = ++nAppMsg;
		InitializeCriticalSection(&mcs_ShellExecuteEx);
		mb_InShellExecuteQueue = false;
	mn_PostConsoleResize = ++nAppMsg;
	mn_ConsoleLangChanged = ++nAppMsg;
	mn_MsgPostOnBufferHeight = ++nAppMsg;
	//mn_MsgSetForeground = RegisterWindowMessage(CONEMUMSG_SETFOREGROUND);
	mn_MsgFlashWindow = RegisterWindowMessage(CONEMUMSG_FLASHWINDOW);
	mn_MsgPostAltF9 = ++nAppMsg;
	//mn_MsgPostSetBackground = ++nAppMsg;
	mn_MsgInitInactiveDC = ++nAppMsg;
	//mn_MsgActivateCon = RegisterWindowMessage(CONEMUMSG_ACTIVATECON);
	mn_MsgUpdateProcDisplay = ++nAppMsg;
	mn_MsgAutoSizeFont = ++nAppMsg;
	mn_MsgDisplayRConError = ++nAppMsg;
	mn_MsgMacroFontSetName = ++nAppMsg;
	mn_MsgCreateViewWindow = ++nAppMsg;
	mn_MsgPostTaskbarActivate = ++nAppMsg; mb_PostTaskbarActivate = FALSE;
	mn_MsgInitVConGhost = ++nAppMsg;
	mn_MsgCreateCon = ++nAppMsg;
	mn_MsgRequestUpdate = ++nAppMsg;
	mn_MsgPanelViewMapCoord = RegisterWindowMessage(CONEMUMSG_PNLVIEWMAPCOORD);
	mn_MsgTaskBarCreated = RegisterWindowMessage(L"TaskbarCreated");
	mn_MsgTaskBarBtnCreated = RegisterWindowMessage(L"TaskbarButtonCreated");
	mn_MsgSheelHook = RegisterWindowMessage(L"SHELLHOOK");
	//// � Win7x64 WM_INPUTLANGCHANGEREQUEST �� �������� (�� ������� ���� ��� ������������ ������)
	//wmInputLangChange = WM_INPUTLANGCHANGE;

	InitFrameHolder();
}

void LogFocusInfo(LPCWSTR asInfo)
{
#ifdef _DEBUG
	if (!asInfo)
		return;

	gpConEmu->LogString(asInfo, TRUE);

	wchar_t szFull[255];
	lstrcpyn(szFull, asInfo, countof(szFull)-3);
	wcscat_c(szFull, L"\n");
	DEBUGSTR(szFull);
#endif
}

void CConEmuMain::RefreshConEmuCurDir()
{
	// ��������� ������� ����� (�� ������ �������)
	DWORD nDirLen = GetCurrentDirectory(MAX_PATH, ms_ConEmuCurDir);

	if (!nDirLen || nDirLen>MAX_PATH)
	{
		ms_ConEmuCurDir[0] = 0;
	}
	else if (ms_ConEmuCurDir[nDirLen-1] == L'\\')
	{
		ms_ConEmuCurDir[nDirLen-1] = 0; // ����� ����� ��� �����, ��� ����������� � ms_ConEmuExeDir
	}
}

bool CConEmuMain::CheckRequiredFiles()
{
	wchar_t szPath[MAX_PATH+32];
	struct ReqFile {
		int  Bits;
		BOOL Req;
		wchar_t File[16];
	} Files[] = {
		{32, TRUE, L"ConEmuC.exe"},
		{64, TRUE, L"ConEmuC64.exe"},
		{32, TRUE, L"ConEmuCD.dll"},
		{64, TRUE, L"ConEmuCD64.dll"},
		{32, gpSet->isUseInjects, L"ConEmuHk.dll"},
		{64, gpSet->isUseInjects, L"ConEmuHk64.dll"},
	};
	
	wchar_t szRequired[128], szRecommended[128]; szRequired[0] = szRecommended[0] = 0;
	bool isWin64 = IsWindows64();
	int  nExeBits = WIN3264TEST(32,64);

	wcscpy_c(szPath, ms_ConEmuBaseDir);
	wcscat_c(szPath, L"\\");
	wchar_t* pszSlash = szPath + _tcslen(szPath);
	DWORD nFileSize;

	for (size_t i = 0; i < countof(Files); i++)
	{
		if (Files[i].Bits == 64)
		{
			if (!isWin64)
				continue; // 64������ ����� � 32������ �� �� �����
		}

		_wcscpy_c(pszSlash, 32, Files[i].File);
		if (!FileExists(szPath, &nFileSize) || !nFileSize)
		{
			if (!Files[i].Req)
				continue;
			wchar_t* pszList = (Files[i].Bits == nExeBits) ? szRequired : szRecommended;
			if (*pszList)
				_wcscat_c(pszList, countof(szRequired), L", ");
			_wcscat_c(pszList, countof(szRequired), Files[i].File);
		}
	}

	if (*szRequired || *szRecommended)
	{
		size_t cchMax = _tcslen(szRequired) + _tcslen(szRecommended) + _tcslen(ms_ConEmuExe) + 255;
		wchar_t* pszMsg = (wchar_t*)calloc(cchMax, sizeof(*pszMsg));
		if (pszMsg)
		{
			_wcscpy_c(pszMsg, cchMax, *szRequired ? L"Critical error\n\n" : L"Warning\n\n");
			if (*szRequired)
			{
				_wcscat_c(pszMsg, cchMax, L"Required files not found!\n");
				_wcscat_c(pszMsg, cchMax, szRequired);
				_wcscat_c(pszMsg, cchMax, L"\n\n");
			}
			if (*szRecommended)
			{
				_wcscat_c(pszMsg, cchMax, L"Recommended files not found!\n");
				_wcscat_c(pszMsg, cchMax, szRecommended);
				_wcscat_c(pszMsg, cchMax, L"\n\n");
			}
			_wcscat_c(pszMsg, cchMax, L"ConEmu was started from:\n");
			_wcscat_c(pszMsg, cchMax, ms_ConEmuExe);
			_wcscat_c(pszMsg, cchMax, L"\n");
			if (*szRequired)
			{
				_wcscat_c(pszMsg, cchMax, L"\nConEmu will exits now");
			}
			MessageBox(NULL, pszMsg, GetDefaultTitle(), MB_SYSTEMMODAL|(*szRequired ? MB_ICONSTOP : MB_ICONWARNING));
			free(pszMsg);
		}
	}

	if (*szRequired)
		return false;
	return true; // ����� ����������
}

bool CConEmuMain::CheckBaseDir()
{
	bool lbBaseFound = false;
	wchar_t szBaseFile[MAX_PATH+12];

	lstrcpyn(szBaseFile, ms_ConEmuBaseDir, countof(szBaseFile));
	wchar_t *pszSlash = szBaseFile + _tcslen(szBaseFile);
	lstrcpy(pszSlash, L"\\ConEmuC.exe");

	if (FileExists(szBaseFile))
	{
		lbBaseFound = true;
	}

	#ifdef WIN64
	if (!lbBaseFound)
	{
		lstrcpy(pszSlash, L"\\ConEmuC64.exe");

		if (FileExists(szBaseFile))
		{
			lbBaseFound = true;
		}
	}
	#endif

	return lbBaseFound;
}

bool CConEmuMain::SetConfigFile(LPCWSTR asFilePath, bool abWriteReq /*= false*/)
{
	LPCWSTR pszExt = asFilePath ? PointToExt(asFilePath) : NULL;
	int nLen = lstrlen(asFilePath);

	if (!asFilePath || !*asFilePath || !pszExt || !*pszExt || (nLen > MAX_PATH))
	{
		DisplayLastError(L"Invalid file path specified in CConEmuMain::SetConfigFile", -1);
		return false;
	}

	wchar_t szPath[MAX_PATH+1] = L"";
	int nFileType = 0;

	if (lstrcmpi(pszExt, L".ini") == 0)
	{
		_ASSERTE(countof(ms_ConEmuIni)==countof(szPath));
		nFileType = 1;
	}
	else if (lstrcmpi(pszExt, L".xml") == 0)
	{
		_ASSERTE(countof(ms_ConEmuXml)==countof(szPath));
		nFileType = 0;
	}
	else
	{
		DisplayLastError(L"Unsupported file extension specified in CConEmuMain::SetConfigFile", -1);
		return false;
	}


	//wcscpy_c(szPath, asFilePath);
	wchar_t* pszFilePart;
	nLen = GetFullPathName(asFilePath, countof(szPath), szPath, &pszFilePart);
	if ((nLen <= 0) || (nLen >= (INT_PTR)countof(szPath)))
	{
		DisplayLastError(L"Failed to expand specified file path in CConEmuMain::SetConfigFile");
		return false;
	}
	
	wchar_t* pszDirEnd = wcsrchr(szPath, L'\\');

	if (!pszDirEnd || (nLen > MAX_PATH))
	{
		DisplayLastError(L"Invalid file path specified in CConEmuMain::SetConfigFile (backslash not found)", -1);
		return false;
	}


	// ����� ����� �� ����
	*pszDirEnd = 0;
	// ��������, ���� ����
	MyCreateDirectory(szPath);
	*pszDirEnd = L'\\';

	// ����� ������� ����, ���� ��� ����.
	// ���� ������� ������ �� ������ - �� ���������� CreateFile
	if (abWriteReq || !FileExists(szPath))
	{
		HANDLE hFile = CreateFile(szPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (!hFile || (hFile == INVALID_HANDLE_VALUE))
		{
			DWORD nErrCode = GetLastError();
			wchar_t szErrMsg[MAX_PATH*2];
			wcscpy_c(szErrMsg, L"Failed to create configuration file:\r\n");
			wcscat_c(szErrMsg, szPath);
			DisplayLastError(szErrMsg, nErrCode);
			return false;
		}
		CloseHandle(hFile);
	}

	if (nFileType == 1)
	{
		ms_ConEmuXml[0] = 0;
		wcscpy_c(ms_ConEmuIni, asFilePath);
	}
	else
	{
		ms_ConEmuIni[0] = 0;
		wcscpy_c(ms_ConEmuXml, asFilePath);
	}

	return true;
}

LPWSTR CConEmuMain::ConEmuXml()
{
	if (ms_ConEmuXml[0])
	{
		if (FileExists(ms_ConEmuXml))
			return ms_ConEmuXml;
	}

	TODO("������ �� ��� ���� ����������� ������������ ������������ ��� ����� - ��������� (�������������) � ���������������� (���������)");

	// ���� ���� ������������ ��������. ������� ������� � BaseDir
	wcscpy_c(ms_ConEmuXml, ms_ConEmuBaseDir); wcscat_c(ms_ConEmuXml, L"\\ConEmu.xml");

	if (!FileExists(ms_ConEmuXml))
	{
		if (lstrcmpi(ms_ConEmuBaseDir, ms_ConEmuExeDir))
		{
			wcscpy_c(ms_ConEmuXml, ms_ConEmuExeDir); wcscat_c(ms_ConEmuXml, L"\\ConEmu.xml");

			if (!FileExists(ms_ConEmuXml))
			{
				// ���� _���������_ �����, �� � BaseDir! ����� � ����� �� ��������
				wcscpy_c(ms_ConEmuXml, ms_ConEmuBaseDir); wcscat_c(ms_ConEmuXml, L"\\ConEmu.xml");
			}
		}
	}

	return ms_ConEmuXml;
}

LPWSTR CConEmuMain::ConEmuIni()
{
	if (ms_ConEmuIni[0])
	{
		if (FileExists(ms_ConEmuIni))
			return ms_ConEmuIni;
	}

	TODO("������ �� ��� ���� ����������� ������������ ������������ ��� ����� - ��������� (�������������) � ���������������� (���������)");

	// ���� ���� ������������ ��������. ������� ������� � BaseDir
	wcscpy_c(ms_ConEmuIni, ms_ConEmuBaseDir); wcscat_c(ms_ConEmuIni, L"\\ConEmu.ini");

	if (!FileExists(ms_ConEmuIni))
	{
		if (lstrcmpi(ms_ConEmuBaseDir, ms_ConEmuExeDir))
		{
			wcscpy_c(ms_ConEmuIni, ms_ConEmuExeDir); wcscat_c(ms_ConEmuIni, L"\\ConEmu.ini");

			if (!FileExists(ms_ConEmuIni))
			{
				// ���� _���������_ �����, �� � BaseDir! ����� � ����� �� ��������
				wcscpy_c(ms_ConEmuIni, ms_ConEmuBaseDir); wcscat_c(ms_ConEmuIni, L"\\ConEmu.ini");
			}
		}
	}

	return ms_ConEmuIni;
}

LPCWSTR CConEmuMain::ConEmuCExeFull(LPCWSTR asCmdLine/*=NULL*/)
{
	// ���� OS - 32������ ��� � ����� ConEmu ��� ������ ������ ���� �� "��������"
	if (!IsWindows64() || !lstrcmp(ms_ConEmuC32Full, ms_ConEmuC64Full))
	{
		// ����� �������
		return ms_ConEmuC32Full;
	}

	//LPCWSTR pszServer = ms_ConEmuC32Full;
	bool lbCmd = false, lbFound = false;
	int Bits = IsWindows64() ? 64 : 32;

	if (!asCmdLine || !*asCmdLine)
	{
		// ���� ������ ������� �� ������� - �������, ��� ����������� ComSpec
		lbCmd = true;
	}
	else
	{
		// ��������� �������� asCmdLine �� ��������� ������ �������� �������� ��� Inject
		// � ���������� �������� ������������ ���������� �� ���������
		wchar_t szTemp[MAX_PATH+1], szExpand[MAX_PATH+1];
		if (!FileExists(asCmdLine))
		{
			const wchar_t *psz = asCmdLine;
			if (NextArg(&psz, szTemp) == 0)
				asCmdLine = szTemp;
		}
		else
		{
			lbFound = true;
		}

		if (wcschr(asCmdLine, L'%'))
		{
			DWORD nLen = ExpandEnvironmentStrings(asCmdLine, szExpand, countof(szExpand));
			if (nLen && (nLen < countof(szExpand)))
				asCmdLine = szExpand;
		}

		// ���� ���� ������ ��������� - ����� �������� �� ����, ����� - ��������� "�� cmd"
		if ((lstrcmpi(asCmdLine, L"cmd") == 0) || (lstrcmpi(asCmdLine, L"cmd.exe") == 0))
		{
			lbCmd = true;
		}
		else
		{
			LPCWSTR pszExt = PointToExt(asCmdLine);
			if (pszExt && (lstrcmpi(pszExt, L".exe") != 0) && (lstrcmpi(pszExt, L".com") != 0))
			{
				// ���� ������� ����������, � ��� �� .exe � �� .com - �������, ��� ������ ����� ComProcessor
				lbCmd = true;
			}
			else
			{
				wchar_t szFind[MAX_PATH+1];
				wcscpy_c(szFind, asCmdLine);
				CharUpperBuff(szFind, lstrlen(szFind));
				// �� ��������, ����� �� ��������� ��� � ������ �� ������������ � "%WinDir%". �� ��� �� ��������.
				if (wcsstr(szFind, L"\\SYSNATIVE\\") || wcsstr(szFind, L"\\SYSWOW64\\"))
				{
					// ���� "SysNative" - ������� ��� 32-bit, �����, 64-������ ������ ������ "�� ������" ��� �����
					// � "SysWow64" ��� �������, ��� ������ 32-������
					Bits = 32;
				}
				else
				{
					MWow64Disable wow; wow.Disable();
					DWORD ImageSubsystem = 0, ImageBits = 0, FileAttrs = 0;
					lbFound = GetImageSubsystem(asCmdLine, ImageSubsystem, ImageBits, FileAttrs);
					// ���� �� ����� � ���� �� ��� ������
					if (!lbFound && !wcschr(asCmdLine, L'\\'))
					{
						// ���������� �����
						wchar_t *pszFilePart;
						DWORD nLen = SearchPath(NULL, asCmdLine, pszExt ? NULL : L".exe", countof(szFind), szFind, &pszFilePart);
						if (!nLen)
						{
							wchar_t szRoot[MAX_PATH+1];
							wcscpy_c(szRoot, ms_ConEmuExeDir);
							wchar_t* pszSlash = wcsrchr(szRoot, L'\\');
							if (pszSlash)
								*pszSlash = 0;
							nLen = SearchPath(szRoot, asCmdLine, pszExt ? NULL : L".exe", countof(szFind), szFind, &pszFilePart);
						}
						if (nLen && (nLen < countof(szFind)))
						{
							lbFound = GetImageSubsystem(szFind, ImageSubsystem, ImageBits, FileAttrs);
						}
					}

					if (lbFound)
						Bits = ImageBits;
				}
			}
		}
	}

	if (lbCmd)
	{
		if (gpSet->ComSpec.csBits == csb_SameApp)
			Bits = WIN3264TEST(32,64);
		else if (gpSet->ComSpec.csBits == csb_x32)
			Bits = 32;
	}

	return (Bits == 64) ? ms_ConEmuC64Full : ms_ConEmuC32Full;
}

void CConEmuMain::InitCommCtrls()
{
	if (mb_CommCtrlsInitialized)
		return;

	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC   = ICC_COOL_CLASSES|ICC_BAR_CLASSES|ICC_TAB_CLASSES|ICC_PROGRESS_CLASS;
	InitCommonControlsEx(&icex);

	mb_CommCtrlsInitialized = true;
}

BOOL CConEmuMain::Init()
{
	_ASSERTE(mp_TabBar == NULL);

	// ������ �� ���������, � �� �������� �������� � ��� �� Affinity �����������...
	// �� ������� - �� ������. �� ����� �� �� ����������������� ��-�� ������ � ������ ����� ���� ������ �������� ������������������, ���� ����� �� �������
	if (gpSet->nAffinity)
		SetProcessAffinityMask(GetCurrentProcess(), gpSet->nAffinity);

	InitCommCtrls();

	//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
	//SetThreadAffinityMask(GetCurrentThread(), 1);
	/*DWORD dwErr = 0;
	HMODULE hInf = LoadLibrary(L"infis.dll");
	if (!hInf)
	dwErr = GetLastError();*/
	//!!!ICON
	LoadIcons();

	mp_Tip = new CToolTip();
	mp_TabBar = new TabBarClass();
	//m_Child = new CConEmuChild();
	//m_Back = new CConEmuBack();
	//m_Macro = new CConEmuMacro();
	//#pragma message("Win2k: EVENT_CONSOLE_START_APPLICATION, EVENT_CONSOLE_END_APPLICATION")
	//��� ���������� ������ START � END. ��� ��������� ������� �������� �� ConEmuC ����� ��������� ����
	//#if defined(__GNUC__)
	//HMODULE hUser32 = GetModuleHandle(L"user32.dll");
	//FSetWinEventHook SetWinEventHook = (FSetWinEventHook)GetProcAddress(hUser32, "SetWinEventHook");
	//#endif
#ifndef _WIN64
	// ���������� ������ ������/������� 16������ ����������
	// NTVDM ���� � 64������ ������� Windows
	if (!IsWindows64())
	{
		mh_WinHook = SetWinEventHook(EVENT_CONSOLE_START_APPLICATION/*EVENT_CONSOLE_CARET*/,EVENT_CONSOLE_END_APPLICATION,
	                             NULL, (WINEVENTPROC)CConEmuMain::WinEventProc, 0,0, WINEVENT_OUTOFCONTEXT);
	}
#endif
	//mh_PopupHook = SetWinEventHook(EVENT_SYSTEM_MENUPOPUPSTART,EVENT_SYSTEM_MENUPOPUPSTART,
	//    NULL, (WINEVENTPROC)CConEmuMain::WinEventProc, 0,0, WINEVENT_OUTOFCONTEXT);
	/*mh_Psapi = LoadLibrary(_T("psapi.dll"));
	if (mh_Psapi) {
	    GetModuleFileNameEx = (FGetModuleFileNameEx)GetProcAddress(mh_Psapi, "GetModuleFileNameExW");
	    if (GetModuleFileNameEx)
	        return TRUE;
	}*/
	/*DWORD dwErr = GetLastError();
	TCHAR szErr[255];
	_wsprintf(szErr, countof(szErr), _T("Can't initialize psapi!\r\nLastError = 0x%08x"), dwErr);
	MBoxA(szErr);
	return FALSE;*/
	return TRUE;
}

void CConEmuMain::OnUseGlass(bool abEnableGlass)
{
	//CheckMenuItem(GetSysMenu(false), ID_USEGLASS, MF_BYCOMMAND | (abEnableGlass ? MF_CHECKED : MF_UNCHECKED));
}
void CConEmuMain::OnUseTheming(bool abEnableTheming)
{
	//CheckMenuItem(GetSysMenu(false), ID_USETHEME, MF_BYCOMMAND | (abEnableTheming ? MF_CHECKED : MF_UNCHECKED));
}
void CConEmuMain::OnUseDwm(bool abEnableDwm)
{
	//CheckMenuItem(GetSysMenu(false), ID_ISDWM, MF_BYCOMMAND | (abEnableDwm ? MF_CHECKED : MF_UNCHECKED));
}

// ���������� ��� ������ ���������, ��� ���������� mrc_Ideal - ������� ���� �� ���������
RECT CConEmuMain::GetDefaultRect()
{
	RECT rcWnd;

	if (m_InsideIntegration)
	{
		WindowMode = wmNormal;
		RECT rcParent = {}, rcRelative = {};
		GetClientRect(mh_InsideParentWND, &rcParent);
		if (m_InsideIntegration == ii_Simple)
		{
			mrc_InsideParent = rcParent;
			ZeroStruct(mrc_InsideParentRel);
			rcWnd = rcParent;
		}
		else
		{
			RECT rcChild = {};
			GetWindowRect(mh_InsideParentRel, &rcChild);
			MapWindowPoints(NULL, mh_InsideParentWND, (LPPOINT)&rcChild, 2);
			mrc_InsideParent = rcParent;
			mrc_InsideParentRel = rcChild;
			IntersectRect(&rcRelative, &rcParent, &rcChild);

			// WinXP & Win2k3
			if (gnOsVer < 0x600)
			{
				rcWnd = rcRelative;
			}
			// Windows 7
			else if ((rcParent.bottom - rcRelative.bottom) >= 100)
			{
				// ���������������
				// ����� - �������� �� OS
				if (gnOsVer < 0x600)
				{
					rcWnd = MakeRect(rcRelative.left, rcRelative.bottom + 4, rcParent.right, rcParent.bottom);
				}
				else
				{
					rcWnd = MakeRect(rcParent.left, rcRelative.bottom + 4, rcParent.right, rcParent.bottom);
				}
			}
			else if ((rcParent.right - rcRelative.right) >= 200)
			{
				rcWnd = MakeRect(rcRelative.right + 4, rcRelative.top, rcParent.right, rcRelative.bottom);
			}
			else
			{
				TODO("������ ������� � �������� �� ����������");
				rcWnd = MakeRect(rcParent.left, rcParent.bottom - 100, rcParent.right, rcParent.bottom);
			}
		}
		gpConEmu->wndX = rcWnd.left;
		gpConEmu->wndY = rcWnd.top;
		RECT rcCon = CalcRect(CER_CONSOLE_ALL, rcWnd, CER_MAIN);
		gpConEmu->wndWidth = rcCon.right;
		gpConEmu->wndHeight = rcCon.bottom;

		OnMoving(&rcWnd);

		return rcWnd;
	}

	int nWidth, nHeight;
	MBoxAssert(gpSetCls->FontWidth() && gpSetCls->FontHeight());

	COORD conSize;
	conSize = MakeCoord(gpConEmu->wndWidth,gpConEmu->wndHeight);

	RECT rcFrameMargin = CalcMargins(CEM_FRAMECAPTION|CEM_SCROLL|CEM_STATUS);
	int nShiftX = rcFrameMargin.left + rcFrameMargin.right;
	int nShiftY = rcFrameMargin.top + rcFrameMargin.bottom;
	RECT rcTabMargins = mp_TabBar->GetMargins();
	// ���� ���� ������������ ������ - ����� ������� �� ������, ����� ������ ������� ��� ����������
	nWidth  = conSize.X * gpSetCls->FontWidth() + nShiftX
	          + ((gpSet->isTabs == 1) ? (rcTabMargins.left+rcTabMargins.right) : 0);
	nHeight = conSize.Y * gpSetCls->FontHeight() + nShiftY
	          + ((gpSet->isTabs == 1) ? (rcTabMargins.top+rcTabMargins.bottom) : 0);
	rcWnd = MakeRect(gpConEmu->wndX, gpConEmu->wndY, gpConEmu->wndX+nWidth, gpConEmu->wndY+nHeight);

	if (gpSet->isQuakeStyle)
	{
		HMONITOR hMon;
		POINT pt = {gpConEmu->wndX+2*nShiftX,gpConEmu->wndY+2*nShiftY};
		if (ghWnd)
		{
			RECT rcWnd; GetWindowRect(ghWnd, &rcWnd);
			pt.x = (rcWnd.left+rcWnd.right)/2;
			pt.y = (rcWnd.top+rcWnd.bottom)/2;
		}
		hMon = MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);

		MONITORINFO mi = {sizeof(mi)};
		if (!GetMonitorInfo(hMon, &mi))
			SystemParametersInfo(SPI_GETWORKAREA, 0, &mi.rcWork, 0);

		bool bChange = false;

		// ���� ������� - ��������� �� ������
		if (mi.rcWork.right > mi.rcWork.left)
		{
			switch (gpSet->_WindowMode)
			{
				case wmMaximized:
				{
					rcWnd.left = mi.rcWork.left - rcFrameMargin.left;
					rcWnd.right = mi.rcWork.right + rcFrameMargin.right;
					rcWnd.top = mi.rcWork.top - rcFrameMargin.top;
					rcWnd.bottom = rcWnd.top + nHeight;

					bChange = true;
					break;
				}

				case wmFullScreen:
				{
					rcWnd.left = mi.rcMonitor.left - rcFrameMargin.left;
					rcWnd.right = mi.rcMonitor.right + rcFrameMargin.right;
					rcWnd.top = mi.rcMonitor.top - rcFrameMargin.top;
					rcWnd.bottom = rcWnd.top + nHeight;

					bChange = true;
					break;
				}

				case wmNormal:
				{
					int nWidth = rcWnd.right - rcWnd.left;
					rcWnd.left = max(mi.rcWork.left,((mi.rcWork.left + mi.rcWork.right - nWidth) / 2));
					rcWnd.right = min(mi.rcWork.right,(rcWnd.left + nWidth));
					rcWnd.top = mi.rcWork.top - rcFrameMargin.top;
					rcWnd.bottom = rcWnd.top + nHeight;

					bChange = true;
					break;
				}
			}
		}

		if (bChange)
		{
			ptFullScreenSize.x = rcWnd.right - rcWnd.left + 1;
			ptFullScreenSize.y = mi.rcMonitor.bottom - mi.rcMonitor.top + nShiftY;

			DEBUGTEST(RECT rcCon = CalcRect(CER_CONSOLE_ALL, rcWnd, CER_MAIN));
			//if (rcCon.right)
			//	gpConEmu->wndWidth = rcCon.right;

			gpConEmu->wndX = rcWnd.left;
			gpConEmu->wndY = rcWnd.top;
		}
	}
	else if ( gpSet->wndCascade)
	{
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

		int nX = GetSystemMetrics(SM_CXSIZEFRAME), nY = GetSystemMetrics(SM_CYSIZEFRAME);
		int nWidth = rcWnd.right - rcWnd.left;
		int nHeight = rcWnd.bottom - rcWnd.top;
		// ������, ���� ����� ������ ������� �� ������� ������ - �������� � ����� ������� ����
		BOOL lbX = ((rcWnd.left+nWidth)>(rcScreen.right+nX));
		BOOL lbY = ((rcWnd.top+nHeight)>(rcScreen.bottom+nY));

		if (lbX && lbY)
		{
			rcWnd = MakeRect(rcScreen.left,rcScreen.top,rcScreen.left+nWidth,rcScreen.top+nHeight);
		}
		else if (lbX)
		{
			rcWnd.left = rcScreen.right - nWidth; rcWnd.right = rcScreen.right;
		}
		else if (lbY)
		{
			rcWnd.top = rcScreen.bottom - nHeight; rcWnd.bottom = rcScreen.bottom;
		}

		if (rcWnd.left<(rcScreen.left-nX))
		{
			rcWnd.left=rcScreen.left-nX; rcWnd.right=rcWnd.left+nWidth;
		}

		if (rcWnd.top<(rcScreen.top-nX))
		{
			rcWnd.top=rcScreen.top-nX; rcWnd.bottom=rcWnd.top+nHeight;
		}

		if ((rcWnd.left+nWidth)>(rcScreen.right+nX))
		{
			rcWnd.left = max((rcScreen.left-nX),(rcScreen.right-nWidth));
			nWidth = min(nWidth, (rcScreen.right-rcWnd.left+2*nX));
			rcWnd.right = rcWnd.left + nWidth;
		}

		if ((rcWnd.top+nHeight)>(rcScreen.bottom+nY))
		{
			rcWnd.top = max((rcScreen.top-nY),(rcScreen.bottom-nHeight));
			nHeight = min(nHeight, (rcScreen.bottom-rcWnd.top+2*nY));
			rcWnd.bottom = rcWnd.top + nHeight;
		}

		// ��������������� X/Y ��� �������
		gpConEmu->wndX = rcWnd.left;
		gpConEmu->wndY = rcWnd.top;
	}

	OnMoving(&rcWnd);

	return rcWnd;
}

RECT CConEmuMain::GetGuiClientRect()
{
	RECT rcClient = CalcRect(CER_MAINCLIENT);
	//RECT rcClient = {};
	//BOOL lbRc = ::GetClientRect(ghWnd, &rcClient); UNREFERENCED_PARAMETER(lbRc);
	return rcClient;
}

RECT CConEmuMain::GetVirtualScreenRect(BOOL abFullScreen)
{
	RECT rcScreen = MakeRect(800,600);
	int nMonitors = GetSystemMetrics(SM_CMONITORS);

	if (nMonitors > 1)
	{
		// ������ ������������ ������ �� ���� ���������
		rcScreen.left = GetSystemMetrics(SM_XVIRTUALSCREEN); // may be <0
		rcScreen.top  = GetSystemMetrics(SM_YVIRTUALSCREEN);
		rcScreen.right = rcScreen.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
		rcScreen.bottom = rcScreen.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);

		// ������ �� ��������� �� ������������ Taskbar � ������ ������
		if (!abFullScreen)
		{
			RECT rcPrimaryWork;

			if (SystemParametersInfo(SPI_GETWORKAREA, 0, &rcPrimaryWork, 0))
			{
				if (rcPrimaryWork.left>0 && rcPrimaryWork.left<rcScreen.right)
					rcScreen.left = rcPrimaryWork.left;

				if (rcPrimaryWork.top>0 && rcPrimaryWork.top<rcScreen.bottom)
					rcScreen.top = rcPrimaryWork.top;

				if (rcPrimaryWork.right<rcScreen.right && rcPrimaryWork.right>rcScreen.left)
					rcScreen.right = rcPrimaryWork.right;

				if (rcPrimaryWork.bottom<rcScreen.bottom && rcPrimaryWork.bottom>rcScreen.top)
					rcScreen.bottom = rcPrimaryWork.bottom;
			}
		}
	}
	else
	{
		if (abFullScreen)
		{
			rcScreen = MakeRect(GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN));
		}
		else
		{
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0);
		}
	}

	if (rcScreen.right<=rcScreen.left || rcScreen.bottom<=rcScreen.top)
	{
		_ASSERTE(rcScreen.right>rcScreen.left && rcScreen.bottom>rcScreen.top);
		rcScreen = MakeRect(800,600);
	}

	return rcScreen;
}

HMENU CConEmuMain::GetSysMenu(BOOL abInitial /*= FALSE*/)
{
	HMENU hwndMain = NULL;
	//MENUITEMINFO mi = {sizeof(mi)};
	//wchar_t szText[255];

	
	if (m_InsideIntegration || gpSet->isQuakeStyle)
	{
		if (!mh_InsideSysMenu || abInitial)
		{
			if (mh_InsideSysMenu)
				DestroyMenu(mh_InsideSysMenu);

			mh_InsideSysMenu = CreatePopupMenu();
			AppendMenu(mh_InsideSysMenu, MF_STRING | MF_ENABLED, SC_CLOSE, MenuAccel(vkCloseConEmu,L"&Close ConEmu"));
		}
		hwndMain = mh_InsideSysMenu;
	}
	else
	{
		hwndMain = ::GetSystemMenu(ghWnd, FALSE);

		//// "Alt+F4" ��� ������ "Close" ������ �� �����
		//mi.fMask = MIIM_STRING; mi.dwTypeData = szText; mi.cch = countof(szText);
		//if (GetMenuItemInfo(hwndMain, SC_CLOSE, FALSE, &mi))
		//{
		//	wchar_t* psz = wcschr(szText, L'\t');
		//	if (psz)
		//	{
		//		*psz = 0;
		//		SetMenuItemInfo(hwndMain, SC_CLOSE, FALSE, &mi);
		//	}
		//}
	}

	UpdateSysMenu(hwndMain);

	return hwndMain;
}

void CConEmuMain::UpdateSysMenu(HMENU hSysMenu)
{
	MENUITEMINFO mi = {sizeof(mi)};
	wchar_t szText[255];

	// "Alt+F4" ��� ������ "Close" ������ �� �����
	mi.fMask = MIIM_STRING; mi.dwTypeData = szText; mi.cch = countof(szText);
	if (GetMenuItemInfo(hSysMenu, SC_CLOSE, FALSE, &mi))
	{
		wchar_t* psz = wcschr(szText, L'\t');
		if (psz)
		{
			*psz = 0;
			//SetMenuItemInfo(hSysMenu, SC_CLOSE, FALSE, &mi);
		}
		mi.dwTypeData = (LPWSTR)MenuAccel(vkCloseConEmu,szText);
		if (lstrcmp(mi.dwTypeData, szText) != 0)
		{
			SetMenuItemInfo(hSysMenu, SC_CLOSE, FALSE, &mi);
		}
	}

	// � ���������� ������ ��������� ����������������� �������� ����� ��������� ����������� ��������� ����
	mi.fMask = MIIM_STRING; mi.dwTypeData = szText; mi.cch = countof(szText);
	if (!GetMenuItemInfo(hSysMenu, ID_NEWCONSOLE, FALSE, &mi))
	{
		if (!m_InsideIntegration)
		{
			if (!gpSet->isQuakeStyle)
				InsertMenu(hSysMenu, 0, MF_BYPOSITION | MF_STRING | MF_ENABLED, ID_TOMONITOR, _T("Bring &here"));
			InsertMenu(hSysMenu, 0, MF_BYPOSITION | MF_STRING | MF_ENABLED, ID_TOTRAY, TRAY_ITEM_HIDE_NAME/* L"Hide to &TSA" */);
		}
		InsertMenu(hSysMenu, 0, MF_BYPOSITION, MF_SEPARATOR, 0);
		
		//InsertMenu(hSysMenu, 0, MF_BYPOSITION | MF_STRING | MF_ENABLED, ID_ABOUT, _T("&About / Help"));
		if (mh_HelpPopup) DestroyMenu(mh_HelpPopup);
		mh_HelpPopup = CreateHelpMenuPopup();
		InsertMenu(hSysMenu, 0, MF_BYPOSITION | MF_POPUP | MF_ENABLED, (UINT_PTR)mh_HelpPopup, _T("Hel&p"));
		//if (ms_ConEmuChm[0])  //���������� ����� ������ ���� ���� conemu.chm
		//	InsertMenu(hSysMenu, 0, MF_BYPOSITION | MF_STRING | MF_ENABLED, ID_HELP, _T("&Help"));

		// --------------------
		InsertMenu(hSysMenu, 0, MF_BYPOSITION, MF_SEPARATOR, 0);
		
		if (mh_DebugPopup) DestroyMenu(mh_DebugPopup);
		mh_DebugPopup = CreateDebugMenuPopup();
		InsertMenu(hSysMenu, 0, MF_BYPOSITION | MF_POPUP | MF_ENABLED, (UINT_PTR)mh_DebugPopup, _T("&Debug"));
		
		if (mh_EditPopup) DestroyMenu(mh_EditPopup);
		mh_EditPopup = CreateEditMenuPopup(NULL);
		InsertMenu(hSysMenu, 0, MF_BYPOSITION | MF_POPUP | MF_ENABLED, (UINT_PTR)mh_EditPopup, _T("Ed&it"));
		
		// --------------------
		InsertMenu(hSysMenu, 0, MF_BYPOSITION, MF_SEPARATOR, 0);
		
		if (mh_VConListPopup) DestroyMenu(mh_VConListPopup);
		mh_VConListPopup = CreateVConListPopupMenu(mh_VConListPopup, TRUE/*abFirstTabOnly*/);
		InsertMenu(hSysMenu, 0, MF_BYPOSITION | MF_POPUP | MF_ENABLED, (UINT_PTR)mh_VConListPopup, _T("Console &list"));
		
		if (mh_ActiveVConPopup) DestroyMenu(mh_ActiveVConPopup);
		if (mh_TerminateVConPopup) { DestroyMenu(mh_TerminateVConPopup); mh_TerminateVConPopup = NULL; }
		mh_ActiveVConPopup = CreateVConPopupMenu(NULL, NULL, FALSE, mh_TerminateVConPopup);
		InsertMenu(hSysMenu, 0, MF_BYPOSITION | MF_POPUP | MF_ENABLED, (UINT_PTR)mh_ActiveVConPopup, _T("Acti&ve console"));
		
		// --------------------
		InsertMenu(hSysMenu, 0, MF_BYPOSITION, MF_SEPARATOR, 0);
		if (!m_InsideIntegration)
		{
			InsertMenu(hSysMenu, 0, MF_BYPOSITION | MF_STRING | MF_ENABLED | (gpSet->isAlwaysOnTop ? MF_CHECKED : 0),
				ID_ALWAYSONTOP, MenuAccel(vkAlwaysOnTop,L"Al&ways on top"));
		}
		#ifdef SHOW_AUTOSCROLL
		InsertMenu(hSysMenu, 0, MF_BYPOSITION | MF_STRING | MF_ENABLED | (gpSetCls->AutoScroll ? MF_CHECKED : 0),
			ID_AUTOSCROLL, _T("Auto scro&ll"));
		#endif
		InsertMenu(hSysMenu, 0, MF_BYPOSITION | MF_STRING | MF_ENABLED, ID_SETTINGS, MenuAccel(vkWinAltP,L"S&ettings..."));
		InsertMenu(hSysMenu, 0, MF_BYPOSITION | MF_STRING | MF_ENABLED, IDM_ATTACHTO, MenuAccel(vkMultiNewAttach,L"Attach t&o..."));
		InsertMenu(hSysMenu, 0, MF_BYPOSITION | MF_STRING | MF_ENABLED, ID_NEWCONSOLE, MenuAccel(vkMultiNew,L"&New console..."));
	}
}

void CConEmuMain::SetWindowStyle(DWORD anStyle)
{
	SetWindowStyle(ghWnd, anStyle);
}

void CConEmuMain::SetWindowStyle(HWND ahWnd, DWORD anStyle)
{
	LONG lRc = SetWindowLong(ahWnd, GWL_STYLE, anStyle);
	UNREFERENCED_PARAMETER(lRc);
}

// ��� ������� ����������� ����������� ����� �� ������� ����������, � �� ���������� GWL_STYLE
DWORD CConEmuMain::GetWindowStyle()
{
	DWORD style = WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	if (m_InsideIntegration)
	{
		style |= WS_CHILD|WS_SYSMENU;
	}
	else
	{
		//if (gpSet->isShowOnTaskBar) // ghWndApp
		//	style |= WS_POPUPWINDOW | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
		//else
		style |= WS_OVERLAPPEDWINDOW;
		style = FixWindowStyle(style);
		//if (gpSet->isTabsOnTaskBar() && gOSVer.dwMajorVersion <= 5)
		//{
		//	style |= WS_POPUP;
		//}

		#ifndef CHILD_DESK_MODE
		if (gpSet->isDesktopMode)
			style |= WS_POPUP;
		#endif

		//if (ghWnd) {
		//	if (gpSet->isHideCaptionAlways)
		//		style &= ~(WS_CAPTION/*|WS_THICKFRAME*/);
		//	else
		//		style |= (WS_CAPTION|/*WS_THICKFRAME|*/WS_MINIMIZEBOX|WS_MAXIMIZEBOX);
		//}
	}
	return style;
}

DWORD CConEmuMain::FixWindowStyle(DWORD dwStyle, ConEmuWindowMode wmNewMode /*= wmCurrent*/)
{
	/* WS_CAPTION == (WS_BORDER | WS_DLGFRAME) */

	if ((wmNewMode == wmCurrent) || (wmNewMode == wmNotChanging))
	{
		wmNewMode = WindowMode;
	}

	if (m_InsideIntegration)
	{
		dwStyle &= ~(WS_CAPTION|WS_THICKFRAME);
		dwStyle |= WS_CHILD|WS_SYSMENU;
	}
	else if (gpSet->isCaptionHidden(wmNewMode))
	{
		//Win& & Quake - �� �������� "Slide up/down" ���� ���� ThickFrame
		//if ((gpSet->isQuakeStyle == 0) // �� ��� Quake. ��� ���� ����� �����, ����� ���������
		if ((wmNewMode == wmFullScreen) || (wmNewMode == wmMaximized))
		{
			dwStyle &= ~(WS_CAPTION|WS_THICKFRAME);
		}
		else if ((m_ForceShowFrame == fsf_Show) || !gpSet->isFrameHidden())
		{
			dwStyle &= ~(WS_CAPTION);
			dwStyle |= WS_THICKFRAME;
		}
		else
		{
			dwStyle &= ~(WS_CAPTION|WS_THICKFRAME);
			dwStyle |= WS_DLGFRAME;
		}
	}
	else
	{
		/* WS_CAPTION == WS_BORDER | WS_DLGFRAME  */
		dwStyle |= WS_CAPTION|WS_THICKFRAME;
	}

	return dwStyle;
}

void CConEmuMain::SetWindowStyleEx(DWORD anStyleEx)
{
	SetWindowStyleEx(ghWnd, anStyleEx);
}

void CConEmuMain::SetWindowStyleEx(HWND ahWnd, DWORD anStyleEx)
{
	LONG lRc = SetWindowLong(ahWnd, GWL_EXSTYLE, anStyleEx);
	UNREFERENCED_PARAMETER(lRc);
}

// ��� ������� ����������� ����������� ����� �� ������� ����������, � �� ���������� GWL_STYLE_EX
DWORD CConEmuMain::GetWindowStyleEx()
{
	DWORD styleEx = 0;

	if (m_InsideIntegration)
	{
		// ������ ����� �� ����
	}
	else
	{
		styleEx |= WS_EX_APPWINDOW;

		if (gpSet->nTransparent < 255 /*&& !gpSet->isDesktopMode*/)
			styleEx |= WS_EX_LAYERED;

		if (gpSet->isAlwaysOnTop)
			styleEx |= WS_EX_TOPMOST;

		if (!gpSet->isWindowOnTaskBar(true))
		{
			styleEx &= ~WS_EX_APPWINDOW;
			//styleEx |= WS_EX_TOOLWINDOW;
		}

		#ifndef CHILD_DESK_MODE
		if (gpSet->isDesktopMode)
			styleEx |= WS_EX_TOOLWINDOW;
		#endif
	}

	return styleEx;
}

// ��� ������� ����������� ����������� ����� �� ������� ����������, � �� ���������� GWL_STYLE
DWORD CConEmuMain::GetWorkWindowStyle()
{
	DWORD style = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE;
	return style;
}

// ��� ������� ����������� ����������� ����� �� ������� ����������, � �� ���������� GWL_STYLE_EX
DWORD CConEmuMain::GetWorkWindowStyleEx()
{
	DWORD styleEx = 0;
	return styleEx;
}

void CConEmuMain::CreateLog()
{
	if (!gpSetCls->isAdvLogging)
	{
		SafeDelete(mp_Log);
	}

	mp_Log = new MFileLog(L"ConEmu-gui", ms_ConEmuExeDir, GetCurrentProcessId());

	HRESULT hr = mp_Log ? mp_Log->CreateLogFile(L"ConEmu-gui", GetCurrentProcessId()) : E_UNEXPECTED;
	if (hr != 0)
	{
		wchar_t szError[MAX_PATH*2];
		_wsprintf(szError, SKIPLEN(countof(szError)) L"Create log file failed! ErrCode=0x%08X\n%s\n", (DWORD)hr, mp_Log->GetLogFileName());
		MBoxA(szError);
		SafeDelete(mp_Log);
		return;
	}

	mp_Log->LogStartEnv(gpStartEnv);
}

void CConEmuMain::LogString(LPCWSTR asInfo, bool abWriteTime /*= true*/, bool abWriteLine /*= true*/)
{
	if (!this || !mp_Log)
		return;

	mp_Log->LogString(asInfo, abWriteTime, NULL, abWriteLine);
}

void CConEmuMain::LogString(LPCSTR asInfo, bool abWriteTime /*= true*/, bool abWriteLine /*= true*/)
{
	if (!this || !mp_Log)
		return;

	mp_Log->LogString(asInfo, abWriteTime, NULL, abWriteLine);
}

BOOL CConEmuMain::CreateMainWindow()
{
	//if (!Init()) -- ���������� �� WinMain
	//	return FALSE; // ������ ��� ��������

	if (_tcscmp(VirtualConsoleClass,VirtualConsoleClassMain)) //-V549
	{
		MBoxA(_T("Error: class names must be equal!"));
		return FALSE;
	}

	// 2009-06-11 ��������, ��� CS_SAVEBITS �������� � ������ ���������
	// �������� ����������������� ��������� ����� ������ (������� ������ ������?)
	WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_DBLCLKS|CS_OWNDC/*|CS_SAVEBITS*/, CConEmuMain::MainWndProc, 0, 16,
	                 g_hInstance, hClassIcon, LoadCursor(NULL, IDC_ARROW),
	                 NULL /*(HBRUSH)COLOR_BACKGROUND*/,
	                 NULL, gsClassNameParent, hClassIconSm
	                };// | CS_DROPSHADOW

	WNDCLASSEX wcWork = {sizeof(WNDCLASSEX), CS_DBLCLKS|CS_OWNDC/*|CS_SAVEBITS*/, CConEmuMain::WorkWndProc, 0, 16,
	                 g_hInstance, hClassIcon, LoadCursor(NULL, IDC_ARROW),
	                 NULL /*(HBRUSH)COLOR_BACKGROUND*/,
	                 NULL, gsClassNameWork, hClassIconSm
	                };// | CS_DROPSHADOW

	WNDCLASSEX wcBack = {sizeof(WNDCLASSEX), CS_DBLCLKS|CS_OWNDC/*|CS_SAVEBITS*/, CConEmuChild::BackWndProc, 0, 16,
	                 g_hInstance, hClassIcon, LoadCursor(NULL, IDC_ARROW),
	                 NULL /*(HBRUSH)COLOR_BACKGROUND*/,
	                 NULL, gsClassNameBack, hClassIconSm
	                };// | CS_DROPSHADOW

	if (!RegisterClassEx(&wc) || !RegisterClassEx(&wcWork) || !RegisterClassEx(&wcBack))
	{
		DisplayLastError(L"Failed to register class names");
		return -1;
	}

	if (m_InsideIntegration)
	{
		if (!mh_InsideParentWND)
		{
			_ASSERTE(!m_InsideIntegration || mh_InsideParentWND);
			m_InsideIntegration = ii_None;
		}
		else
		{
			DWORD nParentTID, nParentPID;
			nParentTID = GetWindowThreadProcessId(mh_InsideParentWND, &nParentPID);
			_ASSERTE(nParentTID && nParentPID);
			BOOL bAttach = AttachThreadInput(GetCurrentThreadId(), nParentTID, TRUE);
			if (!bAttach)
			{
				DisplayLastError(L"Inside: AttachThreadInput() failed!");
			}
		}
	}

	DWORD styleEx = GetWindowStyleEx();
	DWORD style = GetWindowStyle();
	//	WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	////if (gpSet->isShowOnTaskBar) // ghWndApp
	////	style |= WS_POPUPWINDOW | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	////else
	//style |= WS_OVERLAPPEDWINDOW;
	//if (gpSet->nTransparent < 255 /*&& !gpSet->isDesktopMode*/)
	//	styleEx |= WS_EX_LAYERED;
	//if (gpSet->isAlwaysOnTop)
	//	styleEx |= WS_EX_TOPMOST;
	////if (gpSet->isHideCaptionAlways) // ����� ������� ��� ������-�� �� ����������
	////	style &= ~(WS_CAPTION);
	int nWidth=CW_USEDEFAULT, nHeight=CW_USEDEFAULT;

	// In principle, this MUST be wmNormal on startup, even if started in Maximized/FullScreen/Iconic
	_ASSERTE(WindowMode==wmNormal);

	// ������ �������� ���� � Normal ������
	if ((gpConEmu->wndWidth && gpConEmu->wndHeight) || m_InsideIntegration)
	{
		MBoxAssert(gpSetCls->FontWidth() && gpSetCls->FontHeight());
		//COORD conSize; conSize.X=gpSet->wndWidth; conSize.Y=gpSet->wndHeight;
		////int nShiftX = GetSystemMetrics(SM_CXSIZEFRAME)*2;
		////int nShiftY = GetSystemMetrics(SM_CYSIZEFRAME)*2 + (gpSet->isHideCaptionAlways ? 0 : GetSystemMetrics(SM_CYCAPTION));
		//RECT rcFrameMargin = CalcMargins(CEM_FRAME);
		//int nShiftX = rcFrameMargin.left + rcFrameMargin.right;
		//int nShiftY = rcFrameMargin.top + rcFrameMargin.bottom;
		//// ���� ���� ������������ ������ - ����� ������� �� ������, ����� ������ ������� ��� ����������
		//nWidth  = conSize.X * gpSetCls->FontWidth() + nShiftX
		//	+ ((gpSet->isTabs == 1) ? (gpSet->rcTabMargins.left+gpSet->rcTabMargins.right) : 0);
		//nHeight = conSize.Y * gpSetCls->FontHeight() + nShiftY
		//	+ ((gpSet->isTabs == 1) ? (gpSet->rcTabMargins.top+gpSet->rcTabMargins.bottom) : 0);
		//mrc_Ideal = MakeRect(gpSet->wndX, gpSet->wndY, gpSet->wndX+nWidth, gpSet->wndY+nHeight);
		RECT rcWnd = GetDefaultRect();
		UpdateIdealRect(rcWnd);
		nWidth = rcWnd.right - rcWnd.left;
		nHeight = rcWnd.bottom - rcWnd.top;
	}
	else
	{
		_ASSERTE(gpConEmu->wndWidth && gpConEmu->wndHeight);
	}

	HWND hParent = m_InsideIntegration ? mh_InsideParentWND : ghWndApp;

	if (gpSetCls->isAdvLogging)
	{
		wchar_t szCreate[128];
		_wsprintf(szCreate, SKIPLEN(countof(szCreate)) L"Creating main window.%s%s%s Parent=x%08X X=%i Y=%i W=%i H=%i style=x%08X exStyle=x%08X Mode=%u",
			(DWORD)hParent,
			gpSet->isQuakeStyle ? L" Quake" : L"",
			gpConEmu->m_InsideIntegration ? L" Inside" : L"",
			gpSet->isDesktopMode ? L" Desktop" : L"",
			gpConEmu->wndX, gpConEmu->wndY, nWidth, nHeight, style, styleEx,
			gpSet->isQuakeStyle ? gpSet->_WindowMode : WindowMode);
		LogString(szCreate);
	}

	//if (gpConEmu->WindowMode == wmMaximized) style |= WS_MAXIMIZE;
	//style |= WS_VISIBLE;
	// cRect.right - cRect.left - 4, cRect.bottom - cRect.top - 4; -- ��� ����� ��� ���� �� ���������
	WARNING("�� ����� �������� �� ������� ������� �������");
	ghWnd = CreateWindowEx(styleEx, gsClassNameParent, gpSet->GetCmd(), style,
	                       gpConEmu->wndX, gpConEmu->wndY, nWidth, nHeight, hParent, NULL, (HINSTANCE)g_hInstance, NULL);

	if (!ghWnd)
	{
		//if (!ghWnd DC)
		MBoxA(_T("Can't create main window!"));

		return FALSE;
	}

	//if (gpSet->isHideCaptionAlways)
	//	OnHideCaption();
#ifdef _DEBUG
	DWORD style2 = GetWindowLongPtr(ghWnd, GWL_STYLE);
	DWORD styleEx2 = GetWindowLongPtr(ghWnd, GWL_EXSTYLE);
#endif
	OnTransparent();
	//if (gpConEmu->WindowMode == wmFullScreen || gpConEmu->WindowMode == wmMaximized)
	//	gpConEmu->SetWindowMode(gpConEmu->WindowMode);
	UpdateGuiInfoMapping();
	return TRUE;
}

BOOL CConEmuMain::CreateWorkWindow()
{
	HWND  hParent = ghWnd;
	RECT  rcClient = CalcRect(CER_WORKSPACE);
	DWORD styleEx = GetWorkWindowStyleEx();
	DWORD style = GetWorkWindowStyle();

	_ASSERTE(hParent!=NULL);

	ghWndWork = CreateWindowEx(styleEx, gsClassNameWork, L"ConEmu Workspace", style,
	                       rcClient.left, rcClient.top, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
	                       hParent, NULL, (HINSTANCE)g_hInstance, NULL);
	if (!ghWndWork)
	{
		DisplayLastError(L"Failed to create workspace window");
		return FALSE;
	}

	return TRUE;
}

bool CConEmuMain::SetParent(HWND hNewParent)
{
	HWND hCurrent = GetParent(ghWnd);
	if (hCurrent != hNewParent)
	{
		//::SetParent(ghWnd, NULL);
		::SetParent(ghWnd, hNewParent);
	}
	HWND hSet = GetParent(ghWnd);
	bool lbSuccess = (hSet == hNewParent) || (hNewParent == NULL && hSet == GetDesktopWindow());
	return lbSuccess;
}

void CConEmuMain::FillConEmuMainFont(ConEmuMainFont* pFont)
{
	// ��������� ��������� ������ ConEmu
	lstrcpy(pFont->sFontName, gpSetCls->FontFaceName());
	pFont->nFontHeight = gpSetCls->FontHeight();
	pFont->nFontWidth = gpSetCls->FontWidth();
	pFont->nFontCellWidth = gpSet->FontSizeX3 ? gpSet->FontSizeX3 : gpSetCls->FontWidth();
	pFont->nFontQuality = gpSetCls->FontQuality();
	pFont->nFontCharSet = gpSetCls->FontCharSet();
	pFont->Bold = gpSetCls->FontBold();
	pFont->Italic = gpSetCls->FontItalic();
	lstrcpy(pFont->sBorderFontName, gpSetCls->BorderFontFaceName());
	pFont->nBorderFontWidth = gpSetCls->BorderFontWidth();
}

BOOL CConEmuMain::EnumInsideFindParent(HWND hwnd, LPARAM lParam)
{
	DWORD nPID = 0;
	if (IsWindowVisible(hwnd)
		&& GetWindowThreadProcessId(hwnd, &nPID)
		&& (nPID == (DWORD)lParam))
	{
		DWORD nNeedStyles = WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX;
		DWORD nStyles = GetWindowLong(hwnd, GWL_STYLE);
		if ((nStyles & nNeedStyles) == nNeedStyles)
		{
			// �����
			gpConEmu->mn_InsideParentPID = nPID;
			gpConEmu->mh_InsideParentRoot = hwnd;
			return FALSE;
		}
	}

	// Next window
	return TRUE;
}

HWND  CConEmuMain::InsideFindConEmu(HWND hFrom)
{
	wchar_t szClass[128];
	HWND hChild = NULL, hNext = NULL;
	//HWND hXpView = NULL, hXpPlace = NULL;

	while ((hChild = FindWindowEx(hFrom, hChild, NULL, NULL)) != NULL)
	{
		GetClassName(hChild, szClass, countof(szClass));
		if (lstrcmp(szClass, gsClassNameParent) == 0)
		{
			return hChild;
		}

		hNext = InsideFindConEmu(hChild);
		if (hNext)
		{
			return hNext;
		}
	}

	return NULL;
}

bool CConEmuMain::InsideFindShellView(HWND hFrom)
{
	wchar_t szClass[128];
	wchar_t szParent[128];
	wchar_t szRoot[128];
	HWND hChild = NULL;
	// ��� WinXP
	HWND hXpView = NULL, hXpPlace = NULL;

	while ((hChild = FindWindowEx(hFrom, hChild, NULL, NULL)) != NULL)
	{
		// ��� ���������� ������ ������� ����!
		if (!IsWindowVisible(hChild))
			continue;

		// Windows 7, Windows 8.
		GetClassName(hChild, szClass, countof(szClass));
		if (lstrcmp(szClass, L"SHELLDLL_DefView") == 0)
		{
			GetClassName(hFrom, szParent, countof(szParent));
			if (lstrcmp(szParent, L"CtrlNotifySink") == 0)
			{
				HWND hParent = GetParent(hFrom);
				if (hParent)
				{
					GetClassName(hParent, szRoot, countof(szRoot));
					_ASSERTE(lstrcmp(szRoot, L"DirectUIHWND") == 0);

					mh_InsideParentWND = hParent;
					mh_InsideParentRel = hFrom;
					m_InsideIntegration = ii_Explorer;

					return true;
				}
			}
			else if ((gnOsVer < 0x600) && (lstrcmp(szParent, L"ExploreWClass") == 0))
			{
				_ASSERTE(mh_InsideParentRoot == hFrom);
				hXpView = hChild;
			}
		}
		else if ((gnOsVer < 0x600) && (lstrcmp(szClass, L"BaseBar") == 0))
		{
			RECT rcBar = {}; GetWindowRect(hChild, &rcBar);
			MapWindowPoints(NULL, hFrom, (LPPOINT)&rcBar, 2);
			RECT rcParent = {}; GetClientRect(hFrom, &rcParent);
			if ((-10 <= (rcBar.right - rcParent.right))
				&& ((rcBar.right - rcParent.right) <= 10))
			{
				// ��� ���������� �������, ������������ � �������-������� ����
				hXpPlace = hChild;
			}
		}
		// ���� � ���� (hChild) �������� � ������� "Address: D:\users\max"
		else if ((gnOsVer >= 0x600) && lstrcmp(szClass, L"ToolbarWindow32") == 0)
		{
			GetClassName(hFrom, szParent, countof(szParent));
			if (lstrcmp(szParent, L"Breadcrumb Parent") == 0)
			{
				HWND hParent = GetParent(hFrom);
				if (hParent)
				{
					GetClassName(hParent, szRoot, countof(szRoot));
					_ASSERTE(lstrcmp(szRoot, L"msctls_progress32") == 0);

					mh_InsideParentPath = hChild;

					// �������� ComboBox/Edit, � ������� ����� ��������� ����, ����� ��������� ��������� �� ���� �������
					// �� ���� ��������. ���� ������� �� ��������� ��� �������� ����!

					return true;
				}
			}
		}

		if ((hChild != hXpView) && (hChild != hXpPlace))
		{
			if (InsideFindShellView(hChild))
			{
				if (mh_InsideParentRel && mh_InsideParentPath)
					return true;
				else
					break;
			}
		}

		if (hXpView && hXpPlace)
		{
			mh_InsideParentRel = FindWindowEx(hXpPlace, NULL, L"ReBarWindow32", NULL);
			if (!mh_InsideParentRel)
			{
				_ASSERTE(mh_InsideParentRel && L"ReBar must be found on XP & 2k3");
				return true; // ��������� �����
			}
			mh_InsideParentWND = hXpPlace;
			mh_InsideParentPath = mh_InsideParentRoot;
			m_InsideIntegration = ii_Explorer;
			return true;
		}
	}

	return false;
}

HWND CConEmuMain::InsideFindParent()
{
	bool bFirstStep = true;
	DWORD nParentPID = 0;

	if (!m_InsideIntegration)
	{
		return NULL;
	}

	if (mh_InsideParentWND)
	{
		if (IsWindow(mh_InsideParentWND))
		{
			if (m_InsideIntegration == ii_Simple)
			{
				if (mh_InsideParentRoot == NULL)
				{
					// ���� ��� �� ������ "��������" ����
					HWND hParent = mh_InsideParentWND;
					while (hParent)
					{
						mh_InsideParentRoot = hParent;
						hParent = GetParent(hParent);
					}
				}
				// � ���� ������ �������� ��� ���������� �������
				_ASSERTE(mh_InsideParentRel==NULL);
				mh_InsideParentRel = NULL;
			}

			_ASSERTE(mh_InsideParentWND!=NULL);
			goto wrap;
		}
		else
		{
			if (m_InsideIntegration == ii_Simple)
			{
				DisplayLastError(L"Specified window not found");
				mh_InsideParentWND = NULL;
				goto wrap;
			}
			_ASSERTE(IsWindow(mh_InsideParentWND));
			mh_InsideParentRoot = mh_InsideParentWND = mh_InsideParentRel = NULL;
		}
	}

	_ASSERTE(m_InsideIntegration!=ii_Simple);

	if (gpConEmu->mn_InsideParentPID)
	{
		PROCESSENTRY32 pi = {sizeof(pi)};
		if ((gpConEmu->mn_InsideParentPID == GetCurrentProcessId())
			|| !GetProcessInfo(gpConEmu->mn_InsideParentPID, &pi))
		{
			DisplayLastError(L"Invalid parent process specified");
			m_InsideIntegration = ii_None;
			mh_InsideParentWND = NULL;
			goto wrap;
		}
		nParentPID = gpConEmu->mn_InsideParentPID;
	}
	else
	{
		PROCESSENTRY32 pi = {sizeof(pi)};
		if (!GetProcessInfo(GetCurrentProcessId(), &pi) || !pi.th32ParentProcessID)
		{
			DisplayLastError(L"GetProcessInfo(GetCurrentProcessId()) failed");
			m_InsideIntegration = ii_None;
			mh_InsideParentWND = NULL;
			goto wrap;
		}
		nParentPID = pi.th32ParentProcessID;
	}

	EnumWindows(EnumInsideFindParent, nParentPID);
	if (!mh_InsideParentRoot)
	{
		int nBtn = MessageBox(L"Can't find appropriate parent window!\n\nContinue in normal mode?", MB_ICONSTOP|MB_YESNO|MB_DEFBUTTON2);
		if (nBtn != IDYES)
		{
			mh_InsideParentWND = (HWND)-1;
			return mh_InsideParentWND; // ���������!
		}
		// ���������� � ������� ������
		m_InsideIntegration = ii_None;
		mh_InsideParentWND = NULL;
		goto wrap;
	}


    HWND hExistConEmu;
    if ((hExistConEmu = InsideFindConEmu(mh_InsideParentRoot)) != NULL)
    {
    	_ASSERTE(FALSE && "Continue to create tab in existing instance");
    	// ���� � ���������� ��� ���� ConEmu - ������� � ��� ����� �������
    	gpSetCls->SingleInstanceShowHide = sih_None;
    	LPCWSTR pszCmdLine = GetCommandLine();
    	LPCWSTR pszCmd = StrStrI(pszCmdLine, L" /cmd ");
    	RunSingleInstance(hExistConEmu, pszCmd ? (pszCmd + 6) : NULL);

		mh_InsideParentWND = (HWND)-1;
		return mh_InsideParentWND; // ���������!
    }

	// ������ ����� ����� �������� ����
	// 1. � ������� ����� ����������
	// 2. �� �������� ����� �����������������
	// 3. ��� ������������� �������� ����
	InsideFindShellView(mh_InsideParentRoot);

RepeatCheck:
	if (!mh_InsideParentWND || (!mh_InsideParentRel && (m_InsideIntegration == ii_Explorer)))
	{
		int nBtn = IDCANCEL;
		wchar_t szAddMsg[128] = L"", szMsg[1024];
		if (bFirstStep)
		{
			bFirstStep = false;
			if (gnOsVer < 0x600)
			{
				// WinXP, Win2k3
				nBtn = MessageBox(L"Tip pane is not found in Explorer window!\nThis pane is required for 'ConEmu Inside' mode.\nDo you want to show this pane?", MB_ICONQUESTION|MB_YESNO);
				if (nBtn == IDYES)
				{
					DWORD_PTR nRc;
					LRESULT lSendRc;

				#if 0

					HWND hWorker = GetDlgItem(mh_InsideParentRoot, 0xA005);

					#ifdef _DEBUG
					if (hWorker)
					{
						HWND hReBar = FindWindowEx(hWorker, NULL, L"ReBarWindow32", NULL);
						if (hReBar)
						{
							POINT pt = {4,4};
							HWND hTool = ChildWindowFromPoint(hReBar, pt);
							if (hTool)
							{
								//TBBUTTON tb = {};
								//lSendRc = SendMessageTimeout(mh_InsideParentRoot, TB_GETBUTTON, 2, (LPARAM)&tb, SMTO_NOTIMEOUTIFNOTHUNG, 500, &nRc);
								//if (tb.idCommand)
								//{
								//	NMTOOLBAR nm = {{hTool, 0, TBN_DROPDOWN}, 32896/*tb.idCommand*/};
								//	lSendRc = SendMessageTimeout(mh_InsideParentRoot, WM_NOTIFY, 0, (LPARAM)&nm, SMTO_NOTIMEOUTIFNOTHUNG, 500, &nRc);
								//}
								lSendRc = SendMessageTimeout(mh_InsideParentRoot, WM_COMMAND, 32896, 0, SMTO_NOTIMEOUTIFNOTHUNG, 500, &nRc);
							}
						}
					}
					// There is no way to force "Tip pane" in WinXP
					// if popup menu "View -> Explorer bar" was not _shown_ at least once
					// In that case, Explorer ignores WM_COMMAND(41536) and does not reveal tip pane.
					HMENU hMenu1 = GetMenu(mh_InsideParentRoot), hMenu2 = NULL, hMenu3 = NULL;
					if (hMenu1)
					{
						hMenu2 = GetSubMenu(hMenu1, 2);
						//if (!hMenu2)
						//{
						//	lSendRc = SendMessageTimeout(mh_InsideParentRoot, WM_MENUSELECT, MAKELONG(2,MF_POPUP), (LPARAM)hMenu1, SMTO_NOTIMEOUTIFNOTHUNG, 1500, &nRc);
						//	hMenu2 = GetSubMenu(hMenu1, 2);
						//}

						if (hMenu2)
						{
							hMenu3 = GetSubMenu(hMenu2, 2);
						}
					}
					#endif
				#endif

					//INPUT keys[4] = {INPUT_KEYBOARD,INPUT_KEYBOARD,INPUT_KEYBOARD,INPUT_KEYBOARD};
					//keys[0].ki.wVk = VK_F10; keys[0].ki.dwFlags = 0; //keys[0].ki.wScan = MapVirtualKey(VK_F10,MAPVK_VK_TO_VSC);
					//keys[1].ki.wVk = VK_F10; keys[1].ki.dwFlags = KEYEVENTF_KEYUP; //keys[0].ki.wScan = MapVirtualKey(VK_F10,MAPVK_VK_TO_VSC);
					//keys[1].ki.wVk = 'V';
					//keys[2].ki.wVk = 'E';
					//keys[3].ki.wVk = 'T';

					//LRESULT lSentRc = 0;
					//SetForegroundWindow(mh_InsideParentRoot);
					//LRESULT lSentRc = SendInput(2/*countof(keys)*/, keys, sizeof(keys[0]));
					//lSentRc = SendMessageTimeout(mh_InsideParentRoot, WM_SYSKEYUP, 'V', 0, SMTO_NOTIMEOUTIFNOTHUNG, 5000, &nRc);

					lSendRc = SendMessageTimeout(mh_InsideParentRoot, WM_COMMAND, 41536, 0, SMTO_NOTIMEOUTIFNOTHUNG, 5000, &nRc);
					UNREFERENCED_PARAMETER(lSendRc);

					mb_InsidePaneWasForced = true;
					// ���������� ��������� ���� �� �������
					wcscpy_c(szAddMsg, L"Forcing Explorer to show Tip pane failed.\nShow pane yourself and recall ConEmu.\n\n");
				}
			}

			if (nBtn == IDYES)
			{
				// ������ ��������
				mh_InsideParentWND = mh_InsideParentRel = NULL;
				m_InsideIntegration = ii_Auto;
				InsideFindShellView(mh_InsideParentRoot);
				if (mh_InsideParentWND && mh_InsideParentRel)
					goto RepeatCheck;
				// ���� �� ����� - �������� � ��������� ��������
				Sleep(1500);
				mh_InsideParentWND = mh_InsideParentRel = NULL;
				m_InsideIntegration = ii_Auto;
				InsideFindShellView(mh_InsideParentRoot);
				if (mh_InsideParentWND && mh_InsideParentRel)
					goto RepeatCheck;
			}
		}

		//MessageBox(L"Can't find appropriate shell window!", MB_ICONSTOP);
		_wsprintf(szMsg, SKIPLEN(countof(szMsg)) L"%sCan't find appropriate shell window!\nUnrecognized layout of the Explorer.\n\nContinue in normal mode?", szAddMsg);
		nBtn = MessageBox(szMsg, MB_ICONSTOP|MB_YESNO|MB_DEFBUTTON2);

		if (nBtn != IDYES)
		{
			mh_InsideParentWND = (HWND)-1;
			return mh_InsideParentWND; // ���������!
		}
		m_InsideIntegration = ii_None;
		mh_InsideParentRoot = NULL;
		mh_InsideParentWND = NULL;
		goto wrap;
	}

wrap:
	if (!mh_InsideParentWND)
	{
		m_InsideIntegration = ii_None;
		mh_InsideParentRoot = NULL;
	}
	else
	{
		GetWindowThreadProcessId(mh_InsideParentWND, &gpConEmu->mn_InsideParentPID);
		// ��� ����������� �����
		GetCurrentDirectory(countof(ms_InsideParentPath), ms_InsideParentPath);
		int nLen = lstrlen(ms_InsideParentPath);
		if ((nLen > 3) && (ms_InsideParentPath[nLen-1] == L'\\'))
		{
			ms_InsideParentPath[nLen-1] = 0;
		}
	}

	return mh_InsideParentWND;
}

void CConEmuMain::InsideParentMonitor()
{
	// ��� ����� ��������� "����������" - �������� ���� ������/���������
	InsideUpdatePlacement();

	if (mb_InsideSynchronizeCurDir)
	{
		// ��� ����� ����� � ����������
		InsideUpdateDir();
	}
}

void CConEmuMain::InsideUpdateDir()
{
	CVConGuard VCon;
	
	if (mh_InsideParentPath && IsWindow(mh_InsideParentPath) && (GetActiveVCon(&VCon) >= 0) && VCon->RCon())
	{
		wchar_t szCurText[512] = {};
		DWORD_PTR lRc = 0;
		if (SendMessageTimeout(mh_InsideParentPath, WM_GETTEXT, countof(szCurText), (LPARAM)szCurText, SMTO_ABORTIFHUNG|SMTO_NORMAL, 300, &lRc))
		{
			if (gnOsVer < 0x600)
			{
				// ���� � ��������� ��� ������� ����
				if (wcschr(szCurText, L'\\') == NULL)
				{
					// ����� �������
					return;
				}
			}

			LPCWSTR pszPath = NULL;
			// ���� ��� ��� ���� - �� ������� �� ��������
            if ((szCurText[0] == L'\\' && szCurText[1] == L'\\' && szCurText[2]) // ������� ����
            	|| (szCurText[0] && szCurText[1] == L':' && szCurText[2] == L'\\' /*&& szCurText[3]*/)) // ���� ����� ����� �����
        	{
        		pszPath = szCurText;
        	}
        	else
        	{
        		// ����� - �������� �������. �� ���������� ����� ��� "Address: D:\dir1\dir2"
				pszPath = wcschr(szCurText, L':');
        		if (pszPath)
        			pszPath = SkipNonPrintable(pszPath+1);
        	}

        	// ���� ������� - ���������� � ms_InsideParentPath
        	if (pszPath && *pszPath && (lstrcmpi(ms_InsideParentPath, pszPath) != 0))
        	{
        		int nLen = lstrlen(pszPath);
        		if (nLen >= (int)countof(ms_InsideParentPath))
        		{
        			_ASSERTE((nLen<countof(ms_InsideParentPath)) && "Too long path?");
        		}
        		else //if (VCon->RCon())
        		{
        			// ��������� ��� ���������
        			lstrcpyn(ms_InsideParentPath, pszPath, countof(ms_InsideParentPath));
        			// ����������� ������� ��� ���������� � Shell
        			VCon->RCon()->PostPromptCmd(true, pszPath);
        		}
        	}
		}
	}
}

void CConEmuMain::InsideUpdatePlacement()
{
	if (!mh_InsideParentWND || !IsWindow(mh_InsideParentWND))
		return;

	if ((m_InsideIntegration != ii_Explorer) && (m_InsideIntegration != ii_Simple))
		return;

	if ((m_InsideIntegration == ii_Explorer) && mh_InsideParentRel
		&& (!IsWindow(mh_InsideParentRel) || !IsWindowVisible(mh_InsideParentRel)))
	{
		//Vista: ��������� ��� ����������� ������ �� ������� ������, ��� ����� ����� ��������
		HWND hChild = NULL;
		bool bFound = false;
		while (((hChild = FindWindowEx(mh_InsideParentWND, hChild, NULL, NULL)) != NULL))
		{
			HWND hView = FindWindowEx(hChild, NULL, L"SHELLDLL_DefView", NULL);
			if (hView && IsWindowVisible(hView))
			{
				bFound = true;
				mh_InsideParentRel = hView;
				break;
			}
		}

		if (!bFound)
			return;
	}

	//if ((m_InsideIntegration == ii_Simple) || IsWindow(mh_InsideParentRel))
	{
		RECT rcParent = {}, rcRelative = {};
		GetClientRect(mh_InsideParentWND, &rcParent);
		if (m_InsideIntegration != ii_Simple)
		{
			GetWindowRect(mh_InsideParentRel, &rcRelative);
			MapWindowPoints(NULL, mh_InsideParentWND, (LPPOINT)&rcRelative, 2);
		}

		if (memcmp(&mrc_InsideParent, &rcParent, sizeof(rcParent))
			|| ((m_InsideIntegration != ii_Simple) && memcmp(&mrc_InsideParentRel, &rcRelative, sizeof(rcRelative))))
		{
			// ���������
			RECT rcWnd = GetDefaultRect();
			UpdateIdealRect(rcWnd);
			// ���������
			SetWindowPos(ghWnd, HWND_TOP, rcWnd.left, rcWnd.top, rcWnd.right-rcWnd.left, rcWnd.bottom-rcWnd.top, 0);
			//MoveWindow(ghWnd, mrc_Ideal.left, mrc_Ideal.top, mrc_Ideal.right-mrc_Ideal.left, mrc_Ideal.bottom-mrc_Ideal.top, TRUE);
		}
	}
}

BOOL CConEmuMain::CheckDosBoxExists()
{
	BOOL lbExists = FALSE;
	wchar_t szDosBoxPath[MAX_PATH+32];
	wcscpy_c(szDosBoxPath, ms_ConEmuBaseDir);
	wchar_t* pszName = szDosBoxPath+_tcslen(szDosBoxPath);

	wcscpy_add(pszName, szDosBoxPath, L"\\DosBox\\DosBox.exe");
	if (FileExists(szDosBoxPath))
	{
		wcscpy_add(pszName, szDosBoxPath, L"\\DosBox\\DosBox.conf");
		lbExists = FileExists(szDosBoxPath);
	}

	return lbExists;
}

void CConEmuMain::CheckPortableReg()
{
#ifdef USEPORTABLEREGISTRY
	BOOL lbExists = FALSE;
	wchar_t szPath[MAX_PATH*2], szName[MAX_PATH];
	
	wcscpy_c(szPath, ms_ConEmuBaseDir);
	wcscat_c(szPath, L"\\Portable\\Portable.reg");
	if (_tcslen(szPath) >= countof(ms_PortableReg))
	{
		MBoxAssert(_tcslen(szPath) < countof(ms_PortableReg))
		return; // ���������� �����
	}
	if (FileExists(szPath))
	{
		wcscpy_c(ms_PortableReg, szPath);
		mb_PortableRegExist = TRUE;
	}
	else
	{
		// ��� "Portable.reg" ����������� - �� � hive ���������/��������� �� �����
		return;
	}
	
	wchar_t* pszSID = NULL;
	wcscpy_c(szPath, ms_ConEmuBaseDir);
	wcscat_c(szPath, L"\\Portable\\");
	wcscpy_c(szName, L"Portable.");
	LPCWSTR pszSIDPtr = szName+_tcslen(szName);
	if (GetLogonSID (NULL, &pszSID))
	{
		if (_tcslen(pszSID) > 128)
			pszSID[127] = 0;
		wcscat_c(szName, pszSID);
		LocalFree(pszSID); pszSID = NULL;
	}
	else
	{
		wcscat_c(szName, L"S-Unknown");
	}
	
	if (gOSVer.dwMajorVersion <= 5)
	{
		_wsprintf(ms_PortableMountKey, SKIPLEN(countof(ms_PortableMountKey))
			L"%s.%s.%u", VIRTUAL_REGISTRY_GUID, pszSIDPtr, GetCurrentProcessId());
		mh_PortableMountRoot = HKEY_USERS;
	}
	else
	{
		ms_PortableMountKey[0] = 0;
		mh_PortableMountRoot = NULL;
	}
	
	wcscat_c(szPath, szName);
	wcscpy_c(ms_PortableRegHiveOrg, szPath);

	BOOL lbNeedTemp = FALSE;
	if ((_tcslen(szPath)+_tcslen(szName)) >= countof(ms_PortableRegHive))
		lbNeedTemp = TRUE;
	BOOL lbHiveExist = FileExists(szPath);
	
	// � ������� � removable ������ ����������� �� �����
	if (szPath[0] == L'\\' && szPath[1] == L'\\')
	{
		lbNeedTemp = TRUE;
	}
	else
	{
		wchar_t szDrive[MAX_PATH] = {0}, *pszSlash = wcschr(szPath, L'\\');
		if (pszSlash == NULL)
		{
			_ASSERTE(pszSlash!=NULL);
			lbNeedTemp = TRUE;
		}
		else
		{
			_wcscpyn_c(szDrive, MAX_PATH-1, szPath, (pszSlash-szPath)+1);
			DWORD nType = GetDriveType(szDrive);
			if (!nType || (nType & (DRIVE_CDROM|DRIVE_REMOTE|DRIVE_REMOVABLE)))
				lbNeedTemp = TRUE;
		}
	}

	// ���� � ����� ������ �������� ������ �� ������ - ���� temp
	if (!lbNeedTemp)
	{
		HANDLE hFile = CreateFile(szPath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			lbNeedTemp = TRUE;
		}
		else
		{
			CloseHandle(hFile);
			if (lbHiveExist && !lbNeedTemp)
				wcscpy_c(ms_PortableRegHive, szPath);
		}
	}
	else
	{
		wcscpy_c(ms_PortableRegHive, szPath);
	}
	if (lbNeedTemp)
	{
		wchar_t szTemp[MAX_PATH], szTempFile[MAX_PATH+1];
		int nLen = _tcslen(szName)+16;
		if (!GetTempPath(MAX_PATH-nLen, szTemp))
		{
			mb_PortableRegExist = FALSE;
			DisplayLastError(L"GetTempPath failed");
			return;
		}
		if (!GetTempFileName(szTemp, L"CEH", 0, szTempFile))
		{
			mb_PortableRegExist = FALSE;
			DisplayLastError(L"GetTempFileName failed");
			return;
		}
		// System create temp file, we needs directory
		DeleteFile(szTempFile);
		if (!CreateDirectory(szTempFile, NULL))
		{
			mb_PortableRegExist = FALSE;
			DisplayLastError(L"Create temp hive directory failed");
			return;
		}
		wcscat_c(szTempFile, L"\\");
		wcscpy_c(ms_PortableTempDir, szTempFile);
		wcscat_c(szTempFile, szName);
		if (lbHiveExist)
		{
			if (!CopyFile(szPath, szTempFile, FALSE))
			{
				mb_PortableRegExist = FALSE;
				DisplayLastError(L"Copy temp hive file failed");
				return;
			}
		}
		else
		{
			HANDLE hFile = CreateFile(szTempFile, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				mb_PortableRegExist = FALSE;
				DisplayLastError(L"Create temp hive file failed");
				return;
			}
			else
			{
				CloseHandle(hFile);
			}
		}
		
		wcscpy_c(ms_PortableRegHive, szTempFile);
	}
#endif
}

bool CConEmuMain::PreparePortableReg()
{
#ifdef USEPORTABLEREGISTRY
	if (!mb_PortableRegExist)
		return false;
	if (mh_PortableRoot)
		return true; // ���
	
	bool lbRc = false;
	wchar_t szFullInfo[512];
	//typedef int (WINAPI* MountVirtualHive_t)(LPCWSTR asHive, PHKEY phKey, LPCWSTR asXPMountName, wchar_t* pszErrInfo, int cchErrInfoMax);
	MountVirtualHive_t MountVirtualHive = NULL;
	HMODULE hDll = LoadConEmuCD();
	if (!hDll)
	{
		DWORD dwErrCode = GetLastError();
		_wsprintf(szFullInfo, SKIPLEN(countof(szFullInfo))
			L"Portable registry will be NOT available\n\nLoadLibrary('%s') failed, ErrCode=0x%08X",
			#ifdef _WIN64
				L"ConEmuCD64.dll",
			#else
				L"ConEmuCD.dll",
			#endif
			dwErrCode);
	}
	else
	{
		MountVirtualHive = (MountVirtualHive_t)GetProcAddress(hDll, "MountVirtualHive");
		if (!MountVirtualHive)
		{
			_wsprintf(szFullInfo, SKIPLEN(countof(szFullInfo))
				L"Portable registry will be NOT available\n\nMountVirtualHive not found in '%s'",
				#ifdef _WIN64
					L"ConEmuCD64.dll"
				#else
					L"ConEmuCD.dll"
				#endif
				);
		}
		else
		{
			szFullInfo[0] = 0;
			_ASSERTE(ms_PortableMountKey[0]!=0 || (gOSVer.dwMajorVersion>=6));
			wchar_t szErrInfo[255]; szErrInfo[0] = 0;
			int lRc = MountVirtualHive(ms_PortableRegHive, &mh_PortableRoot,
					ms_PortableMountKey, szErrInfo, countof(szErrInfo), &mb_PortableKeyMounted);
			if (lRc != 0)
			{
				_wsprintf(szFullInfo, SKIPLEN(countof(szFullInfo))
					L"Portable registry will be NOT available\n\n%s\nMountVirtualHive result=%i",
					szErrInfo, lRc);
				ms_PortableMountKey[0] = 0;
			}
			else
			{
				lbRc = true;
			}
		}
	}
	
	if (!lbRc)
	{
		mb_PortableRegExist = FALSE;
	}

	// �������� �������
	OnGlobalSettingsChanged();
		
	if (!lbRc)
	{
		mb_PortableRegExist = FALSE; // ����� �������, ��� �� ��������

		wchar_t szFullMsg[1024];
		wcscpy_c(szFullMsg, szFullInfo);
		wcscat_c(szFullMsg, L"\n\nDo You want to continue without portable registry?");
		int nBtn = MessageBox(NULL, szFullMsg, GetDefaultTitle(),
			MB_ICONSTOP|MB_YESNO|MB_DEFBUTTON2);
		if (nBtn != IDYES)
			return false;
	}
	
	return true;
#else
	_ASSERTE(mb_PortableRegExist==FALSE);
	return true;
#endif
}

void CConEmuMain::FinalizePortableReg()
{
#ifdef USEPORTABLEREGISTRY
	TODO("����������� ���������� ���� - � ����� Portable");
	// Portable.reg �� ������� - ��� ��� �� "������" ��� �����

	// ������� - ������� �����, � �� �� ��������� �������������
	if (mh_PortableRoot)
	{
		RegCloseKey(mh_PortableRoot);
		mh_PortableRoot = NULL;
	}

	if (ms_PortableMountKey[0] && mb_PortableKeyMounted)
	{
		wchar_t szErrInfo[255];
		typedef int (WINAPI* UnMountVirtualHive_t)(LPCWSTR asXPMountName, wchar_t* pszErrInfo, int cchErrInfoMax);
		UnMountVirtualHive_t UnMountVirtualHive = NULL;
		HMODULE hDll = LoadConEmuCD();
		if (!hDll)
		{
			DWORD dwErrCode = GetLastError();
			_wsprintf(szErrInfo, SKIPLEN(countof(szErrInfo))
				L"Can't release portable key: 'HKU\\%s'\n\nLoadLibrary('%s') failed, ErrCode=0x%08X",
					ms_PortableMountKey,
				#ifdef _WIN64
					L"ConEmuCD64.dll",
				#else
					L"ConEmuCD.dll",
				#endif
				dwErrCode);
			MBoxA(szErrInfo);
		}
		else
		{
			UnMountVirtualHive = (UnMountVirtualHive_t)GetProcAddress(hDll, "UnMountVirtualHive");
			if (!UnMountVirtualHive)
			{
				_wsprintf(szErrInfo, SKIPLEN(countof(szErrInfo))
					L"Can't release portable key: 'HKU\\%s'\n\nUnMountVirtualHive not found in '%s'",
						ms_PortableMountKey,
					#ifdef _WIN64
						L"ConEmuCD64.dll"
					#else
						L"ConEmuCD.dll"
					#endif
					);
				MessageBox(NULL, szErrInfo, GetDefaultTitle(), MB_ICONSTOP|MB_SYSTEMMODAL);
			}
			else
			{
				szErrInfo[0] = 0;
				int lRc = UnMountVirtualHive(ms_PortableMountKey, szErrInfo, countof(szErrInfo));
				if (lRc != 0)
				{
					wchar_t szFullInfo[512];
					_wsprintf(szFullInfo, SKIPLEN(countof(szFullInfo))
						L"Can't release portable key: 'HKU\\%s'\n\n%s\nRc=%i",
						ms_PortableMountKey, szErrInfo, lRc);
					ms_PortableMountKey[0] = 0; // �� �������� ��������
					MessageBox(NULL, szFullInfo, GetDefaultTitle(), MB_ICONSTOP|MB_SYSTEMMODAL);
				}
			}
		}
	}
	ms_PortableMountKey[0] = 0; mb_PortableKeyMounted = FALSE; // ��� �������, ��� �� �������� ��������

	if (ms_PortableTempDir[0])
	{
		wchar_t szTemp[MAX_PATH*2], *pszSlash;
		wcscpy_c(szTemp, ms_PortableTempDir);
		pszSlash = szTemp + _tcslen(szTemp) - 1;
		_ASSERTE(*pszSlash == L'\\');
		_wcscpy_c(pszSlash+1, MAX_PATH, _T("*.*"));
		WIN32_FIND_DATA fnd;
		HANDLE hFind = FindFirstFile(szTemp, &fnd);
		if (hFind && hFind != INVALID_HANDLE_VALUE)
		{
			do {
				if (!(fnd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					_wcscpy_c(pszSlash+1, MAX_PATH, fnd.cFileName);
					DeleteFile(szTemp);
				}
			} while (FindNextFile(hFind, &fnd));
			
			FindClose(hFind);
		}
		*pszSlash = 0;
		RemoveDirectory(szTemp);
		ms_PortableTempDir[0] = 0; // ���� ���
	}
#endif
}

void CConEmuMain::GetComSpecCopy(ConEmuComspec& ComSpec)
{
	_ASSERTE(m_GuiInfo.ComSpec.csType==gpSet->ComSpec.csType && m_GuiInfo.ComSpec.csBits==gpSet->ComSpec.csBits);
	ComSpec = m_GuiInfo.ComSpec;
}

// ����� ������� GUI ���������� ����� ������� �������,
// ����� ��� ����� ����� ������, ���� ����� ������������
void CConEmuMain::CreateGuiAttachMapping(DWORD nGuiAppPID)
{
	m_GuiAttachMapping.CloseMap();
	m_GuiAttachMapping.InitName(CEGUIINFOMAPNAME, nGuiAppPID);

	if (m_GuiAttachMapping.Create())
	{
		m_GuiAttachMapping.SetFrom(&m_GuiInfo);
	}
}

void CConEmuMain::UpdateGuiInfoMapping()
{
	m_GuiInfo.nProtocolVersion = CESERVER_REQ_VER;
	m_GuiInfo.hGuiWnd = ghWnd;
	m_GuiInfo.nGuiPID = GetCurrentProcessId();
	
	m_GuiInfo.nLoggingType = (ghOpWnd && gpSetCls->mh_Tabs[gpSetCls->thi_Debug]) ? gpSetCls->m_ActivityLoggingType : glt_None;
	m_GuiInfo.bUseInjects = (gpSet->isUseInjects ? 1 : 0) ; // ((gpSet->isUseInjects == BST_CHECKED) ? 1 : (gpSet->isUseInjects == BST_INDETERMINATE) ? 3 : 0);
	m_GuiInfo.bUseTrueColor = gpSet->isTrueColorer;
	m_GuiInfo.bProcessAnsi = (gpSet->isProcessAnsi ? 1 : 0);
	m_GuiInfo.bUseClink = gpSet->isUseClink(); // ������������ ���������� ��������� ������ (ReadConsole). 0 - ���, 1 - ������ ������ (0.1.1), 2 - ����� ������

	m_GuiInfo.bSleepInBackg = gpSet->isSleepInBackground;
	m_GuiInfo.bGuiActive = isMeForeground(true, true);
	{
	CVConGuard VCon;
	m_GuiInfo.hActiveCon = (GetActiveVCon(&VCon) >= 0) ? VCon->RCon()->ConWnd() : NULL;
	}
	m_GuiInfo.dwActiveTick = GetTickCount();

	mb_DosBoxExists = CheckDosBoxExists();
	m_GuiInfo.bDosBox = mb_DosBoxExists;
	
	m_GuiInfo.isHookRegistry = (mb_PortableRegExist ? (gpSet->isPortableReg ? 3 : 1) : 0);
	wcscpy_c(m_GuiInfo.sHiveFileName, ms_PortableRegHive);
	m_GuiInfo.hMountRoot = mh_PortableMountRoot;
	wcscpy_c(m_GuiInfo.sMountKey, ms_PortableMountKey);
	
	wcscpy_c(m_GuiInfo.sConEmuExe, ms_ConEmuExe);
	wcscpy_c(m_GuiInfo.sConEmuDir, ms_ConEmuExeDir);
	wcscpy_c(m_GuiInfo.sConEmuBaseDir, ms_ConEmuBaseDir);
	_wcscpyn_c(m_GuiInfo.sConEmuArgs, countof(m_GuiInfo.sConEmuArgs), mpsz_ConEmuArgs ? mpsz_ConEmuArgs : L"", countof(m_GuiInfo.sConEmuArgs));

	// *********************
	// *** ComSpec begin ***
	// *********************
	{
		// ������� - ���������� ��� ��� �������� (����� tcc � �.�.)
		SetEnvVarExpanded(L"ComSpec", ms_ComSpecInitial); // �.�. ������� ����� ��������������� �� ���������
		FindComspec(&gpSet->ComSpec);
		//wcscpy_c(gpSet->ComSpec.ConEmuDir, gpConEmu->ms_ConEmuDir);
		wcscpy_c(gpSet->ComSpec.ConEmuBaseDir, gpConEmu->ms_ConEmuBaseDir);
		UpdateComspec(&gpSet->ComSpec); // ��������� ���������� ���������, ���� ������� (isAddConEmu2Path)

		// ��������� ��� ��������� �������, ����� ����� "�������" �� ��� �����
		m_GuiInfo.ComSpec = gpSet->ComSpec;

		// ������ ��������� � �������, ��� �������������� ������ ���������
		ComSpecType csType = gpSet->ComSpec.csType;
		ComSpecBits csBits = gpSet->ComSpec.csBits;
		//m_GuiInfo.ComSpec.csType = gpSet->ComSpec.csType;
		//m_GuiInfo.ComSpec.csBits = gpSet->ComSpec.csBits;
		wchar_t szExpand[MAX_PATH] = {}, szFull[MAX_PATH] = {}, *pszFile;
		if (csType == cst_Explicit)
		{
			DWORD nLen;
			// Expand env.vars
			if (wcschr(gpSet->ComSpec.ComspecExplicit, L'%'))
			{
				nLen = ExpandEnvironmentStrings(gpSet->ComSpec.ComspecExplicit, szExpand, countof(szExpand));
				if (!nLen || (nLen>=countof(szExpand)))
				{
					MBoxAssert((nLen>0) && (nLen<countof(szExpand)) && "ExpandEnvironmentStrings(ComSpec)");
					szExpand[0] = 0;
				}
			}
			else
			{
				wcscpy_c(szExpand, gpSet->ComSpec.ComspecExplicit);
				nLen = lstrlen(szExpand);
			}
			// Expand relative paths. Note. Path MUST be specified with root
			// NOT recommended: "..\tcc\tcc.exe" this may lead to unpredictable results cause of CurrentDirectory
			// Allowed: "%ConEmuDir%\..\tcc\tcc.exe"
			if (*szExpand)
			{
				nLen = GetFullPathName(szExpand, countof(szFull), szFull, &pszFile);
				if (!nLen || (nLen>=countof(szExpand)))
				{
					MBoxAssert((nLen>0) && (nLen<countof(szExpand)) && "GetFullPathName(ComSpec)");
					szFull[0] = 0;
				}
				else if (!FileExists(szExpand))
				{
					szFull[0] = 0;
				}
			}
			else
			{
				szFull[0] = 0;
			}
			wcscpy_c(m_GuiInfo.ComSpec.ComspecExplicit, szFull);
			if (!*szFull && (csType == cst_Explicit))
			{
				csType = cst_AutoTccCmd;
			}
		}
		else
		{
			m_GuiInfo.ComSpec.ComspecExplicit[0] = 0; // ��������� �� ���������� ������
		}

		//
		if (csType == cst_Explicit)
		{
			_ASSERTE(*szFull);
			wcscpy_c(m_GuiInfo.ComSpec.Comspec32, szFull);
			wcscpy_c(m_GuiInfo.ComSpec.Comspec64, szFull);
		}
		else
		{
			wcscpy_c(m_GuiInfo.ComSpec.Comspec32, gpSet->ComSpec.Comspec32);
			wcscpy_c(m_GuiInfo.ComSpec.Comspec64, gpSet->ComSpec.Comspec64);
		}
		//wcscpy_c(m_GuiInfo.ComSpec.ComspecInitial, gpConEmu->ms_ComSpecInitial);

		// finalization
		m_GuiInfo.ComSpec.csType = csType;
		m_GuiInfo.ComSpec.csBits = csBits;
		//m_GuiInfo.ComSpec.isUpdateEnv = gpSet->ComSpec.isUpdateEnv;
		//m_GuiInfo.ComSpec.isAddConEmu2Path = gpSet->ComSpec.isAddConEmu2Path;
	}
	// *******************
	// *** ComSpec end ***
	// *******************

	
	FillConEmuMainFont(&m_GuiInfo.MainFont);
	
	TODO("DosBox");
	//BOOL     bDosBox; // ������� DosBox
	//wchar_t  sDosBoxExe[MAX_PATH+1]; // ������ ���� � DosBox.exe
	//wchar_t  sDosBoxEnv[8192]; // ������� �������� (mount, � ��.)

	//if (mb_CreateProcessLogged)
	//	lstrcpy(m_GuiInfo.sLogCreateProcess, ms_LogCreateProcess);
	// sConEmuArgs ��� �������� � PrepareCommandLine
	
	BOOL lbMapIsValid = m_GuiInfoMapping.IsValid();
	
	if (!lbMapIsValid)
	{
		m_GuiInfoMapping.InitName(CEGUIINFOMAPNAME, GetCurrentProcessId());

		if (m_GuiInfoMapping.Create())
			lbMapIsValid = TRUE;
	}
	
	if (lbMapIsValid)
	{
		m_GuiInfoMapping.SetFrom(&m_GuiInfo);
	}
	#ifdef _DEBUG
	else
	{
		_ASSERTE(lbMapIsValid);
	}
	#endif
	
}

void CConEmuMain::UpdateGuiInfoMappingActive(bool bActive)
{
	CVConGuard VCon;
	HWND hActiveRCon = (GetActiveVCon(&VCon) >= 0) ? VCon->RCon()->ConWnd() : NULL;

	m_GuiInfo.bSleepInBackg = gpSet->isSleepInBackground;
	m_GuiInfo.bGuiActive = bActive;
	m_GuiInfo.hActiveCon = hActiveRCon;
	m_GuiInfo.dwActiveTick = GetTickCount();

	ConEmuGuiMapping* pData = m_GuiInfoMapping.Ptr();

	if (pData)
	{
		if ((pData->bSleepInBackg != (BOOL)gpSet->isSleepInBackground)
			|| (pData->bGuiActive != (BOOL)bActive)
			|| (pData->hActiveCon != hActiveRCon))
		{
			pData->bSleepInBackg = gpSet->isSleepInBackground;
			pData->bGuiActive = bActive;
			pData->hActiveCon = hActiveRCon;
			pData->dwActiveTick = m_GuiInfo.dwActiveTick;
		}
	}
}

HRGN CConEmuMain::CreateWindowRgn(bool abTestOnly/*=false*/)
{
	HRGN hRgn = NULL, hExclusion = NULL;

	TODO("DoubleView: ���� ������ ��������� �������� - ����� ���������� �������, ��� ������ ��������, ��� ��������");

	hExclusion = CVConGroup::GetExclusionRgn(abTestOnly);

	if (abTestOnly && hExclusion)
	{
		_ASSERTE(hExclusion == (HRGN)1);
		return (HRGN)1;
	}

	WARNING("��������� ������ �� NULL ������� ������� ���� ��� ��������� ������ � ���������");

	if ((WindowMode == wmFullScreen)
		// ������� ������ ����� (��� isZoomed) - ����� ������ �������� �� ���� �������
		|| ((WindowMode == wmMaximized) && (gpSet->isHideCaption || gpSet->isHideCaptionAlways())))
	{
		if (abTestOnly)
			return (HRGN)1;

		ConEmuRect tFrom = (WindowMode == wmFullScreen) ? CER_FULLSCREEN : CER_MAXIMIZED;
		RECT rcScreen; // = CalcRect(tFrom, MakeRect(0,0), tFrom);
		RECT rcFrame = CalcMargins(CEM_FRAMECAPTION);

		HMONITOR hMon = MonitorFromWindow(ghWnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = {sizeof(mi)};
		if (GetMonitorInfo(hMon, &mi))
			rcScreen = (WindowMode == wmFullScreen) ? mi.rcMonitor : mi.rcWork;
		else
			rcScreen = CalcRect(tFrom, MakeRect(0,0), tFrom);

		// rcFrame, �.�. ������ �������� ������������ �������� ������ ���� ����
		hRgn = CreateWindowRgn(abTestOnly, false, rcFrame.left, rcFrame.top, rcScreen.right-rcScreen.left, rcScreen.bottom-rcScreen.top);
	}
	else if (WindowMode == wmMaximized)
	{
		if (!hExclusion)
		{
			// ���� ���������� �������� � ������� ��� - ������ �� ������
		}
		else
		{
			// � FullScreen �� ���������. ����� � ������ ���������
			// ���� �� �������� ������ ����� ��������� �� ����������
			RECT rcScreen = CalcRect(CER_FULLSCREEN, MakeRect(0,0), CER_FULLSCREEN);
			int nCX = GetSystemMetrics(SM_CXSIZEFRAME);
			int nCY = GetSystemMetrics(SM_CYSIZEFRAME);
			hRgn = CreateWindowRgn(abTestOnly, false, nCX, nCY, rcScreen.right-rcScreen.left, rcScreen.bottom-rcScreen.top);
		}
	}
	else
	{
		// Normal
		if (gpSet->isCaptionHidden())
		{
			if ((mn_QuakePercent != 0) || !isMouseOverFrame())
			{
				// ����� �������� (����� �� ��� ������ ��� ����������)
				RECT rcClient = GetGuiClientRect();
				RECT rcFrame = CalcMargins(CEM_FRAMECAPTION);
				_ASSERTE(!rcClient.left && !rcClient.top);

				bool bRoundTitle = gpSetCls->CheckTheming() && mp_TabBar->IsTabsShown();

				if (gpSet->isQuakeStyle)
				{
					if (mn_QuakePercent > 0)
					{
						int nPercent = (mn_QuakePercent > 100) ? 100 : (mn_QuakePercent == 1) ? 0 : mn_QuakePercent;
						int nQuakeHeight = (rcClient.bottom - rcClient.top + 1) * nPercent / 100;
						if (nQuakeHeight < 1)
						{
							nQuakeHeight = 1; // ����� ������ �� ����������
							rcClient.right = rcClient.left + 1;
						}
						rcClient.bottom = min(rcClient.bottom, (rcClient.top+nQuakeHeight));
					}
					bRoundTitle = false;
				}

				int nFrame = gpSet->HideCaptionAlwaysFrame();
				if (gpSet->isQuakeStyle && mn_QuakePercent && (nFrame < 0))
				{
					nFrame = GetSystemMetrics(SM_CXSIZEFRAME);
				}

				hRgn = (nFrame >= 0) ? CreateWindowRgn(abTestOnly, bRoundTitle,
				                       rcFrame.left-nFrame,
				                       rcFrame.top-nFrame,
				                       rcClient.right+2*nFrame,
									   rcClient.bottom+2*nFrame) : NULL;
			}
		}

		if (!hRgn && hExclusion)
		{
			// ���� ����� �������...
			bool bTheming = gpSetCls->CheckTheming();
			RECT rcWnd; GetWindowRect(ghWnd, &rcWnd);
			hRgn = CreateWindowRgn(abTestOnly, bTheming,
			                       0,0, rcWnd.right-rcWnd.left, rcWnd.bottom-rcWnd.top);
		}
	}

	return hRgn;
}

HRGN CConEmuMain::CreateWindowRgn(bool abTestOnly/*=false*/,bool abRoundTitle/*=false*/,int anX, int anY, int anWndWidth, int anWndHeight)
{
	HRGN hRgn = NULL, hExclusion = NULL, hCombine = NULL;

	TODO("DoubleView: ���� ������ ��������� �������� - ����� ���������� �������, ��� ������ ��������, ��� ��������");

	CVConGuard VCon;
	if (CVConGroup::GetActiveVCon(&VCon) < 0)
	{
		// ��������� �� ������
		//_ASSERTE(FALSE && "No Active VCon");
		return NULL;
	}

	hExclusion = CVConGroup::GetExclusionRgn(abTestOnly);

	if (abTestOnly && hExclusion)
	{
		_ASSERTE(hExclusion == (HRGN)1);
		return (HRGN)1;
	}

	if (abRoundTitle && anX>0 && anY>0)
	{
		int nPoint = 0;
		POINT ptPoly[20];
		ptPoly[nPoint++] = MakePoint(anX, anY+anWndHeight);
		ptPoly[nPoint++] = MakePoint(anX, anY+5);
		ptPoly[nPoint++] = MakePoint(anX+1, anY+3);
		ptPoly[nPoint++] = MakePoint(anX+3, anY+1);
		ptPoly[nPoint++] = MakePoint(anX+5, anY);
		ptPoly[nPoint++] = MakePoint(anX+anWndWidth-5, anY);
		ptPoly[nPoint++] = MakePoint(anX+anWndWidth-3, anY+1);
		ptPoly[nPoint++] = MakePoint(anX+anWndWidth-1, anY+4); //-V112
		ptPoly[nPoint++] = MakePoint(anX+anWndWidth, anY+6);
		ptPoly[nPoint++] = MakePoint(anX+anWndWidth, anY+anWndHeight);
		hRgn = CreatePolygonRgn(ptPoly, nPoint, WINDING);
	}
	else
	{
		hRgn = CreateRectRgn(anX, anY, anX+anWndWidth, anY+anWndHeight);
	}

	if (abTestOnly && (hRgn || hExclusion))
		return (HRGN)1;

	// �������� CombineRgn, OffsetRgn (��� ����������� ������� ��������� � ������������ ����)
	if (hExclusion)
	{
		if (hRgn)
		{
			//_ASSERTE(hRgn != NULL);
			//DeleteObject(hExclusion);
			//} else {
			//POINT ptShift = {0,0};
			//MapWindowPoints(ghWnd DC, NULL, &ptShift, 1);
			//RECT rcWnd = GetWindow
			RECT rcFrame = CalcMargins(CEM_FRAMECAPTION);

			#ifdef _DEBUG
			// CEM_TAB �� ��������� ������������� ���������� ����� � ����������� �������
			RECT rcTab = CalcMargins(CEM_TAB);
			#endif

			POINT ptClient = {0,0};
			MapWindowPoints(VCon->GetView(), ghWnd, &ptClient, 1);

			HRGN hOffset = CreateRectRgn(0,0,0,0);
			int n1 = CombineRgn(hOffset, hExclusion, NULL, RGN_COPY); UNREFERENCED_PARAMETER(n1);
			int n2 = OffsetRgn(hOffset, rcFrame.left+ptClient.x, rcFrame.top+ptClient.y); UNREFERENCED_PARAMETER(n2);
			hCombine = CreateRectRgn(0,0,1,1);
			CombineRgn(hCombine, hRgn, hOffset, RGN_DIFF);
			DeleteObject(hRgn);
			DeleteObject(hOffset); hOffset = NULL;
			hRgn = hCombine; hCombine = NULL;
		}
	}

	return hRgn;
}

void CConEmuMain::Destroy()
{
	CVConGroup::StopSignalAll();

	if (gbInDisplayLastError)
		return; // ����� �� ������������ ���� � ����������� �� �������

	#ifdef _DEBUG
	if (gbInMyAssertTrap)
		return;
	#endif

	if (ghWnd)
	{
		//HWND hWnd = ghWnd;
		//ghWnd = NULL;
		//DestroyWindow(hWnd); -- ����� ���� �������� �� ������ ����
		PostMessage(ghWnd, mn_MsgMyDestroy, GetCurrentThreadId(), 0);
	}
}

CConEmuMain::~CConEmuMain()
{
	_ASSERTE(ghWnd==NULL || !IsWindow(ghWnd));
	//ghWnd = NULL;

	SafeDelete(mp_AttachDlg);
	SafeDelete(mp_RecreateDlg);

	CVConGroup::DestroyAllVCon();

	CVirtualConsole::ClearPartBrushes();

	#ifndef _WIN64
	if (mh_WinHook)
	{
		UnhookWinEvent(mh_WinHook);
		mh_WinHook = NULL;
	}
	#endif

	//if (mh_PopupHook) {
	//	UnhookWinEvent(mh_PopupHook);
	//	mh_PopupHook = NULL;
	//}

	SafeDelete(mp_DragDrop);

	//if (ProgressBars) {
	//    delete ProgressBars;
	//    ProgressBars = NULL;
	//}

	SafeDelete(mp_TabBar);

	SafeDelete(mp_Tip);

	SafeDelete(mp_Status);

	//if (m_Child)
	//{
	//	delete m_Child;
	//	m_Child = NULL;
	//}

	//if (m_Back)
	//{
	//	delete m_Back;
	//	m_Back = NULL;
	//}

	//if (m_Macro)
	//{
	//	delete m_Macro;
	//	m_Macro = NULL;
	//}

	if (mh_DebugPopup)
	{
		DestroyMenu(mh_DebugPopup);
		mh_DebugPopup = NULL;
	}
	
	if (mh_EditPopup)
	{
		DestroyMenu(mh_EditPopup);
		mh_EditPopup = NULL;
	}
	
	if (mh_TerminateVConPopup)
	{
		DestroyMenu(mh_TerminateVConPopup);
		mh_TerminateVConPopup = NULL;
	}
	
	if (mh_ActiveVConPopup)
	{
		DestroyMenu(mh_ActiveVConPopup);
		mh_ActiveVConPopup = NULL;
	}
	
	//if (mh_RecreatePasswFont)
	//{
	//	DeleteObject(mh_RecreatePasswFont);
	//	mh_RecreatePasswFont = NULL;
	//}

	m_GuiServer.Stop(true);

	CVConGroup::Deinitialize();

	DeleteCriticalSection(&mcs_ShellExecuteEx);
	
	// �� ����, ��� ������ ��� ���� ������ � OnDestroy
	FinalizePortableReg();

	LoadImageFinalize();

	SafeDelete(mp_Log);

	_ASSERTE(mh_LLKeyHookDll==NULL);
	CommonShutdown();

	gpConEmu = NULL;
}




/* ****************************************************** */
/*                                                        */
/*                  Sizing methods                        */
/*                                                        */
/* ****************************************************** */

#ifdef colorer_func
{
	Sizing_Methods() {};
}
#endif

/*!!!static!!*/
void CConEmuMain::AddMargins(RECT& rc, RECT& rcAddShift, BOOL abExpand/*=FALSE*/)
{
	if (!abExpand)
	{
		// ��������� ��� ����� �� rcAddShift (left ���������� �� left, � �.�.)
		if (rcAddShift.left)
			rc.left += rcAddShift.left;

		if (rcAddShift.top)
			rc.top += rcAddShift.top;

		if (rcAddShift.right)
			rc.right -= rcAddShift.right;

		if (rcAddShift.bottom)
			rc.bottom -= rcAddShift.bottom;
	}
	else
	{
		// ��������� ������ ������ � ������ �����
		if (rcAddShift.right || rcAddShift.left)
			rc.right += rcAddShift.right + rcAddShift.left;

		if (rcAddShift.bottom || rcAddShift.top)
			rc.bottom += rcAddShift.bottom + rcAddShift.top;
	}
}

void CConEmuMain::AskChangeBufferHeight()
{
	CVConGuard VCon;
	if (GetActiveVCon(&VCon) < 0)
		return;
	CVirtualConsole *pVCon = VCon.VCon();
	CRealConsole *pRCon = pVCon->RCon();
	if (!pRCon) return;


	HWND hGuiClient = pRCon->GuiWnd();
	if (hGuiClient)
	{
		pRCon->ShowGuiClientInt(!pRCon->isGuiVisible());
		return;
	}


	// Win7 BUGBUG: Issue 192: ������� Conhost ��� turn bufferheight ON
	// http://code.google.com/p/conemu-maximus5/issues/detail?id=192
	if (gOSVer.dwMajorVersion == 6 && gOSVer.dwMinorVersion == 1)
		return;


	BOOL lbBufferHeight = pRCon->isBufferHeight();

	int nBtn = 0;

	{
		DontEnable de;
		nBtn = MessageBox(ghWnd, lbBufferHeight ?
						  L"Do You want to turn bufferheight OFF?" :
						  L"Do You want to turn bufferheight ON?",
						  GetDefaultTitle(), MB_ICONQUESTION|MB_OKCANCEL);
	}

	if (nBtn != IDOK) return;

	#ifdef _DEBUG
	HANDLE hFarInExecuteEvent = NULL;

	if (!lbBufferHeight)
	{
		DWORD dwFarPID = pRCon->GetFarPID();

		if (dwFarPID)
		{
			// ��� ������� ��������� � ���������� (���� ������������) ������ ����
			wchar_t szEvtName[64]; _wsprintf(szEvtName, SKIPLEN(countof(szEvtName)) L"FARconEXEC:%08X", (DWORD)pRCon->ConWnd());
			hFarInExecuteEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, szEvtName);

			if (hFarInExecuteEvent)  // ����� ConEmuC ������� _ASSERTE � ���������� ������
				SetEvent(hFarInExecuteEvent);
		}
	}
	#endif

	
	pRCon->ChangeBufferHeightMode(!lbBufferHeight);


	#ifdef _DEBUG
	if (hFarInExecuteEvent)
		ResetEvent(hFarInExecuteEvent);
	#endif

	// �������� �� ������� ������� Scrolling(BufferHeight) & Alternative
	pRCon->OnBufferHeight();
}

void CConEmuMain::AskChangeAlternative()
{
	CVConGuard VCon;
	if (GetActiveVCon(&VCon) < 0)
		return;
	CVirtualConsole *pVCon = VCon.VCon();
	CRealConsole *pRCon = pVCon->RCon();
	if (!pRCon) return;


	HWND hGuiClient = pRCon->GuiWnd();
	if (hGuiClient)
	{
		pRCon->ShowGuiClientInt(!pRCon->isGuiVisible());
		return;
	}


	// ������������� �� ��������������/�������� �����
	RealBufferType CurBuffer = pRCon->GetActiveBufferType();
	if (CurBuffer == rbt_Primary)
	{
		pRCon->SetActiveBuffer(rbt_Alternative);
	}
	else
	{
		pRCon->SetActiveBuffer(rbt_Primary);
	}

	// �������� �� ������� ������� Scrolling(BufferHeight) & Alternative
	pRCon->OnBufferHeight();
}

/*!!!static!!*/
// ������� ����������� �������� (�������������)
// mg �������� �������, �������� (CEM_FRAMECAPTION|CEM_TAB|CEM_CLIENT)
RECT CConEmuMain::CalcMargins(DWORD/*enum ConEmuMargins*/ mg, ConEmuWindowMode wmNewMode /*= wmCurrent*/)
{
	_ASSERTE(this!=NULL);

	// -- ������� ������ �������� ������ � ������� �����
	// -- VCon ��� ������ �� ������
	//if (!apVCon)
	//	apVCon = gpConEmu->mp_ VActive;

	WARNING("CEM_SCROLL ������ �������?");
	WARNING("���������, ����� DC ��������� �������������� ����� �������� CEM_BACK");

	RECT rc = {};

	//if ((wmNewMode == wmCurrent) || (wmNewMode == wmNotChanging))
	//{
	//	wmNewMode = gpConEmu->WindowMode;
	//}

	// ������� ����� �������� ����� ���� � ���������� ������� ���� (����� + ���������)
	if ((mg & ((DWORD)CEM_FRAMECAPTION)) && !m_InsideIntegration)
	{
		// ������ CEM_CAPTION ������� ������
		_ASSERTE((mg & ((DWORD)CEM_FRAMECAPTION)) != CEM_CAPTION);

		// �.�. ��� ������ ��������� - ����� ������� rc ������� ��������������
		_ASSERTE(rc.left==0 && rc.top==0 && rc.right==0 && rc.bottom==0);
		DWORD dwStyle = GetWindowStyle();
		if (wmNewMode != wmCurrent)
			dwStyle = FixWindowStyle(dwStyle, wmNewMode);
		DWORD dwStyleEx = GetWindowStyleEx();
		//static DWORD dwLastStyle, dwLastStyleEx;
		//static RECT rcLastRect;
		//if (dwLastStyle == dwStyle && dwLastStyleEx == dwStyleEx)
		//{
		//	rc = rcLastRect; // ����� �� ������� ������ �������
		//}
		//else
		//{
		const int nTestWidth = 100, nTestHeight = 100;
		RECT rcTest = MakeRect(nTestWidth,nTestHeight);

		bool bHideCaption = gpSet->isCaptionHidden();

		// AdjustWindowRectEx �� ������ ���������� � FullScreen. ������. � ����� � ���������� ���, ��������� ������.
		if (WindowMode == wmFullScreen)
		{
			// ��� FullScreen ��������� ��������� ����� � ���������
			_ASSERTE(rc.left==0 && rc.top==0 && rc.right==0 && rc.bottom==0);
		}
		else if (AdjustWindowRectEx(&rcTest, dwStyle, FALSE, dwStyleEx))
		{
			rc.left = -rcTest.left;
			rc.right = rcTest.right - nTestWidth;
			rc.bottom = rcTest.bottom - nTestHeight;
			if ((mg & ((DWORD)CEM_CAPTION)) && !bHideCaption)
				rc.top = -rcTest.top;
			else
				rc.top = rc.bottom;
			_ASSERTE(rc.top >= 0 && rc.left >= 0 && rc.right >= 0 && rc.bottom >= 0);
		}
		else
		{
			_ASSERTE(FALSE);
			rc.left = rc.right = GetSystemMetrics(SM_CXSIZEFRAME);
			rc.bottom = GetSystemMetrics(SM_CYSIZEFRAME);
			rc.top = rc.bottom; // �����
			if ((mg & ((DWORD)CEM_CAPTION)) && !bHideCaption) // ���� ���� ��������� - ������� � ���
				rc.top += GetSystemMetrics(SM_CYCAPTION);
		}
	}

	// ������� �������� ���� (�� ���� �������� ������ rc.top)
	if (mg & ((DWORD)CEM_TAB))
	{
		if (ghWnd)
		{
			bool lbTabActive = ((mg & CEM_TAB_MASK) == CEM_TAB) ? gpConEmu->mp_TabBar->IsTabsActive()
			                   : ((mg & ((DWORD)CEM_TABACTIVATE)) == ((DWORD)CEM_TABACTIVATE));

			// ������� ���� ��� �������, ������� ���� ����������
			if (lbTabActive)  //TODO: + IsAllowed()?
			{
				RECT rcTab = gpConEmu->mp_TabBar->GetMargins();
				// !!! AddMargins ������������ ������! �� �������� �� ���������
				//AddMargins(rc, rcTab, FALSE);
				_ASSERTE(rcTab.top==0 || rcTab.bottom==0);
				rc.top += rcTab.top;
				rc.bottom += rcTab.bottom;
			}// else { -- ��� ���� ��� - ������ �������������� ������� �� �����

			//    rc = MakeRect(0,0); // ��� ���� ��� - ������ �������������� ������� �� �����
			//}
		}
		else
		{
			// ����� ����� �������� �� ����������
			if (gpSet->isTabs == 1)
			{
				//RECT rcTab = gpSet->rcTabMargins; // ������������� ������� ����
				RECT rcTab = mp_TabBar->GetMargins();
				// ����� ��� - ���� �� ������. ���� ������, ���� �����.
				_ASSERTE(rcTab.top==0 || rcTab.bottom==0);

				//if (!gpSet->isTabFrame)
				//{

				// �� ���� �������� ������ ��������� (��������)
				//rc.left=0; rc.right=0; rc.bottom=0;
				if (gpSet->nTabsLocation == 1)
				{
					rc.bottom += rcTab.top ? rcTab.top : rcTab.bottom;
				}
				else
				{
					rc.top += rcTab.top ? rcTab.top : rcTab.bottom;
				}

				//}
				//else
				//{
				//	_ASSERTE(FALSE && "For deletion!");
				//	// !!! AddMargins ������������ ������! �� �������� �� ���������
				//	//AddMargins(rc, rcTab, FALSE);
				//	rc.top += rcTab.top;
				//	rc.bottom += rcTab.bottom;
				//}
			}// else { -- ��� ���� ��� - ������ �������������� ������� �� �����

			//    rc = MakeRect(0,0); // ��� ���� ��� - ������ �������������� ������� �� �����
			//}
		}
	}

	//    //case CEM_BACK:  // ������� �� ����� ���� ���� (� ����������) �� ���� � ���������� (DC)
	//    //{
	//    //    if (apVCon && apVCon->RCon()->isBufferHeight()) { //TODO: � ������������ �� ������ ���������?
	//    //        rc = MakeRect(0,0,GetSystemMetrics(SM_CXVSCROLL),0);
	//    //    } else {
	//    //        rc = MakeRect(0,0); // ��� ��������� ��� - ������ �������������� ������� �� �����
	//    //    }
	//    //}   break;
	//default:
	//    _ASSERTE(mg==CEM_FRAME || mg==CEM_TAB);
	//}

	//if (mg & ((DWORD)CEM_CLIENT))
	//{
	//	TODO("���������� �� ������ ������, ���������� ��� DoubleView. ������ �������� �� apVCon");
	//	RECT rcDC; GetWindowRect(ghWnd DC, &rcDC);
	//	RECT rcWnd; GetWindowRect(ghWnd, &rcWnd);
	//	RECT rcFrameTab = CalcMargins(CEM_FRAMETAB);
	//	// ������ ���������
	//	rc.left += (rcDC.left - rcWnd.left - rcFrameTab.left);
	//	rc.top += (rcDC.top - rcWnd.top - rcFrameTab.top);
	//	rc.right -= (rcDC.right - rcWnd.right - rcFrameTab.right);
	//	rc.bottom -= (rcDC.bottom - rcWnd.bottom - rcFrameTab.bottom);
	//}

	if ((mg & ((DWORD)CEM_PAD)) && gpSet->isTryToCenter)
	{
		rc.left += gpSet->nCenterConsolePad;
		rc.top += gpSet->nCenterConsolePad;
		rc.right += gpSet->nCenterConsolePad;
		rc.bottom += gpSet->nCenterConsolePad;
	}

	if ((mg & ((DWORD)CEM_SCROLL)) && (gpSet->isAlwaysShowScrollbar == 1))
	{
		rc.right += GetSystemMetrics(SM_CXVSCROLL);
	}

	if ((mg & ((DWORD)CEM_STATUS)) && gpSet->isStatusBarShow)
	{
		TODO("����������� �� ����/������");
		rc.bottom += gpSet->StatusBarHeight();
	}

	return rc;
}

RECT CConEmuMain::CalcRect(enum ConEmuRect tWhat, CVirtualConsole* pVCon/*=NULL*/)
{
	_ASSERTE(ghWnd!=NULL);
	RECT rcMain = {};
	if (isIconic() || isRestoreFromMinimized)
	{
		if (gpSet->isQuakeStyle)
		{
			rcMain = GetDefaultRect();
		}
		else
		{
			WINDOWPLACEMENT wpl = {sizeof(wpl)};
			GetWindowPlacement(ghWnd, &wpl);
			
			rcMain = wpl.rcNormalPosition;

			// ���� ���� ���� �������� �� Maximized/FullScreen ���������
			if ((WindowMode == wmMaximized) || (WindowMode == wmFullScreen))
			{
				ConEmuRect t = (WindowMode == wmMaximized) ? CER_MAXIMIZED : CER_FULLSCREEN;
				rcMain = CalcRect(t, rcMain, t);
			}
		}
	}
	else
	{
		GetWindowRect(ghWnd, &rcMain);
	}

	if (tWhat == CER_MAIN)
	{
		return rcMain;
	}

	return CalcRect(tWhat, rcMain, CER_MAIN, pVCon);
}

// Return true, when rect was changed
bool CConEmuMain::FixWindowRect(RECT& rcWnd, DWORD nBorders /* enum of ConEmuBorders */)
{
	RECT rcStore = rcWnd;
	RECT rcWork;

	// When we live inside parent window - size must be strict
	if (m_InsideIntegration)
	{
		rcWork = GetDefaultRect();
		bool bChanged = (memcmp(&rcWork, &rcWnd, sizeof(rcWnd)) != 0);
		rcWnd = rcWork;
		return bChanged;
	}

	ConEmuWindowMode wndMode = GetWindowMode();

	if (wndMode == wmMaximized || wndMode == wmFullScreen)
	{
		nBorders |= CEB_TOP|CEB_LEFT|CEB_RIGHT|CEB_BOTTOM;
	}


	RECT rcFrame = CalcMargins(CEM_FRAMEONLY);
	

	// We prefer monitor, nearest to upper-left part of window
	//--RECT rcFind = {rcStore.left, rcStore.right, rcStore.left+(rcStore.right-rcStore.left)/3, rcStore.top+(rcStore.bottom-rcStore.top)/3};
	RECT rcFind = {rcStore.left+(rcFrame.left+1)*2, rcStore.top+(rcFrame.top+1)*2, rcStore.left+(rcFrame.left+1)*3, rcStore.top+(rcFrame.top+1)*3};
	MONITORINFO mi; GetNearestMonitor(&mi, &rcFind);
	// Get work area (may be FullScreen)
	rcWork = (wndMode == wmFullScreen) ? mi.rcMonitor : mi.rcWork;


	RECT rcInter = rcWnd;
	BOOL bIn = IntersectRect(&rcInter, &rcWork, &rcStore);

	if (bIn && (EqualRect(&rcInter, &rcStore)) || (nBorders & CEB_ALLOW_PARTIAL))
	{
		// Nothing must be changed
		return false;
	}

	if (nBorders & CEB_ALLOW_PARTIAL)
	{
		RECT rcFind2 = {rcStore.right-rcFrame.left-1, rcStore.bottom-rcFrame.top-1, rcStore.right, rcStore.bottom};
		MONITORINFO mi2; GetNearestMonitor(&mi2, &rcFind2);
		RECT rcInter2, rcInter3;
		if (IntersectRect(&rcInter2, &mi2.rcMonitor, &rcStore) || IntersectRect(&rcInter3, &mi.rcMonitor, &rcStore))
		{
			// It's partially visible, OK
			return false;
		}
	}

	// Calculate border sizes (even though one of left/top/right/bottom requested)
	RECT rcMargins = (nBorders & CEB_ALL) ? rcFrame : MakeRect(0,0);
	if (!(nBorders & CEB_LEFT))
		rcMargins.left = 0;
	if (!(nBorders & CEB_TOP))
		rcMargins.top = 0;
	if (!(nBorders & CEB_RIGHT))
		rcMargins.right = 0;
	if (!(nBorders & CEB_BOTTOM))
		rcMargins.bottom = 0;

	if (!IsRectEmpty(&rcMargins))
	{
		// May be work rect will cover our window without margins?
		RECT rcTest = rcWork;
		AddMargins(rcTest, rcMargins, TRUE);

		BOOL bIn2 = IntersectRect(&rcInter, &rcTest, &rcStore);
		if (bIn2 && EqualRect(&rcInter, &rcStore))
		{
			// Ok, it covers, nothing must be changed
			return false;
		}
	}

	bool bChanged = false;

	// We come here when at least part of ConEmu is "Out-of-screen"
	// "bIn == false" when ConEmu is totally "Out-of-screen"
	if (!bIn || !(nBorders & CEB_ALLOW_PARTIAL))
	{
		int nWidth = rcStore.right-rcStore.left;
		int nHeight = rcStore.bottom-rcStore.top;

		// All is bad. Windows is totally out of screen.
		rcWnd.left = max(rcWork.left-rcMargins.left,min(rcStore.left,rcWork.right-nWidth+rcMargins.right));
		rcWnd.top = max(rcWork.top-rcMargins.top,min(rcStore.top,rcWork.bottom-nHeight+rcMargins.bottom));

		// Add Width/Height. They may exceeds monitor in wmNormal.
		if ((wndMode == wmNormal) && !gpSet->isQuakeStyle && !m_InsideIntegration)
		{
			rcWnd.right = rcWnd.left+nWidth;
			rcWnd.bottom = rcWnd.top+nHeight;

			// Ideally, we must detects most right/bottom monitor to limit our window
			if (!PtInRect(&rcWork, MakePoint(rcWnd.right, rcWnd.bottom)))
			{
				// Yes, right-bottom corner is out-of-screen
				RECT rcTest = {rcWnd.right, rcWnd.bottom, rcWnd.right, rcWnd.bottom};
				
				MONITORINFO mi2; GetNearestMonitor(&mi2, &rcTest);
				RECT rcWork2 = (wndMode == wmFullScreen) ? mi2.rcMonitor : mi2.rcWork;

				rcWnd.right = min(rcWork2.right+rcMargins.right,rcWnd.right);
				rcWnd.bottom = min(rcWork2.bottom+rcMargins.bottom,rcWnd.bottom);
			}
		}
		else
		{
			// Maximized/FullScreen. Window CAN'T exceeds active monitor!
			rcWnd.right = min(rcWork.right+rcMargins.right,rcWork.left+rcStore.right-rcStore.left);
			rcWnd.bottom = min(rcWork.bottom+rcMargins.bottom,rcWork.top+rcStore.bottom-rcStore.top);
		}

		bChanged = (memcmp(&rcWnd, &rcStore, sizeof(rcWnd)) != 0);
	}

	return bChanged;
}

// ��� ���������������� ������� �������� - ����� ������ (������ �������� ����)|(������ �������)
// ��� ������� ������� �������� - ��������� (������ �������� ����) � (������ ���� ���������) ��� �������������
// �� x64 ��������� �����-�� ����� � ",RECT rFrom,". �������� ���������� ����� � rFrom,
// �� ��� �� �����, ����� "RECT rc = rFrom;", rc �������� ���������� �������� >:|
RECT CConEmuMain::CalcRect(enum ConEmuRect tWhat, const RECT &rFrom, enum ConEmuRect tFrom, CVirtualConsole* pVCon, enum ConEmuMargins tTabAction/*=CEM_TAB*/)
{
	_ASSERTE(this!=NULL);
	RECT rc = rFrom; // �������������, ���� �� �� ���������...
	RECT rcShift = MakeRect(0,0);
	enum ConEmuRect tFromNow = tFrom;

	// tTabAction ������ �������� � ���� CEM_TAB
	_ASSERTE((tTabAction & CEM_TAB)==CEM_TAB);

	WARNING("CEM_SCROLL ������ �������?");
	WARNING("���������, ����� DC ��������� �������������� ����� �������� CEM_BACK");

	//if (!pVCon)
	//	pVCon = gpConEmu->mp_ VActive;

	if (rFrom.left || rFrom.top)
	{
		if (rFrom.left >= rFrom.right || rFrom.top >= rFrom.bottom)
		{
			MBoxAssert(!(rFrom.left || rFrom.top));
		}
		else
		{
			// ��� ������� ����� ��������� �� ������ ��������.
			// �.�. ���� ���������� Rect ��������,
			// ���������� �� CalcRect(CER_FULLSCREEN, MakeRect(0,0), CER_FULLSCREEN);
		}
	}

	switch (tFrom)
	{
		case CER_MAIN: // switch (tFrom)
			// ����� ������ ������� ����� � ���������!
		{
			// ��� ����� ����, ���� ������� GetWindowRect ��� ghWnd, ����� �� isIconic!
			_ASSERTE((rc.left!=-32000 && rc.right!=-32000) && "Use CalcRect(CER_MAIN) instead of GetWindowRect() while IsIconic!");
			rcShift = CalcMargins(CEM_FRAMECAPTION);
			rc.right = (rc.right-rc.left) - (rcShift.left+rcShift.right);
			rc.bottom = (rc.bottom-rc.top) - (rcShift.top+rcShift.bottom);
			rc.left = 0;
			rc.top = 0; // �������� ���������� �������
			tFromNow = CER_MAINCLIENT;
		}
		break;
		case CER_MAINCLIENT: // switch (tFrom)
		{
			//
		}
		break;
		case CER_DC: // switch (tFrom)
		{
			// ������ ���� ��������� � ��������!
			//MBoxAssert(!(rFrom.left || rFrom.top));
			TODO("DoubleView");

			switch (tWhat)
			{
				case CER_MAIN:
				{
					//rcShift = CalcMargins(CEM_BACK);
					//AddMargins(rc, rcShift, TRUE/*abExpand*/);
					rcShift = CalcMargins(tTabAction|CEM_ALL_MARGINS);
					AddMargins(rc, rcShift, TRUE/*abExpand*/);
					//rcShift = CalcMargins(CEM_FRAME);
					//AddMargins(rc, rcShift, TRUE/*abExpand*/);
					WARNING("����������� ���������� ��� DoubleView");
					tFromNow = CER_MAINCLIENT;
				} break;
				case CER_MAINCLIENT:
				{
					//rcShift = CalcMargins(CEM_BACK);
					//AddMargins(rc, rcShift, TRUE/*abExpand*/);
					rcShift = CalcMargins(tTabAction);
					AddMargins(rc, rcShift, TRUE/*abExpand*/);
					WARNING("����������� ���������� ��� DoubleView");
					tFromNow = CER_MAINCLIENT;
				} break;
				case CER_TAB:
				{
					//rcShift = CalcMargins(CEM_BACK);
					//AddMargins(rc, rcShift, TRUE/*abExpand*/);
					_ASSERTE(tWhat!=CER_TAB); // ����� ����� ���� �� ������?
					rcShift = CalcMargins(tTabAction);
					AddMargins(rc, rcShift, TRUE/*abExpand*/);
					WARNING("����������� ���������� ��� DoubleView");
					tFromNow = CER_MAINCLIENT;
				} break;
				case CER_WORKSPACE:
				{
					WARNING("��� tFrom. CER_WORKSPACE - �� �������������� (����?)");
					_ASSERTE(tWhat!=CER_WORKSPACE);
				} break;
				case CER_BACK:
				{
					//rcShift = CalcMargins(CEM_BACK);
					//AddMargins(rc, rcShift, TRUE/*abExpand*/);
					rcShift = CalcMargins(tTabAction|CEM_CLIENT_MARGINS);
					//AddMargins(rc, rcShift, TRUE/*abExpand*/);
					rc.top += rcShift.top; rc.bottom += rcShift.top;
					_ASSERTE(rcShift.left == 0 && rcShift.right == 0 && rcShift.bottom == 0);
					WARNING("����������� ���������� ��� DoubleView");
					tFromNow = CER_MAINCLIENT;
				} break;
				default:
					_ASSERTE(FALSE && "Unexpected tWhat");
					break;
			}
		}
		return rc;
		case CER_CONSOLE_ALL: // switch (tFrom)
		case CER_CONSOLE_CUR: // switch (tFrom)
		{
			// ��� ������� ��������������� SplitScreen.
			// �.�. �����, ��������, ����� ���� �� ���������� ������� "Settings" � CVConGroup::SetAllConsoleWindowsSize

			// ������ ������� � ��������!
			//MBoxAssert(!(rFrom.left || rFrom.top));
			_ASSERTE(tWhat!=CER_CONSOLE_ALL && tWhat!=CER_CONSOLE_CUR);
			//if (gpSetCls->FontWidth()==0) {
			//    MBoxAssert(pVCon!=NULL);
			//    pVCon->InitDC(false, true); // ���������������� ������ ������ �� ���������
			//}
			// ��� ������ ���� ��������� DC
			rc = MakeRect((rFrom.right-rFrom.left) * gpSetCls->FontWidth(),
			              (rFrom.bottom-rFrom.top) * gpSetCls->FontHeight());

			RECT rcScroll = CalcMargins(CEM_SCROLL|CEM_PAD);
			AddMargins(rc, rcScroll, TRUE);

			if (tWhat != CER_DC)
				rc = CalcRect(tWhat, rc, CER_DC);

			// -- tFromNow = CER_BACK; -- ������ �� ���������, �.�. �� ����� �� �����
		}
		return rc;
		case CER_FULLSCREEN: // switch (tFrom)
		case CER_MAXIMIZED: // switch (tFrom)
		case CER_RESTORE: // switch (tFrom)
		case CER_MONITOR: // switch (tFrom)
			// ��������, ����� �������� ����� �������� ������ ������������ ���� �� ������� ��������
			// RECT rcMax = CalcRect(CER_MAXIMIZED, MakeRect(0,0), CER_MAXIMIZED);
			_ASSERTE((tFrom!=CER_FULLSCREEN && tFrom!=CER_MAXIMIZED && tFrom!=CER_RESTORE) || (tFrom==tWhat));
			break;
		case CER_BACK: // switch (tFrom)
			break;
		default:
			_ASSERTE(tFrom==CER_MAINCLIENT); // ��� �������, ���� ����� �������� �� ������
			break;
	};

	//// ������ rc ������ ��������������� CER_MAINCLIENT
	//RECT rcAddShift = MakeRect(0,0);

	// ���� �� ����� ���� - ������ tFrom==CER_MAINCLIENT

	switch (tWhat)
	{
		case CER_TAB: // switch (tWhat)
		{
			// ������� �� ���� ����� ��������� ������ �� �������������
		} break;
		case CER_WORKSPACE: // switch (tWhat)
		{
			rcShift = CalcMargins(tTabAction|CEM_CLIENT_MARGINS/*|CEM_SCROLL*/);
			AddMargins(rc, rcShift);
		} break;
		case CER_BACK: // switch (tWhat)
		case CER_SCROLL: // switch (tWhat)
		case CER_DC: // switch (tWhat)
		case CER_CONSOLE_ALL: // switch (tWhat)
		case CER_CONSOLE_CUR: // switch (tWhat)
		case CER_CONSOLE_NTVDMOFF: // switch (tWhat)
		{
			rc = CVConGroup::CalcRect(tWhat, rc, tFromNow, pVCon, tTabAction);
			return rc;
		} break;
		case CER_MONITOR:    // switch (tWhat)
		case CER_FULLSCREEN: // switch (tWhat)
		case CER_MAXIMIZED:  // switch (tWhat)
		case CER_RESTORE:    // switch (tWhat)
			//case CER_CORRECTED:
		{
			//HMONITOR hMonitor = NULL;
			_ASSERTE(tFrom==tWhat || tFrom==CER_MAIN);

			DEBUGTEST(bool bIconic = isIconic());

			//if (ghWnd)
			//{
			//	WINDOWPLACEMENT wpl = {sizeof(wpl)};
			//	GetWindowPlacement(ghWnd, &wpl);
			//	if (bIconic)
			//	{
			//		RECT rcRestored;
			//		if (WindowMode == wmNormal)
			//			rcRestored = wpl.rcNormalPosition;
			//		else
			//			rcRestored = MakeRect(wpl.ptMaxPosition.x, wpl.ptMaxPosition.y, wpl.ptMaxPosition.x + GetSystemMetrics(SM_CXMAXIMIZED), wpl.ptMaxPosition.y + GetSystemMetrics(SM_CYMAXIMIZED));
			//		// Go
			//		hMonitor = MonitorFromRect(&rFrom, MONITOR_DEFAULTTONEAREST);
			//	}
			//	else
			//	{
			//		hMonitor = MonitorFromWindow(ghWnd, MONITOR_DEFAULTTONEAREST);
			//	}
			//}
			//else
			//{
			//	hMonitor = MonitorFromRect(&rFrom, MONITOR_DEFAULTTONEAREST);
			//}

			//if (tWhat != CER_CORRECTED)
			//    tFrom = tWhat;
			MONITORINFO mi = {sizeof(mi)};
			DEBUGTEST(HMONITOR hMonitor =) GetNearestMonitor(&mi);

			//if (GetMonitorInfo(hMonitor, &mi))
			{
				//switch (tFrom) // --����
				switch (tWhat)
				{
					case CER_MONITOR:
					{
						rc = mi.rcWork;

					} break;
					case CER_FULLSCREEN:
					{
						rc = mi.rcMonitor;

					} break;
					case CER_MAXIMIZED:
					{
						rc = mi.rcWork;
						RECT rcFrame = CalcMargins(CEM_FRAMEONLY);
						// ������������� ������ ���� �� �������� �� �������� (����� ��� ������������ ������� �� ������� ������)
						rc.left -= rcFrame.left;
						rc.right += rcFrame.right;
						rc.top -= rcFrame.top;
						rc.bottom += rcFrame.bottom;
						
					} break;
					case CER_RESTORE:
					{
						RECT rcNormal = {0};
						if (gpConEmu)
							rcNormal = gpConEmu->mrc_StoredNormalRect;
						int w = rcNormal.right - rcNormal.left;
						int h = rcNormal.bottom - rcNormal.top;
						if ((w > 0) && (h > 0))
						{
							rc = rcNormal;

							// ���� ����� ��������� ������������ ���� �������� 
							// ������������ ��������� - ����� ��������� ������� �������
							if (((rc.right + 30) <= mi.rcWork.left)
								|| ((rc.left + 30) >= mi.rcWork.right))
							{
								rc.left = mi.rcWork.left; rc.right = rc.left + w;
							}
							if (((rc.bottom + 30) <= mi.rcWork.top)
								|| ((rc.top + 30) >= mi.rcWork.bottom))
							{
								rc.top = mi.rcWork.top; rc.bottom = rc.top + h;
							}
						}
						else
						{
							rc = mi.rcWork;
						}
					} break;
					default:
					{
						_ASSERTE(tWhat==CER_FULLSCREEN || tWhat==CER_MAXIMIZED);
					}
				}
			}
			//else
			//{
			//	switch(tFrom)
			//	{
			//		case CER_MONITOR:
			//		{
			//			SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
			//		} break;
			//		case CER_FULLSCREEN:
			//		{
			//			rc = MakeRect(GetSystemMetrics(SM_CXFULLSCREEN),GetSystemMetrics(SM_CYFULLSCREEN));
			//		} break;
			//		case CER_MAXIMIZED:
			//		{
			//			rc = MakeRect(GetSystemMetrics(SM_CXMAXIMIZED),GetSystemMetrics(SM_CYMAXIMIZED));

			//			//120814 - ~WS_CAPTION
			//			//if (gpSet->isHideCaption && gpConEmu->mb_MaximizedHideCaption && !gpSet->isHideCaptionAlways())
			//			//	rc.top -= GetSystemMetrics(SM_CYCAPTION);

			//		} break;
			//		case CER_RESTORE:
			//		{
			//			RECT work;
			//			SystemParametersInfo(SPI_GETWORKAREA, 0, &work, 0);
			//			RECT rcNormal = {0};
			//			if (gpConEmu)
			//				rcNormal = gpConEmu->mrc_StoredNormalRect;
			//			int w = rcNormal.right - rcNormal.left;
			//			int h = rcNormal.bottom - rcNormal.top;
			//			if ((w > 0) && (h > 0))
			//			{
			//				rc = rcNormal;

			//				// ���� ����� ��������� ������������ ���� �������� 
			//				// ������������ ��������� - ����� ��������� ������� �������
			//				if (((rc.right + 30) <= work.left)
			//					|| ((rc.left + 30) >= work.right))
			//				{
			//					rc.left = work.left; rc.right = rc.left + w;
			//				}
			//				if (((rc.bottom + 30) <= work.top)
			//					|| ((rc.top + 30) >= work.bottom))
			//				{
			//					rc.top = work.top; rc.bottom = rc.top + h;
			//				}
			//			}
			//			else
			//			{
			//				WINDOWPLACEMENT wpl = {sizeof(WINDOWPLACEMENT)};
			//				if (ghWnd && GetWindowPlacement(ghWnd, &wpl))
			//					rc = wpl.rcNormalPosition;
			//				else
			//					rc = work;
			//			}
			//		} break;
			//		default:
			//			_ASSERTE(tFrom==CER_FULLSCREEN || tFrom==CER_MAXIMIZED);
			//	}
			//}

			//if (tWhat == CER_CORRECTED)
			//{
			//    RECT rcMon = rc;
			//    rc = rFrom;
			//    int nX = GetSystemMetrics(SM_CXSIZEFRAME), nY = GetSystemMetrics(SM_CYSIZEFRAME);
			//    int nWidth = rc.right-rc.left;
			//    int nHeight = rc.bottom-rc.top;
			//    static bool bFirstCall = true;
			//    if (bFirstCall) {
			//        if (gpSet->wndCascade && !gpSet->isDesktopMode) {
			//            BOOL lbX = ((rc.left+nWidth)>(rcMon.right+nX));
			//            BOOL lbY = ((rc.top+nHeight)>(rcMon.bottom+nY));
			//            {
			//                if (lbX && lbY) {
			//                    rc = MakeRect(rcMon.left,rcMon.top,rcMon.left+nWidth,rcMon.top+nHeight);
			//                } else if (lbX) {
			//                    rc.left = rcMon.right - nWidth; rc.right = rcMon.right;
			//                } else if (lbY) {
			//                    rc.top = rcMon.bottom - nHeight; rc.bottom = rcMon.bottom;
			//                }
			//            }
			//        }
			//        bFirstCall = false;
			//    }
			//	//2010-02-14 �� ��������������� ������������� ��� �������� ��������
			//	//           ��������� ��������� (����� ���� �� ����� ���������).
			//	//if (rc.left<(rcMon.left-nX)) {
			//	//	rc.left=rcMon.left-nX; rc.right=rc.left+nWidth;
			//	//}
			//	//if (rc.top<(rcMon.top-nX)) {
			//	//	rc.top=rcMon.top-nX; rc.bottom=rc.top+nHeight;
			//	//}
			//	//if ((rc.left+nWidth)>(rcMon.right+nX)) {
			//	//    rc.left = max((rcMon.left-nX),(rcMon.right-nWidth));
			//	//    nWidth = min(nWidth, (rcMon.right-rc.left+2*nX));
			//	//    rc.right = rc.left + nWidth;
			//	//}
			//	//if ((rc.top+nHeight)>(rcMon.bottom+nY)) {
			//	//    rc.top = max((rcMon.top-nY),(rcMon.bottom-nHeight));
			//	//    nHeight = min(nHeight, (rcMon.bottom-rc.top+2*nY));
			//	//    rc.bottom = rc.top + nHeight;
			//	//}
			//}
			return rc;
		} break;
		default:
			break;
	}

	//AddMargins(rc, rcAddShift);
	return rc; // ���������, ����������
}

POINT CConEmuMain::CalcTabMenuPos(CVirtualConsole* apVCon)
{
	POINT ptCur = {};
	if (apVCon)
	{
		RECT rcWnd;
		if (mp_TabBar && mp_TabBar->IsTabsShown())
		{
			mp_TabBar->GetActiveTabRect(&rcWnd);
			ptCur.x = rcWnd.left;
			ptCur.y = rcWnd.bottom;
		}
		else
		{
			CVConGuard VCon;
			if (GetActiveVCon(&VCon) >= 0)
			{
				GetWindowRect(VCon->GetView(), &rcWnd);
			}
			else
			{
				_ASSERTE(FALSE && "No Active VCon");
				GetWindowRect(ghWnd, &rcWnd);
			}

			ptCur.x = rcWnd.left;
			ptCur.y = rcWnd.top;
		}
	}
	return ptCur;
}

#if 0
/*!!!static!!*/
// �������� ������ (������ ������ ����) ���� �� ��� ���������� ������� � ��������
RECT CConEmuMain::MapRect(RECT rFrom, BOOL bFrame2Client)
{
	_ASSERTE(this!=NULL);
	RECT rcShift = CalcMargins(CEM_FRAMECAPTION);

	if (bFrame2Client)
	{
		//rFrom.left -= rcShift.left;
		//rFrom.top -= rcShift.top;
		rFrom.right -= (rcShift.right+rcShift.left);
		rFrom.bottom -= (rcShift.bottom+rcShift.top);
	}
	else
	{
		//rFrom.left += rcShift.left;
		//rFrom.top += rcShift.top;
		rFrom.right += (rcShift.right+rcShift.left);
		rFrom.bottom += (rcShift.bottom+rcShift.top);
	}

	return rFrom;
}
#endif

bool CConEmuMain::ScreenToVCon(LPPOINT pt, CVirtualConsole** ppVCon)
{
	_ASSERTE(this!=NULL);
	CVirtualConsole* lpVCon = GetVConFromPoint(*pt);

	if (!lpVCon)
		return false;

	HWND hView = lpVCon->GetView();
	ScreenToClient(hView, pt);

	if (ppVCon)
		*ppVCon = lpVCon;

	return true;
}

//// returns difference between window size and client area size of inWnd in outShift->x, outShift->y
//void CConEmuMain::GetCWShift(HWND inWnd, POINT *outShift)
//{
//    RECT cRect;
//
//    GetCWShift ( inWnd, &cRect );
//
//    outShift->x = cRect.right  - cRect.left;
//    outShift->y = cRect.bottom - cRect.top;
//}
//
//// returns margins between window frame and client area
//void CConEmuMain::GetCWShift(HWND inWnd, RECT *outShift)
//{
//    RECT cRect, wRect;
//    Get ClientRect(inWnd, &cRect); // The left and top members are zero. The right and bottom members contain the width and height of the window.
//    MapWindowPoints(inWnd, NULL, (LPPOINT)&cRect, 2);
//    GetWindowRect(inWnd, &wRect); // screen coordinates of the upper-left and lower-right corners of the window
//    outShift->top = wRect.top - cRect.top;          // <0
//    outShift->left = wRect.left - cRect.left;       // <0
//    outShift->bottom = wRect.bottom - cRect.bottom; // >0
//    outShift->right = wRect.right - cRect.right;    // >0
//}

//// ������� ������� �� ���� ������ �� ����� ���������� ����� �������� ���� �� ����� ���� ���������
//RECT CConEmuMain::ConsoleOffsetRect()
//{
//    RECT rect; memset(&rect, 0, sizeof(rect));
//
//  if (gpConEmu->mp_TabBar->IsActive())
//      rect = gpConEmu->mp_TabBar->GetMargins();
//
//  /*rect.top = gpConEmu->mp_TabBar->IsActive()?gpConEmu->mp_TabBar->Height():0;
//    rect.left = 0;
//    rect.bottom = 0;
//    rect.right = 0;*/
//
//  return rect;
//}

//// ��������� ��������� ���� ���������
//RECT CConEmuMain::DCClientRect(RECT* pClient/*=NULL*/)
//{
//    RECT rect;
//  if (pClient)
//      rect = *pClient;
//  else
//      Get ClientRect(ghWnd, &rect);
//  if (gpConEmu->mp_TabBar->IsActive()) {
//      RECT mr = gpConEmu->mp_TabBar->GetMargins();
//      //rect.top += gpConEmu->mp_TabBar->Height();
//      rect.top += mr.top;
//      rect.left += mr.left;
//      rect.right -= mr.right;
//      rect.bottom -= mr.bottom;
//  }
//
//  if (pClient)
//      *pClient = rect;
//    return rect;
//}

//// returns console size in columns and lines calculated from current window size
//// rectInWindow - ���� true - � ������, false - ������ ������
//COORD CConEmuMain::ConsoleSizeFromWindow(RECT* arect /*= NULL*/, bool frameIncluded /*= false*/, bool alreadyClient /*= false*/)
//{
//    COORD size;
//
//  if (!gpSet->Log Font.lfWidth || !gpSet->Log Font.lfHeight) {
//      MBoxAssert(FALSE);
//      // ������ ������ ��� �� ���������������! ������ ������� ������ �������! TODO:
//      CONSOLE_SCREEN_BUFFER_INFO inf; memset(&inf, 0, sizeof(inf));
//      GetConsoleScreenBufferInfo(mp_ VActive->hConOut(), &inf);
//      size = inf.dwSize;
//      return size;
//  }
//
//    RECT rect, consoleRect;
//    if (arect == NULL)
//    {
//      frameIncluded = false;
//        Get ClientRect(ghWnd, &rect);
//      consoleRect = ConsoleOffsetRect();
//    }
//    else
//    {
//        rect = *arect;
//      if (alreadyClient)
//          memset(&consoleRect, 0, sizeof(consoleRect));
//      else
//          consoleRect = ConsoleOffsetRect();
//    }
//
//    size.X = (rect.right - rect.left - (frameIncluded ? cwShift.x : 0) - consoleRect.left - consoleRect.right)
//      / gpSet->LogFont.lfWidth;
//    size.Y = (rect.bottom - rect.top - (frameIncluded ? cwShift.y : 0) - consoleRect.top - consoleRect.bottom)
//      / gpSet->LogFont.lfHeight;
//    #ifdef MSGLOGGER
//        char szDbg[100]; wsprintfA(szDbg, "   ConsoleSizeFromWindow={%i,%i}\n", size.X, size.Y);
//        DEBUGLOGFILE(szDbg);
//    #endif
//    return size;
//}

//// return window size in pixels calculated from console size
//RECT CConEmuMain::WindowSizeFromConsole(COORD consoleSize, bool rectInWindow /*= false*/, bool clientOnly /*= false*/)
//{
//    RECT rect;
//    rect.top = 0;
//    rect.left = 0;
//    RECT offsetRect;
//  if (clientOnly)
//      memset(&offsetRect, 0, sizeof(RECT));
//  else
//      offsetRect = ConsoleOffsetRect();
//    rect.bottom = consoleSize.Y * gpSet->LogFont.lfHeight + (rectInWindow ? cwShift.y : 0) + offsetRect.top + offsetRect.bottom;
//    rect.right = consoleSize.X * gpSet->LogFont.lfWidth + (rectInWindow ? cwShift.x : 0) + offsetRect.left + offsetRect.right;
//    #ifdef MSGLOGGER
//        char szDbg[100]; wsprintfA(szDbg, "   WindowSizeFromConsole={%i,%i}\n", rect.right,rect.bottom);
//        DEBUGLOGFILE(szDbg);
//    #endif
//    return rect;
//}

//// size in columns and lines
//// ����� ����� �������� ������ GUI � "�����" ������ �������� � ��������
//// �.�. ���� �� ����������� ���� 2 �������, � ������ ������ 80x25
//// �� ��� ��� ������� ������ ����� 40x25 � GUI ������������� ��� 80x25
//void CConEmuMain::SetAllConsoleWindowsSize(const COORD& size, /*bool updateInfo,*/ bool bSetRedraw /*= false*/)
//{
//	CVConGroup::SetAllConsoleWindowsSize(size, /*updateInfo,*/ bSetRedraw);
//}

// �������� ������ ������� �� ������� ���� (��������)
void CConEmuMain::SyncConsoleToWindow(LPRECT prcNewWnd/*=NULL*/, bool bSync/*=false*/)
{
	CVConGroup::SyncConsoleToWindow(prcNewWnd, bSync);
}

void CConEmuMain::SyncNtvdm()
{
	OnSize();
}

// -- ������� ������, ������������
// ���������� ������ ��������� ���� �� �������� ������� mp_ VActive
void CConEmuMain::SyncWindowToConsole()
{
	CVConGroup::SyncWindowToConsole();
}

void CConEmuMain::AutoSizeFont(RECT arFrom, enum ConEmuRect tFrom)
{
	if (!gpSet->isFontAutoSize)
		return;

	// � 16��� ������ - �� �������������� ����
	if (isNtvdm())
		return;

	if (!wndWidth || !wndHeight)
	{
		MBoxAssert(wndWidth!=0 && wndHeight!=0);
		return;
	}

	//GO

	RECT rc = arFrom;

	if (tFrom == CER_MAIN)
	{
		rc = CalcRect(CER_DC, arFrom, CER_MAIN);
	}
	else if (tFrom == CER_MAINCLIENT)
	{
		rc = CalcRect(CER_DC, arFrom, CER_MAINCLIENT);
	}
	else
	{
		MBoxAssert(tFrom==CER_MAINCLIENT || tFrom==CER_MAIN);
		return;
	}

	// !!! ��� CER_DC ������ � rc.right
	int nFontW = (rc.right - rc.left) / wndWidth;

	if (nFontW < 5) nFontW = 5;

	int nFontH = (rc.bottom - rc.top) / wndHeight;

	if (nFontH < 8) nFontH = 8;

	gpSetCls->AutoRecreateFont(nFontW, nFontH);
}

void CConEmuMain::StoreIdealRect()
{
	if ((WindowMode != wmNormal) || m_InsideIntegration)
		return;

	RECT rcWnd = CalcRect(CER_MAIN);
	UpdateIdealRect(rcWnd);
}

RECT CConEmuMain::GetIdealRect()
{
	RECT rcIdeal = mr_Ideal.rcIdeal;
	if (m_InsideIntegration || (rcIdeal.right <= rcIdeal.left) || (rcIdeal.bottom <= rcIdeal.top))
	{
		rcIdeal = GetDefaultRect();
	}
	return rcIdeal;
}

void CConEmuMain::StoreNormalRect(RECT* prcWnd)
{
	mouse.bCheckNormalRect = false;

	// �������� ���������� � gpSet, ���� ���������
	// ���� ������ ���� � ����� ������� - ����������, ������ �������� SetWindowMode
	if ((WindowMode == wmNormal) && !m_InsideIntegration && !isIconic())
	{
		RECT rcNormal = {};
		if (prcWnd)
			rcNormal = *prcWnd;
		else
			GetWindowRect(ghWnd, &rcNormal);

		gpSetCls->UpdatePos(rcNormal.left, rcNormal.top);

		// 120720 - ���� ���� ������ ����� ������, �� ���� �� ��������� mrc_StoredNormalRect,
		// �����, ���� ��������� Maximize ��� ����������� �� �������� ���� ������ - �� ���
		// �������������� ���� �������� ���� ���������������� - ��� ������� ���������� �� �������.
		if (!isSizing())
			mrc_StoredNormalRect = rcNormal;

		if (CVConGroup::isVConExists(0))
		{
			// ��� ������� ����� ���� ��������� - mp_ VActive ��� �� �������������
			// ��� ��� � TextWidth/TextHeight �� ���������
			//-- gpSetCls->UpdateSize(mp_ VActive->TextWidth, mp_ VActive->TextHeight);
			RECT rcAll = CVConGroup::AllTextRect();
			gpSetCls->UpdateSize(rcAll.right, rcAll.bottom);
		}
	}
}

BOOL CConEmuMain::ShowWindow(int anCmdShow, DWORD nAnimationMS /*= 0*/)
{
	BOOL lbRc = FALSE;
	bool bUseApi = true;

	// In Quake mode - animation proceeds with SetWindowRgn
	//   yes, it is possible to use Slide, but ConEmu may hold
	//   many child windows (outprocess), which don't support
	//   WM_PRINTCLIENT. So, our choice - "trim" window with RGN
	// When Caption is visible - Windows animates window itself.
	if (!gpSet->isQuakeStyle && gpSet->isCaptionHidden())
	{
		// Well, Caption is hidden, Windows does not animates our window.
		if (anCmdShow == SW_HIDE)
		{
			bUseApi = false; // ����� AnimateWindow
		}
		else if (anCmdShow == SW_SHOWMAXIMIZED)
		{
			if (::IsZoomed(ghWnd)) // ��� ����� "RAW" �������� zoomed
				bUseApi = false; // ����� AnimateWindow
		}
		else if (anCmdShow == SW_SHOWNORMAL)
		{
			if (!::IsZoomed(ghWnd) && !::IsIconic(ghWnd)) // ��� ����� "RAW" �������� zoomed
				bUseApi = false; // ����� AnimateWindow
		}
	}

	if (bUseApi)
	{
		bool b, bAllowPreserve = false;
		if (anCmdShow == SW_SHOWMAXIMIZED)
		{
			if (::IsZoomed(ghWnd)) // ��� ����� "RAW" �������� zoomed
				bAllowPreserve = true;
		}
		else if (anCmdShow == SW_SHOWNORMAL)
		{
			if (!::IsZoomed(ghWnd) && !::IsIconic(ghWnd)) // ��� ����� "RAW" �������� zoomed
				bAllowPreserve = true;
		}
		// Allowed?
		if (bAllowPreserve)
		{
			b = SetAllowPreserveClient(true);
		}

		//GO
		lbRc = apiShowWindow(ghWnd, anCmdShow);

		if (bAllowPreserve)
		{
			SetAllowPreserveClient(b);
		}
	}
	else
	{
		// Use animation only when Caption is hidden.
		// Otherwise - artifacts comes: while animation - theming does not applies
		// (Win7, Glass - caption style changes while animation)
		_ASSERTE(gpSet->isCaptionHidden());

		DWORD nFlags = AW_BLEND;
		if (anCmdShow == SW_HIDE)
			nFlags |= AW_HIDE;

		if (nAnimationMS == 0)
			nAnimationMS = CONEMU_ANIMATE_DURATION;
		else if (nAnimationMS > CONEMU_ANIMATE_DURATION_MAX)
			nAnimationMS = CONEMU_ANIMATE_DURATION_MAX;

		AnimateWindow(ghWnd, nAnimationMS, nFlags);
	}

	return lbRc;
}

bool CConEmuMain::SetQuakeMode(BYTE NewQuakeMode, ConEmuWindowMode nNewWindowMode /*= wmNotChanging*/, bool bFromDlg /*= false*/)
{
	BYTE bPrevStyle = gpSet->isQuakeStyle;
	gpSet->isQuakeStyle = NewQuakeMode;

	if (gpSet->isQuakeStyle && bFromDlg)
	{
		gpSet->isTryToCenter = true;
		gpSet->SetMinToTray(true);
	}

	HWND hWnd2 = ghOpWnd ? gpSetCls->mh_Tabs[CSettings::thi_SizePos] : NULL; // ��������� � �����������

	if (hWnd2)
	{
		EnableWindow(GetDlgItem(hWnd2, cbQuakeAutoHide), gpSet->isQuakeStyle);
		EnableWindow(GetDlgItem(hWnd2, tHideCaptionAlwaysFrame), gpSet->isQuakeStyle);
		EnableWindow(GetDlgItem(hWnd2, stHideCaptionAlwaysFrame), gpSet->isQuakeStyle);
		gpSetCls->checkDlgButton(hWnd2, cbQuakeStyle, gpSet->isQuakeStyle!=0);
		gpSetCls->checkDlgButton(hWnd2, cbQuakeAutoHide, gpSet->isQuakeStyle==2);
		gpSetCls->checkDlgButton(hWnd2, cbTryToCenter, gpSet->isTryToCenter);
	}

	HWND hTabsPg = ghOpWnd ? gpSetCls->mh_Tabs[CSettings::thi_Ext] : NULL; // ��������� � �����������
	if (hTabsPg)
	{
		if (gpSet->isHideCaptionAlways())
		{
			gpSetCls->checkDlgButton(hTabsPg, cbHideCaptionAlways, BST_CHECKED);
		}
		EnableWindow(GetDlgItem(hTabsPg, cbHideCaptionAlways), !gpSet->isForcedHideCaptionAlways());
	}

	//ConEmuWindowMode nNewWindowMode = 
	//	IsChecked(hWnd2, rMaximized) ? wmMaximized :
	//	IsChecked(hWnd2, rFullScreen) ? wmFullScreen : 
	//	wmNormal;
	if (nNewWindowMode == wmNotChanging || nNewWindowMode == wmCurrent)
		nNewWindowMode = (ConEmuWindowMode)gpSet->_WindowMode;
	else
		gpSet->_WindowMode = nNewWindowMode;

	if (gpSet->isQuakeStyle && !bPrevStyle)
	{
		m_QuakePrevSize.bWasSaved = true;
		m_QuakePrevSize.wndWidth = gpConEmu->wndWidth;
		m_QuakePrevSize.wndHeight = gpConEmu->wndHeight;
		m_QuakePrevSize.wndX = gpConEmu->wndX;
		m_QuakePrevSize.wndY = gpConEmu->wndY;
		m_QuakePrevSize.nFrame = gpSet->nHideCaptionAlwaysFrame;
		m_QuakePrevSize.WindowMode = gpConEmu->WindowMode;
		m_QuakePrevSize.rcIdealInfo = mr_Ideal;
		//gpSet->nHideCaptionAlwaysFrame = 0;
	}
	else if (!gpSet->isQuakeStyle && m_QuakePrevSize.bWasSaved)
	{
		gpConEmu->wndWidth = m_QuakePrevSize.wndWidth;
		gpConEmu->wndHeight = m_QuakePrevSize.wndHeight;
		gpConEmu->wndX = m_QuakePrevSize.wndX;
		gpConEmu->wndY = m_QuakePrevSize.wndY;
		gpSet->nHideCaptionAlwaysFrame = m_QuakePrevSize.nFrame;
		nNewWindowMode = m_QuakePrevSize.WindowMode;
		mr_Ideal = m_QuakePrevSize.rcIdealInfo;
	}

	RECT rcWnd = gpConEmu->GetDefaultRect();
	UNREFERENCED_PARAMETER(rcWnd);

	// ��� ���������� Quake ������ - ����� "���������" ������ �� "������ �����"
	// ����� ��� ����� �� ������� ������ ��� ������ OnHideCaption
	if (!gpSet->isQuakeStyle && bPrevStyle)
	{
		//SetWindowPos(ghWnd, NULL, rcWnd.left, rcWnd.top, rcWnd.right-rcWnd.left, rcWnd.bottom-rcWnd.top, SWP_NOZORDER);
		m_QuakePrevSize.bWaitReposition = true;
		gpConEmu->UpdateIdealRect(rcWnd);
		mrc_StoredNormalRect = rcWnd;
	}

	gpConEmu->SetWindowMode(nNewWindowMode, TRUE);

	if (m_QuakePrevSize.bWaitReposition)
		m_QuakePrevSize.bWaitReposition = false;

	if (hWnd2)
		SetDlgItemInt(hWnd2, tHideCaptionAlwaysFrame, gpSet->HideCaptionAlwaysFrame(), TRUE);
	if (ghOpWnd && gpSetCls->mh_Tabs[CSettings::thi_Show])
		SetDlgItemInt(gpSetCls->mh_Tabs[CSettings::thi_Show], tHideCaptionAlwaysFrame, gpSet->HideCaptionAlwaysFrame(), TRUE);

	// Save current rect, JIC
	StoreIdealRect();

	apiSetForegroundWindow(ghOpWnd ? ghOpWnd : ghWnd);

	return true;
}

ConEmuWindowMode CConEmuMain::GetWindowMode()
{
	ConEmuWindowMode wndMode = gpSet->isQuakeStyle ? ((ConEmuWindowMode)gpSet->_WindowMode) : WindowMode;
	return wndMode;
}

// inMode: wmNormal, wmMaximized, wmFullScreen
bool CConEmuMain::SetWindowMode(ConEmuWindowMode inMode, BOOL abForce /*= FALSE*/, BOOL abFirstShow /*= FALSE*/)
{
	if (inMode != wmNormal && inMode != wmMaximized && inMode != wmFullScreen)
		inMode = wmNormal; // ������ �������� ��������?

	if (!isMainThread()) 
	{
		PostMessage(ghWnd, mn_MsgSetWindowMode, inMode, 0);
		return false;
	}

	if ((inMode == wmFullScreen) && gpSet->isDesktopMode)
		inMode = (WindowMode != wmNormal) ? wmNormal : wmMaximized; // FullScreen �� Desktop-� ����������

	if (gpSetCls->isAdvLogging)
	{
		wchar_t szInfo[128];
		_wsprintf(szInfo, SKIPLEN(countof(szInfo)) L"SetWindowMode(%u) begin", inMode);
		LogString(szInfo);
	}

	if ((inMode != wmFullScreen) && (WindowMode == wmFullScreen))
	{
		// ������ vkForceFullScreen
		OnForcedFullScreen(false);
	}

	#ifdef _DEBUG
	DWORD_PTR dwStyle = GetWindowLongPtr(ghWnd, GWL_STYLE);
	#endif

	if (isPictureView())
	{
		#ifndef _DEBUG
		//2009-04-22 ���� ������ PictureView - ����� �� ���������...
		return false;
		#else
		_ASSERTE(FALSE && "Change WindowMode will be skipped in Release: isPictureView()");
		#endif
	}

	SetCursor(LoadCursor(NULL,IDC_WAIT));
	mb_PassSysCommand = true;

	//WindowPlacement -- ������������ ������, �.�. �� �������� � ����������� Workspace, � �� Screen!
	RECT rcWnd; GetWindowRect(ghWnd, &rcWnd);

	//bool canEditWindowSizes = false;
	bool lbRc = false;
	static bool bWasSetFullscreen = false;
	// ����� ������ ������� ������
	mouse.state &= ~(MOUSE_SIZING_BEGIN|MOUSE_SIZING_TODO);

	bool bNewFullScreen = (inMode == wmFullScreen) || (gpSet->isQuakeStyle && (gpSet->_WindowMode == wmFullScreen));

	if (bWasSetFullscreen && !bNewFullScreen)
	{
		if (mp_TaskBar2)
		{
			if (!gpSet->isDesktopMode)
				mp_TaskBar2->MarkFullscreenWindow(ghWnd, FALSE);

			bWasSetFullscreen = false;
		}
	}

	bool bIconic = isIconic();

	// When changing more to smth else than wmNormal, and current mode is wmNormal
	// save current window rect for futher usage (when restoring from wmMaximized for example)
	if ((inMode != wmNormal) && isWindowNormal() && !bIconic)
		StoreNormalRect(&rcWnd);

	// ��������� ��� Quake
	ConEmuWindowMode NewMode = (gpSet->isQuakeStyle || m_InsideIntegration) ? wmNormal : inMode;
	// � ����� ���������� _�����_ �����
	changeFromWindowMode = WindowMode;
	isRestoreFromMinimized = bIconic;
	WindowMode = inMode;


	CVConGuard VCon;
	GetActiveVCon(&VCon);
	//CRealConsole* pRCon = (gpSetCls->isAdvLogging!=0) ? (VCon.VCon() ? VCon.VCon()->RCon() : NULL) : NULL;

	LogString((inMode==wmNormal) ? "SetWindowMode(wmNormal)" :
		                           (inMode==wmMaximized) ? "SetWindowMode(wmMaximized)" :
		                           (inMode==wmFullScreen) ? "SetWindowMode(wmFullScreen)" : "SetWindowMode(INVALID)",
		                           TRUE);

	OnHideCaption(); // inMode �� ���������� �����, �.�. WindowMode ��� �������

	DEBUGTEST(BOOL bIsVisible = IsWindowVisible(ghWnd););

	//!!!
	switch (NewMode)
	{
		case wmNormal:
		{
			DEBUGLOGFILE("SetWindowMode(wmNormal)\n");
			RECT consoleSize;

			RECT rcIdeal = GetIdealRect();
			AutoSizeFont(rcIdeal, CER_MAIN);
			// ��������� ������ �� ������������ WindowRect
			if (!CVConGroup::PreReSize(inMode, rcIdeal))
			{
				mb_PassSysCommand = false;
				goto wrap;
			}

			consoleSize = MakeRect(gpConEmu->wndWidth, gpConEmu->wndHeight);

			// ��� ������ "::IsZoomed(ghWnd)"
			if (isIconic() || ::IsZoomed(ghWnd))
			{
				//apiShow Window(ghWnd, SW_SHOWNORMAL); // WM_SYSCOMMAND ������������ �� �������...
				mb_IgnoreSizeChange = true;

				if (gpSet->isDesktopMode)
				{
					RECT rcNormal = CalcRect(CER_RESTORE, MakeRect(0,0), CER_RESTORE);
					DWORD_PTR dwStyle = GetWindowLongPtr(ghWnd, GWL_STYLE);
					if (dwStyle & WS_MAXIMIZE)
						SetWindowStyle(dwStyle&~WS_MAXIMIZE);
					SetWindowPos(ghWnd, HWND_TOP, 
						rcNormal.left, rcNormal.top, 
						rcNormal.right-rcNormal.left, rcNormal.bottom-rcNormal.top,
						SWP_NOCOPYBITS|SWP_SHOWWINDOW);
				}
				else if (IsWindowVisible(ghWnd))
				{
					if (gpSetCls->isAdvLogging) LogString("WM_SYSCOMMAND(SC_RESTORE)");

					DefWindowProc(ghWnd, WM_SYSCOMMAND, SC_RESTORE, 0); //2009-04-22 ���� SendMessage
				}
				else
				{
					if (gpSetCls->isAdvLogging) LogString("ShowWindow(SW_SHOWNORMAL)");

					ShowWindow(SW_SHOWNORMAL);
				}

				//RePaint();
				mb_IgnoreSizeChange = FALSE;

				//// �������� (�������), ����� ��� isIconic?
				//if (mb_MaximizedHideCaption)
				//	mb_MaximizedHideCaption = FALSE;

				if (gpSetCls->isAdvLogging) LogString("OnSize(false).1");

				//OnSize(false); // ���������� ������ �������� ������
			}

			//// �������� (����������)
			//if (mb_MaximizedHideCaption)
			//	mb_MaximizedHideCaption = FALSE;

			RECT rcNew;
			if (m_InsideIntegration)
			{
				rcNew = GetDefaultRect();
			}
			else
			{
				rcNew = CalcRect(CER_MAIN, consoleSize, CER_CONSOLE_ALL);
				//int nWidth = rcNew.right-rcNew.left;
				//int nHeight = rcNew.bottom-rcNew.top;
				//rcNew.left+=gpConEmu->wndX; rcNew.top+=gpConEmu->wndY;
				//rcNew.right+=gpConEmu->wndX; rcNew.bottom+=gpConEmu->wndY;
				if ((mrc_StoredNormalRect.right > mrc_StoredNormalRect.left) && (mrc_StoredNormalRect.bottom > mrc_StoredNormalRect.top))
				{
					rcNew.left+=mrc_StoredNormalRect.left; rcNew.top+=mrc_StoredNormalRect.top;
					rcNew.right+=mrc_StoredNormalRect.left; rcNew.bottom+=mrc_StoredNormalRect.top;
				}
				else
				{
					MONITORINFO mi; GetNearestMonitor(&mi);
					rcNew.left+=mi.rcWork.left; rcNew.top+=mi.rcWork.top;
					rcNew.right+=mi.rcWork.left; rcNew.bottom+=mi.rcWork.top;
				}
			}
			// 2010-02-14 �������� ������ ������ ��� �������� �������� � ���������� �������
			//// ��������� ������ �����, ��������� - ������ �������� rcNew ��� ������� ������� �������� ��������
			//rcNew = CalcRect(CER_CORRECTED, rcNew, CER_MAXIMIZED);
			#ifdef _DEBUG
			WINDOWPLACEMENT wpl; memset(&wpl,0,sizeof(wpl)); wpl.length = sizeof(wpl);
			GetWindowPlacement(ghWnd, &wpl);
			#endif

			if (gpSetCls->isAdvLogging)
			{
				char szInfo[128]; wsprintfA(szInfo, "SetWindowPos(X=%i, Y=%i, W=%i, H=%i)", rcNew.left, rcNew.top, rcNew.right-rcNew.left, rcNew.bottom-rcNew.top);
				LogString(szInfo);
			}

			SetWindowPos(ghWnd, NULL, rcNew.left, rcNew.top, rcNew.right-rcNew.left, rcNew.bottom-rcNew.top, SWP_NOZORDER);
			#ifdef _DEBUG
			GetWindowPlacement(ghWnd, &wpl);
			#endif

			gpSetCls->UpdateWindowMode(wmNormal);

			if (!IsWindowVisible(ghWnd))
			{
				ShowWindow((abFirstShow && WindowStartMinimized) ? SW_SHOWMINNOACTIVE : SW_SHOWNORMAL);
			}

			#ifdef _DEBUG
			GetWindowPlacement(ghWnd, &wpl);
			#endif

			// ���� ��� �� ����� �������� - �� �� ������� ShowWindow - isIconic ���������� FALSE
			// ����� �������� IsZoomed (FullScreen ������ ���� Zoomed)
			if (!(abFirstShow && WindowStartMinimized) && (isIconic() || ::IsZoomed(ghWnd)))
			{
				ShowWindow(SW_SHOWNORMAL); // WM_SYSCOMMAND ������������ �� �������...
				// ���-�� ����� AltF9, AltF9 ������ �������� �� ����������...
				//hRgn = CreateWindowRgn();
				//SetWindowRgn(ghWnd, hRgn, TRUE);
			}

			// Already resored, need to clear the flag to avoid incorrect sizing
			isRestoreFromMinimized = false;

			UpdateWindowRgn();
			OnSize(false); // ���������� ������ �������� ������

			//#ifdef _DEBUG
			//GetWindowPlacement(ghWnd, &wpl);
			//UpdateWindow(ghWnd);
			//#endif
		} break; //wmNormal

		case wmMaximized:
		{
			DEBUGLOGFILE("SetWindowMode(wmMaximized)\n");

			#ifdef _DEBUG
			RECT rcShift = CalcMargins(CEM_FRAMEONLY);
			_ASSERTE((rcShift.left==0 && rcShift.right==0 && rcShift.bottom==0 && rcShift.top==0) || !gpSet->isCaptionHidden());
			#endif

			#ifdef _DEBUG // ����
			CalcRect(CER_MAXIMIZED, MakeRect(0,0), CER_MAXIMIZED);
			#endif
			RECT rcMax = CalcRect(CER_MAXIMIZED);

			WARNING("����� ���������� ��-�� ������������� ������� �������");
			// � ���� ������ ������ �� ���������� ����������� ��������� � �������������� �� � ConEmu
			if (!CVConGroup::PreReSize(inMode, rcMax))
			{
				mb_PassSysCommand = false;
				goto wrap;
			}

			//if (mb_MaximizedHideCaption || !::IsZoomed(ghWnd))
			if (bIconic || (changeFromWindowMode != wmMaximized))
			{
				mb_IgnoreSizeChange = true;
				InvalidateAll();
				if (!gpSet->isDesktopMode)
				{
					DEBUGTEST(WINDOWPLACEMENT wpl1 = {sizeof(wpl1)}; GetWindowPlacement(ghWnd, &wpl1););
					ShowWindow(SW_SHOWMAXIMIZED);
					DEBUGTEST(WINDOWPLACEMENT wpl2 = {sizeof(wpl2)}; GetWindowPlacement(ghWnd, &wpl2););
					if (changeFromWindowMode == wmFullScreen)
					{
						SetWindowPos(ghWnd, HWND_TOP, 
							rcMax.left, rcMax.top, 
							rcMax.right-rcMax.left, rcMax.bottom-rcMax.top,
							SWP_NOCOPYBITS|SWP_SHOWWINDOW);
					}
				}
				else
				{
					/*WINDOWPLACEMENT wpl = {sizeof(WINDOWPLACEMENT)};
					GetWindowPlacement(ghWnd, &wpl);
					wpl.flags = 0;
					wpl.showCmd = SW_SHOWMAXIMIZED;
					wpl.ptMaxPosition.x = rcMax.left;
					wpl.ptMaxPosition.y = rcMax.top;
					SetWindowPlacement(ghWnd, &wpl);*/
					DWORD_PTR dwStyle = GetWindowLongPtr(ghWnd, GWL_STYLE);
					SetWindowStyle(dwStyle|WS_MAXIMIZE);
					SetWindowPos(ghWnd, HWND_TOP, 
						rcMax.left, rcMax.top, 
						rcMax.right-rcMax.left, rcMax.bottom-rcMax.top,
						SWP_NOCOPYBITS|SWP_SHOWWINDOW);
				}
				mb_IgnoreSizeChange = FALSE;

				//RePaint();
				InvalidateAll();

				if (gpSetCls->isAdvLogging) LogString("OnSize(false).2");

				//OnSize(false); // ������� ��� �������� ���� ������

				gpSetCls->UpdateWindowMode(wmMaximized);
			} // if (!gpSet->isHideCaption)

			if (!IsWindowVisible(ghWnd))
			{
				mb_IgnoreSizeChange = true;
				ShowWindow(SW_SHOWMAXIMIZED);
				mb_IgnoreSizeChange = FALSE;

				if (gpSetCls->isAdvLogging) LogString("OnSize(false).3");
			}

			// Already resored, need to clear the flag to avoid incorrect sizing
			isRestoreFromMinimized = false;

			OnSize(false); // ������� ��� �������� ���� ������

			UpdateWindowRgn();

		} break; //wmMaximized

		case wmFullScreen:
		{
			DEBUGLOGFILE("SetWindowMode(wmFullScreen)\n");

			RECT rcMax = CalcRect(CER_FULLSCREEN, MakeRect(0,0), CER_FULLSCREEN);

			WARNING("����� ���������� ��-�� ������������� ������� �������");
			// � ���� ������ ������ �� ���������� ����������� ��������� � �������������� �� � ConEmu
			if (!CVConGroup::PreReSize(inMode, rcMax))
			{
				mb_PassSysCommand = false;
				goto wrap;
			}

			//mb_isFullScreen = true;
			//WindowMode = inMode; // ��������!

			//120820 - �.�. FullScreen ������ SW_SHOWMAXIMIZED, �� ����� ����� �������� �� WindowMode
			isWndNotFSMaximized = (changeFromWindowMode == wmMaximized);
			
			RECT rcShift = CalcMargins(CEM_FRAMEONLY);
			_ASSERTE(rcShift.left==0 && rcShift.right==0 && rcShift.top==0 && rcShift.bottom==0);

			//GetCWShift(ghWnd, &rcShift); // ��������, �� ������ ������
			// ���������
			ptFullScreenSize.x = GetSystemMetrics(SM_CXSCREEN)+rcShift.left+rcShift.right;
			ptFullScreenSize.y = GetSystemMetrics(SM_CYSCREEN)+rcShift.top+rcShift.bottom;
			// ������� ����� �������� ��� �������� ��������!
			MONITORINFO mi; memset(&mi, 0, sizeof(mi)); mi.cbSize = sizeof(mi);
			HMONITOR hMon = MonitorFromWindow(ghWnd, MONITOR_DEFAULTTONEAREST);

			if (hMon)
			{
				if (GetMonitorInfo(hMon, &mi))
				{
					ptFullScreenSize.x = (mi.rcMonitor.right-mi.rcMonitor.left)+rcShift.left+rcShift.right;
					ptFullScreenSize.y = (mi.rcMonitor.bottom-mi.rcMonitor.top)+rcShift.top+rcShift.bottom;
				}
			}

			//120820 - ��� ����� ��������� �������� IsZoomed
			if (isIconic() || !::IsZoomed(ghWnd))
			{
	 				mb_IgnoreSizeChange = true;
				//120820 ��� ��������, � FullScreen ���� ������ Maximized, � �� Normal
				ShowWindow(SW_SHOWMAXIMIZED);

				//// ��������
				//if (mb_MaximizedHideCaption)
				//	mb_MaximizedHideCaption = FALSE;

				mb_IgnoreSizeChange = FALSE;
				//120820 - ������ �����������?
				//-- RePaint();
			}

			if (mp_TaskBar2)
			{
				if (!gpSet->isDesktopMode)
					mp_TaskBar2->MarkFullscreenWindow(ghWnd, TRUE);

				bWasSetFullscreen = true;
			}

			// for virtual screens mi.rcMonitor. may contains negative values...

			if (gpSetCls->isAdvLogging)
			{
				char szInfo[128]; wsprintfA(szInfo, "SetWindowPos(X=%i, Y=%i, W=%i, H=%i)", -rcShift.left+mi.rcMonitor.left,-rcShift.top+mi.rcMonitor.top, ptFullScreenSize.x,ptFullScreenSize.y);
				LogString(szInfo);
			}

			// Already resored, need to clear the flag to avoid incorrect sizing
			isRestoreFromMinimized = false;

			OnSize(false); // ���������� ������ �������� ������

			RECT rcFrame = CalcMargins(CEM_FRAMEONLY);
			// ptFullScreenSize �������� "�����������������" ������ (�� ������ ��������)
			UpdateWindowRgn(rcFrame.left, rcFrame.top,
				            mi.rcMonitor.right-mi.rcMonitor.left, mi.rcMonitor.bottom-mi.rcMonitor.top);
			/* */ SetWindowPos(ghWnd, NULL,
				                -rcShift.left+mi.rcMonitor.left,-rcShift.top+mi.rcMonitor.top,
				                ptFullScreenSize.x,ptFullScreenSize.y,
				                SWP_NOZORDER);

			gpSetCls->UpdateWindowMode(wmFullScreen);

			if (!IsWindowVisible(ghWnd))
			{
				mb_IgnoreSizeChange = true;
				ShowWindow(SW_SHOWNORMAL);
				mb_IgnoreSizeChange = FALSE;
				//WindowMode = inMode; // ��������!

				if (gpSetCls->isAdvLogging) LogString("OnSize(false).5");

				OnSize(false); // ���������� ������ �������� ������
			}

		} break; //wmFullScreen

	default:
		_ASSERTE(FALSE && "Unsupported mode");
	}

	if (gpSetCls->isAdvLogging) LogString("SetWindowMode done");

	//WindowMode = inMode; // ��������!

	//if (VCon.VCon())
	//{
	//	VCon->RCon()->SyncGui2Window();
	//}

	//if (ghOpWnd && (gpSetCls->mh_Tabs[CSettings::thi_SizePos]))
	//{
	//	bool canEditWindowSizes = (inMode == wmNormal);
	//	HWND hSizePos = gpSetCls->mh_Tabs[CSettings::thi_SizePos];
	//	EnableWindow(GetDlgItem(hSizePos, tWndWidth), canEditWindowSizes);
	//	EnableWindow(GetDlgItem(hSizePos, tWndHeight), canEditWindowSizes);
	//}

	if (gpSetCls->isAdvLogging)
	{
		wchar_t szInfo[64];
		_wsprintf(szInfo, SKIPLEN(countof(szInfo)) L"SetWindowMode(%u) end", inMode);
		LogWindowPos(szInfo);
	}

	//Sync ConsoleToWindow(); 2009-09-10 � ��� ����� ������ �� ����� - ������ ������� ��� ������
	mb_PassSysCommand = false;
	lbRc = true;
wrap:
	if (!lbRc)
	{
		if (gpSetCls->isAdvLogging)
		{
			LogString(L"Failed to switch WindowMode");
		}
		_ASSERTE(lbRc && "Failed to switch WindowMode");
		WindowMode = changeFromWindowMode;
	}
	changeFromWindowMode = wmNotChanging;
	isRestoreFromMinimized = false;

	// � ������ ������ ��������� ������� ������� - ����� ������� �������
	// ��������������� � ������ �����. ������ ���...
	if (mp_TaskBar2)
	{
		bNewFullScreen = (WindowMode == wmFullScreen) || (gpSet->isQuakeStyle && (gpSet->_WindowMode == wmFullScreen));
		if (bWasSetFullscreen != bNewFullScreen)
		{
			if (!gpSet->isDesktopMode)
				mp_TaskBar2->MarkFullscreenWindow(ghWnd, bNewFullScreen);

			bWasSetFullscreen = bNewFullScreen;
		}
	}

	mp_TabBar->OnWindowStateChanged();

	#ifdef _DEBUG
	dwStyle = GetWindowLongPtr(ghWnd, GWL_STYLE);
	#endif

	TODO("���-�� ������ ������ �������� APPSTARTING...");
	SetCursor(LoadCursor(NULL,IDC_ARROW));
	//PostMessage(ghWnd, WM_SETCURSOR, -1, -1);
	return lbRc;
}

void CConEmuMain::ForceShowTabs(BOOL abShow)
{
	//if (!mp_ VActive)
	//  return;
	//2009-05-20 ��� ��� Force - ������ �� ����������� �������� ���� �� ���� ��������! ��� ������� ������������ "Console"
	BOOL lbTabsAllowed = abShow /*&& gpConEmu->mp_TabBar->IsAllowed()*/;

	if (abShow && !gpConEmu->mp_TabBar->IsTabsShown() && gpSet->isTabs && lbTabsAllowed)
	{
		gpConEmu->mp_TabBar->Activate(TRUE);
		//ConEmuTab tab; memset(&tab, 0, sizeof(tab));
		//tab.Pos=0;
		//tab.Current=1;
		//tab.Type = 1;
		//gpConEmu->mp_TabBar->Update(&tab, 1);
		//mp_ VActive->RCon()->SetTabs(&tab, 1);
		gpConEmu->mp_TabBar->Update();
		//gbPostUpdateWindowSize = true; // 2009-07-04 Resize ��������� ��� TabBar
	}
	else if (!abShow)
	{
		gpConEmu->mp_TabBar->Deactivate(TRUE);
		//gbPostUpdateWindowSize = true; // 2009-07-04 Resize ��������� ��� TabBar
	}

	// ����� Gaps �� ����������
	gpConEmu->InvalidateAll();
	UpdateWindowRgn();
	// ��� ����������� ����� ����� �������� "[n/n] " � ��� ����������� - ��������
	UpdateTitle(); // ��� ����������
	// 2009-07-04 Resize ��������� ��� TabBar
	//if (gbPostUpdateWindowSize) { // ������ �� ���-�� ��������
	//    ReSize();
	//    /*RECT rcNewCon; Get ClientRect(ghWnd, &rcNewCon);
	//    DCClientRect(&rcNewCon);
	//    MoveWindow(ghWnd DC, rcNewCon.left, rcNewCon.top, rcNewCon.right - rcNewCon.left, rcNewCon.bottom - rcNewCon.top, 0);
	//    dcWindowLast = rcNewCon;
	//
	//    if (gpSet->LogFont.lfWidth)
	//    {
	//        Sync ConsoleToWindow();
	//    }*/
	//}
}

bool CConEmuMain::isIconic(bool abRaw /*= false*/)
{
	bool bIconic = ::IsIconic(ghWnd);
	if (!abRaw && !bIconic)
	{
		bIconic = (gpSet->isQuakeStyle && isQuakeMinimized);
	}
	return bIconic;
}

bool CConEmuMain::isWindowNormal()
{
	#ifdef _DEBUG
	bool bZoomed = ::IsZoomed(ghWnd);
	bool bIconic = ::IsIconic(ghWnd);
	_ASSERTE((WindowMode == wmNormal) || bZoomed || bIconic || (changeFromWindowMode == wmNormal) || mb_InCreateWindow);
	#endif

	if ((WindowMode != wmNormal) || gpSet->isQuakeStyle || m_InsideIntegration)
		return false;

	if (::IsIconic(ghWnd))
		return false;

	return true;
}

bool CConEmuMain::isZoomed()
{
	#ifdef _DEBUG
	bool bZoomed = ::IsZoomed(ghWnd);
	bool bIconic = ::IsIconic(ghWnd);
	_ASSERTE((WindowMode == wmNormal) || bZoomed || bIconic || (changeFromWindowMode == wmNormal) || mb_InCreateWindow);
	#endif

	if (WindowMode != wmMaximized)
		return false;
	if (::IsIconic(ghWnd))
		return false;

	return true;
}

bool CConEmuMain::isFullScreen()
{
	#ifdef _DEBUG
	bool bZoomed = ::IsZoomed(ghWnd);
	bool bIconic = ::IsIconic(ghWnd);
	_ASSERTE((WindowMode == wmNormal) || bZoomed || bIconic || (changeFromWindowMode == wmNormal) || mb_InCreateWindow);
	#endif

	if (WindowMode != wmFullScreen)
		return false;
	if (::IsIconic(ghWnd))
		return false;

	return true;
}

// abCorrect2Ideal==TRUE �������� �� TabBarClass::UpdatePosition(),
// ��� ���������� ��� �����������/������� "���������"
// ��� ��� ����� ������
void CConEmuMain::ReSize(BOOL abCorrect2Ideal /*= FALSE*/)
{
	if (isIconic())
		return;

	RECT client = GetGuiClientRect();

	#ifdef _DEBUG
	DWORD_PTR dwStyle = GetWindowLongPtr(ghWnd, GWL_STYLE);
	#endif

	if (abCorrect2Ideal)
	{
		if (isWindowNormal())
		{
			// ������-�� ���������� ������ X,Y
			RECT rcWnd; GetWindowRect(ghWnd, &rcWnd);

			// ������������ �������� �� ����� ������� �������

			#if 0
			// -- �� �������. ��� ������� ����� ������� � ���������� �����
			// -- ������ "CVConGroup::AllTextRect()" --> {0x0}
			RECT rcConsole = {}, rcCompWnd = {};
			TODO("DoubleView: ����� � ������ ������� ���������");
			if (isVConExists(0))
			{
				rcConsole = CVConGroup::AllTextRect();
				rcCompWnd = CalcRect(CER_MAIN, rcConsole, CER_CONSOLE_ALL);
				//AutoSizeFont(rcCompWnd, CER_MAIN);
			}
			else
			{
				rcCompWnd = rcWnd; // �� ������?
			}
			#endif

			#if 1
			// ��������� ������, ���� ���� ������ ��� ������������...
			// ��� ����� DoubleView/SplitScreen
			RECT rcIdeal = GetIdealRect();
			AutoSizeFont(rcIdeal, CER_MAIN);
			RECT rcConsole = CalcRect(CER_CONSOLE_ALL, rcIdeal, CER_MAIN);
			RECT rcCompWnd = CalcRect(CER_MAIN, rcConsole, CER_CONSOLE_ALL);
			#endif

			// ��� ������/������� ����� ������ ������� ����� "�������"
			// �� ����� ���������������. ��������� ���� �������� ������
			// �������� ���� - OnSize ���������� �������������
			_ASSERTE(isMainThread());

			CVConGroup::PreReSize(WindowMode, rcCompWnd, CER_MAIN, true);

			//#ifdef _DEBUG
			//DebugStep(L"...Sleeping");
			//Sleep(300);
			//DebugStep(NULL);
			//#endif
			MoveWindow(ghWnd, rcWnd.left, rcWnd.top,
			           (rcCompWnd.right - rcCompWnd.left), (rcCompWnd.bottom - rcCompWnd.top), 1);
		}
		else
		{
			CVConGroup::PreReSize(WindowMode, client, CER_MAINCLIENT, true);
		}

		// ���������! ����� ����������!
		client = GetGuiClientRect();
	}

#ifdef _DEBUG
	dwStyle = GetWindowLongPtr(ghWnd, GWL_STYLE);
#endif
	OnSize(true, isZoomed() ? SIZE_MAXIMIZED : SIZE_RESTORED,
	       client.right, client.bottom);

	if (abCorrect2Ideal)
	{
	}
}

//void CConEmuMain::ResizeChildren()
//{
//	CVirtualConsole* pVCon = gpConEmu->ActiveCon();
//	_ASSERTE(isMainThread());
//	if (!pVCon)
//	{
//		// �� ����, �� ������ ����������, ���� �������� ������ ���
//		_ASSERTE(pVCon!=NULL);
//		return;
//	}
//	CVConGuard guard(pVCon);
//
//	//RECT rc = gpConEmu->ConsoleOffsetRect();
//	RECT rcClient; Get ClientRect(ghWnd, &rcClient);
//	bool bTabsShown = gpConEmu->mp_TabBar->IsTabsShown();
//
//	static bool bTabsShownLast, bAlwaysScrollLast;
//	static RECT rcClientLast;
//
//	if (bTabsShownLast == bTabsShown && bAlwaysScrollLast == (gpSet->isAlwaysShowScrollbar == 1))
//	{
//		if (memcmp(&rcClient, &rcClientLast, sizeof(RECT))==0)
//		{
//			#if defined(EXT_GNUC_LOG)
//			char szDbg[255];
//			wsprintfA(szDbg, "  --  CConEmuBack::Resize() - exiting, (%i,%i,%i,%i)==(%i,%i,%i,%i)",
//				rcClient.left, rcClient.top, rcClient.right, rcClient.bottom,
//				rcClientLast.left, rcClientLast.top, rcClientLast.right, rcClientLast.bottom);
//
//			if (gpSetCls->isAdvLogging>1)
//				pVCon->RCon()->LogString(szDbg);
//			#endif
//
//			return; // ������ �� ��������
//		}
//	}
//
//	rcClientLast = rcClient;
//	//memmove(&mrc_LastClient, &rcClient, sizeof(RECT)); // ����� ��������
//	bTabsShownLast = bTabsShown;
//	bAlwaysScrollLast = (gpSet->isAlwaysShowScrollbar == 1);
//	//RECT rcScroll; GetWindowRect(mh_WndScroll, &rcScroll);
//	RECT rc = gpConEmu->CalcRect(CER_BACK, rcClient, CER_MAINCLIENT);
//
//
//	#if defined(EXT_GNUC_LOG)
//	char szDbg[255]; wsprintfA(szDbg, "  --  CConEmuBack::Resize() - X=%i, Y=%i, W=%i, H=%i", rc.left, rc.top, 	rc.right - rc.left,	rc.bottom - rc.top);
//
//	if (gpSetCls->isAdvLogging>1)
//		pVCon->RCon()->LogString(szDbg);
//	#endif
//
//	// ��� ������� ���� ���������. ��� ������ ���������������
//	// ������� ����������� �������. �� ��� ���� ������������� �������!
//	WARNING("DoubleView");
//	rc = gpConEmu->CalcRect(CER_SCROLL, rcClient, CER_MAINCLIENT);
//
//
//	#ifdef _DEBUG
//	if (rc.bottom != rcClient.bottom || rc.right != rcClient.right)
//	{
//		_ASSERTE(rc.bottom == rcClient.bottom && rc.right == rcClient.right);
//	}
//	#endif
//}

// ������ ������� TRUE, ���� ������� ���� �� ����� ���������� � �������
BOOL CConEmuMain::TrackMouse()
{
	BOOL lbCapture = FALSE; // �� ��������� - ���� �� �������������

	TODO("DoubleView: ���������� �� ��������� � ������� ��������");
	CVConGuard VCon;
	if (GetActiveVCon(&VCon) >= 0)
	{
		if (VCon->TrackMouse())
			lbCapture = TRUE;
	}

	return lbCapture;
}

void CConEmuMain::OnAlwaysShowScrollbar(bool abSync /*= true*/)
{
	CVConGroup::OnAlwaysShowScrollbar(abSync);
}

void CConEmuMain::OnConsoleResize(BOOL abPosted/*=FALSE*/)
{
	TODO("�� ��������. ConEmu �� ������ ��������� ��� ����� ������� �� �������");
	//MSetter lInConsoleResize(&mb_InConsoleResize);
	// ����������� ������ � ���� ����, ����� ����� ���������
	static bool lbPosted = false;
	abPosted = (mn_MainThreadId == GetCurrentThreadId());

	if (!abPosted)
	{
		if (gpSetCls->isAdvLogging)
			CVConGroup::LogString("OnConsoleResize(abPosted==false)", TRUE);

		if (!lbPosted)
		{
			lbPosted = true; // ����� post �� �������������
			#ifdef _DEBUG
			int nCurConWidth = (int)CVConGroup::TextWidth();
			int nCurConHeight = (int)CVConGroup::TextHeight();
			#endif
			PostMessage(ghWnd, mn_PostConsoleResize, 0,0);
		}

		return;
	}

	lbPosted = false;

	if (isIconic())
	{
		if (gpSetCls->isAdvLogging)
			CVConGroup::LogString("OnConsoleResize ignored, because of iconic");

		return; // ���� �������������� - ������ �� ������
	}

	// ���� �� �������� ��������� ��������?
	BOOL lbSizingToDo  = (mouse.state & MOUSE_SIZING_TODO) == MOUSE_SIZING_TODO;
	bool lbIsSizing = isSizing();
	bool lbLBtnPressed = isPressed(VK_LBUTTON);

	if (lbIsSizing && !lbLBtnPressed)
	{
		// ����� ���� ������ ������� ������
		mouse.state &= ~(MOUSE_SIZING_BEGIN|MOUSE_SIZING_TODO);
	}

	if (gpSetCls->isAdvLogging)
	{
		char szInfo[160]; wsprintfA(szInfo, "OnConsoleResize: mouse.state=0x%08X, SizingToDo=%i, IsSizing=%i, LBtnPressed=%i, gbPostUpdateWindowSize=%i",
		                            mouse.state, (int)lbSizingToDo, (int)lbIsSizing, (int)lbLBtnPressed, (int)gbPostUpdateWindowSize);
		CVConGroup::LogString(szInfo, TRUE);
	}

	CVConGroup::OnConsoleResize(lbSizingToDo);
}

//bool CConEmuMain::CorrectWindowPos(WINDOWPOS *wp)
//{
//	bool lbChanged = false;
//
//	// wp->flags != (SWP_NOMOVE|SWP_NOSIZE)
//	if ((gpSet->isHideCaption || gpSet->isHideCaptionAlways()) && ((wp->flags&3)!=3) && isZoomed())
//	{
//		RECT rcShift = CalcMargins(CEM_FRAME);
//		MONITORINFO mi = {sizeof(mi)};
//		HMONITOR hMon = MonitorFromWindow(ghWnd, MONITOR_DEFAULTTONEAREST);
//
//		if ((wp->x != (-rcShift.left+mi.rcWork.left)) || (wp->y != (-rcShift.top+mi.rcWork.top)))
//		{
//			wp->x = (-rcShift.left+mi.rcWork.left);
//			wp->y = (-rcShift.top+mi.rcWork.top);
//			TODO("�������� � ������ ��������� ����?");
//			lbChanged = true;
//		}
//
//		//ptFullScreenSize.x,ptFullScreenSize.y,
//	}
//	return lbChanged;
//}

LRESULT CConEmuMain::OnSize(bool bResizeRCon/*=true*/, WPARAM wParam/*=0*/, WORD newClientWidth/*=(WORD)-1*/, WORD newClientHeight/*=(WORD)-1*/)
{
	LRESULT result = 0;
#ifdef _DEBUG
	RECT rcDbgSize; GetWindowRect(ghWnd, &rcDbgSize);
	wchar_t szSize[255]; _wsprintf(szSize, SKIPLEN(countof(szSize)) L"OnSize(%u, %i, %ix%i) Current window size (X=%i, Y=%i, W=%i, H=%i)\n",
	                               (int)bResizeRCon, (DWORD)wParam, (int)(short)newClientWidth, (int)(short)newClientHeight,
	                               rcDbgSize.left, rcDbgSize.top, (rcDbgSize.right-rcDbgSize.left), (rcDbgSize.bottom-rcDbgSize.top));
	DEBUGSTRSIZE(szSize);
#endif

	if (wParam == SIZE_MINIMIZED || isIconic())
	{
		return 0;
	}

	if (mb_IgnoreSizeChange)
	{
		// �� ����� ��������� WM_SYSCOMMAND
		return 0;
	}

	if (mn_MainThreadId != GetCurrentThreadId())
	{
		//MBoxAssert(mn_MainThreadId == GetCurrentThreadId());
		PostMessage(ghWnd, WM_SIZE, MAKELPARAM(wParam,(bResizeRCon?1:2)), MAKELONG(newClientWidth,newClientHeight));
		return 0;
	}

	UpdateWindowRgn();

#ifdef _DEBUG
	DWORD_PTR dwStyle = GetWindowLongPtr(ghWnd, GWL_STYLE);
#endif
	//if (mb_InResize) {
	//	_ASSERTE(!mb_InResize);
	//	PostMessage(ghWnd, WM_SIZE, wParam, MAKELONG(newClientWidth,newClientHeight));
	//	return 0;
	//}
	mn_InResize++;


	if (newClientWidth==(WORD)-1 || newClientHeight==(WORD)-1)
	{
		RECT rcClient = GetGuiClientRect();
		newClientWidth = rcClient.right;
		newClientHeight = rcClient.bottom;
	}

	RECT mainClient = MakeRect(newClientWidth,newClientHeight);
	RECT work = CalcRect(CER_WORKSPACE, mainClient, CER_MAINCLIENT);
	_ASSERTE(ghWndWork && GetParent(ghWndWork)==ghWnd); // ���� ��������� �� �������� �����
	MoveWindow(ghWndWork, work.left, work.top, work.right-work.left, work.bottom-work.top, TRUE);

	// ��������� "���������" ������ ����, ��������� �������������
	if (isSizing() && isWindowNormal())
	{
		//GetWindowRect(ghWnd, &mrc_Ideal);
		//UpdateIdealRect();
		StoreIdealRect();
	}

	if (gpConEmu->mp_TabBar->IsTabsActive())
		gpConEmu->mp_TabBar->Reposition();

	// Background - ������ ������ ��� ���������� ����� ��� ��������
	// ��� �� ���������� ScrollBar
	//ResizeChildren();

	BOOL lbIsPicView = isPictureView();		UNREFERENCED_PARAMETER(lbIsPicView);

	if (bResizeRCon && (changeFromWindowMode == wmNotChanging) && (mn_InResize <= 1))
	{
		CVConGroup::SyncAllConsoles2Window(mainClient, CER_MAINCLIENT, true);
	}

	if (CVConGroup::isVConExists(0))
	{
		CVConGroup::ReSizePanes(mainClient);
	}

	InvalidateGaps();

	if (mn_InResize>0)
		mn_InResize--;

	#ifdef _DEBUG
	dwStyle = GetWindowLongPtr(ghWnd, GWL_STYLE);
	#endif
	return result;
}

LRESULT CConEmuMain::OnSizing(WPARAM wParam, LPARAM lParam)
{
	LRESULT result = true;
#if defined(EXT_GNUC_LOG)
	char szDbg[255];
	wsprintfA(szDbg, "CConEmuMain::OnSizing(wParam=%i, L.Lo=%i, L.Hi=%i)\n",
	          wParam, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));

	if (gpSetCls->isAdvLogging)
		LogString(szDbg);

#endif
#ifndef _DEBUG

	if (isPictureView())
	{
		RECT *pRect = (RECT*)lParam; // � ������
		*pRect = mrc_WndPosOnPicView;
		//pRect->right = pRect->left + (mrc_WndPosOnPicView.right-mrc_WndPosOnPicView.left);
		//pRect->bottom = pRect->top + (mrc_WndPosOnPicView.bottom-mrc_WndPosOnPicView.top);
	}
	else
#endif
		if (mb_IgnoreSizeChange)
		{
			// �� ����� ��������� WM_SYSCOMMAND
		}
		else if (isNtvdm())
		{
			// �� ������ ��� 16��� ����������
		}
		else if (isWindowNormal())
		{
			RECT srctWindow;
			RECT wndSizeRect, restrictRect, calcRect;
			RECT *pRect = (RECT*)lParam; // � ������
			RECT rcCurrent; GetWindowRect(ghWnd, &rcCurrent);

			if ((mouse.state & (MOUSE_SIZING_BEGIN|MOUSE_SIZING_TODO))==MOUSE_SIZING_BEGIN
			        && isPressed(VK_LBUTTON))
			{
				mouse.state |= MOUSE_SIZING_TODO;
			}

			wndSizeRect = *pRect;
			// ��� ���������� ����� ��� ������
			LONG nWidth = gpSetCls->FontWidth(), nHeight = gpSetCls->FontHeight();

			if (nWidth && nHeight)
			{
				wndSizeRect.right += (nWidth-1)/2;
				wndSizeRect.bottom += (nHeight-1)/2;
			}

			bool bNeedFixSize = gpSet->isIntegralSize();

			// ���������� �������� ������ �������
			//srctWindow = ConsoleSizeFromWindow(&wndSizeRect, true /* frameIncluded */);
			AutoSizeFont(wndSizeRect, CER_MAIN);
			srctWindow = CalcRect(CER_CONSOLE_ALL, wndSizeRect, CER_MAIN);

			// ���������� ���������� ������� �������
			if (srctWindow.right < MIN_CON_WIDTH)
			{
				srctWindow.right = MIN_CON_WIDTH;
				bNeedFixSize = true;
			}

			if (srctWindow.bottom < MIN_CON_HEIGHT)
			{
				srctWindow.bottom = MIN_CON_HEIGHT;
				bNeedFixSize = true;
			}


			if (bNeedFixSize)
			{
				calcRect = CalcRect(CER_MAIN, srctWindow, CER_CONSOLE_ALL);
				#ifdef _DEBUG
				RECT rcRev = CalcRect(CER_CONSOLE_ALL, calcRect, CER_MAIN);
				_ASSERTE(rcRev.right==srctWindow.right && rcRev.bottom==srctWindow.bottom);
				#endif
				restrictRect.right = pRect->left + calcRect.right;
				restrictRect.bottom = pRect->top + calcRect.bottom;
				restrictRect.left = pRect->right - calcRect.right;
				restrictRect.top = pRect->bottom - calcRect.bottom;

				switch(wParam)
				{
					case WMSZ_RIGHT:
					case WMSZ_BOTTOM:
					case WMSZ_BOTTOMRIGHT:
						pRect->right = restrictRect.right;
						pRect->bottom = restrictRect.bottom;
						break;
					case WMSZ_LEFT:
					case WMSZ_TOP:
					case WMSZ_TOPLEFT:
						pRect->left = restrictRect.left;
						pRect->top = restrictRect.top;
						break;
					case WMSZ_TOPRIGHT:
						pRect->right = restrictRect.right;
						pRect->top = restrictRect.top;
						break;
					case WMSZ_BOTTOMLEFT:
						pRect->left = restrictRect.left;
						pRect->bottom = restrictRect.bottom;
						break;
				}
			}

			// ��� ����� ������� (���� ������ �� ������)
			if ((pRect->right - pRect->left) != (rcCurrent.right - rcCurrent.left)
			        || (pRect->bottom - pRect->top) != (rcCurrent.bottom - rcCurrent.top))
			{
				// ����� ������������ �������, ����� ��� WM_PAINT ����� ���� ���������� ��� ������� ������
				TODO("DoubleView");
				//ActiveCon()->RCon()->SyncConsole2Window(FALSE, pRect);
				#ifdef _DEBUG
				wchar_t szSize[255]; _wsprintf(szSize, SKIPLEN(countof(szSize)) L"New window size (X=%i, Y=%i, W=%i, H=%i); Current size (X=%i, Y=%i, W=%i, H=%i)\n",
				                               pRect->left, pRect->top, (pRect->right-pRect->left), (pRect->bottom-pRect->top),
				                               rcCurrent.left, rcCurrent.top, (rcCurrent.right-rcCurrent.left), (rcCurrent.bottom-rcCurrent.top));
				DEBUGSTRSIZE(szSize);
				#endif
			}
		}

	if (gpSetCls->isAdvLogging)
		LogWindowPos(L"OnSizing.end");

	return result;
}

LRESULT CConEmuMain::OnMoving(LPRECT prcWnd /*= NULL*/, bool bWmMove /*= false*/)
{
	if (!gpSet->isSnapToDesktopEdges)
		return FALSE;

	if (isIconic() || isZoomed() || isFullScreen() || gpSet->isQuakeStyle)
		return FALSE;

	HMONITOR hMon;
	if (bWmMove && isPressed(VK_LBUTTON))
	{
		// ���� ������� ������ - �� ���� ����������� �������� �� ������� ��� ������
		POINT ptCur = {}; GetCursorPos(&ptCur);
		hMon = MonitorFromPoint(ptCur, MONITOR_DEFAULTTONEAREST);
	}
	else if (prcWnd)
	{
		hMon = MonitorFromRect(prcWnd, MONITOR_DEFAULTTONEAREST);
	}
	else
	{
		hMon = MonitorFromWindow(ghWnd, MONITOR_DEFAULTTONEAREST);
	}

	MONITORINFO mi = {sizeof(mi)};
	if (!GetMonitorInfo(hMon, &mi))
		return FALSE;

	RECT rcShift = {};
	if (gpSet->isHideCaptionAlways())
	{
		rcShift = CalcMargins(CEM_FRAMECAPTION);
		mi.rcWork.left -= rcShift.left;
		mi.rcWork.top -= rcShift.top;
		mi.rcWork.right += rcShift.right;
		mi.rcWork.bottom += rcShift.bottom;
	}

	RECT rcCur = {};
	if (prcWnd)
		rcCur = *prcWnd;
	else
		GetWindowRect(ghWnd, &rcCur);

	int nWidth = rcCur.right - rcCur.left;
	int nHeight = rcCur.bottom - rcCur.top;

	RECT rcWnd = {};
	rcWnd.left = max(mi.rcWork.left, min(rcCur.left, mi.rcWork.right - nWidth));
	rcWnd.top = max(mi.rcWork.top, min(rcCur.top, mi.rcWork.bottom - nHeight));
	rcWnd.right = min(mi.rcWork.right, rcWnd.left + nWidth);
	rcWnd.bottom = min(mi.rcWork.bottom, rcWnd.top + nHeight);
	
	if (memcmp(&rcWnd, &rcCur, sizeof(rcWnd)) == 0)
		return FALSE;

	if (prcWnd == NULL)
	{
		TODO("Desktop mode?");
		MoveWindow(ghWnd, rcWnd.left, rcWnd.right, nWidth+1, nHeight+1, TRUE);

		if (gpSetCls->isAdvLogging)
			LogWindowPos(L"OnMoving.end");
	}
	else
	{
		*prcWnd = rcWnd;
	}

	return TRUE;
}

void CConEmuMain::CheckTopMostState()
{
	static bool bInCheck = false;
	if (bInCheck)
		return;

	bInCheck = true;

	DWORD dwStyleEx = GetWindowLong(ghWnd, GWL_EXSTYLE);

	if (!gpSet->isAlwaysOnTop && ((dwStyleEx & WS_EX_TOPMOST) == WS_EX_TOPMOST))
	{
		if (IDYES == MessageBox(L"Some external program bring ConEmu OnTop\nRevert?", MB_SYSTEMMODAL|MB_ICONQUESTION|MB_YESNO))
		{
	        //SetWindowStyleEx(dwStyleEx & ~WS_EX_TOPMOST);
			OnAlwaysOnTop();
		}
		else
		{
			gpSet->isAlwaysOnTop = true;
		}
	}

	bInCheck = false;
}

// ��� IntegralSize - �������� CConEmuMain::OnSizing
LRESULT CConEmuMain::OnWindowPosChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0; // DefWindowProc ������� � �������� �������

	// ���� ����� ��������� ��������� DWM
	gpConEmu->ExtendWindowFrame();

	//static int WindowPosStackCount = 0;
	WINDOWPOS *p = (WINDOWPOS*)lParam;
	DWORD dwStyle = GetWindowLong(ghWnd, GWL_STYLE);
	DWORD dwStyleEx = GetWindowLong(ghWnd, GWL_EXSTYLE);

	if (gpSetCls->isAdvLogging >= 2)
	{
		wchar_t szInfo[255];
		wcscpy_c(szInfo, L"OnWindowPosChanged:");
		if (dwStyle & WS_MAXIMIZE) wcscat_c(szInfo, L" (zoomed)");
		if (dwStyle & WS_MINIMIZE) wcscat_c(szInfo, L" (iconic)");
		size_t cchLen = wcslen(szInfo);
		_wsprintf(szInfo+cchLen, SKIPLEN(countof(szInfo)-cchLen) L" x%08X x%08X (F:x%08X X:%i Y:%i W:%i H:%i)", dwStyle, dwStyleEx, p->flags, p->x, p->y, p->cx, p->cy);
		LogString(szInfo);
	}

	#ifdef _DEBUG
	static int cx, cy;

	if (!(p->flags & SWP_NOSIZE) && (cx != p->cx || cy != p->cy))
	{
		cx = p->cx; cy = p->cy;
	}

	// ����� ����������� ��������� "AlwaysOnTop"
	//static bool bWasTopMost = false;
	//_ASSERTE(((p->flags & SWP_NOZORDER) || (p->hwndInsertAfter!=HWND_TOPMOST)) && "OnWindowPosChanged");
	//_ASSERTE(((dwStyleEx & WS_EX_TOPMOST) == 0) && "OnWindowPosChanged");
	//bWasTopMost = ((dwStyleEx & WS_EX_TOPMOST)==WS_EX_TOPMOST);
	#endif

	wchar_t szDbg[128]; _wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"WM_WINDOWPOSCHANGED ({%i-%i}x{%i-%i} Flags=0x%08X), style=0x%08X\n", p->x, p->y, p->cx, p->cy, p->flags, dwStyle);
	DEBUGSTRSIZE(szDbg);
	//WindowPosStackCount++;

	
	if (!gpSet->isAlwaysOnTop && ((dwStyleEx & WS_EX_TOPMOST) == WS_EX_TOPMOST))
	{
		CheckTopMostState();
		//_ASSERTE(((dwStyleEx & WS_EX_TOPMOST) == 0) && "Determined TopMost in OnWindowPosChanged");
		//SetWindowStyleEx(dwStyleEx & ~WS_EX_TOPMOST);
	}

	//if (WindowPosStackCount == 1)
	//{
	//	#ifdef _DEBUG
	//	bool bNoMove = (p->flags & SWP_NOMOVE); 
	//	bool bNoSize = (p->flags & SWP_NOSIZE);
	//	#endif

	//	if (gpConEmu->CorrectWindowPos(p))
	//	{
	//		MoveWindow(ghWnd, p->x, p->y, p->cx, p->cy, TRUE);
	//	}
	//}

	// ����� ����� �� ��������� ������� WM_SIZE/WM_MOVE
	result = DefWindowProc(hWnd, uMsg, wParam, lParam);

	//WindowPosStackCount--;

	if (hWnd == ghWnd /*&& ghOpWnd*/)  //2009-05-08 ���������� wndX/wndY ������, � �� ������ ���� ���� �������� �������
	{
		if (!gpConEmu->mb_IgnoreSizeChange && !gpConEmu->isIconic())
		{
			RECT rc; GetWindowRect(ghWnd, &rc);
			mp_Status->OnWindowReposition(&rc);

			if (isWindowNormal())
			{
				StoreNormalRect(&rc);
			}

			if (gpConEmu->hPictureView)
			{
				gpConEmu->mrc_WndPosOnPicView = rc;
			}

			//else
			//{
			//	TODO("����������, ����� ����� ������ PicView �� ����");
			//	if (!(p->flags & SWP_NOSIZE))
			//	{
			//		RECT rcWnd = {0,0,p->cx,p->cy};
			//		ActiveCon()->RCon()->SyncConsole2Window(FALSE, &rcWnd);
			//	}
			//}
		}
		else
		{
			mp_Status->OnWindowReposition(NULL);
		}
	}
	else if (gpConEmu->hPictureView)
	{
		GetWindowRect(ghWnd, &gpConEmu->mrc_WndPosOnPicView);
	}

	// ���� ������ ������������� �������� Win+Down (Win7) �� SC_MINIMIZE �� ��������
	if (gpSet->isMinToTray() && isIconic(true) && IsWindowVisible(ghWnd))
	{
		Icon.HideWindowToTray();
	}

	//�����������, ��� ����������
	if (gpSetCls->isAdvLogging)
	{
		LogWindowPos(L"WM_WINDOWPOSCHANGED.end");
	}

	return result;
}

// ��� IntegralSize - �������� CConEmuMain::OnSizing
LRESULT CConEmuMain::OnWindowPosChanging(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	WINDOWPOS *p = (WINDOWPOS*)lParam;

	bool zoomed = ::IsZoomed(ghWnd);
	bool iconic = ::IsIconic(ghWnd);

	DWORD dwStyle = GetWindowLong(ghWnd, GWL_STYLE);
	DWORD dwStyleEx = GetWindowLong(ghWnd, GWL_EXSTYLE);


	#ifdef _DEBUG
	_ASSERTE(zoomed == ((dwStyle & WS_MAXIMIZE) == WS_MAXIMIZE));
	_ASSERTE(iconic == ((dwStyle & WS_MINIMIZE) == WS_MINIMIZE));
	#endif


	wchar_t szInfo[255];
	if (gpSetCls->isAdvLogging >= 2)
	{
		wcscpy_c(szInfo, L"OnWindowPosChanging:");
		if (zoomed) wcscat_c(szInfo, L" (zoomed)");
		if (iconic) wcscat_c(szInfo, L" (iconic)");
		size_t cchLen = wcslen(szInfo);
		_wsprintf(szInfo+cchLen, SKIPLEN(countof(szInfo)-cchLen) L" x%08X x%08X (F:x%08X X:%i Y:%i W:%i H:%i)", dwStyle, dwStyleEx, p->flags, p->x, p->y, p->cx, p->cy);
	}


	//#ifdef _DEBUG
	#if 1
	{
		wchar_t szDbg[128]; _wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"WM_WINDOWPOSCHANGING ({%i-%i}x{%i-%i} Flags=0x%08X) style=0x%08X%s\n", p->x, p->y, p->cx, p->cy, p->flags, dwStyle, zoomed ? L" (zoomed)" : L"");
		DEBUGSTRSIZE(szDbg);
		DWORD nCurTick = GetTickCount();
		static int cx, cy;

		if (!(p->flags & SWP_NOSIZE) && (cx != p->cx || cy != p->cy))
		{
			cx = p->cx; cy = p->cy;
		}

			#ifdef CATCH_TOPMOST_SET
			// ����� ����������� ��������� "AlwaysOnTop"
			static bool bWasTopMost = false;
			_ASSERTE(((p->flags & SWP_NOZORDER) || (p->hwndInsertAfter!=HWND_TOPMOST)) && "OnWindowPosChanging");
			_ASSERTE(((dwStyleEx & WS_EX_TOPMOST) == 0) && "OnWindowPosChanging");
			_ASSERTE((bWasTopMost || gpSet->isAlwaysOnTop || ((dwStyleEx & WS_EX_TOPMOST)==0)) && "TopMost mode detected in OnWindowPosChanging");
			bWasTopMost = ((dwStyleEx & WS_EX_TOPMOST)==WS_EX_TOPMOST);
			#endif
	}
	#endif

	if (m_FixPosAfterStyle)
	{
		DWORD nCurTick = GetTickCount();

		if ((nCurTick - m_FixPosAfterStyle) < FIXPOSAFTERSTYLE_DELTA)
		{
			p->flags &= ~(SWP_NOMOVE|SWP_NOSIZE);
			p->x = mrc_FixPosAfterStyle.left;
			p->y = mrc_FixPosAfterStyle.top;
			p->cx = mrc_FixPosAfterStyle.right - mrc_FixPosAfterStyle.left;
			p->cy = mrc_FixPosAfterStyle.bottom - mrc_FixPosAfterStyle.top;
		}
		m_FixPosAfterStyle = 0;
	}

	//120821 - ������ ��� ���� ������� ����� (Win+Up ��������)
	//120830 - ������ ���� �� "Minimized"
	if (!iconic
		&& !mb_InCaptionChange && !(p->flags & (SWP_NOSIZE|SWP_NOMOVE))
		&& (changeFromWindowMode == wmNotChanging) && !gpSet->isQuakeStyle)
	{
		// � ���� ������ ����� ���������, ����� ���� ��������� ��������� ������ ����?
		if (zoomed != (WindowMode == wmMaximized || WindowMode == wmFullScreen))
		{
			// � ������� Maximized/FullScreen ������ ���� WS_MAXIMIZED
			WindowMode = zoomed ? wmMaximized : wmNormal;
			// ��������� WindowMode �� �������� ��� ����������� ��������
			// �� ����� ������, ���������� ��������, � �.�.
			OnHideCaption();
		}
	}
	
	// -- // ���� � ��� ����� ������� ��������� (��� ������������/���������)
	// ��� ����� �����. �.�. �� ������ WM_GETMINMAXINFO - ����� �������������� � ������� :(
	// ����� �������� �����
	if (!(p->flags & (SWP_NOSIZE|SWP_NOMOVE)) && !mb_InScMinimize)
	{
		if (gpSet->isQuakeStyle)
		{
			RECT rc = GetDefaultRect();
			p->x = rc.left;
			p->y = rc.top;
			p->cx = rc.right - rc.left; // + 1;
		}
		else if (zoomed)
		{
			RECT rc = CalcRect((WindowMode == wmFullScreen) ? CER_FULLSCREEN : CER_MAXIMIZED, NULL);
			p->x = rc.left;
			p->y = rc.top;
			p->cx = rc.right - rc.left;
			p->cy = rc.bottom - rc.top;
		}
		else // normal only
		{
			_ASSERTE(GetWindowMode() == wmNormal);
			RECT rc = {p->x, p->y, p->x + p->cx, p->y + p->cy};
			if (FixWindowRect(rc, CEB_ALL|CEB_ALLOW_PARTIAL))
			{
				p->x = rc.left;
				p->y = rc.top;
				p->cx = rc.right - rc.left;
				p->cy = rc.bottom - rc.top;
			}
		}
	//#ifdef _DEBUG
	#if 0
		else if (isWindowNormal())
		{
			HMONITOR hMon = NULL;
			RECT rcNew = {}; GetWindowRect(ghWnd, &rcNew);
			if (!(p->flags & SWP_NOSIZE) && !(p->flags & SWP_NOMOVE))
			{
				// �������� � ������ � ���������
				rcNew = MakeRect(p->x, p->y, p->x+p->cx, p->y+p->cy);
			}
			else if (!(p->flags & SWP_NOSIZE))
			{
				// �������� ������ ������
				rcNew.right = rcNew.left + p->cx;
				rcNew.bottom = rcNew.top + p->cy;
			}
			else
			{
				// �������� ������ ���������
				rcNew = MakeRect(p->x, p->y, rcNew.right-rcNew.left, rcNew.bottom-rcNew.top);
			}

			hMon = MonitorFromRect(&rcNew, MONITOR_DEFAULTTONEAREST);
			MONITORINFO mi = {sizeof(mi)};
			if (GetMonitorInfo(hMon, &mi))
			{
				//if (WindowMode == wmFullScreen)
				//{
				//	p->x = mi.rcMonitor.left;
				//	p->y = mi.rcMonitor.top;
				//	p->cx = mi.rcMonitor.right-mi.rcMonitor.left;
				//	p->cy = mi.rcMonitor.bottom-mi.rcMonitor.top;
				//}
				//else
				//{
				//	p->x = mi.rcWork.left;
				//	p->y = mi.rcWork.top;
				//	p->cx = mi.rcWork.right-mi.rcWork.left;
				//	p->cy = mi.rcWork.bottom-mi.rcWork.top;
				//}
			}
			else
			{
				_ASSERTE(FALSE && "GetMonitorInfo failed");
			}

			//// ����� ��������������� �������, � �� ��� ����� ���������� �������� (� ��������� ��� �������� ������) ����� �����
			//p->flags |= (SWP_NOSIZE|SWP_NOMOVE);
			//// � �������� ������ ��������
			//SetWindowMode(wmMaximized, TRUE);
		}
	#endif
	}

	//if (gpSet->isDontMinimize) {
	//	if ((p->flags & (0x8000|SWP_NOACTIVATE)) == (0x8000|SWP_NOACTIVATE)
	//		|| ((p->flags & (SWP_NOMOVE|SWP_NOSIZE)) == 0 && p->x < -30000 && p->y < -30000 )
	//		)
	//	{
	//		p->flags = SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE;
	//		p->hwndInsertAfter = HWND_BOTTOM;
	//		result = 0;
	//		if ((dwStyle & WS_MINIMIZE) == WS_MINIMIZE) {
	//			dwStyle &= ~WS_MINIMIZE;
	//			SetWindowStyle(dwStyle);
	//			gpConEmu->InvalidateAll();
	//		}
	//		break;
	//	}
	//}

	if (!(p->flags & SWP_NOSIZE)
	        && (hWnd == ghWnd) && !gpConEmu->mb_IgnoreSizeChange
	        && isWindowNormal())
	{
		if (!hPictureView)
		{
			TODO("����������, ����� ����� ������ PicView �� ����");
			RECT rcWnd = {0,0,p->cx,p->cy};
			CVConGroup::SyncAllConsoles2Window(rcWnd, CER_MAIN, true);
			//CVConGuard VCon;
			//CVirtualConsole* pVCon = (GetActiveVCon(&VCon) >= 0) ? VCon.VCon() : NULL;

			//if (pVCon && pVCon->RCon())
			//	pVCon->RCon()->SyncConsole2Window(FALSE, &rcWnd);
		}
	}

#if 0
	if (!(p->flags & SWP_NOMOVE) && gpSet->isQuakeStyle && (p->y > -30))
	{
		int nDbg = 0;
	}
#endif

	/*
	-- DWM, Glass --
	dwmm.cxLeftWidth = 0;
	dwmm.cxRightWidth = 0;
	dwmm.cyTopHeight = kClientRectTopOffset;
	dwmm.cyBottomHeight = 0;
	DwmExtendFrameIntoClientArea(hwnd, &dwmm);
	-- DWM, Glass --
	*/
	// ����� �� �������� ������� WM_SIZE/WM_MOVE
	result = DefWindowProc(hWnd, uMsg, wParam, lParam);
	p = (WINDOWPOS*)lParam;

	if (gpSetCls->isAdvLogging >= 2)
	{
		size_t cchLen = wcslen(szInfo);
		_wsprintf(szInfo+cchLen, SKIPLEN(countof(szInfo)-cchLen) L" --> (F:x%08X X:%i Y:%i W:%i H:%i)", p->flags, p->x, p->y, p->cx, p->cy);
		LogString(szInfo);
	}

	return result;
}

void CConEmuMain::OnSizePanels(COORD cr)
{
	INPUT_RECORD r;
	int nRepeat = 0;
	wchar_t szKey[32];
	bool bShifted = (mouse.state & MOUSE_DRAGPANEL_SHIFT) && isPressed(VK_SHIFT);
	CRealConsole* pRCon = NULL;
	CVConGuard VCon;
	if (GetActiveVCon(&VCon) >= 0)
		pRCon = VCon->RCon();

	if (!pRCon)
	{
		mouse.state &= ~MOUSE_DRAGPANEL_ALL;
		return; // �����������, ������� ���
	}

	int nConWidth = pRCon->TextWidth();
	// ��������� ������� �� CtrlLeft/Right... ���������� � ��������� - ��
	// ��������� rcPanel - ������ ��� �������� �� �������!
	{
		RECT rcPanel;

		if (!pRCon->GetPanelRect((mouse.state & (MOUSE_DRAGPANEL_RIGHT|MOUSE_DRAGPANEL_SPLIT)), &rcPanel, TRUE))
		{
			// �� ����� ��������� ������� ������� ��������������� Rect ����� ���� �������?
#ifdef _DEBUG
			if (mouse.state & MOUSE_DRAGPANEL_SPLIT)
			{
				DEBUGSTRPANEL2(L"PanelDrag: Skip of NO right panel\n");
			}
			else
			{
				DEBUGSTRPANEL2((mouse.state & MOUSE_DRAGPANEL_RIGHT) ? L"PanelDrag: Skip of NO right panel\n" : L"PanelDrag: Skip of NO left panel\n");
			}

#endif
			return;
		}
	}
	r.EventType = KEY_EVENT;
	r.Event.KeyEvent.dwControlKeyState = 0x128; // ����� �������� SHIFT_PRESSED, ���� �����...
	r.Event.KeyEvent.wVirtualKeyCode = 0;

	// ����� �������� ��������� ���������, ����� �� "���������" ����-����,
	// ���� ��� �� ���������� ��������� ���������

	if (mouse.state & MOUSE_DRAGPANEL_SPLIT)
	{
		//FAR BUGBUG: ��� ������� ����� � ��������� ������� � �������� ������ ����
		// ������� ��������� ������ ������, ���� ��� ��� 11 ��������. �������� ������� 12
		if (cr.X >= (nConWidth-13))
			cr.X = max((nConWidth-12),mouse.LClkCon.X);

		//rcPanel.left = mouse.LClkCon.X; -- ����� ��� �����
		mouse.LClkCon.Y = cr.Y;

		if (cr.X < mouse.LClkCon.X)
		{
			r.Event.KeyEvent.wVirtualKeyCode = VK_LEFT;
			nRepeat = mouse.LClkCon.X - cr.X;
			mouse.LClkCon.X = cr.X; // max(cr.X, (mouse.LClkCon.X-1));
			wcscpy(szKey, L"CtrlLeft");
		}
		else if (cr.X > mouse.LClkCon.X)
		{
			r.Event.KeyEvent.wVirtualKeyCode = VK_RIGHT;
			nRepeat = cr.X - mouse.LClkCon.X;
			mouse.LClkCon.X = cr.X; // min(cr.X, (mouse.LClkCon.X+1));
			wcscpy(szKey, L"CtrlRight");
		}
	}
	else
	{
		//rcPanel.bottom = mouse.LClkCon.Y; -- ����� ��� �����
		mouse.LClkCon.X = cr.X;

		if (cr.Y < mouse.LClkCon.Y)
		{
			r.Event.KeyEvent.wVirtualKeyCode = VK_UP;
			nRepeat = mouse.LClkCon.Y - cr.Y;
			mouse.LClkCon.Y = cr.Y; // max(cr.Y, (mouse.LClkCon.Y-1));
			wcscpy(szKey, bShifted ? L"CtrlShiftUp" : L"CtrlUp");
		}
		else if (cr.Y > mouse.LClkCon.Y)
		{
			r.Event.KeyEvent.wVirtualKeyCode = VK_DOWN;
			nRepeat = cr.Y - mouse.LClkCon.Y;
			mouse.LClkCon.Y = cr.Y; // min(cr.Y, (mouse.LClkCon.Y+1));
			wcscpy(szKey, bShifted ? L"CtrlShiftDown" : L"CtrlDown");
		}

		if (bShifted)
		{
			// ����� ���������� �� ����� � ������ ������
			TODO("������������ ������, ����� ����� �������� ������ �������� ������, � ������ ������...");
			r.Event.KeyEvent.dwControlKeyState |= SHIFT_PRESSED;
		}
	}

	if (r.Event.KeyEvent.wVirtualKeyCode)
	{
		// ������ ���������� ����� ���������� ������ �����
		if (gpSet->isDragPanel == 2)
		{
			if (pRCon->isFar(TRUE))
			{
				mouse.LClkCon = cr;
				wchar_t szMacro[128]; szMacro[0] = L'@';

				if (nRepeat > 1)
					wsprintf(szMacro+1, L"$Rep (%i) %s $End", nRepeat, szKey);
				else
					wcscpy(szMacro+1, szKey);

				PostMacro(szMacro);
			}
		}
		else
		{
#ifdef _DEBUG
			wchar_t szDbg[128]; _wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"PanelDrag: Sending '%s'\n", szKey);
			DEBUGSTRPANEL(szDbg);
#endif
			// �������� �� ����� - ������ ����������� ������� ������, ����� ��� �� ����������� ��������� ������� �����
			// �������
			r.Event.KeyEvent.wVirtualScanCode = MapVirtualKey(r.Event.KeyEvent.wVirtualKeyCode, 0/*MAPVK_VK_TO_VSC*/);
			r.Event.KeyEvent.wRepeatCount = nRepeat; //-- repeat - ���-�� ������...
			//while (nRepeat-- > 0)
			{
				r.Event.KeyEvent.bKeyDown = TRUE;
				pRCon->PostConsoleEvent(&r);
				r.Event.KeyEvent.bKeyDown = FALSE;
				r.Event.KeyEvent.wRepeatCount = 1;
				r.Event.KeyEvent.dwControlKeyState = 0x120; // "������� Ctrl|Shift"
				pRCon->PostConsoleEvent(&r);
			}
		}
	}
	else
	{
		DEBUGSTRPANEL2(L"PanelDrag: Skip of NO key selected\n");
	}
}








/* ****************************************************** */
/*                                                        */
/*                  System Routines                       */
/*                                                        */
/* ****************************************************** */

#ifdef colorer_func
{
	System_Routines() {};
}
#endif

void CConEmuMain::OnActiveConWndStore(HWND hConWnd)
{
	// ��� �������� ������� ��������, �������� � ������
	// ���������� � UserData ���������� "���������" ���� �������
	SetWindowLongPtr(ghWnd, GWLP_USERDATA, (LONG_PTR)hConWnd);

	// ghWndWork - ����������� ��� Holder ��� ����������� ��������,
	// ������ �� ����� ���� ��� �������� ��� ghWnd, ��� � ��������
	// �������, � �������� ���, ��� ��� �������� ��� ghWnd.
	if (ghWndWork && (ghWndWork != ghWnd))
	{	
		SetWindowLongPtr(ghWndWork, GWLP_USERDATA, (LONG_PTR)hConWnd);
	}

	UpdateGuiInfoMappingActive(isMeForeground(true,true));
}

BOOL CConEmuMain::Activate(CVirtualConsole* apVCon)
{
	return CVConGroup::Activate(apVCon);
}

void CConEmuMain::MoveActiveTab(CVirtualConsole* apVCon, bool bLeftward)
{
	CVConGroup::MoveActiveTab(apVCon, bLeftward);
}

//CVirtualConsole* CConEmuMain::ActiveCon()
//{
//	WARNING("�� ��������");
//	CVConGuard VCon;
//	if (CVConGroup::GetActiveVCon(&VCon) < 0)
//		return NULL;
//	return VCon.VCon();
//	/*if (mn_ActiveCon >= countof(mp_VCon))
//	    mn_ActiveCon = -1;
//	if (mn_ActiveCon < 0)
//	    return NULL;
//	return mp_VCon[mn_ActiveCon];*/
//}

// 0 - based
int CConEmuMain::ActiveConNum()
{
	return CVConGroup::ActiveConNum();
}

int CConEmuMain::GetConCount()
{
	return CVConGroup::GetConCount();
}

BOOL CConEmuMain::AttachRequested(HWND ahConWnd, const CESERVER_REQ_STARTSTOP* pStartStop, CESERVER_REQ_STARTSTOPRET* pRet)
{
	return CVConGroup::AttachRequested(ahConWnd, pStartStop, pRet);
}

CRealConsole* CConEmuMain::AttachRequestedGui(LPCWSTR asAppFileName, DWORD anAppPID)
{
	wchar_t szLogInfo[MAX_PATH];

	if (gpSetCls->isAdvLogging!=0)
	{
		_wsprintf(szLogInfo, SKIPLEN(countof(szLogInfo)) L"AttachRequestedGui. AppPID=%u, FileName=", anAppPID);
		lstrcpyn(szLogInfo+_tcslen(szLogInfo), asAppFileName ? asAppFileName : L"<NULL>", 128);
		LogString(szLogInfo);
	}

	CRealConsole* pRCon = CVConGroup::AttachRequestedGui(asAppFileName, anAppPID);

	if (gpSetCls->isAdvLogging!=0)
	{
		wchar_t szRc[64];
		if (pRCon)
			_wsprintf(szRc, SKIPLEN(countof(szRc)) L"Succeeded. ServerPID=%u", pRCon->GetServerPID());
		else
			wcscpy_c(szRc, L"Rejected");
		_wsprintf(szLogInfo, SKIPLEN(countof(szLogInfo)) L"AttachRequestedGui. AppPID=%u. %s", anAppPID, szRc);
		LogString(szLogInfo);
	}

	return pRCon;
}

// ������� ����� ���������� ��������� �� ���� ��������
DWORD CConEmuMain::CheckProcesses()
{
	return CVConGroup::CheckProcesses();
}

bool CConEmuMain::ConActivateNext(BOOL abNext)
{
	return CVConGroup::ConActivateNext(abNext);
}

// nCon - zero-based index of console
bool CConEmuMain::ConActivate(int nCon)
{
	return CVConGroup::ConActivate(nCon);
}

// args ������ ���� ������� ����� "new"
// �� ���������� - �� ���� ����� ������ "delete"
void CConEmuMain::PostCreateCon(RConStartArgs *pArgs)
{
	_ASSERTE((pArgs->pszStartupDir == NULL) || (*pArgs->pszStartupDir != 0));
	PostMessage(ghWnd, mn_MsgCreateCon, mn_MsgCreateCon+1, (LPARAM)pArgs);
}

bool CConEmuMain::CreateWnd(RConStartArgs *args)
{
	if (!args->pszSpecialCmd || !*args->pszSpecialCmd)
	{
		_ASSERTE(args->pszSpecialCmd && *args->pszSpecialCmd);
		return false;
	}

	BOOL bStart = FALSE;

	// Start new ConEmu.exe process with choosen arguments...
	STARTUPINFO si = {sizeof(si)};
	PROCESS_INFORMATION pi = {};
	wchar_t* pszCmdLine = NULL;
	LPCWSTR pszConfig = gpSetCls->GetConfigName();
	size_t cchMaxLen = _tcslen(ms_ConEmuExe)
		+ _tcslen(args->pszSpecialCmd)
		+ (pszConfig ? (_tcslen(pszConfig) + 32) : 0)
		+ 128; // �� ������ ������ � -new_console
	if ((pszCmdLine = (wchar_t*)malloc(cchMaxLen*sizeof(*pszCmdLine))) == NULL)
	{
		_ASSERTE(pszCmdLine);
	}
	else
	{
		pszCmdLine[0] = L'"'; pszCmdLine[1] = 0;
		_wcscat_c(pszCmdLine, cchMaxLen, ms_ConEmuExe);
		_wcscat_c(pszCmdLine, cchMaxLen, L"\" ");
		if (pszConfig && *pszConfig)
		{
			_wcscat_c(pszCmdLine, cchMaxLen, L"/config \"");
			_wcscat_c(pszCmdLine, cchMaxLen, pszConfig);
			_wcscat_c(pszCmdLine, cchMaxLen, L"\" ");
		}
		_wcscat_c(pszCmdLine, cchMaxLen, L"/cmd ");
		_wcscat_c(pszCmdLine, cchMaxLen, args->pszSpecialCmd);
		if (args->bRunAsAdministrator || args->bRunAsRestricted || args->pszUserName)
		{
			if (args->bRunAsAdministrator)
			{
				// Create ConEmu.exe with current credentials, implying elevation for the console
				_wcscat_c(pszCmdLine, cchMaxLen, L" -new_console:a");
			}
			else if (args->bRunAsRestricted)
			{
				_wcscat_c(pszCmdLine, cchMaxLen, L" -new_console:r");
			}
			//else if (args->pszUserName)
			//{
			//	_wcscat_c(pszCmdLine, cchMaxLen, L" -new_console:u:");
			//	_wcscat_c(pszCmdLine, cchMaxLen, args->pszUserName);
			//}
		}

		if (!args->bRunAsAdministrator && !args->bRunAsRestricted && (args->pszUserName && *args->pszUserName))
			bStart = CreateProcessWithLogonW(args->pszUserName, args->pszDomain, args->szUserPassword,
		                           LOGON_WITH_PROFILE, NULL, pszCmdLine,
		                           NORMAL_PRIORITY_CLASS|CREATE_DEFAULT_ERROR_MODE
		                           , NULL, args->pszStartupDir, &si, &pi);
		else
			bStart = CreateProcess(NULL, pszCmdLine, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, args->pszStartupDir, &si, &pi);

		if (!bStart)
		{
			DWORD nErr = GetLastError();
			DisplayLastError(pszCmdLine, nErr, MB_ICONSTOP, L"Failed to start new ConEmu window");
		}
		else
		{
			SafeCloseHandle(pi.hProcess);
			SafeCloseHandle(pi.hThread);
		}
	}

	return (bStart != FALSE);
}

CVirtualConsole* CConEmuMain::CreateCon(RConStartArgs *args, bool abAllowScripts /*= false*/, bool abForceCurConsole /*= false*/)
{
	_ASSERTE(args!=NULL);
	if (!gpConEmu->isMainThread())
	{
		// �������� VCon � ������� ������� �� �����������, �.�. ����� ��������� HWND
		MBoxAssert(gpConEmu->isMainThread());
		return NULL;
	}

	CVirtualConsole* pVCon = CVConGroup::CreateCon(args, abAllowScripts, abForceCurConsole);

	return pVCon;
}

LPCWSTR CConEmuMain::ParseScriptLineOptions(LPCWSTR apszLine, bool* rpbAsAdmin, bool* rpbSetActive, size_t cchNameMax, wchar_t* rsName/*[MAX_RENAME_TAB_LEN]*/)
{
	if (!apszLine)
	{
		_ASSERTE(apszLine!=NULL);
		return NULL;
	}

	// !!! Don't Reset values (rpbSetActive, rpbAsAdmin, rsName)
	// !!! They may be defined in the other place

	// Go
	while (*apszLine == L'>' || *apszLine == L'*' /*|| *apszLine == L'?'*/ || *apszLine == L' ' || *apszLine == L'\t')
	{
		if (*apszLine == L'>' && rpbSetActive)
			*rpbSetActive = true;

		if (*apszLine == L'*' && rpbAsAdmin)
			*rpbAsAdmin = true;

		//if (*apszLine == L'?')
		//{
		//	LPCWSTR pszStart = apszLine+1;
		//	LPCWSTR pszEnd = wcschr(pszStart, L'?');
		//	if (!pszEnd) pszEnd = pszStart + _tcslen(pszStart);

		//	if (rsName && (pszEnd > pszStart) && (cchNameMax > 1))
		//	{
		//		UINT nLen = min((INT_PTR)cchNameMax, (pszEnd-pszStart+1));
		//		lstrcpyn(rsName, pszStart, nLen);
		//	}

		//	apszLine = pszEnd;
		//	if (!*pszEnd)
		//		break;
		//}

		apszLine++;
	}

	return apszLine;
}

// ���������� ��������� �� �������� ������� (��� �������� ������)
// apszScript �������� ������ ������, ����������� \r\n
CVirtualConsole* CConEmuMain::CreateConGroup(LPCWSTR apszScript, bool abForceAsAdmin /*= false*/, LPCWSTR asStartupDir /*= NULL*/)
{
	CVirtualConsole* pVConResult = NULL;
	// �������
	wchar_t *pszDataW = lstrdup(apszScript);
	wchar_t *pszLine = pszDataW;
	wchar_t *pszNewLine = wcschr(pszLine, L'\n');
	CVirtualConsole *pSetActive = NULL, *pVCon = NULL, *pLastVCon = NULL;
	bool lbSetActive = false, lbOneCreated = false, lbRunAdmin = false;

	CVConGroup::OnCreateGroupBegin();

	while (*pszLine)
	{
		lbSetActive = false;
		lbRunAdmin = abForceAsAdmin;

		// Task pre-options, for example ">*cmd"
		pszLine = (wchar_t*)ParseScriptLineOptions(pszLine, &lbRunAdmin, &lbSetActive);

		if (pszNewLine)
		{
			*pszNewLine = 0;
			if ((pszNewLine > pszDataW) && (*(pszNewLine-1) == L'\r'))
				*(pszNewLine-1) = 0;
		}

		while (*pszLine == L' ') pszLine++;

		if (*pszLine)
		{
			while (pszLine[0] == L'/')
			{
				if (CSTR_EQUAL == CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE|SORT_STRINGSORT,
				                               pszLine, 14, L"/bufferheight ", 14))
				{
					pszLine += 14;

					while(*pszLine == L' ') pszLine++;

					wchar_t* pszEnd = NULL;
					long lBufHeight = wcstol(pszLine, &pszEnd, 10);
					gpSetCls->SetArgBufferHeight(lBufHeight);

					if (pszEnd) pszLine = pszEnd;
				}

				TODO("����� �������� ���� /mouse - �������� ���� ���������");

				if (CSTR_EQUAL == CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE|SORT_STRINGSORT,
				                               pszLine, 5, L"/cmd ", 5))
				{
					pszLine += 5;
				}

				while (*pszLine == L' ') pszLine++;
			}

			if (*pszLine)
			{
				RConStartArgs args;
				args.pszSpecialCmd = lstrdup(pszLine);
				args.pszStartupDir = (asStartupDir && *asStartupDir) ? lstrdup(asStartupDir) : NULL;
				args.bRunAsAdministrator = lbRunAdmin;
				pVCon = CreateCon(&args, false, true);

				if (!pVCon)
				{
					DisplayLastError(L"Can't create new virtual console!");

					if (!lbOneCreated)
					{
						//Destroy(); -- ������ ���������� �������
						goto wrap;
					}
				}
				else
				{
					lbOneCreated = TRUE;

					pLastVCon = pVCon;
					if (lbSetActive && !pSetActive)
						pSetActive = pVCon;

					if (GetVCon((int)MAX_CONSOLE_COUNT-1))
						break; // ������ ������� �� ���������
				}
			}

			// ��� �������� ������ �������� ��������� ������������ ���������,
			// ����� ����� ���������� ���������� ������������ �������
			MSG Msg;
			while (PeekMessage(&Msg,0,0,0,PM_REMOVE))
			{
				ConEmuMsgLogger::Log(Msg, ConEmuMsgLogger::msgCommon);

				if (Msg.message == WM_QUIT)
					goto wrap;

				BOOL lbDlgMsg = isDialogMessage(Msg);
				if (!lbDlgMsg)
				{
					TranslateMessage(&Msg);
					DispatchMessage(&Msg);
				}
			}

			if (!ghWnd || !IsWindow(ghWnd))
				goto wrap;
		}

		if (!pszNewLine) break;

		pszLine = pszNewLine+1;

		if (!*pszLine) break;

		while ((*pszLine == L'\r') || (*pszLine == L'\n'))
			pszLine++; // ���������� ��� �������� �����

		pszNewLine = wcschr(pszLine, L'\n');
	}

	if (pSetActive)
	{
		Activate(pSetActive);
	}

	pVConResult = (pSetActive ? pSetActive : pLastVCon);
wrap:
	SafeFree(pszDataW);
	CVConGroup::OnCreateGroupEnd();
	return pVConResult;
}

void CConEmuMain::CreateGhostVCon(CVirtualConsole* apVCon)
{
	PostMessage(ghWnd, mn_MsgInitVConGhost, 0, (LPARAM)apVCon);
}

void CConEmuMain::UpdateActiveGhost(CVirtualConsole* apVCon)
{
	_ASSERTE(isActive(apVCon));
	if (mh_LLKeyHookDll && mph_HookedGhostWnd)
	{
		// Win7 � ����!
		if (IsWindows7 || !gpSet->isTabsOnTaskBar())
			*mph_HookedGhostWnd = NULL; //ghWndApp ? ghWndApp : ghWnd;
		else
			*mph_HookedGhostWnd = apVCon->GhostWnd();
	}
}

// ������� �� ��� �������� ���� CMD_FARSETCHANGED
// ����������� ���������: gpSet->isFARuseASCIIsort, gpSet->isShellNoZoneCheck;
void CConEmuMain::UpdateFarSettings()
{
	CVConGroup::OnUpdateFarSettings();
}

//void CConEmuMain::UpdateIdealRect(BOOL abAllowUseConSize/*=FALSE*/)
//{
//	// ��������� "���������" ������ ����, ��������� �������������
//	if (isWindowNormal())
//	{
//		RECT rcWnd = {};
//		GetWindowRect(ghWnd, &rcWnd);
//		UpdateIdealRect(rcWnd);
//	}
//	else if (abAllowUseConSize)
//	{
//		//if (isVConExists(0))
//		//	return;
//
//		//RECT rcCon = CVConGroup::AllTextRect();
//		//RECT rcCon = CalcRect(CER_CONSOLE_ALL);
//		RECT rcCon = MakeRect(gpConEmu->wndWidth, gpConEmu->wndHeight);
//		RECT rcWnd = CalcRect(CER_MAIN, rcCon, CER_CONSOLE_ALL);
//		UpdateIdealRect(rcWnd);
//	}
//}

void CConEmuMain::UpdateIdealRect(RECT rcNewIdeal)
{
#ifdef _DEBUG
	RECT rc = rcNewIdeal;
	_ASSERTE(rc.right>rc.left && rc.bottom>rc.top);
	if (memcmp(&mr_Ideal.rcIdeal, &rc, sizeof(rc)) == 0)
		return;
#endif

	mr_Ideal.rcIdeal = rcNewIdeal;
}

void CConEmuMain::UpdateTextColorSettings(BOOL ChangeTextAttr /*= TRUE*/, BOOL ChangePopupAttr /*= TRUE*/)
{
	CVConGroup::OnUpdateTextColorSettings(ChangeTextAttr, ChangePopupAttr);
}

void CConEmuMain::DebugStep(LPCWSTR asMsg, BOOL abErrorSeverity/*=FALSE*/)
{
	if (ghWnd)
	{
		static bool bWasDbgStep, bWasDbgError;

		if (asMsg && *asMsg)
		{
			// ���� ������� ��� - ��������� � ����
			LogString(asMsg);

			if (gpSet->isDebugSteps || abErrorSeverity)
			{
				bWasDbgStep = true;

				if (abErrorSeverity) bWasDbgError = true;

				SetWindowText(ghWnd, asMsg);
			}
		}
		else
		{
			// ������� ��������� � ������������ � ���������� ���������� � ���������� �������
			// � �������� ��� � ������� ����, ���� ����������
			// abErrorSeverity ���������, ����� ����� ���� �������� "���������" ������� ���������
			if (bWasDbgStep || abErrorSeverity)
			{
				bWasDbgStep = false;

				if (bWasDbgError)
				{
					bWasDbgError = false;
					return;
				}

				UpdateTitle();
			}
		}
	}
}

DWORD_PTR CConEmuMain::GetActiveKeyboardLayout()
{
	_ASSERTE(mn_MainThreadId!=0);
	DWORD_PTR dwActive = (DWORD_PTR)GetKeyboardLayout(mn_MainThreadId);
	return dwActive;
}

//LPTSTR CConEmuMain::GetTitleStart()
//{
//	//mn_ActiveStatus &= ~CES_CONALTERNATIVE; // ����� ����� �������������� �������
//	return Title;
//}

LPCWSTR CConEmuMain::GetDefaultTitle()
{
	return ms_ConEmuDefTitle;
}

void CConEmuMain::SetTitleTemplate(LPCWSTR asTemplate)
{
	lstrcpyn(TitleTemplate, asTemplate ? asTemplate : L"", countof(TitleTemplate));
}

LRESULT CConEmuMain::GuiShellExecuteEx(SHELLEXECUTEINFO* lpShellExecute, CVirtualConsole* apVCon)
{
	LRESULT lRc = 0;

	if (!isMainThread())
	{
		GuiShellExecuteExArg* pArg = (GuiShellExecuteExArg*)calloc(1,sizeof(*pArg));
		pArg->pVCon = apVCon;
		pArg->lpShellExecute = lpShellExecute;
		pArg->hReadyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		EnterCriticalSection(&mcs_ShellExecuteEx);
		m_ShellExecuteQueue.push_back(pArg);
		LeaveCriticalSection(&mcs_ShellExecuteEx);

		PostMessage(ghWnd, mn_ShellExecuteEx, 0, 0);

		if (ghWnd && IsWindow(ghWnd))
		{
			WaitForSingleObject(pArg->hReadyEvent, INFINITE);

			lRc = pArg->bResult;
		}

		SafeCloseHandle(pArg->hReadyEvent);
	}
	else
	{
		_ASSERTE(FALSE && "GuiShellExecuteEx must not be called in Main thread!");
	}

	return lRc;
}

void CConEmuMain::GuiShellExecuteExQueue()
{
	if (mb_InShellExecuteQueue)
	{
		// ������� ��� ��������������!
		return;
	}

	_ASSERTE(isMainThread()); // ������ ����������� � �������� ������!

	mb_InShellExecuteQueue = true;

	while (m_ShellExecuteQueue.size() != 0)
	{
		GuiShellExecuteExArg* pArg = NULL;

		EnterCriticalSection(&mcs_ShellExecuteEx);
		for (INT_PTR i = 0; i < m_ShellExecuteQueue.size(); i++)
		{
			if (m_ShellExecuteQueue[i]->bInProcess)
			{
				_ASSERTE(m_ShellExecuteQueue[i]->bInProcess == FALSE); // TRUE - ������ ���� ����� �� �������!
			}
			else
			{
				m_ShellExecuteQueue[i]->bInProcess = TRUE;
				pArg = m_ShellExecuteQueue[i];
				m_ShellExecuteQueue.erase(i);
				break;
			}
		}
		LeaveCriticalSection(&mcs_ShellExecuteEx);

		if (!pArg)
		{
			break; // ������� ����������?
		}

		SHELLEXECUTEINFO seiPre = *pArg->lpShellExecute;

		BOOL lRc = ::ShellExecuteEx(pArg->lpShellExecute);

		SHELLEXECUTEINFO seiPst = *pArg->lpShellExecute;
		DWORD dwErr = (lRc == FALSE) ? GetLastError() : 0;

		pArg->bResult = lRc;
		pArg->dwErrCode = dwErr;		

		if (lRc != FALSE)
		{
			if (pArg->lpShellExecute->fMask & SEE_MASK_NOCLOSEPROCESS)
			{
				// OK, �� ��� ����� ����� ����������� ��������
				_ASSERTE(seiPst.hProcess != NULL);
				_ASSERTE(pArg->lpShellExecute->hProcess != NULL);
			}
		}
		else // ������
		{
			//120429 - ���� �� ���� � Recreate - �� �������� �� ���������, ����� �������� �����?
			if ((isValid(pArg->pVCon) && !pArg->pVCon->RCon()->InRecreate()))
			{
				pArg->pVCon->RCon()->CloseConsole(false, false);
			}
		}

		SetEvent(pArg->hReadyEvent);

		UNREFERENCED_PARAMETER(seiPre.cbSize);
		UNREFERENCED_PARAMETER(seiPst.cbSize);
		UNREFERENCED_PARAMETER(dwErr);
	}

	mb_InShellExecuteQueue = false;
}

//BOOL CConEmuMain::HandlerRoutine(DWORD dwCtrlType)
//{
//    return (dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_BREAK_EVENT ? true : false);
//}

// ����� �������� ����� �������� ��������!
void CConEmuMain::LoadIcons()
{
	if (hClassIcon)
		return; // ��� ���������

	wchar_t *lpszExt = NULL;
	wchar_t szIconPath[MAX_PATH] = {};

	if (gpConEmu->mps_IconPath)
	{
		if (FileExists(gpConEmu->mps_IconPath))
		{
			wcscpy_c(szIconPath, gpConEmu->mps_IconPath);
		}
		else
		{
			wchar_t* pszFilePart;
			DWORD n = SearchPath(NULL, gpConEmu->mps_IconPath, NULL, countof(szIconPath), szIconPath, &pszFilePart);
			if (!n || (n >= countof(szIconPath)))
				szIconPath[0] = 0;
		}
	}
	else
	{
		lstrcpyW(szIconPath, ms_ConEmuExe);
		lpszExt = (wchar_t*)PointToExt(szIconPath);

		if (!lpszExt)
		{
			szIconPath[0] = 0;
		}
		else
		{
			_tcscpy(lpszExt, _T(".ico"));
			if (!FileExists(szIconPath))
				szIconPath[0]=0;
		}
	}

	if (szIconPath[0])
	{
		lpszExt = (wchar_t*)PointToExt(szIconPath);

		if (lpszExt && (lstrcmpi(lpszExt, L".ico") == 0))
		{
			hClassIcon = (HICON)LoadImage(0, szIconPath, IMAGE_ICON,
			                              GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR|LR_LOADFROMFILE);
			hClassIconSm = (HICON)LoadImage(0, szIconPath, IMAGE_ICON,
			                                GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR|LR_LOADFROMFILE);
        }
        else
        {
        	ExtractIconEx(szIconPath, 0, &hClassIcon, &hClassIconSm, 1);
        }
	}

	if (!hClassIcon)
	{
		szIconPath[0]=0;
		hClassIcon = (HICON)LoadImage(GetModuleHandle(0),
		                              MAKEINTRESOURCE(gpSet->nIconID), IMAGE_ICON,
		                              GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);

		if (hClassIconSm) DestroyIcon(hClassIconSm);

		hClassIconSm = (HICON)LoadImage(GetModuleHandle(0),
		                                MAKEINTRESOURCE(gpSet->nIconID), IMAGE_ICON,
		                                GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	}
}

//bool CConEmuMain::LoadVersionInfo(wchar_t* pFullPath)
//{
//	LPBYTE pBuffer=NULL;
//	wchar_t* pVersion=NULL;
//	//wchar_t* pDesc=NULL;
//	const wchar_t WSFI[] = L"StringFileInfo";
//	DWORD size = GetFileVersionInfoSizeW(pFullPath, &size);
//
//	if (!size) return false;
//
//	MCHKHEAP
//	pBuffer = new BYTE[size];
//	MCHKHEAP
//	GetFileVersionInfoW((wchar_t*)pFullPath, 0, size, pBuffer);
//	//Find StringFileInfo
//	DWORD ofs;
//
//	for(ofs = 92; ofs < size; ofs += *(WORD*)(pBuffer+ofs))
//		if (!lstrcmpiW((wchar_t*)(pBuffer+ofs+6), WSFI))
//			break;
//
//	if (ofs >= size)
//	{
//		delete pBuffer;
//		return false;
//	}
//
//	TCHAR *langcode;
//	langcode = (TCHAR*)(pBuffer + ofs + 42);
//	TCHAR blockname[48];
//	unsigned dsize;
//	_wsprintf(blockname, SKIPLEN(countof(blockname)) _T("\\%s\\%s\\FileVersion"), WSFI, langcode);
//
//	if (!VerQueryValue(pBuffer, blockname, (void**)&pVersion, &dsize))
//	{
//		pVersion = 0;
//	}
//	else
//	{
//		if (dsize>=31) pVersion[31]=0;
//
//		wcscpy(szConEmuVersion, pVersion);
//		pVersion = wcsrchr(szConEmuVersion, L',');
//
//		if (pVersion && wcscmp(pVersion, L", 0")==0)
//			*pVersion = 0;
//	}
//
//	delete[] pBuffer;
//	return true;
//}

void CConEmuMain::OnCopyingState()
{
	TODO("CConEmuMain::OnCopyingState()");
}

void CConEmuMain::PostDragCopy(BOOL abMove, BOOL abRecieved/*=FALSE*/)
{
	if (!abRecieved)
	{
		PostMessage(ghWnd, mn_MsgPostCopy, 0, (LPARAM)abMove);
	}
	else
	{
		//PostMacro(apszMacro);
		//free(apszMacro);
		CVConGuard VCon;
		if (GetActiveVCon(&VCon) >= 0)
		{
			VCon->RCon()->PostDragCopy(abMove);
		}
	}
}

void CConEmuMain::PostMacro(LPCWSTR asMacro)
{
	if (!asMacro || !*asMacro)
		return;

	CVConGuard VCon;
	if (GetActiveVCon(&VCon) >= 0)
	{
		VCon->RCon()->PostMacro(asMacro);
	}
	//#ifdef _DEBUG
	//DEBUGSTRMACRO(asMacro); OutputDebugStringW(L"\n");
	//#endif
	//
	//CConEmuPipe pipe(GetFarPID(), CONEMUREADYTIMEOUT);
	//if (pipe.Init(_T("CConEmuMain::PostMacro"), TRUE))
	//{
	//    //DWORD cbWritten=0;
	//    DebugStep(_T("Macro: Waiting for result (10 sec)"));
	//    pipe.Execute(CMD_POSTMACRO, asMacro, (_tcslen(asMacro)+1)*2);
	//    DebugStep(NULL);
	//}
}

void CConEmuMain::PostAutoSizeFont(int nRelative/*0/1*/, int nValue/*��� nRelative==0 - ������, ��� ==1 - +-1, +-2,...*/)
{
	PostMessage(ghWnd, mn_MsgAutoSizeFont, (WPARAM)nRelative, (LPARAM)nValue);
}

void CConEmuMain::PostMacroFontSetName(wchar_t* pszFontName, WORD anHeight /*= 0*/, WORD anWidth /*= 0*/, BOOL abPosted)
{
	if (!abPosted)
	{
		wchar_t* pszDup = lstrdup(pszFontName);
		WPARAM wParam = (((DWORD)anHeight) << 16) | (anWidth);
		PostMessage(ghWnd, mn_MsgMacroFontSetName, wParam, (LPARAM)pszDup);
	}
	else
	{
		if (gpSetCls)
			gpSetCls->MacroFontSetName(pszFontName, anHeight, anWidth);
		free(pszFontName);
	}
}

void CConEmuMain::PostDisplayRConError(CRealConsole* apRCon, wchar_t* pszErrMsg)
{
	PostMessage(ghWnd, mn_MsgDisplayRConError, (WPARAM)apRCon, (LPARAM)pszErrMsg);
}

bool CConEmuMain::PtDiffTest(POINT C, int aX, int aY, UINT D)
{
	//(((abs(C.x-(int)(short)LOWORD(lParam)))<D) && ((abs(C.y-(int)(short)HIWORD(lParam)))<D))
	int nX = C.x - aX;

	if (nX < 0) nX = -nX;

	if (nX > (int)D)
		return false;

	int nY = C.y - aY;

	if (nY < 0) nY = -nY;

	if (nY > (int)D)
		return false;

	return true;
}

void CConEmuMain::RegisterMinRestore(bool abRegister)
{
	wchar_t szErr[512];

	if (abRegister && !m_InsideIntegration)
	{
		for (size_t i = 0; i < countof(gRegisteredHotKeys); i++)
		{
			//if (!gpSet->vmMinimizeRestore)

			const ConEmuHotKey* pHk = NULL;
			DWORD VkMod = gpSet->GetHotkeyById(gRegisteredHotKeys[i].DescrID, &pHk);
			UINT vk = gpSet->GetHotkey(VkMod);
			if (!vk)
				continue;  // �� �������
			UINT nMOD = gpSet->GetHotKeyMod(VkMod);

			if (gRegisteredHotKeys[i].RegisteredID
					&& ((gRegisteredHotKeys[i].VK != vk) || (gRegisteredHotKeys[i].MOD != nMOD)))
			{
				UnregisterHotKey(ghWnd, gRegisteredHotKeys[i].RegisteredID);

				if (gpSetCls->isAdvLogging)
				{
					_wsprintf(szErr, SKIPLEN(countof(szErr)) L"UnregisterHotKey(ID=%u)", gRegisteredHotKeys[i].RegisteredID);
					LogString(szErr, TRUE);
				}

				gRegisteredHotKeys[i].RegisteredID = 0;
			}

			if (!gRegisteredHotKeys[i].RegisteredID)
			{
				wchar_t szKey[128];
				gpSet->GetHotkeyName(pHk, szKey);

				BOOL bRegRc = RegisterHotKey(ghWnd, HOTKEY_GLOBAL_START+i, nMOD, vk);
				DWORD dwErr = bRegRc ? 0 : GetLastError();

				if (gpSetCls->isAdvLogging)
				{
					_wsprintf(szErr, SKIPLEN(countof(szErr)) L"RegisterHotKey(ID=%u, %s, VK=%u, MOD=x%X) - %s, Code=%u", HOTKEY_GLOBAL_START+i, szKey, vk, nMOD, bRegRc ? L"OK" : L"FAILED", dwErr);
					LogString(szErr, TRUE);
				}

				if (bRegRc)
				{
					gRegisteredHotKeys[i].RegisteredID = HOTKEY_GLOBAL_START+i;
					gRegisteredHotKeys[i].VK = vk;
					gRegisteredHotKeys[i].MOD = nMOD;
				}
				else
				{
					if (isFirstInstance())
					{
						// -- ��� ������������� ������� ���� ����� - ������ �����, ��� ��� ���������
						// -- �������� ������ �� ����� ���������� ������
						// -- ����� ����, isFirstInstance() �� ��������, ���� ����� ConEmu.exe �������� ��� ������ ������
						wchar_t szName[128];

						if (!LoadString(g_hInstance, gRegisteredHotKeys[i].DescrID, szName, countof(szName)))
							_wsprintf(szName, SKIPLEN(countof(szName)) L"DescrID=%i", gRegisteredHotKeys[i].DescrID);

						_wsprintf(szErr, SKIPLEN(countof(szErr))
							L"Can't register hotkey for\n%s\n%s"
							L"%s, ErrCode=%u",
							szName,
							(dwErr == 1409) ? L"Hotkey already registered by another App\n" : L"",
							szKey, dwErr);
						Icon.ShowTrayIcon(szErr, tsa_Config_Error);
					}
				}
			}
		}
	}
	else
	{
		for (size_t i = 0; i < countof(gRegisteredHotKeys); i++)
		{
			if (gRegisteredHotKeys[i].RegisteredID)
			{
				UnregisterHotKey(ghWnd, gRegisteredHotKeys[i].RegisteredID);

				if (gpSetCls->isAdvLogging)
				{
					_wsprintf(szErr, SKIPLEN(countof(szErr)) L"UnregisterHotKey(ID=%u)", gRegisteredHotKeys[i].RegisteredID);
					LogString(szErr, TRUE);
				}

				gRegisteredHotKeys[i].RegisteredID = 0;
			}
		}
		//if (mn_MinRestoreRegistered)
		//{
		//	UnregisterHotKey(ghWnd, mn_MinRestoreRegistered);
		//	mn_MinRestoreRegistered = 0;
		//}
	}
}

void CConEmuMain::RegisterHotKeys()
{
	if (isIconic())
	{
		UnRegisterHotKeys();
		return;
	}

	if (!mb_HotKeyRegistered)
	{
		BOOL bRegRc = RegisterHotKey(ghWnd, HOTKEY_CTRLWINALTSPACE_ID, MOD_CONTROL|MOD_WIN|MOD_ALT, VK_SPACE);
		DWORD dwErr = bRegRc ? 0 : GetLastError();

		if (gpSetCls->isAdvLogging)
		{
			char szErr[512];
			_wsprintfA(szErr, SKIPLEN(countof(szErr)) "RegisterHotKey(ID=%u, %s, VK=%u, MOD=x%X) - %s, Code=%u", HOTKEY_CTRLWINALTSPACE_ID, L"Ctrl+Win+Alt+Space", VK_SPACE, MOD_CONTROL|MOD_WIN|MOD_ALT, bRegRc ? "OK" : "FAILED", dwErr);
			LogString(szErr, TRUE);
		}

		if (bRegRc)
		{
			mb_HotKeyRegistered = TRUE;
		}
	}

	if (!mh_LLKeyHook)
	{
		RegisterHoooks();
	}
}

HMODULE CConEmuMain::LoadConEmuCD()
{
	if (!mh_LLKeyHookDll)
	{
		wchar_t szConEmuDll[MAX_PATH+32];
		lstrcpy(szConEmuDll, ms_ConEmuBaseDir);
#ifdef WIN64
		lstrcat(szConEmuDll, L"\\ConEmuCD64.dll");
#else
		lstrcat(szConEmuDll, L"\\ConEmuCD.dll");
#endif
		//wchar_t szSkipEventName[128];
		//_wsprintf(szSkipEventName, SKIPLEN(countof(szSkipEventName)) CEHOOKDISABLEEVENT, GetCurrentProcessId());
		//HANDLE hSkipEvent = CreateEvent(NULL, TRUE, TRUE, szSkipEventName);
		mh_LLKeyHookDll = LoadLibrary(szConEmuDll);
		//CloseHandle(hSkipEvent);

		if (mh_LLKeyHookDll)
			mph_HookedGhostWnd = (HWND*)GetProcAddress(mh_LLKeyHookDll, "ghActiveGhost");
	}

	if (!mh_LLKeyHookDll)
		mph_HookedGhostWnd = NULL;
	
	return mh_LLKeyHookDll;
}

void CConEmuMain::UpdateWinHookSettings()
{
	if (mh_LLKeyHookDll)
	{
		gpSetCls->UpdateWinHookSettings(mh_LLKeyHookDll);

		CVConGuard VCon;
		if (GetActiveVCon(&VCon) >= 0)
		{
			UpdateActiveGhost(VCon.VCon());
		}
	}
}

void CConEmuMain::RegisterHoooks()
{
//	#ifndef _DEBUG
	// -- ��� GUI ������ ���� �����
	//// ��� WinXP ��� �� ���� �����
	//if (gOSVer.dwMajorVersion < 6)
	//{
	//	return;
	//}
//	#endif

	// ���� Host-������� �� Win, ��� ���� �� ����� ������������� Win+Number - ��� �� �����
	//if (!gpSet->isUseWinNumber || !gpSet->IsHostkeySingle(VK_LWIN))
	if (!gpSetCls->HasSingleWinHotkey())
	{
		if (gpSetCls->isAdvLogging)
		{
			LogString("CConEmuMain::RegisterHoooks() skipped, cause of !HasSingleWinHotkey()", TRUE);
		}

		UnRegisterHoooks();

		return;
	}

	DWORD dwErr = 0;

	if (!mh_LLKeyHook)
	{
		// ���������, �������� �� ������������ ��������� �����.
		if (!gpSet->isKeyboardHooks())
		{
			if (gpSetCls->isAdvLogging)
			{
				LogString("CConEmuMain::RegisterHoooks() skipped, cause of !isKeyboardHooks()", TRUE);
			}
		}
		else
		{
			if (!mh_LLKeyHookDll)
				LoadConEmuCD();
			//{
			//	wchar_t szConEmuDll[MAX_PATH+32];
			//	lstrcpy(szConEmuDll, ms_ConEmuBaseDir);
			//#ifdef WIN64
			//	lstrcat(szConEmuDll, L"\\ConEmuCD64.dll");
			//#else
			//	lstrcat(szConEmuDll, L"\\ConEmuCD.dll");
			//#endif
			//	wchar_t szSkipEventName[128];
			//	_wsprintf(szSkipEventName, SKIPLEN(countof(szSkipEventName)) CEHOOKDISABLEEVENT, GetCurrentProcessId());
			//	HANDLE hSkipEvent = CreateEvent(NULL, TRUE, TRUE, szSkipEventName);
			//	mh_LLKeyHookDll = LoadLibrary(szConEmuDll);
			//	CloseHandle(hSkipEvent);
			//}
			
			_ASSERTE(mh_LLKeyHook==NULL); // �� ������� ������ ����������� ������?

			if (!mh_LLKeyHook && mh_LLKeyHookDll)
			{
				HOOKPROC pfnLLHK = (HOOKPROC)GetProcAddress(mh_LLKeyHookDll, "LLKeybHook");
				HHOOK *pKeyHook = (HHOOK*)GetProcAddress(mh_LLKeyHookDll, "ghKeyHook");
				HWND *pConEmuRoot = (HWND*)GetProcAddress(mh_LLKeyHookDll, "ghKeyHookConEmuRoot");
				//BOOL *pbWinTabHook = (BOOL*)GetProcAddress(mh_LLKeyHookDll, "gbWinTabHook");
				//HWND *pConEmuDc = (HWND*)GetProcAddress(mh_LLKeyHookDll, "ghConEmuWnd");

				if (pConEmuRoot)
					*pConEmuRoot = ghWnd;

				UpdateWinHookSettings();

				//if (pConEmuDc)
				//	*pConEmuDc = ghWnd DC;

				if (pfnLLHK)
				{
					// WH_KEYBOARD_LL - ����� ���� ������ ����������
					mh_LLKeyHook = SetWindowsHookEx(WH_KEYBOARD_LL, pfnLLHK, mh_LLKeyHookDll, 0);

					if (!mh_LLKeyHook)
					{
						dwErr = GetLastError();
						if (gpSetCls->isAdvLogging)
						{
							char szErr[128];
							_wsprintfA(szErr, SKIPLEN(countof(szErr)) "CConEmuMain::RegisterHoooks() failed, Code=%u", dwErr);
							LogString(szErr, TRUE);
						}
						_ASSERTE(mh_LLKeyHook!=NULL);
					}
					else
					{
						if (pKeyHook) *pKeyHook = mh_LLKeyHook;
						if (gpSetCls->isAdvLogging)
						{
							LogString("CConEmuMain::RegisterHoooks() succeeded", TRUE);
						}
					}
				}
			}
			else
			{
				if (gpSetCls->isAdvLogging)
				{
					LogString("CConEmuMain::RegisterHoooks() failed, cause of !mh_LLKeyHookDll", TRUE);
				}
			}
		}
	}
	else
	{
		if (gpSetCls->isAdvLogging >= 2)
		{
			LogString("CConEmuMain::RegisterHoooks() skipped, already was set", TRUE);
		}
	}
}

//BOOL CConEmuMain::LowLevelKeyHook(UINT nMsg, UINT nVkKeyCode)
//{
//    //if (nVkKeyCode == ' ' && gpSet->isSendAltSpace == 2 && gpSet->IsHostkeyPressed())
//	//{
//    //	ShowSysmenu();
//	//	return TRUE;
//	//}
//
//	if (!gpSet->isUseWinNumber || !gpSet->IsHostkeyPressed())
//		return FALSE;
//
//	// ������ ���������� ���������
//	if (nVkKeyCode >= '0' && nVkKeyCode <= '9')
//	{
//		if (nMsg == WM_KEYDOWN)
//		{
//			if (nVkKeyCode>='1' && nVkKeyCode<='9') // ##1..9
//				ConActivate(nVkKeyCode - '1');
//
//			else if (nVkKeyCode=='0') // #10.
//				ConActivate(9);
//		}
//
//		return TRUE;
//	}
//
//	return FALSE;
//}

void CConEmuMain::UnRegisterHotKeys(BOOL abFinal/*=FALSE*/)
{
	if (mb_HotKeyRegistered)
	{
		UnregisterHotKey(ghWnd, HOTKEY_CTRLWINALTSPACE_ID);
		mb_HotKeyRegistered = FALSE;

		if (gpSetCls->isAdvLogging)
		{
			char szErr[128];
			_wsprintfA(szErr, SKIPLEN(countof(szErr)) "UnregisterHotKey(ID=%u)", HOTKEY_CTRLWINALTSPACE_ID);
			LogString(szErr, TRUE);
		}
	}

	UnRegisterHoooks(abFinal);
}

void CConEmuMain::UnRegisterHoooks(BOOL abFinal/*=FALSE*/)
{
	if (mh_LLKeyHook)
	{
		UnhookWindowsHookEx(mh_LLKeyHook);
		mh_LLKeyHook = NULL;

		if (gpSetCls->isAdvLogging)
		{
			LogString("CConEmuMain::UnRegisterHoooks() done", TRUE);
		}
	}

	if (abFinal)
	{
		if (mh_LLKeyHookDll)
		{
			FreeLibrary(mh_LLKeyHookDll);
			mh_LLKeyHookDll = NULL;
		}
	}
}

// ��������� WM_HOTKEY
void CConEmuMain::OnWmHotkey(WPARAM wParam)
{
	// Ctrl+Win+Alt+Space
	if (wParam == HOTKEY_CTRLWINALTSPACE_ID)
	{
		CtrlWinAltSpace();
	}
	else
	{
		//// vmMinimizeRestore -> Win+C
		//else if (gpConEmu->mn_MinRestoreRegistered && (int)wParam == gpConEmu->mn_MinRestoreRegistered)
		//{
		//	gpConEmu->OnMinimizeRestore();
		//}

		for (size_t i = 0; i < countof(gRegisteredHotKeys); i++)
		{
			if (gRegisteredHotKeys[i].RegisteredID && ((int)wParam == gRegisteredHotKeys[i].RegisteredID))
			{
				switch (gRegisteredHotKeys[i].DescrID)
				{
				case vkMinimizeRestore:
				case vkMinimizeRestor2:
					OnMinimizeRestore();
					break;
				case vkGlobalRestore:
					OnMinimizeRestore(sih_Show);
					break;
				case vkForceFullScreen:
					OnForcedFullScreen(true);
					break;
				}
				break;
			}
		}
	}
}

void CConEmuMain::CtrlWinAltSpace()
{
	CVConGuard VCon;
	if ((GetActiveVCon(&VCon) >= 0) && VCon->RCon())
	{
		VCon->RCon()->CtrlWinAltSpace(); // Toggle visibility
	}
}

// abRecreate: TRUE - ����������� �������, FALSE - ������� �����
// abConfirm:  TRUE - �������� ������ �������������
// abRunAs:    TRUE - ��� �������
void CConEmuMain::RecreateAction(RecreateActionParm aRecreate, BOOL abConfirm, BOOL abRunAs)
{
	FLASHWINFO fl = {sizeof(FLASHWINFO)}; fl.dwFlags = FLASHW_STOP; fl.hwnd = ghWnd;
	FlashWindowEx(&fl); // ��� ������������ ��������� ������ ��������...
	RConStartArgs args;
	args.aRecreate = aRecreate;
	args.bRunAsAdministrator = abRunAs;

	WARNING("��� �������� �� ����� ��������� ������ ������ �� �����");
	//if (!abConfirm && isPressed(VK_SHIFT))
	//	abConfirm = TRUE;

	if ((args.aRecreate == cra_CreateTab) || (args.aRecreate == cra_CreateWindow))
	{
		// ������� ����� �������
		BOOL lbSlotFound = (args.aRecreate == cra_CreateWindow);

		if (args.aRecreate == cra_CreateWindow && gpSet->isMulti)
			abConfirm = TRUE;

		if (args.aRecreate == cra_CreateTab)
		{
			//for (size_t i = 0; i < countof(mp_VCon); i++)
			//{
			//	if (!mp_VCon[i]) { lbSlotFound = TRUE; break; }
			//}
			// ��������� �� ���������, �.�. "�����" � ��� ���� �� �����
			if (!isVConExists(MAX_CONSOLE_COUNT-1))
				lbSlotFound = TRUE;
		}

		if (!lbSlotFound)
		{
			static bool bBoxShowed = false;

			if (!bBoxShowed)
			{
				bBoxShowed = true;
				FlashWindowEx(&fl); // ��� ������������ ��������� ������ ��������...
				MBoxA(L"Maximum number of consoles was reached.");
				bBoxShowed = false;
			}

			FlashWindowEx(&fl); // ��� ������������ ��������� ������ ��������...
			return;
		}

		if (abConfirm)
		{
			int nRc = RecreateDlg(&args);

			//int nRc = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_RESTART), ghWnd, Recreate DlgProc, (LPARAM)&args);
			if (nRc != IDC_START)
				return;

			CVConGroup::Redraw();
		}

		if (args.aRecreate == cra_CreateTab)
		{
			//����������, ������
			CreateCon(&args, true);
		}
		else
		{
			if (!args.pszSpecialCmd || !*args.pszSpecialCmd)
			{
				_ASSERTE((args.pszSpecialCmd && *args.pszSpecialCmd) || !abConfirm);
				args.pszSpecialCmd = lstrdup(gpSet->GetCmd());
			}

			if (!args.pszSpecialCmd || !*args.pszSpecialCmd)
			{
				_ASSERTE(args.pszSpecialCmd && *args.pszSpecialCmd);
			}
			else
			{
				// Start new ConEmu.exe process with choosen arguments...
				CreateWnd(&args);
			}
		}
	}
	else
	{
		// Restart or close console
		CVConGuard VCon;
		if ((GetActiveVCon(&VCon) >= 0) && VCon->RCon())
		{
			args.bRunAsAdministrator = abRunAs || VCon->RCon()->isAdministrator();

			if (abConfirm)
			{
				int nRc = RecreateDlg(&args);

				//int nRc = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_RESTART), ghWnd, Recreate DlgProc, (LPARAM)&args);
				if (nRc == IDC_TERMINATE)
				{
					VCon->RCon()->CloseConsole(true, false);
					return;
				}

				if (nRc != IDC_START)
					return;
			}

			if (VCon.VCon())
			{
				VCon->Redraw();
				// ����������, Recreate
				VCon->RCon()->RecreateProcess(&args);
			}
		}
	}

	SafeFree(args.pszSpecialCmd);
	SafeFree(args.pszStartupDir);
	SafeFree(args.pszUserName);
	//SafeFree(args.pszUserPassword);
}

int CConEmuMain::RecreateDlg(RConStartArgs* apArg)
{
	if (!mp_RecreateDlg)
		mp_RecreateDlg = new CRecreateDlg();

	int nRc = mp_RecreateDlg->RecreateDlg(apArg);

	return nRc;
}



BOOL CConEmuMain::RunSingleInstance(HWND hConEmuWnd /*= NULL*/, LPCWSTR apszCmd /*= NULL*/)
{
	BOOL lbAccepted = FALSE;
	LPCWSTR lpszCmd = apszCmd ? apszCmd : gpSet->GetCmd();

	if ((lpszCmd && *lpszCmd) || (gpSetCls->SingleInstanceShowHide != sih_None))
	{
		HWND ConEmuHwnd = hConEmuWnd ? hConEmuWnd : FindWindowExW(NULL, NULL, VirtualConsoleClassMain, NULL);
		MArray<HWND> hConEmuS;

		if (ConEmuHwnd)
		{
			if (hConEmuWnd)
			{
				hConEmuS.push_back(ConEmuHwnd);
			}
			else
			{
				// ����� ������ ������ ����
				while (ConEmuHwnd)
				{
					hConEmuS.push_back(ConEmuHwnd);

					ConEmuHwnd = FindWindowExW(NULL, ConEmuHwnd, VirtualConsoleClassMain, NULL);
				}
			}
		}

		for (INT_PTR i = 0; i < hConEmuS.size(); i++)
		{
			ConEmuHwnd = hConEmuS[i];

			CESERVER_REQ *pIn = NULL, *pOut = NULL;
			int nCmdLen = lpszCmd ? lstrlenW(lpszCmd) : 1;
			int nSize = sizeof(CESERVER_REQ_HDR) + sizeof(CESERVER_REQ_NEWCMD) + (nCmdLen*sizeof(wchar_t));

			pIn = ExecuteNewCmd(CECMD_NEWCMD, nSize);

			if (pIn)
			{
				pIn->NewCmd.ShowHide = gpSetCls->SingleInstanceShowHide;
				GetCurrentDirectory(countof(pIn->NewCmd.szCurDir), pIn->NewCmd.szCurDir);

				//GetModuleFileName(NULL, pIn->NewCmd.szConEmu, countof(pIn->NewCmd.szConEmu));
				wcscpy_c(pIn->NewCmd.szConEmu, ms_ConEmuExeDir);

				lstrcpyW(pIn->NewCmd.szCommand, lpszCmd ? lpszCmd : L"");
				DWORD dwPID = 0;

				if (GetWindowThreadProcessId(ConEmuHwnd, &dwPID))
					AllowSetForegroundWindow(dwPID);

				pOut = ExecuteGuiCmd(ConEmuHwnd, pIn, NULL);

				if (pOut && pOut->Data[0])
					lbAccepted = TRUE;
			}

			if (pIn) ExecuteFreeResult(pIn);

			if (pOut) ExecuteFreeResult(pOut);

			if (lbAccepted)
				break;

			// ���������� �� ��������� ����� (����� ���� �������� ��������� ����������� �� ������ �����
		}
	}

	return lbAccepted;
}

void CConEmuMain::ReportOldCmdVersion(DWORD nCmd, DWORD nVersion, int bFromServer, DWORD nFromProcess, u64 hFromModule, DWORD nBits)
{
	if (!isMainThread())
	{
		if (mb_InShowOldCmdVersion)
			return; // ��� �������
		mb_InShowOldCmdVersion = TRUE;
		
		static CESERVER_REQ_HDR info = {};
		info.cbSize = sizeof(info);
		info.nCmd = nCmd;
		info.nVersion = nVersion;
		info.nSrcPID = nFromProcess;
		info.hModule = hFromModule;
		info.nBits = nBits;
		PostMessage(ghWnd, mn_MsgOldCmdVer, bFromServer, (LPARAM)&info);
		
		return;
	}

	// ���� ��� �� ������?
	static bool lbErrorShowed = false;
	if (lbErrorShowed) return;
	
	// ���������� �������� ���������� �� �������� � ������
	HANDLE h = NULL;
	PROCESSENTRY32 pi = {sizeof(pi)};
	MODULEENTRY32  mi = {sizeof(mi)};
	LPCTSTR pszCallPath = NULL;
	#ifdef _WIN64
	if (nBits == 64)
		h = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, nFromProcess);
	else
		h = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32, nFromProcess);
	#else
	h = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, nFromProcess);
	#endif
	if (h && h != INVALID_HANDLE_VALUE)
	{
		LPCTSTR pszExePath = NULL;
		if (Module32First(h, &mi))
		{
			do {
				if (mi.th32ProcessID == nFromProcess)
				{
					if (!pszExePath)
						pszExePath = mi.szExePath[0] ? mi.szExePath : mi.szModule;

					if (!hFromModule || (mi.hModule == (HMODULE)hFromModule))
					{
						pszCallPath = mi.szExePath[0] ? mi.szExePath : mi.szModule;
						break;
					}
				}
			} while (Module32Next(h, &mi));
			if (!pszCallPath && pszExePath)
				pszCallPath = pszExePath;
		}
		CloseHandle(h);
	}
	if (!pszCallPath)
	{
		h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (h && h != INVALID_HANDLE_VALUE)
		{
			if (Process32First(h, &pi))
			{
				do {
					if (pi.th32ProcessID == nFromProcess)
					{
						pszCallPath = pi.szExeFile;
						break;
					}
				} while (Process32Next(h, &pi));
			}
			CloseHandle(h);
		}
		pszCallPath = _T("");
	}

	lbErrorShowed = true;
	int nMaxLen = 255+_tcslen(pszCallPath);
	wchar_t *pszMsg = (wchar_t*)calloc(nMaxLen,sizeof(wchar_t));
	_wsprintf(pszMsg, SKIPLEN(nMaxLen)
		L"ConEmu received wrong version packet!\nCommandID: %i, Version: %i, ReqVersion: %i, PID: %u\nPlease check %s\n%s",
		nCmd, nVersion, CESERVER_REQ_VER, nFromProcess,
		(bFromServer==1) ? L"ConEmuC*.exe" : (bFromServer==0) ? L"ConEmu*.dll and Far plugins" : L"ConEmuC*.exe and ConEmu*.dll",
		pszCallPath);
	MBox(pszMsg);
	free(pszMsg);
	mb_InShowOldCmdVersion = FALSE; // ������ ����� �������� ��� ����...
}

LRESULT CConEmuMain::OnInitMenuPopup(HWND hWnd, HMENU hMenu, LPARAM lParam)
{
	// ��� ������ ���� ��������� ��� ����, ����� �� ����� ��������� ��������� ��� ������� ����
	_ASSERTE(mn_TrackMenuPlace != tmp_None);

	DefWindowProc(hWnd, WM_INITMENUPOPUP, (WPARAM)hMenu, lParam);

	MENUITEMINFO mi = {sizeof(mi)};
	wchar_t szText[255];
	mi.fMask = MIIM_STRING; mi.dwTypeData = szText; mi.cch = countof(szText);
	BOOL bIsSysMenu = GetMenuItemInfo(hMenu, SC_CLOSE, FALSE, &mi);


	if (HIWORD(lParam))
	{
		_ASSERTE(mn_TrackMenuPlace == tmp_System);

		// ��� �������� "Help/Debug/..." submenu ���� �� ���� ��������

		if (bIsSysMenu)
		{
			UpdateSysMenu(hMenu);			

			//BOOL bSelectionExist = FALSE;

			CVConGuard VCon;
			CVirtualConsole* pVCon = (CVConGroup::GetActiveVCon(&VCon) >= 0) ? VCon.VCon() : NULL;
			//if (pVCon && pVCon->RCon())
			//	bSelectionExist = pVCon->RCon()->isSelectionPresent();

			//EnableMenuItem(hMenu, ID_CON_COPY, MF_BYCOMMAND | (bSelectionExist?MF_ENABLED:MF_GRAYED));
			if (mh_EditPopup)
			{
				TODO("���������, ��������� ��, ���� mh_EditPopup ��� ��� �������� � SystemMenu?");
				CreateEditMenuPopup(pVCon, mh_EditPopup);
			}
			else
			{
				_ASSERTE(mh_EditPopup!=NULL);
			}
			
			if (mh_VConListPopup)
			{
				CreateVConListPopupMenu(mh_VConListPopup, TRUE/*abFirstTabOnly*/);
			}
			else
			{
				_ASSERTE(mh_VConListPopup!=NULL);
			}
			
			if (mh_ActiveVConPopup)
			{
				CreateVConPopupMenu(NULL, mh_ActiveVConPopup, FALSE, mh_TerminateVConPopup);
			}
			else
			{
				_ASSERTE(mh_ActiveVConPopup!=NULL);
			}
			
			
			CheckMenuItem(hMenu, ID_DEBUG_SHOWRECTS, MF_BYCOMMAND|(gbDebugShowRects ? MF_CHECKED : MF_UNCHECKED));
			//#ifdef _DEBUG
			//		wchar_t szText[128];
			//		MENUITEMINFO mi = {sizeof(MENUITEMINFO)};
			//		mi.fMask = MIIM_STRING|MIIM_STATE;
			//		bool bLogged = false, bAllowed = false;
			//		CRealConsole* pRCon = mp_ VActive ? mp_ VActive->RCon() : NULL;
			//
			//		if (pRCon)
			//		{
			//			bLogged = pRCon->IsLogShellStarted();
			//			bAllowed = (pRCon->GetFarPID(TRUE) != 0);
			//		}
			//
			//		lstrcpy(szText, bLogged ? _T("Disable &shell log") : _T("Enable &shell log..."));
			//		mi.dwTypeData = szText;
			//		mi.fState = bAllowed ? MFS_ENABLED : MFS_GRAYED;
			//		SetMenuItemInfo(hMenu, ID_MONITOR_SHELLACTIVITY, FALSE, &mi);
			//#endif
		}
	}

	return 0;
}

int CConEmuMain::trackPopupMenu(TrackMenuPlace place, HMENU hMenu, UINT uFlags, int x, int y, HWND hWnd, RECT *prcRect /* = NULL*/)
{
	mp_Tip->HideTip();
	TrackMenuPlace prevPlace = mn_TrackMenuPlace;
	if (prevPlace == place)
	{
		_ASSERTE(prevPlace==tmp_System);
		prevPlace = tmp_None; 
	}
	_ASSERTE(prevPlace==tmp_None || prevPlace==tmp_Cmd);

	mn_TrackMenuPlace = place;

	TPMPARAMS ex = {sizeof(ex)};
	if (prcRect)
		ex.rcExclude = *prcRect;
	else
		ex.rcExclude = MakeRect(x-1,y-1,x+1,y+1);

	if (!(uFlags & (TPM_HORIZONTAL|TPM_VERTICAL)))
		uFlags |= TPM_HORIZONTAL;

	int cmd = TrackPopupMenuEx(hMenu, uFlags, x, y, hWnd, &ex);

	mn_TrackMenuPlace = prevPlace;

	mp_Tip->HideTip();

	return cmd;
}

void CConEmuMain::ShowMenuHint(HMENU hMenu, WORD nID, WORD nFlags)
{
	if (nID && (nID != MF_SEPARATOR) && !(nFlags & MF_POPUP))
	{
		//POINT pt; GetCursorPos(&pt);
		RECT rcMenuItem = {};
		BOOL lbMenuItemPos = FALSE;
		UINT nMenuID = 0;
		for (int i = 0; i < 100; i++)
		{
			nMenuID = GetMenuItemID(hMenu, i);
			if (nMenuID == nID)
			{
				lbMenuItemPos = GetMenuItemRect(ghWnd, hMenu, i, &rcMenuItem);
				break;
			}
		}
		if (lbMenuItemPos)
		{
			POINT pt = {rcMenuItem.left + (rcMenuItem.bottom - rcMenuItem.top)*2, rcMenuItem.bottom};
			//pt.x = rcMenuItem.left; //(rcMenuItem.left + rcMenuItem.right) >> 1;
			//pt.y = rcMenuItem.bottom;
			TCHAR szText[0x200];
			if (LoadString(g_hInstance, nMenuID, szText, countof(szText)))
			{
				mp_Tip->ShowTip(ghWnd, ghWnd, szText, TRUE, pt, g_hInstance);
				return;
			}
		}
	}

	mp_Tip->HideTip();
}

void CConEmuMain::ShowKeyBarHint(HMENU hMenu, WORD nID, WORD nFlags)
{
	if (nID && (nID != MF_SEPARATOR) && !(nFlags & MF_POPUP))
	{
		CVConGuard VCon;
		CVirtualConsole* pVCon = (CVConGroup::GetActiveVCon(&VCon) >= 0) ? VCon.VCon() : NULL;
		if (pVCon && pVCon->RCon())
			pVCon->RCon()->ShowKeyBarHint(nID);
	}
}

void CConEmuMain::ShowSysmenu(int x, int y, bool bAlignUp /*= false*/)
{
	//if (!Wnd)
	//	Wnd = ghWnd;

	WARNING("SysMenu: ���������� DblClick �� ������!");

	if ((x == -32000) || (y == -32000))
	{
		RECT rect, cRect;
		GetWindowRect(ghWnd, &rect);
		cRect = GetGuiClientRect();
		WINDOWINFO wInfo;   GetWindowInfo(ghWnd, &wInfo);
		int nTabShift =
		    ((gpSet->isCaptionHidden()) && gpConEmu->mp_TabBar->IsTabsShown() && (gpSet->nTabsLocation != 1))
		    ? gpConEmu->mp_TabBar->GetTabbarHeight() : 0;

		if (x == -32000)
			x = rect.right - cRect.right - wInfo.cxWindowBorders;

		if (y == -32000)
			y = rect.bottom - cRect.bottom - wInfo.cyWindowBorders + nTabShift;
	}

	bool iconic = isIconic();
	bool zoomed = isZoomed();
	bool visible = IsWindowVisible(ghWnd);
	int style = GetWindowLong(ghWnd, GWL_STYLE);
	HMENU systemMenu = gpConEmu->GetSysMenu();

	if (!systemMenu)
		return;

	if (!m_InsideIntegration)
	{
		EnableMenuItem(systemMenu, SC_RESTORE,
		               MF_BYCOMMAND | ((visible && (iconic || zoomed)) ? MF_ENABLED : MF_GRAYED));
		EnableMenuItem(systemMenu, SC_MOVE,
		               MF_BYCOMMAND | ((visible && !(iconic || zoomed)) ? MF_ENABLED : MF_GRAYED));
		EnableMenuItem(systemMenu, SC_SIZE,
		               MF_BYCOMMAND | ((visible && (!(iconic || zoomed) && (style & WS_SIZEBOX))) ? MF_ENABLED : MF_GRAYED));
		EnableMenuItem(systemMenu, SC_MINIMIZE,
		               MF_BYCOMMAND | ((visible && (!iconic && (style & WS_MINIMIZEBOX))) ? MF_ENABLED : MF_GRAYED));
		EnableMenuItem(systemMenu, SC_MAXIMIZE,
		               MF_BYCOMMAND | ((visible && (!zoomed && (style & WS_MAXIMIZEBOX))) ? MF_ENABLED : MF_GRAYED));
		EnableMenuItem(systemMenu, ID_TOTRAY, MF_BYCOMMAND | MF_ENABLED);
	}

	mn_TrackMenuPlace = tmp_System;
	SendMessage(ghWnd, WM_INITMENU, (WPARAM)systemMenu, 0);
	SendMessage(ghWnd, WM_INITMENUPOPUP, (WPARAM)systemMenu, MAKELPARAM(0, true));

	// ��������� � OnMenuPopup
	//BOOL bSelectionExist = ActiveCon()->RCon()->isSelectionPresent();
	//EnableMenuItem(systemMenu, ID_CON_COPY, MF_BYCOMMAND | (bSelectionExist?MF_ENABLED:MF_GRAYED));
	SetActiveWindow(ghWnd);
	//mb_InTrackSysMenu = TRUE;
	mn_SysMenuOpenTick = GetTickCount();
	POINT ptCurBefore = {}; GetCursorPos(&ptCurBefore);

	int command = trackPopupMenu(tmp_System, systemMenu,
		 TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_RIGHTBUTTON | (bAlignUp ? TPM_BOTTOMALIGN : 0),
		 x, y, ghWnd);
	//mb_InTrackSysMenu = FALSE;
	if (command == 0)
	{
		mn_SysMenuCloseTick = GetTickCount();

		if ((mn_SysMenuCloseTick - gpConEmu->mn_SysMenuOpenTick) < GetDoubleClickTime())
		{
			POINT ptCur = {}; GetCursorPos(&ptCur);
			if (PtDiffTest(ptCur, ptCurBefore.x, ptCurBefore.y, 8))
			{
				LRESULT lHitTest = SendMessage(ghWnd, WM_NCHITTEST, 0, MAKELONG(ptCur.x,ptCur.y));
				if (lHitTest == HTSYSMENU)
				{
					command = SC_CLOSE;
				}
			}
		}
	}
	else
	{
		mn_SysMenuCloseTick = 0;
	}

	if (Icon.isWindowInTray())
	{
		_ASSERTE(!m_InsideIntegration);

		switch (command)
		{
			case SC_RESTORE:
			case SC_MOVE:
			case SC_SIZE:
			case SC_MINIMIZE:
			case SC_MAXIMIZE:
				SendMessage(ghWnd, WM_TRAYNOTIFY, 0, WM_LBUTTONDOWN);
				break;
		}
	}

	if (command)
	{
		PostMessage(ghWnd, WM_SYSCOMMAND, (WPARAM)command, 0);
	}
}

// ������ ������� �������� GUI
bool CConEmuMain::StartDebugLogConsole()
{
	if (IsDebuggerPresent())
		return false; // ���!

	// Create process, with flag /Attach GetCurrentProcessId()
	// Sleep for sometimes, try InitHWND(hConWnd); several times
	WCHAR  szExe[MAX_PATH*2] = {0};
	bool lbRc = false;
	//DWORD nLen = 0;
	PROCESS_INFORMATION pi; memset(&pi, 0, sizeof(pi));
	STARTUPINFO si; memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	DWORD dwSelfPID = GetCurrentProcessId();
	// "/ATTACH" - ����, � �� ������������� ��� ������� ����������� � "�������������" GUI
	_wsprintf(szExe, SKIPLEN(countof(szExe)) L"\"%s\" /DEBUGPID=%u /BW=80 /BH=25 /BZ=%u",
	          WIN3264TEST(ms_ConEmuC32Full,ms_ConEmuC64Full), dwSelfPID, LONGOUTPUTHEIGHT_MAX);

	#ifdef _DEBUG
	if (MessageBox(NULL, szExe, L"StartDebugLogConsole", MB_OKCANCEL|MB_SYSTEMMODAL) != IDOK)
		return false;
	#endif
	          
	if (!CreateProcess(NULL, szExe, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS|CREATE_NEW_CONSOLE, NULL,
	                  NULL, &si, &pi))
	{
		// ������ �� ������ ��������?
		DWORD dwErr = GetLastError();
		wchar_t szErr[128]; _wsprintf(szErr, SKIPLEN(countof(szErr)) L"Can't create debugger console! ErrCode=0x%08X", dwErr);
		MBoxA(szErr);
	}
	else
	{
		gbDebugLogStarted = TRUE;
		lbRc = true;
	}

	return lbRc;
}

bool CConEmuMain::StartDebugActiveProcess()
{
	CVConGuard VCon;
	CRealConsole* pRCon = (CVConGroup::GetActiveVCon(&VCon) >= 0) ? VCon->RCon() : NULL;
	if (!pRCon)
		return false;
	DWORD dwPID = pRCon->GetActivePID();
	if (!dwPID)
		return false;

	// Create process, with flag /Attach GetCurrentProcessId()
	// Sleep for sometimes, try InitHWND(hConWnd); several times
	WCHAR  szExe[0x400] = {0};
	bool lbRc = false;
	//DWORD nLen = 0;
	PROCESS_INFORMATION pi; memset(&pi, 0, sizeof(pi));
	STARTUPINFO si = {sizeof(si)};
	WARNING("�������� ����� ���������� �� CreateCon...");
	si.dwFlags |= STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	DWORD dwSelfPID = GetCurrentProcessId();
	int W = pRCon->TextWidth();
	int H = pRCon->TextHeight();
	int nBits = GetProcessBits(dwPID);
	LPCWSTR pszServer = (nBits == 64) ? ms_ConEmuC64Full : ms_ConEmuC32Full;
	_wsprintf(szExe, SKIPLEN(countof(szExe)) L"\"%s\" /ATTACH /GID=%i /GHWND=%08X /BW=%i /BH=%i /BZ=%u /ROOT \"%s\" /DEBUGPID=%i ",
		pszServer, dwSelfPID, (DWORD)ghWnd, W, H, LONGOUTPUTHEIGHT_MAX, pszServer, dwPID);

	#ifdef _DEBUG
	if (MessageBox(NULL, szExe, L"StartDebugLogConsole", MB_OKCANCEL|MB_SYSTEMMODAL) != IDOK)
		return false;
	#endif
		
	if (!CreateProcess(NULL, szExe, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS|CREATE_NEW_CONSOLE, NULL,
		NULL, &si, &pi))
	{
		// ������ �� ������ ��������?
		DWORD dwErr = GetLastError();
		wchar_t szErr[128]; _wsprintf(szErr, SKIPLEN(countof(szErr)) L"Can't create debugger console! ErrCode=0x%08X", dwErr);
		MBoxA(szErr);
	}
	else
	{
		gbDebugLogStarted = TRUE;
		lbRc = true;
	}

	return lbRc;
}

bool CConEmuMain::MemoryDumpActiveProcess()
{
	CVConGuard VCon;
	CRealConsole* pRCon = (CVConGroup::GetActiveVCon(&VCon) >= 0) ? VCon->RCon() : NULL;
	if (!pRCon)
		return false;
	DWORD dwPID = pRCon->GetActivePID();
	if (!dwPID)
		return false;

	// Create process, with flag /Attach GetCurrentProcessId()
	// Sleep for sometimes, try InitHWND(hConWnd); several times
	WCHAR  szExe[0x400] = {0};
	bool lbRc = false;
	//DWORD nLen = 0;
	PROCESS_INFORMATION pi; memset(&pi, 0, sizeof(pi));
	STARTUPINFO si = {sizeof(si)};
	si.dwFlags |= STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWNORMAL;
	int nBits = GetProcessBits(dwPID);
	LPCWSTR pszServer = (nBits == 64) ? ms_ConEmuC64Full : ms_ConEmuC32Full;
	_wsprintf(szExe, SKIPLEN(countof(szExe)) L"\"%s\" /DEBUGPID=%i /DUMP", pszServer, dwPID);

	//#ifdef _DEBUG
	//if (MessageBox(NULL, szExe, L"StartDebugLogConsole", MB_OKCANCEL|MB_SYSTEMMODAL) != IDOK)
	//	return;
	//#endif
		
	if (!CreateProcess(NULL, szExe, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS|CREATE_NEW_CONSOLE, NULL,
		NULL, &si, &pi))
	{
		// ������ �� ������ ��������?
		DWORD dwErr = GetLastError();
		wchar_t szErr[128]; _wsprintf(szErr, SKIPLEN(countof(szErr)) L"Can't create debugger console! ErrCode=0x%08X", dwErr);
		MBoxA(szErr);
	}
	else
	{
		gbDebugLogStarted = TRUE;
		lbRc = true;
	}

	return lbRc;
}

//void CConEmuMain::StartLogCreateProcess()
//{
//    OPENFILENAME ofn; memset(&ofn,0,sizeof(ofn));
//    ofn.lStructSize=sizeof(ofn);
//    ofn.hwndOwner = ghWnd;
//    ofn.lpstrFilter = _T("Log files (*.log)\0*.log\0\0");
//    ofn.nFilterIndex = 1;
//    ofn.lpstrFile = ms_LogCreateProcess;
//    ofn.nMaxFile = MAX_PATH;
//    ofn.lpstrTitle = L"Log CreateProcess...";
//    ofn.lpstrDefExt = L"log";
//    ofn.Flags = OFN_ENABLESIZING|OFN_NOCHANGEDIR
//            | OFN_PATHMUSTEXIST|OFN_EXPLORER|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;
//    if (!GetSaveFileName(&ofn))
//        return;
//
//	mb_CreateProcessLogged = true;
//	UpdateLogCreateProcess();
//}
//
//void CConEmuMain::StopLogCreateProcess()
//{
//	mb_CreateProcessLogged = false;
//	UpdateLogCreateProcess();
//}
//
//void CConEmuMain::UpdateLogCreateProcess()
//{
//	UpdateGuiInfoMapping();
//
//	for (size_t i = 0; i < countof(mp_VCon); i++)
//	{
//		if (mp_VCon[i] == NULL)
//			continue;
//
//		DWORD nFarPID = mp_VCon[i]->RCon()->GetFarPID(TRUE);
//		if (nFarPID)
//		{
//			// ��������� � �������
//			CConEmuPipe pipe(nFarPID, 300);
//			if (pipe.Init(L"LogShell", TRUE))
//    			pipe.Execute(CMD_LOG_SHELL);
//		}
//	}
//}

void CConEmuMain::UpdateProcessDisplay(BOOL abForce)
{
	if (!ghOpWnd)
		return;

	if (!isMainThread())
	{
		PostMessage(ghWnd, mn_MsgUpdateProcDisplay, abForce, 0);
		return;
	}

	HWND hInfo = gpSetCls->mh_Tabs[gpSetCls->thi_Info];

	CVConGuard VCon;
	GetActiveVCon(&VCon);

	wchar_t szNo[32], szFlags[255]; szNo[0] = szFlags[0] = 0;
	DWORD nProgramStatus = VCon->RCon()->GetProgramStatus();
	DWORD nFarStatus = VCon->RCon()->GetFarStatus();
	if (nProgramStatus&CES_TELNETACTIVE) wcscat_c(szFlags, L"Telnet ");
	//CheckDlgButton(hInfo, cbsTelnetActive, (nProgramStatus&CES_TELNETACTIVE) ? BST_CHECKED : BST_UNCHECKED);
	if (nProgramStatus&CES_NTVDM) wcscat_c(szFlags, L"16bit ");
	//CheckDlgButton(hInfo, cbsNtvdmActive, (nProgramStatus&CES_NTVDM) ? BST_CHECKED : BST_UNCHECKED);
	if (nProgramStatus&CES_FARACTIVE) wcscat_c(szFlags, L"Far ");
	//CheckDlgButton(hInfo, cbsFarActive, (nProgramStatus&CES_FARACTIVE) ? BST_CHECKED : BST_UNCHECKED);
	if (nFarStatus&CES_FILEPANEL) wcscat_c(szFlags, L"Panels ");
	//CheckDlgButton(hInfo, cbsFilePanel, (nFarStatus&CES_FILEPANEL) ? BST_CHECKED : BST_UNCHECKED);
	if (nFarStatus&CES_EDITOR) wcscat_c(szFlags, L"Editor ");
	//CheckDlgButton(hInfo, cbsEditor, (nFarStatus&CES_EDITOR) ? BST_CHECKED : BST_UNCHECKED);
	if (nFarStatus&CES_VIEWER) wcscat_c(szFlags, L"Viewer ");
	//CheckDlgButton(hInfo, cbsViewer, (nFarStatus&CES_VIEWER) ? BST_CHECKED : BST_UNCHECKED);
	if (nFarStatus&CES_WASPROGRESS) wcscat_c(szFlags, L"%%Progress ");
	//CheckDlgButton(hInfo, cbsProgress, ((nFarStatus&CES_WASPROGRESS) /*|| VCon->RCon()->GetProgress(NULL)>=0*/) ? BST_CHECKED : BST_UNCHECKED);
	if (nFarStatus&CES_OPER_ERROR) wcscat_c(szFlags, L"%%Error ");
	//CheckDlgButton(hInfo, cbsProgressError, (nFarStatus&CES_OPER_ERROR) ? BST_CHECKED : BST_UNCHECKED);
	_wsprintf(szNo, SKIPLEN(countof(szNo)) L"%i/%i", VCon->RCon()->GetFarPID(), VCon->RCon()->GetFarPID(TRUE));

	if (hInfo)
	{
		SetDlgItemText(hInfo, tsTopPID, szNo);
		SetDlgItemText(hInfo, tsRConFlags, szFlags);
	}

	if (!abForce)
		return;

	MCHKHEAP;

	if (hInfo)
	{
		CVConGroup::OnUpdateProcessDisplay(hInfo);
	}

	MCHKHEAP
}

void CConEmuMain::UpdateCursorInfo(const CONSOLE_SCREEN_BUFFER_INFO* psbi, COORD crCursor, CONSOLE_CURSOR_INFO cInfo)
{
	if (psbi)
		mp_Status->OnConsoleChanged(psbi, &cInfo, false);
	else
		mp_Status->OnCursorChanged(&crCursor, &cInfo);

	if (!ghOpWnd || !gpSetCls->mh_Tabs[gpSetCls->thi_Info]) return;

	if (!isMainThread())
	{
		DWORD wParam = MAKELONG(crCursor.X, crCursor.Y);
		DWORD lParam = MAKELONG(cInfo.dwSize, cInfo.bVisible);
		PostMessage(ghWnd, mn_MsgUpdateCursorInfo, wParam, lParam);
		return;
	}

	TCHAR szCursor[64];
	_wsprintf(szCursor, SKIPLEN(countof(szCursor)) _T("%ix%i, %i %s"),
		(int)crCursor.X, (int)crCursor.Y,
		cInfo.dwSize, cInfo.bVisible ? L"vis" : L"hid");
	SetDlgItemText(gpSetCls->mh_Tabs[gpSetCls->thi_Info], tCursorPos, szCursor);
}

void CConEmuMain::UpdateSizes()
{
	POINT ptCur = {}; GetCursorPos(&ptCur);
	HWND hPoint = WindowFromPoint(ptCur);

	HWND hInfo = gpSetCls->mh_Tabs[gpSetCls->thi_Info];

	if (!ghOpWnd || !hInfo)
	{
		// ����� ������-�������� ����� ������ ��� ���������
		if (hPoint && ((hPoint == ghWnd) || (GetParent(hPoint) == ghWnd)))
		{
			PostMessage(ghWnd, WM_SETCURSOR, -1, -1);
		}
		return;
	}

	if (!isMainThread())
	{
		PostMessage(ghWnd, mn_MsgUpdateSizes, 0, 0);
		return;
	}

	// ����� ������-�������� ����� ������ ��� ���������
	if (hPoint && ((hPoint == ghWnd) || (GetParent(hPoint) == ghWnd)))
	{
		SendMessage(ghWnd, WM_SETCURSOR, -1, -1);
	}

	CVConGuard guard;
	GetActiveVCon(&guard);
	CVirtualConsole* pVCon = guard.VCon();

	if (pVCon)
	{
		pVCon->UpdateInfo();
	}
	else
	{
		SetDlgItemText(hInfo, tConSizeChr, _T("?"));
		SetDlgItemText(hInfo, tConSizePix, _T("?"));
		SetDlgItemText(hInfo, tPanelLeft, _T("?"));
		SetDlgItemText(hInfo, tPanelRight, _T("?"));
	}

	if (pVCon && pVCon->GetView())
	{
		RECT rcClient = pVCon->GetDcClientRect();
		TCHAR szSize[32]; _wsprintf(szSize, SKIPLEN(countof(szSize)) _T("%ix%i"), rcClient.right, rcClient.bottom);
		SetDlgItemText(hInfo, tDCSize, szSize);
	}
	else
	{
		SetDlgItemText(hInfo, tDCSize, L"<none>");
	}
}

// !!!Warning!!! ������� return. � ����� ������� ���������� ����������� CheckProcesses
void CConEmuMain::UpdateTitle(/*LPCTSTR asNewTitle*/)
{
	if (GetCurrentThreadId() != mn_MainThreadId)
	{
		/*if (TitleCmp != asNewTitle) -- ����� ���������� �� ���������������. ����� ������� ��������
		    wcscpy(TitleCmp, asNewTitle);*/
		PostMessage(ghWnd, mn_MsgUpdateTitle, 0, 0);
		return;
	}

	LPCTSTR pszNewTitle = NULL;

	if (TitleTemplate[0] != 0)
	{
		DWORD n = ExpandEnvironmentStrings(TitleTemplate, Title, countof(Title));
		if (n && (n < countof(Title)))
			pszNewTitle = Title;
		else
			pszNewTitle = GetDefaultTitle();
	}

	if (!pszNewTitle)
	{
		CVConGuard VCon;
		if ((GetActiveVCon(&VCon) >= 0) && VCon->RCon())
			pszNewTitle = VCon->RCon()->GetTitle();
	}

	if (!pszNewTitle)
	{
		//if ((pszNewTitle = mp_ VActive->RCon()->GetTitle()) == NULL)
		//	return;
		pszNewTitle = GetDefaultTitle();
	}


	if (pszNewTitle && (pszNewTitle != Title) && lstrcmpi(Title, pszNewTitle))
		lstrcpyn(Title, pszNewTitle, countof(Title));


	// SetWindowText(ghWnd, psTitle) ���������� �����
	// ��� �� ��������� L"[%i/%i] " ���� ��������� �������� � ���� ���������
	UpdateProgress(/*TRUE*/);
	Icon.UpdateTitle();
	// ��� ����� - ��������� ������ ��������� �������
	CheckProcesses();
}

// ���� � ������� ������� ���� �������� - ������������ ���
// ����� - ������������ ������������ �������� ��������� �� ���� ��������
void CConEmuMain::UpdateProgress()
{
	if (!this)
		return;

	if (GetCurrentThreadId() != mn_MainThreadId)
	{
		/*����� �� ���������� �� ��������������� */
		PostMessage(ghWnd, mn_MsgUpdateTitle, 0, 0);
		return;
	}

	LPCWSTR psTitle = NULL;
	LPCWSTR pszFixTitle = GetLastTitle(true);
	wchar_t MultiTitle[MAX_TITLE_SIZE+30];
	MultiTitle[0] = 0;
	short nProgress = -1;
	BOOL bActiveHasProgress = FALSE;
	BOOL bNeedAddToTitle = FALSE;
	BOOL bWasError = FALSE;
	BOOL bWasIndeterminate = FALSE;

	CVConGroup::GetProgressInfo(&nProgress, &bActiveHasProgress, &bWasError, &bWasIndeterminate);

	mn_Progress = min(nProgress,100);

	if (!bActiveHasProgress)
	{
		if (!bNeedAddToTitle && (nProgress >= 0))
			bNeedAddToTitle = TRUE;
	}

	static short nLastProgress = -1;
	static BOOL  bLastProgressError = FALSE;
	static BOOL  bLastProgressIndeterminate = FALSE;

	if (nLastProgress != mn_Progress  || bLastProgressError != bWasError || bLastProgressIndeterminate != bWasIndeterminate)
	{
		HRESULT hr = S_OK;

		//if (mp_TaskBar3)
		//{
		if (mn_Progress >= 0)
		{
			hr = Taskbar_SetProgressValue(mn_Progress);

			if (nLastProgress == -1 || bLastProgressError != bWasError || bLastProgressIndeterminate != bWasIndeterminate)
				hr = Taskbar_SetProgressState(bWasError ? TBPF_ERROR : bWasIndeterminate ? TBPF_INDETERMINATE : TBPF_NORMAL);
		}
		else
		{
			hr = Taskbar_SetProgressState(TBPF_NOPROGRESS);
		}
		//}

		// ��������� ���������
		nLastProgress = mn_Progress;
		bLastProgressError = bWasError;
		bLastProgressIndeterminate = bWasIndeterminate;
		UNREFERENCED_PARAMETER(hr);
	}

	if ((mn_Progress >= 0) && bNeedAddToTitle)
	{
		psTitle = MultiTitle;
		wsprintf(MultiTitle+_tcslen(MultiTitle), L"{*%i%%} ", mn_Progress);
	}

	if (gpSet->isMulti && (gpSet->isNumberInCaption || !gpConEmu->mp_TabBar->IsTabsShown()))
	{
		int nCur = 1, nCount = 0;

		nCur = GetActiveVCon(NULL, &nCount);
		if (nCur < 0)
			nCur = (nCount > 0) ? 1 : 0;
		else
			nCur++;

		if (gpSet->isNumberInCaption || (nCount > 1))
		{
			psTitle = MultiTitle;
			wsprintf(MultiTitle+_tcslen(MultiTitle), L"[%i/%i] ", nCur, nCount);
		}
	}

	if (psTitle)
		wcscat(MultiTitle, pszFixTitle);
	else
		psTitle = pszFixTitle;

	SetWindowText(ghWnd, psTitle);

	// ����� �� �������
	if (ghWndApp)
		SetWindowText(ghWndApp, psTitle);
}

void CConEmuMain::StartForceShowFrame()
{
	KillTimer(ghWnd, TIMER_CAPTION_APPEAR_ID);
	KillTimer(ghWnd, TIMER_CAPTION_DISAPPEAR_ID);
	m_ForceShowFrame = fsf_Show;
	OnHideCaption();
	//UpdateWindowRgn();
}

void CConEmuMain::StopForceShowFrame()
{
	m_ForceShowFrame = fsf_Hide;
	KillTimer(ghWnd, TIMER_CAPTION_APPEAR_ID);
	KillTimer(ghWnd, TIMER_CAPTION_DISAPPEAR_ID);
	OnHideCaption();
	//UpdateWindowRgn();
}

void CConEmuMain::UpdateWindowRgn(int anX/*=-1*/, int anY/*=-1*/, int anWndWidth/*=-1*/, int anWndHeight/*=-1*/)
{
	if (mb_LockWindowRgn)
		return;

	HRGN hRgn = NULL;

	//if (gpSet->isHideCaptionAlways) {
	//	KillTimer(ghWnd, TIMER_CAPTION_APPEAR_ID);
	//	KillTimer(ghWnd, TIMER_CAPTION_DISAPPEAR_ID);
	//}

	if (m_ForceShowFrame == fsf_Show)
		hRgn = NULL; // ����� ��� ������� ���������� ���������� (��� XP Theme) ������ � ���������
	else if (anWndWidth != -1 && anWndHeight != -1)
		hRgn = CreateWindowRgn(false, false, anX, anY, anWndWidth, anWndHeight);
	else
		hRgn = CreateWindowRgn();

	if (hRgn)
	{
		mb_LastRgnWasNull = FALSE;
	}
	else
	{
		BOOL lbPrev = mb_LastRgnWasNull;
		mb_LastRgnWasNull = TRUE;

		if (lbPrev)
			return; // ������ �� �����
	}

	// ����� ���������� ��� ���������� SetWindowRgn(ghWnd, NULL, TRUE);
	// ������ ����� ���������� ������� - ����� �������� � �����
	bool b = SetDontPreserveClient(true);
	BOOL bRc = SetWindowRgn(ghWnd, hRgn, TRUE);
	DWORD dwErr = bRc ? 0 : GetLastError();
	SetDontPreserveClient(b);
	UNREFERENCED_PARAMETER(dwErr);
	UNREFERENCED_PARAMETER(bRc);
}

#ifndef _WIN64
VOID CConEmuMain::WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD anEvent, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	_ASSERTE(hwnd!=NULL);

	#ifdef _DEBUG
	switch(anEvent)
	{
		case EVENT_CONSOLE_START_APPLICATION:

			//A new console process has started.
			//The idObject parameter contains the process identifier of the newly created process.
			//If the application is a 16-bit application, the idChild parameter is CONSOLE_APPLICATION_16BIT and idObject is the process identifier of the NTVDM session associated with the console.
			if ((idChild == CONSOLE_APPLICATION_16BIT) && gpSetCls->isAdvLogging)
			{
				char szInfo[64]; wsprintfA(szInfo, "NTVDM started, PID=%i\n", idObject);
				CVConGroup::LogString(szInfo, TRUE);
			}

			#ifdef _DEBUG
			WCHAR szDbg[128]; _wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"EVENT_CONSOLE_START_APPLICATION(HWND=0x%08X, PID=%i%s)\n", (DWORD)hwnd, idObject, (idChild == CONSOLE_APPLICATION_16BIT) ? L" 16bit" : L"");
			DEBUGSTRCONEVENT(szDbg);
			#endif

			break;
		case EVENT_CONSOLE_END_APPLICATION:

			//A console process has exited.
			//The idObject parameter contains the process identifier of the terminated process.
			if ((idChild == CONSOLE_APPLICATION_16BIT) && gpSetCls->isAdvLogging)
			{
				char szInfo[64]; wsprintfA(szInfo, "NTVDM stopped, PID=%i\n", idObject);
				CVConGroup::LogString(szInfo, TRUE);
			}

			#ifdef _DEBUG
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"EVENT_CONSOLE_END_APPLICATION(HWND=0x%08X, PID=%i%s)\n", (DWORD)hwnd, idObject,
			          (idChild == CONSOLE_APPLICATION_16BIT) ? L" 16bit" : L"");
			DEBUGSTRCONEVENT(szDbg);
			#endif

			break;
		case EVENT_CONSOLE_UPDATE_REGION: // 0x4002
		{
			//More than one character has changed.
			//The idObject parameter is a COORD structure that specifies the start of the changed region.
			//The idChild parameter is a COORD structure that specifies the end of the changed region.
			#ifdef _DEBUG
			COORD crStart, crEnd; memmove(&crStart, &idObject, sizeof(idObject)); memmove(&crEnd, &idChild, sizeof(idChild));
			WCHAR szDbg[128]; _wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"EVENT_CONSOLE_UPDATE_REGION({%i, %i} - {%i, %i})\n", crStart.X,crStart.Y, crEnd.X,crEnd.Y);
			DEBUGSTRCONEVENT(szDbg);
			#endif
		} break;
		case EVENT_CONSOLE_UPDATE_SCROLL: //0x4004
		{
			//The console has scrolled.
			//The idObject parameter is the horizontal distance the console has scrolled.
			//The idChild parameter is the vertical distance the console has scrolled.
			#ifdef _DEBUG
			WCHAR szDbg[128]; _wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"EVENT_CONSOLE_UPDATE_SCROLL(X=%i, Y=%i)\n", idObject, idChild);
			DEBUGSTRCONEVENT(szDbg);
			#endif
		} break;
		case EVENT_CONSOLE_UPDATE_SIMPLE: //0x4003
		{
			//A single character has changed.
			//The idObject parameter is a COORD structure that specifies the character that has changed.
			//Warning! � ������� ��  ���������� ��� ������ (����� ������� ���� ����������)
			//The idChild parameter specifies the character in the low word and the character attributes in the high word.
			#ifdef _DEBUG
			COORD crWhere; memmove(&crWhere, &idObject, sizeof(idObject));
			WCHAR ch = (WCHAR)LOWORD(idChild); WORD wA = HIWORD(idChild);
			WCHAR szDbg[128]; _wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"EVENT_CONSOLE_UPDATE_SIMPLE({%i, %i} '%c'(\\x%04X) A=%i)\n", crWhere.X,crWhere.Y, ch, ch, wA);
			DEBUGSTRCONEVENT(szDbg);
			#endif
		} break;
		case EVENT_CONSOLE_CARET: //0x4001
		{
			//Warning! WinXPSP3. ��� ������� �������� ������ ���� ������� � ������.
			//         � � ConEmu ��� ������� �� � ������, ��� ��� ������ �� �����������.
			//The console caret has moved.
			//The idObject parameter is one or more of the following values:
			//      CONSOLE_CARET_SELECTION or CONSOLE_CARET_VISIBLE.
			//The idChild parameter is a COORD structure that specifies the cursor's current position.
			#ifdef _DEBUG
			COORD crWhere; memmove(&crWhere, &idChild, sizeof(idChild));
			WCHAR szDbg[128]; _wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"EVENT_CONSOLE_CARET({%i, %i} Sel=%c, Vis=%c\n", crWhere.X,crWhere.Y,
			                            ((idObject & CONSOLE_CARET_SELECTION)==CONSOLE_CARET_SELECTION) ? L'Y' : L'N',
			                            ((idObject & CONSOLE_CARET_VISIBLE)==CONSOLE_CARET_VISIBLE) ? L'Y' : L'N');
			DEBUGSTRCONEVENT(szDbg);
			#endif
		} break;
		case EVENT_CONSOLE_LAYOUT: //0x4005
		{
			//The console layout has changed.
			DEBUGSTRCONEVENT(L"EVENT_CONSOLE_LAYOUT\n");
		} break;
	}
	#endif

	// ���������� ������ 16������ ����������.
	if (!(((anEvent == EVENT_CONSOLE_START_APPLICATION) || (anEvent == EVENT_CONSOLE_END_APPLICATION))
			&& (idChild == CONSOLE_APPLICATION_16BIT)))
		return;

	StartStopType sst = (anEvent == EVENT_CONSOLE_START_APPLICATION) ? sst_App16Start : sst_App16Stop;

	CVConGroup::OnDosAppStartStop(hwnd, sst, idChild);

}
#endif

/* ****************************************************** */
/*                                                        */
/*                      Painting                          */
/*                                                        */
/* ****************************************************** */

#ifdef colorer_func
{
	Painting() {};
}
#endif

void CConEmuMain::InitInactiveDC(CVirtualConsole* apVCon)
{
	PostMessage(ghWnd, mn_MsgInitInactiveDC, 0, (LPARAM)apVCon);
}

void CConEmuMain::Invalidate(CVirtualConsole* apVCon)
{
	if (!this || !apVCon || !apVCon->isVisible()) return;

	apVCon->Invalidate();
}

void CConEmuMain::InvalidateAll()
{
	InvalidateRect(ghWnd, NULL, TRUE);

	CVConGroup::InvalidateAll();

	gpConEmu->mp_TabBar->Invalidate();
}

void CConEmuMain::UpdateWindowChild(CVirtualConsole* apVCon)
{
	CVConGroup::UpdateWindowChild(apVCon);
}

//bool CConEmuMain::IsGlass()
//{
//	if (gOSVer.dwMajorVersion < 6)
//		return false;
//
//	if (!mh_DwmApi)
//	{
//		mh_DwmApi = LoadLibrary(L"dwmapi.dll");
//
//		if (!mh_DwmApi)
//			mh_DwmApi = (HMODULE)INVALID_HANDLE_VALUE;
//	}
//
//	if (mh_DwmApi != INVALID_HANDLE_VALUE)
//		return false;
//
//	if (!DwmIsCompositionEnabled && mh_DwmApi)
//	{
//		DwmIsCompositionEnabled = (FDwmIsCompositionEnabled)GetProcAddress(mh_DwmApi, "DwmIsCompositionEnabled");
//
//		if (!DwmIsCompositionEnabled)
//			return false;
//	}
//
//	BOOL composition_enabled = FALSE;
//	return DwmIsCompositionEnabled(&composition_enabled) == S_OK &&
//	       composition_enabled /*&& g_glass*/;
//}

//LRESULT CConEmuMain::OnNcMessage(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
//{
//	LRESULT lRc = 0;
//
//	if (hWnd != ghWnd)
//	{
//		lRc = DefWindowProc(hWnd, messg, wParam, lParam);
//	}
//	else
//	{
//		BOOL lbUseCaption = gpSet->isTabsInCaption && gpConEmu->mp_TabBar->IsActive();
//
//		switch(messg)
//		{
//			case WM_NCHITTEST:
//			{
//				lRc = DefWindowProc(hWnd, messg, wParam, lParam);
//
//				if (gpSet->isHideCaptionAlways() && !gpConEmu->mp_TabBar->IsShown() && lRc == HTTOP)
//					lRc = HTCAPTION;
//
//				break;
//			}
//			case WM_NCPAINT:
//			{
//				if (lbUseCaption)
//					lRc = gpConEmu->OnNcPaint((HRGN)wParam);
//				else
//					lRc = DefWindowProc(hWnd, messg, wParam, lParam);
//
//				break;
//			}
//			case WM_NCACTIVATE:
//			{
//				// Force paint our non-client area otherwise Windows will paint its own.
//				if (lbUseCaption)
//				{
//					// ��� ��� ���������� �����, ����� ���� ����� �����
//					_ASSERTE(lbUseCaption==FALSE);
//					RedrawWindow(hWnd, NULL, NULL, RDW_UPDATENOW);
//				}
//				else
//				{
//					// ��� ������ ������
//					if (!wParam)
//					{
//						CVirtualConsole* pVCon = ActiveCon();
//						CRealConsole* pRCon;
//						// ����� ����������, ����� ��� �������� ������ � �������� GUI ����������
//						// ����� ���� ������ ConEmu �� ����� ���������� (�����)
//						if (pVCon && ((pRCon = ActiveCon()->RCon()) != NULL) && pRCon->GuiWnd() && !pRCon->isBufferHeight())
//							wParam = TRUE;
//					}
//					lRc = DefWindowProc(hWnd, messg, wParam, lParam);
//				}
//
//				break;
//			}
//			case 0x31E: // WM_DWMCOMPOSITIONCHANGED:
//			{
//				if (lbUseCaption)
//				{
//					lRc = 0;
//					/*
//					DWMNCRENDERINGPOLICY policy = g_glass ? DWMNCRP_ENABLED : DWMNCRP_DISABLED;
//					DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_POLICY,
//					                    &policy, sizeof(DWMNCRENDERINGPOLICY));
//
//					SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
//					           SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
//					RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
//					*/
//				}
//				else
//					lRc = DefWindowProc(hWnd, messg, wParam, lParam);
//
//				break;
//			}
//			case 0xAE: // WM_NCUAHDRAWCAPTION:
//			case 0xAF: // WM_NCUAHDRAWFRAME:
//			{
//				lRc = lbUseCaption ? 0 : DefWindowProc(hWnd, messg, wParam, lParam);
//				break;
//			}
//			case WM_NCCALCSIZE:
//			{
//				NCCALCSIZE_PARAMS *pParms = NULL;
//				LPRECT pRect = NULL;
//				if (wParam) pParms = (NCCALCSIZE_PARAMS*)lParam; else pRect = (LPRECT)lParam;
//
//				lRc = DefWindowProc(hWnd, messg, wParam, lParam);
//				break;
//			}
//			default:
//				if (messg == WM_NCLBUTTONDOWN && wParam == HTSYSMENU)
//					GetSysMenu(); // ��������� ������������ ���������� ����
//				lRc = DefWindowProc(hWnd, messg, wParam, lParam);
//		}
//	}
//
//	return lRc;
//}

//LRESULT CConEmuMain::OnNcPaint(HRGN hRgn)
//{
//	LRESULT lRc = 0, lMyRc = 0;
//	//HRGN hFrameRgn = hRgn;
//	RECT wr = {0}, dirty = {0}, dirty_box = {0};
//	GetWindowRect(ghWnd, &wr);
//
//	if (!hRgn || ((WPARAM)hRgn) == 1)
//	{
//		dirty = wr;
//		dirty.left = dirty.top = 0;
//	}
//	else
//	{
//		GetRgnBox(hRgn, &dirty_box);
//
//		if (!IntersectRect(&dirty, &dirty_box, &wr))
//			return 0;
//
//		OffsetRect(&dirty, -wr.left, -wr.top);
//	}
//
//	//hdc = GetWindowDC(hwnd);
//	//br = CreateSolidBrush(RGB(255,0,0));
//	//FillRect(hdc, &dirty, br);
//	//DeleteObject(br);
//	//ReleaseDC(hwnd, hdc);
//	int nXFrame = GetSystemMetrics(SM_CXSIZEFRAME);
//	int nYFrame = GetSystemMetrics(SM_CYSIZEFRAME);
//	int nPad = GetSystemMetrics(92/*SM_CXPADDEDBORDER*/);
//	int nXBtn = GetSystemMetrics(SM_CXSIZE);
//	int nYBtn = GetSystemMetrics(SM_CYSIZE);
//	int nYCaption = GetSystemMetrics(SM_CYCAPTION);
//	int nXMBtn = GetSystemMetrics(SM_CXSMSIZE);
//	int nYMBtn = GetSystemMetrics(SM_CYSMSIZE);
//	int nXIcon = GetSystemMetrics(SM_CXSMICON);
//	int nYIcon = GetSystemMetrics(SM_CYSMICON);
//	RECT rcWnd; GetWindowRect(ghWnd, &rcWnd);
//	RECT rcUsing =
//	{
//		nXFrame + nXIcon, nYFrame,
//		(rcWnd.right-rcWnd.left) - nXFrame - 3*nXBtn,
//		nYFrame + nYCaption
//	};
//	//lRc = DefWindowProc(ghWnd, WM_NCPAINT, (WPARAM)hRgn, 0);
//	//HRGN hRect = CreateRectRgn(rcUsing.left,rcUsing.top,rcUsing.right,rcUsing.bottom);
//	//hFrameRgn = CreateRectRgn(0,0,1,1);
//	//int nRgn = CombineRgn(hFrameRgn, hRgn, hRect, RGN_XOR);
//	//if (nRgn == ERROR)
//	//{
//	//	DeleteObject(hFrameRgn);
//	//	hFrameRgn = hRgn;
//	//}
//	//else
//	{
//		// ������
//		HDC hdc = NULL;
//		//hdc = GetDCEx(ghWnd, hRect, DCX_WINDOW|DCX_INTERSECTRGN);
//		//hRect = NULL; // system maintains this region
//		hdc = GetWindowDC(ghWnd);
//		mp_TabBar->PaintHeader(hdc, rcUsing);
//		ReleaseDC(ghWnd, hdc);
//	}
//	//if (hRect)
//	//	DeleteObject(hRect);
//	lMyRc = TRUE;
//	//lRc = DefWindowProc(ghWnd, WM_NCPAINT, (WPARAM)hFrameRgn, 0);
//	//
//	//if (hRgn != hFrameRgn)
//	//{
//	//	DeleteObject(hFrameRgn); hFrameRgn = NULL;
//	//}
//	return lMyRc ? lMyRc : lRc;
//}

bool CConEmuMain::isRightClickingPaint()
{
	return (bool)mb_RightClickingPaint;
}

void CConEmuMain::RightClickingPaint(HDC hdcIntVCon, CVirtualConsole* apVCon)
{
	//TODO: ���� �������� PanelView - �� "��� ���������" ��������� ������ �����
	if (hdcIntVCon == (HDC)INVALID_HANDLE_VALUE)
	{
		//if (mh_RightClickingWnd)
		//	apiShowWindow(mh_RightClickingWnd, SW_HIDE);
		return;
	}

	BOOL lbSucceeded = FALSE;

	CVConGuard VCon;
	if (GetActiveVCon(&VCon) >= 0)
	{
		if (!apVCon)
		{
			apVCon = VCon.VCon();
		}
		// �� ����, ������ ����� ��������� ������ � �������� �������
		_ASSERTE(apVCon == VCon.VCon());
	}


	HWND hView = apVCon ? apVCon->GetView() : NULL;

	if (!hView || gpSet->isDisableMouse)
	{
		DEBUGSTRRCLICK(L"RightClickingPaint: !hView || gpSet->isDisableMouse\n");
	}
	else
	{
		if (gpSet->isRClickTouchInvert())
		{
			// ������� ���� � ������ ��������?
			lbSucceeded = FALSE;

			DEBUGSTRRCLICK(L"RightClickingPaint: gpSet->isRClickTouchInvert()\n");

			//if (!mb_RightClickingLSent && apVCon)
			//{
			//	mb_RightClickingLSent = TRUE;
			//	// ����� ���������� ������ � ������� ����� ��� ������
			//	// ����� ���������� ���������, ��� ������ ������� ������ �����
			//	// ���������� EMenu, � �� ����� (���� �������� "������") ������ �� ���������.
			//	apVCon->RCon()->PostLeftClickSync(mouse.RClkDC);
			//	//apVCon->RCon()->OnMouse(WM_MOUSEMOVE, 0, mouse.RClkDC.X, mouse.RClkDC.Y, true);
			//	//WARNING("�� ��������, ����� ��������� ���� ���� ������������");
			//	//apVCon->RCon()->PostMacro(L"MsLClick");
			//	//WARNING("!!! �������� �� CMD_LEFTCLKSYNC?");
			//}
		}
		else if ((gpSet->isRClickSendKey > 1) && (mouse.state & MOUSE_R_LOCKED)
			&& (m_RightClickingFrames > 0) && mh_RightClickingBmp)
		{
			//WORD nRDown = GetKeyState(VK_RBUTTON);
			//POINT ptCur; GetCursorPos(&ptCur);
			//ScreenToClient(hView, &ptCur);
			bool bRDown = isPressed(VK_RBUTTON);

			if (bRDown)
			{
				DWORD dwCurTick = TimeGetTime(); //GetTickCount();
				DWORD dwDelta = dwCurTick - mouse.RClkTick;

				if (dwDelta < RCLICKAPPS_START)
				{
					// ���� �������� �� ������
					lbSucceeded = TRUE;
				}
				// ���� ������� ������ 10��� - ��� �����
				else if (dwDelta > RCLICKAPPSTIMEOUT_MAX/*10���*/)
				{
					lbSucceeded = FALSE;
				}
				else
				{
					lbSucceeded = TRUE;

					if (!mb_RightClickingLSent && apVCon)
					{
						mb_RightClickingLSent = TRUE;
						// ����� ���������� ������ � ������� ����� ��� ������
						// ����� ���������� ���������, ��� ������ ������� ������ �����
						// ���������� EMenu, � �� ����� (���� �������� "������") ������ �� ���������.
						apVCon->RCon()->PostLeftClickSync(mouse.RClkDC);
						//apVCon->RCon()->OnMouse(WM_MOUSEMOVE, 0, mouse.RClkDC.X, mouse.RClkDC.Y, true);
						//WARNING("�� ��������, ����� ��������� ���� ���� ������������");
						//apVCon->RCon()->PostMacro(L"MsLClick");
						//WARNING("!!! �������� �� CMD_LEFTCLKSYNC?");
					}

					// ��������� ������ ������
					int nIndex = (dwDelta - RCLICKAPPS_START) * m_RightClickingFrames / (RCLICKAPPSTIMEOUT - RCLICKAPPS_START);

					if (nIndex >= m_RightClickingFrames)
					{
						nIndex = (m_RightClickingFrames-1); // ������ ��������� �����, ����� ����� ���������
						//-- KillTimer(ghWnd, TIMER_RCLICKPAINT); // ������ ����������� ��� "�������" �������� ����� RCLICKAPPSTIMEOUT_MAX
					}

					#ifdef _DEBUG
					wchar_t szDbg[128];
					_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"RightClickingPaint: Delta=%u, nIndex=%u, {%ix%i}\n", dwDelta, nIndex, Rcursor.x, Rcursor.y);
					DEBUGSTRRCLICK(szDbg);
					#endif

					//if (hdcIntVCon || (m_RightClickingCurrent != nIndex))
					{
						// ������
						HDC hdcSelf = NULL;
						const wchar_t szRightClickingClass[] = L"ConEmu_RightClicking";
						//BOOL lbSelfDC = FALSE;

						if (!mb_RightClickingRegistered)
						{
							WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_OWNDC, CConEmuMain::RightClickingProc, 0, 0,
							                 g_hInstance, NULL, LoadCursor(NULL, IDC_ARROW),
							                 NULL, NULL, szRightClickingClass, NULL};
							if (!RegisterClassEx(&wc))
							{
								DisplayLastError(L"Regitser class failed");
							}
							else
							{
								mb_RightClickingRegistered = TRUE;
							}
						}

						int nHalf = m_RightClickingSize.y>>1;

						if (mb_RightClickingRegistered && (!mh_RightClickingWnd || !IsWindow(mh_RightClickingWnd)))
						{
							POINT pt = {Rcursor.x-nHalf, Rcursor.y-nHalf};
							MapWindowPoints(hView, ghWnd, &pt, 1);
							mh_RightClickingWnd = CreateWindow(szRightClickingClass, L"",
								WS_VISIBLE|WS_CHILD|WS_DISABLED|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
                                pt.x, pt.y, m_RightClickingSize.y, m_RightClickingSize.y,
                                ghWnd, (HMENU)9999, g_hInstance, NULL);
							SetWindowPos(mh_RightClickingWnd, HWND_TOP, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);
						}
						else
						{
							if (!IsWindowVisible(mh_RightClickingWnd))
								apiShowWindow(mh_RightClickingWnd, SW_SHOWNORMAL);
						}

						if (mh_RightClickingWnd && ((hdcSelf = GetDC(mh_RightClickingWnd)) != NULL))
						{
							DEBUGSTRRCLICK(L"RightClickingPaint: Painting...\n");

							HDC hCompDC = CreateCompatibleDC(hdcSelf);
							HBITMAP hOld = (HBITMAP)SelectObject(hCompDC, mh_RightClickingBmp);

							BLENDFUNCTION bf = {AC_SRC_OVER,0,255,AC_SRC_ALPHA};

							if (hdcIntVCon)
							{
								// ���� �������� ���������� ������� - ��� ����� "��������" � � ����� "������" � ���������
								BitBlt(hdcSelf, 0, 0, m_RightClickingSize.y, m_RightClickingSize.y,
									hdcIntVCon, Rcursor.x-nHalf, Rcursor.y-nHalf, SRCCOPY);
							}

							GdiAlphaBlend(hdcSelf, 0, 0, m_RightClickingSize.y, m_RightClickingSize.y,
								  hCompDC, nIndex*m_RightClickingSize.y, 0, m_RightClickingSize.y, m_RightClickingSize.y, bf);

							if (hOld && hCompDC)
								SelectObject(hCompDC, hOld);

							//if (/*lbSelfDC &&*/ hdcSelf)
							DeleteDC(hdcSelf);
						}
					}

					// �������� �����, ��� �������� � ��������� ���
					m_RightClickingCurrent = nIndex;
				}
			}
		}
		else
		{
			DEBUGSTRRCLICK(L"RightClickingPaint: Condition failed\n");
		}
	}

	if (!lbSucceeded)
	{
		StopRightClickingPaint();
	}
}

void CConEmuMain::StartRightClickingPaint()
{
	if (!mb_RightClickingPaint)
	{
		if (m_RightClickingFrames > 0 && mh_RightClickingBmp)
		{
			m_RightClickingCurrent = -1;
			mb_RightClickingPaint = TRUE;
			mb_RightClickingLSent = FALSE;
			SetTimer(ghWnd, TIMER_RCLICKPAINT, TIMER_RCLICKPAINT_ELAPSE, NULL);
		}
	}
}

void CConEmuMain::StopRightClickingPaint()
{
	DEBUGSTRRCLICK(L"StopRightClickingPaint\n");

	if (mh_RightClickingWnd)
	{
		DestroyWindow(mh_RightClickingWnd);
		mh_RightClickingWnd = NULL;
	}

	if (mb_RightClickingPaint)
	{
		mb_RightClickingPaint = FALSE;
		mb_RightClickingLSent = FALSE;
		KillTimer(ghWnd, TIMER_RCLICKPAINT);
		m_RightClickingCurrent = -1;
		CVConGroup::InvalidateAll();
	}
}

// ����� ����� ������ � ���, ����� ������������ ������ ���������� PanelView
LRESULT CConEmuMain::RightClickingProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	switch (messg)
	{
	case WM_CREATE:
		return 0; // allow
	case WM_PAINT:
		{
			// ���� ����� �� ������� - � ������� "��������" WM_PAINT
			PAINTSTRUCT ps = {};
			BeginPaint(hWnd, &ps);
			//RECT rcClient; GetClientRect(hWnd, &rcClient);
			//FillRect(ps.hdc, &rcClient, (HBRUSH)GetStockObject(WHITE_BRUSH));
			EndPaint(hWnd, &ps);
		}
		return 0;
	case WM_ERASEBKGND:
		return 0;
	}

	return ::DefWindowProc(hWnd, messg, wParam, lParam);
}

void CConEmuMain::OnPaintClient(HDC hdc, int width, int height)
{
	// ���� "�����" PostUpdate
	if (mp_TabBar->NeedPostUpdate())
		mp_TabBar->Update();

#ifdef _DEBUG
	RECT rcDbgSize; GetWindowRect(ghWnd, &rcDbgSize);
	wchar_t szSize[255]; _wsprintf(szSize, SKIPLEN(countof(szSize)) L"WM_PAINT -> Window size (X=%i, Y=%i, W=%i, H=%i)\n",
	                               rcDbgSize.left, rcDbgSize.top, (rcDbgSize.right-rcDbgSize.left), (rcDbgSize.bottom-rcDbgSize.top));
	DEBUGSTRSIZE(szSize);
	static RECT rcDbgSize1;

	if (memcmp(&rcDbgSize1, &rcDbgSize, sizeof(rcDbgSize1)))
	{
		rcDbgSize1 = rcDbgSize;
	}
#endif

	PaintGaps(hdc);
}

#if 0
LRESULT CConEmuMain::OnPaint(WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	// ���� "�����" PostUpdate
	if (mp_TabBar->NeedPostUpdate())
		mp_TabBar->Update();

#ifdef _DEBUG
	RECT rcDbgSize; GetWindowRect(ghWnd, &rcDbgSize);
	wchar_t szSize[255]; _wsprintf(szSize, SKIPLEN(countof(szSize)) L"WM_PAINT -> Window size (X=%i, Y=%i, W=%i, H=%i)\n",
	                               rcDbgSize.left, rcDbgSize.top, (rcDbgSize.right-rcDbgSize.left), (rcDbgSize.bottom-rcDbgSize.top));
	DEBUGSTRSIZE(szSize);
	static RECT rcDbgSize1;

	if (memcmp(&rcDbgSize1, &rcDbgSize, sizeof(rcDbgSize1)))
	{
		rcDbgSize1 = rcDbgSize;
	}
#endif


	PAINTSTRUCT ps;
	#ifdef _DEBUG
	HDC hDc =
	#endif
	BeginPaint(ghWnd, &ps);
	//RECT rcClient; Get ClientRect(ghWnd, &rcClient);
	//RECT rcTabMargins = CalcMargins(CEM_TAB);
	//AddMargins(rcClient, rcTabMargins, FALSE);
	//HDC hdc = GetDC(ghWnd);
	PaintGaps(ps.hdc);
	//PaintCon(ps.hdc);

	// ��������� ���� � hView
	//if (mb_RightClickingPaint)
	//{
	//	// �������� ��������, ��� ������� ������, ���� ������ ���������
	//	RightClickingPaint(ps.hdc);
	//}

	EndPaint(ghWnd, &ps);
	//result = DefWindowProc(ghWnd, WM_PAINT, wParam, lParam);
	//ReleaseDC(ghWnd, hdc);
	//ValidateRect(ghWnd, &rcClient);
	return result;
}
#endif

void CConEmuMain::InvalidateGaps()
{
	if (isIconic())
		return;

	CVConGroup::InvalidateGaps();
}

void CConEmuMain::PaintGaps(HDC hDC)
{
	CVConGroup::PaintGaps(hDC);
}

//void CConEmuMain::PaintCon(HDC hPaintDC)
//{
//	//if (ProgressBars)
//	//    ProgressBars->OnTimer();
//
//	// ���� "�����" PostUpdate
//	if (mp_TabBar->NeedPostUpdate())
//		mp_TabBar->Update();
//
//	RECT rcClient = {0};
//
//	if ('ghWnd DC')
//	{
//		Get ClientRect('ghWnd DC', &rcClient);
//		MapWindowPoints('ghWnd DC', ghWnd, (LPPOINT)&rcClient, 2);
//	}
//
//	// ���� mp_ VActive==NULL - ����� ������ ��������� ������� �����.
//	mp_ VActive->Paint(hPaintDC, rcClient);
//
//#ifdef _DEBUG
//	if ((GetKeyState(VK_SCROLL) & 1) && (GetKeyState(VK_CAPITAL) & 1))
//	{
//		DebugStep(L"ConEmu: Sleeping in PaintCon for 1s");
//		Sleep(1000);
//		DebugStep(NULL);
//	}
//#endif
//}

void CConEmuMain::RePaint()
{
	mb_SkipSyncSize = true;

	gpConEmu->mp_TabBar->RePaint();
	//m_Back->RePaint();
	HDC hDc = GetDC(ghWnd);
	//mp_ VActive->Paint(hDc); // ���� mp_ VActive==NULL - ����� ������ ��������� ������� �����.
	PaintGaps(hDc);
	//PaintCon(hDc);
	ReleaseDC(ghWnd, hDc);

	CVConGroup::RePaint();

	mb_SkipSyncSize = false;
}

void CConEmuMain::Update(bool isForce /*= false*/)
{
	CVConGroup::Update(isForce);
}

/* ****************************************************** */
/*                                                        */
/*                 Status functions                       */
/*                                                        */
/* ****************************************************** */

#ifdef colorer_func
{
	Status_Functions() {};
}
#endif

DWORD CConEmuMain::GetFarPID(BOOL abPluginRequired/*=FALSE*/)
{
	DWORD dwPID = CVConGroup::GetFarPID(abPluginRequired);
	return dwPID;
}

LPCTSTR CConEmuMain::GetLastTitle(bool abUseDefault/*=true*/)
{
	if (!Title[0] && abUseDefault)
		return GetDefaultTitle();

	return Title;
}

LPCTSTR CConEmuMain::GetVConTitle(int nIdx)
{
	//if (nIdx < 0 || nIdx >= MAX_CONSOLE_COUNT)
	//	return NULL;

	CVConGuard VCon;
	if (!CVConGroup::GetVCon(nIdx, &VCon))
		return NULL;

	if (!VCon.VCon() || !VCon->RCon())
		return NULL;

	LPCWSTR pszTitle = VCon->RCon()->GetTitle();
	if (pszTitle == NULL)
	{
		_ASSERTE(pszTitle!=NULL);
		pszTitle = GetDefaultTitle();
	}
	return pszTitle;
}

// ���������� ������ (0-based) �������� �������
int CConEmuMain::GetActiveVCon(CVConGuard* pVCon /*= NULL*/, int* pAllCount /*= NULL*/)
{
	return CVConGroup::GetActiveVCon(pVCon, pAllCount);
}

bool CConEmuMain::IsActiveConAdmin()
{
	CVConGuard VCon;
	bool bAdmin = (gpConEmu->GetActiveVCon(&VCon) >= 0)
		? VCon->RCon()->isAdministrator()
		: gpConEmu->mb_IsUacAdmin;
	return bAdmin;
}

// bFromCycle = true, ��� �������� � ������ (��������, � CSettings::UpdateWinHookSettings), ����� �� �������� �������
CVirtualConsole* CConEmuMain::GetVCon(int nIdx, bool bFromCycle /*= false*/)
{
	CVConGuard VCon;
	if (!CVConGroup::GetVCon(nIdx, &VCon))
	{
		_ASSERTE((nIdx>=0 && (nIdx<(int)MAX_CONSOLE_COUNT || (bFromCycle && nIdx==(int)MAX_CONSOLE_COUNT))));
		return NULL;
	}

	return VCon.VCon();
}

// 0 - ����� ������� ���
// 1..MAX_CONSOLE_COUNT - "�����" ������� (1 based!)
int CConEmuMain::isVConValid(CVirtualConsole* apVCon)
{
	int nIdx = CVConGroup::GetVConIndex(apVCon);
	if (nIdx >= 0)
		return (nIdx+1);
	return 0;
}

CVirtualConsole* CConEmuMain::GetVConFromPoint(POINT ptScreen)
{
	CVConGuard VCon;
	if (!CVConGroup::GetVConFromPoint(ptScreen, &VCon))
		return NULL;

	return VCon.VCon();
}

bool CConEmuMain::isActive(CVirtualConsole* apVCon, bool abAllowGroup /*= true*/)
{
	return CVConGroup::isActive(apVCon, abAllowGroup);
}

bool CConEmuMain::isConSelectMode()
{
	return CVConGroup::isConSelectMode();
}

// ���������� true ���� ����� ShellDrag
bool CConEmuMain::isDragging()
{
	if ((mouse.state & (DRAG_L_STARTED | DRAG_R_STARTED)) == 0)
		return false;

	if (isConSelectMode())
	{
		mouse.state &= ~(DRAG_L_STARTED | DRAG_L_ALLOWED | DRAG_R_STARTED | DRAG_R_ALLOWED | MOUSE_DRAGPANEL_ALL);
		return false;
	}

	return true;
}

bool CConEmuMain::isFilePanel(bool abPluginAllowed/*=false*/)
{
	return CVConGroup::isFilePanel(abPluginAllowed);
}

bool CConEmuMain::isFirstInstance()
{
	if (!mb_AliveInitialized)
	{
		mb_AliveInitialized = TRUE;
		// �������� �������, ����� �� ���� ������� � ������ /SINGLE
		lstrcpy(ms_ConEmuAliveEvent, CEGUI_ALIVE_EVENT);
		DWORD nSize = MAX_PATH;
		// ������� ��� �������� �����. ��� �� ����� ��������� ��� ������� ���������� ������.
		GetUserName(ms_ConEmuAliveEvent+_tcslen(ms_ConEmuAliveEvent), &nSize);
		WARNING("Event �� ��������, ���� conemu.exe ������� ��� ������ �������������");
		mh_ConEmuAliveEvent = CreateEvent(NULL, TRUE, TRUE, ms_ConEmuAliveEvent);
		nSize = GetLastError();

		// ��� ������������ ������������ ����� ��������� �������, ������� ����������� � ����� Event
		if (!mh_ConEmuAliveEvent /* || nSize == ERROR_PATH_NOT_FOUND */)
		{
			lstrcpy(ms_ConEmuAliveEvent, CEGUI_ALIVE_EVENT);
			mh_ConEmuAliveEvent = CreateEvent(NULL, TRUE, TRUE, ms_ConEmuAliveEvent);
			nSize = GetLastError();
		}

		mb_ConEmuAliveOwned = mh_ConEmuAliveEvent && (nSize!=ERROR_ALREADY_EXISTS);
	}

	if (mh_ConEmuAliveEvent && !mb_ConEmuAliveOwned)
	{
		if (WaitForSingleObject(mh_ConEmuAliveEvent,0) == WAIT_TIMEOUT)
		{
			SetEvent(mh_ConEmuAliveEvent);
			mb_ConEmuAliveOwned = TRUE;

			// ���� ��������� ���������� "��������" (������, ����� ������ ��������, ��� ������)
			RegisterMinRestore(true);
		}
	}

	return mb_ConEmuAliveOwned;
}

bool CConEmuMain::isEditor()
{
	return CVConGroup::isEditor();
}

bool CConEmuMain::isFar(bool abPluginRequired/*=false*/)
{
	return CVConGroup::isFar(abPluginRequired);
}

// ���� �� ��� ���-��?
bool CConEmuMain::isFarExist(CEFarWindowType anWindowType/*=fwt_Any*/, LPWSTR asName/*=NULL*/, CVConGuard* rpVCon/*=NULL*/)
{
	return CVConGroup::isFarExist(anWindowType, asName, rpVCon);
}

bool CConEmuMain::isLBDown()
{
	return (mouse.state & DRAG_L_ALLOWED) == DRAG_L_ALLOWED;
}

bool CConEmuMain::isMainThread()
{
	DWORD dwTID = GetCurrentThreadId();
	return dwTID == mn_MainThreadId;
}

bool CConEmuMain::isMeForeground(bool abRealAlso/*=false*/, bool abDialogsAlso/*=true*/)
{
	if (!this) return false;

	static HWND hLastFore = NULL;
	static bool isMe = false;
	static bool bLastRealAlso, bLastDialogsAlso;
	HWND h = getForegroundWindow();

	if ((h != hLastFore) || (bLastRealAlso != abRealAlso) || (bLastDialogsAlso != abDialogsAlso))
	{
		DWORD nForePID = 0;
		if (h)
			GetWindowThreadProcessId(h, &nForePID);
		isMe = (h != NULL)
			&& ((h == ghWnd)
				|| (abDialogsAlso && (nForePID == GetCurrentProcessId()))
					//((h == ghOpWnd)
					//|| (mp_AttachDlg && (mp_AttachDlg->GetHWND() == h))
					//|| (mp_RecreateDlg && (mp_RecreateDlg->GetHWND() == h))
					//|| (gpSetCls->mh_FindDlg == h)))
				|| (m_InsideIntegration && (h == mh_InsideParentRoot)))
			;

		if (h && !isMe && abRealAlso)
		{
			if (CVConGroup::isOurConsoleWindow(h))
				isMe = true;
		}

		hLastFore = h;
		bLastRealAlso = abRealAlso;
		bLastDialogsAlso = abDialogsAlso;
	}

	return isMe;
}

bool CConEmuMain::isMouseOverFrame(bool abReal)
{
	if (m_ForceShowFrame && isSizing())
	{
		if (!isPressed(VK_LBUTTON))
		{
			mouse.state &= ~MOUSE_SIZING_BEGIN;
		}
		else
		{
			return true;
		}
	}

	bool bCurForceShow = false;

	if (abReal)
	{
		POINT ptMouse; GetCursorPos(&ptMouse);
		RECT rcWnd; GetWindowRect(ghWnd, &rcWnd);
		// ����� ������� ��������� ����� ���� ���� ��������, �������� ������
		int nAdd = 2;
		rcWnd.top -= nAdd; rcWnd.left -= nAdd; rcWnd.right += nAdd; rcWnd.bottom += nAdd;

		if (PtInRect(&rcWnd, ptMouse))
		{
			RECT rcClient = GetGuiClientRect();
			// ����� ������� ��������� ����� ���� ���� ��������, �������� ������
			int nSub = 1;
			rcClient.top += nSub; rcClient.left += nSub; rcClient.right -= nSub; rcClient.bottom -= nSub;
			MapWindowPoints(ghWnd, NULL, (LPPOINT)&rcClient, 2);

			if (!PtInRect(&rcClient, ptMouse))
				bCurForceShow = true;
		}
	}
	else
	{
		bCurForceShow = (m_ForceShowFrame == fsf_Show);
	}

	return bCurForceShow;
}

bool CConEmuMain::isNtvdm(BOOL abCheckAllConsoles/*=FALSE*/)
{
	return CVConGroup::isNtvdm(abCheckAllConsoles);
}

bool CConEmuMain::isOurConsoleWindow(HWND hCon)
{
	return CVConGroup::isOurConsoleWindow(hCon);
}

bool CConEmuMain::isValid(CRealConsole* apRCon)
{
	return CVConGroup::isValid(apRCon);
}

bool CConEmuMain::isValid(CVirtualConsole* apVCon)
{
	return CVConGroup::isValid(apVCon);
}

bool CConEmuMain::isVConExists(int nIdx)
{
	return CVConGroup::isVConExists(nIdx);
}

bool CConEmuMain::isVConHWND(HWND hChild, CVConGuard* pVCon /*= NULL*/)
{
	return CVConGroup::isVConHWND(hChild, pVCon);
}

bool CConEmuMain::isViewer()
{
	return CVConGroup::isViewer();
}

bool CConEmuMain::isVisible(CVirtualConsole* apVCon)
{
	return CVConGroup::isVisible(apVCon);
}

bool CConEmuMain::isChildWindow()
{
	return CVConGroup::isChildWindow();
}

// ��������� ���� ��� PictureView ������ ����� ������������� �������������, ��� ���
// ������������ �� ���� ��� ����������� "���������" - ������
bool CConEmuMain::isPictureView()
{
	return CVConGroup::isPictureView();
}

bool CConEmuMain::isProcessCreated()
{
	return (mb_ProcessCreated != FALSE);
}

bool CConEmuMain::isSizing()
{
	// ���� ����� ������ ����� ����
	if ((mouse.state & MOUSE_SIZING_BEGIN) != MOUSE_SIZING_BEGIN)
		return false;

	// ����� �� ����������, ��������
	if (!isPressed(VK_LBUTTON))
	{
		mouse.state &= ~MOUSE_SIZING_BEGIN;
		mouse.bCheckNormalRect = true;
		return false;
	}

	return true;
}

// ���� ����� ������ ������ LOWORD �� HKL
void CConEmuMain::SwitchKeyboardLayout(DWORD_PTR dwNewKeybLayout)
{
	if ((gpSet->isMonitorConsoleLang & 1) == 0)
	{
		if (gpSetCls->isAdvLogging > 1)
			LogString(L"CConEmuMain::SwitchKeyboardLayout skipped, cause of isMonitorConsoleLang==0");

		return;
	}

#ifdef _DEBUG
	wchar_t szDbg[128]; _wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"CConEmuMain::SwitchKeyboardLayout(0x%08I64X)\n", (unsigned __int64)dwNewKeybLayout);
	DEBUGSTRLANG(szDbg);
#endif
	HKL hKeyb[20]; UINT nCount, i; BOOL lbFound = FALSE;
	nCount = GetKeyboardLayoutList(countof(hKeyb), hKeyb);

	for(i = 0; !lbFound && i < nCount; i++)
	{
		if (hKeyb[i] == (HKL)dwNewKeybLayout)
			lbFound = TRUE;
	}

	WARNING("������ � ������� ����������� ����� �������. US Dvorak?");

	for(i = 0; !lbFound && i < nCount; i++)
	{
		if ((((DWORD_PTR)hKeyb[i]) & 0xFFFF) == (dwNewKeybLayout & 0xFFFF))
		{
			lbFound = TRUE; dwNewKeybLayout = (DWORD_PTR)hKeyb[i];
		}
	}

	// ���� �� ������ ��������� (������ ����?) ��������� �� ���������
	if (!lbFound && (dwNewKeybLayout == (dwNewKeybLayout & 0xFFFF)))
		dwNewKeybLayout |= (dwNewKeybLayout << 16);

	// ����� ��� ������ � �������?
	if (dwNewKeybLayout != GetActiveKeyboardLayout())
	{
#ifdef _DEBUG
		_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"CConEmuMain::SwitchKeyboardLayout change to(0x%08I64X)\n", (unsigned __int64)dwNewKeybLayout);
		DEBUGSTRLANG(szDbg);
#endif

		if (gpSetCls->isAdvLogging > 1)
		{
			wchar_t szInfo[255];
			_wsprintf(szInfo, SKIPLEN(countof(szInfo)) L"CConEmuMain::SwitchKeyboardLayout, posting WM_INPUTLANGCHANGEREQUEST, WM_INPUTLANGCHANGE for 0x%08X",
			          (DWORD)dwNewKeybLayout);
			LogString(szInfo);
		}

		// ������ ����������� ���������
		PostMessage(ghWnd, WM_INPUTLANGCHANGEREQUEST, 0, (LPARAM)dwNewKeybLayout);
		PostMessage(ghWnd, WM_INPUTLANGCHANGE, 0, (LPARAM)dwNewKeybLayout);
	}
	else
	{
		if (gpSetCls->isAdvLogging > 1)
		{
			wchar_t szInfo[255];
			_wsprintf(szInfo, SKIPLEN(countof(szInfo)) L"CConEmuMain::SwitchKeyboardLayout skipped, cause of GetActiveKeyboardLayout()==0x%08X",
			          (DWORD)dwNewKeybLayout);
			LogString(szInfo);
		}
	}
}





/* ****************************************************** */
/*                                                        */
/*                        EVENTS                          */
/*                                                        */
/* ****************************************************** */

#ifdef colorer_func
{
	EVENTS() {};
}
#endif

// �������� �� ������� ������� Scrolling(BufferHeight) & Alternative
void CConEmuMain::OnBufferHeight() //BOOL abBufferHeight)
{
	if (!isMainThread())
	{
		PostMessage(ghWnd, mn_MsgPostOnBufferHeight, 0, 0);
		return;
	}

	CVirtualConsole* pVCon = NULL;
	CVConGuard VCon;
	if (GetActiveVCon(&VCon) >= 0)
		pVCon = VCon.VCon();


	BOOL lbBufferHeight = FALSE;
	BOOL lbAlternative = FALSE;

	if (pVCon)
	{
		mp_Status->OnConsoleBufferChanged(pVCon->RCon());

		lbBufferHeight = pVCon->RCon()->isBufferHeight();
		lbAlternative = pVCon->RCon()->isAlternative();
	}

	TrackMouse(); // �������� ��� �������� ���������, ���� ��� ��� �����

	mp_TabBar->OnBufferHeight(lbBufferHeight);
	mp_TabBar->OnAlternative(lbAlternative);
}


//bool CConEmuMain::DoClose()
//{
//	bool lbProceed = CVConGroup::OnScClose();
//
//	return lbProceed;
//}

bool CConEmuMain::isCloseConfirmed()
{
	return gpSet->isCloseConsoleConfirm ? mb_CloseGuiConfirmed : true;
}

HMENU CConEmuMain::CreateDebugMenuPopup()
{
	HMENU hDebug = CreatePopupMenu();
	AppendMenu(hDebug, MF_STRING | MF_ENABLED, ID_CON_TOGGLE_VISIBLE, MenuAccel(vkCtrlWinAltSpace,L"&Real console"));
	AppendMenu(hDebug, MF_STRING | MF_ENABLED, ID_CONPROP, _T("&Properties..."));
	AppendMenu(hDebug, MF_STRING | MF_ENABLED, ID_SCREENSHOT, MenuAccel(vkScreenshot,L"Make &screenshot..."));
	AppendMenu(hDebug, MF_STRING | MF_ENABLED, ID_DUMPCONSOLE, _T("&Dump screen..."));
	AppendMenu(hDebug, MF_STRING | MF_ENABLED, ID_LOADDUMPCONSOLE, _T("&Load screen dump..."));
	AppendMenu(hDebug, MF_STRING | MF_ENABLED, ID_DEBUGGUI, _T("Debug &log (GUI)"));
//#ifdef _DEBUG
//	AppendMenu(hDebug, MF_STRING | MF_ENABLED, ID_MONITOR_SHELLACTIVITY, _T("Enable &shell log..."));
//#endif
	AppendMenu(hDebug, MF_STRING | MF_ENABLED, ID_DEBUG_SHOWRECTS, _T("Show debug rec&ts"));
	AppendMenu(hDebug, MF_SEPARATOR, 0, NULL);
	AppendMenu(hDebug, MF_STRING | MF_ENABLED, ID_DEBUGCON, _T("Debug &active process"));
	AppendMenu(hDebug, MF_STRING | MF_ENABLED, ID_MINIDUMP, _T("Active process &memory dump..."));
	return hDebug;
}

HMENU CConEmuMain::CreateVConListPopupMenu(HMENU ahExist, BOOL abFirstTabOnly)
{
	HMENU h = ahExist ? ahExist : CreatePopupMenu();
	wchar_t szText[128];
	const int nMaxStrLen = 32;

	BOOL lbActiveVCon = FALSE;
	int nActiveCmd = -1; // DWORD MAKELONG(WORD wLow,WORD wHigh);
	CVirtualConsole* pVCon = NULL;
	DWORD nAddFlags = 0;
	
	if (ahExist)
	{
		while (DeleteMenu(ahExist, 0, MF_BYPOSITION))
			;
	}
	
	for (int V = 0; (pVCon = GetVCon(V, true))!=NULL; V++)
	{
		if ((lbActiveVCon = isActive(pVCon)))
			nActiveCmd = MAKELONG(1, V+1);
		nAddFlags = 0; //(lbActiveVCon ? MF_DEFAULT : 0);
		CRealConsole* pRCon = pVCon->RCon();
		if (!pRCon)
		{
			wsprintf(szText, L"%i: VConsole", V+1);
			AppendMenu(h, MF_STRING|nAddFlags, MAKELONG(1, V+1), szText);
		}
		else
		{
			ConEmuTab tab = {};
			int R = 0;
			if (!pRCon->GetTab(R, &tab))
			{
				wsprintf(szText, L"%i: RConsole", V+1);
				AppendMenu(h, MF_STRING|nAddFlags, MAKELONG(1, V+1), szText);
			}
			else
			{
				do
				{
					nAddFlags = 0/*((lbActiveVCon && (R==0)) ? MF_DEFAULT : 0)*/
						| ((lbActiveVCon && (abFirstTabOnly || pRCon->GetActiveTab() == R)) ? MF_CHECKED : MF_UNCHECKED)
						#if 0
						| ((tab->Flags() & etfDisabled) ? (MF_DISABLED|MF_GRAYED) : 0)
						#endif
						;
					int nLen = lstrlen(tab.Name/*.Ptr()*/);
					if (!R)
						wsprintf(szText, L"%i: ", V+1);
					else
						wcscpy_c(szText, L"      ");
					if (nLen <= nMaxStrLen)
					{
						wcscat_c(szText, tab.Name/*.Ptr()*/);
					}
					else
					{
						int nCurLen = lstrlen(szText);
						_ASSERTE((nCurLen+10)<nMaxStrLen);
						if ((tab.Type & fwt_TypeMask) == fwt_Panels)
						{
							lstrcpyn(szText+nCurLen, tab.Name/*.Ptr()*/, nMaxStrLen-1-nCurLen);
						}
						else
						{
							szText[nCurLen++] = L'\x2026'; szText[nCurLen] = 0;
							lstrcpyn(szText+nCurLen, tab.Name+nLen-nMaxStrLen, nMaxStrLen-1-nCurLen);
						}
						wcscat_c(szText, L"\x2026"); //...
					}
					AppendMenu(h, MF_STRING|nAddFlags, MAKELONG(R+1, V+1), szText);
				} while (!abFirstTabOnly && pRCon->GetTab(++R, &tab));
			}
		}
	}

	if (nActiveCmd != -1 && !abFirstTabOnly)
	{
		MENUITEMINFO mi = {sizeof(mi), MIIM_STATE|MIIM_ID};
		mi.wID = nActiveCmd;
		GetMenuItemInfo(h, nActiveCmd, FALSE, &mi);
		mi.fState |= MF_DEFAULT;
		SetMenuItemInfo(h, nActiveCmd, FALSE, &mi);
	}
	
	return h;
}

HMENU CConEmuMain::CreateVConPopupMenu(CVirtualConsole* apVCon, HMENU ahExist, BOOL abAddNew, HMENU& hTerminate)
{
	//BOOL lbEnabled = TRUE;
	HMENU hMenu = ahExist;
	
	CVConGuard VCon;
	if (!apVCon && (CVConGroup::GetActiveVCon(&VCon) >= 0))
		apVCon = VCon.VCon();

	if (!hTerminate)
		hTerminate = CreatePopupMenu();
	
	if (!hMenu)
	{	
		hMenu = CreatePopupMenu();
		
		/*
        MENUITEM "&Close",                      IDM_CLOSE
        MENUITEM "Detach",                      IDM_DETACH
        MENUITEM "&Terminate",                  IDM_TERMINATE
        MENUITEM SEPARATOR
        MENUITEM "&Restart",                    IDM_RESTART
        MENUITEM "Restart as...",               IDM_RESTARTAS
        MENUITEM SEPARATOR
        MENUITEM "New console...",              IDM_NEW
        MENUITEM SEPARATOR
        MENUITEM "&Save",                       IDM_SAVE
        MENUITEM "Save &all",                   IDM_SAVEALL
		*/
		
		AppendMenu(hMenu, MF_STRING | MF_ENABLED,     IDM_CLOSE,     MenuAccel(vkCloseTab,L"&Close tab"));
		AppendMenu(hMenu, MF_STRING | MF_ENABLED,     IDM_DETACH,    L"Detach");
		AppendMenu(hMenu, MF_STRING | MF_ENABLED,     IDM_DUPLICATE, MenuAccel(vkDuplicateRoot,L"Duplica&te root..."));
		//AppendMenu(hMenu, MF_STRING | MF_ENABLED,     IDM_ADMIN_DUPLICATE, MenuAccel(vkDuplicateRootAs,L"Duplica&te as Admin..."));
		AppendMenu(hMenu, MF_STRING | MF_ENABLED,     IDM_RENAMETAB, MenuAccel(vkRenameTab,L"Rena&me tab"));
		AppendMenu(hTerminate, MF_STRING | MF_ENABLED, IDM_TERMINATECON, MenuAccel(vkMultiClose,L"&Console"));
		AppendMenu(hTerminate, MF_SEPARATOR, 0, L"");
		AppendMenu(hTerminate, MF_STRING | MF_ENABLED, IDM_TERMINATEPRC, MenuAccel(vkTerminateApp,L"&Active process"));
		AppendMenu(hMenu, MF_POPUP | MF_ENABLED, (UINT_PTR)hTerminate, L"&Terminate");
		AppendMenu(hMenu, MF_SEPARATOR, 0, L"");
		AppendMenu(hMenu, MF_STRING | MF_ENABLED,     IDM_RESTART,   MenuAccel(vkMultiRecreate,L"&Restart"));
		AppendMenu(hMenu, MF_STRING | MF_ENABLED,     IDM_RESTARTAS, L"Restart as...");
		if (abAddNew)
		{
			AppendMenu(hMenu, MF_SEPARATOR, 0, L"");
			AppendMenu(hMenu, MF_STRING | MF_ENABLED, ID_NEWCONSOLE, MenuAccel(vkMultiNew,L"New console..."));
			AppendMenu(hMenu, MF_STRING | MF_ENABLED, IDM_ATTACHTO,  MenuAccel(vkMultiNewAttach,L"Attach to..."));
		}
		AppendMenu(hMenu, MF_SEPARATOR, 0, L"");
		AppendMenu(hMenu, MF_STRING | MF_ENABLED,     IDM_SAVE,      L"&Save");
		AppendMenu(hMenu, MF_STRING | MF_ENABLED,     IDM_SAVEALL,   L"Save &all");
	}

	if (apVCon)
	{
		bool lbIsFar = apVCon->RCon()->isFar(TRUE/* abPluginRequired */)!=FALSE;
		#ifdef _DEBUG
		bool lbIsPanels = lbIsFar && apVCon->RCon()->isFilePanel(false/* abPluginAllowed */)!=FALSE;
		#endif
		bool lbIsEditorModified = lbIsFar && apVCon->RCon()->isEditorModified()!=FALSE;
		bool lbHaveModified = lbIsFar && apVCon->RCon()->GetModifiedEditors()!=0;
		bool lbCanCloseTab = apVCon->RCon()->CanCloseTab();

		if (lbHaveModified)
		{
			if (!gpSet->sSaveAllMacro || !*gpSet->sSaveAllMacro)
				lbHaveModified = false;
		}

		EnableMenuItem(hMenu, IDM_CLOSE, MF_BYCOMMAND | (lbCanCloseTab ? MF_ENABLED : MF_GRAYED));
		EnableMenuItem(hMenu, IDM_DETACH, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hMenu, IDM_DUPLICATE, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hMenu, IDM_RENAMETAB, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hTerminate, IDM_TERMINATECON, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hTerminate, IDM_TERMINATEPRC, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hMenu, IDM_RESTART, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hMenu, IDM_RESTARTAS, MF_BYCOMMAND | MF_ENABLED);
		//EnableMenuItem(hMenu, IDM_ADMIN_DUPLICATE, MF_BYCOMMAND | (lbIsPanels ? MF_ENABLED : MF_GRAYED));
		EnableMenuItem(hMenu, IDM_SAVE, MF_BYCOMMAND | (lbIsEditorModified ? MF_ENABLED : MF_GRAYED));
		EnableMenuItem(hMenu, IDM_SAVEALL, MF_BYCOMMAND | (lbHaveModified ? MF_ENABLED : MF_GRAYED));
	}
	else
	{
		EnableMenuItem(hMenu, IDM_CLOSE, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, IDM_DETACH, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, IDM_DUPLICATE, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, IDM_RENAMETAB, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hTerminate, IDM_TERMINATECON, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hTerminate, IDM_TERMINATEPRC, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, IDM_RESTART, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, IDM_RESTARTAS, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, IDM_SAVE, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, IDM_SAVEALL, MF_BYCOMMAND | MF_GRAYED);
	}

	return hMenu;
}

//void CConEmuMain::PopulateEditMenuPopup(HMENU hMenu)
HMENU CConEmuMain::CreateEditMenuPopup(CVirtualConsole* apVCon, HMENU ahExist /*= NULL*/)
{
	CVConGuard VCon;
	if (!apVCon && (CVConGroup::GetActiveVCon(&VCon) >= 0))
		apVCon = VCon.VCon();
		
	BOOL lbEnabled = TRUE;
	BOOL lbSelectionExist = FALSE;
	if (apVCon && apVCon->RCon())
	{
		if (apVCon->RCon()->GuiWnd() && !apVCon->RCon()->isBufferHeight())
			lbEnabled = FALSE; // ���� ������ �������� ����������� ���� - ��������� ������ �� �����
		// ����� �� ������ ����� "Copy"
		lbSelectionExist = lbEnabled && apVCon->RCon()->isSelectionPresent();
	}

	HMENU hMenu = ahExist;
	
	if (!hMenu)
	{	
		hMenu = CreatePopupMenu();
		AppendMenu(hMenu, MF_STRING | (lbEnabled?MF_ENABLED:MF_GRAYED), ID_CON_MARKBLOCK, MenuAccel(vkCTSVkBlockStart,L"Mark &block"));
		AppendMenu(hMenu, MF_STRING | (lbEnabled?MF_ENABLED:MF_GRAYED), ID_CON_MARKTEXT, MenuAccel(vkCTSVkTextStart,L"Mar&k text"));
		AppendMenu(hMenu, MF_STRING | (lbSelectionExist?MF_ENABLED:MF_GRAYED), ID_CON_COPY, _T("Cop&y"));
		AppendMenu(hMenu, MF_STRING | (lbEnabled?MF_ENABLED:MF_GRAYED), ID_CON_PASTE, _T("&Paste"));
		AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
		AppendMenu(hMenu, MF_STRING | (lbEnabled?MF_ENABLED:MF_GRAYED), ID_CON_FIND, MenuAccel(vkFindTextDlg,L"&Find text..."));
	}
	else
	{
		EnableMenuItem(hMenu, ID_CON_MARKBLOCK, MF_BYCOMMAND | (lbEnabled?MF_ENABLED:MF_GRAYED));
		EnableMenuItem(hMenu, ID_CON_MARKTEXT, MF_BYCOMMAND | (lbEnabled?MF_ENABLED:MF_GRAYED));
		EnableMenuItem(hMenu, ID_CON_COPY, MF_BYCOMMAND | (lbSelectionExist?MF_ENABLED:MF_GRAYED));
		EnableMenuItem(hMenu, ID_CON_PASTE, MF_BYCOMMAND | (lbEnabled?MF_ENABLED:MF_GRAYED));
		EnableMenuItem(hMenu, ID_CON_FIND, MF_BYCOMMAND | (lbEnabled?MF_ENABLED:MF_GRAYED));
	}
	
	return hMenu;
}

HMENU CConEmuMain::CreateHelpMenuPopup()
{
	HMENU hHelp = CreatePopupMenu();

	if (!gpConEmu->mb_MingwMode)
	{
		if (gpUpd && gpUpd->InUpdate())
			AppendMenu(hHelp, MF_STRING | MF_ENABLED, ID_STOPUPDATE, _T("&Stop updates checking"));
		else
			AppendMenu(hHelp, MF_STRING | MF_ENABLED, ID_CHECKUPDATE, _T("&Check for updates"));
	}
	
	AppendMenu(hHelp, MF_STRING | MF_ENABLED, ID_HOMEPAGE, _T("&Visit home page"));
	AppendMenu(hHelp, MF_STRING | MF_ENABLED, ID_REPORTBUG, _T("&Report a bug..."));
	
	if (ms_ConEmuChm[0])  //���������� ����� ������ ���� ���� conemu.chm
		AppendMenu(hHelp, MF_STRING | MF_ENABLED, ID_HELP, _T("&Help"));

	AppendMenu(hHelp, MF_SEPARATOR, 0, NULL);	
	AppendMenu(hHelp, MF_STRING | MF_ENABLED, ID_HOTKEYS, MenuAccel(vkWinAltK,L"Hot&keys"));
	AppendMenu(hHelp, MF_STRING | MF_ENABLED, ID_ABOUT, MenuAccel(vkWinAltA,L"&About / Help"));
	
	return hHelp;
}

LRESULT CConEmuMain::OnCreate(HWND hWnd, LPCREATESTRUCT lpCreate)
{
	_ASSERTE(ghWnd == hWnd); // ��� ������ ���� ���� ���������� (CConEmuMain::MainWndProc)
	ghWnd = hWnd; // ������ �����, ����� ������� ����� ������������

	OnTaskbarButtonCreated();

	//DWORD_PTR dwStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
	//if (gpSet->isHideCaptionAlways) {
	//	if ((dwStyle & (WS_CAPTION|WS_THICKFRAME)) != 0) {
	//		lpCreate->style &= ~(WS_CAPTION|WS_THICKFRAME);
	//		dwStyle = lpCreate->style;
	//		SetWindowStyle(dwStyle);
	//	}
	//}

	// lpCreate->cx/cy ����� ��������� CW_USEDEFAULT
	//RECT rcWnd = CalcRect(CER_MAIN);
	//GetWindowRect(ghWnd, &rcWnd);

	// It's better to store current mrc_Ideal values now, after real window creation
	StoreIdealRect();
	//UpdateIdealRect();

	// Used for restoring from Maximized/Fullscreen/Iconic. Remember current Pos/Size.
	StoreNormalRect(NULL);

	// Win ������? ������ ������� ��� WS_CAPTION, � ��������� � WS_CAPTION
	DWORD dwStyle = GetWindowLong(ghWnd, GWL_STYLE);
	DWORD dwNewStyle = FixWindowStyle(dwStyle, WindowMode);
	DEBUGTEST(WINDOWPLACEMENT wpl1 = {sizeof(wpl1)}; GetWindowPlacement(ghWnd, &wpl1););
	if (dwStyle != dwNewStyle)
	{
		//TODO: ���������, ����� � ���� ������ ������ �� �������� ����-�� ������
		SetWindowStyle(dwNewStyle);
	}
	DEBUGTEST(WINDOWPLACEMENT wpl2 = {sizeof(wpl2)}; GetWindowPlacement(ghWnd, &wpl2););


	Icon.LoadIcon(hWnd, gpSet->nIconID/*IDI_ICON1*/);
	// ��������� ����������� �� ������� FlashWindow �� ���� � ������ ����������
	HMODULE hUser32 = GetModuleHandle(L"user32.dll");
	FRegisterShellHookWindow fnRegisterShellHookWindow = NULL;

	if (hUser32) fnRegisterShellHookWindow = (FRegisterShellHookWindow)GetProcAddress(hUser32, "RegisterShellHookWindow");

	if (fnRegisterShellHookWindow) fnRegisterShellHookWindow(hWnd);

	_ASSERTE(ghConWnd==NULL && "ConWnd must not be created yet"); // ��� ��� �� ������ ���� �������
	// ����� ����� ���� ����� ����� ���� �� ������ �������
	OnActiveConWndStore(NULL); // 31.03.2009 Maximus - ������ ������� ��� ��� �� �������!
	//m_Back->Create();

	//if (!m_Child->Create())
	//	return -1;

	//mn_StartTick = GetTickCount();
	//if (gpSet->isGUIpb && !ProgressBars) {
	//	ProgressBars = new CProgressBars(ghWnd, g_hInstance);
	//}
	//// ���������� ���������� ����� � ������������ ����
	//SetConEmuEnvVar('ghWnd DC');

	// ������������ ��� �������� ��������� ����
	GetSysMenu(TRUE);

	// Holder
	_ASSERTE(ghWndWork==NULL); // ��� �� ������ ��� ���� ������
	if (!ghWndWork && !CreateWorkWindow())
	{
		// ������ ��� ��������
		return -1;
	}

	//HMENU hwndMain = GetSysMenu(ghWnd, FALSE), hDebug = NULL;
	//InsertMenu(hwndMain, 0, MF_BYPOSITION | MF_STRING | MF_ENABLED, ID_TOMONITOR, _T("Bring &here"));
	//InsertMenu(hwndMain, 0, MF_BYPOSITION | MF_STRING | MF_ENABLED, ID_TOTRAY, TRAY_ITEM_HIDE_NAME/* L"Hide to &TSA" */);
	//InsertMenu(hwndMain, 0, MF_BYPOSITION, MF_SEPARATOR, 0);
	//InsertMenu(hwndMain, 0, MF_BYPOSITION | MF_STRING | MF_ENABLED, ID_ABOUT, _T("&About / Help"));

	//if (ms_ConEmuChm[0])  //���������� ����� ������ ���� ���� conemu.chm
	//	InsertMenu(hwndMain, 0, MF_BYPOSITION | MF_STRING | MF_ENABLED, ID_HELP, _T("&Help"));

	//InsertMenu(hwndMain, 0, MF_BYPOSITION, MF_SEPARATOR, 0);
	//hDebug = CreateDebugMenuPopup();
	//InsertMenu(hwndMain, 0, MF_BYPOSITION | MF_POPUP | MF_ENABLED, (UINT_PTR)hDebug, _T("&Debug"));
	//PopulateEditMenuPopup(hwndMain);
	//InsertMenu(hwndMain, 0, MF_BYPOSITION, MF_SEPARATOR, 0);
	//InsertMenu(hwndMain, 0, MF_BYPOSITION | MF_STRING | MF_ENABLED | (gpSet->isAlwaysOnTop ? MF_CHECKED : 0),
	//           ID_ALWAYSONTOP, _T("Al&ways on top"));
	//InsertMenu(hwndMain, 0, MF_BYPOSITION | MF_STRING | MF_ENABLED | (gpSetCls->AutoScroll ? MF_CHECKED : 0),
	//           ID_AUTOSCROLL, _T("Auto scro&ll"));
	//InsertMenu(hwndMain, 0, MF_BYPOSITION | MF_STRING | MF_ENABLED, ID_SETTINGS, _T("S&ettings..."));
	//InsertMenu(hwndMain, 0, MF_BYPOSITION | MF_STRING | MF_ENABLED, ID_NEWCONSOLE, _T("&New console..."));

#ifdef _DEBUG
	dwStyle = GetWindowLong(ghWnd, GWL_STYLE);
#endif

	_ASSERTE(InCreateWindow() == true);

	if (gpSet->isTabs==1)  // "���� ������"
	{
		ForceShowTabs(TRUE); // �������� ����

		// ������ ������ ������� ��� �������� "��������"
		if ((WindowMode == wmNormal) && !m_InsideIntegration)
		{
			RECT rcWnd = GetDefaultRect();
			SetWindowPos(ghWnd, NULL, rcWnd.left, rcWnd.top, rcWnd.right-rcWnd.left, rcWnd.bottom-rcWnd.top, SWP_NOZORDER);
			UpdateIdealRect(rcWnd);
		}
	}

	// ��������� ������� ����������
	gpConEmu->ReSize(TRUE);

	//CreateCon();
	// ��������� ��������� ����
	// 120122 - ������ ����� PipeServer
	if (!m_GuiServer.Start())
	{
		// ������ ��� ��������
		return -1;
	}

	return 0;
}

wchar_t* CConEmuMain::LoadConsoleBatch(LPCWSTR asSource, wchar_t** ppszStartupDir /*= NULL*/)
{
	wchar_t* pszDataW = NULL;

	if (*asSource == CmdFilePrefix)
	{
		// � �������� "�������" ������ "�������� ����" �������������� ������� ���������� ��������
		pszDataW = LoadConsoleBatch_File(asSource);
	}
	else if ((*asSource == TaskBracketLeft) || (lstrcmp(asSource, AutoStartTaskName) == 0))
	{
		// ��� ������
		pszDataW = LoadConsoleBatch_Task(asSource, ppszStartupDir);
	}
	else if (*asSource == DropLnkPrefix)
	{
		// ���� �� ��������, ���� �� ConEmu (��� ��� �����)
		// ����������� (� ����������?) ���� ��� ��������� ������ ������/��������
		pszDataW = LoadConsoleBatch_Drops(asSource);
	}
	else
	{
		_ASSERTE(*asSource==CmdFilePrefix || *asSource==TaskBracketLeft);
	}

	return pszDataW;
}

wchar_t* CConEmuMain::LoadConsoleBatch_File(LPCWSTR asSource)
{
	wchar_t* pszDataW = NULL;

	if (asSource && (*asSource == CmdFilePrefix))
	{
		// � �������� "�������" ������ "�������� ����" �������������� ������� ���������� ��������
		HANDLE hFile = CreateFile(asSource+1, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

		if (!hFile || hFile == INVALID_HANDLE_VALUE)
		{
			DWORD dwErr = GetLastError();
			wchar_t szCurDir[MAX_PATH*2]; szCurDir[0] = 0; GetCurrentDirectory(countof(szCurDir), szCurDir);
			size_t cchMax = _tcslen(asSource)+100+_tcslen(szCurDir);
			wchar_t* pszErrMsg = (wchar_t*)calloc(cchMax,2);
			_wcscpy_c(pszErrMsg, cchMax, L"Can't open console batch file:\n\xAB"/*�*/);
			_wcscat_c(pszErrMsg, cchMax, asSource+1);
			_wcscat_c(pszErrMsg, cchMax, L"\xBB"/*�*/ L"\nCurrent directory:\n\xAB"/*�*/);
			_wcscat_c(pszErrMsg, cchMax, szCurDir);
			_wcscat_c(pszErrMsg, cchMax, L"\xBB"/*�*/);
			DisplayLastError(pszErrMsg, dwErr);
			free(pszErrMsg);
			//Destroy(); -- must caller
			return NULL;
		}

		DWORD nSize = GetFileSize(hFile, NULL);

		if (!nSize || nSize > (1<<20))
		{
			DWORD dwErr = GetLastError();
			CloseHandle(hFile);
			wchar_t* pszErrMsg = (wchar_t*)calloc(_tcslen(asSource)+100,2);
			lstrcpy(pszErrMsg, L"Console batch file is too large or empty:\n\xAB"/*�*/); lstrcat(pszErrMsg, asSource+1); lstrcat(pszErrMsg, L"\xBB"/*�*/);
			DisplayLastError(pszErrMsg, dwErr);
			free(pszErrMsg);
			//Destroy(); -- must caller
			return NULL;
		}

		char* pszDataA = (char*)calloc(nSize+4,1); //-V112
		_ASSERTE(pszDataA);
		DWORD nRead = 0;
		BOOL lbRead = ReadFile(hFile, pszDataA, nSize, &nRead, 0);
		DWORD dwErr = GetLastError();
		CloseHandle(hFile);

		if (!lbRead || nRead != nSize)
		{
			free(pszDataA);
			wchar_t* pszErrMsg = (wchar_t*)calloc(_tcslen(asSource)+100,2);
			lstrcpy(pszErrMsg, L"Reading console batch file failed:\n\xAB"/*�*/); lstrcat(pszErrMsg, asSource+1); lstrcat(pszErrMsg, L"\xBB"/*�*/);
			DisplayLastError(pszErrMsg, dwErr);
			free(pszErrMsg);
			//Destroy(); -- must caller
			return NULL;
		}

		// ��������� ���.�������� �����
		if (pszDataA[0] == '\xEF' && pszDataA[1] == '\xBB' && pszDataA[2] == '\xBF')
		{
			// UTF-8 BOM
			pszDataW = (wchar_t*)calloc(nSize+2,2);
			_ASSERTE(pszDataW);
			MultiByteToWideChar(CP_UTF8, 0, pszDataA+3, -1, pszDataW, nSize);
		}
		else if (pszDataA[0] == '\xFF' && pszDataA[1] == '\xFE')
		{
			// CP-1200 BOM
			pszDataW = lstrdup((wchar_t*)(pszDataA+2));
			_ASSERTE(pszDataW);
		}
		else
		{
			// Plain ANSI
			pszDataW = (wchar_t*)calloc(nSize+2,2);
			_ASSERTE(pszDataW);
			MultiByteToWideChar(CP_ACP, 0, pszDataA, -1, pszDataW, nSize+1);
		}

	}
	else
	{
		_ASSERTE(FALSE && "Invalid CmdFilePrefix");
	}

	return pszDataW;
}

wchar_t* CConEmuMain::LoadConsoleBatch_Drops(LPCWSTR asSource)
{
	wchar_t* pszDataW = NULL;

	if (asSource && (*asSource == DropLnkPrefix))
	{
		// ���� �� ��������, ���� �� ConEmu (��� ��� �����)
		// ����������� (� ����������?) ���� ��� ��������� ������ ������/��������
		asSource++;
		if (!*asSource)
		{
			DisplayLastError(L"Empty command", (DWORD)-1);
			return NULL;
		}

		// �������, ��� ���� ���� (*.exe, *.cmd, ...) ��� ����� (*.lnk)
		// ��� ���� ����������� ������� � ConEmu.
		wchar_t szPart[MAX_PATH+1], szExe[MAX_PATH+1], szArguments[32768], szDir[MAX_PATH+1];
		HRESULT hr = S_OK;
		IShellLinkW* pShellLink = NULL;
		IPersistFile* pFile = NULL;
		if (StrStrI(asSource, L".lnk"))
		{
			hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (void**)&pShellLink);
			if (FAILED(hr) || !pShellLink)
			{
				DisplayLastError(L"Can't create IID_IShellLinkW", (DWORD)hr);
				return NULL;
			}
			hr = pShellLink->QueryInterface(IID_IPersistFile, (void**)&pFile);
			if (FAILED(hr) || !pFile)
			{
				DisplayLastError(L"Can't create IID_IPersistFile", (DWORD)hr);
				pShellLink->Release();
				return NULL;
			}
		}
		
		// �������
		LPWSTR pszConsoles[MAX_CONSOLE_COUNT] = {};
		size_t cchLen, cchAllLen = 0, iCount = 0;
		while ((iCount < MAX_CONSOLE_COUNT) && (0 == NextArg(&asSource, szPart)))
		{
			if (lstrcmpi(PointToExt(szPart), L".lnk") == 0)
			{
				// �����
				hr = pFile->Load(szPart, STGM_READ);
				if (SUCCEEDED(hr))
				{
					hr = pShellLink->GetPath(szExe, countof(szExe), NULL, 0);
					if (SUCCEEDED(hr) && *szExe)
					{
						hr = pShellLink->GetArguments(szArguments, countof(szArguments));
						if (FAILED(hr))
							szArguments[0] = 0;
						hr = pShellLink->GetWorkingDirectory(szDir, countof(szDir));
						if (FAILED(hr))
							szDir[0] = 0;

						cchLen = _tcslen(szExe)+3
							+ _tcslen(szArguments)+1
							+ (*szDir ? (_tcslen(szDir)+32) : 0); // + "-new_console:d<Dir>
						pszConsoles[iCount] = (wchar_t*)malloc(cchLen*sizeof(wchar_t));
						_wsprintf(pszConsoles[iCount], SKIPLEN(cchLen) L"\"%s\"%s%s",
							Unquote(szExe), *szArguments ? L" " : L"", szArguments);
						if (*szDir)
						{
							_wcscat_c(pszConsoles[iCount], cchLen, L" \"-new_console:d");
							_wcscat_c(pszConsoles[iCount], cchLen, Unquote(szDir));
							_wcscat_c(pszConsoles[iCount], cchLen, L"\"");
						}
						iCount++;

						cchAllLen += cchLen+3;
					}
				}
			}
			else
			{
				cchLen = _tcslen(szPart) + 3;
				pszConsoles[iCount] = (wchar_t*)malloc(cchLen*sizeof(wchar_t));
				_wsprintf(pszConsoles[iCount], SKIPLEN(cchLen) L"\"%s\"", szPart);
				iCount++;

				cchAllLen += cchLen+3;
			}
		}

		if (pShellLink)
			pShellLink->Release();
		if (pFile)
			pFile->Release();

		if (iCount == 0)
			return NULL;

		if (iCount == 1)
			return pszConsoles[0];

		// ������ - ������� pszDataW
		pszDataW = (wchar_t*)malloc(cchAllLen*sizeof(*pszDataW));
		*pszDataW = 0;
		for (size_t i = 0; i < iCount; i++)
		{
			_wcscat_c(pszDataW, cchAllLen, pszConsoles[i]);
			_wcscat_c(pszDataW, cchAllLen, L"\r\n");
			free(pszConsoles[i]);
		}
	}
	else
	{
		_ASSERTE(FALSE && "Invalid DropLnkPrefix");
	}

	return pszDataW;
}

wchar_t* CConEmuMain::LoadConsoleBatch_Task(LPCWSTR asSource, wchar_t** ppszStartupDir /*= NULL*/)
{
	wchar_t* pszDataW = NULL;

	if (asSource && ((*asSource == TaskBracketLeft) || (lstrcmp(asSource, AutoStartTaskName) == 0)))
	{
		wchar_t szName[MAX_PATH]; lstrcpyn(szName, asSource, countof(szName));
		wchar_t* psz = wcschr(szName, TaskBracketRight);
		if (psz) psz[1] = 0;

		const Settings::CommandTasks* pGrp;
		for (int i = 0; (pGrp = gpSet->CmdTaskGet(i)) != NULL; i++)
		{
			if (lstrcmpi(pGrp->pszName, szName) == 0)
			{
				pszDataW = lstrdup(pGrp->pszCommands);
				if (ppszStartupDir && !*ppszStartupDir && pGrp->pszGuiArgs)
				{
					pGrp->ParseGuiArgs(ppszStartupDir, NULL);
				}
				break;
			}
		}

		if (!pszDataW || !*pszDataW)
		{
			size_t cchMax = _tcslen(szName)+100;
			wchar_t* pszErrMsg = (wchar_t*)calloc(cchMax,sizeof(*pszErrMsg));
			_wsprintf(pszErrMsg, SKIPLEN(cchMax) L"Command group %s %s!\nStart default shell?",
				szName, pszDataW ? L"is empty" : L"not found");

			int nBtn = MessageBox(pszErrMsg, MB_YESNO|MB_ICONEXCLAMATION);

			SafeFree(pszErrMsg);
			SafeFree(pszDataW);

			if (nBtn != IDNO)
			{
				LPCWSTR pszDefCmd = gpSetCls->GetDefaultCmd();
				return lstrdup(pszDefCmd);
			}
			return NULL;
		}

	}
	else
	{
		_ASSERTE(FALSE && "Invalid task name");
	}

	return pszDataW;
}

void CConEmuMain::PostCreate(BOOL abRecieved/*=FALSE*/)
{
	if (gpSetCls->isAdvLogging)
	{
		LogString(abRecieved ? L"PostCreate.2.begin" : L"PostCreate.1.begin");
	}

	mn_StartupFinished = abRecieved ? ss_PostCreate2Called : ss_PostCreate1Called;

	// First ShowWindow forced to use nCmdShow. This may be weird...
	SkipOneShowWindow();

	if (!abRecieved)
	{
		//if (gpConEmu->WindowMode == wmFullScreen || gpConEmu->WindowMode == wmMaximized) {
#ifdef MSGLOGGER
		WINDOWPLACEMENT wpl; memset(&wpl, 0, sizeof(wpl)); wpl.length = sizeof(wpl);
		GetWindowPlacement(ghWnd, &wpl);
#endif

		if (gpSet->isHideCaptionAlways())
		{
			OnHideCaption();
		}

		if (gpSet->isDesktopMode)
			OnDesktopMode();

		SetWindowMode((ConEmuWindowMode)gpSet->_WindowMode, FALSE, TRUE);

		mb_InCreateWindow = false;

		PostMessage(ghWnd, mn_MsgPostCreate, 0, 0);

		if (!mh_RightClickingBmp)
		{
			mh_RightClickingBmp = (HBITMAP)LoadImage(g_hInstance, MAKEINTRESOURCE(IDB_RIGHTCLICKING),
			                      IMAGE_BITMAP, 0,0, LR_CREATEDIBSECTION);

			if (mh_RightClickingBmp)
			{
				BITMAP bi = {0};

				if (GetObject(mh_RightClickingBmp, sizeof(bi), &bi)
				        && bi.bmWidth > bi.bmHeight && bi.bmHeight > 0)
				{
					m_RightClickingSize.x = bi.bmWidth; m_RightClickingSize.y = bi.bmHeight;
				}
				else
				{
					m_RightClickingSize.x = 384; m_RightClickingSize.y = 16;
				}

				_ASSERTE(m_RightClickingSize.y>0);
				m_RightClickingFrames = m_RightClickingSize.x / m_RightClickingSize.y;
				// �� ����, ������ ���� �����
				_ASSERTE(m_RightClickingSize.x == m_RightClickingFrames * m_RightClickingSize.y);
			}
		}
	}
	else
	{
		HWND hCurForeground = GetForegroundWindow();

		//-- ���������� � Taskbar_Init();
		//HRESULT hr = S_OK;
		//hr = OleInitialize(NULL);  // ��� �� ����������� �������� Ole ������ �� ����� �����. ������� ��� ��-�� ���� ������ ������������ �����
		//CoInitializeEx(NULL, COINIT_MULTITHREADED);

		// ����� ���� ��� ������ � SkipOneShowWindow, � ����� � ���
		Taskbar_Init();

		RegisterMinRestore(true);

		RegisterHotKeys();

		// ���� ����� ���������, ��� ������ (�� ������) ���������� ���
		// ���� ��������� �� ��������
		if (!gnWndSetHotkeyOk && gnWndSetHotkey)
		{
			if (ghWndApp)
			{
				SendMessage(ghWndApp, WM_SETHOTKEY, 0, 0);
			}
			SendMessage(ghWnd, WM_SETHOTKEY, gnWndSetHotkey, 0);
		}

		//TODO: ��������, ����� �������� ������ ������ �� 5, ����� �� ������ �������������?
		if (gpSet->UpdSet.isUpdateCheckOnStartup && !DisableAutoUpdate)
			CheckUpdates(FALSE); // �� ���������� ��������� "You are using latest available version"

		//SetWindowRgn(ghWnd, CreateWindowRgn(), TRUE);

		if (gpSetCls->szFontError[0])
		{
			MBoxA(gpSetCls->szFontError);
			gpSetCls->szFontError[0] = 0;
		}

		if (!gpSetCls->CheckConsoleFontFast())
		{
			DontEnable de;
			gpSetCls->EditConsoleFont(ghWnd);
		}
		
		// ���� � ����� [HKEY_CURRENT_USER\Console] ����� ����� �������� - �� � Win7 �����
		// �������� �������� ����� :-) 
		// ��������, ���������� ���� ����� "�������" - ����� ����, � ����������� - ��� :-P
		gpSet->CheckConsoleSettings();

		if (gpSet->isShowBgImage)
			gpSetCls->LoadBackgroundFile(gpSet->sBgImage);

		// ������� ����� ��������� �������, ����� �� ����������� SendMessage
		// (mp_DragDrop->Register() ����� ���� ������������ ���������� ���������)
		if (!mp_DragDrop)
		{
			// ���� 'ghWnd DC'. ��������� �� ������� ����, ���� �� ������
			// "�������" �� ��� (� �������������� ���������� �������)
			mp_DragDrop = new CDragDrop();

			if (!mp_DragDrop->Register())
			{
				CDragDrop *p = mp_DragDrop; mp_DragDrop = NULL;
				delete p;
			}
		}

		// ����� ���� � ��������� ������� - ������ ���������� ������ � TSA
		Icon.SettingsChanged();

		// �������� ������������ �������
		gpSet->CheckHotkeyUnique();


		if (!isVConExists(0) || !gpConEmu->mb_StartDetached)  // ������� ��� ����� ���� �������, ���� ������ Attach �� ConEmuC
		{
			// ���� ���� - ����������� ������������ ������
			if (mb_PortableRegExist)
			{
				// ���� ������ ���������, ��� ���� ������ "�� ����������"
				if (!gpConEmu->PreparePortableReg())
				{
					Destroy();
					return;
				}
			}


			BOOL lbCreated = FALSE;
			LPCWSTR pszCmd = gpSet->GetCmd();

			if ((*pszCmd == CmdFilePrefix || *pszCmd == TaskBracketLeft) && !gpConEmu->mb_StartDetached)
			{
				wchar_t* pszStartupDir = NULL;
				// � �������� "�������" ������ "�������� ����" ��� "������ ������" �������������� ������� ���������� ��������
				wchar_t* pszDataW = LoadConsoleBatch(pszCmd, &pszStartupDir);
				if (!pszDataW)
				{
					Destroy();
					return;
				}

				// GO
				if (!CreateConGroup(pszDataW, FALSE, pszStartupDir))
				{
					Destroy();
					return;
				}

				SafeFree(pszDataW);
				SafeFree(pszStartupDir);

				// ���� ConEmu ��� ������� � ������ "/single /cmd xxx" �� ����� ���������
				// �������� - �������� �������, ������� ������ �� "/cmd" - ��������� ���������
				if (gpSetCls->SingleInstanceArg)
				{
					gpSetCls->ResetCmdArg();
				}

				lbCreated = TRUE;
			}

			if (!lbCreated)
			{
				RConStartArgs args;
				args.bDetached = gpConEmu->mb_StartDetached;

				if (!args.bDetached)
					args.pszSpecialCmd = lstrdup(gpSet->GetCmd());

				if (!CreateCon(&args, TRUE))
				{
					DisplayLastError(L"Can't create new virtual console!");
					Destroy();
					return;
				}
			}
		}

		if (gpConEmu->mb_StartDetached)
			gpConEmu->mb_StartDetached = FALSE;  // ��������� ������ �� ������ �������

		//// ����� ���� � ��������� ������� - ������ ���������� ������ � TSA
		//Icon.SettingsChanged();

		//if (!mp_TaskBar2)
		//{
		//	// � PostCreate ��� ����������� ������ �����. �� ���� ������ �� ������,
		//	// �.�. ��������� ���� ��� ��������.
		//	hr = CoCreateInstance(CLSID_TaskbarList,NULL,CLSCTX_INPROC_SERVER,IID_ITaskbarList2,(void**)&mp_TaskBar2);
		//
		//	if (hr == S_OK && mp_TaskBar2)
		//	{
		//		hr = mp_TaskBar2->HrInit();
		//	}
		//
		//	if (hr != S_OK && mp_TaskBar2)
		//	{
		//		if (mp_TaskBar2) mp_TaskBar2->Release();
		//
		//		mp_TaskBar2 = NULL;
		//	}
		//}
		//
		//if (!mp_TaskBar3 && mp_TaskBar2)
		//{
		//	hr = mp_TaskBar2->QueryInterface(IID_ITaskbarList3, (void**)&mp_TaskBar3);
		//}

		//if (!mp_DragDrop)
		//{
		//	// ���� 'ghWnd DC'. ��������� �� ������� ����, ���� �� ������
		//	// "�������" �� ��� (� �������������� ���������� �������)
		//	mp_DragDrop = new CDragDrop();

		//	if (!mp_DragDrop->Register())
		//	{
		//		CDragDrop *p = mp_DragDrop; mp_DragDrop = NULL;
		//		delete p;
		//	}
		//}

		//TODO terst
		WARNING("���� ������� �� ������� - handler �� �����������!")
		//SetConsoleCtrlHandler((PHANDLER_ROUTINE)CConEmuMain::HandlerRoutine, true);
		
		// ���� ����� ��� � ��� - ������� ��� (�� ������ ������, ����� RealConsole ����� �������
		if (m_InsideIntegration)
		{
			if ((hCurForeground == ghWnd) || (hCurForeground != mh_InsideParentRoot))
				SetForegroundWindow(mh_InsideParentRoot);
			SetWindowPos(ghWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		}
		else if ((hCurForeground == ghWnd) && !WindowStartMinimized)
		{
			apiSetForegroundWindow(ghWnd);
		}

		if (WindowStartMinimized)
		{
			if (IsWindowVisible(ghWnd) && !isIconic())
				PostMessage(ghWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
			WindowStartMinimized = false;
		}

		UpdateWin7TaskList(mb_UpdateJumpListOnStartup);
		mb_UpdateJumpListOnStartup = false;

		//RegisterHotKeys();
		//SetParent(GetParent(GetShellWindow()));
		UINT n = SetTimer(ghWnd, TIMER_MAIN_ID, 500/*gpSet->nMainTimerElapse*/, NULL);
		#ifdef _DEBUG
		DWORD dw = GetLastError();
		#endif
		n = SetTimer(ghWnd, TIMER_CONREDRAW_ID, CON_REDRAW_TIMOUT*2, NULL);
		UNREFERENCED_PARAMETER(n);

		if (gpSet->isTaskbarShield && gpConEmu->mb_IsUacAdmin && gpSet->isWindowOnTaskBar())
		{
			// Bug in Win7? Sometimes after startup "As Admin" sheild does not appeears.
			SetTimer(ghWnd, TIMER_ADMSHIELD_ID, TIMER_ADMSHIELD_ELAPSE, NULL);
		}
	}

	mn_StartupFinished = abRecieved ? ss_PostCreate2Finished : ss_PostCreate1Finished;

	if (gpSetCls->isAdvLogging)
	{
		wchar_t szInfo[64];
		_wsprintf(szInfo, SKIPLEN(countof(szInfo)) L"PostCreate.%i.end", abRecieved ? 2 : 1);
		LogWindowPos(szInfo);
	}
}

// ��� �� Post, � Send. ���������� �������� ���� � ������� ����
HWND CConEmuMain::PostCreateView(CConEmuChild* pChild)
{
	HWND hChild = NULL;
	if (GetCurrentThreadId() == mn_MainThreadId)
	{
		hChild = pChild->CreateView();
	}
	else
	{
		hChild = (HWND)SendMessage(ghWnd, mn_MsgCreateViewWindow, 0, (LPARAM)pChild);
	}
	return hChild;
}

LRESULT CConEmuMain::OnDestroy(HWND hWnd)
{
	// ����� ���������, ����� ���� ��������� ��� ������ ������ (InsideIntegration)
	CVConGroup::OnDestroyConEmu();


	if (mp_AttachDlg)
	{
		mp_AttachDlg->Close();
	}
	if (mp_RecreateDlg)
	{
		mp_RecreateDlg->Close();
	}
	
	gpSet->SaveSizePosOnExit();
	// ������ ����������� ����� ResetEvent(mh_ConEmuAliveEvent), ����� � �������
	// ���������� �� �������� ������� � ������������ hotkey
	RegisterMinRestore(false);
	
	FinalizePortableReg();

	if (mb_ConEmuAliveOwned && mh_ConEmuAliveEvent)
	{
		ResetEvent(mh_ConEmuAliveEvent); // ����� ������ ��������� "���������" �����������
		SafeCloseHandle(mh_ConEmuAliveEvent);
		mb_ConEmuAliveOwned = FALSE;
	}

	if (mb_MouseCaptured)
	{
		ReleaseCapture();
		mb_MouseCaptured = FALSE;
	}

	//120122 - ������ ����� PipeServer
	m_GuiServer.Stop(true);


	CVConGroup::DestroyAllVCon();


	#ifndef _WIN64
	if (mh_WinHook)
	{
		UnhookWinEvent(mh_WinHook);
		mh_WinHook = NULL;
	}
	#endif

	//if (mh_PopupHook) {
	//	UnhookWinEvent(mh_PopupHook);
	//	mh_PopupHook = NULL;
	//}

	if (mp_DragDrop)
	{
		delete mp_DragDrop;
		mp_DragDrop = NULL;
	}

	//if (ProgressBars) {
	//    delete ProgressBars;
	//    ProgressBars = NULL;
	//}
	Icon.RemoveTrayIcon(true);

	Taskbar_Release();
	//if (mp_TaskBar3)
	//{
	//	mp_TaskBar3->Release();
	//	mp_TaskBar3 = NULL;
	//}
	//if (mp_TaskBar2)
	//{
	//	mp_TaskBar2->Release();
	//	mp_TaskBar2 = NULL;
	//}

	if (mh_InsideSysMenu)
		DestroyMenu(mh_InsideSysMenu);

	UnRegisterHotKeys(TRUE);
	//if (mh_DwmApi && mh_DwmApi != INVALID_HANDLE_VALUE)
	//{
	//	FreeLibrary(mh_DwmApi); mh_DwmApi = NULL;
	//	DwmIsCompositionEnabled = NULL;
	//}
	KillTimer(ghWnd, TIMER_MAIN_ID);
	KillTimer(ghWnd, TIMER_CONREDRAW_ID);
	KillTimer(ghWnd, TIMER_CAPTION_APPEAR_ID);
	KillTimer(ghWnd, TIMER_CAPTION_DISAPPEAR_ID);
	PostQuitMessage(0);
	return 0;
}

LRESULT CConEmuMain::OnFlashWindow(DWORD nFlags, DWORD nCount, HWND hCon)
{
	CVConGroup::OnFlashWindow(nFlags, nCount, hCon);

	return 0;
}

void CConEmuMain::setFocus()
{
#ifdef _DEBUG
	DWORD nSelfPID = GetCurrentProcessId();
	wchar_t sFore[1024], sFocus[1024]; sFore[0] = sFocus[0] = 0;
	HWND hFocused = GetFocus();
	DWORD nFocusPID = 0; if (hFocused) { GetWindowThreadProcessId(hFocused, &nFocusPID); getWindowInfo(hFocused, sFocus); }
	HWND hFore = GetForegroundWindow();
	DWORD nForePID = 0; if (hFore) { GetWindowThreadProcessId(hFore, &nForePID); getWindowInfo(hFore, sFore); }
	bool bNeedFocus = false;
	if (hFocused != ghWnd)
		bNeedFocus = true;
#endif
	SetFocus(ghWnd);
}

void CConEmuMain::SetSkipOnFocus(bool abSkipOnFocus)
{
	mb_SkipOnFocus = abSkipOnFocus;
}

LRESULT CConEmuMain::OnFocus(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam, LPCWSTR asMsgFrom /*= NULL*/)
{
	// ����� �������� ������ ������� �� CtrlWinAltSpace ��� ������ � GUI �����������
	if (mb_SkipOnFocus)
	{
		return 0;
	}

	bool lbSetFocus = false;
	
	#ifdef _DEBUG
	WCHAR szDbg[128];
	#endif

	LPCWSTR pszMsgName = L"Unknown";
	HWND hNewFocus = NULL;

	if (messg == WM_SETFOCUS)
	{
		lbSetFocus = true;
		#ifdef _DEBUG
		_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"WM_SETFOCUS(From=0x%08X)", (DWORD)wParam);
		DEBUGSTRFOCUS(szDbg);
		#endif
		pszMsgName = L"WM_SETFOCUS";
	}
	else if (messg == WM_ACTIVATE)
	{
		lbSetFocus = (LOWORD(wParam)==WA_ACTIVE) || (LOWORD(wParam)==WA_CLICKACTIVE);

		if (LOWORD(wParam)==WA_INACTIVE)
			hNewFocus = lParam ? (HWND)lParam : GetForegroundWindow();

		if (!lbSetFocus && gpSet->isDesktopMode && mh_ShellWindow)
		{
			if (isPressed(VK_LBUTTON) || isPressed(VK_RBUTTON))
			{
				// ��� ��������� _��������_ ������ - ���������, ��� ������� ����� � ���� �� �����
				POINT ptCur; GetCursorPos(&ptCur);
				HWND hFromPoint = WindowFromPoint(ptCur);

				//if (hFromPoint == mh_ShellWindow) TODO: ���� ����� ������� - ����� �������

				// mh_ShellWindow ����� ��� ����������. ���� �������� ConEmu � �������� �� mh_ShellWindow
				// �� ��������� ����� ���������� ���� ���� � ������ (WorkerW ��� Progman)
				if (hFromPoint)
				{
					bool lbDesktopActive = false;
					wchar_t szClass[128];

					// ����� ������, ��� ��� ����� ���� ������ ����, ������, � ������, ������� �� ��������
					while (hFromPoint)
					{
						if (GetClassName(hFromPoint, szClass, 127))
						{
							if (!wcscmp(szClass, L"WorkerW") || !wcscmp(szClass, L"Progman"))
							{
								DWORD dwPID;
								GetWindowThreadProcessId(hFromPoint, &dwPID);
								lbDesktopActive = (dwPID == mn_ShellWindowPID);
								break;
							}
						}

						// ����� ���� ���-�� �������� ��������?
						hFromPoint = GetParent(hFromPoint);
					}

					if (lbDesktopActive)
						mb_FocusOnDesktop = FALSE;
				}
			}
		}
		else if (lbSetFocus && gpSet->isQuakeStyle && isQuakeMinimized)
		{
			OnMinimizeRestore(sih_ShowMinimize);
		}

		#ifdef _DEBUG
		if (lbSetFocus)
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"WM_ACTIVATE(From=0x%08X)", (DWORD)lParam);
		else
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"WM_ACTIVATE.WA_INACTIVE(To=0x%08X)", (DWORD)lParam);
		DEBUGSTRFOCUS(szDbg);
		#endif

		pszMsgName = L"WM_ACTIVATE";
	}
	else if (messg == WM_ACTIVATEAPP)
	{
		lbSetFocus = (wParam!=0);
		if (!lbSetFocus)
			hNewFocus = GetForegroundWindow();

		#ifdef _DEBUG
		if (lbSetFocus)
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"WM_ACTIVATEAPP.Activate(FromTID=%i)", (DWORD)lParam);
		else
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"WM_ACTIVATEAPP.Deactivate(ToTID=%i)", (DWORD)lParam);
		DEBUGSTRFOCUS(szDbg);
		#endif

		pszMsgName = L"WM_ACTIVATEAPP";
	}
	else if (messg == WM_KILLFOCUS)
	{
		#ifdef _DEBUG
		_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"WM_KILLFOCUS(To=0x%08X)", (DWORD)wParam);
		DEBUGSTRFOCUS(szDbg);
		#endif
		pszMsgName = L"WM_KILLFOCUS";
		hNewFocus = wParam ? (HWND)wParam : GetForegroundWindow();
	}
	else
	{
		_ASSERTE(messg == 0 && asMsgFrom != NULL);
		if (asMsgFrom)
			pszMsgName = asMsgFrom;

		if (gpSet->isDesktopMode)
		{
			CheckFocus(pszMsgName);
		}

		if (gpSet->isDesktopMode || gpConEmu->m_InsideIntegration)
		{
			// � ���� ������� ������������
			return 0;
		}

		hNewFocus = GetForegroundWindow();
		if (hNewFocus == ghWnd)
			lbSetFocus = true;
		else if (hNewFocus)
		{
			DWORD nForePID; GetWindowThreadProcessId(hNewFocus, &nForePID);
			lbSetFocus = (nForePID == GetCurrentProcessId());
		}

		if (!lbSetFocus || mb_LastConEmuFocusState)
		{
			#ifdef _DEBUG
			HWND hFocus = GetFocus();
			wchar_t szInfo[128];
			_wsprintf(szInfo, SKIPLEN(countof(szInfo)) L"Focus event skipped, lbSetFocus=%u, mb_LastConEmuFocusState=%u, ghWnd=x%08X, hFore=x%08X, hFocus=x%08X", lbSetFocus, mb_LastConEmuFocusState, (DWORD)ghWnd, (DWORD)hNewFocus, (DWORD)hFocus);
			DEBUGSTRFOCUS2(szInfo);
			#endif
			// �� ���������
			return 0;
		}

		// ���� �� �������, ���� ��������� �����-�� ������, � ConEmu �� �������
		// ���������� � ���, ��� ��� ���� ���� ������������. ����� ��������� ������.
		_ASSERTE(lbSetFocus);
	}


	CheckFocus(pszMsgName);

	CVirtualConsole* pVCon = NULL;
	CVConGuard VCon;
	if (GetActiveVCon(&VCon) >= 0)
		pVCon = VCon.VCon();
		
	HWND hGuiWnd = pVCon ? pVCon->RCon()->GuiWnd() : NULL; // GUI ����� (�������� ����)

	// ���� ����� "�������" �����-���� �������� ���� � ConEmu (VideoRenderer - 'ActiveMovie Window')
	if (hNewFocus && hNewFocus != ghWnd && hNewFocus != hGuiWnd)
	{
		HWND hParent = hNewFocus;

		while ((hParent = GetParent(hNewFocus)) != NULL)
		{
			if (hParent == hGuiWnd)
				break;
			else if (hParent == ghWnd)
			{
				DWORD dwStyle = GetWindowLong(hNewFocus, GWL_STYLE);

				if ((dwStyle & (WS_POPUP|WS_OVERLAPPEDWINDOW|WS_DLGFRAME)) != 0)
					break; // ��� ������, �� �������

				setFocus();
				hNewFocus = GetFocus();

				#ifdef _DEBUG
				if (hNewFocus != ghWnd)
				{
					_ASSERTE(hNewFocus == ghWnd);
				}
				else
				{
					DEBUGSTRFOCUS(L"Focus was returned to ConEmu");
				}
				#endif

				lbSetFocus = (hNewFocus == ghWnd);
				break;
			}

			hNewFocus = hParent;
		}
	}

	if (!lbSetFocus && !hNewFocus && (isPressed(VK_LBUTTON) || isPressed(VK_RBUTTON)))
	{
		// ���� �� ��������, ��� ��� ����...
		POINT ptCur = {}; GetCursorPos(&ptCur);
		HWND hUnderMouse = WindowFromPoint(ptCur);
		DWORD nPID = 0;
		if (hUnderMouse && GetWindowThreadProcessId(hUnderMouse, &nPID) && (nPID != GetCurrentProcessId()))
		{
			hNewFocus = hUnderMouse;
		}
	}

	// GUI client?
	if (!lbSetFocus && hNewFocus)
	{
		DWORD nNewPID = 0; GetWindowThreadProcessId(hNewFocus, &nNewPID);
		if (nNewPID == GetCurrentProcessId())
		{
			lbSetFocus = true;
		}
		else if (pVCon)
		{
			if (hNewFocus == pVCon->RCon()->ConWnd())
			{
				lbSetFocus = true;
			}
			else
			{
				
				DWORD nRConPID = pVCon->RCon()->GetActivePID();
				if (nNewPID == nRConPID)
					lbSetFocus = true;
			}
		}
	}



	/****************************************************************/
	/******      Done, Processing Activation/Deactivation      ******/
	/****************************************************************/

	UpdateGuiInfoMappingActive(lbSetFocus);

	mb_LastConEmuFocusState = lbSetFocus;
	DEBUGSTRFOCUS(lbSetFocus ? L"mb_LastConEmuFocusState set to TRUE" : L"mb_LastConEmuFocusState set to FALSE");


	// ���� ��� ��������� � ����������� ���� ��������� ������ ��������
	if (gpSet->isTransparentSeparate
		&& (gpSet->nTransparent != gpSet->nTransparentInactive)
		&& gpSet->isTransparentAllowed())
	{
		// �� ����� �������� "������������"
		OnTransparent(true, lbSetFocus);
	}

	//
	if (!lbSetFocus)
	{
		if (pVCon)
			FixSingleModifier(0, pVCon->RCon());
		mp_TabBar->SwitchRollback();
		UnRegisterHotKeys();
	}
	else if (!mb_HotKeyRegistered)
	{
		RegisterHotKeys();
	}

	if (pVCon && pVCon->RCon())
	{
		// ����� ����� ������ CRealConsole::UpdateServerActive
		pVCon->RCon()->OnGuiFocused(lbSetFocus, (messg == WM_ACTIVATEAPP));
	}

	if ((gpSet->isQuakeStyle == 2)
		&& (!lbSetFocus && !isMeForeground(true,true)) && !isIconic()
		&& IsWindowVisible(ghWnd))
	{
		wchar_t szClassName[255]; szClassName[0] = 0;
		bool bIsTaskbar = false;
		HWND h = hNewFocus;
		while (h)
		{
			GetClassName(h, szClassName, countof(szClassName));
			if (lstrcmpi(szClassName, L"Shell_TrayWnd") == 0)
			{
				bIsTaskbar = true;
				break;
			}
			h = GetParent(h);
		}

		// ���� ������������ TaskBar - �� ���������.
		// ����� ���������� ���� ��� ����� �� ������ � TSA.
		if (!bIsTaskbar)
		{
			OnMinimizeRestore(sih_HideTSA);
		}
	}
	
	if (gpSet->isFadeInactive)
	{
		if (pVCon)
		{
			bool bForeground = lbSetFocus || isMeForeground();
			bool bLastFade = (pVCon!=NULL) ? pVCon->mb_LastFadeFlag : false;
			bool bNewFade = (gpSet->isFadeInactive && !bForeground && !isPictureView());

			if (bLastFade != bNewFade)
			{
				if (pVCon) pVCon->mb_LastFadeFlag = bNewFade;

				pVCon->Invalidate();
			}
		}

		mp_Status->UpdateStatusBar(true);
	}

	// ����� ��������� ControlPanel ����������?
	mouse.ReloadWheelScroll();

	if (lbSetFocus && pVCon && gpSet->isTabsOnTaskBar())
	{
		HWND hFore = GetForegroundWindow();
		if (hFore == ghWnd)
		{
			if (!mb_PostTaskbarActivate)
			{
				//mp_ VActive->OnTaskbarFocus();
				mb_PostTaskbarActivate = TRUE;
				PostMessage(ghWnd, mn_MsgPostTaskbarActivate, 0, 0);
			}
		}
	}

	if (gpSet->isSkipFocusEvents)
		return 0;

#ifdef MSGLOGGER
	/*if (messg == WM_ACTIVATE && wParam == WA_INACTIVE) {
	    WCHAR szMsg[128]; _wsprintf(szMsg, countof(szMsg), L"--Deactivating to 0x%08X\n", lParam);
	    DEBUGSTR(szMsg);
	}
	switch (messg) {
	    case WM_SETFOCUS:
	        {
	            DEBUGSTR(L"--Get focus\n");
	            //return 0;
	        } break;
	    case WM_KILLFOCUS:
	        {
	            DEBUGSTR(L"--Loose focus\n");
	        } break;
	}*/
#endif

	if (pVCon /*&& (messg == WM_SETFOCUS || messg == WM_KILLFOCUS)*/)
	{
		pVCon->RCon()->OnFocus(lbSetFocus);
	}

	return 0;
}

HMONITOR CConEmuMain::GetNearestMonitor(MONITORINFO* pmi /*= NULL*/, LPRECT prcWnd /*= NULL*/)
{
	HMONITOR hMon = NULL;
	MONITORINFO mi = {sizeof(mi)};
	BOOL bRc = FALSE;

	if (prcWnd)
	{
		hMon = MonitorFromRect(prcWnd, MONITOR_DEFAULTTONEAREST);
	}
	else if (!ghWnd || (gpSet->isQuakeStyle && isIconic()))
	{
		_ASSERTE(gpConEmu->wndWidth>0 && gpConEmu->wndHeight>0);
		COORD conSize = MakeCoord(gpConEmu->wndWidth,gpConEmu->wndHeight);
		RECT rcEvalWnd = MakeRect(gpConEmu->wndX, gpConEmu->wndY, gpConEmu->wndX + conSize.X * gpSetCls->FontWidth(), gpConEmu->wndY + conSize.Y * gpSetCls->FontHeight());

		hMon = MonitorFromRect(&rcEvalWnd, MONITOR_DEFAULTTONEAREST);
	}
	else if (!isRestoreFromMinimized && !isIconic())
	{
		hMon = MonitorFromWindow(ghWnd, MONITOR_DEFAULTTONEAREST);
	}
	else
	{
		_ASSERTE(!m_InsideIntegration); // �� ����, ��� "Inside" ���������� �� ������

		if (mh_MinFromMonitor && GetMonitorInfo(mh_MinFromMonitor, &mi))
		{
			hMon = mh_MinFromMonitor;
		}
		
		if (!hMon)
		{
			WINDOWPLACEMENT wpl = {sizeof(wpl)};
			bRc = GetWindowPlacement(ghWnd, &wpl);
			UNREFERENCED_PARAMETER(bRc);

			hMon = MonitorFromRect(&wpl.rcNormalPosition, MONITOR_DEFAULTTONEAREST);
		}
	}

	mi.cbSize = sizeof(mi);

	if (!hMon || !GetMonitorInfo(hMon, &mi))
	{
		_ASSERTE(FALSE && "GetMonitorInfo failed");
		// ���� ����� � ���������� - ����� ������ �� Primary
		ZeroStruct(mi);
		SystemParametersInfo(SPI_GETWORKAREA, 0, &mi.rcWork, 0);
		mi.rcMonitor = MakeRect(0,0, GetSystemMetrics(SM_CXFULLSCREEN), GetSystemMetrics(SM_CYFULLSCREEN));
	}

	if (pmi)
	{
		*pmi = mi;
	}
	return hMon;
}

HMONITOR CConEmuMain::GetPrimaryMonitor(MONITORINFO* pmi /*= NULL*/)
{
	wchar_t szInfo[128];
	MONITORINFO mi = {sizeof(mi)};
	RECT rcMonitor = MakeRect(0,0, GetSystemMetrics(SM_CXFULLSCREEN), GetSystemMetrics(SM_CYFULLSCREEN));

	HMONITOR hMon = MonitorFromRect(&rcMonitor, MONITOR_DEFAULTTOPRIMARY);
	BOOL bMonOk = hMon ? GetMonitorInfo(hMon, &mi) : FALSE;

	if (!bMonOk)
	{
		_ASSERTE(FALSE && "GetMonitorInfo failed");
		// ���� ����� � ���������� - ����� ������ �� ���������
		ZeroStruct(mi);
		SystemParametersInfo(SPI_GETWORKAREA, 0, &mi.rcWork, 0);
		mi.rcMonitor = rcMonitor;
	}

	if (gpSetCls->isAdvLogging >= 2)
	{
		_wsprintf(szInfo, SKIPLEN(countof(szInfo)) L"  GetPrimaryMonitor=%u -> hMon=x%08X Work=({%i,%i}-{%i,%i}) Area=({%i,%i}-{%i,%i})",
			bMonOk, (DWORD)hMon,
			mi.rcWork.left, mi.rcWork.top, mi.rcWork.right, mi.rcWork.bottom,
            mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right, mi.rcMonitor.bottom);
        LogString(szInfo);
	}

	if (pmi)
	{
		*pmi = mi;
	}
	return hMon;
}

LRESULT CConEmuMain::OnGetMinMaxInfo(LPMINMAXINFO pInfo)
{
	bool bLog = RELEASEDEBUGTEST((gpSetCls->isAdvLogging >= 2),true);
	wchar_t szMinMax[255];
	RECT rcWnd;

	if (bLog)
	{
		GetWindowRect(ghWnd, &rcWnd);
		_wsprintf(szMinMax, SKIPLEN(countof(szMinMax)) L"OnGetMinMaxInfo[before] MaxSize={%i,%i}, MaxPos={%i,%i}, MinTrack={%i,%i}, MaxTrack={%i,%i}, Cur={%i,%i}-{%i,%i}\n",
	          pInfo->ptMaxSize.x, pInfo->ptMaxSize.y,
	          pInfo->ptMaxPosition.x, pInfo->ptMaxPosition.y,
	          pInfo->ptMinTrackSize.x, pInfo->ptMinTrackSize.y,
	          pInfo->ptMaxTrackSize.x, pInfo->ptMaxTrackSize.y,
			  rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom);
		if (gpSetCls->isAdvLogging >= 2)
			LogString(szMinMax, true, false);
		DEBUGSTRSIZE(szMinMax);
	}


	MONITORINFO mi = {sizeof(mi)}, prm = {sizeof(prm)};
	GetNearestMonitor(&mi);
	if (gnOsVer >= 0x600)
		prm = mi;
	else
		GetPrimaryMonitor(&prm);
		

	// *** ���������� ���������� ������� �������
	RECT rcMin = MakeRect(MIN_CON_WIDTH,MIN_CON_HEIGHT);
	if (isVConExists(0))
		rcMin = CVConGroup::AllTextRect(true);
	RECT rcFrame = CalcRect(CER_MAIN, rcMin, CER_CONSOLE_ALL);
	pInfo->ptMinTrackSize.x = rcFrame.right;
	pInfo->ptMinTrackSize.y = rcFrame.bottom;


	if ((WindowMode == wmFullScreen) || (gpSet->isQuakeStyle && (gpSet->_WindowMode == wmFullScreen)))
	{
		if (pInfo->ptMaxTrackSize.x < ptFullScreenSize.x)
			pInfo->ptMaxTrackSize.x = ptFullScreenSize.x;

		if (pInfo->ptMaxTrackSize.y < ptFullScreenSize.y)
			pInfo->ptMaxTrackSize.y = ptFullScreenSize.y;

		pInfo->ptMaxPosition.x = 0;
		pInfo->ptMaxPosition.y = 0;
		pInfo->ptMaxSize.x = GetSystemMetrics(SM_CXFULLSCREEN);
		pInfo->ptMaxSize.y = GetSystemMetrics(SM_CYFULLSCREEN);

		//if (pInfo->ptMaxSize.x < ptFullScreenSize.x)
		//	pInfo->ptMaxSize.x = ptFullScreenSize.x;

		//if (pInfo->ptMaxSize.y < ptFullScreenSize.y)
		//	pInfo->ptMaxSize.y = ptFullScreenSize.y;
	}
	else if (gpSet->isQuakeStyle)
	{
		_ASSERTE(::IsZoomed(ghWnd) == FALSE); // ����� WS_MAXIMIZE ��� Quake ������������ �� ������!
		// ������� ����� ��� ���������� ������ "Normal"

		RECT rcWork;
		if (gpSet->_WindowMode == wmFullScreen)
		{
			rcWork = mi.rcMonitor;
		}
		else
		{
			rcWork = mi.rcWork;
		}

		if (pInfo->ptMaxTrackSize.x < (rcWork.right - rcWork.left))
			pInfo->ptMaxTrackSize.x = (rcWork.right - rcWork.left);

		if (pInfo->ptMaxTrackSize.y < (rcWork.bottom - rcWork.top))
			pInfo->ptMaxTrackSize.y = (rcWork.bottom - rcWork.top);
	}
	else //if (WindowMode == wmMaximized)
	{
		RECT rcShift = CalcMargins(CEM_FRAMEONLY, (WindowMode == wmNormal) ? wmMaximized : WindowMode);

		if (gnOsVer >= 0x600)
		{
			pInfo->ptMaxPosition.x = (mi.rcWork.left - mi.rcMonitor.left) - rcShift.left;
			pInfo->ptMaxPosition.y = (mi.rcWork.top - mi.rcMonitor.top) - rcShift.top;

			pInfo->ptMaxSize.x = mi.rcWork.right - mi.rcWork.left + (rcShift.left + rcShift.right);
			pInfo->ptMaxSize.y = mi.rcWork.bottom - mi.rcWork.top + (rcShift.top + rcShift.bottom);
		}
		else
		{
			// Issue 721: WinXP, TaskBar on top of screen. Assuming, mi.rcWork as {0,0,...}
			pInfo->ptMaxPosition.x = /*mi.rcWork.left*/ - rcShift.left;
			pInfo->ptMaxPosition.y = /*mi.rcWork.top*/ - rcShift.top;

			pInfo->ptMaxSize.x = prm.rcWork.right - prm.rcWork.left + (rcShift.left + rcShift.right);
			pInfo->ptMaxSize.y = prm.rcWork.bottom - prm.rcWork.top + (rcShift.top + rcShift.bottom);
		}
	}


	// ��� ����������...
	if (bLog)
	{
		_wsprintf(szMinMax, SKIPLEN(countof(szMinMax)) L"OnGetMinMaxInfo[after]  MaxSize={%i,%i}, MaxPos={%i,%i}, MinTrack={%i,%i}, MaxTrack={%i,%i}, Cur={%i,%i}-{%i,%i}\n",
	          pInfo->ptMaxSize.x, pInfo->ptMaxSize.y,
	          pInfo->ptMaxPosition.x, pInfo->ptMaxPosition.y,
	          pInfo->ptMinTrackSize.x, pInfo->ptMinTrackSize.y,
	          pInfo->ptMaxTrackSize.x, pInfo->ptMaxTrackSize.y,
			  rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom);
		if (gpSetCls->isAdvLogging >= 2)
			LogString(szMinMax, true, false);
		DEBUGSTRSIZE(szMinMax);
	}


	// If an application processes this message, it should return zero.
	return 0;
}

void CConEmuMain::OnHideCaption()
{
	mp_TabBar->OnCaptionHidden();

	//if (isZoomed())
	//{
	//	SetWindowMode(wmMaximized, TRUE);
	//}

	DEBUGTEST(bool bHideCaption = gpSet->isCaptionHidden());

	DWORD nStyle = GetWindowLong(ghWnd, GWL_STYLE);
	DWORD nNewStyle = FixWindowStyle(nStyle, WindowMode);
	DWORD nStyleEx = GetWindowLong(ghWnd, GWL_EXSTYLE);
	bool bNeedCorrect = (!isIconic() && (WindowMode != wmFullScreen)) && !m_QuakePrevSize.bWaitReposition;
	
	if (nNewStyle != nStyle)
	{
		if (gpSetCls->isAdvLogging)
		{
			wchar_t szInfo[128];
			_wsprintf(szInfo, SKIPLEN(countof(szInfo)) L"Changing main window 0x%08X style to 0x%08X", (DWORD)ghWnd, nNewStyle);
			gpConEmu->LogString(szInfo);
		}

		mb_IgnoreSizeChange = true;

		RECT rcClient = {}, rcBefore = {}, rcAfter = {};
		if (bNeedCorrect)
		{
			// AdjustWindowRectEx �� ������ ���������� � FullScreen. ������. � ����� � ���������� ���, ��������� ������.
			_ASSERTE(!isFullScreen() || (WindowMode==wmNormal || WindowMode==wmMaximized));

			// ��� ����� "�����������" ClientRect
			::GetClientRect(ghWnd, &rcClient);
			rcBefore = rcClient;
			MapWindowPoints(ghWnd, NULL, (LPPOINT)&rcBefore, 2);
			rcAfter = rcBefore;
			AdjustWindowRectEx(&rcBefore, nStyle, FALSE, nStyleEx);
			if (isZoomed())
			{
				int nCapY = GetSystemMetrics(SM_CYCAPTION);
				if (gpSet->isCaptionHidden())
					rcAfter.top -= nCapY;
				else
					rcAfter.top += nCapY;
			}
			AdjustWindowRectEx(&rcAfter, nNewStyle, FALSE, nStyleEx);

			// When switching from "No caption" to "Caption"
			if (WindowMode == wmNormal)
			{
				// we need to check window location, caption must not be "Out-of-screen"
				FixWindowRect(rcAfter, 0);
			}
		}

		//nStyle &= ~(WS_CAPTION|WS_THICKFRAME|WS_DLGFRAME);
		//SetWindowStyle(nStyle);
		//nStyle = FixWindowStyle(nStyle);
		//POINT ptBefore = {}; ClientToScreen(ghWnd, &ptBefore);

		if (bNeedCorrect && (rcBefore.left != rcAfter.left || rcBefore.top != rcAfter.top))
		{
			m_FixPosAfterStyle = GetTickCount();
			mrc_FixPosAfterStyle = rcAfter;
			//DWORD nFlags = SWP_NOSIZE|SWP_NOZORDER;
			/////nFlags |= SWP_DRAWFRAME;
			//SetWindowPos(ghWnd, NULL, rcAfter.left, rcAfter.top, 0,0, nFlags);
		}
		else
		{
			m_FixPosAfterStyle = 0;
		}

		mb_InCaptionChange = true;

		SetWindowStyle(nNewStyle);

		mb_InCaptionChange = false;

		//POINT ptAfter = {}; ClientToScreen(ghWnd, &ptAfter);
		//if ((!isIconic() && !isZoomed() && !isFullScreen())
		//	&& (ptBefore.x != ptAfter.x || ptBefore.y != ptAfter.y))
		//{
		//	RECT rcPos = {}; GetWindowRect(ghWnd, &rcPos);
		//	SetWindowPos(ghWnd, NULL, rcPos.left+ptBefore.x-ptAfter.x, rcPos.top+ptBefore.y-ptAfter.y, 0,0, SWP_NOSIZE|SWP_NOZORDER);
		//}
		//if (bNeedCorrect && (rcBefore.left != rcAfter.left || rcBefore.top != rcAfter.top))
		//{
		//	m_FixPosAfterStyle = GetTickCount();
		//	mrc_FixPosAfterStyle = rcAfter;
		//	//DWORD nFlags = SWP_NOSIZE|SWP_NOZORDER;
		//	/////nFlags |= SWP_DRAWFRAME;
		//	//SetWindowPos(ghWnd, NULL, rcAfter.left, rcAfter.top, 0,0, nFlags);
		//}
		//else
		//{
		//	m_FixPosAfterStyle = 0;
		//}
		mb_IgnoreSizeChange = FALSE;

		//if (changeFromWindowMode == wmNotChanging)
		//{
		//	//TODO: �������� �� FALSE
		//	ReSize(TRUE);
		//}
	}

	if (changeFromWindowMode == wmNotChanging)
	{
		UpdateWindowRgn();
		ReSize(FALSE);
	}
}

void CConEmuMain::OnGlobalSettingsChanged()
{
	gpConEmu->UpdateGuiInfoMapping();

	// � �������� � �������
	CVConGroup::OnUpdateGuiInfoMapping(&m_GuiInfo);
	
	// � ���� ���� ���������
	gpConEmu->UpdateFarSettings();
}

void CConEmuMain::OnPanelViewSettingsChanged(BOOL abSendChanges/*=TRUE*/)
{
	gpConEmu->UpdateGuiInfoMapping();
	
	// ��������� ����� gpSet->ThSet.crPalette[16], gpSet->ThSet.crFadePalette[16]
	COLORREF *pcrNormal = gpSet->GetColors(-1, FALSE);
	COLORREF *pcrFade = gpSet->GetColors(-1, TRUE);

	for(int i=0; i<16; i++)
	{
		// ����� FOR ����� � BitMask �� ����������
		gpSet->ThSet.crPalette[i] = (pcrNormal[i]) & 0xFFFFFF;
		gpSet->ThSet.crFadePalette[i] = (pcrFade[i]) & 0xFFFFFF;
	}

	gpSet->ThSet.nFontQuality = gpSetCls->FontQuality();
	
	// ��������� ��������� ������ ConEmu
	FillConEmuMainFont(&gpSet->ThSet.MainFont);
	//lstrcpy(gpSet->ThSet.MainFont.sFontName, gpSet->FontFaceName());
	//gpSet->ThSet.MainFont.nFontHeight = gpSetCls->FontHeight();
	//gpSet->ThSet.MainFont.nFontWidth = gpSetCls->FontWidth();
	//gpSet->ThSet.MainFont.nFontCellWidth = gpSet->FontSizeX3 ? gpSet->FontSizeX3 : gpSetCls->FontWidth();
	//gpSet->ThSet.MainFont.nFontQuality = gpSet->FontQuality();
	//gpSet->ThSet.MainFont.nFontCharSet = gpSet->FontCharSet();
	//gpSet->ThSet.MainFont.Bold = gpSet->FontBold();
	//gpSet->ThSet.MainFont.Italic = gpSet->FontItalic();
	//lstrcpy(gpSet->ThSet.MainFont.sBorderFontName, gpSet->BorderFontFaceName());
	//gpSet->ThSet.MainFont.nBorderFontWidth = gpSet->BorderFontWidth();
	
	// ��������� � ������� (������������ � "Panel View" � "Background"-��������
	bool lbChanged = gpSetCls->m_ThSetMap.SetFrom(&gpSet->ThSet);

	if (abSendChanges && lbChanged)
	{
		// � �������� ����������������
		CVConGroup::OnPanelViewSettingsChanged();
	}
}

void CConEmuMain::OnTaskbarCreated()
{
	Icon.OnTaskbarCreated();
	if (mp_DragDrop)
		mp_DragDrop->OnTaskbarCreated();
}

void CConEmuMain::OnTaskbarButtonCreated()
{
	// May be not initialized yet
	Taskbar_Init();

	if (!gpSet->isWindowOnTaskBar())
	{
		DWORD styleEx = GetWindowLong(ghWnd, GWL_EXSTYLE);
		if (styleEx & WS_EX_APPWINDOW)
		{
			styleEx &= ~WS_EX_APPWINDOW;
			SetWindowStyleEx(styleEx);
		}

		// ���� ��� ����� ���� �������� �� ��������
		Taskbar_DeleteTabXP(ghWnd);
	}
	else
	{
		if (gpConEmu->mb_IsUacAdmin)
		{
			// Bug in Win7? Sometimes after startup "As Admin" sheild does not appeears.
			SetTimer(ghWnd, TIMER_ADMSHIELD_ID, TIMER_ADMSHIELD_ELAPSE, NULL);
		}
	}
}

void CConEmuMain::OnTaskbarSettingsChanged()
{
	CVConGroup::OnTaskbarSettingsChanged();

	if (!gpSet->isTabsOnTaskBar())
		OnAllGhostClosed();

	DWORD styleEx = GetWindowLong(ghWnd, GWL_EXSTYLE);
	if (!gpSet->isWindowOnTaskBar())
	{
		if (styleEx & WS_EX_APPWINDOW)
		{
			styleEx &= ~WS_EX_APPWINDOW;
			SetWindowStyleEx(styleEx);
			// ���� ��� ����� ���� �������� �� ��������
			Taskbar_DeleteTabXP(ghWnd);
		}
	}
	else
	{
		if (!(styleEx & WS_EX_APPWINDOW))
		{
			styleEx |= WS_EX_APPWINDOW;
			SetWindowStyleEx(styleEx);
			// �������� �� ��������
			Taskbar_AddTabXP(ghWnd);
		}
	}

	apiSetForegroundWindow(ghOpWnd ? ghOpWnd : ghWnd);
}

void CConEmuMain::OnAltEnter()
{
	if (m_InsideIntegration)
	{
		_ASSERTE(FALSE && "Must not change mode in the Inside mode");
		return;
	}

	if (WindowMode != wmFullScreen)
		gpConEmu->SetWindowMode(wmFullScreen);
	else if (gpSet->isDesktopMode && (WindowMode != wmNormal))
		gpConEmu->SetWindowMode(wmNormal);
	else
		gpConEmu->SetWindowMode(gpConEmu->isWndNotFSMaximized ? wmMaximized : wmNormal);
}

void CConEmuMain::OnAltF9(BOOL abPosted/*=FALSE*/)
{
	if (m_InsideIntegration)
	{
		_ASSERTE(FALSE && "Must not change mode in the Inside mode");
		return;
	}

	if (!abPosted)
	{
		PostMessage(ghWnd, gpConEmu->mn_MsgPostAltF9, 0, 0);
		return;
	}

	// -- this will be done in SetWindowMode
	//StoreNormalRect(NULL); // ���� ����������, ����/�� ����

	gpConEmu->SetWindowMode((WindowMode != wmMaximized) ? wmMaximized : wmNormal);
}

bool CConEmuMain::InCreateWindow()
{
	return mb_InCreateWindow;
}

bool CConEmuMain::InQuakeAnimation()
{
	return (mn_QuakePercent != 0);
}

void CConEmuMain::OnMinimizeRestore(SingleInstanceShowHideType ShowHideType /*= sih_None*/)
{
	if (m_InsideIntegration)
		return;

	static bool bInFunction = false;
	if (bInFunction)
		return;

	bool bIsForeground = isMeForeground();
	bool bIsIconic = isIconic();
	BOOL bVis = IsWindowVisible(ghWnd);

	if ((ShowHideType == sih_HideTSA) || (ShowHideType == sih_Minimize))
	{
		if (bIsIconic || !bVis)
		{
			return;
		}
	}

	bInFunction = true;

	// 1. ������� ���������� ��� ������� ��������� ��������������� ������
	//    Win+C  -->  ShowHideType = sih_None
	// 2. ��� ������ �� ������� ���������� 
	//    �������� /single  --> ShowHideType = sih_Show
	// 3. ��� ������ �� ������� ����������
	//    �������� /showhide     --> ShowHideType = sih_ShowMinimize
	//    �������� /showhideTSA  --> ShowHideType = sih_ShowHideTSA
	SingleInstanceShowHideType cmd = sih_None;

	if (ShowHideType == sih_None)
	{
		// �� ����������
		ShowHideType = gpSet->isMinToTray() ? sih_ShowHideTSA : sih_ShowMinimize;
	}

	// Go
	if (ShowHideType == sih_Show)
	{
		cmd = sih_Show;
	}
	else if ((ShowHideType == sih_ShowMinimize)
		|| (ShowHideType == sih_Minimize)
		|| (ShowHideType == sih_ShowHideTSA)
		|| (ShowHideType == sih_HideTSA))
	{
		if ((bVis && (bIsForeground || gpSet->isAlwaysOnTop) && !bIsIconic)
			|| (ShowHideType == sih_HideTSA) || (ShowHideType == sih_Minimize))
		{
			// ���� ������ - ��������
			cmd = (ShowHideType == sih_HideTSA) ? sih_ShowHideTSA : ShowHideType;
		}
		else
		{
			// ����� - ��������
			cmd = sih_Show;
		}
	}
	else
	{
		_ASSERTE(ShowHideType == sih_SetForeground);
		// ����� - ���������� (� ����������� �� ������� ���������)
		if (IsWindowVisible(ghWnd) && (bIsForeground || !bIsIconic)) // (!bIsIconic) - ������ ����������, ���� ��������
			cmd = sih_SetForeground;
		else
			cmd = sih_Show;
	}

	// �������
	int nQuakeMin = 1;
	int nQuakeShift = 10;
	int nQuakeDelay = 20;

	if (cmd == sih_SetForeground)
	{
		apiSetForegroundWindow(ghWnd);
	}
	else if (cmd == sih_Show)
	{
		HWND hWndFore = GetForegroundWindow();
		// ���� ��������� ���� ������ �� ������ �� �������� (Quake ��� �������) �� �� WinXP
		// ����� ���� ���� - ������� ��������� ��������� (WM_ACTIVATE, WM_SYSCOMMAND)
		bool bNoQuakeAnimation = false;

		apiSetForegroundWindow(ghWnd);

		DEBUGTEST(bool bWasQuakeIconic = isQuakeMinimized);

		if (gpSet->isQuakeStyle /*&& !isMouseOverFrame()*/)
		{
			if ((hWndFore == ghWnd) && !bIsIconic && bVis)
			{
				bNoQuakeAnimation = true;
				mn_QuakePercent = 0;
			}
			else
			{
				StopForceShowFrame();
				mn_QuakePercent = nQuakeMin;
				UpdateWindowRgn();
			}
		}
		else
			mn_QuakePercent = 0;

		if (Icon.isWindowInTray() || !IsWindowVisible(ghWnd))
		{
			mb_LockWindowRgn = TRUE;
			Icon.RestoreWindowFromTray();
			mb_LockWindowRgn = FALSE;
			bIsIconic = isIconic();
		}
		
		// ����� - ���������� �������� IsIconic. ��� isQuakeStyle ����� ���� ����
		if (::IsIconic(ghWnd))
		{
			bool b = mb_PassSysCommand; mb_PassSysCommand = true;
			SendMessage(ghWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
			mb_PassSysCommand = b;
		}

		// ���������, ��������� ������� ��� Quake
		if (gpSet->isQuakeStyle)
		{
			RECT rcWnd = GetDefaultRect();
			SetWindowPos(ghWnd, NULL, rcWnd.left, rcWnd.top, 0,0, SWP_NOZORDER|SWP_NOSIZE);
		}

		isQuakeMinimized = false; // ������ ����� ��������

		apiSetForegroundWindow(ghWnd);

		if (gpSet->isQuakeStyle /*&& !isMouseOverFrame()*/)
		{
			if (!bNoQuakeAnimation)
			{
				StopForceShowFrame();
				while (mn_QuakePercent < 100)
				{
					mn_QuakePercent += nQuakeShift;
					UpdateWindowRgn();
					RedrawWindow(ghWnd, NULL, NULL, RDW_UPDATENOW);
					Sleep(nQuakeDelay);
				}
			}
			if (mn_QuakePercent != 100)
			{
				mn_QuakePercent = 100;
				UpdateWindowRgn();
			}
		}
		mn_QuakePercent = 0; // 0 - ��������
	}
	else
	{
		// �����������
		_ASSERTE(((cmd == sih_ShowHideTSA) || (cmd == sih_ShowMinimize) || (cmd == sih_Minimize)) && "cmd must be determined!");

		isQuakeMinimized = true; // ����� �������, ����� �� ������. ������������ ������ ��� gpSet->isQuakeStyle

		if (bVis && !bIsIconic)
		{
			//UpdateIdealRect();
			StoreIdealRect();
		}

		if (gpSet->isQuakeStyle /*&& !isMouseOverFrame()*/)
		{
			mn_QuakePercent = 100 + nQuakeMin - nQuakeShift;
			StopForceShowFrame();
			while (mn_QuakePercent > 0)
			{
				UpdateWindowRgn();
				RedrawWindow(ghWnd, NULL, NULL, RDW_UPDATENOW);
				Sleep(nQuakeDelay);
				mn_QuakePercent -= nQuakeShift;
			}
		}
		mn_QuakePercent = 0; // 0 - ��������


		if (cmd == sih_ShowHideTSA)
		{
			mb_LockWindowRgn = TRUE;
			// ���� ��������� � TSA ��������
			Icon.HideWindowToTray();
			mb_LockWindowRgn = FALSE;
		}
		else if ((cmd == sih_ShowMinimize) || (ShowHideType == sih_Minimize))
		{
			if (gpSet->isQuakeStyle)
			{
				// ��� ������ ���� - ������ �������� ������ � �������� �� �����. ����� ������...

				// ����� ���� "���" ����
				HWND hNext = FindWindowEx(NULL, ghWnd, NULL, NULL);
				while (hNext)
				{
					if (::IsWindowVisible(hNext))
					{
						// ��� �������, ������ Alt-Tab?
						DWORD nStylesEx = GetWindowLong(hNext, GWL_EXSTYLE);
						DEBUGTEST(DWORD nStyles = GetWindowLong(hNext, GWL_STYLE));
						if (!(nStylesEx & WS_EX_TOOLWINDOW))
						{
							break;
						}
					}
					hNext = FindWindowEx(NULL, hNext, NULL, NULL);
				}
				// � ��������� � ���
				SetWindowPos(ghWnd, NULL, -32000, -32000, 0,0, SWP_NOZORDER|SWP_NOSIZE);
				SetForegroundWindow(hNext ? hNext : GetDesktopWindow());
			}
			else
			{
				// SC_MINIMIZE ��� ���������� (gpSet->isMinToTray || gpConEmu->ForceMinimizeToTray)
				SendMessage(ghWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
			}
		}
	}

	bInFunction = false;
 }

void CConEmuMain::OnForcedFullScreen(bool bSet /*= true*/)
{
	static bool bWasSetTopMost = false;

	if (!bSet)
	{
		// ����� ���� "OnTop", ������� ���������� ���������� ���������
		if (bWasSetTopMost)
		{
			gpSet->isAlwaysOnTop = false;
			OnAlwaysOnTop();
			bWasSetTopMost = false;
		}
		return;
	}

	TODO("���� �������� - �� ���������");
#if 0
	// ���������� ����������� ������� � ��������� FullScreen
	if (!IsWindows64())
	{
		//BOOL WINAPI SetConsoleDisplayMode(HANDLE hConsoleOutput, DWORD dwFlags, PCOORD lpNewScreenBufferDimensions);
		if (mp_ VActive && mp_ VActive->RCon())
		{
			if (!isIconic())
				SendMessage(ghWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);

			CVConGuard guard(mp_ VActive);
			if (mp_ VActive->RCon()->SetFullScreen())
				return;

			SendMessage(ghWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
		}
	}
#endif


	if (gpSet->isDesktopMode)
	{
		DisplayLastError(L"Can't set FullScreen in DesktopMode", -1);
		return;
	}

	TODO("������� ��������� ���������");
	
	// ���������� AlwaysOnTop
	if (!gpSet->isAlwaysOnTop)
	{
		gpSet->isAlwaysOnTop = true;
		OnAlwaysOnTop();
		bWasSetTopMost = true;
	}

	if (!isMeForeground())
	{
		if (isIconic())
		{
			if (Icon.isWindowInTray() || !IsWindowVisible(ghWnd))
				Icon.RestoreWindowFromTray();
			else
				SendMessage(ghWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
		}
		//else
		//{
		//	//SendMessage(ghWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
		//	SendMessage(ghWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
		//}
	}

	SetWindowMode(wmFullScreen);

	if (!isMeForeground())
	{
		SwitchToThisWindow(ghWnd, FALSE);
	}
}

LRESULT CConEmuMain::OnKeyboard(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
#ifdef _DEBUG
	#if 0
	if (messg == WM_KEYDOWN || messg == WM_KEYUP)
	{
		if (wParam >= VK_F1 && wParam <= VK_F4)
		{
			if (messg == WM_KEYUP)
			{
				DWORD nFlags = 0;
				switch ((DWORD)wParam)
				{
				case VK_F1:
					nFlags = AW_SLIDE|AW_HIDE|AW_HOR_NEGATIVE|AW_VER_NEGATIVE;
					break;
				case VK_F2:
					nFlags = AW_SLIDE|AW_HOR_POSITIVE|AW_VER_POSITIVE;
					break;
				case VK_F3:
					nFlags = AW_BLEND|AW_HIDE|AW_CENTER;
					break;
				case VK_F4:
					nFlags = AW_BLEND|AW_CENTER;
					break;
				}
				BOOL b = AnimateWindow(ghWnd, 500, nFlags);
				DWORD nErr = GetLastError();
				wchar_t szErr[128];
				_wsprintf(szErr, SKIPLEN(countof(szErr)) L"AnimateWindow(%08x)=%u, Err=%u\n", nFlags, b, nErr);
				DEBUGSTRANIMATE(szErr);
			}
			return 0;
		}
	}
	#endif
#endif

	if (!m_GuiInfo.bGuiActive)
	{
		UpdateGuiInfoMappingActive(true);
	}

	if (mb_ImeMethodChanged)
	{
		if (messg == WM_SYSKEYDOWN || messg == WM_SYSKEYUP)
		{
			if (messg == WM_SYSKEYUP)
				mb_ImeMethodChanged = false;
			return 0;
		}
		else
		{
			mb_ImeMethodChanged = false;
		}
	}

	WORD bVK = (WORD)(wParam & 0xFF);
	//-- �����������. ��������� �� LCtrl+LAlt
	//bool bAltGr = (messg == WM_KEYDOWN && bVK == VK_MENU);
	//_ASSERTE(!bAltGr);

	#ifdef _DEBUG
	wchar_t szDebug[255];
	_wsprintf(szDebug, SKIPLEN(countof(szDebug)) L"%s(VK=0x%02X, Scan=%i, lParam=0x%08X)\n",
	          (messg == WM_KEYDOWN) ? L"WM_KEYDOWN" :
	          (messg == WM_KEYUP) ? L"WM_KEYUP" :
	          (messg == WM_SYSKEYDOWN) ? L"WM_SYSKEYDOWN" :
	          (messg == WM_SYSKEYUP) ? L"WM_SYSKEYUP" :
	          L"<Unknown Message> ",
	          bVK, ((DWORD)lParam & 0xFF0000) >> 16, (DWORD)lParam);
	DEBUGSTRKEY(szDebug);
	#endif


	if (gpSetCls->isAdvLogging)
		CVConGroup::LogInput(messg, wParam, lParam);


#if 1
	// Works fine, but need to cache last pressed key, cause of need them in WM_KEYUP (send char to console)
	wchar_t szTranslatedChars[16] = {0};
	int nTranslatedChars = 0;
	MSG msgList[16] = {{hWnd,messg,wParam,lParam}};
	int iAddMsgList = 0, iDeadMsgIndex = 0;
	int iProcessDeadChars = 0;
	//static bool bWasDeadChar = false;
	//static LPARAM nDeadCharLParam = 0;

	if (messg == WM_KEYDOWN || messg == WM_SYSKEYDOWN)
	{
		_ASSERTE(sizeof(szTranslatedChars) == sizeof(m_TranslatedChars[0].szTranslatedChars));
		//BOOL lbDeadChar = FALSE;
		MSG msg, msg1;
		LPARAM lKey1ScanCode = (lParam & 0x00FF0000);
		DEBUGTEST(LPARAM lKey2ScanCode = 0);

		// 120722 - Spanish (for example) layout. ���� ��������� ������ ��� �����
		// �������� � ������� Grave/Acute/Circumflex/Tilde/Diaeresis/Ring Above
		// ��������, ��� ����� "~" � "a" ������� ����������� ��������� ������������������
		// WM_KEYDOWN, WM_DEADCHAR (~), WM_KEYUP, WM_KEYDOWN, WM_CHAR ("a" � "~"), WM_KEYUP
		// ������ ������ � ��� ������, ���� ��� �������� � ��������� ���� ���������.
		// ��� ������� �������� � ��������� - �������� ������ "a" ��� ������.

		TODO("��������� �������� ��������� � '^'");

		while (nTranslatedChars < 15  // ������� �� ������ ��� ����������� WM_CHAR & WM_SYSCHAR
		        && PeekMessage(&msg, 0,0,0, PM_NOREMOVE)
		      )
		{
			if (!(msg.message == WM_CHAR || msg.message == WM_SYSCHAR
					|| msg.message == WM_DEADCHAR || msg.message == WM_SYSDEADCHAR
					|| (iProcessDeadChars == 1 // ��� WM_DEADCHAR/WM_SYSDEADCHAR, ������� WM_KEYUP � WM_KEYDOWN
						&& ((msg.message == WM_KEYUP && (msg.lParam & 0x00FF0000) == lKey1ScanCode)
							|| (msg.message == WM_KEYDOWN /* ��������� �� DeadChar ���������� ����� */))
						)
					|| (iProcessDeadChars == 2 // ��� WM_DEADCHAR/WM_SYSDEADCHAR, ������ ������� WM_CHAR
						&& (msg.message == WM_CHAR || msg.message == WM_SYSCHAR)
						)
					))
				//|| (msg.lParam & 0xFF0000) != lKey1ScanCode /* ���������� ����-���� */) // 120722 - �����
			{
				break;
			}

			if (GetMessage(&msg1, 0,0,0))  // ������ �� ������
			{
				ConEmuMsgLogger::Log(msg1, ConEmuMsgLogger::msgCommon);
				_ASSERTE(msg1.message == msg.message && msg1.wParam == msg.wParam && msg1.lParam == msg.lParam);
				msgList[++iAddMsgList] = msg1;

				if (msg.message == WM_CHAR || msg.message == WM_SYSCHAR)
				{
					szTranslatedChars[nTranslatedChars ++] = (wchar_t)msg1.wParam;
					if (iProcessDeadChars == 1)
					{
						_ASSERTE(iProcessDeadChars == 2); // ��� ������ ���� ���� ���������� � WM_KEYDOWN
						iProcessDeadChars = 2;
					}
				}
				else if (msg.message == WM_DEADCHAR || msg.message == WM_SYSDEADCHAR)
				{
					_ASSERTE(iProcessDeadChars==0 && "Must be first entrance");
					//lbDeadChar = TRUE;
					iProcessDeadChars = 1;
				}
				else if (msg.message == WM_KEYDOWN)
				{
					if (iProcessDeadChars == 1)
					{
						iProcessDeadChars = 2;
						DEBUGTEST(lKey2ScanCode = (msg.lParam & 0x00FF0000));
						iDeadMsgIndex = iAddMsgList;
					}
				}

				// 120722 - ���� DispatchMessage ������ TranslateMessage.
				// ��������� ���������� ���������
				TranslateMessage(&msg1);
				//DispatchMessage(&msg1);
			}
		}

		if (gpSetCls->isAdvLogging)
		{
			for (int i = 1; i <= iAddMsgList; i++)
			{
				CVConGroup::LogInput(msgList[i].message, msgList[i].wParam, msgList[i].lParam);
			}
		}

		if (iProcessDeadChars == 2)
		{
			_ASSERTE(iDeadMsgIndex>0);
			_ASSERTE(msgList[iDeadMsgIndex].message == messg);
			wParam = msgList[iDeadMsgIndex].wParam;
			lParam = msgList[iDeadMsgIndex].lParam;
			bVK = (WORD)(wParam & 0xFF);
		}

		//if (lbDeadChar)
		//{
		//	bWasDeadChar = true;
		//	nDeadCharLParam = lParam;
		//}
		//else
		//{
		//	bWasDeadChar = false;
		//}

		//if (lbDeadChar && nTranslatedChars)
		//{
		//	_ASSERTE(FALSE && "Dead char does not produces szTranslatedChars");
		//	lbDeadChar = FALSE;
		//}

		memmove(m_TranslatedChars[bVK].szTranslatedChars, szTranslatedChars, sizeof(szTranslatedChars));
	}
	else
	{
		_ASSERTE((messg == WM_KEYUP || messg == WM_SYSKEYUP) && "Unexpected msg");
		//if (messg == WM_KEYUP || messg == WM_SYSKEYUP)
		//{
		//	if (bWasDeadChar)
		//	{
		//		if ((nDeadCharLParam & 0xFF0000) == (lParam & 0xFF0000))
		//		{
		//			//DefWindowProc(hWnd, messg, wParam, lParam);
		//		}
		//		bWasDeadChar = false;
		//	}
		//}
		//else
		//{
		//	_ASSERTE((messg == WM_KEYUP || messg == WM_SYSKEYUP) && "Unexpected msg");
		//}

		szTranslatedChars[0] = m_TranslatedChars[bVK].szTranslatedChars[0];
		szTranslatedChars[1] = 0;
		nTranslatedChars = szTranslatedChars[0] ? 1 : 0;
	}

#endif
#if 0
	/* Works, but has problems with dead chars */
	wchar_t szTranslatedChars[11] = {0};
	int nTranslatedChars = 0;

	if (!GetKeyboardState(m_KeybStates))
	{
		#ifdef _DEBUG
		DWORD dwErr = GetLastError();
		_ASSERTE(FALSE);
		#endif
		static bool sbErrShown = false;

		if (!sbErrShown)
		{
			sbErrShown = true;
			DisplayLastError(L"GetKeyboardState failed!");
		}
	}
	else
	{
		HKL hkl = (HKL)GetActiveKeyboardLayout();
		UINT nVK = wParam & 0xFFFF;
		UINT nSC = ((DWORD)lParam & 0xFF0000) >> 16;
		WARNING("BUGBUG: ������ ������ � x64 �� US-Dvorak");
		nTranslatedChars = ToUnicodeEx(nVK, nSC, m_KeybStates, szTranslatedChars, 10, 0, hkl);
		if (nTranslatedChars>0) szTranslatedChars[min(10,nTranslatedChars)] = 0; else szTranslatedChars[0] = 0;
	}

#endif
#if 0
	/* Invalid ? */
	wchar_t szTranslatedChars[16] = {0};

	if (!GetKeyboardState(m_KeybStates))
	{
#ifdef _DEBUG
		DWORD dwErr = GetLastError();
		_ASSERTE(FALSE);
#endif
		static bool sbErrShown = false;

		if (!sbErrShown)
		{
			sbErrShown = true;
			DisplayLastError(L"GetKeyboardState failed!");
		}
	}

	//MSG smsg = {hWnd, messg, wParam, lParam};
	//TranslateMessage(&smsg);

	/*if (bVK == VK_SHIFT || bVK == VK_MENU || bVK == VK_CONTROL || bVK == VK_LWIN || bVK == VK_RWIN) {
		if (messg == WM_KEYDOWN || messg == WM_SYSKEYDOWN) {
			if (
		} else if (messg == WM_KEYUP || messg == WM_SYSKEYUP) {
		}
	}*/

	if (messg == WM_KEYDOWN || messg == WM_SYSKEYDOWN)
	{
		HKL hkl = (HKL)m_ActiveKeybLayout; //GetActiveKeyboardLayout();
		UINT nVK = wParam & 0xFFFF;
		UINT nSC = ((DWORD)lParam & 0xFF0000) >> 16;
		WARNING("BUGBUG: ������ ������ � x64 �� US-Dvorak");
		int nTranslatedChars = ToUnicodeEx(nVK, nSC, m_KeybStates, szTranslatedChars, 15, 0, hkl);

		if (nTranslatedChars >= 0)
		{
			// 2 or more
			// Two or more characters were written to the buffer specified by pwszBuff.
			// The most common cause for this is that a dead-key character (accent or diacritic)
			// stored in the keyboard layout could not be combined with the specified virtual key
			// to form a single character. However, the buffer may contain more characters than the
			// return value specifies. When this happens, any extra characters are invalid and should be ignored.
			szTranslatedChars[min(15,nTranslatedChars)] = 0;
		}
		else if (nTranslatedChars == -1)
		{
			// The specified virtual key is a dead-key character (accent or diacritic).
			// This value is returned regardless of the keyboard layout, even if several
			// characters have been typed and are stored in the keyboard state. If possible,
			// even with Unicode keyboard layouts, the function has written a spacing version
			// of the dead-key character to the buffer specified by pwszBuff. For example, the
			// function writes the character SPACING ACUTE (0x00B4),
			// rather than the character NON_SPACING ACUTE (0x0301).
			szTranslatedChars[0] = 0;
		}
		else
		{
			// Invalid
			szTranslatedChars[0] = 0;
		}

		mn_LastPressedVK = bVK;
	}

#endif

	if (messg == WM_KEYDOWN && !mb_HotKeyRegistered)
		RegisterHotKeys(); // Win � ������

	_ASSERTE(messg != WM_CHAR && messg != WM_SYSCHAR);

	// ������ ���������� ��������� "�����" ������
	if (wParam == VK_ESCAPE)
	{
		if (mp_DragDrop->InDragDrop())
			return 0;

		static bool bEscPressed = false;

		if (messg != WM_KEYUP)
			bEscPressed = true;
		else if (!bEscPressed)
			return 0;
		else
			bEscPressed = false;
	}
	
	if ((wParam == VK_CAPITAL) || (wParam == VK_NUMLOCK) || (wParam == VK_SCROLL))
		mp_Status->OnKeyboardChanged();

	// ������ - ����� ��������� � �������
	CVConGuard VCon;
	if ((GetActiveVCon(&VCon) >= 0) && VCon->RCon())
	{
		VCon->RCon()->OnKeyboard(hWnd, messg, wParam, lParam, szTranslatedChars);
	}
	else if (((wParam & 0xFF) >= VK_WHEEL_FIRST) && ((wParam & 0xFF) <= VK_WHEEL_LAST))
	{
		// ����� ���� � ���������� ��������� �� ������, � �� ��� "�����" ������ �� ���������
		_ASSERTE(!(((wParam & 0xFF) >= VK_WHEEL_FIRST) && ((wParam & 0xFF) <= VK_WHEEL_LAST)));
	}
	else
	{
		// ���� ����� �������� ������� ���� (������?) �� ������� ���������� ���-����� ����
		ProcessHotKeyMsg(messg, wParam, lParam, szTranslatedChars, NULL);
	}

	return 0;
}

LRESULT CConEmuMain::OnKeyboardHook(DWORD VkMod)
{
	if (!VkMod)
		return 0;

	CVConGuard VCon;
	CRealConsole* pRCon = (GetActiveVCon(&VCon) >= 0) ? VCon->RCon() : NULL;

	const ConEmuHotKey* pHotKey = ProcessHotKey(VkMod, true/*bKeyDown*/, NULL, pRCon);

	if (pHotKey == ConEmuSkipHotKey)
	{
		// ���� ������� ����������� �� ����������
		pHotKey = ProcessHotKey(VkMod, false/*bKeyDown*/, NULL, pRCon);
	}

	return 0;
}

void CConEmuMain::OnConsoleKey(WORD vk, LPARAM Mods)
{
	// ��������� ���������� �������������� � ����� ConEmu � ���� ��������� �� ���� �� ������
#if 0
	if (vk == VK_SPACE && (Mods == (Mods & (MOD_ALT|MOD_LALT|MOD_RALT))))
	{
		_ASSERTE(vk != VK_SPACE);
		if (!gpSet->isSendAltSpace)
		{
			ShowSysmenu();
			return;
		}
	}
	if (vk == VK_RETURN && (Mods == (Mods & (MOD_ALT|MOD_LALT|MOD_RALT))))
	{
		_ASSERTE(vk != VK_RETURN);
		if (!gpSet->isSendAltEnter)
		{
			OnAltEnter();
			return;
		}
	}
#endif

	// ��� ��������� - ������� �������
	CVConGuard VCon;
	CRealConsole* pRCon = (CVConGroup::GetActiveVCon(&VCon) >= 0) ? VCon->RCon() : NULL;
	if (pRCon)
	{
		INPUT_RECORD r = {KEY_EVENT};
		// Alt � ������ ����� �� ����� - �� ��� ������
		r.Event.KeyEvent.bKeyDown = TRUE;
		r.Event.KeyEvent.wRepeatCount = 1;
		r.Event.KeyEvent.wVirtualKeyCode = vk;
		r.Event.KeyEvent.wVirtualScanCode = /*28 �� ���� ����������*/MapVirtualKey(vk, 0/*MAPVK_VK_TO_VSC*/);
		if (Mods & MOD_LALT)
			r.Event.KeyEvent.dwControlKeyState |= LEFT_ALT_PRESSED;
		if (Mods & MOD_RALT)
			r.Event.KeyEvent.dwControlKeyState |= RIGHT_ALT_PRESSED;
		if (Mods & MOD_LCONTROL)
			r.Event.KeyEvent.dwControlKeyState |= LEFT_CTRL_PRESSED;
		if (Mods & MOD_RCONTROL)
			r.Event.KeyEvent.dwControlKeyState |= RIGHT_CTRL_PRESSED;
		if (Mods & MOD_SHIFT)
			r.Event.KeyEvent.dwControlKeyState |= SHIFT_PRESSED;
		pRCon->PostConsoleEvent(&r);
		//On Keyboard(hConWnd, WM_KEYUP, VK_RETURN, 0);
		r.Event.KeyEvent.bKeyDown = FALSE;
		pRCon->PostConsoleEvent(&r);
	}
}

bool CConEmuMain::isInImeComposition()
{
	return mb_InImeComposition;
}

void CConEmuMain::UpdateImeComposition()
{
	if (!mh_Imm32)
		return;

	CVConGuard VCon;
	if (GetActiveVCon(&VCon) >= 0)
	{
		HWND hView = VCon->GetView();

		CONSOLE_CURSOR_INFO ci = {};
		CONSOLE_SCREEN_BUFFER_INFO sbi = {};
		CRealConsole* pRCon = VCon->RCon();
		pRCon->GetConsoleCursorInfo(&ci);
		pRCon->GetConsoleScreenBufferInfo(&sbi);
		COORD crVisual = pRCon->BufferToScreen(sbi.dwCursorPosition);
		crVisual.X = max(0,min(crVisual.X,(int)pRCon->TextWidth()));
		crVisual.Y = max(0,min(crVisual.Y,(int)pRCon->TextHeight()));
		POINT ptCurPos = VCon->ConsoleToClient(crVisual.X, crVisual.Y);

		MapWindowPoints(hView, ghWnd, &ptCurPos, 1);

		COMPOSITIONFORM cf = {CFS_POINT, ptCurPos};

		HIMC hImc = _ImmGetContext(ghWnd/*hView?*/);
		_ImmSetCompositionWindow(hImc, &cf);

		LOGFONT lf; gpSetCls->GetMainLogFont(lf);
		_ImmSetCompositionFont(hImc, &lf);
	}
}

LRESULT CConEmuMain::OnKeyboardIme(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	bool lbProcessed = false;
	wchar_t szDbgMsg[255];
	switch (messg)
	{
		case WM_IME_CHAR:
			{
				// ���������� � RealConsole (��� ���� �� ������ IME)
				_wsprintf(szDbgMsg, SKIPLEN(countof(szDbgMsg)) L"WM_IME_CHAR: char=%c, wParam=%u, lParam=0x%08X\n", (wchar_t)wParam, (DWORD)wParam, (DWORD)lParam);
				DEBUGSTRIME(szDbgMsg);
				CVConGuard VCon;
				if (GetActiveVCon(&VCon) >= 0)
				{
					VCon->RCon()->OnKeyboardIme(hWnd, messg, wParam, lParam);
				}
			}
			break;
		case WM_IME_COMPOSITION:
			_wsprintf(szDbgMsg, SKIPLEN(countof(szDbgMsg)) L"WM_IME_COMPOSITION: wParam=0x%08X, lParam=0x%08X\n", (DWORD)wParam, (DWORD)lParam);
			DEBUGSTRIME(szDbgMsg);
			UpdateImeComposition();
			//if (lParam & GCS_RESULTSTR) 
			//{
			//	HIMC hIMC = ImmGetContext(hWnd);
			//	if (hIMC)
			//	{
			//		DWORD dwSize;
			//		wchar_t* lpstr;
			//		// Get the size of the result string. 
			//		dwSize = ImmGetCompositionString(hIMC, GCS_RESULTSTR, NULL, 0);
			//		// increase buffer size for terminating null character,  
			//		dwSize += sizeof(wchar_t);
			//		lpstr = (wchar_t*)calloc(dwSize,1);
			//		if (lpstr)
			//		{
			//			// Get the result strings that is generated by IME into lpstr. 
			//			ImmGetCompositionString(hIMC, GCS_RESULTSTR, lpstr, dwSize);
			//			DEBUGSTRIME(L"GCS_RESULTSTR: ");
			//			DEBUGSTRIME(lpstr);
			//			// add this string into text buffer of application 
			//			free(lpstr);
			//		}
			//		ImmReleaseContext(hWnd, hIMC);
			//	}
			//}
			break;
		case WM_IME_COMPOSITIONFULL:
			_wsprintf(szDbgMsg, SKIPLEN(countof(szDbgMsg)) L"WM_IME_COMPOSITIONFULL: wParam=0x%08X, lParam=0x%08X\n", (DWORD)wParam, (DWORD)lParam);
			DEBUGSTRIME(szDbgMsg);
			break;
		case WM_IME_CONTROL:
			_wsprintf(szDbgMsg, SKIPLEN(countof(szDbgMsg)) L"WM_IME_CONTROL: wParam=0x%08X, lParam=0x%08X\n", (DWORD)wParam, (DWORD)lParam);
			DEBUGSTRIME(szDbgMsg);
			break;
		case WM_IME_ENDCOMPOSITION:
			_wsprintf(szDbgMsg, SKIPLEN(countof(szDbgMsg)) L"WM_IME_ENDCOMPOSITION: wParam=0x%08X, lParam=0x%08X\n", (DWORD)wParam, (DWORD)lParam);
			DEBUGSTRIME(szDbgMsg);
			// IME ��������. ������� ������� ����� ����� �������� � �������
			mb_InImeComposition = false;
			break;
		case WM_IME_KEYDOWN:
			_wsprintf(szDbgMsg, SKIPLEN(countof(szDbgMsg)) L"WM_IME_KEYDOWN: wParam=0x%08X, lParam=0x%08X\n", (DWORD)wParam, (DWORD)lParam);
			DEBUGSTRIME(szDbgMsg);
			break;
		case WM_IME_KEYUP:
			_wsprintf(szDbgMsg, SKIPLEN(countof(szDbgMsg)) L"WM_IME_KEYUP: wParam=0x%08X, lParam=0x%08X\n", (DWORD)wParam, (DWORD)lParam);
			DEBUGSTRIME(szDbgMsg);
			break;
		case WM_IME_NOTIFY:
			_wsprintf(szDbgMsg, SKIPLEN(countof(szDbgMsg)) L"WM_IME_NOTIFY: wParam=0x%08X, lParam=0x%08X\n", (DWORD)wParam, (DWORD)lParam);
			DEBUGSTRIME(szDbgMsg);
			switch (wParam)
			{
				case IMN_SETOPENSTATUS:
					// ���-�� �������, ������� Alt` (� �������� ���������) �������� � ������� ����������� ������
					// (���������� ������� � �������)
					mb_ImeMethodChanged = (wParam == IMN_SETOPENSTATUS) && isPressed(VK_MENU);
					break;
				//case 0xF:
				//	{
				//		wchar_t szInfo[1024];
				//		getWindowInfo((HWND)lParam, szInfo);
				//	}
				//	break;
			}
			break;
		case WM_IME_REQUEST:
			_wsprintf(szDbgMsg, SKIPLEN(countof(szDbgMsg)) L"WM_IME_REQUEST: wParam=0x%08X, lParam=0x%08X\n", (DWORD)wParam, (DWORD)lParam);
			DEBUGSTRIME(szDbgMsg);
			//if (wParam == IMR_QUERYCHARPOSITION)
			//{
			//	IMECHARPOSITION* p = (IMECHARPOSITION*)lParam;
			//	GetWindowRect(ghWnd DC, &p->rcDocument);
			//	p->cLineHeight = gpSetCls->FontHeight();
			//	if (mp_ VActive && mp_ VActive->RCon())
			//	{
			//		COORD crCur = {};
			//		mp_ VActive->RCon()->GetConsoleCursorPos(&crCur);
			//		p->pt = mp_ VActive->ConsoleToClient(crCur.X, crCur.Y);
			//	}
			//	lbProcessed = true;
			//	result = TRUE;
			//}
			break;
		case WM_IME_SELECT:
			_wsprintf(szDbgMsg, SKIPLEN(countof(szDbgMsg)) L"WM_IME_SELECT: wParam=0x%08X, lParam=0x%08X\n", (DWORD)wParam, (DWORD)lParam);
			DEBUGSTRIME(szDbgMsg);
			break;
		case WM_IME_SETCONTEXT:
			// If the application draws the composition window, 
			// the default IME window does not have to show its composition window.
			// In this case, the application must clear the ISC_SHOWUICOMPOSITIONWINDOW
			// value from the lParam parameter before passing the message to DefWindowProc
			// or ImmIsUIMessage. To display a certain user interface window, an application
			// should remove the corresponding value so that the IME will not display it.
			_wsprintf(szDbgMsg, SKIPLEN(countof(szDbgMsg)) L"WM_IME_SETCONTEXT: wParam=0x%08X, lParam=0x%08X\n", (DWORD)wParam, (DWORD)lParam);
			DEBUGSTRIME(szDbgMsg);
			break;
		case WM_IME_STARTCOMPOSITION:
			_wsprintf(szDbgMsg, SKIPLEN(countof(szDbgMsg)) L"WM_IME_STARTCOMPOSITION: wParam=0x%08X, lParam=0x%08X\n", (DWORD)wParam, (DWORD)lParam);
			DEBUGSTRIME(szDbgMsg);
			UpdateImeComposition();
			// ������ IME. ������ ����� ������������ ������� ������� (�� �������� � �������)
			mb_InImeComposition = true;
			break;
		default:
			_ASSERTE(messg==0);
	}
	if (!lbProcessed)
		result = DefWindowProc(hWnd, messg, wParam, lParam);
	return result;
}

LRESULT CConEmuMain::OnLangChange(UINT messg, WPARAM wParam, LPARAM lParam)
{
	/*
	**********
	������ ���� � ��������� ������� (WinXP SP3)
	**********
	En -> Ru : --> ������� ����� �����!
	**********
	19:17:15.043(gui.4720) ConEmu: WM_INPUTLANGCHANGEREQUEST(CP:1, HKL:0x04190419)
	19:17:17.043(gui.4720) ConEmu: WM_INPUTLANGCHANGE(CP:204, HKL:0x04190419)
	19:17:19.043(gui.4720) ConEmu: GetKeyboardLayout(0) after DefWindowProc(WM_INPUTLANGCHANGE) = 0x04190419)
	19:17:21.043(gui.4720) ConEmu: GetKeyboardLayout(0) after DefWindowProc(WM_INPUTLANGCHANGEREQUEST) = 0x04190419)
	19:17:23.043(gui.4720) CRealConsole::SwitchKeyboardLayout(CP:1, HKL:0x04190419)
	19:17:25.044(gui.4720) RealConsole: WM_INPUTLANGCHANGEREQUEST, CP:1, HKL:0x04190419 via CmdExecute
	19:17:27.044(gui.3072) GUI recieved CECMD_LANGCHANGE
	19:17:29.044(gui.4720) ConEmu: GetKeyboardLayout(0) in OnLangChangeConsole after GetKeyboardLayout(0) = 0x04190419
	19:17:31.044(gui.4720) ConEmu: GetKeyboardLayout(0) in OnLangChangeConsole after GetKeyboardLayout(0) = 0x04190419
	--> ������� ����� �����!
	'ConEmu.exe': Loaded 'C:\WINDOWS\system32\kbdru.dll'
	'ConEmu.exe': Unloaded 'C:\WINDOWS\system32\kbdru.dll'
	19:17:33.075(gui.4720) ConEmu: Calling GetKeyboardLayout(0)
	19:17:35.075(gui.4720) ConEmu: GetKeyboardLayout(0) after LoadKeyboardLayout = 0x04190419
	19:17:37.075(gui.4720) ConEmu: GetKeyboardLayout(0) after SwitchKeyboardLayout = 0x04190419
	**********
	Ru -> En : --> ������� ����� �����!
	**********
	17:23:36.013(gui.3152) ConEmu: WM_INPUTLANGCHANGEREQUEST(CP:1, HKL:0x04090409)
	17:23:36.013(gui.3152) ConEmu: WM_INPUTLANGCHANGE(CP:0, HKL:0x04090409)
	17:23:36.013(gui.3152) ConEmu: GetKeyboardLayout(0) after DefWindowProc(WM_INPUTLANGCHANGE) = 0x04090409)
	17:23:36.013(gui.3152) ConEmu: GetKeyboardLayout(0) after DefWindowProc(WM_INPUTLANGCHANGEREQUEST) = 0x04090409)
	17:23:36.013(gui.3152) CRealConsole::SwitchKeyboardLayout(CP:1, HKL:0x04090409)
	17:23:36.013(gui.3152) RealConsole: WM_INPUTLANGCHANGEREQUEST, CP:1, HKL:0x04090409 via CmdExecute
	ConEmuC: PostMessage(WM_INPUTLANGCHANGEREQUEST, CP:1, HKL:0x04090409)
	The thread 'Win32 Thread' (0x3f0) has exited with code 1 (0x1).
	ConEmuC: InputLayoutChanged (GetConsoleKeyboardLayoutName returns) '00000409'
	17:23:38.013(gui.4460) GUI recieved CECMD_LANGCHANGE
	17:23:40.028(gui.4460) ConEmu: GetKeyboardLayout(0) on CECMD_LANGCHANGE after GetKeyboardLayout(0) = 0x04090409
	--> ������� ����� �����!
	'ConEmu.exe': Loaded 'C:\WINDOWS\system32\kbdus.dll'
	'ConEmu.exe': Unloaded 'C:\WINDOWS\system32\kbdus.dll'
	17:23:42.044(gui.4460) ConEmu: Calling GetKeyboardLayout(0)
	17:23:44.044(gui.4460) ConEmu: GetKeyboardLayout(0) after LoadKeyboardLayout = 0x04090409
	17:23:46.044(gui.4460) ConEmu: GetKeyboardLayout(0) after SwitchKeyboardLayout = 0x04090409
	*/
	LRESULT result = 1;
#ifdef _DEBUG
	WCHAR szMsg[255];
	_wsprintf(szMsg, SKIPLEN(countof(szMsg)) L"ConEmu: %s(CP:%i, HKL:0x%08I64X)\n",
	          (messg == WM_INPUTLANGCHANGE) ? L"WM_INPUTLANGCHANGE" : L"WM_INPUTLANGCHANGEREQUEST",
	          (DWORD)wParam, (unsigned __int64)(DWORD_PTR)lParam);
	DEBUGSTRLANG(szMsg);
#endif

	if (gpSetCls->isAdvLogging > 1)
	{
		WCHAR szInfo[255];
		_wsprintf(szInfo, SKIPLEN(countof(szInfo)) L"CConEmuMain::OnLangChange: %s(CP:%i, HKL:0x%08I64X)",
		          (messg == WM_INPUTLANGCHANGE) ? L"WM_INPUTLANGCHANGE" : L"WM_INPUTLANGCHANGEREQUEST",
		          (DWORD)wParam, (unsigned __int64)(DWORD_PTR)lParam);
		LogString(szInfo);
	}

	/*
	wParam = 204, lParam = 0x04190419 - Russian
	wParam = 0,   lParam = 0x04090409 - US
	wParam = 0,   lParam = 0xfffffffff0020409 - US Dvorak
	wParam = 0,   lParam = 0xfffffffff01a0409 - US Dvorak left hand
	wParam = 0,   lParam = 0xfffffffff01b0409 - US Dvorak
	*/
	//POSTMESSAGE(ghConWnd, messg, wParam, lParam, FALSE);
	//mn_CurrentKeybLayout = lParam;
#ifdef _DEBUG
	wchar_t szBeforeChange[KL_NAMELENGTH] = {0}; GetKeyboardLayoutName(szBeforeChange);
#endif
	// ���������� �����
	result = DefWindowProc(ghWnd, messg, wParam, lParam);
	// ���������, ��� ����������
	wchar_t szAfterChange[KL_NAMELENGTH] = {0}; GetKeyboardLayoutName(szAfterChange);
	//if (wcscmp(szAfterChange, szBeforeChange)) -- �������� ����������, ��� ��� ���������.
	//{
	wchar_t *pszEnd = szAfterChange+8;
	DWORD dwLayoutAfterChange = wcstoul(szAfterChange, &pszEnd, 16);
	// ���������, ��� ���������� � m_LayoutNames
	int i, iUnused = -1;

	for(i = 0; i < (int)countof(m_LayoutNames); i++)
	{
		if (!m_LayoutNames[i].bUsed)
		{
			if (iUnused == -1) iUnused = i; continue;
		}

		if (m_LayoutNames[i].klName == dwLayoutAfterChange)
		{
			iUnused = -1; break;
		}
	}

	if (iUnused != -1)
	{
		m_LayoutNames[iUnused].bUsed = TRUE;
		m_LayoutNames[iUnused].klName = dwLayoutAfterChange;
		m_LayoutNames[iUnused].hkl = lParam;
	}

	//}
#ifdef _DEBUG
	HKL hkl = GetKeyboardLayout(0);
	_wsprintf(szMsg, SKIPLEN(countof(szMsg)) L"ConEmu: GetKeyboardLayout(0) after DefWindowProc(%s) = 0x%08I64X)\n",
	          (messg == WM_INPUTLANGCHANGE) ? L"WM_INPUTLANGCHANGE" : L"WM_INPUTLANGCHANGEREQUEST",
	          (unsigned __int64)(DWORD_PTR)hkl);
	DEBUGSTRLANG(szMsg);
#endif

	// � Win7x64 WM_INPUTLANGCHANGEREQUEST ������ �� ��������, �� ������� ���� ��� ������������ ������
	if (messg == WM_INPUTLANGCHANGEREQUEST || messg == WM_INPUTLANGCHANGE)
	{
		static UINT   nLastMsg;
		static LPARAM lLastPrm;

		CVConGuard VCon;
		CRealConsole* pRCon = (GetActiveVCon(&VCon) >= 0) ? VCon->RCon() : NULL;

		if (pRCon && !(nLastMsg == WM_INPUTLANGCHANGEREQUEST && messg == WM_INPUTLANGCHANGE && lLastPrm == lParam))
		{
			pRCon->SwitchKeyboardLayout(
			    (messg == WM_INPUTLANGCHANGEREQUEST) ? wParam : 0, lParam
			);
		}

		nLastMsg = messg;
		lLastPrm = lParam;

		// -- ������� �� �����
		//if (pRCon)
		//{
		//	HWND hGuiWnd = pRCon->GuiWnd();
		//	if (hGuiWnd)
		//		pRCon->PostConsoleMessage(hGuiWnd, messg, wParam, lParam);
		//}
	}

	m_ActiveKeybLayout = (DWORD_PTR)lParam;

	mp_Status->OnKeyboardChanged();

	//  if (isFar() && gpSet->isLangChangeWsPlugin)
	//  {
	//   //LONG lLastLang = GetWindowLong ( ghWnd DC, GWL_LANGCHANGE );
	//   //SetWindowLong ( ghWnd DC, GWL_LANGCHANGE, lParam );
	//
	//   /*if (lLastLang == lParam)
	//    return result;*/
	//
	//CConEmuPipe pipe(GetFarPID(), 10);
	//if (pipe.Init(_T("CConEmuMain::OnLangChange"), FALSE))
	//{
	//  if (pipe.Execute(CMD_LANGCHANGE, &lParam, sizeof(LPARAM)))
	//  {
	//      //gpConEmu->DebugStep(_T("ConEmu: Switching language (1 sec)"));
	//      // �������� ��������, �������� ��� ������ �����
	//      /*DWORD dwWait = WaitForSingleObject(pipe.hEventAlive, CONEMUALIVETIMEOUT);
	//      if (dwWait == WAIT_OBJECT_0)*/
	//          return result;
	//  }
	//}
	//  }
	//  //POSTMESSAGE(ghConWnd, messg, wParam, lParam, FALSE);
	//  //SENDMESSAGE(ghConWnd, messg, wParam, lParam);
	//  //if (messg == WM_INPUTLANGCHANGEREQUEST)
	//  {
	//      //wParam Specifies the character set of the new locale.
	//      //lParam - HKL
	//      //ActivateKeyboardLayout((HKL)lParam, 0);
	//      //POSTMESSAGE(ghConWnd, messg, wParam, lParam, FALSE);
	//      //SENDMESSAGE(ghConWnd, messg, wParam, lParam);
	//  }
	//  if (messg == WM_INPUTLANGCHANGE)
	//  {
	//      //SENDMESSAGE(ghConWnd, WM_SETFOCUS, 0,0);
	//      //POSTMESSAGE(ghConWnd, WM_SETFOCUS, 0,0, TRUE);
	//      //POSTMESSAGE(ghWnd, WM_SETFOCUS, 0,0, TRUE);
	//  }
	return result;
}

// dwLayoutName �������� �� HKL, � "���" (i.e. "00030409") ����������������� � DWORD
LRESULT CConEmuMain::OnLangChangeConsole(CVirtualConsole *apVCon, DWORD dwLayoutName)
{
	if ((gpSet->isMonitorConsoleLang & 1) != 1)
		return 0;

	if (!isValid(apVCon))
		return 0;

	if (!isMainThread())
	{
		if (gpSetCls->isAdvLogging > 1)
		{
			WCHAR szInfo[255];
			_wsprintf(szInfo, SKIPLEN(countof(szInfo)) L"CConEmuMain::OnLangChangeConsole (0x%08X), Posting to main thread", dwLayoutName);
			LogString(szInfo);
		}

		PostMessage(ghWnd, mn_ConsoleLangChanged, dwLayoutName, (LPARAM)apVCon);
		return 0;
	}
	else
	{
		if (gpSetCls->isAdvLogging > 1)
		{
			WCHAR szInfo[255];
			_wsprintf(szInfo, SKIPLEN(countof(szInfo)) L"CConEmuMain::OnLangChangeConsole (0x%08X), MainThread", dwLayoutName);
			LogString(szInfo);
		}
	}

#ifdef _DEBUG
	//Sleep(2000);
	WCHAR szMsg[255];
	// --> ������ ������ ��� �� "��������" �������. ����� ����������� Post'�� � �������� ����
	HKL hkl = GetKeyboardLayout(0);
	_wsprintf(szMsg, SKIPLEN(countof(szMsg)) L"ConEmu: GetKeyboardLayout(0) in OnLangChangeConsole after GetKeyboardLayout(0) = 0x%08I64X\n",
	          (unsigned __int64)(DWORD_PTR)hkl);
	DEBUGSTRLANG(szMsg);
	//Sleep(2000);
#endif
	wchar_t szName[10]; _wsprintf(szName, SKIPLEN(countof(szName)) L"%08X", dwLayoutName);
#ifdef _DEBUG
	DEBUGSTRLANG(szName);
#endif
	// --> ��� �������!
	DWORD_PTR dwNewKeybLayout = dwLayoutName; //(DWORD_PTR)LoadKeyboardLayout(szName, 0);
	HKL hKeyb[20]; UINT nCount, i;
	nCount = GetKeyboardLayoutList(countof(hKeyb), hKeyb);
	/*
	HKL:
	0x0000000004090409 - US
	0x0000000004190419 - Russian
	0xfffffffff0020409 - US - Dvorak
	0xfffffffff01a0409 - US - Dvorak left hand
	0xfffffffff01b0409 - US - Dvorak right hand
	Layout (dwLayoutName):
	0x00010409 - US - Dvorak
	0x00030409 - US - Dvorak left hand
	0x00040409 - US - Dvorak right hand
	*/
	BOOL lbFound = FALSE;
	int iUnused = -1;

	for(i = 0; !lbFound && i < countof(m_LayoutNames); i++)
	{
		if (!m_LayoutNames[i].bUsed)
		{
			if (iUnused == -1) iUnused = i;

			continue;
		}

		if (m_LayoutNames[i].klName == dwLayoutName)
		{
			lbFound = TRUE;
			dwNewKeybLayout = m_LayoutNames[i].hkl;
			iUnused = -1; // ���������� �� �����������
			break;
		}
	}

	// ���� �� �����, � ��� "������������" ���������, � ������� �� ��������� ��������� � ������
	if (!lbFound && ((dwLayoutName & 0xFFFF) == dwLayoutName))
	{
		DWORD_PTR dwTest = dwNewKeybLayout | (dwNewKeybLayout << 16);

		for(i = 0; !lbFound && i < nCount; i++)
		{
			if (((DWORD_PTR)hKeyb[i]) == dwTest)
			{
				lbFound = TRUE;
				dwNewKeybLayout = (DWORD_PTR)hKeyb[i];
				break;
			}
		}
	}

	if (!lbFound)
	{
		wchar_t szLayoutName[9] = {0};
		_wsprintf(szLayoutName, SKIPLEN(countof(szLayoutName)) L"%08X", dwLayoutName);

		if (gpSetCls->isAdvLogging > 1)
		{
			WCHAR szInfo[255];
			_wsprintf(szInfo, SKIPLEN(countof(szInfo)) L"CConEmuMain::OnLangChangeConsole -> LoadKeyboardLayout(0x%08X)", dwLayoutName);
			LogString(szInfo);
		}

		dwNewKeybLayout = (DWORD_PTR)LoadKeyboardLayout(szLayoutName, 0);

		if (gpSetCls->isAdvLogging > 1)
		{
			WCHAR szInfo[255];
			_wsprintf(szInfo, SKIPLEN(countof(szInfo)) L"CConEmuMain::OnLangChangeConsole -> LoadKeyboardLayout()=0x%08X", (DWORD)dwNewKeybLayout);
			LogString(szInfo);
		}

		lbFound = TRUE;
	}

	if (lbFound && iUnused != -1)
	{
		m_LayoutNames[iUnused].bUsed = TRUE;
		m_LayoutNames[iUnused].klName = dwLayoutName;
		m_LayoutNames[iUnused].hkl = dwNewKeybLayout;
	}

	//dwNewKeybLayout = (DWORD_PTR)hklNew;
	//for (i = 0; !lbFound && i < nCount; i++)
	//{
	//	if (hKeyb[i] == (HKL)dwNewKeybLayout)
	//		lbFound = TRUE;
	//}
	//WARNING("������ � ������� ����������� ����� �������. US Dvorak?");
	//for (i = 0; !lbFound && i < nCount; i++)
	//{
	//	if ((((DWORD_PTR)hKeyb[i]) & 0xFFFF) == (dwNewKeybLayout & 0xFFFF))
	//	{
	//		lbFound = TRUE; dwNewKeybLayout = (DWORD_PTR)hKeyb[i];
	//	}
	//}
	//// ���� �� ������ ��������� (������ ����?) ��������� �� ���������
	//if (!lbFound && (dwNewKeybLayout == (dwNewKeybLayout & 0xFFFF)))
	//{
	//	dwNewKeybLayout |= (dwNewKeybLayout << 16);
	//}
#ifdef _DEBUG
	DEBUGSTRLANG(L"ConEmu: Calling GetKeyboardLayout(0)\n");
	//Sleep(2000);
	hkl = GetKeyboardLayout(0);
	_wsprintf(szMsg, SKIPLEN(countof(szMsg)) L"ConEmu: GetKeyboardLayout(0) after LoadKeyboardLayout = 0x%08I64X\n",
	          (unsigned __int64)(DWORD_PTR)hkl);
	DEBUGSTRLANG(szMsg);
	//Sleep(2000);
#endif

	if (isActive(apVCon))
	{
		apVCon->RCon()->OnConsoleLangChange(dwNewKeybLayout);
	}

	return 0;
}

bool CConEmuMain::PatchMouseEvent(UINT messg, POINT& ptCurClient, POINT& ptCurScreen, WPARAM wParam, bool& isPrivate)
{
	isPrivate = false;

	// ��� ���� ���������, lParam - relative to the upper-left corner of the screen.
	if (messg == WM_MOUSEWHEEL || messg == WM_MOUSEHWHEEL)
		ScreenToClient(ghWnd, &ptCurClient);
	else // ��� ��������� lParam �������� ���������� ����������
		ClientToScreen(ghWnd, &ptCurScreen);

	if (mb_MouseCaptured || (messg == WM_LBUTTONDOWN))
	{
		HWND hChild = ::ChildWindowFromPointEx(ghWnd, ptCurClient, CWP_SKIPINVISIBLE|CWP_SKIPDISABLED|CWP_SKIPTRANSPARENT);
		CVConGuard VCon;
		if (isVConHWND(hChild, &VCon)/*(hChild != ghWnd)*/) // ��� ������ ���� VCon
		{
			#ifdef _DEBUG
			wchar_t szClass[128]; GetClassName(hChild, szClass, countof(szClass));
			_ASSERTE(lstrcmp(szClass, VirtualConsoleClass)==0 && "This must be VCon DC window");
			#endif

			WARNING("��� ������, ��� ����� ���������� ������!");
			if (VCon.VCon() && isVisible(VCon.VCon()) && !isActive(VCon.VCon(), false))
			{
				Activate(VCon.VCon());
			}

			// ���� ������� PanelView - ��� ����� ������������� ����������
			POINT ptVConCoord = ptCurClient;
			::MapWindowPoints(ghWnd, hChild, &ptVConCoord, 1);
			HWND hPanelView = ::ChildWindowFromPointEx(hChild, ptVConCoord, CWP_SKIPINVISIBLE|CWP_SKIPDISABLED|CWP_SKIPTRANSPARENT);
			if (hPanelView && (hPanelView != hChild))
			{
				// LClick ���������� � ������� ������, ����� �� ��������� ������ � ����������������� �� �����...
				isPrivate = (messg == WM_LBUTTONDOWN);

				if (mb_MouseCaptured)
				{
					#ifdef _DEBUG
					GetClassName(hPanelView, szClass, countof(szClass));
					_ASSERTE(lstrcmp(szClass, ConEmuPanelViewClass)==0 && "This must be Thumbnail or Tile window");
					#endif

					::MapWindowPoints(hChild, hPanelView, &ptVConCoord, 1);

					DWORD_PTR lRc = (DWORD_PTR)-1;
					if (SendMessageTimeout(hPanelView, mn_MsgPanelViewMapCoord, MAKELPARAM(ptVConCoord.x,ptVConCoord.y), 0, SMTO_NORMAL, PNLVIEWMAPCOORD_TIMEOUT, &lRc) && (lRc != (DWORD_PTR)-1))
					{
						ptCurClient.x = LOWORD(lRc);
						ptCurClient.y = HIWORD(lRc);
						ptCurScreen = ptCurClient;
						ClientToScreen(ghWnd, &ptCurScreen);
					}
				}
			}
		}
	}

	if (messg == WM_LBUTTONDBLCLK)
	{
		mouse.LDblClkDC = ptCurClient;
		mouse.LDblClkTick = TimeGetTime();
	}
	else if ((mouse.lastMsg == WM_LBUTTONDBLCLK) && ((messg == WM_MOUSEMOVE) || (messg == WM_LBUTTONUP)))
	{
		// ������� � ���������.
		// ��� ������� ���� ����� ���������� ��������� �����:
		//17:40:25.787(gui.4460) GUI::Mouse WM_ MOUSEMOVE at screen {603x239} x00000000
		//17:40:25.787(gui.4460) GUI::Mouse WM_ LBUTTONDOWN at screen {603x239} x00000001
		//17:40:25.787(gui.4460) GUI::Mouse WM_ LBUTTONUP at screen {603x239} x00000000
		//17:40:25.787(gui.4460) GUI::Mouse WM_ MOUSEMOVE at screen {603x239} x00000000
		//17:40:25.880(gui.4460) GUI::Mouse WM_ LBUTTONDBLCLK at screen {603x239} x00000001
		//17:40:25.880(gui.4460) GUI::Mouse WM_ MOUSEMOVE at screen {598x249} x00000001
		//17:40:25.927(gui.4460) GUI::Mouse WM_ LBUTTONUP at screen {598x249} x00000000
		//17:40:25.927(gui.4460) GUI::Mouse WM_ MOUSEMOVE at screen {598x249} x00000000
		// �.�. ����� ������� ���� ������� ������� (95% �������)
		// � �����, ����� ��������� MOUSEMOVE & LBUTTONUP ��� ����� ��������, ��� ������������,
		// �.�. Far, ��������, ����� �������� ���� �� ����� ��������� ����������������
		// �� ���� � ���� �����. ��� ���� �� ����� DblClick ��� ��� ���� ������ Click.
		DWORD dwDelta = TimeGetTime() - mouse.LDblClkTick;
		if (dwDelta <= TOUCH_DBLCLICK_DELTA)
		{
			if (messg == WM_MOUSEMOVE)
			{
				if ((wParam & (MK_LBUTTON|MK_RBUTTON|MK_MBUTTON)) == MK_LBUTTON)
				{
					return false; // �� ���������� ��� ������� � �������
				}
			}
			else
			{
				_ASSERTE(messg==WM_LBUTTONUP);
				// � ��� �� ������������� �������, ����� ������� ������, ��� "������ �� ��������"
				ptCurClient = mouse.LDblClkDC;
				ptCurScreen = ptCurClient;
				//MapWindowPoints(
				ClientToScreen(ghWnd, &ptCurScreen);
			}
		}
	}

	return true;
}

LRESULT CConEmuMain::OnMouse(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	// ��� ��� �����, � ����� ����������� ��� ������...
	//short winX = GET_X_LPARAM(lParam);
	//short winY = GET_Y_LPARAM(lParam);

	if (gpSetCls->isAdvLogging)
		CVConGroup::LogInput(messg, wParam, lParam);

	if (mn_TrackMenuPlace != tmp_None)
	{
		if (mp_Tip)
		{
			mp_Tip->HideTip();
		}

		if (gpSetCls->isAdvLogging)
			LogString("Mouse event skipped due to (mn_TrackMenuPlace != tmp_None)");

		return 0;
	}

	//2010-05-20 ���-���� ����� ��������������� �� lParam, ������ ���
	//  ������ ��� ConEmuTh ����� �������� ���������� ����������
	//POINT ptCur = {-1, -1}; GetCursorPos(&ptCur);
	POINT ptCurClient = {(int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)};
	POINT ptCurScreen = ptCurClient;

	// ��������� ��������� ��� ������� ���������
	bool isPrivate = false;
	bool bContinue = PatchMouseEvent(messg, ptCurClient, ptCurScreen, wParam, isPrivate);

#ifdef _DEBUG
	wchar_t szDbg[128];
	_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"GUI::Mouse %s at screen {%ix%i} x%08X%s\n",
		(messg==WM_MOUSEMOVE) ? L"WM_MOUSEMOVE" :
		(messg==WM_LBUTTONDOWN) ? L"WM_LBUTTONDOWN" :
		(messg==WM_LBUTTONUP) ? L"WM_LBUTTONUP" :
		(messg==WM_LBUTTONDBLCLK) ? L"WM_LBUTTONDBLCLK" :
		(messg==WM_RBUTTONDOWN) ? L"WM_RBUTTONDOWN" :
		(messg==WM_RBUTTONUP) ? L"WM_RBUTTONUP" :
		(messg==WM_RBUTTONDBLCLK) ? L"WM_RBUTTONDBLCLK" :
		(messg==WM_MBUTTONDOWN) ? L"WM_MBUTTONDOWN" :
		(messg==WM_MBUTTONUP) ? L"WM_MBUTTONUP" :
		(messg==WM_MBUTTONDBLCLK) ? L"WM_MBUTTONDBLCLK" :
		(messg==0x020A) ? L"WM_MOUSEWHEEL" :
		(messg==0x020B) ? L"WM_XBUTTONDOWN" :
		(messg==0x020C) ? L"WM_XBUTTONUP" :
		(messg==0x020D) ? L"WM_XBUTTONDBLCLK" :
		(messg==0x020E) ? L"WM_MOUSEHWHEEL" :
		L"UnknownMsg",
		ptCurScreen.x,ptCurScreen.y,(DWORD)wParam,
		bContinue ? L"" : L" - SKIPPED!");
	DEBUGSTRMOUSE(szDbg);
#endif

	//gpSet->IsModifierPressed(vkWndDragKey, false))
	if ((messg == WM_LBUTTONDOWN)
		// 120820 - ������� ����� ������ � "Normal"
		&& !(isZoomed() || isFullScreen() || gpSet->isQuakeStyle)
		&& ProcessHotKeyMsg(WM_KEYDOWN, VK_LBUTTON, 0, NULL, NULL))
	{
		mouse.state |= MOUSE_WINDOW_DRAG;
		GetCursorPos(&mouse.ptWndDragStart);
		GetWindowRect(ghWnd, &mouse.rcWndDragStart);
		SetCapture(ghWnd);
		SetCursor(LoadCursor(NULL,IDC_SIZEALL));
		return 0;
	}
	else if (mouse.state & MOUSE_WINDOW_DRAG)
	{
		if ((messg == WM_LBUTTONUP) || !isPressed(VK_LBUTTON))
		{
			mouse.state &= ~MOUSE_WINDOW_DRAG;
			ReleaseCapture();
		}
		else
		{
			POINT ptCurPos = {};
			GetCursorPos(&ptCurPos);
			RECT rcNew = {
				mouse.rcWndDragStart.left  + (ptCurPos.x - mouse.ptWndDragStart.x),
				mouse.rcWndDragStart.top   + (ptCurPos.y - mouse.ptWndDragStart.y)
			};
			rcNew.right = rcNew.left + (mouse.rcWndDragStart.right - mouse.rcWndDragStart.left);
			rcNew.bottom = rcNew.top + (mouse.rcWndDragStart.bottom - mouse.rcWndDragStart.top);

			OnMoving(&rcNew, true);

			TODO("Desktop mode?");
			MoveWindow(ghWnd, rcNew.left, rcNew.top, rcNew.right-rcNew.left, rcNew.bottom-rcNew.top, TRUE);
			//SetWindowPos(ghWnd, NULL, rcNew.left, rcNew.top, 0,0, SWP_NOSIZE|SWP_NOZORDER);
		}
		return 0;
	}


	// ���������� ����� �� StatusBar
	LRESULT lRc = 0;
	if (mp_Status->ProcessStatusMessage(hWnd, messg, wParam, lParam, ptCurClient, lRc))
		return lRc;

	if (!bContinue)
		return 0;

	if (m_InsideIntegration && ((messg == WM_LBUTTONDOWN) || (messg == WM_RBUTTONDOWN)))
	{
		#ifdef _DEBUG
		HWND hFocus = GetFocus();
		HWND hFore = GetForegroundWindow();
		#endif
		SetForegroundWindow(ghWnd);
		SetFocus(ghWnd);
	}


	TODO("DoubleView. ������ �� �������� ����� �������������� � ������� ��� ������� �������, � �� � ��������");
	RECT /*conRect = {0},*/ dcRect = {0};
	//GetWindowRect('ghWnd DC', &dcRect);
	CVirtualConsole* pVCon = NULL;
	CVConGuard VCon;

	if ((messg == WM_LBUTTONUP) && (GetActiveVCon(&VCon) >= 0) && VCon->RCon()->isMouseSelectionPresent())
		pVCon = VCon.VCon();
	else if (CVConGroup::GetVConFromPoint(ptCurScreen, &VCon))
		pVCon = VCon.VCon();
	CRealConsole *pRCon = pVCon ? pVCon->RCon() : NULL;
	HWND hView = pVCon ? pVCon->GetView() : NULL;
	if (hView)
	{
		GetWindowRect(hView, &dcRect);
		MapWindowPoints(NULL, ghWnd, (LPPOINT)&dcRect, 2);
	}

	HWND hChild = ::ChildWindowFromPointEx(ghWnd, ptCurClient, CWP_SKIPINVISIBLE|CWP_SKIPDISABLED|CWP_SKIPTRANSPARENT);


	//enum DragPanelBorder dpb = DPB_NONE; //CConEmuMain::CheckPanelDrag(COORD crCon)
	if (((messg == WM_MOUSEWHEEL) || (messg == 0x020E/*WM_MOUSEHWHEEL*/)) && HIWORD(wParam))
	{
		BYTE vk = 0;
		if (messg == WM_MOUSEWHEEL)
			vk =  (((short)(WORD)HIWORD(wParam)) > 0) ? VK_WHEEL_UP : VK_WHEEL_DOWN;
		else if (messg == 0x020E/*WM_MOUSEHWHEEL*/)
			vk =  (((short)(WORD)HIWORD(wParam)) > 0) ? VK_WHEEL_RIGHT : VK_WHEEL_LEFT; // ���� MSDN �� ���� - ��������� �� �� ���

		// ����� "�����������" ��������� ������� (���� ���������)
		if (vk && ProcessHotKeyMsg(WM_KEYDOWN, vk, 0, NULL, pRCon))
		{
			// ���������, � ������� �� ����������
			return 0;
		}
	}
	else if (messg >= WM_LBUTTONDOWN && messg <= WM_MBUTTONDBLCLK)
	{
		BYTE vk;
		if (messg >= WM_LBUTTONDOWN && messg <= WM_LBUTTONDBLCLK)
			vk = VK_LBUTTON;
		else if (messg >= WM_RBUTTONDOWN && messg <= WM_RBUTTONDBLCLK)
			vk = VK_RBUTTON;
		else
			vk = VK_MBUTTON;

		UpdateControlKeyState();

		bool bDn = (messg == WM_LBUTTONDOWN || messg == WM_RBUTTONDOWN || messg == WM_MBUTTONDOWN);
		bool bUp = (messg == WM_LBUTTONUP || messg == WM_RBUTTONUP || messg == WM_MBUTTONUP);

		const ConEmuHotKey* pHotKey = gpSetCls->GetHotKeyInfo(VkModFromVk(vk), bDn, pRCon);

		if (pHotKey)
		{
			if ((pHotKey != ConEmuSkipHotKey) && (bDn || bUp))
			{
				// ����� "�����������" ��������� �������
				ProcessHotKeyMsg(bDn ? WM_KEYDOWN : WM_KEYUP, vk, 0, NULL, pRCon);
			}

			// ���������, � ������� �� ����������
			return 0;
		}
	}

	//BOOL lbMouseWasCaptured = mb_MouseCaptured;
	if (!mb_MouseCaptured)
	{
		// ���� ����
		if ((hView && hChild == hView) &&
			(messg == WM_LBUTTONDOWN || messg == WM_RBUTTONDOWN || messg == WM_MBUTTONDOWN
			|| (isPressed(VK_LBUTTON) || isPressed(VK_RBUTTON) || isPressed(VK_MBUTTON))))
		{
			// � ���������� ������� (������� ���������)
			//if (hView /*PtInRect(&dcRect, ptCurClient)*/)
			//{
			mb_MouseCaptured = TRUE;
			//TODO("����� ������� ViewPort �������� SetCapture ����� ����� ������ �� ghWnd");
			SetCapture(ghWnd); // 100208 ���� 'ghWnd DC'
			//}
		}
	}
	else
	{
		// ��� ������ ����� �������� - release
		if (!isPressed(VK_LBUTTON) && !isPressed(VK_RBUTTON) && !isPressed(VK_MBUTTON))
		{
			ReleaseCapture();
			mb_MouseCaptured = FALSE;

			if (pRCon && pRCon->isMouseButtonDown())
			{
				if (ptCurClient.x < dcRect.left)
					ptCurClient.x = dcRect.left;
				else if (ptCurClient.x >= dcRect.right)
					ptCurClient.x = dcRect.right-1;

				if (ptCurClient.y < dcRect.top)
					ptCurClient.y = dcRect.top;
				else if (ptCurClient.y >= dcRect.bottom)
					ptCurClient.y = dcRect.bottom-1;
			}
		}
	}

	if (!pVCon)
		return 0;

	if (gpSetCls->FontWidth()==0 || gpSetCls->FontHeight()==0)
		return 0;

	if ((messg==WM_LBUTTONUP || messg==WM_MOUSEMOVE) && (gpConEmu->mouse.state & MOUSE_SIZING_DBLCKL))
	{
		if (messg==WM_LBUTTONUP)
			gpConEmu->mouse.state &= ~MOUSE_SIZING_DBLCKL;

		return 0; //2009-04-22 ����� DblCkl �� ��������� � ������� ��� ������������� LBUTTONUP
	}

	if (messg == WM_MOUSEMOVE)
	{
		if (TrackMouse())
			return 0;
	}

	if (pRCon && pRCon->GuiWnd() && !pRCon->isBufferHeight())
	{
		// ��� ��������� ����� �������� ����������, ����� �� �������
		if ((messg == WM_MOUSEWHEEL) || (messg == WM_MOUSEHWHEEL))
		{
			pRCon->PostConsoleMessage(pRCon->GuiWnd(), messg, wParam, lParam);
		}
		return 0;
	}

	if (pRCon && pRCon->isSelectionPresent()
	        && ((wParam & MK_LBUTTON) || messg == WM_LBUTTONUP))
	{
		ptCurClient.x -= dcRect.left; ptCurClient.y -= dcRect.top;
		pRCon->OnMouse(messg, wParam, ptCurClient.x, ptCurClient.y);
		return 0;
	}

	// ����� � ������� ������������� ������ �� ���������� ���� ����...
	// ���, ��� ����� - ������� ������� � ��� ������ �������� � dcRect
	if (messg != WM_MOUSEMOVE && messg != WM_MOUSEWHEEL && messg != WM_MOUSEHWHEEL)
	{
		if (gpConEmu->mouse.state & MOUSE_SIZING_DBLCKL)
			gpConEmu->mouse.state &= ~MOUSE_SIZING_DBLCKL;

		if (hView == NULL /*!PtInRect(&dcRect, ptCur)*/)
		{
			DEBUGLOGFILE("Click outside of DC");
			return 0;
		}
	}

	// ��������� � ���������� (������������ hView) ����������
	ptCurClient.x -= dcRect.left; ptCurClient.y -= dcRect.top;
	//WARNING("����������� �� ghConWnd. ������ ���� ��������� ����� ������");
	//::GetClientRect(ghConWnd, &conRect);
	COORD cr = pVCon->ClientToConsole(ptCurClient.x,ptCurClient.y);
	short conX = cr.X; //winX/gpSet->Log Font.lfWidth;
	short conY = cr.Y; //winY/gpSet->Log Font.lfHeight;

	if ((messg != WM_MOUSEWHEEL && messg != WM_MOUSEHWHEEL) && (conX<0 || conY<0))
	{
		DEBUGLOGFILE("Mouse outside of upper-left");
		return 0;
	}

	//* ****************************
	//* ���� ���� ConEmu �� � ������ - �� ����� � ������� �������� �����,
	//* ����� ���������� ���������� ������� ������� ���� � �.�.
	//* ****************************
	if (gpSet->isMouseSkipMoving && !isMeForeground(false,false))
	{
		DEBUGLOGFILE("ConEmu is not foreground window, mouse event skipped");
		return 0;
	}

	//* ****************************
	//* ����� ��������� WM_MOUSEACTIVATE ����������� ����� �� ����������
	//* ���� � �������, ����� ��� ��������� ConEmu �������� �� ������
	//* (������� ��� ��������) ������� � FAR ������
	//* ****************************
	if ((gpConEmu->mouse.nSkipEvents[0] && gpConEmu->mouse.nSkipEvents[0] == messg)
	        || (gpConEmu->mouse.nSkipEvents[1]
	            && (gpConEmu->mouse.nSkipEvents[1] == messg || messg == WM_MOUSEMOVE)))
	{
		if (gpConEmu->mouse.nSkipEvents[0] == messg)
		{
			gpConEmu->mouse.nSkipEvents[0] = 0;
			DEBUGSTRMOUSE(L"Skipping Mouse down\n");
		}
		else if (gpConEmu->mouse.nSkipEvents[1] == messg)
		{
			gpConEmu->mouse.nSkipEvents[1] = 0;
			DEBUGSTRMOUSE(L"Skipping Mouse up\n");
		}

#ifdef _DEBUG
		else if (messg == WM_MOUSEMOVE)
		{
			DEBUGSTRMOUSE(L"Skipping Mouse move\n");
		}

#endif
		DEBUGLOGFILE("ConEmu was not foreground window, mouse activation event skipped");
		return 0;
	}

	//* ****************************
	//* ����� ��������� WM_MOUSEACTIVATE � ���������� gpSet->isMouseSkipActivation
	//* ������� ���� ��������� � ������� ��� ���������, ����� ����� ���������
	//* ������ ������� ���� � ��� �� ����� ����� ��������� �����������
	//* ****************************
	if (mouse.nReplaceDblClk)
	{
		if (!gpSet->isMouseSkipActivation)
		{
			mouse.nReplaceDblClk = 0;
		}
		else
		{
			if (messg == WM_LBUTTONDOWN && mouse.nReplaceDblClk == WM_LBUTTONDBLCLK)
			{
				mouse.nReplaceDblClk = 0;
			}
			else if (messg == WM_RBUTTONDOWN && mouse.nReplaceDblClk == WM_RBUTTONDBLCLK)
			{
				mouse.nReplaceDblClk = 0;
			}
			else if (messg == WM_MBUTTONDOWN && mouse.nReplaceDblClk == WM_MBUTTONDBLCLK)
			{
				mouse.nReplaceDblClk = 0;
			}
			else if (mouse.nReplaceDblClk == messg)
			{
				switch(mouse.nReplaceDblClk)
				{
					case WM_LBUTTONDBLCLK:
						messg = WM_LBUTTONDOWN;
						break;
					case WM_RBUTTONDBLCLK:
						messg = WM_RBUTTONDOWN;
						break;
					case WM_MBUTTONDBLCLK:
						messg = WM_MBUTTONDOWN;
						break;
				}

				mouse.nReplaceDblClk = 0;
			}
		}
	}

	// Forwarding ��������� � ���� �����
	if (mp_DragDrop && mp_DragDrop->IsDragStarting())
	{
		if (mp_DragDrop->ForwardMessage(messg, wParam, lParam))
			return 0;
	}

	// -- ��������� ��������� � ��� ������ ���� - ����� ������ �� ���������. ������ �������� ����
	// -- �������� side-effects �� ��������� ��������. Issue 274: ���� �������� ������� ��������������� � ��������� �����
	/////*&& isPressed(VK_LBUTTON)*/) && // ���� ����� �� ������ - ��� ��������� ������ ������� ����� ��������������� ������������
	//// ������ ��� ������� �������. ����� �� ConEmu ������ ������ �� �����
	//if (pRCon && (/*pRCon->isBufferHeight() ||*/ !pRCon->isWindowVisible())
	//    && (messg == WM_LBUTTONDOWN || messg == WM_RBUTTONDOWN || messg == WM_LBUTTONDBLCLK || messg == WM_RBUTTONDBLCLK
	//        || (messg == WM_MOUSEMOVE && isPressed(VK_LBUTTON)))
	//    )
	//{
	//   // buffer mode: cheat the console window: adjust its position exactly to the cursor
	//   RECT win;
	//   GetWindowRect(ghWnd, &win);
	//   short x = win.left + ptCurClient.x - MulDiv(ptCurClient.x, conRect.right, klMax<uint>(1, pVCon->Width));
	//   short y = win.top + ptCurClient.y - MulDiv(ptCurClient.y, conRect.bottom, klMax<uint>(1, pVCon->Height));
	//   RECT con;
	//   GetWindowRect(ghConWnd, &con);
	//   if (con.left != x || con.top != y)
	//       MOVEWINDOW(ghConWnd, x, y, con.right - con.left + 1, con.bottom - con.top + 1, TRUE);
	//}

	if (!isFar())
	{
		if (messg != WM_MOUSEMOVE) { DEBUGLOGFILE("FAR not active, all clicks forced to console"); }

		goto fin;
	}

	// ��� ���� ���������, lParam - relative to the upper-left corner of the screen.
	if (messg == WM_MOUSEWHEEL || messg == WM_MOUSEHWHEEL)
		lParam = MAKELONG((short)ptCurScreen.x, (short)ptCurScreen.y);
	else
		lParam = MAKELONG((short)ptCurClient.x, (short)ptCurClient.y);

	// �������� ��������� ������� ������� (��� PatchMouseEvent)
	mouse.lastMsg = messg;

	// ������ ����� ������������ �����, � ���� ����� - ����� �� � �������
	if (messg == WM_MOUSEMOVE)
	{
		// ptCurClient ��� ������������ � ���������� (������������ hView) ����������
		if (!OnMouse_Move(pVCon, hWnd, WM_MOUSEMOVE, wParam, lParam, ptCurClient, cr))
			return 0;
	}
	else
	{
		mouse.lastMMW=-1; mouse.lastMML=-1;

		if (messg == WM_LBUTTONDBLCLK)
		{
			// ptCurClient ��� ������������ � ���������� (������������ hView) ����������
			if (!OnMouse_LBtnDblClk(pVCon, hWnd, messg, wParam, lParam, ptCurClient, cr))
				return 0;
		} // !!! ��� else, �.�. ������������ ������� ����� �������� ���� �� ���������

		if (messg == WM_RBUTTONDBLCLK)
		{
			// ptCurClient ��� ������������ � ���������� (������������ hView) ����������
			if (!OnMouse_RBtnDblClk(pVCon, hWnd, messg, wParam, lParam, ptCurClient, cr))
				return 0;
		} // !!! ��� else, �.�. ������� ����� �������� ���� �� ���������

		// ������ ������������ ��� ��������
		if (messg == WM_LBUTTONDOWN)
		{
			// ptCurClient ��� ������������ � ���������� (������������ hView) ����������
			if (!OnMouse_LBtnDown(pVCon, hWnd, WM_LBUTTONDOWN, wParam, lParam, ptCurClient, cr))
				return 0;
		}
		else if (messg == WM_LBUTTONUP)
		{
			// ptCurClient ��� ������������ � ���������� (������������ hView) ����������
			if (!OnMouse_LBtnUp(pVCon, hWnd, WM_LBUTTONUP, wParam, lParam, ptCurClient, cr))
				return 0;
		}
		else if (messg == WM_RBUTTONDOWN)
		{
			// ptCurClient ��� ������������ � ���������� (������������ hView) ����������
			if (!OnMouse_RBtnDown(pVCon, hWnd, WM_RBUTTONDOWN, wParam, lParam, ptCurClient, cr))
				return 0;
		}
		else if (messg == WM_RBUTTONUP)
		{
			// ptCurClient ��� ������������ � ���������� (������������ hView) ����������
			if (!OnMouse_RBtnUp(pVCon, hWnd, WM_RBUTTONUP, wParam, lParam, ptCurClient, cr))
				return 0;
		}
	}

#ifdef MSGLOGGER

	if (messg == WM_MOUSEMOVE)
		messg = WM_MOUSEMOVE;

#endif
fin:
	// ���� �� �������� ���� �� ���� �� ������� (OnMouse_xxx) �� ��������� ��������� � �������
	// ����� ����� (��������) RBtnDown � ������� ����� ����� ����������� MOUSEMOVE
	// ptCurClient ��� ������������ � ���������� (������������ hView) ����������
	mouse.lastMMW=wParam; mouse.lastMML=MAKELPARAM(ptCurClient.x, ptCurClient.y);
	
	// LClick �� PanelViews ���������� � ������� ������, ����� �� ��������� ������ � ����������������� �� �����...
	if (!isPrivate)
	{
		// ������ �������� ������� ������� � �������
		if (pRCon)
			pRCon->OnMouse(messg, wParam, ptCurClient.x, ptCurClient.y);
	}

	return 0;
}

LRESULT CConEmuMain::OnMouse_Move(CVirtualConsole* pVCon, HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam, POINT ptCur, COORD cr)
{
	// WM_MOUSEMOVE ����� �� ������� ����� ���������� ���� ��� ������� ��� ������ �� ���������...
	if (wParam==mouse.lastMMW && lParam==mouse.lastMML
	        && (mouse.state & MOUSE_DRAGPANEL_ALL) == 0)
	{
		DEBUGSTRPANEL2(L"PanelDrag: Skip of wParam==mouse.lastMMW && lParam==mouse.lastMML\n");
		return 0;
	}

	mouse.lastMMW=wParam; mouse.lastMML=lParam;

	gpSetCls->UpdateFindDlgAlpha();

	// ��� �� ����������, ��������
	isSizing();

	// 18.03.2009 Maks - ���� ��� ����� - ���� �� �����
	if (isDragging())
	{
		// ����� ������ �������� �������?
		if ((mouse.state & DRAG_L_STARTED) && ((wParam & MK_LBUTTON)==0))
			mouse.state &= ~(DRAG_L_STARTED | DRAG_L_ALLOWED);

		if ((mouse.state & DRAG_R_STARTED) && ((wParam & MK_RBUTTON)==0))
			mouse.state &= ~(DRAG_R_STARTED | DRAG_R_ALLOWED);

		// �������� �������?
		mouse.state &= ~MOUSE_DRAGPANEL_ALL;

		if (mouse.state & (DRAG_L_STARTED | DRAG_R_STARTED))
		{
			DEBUGSTRPANEL2(L"PanelDrag: Skip of isDragging\n");
			return 0;
		}
	}
	else if ((mouse.state & (DRAG_L_ALLOWED | DRAG_R_ALLOWED)) != 0)
	{
		if (isConSelectMode())
		{
			mouse.state &= ~(DRAG_L_STARTED | DRAG_L_ALLOWED | DRAG_R_STARTED | DRAG_R_ALLOWED | MOUSE_DRAGPANEL_ALL);
		}
	}

	if (mouse.state & MOUSE_DRAGPANEL_ALL)
	{
		if (!gpSet->isDragPanel)
		{
			mouse.state &= ~MOUSE_DRAGPANEL_ALL;
			DEBUGSTRPANEL2(L"PanelDrag: Skip of isDragPanel==0\n");
		}
		else
		{
			if (!isPressed(VK_LBUTTON))
			{
				mouse.state &= ~MOUSE_DRAGPANEL_ALL;
				DEBUGSTRPANEL2(L"PanelDrag: Skip of LButton not pressed\n");
				return 0;
			}

			if (cr.X == mouse.LClkCon.X && cr.Y == mouse.LClkCon.Y)
			{
				DEBUGSTRPANEL2(L"PanelDrag: Skip of cr.X == mouse.LClkCon.X && cr.Y == mouse.LClkCon.Y\n");
				return 0;
			}

			if (gpSet->isDragPanel == 1)
				OnSizePanels(cr);

			// ���� �� �����...
			return 0;
		}
	}

	//TODO: ����� �� ������ isSizing() �� ������������?
	//? ����� ���: if (gpSet->isDragEnabled & mouse.state)
	if (gpSet->isDragEnabled & ((mouse.state & (DRAG_L_ALLOWED|DRAG_R_ALLOWED))!=0)
	        && !isPictureView())
	{
		if (mp_DragDrop==NULL)
		{
			DebugStep(_T("DnD: Drag-n-Drop is null"));
		}
		else
		{
			mouse.bIgnoreMouseMove = true;
			BOOL lbDiffTest = PTDIFFTEST(cursor,DRAG_DELTA); // TRUE - ���� ������ �� �������� (������)

			if (!lbDiffTest && !isDragging() && !isSizing())
			{
				// 2009-06-19 ����������� ����� ����� "if (gpSet->isDragEnabled & (mouse.state & (DRAG_L_ALLOWED|DRAG_R_ALLOWED)))"
				if (mouse.state & MOUSE_R_LOCKED)
				{
					// ����� ��� RightUp �� ���� APPS
					mouse.state &= ~MOUSE_R_LOCKED;
					//PtDiffTest(C, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), D)
					char szLog[255];
					wsprintfA(szLog, "Right drag started, MOUSE_R_LOCKED cleared: cursor={%i-%i}, Rcursor={%i-%i}, Now={%i-%i}, MinDelta=%i",
					          (int)cursor.x, (int)cursor.y,
					          (int)Rcursor.x, (int)Rcursor.y,
					          (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam),
					          (int)DRAG_DELTA);
					LogString(szLog);
				}

				BOOL lbLeftDrag = (mouse.state & DRAG_L_ALLOWED) == DRAG_L_ALLOWED;
				LogString(lbLeftDrag ? "Left drag about to start" : "Right drag about to start");
				// ���� ������� ����� ��� �� �������� ������, �� ����� LClick �� ����� �� �� �������� - �������� ShellDrag
				bool bFilePanel = isFilePanel();

				if (!bFilePanel)
				{
					DebugStep(_T("DnD: not file panel"));
					//isLBDown = false;
					mouse.state &= ~(DRAG_L_ALLOWED | DRAG_L_STARTED | DRAG_R_ALLOWED | DRAG_R_STARTED);
					mouse.bIgnoreMouseMove = false;
					//POSTMESSAGE(ghConWnd, WM_LBUTTONUP, wParam, MAKELPARAM( newX, newY ), TRUE);     //�������� ������� ����������
					pVCon->RCon()->OnMouse(WM_LBUTTONUP, wParam, ptCur.x, ptCur.x);
					//ReleaseCapture(); --2009-03-14
					return 0;
				}

				// ����� ��� ��� �� �������� �� MouseMove...
				//isLBDown = false;
				mouse.state &= ~(DRAG_L_ALLOWED | DRAG_L_STARTED | DRAG_R_STARTED); // ������ �������� ��� CDragDrop::Drag() �� ������� DRAG_R_ALLOWED
				// ����� �������, ���� ����� Drag
				//POSTMESSAGE(ghConWnd, WM_LBUTTONUP, wParam, MAKELPARAM( newX, newY ), TRUE);     //�������� ������� ����������

				//TODO("��� �� �� �������� ���� � �������, � ���������� ����� �������� ������� GetDragInfo � ������");
				//if (lbLeftDrag) { // ����� "���������" �������
				//	//POSTMESSAGE(ghConWnd, WM_LBUTTONUP, wParam, MAKELPARAM( mouse.LClkCon.X, mouse.LClkCon.Y ), TRUE);     //�������� ������� ����������
				//	pVCon->RCon()->OnMouse(WM_LBUTTONUP, wParam, mouse.LClkDC.X, mouse.LClkDC.Y);
				//} else {
				//	//POSTMESSAGE(ghConWnd, WM_LBUTTONDOWN, wParam, MAKELPARAM( mouse.RClkCon.X, mouse.RClkCon.Y ), TRUE);     //�������� ������� ����������
				//	pVCon->RCon()->OnMouse(WM_LBUTTONDOWN, wParam, mouse.RClkDC.X, mouse.RClkDC.Y);
				//	//POSTMESSAGE(ghConWnd, WM_LBUTTONUP, wParam, MAKELPARAM( mouse.RClkCon.X, mouse.RClkCon.Y ), TRUE);     //�������� ������� ����������
				//	pVCon->RCon()->OnMouse(WM_LBUTTONUP, wParam, mouse.RClkDC.X, mouse.RClkDC.Y);
				//}
				//pVCon->RCon()->FlushInputQueue();

				// ����� ������ ����������� FAR'������ D'n'D
				//SENDMESSAGE(ghConWnd, WM_LBUTTONUP, wParam, MAKELPARAM( newX, newY ));     //�������� ������� ����������
				if (mp_DragDrop)
				{
					//COORD crMouse = pVCon->ClientToConsole(
					//	lbLeftDrag ? mouse.LClkDC.X : mouse.RClkDC.X,
					//	lbLeftDrag ? mouse.LClkDC.Y : mouse.RClkDC.Y);
					DebugStep(_T("DnD: Drag-n-Drop starting"));
					mp_DragDrop->Drag(!lbLeftDrag, lbLeftDrag ? mouse.LClkDC : mouse.RClkDC);
					DebugStep(Title); // ������� ���������
				}
				else
				{
					_ASSERTE(mp_DragDrop); // ������ ���� ���������� ����
					DebugStep(_T("DnD: Drag-n-Drop is null"));
				}

				mouse.bIgnoreMouseMove = false;
				//#ifdef NEWMOUSESTYLE
				//newX = cursor.x; newY = cursor.y;
				//#else
				//newX = MulDiv(cursor.x, conRect.right, klMax<uint>(1, pVCon->Width));
				//newY = MulDiv(cursor.y, conRect.bottom, klMax<uint>(1, pVCon->Height));
				//#endif
				//if (lbLeftDrag)
				//  POSTMESSAGE(ghConWnd, WM_LBUTTONUP, wParam, MAKELPARAM( newX, newY ), TRUE);     //�������� ������� ����������
				//isDragProcessed=false; -- �����, ����� ��� �������� � ��������� ������ ������� ������ ���� ����� ��������� ��� ���???
				return 0;
			}
		}
	}
	else if (!gpSet->isDisableMouse && gpSet->isRClickSendKey && (mouse.state & MOUSE_R_LOCKED))
	{
		//���� ������� ������, � ���� �������� ����� RClick - �� ��������
		//����������� ���� - ������ ������� ������ ����
		if (!PTDIFFTEST(Rcursor, RCLICKAPPSDELTA))
		{
			//isRBDown=false;
			mouse.state &= ~(DRAG_R_ALLOWED | DRAG_R_STARTED | MOUSE_R_LOCKED);

			if (mb_RightClickingPaint)
			{
				// ���� ������ �������� Waiting ������ �������
				StopRightClickingPaint();
				_ASSERTE(mb_RightClickingPaint==FALSE);
			}

			char szLog[255];
			wsprintfA(szLog, "Mouse was moved, MOUSE_R_LOCKED cleared: Rcursor={%i-%i}, Now={%i-%i}, MinDelta=%i",
			          (int)Rcursor.x, (int)Rcursor.y,
			          (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam),
			          (int)RCLICKAPPSDELTA);
			LogString(szLog);
			//POSTMESSAGE(ghConWnd, WM_RBUTTONDOWN, 0, MAKELPARAM( mouse.RClkCon.X, mouse.RClkCon.Y ), TRUE);
			pVCon->RCon()->OnMouse(WM_RBUTTONDOWN, 0, mouse.RClkDC.X, mouse.RClkDC.Y);
		}

		return 0;
	}

	/*if (!isRBDown && (wParam==MK_RBUTTON)) {
	// ����� ��� ��������� ������ ������� ����� �� ������������
	if ((newY-RBDownNewY)>5) {// ���� ��������� ��� ������ ������ ����
	for (short y=RBDownNewY;y<newY;y+=5)
	POSTMESSAGE(ghConWnd, WM_MOUSEMOVE, wParam, MAKELPARAM( RBDownNewX, y ), TRUE);
	}
	RBDownNewX=newX; RBDownNewY=newY;
	}*/

	if (mouse.bIgnoreMouseMove)
		return 0;

	return TRUE; // ��������� � �������
}

LRESULT CConEmuMain::OnMouse_LBtnDown(CVirtualConsole* pVCon, HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam, POINT ptCur, COORD cr)
{
	enum DragPanelBorder dpb = DPB_NONE;
	//if (isLBDown()) ReleaseCapture(); // ����� ��������? --2009-03-14
	//isLBDown = false;
	mouse.state &= ~(DRAG_L_ALLOWED | DRAG_L_STARTED | MOUSE_DRAGPANEL_ALL);
	mouse.bIgnoreMouseMove = false;
	mouse.LClkCon = cr;
	mouse.LClkDC = MakeCoord(ptCur.x,ptCur.y);
	CRealConsole *pRCon = pVCon->RCon();

	if (!pRCon)  // ���� ������� ��� - �� � ����� ������
		return 0;

	dpb = CConEmuMain::CheckPanelDrag(cr);

	if (dpb != DPB_NONE)
	{
		if (dpb == DPB_SPLIT)
			mouse.state |= MOUSE_DRAGPANEL_SPLIT;
		else if (dpb == DPB_LEFT)
			mouse.state |= MOUSE_DRAGPANEL_LEFT;
		else if (dpb == DPB_RIGHT)
			mouse.state |= MOUSE_DRAGPANEL_RIGHT;

		// ���� ����� ���� - � FAR2 �������� ������ �������� ������
		if (isPressed(VK_SHIFT))
		{
			mouse.state |= MOUSE_DRAGPANEL_SHIFT;

			if (dpb == DPB_LEFT)
			{
				PostMacro(L"@$If (!APanel.Left) Tab $End");
			}
			else if (dpb == DPB_RIGHT)
			{
				PostMacro(L"@$If (APanel.Left) Tab $End");
			}
		}

		// LBtnDown � ������� �� ��������, �� ��������� ������� MouseMove?
		// (����� ���������� ����� � PanelTabs - �� ������������� � ������������ ��� ����������)
		INPUT_RECORD r = {MOUSE_EVENT};
		r.Event.MouseEvent.dwMousePosition = mouse.LClkCon;
		r.Event.MouseEvent.dwEventFlags = MOUSE_MOVED;
		pVCon->RCon()->PostConsoleEvent(&r);
		return 0;
	}

	if (!isConSelectMode() && isFilePanel() && pVCon &&
	        pVCon->RCon()->CoordInPanel(mouse.LClkCon))
	{
		//SetCapture('ghWnd DC'); --2009-03-14
		cursor.x = (int)(short)LOWORD(lParam);
		cursor.y = (int)(short)HIWORD(lParam);
		//isLBDown=true;
		//isDragProcessed=false;
		CONSOLE_CURSOR_INFO ci;
		pVCon->RCon()->GetConsoleCursorInfo(&ci);

		if ((ci.bVisible && ci.dwSize>0)  // ������ ������ ���� �����, ����� ��� ���-�� ����
		        && gpSet->isDragEnabled & DRAG_L_ALLOWED)
		{
			//if (!gpSet->nLDragKey || isPressed(gpSet->nLDragKey))
			// ������� �������� ����� �� nLDragKey (��������� nLDragKey==0), � ������ - �� ������
			// �� ���� ����� SHIFT(==nLDragKey), � CTRL & ALT - �� ������
			if (gpSet->IsModifierPressed(vkLDragKey, true))
			{
				mouse.state = DRAG_L_ALLOWED;
			}
		}

		// ����� ����� LBtnDown � ������� ����� ����� ����������� MOUSEMOVE
		mouse.lastMMW=wParam; mouse.lastMML=lParam;
		//if (gpSet->is DnD) mouse.bIgnoreMouseMove = true;
		//POSTMESSAGE(ghConWnd, messg, wParam, MAKELPARAM( newX, newY ), FALSE); // ���� SEND
		pVCon->RCon()->OnMouse(messg, wParam, ptCur.x, ptCur.y);
		return 0;
	}

	return TRUE; // ��������� � �������
}
LRESULT CConEmuMain::OnMouse_LBtnUp(CVirtualConsole* pVCon, HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam, POINT ptCur, COORD cr)
{
	BOOL lbLDrag = (mouse.state & DRAG_L_STARTED) == DRAG_L_STARTED;

	if (mouse.state & MOUSE_DRAGPANEL_ALL)
	{
		if (gpSet->isDragPanel == 2)
		{
			OnSizePanels(cr);
		}

		mouse.state &= ~(DRAG_L_ALLOWED | DRAG_L_STARTED | MOUSE_DRAGPANEL_ALL);
		// ���� �� �����...
		return 0;
	}

	mouse.state &= ~(DRAG_L_ALLOWED | DRAG_L_STARTED | MOUSE_DRAGPANEL_ALL);

	if (lbLDrag)
		return 0; // ������ ��� "��������"

	if (mouse.bIgnoreMouseMove)
	{
		mouse.bIgnoreMouseMove = false;
		//#ifdef NEWMOUSESTYLE
		//newX = cursor.x; newY = cursor.y;
		//#else
		//newX = MulDiv(cursor.x, conRect.right, klMax<uint>(1, pVCon->Width));
		//newY = MulDiv(cursor.y, conRect.bottom, klMax<uint>(1, pVCon->Height));
		//#endif
		//POSTMESSAGE(ghConWnd, messg, wParam, MAKELPARAM( newX, newY ), FALSE);
		pVCon->RCon()->OnMouse(messg, wParam, cursor.x, cursor.y);
		return 0;
	}

	return TRUE; // ��������� � �������
}
LRESULT CConEmuMain::OnMouse_LBtnDblClk(CVirtualConsole* pVCon, HWND hWnd, UINT& messg, WPARAM wParam, LPARAM lParam, POINT ptCur, COORD cr)
{
	CRealConsole *pRCon = pVCon->RCon();

	if (!pRCon)  // ���� ������� ��� - �� ����� ������
		return 0;

	enum DragPanelBorder dpb = CConEmuMain::CheckPanelDrag(cr);

	if (dpb != DPB_NONE)
	{
		wchar_t szMacro[128]; szMacro[0] = 0;

		if (dpb == DPB_SPLIT)
		{
			RECT rcRight; pRCon->GetPanelRect(TRUE, &rcRight, TRUE);
			int  nCenter = pRCon->TextWidth() / 2;

			if (nCenter < rcRight.left)
				_wsprintf(szMacro, SKIPLEN(countof(szMacro)) L"@$Rep (%i) CtrlLeft $End", rcRight.left - nCenter);
			else if (nCenter > rcRight.left)
				_wsprintf(szMacro, SKIPLEN(countof(szMacro)) L"@$Rep (%i) CtrlRight $End", nCenter - rcRight.left);
		}
		else
		{
			_wsprintf(szMacro, SKIPLEN(countof(szMacro)) L"@$Rep (%i) CtrlDown $End", pRCon->TextHeight());
		}

		if (szMacro[0])
			PostMacro(szMacro);

		return 0;
	}

	return TRUE; // ��������� � �������
}
LRESULT CConEmuMain::OnMouse_RBtnDown(CVirtualConsole* pVCon, HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam, POINT ptCur, COORD cr)
{
	Rcursor = ptCur;
	mouse.RClkCon = cr;
	mouse.RClkDC = MakeCoord(ptCur.x,ptCur.y);
	//isRBDown=false;
	mouse.state &= ~(DRAG_R_ALLOWED | DRAG_R_STARTED | MOUSE_R_LOCKED);
	mouse.bIgnoreMouseMove = false;

	if (gpSetCls->isAdvLogging)
	{
		char szLog[100];
		wsprintfA(szLog, "Right button down: Rcursor={%i-%i}", (int)Rcursor.x, (int)Rcursor.y);
		LogString(szLog);
	}

	//if (isFilePanel()) // Maximus5
	bool bSelect = false, bPanel = false, bActive = false, bCoord = false;

	if (!(bSelect = isConSelectMode())
	        && (bPanel = isFilePanel())
	        && (bActive = (pVCon != NULL))
	        && (bCoord = pVCon->RCon()->CoordInPanel(mouse.RClkCon)))
	{
		if (gpSet->isDragEnabled & DRAG_R_ALLOWED)
		{
			//if (!gpSet->nRDragKey || isPressed(gpSet->nRDragKey)) {
			// ������� �������� ����� �� nRDragKey (��������� nRDragKey==0), � ������ - �� ������
			// �� ���� ����� SHIFT(==nRDragKey), � CTRL & ALT - �� ������
			if (gpSet->IsModifierPressed(vkRDragKey, true))
			{
				mouse.state = DRAG_R_ALLOWED;

				if (gpSetCls->isAdvLogging) LogString("RightClick ignored of gpSet->nRDragKey pressed");

				return 0;
			}
		}

		// ���� ������ ������� �� ������!
		if (!gpSet->isDisableMouse && gpSet->isRClickSendKey && !(wParam&(MK_CONTROL|MK_LBUTTON|MK_MBUTTON|MK_SHIFT|MK_XBUTTON1|MK_XBUTTON2)))
		{
			//������� ������ �� .3
			//���� ������ - ������ apps
			mouse.state |= MOUSE_R_LOCKED; mouse.bSkipRDblClk = false;
			char szLog[100];
			wsprintfA(szLog, "MOUSE_R_LOCKED was set: Rcursor={%i-%i}", (int)Rcursor.x, (int)Rcursor.y);
			LogString(szLog);
			mouse.RClkTick = TimeGetTime(); //GetTickCount();
			StartRightClickingPaint();
			return 0;
		}
		else
		{
			if (gpSetCls->isAdvLogging)
				LogString(
					gpSet->isDisableMouse ? "RightClick ignored of gpSet->isDisableMouse" :
				    !gpSet->isRClickSendKey ? "RightClick ignored of !gpSet->isRClickSendKey" :
				    "RightClick ignored of wParam&(MK_CONTROL|MK_LBUTTON|MK_MBUTTON|MK_SHIFT|MK_XBUTTON1|MK_XBUTTON2)"
				);
		}
	}
	else
	{
		if (gpSetCls->isAdvLogging)
			LogString(
			    bSelect ? "RightClick ignored of isConSelectMode" :
			    !bPanel ? "RightClick ignored of NOT isFilePanel" :
			    !bActive ? "RightClick ignored of NOT isFilePanel" :
			    !bCoord ? "RightClick ignored of NOT isFilePanel" :
			    "RightClick ignored, unknown cause"
			);
	}

	return TRUE; // ��������� � �������
}
LRESULT CConEmuMain::OnMouse_RBtnUp(CVirtualConsole* pVCon, HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam, POINT ptCur, COORD cr)
{
	// ����� ��������, ���� �����������
	StopRightClickingPaint();

	if (!gpSet->isDisableMouse && gpSet->isRClickSendKey && (mouse.state & MOUSE_R_LOCKED))
	{
		//isRBDown=false; // ����� �������!
		mouse.state &= ~(DRAG_R_ALLOWED | DRAG_R_STARTED | MOUSE_R_LOCKED);

		if (PTDIFFTEST(Rcursor,RCLICKAPPSDELTA))
		{
			//������� ������� <.3
			//����� ������, �������� ������ �������
			//KillTimer(hWnd, 1); -- Maximus5, ������ ����� �� ������������
			DWORD dwCurTick = TimeGetTime(); //GetTickCount();
			DWORD dwDelta=dwCurTick-mouse.RClkTick;

			// ���� ������� ������ .3�, �� �� ������� ����� :)
			if ((gpSet->isRClickSendKey==1)
				|| (dwDelta>RCLICKAPPSTIMEOUT/*.3���*/ && dwDelta<RCLICKAPPSTIMEOUT_MAX/*10000*/)
				// ��� � ������ ��������� - ������� _���_
				|| (gpSet->isRClickTouchInvert() && (dwDelta<RCLICKAPPSTIMEOUT))
				)
			{
				if (!mb_RightClickingLSent && pVCon)
				{
					mb_RightClickingLSent = TRUE;
					// ����� ���������� ������ � ������� ����� ��� ������
					// ����� ���������� ���������, ��� ������ ������� ������ �����
					// ���������� EMenu, � �� ����� (���� �������� "������") ������ �� ���������.
					pVCon->RCon()->PostLeftClickSync(mouse.RClkDC);
					//apVCon->RCon()->OnMouse(WM_MOUSEMOVE, 0, mouse.RClkDC.X, mouse.RClkDC.Y, true);
					//WARNING("�� ��������, ����� ��������� ���� ���� ������������");
					//apVCon->RCon()->PostMacro(L"MsLClick");
					//WARNING("!!! �������� �� CMD_LEFTCLKSYNC?");
				}

				//// ������� �������� ���� ��� ��������
				////POSTMESSAGE(ghConWnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM( mouse.RClkCon.X, mouse.RClkCon.Y ), TRUE);
				//pVCon->RCon()->OnMouse(WM_LBUTTONDOWN, MK_LBUTTON, mouse.RClkDC.X, mouse.RClkDC.Y);
				////POSTMESSAGE(ghConWnd, WM_LBUTTONUP, 0, MAKELPARAM( mouse.RClkCon.X, mouse.RClkCon.Y ), TRUE);
				//pVCon->RCon()->OnMouse(WM_LBUTTONUP, 0, mouse.RClkDC.X, mouse.RClkDC.Y);
				//
				//pVCon->RCon()->FlushInputQueue();
				//pVCon->Update(true);
				//INVALIDATE(); //InvalidateRect(HDCWND, NULL, FALSE);
				// � ������ ����� � Apps ������
				mouse.bSkipRDblClk=true; // ����� ���� FAR ������ � ������� �� ���������� ������� ���������
				//POSTMESSAGE(ghConWnd, WM_KEYDOWN, VK_APPS, 0, TRUE);
				DWORD dwFarPID = pVCon->RCon()->GetFarPID();

				if (dwFarPID)
				{
					AllowSetForegroundWindow(dwFarPID);
					//if (gpSet->sRClickMacro && *gpSet->sRClickMacro) {
					//    //// ������� �������� ���� ��� ��������
					//    ////POSTMESSAGE(ghConWnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM( mouse.RClkCon.X, mouse.RClkCon.Y ), TRUE);
					//    //pVCon->RCon()->OnMouse(WM_LBUTTONDOWN, MK_LBUTTON, mouse.RClkDC.X, mouse.RClkDC.Y);
					//    ////POSTMESSAGE(ghConWnd, WM_LBUTTONUP, 0, MAKELPARAM( mouse.RClkCon.X, mouse.RClkCon.Y ), TRUE);
					//    //pVCon->RCon()->OnMouse(WM_LBUTTONUP, 0, mouse.RClkDC.X, mouse.RClkDC.Y);
					//    //
					//    //pVCon->RCon()->FlushInputQueue();
					//
					//    // ���� ���� ����� ���� ������ - ��������� ���
					//    PostMacro(gpSet->sRClickMacro);
					//} else {
					COORD crMouse = pVCon->RCon()->ScreenToBuffer(
					                    pVCon->ClientToConsole(mouse.RClkDC.X, mouse.RClkDC.Y)
					                );
					CConEmuPipe pipe(GetFarPID(), CONEMUREADYTIMEOUT);

					if (pipe.Init(_T("CConEmuMain::EMenu"), TRUE))
					{
						//DWORD cbWritten=0;
						DebugStep(_T("EMenu: Waiting for result (10 sec)"));
						int nLen = 0;
						int nSize = sizeof(crMouse) + sizeof(wchar_t);

						if (gpSet->sRClickMacro && *gpSet->sRClickMacro)
						{
							nLen = _tcslen(gpSet->sRClickMacro);
							nSize += nLen*2;
							// -- �������� �� �������� ������� ScreenToClient
							//pVCon->RCon()->RemoveFromCursor();
						}

						LPBYTE pcbData = (LPBYTE)calloc(nSize,1);
						_ASSERTE(pcbData);
						memmove(pcbData, &crMouse, sizeof(crMouse));

						if (nLen)
							lstrcpy((wchar_t*)(pcbData+sizeof(crMouse)), gpSet->sRClickMacro);

						if (!pipe.Execute(CMD_EMENU, pcbData, nSize))
						{
							LogString("RightClicked, but pipe.Execute(CMD_EMENU) failed");
						}
						else
						{
							// OK
							if (gpSetCls->isAdvLogging)
							{
								char szInfo[255] = {0};
								lstrcpyA(szInfo, "RightClicked, pipe.Execute(CMD_EMENU) OK");

								if (gpSet->sRClickMacro && *gpSet->sRClickMacro)
								{
									lstrcatA(szInfo, ", Macro: "); int nLen = lstrlenA(szInfo);
									WideCharToMultiByte(CP_ACP,0, gpSet->sRClickMacro,-1, szInfo+nLen, countof(szInfo)-nLen-1, 0,0);
								}
								else
								{
									lstrcatA(szInfo, ", NoMacro");
								}

								LogString(szInfo);
							}
						}

						DebugStep(NULL);
						free(pcbData);
					}
					else
					{
						LogString("RightClicked, but pipe.Init() failed");
					}

					//INPUT_RECORD r = {KEY_EVENT};
					////pVCon->RCon()->On Keyboard(ghConWnd, WM_KEYDOWN, VK_APPS, (VK_APPS << 16) | (1 << 24));
					////pVCon->RCon()->On Keyboard(ghConWnd, WM_KEYUP, VK_APPS, (VK_APPS << 16) | (1 << 24));
					//r.Event.KeyEvent.bKeyDown = TRUE;
					//r.Event.KeyEvent.wVirtualKeyCode = VK_APPS;
					//r.Event.KeyEvent.wVirtualScanCode = /*28 �� ���� ����������*/MapVirtualKey(VK_APPS, 0/*MAPVK_VK_TO_VSC*/);
					//r.Event.KeyEvent.dwControlKeyState = 0x120;
					//pVCon->RCon()->PostConsoleEvent(&r);
					////On Keyboard(hConWnd, WM_KEYUP, VK_RETURN, 0);
					//r.Event.KeyEvent.bKeyDown = FALSE;
					//r.Event.KeyEvent.dwControlKeyState = 0x120;
					//pVCon->RCon()->PostConsoleEvent(&r);
				}
				else
				{
					LogString("RightClicked, but FAR PID is 0");
				}

				return 0;
			}
			else
			{
				char szLog[255];
				// if ((gpSet->isRClickSendKey==1) || (dwDelta>RCLICKAPPSTIMEOUT && dwDelta<10000))
				lstrcpyA(szLog, "RightClicked, but condition failed: ");

				if (gpSet->isRClickSendKey!=1)
				{
					wsprintfA(szLog+lstrlenA(szLog), "((isRClickSendKey=%i)!=1)", (UINT)gpSet->isRClickSendKey);
				}
				else
				{
					wsprintfA(szLog+lstrlenA(szLog), "(isRClickSendKey==%i)", (UINT)gpSet->isRClickSendKey);
				}

				if (dwDelta <= RCLICKAPPSTIMEOUT)
				{
					wsprintfA(szLog+lstrlenA(szLog), ", ((Delay=%i)<=%i)", dwDelta, (int)RCLICKAPPSTIMEOUT);
				}
				else if (dwDelta >= 10000)
				{
					wsprintfA(szLog+lstrlenA(szLog), ", ((Delay=%i)>=10000)", dwDelta);
				}
				else
				{
					wsprintfA(szLog+lstrlenA(szLog), ", (Delay==%i)", dwDelta);
				}

				LogString(szLog);
			}
		}
		else
		{
			char szLog[100];
			wsprintfA(szLog, "RightClicked, but mouse was moved abs({%i-%i}-{%i-%i})>%i", Rcursor.x, Rcursor.y, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (int)RCLICKAPPSDELTA);
			LogString(szLog);
		}

		// ����� ����� RBtnDown � ������� ����� ����� ����������� MOUSEMOVE
		mouse.lastMMW=MK_RBUTTON|wParam; mouse.lastMML=lParam; // ���������, �.�. �� � RButtonUp
		// ����� ����� ������� ������� WM_RBUTTONDOWN
		//POSTMESSAGE(ghConWnd, WM_RBUTTONDOWN, wParam, MAKELPARAM( newX, newY ), TRUE);
		pVCon->RCon()->OnMouse(WM_RBUTTONDOWN, wParam, ptCur.x, ptCur.y);
	}
	else
	{
		char szLog[255];
		// if (gpSet->isRClickSendKey && (mouse.state & MOUSE_R_LOCKED))
		//wsprintfA(szLog, "RightClicked, but condition failed (RCSK:%i, State:%u)", (int)gpSet->isRClickSendKey, (DWORD)mouse.state);
		lstrcpyA(szLog, "RightClicked, but condition failed: ");

		if (gpSet->isDisableMouse)
		{
			wsprintfA(szLog+lstrlenA(szLog), "((isDisableMouse=%i)==0)", (UINT)gpSet->isDisableMouse);
		}
		if (gpSet->isRClickSendKey==0)
		{
			wsprintfA(szLog+lstrlenA(szLog), "((isRClickSendKey=%i)==0)", (UINT)gpSet->isRClickSendKey);
		}
		else
		{
			wsprintfA(szLog+lstrlenA(szLog), "(isRClickSendKey==%i)", (UINT)gpSet->isRClickSendKey);
		}

		if ((mouse.state & MOUSE_R_LOCKED) == 0)
		{
			wsprintfA(szLog+lstrlenA(szLog), ", (((state=0x%X)&MOUSE_R_LOCKED)==0)", (DWORD)mouse.state);
		}
		else
		{
			wsprintfA(szLog+lstrlenA(szLog), ", (state==0x%X)", (DWORD)mouse.state);
		}

		LogString(szLog);
	}

	//isRBDown=false; // ����� �� �������� ��������
	mouse.state &= ~(DRAG_R_ALLOWED | DRAG_R_STARTED | MOUSE_R_LOCKED);
	return TRUE; // ��������� � �������
}
LRESULT CConEmuMain::OnMouse_RBtnDblClk(CVirtualConsole* pVCon, HWND hWnd, UINT& messg, WPARAM wParam, LPARAM lParam, POINT ptCur, COORD cr)
{
	if (mouse.bSkipRDblClk)
	{
		mouse.bSkipRDblClk = false;
		return 0; // �� ������������, ������ ����� ����������� ����
	}

	//if (gpSet->isRClickSendKey) {
	//	// �������� �� ��������� ����, ����� ����� �� ��������� ����������� ����
	//	messg = WM_RBUTTONDOWN;
	//} -- ����, ������ ��� ������ � �������� � ����� ������...
	messg = WM_RBUTTONDOWN;
	return TRUE; // ��������� � �������
}

BOOL CConEmuMain::OnMouse_NCBtnDblClk(HWND hWnd, UINT& messg, WPARAM wParam, LPARAM lParam)
{
	// �� DblClick �� ����� - ���������� ���� �� ����������� (���������) �� ���� �������
	if (wParam == HTLEFT || wParam == HTRIGHT || wParam == HTTOP || wParam == HTBOTTOM)
	{
		if (WindowMode != wmNormal)
		{
			// ��� ���� �� ������ - ����� �� ������ ���� ����� � ���� �������
			// ��� Quake - ������������
			_ASSERTE((WindowMode == wmNormal) || gpSet->isQuakeStyle);
			return FALSE;
		}

		RECT rcWnd, rcNewWnd;
		GetWindowRect(ghWnd, &rcWnd);
		rcNewWnd = rcWnd;
		MONITORINFO mon = {sizeof(MONITORINFO)};

		if (wParam == HTLEFT || wParam == HTRIGHT)
		{
			// ����� ����� ����� �������
			POINT pt = {rcWnd.left,((rcWnd.top+rcWnd.bottom)>>2)};
			HMONITOR hMonLeft = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

			if (!GetMonitorInfo(hMonLeft, &mon))
				return FALSE;

			rcNewWnd.left = mon.rcWork.left;
			// ����� ����� ������ �������
			pt.x = rcWnd.right;
			HMONITOR hMonRight = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

			if (hMonRight != hMonLeft)
				if (!GetMonitorInfo(hMonRight, &mon))
					return FALSE;

			rcNewWnd.right = mon.rcWork.right;
			// ��������������� ������� �� ������ �����
			RECT rcFrame = CalcMargins(CEM_FRAMECAPTION);
			rcNewWnd.left -= rcFrame.left;
			rcNewWnd.right += rcFrame.right;
		}
		else if (wParam == HTTOP || wParam == HTBOTTOM)
		{
			// ����� ����� ������� �������
			POINT pt = {((rcWnd.left+rcWnd.right)>>2),rcWnd.top};
			HMONITOR hMonTop = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

			if (!GetMonitorInfo(hMonTop, &mon))
				return FALSE;

			rcNewWnd.top = mon.rcWork.top;
			// ����� ����� ������ �������
			pt.y = rcWnd.bottom;
			HMONITOR hMonBottom = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

			if (hMonBottom != hMonTop)
				if (!GetMonitorInfo(hMonBottom, &mon))
					return FALSE;

			rcNewWnd.bottom = mon.rcWork.bottom;
			// ��������������� ������� �� ������ �����
			RECT rcFrame = CalcMargins(CEM_FRAMECAPTION);
			rcNewWnd.top -= rcFrame.bottom; // �.�. � top ������ ������ ���������
			rcNewWnd.bottom += rcFrame.bottom;
		}

		// ������� ������
		if (rcNewWnd.left != rcWnd.left || rcNewWnd.right != rcWnd.right || rcNewWnd.top != rcWnd.top || rcNewWnd.bottom != rcWnd.bottom)
			MOVEWINDOW(ghWnd, rcNewWnd.left, rcNewWnd.top, rcNewWnd.right-rcNewWnd.left, rcNewWnd.bottom-rcNewWnd.top, 1);

		return TRUE;
	}

	return FALSE;
}

void CConEmuMain::SetDragCursor(HCURSOR hCur)
{
	mh_DragCursor = hCur;

	if (mh_DragCursor)
	{
		SetCursor(mh_DragCursor);
	}
	else
	{
		OnSetCursor();
	}
}

void CConEmuMain::SetWaitCursor(BOOL abWait)
{
	mb_WaitCursor = abWait;

	if (mb_WaitCursor)
		SetCursor(mh_CursorWait);
	else
		SetCursor(mh_CursorArrow);
}

void CConEmuMain::CheckFocus(LPCWSTR asFrom)
{
	HWND hCurForeground = GetForegroundWindow();
	static HWND hPrevForeground;

	if (hPrevForeground == hCurForeground)
		return;

	hPrevForeground = hCurForeground;
	DWORD dwPID = 0, dwTID = 0;

	#ifdef _DEBUG
	wchar_t szDbg[255], szClass[128];

	if (!hCurForeground || !GetClassName(hCurForeground, szClass, 127)) lstrcpy(szClass, L"<NULL>");
	#endif

	BOOL lbConEmuActive = (hCurForeground == ghWnd || (ghOpWnd && hCurForeground == ghOpWnd));
	BOOL lbLDown = isPressed(VK_LBUTTON);

	if (!lbConEmuActive)
	{
		if (!hCurForeground)
		{
			#ifdef _DEBUG
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"Foreground changed (%s). NewFore=0x00000000, LBtn=%i\n", asFrom, lbLDown);
			#endif
		}
		else
		{
			dwTID = GetWindowThreadProcessId(hCurForeground, &dwPID);
			// �������� ���������� �� �������� �����
			GUITHREADINFO gti = {sizeof(GUITHREADINFO)};
			GetGUIThreadInfo(0/*dwTID*/, &gti);

			#ifdef _DEBUG
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"Foreground changed (%s). NewFore=0x%08X, Active=0x%08X, Focus=0x%08X, Class=%s, LBtn=%i\n", asFrom, (DWORD)hCurForeground, (DWORD)gti.hwndActive, (DWORD)gti.hwndFocus, szClass, lbLDown);
			#endif

			// mh_ShellWindow ����� ��� ����������. ���� �������� ConEmu � �������� �� mh_ShellWindow
			// �� ��������� ����� ���������� ���� ���� � ������ (WorkerW ��� Progman)
			if (gpSet->isDesktopMode && mh_ShellWindow)
			{
				//HWND hShell = GetShellWindow(); // Progman
				bool lbDesktopActive = false;

				if (dwPID == mn_ShellWindowPID)  // ������ ������ ��� �������� Explorer.exe (������� ������ Desktop)
				{
					// ��� WinD ������������ (Foreground) �� Progman, � WorkerW
					// ��� �� �����, ����� - ���������� � �������� ���� Progman
					if (hCurForeground == mh_ShellWindow)
					{
						lbDesktopActive = true;
					}
					else
					{
						wchar_t szClass[128];

						if (!GetClassName(hCurForeground, szClass, 127)) szClass[0] = 0;

						if (!wcscmp(szClass, L"WorkerW") || !wcscmp(szClass, L"Progman"))
							lbDesktopActive = true;

						//HWND hDesktop = GetDesktopWindow();
						//HWND hParent = GetParent(gti.hwndFocus);
						////	GetWindow(gti.hwndFocus, GW_OWNER);
						//while (hParent) {
						//	if (hParent == mh_ShellWindow) {
						//		lbDesktopActive = true;
						//		break;
						//	}
						//	hParent = GetParent(hParent);
						//	if (hParent == hDesktop) break;
						//}
					}
				}

				if (lbDesktopActive)
				{
					CVConGuard VCon;

					if (lbLDown)
					{
						// �������� ������ ��� ��������� �������� ����� � ��������
						mb_FocusOnDesktop = FALSE; // ��������, ��� ConEmu ������������ �� �����
					}
					else if (mb_FocusOnDesktop && (GetActiveVCon(&VCon) >= 0))
					{
						// ����� ������������ �� ����������� ������� ������������ ConEmu ����� WinD / WinM
						//apiSetForegroundWindow(ghWnd); // ��� ������ ����� �� ���������, �.�. ����� ������ � ������� ��������!
						// ��� ��� "����������" ������
						COORD crOpaque = VCon->FindOpaqueCell();

						if (crOpaque.X<0 || crOpaque.Y<0)
						{
							DEBUGSTRFOREGROUND(L"Can't activate ConEmu on desktop. No opaque cell was found in VCon\n");
						}
						else
						{
							POINT pt = VCon->ConsoleToClient(crOpaque.X,crOpaque.Y);
							MapWindowPoints(VCon->GetView(), NULL, &pt, 1);
							HWND hAtPoint = WindowFromPoint(pt);

							if (hAtPoint != ghWnd)
							{
								#ifdef _DEBUG
								wchar_t szDbg[255], szClass[64];

								if (!hAtPoint || !GetClassName(hAtPoint, szClass, 63)) szClass[0] = 0;

								_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"Can't activate ConEmu on desktop. Opaque cell={%i,%i} screen={%i,%i}. WindowFromPoint=0x%08X (%s)\n",
								          crOpaque.X, crOpaque.Y, pt.x, pt.y, (DWORD)hAtPoint, szClass);
								DEBUGSTRFOREGROUND(szDbg);
								#endif
							}
							else
							{
								DEBUGSTRFOREGROUND(L"Activating ConEmu on desktop by mouse click\n");
								mouse.bForceSkipActivation = TRUE; // �� ���������� ���� ���� � �������!
								// ���������, ��� ������ ������. ������� ���� �����
								POINT ptCur; GetCursorPos(&ptCur);
								SetCursorPos(pt.x,pt.y); // ����� ����������� "���������", ����� mouse_event �� ���������
								// "�������"
								mouse_event(MOUSEEVENTF_ABSOLUTE+MOUSEEVENTF_LEFTDOWN, pt.x,pt.y, 0,0);
								mouse_event(MOUSEEVENTF_ABSOLUTE+MOUSEEVENTF_LEFTUP, pt.x,pt.y, 0,0);
								// ������� ������
								SetCursorPos(ptCur.x,ptCur.y);
								//
								//#ifdef _DEBUG -- ������� ��� �� ���������� ��������...
								//HWND hPost = GetForegroundWindow();
								//DEBUGSTRFOREGROUND((hPost==ghWnd) ? L"ConEmu on desktop activation Succeeded\n" : L"ConEmu on desktop activation FAILED\n");
								//#endif
							}
						}
					}
				}
			}
		}
	}
	else
	{
		#ifdef _DEBUG
		_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"Foreground changed (%s). NewFore=0x%08X, ConEmu has focus, LBtn=%i\n", asFrom, (DWORD)hCurForeground, lbLDown);
		#endif
		mb_FocusOnDesktop = TRUE;
	}

	UNREFERENCED_PARAMETER(dwTID);
	DEBUGSTRFOREGROUND(szDbg);
}

void CConEmuMain::CheckUpdates(BOOL abShowMessages)
{
	if (gpConEmu->mb_MingwMode)
		return;

	if (!gpUpd)
		gpUpd = new CConEmuUpdate;
	
	if (gpUpd)
		gpUpd->StartCheckProcedure(abShowMessages);
}

bool CConEmuMain::ReportUpdateConfirmation()
{
	bool lbRc = false;

	if (isMainThread())
	{
		if (gpUpd)
			lbRc = gpUpd->ShowConfirmation();
	}
	else
	{
		lbRc = (SendMessage(ghWnd, mn_MsgRequestUpdate, 1, 0) != 0);
	}

	return lbRc;
}

void CConEmuMain::ReportUpdateError()
{
	if (isMainThread())
	{
		if (gpUpd)
			gpUpd->ShowLastError();
	}
	else
	{
		PostMessage(ghWnd, mn_MsgRequestUpdate, 0, 0);
	}
}

void CConEmuMain::RequestExitUpdate()
{
	CConEmuUpdate::UpdateStep step = CConEmuUpdate::us_NotStarted;
	if (!gpUpd)
		return;

	step = gpUpd->InUpdate();
	if (step != CConEmuUpdate::us_ExitAndUpdate)
	{
		_ASSERTE(step == CConEmuUpdate::us_ExitAndUpdate);
		return;
	}

	if (isMainThread())
	{
		UpdateProgress();
		PostMessage(ghWnd, WM_SYSCOMMAND, SC_CLOSE, 0);
	}
	else
	{
		PostMessage(ghWnd, mn_MsgRequestUpdate, 2, 0);
	}
}

enum DragPanelBorder CConEmuMain::CheckPanelDrag(COORD crCon)
{
	if (!gpSet->isDragPanel || isPictureView())
		return DPB_NONE;

	CVConGuard VCon;
	CRealConsole* pRCon = (GetActiveVCon(&VCon) >= 0) ? VCon->RCon() : NULL;

	if (!pRCon)
		return DPB_NONE;

	// ������ ���� "���", "������", "�������" (��� ������ �������� ������� ������, ���� ��� "�����")
	if (!pRCon->isFar() || !pRCon->isFilePanel(true) || !pRCon->isAlive())
		return DPB_NONE;

	// ���� ������� ��� ��� ��������� �������
	if (pRCon->isConSelectMode())
		return DPB_NONE;

	// ���� ������������ ����������� ������� ��������
	if ((gpSet->isCTSSelectBlock && gpSet->IsModifierPressed(vkCTSVkBlock, false))
	        || (gpSet->isCTSSelectText && gpSet->IsModifierPressed(vkCTSVkText, false)))
		return DPB_NONE;

	//CONSOLE_CURSOR_INFO ci;
	//mp_ VActive->RCon()->GetConsoleCursorInfo(&ci);
	//   if (!ci.bVisible || ci.dwSize>40) // ������ ������ ���� �����, � �� � ������ �����
	//   	return DPB_NONE;
	// ������ - ����� ���������
	enum DragPanelBorder dpb = DPB_NONE;
	RECT rcPanel;
	
	//TODO("������� ���-���� ���� �����-������ ��������� �� ����������� ����� �������");
	//int nSplitWidth = gpSetCls->BorderFontWidth()/5;
	//if (nSplitWidth < 1) nSplitWidth = 1;
	

	if (VCon->RCon()->GetPanelRect(TRUE, &rcPanel, TRUE))
	{
		if (crCon.X == rcPanel.left && (rcPanel.top <= crCon.Y && crCon.Y <= rcPanel.bottom))
			dpb = DPB_SPLIT;
		else if (crCon.Y == rcPanel.bottom && (rcPanel.left <= crCon.X && crCon.X <= rcPanel.right))
			dpb = DPB_RIGHT;
	}

	if (dpb == DPB_NONE && VCon->RCon()->GetPanelRect(FALSE, &rcPanel, TRUE))
	{
		if (gpSet->isDragPanelBothEdges && crCon.X == rcPanel.right && (rcPanel.top <= crCon.Y && crCon.Y <= rcPanel.bottom))
			dpb = DPB_SPLIT;
		if (crCon.Y == rcPanel.bottom && (rcPanel.left <= crCon.X && crCon.X <= rcPanel.right))
			dpb = DPB_LEFT;
	}

	return dpb;
}

LRESULT CConEmuMain::OnSetCursor(WPARAM wParam, LPARAM lParam)
{
	DEBUGSTRSETCURSOR(lParam==-1 ? L"WM_SETCURSOR (int)" : L"WM_SETCURSOR");
#ifdef _DEBUG
	if (isPressed(VK_LBUTTON))
	{
		int nDbg = 0;
	}
#endif

	POINT ptCur; GetCursorPos(&ptCur);
	// ���� ������ ���� trackPopupMenu - �� �� �����
	CVirtualConsole* pVCon = (mn_TrackMenuPlace == tmp_None) ? GetVConFromPoint(ptCur) : NULL;
	if (pVCon && !isActive(pVCon, false))
		pVCon = NULL;
	CRealConsole *pRCon = pVCon ? pVCon->RCon() : NULL;
	if (!pRCon)
	{
		// ���� ������ �� ��� �������� - �� ������ �� ���������
		DEBUGSTRSETCURSOR(L" ---> skipped, not over VCon\n");
		return FALSE;
	}

	// � GUI ������ - �� ������ ������, �������� �������� �����������
	HWND hGuiClient = pRCon->GuiWnd();
	if (pRCon && hGuiClient && pRCon->isGuiVisible())
	{
		DEBUGSTRSETCURSOR(L" ---> skipped (TRUE), GUI App Visible\n");
		return TRUE;
	}

	if (lParam == (LPARAM)-1)
	{ 
		RECT rcWnd; GetWindowRect(ghWnd, &rcWnd);

		if (!PtInRect(&rcWnd, ptCur))
		{
			if (!isMeForeground())
			{
				DEBUGSTRSETCURSOR(L" ---> skipped, !isMeForeground()\n");
				return FALSE;
			}

			lParam = HTNOWHERE;
		}
		else
		{
			//CVirtualConsole* pVCon = GetVConFromPoint(ptCur); -- ���
			if (pVCon)
				lParam = HTCLIENT;
			else
				lParam = HTCAPTION;
		}

		wParam = pVCon ? (WPARAM)pVCon->GetView() : (WPARAM)ghWnd;
	}

	if (!(((HWND)wParam) == ghWnd || pVCon)
		|| isSizing()
		|| (LOWORD(lParam) != HTCLIENT && LOWORD(lParam) != HTNOWHERE))
	{
		/*if (gpSet->isHideCaptionAlways && !mb_InTrackSysMenu && !isSizing()
			&& (LOWORD(lParam) == HTTOP || LOWORD(lParam) == HTCAPTION))
		{
			SetCursor(mh_CursorMove);
			return TRUE;
		}*/
		DEBUGSTRSETCURSOR(L" ---> skipped, condition\n");
		return FALSE;
	}

	HCURSOR hCur = NULL;
	//LPCWSTR pszCurName = NULL;
	BOOL lbMeFore = TRUE;
	ExpandTextRangeType etr = etr_None;

	if (LOWORD(lParam) == HTCLIENT && pVCon)
	{
		if (mouse.state & MOUSE_WINDOW_DRAG)
		{
			hCur = LoadCursor(NULL, IDC_SIZEALL);
		}
		// ���� ����� ShellDrag
		else if (mh_DragCursor && isDragging())
		{
			hCur = mh_DragCursor;
			DEBUGSTRSETCURSOR(L" ---> DragCursor\n");
		}
		else if (pRCon->isSelectionPresent())
		{
			hCur = LoadCursor(NULL, IDC_IBEAM);
		}
		else if ((etr = pRCon->GetLastTextRangeType()) != etr_None)
		{
			hCur = LoadCursor(NULL, IDC_HAND);
		}
		else if (mouse.state & MOUSE_DRAGPANEL_ALL)
		{
			if (mouse.state & MOUSE_DRAGPANEL_SPLIT)
			{
				hCur = mh_SplitH;
				DEBUGSTRSETCURSOR(L" ---> SplitH (dragging)\n");
			}
			else
			{
				hCur = mh_SplitV;
				DEBUGSTRSETCURSOR(L" ---> SplitV (dragging)\n");
			}
		}
		else
		{
			lbMeFore = isMeForeground();

			if (lbMeFore)
			{
				HWND hWndDC = pVCon ? pVCon->GetView() : NULL;
				if (pRCon && pRCon->isFar(FALSE) && hWndDC)  // ������ �� �����, ��� ���...
				{
					MapWindowPoints(NULL, hWndDC, &ptCur, 1);
					COORD crCon = pVCon->ClientToConsole(ptCur.x,ptCur.y);
					enum DragPanelBorder dpb = CheckPanelDrag(crCon);

					if (dpb == DPB_SPLIT)
					{
						hCur = mh_SplitH;
						DEBUGSTRSETCURSOR(L" ---> SplitH (allow)\n");
					}
					else if (dpb != DPB_NONE)
					{
						hCur = mh_SplitV;
						DEBUGSTRSETCURSOR(L" ---> SplitV (allow)\n");
					}
				}
			}
		}
	}

	if (!hCur && lbMeFore)
	{
		if (mb_WaitCursor)
		{
			hCur = mh_CursorWait;
			DEBUGSTRSETCURSOR(L" ---> CursorWait (mb_CursorWait)\n");
		}
		else if (gpSet->isFarHourglass)
		{
			if (pRCon)
			{
				BOOL lbAlive = pRCon->isAlive();
				if (!lbAlive)
				{
					hCur = mh_CursorAppStarting;
					DEBUGSTRSETCURSOR(L" ---> AppStarting (!pRCon->isAlive())\n");
				}
			}
		}
	}

	if (!hCur)
	{
		hCur = mh_CursorArrow;
		DEBUGSTRSETCURSOR(L" ---> Arrow (default)\n");
	}

	SetCursor(hCur);
	return TRUE;
}

// true - ���� nPID ������� � ����� �� ��������
bool CConEmuMain::isConsolePID(DWORD nPID)
{
	return CVConGroup::isConsolePID(nPID);
}

LRESULT CConEmuMain::OnShellHook(WPARAM wParam, LPARAM lParam)
{
	/*
	wParam lParam
	HSHELL_GETMINRECT A pointer to a SHELLHOOKINFO structure.
	HSHELL_WINDOWACTIVATEED The HWND handle of the activated window.
	HSHELL_RUDEAPPACTIVATEED The HWND handle of the activated window.
	HSHELL_WINDOWREPLACING The HWND handle of the window replacing the top-level window.
	HSHELL_WINDOWREPLACED The HWND handle of the window being replaced.
	HSHELL_WINDOWCREATED The HWND handle of the window being created.
	HSHELL_WINDOWDESTROYED The HWND handle of the top-level window being destroyed.
	HSHELL_ACTIVATESHELLWINDOW Not used.
	HSHELL_TASKMAN Can be ignored.
	HSHELL_REDRAW The HWND handle of the window that needs to be redrawn.
	HSHELL_FLASH The HWND handle of the window that needs to be flashed.
	HSHELL_ENDTASK The HWND handle of the window that should be forced to exit.
	HSHELL_APPCOMMAND The APPCOMMAND which has been unhandled by the application or other hooks. See WM_APPCOMMAND and use the GET_APPCOMMAND_LPARAM macro to retrieve this parameter.
	*/
	switch(wParam)
	{
		case HSHELL_FLASH:
		{
			//HWND hCon = (HWND)lParam;
			//if (!hCon) return 0;
			//for (size_t i = 0; i < countof(mp_VCon); i++) {
			//    if (!mp_VCon[i]) continue;
			//    if (mp_VCon[i]->RCon()->ConWnd() == hCon) {
			//        FLASHWINFO fl = {sizeof(FLASHWINFO)};
			//        if (isMeForeground()) {
			//        	if (mp_VCon[i] != mp_ VActive) { // ������ ��� ���������� �������
			//                fl.dwFlags = FLASHW_STOP; fl.hwnd = ghWnd;
			//                FlashWindowEx(&fl); // ����� ������� �� �������������
			//        		fl.uCount = 4; fl.dwFlags = FLASHW_ALL; fl.hwnd = ghWnd;
			//        		FlashWindowEx(&fl);
			//        	}
			//        } else {
			//        	fl.dwFlags = FLASHW_ALL|FLASHW_TIMERNOFG; fl.hwnd = ghWnd;
			//        	FlashWindowEx(&fl); // �������� � GUI
			//        }
			//
			//        fl.dwFlags = FLASHW_STOP; fl.hwnd = hCon;
			//        FlashWindowEx(&fl);
			//        break;
			//    }
			//}
		}
		break;

		case HSHELL_WINDOWCREATED:
		{
			if (isMeForeground())
			{
				HWND hWnd = (HWND)lParam;

				if (!hWnd) return 0;

				DWORD dwPID = 0, dwParentPID = 0; // dwFarPID = 0;
				GetWindowThreadProcessId(hWnd, &dwPID);

				if (dwPID && dwPID != GetCurrentProcessId())
				{
					AllowSetForegroundWindow(dwPID);

					WARNING("���������, �������� �� ������ ����������, ��� ����� ��� ���� ShowWindow �������?");
					if (IsWindowVisible(hWnd))  // ? ��� ������ ?
					{
						bool lbConPID = isConsolePID(dwPID);

						PROCESSENTRY32 prc = {sizeof(PROCESSENTRY32)};
						if (!lbConPID)
						{
							// �������� PID ������������� �������� ����� ������
							if (GetProcessInfo(dwPID, &prc))
							{
								dwParentPID = prc.th32ParentProcessID;

								lbConPID = (dwParentPID && isConsolePID(dwParentPID));
							}
						}

						if (lbConPID)
						{
							apiSetForegroundWindow(hWnd);
						}
					}

					//#ifdef _DEBUG
					//wchar_t szTitle[255], szClass[255], szMsg[1024];
					//GetWindowText(hWnd, szTitle, 254); GetClassName(hWnd, szClass, 254);
					//_wsprintf(szMsg, countof(szMsg), L"Window was created:\nTitle: %s\nClass: %s\nPID: %i", szTitle, szClass, dwPID);
					//MBox(szMsg);
					//#endif
				}
			}
		}
		break;
		
		#ifdef _DEBUG
		case HSHELL_ACTIVATESHELLWINDOW:
		{
			// �� ����������
			wchar_t szDbg[128]; _wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"HSHELL_ACTIVATESHELLWINDOW(lParam=0x%08X)\n", (DWORD)lParam);
			DEBUGSTRFOREGROUND(szDbg);
		}
		break;
		#endif

		case HSHELL_WINDOWACTIVATED:
		{
			// �������� ����� ��� WM_ACTIVATE(WA_INACTIVE), �� ����������, ���� CE ��� �� � ������
			#ifdef _DEBUG
			// ����� ������������ Desktop - lParam == 0

			wchar_t szDbg[128], szClass[64];
			if (!lParam || !GetClassName((HWND)lParam, szClass, 63))
				wcscpy(szClass, L"<NULL>");

			BOOL lbLBtn = isPressed(VK_LBUTTON);
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"HSHELL_WINDOWACTIVATED(lParam=0x%08X, %s, %i)\n", (DWORD)lParam, szClass, lbLBtn);
			DEBUGSTRFOREGROUND(szDbg);
			#endif

			//CheckFocus(L"HSHELL_WINDOWACTIVATED");
			OnFocus(NULL, 0, 0, 0, L"HSHELL_WINDOWACTIVATED");
		}
		break;
	}

	return 0;
}

void CConEmuMain::OnAlwaysOnTop()
{
	HWND hwndAfter = (gpSet->isAlwaysOnTop || gpSet->isDesktopMode) ? HWND_TOPMOST : HWND_NOTOPMOST;

	#ifdef CATCH_TOPMOST_SET
	_ASSERTE((hwndAfter!=HWND_TOPMOST) && "Setting TopMost mode - CConEmuMain::OnAlwaysOnTop()");
	#endif

	CheckMenuItem(gpConEmu->GetSysMenu(), ID_ALWAYSONTOP, MF_BYCOMMAND |
	              (gpSet->isAlwaysOnTop ? MF_CHECKED : MF_UNCHECKED));
	SetWindowPos(ghWnd, hwndAfter, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);

	if (ghOpWnd && gpSet->isAlwaysOnTop)
	{
		SetWindowPos(ghOpWnd, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);
		apiSetForegroundWindow(ghOpWnd);
	}
}

void CConEmuMain::OnDesktopMode()
{
	if (!this) return;

	if (WindowStartMinimized || ForceMinimizeToTray)
		return;

#ifndef CHILD_DESK_MODE
	DWORD dwStyleEx = GetWindowLong(ghWnd, GWL_EXSTYLE);
	DWORD dwNewStyleEx = dwStyleEx;
	DWORD dwStyle = GetWindowLong(ghWnd, GWL_STYLE);
	DWORD dwNewStyle = dwStyle;

	if (gpSet->isDesktopMode)
	{
		dwNewStyleEx |= WS_EX_TOOLWINDOW;
		dwNewStyle |= WS_POPUP;
	}
	else
	{
		dwNewStyleEx &= ~WS_EX_TOOLWINDOW;
		dwNewStyle &= ~WS_POPUP;
	}

	if (dwNewStyleEx != dwStyleEx || dwNewStyle != dwStyle)
	{
		SetWindowStyle(dwStyle);
		SetWindowStyleEx(dwStyleEx);
		SyncWindowToConsole(); // -- ������� ������, ������������
		UpdateWindowRgn();
	}

#endif
#ifdef CHILD_DESK_MODE
	HWND hDesktop = GetDesktopWindow();

	//HWND hProgman = FindWindowEx(hDesktop, NULL, L"Progman", L"Program Manager");
	//HWND hParent = NULL;gpSet->isDesktopMode ?  : GetDesktopWindow();

	OnTaskbarSettingsChanged();

	if (gpSet->isDesktopMode)
	{
		// Shell windows is FindWindowEx(hDesktop, NULL, L"Progman", L"Program Manager");
		HWND hShellWnd = GetShellWindow();
		DWORD dwShellPID = 0;

		if (hShellWnd)
			GetWindowThreadProcessId(hShellWnd, &dwShellPID);

		// But in Win7 it is not a real desktop holder :(
		if (gOSVer.dwMajorVersion >= 6)  // Vista too?
		{
			// � �����-�� ������� (�� �����-�� �����?) ������ ��������� � "Progman", � � ����� �� "WorkerW" �������
			// ��� ��� ���� ����������� ������ �������� explorer.exe
			HWND hShell = FindWindowEx(hDesktop, NULL, L"WorkerW", NULL);

			while(hShell)
			{
				// � ���� ������ ���� �������� ����
				if (IsWindowVisible(hShell) && FindWindowEx(hShell, NULL, NULL, NULL))
				{
					// ������������, ��� ���� ������ ������������ ������ �������� (Explorer.exe)
					if (dwShellPID)
					{
						DWORD dwTestPID;
						GetWindowThreadProcessId(hShell, &dwTestPID);

						if (dwTestPID != dwShellPID)
						{
							hShell = FindWindowEx(hDesktop, hShell, L"WorkerW", NULL);
							continue;
						}
					}

					break;
				}

				hShell = FindWindowEx(hDesktop, hShell, L"WorkerW", NULL);
			}

			if (hShell)
				hShellWnd = hShell;
		}

		if (gpSet->isQuakeStyle)  // ���� ����� � Desktop �����������
		{
			SetQuakeMode(0, (WindowMode == wmFullScreen) ? wmMaximized : wmNormal);
		}

		if (WindowMode == wmFullScreen)  // ���� ����� � Desktop �����������
		{
			SetWindowMode(wmMaximized);
		}

		if (!hShellWnd)
		{
			gpSet->isDesktopMode = false;

			HWND hExt = gpSetCls->mh_Tabs[gpSetCls->thi_Ext];

			if (ghOpWnd && hExt)
			{
				CheckDlgButton(hExt, cbDesktopMode, BST_UNCHECKED);
			}
		}
		else
		{
			mh_ShellWindow = hShellWnd;
			GetWindowThreadProcessId(mh_ShellWindow, &mn_ShellWindowPID);
			RECT rcWnd; GetWindowRect(ghWnd, &rcWnd);
			MapWindowPoints(NULL, mh_ShellWindow, (LPPOINT)&rcWnd, 2);
			//ShowWindow(SW_HIDE);
			//SetWindowPos(ghWnd, NULL, rcWnd.left,rcWnd.top,0,0, SWP_NOSIZE|SWP_NOZORDER);
			SetParent(mh_ShellWindow);
			SetWindowPos(ghWnd, NULL, rcWnd.left,rcWnd.top,0,0, SWP_NOSIZE|SWP_NOZORDER);
			SetWindowPos(ghWnd, HWND_TOPMOST, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE);
			//ShowWindow(SW_SHOW);
#ifdef _DEBUG
			RECT rcNow; GetWindowRect(ghWnd, &rcNow);
#endif
		}
	}

	if (!gpSet->isDesktopMode)
	{
		//dwStyle |= WS_POPUP;
		RECT rcWnd; GetWindowRect(ghWnd, &rcWnd);
		RECT rcVirtual = GetVirtualScreenRect(TRUE);
		SetWindowPos(ghWnd, NULL, max(rcWnd.left,rcVirtual.left),max(rcWnd.top,rcVirtual.top),0,0, SWP_NOSIZE|SWP_NOZORDER);
		SetParent(hDesktop);
		SetWindowPos(ghWnd, NULL, max(rcWnd.left,rcVirtual.left),max(rcWnd.top,rcVirtual.top),0,0, SWP_NOSIZE|SWP_NOZORDER);
		OnAlwaysOnTop();

		if (ghOpWnd && !gpSet->isAlwaysOnTop)
			apiSetForegroundWindow(ghOpWnd);
	}

	//SetWindowStyle(dwStyle);
#endif
}

INT_PTR CConEmuMain::aboutProc(HWND hWnd2, UINT messg, WPARAM wParam, LPARAM lParam)
{
	static struct {LPCWSTR Title; LPCWSTR Text;} Pages[] =
	{
		{L"About", pAbout},
		{L"Command line", pCmdLine},
		{L"Macro", pGuiMacro},
		{L"Console", pConsoleHelpFull},
		{L"DosBox", pDosBoxHelpFull},
		{L"Contributors", pAboutContributors},
		{L"License", pAboutLicense},
	};

	switch (messg)
	{
		case WM_INITDIALOG:
		{
			gpConEmu->mh_AboutDlg = hWnd2;

			if ((ghOpWnd && IsWindow(ghOpWnd)) || (WS_EX_TOPMOST & GetWindowLongPtr(ghWnd, GWL_EXSTYLE)))
			{
				SetWindowPos(hWnd2, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);
			}

			LPCWSTR pszActivePage = (LPCWSTR)lParam;

			WCHAR szTitle[255];
			LPCWSTR pszBits = WIN3264TEST(L"x86",L"x64");
			LPCWSTR pszDebug = L"";
			#ifdef _DEBUG
			pszDebug = L"[DEBUG] ";
			#endif
			_wsprintf(szTitle, SKIPLEN(countof(szTitle)) L"About ConEmu (%02u%02u%02u%s %s%s)", 
				(MVV_1%100),MVV_2,MVV_3,_T(MVV_4a), pszDebug, pszBits);
			SetWindowText(hWnd2, szTitle);

			if (hClassIcon)
			{
				SendMessage(hWnd2, WM_SETICON, ICON_BIG, (LPARAM)hClassIcon);
				SendMessage(hWnd2, WM_SETICON, ICON_SMALL, (LPARAM)hClassIconSm);
				SetClassLongPtr(hWnd2, GCLP_HICON, (LONG_PTR)hClassIcon);
			}

			SetDlgItemText(hWnd2, stConEmuAbout, pAboutTitle);
			SetDlgItemText(hWnd2, stConEmuUrl, gsHomePage);

			HWND hTab = GetDlgItem(hWnd2, tbAboutTabs);
			size_t nPage = 0;

			for (size_t i = 0; i < countof(Pages); i++)
			{
				TCITEM tie = {};
				tie.mask = TCIF_TEXT;
				tie.pszText = (LPWSTR)Pages[i].Title;
				TabCtrl_InsertItem(hTab, i, &tie);

				if (pszActivePage && (lstrcmpi(pszActivePage, Pages[i].Title) == 0))
					nPage = i;
			}

			SetDlgItemText(hWnd2, tAboutText, Pages[nPage].Text);

			if (nPage != 0)
			{
				TabCtrl_SetCurSel(hTab, (int)nPage);
			}
			else
			{
				_ASSERTE(pszActivePage==NULL && "Unknown page name?");
			}

			SetFocus(hTab);

			return FALSE;
		}

		case WM_CTLCOLORSTATIC:
			if (GetDlgItem(hWnd2, stConEmuUrl) == (HWND)lParam)
			{
				SetTextColor((HDC)wParam, GetSysColor(COLOR_HOTLIGHT));
				HBRUSH hBrush = GetSysColorBrush(COLOR_3DFACE);
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (INT_PTR)hBrush;
			}
			else
			{
				SetTextColor((HDC)wParam, GetSysColor(COLOR_WINDOWTEXT));
				HBRUSH hBrush = GetSysColorBrush(COLOR_3DFACE);
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (INT_PTR)hBrush;
			}
			break;

		case WM_SETCURSOR:
			{
				if (((HWND)wParam) == GetDlgItem(hWnd2, stConEmuUrl))
				{
					SetCursor(LoadCursor(NULL, IDC_HAND));
					SetWindowLongPtr(hWnd2, DWLP_MSGRESULT, TRUE);
					return TRUE;
				}
				return FALSE;
			}
			break;

		case WM_COMMAND:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				switch (LOWORD(wParam))
				{
					case IDOK:
					case IDCANCEL:
					case IDCLOSE:
						aboutProc(hWnd2, WM_CLOSE, 0, 0);
						return 1;
					case stConEmuUrl:
						gpConEmu->OnInfo_HomePage();
						return 1;
				}
			}
			break;

		case WM_NOTIFY:
		{
			LPNMHDR nmhdr = (LPNMHDR)lParam;
			if ((nmhdr->code == TCN_SELCHANGE) && (nmhdr->idFrom == tbAboutTabs))
			{
				int iPage = TabCtrl_GetCurSel(nmhdr->hwndFrom);
				if ((iPage >= 0) && (iPage < (int)countof(Pages)))
					SetDlgItemText(hWnd2, tAboutText, Pages[iPage].Text);
			}
			break;
		}

		case WM_CLOSE:
			//if (ghWnd == NULL)
			EndDialog(hWnd2, IDOK);
			//else
			//	DestroyWindow(hWnd2);
			break;

		case WM_DESTROY:
			gpConEmu->mh_AboutDlg = NULL;
			break;

		default:
			return FALSE;
	}

	return FALSE;
}

void CConEmuMain::OnInfo_About(LPCWSTR asPageName /*= NULL*/)
{
	InitCommCtrls();

	bool bOk = false;

	//if (ghWnd)
	//{
	//	HWND hAbout = NULL;
	//	if (mh_AboutDlg && IsWindow(mh_AboutDlg))
	//	{
	//		hAbout = mh_AboutDlg;
	//		apiShowWindow(hAbout, SW_SHOWNORMAL);
	//		SetFocus(hAbout);
	//	}
	//	else
	//	{
	//		hAbout = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_ABOUT), NULL, aboutProc);
	//	}
	//	bOk = (hAbout != NULL);
	//}
	//else
	{
		DontEnable de;
		HWND hParent = (ghOpWnd && IsWindowVisible(ghOpWnd)) ? ghOpWnd : ghWnd;
		// ���������?
		INT_PTR iRc = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_ABOUT), hParent, aboutProc, (LPARAM)asPageName);
		bOk = (iRc != 0 && iRc != -1);
	}

	if (!bOk)
	{
		WCHAR szTitle[255];
		LPCWSTR pszBits = WIN3264TEST(L"x86",L"x64");
		LPCWSTR pszDebug = L"";
		#ifdef _DEBUG
		pszDebug = L"[DEBUG] ";
		#endif
		_wsprintf(szTitle, SKIPLEN(countof(szTitle)) L"About ConEmu (%02u%02u%02u%s %s%s)", 
			(MVV_1%100),MVV_2,MVV_3,_T(MVV_4a), pszDebug, pszBits);
		DontEnable de;
		MSGBOXPARAMS mb = {sizeof(MSGBOXPARAMS), ghWnd, g_hInstance,
			pAbout,
			szTitle,
			MB_USERICON, MAKEINTRESOURCE(IMAGE_ICON), 0, NULL, LANG_NEUTRAL
		};
		MessageBoxIndirectW(&mb);
		//MessageBoxW(ghWnd, pHelp, szTitle, MB_ICONQUESTION);
	}
}

void CConEmuMain::OnInfo_Help()
{
	static HMODULE hhctrl = NULL;

	if (!hhctrl) hhctrl = GetModuleHandle(L"hhctrl.ocx");

	if (!hhctrl) hhctrl = LoadLibrary(L"hhctrl.ocx");

	if (hhctrl)
	{
		typedef BOOL (WINAPI* HTMLHelpW_t)(HWND hWnd, LPCWSTR pszFile, INT uCommand, INT dwData);
		HTMLHelpW_t fHTMLHelpW = (HTMLHelpW_t)GetProcAddress(hhctrl, "HtmlHelpW");

		if (fHTMLHelpW)
		{
			wchar_t szHelpFile[MAX_PATH*2];
			lstrcpy(szHelpFile, ms_ConEmuChm);
			//wchar_t* pszSlash = wcsrchr(szHelpFile, L'\\');
			//if (pszSlash) pszSlash++; else pszSlash = szHelpFile;
			//lstrcpy(pszSlash, L"ConEmu.chm");
			// lstrcat(szHelpFile, L::/Intro.htm");
			#define HH_HELP_CONTEXT 0x000F
			#define HH_DISPLAY_TOC  0x0001
			//fHTMLHelpW(NULL /*����� ���� �� �������������*/, szHelpFile, HH_HELP_CONTEXT, contextID);
			fHTMLHelpW(NULL /*����� ���� �� �������������*/, szHelpFile, HH_DISPLAY_TOC, 0);
		}
	}
}

void CConEmuMain::OnInfo_HomePage()
{
	DWORD shellRc = (DWORD)(INT_PTR)ShellExecute(ghWnd, L"open", gsHomePage, NULL, NULL, SW_SHOWNORMAL);
	if (shellRc <= 32)
	{
		DisplayLastError(L"ShellExecute failed", shellRc);
	}
}

void CConEmuMain::OnInfo_ReportBug()
{
	DWORD shellRc = (DWORD)(INT_PTR)ShellExecute(ghWnd, L"open", gsReportBug, NULL, NULL, SW_SHOWNORMAL);
	if (shellRc <= 32)
	{
		DisplayLastError(L"ShellExecute failed", shellRc);
	}
}

LRESULT CConEmuMain::OnSysCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
#ifdef _DEBUG
	wchar_t szDbg[128]; _wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"OnSysCommand (%i(0x%X), %i)\n", (DWORD)wParam, (DWORD)wParam, (DWORD)lParam);
	DEBUGSTRSIZE(szDbg);
	if (wParam == SC_HOTKEY)
	{
		_ASSERTE(wParam!=SC_HOTKEY);
	}
#endif
	LRESULT result = 0;

	if (wParam >= IDM_VCON_FIRST && wParam <= IDM_VCON_LAST)
	{
		int nNewV = ((int)HIWORD(wParam))-1;
		int nNewR = ((int)LOWORD(wParam))-1; UNREFERENCED_PARAMETER(nNewR);
		
		CVirtualConsole* pVCon = GetVCon(nNewV);
		if (pVCon)
		{
			// -- � SysMenu ������������ ������ ������� (����������/�������� ��� ���)
			//CRealConsole* pRCon = pVCon->RCon();
			//if (pRCon)
			//{
			//	//if (pRCon->CanActivateFarWindow(nNewR))
			//	pRCon->ActivateFarWindow(nNewR);
			//}
			if (!isActive(pVCon))
				Activate(pVCon);
			//else
			//	UpdateTabs();
		}
		return 0;
	}

	//switch(LOWORD(wParam))
	switch (wParam)
	{
		case ID_NEWCONSOLE:
			// ������� ����� �������
			RecreateAction(gpSet->GetDefaultCreateAction(), gpSet->isMultiNewConfirm || isPressed(VK_SHIFT));
			return 0;
			
		case IDM_ATTACHTO:
			if (!mp_AttachDlg)
				mp_AttachDlg = new CAttachDlg;
			mp_AttachDlg->AttachDlg();
			return 0;
			
		case ID_SETTINGS:
			CSettings::Dialog();
			return 0;
			
		case ID_CON_PASTE:
			{
				CVConGuard VCon;
				if (GetActiveVCon(&VCon) >= 0)
				{
					VCon->RCon()->Paste();
				}
			}
			return 0;

		case ID_CON_FIND:
			gpSetCls->FindTextDialog();
			return 0;

		case ID_CON_COPY:
			{
				CVConGuard VCon;
				if (GetActiveVCon(&VCon) >= 0)
				{
					VCon->RCon()->DoSelectionCopy();
				}
			}
			return 0;
			
		case ID_CON_MARKBLOCK:
		case ID_CON_MARKTEXT:
			{
				CVConGuard VCon;
				if (GetActiveVCon(&VCon) >= 0)
				{
					VCon->RCon()->StartSelection(LOWORD(wParam) == ID_CON_MARKTEXT);
				}
			}
			return 0;
			
		#ifdef SHOW_AUTOSCROLL
		case ID_AUTOSCROLL:
			gpSetCls->AutoScroll = !gpSetCls->AutoScroll;
			CheckMenuItem(gpConEmu->GetSysMenu(), ID_AUTOSCROLL, MF_BYCOMMAND |
			              (gpSetCls->AutoScroll ? MF_CHECKED : MF_UNCHECKED));
			return 0;
		#endif
			
		case ID_ALWAYSONTOP:
			{
				gpSet->isAlwaysOnTop = !gpSet->isAlwaysOnTop;
				OnAlwaysOnTop();

				HWND hExt = gpSetCls->mh_Tabs[gpSetCls->thi_Ext];

				if (ghOpWnd && hExt)
				{
					CheckDlgButton(hExt, cbAlwaysOnTop, gpSet->isAlwaysOnTop ? BST_CHECKED : BST_UNCHECKED);
				}
			}
			return 0;
			
		case ID_DUMPCONSOLE:
			{
				CVConGuard VCon;
				if (GetActiveVCon(&VCon) >= 0)
				{
					VCon->DumpConsole();
				}
			 }
			return 0;

		case ID_SCREENSHOT:
			CConEmuCtrl::MakeScreenshot();

			return 0;

		case ID_LOADDUMPCONSOLE:
			{
				CVConGuard VCon;
				if (GetActiveVCon(&VCon) >= 0)
				{
					VCon->LoadDumpConsole();
				}
			}
			return 0;
			
		case ID_DEBUGGUI:
			StartDebugLogConsole();
			return 0;
			
		case ID_DEBUGCON:
			StartDebugActiveProcess();
			return 0;
		case ID_MINIDUMP:
			MemoryDumpActiveProcess();
			return 0;
			
		//case ID_MONITOR_SHELLACTIVITY:
		//{
		//	CRealConsole* pRCon = mp_ VActive->RCon();

		//	if (pRCon)
		//		pRCon->LogShellStartStop();

		//	//if (!mb_CreateProcessLogged)
		//	//	StartLogCreateProcess();
		//	//else
		//	//	StopLogCreateProcess();
		//}
		//return 0;
		
		case ID_DEBUG_SHOWRECTS:
			gbDebugShowRects = !gbDebugShowRects;
			InvalidateAll();
			return 0;
			
		case ID_CON_TOGGLE_VISIBLE:
			{
				CVConGuard VCon;
				if (GetActiveVCon(&VCon) >= 0)
				{
					VCon->RCon()->ShowConsoleOrGuiClient(-1); // Toggle visibility
				}
			}
			return 0;
			
		case ID_HELP:
		{
			gpConEmu->OnInfo_Help();
			return 0;
		} // case ID_HELP:
		
		case ID_HOMEPAGE:
		{
			gpConEmu->OnInfo_HomePage();
			return 0;
		}
		
		case ID_REPORTBUG:
		{
			gpConEmu->OnInfo_ReportBug();
			return 0;
		}
		
		case ID_CHECKUPDATE:
			gpConEmu->CheckUpdates(TRUE);
			return 0;
		
		case ID_STOPUPDATE:
			if (gpUpd)
				gpUpd->StopChecking();
			return 0;
		
		case ID_HOTKEYS:
		{
			CSettings::Dialog(IDD_SPG_KEYS);
			return 0;
		}

		case ID_ABOUT:
		{
			gpConEmu->OnInfo_About();
			return 0;
		}

		case ID_TOMONITOR:
		{
			if (gpSet->isQuakeStyle || gpConEmu->m_InsideIntegration)
				return 0;
			if (!IsWindowVisible(ghWnd))
				Icon.RestoreWindowFromTray();
			POINT ptCur = {}; GetCursorPos(&ptCur);
			HMONITOR hMon = MonitorFromPoint(ptCur, MONITOR_DEFAULTTOPRIMARY);
			MONITORINFO mi = {sizeof(mi)};
			GetMonitorInfo(hMon, &mi);
			SetWindowPos(ghWnd, HWND_TOP, mi.rcWork.left, mi.rcWork.top, 0,0, SWP_NOSIZE);
			return 0;
		}

		case ID_TOTRAY:
			if (IsWindowVisible(ghWnd))
				Icon.HideWindowToTray();
			else
				Icon.RestoreWindowFromTray();

			return 0;
			
		case ID_CONPROP:
		{
			CVConGuard VCon;
			if ((GetActiveVCon(&VCon) >= 0) && VCon->RCon())
			{
				#ifdef MSGLOGGER
				{
					// ��� �������, ����������, ����� ������ ���� ���� � RealConsole
					HMENU hMenu = ::GetSystemMenu(VCon->RCon()->ConWnd(), FALSE);
					MENUITEMINFO mii; TCHAR szText[255];

					for(int i=0; i<15; i++)
					{
						memset(&mii, 0, sizeof(mii));
						mii.cbSize = sizeof(mii); mii.dwTypeData=szText; mii.cch=255;
						mii.fMask = MIIM_ID|MIIM_STRING|MIIM_SUBMENU;

						if (GetMenuItemInfo(hMenu, i, TRUE, &mii))
						{
							mii.cbSize = sizeof(mii);

							if (mii.hSubMenu)
							{
								MENUITEMINFO mic;

								for(int i=0; i<15; i++)
								{
									memset(&mic, 0, sizeof(mic));
									mic.cbSize = sizeof(mic); mic.dwTypeData=szText; mic.cch=255;
									mic.fMask = MIIM_ID|MIIM_STRING;

									if (GetMenuItemInfo(mii.hSubMenu, i, TRUE, &mic))
									{
										mic.cbSize = sizeof(mic);
									}
									else
									{
										break;
									}
								}
							}
						}
						else
							break;
					}
				}
				#endif
			
			    // Go!
				VCon->RCon()->ShowPropertiesDialog();
			}
			return 0;
		} // case ID_CONPROP:

		case SC_MAXIMIZE_SECRET:
			SetWindowMode(wmMaximized);
			break;
			
		case SC_RESTORE_SECRET:
			SetWindowMode(wmNormal);
			break;
			
		case SC_CLOSE:
			CVConGroup::OnScClose();
			break;
		
		case SC_MAXIMIZE:
		{
			DEBUGSTRSYS(L"OnSysCommand(SC_MAXIMIZE)\n");

			if (!mb_PassSysCommand)
			{
				#ifndef _DEBUG
				if (isPictureView())
					break;
				#endif
				
				SetWindowMode(wmMaximized);
			}
			else
			{
				result = DefWindowProc(hWnd, WM_SYSCOMMAND, wParam, lParam);
			}

			break;
		} // case SC_MAXIMIZE:
		
		case SC_RESTORE:
		{
			DEBUGSTRSYS(L"OnSysCommand(SC_RESTORE)\n");

			if (!mb_PassSysCommand)
			{
				#ifndef _DEBUG
				if (!isIconic() && isPictureView())
					break;
				#endif

				if (gpSet->isQuakeStyle)
				{
					gpConEmu->OnMinimizeRestore(sih_Show/*sih_HideTSA*/);
					break;
				}

				if (SetWindowMode(isIconic() ? WindowMode : wmNormal))
					break;
			}
			
			// ***
			{
				bool bIconic = ::IsIconic(hWnd);
				bool bPrev = isRestoreFromMinimized;
				if (bIconic)
					isRestoreFromMinimized = true;

				result = DefWindowProc(hWnd, WM_SYSCOMMAND, wParam, lParam);

				isRestoreFromMinimized = bPrev;
			}

			break;
		} // case SC_RESTORE:
		
		case SC_MINIMIZE:
		{
			DEBUGSTRSYS(L"OnSysCommand(SC_MINIMIZE)\n");

			bool bMin2TSA = gpSet->isMinToTray();

			if (!mb_InScMinimize)
			{
				mb_InScMinimize = true;

				// ��������, �� ����� �������� �� ���� �� ����������
				if (!isIconic())
				{
					mh_MinFromMonitor = MonitorFromWindow(ghWnd, MONITOR_DEFAULTTONEAREST);
				}

				// ���� "�����" � �������� Gui ���������� - ����� ����� �������� ConEmu "�������" ���
				CVConGuard VCon;
				if ((GetActiveVCon(&VCon) >= 0) && VCon->RCon()->GuiWnd())
				{
					apiSetForegroundWindow(ghWnd);
				}

				if (gpSet->isQuakeStyle)
				{
					gpConEmu->OnMinimizeRestore(bMin2TSA ? sih_HideTSA : sih_Minimize);
				}
				else if (bMin2TSA)
				{
					Icon.HideWindowToTray();
				}

				if (bMin2TSA)
				{
					// ������ ��� "��������", �������������� �� �����
					mb_InScMinimize = false;
					break;
				}

				result = DefWindowProc(hWnd, WM_SYSCOMMAND, wParam, lParam);

				mb_InScMinimize = false;
			}
			else
			{
				DEBUGSTRSYS(L"--OnSysCommand(SC_MINIMIZE) skipped, already in cycle\n");
			}
			
			break;
		} // case SC_MINIMIZE:
		
		default:
		{
			if (wParam >= IDM_VCONCMD_FIRST && wParam <= IDM_VCONCMD_LAST)
			{
				CVConGuard VCon;
				if (CVConGroup::GetActiveVCon(&VCon) >= 0)
					VCon->ExecPopupMenuCmd((int)(DWORD)wParam);
				result = 0; 
			}
			else if (wParam != 0xF100)
			{
				#ifdef _DEBUG
				wchar_t szDbg[64]; _wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"OnSysCommand(%i)\n", (DWORD)wParam);
				DEBUGSTRSYS(szDbg);
				#endif

				// ����� ������ SysCommand, ���������� � ConEmu, �������������� � RealConsole?
				#if 0
				// ����� ��� �������� � ������ ������ � ��������� ��������� �������,
				// ��������� ������� ����������, � ������ �����...
				if (wParam<0xF000)
				{
					POSTMESSAGE(ghConWnd, WM_SYSCOMMAND, wParam, lParam, FALSE);
				}
				#endif

				if (wParam == SC_SYSMENUPOPUP_SECRET)
				{
					mn_TrackMenuPlace = tmp_System;
					mp_Tip->HideTip();
				}

				result = DefWindowProc(hWnd, WM_SYSCOMMAND, wParam, lParam);

				if (wParam == SC_SYSMENUPOPUP_SECRET)
				{
					mn_TrackMenuPlace = tmp_None;
					mp_Tip->HideTip();
				}
			}
		} // default:
	}

	return result;
}

LRESULT CConEmuMain::OnTimer(WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

#ifdef _DEBUG
	wchar_t szDbg[128]; _wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"ConEmu:MainTimer(%u)\n", wParam);
	DEBUGSTRTIMER(szDbg);
#endif

	//if (mb_InTimer) return 0; // ����� ��������� ��� ���� � ���� ������� �� ����� (���� �� ������)
	mb_InTimer = TRUE;
	//result = gpConEmu->OnTimer(wParam, lParam);
#ifdef DEBUGSHOWFOCUS
	HWND hFocus = GetFocus();
	HWND hFore = GetForegroundWindow();
	static HWND shFocus, shFore;
	static wchar_t szWndInfo[1200];

	if (hFocus != shFocus)
	{
		_wsprintf(szWndInfo, SKIPLEN(countof(szWndInfo)) L"(Fore=0x%08X) Focus was changed to ", (DWORD)hFore);
		getWindowInfo(hFocus, szWndInfo+_tcslen(szWndInfo));
		wcscat(szWndInfo, L"\n");
		DEBUGSHOWFOCUS(szWndInfo);
		shFocus = hFocus;
	}

	if (hFore != shFore)
	{
		if (hFore != hFocus)
		{
			wcscpy(szWndInfo, L"Foreground window was changed to ");
			getWindowInfo(hFore, szWndInfo+_tcslen(szWndInfo));
			wcscat(szWndInfo, L"\n");
			DEBUGSHOWFOCUS(szWndInfo);
		}

		shFore = hFore;
	}

#endif

	CVirtualConsole* pVCon = NULL;
	CVConGuard VCon;
	if (GetActiveVCon(&VCon) >= 0)
		pVCon = VCon.VCon();

	switch(wParam)
	{
		case TIMER_MAIN_ID: // ������: 500 ��
		{
			//Maximus5. Hack - ���� �����-�� ������ ����������� ����
			if (!gbDontEnable)
			{
				DWORD dwStyle = GetWindowLong(ghWnd, GWL_STYLE);

				if (dwStyle & WS_DISABLED)
					EnableWindow(ghWnd, TRUE);
			}

			bool bForeground = isMeForeground();
			if (bForeground && !m_GuiInfo.bGuiActive)
			{
				UpdateGuiInfoMappingActive(true);
			}

			DWORD dwStyleEx = GetWindowLong(ghWnd, GWL_EXSTYLE);
			#ifdef CATCH_TOPMOST_SET
			static bool bWasTopMost = false;
			_ASSERTE((bWasTopMost || gpSet->isAlwaysOnTop || ((dwStyleEx & WS_EX_TOPMOST)==0)) && "TopMost mode was set (WM_TIMER)");
			bWasTopMost = ((dwStyleEx & WS_EX_TOPMOST)==WS_EX_TOPMOST);
			#endif
			if (!gpSet->isAlwaysOnTop && ((dwStyleEx & WS_EX_TOPMOST)==WS_EX_TOPMOST))
			{
				CheckTopMostState();
			}

			mp_Status->OnTimer();

			CheckProcesses();
			TODO("������ ��� ������� �� ��������. 1 - ������ ��� ��� ��� ConEmu.exe");

			if (m_ProcCount == 0)
			{
				// ��� ������� ������� ����������� ���������� ���� �� ����� ����� �������, ��� ��� ��������...
				if (mb_ProcessCreated)
				{
					OnAllVConClosed();
					break;
				}
			}
			else
			{
				//if (!mb_ProcessCreated && m_ProcCount>=1) --> OnRConStartedSuccess
				//{
				//	if ((GetTickCount() - mn_StartTick)>PROCESS_WAIT_START_TIME)
				//		mb_ProcessCreated = TRUE;
				//}

				if (!mb_WorkspaceErasedOnClose)
					mb_WorkspaceErasedOnClose = FALSE;
			}

			// TODO: ��������� SlideShow �������� �� ��������� ������
			BOOL lbIsPicView = isPictureView();

			if (bPicViewSlideShow)
			{
				DWORD dwTicks = GetTickCount();
				DWORD dwElapse = dwTicks - dwLastSlideShowTick;

				if (dwElapse > gpSet->nSlideShowElapse)
				{
					if (IsWindow(hPictureView) && pVCon)
					{
						//
						bPicViewSlideShow = false;
						SendMessage(pVCon->RCon()->ConWnd(), WM_KEYDOWN, VK_NEXT, 0x01510001);
						SendMessage(pVCon->RCon()->ConWnd(), WM_KEYUP, VK_NEXT, 0xc1510001);
						// ���� ����� ����������?
						isPictureView();
						dwLastSlideShowTick = GetTickCount();
						bPicViewSlideShow = true;
					}
					else
					{
						hPictureView = NULL;
						bPicViewSlideShow = false;
					}
				}
			}

			if (m_InsideIntegration)
			{
				InsideParentMonitor();
			}

			//2009-04-22 - ����� �� ���������
			/*if (lbIsPicView && !isPiewUpdate)
			{
			    // ����� ������������� ���������� ����� �������� PicView
			    isPiewUpdate = true;
			}*/

			if (!lbIsPicView && isPiewUpdate)
			{
				// ����� �������/�������� PictureView ����� ����������� ������� - �� ������ ��� ����������
				isPiewUpdate = false;
				SyncConsoleToWindow();
				//INVALIDATE(); //InvalidateRect(HDCWND, NULL, FALSE);
				InvalidateAll();
			}

			if (!isIconic())
			{
				// ��� ����� ����? �������� ����� ���������� ������ �����
				if ((mouse.state & MOUSE_SIZING_BEGIN) || mouse.bCheckNormalRect)
				{
					bool bIsSizing = isSizing();
					if (!bIsSizing)
					{
						StoreNormalRect(NULL); // ���� ����������, ����/�� ����
					}
				}
				//// ���� �� �������� ��������� ��������?
				//BOOL lbSizingToDo  = (mouse.state & MOUSE_SIZING_TODO) == MOUSE_SIZING_TODO;

				//if (isSizing() && !isPressed(VK_LBUTTON)) {
				//    // ����� ���� ������ ������� ������
				//    mouse.state &= ~(MOUSE_SIZING_BEGIN|MOUSE_SIZING_TODO);
				//}

				//TODO("�������� ���� ������ (����� SyncNtvdm?) ����� ��������� � ���� �������")
				//OnConsoleResize();

				// update scrollbar
				OnUpdateScrollInfo(TRUE);
			}
			else
			{
				if (mouse.bCheckNormalRect)
					mouse.bCheckNormalRect = false;
			}

			// ����� ������� ������� ���������
			if (gpSet->isCaptionHidden())
			{
				if (!bForeground)
				{
					if (m_ForceShowFrame)
					{
						StopForceShowFrame();
					}
				}
				else
				{
					// � Normal ������ ��� ��������� ����� ��� ������, ��� ������ ����
					// ��������� ��� ����� - �������� ��
					if (!isIconic() && (WindowMode == wmNormal))
					{
						TODO("�� ���������� �� � ���������� �������� ��� �������?");
						//static bool bPrevForceShow = false;
						bool bCurForceShow = isMouseOverFrame(true);

						if (bCurForceShow != (m_ForceShowFrame != fsf_Hide))
						{
							m_ForceShowFrame = bCurForceShow ? fsf_WaitShow : fsf_Hide;
							KillTimer(ghWnd, TIMER_CAPTION_APPEAR_ID);
							KillTimer(ghWnd, TIMER_CAPTION_DISAPPEAR_ID);
							WORD nID = bCurForceShow ? TIMER_CAPTION_APPEAR_ID : TIMER_CAPTION_DISAPPEAR_ID;
							DWORD nDelay = bCurForceShow ? gpSet->nHideCaptionAlwaysDelay : gpSet->nHideCaptionAlwaysDisappear;

							// ���� ������� ���������� � ��������� - �� �� �������
							if (nDelay)
								SetTimer(ghWnd, nID, nDelay, NULL);
							else if (bCurForceShow)
								StartForceShowFrame();
							else
								StopForceShowFrame();
						}
					}
				}
			}

			if (pVCon)
			{
				bool bLastFade = pVCon->mb_LastFadeFlag;
				bool bNewFade = (gpSet->isFadeInactive && !bForeground && !lbIsPicView);

				// ��� ������� ������ ����� ������� �� ����������, �.�.
				// ����� Fade �������������� � WM_ACTIVATE/WM_SETFOCUS/WM_KILLFOCUS
				if (bLastFade != bNewFade)
				{
					pVCon->mb_LastFadeFlag = bNewFade;
					Invalidate(pVCon);
				}
			}

			if (mh_ConEmuAliveEvent && !mb_ConEmuAliveOwned)
				isFirstInstance(); // ������ � ��������...

			// ���� ��� ������� ���� background
			if (gpSetCls->PollBackgroundFile())
			{
				gpConEmu->Update(true);
			}

			if (mn_TrackMenuPlace != tmp_None && mp_Tip)
			{
				POINT ptCur; GetCursorPos(&ptCur);
				HWND hPoint = WindowFromPoint(ptCur);
				if (hPoint)
				{
					#if 0
					wchar_t szWinInfo[1024];
					_wsprintf(szWinInfo, SKIPLEN(countof(szWinInfo)) L"WindowFromPoint(%i,%i) ", ptCur.x, ptCur.y);
					OutputDebugString(szWinInfo);
					getWindowInfo(hPoint, szWinInfo);
					wcscat_c(szWinInfo, L"\n");
					OutputDebugString(szWinInfo);
					#endif

					wchar_t szClass[128];
					if (GetClassName(hPoint, szClass, countof(szClass))
						&& (lstrcmp(szClass, VirtualConsoleClass) == 0 || lstrcmp(szClass, VirtualConsoleClassMain) == 0))
					{
						mp_Tip->HideTip();
					}
				}
			}

			// -- ������ �� OnFocus
			//CheckFocus(L"TIMER_MAIN_ID");
			// ���������, ����� ConEmu ��� �����������, � ������ ���?
			OnFocus(NULL, 0, 0, 0, L"TIMER_MAIN_ID");

			if (!lbIsPicView && gpSet->UpdSet.isUpdateCheckHourly)
			{
				gpSet->UpdSet.CheckHourlyUpdate();
			}

		} break; // case 0:
		case TIMER_CONREDRAW_ID: // ������: CON_REDRAW_TIMOUT*2
		{
			if (pVCon && !isIconic())
			{
				pVCon->CheckPostRedraw();
			}
		} break; // case 1:
		case TIMER_CAPTION_APPEAR_ID:
		case TIMER_CAPTION_DISAPPEAR_ID:
		{
			KillTimer(ghWnd, wParam);
			if (isMouseOverFrame(true))
				StartForceShowFrame();
			else
				StopForceShowFrame();
		} break;
		case TIMER_RCLICKPAINT:
		{
			RightClickingPaint(NULL, NULL);
		} break;
		case TIMER_ADMSHIELD_ID:
		{
			static int nStep = 0;
			if (gpSet->isTaskbarShield)
			{
				Taskbar_UpdateOverlay();
				nStep++;
			}
			if ((nStep >= 5) || !gpSet->isTaskbarShield)
			{
				KillTimer(ghWnd, TIMER_ADMSHIELD_ID);
			}
		} break;
	}

	mb_InTimer = FALSE;
	return result;
}

void CConEmuMain::OnTransparent(bool abFromFocus /*= false*/, bool bSetFocus /*= true*/)
{
	bool bForceLayered = false;
	bool bActive = abFromFocus ? bSetFocus : isMeForeground();
	bool bColorKey = gpSet->isColorKeyTransparent;
	UINT nAlpha = (bActive || !gpSet->isTransparentSeparate) ? gpSet->nTransparent : gpSet->nTransparentInactive;
	nAlpha = max((UINT)(bActive?MIN_ALPHA_VALUE:MIN_INACTIVE_ALPHA_VALUE),(UINT)min(nAlpha,255));

	if (((gpSet->nTransparent < 255)
			|| (gpSet->isTransparentSeparate && (gpSet->nTransparentInactive < 255))
			|| bColorKey)
		&& !gpSet->isTransparentAllowed())
	{
		nAlpha = 255; bColorKey = false;
	}
	else if (gpSet->isTransparentSeparate && (nAlpha == 255)
		&& ((gpSet->nTransparent < 255) || (gpSet->nTransparentInactive < 255)))
	{
		bForceLayered = true;
	}

	TODO("EnableBlurBehind: tabs and toolbar must be rewritten, all parts of GUI must be drawn with Alpha channel");
#ifdef _DEBUG
	EnableBlurBehind((nAlpha < 255));
#endif

	// return true - when state was changes
	if (SetTransparent(ghWnd, nAlpha, bColorKey, gpSet->nColorKeyValue, bForceLayered))
	{
		if (mn_TrackMenuPlace == tmp_None)
		{
			OnSetCursor();
		}
	}
}

// return true - when state was changes
bool CConEmuMain::SetTransparent(HWND ahWnd, UINT anAlpha/*0..255*/, bool abColorKey /*= false*/, COLORREF acrColorKey /*= 0*/, bool abForceLayered /*= false*/)
{
	#ifdef __GNUC__
	if (!SetLayeredWindowAttributes)
	{
		_ASSERTE(SetLayeredWindowAttributes!=NULL);
		return false;
	}
	#endif

	BOOL bNeedRedrawOp = FALSE;
	// ��� �� ��������� �� Active/Inactive, �� ��� ����� ���������.
	// �������� ��� ������� � OnTransparent
	UINT nTransparent = max(MIN_INACTIVE_ALPHA_VALUE,min(anAlpha,255));
	DWORD dwExStyle = GetWindowLongPtr(ahWnd, GWL_EXSTYLE);
	bool lbChanged = false;

	if (ahWnd == ghWnd)
		mp_Status->OnTransparency();

	if ((nTransparent >= 255) && !abColorKey && !abForceLayered)
	{
		// ������������ ����������� (��������� ������������)
		//SetLayeredWindowAttributes(ahWnd, 0, 255, LWA_ALPHA);
		if ((dwExStyle & WS_EX_LAYERED) == WS_EX_LAYERED)
		{
			dwExStyle &= ~WS_EX_LAYERED;
			SetLayeredWindowAttributes(ahWnd, 0, 255, LWA_ALPHA);
			SetWindowStyleEx(ahWnd, dwExStyle);
			lbChanged = true;
		}
	}
	else
	{
		if ((dwExStyle & WS_EX_LAYERED) == 0)
		{
			dwExStyle |= WS_EX_LAYERED;
			SetWindowStyleEx(ahWnd, dwExStyle);
			bNeedRedrawOp = TRUE;
			lbChanged = true;
		}

		DWORD nNewFlags = (((nTransparent < 255) || abForceLayered) ? LWA_ALPHA : 0) | (abColorKey ? LWA_COLORKEY : 0);

		BYTE nCurAlpha = 0;
		DWORD nCurFlags = 0;
		COLORREF nCurColorKey = 0;

		if (lbChanged
			|| (!_GetLayeredWindowAttributes || !(_GetLayeredWindowAttributes(ahWnd, &nCurColorKey, &nCurAlpha, &nCurFlags)))
			|| (nCurAlpha != nTransparent) || (nCurFlags != nNewFlags)
			|| (abColorKey && (nCurColorKey != acrColorKey)))
		{
			lbChanged = true;
			SetLayeredWindowAttributes(ahWnd, acrColorKey, nTransparent, nNewFlags);
		}

		// ����� ����� ����� (�� ���� ����� - ��������� �����) ���������� ���� "��������� ������"
		// � ������������� ����������������. ���� � ���� ������ ����� ������ �������� - �� ����������.
		if (bNeedRedrawOp)
		{
			HWND hWindows[] = {ghOpWnd, (mp_AttachDlg ? mp_AttachDlg->GetHWND() : NULL), gpSetCls->mh_FindDlg};
			for (size_t i = 0; i < countof(hWindows); i++)
			{
				if (hWindows[i] && (hWindows[i] != ahWnd) && IsWindow(hWindows[i]))
				{
					// Ask the window and its children to repaint
					RedrawWindow(hWindows[i], NULL, NULL, RDW_ERASE|RDW_INVALIDATE|RDW_FRAME|RDW_ALLCHILDREN);
				}
			}
		}
	}

	return lbChanged;
}

// ������� UpdateScrollPos ��� �������� �������
LRESULT CConEmuMain::OnUpdateScrollInfo(BOOL abPosted/* = FALSE*/)
{
	if (!abPosted)
	{
		PostMessage(ghWnd, mn_MsgUpdateScrollInfo, 0, 0);
		return 0;
	}

	CVConGroup::OnUpdateScrollInfo();

	return 0;
}

// ����� ��� �������� ������ ������� �� ������ ����� ����� ���� ���-�� ����������
void CConEmuMain::OnVConCreated(CVirtualConsole* apVCon, const RConStartArgs *args)
{
	mb_CloseGuiConfirmed = false; // �����

	CVConGroup::OnVConCreated(apVCon, args);
}

void CConEmuMain::OnAllVConClosed()
{
	ShutdownGuiStep(L"AllVConClosed");

	OnAllGhostClosed();

	Taskbar_SetShield(false);

	if (!gpSet->isMultiLeaveOnClose)
	{
		Destroy();
	}
	else if (!mb_WorkspaceErasedOnClose)
	{
		mb_WorkspaceErasedOnClose = TRUE;
		UpdateTitle();
		InvalidateAll();
	}
}

void CConEmuMain::OnGhostCreated(CVirtualConsole* apVCon, HWND ahGhost)
{
	// ��� ��������� �� �������� ������ ������ � �������� (WinXP � ����)
	// ����� ������ � ���� ������ ������ ����������
	// 111127 �� Vista ���� ������ "�������" �����
	if (gpSet->isTabsOnTaskBar() && !IsWindows7)
	{
		DWORD curStyle = GetWindowLong(ghWnd, GWL_STYLE);
		DWORD curStyleEx = GetWindowLong(ghWnd, GWL_EXSTYLE);
		DWORD style = (curStyle | WS_POPUP);
		DWORD styleEx = (curStyleEx & ~WS_EX_APPWINDOW);
		if (style != curStyle || styleEx != curStyleEx)
		{
			SetWindowStyle(style);
			SetWindowStyleEx(styleEx);
		}
		Taskbar_DeleteTabXP(ghWnd);
	}
}

void CConEmuMain::OnAllGhostClosed()
{
	if (!gpSet->isWindowOnTaskBar(true))
		return;

	DWORD curStyle = GetWindowLong(ghWnd, GWL_STYLE);
	DWORD curStyleEx = GetWindowLong(ghWnd, GWL_EXSTYLE);
	DWORD style = (curStyle & ~WS_POPUP);
	DWORD styleEx = (curStyleEx | WS_EX_APPWINDOW);
	if (style != curStyle || styleEx != curStyleEx)
	{
		SetWindowStyle(style);
		SetWindowStyleEx(styleEx);

		Taskbar_AddTabXP(ghWnd);
	}
}

void CConEmuMain::OnRConStartedSuccess(CRealConsole* apRCon)
{
	// Note, apRCon MAY be NULL
	mb_ProcessCreated = TRUE;
}

LRESULT CConEmuMain::OnVConClosed(CVirtualConsole* apVCon, BOOL abPosted /*= FALSE*/)
{
	_ASSERTE(apVCon);

	if (!apVCon)
		return 0;

	if (!abPosted)
	{
		ShutdownGuiStep(L"OnVConClosed - repost");
		PostMessage(ghWnd, mn_MsgVConTerminated, 0, (LPARAM)apVCon);
		return 0;
	}

	CVConGroup::OnVConClosed(apVCon);

	// ������ ������������ ��������� (���� ����� ���� ��������� � � ��������� ������������ ���������� ��������)
	UpdateTitle(); // ��� ����������
	//
	mp_TabBar->Update(); // ����� �� ����� ��������� ��������
	// � ������ ����� �������� �������� ��������
	int nActiveConNum = ActiveConNum();
	mp_TabBar->OnConsoleActivated(nActiveConNum/*, FALSE*/);
	// StatusBar
	CVConGuard VCon;
	mp_Status->OnActiveVConChanged(nActiveConNum, (GetActiveVCon(&VCon) >= 0) ? VCon->RCon() : NULL);

	ShutdownGuiStep(L"OnVConClosed - done");

	// ����������� ������� ������, � �� GUI ����� ������, ���� �� ����� ������� ��� �� ��������
	//if (mp_VCon[0] == NULL)
	if (!GetVCon(0))
		OnTimer(TIMER_MAIN_ID, 0);

	return 0;
}

//void CConEmuMain::PostSetBackground(CVirtualConsole* apVCon, CESERVER_REQ_SETBACKGROUND* apImgData)
//{
//	PostMessage(ghWnd, mn_MsgPostSetBackground, (WPARAM)apVCon, (LPARAM)apImgData);
//}

void CConEmuMain::LogWindowPos(LPCWSTR asPrefix)
{
	if (gpSetCls->isAdvLogging)
	{
		wchar_t szInfo[255];
		RECT rcWnd = {}; GetWindowRect(ghWnd, &rcWnd);
		MONITORINFO mi;
		HMONITOR hMon = GetNearestMonitor(&mi);
		_wsprintf(szInfo, SKIPLEN(countof(szInfo)) L"%s: %s %s WindowMode=%u Rect={%i,%i}-{%i,%i} Mon(x%08X)=({%i,%i}-{%i,%i}),({%i,%i}-{%i,%i})",
			asPrefix ? asPrefix : L"WindowPos",
			::IsWindowVisible(ghWnd) ? L"Visible" : L"Hidden",
			::IsIconic(ghWnd) ? L"Iconic" : ::IsZoomed(ghWnd) ? L"Maximized" : L"Normal",
			gpSet->isQuakeStyle ? gpSet->_WindowMode : WindowMode,
			rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom,
			(DWORD)hMon, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right, mi.rcMonitor.bottom,
			mi.rcWork.left, mi.rcWork.top, mi.rcWork.right, mi.rcWork.bottom);
		LogString(szInfo);
	}
}

void CConEmuMain::LogMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!this || !mp_Log || (gpSetCls->isAdvLogging < 4))
		return;

	char szLog[128];
	#define CASE_MSG(x) case x: lstrcpynA(szLog, #x, 64); break
	switch (uMsg)
	{
	CASE_MSG(WM_CREATE);
	CASE_MSG(WM_SHOWWINDOW);
	CASE_MSG(WM_PAINT);
	CASE_MSG(WM_SETHOTKEY);
	CASE_MSG(WM_WINDOWPOSCHANGING);
	CASE_MSG(WM_WINDOWPOSCHANGED);
	CASE_MSG(WM_SIZE);
	CASE_MSG(WM_SIZING);
	CASE_MSG(WM_MOVE);
	CASE_MSG(WM_MOVING);
	CASE_MSG(WM_MOUSEMOVE);
	CASE_MSG(WM_MOUSEWHEEL);
	CASE_MSG(WM_LBUTTONDOWN);
	CASE_MSG(WM_LBUTTONUP);
	CASE_MSG(WM_LBUTTONDBLCLK);
	CASE_MSG(WM_RBUTTONDOWN);
	CASE_MSG(WM_RBUTTONUP);
	CASE_MSG(WM_RBUTTONDBLCLK);
	CASE_MSG(WM_KEYDOWN);
	CASE_MSG(WM_KEYUP);
	CASE_MSG(WM_CHAR);
	CASE_MSG(WM_DEADCHAR);
	CASE_MSG(WM_SYSKEYDOWN);
	CASE_MSG(WM_SYSKEYUP);
	CASE_MSG(WM_SYSCHAR);
	CASE_MSG(WM_SYSDEADCHAR);
	CASE_MSG(WM_COMMAND);
	CASE_MSG(WM_SYSCOMMAND);
	CASE_MSG(WM_TIMER);
	CASE_MSG(WM_VSCROLL);
	default:
		_wsprintfA(szLog, SKIPLEN(countof(szLog)) "Msg%04X(%u)", uMsg, uMsg);
	}
	#undef CASE_MSG

	size_t cchLen = strlen(szLog);
	_wsprintfA(szLog+cchLen, SKIPLEN(countof(szLog)-cchLen)
		": HWND=x%08X," WIN3264TEST(" W=x%08X, L=x%08X", " W=x%08X%08X, L=x%08X%08X"),
		(DWORD)hWnd, WIN3264WSPRINT(wParam), WIN3264WSPRINT(lParam));

	LogString(szLog);
}

LRESULT CConEmuMain::MainWndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	//if (messg == WM_CREATE)
	//{
	if (ghWnd == NULL)
		ghWnd = hWnd; // ������ �����, ����� ������� ����� ������������
	//else if ('ghWnd DC' == NULL)
	//	'ghWnd DC' = hWnd; // ������ �����, ����� ������� ����� ������������
	//}

	if (hWnd == ghWnd)
		result = gpConEmu->WndProc(hWnd, messg, wParam, lParam);
	else //if (hWnd == 'ghWnd DC')
		result = CConEmuChild::ChildWndProc(hWnd, messg, wParam, lParam);
	//else if (messg)
	//	result = DefWindowProc(hWnd, messg, wParam, lParam);

	return result;
}

BOOL CConEmuMain::isDialogMessage(MSG &Msg)
{
	BOOL lbDlgMsg = FALSE;
	_ASSERTE(this!=NULL && this==gpConEmu);

	HWND hDlg = NULL;

	if (ghOpWnd)
	{
		if (IsWindow(ghOpWnd))
			lbDlgMsg = IsDialogMessage(ghOpWnd, &Msg);
	}

	if (!lbDlgMsg && mp_AttachDlg && (hDlg = mp_AttachDlg->GetHWND()))
	{
		if (IsWindow(hDlg))
			lbDlgMsg = IsDialogMessage(hDlg, &Msg);
	}

	if (!lbDlgMsg && (hDlg = gpSetCls->mh_FindDlg))
	{
		if (IsWindow(hDlg))
			lbDlgMsg = IsDialogMessage(hDlg, &Msg);
	}

	return lbDlgMsg;
}

LPCWSTR CConEmuMain::MenuAccel(int DescrID, LPCWSTR asText)
{
	if (!asText || !*asText)
	{
		_ASSERTE(asText!=NULL);
		return L"";
	}

	static wchar_t szTemp[255];
	wchar_t szKey[128] = {};
	
	const ConEmuHotKey* pHK = NULL;
	DWORD VkMod = gpSet->GetHotkeyById(DescrID, &pHK);
	if (!gpSet->GetHotkey(VkMod) || !pHK)
		return asText;

	gpSet->GetHotkeyName(pHK, szKey);
	if (!*szKey)
		return asText;
	int nLen = lstrlen(szKey);
	lstrcpyn(szTemp, asText, countof(szTemp)-nLen-4);
	wcscat_c(szTemp, L"\t");
	wcscat_c(szTemp, szKey);

	return szTemp;
}

// Window procedure for ghWndWork
LRESULT CConEmuMain::WorkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	_ASSERTE(ghWndWork==NULL || ghWndWork==hWnd);

	// Logger
	MSG msgStr = {hWnd, uMsg, wParam, lParam};
	ConEmuMsgLogger::Log(msgStr, ConEmuMsgLogger::msgWork);

	if (gpSetCls->isAdvLogging >= 4)
	{
		gpConEmu->LogMessage(hWnd, uMsg, wParam, lParam);
	}

	switch (uMsg)
	{
	case WM_CREATE:
		if (!ghWndWork)
			ghWndWork = hWnd;
		_ASSERTE(ghWndWork == hWnd);
		break;

	case WM_PAINT:
		{
			// �� ����, ������� ������ ghWndWork ���� �� ������, �� ���� ���� ���� - ������
			PAINTSTRUCT ps = {};
			BeginPaint(hWnd, &ps);

			CVConGroup::PaintGaps(ps.hdc);

			//int nAppId = -1;
			//CVConGuard VCon;
			//if ((gpConEmu->GetActiveVCon(&VCon) >= 0) && VCon->RCon())
			//{
			//	nAppId = VCon->RCon()->GetActiveAppSettingsId();
			//}

			//int nColorIdx = RELEASEDEBUGTEST(0/*Black*/,1/*Blue*/);
			//HBRUSH hBrush = CreateSolidBrush(gpSet->GetColors(nAppId, !gpConEmu->isMeForeground())[nColorIdx]);
			//if (hBrush)
			//{
			//	FillRect(ps.hdc, &ps.rcPaint, hBrush);

			//	DeleteObject(hBrush);
			//}

			EndPaint(hWnd, &ps);
		} // WM_PAINT
		break;

	default:
		result = DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return result;
}

// Window procedure for ghWnd
// BUT! It may be called from child windows, e.g. for keyboard processing
LRESULT CConEmuMain::WndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	//MCHKHEAP

	// Logger
	MSG msgStr = {hWnd, messg, wParam, lParam};
	ConEmuMsgLogger::Log(msgStr, ConEmuMsgLogger::msgMain);

	if (gpSetCls->isAdvLogging >= 4)
	{
		LogMessage(hWnd, messg, wParam, lParam);
	}

	#ifdef _DEBUG
	if (messg == WM_TIMER || messg == WM_GETICON)
	{
		bool lbDbg1 = false;
	}
	else if (messg >= WM_APP)
	{
		bool lbDbg2 = false;
	}
	else
	{
		wchar_t szDbg[127]; _wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"WndProc(%i{x%03X},%i,%i)\n", messg, messg, (DWORD)wParam, (DWORD)lParam);
		DEBUGSTRMSG(szDbg);
	}
	#endif

	if (messg == WM_SYSCHAR)  // ��� ��������� � ������� �� ������������, �� ����� �� ������ - ����������
		return TRUE;

	if (gpConEmu->ProcessNcMessage(hWnd, messg, wParam, lParam, result))
		return result;

	if (gpConEmu->ProcessGestureMessage(hWnd, messg, wParam, lParam, result))
		return result;
		
	//if (messg == WM_CHAR)
	//  return TRUE;

	switch (messg)
	{
		case WM_CREATE:
			result = gpConEmu->OnCreate(hWnd, (LPCREATESTRUCT)lParam);
			break;

		case WM_SETHOTKEY:
			gnWndSetHotkeyOk = wParam;
			result = ::DefWindowProc(hWnd, messg, wParam, lParam);
			break;

		case WM_NOTIFY:
		{
			if (gpConEmu->mp_TabBar)
				result = gpConEmu->mp_TabBar->OnNotify((LPNMHDR)lParam);

			break;
		}
		case WM_COMMAND:
		{
			if (gpConEmu->mp_TabBar)
				gpConEmu->mp_TabBar->OnCommand(wParam, lParam);

			result = 0;
			break;
		}
		case WM_INITMENUPOPUP:
		{
			return gpConEmu->OnInitMenuPopup(hWnd, (HMENU)wParam, lParam);
		}
		case WM_MENUSELECT:
		{
			switch (gpConEmu->mn_TrackMenuPlace)
			{
			case tmp_Cmd:
				if (gpConEmu->mp_TabBar)
					gpConEmu->mp_TabBar->OnMenuSelected((HMENU)lParam, LOWORD(wParam), HIWORD(wParam));
				return 0;
			case tmp_System:
			case tmp_VCon:
				gpConEmu->ShowMenuHint((HMENU)lParam, LOWORD(wParam), HIWORD(wParam));
				return 0;
			case tmp_KeyBar:
				gpConEmu->ShowKeyBarHint((HMENU)lParam, LOWORD(wParam), HIWORD(wParam));
				return 0;
			case tmp_StatusBarCols:
				gpConEmu->mp_Status->ProcessMenuHighlight((HMENU)lParam, LOWORD(wParam), HIWORD(wParam));
				return 0;
			case tmp_None:
				break;
			default:
				; // Take no action
			}
			// Else ...
			return 0;
		}
		case WM_MENURBUTTONUP:
		{
			if (gpConEmu->mn_TrackMenuPlace == tmp_Cmd)
			{
				gpConEmu->mp_TabBar->OnNewConPopupMenuRClick((HMENU)lParam, (UINT)wParam);
			}
			return 0;
		}
		case WM_ERASEBKGND:
			//return 0;
			return 1; //2010-10-05
		//case WM_NCPAINT:
		//case WM_NCACTIVATE:
		//case WM_NCCALCSIZE:
		//case WM_NCHITTEST:
		//case 0x31E: // WM_DWMCOMPOSITIONCHANGED:
		//case 0xAE: // WM_NCUAHDRAWCAPTION:
		//case 0xAF: // WM_NCUAHDRAWFRAME:
		//	//result = gpConEmu->OnNcPaint((HRGN)wParam);
		//	result = gpConEmu->OnNcMessage(hWnd, messg, wParam, lParam);
		//	break;
		//case WM_PAINT:
		//	result = gpConEmu->OnPaint(wParam, lParam);
		//	break;
		case WM_PRINTCLIENT:
			DefWindowProc(hWnd, messg, wParam, lParam);
			if (wParam && (hWnd == ghWnd) && gpSet->isStatusBarShow && (lParam & PRF_CLIENT))
			{
				int nHeight = gpSet->StatusBarHeight();
				RECT wr = CalcRect(CER_MAINCLIENT);
				RECT rcStatus = {wr.left, wr.bottom - nHeight, wr.right, wr.bottom};
				mp_Status->PaintStatus((HDC)wParam, rcStatus);
			}
			break;
		case WM_TIMER:
			result = gpConEmu->OnTimer(wParam, lParam);
			break;
		case WM_SIZING:
		{
			RECT* pRc = (RECT*)lParam;
			wchar_t szDbg[128]; _wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"WM_SIZING (Edge%i, {%i-%i}-{%i-%i})\n", (DWORD)wParam, pRc->left, pRc->top, pRc->right, pRc->bottom);

			if (!isIconic())
				result = gpConEmu->OnSizing(wParam, lParam);

			size_t cchLen = wcslen(szDbg);
			_wsprintf(szDbg+cchLen, SKIPLEN(countof(szDbg)-cchLen) L" -> ({%i-%i}-{%i-%i})\n", pRc->left, pRc->top, pRc->right, pRc->bottom);
			LogString(szDbg, true, false);
			DEBUGSTRSIZE(szDbg);
		} break;

		case WM_MOVING:
		{
			RECT* pRc = (RECT*)lParam;
			if (pRc)
			{
				wchar_t szDbg[128]; _wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"WM_MOVING ({%i-%i}-{%i-%i})", pRc->left, pRc->top, pRc->right, pRc->bottom);

				result = gpConEmu->OnMoving(pRc, true);

				size_t cchLen = wcslen(szDbg);
				_wsprintf(szDbg+cchLen, SKIPLEN(countof(szDbg)-cchLen) L" -> ({%i-%i}-{%i-%i})", pRc->left, pRc->top, pRc->right, pRc->bottom);
				LogString(szDbg, true, false);
				DEBUGSTRSIZE(szDbg);
			}
		} break;

		case WM_SHOWWINDOW:
		{
			wchar_t szDbg[128]; _wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"WM_SHOWWINDOW (Show=%i, Status=%i)\r\n", (DWORD)wParam, (DWORD)lParam);
			LogString(szDbg, true, false);
			DEBUGSTRSIZE(szDbg);
			result = DefWindowProc(hWnd, messg, wParam, lParam);
			if (wParam == FALSE)
			{
				if (!Icon.isWindowInTray() && !Icon.isHidingToTray())
				{
					//_ASSERTE(Icon.isHidingToTray());
					Icon.HideWindowToTray(gpSet->isDownShowHiddenMessage ? NULL : _T("ConEmu was hidden from some program"));
					gpConEmu->mb_ExternalHidden = TRUE;
				}
			}
			else
			{
				if (gpConEmu->mb_ExternalHidden)
					Icon.RestoreWindowFromTray(TRUE);
				gpConEmu->mb_ExternalHidden = FALSE;
			}

			//�����������, ��� ����������
			if (gpSetCls->isAdvLogging)
			{
				LogWindowPos(L"WM_SHOWWINDOW.end");
			}
		} break;

		case WM_SIZE:
		{
			#ifdef _DEBUG
			DWORD_PTR dwStyle = GetWindowLong(ghWnd, GWL_STYLE);
			wchar_t szDbg[128]; _wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"WM_SIZE (Type:x%X, {%i, %i}) style=0x%08X\n", (DWORD)wParam, LOWORD(lParam), HIWORD(lParam), (DWORD)dwStyle);
			DEBUGSTRSIZE(szDbg);
			#endif

			//if (gpSet->isDontMinimize && wParam == SIZE_MINIMIZED) {
			//	result = 0;
			//	break;
			//}
			if (!isIconic())
			{
				result = gpConEmu->OnSize(HIWORD(wParam)!=2, LOWORD(wParam), LOWORD(lParam), HIWORD(lParam));
			}
		} break;
		case WM_MOVE:
		{
#ifdef _DEBUG
			wchar_t szDbg[128]; _wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"WM_MOVE ({%i, %i})\n", (int)(SHORT)LOWORD(lParam), (int)(SHORT)HIWORD(lParam));
			DEBUGSTRSIZE(szDbg);
#endif
		} break;
		case WM_WINDOWPOSCHANGING:
		{
			result = gpConEmu->OnWindowPosChanging(hWnd, messg, wParam, lParam);
		} break;
		//case WM_NCCALCSIZE:
		//	{
		//		NCCALCSIZE_PARAMS *pParms = NULL;
		//		LPRECT pRect = NULL;
		//		if (wParam) pParms = (NCCALCSIZE_PARAMS*)lParam; else pRect = (LPRECT)lParam;
		//		result = DefWindowProc(hWnd, messg, wParam, lParam);
		//		break;
		//	}
		case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO pInfo = (LPMINMAXINFO)lParam;
			result = gpConEmu->OnGetMinMaxInfo(pInfo);
			break;
		}
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
			//case WM_CHAR: -- �����. ������ �������� ToUnicodeEx.
			//case WM_SYSCHAR:
			result = gpConEmu->OnKeyboard(hWnd, messg, wParam, lParam);

			if (messg == WM_SYSKEYUP || messg == WM_SYSKEYDOWN)
				result = TRUE;

			//if (messg == WM_SYSCHAR)
			//    return TRUE;
			break;
#ifdef _DEBUG
		case WM_CHAR:
		case WM_SYSCHAR:
		case WM_DEADCHAR:
		case WM_SYSDEADCHAR:
		{
			wchar_t szDbg[128]; _wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"%s(%i='%c', Scan=%i, lParam=0x%08X)\n",
				(messg == WM_CHAR) ? L"WM_CHAR" : (messg == WM_SYSCHAR) ? L"WM_SYSCHAR" : (messg == WM_SYSDEADCHAR) ? L"WM_DEADCHAR" : L"WM_DEADCHAR",
			                              (DWORD)wParam, (wchar_t)wParam, ((DWORD)lParam & 0xFF0000) >> 16, (DWORD)lParam);
			//_ASSERTE(FALSE && "WM_CHAR, WM_SYSCHAR, WM_DEADCHAR, WM_SYSDEADCHAR must be processed internally in CConEmuMain::OnKeyboard");
			gpConEmu->DebugStep(L"WM_CHAR, WM_SYSCHAR, WM_DEADCHAR, WM_SYSDEADCHAR must be processed internally in CConEmuMain::OnKeyboard", TRUE);
			DEBUGSTRCHAR(szDbg);
		}
		break;
#endif
		case WM_ACTIVATE:
#ifdef MSGLOGGER
			result = gpConEmu->OnFocus(hWnd, messg, wParam, lParam);
			result = DefWindowProc(hWnd, messg, wParam, lParam);
			break;
#endif
		case WM_ACTIVATEAPP:
#ifdef MSGLOGGER
			result = gpConEmu->OnFocus(hWnd, messg, wParam, lParam);
			result = DefWindowProc(hWnd, messg, wParam, lParam);
			break;
#endif
		case WM_KILLFOCUS:
#ifdef MSGLOGGER
			result = gpConEmu->OnFocus(hWnd, messg, wParam, lParam);
			result = DefWindowProc(hWnd, messg, wParam, lParam);
			break;
#endif
		case WM_SETFOCUS:
			result = gpConEmu->OnFocus(hWnd, messg, wParam, lParam);
			result = DefWindowProc(hWnd, messg, wParam, lParam); //-V519
			break;
		case WM_MOUSEACTIVATE:
			//return MA_ACTIVATEANDEAT; -- ��� ��� ������, � LBUTTONUP ���������� :(
			gpConEmu->mouse.nSkipEvents[0] = 0;
			gpConEmu->mouse.nSkipEvents[1] = 0;

			if (gpConEmu->mouse.bForceSkipActivation  // �������������� ��������� ����, �������� �� Desktop
			        || (gpSet->isMouseSkipActivation && LOWORD(lParam) == HTCLIENT
					&& !isMeForeground(false,false)))
			{
				gpConEmu->mouse.bForceSkipActivation = FALSE; // ����������
				POINT ptMouse = {0}; GetCursorPos(&ptMouse);
				//RECT  rcDC = {0}; GetWindowRect('ghWnd DC', &rcDC);
				//if (PtInRect(&rcDC, ptMouse))
				CVirtualConsole* pVCon = GetVConFromPoint(ptMouse);
				if (pVCon)
				{
					if (HIWORD(lParam) == WM_LBUTTONDOWN)
					{
						gpConEmu->mouse.nSkipEvents[0] = WM_LBUTTONDOWN;
						gpConEmu->mouse.nSkipEvents[1] = WM_LBUTTONUP;
						gpConEmu->mouse.nReplaceDblClk = WM_LBUTTONDBLCLK;
					}
					else if (HIWORD(lParam) == WM_RBUTTONDOWN)
					{
						gpConEmu->mouse.nSkipEvents[0] = WM_RBUTTONDOWN;
						gpConEmu->mouse.nSkipEvents[1] = WM_RBUTTONUP;
						gpConEmu->mouse.nReplaceDblClk = WM_RBUTTONDBLCLK;
					}
					else if (HIWORD(lParam) == WM_MBUTTONDOWN)
					{
						gpConEmu->mouse.nSkipEvents[0] = WM_MBUTTONDOWN;
						gpConEmu->mouse.nSkipEvents[1] = WM_MBUTTONUP;
						gpConEmu->mouse.nReplaceDblClk = WM_MBUTTONDBLCLK;
					}
				}
			}

			if (gpConEmu->m_InsideIntegration)
			{
				SetForegroundWindow(ghWnd);
			}

			result = DefWindowProc(hWnd, messg, wParam, lParam);
			break;
		case WM_MOUSEMOVE:
		case WM_MOUSEWHEEL:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_XBUTTONDBLCLK:
			result = gpConEmu->OnMouse(hWnd, messg, wParam, lParam);
			break;
		case WM_CLOSE:
			_ASSERTE(FALSE && "WM_CLOSE is not called in normal behavior");
			CVConGroup::OnScClose();
			result = 0;
			break;
		case WM_SYSCOMMAND:
			result = gpConEmu->OnSysCommand(hWnd, wParam, lParam);
			break;
		case WM_NCLBUTTONDOWN:
			// ��� ������� WM_NCLBUTTONUP � ��������� �� ��������
			gpConEmu->mouse.state |= MOUSE_SIZING_BEGIN;

			if (gpSet->isHideCaptionAlways())
			{
				KillTimer(ghWnd, TIMER_CAPTION_DISAPPEAR_ID);
				// -- ���� �� ������ WM_NCLBUTTONDOWN - ������ ����� ��� ��������?
				//gpConEmu->OnTimer(TIMER_CAPTION_APPEAR_ID,0);
				//UpdateWindowRgn();
			}

			result = DefWindowProc(hWnd, messg, wParam, lParam);
			break;

		case WM_NCLBUTTONDBLCLK:
			gpConEmu->mouse.state |= MOUSE_SIZING_DBLCKL; // ����� � ������� �� ���������� LBtnUp ���� ������ �����������

			if (gpConEmu->OnMouse_NCBtnDblClk(hWnd, messg, wParam, lParam))
				result = 0;
			else
				result = DefWindowProc(hWnd, messg, wParam, lParam);

			break;

		case WM_NCRBUTTONUP:
			if (wParam == HTCLOSE)
			{
				Icon.HideWindowToTray();
			}
			break;

		case WM_TRAYNOTIFY:
			result = Icon.OnTryIcon(hWnd, messg, wParam, lParam);
			break;

		case WM_HOTKEY:
			gpConEmu->OnWmHotkey(wParam);
			return 0;

		case WM_SETCURSOR:
			result = gpConEmu->OnSetCursor(wParam, lParam);

			if (!result && (lParam != -1))
			{
				// ��������� resize-�������� �� �����, � �.�.
				result = DefWindowProc(hWnd, messg, wParam, lParam);
			}

			MCHKHEAP;
			// If an application processes this message, it should return TRUE to halt further processing or FALSE to continue.
			return result;
		case WM_DESTROY:
			result = gpConEmu->OnDestroy(hWnd);
			break;
		case WM_IME_CHAR:
		case WM_IME_COMPOSITION:
		case WM_IME_COMPOSITIONFULL:
		case WM_IME_CONTROL:
		case WM_IME_ENDCOMPOSITION:
		case WM_IME_KEYDOWN:
		case WM_IME_KEYUP:
		case WM_IME_NOTIFY:
		case WM_IME_REQUEST:
		case WM_IME_SELECT:
		case WM_IME_SETCONTEXT:
		case WM_IME_STARTCOMPOSITION:
			result = gpConEmu->OnKeyboardIme(hWnd, messg, wParam, lParam);
			break;
		case WM_INPUTLANGCHANGE:
		case WM_INPUTLANGCHANGEREQUEST:

			if (hWnd == ghWnd)
				result = gpConEmu->OnLangChange(messg, wParam, lParam);
			else
				break;

			break;
			//case WM_NCHITTEST:
			//	{
			//		/*result = -1;
			//		if (gpSet->isHideCaptionAlways && gpSet->isTabs) {
			//			if (gpConEmu->mp_TabBar->IsShown()) {
			//				HWND hTabBar = gpConEmu->mp_TabBar->GetTabbar();
			//				RECT rcWnd; GetWindowRect(hTabBar, &rcWnd);
			//				TCHITTESTINFO tch = {{(int)(short)LOWORD(lParam),(int)(short)HIWORD(lParam)}};
			//				if (PtInRect(&rcWnd, tch.pt)) {
			//					// ������������� � ������������� ����������
			//					tch.pt.x -= rcWnd.left; tch.pt.y -= rcWnd.top;
			//					LRESULT nTest = SendMessage(hTabBar, TCM_HITTEST, 0, (LPARAM)&tch);
			//					if (nTest == -1) {
			//						result = HTCAPTION;
			//					}
			//				}
			//			}
			//		}
			//		if (result == -1)*/
			//		result = DefWindowProc(hWnd, messg, wParam, lParam);
			//		if (gpSet->isHideCaptionAlways() && !gpConEmu->mp_TabBar->IsShown() && result == HTTOP)
			//			result = HTCAPTION;
			//	} break;
		default:

			if (messg == gpConEmu->mn_MsgPostCreate)
			{
				gpConEmu->PostCreate(TRUE);
				return 0;
			}
			else if (messg == gpConEmu->mn_MsgPostCopy)
			{
				gpConEmu->PostDragCopy(lParam!=0, TRUE);
				return 0;
			}
			else if (messg == gpConEmu->mn_MsgMyDestroy)
			{
				ShutdownGuiStep(L"DestroyWindow");
				//gpConEmu->OnDestroy(hWnd);
				DestroyWindow(hWnd);
				return 0;
			}
			else if (messg == gpConEmu->mn_MsgUpdateSizes)
			{
				gpConEmu->UpdateSizes();
				return 0;
			}
			else if (messg == gpConEmu->mn_MsgUpdateCursorInfo)
			{
				COORD cr; cr.X = LOWORD(wParam); cr.Y = HIWORD(wParam);
				CONSOLE_CURSOR_INFO ci; ci.dwSize = LOWORD(lParam); ci.bVisible = HIWORD(lParam);
				gpConEmu->UpdateCursorInfo(NULL, cr, ci);
				return 0;
			}
			else if (messg == gpConEmu->mn_MsgSetWindowMode)
			{
				gpConEmu->SetWindowMode((ConEmuWindowMode)wParam);
				return 0;
			}
			else if (messg == gpConEmu->mn_MsgUpdateTitle)
			{
				//gpConEmu->UpdateTitle(TitleCmp);
				gpConEmu->UpdateTitle(/*mp_ VActive->RCon()->GetTitle()*/);
				return 0;
				//} else if (messg == gpConEmu->mn_MsgAttach) {
				//    return gpConEmu->AttachRequested ( (HWND)wParam, (DWORD)lParam );
			}
			else if (messg == gpConEmu->mn_MsgSrvStarted)
			{
				MsgSrvStartedArg *pArg = (MsgSrvStartedArg*)lParam;
				HWND hWndDC = NULL, hWndBack = NULL;
				//111002 - ������� ������ HWND ���� ��������� (�������� ���� ConEmu)

				DWORD nServerPID = pArg->nSrcPID;
				HWND  hWndCon = pArg->hConWnd;
				pArg->timeRecv = timeGetTime();

				DWORD t1, t2, t3; int iFound = -1;

				hWndDC = CVConGroup::DoSrvCreated(nServerPID, hWndCon, t1, t2, t3, iFound, hWndBack);

				pArg->hWndDc = hWndDC;
				pArg->hWndBack = hWndBack;

				pArg->timeFin = timeGetTime();
				if (hWndDC == NULL)
				{
					_ASSERTE(hWndDC!=NULL);
				}
				else
				{
					#ifdef _DEBUG
					DWORD nRecvDur = pArg->timeRecv - pArg->timeStart;
					DWORD nProcDur = pArg->timeFin - pArg->timeRecv;
					
					#define MSGSTARTED_TIMEOUT 10000
					if ((nRecvDur > MSGSTARTED_TIMEOUT) || (nProcDur > MSGSTARTED_TIMEOUT))
					{
						_ASSERTE((nRecvDur <= MSGSTARTED_TIMEOUT) && (nProcDur <= MSGSTARTED_TIMEOUT));
					}
					#endif
				}

				return (LRESULT)hWndDC;
			}
			else if (messg == gpConEmu->mn_MsgVConTerminated)
			{
				#ifdef _DEBUG
				int i = -100;
				wchar_t szDbg[200];
				{
					lstrcpy(szDbg, L"OnVConClosed");
					CVirtualConsole* pVCon = (CVirtualConsole*)lParam;

					i = isVConValid(pVCon);
					if (i >= 1)
					{
						ConEmuTab tab = {0};
						pVCon->RCon()->GetTab(0, &tab);
						tab.Name[128] = 0; // ����� �� ������� �� szDbg
						wsprintf(szDbg+_tcslen(szDbg), L": #%i: %s", i, tab.Name);
					}

					lstrcat(szDbg, L"\n");
					DEBUGSTRCONS(szDbg);
				}
				#endif

				return gpConEmu->OnVConClosed((CVirtualConsole*)lParam, TRUE);
			}
			else if (messg == gpConEmu->mn_MsgUpdateScrollInfo)
			{
				return OnUpdateScrollInfo(TRUE);
			}
			else if (messg == gpConEmu->mn_MsgUpdateTabs)
			{
				DEBUGSTRTABS(L"OnUpdateTabs\n");
				gpConEmu->mp_TabBar->Update(TRUE);
				return 0;
			}
			else if (messg == gpConEmu->mn_MsgOldCmdVer)
			{
				CESERVER_REQ_HDR* p = (CESERVER_REQ_HDR*)lParam;
				_ASSERTE(p->cbSize == sizeof(CESERVER_REQ_HDR));
				gpConEmu->ReportOldCmdVersion(p->nCmd, p->nVersion, (int)wParam, p->nSrcPID, p->hModule, p->nBits);
				return 0;
			}
			else if (messg == gpConEmu->mn_MsgTabCommand)
			{
				gpConEmu->TabCommand((ConEmuTabCommand)(int)wParam);
				return 0;
			}
			else if (messg == gpConEmu->mn_MsgTabSwitchFromHook)
			{
				gpConEmu->TabCommand((wParam & cvk_Shift) ? ctc_SwitchPrev : ctc_SwitchNext);
				if (!gpSet->isTabLazy)
				{
					mb_InWinTabSwitch = FALSE;
					gpConEmu->TabCommand(ctc_SwitchCommit);
				}
				else
				{
					mb_InWinTabSwitch = TRUE;
				}
				return 0;
			}
			else if (messg == gpConEmu->mn_MsgWinKeyFromHook)
			{
				// ��� ������ ���������� ��� ������������!
				DWORD VkMod = (DWORD)wParam;
				gpConEmu->OnKeyboardHook(VkMod);
				//BOOL lbShift = (lParam!=0);
				//OnKeyboardHook(vk, lbShift);
				return 0;
			}
			#if 0
			else if (messg == gpConEmu->mn_MsgConsoleHookedKey)
			{
				WORD vk = LOWORD(wParam);
				gpConEmu->OnConsoleKey(vk, lParam);
				return 0;
			}
			#endif
			else if (messg == gpConEmu->mn_MsgSheelHook)
			{
				gpConEmu->OnShellHook(wParam, lParam);
				return 0;
			}
			else if (messg == gpConEmu->mn_ShellExecuteEx)
			{
				gpConEmu->GuiShellExecuteExQueue();
				return 0;
				//GuiShellExecuteExArg* pArg = (GuiShellExecuteExArg*)wParam;
				//if (pArg && (wParam != TRUE))
				//{
				//	_ASSERTE(pArg->lpShellExecute == (SHELLEXECUTEINFO*)lParam);
				//	result = gpConEmu->GuiShellExecuteEx(pArg->lpShellExecute, pArg->pVCon);
				//	SafeFree(pArg);
				//}
				//return result;
			}
			else if (messg == gpConEmu->mn_PostConsoleResize)
			{
				gpConEmu->OnConsoleResize(TRUE);
				return 0;
			}
			else if (messg == gpConEmu->mn_ConsoleLangChanged)
			{
				gpConEmu->OnLangChangeConsole((CVirtualConsole*)lParam, (DWORD)wParam);
				return 0;
			}
			else if (messg == gpConEmu->mn_MsgPostOnBufferHeight)
			{
				CVConGuard VCon;
				if (gpConEmu->GetActiveVCon(&VCon) >= 0)
				{
					VCon->RCon()->OnBufferHeight();
				}
				return 0;
				//} else if (messg == gpConEmu->mn_MsgSetForeground) {
				//	apiSetForegroundWindow((HWND)lParam);
				//	return 0;
			}
			else if (messg == gpConEmu->mn_MsgFlashWindow)
			{
				return OnFlashWindow((DWORD)(wParam & 0xFF000000) >> 24, (DWORD)wParam & 0xFFFFFF, (HWND)lParam);
			}
			else if (messg == gpConEmu->mn_MsgPostAltF9)
			{
				OnAltF9(TRUE);
				return 0;
			}
			else if (messg == gpConEmu->mn_MsgInitInactiveDC)
			{
				CVirtualConsole* pVCon = (CVirtualConsole*)lParam;
				if (isValid(pVCon) && !isActive(pVCon))
				{
					CVConGuard guard(pVCon);
					pVCon->InitDC(true, true, NULL, NULL);
					pVCon->LoadConsoleData();
				}

				return 0;
			}
			else if (messg == gpConEmu->mn_MsgUpdateProcDisplay)
			{
				gpConEmu->UpdateProcessDisplay(wParam!=0);
				return 0;
			}
			else if (messg == gpConEmu->mn_MsgAutoSizeFont)
			{
				gpSetCls->MacroFontSetSize((int)wParam, (int)lParam);
				return 0;
			}
			else if (messg == gpConEmu->mn_MsgMacroFontSetName)
			{
				gpConEmu->PostMacroFontSetName((wchar_t*)lParam, (WORD)(((DWORD)wParam & 0xFFFF0000)>>16), (WORD)(wParam & 0xFFFF), TRUE);
				return 0;
			}
			else if (messg == gpConEmu->mn_MsgDisplayRConError)
			{
				CRealConsole *pRCon = (CRealConsole*)wParam;
				wchar_t* pszErrMsg = (wchar_t*)lParam;

				if (gpConEmu->isValid(pRCon))
					MessageBox(ghWnd, pszErrMsg, gpConEmu->GetLastTitle(), MB_ICONSTOP|MB_SYSTEMMODAL);

				free(pszErrMsg);
				return 0;
			}
			else if (messg == gpConEmu->mn_MsgCreateViewWindow)
			{
				CConEmuChild* pChild = (CConEmuChild*)lParam;
				HWND hChild = pChild ? pChild->CreateView() : NULL;
				return (LRESULT)hChild;
			}
			else if (messg == gpConEmu->mn_MsgPostTaskbarActivate)
			{
				mb_PostTaskbarActivate = FALSE;
				HWND hFore = GetForegroundWindow();
				CVConGuard VCon;
				if ((gpConEmu->GetActiveVCon(&VCon) >= 0) && (hFore == ghWnd))
					VCon->OnTaskbarFocus();
				return 0;
			}
			else if (messg == gpConEmu->mn_MsgInitVConGhost)
			{
				CVirtualConsole* pVCon = (CVirtualConsole*)lParam;
				if (gpConEmu->isValid(pVCon))
					pVCon->InitGhost();
				return 0;
			}
			else if (messg == gpConEmu->mn_MsgCreateCon)
			{
				_ASSERTE(wParam == gpConEmu->mn_MsgCreateCon || wParam == (gpConEmu->mn_MsgCreateCon+1));
				RConStartArgs *pArgs = (RConStartArgs*)lParam;
				CVirtualConsole* pVCon = CreateCon(pArgs, (wParam == (gpConEmu->mn_MsgCreateCon+1)));
				UNREFERENCED_PARAMETER(pVCon);
				delete pArgs;
				return (LRESULT)pVCon;
			}
			else if (messg == gpConEmu->mn_MsgRequestUpdate)
			{
				switch (wParam)
				{
				case 0:
					gpConEmu->ReportUpdateError();
					return 0;
				case 1:
					return (LRESULT)gpConEmu->ReportUpdateConfirmation();
				case 2:
					gpConEmu->RequestExitUpdate();
					return 0;
				}
				return 0;
			}
			else if (messg == gpConEmu->mn_MsgTaskBarCreated)
			{
				gpConEmu->OnTaskbarCreated();
			}
			else if (messg == gpConEmu->mn_MsgTaskBarBtnCreated)
			{
				gpConEmu->OnTaskbarButtonCreated();
			}

			//else if (messg == gpConEmu->mn_MsgCmdStarted || messg == gpConEmu->mn_MsgCmdStopped) {
			//  return gpConEmu->OnConEmuCmd( (messg == gpConEmu->mn_MsgCmdStarted), (HWND)wParam, (DWORD)lParam);
			//}


			if (messg)
			{
				// 0x0313 - Undocumented. When you right-click on a taskbar button,
				// Windows sends an undocumented message ($0313) to the corresponding
				// application window. The WPARAM is unused (zero) and the
				// LPARAM contains the mouse position in screen coordinates, in the usual
				// format. By default, WindowProc handles this message by popping up the
				// system menu at the given coordinates.
				if ((messg == WM_CONTEXTMENU) || (messg == 0x0313))
				{
					mn_TrackMenuPlace = tmp_System;
					mp_Tip->HideTip();
				}

				result = DefWindowProc(hWnd, messg, wParam, lParam);

				if ((messg == WM_CONTEXTMENU) || (messg == 0x0313))
				{
					mn_TrackMenuPlace = tmp_None;
					mp_Tip->HideTip();
				}
			}
	}

	return result;
}
