
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


#ifdef _DEBUG
//  �����������������, ����� ����� ����� �������� ������� �������� MessageBox, ����� ����������� ����������
//  #define SHOW_STARTED_MSGBOX
//  #define SHOW_WRITING_RECTS
#endif

#define SHOWDEBUGSTR
//#define MCHKHEAP
#define DEBUGSTRMENU(s) DEBUGSTR(s)
#define DEBUGSTRCTRL(s) DEBUGSTR(s)


//#include <stdio.h>
#include <windows.h>
#include <Tlhelp32.h>
//#include <windowsx.h>
//#include <string.h>
//#include <tchar.h>
#include "../common/common.hpp"
#pragma warning( disable : 4995 )
#include "../common/pluginW1761.hpp" // ���������� �� 995 �������� SynchoApi
#pragma warning( default : 4995 )
#include "../common/RgnDetect.h"
#include "../common/TerminalMode.h"
#include "ConEmuTh.h"
#include "ImgCache.h"

#define Free free
#define Alloc calloc

#define MAKEFARVERSION(major,minor,build) ( ((major)<<8) | (minor) | ((build)<<16))

//#define ConEmuTh_SysID 0x43455568 // 'CETh'

#ifdef _DEBUG
wchar_t gszDbgModLabel[6] = {0};
#endif

#if defined(__GNUC__)
extern "C" {
	BOOL WINAPI DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved);
	void WINAPI SetStartupInfoW(void *aInfo);
};
#endif

PanelViewSetMapping gThSet = {0}; // ��������� ���������� �� �������� ��� �������� ����� ��� ��� ���������������

HWND ghConEmuRoot = NULL;
HMODULE ghPluginModule = NULL; // ConEmuTh.dll - ��� ������
BOOL TerminalMode = FALSE;
DWORD gnSelfPID = 0;
DWORD gnMainThreadId = 0;
HANDLE ghDisplayThread = NULL; DWORD gnDisplayThreadId = 0;
//HWND ghLeftView = NULL, ghRightView = NULL;
//wchar_t* gszRootKey = NULL;
FarVersion gFarVersion;
//HMODULE ghConEmuDll = NULL;
RegisterPanelView_t gfRegisterPanelView = NULL;
GetFarHWND2_t gfGetFarHWND2 = NULL;
CeFullPanelInfo pviLeft = {0}, pviRight = {0};
int ShowLastError();
CRgnDetect *gpRgnDetect = NULL;
CImgCache  *gpImgCache = NULL;
CEFAR_INFO_MAPPING gFarInfo = {0};
COLORREF /*gcrActiveColors[16], gcrFadeColors[16],*/ *gcrCurColors = gThSet.crPalette;
bool gbFadeColors = false;
//bool gbLastCheckWindow = false;
bool gbFarPanelsReady = false;
DWORD gnRgnDetectFlags = 0;
void CheckVarsInitialized();
SECURITY_ATTRIBUTES* gpLocalSecurity = NULL;
// *** lng resources begin ***
wchar_t gsFolder[64], gsHardLink[64], gsSymLink[64], gsJunction[64], gsTitleThumbs[64], gsTitleTiles[64];
// *** lng resources end ***
DWORD gnFarPanelSettings = 0, gnFarInterfaceSettings = 0;

//bool gbWaitForKeySequenceEnd = false;
DWORD gnWaitForKeySeqTick = 0;
//int gnUngetCount = 0;
//INPUT_RECORD girUnget[100];
WORD wScanCodeUp=0, wScanCodeDown=0; //p->Event.KeyEvent.wVirtualScanCode = MapVirtualKey(vk, 0/*MAPVK_VK_TO_VSC*/);
#ifdef _DEBUG
WORD wScanCodeLeft=0, wScanCodeRight=0, wScanCodePgUp=0, wScanCodePgDn=0;
#endif
//BOOL GetBufferInput(BOOL abRemove, PINPUT_RECORD lpBuffer, DWORD nBufSize, LPDWORD lpNumberOfEventsRead);
//BOOL UngetBufferInput(WORD nCount, PINPUT_RECORD lpOneInput);
void ResetUngetBuffer();
BOOL ProcessConsoleInput(BOOL abReadMode, PINPUT_RECORD lpBuffer, DWORD nBufSize, LPDWORD lpNumberOfEventsRead);
DWORD gnConsoleChanges = 0; // bitmask: 1-left, 2-right
bool  gbConsoleChangesSyncho = false;
void OnReadyForPanelsReload();
void ReloadResourcesW();

ConEmuThSynchroArg* gpLastSynchroArg = NULL;
bool gbSynchoRedrawPanelRequested = false;

#ifdef _DEBUG
MFileMapping<DetectedDialogs> *gpDbgDlg = NULL;
#endif


// minimal(?) FAR version 2.0 alpha build FAR_X_VER
int WINAPI _export GetMinFarVersionW(void)
{
	// ACTL_SYNCHRO required
	return MAKEFARVERSION(2,0,max(1007,FAR_X_VER));
}

void WINAPI _export GetPluginInfoWcmn(void *piv)
{
	if (gFarVersion.dwBuild>=FAR_Y_VER)
		FUNC_Y(GetPluginInfoW)(piv);
	else
		FUNC_X(GetPluginInfoW)(piv);
}


BOOL gbInfoW_OK = FALSE;
HANDLE OpenPluginWcmn(int OpenFrom,INT_PTR Item)
{
	if (!gbInfoW_OK)
		return INVALID_HANDLE_VALUE;

	ReloadResourcesW();
	EntryPoint(OpenFrom, Item);
	return INVALID_HANDLE_VALUE;
}

// !!! WARNING !!! Version independent !!!
void EntryPoint(int OpenFrom,INT_PTR Item)
{
	if (!CheckConEmu())
		return;

	if (ghDisplayThread && gnDisplayThreadId == 0)
	{
		CloseHandle(ghDisplayThread); ghDisplayThread = NULL;
	}

	//gThSet.Load();
	// ��� �������� ������� - ��������� ���������� �� ����� �������. ����� ��� ����������� ��������!
	ReloadPanelsInfo();
	// �������� ��������
	CeFullPanelInfo* pi = GetActivePanel();

	if (!pi)
	{
		return;
	}

	pi->OurTopPanelItem = pi->TopPanelItem;
	HWND lhView = pi->hView; // (pi->bLeftPanel) ? ghLeftView : ghRightView;
	PanelViewMode PVM = pvm_None;

	// � Far2 ������ ����� ������� ����� callplugin(...)
	if (gFarVersion.dwVerMajor >= 2)
	{
		if ((OpenFrom & OPEN_FROMMACRO) == OPEN_FROMMACRO)
		{
			if (Item == pvm_Thumbnails || Item == pvm_Tiles)
				PVM = (PanelViewMode)Item;
		}
	}

	// ����� ������� �� ���� - ����� ������� �����
	if (PVM == pvm_None)
	{
		switch(ShowPluginMenu())
		{
			case 0:
				PVM = pvm_Thumbnails;
				break;
			case 1:
				PVM = pvm_Tiles;
				break;
			default:
				// ������
				return;
		}
	}

	BOOL lbRc = FALSE;
	DWORD dwErr = 0;
	DWORD dwMode = pvm_None; //PanelViewMode

	// ���� View �� ������, ��� ����� ������
	if ((lhView == NULL) || (PVM != pi->PVM))
	{
		// ��� ����������� ����������� ��������� ������� ��������� ���� �� ������� � ��������� ������:
		// [x] ���������� ��������� ������� [x] ���������� ��������� ����������
		if (!CheckPanelSettings(FALSE))
		{
			return;
		}

		pi->PVM = PVM;
		pi->DisplayReloadPanel();

		if (lhView == NULL)
		{
			// ����� ������� View
			lhView = pi->CreateView();
		}

		if (lhView == NULL)
		{
			// �������� ������
			ShowLastError();
		}
		else
		{
			// ������������������
			_ASSERTE(pi->PVM==PVM);
			pi->RegisterPanelView();
			_ASSERTE(pi->PVM==PVM);
		}

		if (pi->hView)
			dwMode = pi->PVM;
	}
	else
	{
		// ������������������
		pi->UnregisterPanelView();
		dwMode = pvm_None;
	}

	HKEY hk = NULL;

	SettingsSave(pi->bLeftPanel ? L"LeftPanelView" : L"RightPanelView", &dwMode);
	//if (!RegCreateKeyExW(HKEY_CURRENT_USER, gszRootKey, 0, NULL, 0, KEY_WRITE, NULL, &hk, NULL))
	//{
	//	RegSetValueEx(hk, pi->bLeftPanel ? L"LeftPanelView" : L"RightPanelView", 0,
	//	              REG_DWORD, (LPBYTE)&dwMode, sizeof(dwMode));
	//	RegCloseKey(hk);
	//}

	return;
}

// ������ ����� ���� ������ � ������ ��� �� ������� ����.
// ������� ������� "gnMainThreadId = GetCurrentThreadId();" �� ��������. ����� ������ ������ ���� ��������!
DWORD GetMainThreadId()
{
	DWORD nThreadID = 0;
	DWORD nProcID = GetCurrentProcessId();
	HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	if (h != INVALID_HANDLE_VALUE)
	{
		THREADENTRY32 ti = {sizeof(THREADENTRY32)};

		if (Thread32First(h, &ti))
		{
			do
			{
				// ����� ����� ������ ���� ��������
				if (ti.th32OwnerProcessID == nProcID)
				{
					nThreadID = ti.th32ThreadID;
					break;
				}
			}
			while(Thread32Next(h, &ti));
		}

		CloseHandle(h);
	}

	// ��������. ������ ���� �������. ������ ���� ���-�� (������� ����)
	if (!nThreadID)
	{
		_ASSERTE(nThreadID!=0);
		nThreadID = GetCurrentThreadId();
	}

	return nThreadID;
}

void WINAPI _export ExitFARW(void);
void WINAPI _export ExitFARW3(void*);

#include "../common/SetExport.h"
ExportFunc Far3Func[] =
{
	{"ExitFARW", ExitFARW, ExitFARW3},
	{NULL}
};


BOOL WINAPI DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch(ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			ghPluginModule = (HMODULE)hModule;
			gnSelfPID = GetCurrentProcessId();
			gnMainThreadId = GetMainThreadId();
			HeapInitialize();
			gpLocalSecurity = LocalSecurity();
			_ASSERTE(FAR_X_VER<=FAR_Y_VER);
#ifdef SHOW_STARTED_MSGBOX

			if (!IsDebuggerPresent()) MessageBoxA(NULL, "ConEmuTh*.dll loaded", "ConEmu Thumbnails", 0);

#endif
			// Check Terminal mode
			TerminalMode = isTerminalMode();
			//TCHAR szVarValue[MAX_PATH];
			//szVarValue[0] = 0;
			//if (GetEnvironmentVariable(L"TERM", szVarValue, 63)) {
			//    TerminalMode = TRUE;
			//}
			bool lbExportsChanged = false;
			if (LoadFarVersion())
			{
				if (gFarVersion.dwVerMajor == 3)
				{
					lbExportsChanged = ChangeExports( Far3Func, ghPluginModule );
					if (!lbExportsChanged)
					{
						_ASSERTE(lbExportsChanged);
					}
				}
			}
		}
		break;
		case DLL_PROCESS_DETACH:
			CommonShutdown();
			HeapDeinitialize();
			break;
	}

	return TRUE;
}

#if defined(CRTSTARTUP)
extern "C" {
	BOOL WINAPI _DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
};

BOOL WINAPI _DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
	DllMain(hDll, dwReason, lpReserved);
	return TRUE;
}
#endif




BOOL LoadFarVersion()
{
	BOOL lbRc=FALSE;
	WCHAR FarPath[MAX_PATH+1];

	if (GetModuleFileName(0,FarPath,MAX_PATH))
	{
		DWORD dwRsrvd = 0;
		DWORD dwSize = GetFileVersionInfoSize(FarPath, &dwRsrvd);

		if (dwSize>0)
		{
			void *pVerData = Alloc(dwSize, 1);

			if (pVerData)
			{
				VS_FIXEDFILEINFO *lvs = NULL;
				UINT nLen = sizeof(lvs);

				if (GetFileVersionInfo(FarPath, 0, dwSize, pVerData))
				{
					TCHAR szSlash[3]; wcscpy_c(szSlash, L"\\");

					if (VerQueryValue((void*)pVerData, szSlash, (void**)&lvs, &nLen))
					{
						gFarVersion.dwVer = lvs->dwFileVersionMS;
						gFarVersion.dwBuild = lvs->dwFileVersionLS;
						lbRc = TRUE;
					}
				}

				Free(pVerData);
			}
		}
	}

	if (!lbRc)
	{
		gFarVersion.dwVerMajor = 2;
		gFarVersion.dwVerMinor = 0;
		gFarVersion.dwBuild = FAR_X_VER;
	}

	return lbRc;
}

#define CONEMUCHECKDELTA 2000
//static DWORD nLastCheckConEmu = 0;
BOOL CheckConEmu(BOOL abSilence/*=FALSE*/)
{
	// ������� ��������� ����������� ������� conemu.dll � GUI
	// ������ ��� ����� ��� ������ ������, �.�. ������ ����� � ���������...

	//// ��������� �� ����, ��� conemu.dll ����� ��������� (unload:...)
	//if (!abForceCheck) {
	//	if ((GetTickCount() - nLastCheckConEmu) > CONEMUCHECKDELTA) {
	//		if (!ghConEmuRoot)
	//			return FALSE;
	//		return TRUE;
	//	}
	//}
	if (TerminalMode)
	{
		if (!abSilence)
			ShowMessage(CEUnavailableInTerminal, 0);

		return FALSE;
	}

	//nLastCheckConEmu = GetTickCount();
	HMODULE hConEmu = GetModuleHandle
	                  (
#ifdef WIN64
	                      L"ConEmu.x64.dll"
#else
	                      L"ConEmu.dll"
#endif
	                  );

	if (!hConEmu)
	{
		gfRegisterPanelView = NULL;
		gfGetFarHWND2 = NULL;

		if (!abSilence)
			ShowMessage(CEPluginNotFound, 0);

		return FALSE;
	}

	gfRegisterPanelView = (RegisterPanelView_t)GetProcAddress(hConEmu, "RegisterPanelView");
	gfGetFarHWND2 = (GetFarHWND2_t)GetProcAddress(hConEmu, "GetFarHWND2");

	if (!gfRegisterPanelView || !gfGetFarHWND2)
	{
		if (!abSilence)
			ShowMessage(CEOldPluginVersion, 0);

		return FALSE;
	}

	HWND hWnd = gfGetFarHWND2(TRUE);
	HWND hRoot = hWnd ? GetParent(hWnd) : NULL;

	if (hRoot != ghConEmuRoot)
	{
		ghConEmuRoot = hRoot;

		if (hRoot)
		{
			//MFileMapping<PanelViewSetMapping> ThSetMap;
			//DWORD nGuiPID;
			//GetWindowThreadProcessId(ghConEmuRoot, &nGuiPID);
			//_ASSERTE(nGuiPID!=0);
			//ThSetMap.InitName(CECONVIEWSETNAME, nGuiPID);
			//if (!ThSetMap.Open()) {
			//	MessageBox(NULL, ThSetMap.GetErrorText(), L"ConEmuTh", MB_ICONSTOP|MB_SETFOREGROUND|MB_SYSTEMMODAL);
			//} else {
			//	ThSetMap.GetTo(&gThSet);
			//	ThSetMap.CloseMap();
			//}
			LoadThSet();
		}
	}

	if (!hRoot)
	{
		if (!abSilence)
			ShowMessage(CEFarNonGuiMode, 0);

		return FALSE;
	}

	return TRUE;
}













void ReloadResourcesW()
{
	lstrcpynW(gsFolder, GetMsgW(CEDirFolder), countof(gsFolder));
	//lstrcpynW(gsHardLink, GetMsgW(CEDirHardLink), countof(gsHardLink));
	lstrcpynW(gsSymLink, GetMsgW(CEDirSymLink), countof(gsSymLink));
	lstrcpynW(gsJunction, GetMsgW(CEDirJunction), countof(gsJunction));
	lstrcpynW(gsTitleThumbs, GetMsgW(CEColTitleThumbnails), countof(gsTitleThumbs));
	lstrcpynW(gsTitleTiles, GetMsgW(CEColTitleTiles), countof(gsTitleTiles));
}

void WINAPI _export SetStartupInfoW(void *aInfo)
{
	if (!gFarVersion.dwVerMajor) LoadFarVersion();

	if (gFarVersion.dwBuild>=FAR_Y_VER)
		FUNC_Y(SetStartupInfoW)(aInfo);
	else
		FUNC_X(SetStartupInfoW)(aInfo);

	//_ASSERTE(gszRootKey!=NULL && *gszRootKey!=0);
	ReloadResourcesW();
	gbInfoW_OK = TRUE;
	StartPlugin(FALSE);
}







#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif






void StartPlugin(BOOL abManual)
{
	// ��� ������ ������, ������ ��� ��� ����� ����� ����� � ������
	if (!gpImgCache)
	{
		gpImgCache = new CImgCache(ghPluginModule);
	}

	if (!CheckConEmu(!abManual))
		return;

	HKEY hk = NULL;

	//if (!RegOpenKeyExW(HKEY_CURRENT_USER, gszRootKey, 0, KEY_READ, &hk))
	{
		DWORD dwModes[2] = {0,0}; //, dwSize = 4;
		SettingsLoad(L"LeftPanelView", dwModes);
		SettingsLoad(L"RightPanelView", dwModes+1);

		//if (RegQueryValueEx(hk, L"LeftPanelView", NULL, NULL, (LPBYTE)dwModes, &(dwSize=4)))
		//	dwModes[0] = 0;

		//if (RegQueryValueEx(hk, L"RightPanelView", NULL, NULL, (LPBYTE)(dwModes+1), &(dwSize=4)))
		//	dwModes[1] = 0;

		//RegCloseKey(hk);

		if (ghDisplayThread && gnDisplayThreadId == 0)
		{
			CloseHandle(ghDisplayThread); ghDisplayThread = NULL;
		}

		if (dwModes[0] || dwModes[1])
		{
			if (gThSet.bRestoreOnStartup)
			{
				// ��� ����������� ����������� ��������� ������� ��������� ���� �� ������� � ��������� ������:
				// [x] ���������� ��������� ������� [x] ���������� ��������� ����������
				if (!CheckPanelSettings(!abManual))
				{
					return;
				}

				// ��� �������� ������� - ��������� ���������� �� ����� �������. ����� ��� ����������� ��������!
				ReloadPanelsInfo();
				CeFullPanelInfo* pi[2] = {&pviLeft, &pviRight};

				for(int i = 0; i < 2; i++)
				{
					if (!pi[i]->hView && dwModes[i])
					{
						pi[i]->PVM = (PanelViewMode)dwModes[i];
						pi[i]->DisplayReloadPanel();

						if (pi[i]->hView == NULL)
						{
							// ����� ������� View
							pi[i]->hView = pi[i]->CreateView();
						}

						if (pi[i]->hView == NULL)
						{
							// �������� ������
							ShowLastError();
						}
						else
						{
							pi[i]->RegisterPanelView();
						}
					}
				}
			}
		}
	}
}

void ExitPlugin(void)
{
	//if (ghLeftView)
	if (pviLeft.hView)
		pviLeft.UnregisterPanelView();

	//PostMessage(ghLeftView, WM_CLOSE,0,0);
	//if (ghRightView)
	if (pviRight.hView)
		pviRight.UnregisterPanelView();

	//PostMessage(ghRightView, WM_CLOSE,0,0);
	DWORD dwWait = 0;

	if (ghDisplayThread)
		dwWait = WaitForSingleObject(ghDisplayThread, 1000);

	if (dwWait)
		TerminateThread(ghDisplayThread, 100);

	if (ghDisplayThread)
	{
		CloseHandle(ghDisplayThread); ghDisplayThread = NULL;
	}

	gnDisplayThreadId = 0;

	// ���������� ������
	if (gpRgnDetect)
	{
		delete gpRgnDetect;
		gpRgnDetect = NULL;
	}

	if (gpImgCache)
	{
		delete gpImgCache;
		gpImgCache = NULL;
	}

	// ����� ����������, ����, � �.�.
	pviLeft.FinalRelease();
	pviRight.FinalRelease();

	if (gpLastSynchroArg)
	{
		LocalFree(gpLastSynchroArg); gpLastSynchroArg = NULL;
	}

	//if (gszRootKey)
	//{
	//	free(gszRootKey); gszRootKey = NULL;
	//}

#ifdef _DEBUG

	if (gpDbgDlg)
	{
		delete gpDbgDlg; gpDbgDlg = NULL;
	}

#endif
}

void   WINAPI _export ExitFARW(void)
{
	ExitPlugin();

	if (gFarVersion.dwBuild>=FAR_Y_VER)
		FUNC_Y(ExitFARW)();
	else
		FUNC_X(ExitFARW)();
}

void WINAPI _export ExitFARW3(void*)
{
	ExitPlugin();

	if (gFarVersion.dwBuild>=FAR_Y_VER)
		FUNC_Y(ExitFARW)();
	else
		FUNC_X(ExitFARW)();
}






int ShowMessage(int aiMsg, int aiButtons)
{
	if (gFarVersion.dwVerMajor==1)
		return ShowMessageA(aiMsg, aiButtons);
	else if (gFarVersion.dwBuild>=FAR_Y_VER)
		return FUNC_Y(ShowMessageW)(aiMsg, aiButtons);
	else
		return FUNC_X(ShowMessageW)(aiMsg, aiButtons);
}

LPCWSTR GetMsgW(int aiMsg)
{
	if (gFarVersion.dwVerMajor==1)
		return L"";
	else if (gFarVersion.dwBuild>=FAR_Y_VER)
		return FUNC_Y(GetMsgW)(aiMsg);
	else
		return FUNC_X(GetMsgW)(aiMsg);
}

void PostMacro(wchar_t* asMacro)
{
	if (!asMacro || !*asMacro)
		return;

	if (gFarVersion.dwVerMajor==1)
	{
		int nLen = lstrlenW(asMacro);
		char* pszMacro = (char*)Alloc(nLen+1,1);

		if (pszMacro)
		{
			WideCharToMultiByte(CP_OEMCP,0,asMacro,nLen+1,pszMacro,nLen+1,0,0);
			PostMacroA(pszMacro);
			Free(pszMacro);
		}
	}
	else if (gFarVersion.dwBuild>=FAR_Y_VER)
	{
		FUNC_Y(PostMacroW)(asMacro);
	}
	else
	{
		FUNC_X(PostMacroW)(asMacro);
	}

	//FAR BUGBUG: ������ �� ����������� �� ����������, ���� ������ �� ������ :(
	//  ��� ���� ����� ����������� ��� ������ ���� �� RClick
	//  ���� ������ �� ������ ������, �� RClick ����� �� ���������
	//  �� �������� ��������� :(
	//if (!mcr.Param.PlainText.Flags) {
	INPUT_RECORD ir[2] = {{MOUSE_EVENT},{MOUSE_EVENT}};

	if (isPressed(VK_CAPITAL))
		ir[0].Event.MouseEvent.dwControlKeyState |= CAPSLOCK_ON;

	if (isPressed(VK_NUMLOCK))
		ir[0].Event.MouseEvent.dwControlKeyState |= NUMLOCK_ON;

	if (isPressed(VK_SCROLL))
		ir[0].Event.MouseEvent.dwControlKeyState |= SCROLLLOCK_ON;

	ir[0].Event.MouseEvent.dwEventFlags = MOUSE_MOVED;
	ir[1].Event.MouseEvent.dwControlKeyState = ir[0].Event.MouseEvent.dwControlKeyState;
	ir[1].Event.MouseEvent.dwEventFlags = MOUSE_MOVED;
	ir[1].Event.MouseEvent.dwMousePosition.X = 1;
	ir[1].Event.MouseEvent.dwMousePosition.Y = 1;
	//2010-01-29 ��������� STD_OUTPUT
	//if (!ghConIn) {
	//	ghConIn  = CreateFile(L"CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_READ,
	//		0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	//	if (ghConIn == INVALID_HANDLE_VALUE) {
	//		#ifdef _DEBUG
	//		DWORD dwErr = GetLastError();
	//		_ASSERTE(ghConIn!=INVALID_HANDLE_VALUE);
	//		#endif
	//		ghConIn = NULL;
	//		return;
	//	}
	//}
	HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
	DWORD cbWritten = 0;
#ifdef _DEBUG
	BOOL fSuccess =
#endif
	    WriteConsoleInput(hIn/*ghConIn*/, ir, 1, &cbWritten);
	_ASSERTE(fSuccess && cbWritten==1);
	//}
	//InfoW995->AdvControl(InfoW995->ModuleNumber,ACTL_REDRAWALL,NULL);
}





int ShowPluginMenu()
{
	int nItem = -1;
	//if (!FarHwnd) {
	//	SHOWDBGINFO(L"*** ShowPluginMenu failed, FarHwnd is NULL\n");
	//	return;
	//}

	//if (nID != -1) {
	//	nItem = nID;
	//	if (nItem >= 2) nItem++; //Separator
	//	if (nItem >= 7) nItem++; //Separator
	//	if (nItem >= 9) nItem++; //Separator
	//	SHOWDBGINFO(L"*** ShowPluginMenu used default item\n");
	//} else
	if (gFarVersion.dwVerMajor==1)
	{
		SHOWDBGINFO(L"*** calling ShowPluginMenuA\n");
		nItem = ShowPluginMenuA();
	}
	else if (gFarVersion.dwBuild>=FAR_Y_VER)
	{
		SHOWDBGINFO(L"*** calling ShowPluginMenuWY\n");
		nItem = FUNC_Y(ShowPluginMenuW)();
	}
	else
	{
		SHOWDBGINFO(L"*** calling ShowPluginMenuWX\n");
		nItem = FUNC_X(ShowPluginMenuW)();
	}

	return nItem;
	//if (nItem < 0) {
	//	SHOWDBGINFO(L"*** ShowPluginMenu cancelled, nItem < 0\n");
	//	return;
	//}
	//#ifdef _DEBUG
	//wchar_t szInfo[128]; StringCchPrintf(szInfo, countof(szInfo), L"*** ShowPluginMenu done, nItem == %i\n", nItem);
	//SHOWDBGINFO(szInfo);
	//#endif
	//switch (nItem) {
	//	case 0: case 1:
	//	{ // ������� � ��������� ����� ��������� ���������� ���������
	//		CESERVER_REQ* pIn = (CESERVER_REQ*)calloc(sizeof(CESERVER_REQ_HDR)+4,1);
	//		if (!pIn) return;
	//		CESERVER_REQ* pOut = NULL;
	//		ExecutePrepareCmd(pIn, CECMD_GETOUTPUTFILE, sizeof(CESERVER_REQ_HDR)+4);
	//		pIn->OutputFile.bUnicode = (gFarVersion.dwVerMajor>=2);
	//		pOut = ExecuteGuiCmd(FarHwnd, pIn, FarHwnd);
	//		if (pOut) {
	//			if (pOut->OutputFile.szFilePathName[0]) {
	//				BOOL lbRc = FALSE;
	//				if (gFarVersion.dwVerMajor==1)
	//					lbRc = EditOutputA(pOut->OutputFile.szFilePathName, (nItem==1));
	//				else if (gFarVersion.dwBuild>=FAR_Y_VER)
	//					lbRc = FUNC_Y(EditOutput)(pOut->OutputFile.szFilePathName, (nItem==1));
	//				else
	//					lbRc = FUNC_X(EditOutput)(pOut->OutputFile.szFilePathName, (nItem==1));
	//				if (!lbRc) {
	//					DeleteFile(pOut->OutputFile.szFilePathName);
	//				}
	//			}
	//			ExecuteFreeResult(pOut);
	//		}
	//		free(pIn);
	//	} break;
	//	case 3: // ��������/�������� ����
	//	case 4: case 5: case 6:
	//	{
	//		CESERVER_REQ in, *pOut = NULL;
	//		ExecutePrepareCmd(&in, CECMD_TABSCMD, sizeof(CESERVER_REQ_HDR)+1);
	//		in.Data[0] = nItem - 3;
	//		pOut = ExecuteGuiCmd(FarHwnd, &in, FarHwnd);
	//		if (pOut) ExecuteFreeResult(pOut);
	//	} break;
	//	case 8: // Attach to GUI (���� FAR ��� CtrlAltTab)
	//	{
	//		if (TerminalMode) break; // �����
	//		if (ConEmuHwnd && IsWindow(ConEmuHwnd)) break; // �� � ��� ����������?
	//		Attach2Gui();
	//	} break;
	//	case 10: // Start "ConEmuC.exe /DEBUGPID="
	//	{
	//		if (TerminalMode) break; // �����
	//		StartDebugger();
	//	}
	//}
}




BOOL IsMacroActive()
{
	BOOL lbActive = FALSE;

	if (gFarVersion.dwVerMajor==1)
		lbActive = IsMacroActiveA();
	else if (gFarVersion.dwBuild>=FAR_Y_VER)
		lbActive = FUNC_Y(IsMacroActiveW)();
	else
		lbActive = FUNC_X(IsMacroActiveW)();

	return lbActive;
}

BOOL CheckPanelSettings(BOOL abSilence)
{
	BOOL lbOk = FALSE;

	if (gFarVersion.dwVerMajor==1)
		lbOk = CheckPanelSettingsA(abSilence);
	else if (gFarVersion.dwBuild>=FAR_Y_VER)
		lbOk = FUNC_Y(CheckPanelSettingsW)(abSilence);
	else
		lbOk = FUNC_X(CheckPanelSettingsW)(abSilence);

	return lbOk;
}

void LoadPanelItemInfo(CeFullPanelInfo* pi, int nItem)
{
	if (gFarVersion.dwVerMajor==1)
		LoadPanelItemInfoA(pi, nItem);
	else if (gFarVersion.dwBuild>=FAR_Y_VER)
		FUNC_Y(LoadPanelItemInfoW)(pi, nItem);
	else
		FUNC_X(LoadPanelItemInfoW)(pi, nItem);
}


// ���������� (��� ��������) ������ �� ���� �� ���������� ���������� (pviLeft/pviRight)
CeFullPanelInfo* GetActivePanel()
{
	if (pviLeft.Visible && pviLeft.Focus && pviLeft.IsFilePanel)
		return &pviLeft;

	if (pviRight.Visible && pviRight.Focus && pviRight.IsFilePanel)
		return &pviRight;

	return NULL;
}
//CeFullPanelInfo* LoadPanelInfo(BOOL abActive)
//{
//	TODO("�������� ����� ACTL_GETWINDOW ���-��?");
//
//	CheckVarsInitialized();
//
//	CeFullPanelInfo* ppi = NULL;
//
//	if (gFarVersion.dwVerMajor==1)
//		ppi = LoadPanelInfoA(abActive);
//	else if (gFarVersion.dwBuild>=FAR_Y_VER)
//		ppi = FUNC_Y(LoadPanelInfo)(abActive);
//	else
//		ppi = FUNC_X(LoadPanelInfo)(abActive);
//
//	if (ppi) {
//		ppi->cbSize = sizeof(*ppi);
//
//		int n = min(ppi->nMaxFarColors, countof(gFarInfo.nFarColors));
//		if (n && ppi->nFarColors) memmove(gFarInfo.nFarColors, ppi->nFarColors, n);
//		gFarInfo.nFarInterfaceSettings = ppi->nFarInterfaceSettings;
//		gFarInfo.nFarPanelSettings = ppi->nFarPanelSettings;
//		gFarInfo.bFarPanelAllowed = TRUE;
//	}
//
//	return ppi;
//}

void ReloadPanelsInfo()
{
	TODO("�������� ����� ACTL_GETWINDOW ���-��?");
	// ���� ��� � ������ ���� ������
	CheckVarsInitialized();
	// ���� �������� ������������� ������ - ����� �������� ������������������ � GUI
	RECT rcLeft = pviLeft.PanelRect;
	BOOL bLeftVisible = pviLeft.Visible;
	RECT rcRight = pviRight.PanelRect;
	BOOL bRightVisible = pviRight.Visible;

	if (gFarVersion.dwVerMajor==1)
		ReloadPanelsInfoA();
	else if (gFarVersion.dwBuild>=FAR_Y_VER)
		FUNC_Y(ReloadPanelsInfoW)();
	else
		FUNC_X(ReloadPanelsInfoW)();

	// �������� gFarInfo (������������ � RgnDetect)
	CeFullPanelInfo* p = pviLeft.hView ? &pviLeft : &pviRight;
	int n = min(p->nMaxFarColors, countof(gFarInfo.nFarColors));

	if (n && p->nFarColors) memmove(gFarInfo.nFarColors, p->nFarColors, n);

	gFarInfo.nFarInterfaceSettings = p->nFarInterfaceSettings;
	gFarInfo.nFarPanelSettings = p->nFarPanelSettings;
	gFarInfo.bFarPanelAllowed = TRUE;
	// ��������� �������
	gFarInfo.bFarLeftPanel = pviLeft.Visible;
	gFarInfo.FarLeftPanel.PanelRect = pviLeft.PanelRect;
	gFarInfo.bFarRightPanel = pviRight.Visible;
	gFarInfo.FarRightPanel.PanelRect = pviRight.PanelRect;

	if (pviLeft.hView)
	{
		if (bLeftVisible && pviLeft.Visible)
		{
			if (memcmp(&rcLeft, &pviLeft.PanelRect, sizeof(RECT)))
				pviLeft.RegisterPanelView();
		}
	}

	if (pviRight.hView)
	{
		if (bRightVisible && pviRight.Visible)
		{
			if (memcmp(&rcRight, &pviRight.PanelRect, sizeof(RECT)))
				pviRight.RegisterPanelView();
		}
	}
}


//BOOL IsLeftPanelActive()
//{
//	BOOL lbLeftActive = FALSE;
//	if (gFarVersion.dwVerMajor==1)
//		lbLeftActive = IsLeftPanelActiveA();
//	else if (gFarVersion.dwBuild>=FAR_Y_VER)
//		lbLeftActive = FUNC_Y(IsLeftPanelActive)();
//	else
//		lbLeftActive = FUNC_X(IsLeftPanelActive)();
//	return lbLeftActive;
//}

//WORD PopUngetBuffer(BOOL abRemove, PINPUT_RECORD lpDst)
//{
//	if (gnUngetCount<1) {
//		lpDst->EventType = 0;
//	} else {
//		*lpDst = girUnget[0];
//		BOOL lbNeedPop = FALSE;
//		if (girUnget[0].EventType == EVENT_TYPE_REDRAW)
//			abRemove = TRUE; // ��� - ����� ������� �� ������
//
//		if (girUnget[0].EventType == KEY_EVENT) {
//			_ASSERTE(((short)girUnget[0].Event.KeyEvent.wRepeatCount) > 0);
//			lpDst->Event.KeyEvent.wRepeatCount = 1;
//
//			if (girUnget[0].Event.KeyEvent.wRepeatCount == 1) {
//				lbNeedPop = abRemove;
//			} else if (abRemove) {
//				girUnget[0].Event.KeyEvent.wRepeatCount --;
//			}
//		} else {
//			lbNeedPop = abRemove;
//		}
//
//		if (lbNeedPop) {
//			_ASSERTE(abRemove);
//			gnUngetCount --;
//			_ASSERTE(gnUngetCount >= 0);
//			if (gnUngetCount > 0) {
//				// ��������� � ������ �� ��� �������� � ������
//				memmove(girUnget, girUnget+1, sizeof(girUnget[0])*gnUngetCount);
//			}
//			girUnget[gnUngetCount].EventType = 0;
//		}
//	}
//
//	return lpDst->EventType;
//}

//BOOL GetBufferInput(BOOL abRemove, PINPUT_RECORD lpBuffer, DWORD nBufSize, LPDWORD lpNumberOfEventsRead)
//{
//	if (gnUngetCount<0)
//	{
//		_ASSERTE(gnUngetCount>=0);
//		gnUngetCount = 0;
//		*lpNumberOfEventsRead = 0;
//		return FALSE;
//	}
//	if (nBufSize < 1)
//	{
//		_ASSERTE(nBufSize>=1);
//		*lpNumberOfEventsRead = 0;
//		return FALSE;
//	}
//
//	BOOL lbRedraw = FALSE, lbSetEnvVar = FALSE;
//	WORD wType = 0;
//	PINPUT_RECORD p = lpBuffer;
//
//	while (nBufSize && gnUngetCount)
//	{
//		if ((wType = PopUngetBuffer(abRemove, p)) == 0)
//			break; // ����� ��������
//		if (wType == EVENT_TYPE_REDRAW)
//		{
//			lbRedraw = TRUE;
//		}
//		else
//		{
//			if (abRemove && gnUngetCount && girUnget[0].EventType == EVENT_TYPE_REDRAW)
//			{
//				lbSetEnvVar = TRUE;
//			}
//			nBufSize--; p++;
//			if (!abRemove) break; // � ������ Peek - ���������� �� ����� ������ �������s
//		}
//	}
//
//	//// ���� ���������� �� ������� EVENT_TYPE_REDRAW - ���������� ���������� EnvVar
//	//// SetEnvironmentVariable(TH_ENVVAR_NAME, TH_ENVVAR_ACTIVE);
//	//if (girUnget[0].EventType == EVENT_TYPE_REDRAW)
//	//{
//	//	lbRedraw = TRUE;
//	//	gnUngetCount --;
//	//	_ASSERTE(gnUngetCount >= 0);
//	//	if (gnUngetCount > 0) {
//	//		// ��������� � ������ �� ��� �������� � ������
//	//		memmove(girUnget, girUnget+1, sizeof(girUnget[0])*gnUngetCount);
//	//	}
//	//	girUnget[gnUngetCount].EventType = 0;
//	//}
//
//	//int nMax = countof(girUnget);
//	//if (gnUngetCount>=nMax) {
//	//	_ASSERTE(gnUngetCount==nMax);
//	//	if (gnUngetCount>nMax) gnUngetCount = nMax;
//	//}
//
//	//nMax = min(gnUngetCount,(int)nBufSize);
//
//	//int i = 0, j = 0;
//	//while (i < nMax && j < gnUngetCount) {
//	//	if (girUnget[j].EventType == EVENT_TYPE_REDRAW) {
//	//		j++; lbRedraw = TRUE; continue;
//	//	}
//
//	//	lpBuffer[i++] = girUnget[j++];
//	//}
//	//nMax = i;
//
//	//if (abRemove) {
//	//	gnUngetCount -= j;
//	//	_ASSERTE(gnUngetCount >= 0);
//	//	if (gnUngetCount > 0) {
//	//		// ��������� � ������ �� ��� �������� � ������
//	//		memmove(girUnget, girUnget+nMax, sizeof(girUnget[0])*gnUngetCount);
//	//		girUnget[gnUngetCount].EventType = 0;
//	//	}
//	//}
//
//	if (lbRedraw || lbSetEnvVar)
//	{
//		gbWaitForKeySequenceEnd = (gnUngetCount > 0) && !lbSetEnvVar;
//		UpdateEnvVar(TRUE);
//
//		if (IsThumbnailsActive(FALSE))
//		{
//			if (lbRedraw && !gbWaitForKeySequenceEnd)
//				OnReadyForPanelsReload();
//		}
//	}
//
//	*lpNumberOfEventsRead = (DWORD)(p - lpBuffer);
//
//	return TRUE;
//}

//BOOL UngetBufferInput(WORD nCount, PINPUT_RECORD lpOneInput)
//{
//	_ASSERTE(nCount);
//	if (gnUngetCount<0) {
//		_ASSERTE(gnUngetCount>=0);
//		gnUngetCount = 0;
//	}
//	int nMax = countof(girUnget);
//	if (gnUngetCount>=nMax) {
//		_ASSERTE(gnUngetCount==nMax);
//		if (gnUngetCount>nMax) gnUngetCount = nMax;
//		return FALSE;
//	}
//
//	girUnget[gnUngetCount] = *lpOneInput;
//	if (lpOneInput->EventType == KEY_EVENT) {
//		girUnget[gnUngetCount].Event.KeyEvent.wRepeatCount = nCount;
//	}
//	gnUngetCount ++;
//
//	return TRUE;
//}

void ResetUngetBuffer()
{
	gnConsoleChanges = 0;
	gbConsoleChangesSyncho = false;
	//gnUngetCount = 0;
	//gbWaitForKeySequenceEnd = false;
}

// ������ �������������� ��� ����� Synchro ��� ����� PeekConsoleInput � FAR1
void OnReadyForPanelsReload()
{
	if (!gnConsoleChanges)
		return;

	gbConsoleChangesSyncho = false;
	DWORD nCurChanges = gnConsoleChanges; gnConsoleChanges = 0;
	// �������, ����� RgnDetect ��������� ��� ����� ������ � �������.
	// ��� ����� ����� �������� ��������� ���������� ����
	gFarInfo.bFarPanelInfoFilled = gFarInfo.bFarLeftPanel = gFarInfo.bFarRightPanel = FALSE;
	gpRgnDetect->PrepareTransparent(&gFarInfo, gcrCurColors);
	gnRgnDetectFlags = gpRgnDetect->GetFlags();
#ifdef _DEBUG

	if (!gpDbgDlg)
	{
		gpDbgDlg = new MFileMapping<DetectedDialogs>();
		gpDbgDlg->InitName(CEPANELDLGMAPNAME, GetCurrentProcessId());
		gpDbgDlg->Create();
	}

	gpDbgDlg->SetFrom(gpRgnDetect->GetDetectedDialogsPtr());
#endif
	WARNING("���� ������ ������ (������� ��������/������) - �� �������� ��������� ������");

	if (!CheckWindows())
	{
		// ��������/�����������������?
	}
	else
	{
		//if (pviLeft.hView || pviRight.hView) {
		//	ReloadPanelsInfo();
		//	/* ����� ��������� ��������� ������� - ����� �������� "���������� �������"? */
		//	CeFullPanelInfo* p = pviLeft.hView ? &pviLeft : &pviRight;
		//	gFarInfo.bFarPanelInfoFilled = TRUE;
		//	gFarInfo.bFarLeftPanel = (pviLeft.Visible!=0);
		//	gFarInfo.FarLeftPanel.PanelRect = pviLeft.PanelRect;
		//	gFarInfo.bFarRightPanel = (pviRight.Visible!=0);
		//	gFarInfo.FarRightPanel.PanelRect = pviRight.PanelRect;
		//	gpRgnDetect->PrepareTransparent(&gFarInfo, gcrColors);
		//}
		//if (!gbWaitForKeySequenceEnd)
		//|| girUnget[0].EventType == EVENT_TYPE_REDRAW)
		{
			if (pviLeft.hView || pviRight.hView)
			{
				ReloadPanelsInfo();
			}

			if (pviLeft.hView)
			{
				pviLeft.DisplayReloadPanel();
				//Inva lidateRect(pviLeft.hView, NULL, FALSE);
				pviLeft.Invalidate();
			}

			if (pviRight.hView)
			{
				pviRight.DisplayReloadPanel();
				//Inva lidateRect(pviRight.hView, NULL, FALSE);
				pviRight.Invalidate();
			}
		}
	}

	//WARNING("��������� ������ ���� FAR � ���� �������� �� ������ - ����� ����� (� �������� �����������������)");
	//WARNING("��������� � ��������, ���� ��������� ������ ������ - �������� PanelView");
	//!. ������� ��������� �� ����� ������ ��������� �����!
	//   ��� ��������� ��� ����, ����� ���������� ������ �� ������������� �������
	//�����
	//1. ������ ������ -> ������� ���������� � ��������������, ��������� ������, � ��������� ���������
	//   ���������� � �������������� (���� �� ���������) �������� � GUI
	//   ���� ������ ������ �������� - ��������� ��������� ����������� � GUI - ��� ���� ������� apiShowWindow
	//2. ������ �������� -> �������� ������ ������ � ������������������� � GUI
}

BOOL WINAPI OnPrePeekConsole(HANDLE hInput, PINPUT_RECORD lpBuffer, DWORD nBufSize, LPDWORD lpNumberOfEventsRead, BOOL* pbResult)
{
	// ������ ��� FAR1, � FAR2 - ����� Synchro!
	if (gFarVersion.dwVerMajor == 1)
	{
		// ��������� ������ ���� ��� ��������� ������������ ������� (������ Synchro � Far2)
		if (nBufSize == 1)
		{
			if (gbSynchoRedrawPanelRequested)
			{
				ProcessSynchroEventW(SE_COMMONSYNCHRO, SYNCHRO_REDRAW_PANEL);
			}
			else if (gnConsoleChanges && !gbConsoleChangesSyncho)
			{
				gbConsoleChangesSyncho = true;
			}
			else if (gbConsoleChangesSyncho)
			{
				OnReadyForPanelsReload();
			}

			// �������� ������� - ���������� �������
			if (gpLastSynchroArg)
			{
				ProcessSynchroEventW(SE_COMMONSYNCHRO, gpLastSynchroArg);
			}
		}
	}
	else
	{
		if (gnConsoleChanges && !gbConsoleChangesSyncho)
		{
			gbConsoleChangesSyncho = true;
			ExecuteInMainThread(SYNCHRO_RELOAD_PANELS);
		}
	}

	if (!lpBuffer || !lpNumberOfEventsRead)
	{
		*pbResult = FALSE;
		return FALSE; // ������ � ����������
	}

	//// ���� � girUnget ���-�� ���� ������� �� ���� � FALSE (����� OnPostPeekConsole �� ����� ������)");
	//if (gnUngetCount)
	//{
	//	if (GetBufferInput(FALSE/*abRemove*/, lpBuffer, nBufSize, lpNumberOfEventsRead))
	//		return FALSE; // PeekConsoleInput & OnPostPeekConsole �� ����� ������
	//}
	//// ��� FAR1 - �������� ACTL_SYNCHRO
	//if ((gpLastSynchroArg || gbSynchoRedrawPanelRequested) // ������� �������
	//	&& nBufSize == 1  // ������ ����� ������ ������ == 1 - ��������� ��� ��� �����
	//	&& gFarVersion.dwVerMajor==1) // FAR1
	//{
	//	if (gbSynchoRedrawPanelRequested)
	//		ProcessSynchroEventW(SE_COMMONSYNCHRO, SYNCHRO_REDRAW_PANEL);
	//	if (gpLastSynchroArg)
	//		ProcessSynchroEventW(SE_COMMONSYNCHRO, gpLastSynchroArg);
	//}
	return TRUE; // ���������� ��� ���������
}

BOOL WINAPI OnPostPeekConsole(HANDLE hInput, PINPUT_RECORD lpBuffer, DWORD nBufSize, LPDWORD lpNumberOfEventsRead, BOOL* pbResult)
{
	if (!lpBuffer || !lpNumberOfEventsRead)
	{
		*pbResult = FALSE;
		return FALSE; // ������ � ����������
	}

	// �������� ��������� �������, �� girUnget �� �������
	if (*lpNumberOfEventsRead)
	{
		if (ProcessConsoleInput(FALSE/*abReadMode*/, lpBuffer, nBufSize, lpNumberOfEventsRead))
		{
			if (*lpNumberOfEventsRead == 0)
			{
				*pbResult = FALSE;
			}

			return FALSE; // ��������� � ���������� �������
		}
	}

	return TRUE; // ���������� ��� ���������
}

BOOL WINAPI OnPreReadConsole(HANDLE hInput, PINPUT_RECORD lpBuffer, DWORD nBufSize, LPDWORD lpNumberOfEventsRead, BOOL* pbResult)
{
	if (!lpBuffer || !lpNumberOfEventsRead)
	{
		*pbResult = FALSE;
		return FALSE; // ������ � ����������
	}

	//// ���� � girUnget ���-�� ���� ������� �� ���� � FALSE (����� OnPostPeekConsole �� ����� ������)");
	//if (gnUngetCount)
	//{
	//	if (GetBufferInput(TRUE/*abRemove*/, lpBuffer, nBufSize, lpNumberOfEventsRead))
	//		return FALSE; // ReadConsoleInput & OnPostReadConsole �� ����� ������
	//}
	return TRUE; // ���������� ��� ���������
}

BOOL WINAPI OnPostReadConsole(HANDLE hInput, PINPUT_RECORD lpBuffer, DWORD nBufSize, LPDWORD lpNumberOfEventsRead, BOOL* pbResult)
{
	if (!lpBuffer || !lpNumberOfEventsRead)
	{
		*pbResult = FALSE;
		return FALSE; // ������ � ����������
	}

	// �������� ��������� �������, ���� ���� - ��������� ����� ������� � girUnget
	if (*lpNumberOfEventsRead)
	{
		if (ProcessConsoleInput(TRUE/*abReadMode*/, lpBuffer, nBufSize, lpNumberOfEventsRead))
		{
			if (*lpNumberOfEventsRead == 0)
			{
				*pbResult = FALSE;
			}

			return FALSE; // ��������� � ���������� �������
		}
	}

	return TRUE; // ���������� ��� ���������
}




//lpBuffer
//The data to be written to the console screen buffer. This pointer is treated as the origin of a
//two-dimensional array of CHAR_INFO structures whose size is specified by the dwBufferSize parameter.
//The total size of the array must be less than 64K.
//
//dwBufferSize
//The size of the buffer pointed to by the lpBuffer parameter, in character cells.
//The X member of the COORD structure is the number of columns; the Y member is the number of rows.
//
//dwBufferCoord
//The coordinates of the upper-left cell in the buffer pointed to by the lpBuffer parameter.
//The X member of the COORD structure is the column, and the Y member is the row.
//
//lpWriteRegion
//A pointer to a SMALL_RECT structure. On input, the structure members specify the upper-left and lower-right
//coordinates of the console screen buffer rectangle to write to.
//On output, the structure members specify the actual rectangle that was used.
BOOL WINAPI OnPreWriteConsoleOutput(HANDLE hOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion)
{
	WARNING("����� ���������� ����������� view - ������ �� ������� ��������� ������� gpRgnDetect �� �������");

	if (gpRgnDetect && lpBuffer && lpWriteRegion)
	{
#ifdef SHOW_WRITING_RECTS

		if (IsDebuggerPresent())
		{
			wchar_t szDbg[80]; StringCchPrintf(szDbg, countof(szDbg), L"ConEmuTh.OnPreWriteConsoleOutput( {%ix%i} - {%ix%i} )\n",
			                                   lpWriteRegion->Left, lpWriteRegion->Top, lpWriteRegion->Right, lpWriteRegion->Bottom);
			OutputDebugStringW(szDbg);
		}

#endif
		SMALL_RECT rcFarRect; GetFarRect(&rcFarRect);
		gpRgnDetect->SetFarRect(&rcFarRect);
		gpRgnDetect->OnWriteConsoleOutput(lpBuffer, dwBufferSize, dwBufferCoord, lpWriteRegion, gcrCurColors);
		WARNING("����� ���������� ������ ������������ - ������� ������ �� ���������, � �������� �� ���� ������?");
		DWORD nChanges = 0;
		RECT rcTest = {0};
		RECT rcWrite =
		{
			lpWriteRegion->Left-rcFarRect.Left,lpWriteRegion->Top-rcFarRect.Top,
			lpWriteRegion->Right+1-rcFarRect.Left,lpWriteRegion->Bottom+1-rcFarRect.Top
		};

		if (pviLeft.hView)
		{
			WARNING("��������� � far/w");

			WARNING("IntersectRect �� ��������, ���� ��� ���������?");
			RECT rcPanel = pviLeft.WorkRect;
			rcPanel.bottom++;
			if (IntersectRect(&rcTest, &rcPanel, &rcWrite))
			{
				nChanges |= 1; // 1-left, 2-right.
			}
		}

		if (pviRight.hView)
		{
			WARNING("��������� � far/w");

			WARNING("IntersectRect �� ��������, ���� ��� ���������?");
			RECT rcPanel = pviRight.WorkRect;
			rcPanel.bottom++;
			if (IntersectRect(&rcTest, &rcPanel, &rcWrite))
			{
				nChanges |= 2; // 1-left, 2-right.
			}
		}

		//if (pviLeft.hView || pviRight.hView)
		if (nChanges)
		{
			//bool lbNeedSynchro = (gn ConsoleChanges == 0 && gFarVersion.dwVerMajor >= 2);
			// 1-left, 2-right. �� ���� - ������ ���, �.�. ��� �� ����������� ��� ���������� ����������!
			//gnConsoleChanges |= 3;
			gnConsoleChanges |= nChanges;
			//if (lbNeedSynchro)
			//{
			//	ExecuteInMainThread(SYNCHRO_RELOAD_PANELS);
			//}
		}
	}

	return TRUE; // ���������� ��� ���������
}


//// ������� TRUE, ���� ��� Unget
//BOOL ProcessKeyPress(CeFullPanelInfo* pi, BOOL abReadMode, PINPUT_RECORD lpBuffer)
//{
//	_ASSERTE(lpBuffer->EventType == KEY_EVENT);
//
//	// ��������������� �������
//	WORD vk = lpBuffer->Event.KeyEvent.wVirtualKeyCode;
//	BOOL lbWasUnget = FALSE;
//
//	if (vk == VK_UP || vk == VK_DOWN || vk == VK_LEFT || vk == VK_RIGHT)
//	{
//		if (!lpBuffer->Event.KeyEvent.bKeyDown) {
//			switch (vk) {
//				case VK_LEFT:  lpBuffer->Event.KeyEvent.wVirtualKeyCode = VK_UP;   break;
//				case VK_RIGHT: lpBuffer->Event.KeyEvent.wVirtualKeyCode = VK_DOWN; break;
//			}
//
//		} else {
//			// ������������
//			int n = 0;
//			switch (vk) {
//				case VK_UP: {
//					n = min(pi->CurrentItem,pi->nXCount);
//				} break;
//				case VK_DOWN: {
//					n = min((pi->ItemsNumber-pi->CurrentItem-1),pi->nXCount);
//				} break;
//				case VK_LEFT: {
//					lpBuffer->Event.KeyEvent.wVirtualKeyCode = VK_UP;
//				} break;
//				case VK_RIGHT: {
//					lpBuffer->Event.KeyEvent.wVirtualKeyCode = VK_DOWN;
//				} break;
//			}
//
//			lbWasUnget = (n > 1);
//			while ((n--) > 1) {
//				UngetBufferInput(lpBuffer);
//			}
//		}
//	}
//	return lbWasUnget;
//}

// ��������� ������ ������ � ��� ������������� ��������� SYNCHRO_REDRAW_PANEL
// ��������������� ��� VK_LEFT/RIGHT/UP/DOWN/PgUp/PgDn/WHEEL_UP/WHEEL_DOWN
// �����, ���� �� ���������� ��� ��� - �� �������� TopPanelItem!
// ������� TRUE, ���� ������ ���� �����������
BOOL ProcessConsoleInput(BOOL abReadMode, PINPUT_RECORD lpBuffer, DWORD nBufSize, LPDWORD lpNumberOfEventsRead)
{
	static bool sbInClearing = false;

	if (sbInClearing)
	{
		// ���� ��� ���� ���������� ������, ����� ������ �� ���� ������������ �������
		return FALSE;
	}

	// ������������ ������ ��������� ������� (��� ������� �� ������ ������� �� ������ �����!)
	if (nBufSize != 1 || !lpNumberOfEventsRead || *lpNumberOfEventsRead != 1)
	{
		return FALSE;
	}

	// �������� ���������� ��������
	CeFullPanelInfo* pi = IsThumbnailsActive(TRUE/*abFocusRequired*/);

	if (!pi)
	{
		return FALSE; // ������ �������, �� ��� �� ������� ��� ������ �������� ��� �������� ��������� ������
	}

	if (!wScanCodeUp)
	{
		wScanCodeUp = MapVirtualKey(VK_UP, 0/*MAPVK_VK_TO_VSC*/);
		wScanCodeDown = MapVirtualKey(VK_DOWN, 0/*MAPVK_VK_TO_VSC*/);
#ifdef _DEBUG
		wScanCodeLeft = MapVirtualKey(VK_LEFT, 0/*MAPVK_VK_TO_VSC*/);
		wScanCodeRight = MapVirtualKey(VK_RIGHT, 0/*MAPVK_VK_TO_VSC*/);
		wScanCodePgUp = MapVirtualKey(VK_PRIOR, 0/*MAPVK_VK_TO_VSC*/);
		wScanCodePgDn = MapVirtualKey(VK_NEXT, 0/*MAPVK_VK_TO_VSC*/);
#endif
	}

	WARNING("��������� ���� �� DWORD-�� ���� �� ������� ������� ��������");
	WARNING("������� ��� ������ GUI");
	WARNING("���� ���� ���� �� ���� ������ - ������ ������������� ������");
	PanelViewMode PVM = pi->PVM;
	BOOL lbWasChanges = FALSE;
	int iCurItem, iTopItem, iShift = 0;

	if (pi->bRequestItemSet)
	{
		iCurItem = pi->ReqCurrentItem; iTopItem = pi->ReqTopPanelItem;
	}
	else
	{
		iCurItem = pi->CurrentItem; iTopItem = max(pi->TopPanelItem,pi->ReqTopPanelItem);
	}

	// ������ � ��� �������. � ������ - ��������� �����, � ��������� � ����� Unget.
	PINPUT_RECORD p = lpBuffer;
	//PINPUT_RECORD pEnd = lpBuffer+*lpNumberOfEventsRead;
	//PINPUT_RECORD pReplace = lpBuffer;
	//while (p != pEnd)
	{
		bool bEraseEvent = false;

		if (p->EventType == KEY_EVENT)
		{
			int iCurKeyShift = 0;
			// ��������������� �������
			WORD vk = p->Event.KeyEvent.wVirtualKeyCode;

			if (vk == VK_UP || vk == VK_DOWN || vk == VK_LEFT || vk == VK_RIGHT
			        || vk == VK_PRIOR || vk == VK_NEXT)
			{
				if (!(p->Event.KeyEvent.dwControlKeyState
				        & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED|LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED|SHIFT_PRESSED)))
				{
					// ������������
					int n = 1;

					switch(vk)
					{
						case VK_UP:
						{
							//if (PVM == pvm_Thumbnails)
							//	n = min(pi->CurrentItem,pi->nXCountFull);
							DEBUGSTRCTRL(L"ProcessConsoleInput(VK_UP)\n");

							if (PVM == pvm_Thumbnails)
								iCurKeyShift = -pi->nXCountFull;
							else
								iCurKeyShift = -1;
						} break;
						case VK_DOWN:
						{
							//if (PVM == pvm_Thumbnails)
							//	n = min((pi->ItemsNumber-pi->CurrentItem-1),pi->nXCountFull);
							DEBUGSTRCTRL(L"ProcessConsoleInput(VK_DOWN)\n");

							if (PVM == pvm_Thumbnails)
								iCurKeyShift = pi->nXCountFull;
							else
								iCurKeyShift = 1;
						} break;
						case VK_LEFT:
						{
							//p->Event.KeyEvent.wVirtualKeyCode = VK_UP;
							//p->Event.KeyEvent.wVirtualScanCode = wScanCodeUp;
							//if (PVM != pvm_Thumbnails)
							//	n = min(pi->CurrentItem,pi->nYCountFull);
							DEBUGSTRCTRL(L"ProcessConsoleInput(VK_LEFT)\n");

							if (PVM != pvm_Thumbnails)
								iCurKeyShift = -pi->nYCountFull;
							else
								iCurKeyShift = -1;
						} break;
						case VK_RIGHT:
						{
							//p->Event.KeyEvent.wVirtualKeyCode = VK_DOWN;
							//p->Event.KeyEvent.wVirtualScanCode = wScanCodeDown;
							//if (PVM != pvm_Thumbnails)
							//	n = min((pi->ItemsNumber-pi->CurrentItem-1),pi->nYCountFull);
							DEBUGSTRCTRL(L"ProcessConsoleInput(VK_RIGHT)\n");

							if (PVM != pvm_Thumbnails)
								iCurKeyShift = pi->nYCountFull;
							else
								iCurKeyShift = 1;
						} break;
						case VK_PRIOR:
						{
							//p->Event.KeyEvent.wVirtualKeyCode = VK_UP;
							//p->Event.KeyEvent.wVirtualScanCode = wScanCodeUp;
							//n = min(pi->CurrentItem,pi->nXCountFull*pi->nYCountFull);
							DEBUGSTRCTRL(L"ProcessConsoleInput(VK_PRIOR)\n");
							int nRowCol = (PVM == pvm_Thumbnails) ? pi->nXCountFull : pi->nYCountFull;

							if (iCurItem >= (iTopItem + nRowCol))
							{
								int nCorrection = (PVM == pvm_Thumbnails)
								                  ? (iCurItem % nRowCol)
								                  : 0;
								// ���� PgUp ����� ����� ������� ������� �� �� ������� ������
								iCurKeyShift = (iTopItem - iCurItem);
							}
							else
							{
								iCurKeyShift = -(pi->nXCountFull*pi->nYCountFull);
							}
						} break;
						case VK_NEXT:
						{
							//p->Event.KeyEvent.wVirtualKeyCode = VK_DOWN;
							//p->Event.KeyEvent.wVirtualScanCode = wScanCodeUp;
							//n = min((pi->ItemsNumber-pi->CurrentItem-1),pi->nXCountFull*pi->nYCountFull);
							DEBUGSTRCTRL(L"ProcessConsoleInput(VK_NEXT)\n");
							int nRowCol = (PVM == pvm_Thumbnails) ? pi->nXCountFull : pi->nYCountFull;
							int nFull = (pi->nXCountFull*pi->nYCountFull);

							if (iCurItem >= iTopItem && iCurItem < (iTopItem + nFull - nRowCol))
							{
								// ���� PgDn ����� ����� ������� ������� �� �� ��������� ������
								int nCorrection = (PVM == pvm_Thumbnails)
								                  ? (iCurItem % nRowCol)
								                  : (nRowCol - 1);
								iCurKeyShift = (iTopItem + nFull - nRowCol - iCurItem) + nCorrection;
							}
							else
							{
								iCurKeyShift = nFull;
							}
						} break;
					}

					// ���� ��� ������� - �� �������������� ����� ��������� � Unget �����
					if (iCurKeyShift)
					{
						// ��� ������� �� ������ ����� ������
						bEraseEvent = true;

						// ��������� � ������ ������
						if (p->Event.KeyEvent.bKeyDown)
						{
							iShift += iCurKeyShift;
						}

						//if (abReadMode && n > 1)
						//{
						//	pFirstReplace = p+1;
						//	UngetBufferInput(n-1, p);
						//}
					}

					//end: if (vk == VK_UP || vk == VK_DOWN || vk == VK_LEFT || vk == VK_RIGHT)
					//PRAGMA_ERROR("!!!");
					//p++; continue;
				}
			}

			//end: if (p->EventType == KEY_EVENT)
		}
		// ������ ����� ���� ������������� �����
		else if (p->EventType == MOUSE_EVENT)
		{
			WARNING("��������� Wheel, ����� �� ������ TopPanelItem");
		}
		// ��� ����� ������� ���� - ����� ����������� ��������
		else if (p->EventType == WINDOW_BUFFER_SIZE_EVENT)
		{
			if (gpRgnDetect)
				gpRgnDetect->OnWindowSizeChanged();
		}

		//// ���� ���� �������� ������� � ����� Unget - ��������� ��������� � ����� � ��� ��� ���� �� ���,
		//// �.�. ����� ���� ����� "�����������" ������� �������
		//if (pFirstReplace && abReadMode)
		//{
		//	UngetBufferInput(1,p);
		//}

		//p++;
		if (bEraseEvent)
		{
			lbWasChanges = TRUE;
			// ���� � ������ ������ ������ ������� - �������� �����
			//if (p < pEnd)
			//{
			//	memmove(pReplace, p, pEnd-p);
			//	pReplace++;
			//}
			// ����� ������ ��� ������� �� ������
			DWORD nRead = 0;
			INPUT_RECORD rr[2];

			if (!abReadMode)
			{
				sbInClearing = true;
				DEBUGSTRCTRL(L"-- removing processed event from input queue\n");
				ReadConsoleInputW(GetStdHandle(STD_INPUT_HANDLE), rr, 1, &nRead);
				DEBUGSTRCTRL(nRead ? L"-- removing succeeded\n" : L"-- removing failed\n");
				sbInClearing = false;
			}

			if ((*lpNumberOfEventsRead) >= 1)
			{
				*lpNumberOfEventsRead = (*lpNumberOfEventsRead) - 1;
			}

			bEraseEvent = false;
		}
	}
	//// ��������������� ���������� "���������" �������
	//if (pFirstReplace && abReadMode)
	//{
	//	DWORD nReady = (int)(pFirstReplace - lpBuffer);
	//	if (nReady != *lpNumberOfEventsRead)
	//	{
	//		_ASSERTE(nReady <= nBufSize);
	//		_ASSERTE(nReady < *lpNumberOfEventsRead);
	//		*lpNumberOfEventsRead = nReady;
	//	}
	//}

	// ���� � ������ ���� "����������" ������� - �������� ����������� ��������� ���������� ���������
	//if (pFirstReplace && abReadMode && gnUngetCount)
	if (iShift != 0)
	{
		_ASSERTE(lbWasChanges); lbWasChanges = TRUE;
		// ��������� ����� CurItem & TopItem ���������� ��� PanelViews
		iCurItem += iShift;

		if (iCurItem >= pi->ItemsNumber) iCurItem = pi->ItemsNumber-1;

		if (iCurItem < 0) iCurItem = 0;

		// ��������� ��������� TopItem ��� �������� ����������� �������� �������
		iTopItem = pi->CalcTopPanelItem(iCurItem, iTopItem);
#ifdef _DEBUG
		wchar_t szDbg[512];
		_wsprintf(szDbg, SKIPLEN(countof(szDbg))
		          L"Requesting panel redraw: {Cur:%i, Top:%i}.\n"
		          L"  Current state: {Cur:%i, Top:%i, Count:%i, OurTop:%i}\n"
		          L"  Current request: {%s, Cur:%i, Top=%i}\n",
		          iCurItem, iTopItem,
		          pi->CurrentItem, pi->TopPanelItem, pi->ItemsNumber, pi->OurTopPanelItem,
		          pi->bRequestItemSet ? L"YES" : L"No", pi->ReqCurrentItem, pi->ReqTopPanelItem);
		DEBUGSTRCTRL(szDbg);
#endif
		// ������� ���������� ������ � ����� ��������
		pi->RequestSetPos(iCurItem, iTopItem);
		//pi->ReqCurrentItem = iCurItem; pi->ReqTopPanelItem = iTopItem;
		//pi->bRequestItemSet = true;
		//if (!gbSynchoRedrawPanelRequested)
		//{
		//	gbWaitForKeySequenceEnd = true;
		//	UpdateEnvVar(FALSE);
		//
		//	gbSynchoRedrawPanelRequested = true;
		//	ExecuteInMainThread(SYNCHRO_REDRAW_PANEL);
		//}
		//_ASSERTE(gnUngetCount>0);
		//INPUT_RECORD r = {EVENT_TYPE_REDRAW};
		//UngetBufferInput(1,&r);
		////SetEnvironmentVariable(TH_ENVVAR_NAME, TH_ENVVAR_SCROLL);
		//gbWaitForKeySequenceEnd = true;
		//UpdateEnvVar(FALSE);
	}

	//// ������ ������ - ���� ����� ���������� ������� - �������� � ���� ������� �� Unget ������
	//if (gnUngetCount && nBufSize > *lpNumberOfEventsRead)
	//{
	//	DWORD nAdd = 0;
	//	if (GetBufferInput(abReadMode/*abRemove*/, pFirstReplace, nBufSize-(*lpNumberOfEventsRead), &nAdd))
	//		*lpNumberOfEventsRead += nAdd;
	//}
	return lbWasChanges;
}

int ShowLastError()
{
	if (gnCreateViewError)
	{
		wchar_t szErrMsg[512];
		const wchar_t* pszTempl = GetMsgW(gnCreateViewError);

		if (pszTempl && *pszTempl)
		{
			_wsprintf(szErrMsg, SKIPLEN(countof(szErrMsg)) pszTempl, gnWin32Error);

			if (gFarVersion.dwBuild>=FAR_Y_VER)
				return FUNC_Y(ShowMessageW)(szErrMsg, 0);
			else
				return FUNC_X(ShowMessageW)(szErrMsg, 0);
		}
	}

	return 0;
}

void UpdateEnvVar(BOOL abForceRedraw)
{
	//WARNING("���������� ������ ����� ����� ��������� ��������� ������� �� REDRAW");
	//if (gbWaitForKeySequenceEnd)
	//{
	//	SetEnvironmentVariable(TH_ENVVAR_NAME, TH_ENVVAR_SCROLL);
	//}
	//else
	if (IsThumbnailsActive(FALSE/*abFocusRequired*/))
	{
		SetEnvironmentVariable(TH_ENVVAR_NAME, TH_ENVVAR_ACTIVE);
		//abForceRedraw = TRUE;
	}
	else
	{
		SetEnvironmentVariable(TH_ENVVAR_NAME, NULL);
	}

	//if (abForceRedraw)
	//{
	//	if (pviLeft.hView && IsWindowVisible(pviLeft.hView))
	//		Inva lidateRect(pviLeft.hView, NULL, FALSE);
	//	if (pviRight.hView && IsWindowVisible(pviRight.hView))
	//		Inva lidateRect(pviRight.hView, NULL, FALSE);
	//}
}

CeFullPanelInfo* IsThumbnailsActive(BOOL abFocusRequired)
{
	if (pviLeft.hView == NULL && pviRight.hView == NULL)
		return NULL;

	if (!CheckWindows())
		return NULL;

	CeFullPanelInfo* pi = NULL;

	if (gpRgnDetect)
	{
		DWORD dwFlags = gpRgnDetect->GetFlags();

		if ((dwFlags & FR_ACTIVEMENUBAR) == FR_ACTIVEMENUBAR)
			return NULL; // ������� ����

		if ((dwFlags & FR_FREEDLG_MASK) != 0)
			return NULL; // ���� �����-�� ������
	}

	if (abFocusRequired)
	{
		if (pviLeft.hView && pviLeft.Focus && pviLeft.Visible)
			pi = &pviLeft;
		else if (pviRight.hView && pviRight.Focus && pviRight.Visible)
			pi = &pviRight;

		// �����?
		if (pi)
		{
			if (!pi->hView || !IsWindowVisible(pi->hView))
			{
				return NULL; // ������ ��������� ������ �������� ��� �������� ��������� ������
			}
		}
	}
	else
	{
		if (pviLeft.hView && IsWindowVisible(pviLeft.hView))
			pi = &pviLeft;
		else if (pviRight.hView && IsWindowVisible(pviRight.hView))
			pi = &pviRight;
	}

	// ����� ���� PicView/MMView...
	if (pi)
	{
		RECT rc;
		GetClientRect(pi->hView, &rc);
		POINT pt = {((rc.left+rc.right)>>1),((rc.top+rc.bottom)>>1)};
		MapWindowPoints(pi->hView, ghConEmuRoot, &pt, 1);
		HWND hChild[2];
		hChild[0] = ChildWindowFromPointEx(ghConEmuRoot, pt, CWP_SKIPINVISIBLE|CWP_SKIPTRANSPARENT);
		// ������ �������� ������������� ����
		MapWindowPoints(ghConEmuRoot, NULL, &pt, 1);
		hChild[1] = WindowFromPoint(pt);

		for(int i = 0; i <= 1; i++)
		{
			// � ��������, ����� ���� � NULL, ���� ���������� ������ � "����������" ����� hView
			if (hChild[i] && hChild[i] != pi->hView)
			{
				wchar_t szClass[128];

				if (GetClassName(hChild[i], szClass, 128))
				{
					if (lstrcmpi(szClass, L"FarPictureViewControlClass") == 0)
						return NULL; // ������� PicView!

					if (lstrcmpi(szClass, L"FarMultiViewControlClass") == 0)
						return NULL; // ������� MMView!
				}
			}
		}
	}

	return pi;
}

// ������ ������� true, ���� ������� ������ ������ (��� �������� ��� ��� ����� ����)
bool CheckWindows()
{
	bool lbRc = false;
	// ��������� �������� ������� �������� ���� ����� Far, �� ������ ACTL_GETSHORTWINDOWINFO
	bool lbFarPanels = false;

	//if (gFarVersion.dwVerMajor==1)
	//	gbLastCheckWindow = CheckWindowsA();
	//else if (gFarVersion.dwBuild>=FAR_Y_VER)
	//	gbLastCheckWindow = FUNC_Y(CheckWindows)();
	//else
	//	gbLastCheckWindow = FUNC_X(CheckWindows)();
	//return gbLastCheckWindow;
	if (gFarVersion.dwVerMajor==1)
		lbFarPanels = CheckFarPanelsA();
	else if (gFarVersion.dwBuild>=FAR_Y_VER)
		lbFarPanels = FUNC_Y(CheckFarPanelsW)();
	else
		lbFarPanels = FUNC_X(CheckFarPanelsW)();

	// ��������� ���������� �������.
	if (gbFarPanelsReady != lbFarPanels)
		gbFarPanelsReady = lbFarPanels;

	// ������, ���� API �������, ��� ������ ���� � �������
	if (lbFarPanels && gpRgnDetect)
	{
		//WARNING: ������� ��� ������ ���� "����������"
		// ���������� gnRgnDetectFlags, �.�. gpRgnDetect ����� ��������� � �������� �������������
		DWORD dwFlags = gnRgnDetectFlags; // gpRgnDetect->GetFlags();

		// ����� ������� ������ �� ����������?
		if ((dwFlags & (FR_LEFTPANEL|FR_RIGHTPANEL|FR_FULLPANEL)) != 0)
		{
			// ��� ��������
			if ((dwFlags & FR_FREEDLG_MASK) == 0)
			{
				// � ��� ��������������� ����
				if ((dwFlags & FR_ACTIVEMENUBAR) != FR_ACTIVEMENUBAR)
				{
					lbRc = true;
				}
			}
		}
	}

	//gbLastCheckWindow = lbRc;
	return lbRc;
}

void CheckVarsInitialized()
{
	if (!gpRgnDetect)
	{
		gpRgnDetect = new CRgnDetect();
	}

	// ������ ��������� � SetStartupInfo
	_ASSERTE(gpImgCache!=NULL);
	//if (!gpImgCache) {
	//	gpImgCache = new CImgCache(ghPluginModule);
	//}

	//CeFullPanelInfo* p = pviLeft.hView ? &pviLeft : &pviRight;

	if (gFarInfo.cbSize == 0)
	{
		gFarInfo.cbSize = sizeof(gFarInfo);
		gFarInfo.FarVer = gFarVersion;
		gFarInfo.nFarPID = GetCurrentProcessId();
		gFarInfo.nFarTID = GetCurrentThreadId();
		gFarInfo.bFarPanelAllowed = TRUE;
		// ��������� �� ������� ��������� PanelTabs
		gFarInfo.PanelTabs.SeparateTabs = gFarInfo.PanelTabs.ButtonColor = -1;


		// ������������� ��������� PanelTabs
		gFarInfo.PanelTabs.SeparateTabs = 1; gFarInfo.PanelTabs.ButtonColor = 0x1B; // ���������...
		if (gFarVersion.dwVerMajor == 1)
			SettingsLoadOtherA();
		else if (gFarVersion.dwBuild>=FAR_Y_VER)
			FUNC_Y(SettingsLoadOtherW)();
		else
			FUNC_X(SettingsLoadOtherW)();

		//if (gszRootKey && *gszRootKey)
		//{
		//	int nLen = lstrlenW(gszRootKey);
		//	int cchSize = nLen+32;
		//	wchar_t* pszTabsKey = (wchar_t*)malloc(cchSize*2);
		//	_wcscpy_c(pszTabsKey, cchSize, gszRootKey);
		//	pszTabsKey[nLen-1] = 0;
		//	wchar_t* pszSlash = wcsrchr(pszTabsKey, L'\\');
		//	if (pszSlash)
		//	{
		//		_wcscpy_c(pszSlash, cchSize-(pszSlash-pszTabsKey), L"\\Plugins\\PanelTabs");
		//		HKEY hk;
		//		if (0 == RegOpenKeyExW(HKEY_CURRENT_USER, pszTabsKey, 0, KEY_READ, &hk))
		//		{
		//			DWORD dwVal, dwSize;
		//			if (!RegQueryValueExW(hk, L"SeparateTabs", NULL, NULL, (LPBYTE)&dwVal, &(dwSize = 4)))
		//				gFarInfo.PanelTabs.SeparateTabs = dwVal ? 1 : 0;
		//			if (!RegQueryValueExW(hk, L"ButtonColor", NULL, NULL, (LPBYTE)&dwVal, &(dwSize = 4)))
		//				gFarInfo.PanelTabs.ButtonColor = dwVal & 0xFF;
		//			RegCloseKey(hk);
		//		}
		//	}
		//	free(pszTabsKey);
		//}
	}

	if (!pviLeft.pSection)
	{
		pviLeft.pSection = new MSection();
	}

	if (!pviRight.pSection)
	{
		pviRight.pSection = new MSection();
	}
}

void ExecuteInMainThread(ConEmuThSynchroArg* pCmd)
{
	if (!pCmd) return;

	if (pCmd != SYNCHRO_REDRAW_PANEL && pCmd != SYNCHRO_RELOAD_PANELS)
	{
		if (gpLastSynchroArg && gpLastSynchroArg != pCmd)
		{
			LocalFree(gpLastSynchroArg); gpLastSynchroArg = NULL;
		}

		gpLastSynchroArg = pCmd;
		_ASSERTE(gpLastSynchroArg->bValid==1 && gpLastSynchroArg->bExpired==0);
	}

	if (pCmd == SYNCHRO_REDRAW_PANEL)
		gbSynchoRedrawPanelRequested = true;

	DEBUGSTRCTRL(
	    (pCmd == SYNCHRO_REDRAW_PANEL) ? L"ExecuteInMainThread(SYNCHRO_REDRAW_PANEL)\n" :
	    (pCmd == SYNCHRO_RELOAD_PANELS) ? L"ExecuteInMainThread(SYNCHRO_RELOAD_PANELS)\n" :
	    L"ExecuteInMainThread(...)\n"
	);
	BOOL lbLeftActive = FALSE;

	if (gFarVersion.dwVerMajor == 1)
	{
		// � 1.75 ����� ������� ���, �������� �����
	}
	else if (gFarVersion.dwBuild>=FAR_Y_VER)
	{
		FUNC_Y(ExecuteInMainThreadW)(pCmd);
	}
	else
	{
		FUNC_X(ExecuteInMainThreadW)(pCmd);
	}
}

int WINAPI ProcessSynchroEventW(int Event, void *Param)
{
	if (Event != SE_COMMONSYNCHRO) return 0;

	if (Param == SYNCHRO_REDRAW_PANEL)
	{
		DEBUGSTRCTRL(L"ProcessSynchroEventW(SYNCHRO_REDRAW_PANEL)\n");
		gbSynchoRedrawPanelRequested = false;

		for(int i = 0; i <= 1; i++)
		{
			CeFullPanelInfo* pp = (i==0) ? &pviLeft : &pviRight;

			if (pp->hView && pp->Visible && pp->bRequestItemSet)
			{
				// ����� �������� ����, ����� ����� �� ������������
				pp->bRequestItemSet = false;

				// � ������ - ���������� ������
				if (gFarVersion.dwVerMajor==1)
					SetCurrentPanelItemA((i==0), pp->ReqTopPanelItem, pp->ReqCurrentItem);
				else if (gFarVersion.dwBuild>=FAR_Y_VER)
					FUNC_Y(SetCurrentPanelItemW)((i==0), pp->ReqTopPanelItem, pp->ReqCurrentItem);
				else
					FUNC_X(SetCurrentPanelItemW)((i==0), pp->ReqTopPanelItem, pp->ReqCurrentItem);
			}
		}

		// ���� ��������� ���� �������� �� ��������� ��������� ���������� - �����������
		if (/*gbWaitForKeySequenceEnd &&*/ !gbConsoleChangesSyncho)
		{
			//gbWaitForKeySequenceEnd = false;
			UpdateEnvVar(FALSE);
			gbConsoleChangesSyncho = true;
			ExecuteInMainThread(SYNCHRO_RELOAD_PANELS);
		}
	}
	else if (Param == SYNCHRO_RELOAD_PANELS)
	{
		DEBUGSTRCTRL(L"ProcessSynchroEventW(SYNCHRO_RELOAD_PANELS)\n");
		_ASSERTE(gbConsoleChangesSyncho);
		gbConsoleChangesSyncho = false;
		OnReadyForPanelsReload();
	}
	else if (Param != NULL)
	{
		DEBUGSTRCTRL(L"ProcessSynchroEventW(...)\n");
		ConEmuThSynchroArg* pCmd = (ConEmuThSynchroArg*)Param;

		if (gpLastSynchroArg == pCmd) gpLastSynchroArg = NULL;

		if (pCmd->bValid == 1)
		{
			if (pCmd->bExpired == 0)
			{
				if (pCmd->nCommand == ConEmuThSynchroArg::eExecuteMacro)
				{
					PostMacro((wchar_t*)pCmd->Data);
				}
			}

			LocalFree(pCmd);
		}
	}

	return 0;
}


BOOL LoadThSet(DWORD anGuiPid/* =-1 */)
{
	BOOL lbRc = FALSE;
	MFileMapping<PanelViewSetMapping> ThSetMap;
	_ASSERTE(ghConEmuRoot!=NULL);
	DWORD nGuiPID;
	GetWindowThreadProcessId(ghConEmuRoot, &nGuiPID);

	if (anGuiPid != -1)
	{
		_ASSERTE(nGuiPID == anGuiPid);
		nGuiPID = anGuiPid;
	}

	ThSetMap.InitName(CECONVIEWSETNAME, nGuiPID);

	if (!ThSetMap.Open())
	{
		MessageBox(NULL, ThSetMap.GetErrorText(), L"ConEmuTh", MB_ICONSTOP|MB_SETFOREGROUND|MB_SYSTEMMODAL);
	}
	else
	{
		ThSetMap.GetTo(&gThSet);
		ThSetMap.CloseMap();
		lbRc = TRUE;
	}

	return lbRc;
}

//sx, sy - �������� ������� �������� ���� ������� �� ������� �������� ���� �������
//cx, cy - ���� �� 0 - �� ���������� ������ � ������ �������
BOOL GetFarRect(SMALL_RECT* prcFarRect)
{
	BOOL lbFarBuffer = FALSE;
	prcFarRect->Left = prcFarRect->Right = prcFarRect->Top = prcFarRect->Bottom = 0;

	if (gFarVersion.dwVerMajor>2
	        || (gFarVersion.dwVerMajor==2 && gFarVersion.dwBuild>=1573))
	{
		if (gFarVersion.dwBuild>=FAR_Y_VER)
			FUNC_Y(GetFarRectW)(prcFarRect);
		else
			FUNC_X(GetFarRectW)(prcFarRect);
		lbFarBuffer = (prcFarRect->Bottom && prcFarRect->Right);
	}

	return lbFarBuffer;
}

BOOL SettingsLoad(LPCWSTR pszName, DWORD* pValue)
{
	if (gFarVersion.dwVerMajor == 1)
		return SettingsLoadA(pszName, pValue);
	else if (gFarVersion.dwBuild>=FAR_Y_VER)
		return FUNC_Y(SettingsLoadW)(pszName, pValue);
	else
		return FUNC_X(SettingsLoadW)(pszName, pValue);
}
BOOL SettingsLoadReg(LPCWSTR pszRegKey, LPCWSTR pszName, DWORD* pValue)
{
	BOOL lbValue = FALSE;
	HKEY hk = NULL;
	if (!RegOpenKeyExW(HKEY_CURRENT_USER, pszRegKey, 0, KEY_READ, &hk))
	{
		DWORD dwValue, dwSize;
		if (!RegQueryValueEx(hk, pszName, NULL, NULL, (LPBYTE)&dwValue, &(dwSize=4)))
		{
			*pValue = dwValue;
			lbValue = TRUE;
		}
		//RegSetValueEx(hk, pi->bLeftPanel ? L"LeftPanelView" : L"RightPanelView", 0,
		//	REG_DWORD, (LPBYTE)&dwMode, sizeof(dwMode));
		//if (RegQueryValueEx(hk, L"LeftPanelView", NULL, NULL, (LPBYTE)dwModes, &(dwSize=4)))
		RegCloseKey(hk);
	}
	return lbValue;
}
void SettingsSave(LPCWSTR pszName, DWORD* pValue)
{
	if (gFarVersion.dwVerMajor == 1)
		return SettingsSaveA(pszName, pValue);
	else if (gFarVersion.dwBuild>=FAR_Y_VER)
		return FUNC_Y(SettingsSaveW)(pszName, pValue);
	else
		return FUNC_X(SettingsSaveW)(pszName, pValue);
}
void SettingsSaveReg(LPCWSTR pszRegKey, LPCWSTR pszName, DWORD* pValue)
{
	HKEY hk = NULL;
	if (!RegCreateKeyExW(HKEY_CURRENT_USER, pszRegKey, 0, NULL, 0, KEY_WRITE, NULL, &hk, NULL))
	{
		RegSetValueEx(hk, pszName, 0, REG_DWORD, (LPBYTE)pValue, sizeof(*pValue));
		RegCloseKey(hk);
	}
}

void SettingsLoadOther(LPCWSTR pszRegKey)
{
	_ASSERTE(gFarVersion.dwVerMajor<=2);
	if (pszRegKey && *pszRegKey)
	{
		int nLen = lstrlenW(pszRegKey);
		int cchSize = nLen+32;
		wchar_t* pszTabsKey = (wchar_t*)malloc(cchSize*2);
		_wcscpy_c(pszTabsKey, cchSize, pszRegKey);
		pszTabsKey[nLen-1] = 0;
		wchar_t* pszSlash = wcsrchr(pszTabsKey, L'\\');

		if (pszSlash)
		{
			_wcscpy_c(pszSlash, cchSize-(pszSlash-pszTabsKey), L"\\Plugins\\PanelTabs");
			HKEY hk;

			if (0 == RegOpenKeyExW(HKEY_CURRENT_USER, pszTabsKey, 0, KEY_READ, &hk))
			{
				DWORD dwVal, dwSize;

				if (!RegQueryValueExW(hk, L"SeparateTabs", NULL, NULL, (LPBYTE)&dwVal, &(dwSize = 4)))
					gFarInfo.PanelTabs.SeparateTabs = dwVal ? 1 : 0;

				if (!RegQueryValueExW(hk, L"ButtonColor", NULL, NULL, (LPBYTE)&dwVal, &(dwSize = 4)))
					gFarInfo.PanelTabs.ButtonColor = dwVal & 0xFF;

				RegCloseKey(hk);
			}
		}

		free(pszTabsKey);
	}
}