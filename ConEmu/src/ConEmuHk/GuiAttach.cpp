
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

#ifdef _DEBUG
#define DebugString(x) OutputDebugString(x)
#else
#define DebugString(x) //OutputDebugString(x)
#endif

#include <windows.h>
#include "UserImp.h"
#include "GuiAttach.h"
#include "../common/common.hpp"
#include "../common/ConEmuCheck.h"
#include "../common/WinObjects.h"

extern HMODULE ghOurModule;
extern HWND    ghConEmuWnd;     // Root! ConEmu window
extern HWND    ghConEmuWndDC;   // ConEmu DC window
extern HWND    ghConEmuWndBack; // ConEmu Back window - holder for GUI client
extern DWORD   gnHookMainThreadId;

// ���� TID ����� ���������� �� ��������� ������.
// ��������, VLC ������� ���� �� � �������, � � ������� ������.
DWORD  gnAttachGuiClientThreadId = 0;

RECT    grcConEmuClient = {}; // ��� ������ ������ ����
BOOL    gbAttachGuiClient = FALSE; // ��� ������ ������ ����
BOOL 	gbGuiClientAttached = FALSE; // ��� ������ ������ ���� (������� ������������)
BOOL 	gbGuiClientExternMode = FALSE; // ���� ����� �������� Gui-���������� ��� ������� ConEmu
struct GuiStylesAndShifts gGuiClientStyles = {}; // ��������� ������ ���� ������ ConEmu
HWND 	ghAttachGuiClient = NULL;
DWORD 	gnAttachGuiClientFlags = 0;
DWORD 	gnAttachGuiClientStyle = 0, gnAttachGuiClientStyleEx = 0;
BOOL  	gbAttachGuiClientStyleOk = FALSE;
BOOL 	gbForceShowGuiClient = FALSE; // --
//HMENU ghAttachGuiClientMenu = NULL;
RECT 	grcAttachGuiClientOrig = {0,0,100,100};
#ifdef _DEBUG
HHOOK 	ghGuiClientRetHook = NULL;
#endif

#ifdef _DEBUG
LRESULT CALLBACK GuiClientRetHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION)
	{
		CWPRETSTRUCT* p = (CWPRETSTRUCT*)lParam;
		CREATESTRUCT* pc = NULL;
		wchar_t szDbg[200]; szDbg[0] = 0;
		wchar_t szClass[127]; szClass[0] = 0;
		switch (p->message)
		{
			case WM_DESTROY:
			{
				user->getClassNameW(p->hwnd, szClass, countof(szClass));
				msprintf(szDbg, countof(szDbg), L"WM_DESTROY on 0x%08X (%s)\n", (DWORD)p->hwnd, szClass);
				break;
			}
			case WM_CREATE:
			{
				pc = (CREATESTRUCT*)p->lParam;
				if (p->lResult == -1)
				{
					wcscpy_c(szDbg, L"WM_CREATE --FAILED--\n");
				}
				break;
			}
			case WM_SIZE:
			case WM_MOVE:
			case WM_WINDOWPOSCHANGED:
			case WM_WINDOWPOSCHANGING:
			{
				WINDOWPOS *wp = (WINDOWPOS*)p->lParam;
				WORD x = LOWORD(p->lParam);
				WORD y = HIWORD(p->lParam);
				if (p->hwnd == ghAttachGuiClient)
				{
					int nDbg = 0;
					if (p->message == WM_WINDOWPOSCHANGING || p->message == WM_WINDOWPOSCHANGED)
					{
						if ((wp->x > 0 || wp->y > 0) && !isPressed(VK_LBUTTON))
						{
							if (user->getParent(p->hwnd) == ghConEmuWndBack)
							{
								//_ASSERTEX(!(wp->x > 0 || wp->y > 0));
								break;
							}
						}
					}
				}
				break;
			}
		}

		if (*szDbg)
		{
			DebugString(szDbg);
		}
	}

	return user->callNextHookEx(ghGuiClientRetHook, nCode, wParam, lParam);
}
#endif


bool CheckCanCreateWindow(LPCSTR lpClassNameA, LPCWSTR lpClassNameW, DWORD& dwStyle, DWORD& dwExStyle, HWND& hWndParent, BOOL& bAttachGui, BOOL& bStyleHidden)
{
	bAttachGui = FALSE;

#ifdef _DEBUG
	// "!dwStyle" ������� ��� shell32.dll!CExecuteApplication::_CreateHiddenDDEWindow()
	_ASSERTE(hWndParent==NULL || ghConEmuWnd == NULL || hWndParent!=ghConEmuWnd || !dwStyle);
	STARTUPINFO si = {sizeof(si)};
	GetStartupInfo(&si);
#endif
	
	if (gbAttachGuiClient && ghConEmuWndBack)
	{
		#ifdef _DEBUG
		WNDCLASS wc = {}; BOOL lbClass = FALSE;
		if ((lpClassNameW && ((DWORD_PTR)lpClassNameW) <= 0xFFFF))
		{
			lbClass = GetClassInfo((HINSTANCE)GetModuleHandle(NULL), lpClassNameW, &wc);
		}
		#endif

		DWORD nTID = GetCurrentThreadId();
		if ((nTID != gnHookMainThreadId) && (gnAttachGuiClientThreadId && nTID != gnAttachGuiClientThreadId))
		{
			_ASSERTEX(nTID==gnHookMainThreadId || !gnAttachGuiClientThreadId || (ghAttachGuiClient && user->isWindow(ghAttachGuiClient)));
		}
		else
		{
			bool lbCanAttach =
							// ������� ���� � ����������
							((dwStyle & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW)
							// ������ � �������� �����
							|| ((dwStyle & (WS_POPUP|WS_THICKFRAME)) == (WS_POPUP|WS_THICKFRAME))
							// ������� ���� ��� ���������
							|| ((dwStyle & (WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_POPUP|DS_MODALFRAME|WS_CHILDWINDOW)) == (WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX)) 
							;
			if (dwStyle & (DS_MODALFRAME|WS_CHILDWINDOW))
				lbCanAttach = false;
			else if ((dwStyle & WS_POPUP) && !(dwStyle & WS_THICKFRAME))
				lbCanAttach = false;
			else if (dwExStyle & (WS_EX_TOOLWINDOW|WS_EX_TOPMOST|WS_EX_DLGMODALFRAME|WS_EX_MDICHILD))
				lbCanAttach = false;

			if (lbCanAttach)
			{
				// ������������ ���� - ConEmu DC
				// -- hWndParent = ghConEmuWndBack; // ���� �� ��� ������� �����, ���� �� �������� WS_CHILD?

				// WS_CHILDWINDOW ����� ��������� ���������� ������, �.�. �������� � WordPad.exe ������ �����:
				// ��� ��� ���� ��������� ���������, �� �� ���������� ������ "�� ������� ������� ����� ��������"
				//// ������ �����, ���� � ��������� - �������
				//dwStyle = (dwStyle | WS_CHILDWINDOW|WS_TABSTOP) & ~(WS_THICKFRAME/*|WS_CAPTION|WS_MINIMIZEBOX|WS_MAXIMIZEBOX*/);
				bStyleHidden = (dwStyle & WS_VISIBLE) == WS_VISIBLE;
				dwStyle &= ~WS_VISIBLE; // � ��� ��������� - ����� �������
				////dwExStyle = dwExStyle & ~WS_EX_WINDOWEDGE;

				bAttachGui = TRUE;
				//gbAttachGuiClient = FALSE; // ������ ���� ���� ���������� -- ������� ����� ��������� �������� ����
				gbGuiClientAttached = TRUE; // ����� ������� ������ ������

				#ifdef _DEBUG
				if (!ghGuiClientRetHook)
					ghGuiClientRetHook = user->setWindowsHookExW(WH_CALLWNDPROCRET, GuiClientRetHook, NULL, GetCurrentThreadId());
				#endif

				//gnAttachGuiClientThreadId = nTID; -- ������� � "ghAttachGuiClient = hWindow;"
			}
			return true;
		}
	}

	if (gbGuiClientAttached /*ghAttachGuiClient*/)
	{
		return true; // � GUI ����������� - ��������� ���
	}

#ifndef _DEBUG
	return true;
#else
	if (gnHookMainThreadId && gnHookMainThreadId != GetCurrentThreadId())
		return true; // ���������, �������� �� ����� ���������� ���������/��������
	
	if ((dwStyle & (WS_POPUP|DS_MODALFRAME)) == (WS_POPUP|DS_MODALFRAME))
	{
		// ��� ������ ����� ������� ������, ��������, �� ���� ��� ��������� - assert
		_ASSERTE((dwStyle & WS_POPUP) == 0);
		return true;
	}

	if ((lpClassNameA && ((DWORD_PTR)lpClassNameA) <= 0xFFFF)
		|| (lpClassNameW && ((DWORD_PTR)lpClassNameW) <= 0xFFFF))
	{
		// ���-�� ���������
		return true;
	}
	
	// ���� �� ����� ��� ���������. dwStyle == 0x88000000.
	if ((lpClassNameW && lstrcmpW(lpClassNameW, L"CicMarshalWndClass") == 0)
		|| (lpClassNameA && lstrcmpA(lpClassNameA, "CicMarshalWndClass") == 0)
		)
	{
		return true;
	}

	// WiX
	if ((lpClassNameW && lstrcmpW(lpClassNameW, L"MsiHiddenWindow") == 0)
		)
	{
		return true;
	}

	#ifdef _DEBUG
	// � ������� ��� ����������� ���������, ������� �������� ���� � �������
	// ���� �������� � "���������" ���������� - ��������, ����� ���������,
	// ������������ DDE ����� �������.
	wchar_t szModule[MAX_PATH] = {}; GetModuleFileName(ghOurModule, szModule, countof(szModule));
	//const wchar_t* pszSlash = PointToName(szModule);
	//if (lstrcmpi(pszSlash, L"far.exe")==0 || lstrcmpi(szModule, L"far64.exe")==0)
	if (IsFarExe(szModule))
	{
		_ASSERTE(dwStyle == 0 && FALSE);
	}
	//SetLastError(ERROR_THREAD_MODE_NOT_BACKGROUND);
	//return false;
	#endif
	
	// ���������? �� ���������?
	return true;
#endif
}

void ReplaceGuiAppWindow(BOOL abStyleHidden)
{
	if (!ghAttachGuiClient)
	{
		_ASSERTEX(ghAttachGuiClient!=NULL);
		return;
	}

	DWORD_PTR dwStyle = user->getWindowLongPtrW(ghAttachGuiClient, GWL_STYLE);
	DWORD_PTR dwStyleEx = user->getWindowLongPtrW(ghAttachGuiClient, GWL_EXSTYLE);

	if (!gbAttachGuiClientStyleOk)
	{
		gbAttachGuiClientStyleOk = TRUE;
		gnAttachGuiClientStyle = (DWORD)dwStyle;
		gnAttachGuiClientStyleEx = (DWORD)dwStyleEx;
	}

	if (grcAttachGuiClientOrig.left==grcAttachGuiClientOrig.right && grcAttachGuiClientOrig.bottom==grcAttachGuiClientOrig.top)
	{
		user->getWindowRect(ghAttachGuiClient, &grcAttachGuiClientOrig);
	}
	
	if (!gbGuiClientExternMode)
	{
		// DotNet: ���� �� �������� WS_CHILD - �� �������� toolStrip & menuStrip
		// Native: ���� �������� WS_CHILD - �������� ������� ����
		DWORD_PTR dwNewStyle = dwStyle & ~(WS_MAXIMIZEBOX|WS_MINIMIZEBOX);
		if (gnAttachGuiClientFlags & agaf_WS_CHILD)
			dwNewStyle = (dwNewStyle|WS_CHILD/*|DS_CONTROL*/) & ~(WS_POPUP);

		if (dwStyle != dwNewStyle)
			user->setWindowLongPtrW(ghAttachGuiClient, GWL_STYLE, dwNewStyle);

		/*
		
		DWORD_PTR dwNewStyleEx = (dwStyleEx|WS_EX_CONTROLPARENT);
		if (dwStyleEx != dwNewStyleEx)
			user->setWindowLongPtrW(ghAttachGuiClient, GWL_EXSTYLE, dwNewStyleEx);
		*/

		HWND hCurParent = user->getParent(ghAttachGuiClient);
		_ASSERTEX(ghConEmuWndBack && user->isWindow(ghConEmuWndBack));
		if (hCurParent != ghConEmuWndBack)
		{
			user->setParent(ghAttachGuiClient, ghConEmuWndBack);
		}

		RECT rcGui = AttachGuiClientPos();
		
		if (user->setWindowPos(ghAttachGuiClient, HWND_TOP, rcGui.left,rcGui.top, rcGui.right-rcGui.left, rcGui.bottom-rcGui.top,
			SWP_DRAWFRAME | SWP_NOCOPYBITS | /*SWP_FRAMECHANGED |*/ (abStyleHidden ? SWP_SHOWWINDOW : 0)))
		{
			if (abStyleHidden && IsWindowVisible(ghAttachGuiClient))
				abStyleHidden = FALSE;
		}
		
		// !!! OnSetForegroundWindow �� �������� - �� ������� Cmd.
		user->setForegroundWindow(ghConEmuWnd);

		user->postMessageW(ghAttachGuiClient, WM_NCPAINT, 0, 0);
	}
}

UINT gnAttachMsgId = 0;
HHOOK ghAttachMsgHook = NULL;
struct AttachMsgArg
{
	UINT MsgId;
	UINT Result;
	HWND hConEmu;
	HWND hProcessed;
};

LRESULT CALLBACK AttachGuiWindowCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION)
	{
		CWPSTRUCT* p = (CWPSTRUCT*)lParam;
		if (p->message == gnAttachMsgId)
		{
			_ASSERTEX(wParam==1 && p->wParam==gnAttachMsgId);
			AttachMsgArg* args = (AttachMsgArg*)p->lParam;
			_ASSERTEX(args->MsgId==gnAttachMsgId && args->hConEmu==ghConEmuWnd && args->hProcessed==p->hwnd);

			HWND hOurWindow = p->hwnd;
			HMENU hMenu = user->getMenu(hOurWindow);
			wchar_t szClassName[255]; user->getClassNameW(hOurWindow, szClassName, countof(szClassName));
			DWORD nCurStyle = (DWORD)user->getWindowLongPtrW(hOurWindow, GWL_STYLE);
			DWORD nCurStyleEx = (DWORD)user->getWindowLongPtrW(hOurWindow, GWL_EXSTYLE);
			//BOOL bWasVisible = IsWindowVisible(hOurWindow);
			_ASSERTEX(IsWindowVisible(hOurWindow));
			OnGuiWindowAttached(hOurWindow, hMenu, NULL, szClassName, nCurStyle, nCurStyleEx, FALSE, SW_SHOW);
			args->Result = gnAttachMsgId;
			//if (!bWasVisible) // ����?
			//	ShowWindow(hOurWindow, SW_SHOW);
		}
	}

	return user->callNextHookEx(ghAttachMsgHook, nCode, wParam, lParam);
}

void AttachGuiWindow(HWND hOurWindow)
{
	_ASSERTEX(gbAttachGuiClient); // ��� ������ ��� ���� ����������?
	_ASSERTEX(user);
	gnAttachMsgId = user->registerWindowMessageW(L"ConEmu:Attach2Gui");
	if (gnAttachMsgId)
	{
		DWORD nWndTID = user->getWindowThreadProcessId(hOurWindow, NULL);
		ghAttachMsgHook = user->setWindowsHookExW(WH_CALLWNDPROC, AttachGuiWindowCallback, NULL, nWndTID);

		// ��������� ����� ������ �� ��������� � ��� ����, � ������� �������� ���� - �� ����� ���
		AttachMsgArg args = {gnAttachMsgId, 0, ghConEmuWnd, hOurWindow};
		LRESULT lRc = user->sendMessageW(hOurWindow, gnAttachMsgId, gnAttachMsgId, (LPARAM)&args);
		_ASSERTEX(args.Result == gnAttachMsgId);
		UNREFERENCED_PARAMETER(lRc);

		user->unhookWindowsHookEx(ghAttachMsgHook);
		ghAttachMsgHook = NULL;
	}
}

bool IsDotNetWindow(HWND hWindow)
{
	// � ����������� .Net - ���������� �������� ��� � WS_CHILD,
	// ����� � ��� "�� ����������" ������� � ����

	//Issue 624: mscoree ����� ���� �������� � �� � .Net ����������
	//if (GetModuleHandle(L"mscoree.dll") != NULL) -- �����������

	wchar_t szClass[255];
	const wchar_t szWindowsForms[] = L"WindowsForms";
	int nNeedLen = lstrlen(szWindowsForms);

	// WindowsForms10.Window.8.app.0.378734a
	int nLen = user->getClassNameW(hWindow, szClass, countof(szClass));
	if (nLen > nNeedLen)
	{
		szClass[nNeedLen] = 0;
		if (lstrcmpi(szClass, szWindowsForms) == 0)
		{
			// �� � ������ ��������, ��� ��� ".Net"
			HMODULE hCore = GetModuleHandle(L"mscoree.dll");
			if (hCore != NULL)
			{
				return true;
			}
		}
	}

	return false;
}

// ���� (anFromShowWindow != -1), ������ ����� ����� �� ShowWindow
void OnGuiWindowAttached(HWND hWindow, HMENU hMenu, LPCSTR asClassA, LPCWSTR asClassW, DWORD anStyle, DWORD anStyleEx, BOOL abStyleHidden, int anFromShowWindow/*=-1*/)
{
	DWORD nCurStyle = (DWORD)user->getWindowLongPtrW(hWindow, GWL_STYLE);
	DWORD nCurStyleEx = (DWORD)user->getWindowLongPtrW(hWindow, GWL_EXSTYLE);

	user->allowSetForegroundWindow(ASFW_ANY);

	// VLC ������� ��������� "����������" ����, �� ShowWindow �����
	// ������ ��� ������ �� ���. ������� ����������� ����� ������
	// ������ � ��� ������, ���� ���� "�������"
	if ((!(nCurStyle & WS_VISIBLE)) && (anFromShowWindow <= SW_HIDE))
	{
		// ������ �����, �� ShowWindow
		return;
	}

	ghAttachGuiClient = hWindow;
	gnAttachGuiClientThreadId = user->getWindowThreadProcessId(hWindow, NULL);
	gbForceShowGuiClient = TRUE;
	gbAttachGuiClient = FALSE; // ������ ���� ���� ����������. ����?

#if 0
	// ��� WS_CHILDWINDOW ���� ������ ������� ��� �������� ����
	if (!hMenu && !ghAttachGuiClientMenu && (asClassA || asClassW))
	{
		BOOL lbRcClass;
		WNDCLASSEXA wca = {sizeof(WNDCLASSEXA)};
		WNDCLASSEXW wcw = {sizeof(WNDCLASSEXW)};
		if (asClassA)
		{
			lbRcClass = GetClassInfoExA(GetModuleHandle(NULL), asClassA, &wca);
			if (lbRcClass)
				ghAttachGuiClientMenu = LoadMenuA(wca.hInstance, wca.lpszMenuName);
		}
		else
		{
			lbRcClass = GetClassInfoExW(GetModuleHandle(NULL), asClassW, &wcw);
			if (lbRcClass)
				ghAttachGuiClientMenu = LoadMenuW(wca.hInstance, wcw.lpszMenuName);
		}
		hMenu = ghAttachGuiClientMenu;
	}
	if (hMenu)
	{
		// ��� WS_CHILDWINDOW - �� ��������
		SetMenu(hWindow, hMenu);
		HMENU hSys = GetSystemMenu(hWindow, FALSE);
		TODO("��� � �������� �����������, �� ����� ������������� WM_SYSCOMMAND -> WM_COMMAND, ��������������, ������������� WndProc, ��� ��� �������");
		if (hSys)
		{
			TODO("����, ������ �� �� ��� � Popup ���������, � ������� ChildPopups �� hMenu");
			InsertMenu(hSys, 0, MF_BYPOSITION|MF_POPUP, (UINT_PTR)hMenu, L"Window menu");
			InsertMenu(hSys, 1, MF_BYPOSITION|MF_SEPARATOR, NULL, NULL);
		}
	}
#endif

	DWORD nSize = sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_ATTACHGUIAPP);
	CESERVER_REQ *pIn = ExecuteNewCmd(CECMD_ATTACHGUIAPP, nSize);

	gnAttachGuiClientFlags = agaf_Success;
	// � ����������� .Net - ���������� �������� ��� � WS_CHILD,
	// ����� � ��� "�� ����������" ������� � ����
	if (IsDotNetWindow(hWindow))
	{
		gnAttachGuiClientFlags |= (agaf_DotNet|agaf_WS_CHILD);
	}
	// ���� � ���� ��� ���� - �������� � ��� ��� � WS_CHILD
	// ��� �� ��������� ������� � ���������� � �.�.
	else if (user->getMenu(hWindow) == NULL)
	{
		gnAttachGuiClientFlags |= (agaf_NoMenu|agaf_WS_CHILD);
	}
	pIn->AttachGuiApp.nFlags = gnAttachGuiClientFlags;
	pIn->AttachGuiApp.nPID = GetCurrentProcessId();
	pIn->AttachGuiApp.hAppWindow = hWindow;
	pIn->AttachGuiApp.Styles.nStyle = nCurStyle; // ����� ����� ���������� ����� �������� ����,
	pIn->AttachGuiApp.Styles.nStyleEx = nCurStyleEx; // ������� ������� ����������
	user->getWindowRect(hWindow, &pIn->AttachGuiApp.rcWindow);
	GetModuleFileName(NULL, pIn->AttachGuiApp.sAppFileName, countof(pIn->AttachGuiApp.sAppFileName));
	pIn->AttachGuiApp.hkl = (DWORD)(LONG)(LONG_PTR)GetKeyboardLayout(0);

	wchar_t szGuiPipeName[128];
	msprintf(szGuiPipeName, countof(szGuiPipeName), CEGUIPIPENAME, L".", (DWORD)ghConEmuWnd);


	// AttachThreadInput
	DWORD nConEmuTID = user->getWindowThreadProcessId(ghConEmuWnd, NULL);
	DWORD nTID = GetCurrentThreadId();
	_ASSERTEX(nTID==gnHookMainThreadId || nTID==gnAttachGuiClientThreadId);
	BOOL bAttachRc = user->attachThreadInput(nTID, nConEmuTID, TRUE);
	DWORD nAttachErr = GetLastError();
	UNREFERENCED_PARAMETER(bAttachRc); UNREFERENCED_PARAMETER(nAttachErr);


	CESERVER_REQ* pOut = ExecuteCmd(szGuiPipeName, pIn, 0/*Default timeout*/, NULL);

	ExecuteFreeResult(pIn);

	// abStyleHidden == TRUE, ���� ���� ��� �������� ������� ���� WS_VISIBLE (�.�. �� ���������� ����� ShowWindow)

	if (pOut)
	{
		if (pOut->hdr.cbSize > sizeof(CESERVER_REQ_HDR))
		{
			_ASSERTE((pOut->AttachGuiApp.nFlags & agaf_Success) == agaf_Success);

            BOOL lbRc = FALSE;

			_ASSERTE(pOut->AttachGuiApp.hConEmuBack && pOut->AttachGuiApp.hConEmuDc && (HWND)pOut->AttachGuiApp.hConEmuDc!=(HWND)pOut->AttachGuiApp.hConEmuBack);
			_ASSERTE((ghConEmuWndBack==NULL) || (pOut->AttachGuiApp.hConEmuBack==ghConEmuWndBack));
			_ASSERTE(ghConEmuWnd && (ghConEmuWnd==pOut->AttachGuiApp.hConEmuWnd));
			ghConEmuWnd = pOut->AttachGuiApp.hConEmuWnd;
			SetConEmuHkWindows(pOut->AttachGuiApp.hConEmuDc, pOut->AttachGuiApp.hConEmuBack);

			//gbGuiClientHideCaption = pOut->AttachGuiApp.bHideCaption;
			gGuiClientStyles = pOut->AttachGuiApp.Styles;
            
			#ifdef _DEBUG
            HWND hFocus = user->getFocus();
            DWORD nFocusPID = 0;
            
            if (hFocus)
            {
                user->getWindowThreadProcessId(hFocus, &nFocusPID);
				DWORD nConEmuPID = 0; user->getWindowThreadProcessId(ghConEmuWnd, &nConEmuPID);
                if (nFocusPID != GetCurrentProcessId() && nFocusPID != nConEmuPID)
                {                                                    
                    _ASSERTE(hFocus==NULL || (nFocusPID==GetCurrentProcessId() || nFocusPID == nConEmuPID));
                    hFocus = NULL;
                }
            }
			#endif
            
			if (pOut->AttachGuiApp.hkl)
			{
				LONG_PTR hkl = (LONG_PTR)(LONG)pOut->AttachGuiApp.hkl;
				lbRc = ActivateKeyboardLayout((HKL)hkl, KLF_SETFORPROCESS) != NULL;
				UNREFERENCED_PARAMETER(lbRc);
			}

            //grcAttachGuiClientPos = pOut->AttachGuiApp.rcWindow;
            ReplaceGuiAppWindow(abStyleHidden);

			//// !!! OnSetForegroundWindow �� �������� - �� ������� Cmd.
			////user->setForegroundWindow(ghConEmuWnd);
			//#if 0
			//wchar_t szClass[64] = {}; user->getClassNameW(hFocus, szClass, countof(szClass));
			//MessageBox(NULL, szClass, L"WasFocused", MB_SYSTEMMODAL);
			//#endif
			////if (!(nCurStyle & WS_CHILDWINDOW))
			//{
			//	// ���� ������� WS_CHILD - �������� ����!
			//	//nCurStyle = (nCurStyle | WS_CHILDWINDOW|WS_TABSTOP); // & ~(WS_THICKFRAME/*|WS_CAPTION|WS_MINIMIZEBOX|WS_MAXIMIZEBOX*/);
			//	//user->setWindowLongPtrW(hWindow, GWL_STYLE, nCurStyle);
			//	if (gnAttachGuiClientFlags & agaf_DotNet)
			//	{
			//	}
			//	else
			//	{
			//		SetParent(hWindow, ghConEmuWndBack);
			//	}
			//}
			//
			//RECT rcGui = grcAttachGuiClientPos = pOut->AttachGuiApp.rcWindow;
			//if (user->setWindowPos(hWindow, HWND_TOP, rcGui.left,rcGui.top, rcGui.right-rcGui.left, rcGui.bottom-rcGui.top,
			//	SWP_DRAWFRAME | /*SWP_FRAMECHANGED |*/ (abStyleHidden ? SWP_SHOWWINDOW : 0)))
			//{
			//	if (abStyleHidden)
			//		abStyleHidden = FALSE;
			//}
			//
			//// !!! OnSetForegroundWindow �� �������� - �� ������� Cmd.
			//user->setForegroundWindow(ghConEmuWnd);
			////if (hFocus)
			////SetFocus(hFocus ? hFocus : hWindow); // hFocus==NULL, ������� ���
			////OnSetForegroundWindow(hWindow);
			////user->postMessage(ghConEmuWnd, WM_NCACTIVATE, TRUE, 0);
			////user->postMessage(ghConEmuWnd, WM_NCPAINT, 0, 0);
			//user->postMessage(hWindow, WM_NCPAINT, 0, 0);
		}
		ExecuteFreeResult(pOut);
	}

	if (abStyleHidden)
	{
		user->showWindow(hWindow, SW_SHOW);
	}
}

void OnShowGuiClientWindow(HWND hWnd, int &nCmdShow, BOOL &rbGuiAttach)
{
#ifdef _DEBUG
	STARTUPINFO si = {sizeof(si)};
	GetStartupInfo(&si);
#endif

	if ((!ghAttachGuiClient) && gbAttachGuiClient && (nCmdShow >= SW_SHOWNORMAL))
	{
		// VLC ������� ��������� "����������" ����, �� ShowWindow �����
		// ������ ��� ������ �� ���. ������� ����������� ����� ������
		// ������ � ��� ������, ���� ���� "�������"
		HMENU hMenu = user->getMenu(hWnd);
		wchar_t szClassName[255]; user->getClassNameW(hWnd, szClassName, countof(szClassName));
		DWORD nCurStyle = (DWORD)user->getWindowLongPtrW(hWnd, GWL_STYLE);
		DWORD nCurStyleEx = (DWORD)user->getWindowLongPtrW(hWnd, GWL_EXSTYLE);
		BOOL bAttachGui = TRUE;

		if (ghAttachGuiClient == NULL)
		{
			HWND hWndParent = ::GetParent(hWnd);
			DWORD dwStyle = nCurStyle, dwExStyle = nCurStyleEx;
			BOOL bStyleHidden;

			CheckCanCreateWindow(NULL, szClassName, dwStyle, dwExStyle, hWndParent, bAttachGui, bStyleHidden);
		}

		// �������
		if (bAttachGui)
		{
			OnGuiWindowAttached(hWnd, hMenu, NULL, szClassName, nCurStyle, nCurStyleEx, FALSE, nCmdShow);
		}
	}

	if (gbForceShowGuiClient && (ghAttachGuiClient == hWnd))
	{
		//if (nCmdShow == SW_HIDE)
		//	nCmdShow = SW_SHOWNORMAL;

		HWND hCurParent = user->getParent(hWnd);
		if (hCurParent != ghConEmuWndBack)
		{
			DWORD nCurStyle = (DWORD)user->getWindowLongPtrW(hWnd, GWL_STYLE);
			DWORD nCurStyleEx = (DWORD)user->getWindowLongPtrW(hWnd, GWL_EXSTYLE);

			DWORD nSize = sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_ATTACHGUIAPP);
			CESERVER_REQ *pIn = ExecuteNewCmd(CECMD_ATTACHGUIAPP, nSize);

			pIn->AttachGuiApp.nFlags = gnAttachGuiClientFlags;
			pIn->AttachGuiApp.nPID = GetCurrentProcessId();
			pIn->AttachGuiApp.hAppWindow = hWnd;
			pIn->AttachGuiApp.Styles.nStyle = nCurStyle; // ����� ����� ���������� ����� �������� ����,
			pIn->AttachGuiApp.Styles.nStyleEx = nCurStyleEx; // ������� ������� ����������
			user->getWindowRect(hWnd, &pIn->AttachGuiApp.rcWindow);
			GetModuleFileName(NULL, pIn->AttachGuiApp.sAppFileName, countof(pIn->AttachGuiApp.sAppFileName));

			wchar_t szGuiPipeName[128];
			msprintf(szGuiPipeName, countof(szGuiPipeName), CEGUIPIPENAME, L".", (DWORD)ghConEmuWnd);

			CESERVER_REQ* pOut = ExecuteCmd(szGuiPipeName, pIn, 0/*Default timeout*/, NULL);
			ExecuteFreeResult(pIn);
			if (pOut)
			{
				if (pOut->hdr.cbSize > sizeof(CESERVER_REQ_HDR) && (pOut->AttachGuiApp.nFlags & agaf_Success))
				{
					//grcAttachGuiClientPos = pOut->AttachGuiApp.rcWindow;

					_ASSERTE((ghConEmuWndBack==NULL) || (pOut->AttachGuiApp.hConEmuBack==ghConEmuWndBack));
					_ASSERTE(ghConEmuWnd && (ghConEmuWnd==pOut->AttachGuiApp.hConEmuWnd));
					ghConEmuWnd = pOut->AttachGuiApp.hConEmuWnd;
					SetConEmuHkWindows(pOut->AttachGuiApp.hConEmuDc, pOut->AttachGuiApp.hConEmuBack);
					//gbGuiClientHideCaption = pOut->AttachGuiApp.bHideCaption;
					gGuiClientStyles = pOut->AttachGuiApp.Styles;
				}
				ExecuteFreeResult(pOut);
			}

			//OnSetParent(hWnd, ghConEmuWndBack);
		}

        ReplaceGuiAppWindow(FALSE);
		
		//RECT rcGui = grcAttachGuiClientPos;
		//user->setWindowPos(hWnd, HWND_TOP, rcGui.left,rcGui.top, rcGui.right-rcGui.left, rcGui.bottom-rcGui.top,
		//	SWP_FRAMECHANGED);
	
		nCmdShow = SW_SHOWNORMAL;
		gbForceShowGuiClient = FALSE; // ���� ���?
		rbGuiAttach = TRUE;
	}
}

void OnPostShowGuiClientWindow(HWND hWnd, int nCmdShow)
{
	RECT rcGui = {0,0,grcAttachGuiClientOrig.right-grcAttachGuiClientOrig.left,grcAttachGuiClientOrig.bottom-grcAttachGuiClientOrig.top};
	user->getClientRect(hWnd, &rcGui);
	user->sendMessageW(hWnd, WM_SIZE, SIZE_RESTORED, MAKELONG((rcGui.right-rcGui.left),(rcGui.bottom-rcGui.top)));
}

void CorrectGuiChildRect(DWORD anStyle, DWORD anStyleEx, RECT& rcGui)
{
	RECT rcShift = {};

	if ((anStyle != gGuiClientStyles.nStyle) || (anStyleEx != gGuiClientStyles.nStyleEx))
	{
		DWORD nSize = sizeof(CESERVER_REQ_HDR)+sizeof(GuiStylesAndShifts);
		CESERVER_REQ *pIn = ExecuteNewCmd(CECMD_GUICLIENTSHIFT, nSize);
		if (pIn)
		{
			pIn->GuiAppShifts.nStyle = anStyle;
			pIn->GuiAppShifts.nStyleEx = anStyleEx;

			wchar_t szGuiPipeName[128];
			msprintf(szGuiPipeName, countof(szGuiPipeName), CEGUIPIPENAME, L".", (DWORD)ghConEmuWnd);

			CESERVER_REQ* pOut = ExecuteCmd(szGuiPipeName, pIn, 10000, NULL);
			if (pOut)
			{
				rcShift = pOut->GuiAppShifts.Shifts;
				gGuiClientStyles = pOut->GuiAppShifts;
				ExecuteFreeResult(pOut);
			}
			ExecuteFreeResult(pIn);
		}
	}

	rcGui.left += rcShift.left; rcGui.right -= rcShift.right; rcGui.top += rcShift.top; rcGui.bottom -= rcShift.bottom;

#if 0
	//WARNING!! Same as "CRealConsole::CorrectGuiChildRect"
	int nX = 0, nY = 0, nY0 = 0;
	if (anStyle & WS_THICKFRAME)
	{
		nX = user->getSystemMetrics(SM_CXSIZEFRAME);
		nY = user->getSystemMetrics(SM_CXSIZEFRAME);
	}
	else if (anStyleEx & WS_EX_WINDOWEDGE)
	{
		nX = user->getSystemMetrics(SM_CXFIXEDFRAME);
		nY = user->getSystemMetrics(SM_CXFIXEDFRAME);
	}
	else if (anStyle & WS_DLGFRAME)
	{
		nX = user->getSystemMetrics(SM_CXEDGE);
		nY = user->getSystemMetrics(SM_CYEDGE);
	}
	else if (anStyle & WS_BORDER)
	{
		nX = user->getSystemMetrics(SM_CXBORDER);
		nY = user->getSystemMetrics(SM_CYBORDER);
	}
	else
	{
		nX = user->getSystemMetrics(SM_CXFIXEDFRAME);
		nY = user->getSystemMetrics(SM_CXFIXEDFRAME);
	}
	if ((anStyle & WS_CAPTION) && gbGuiClientHideCaption)
	{
		if (anStyleEx & WS_EX_TOOLWINDOW)
			nY0 += user->getSystemMetrics(SM_CYSMCAPTION);
		else
			nY0 += user->getSystemMetrics(SM_CYCAPTION);
	}
	rcGui.left -= nX; rcGui.right += nX; rcGui.top -= nY+nY0; rcGui.bottom += nY;
#endif
}

RECT AttachGuiClientPos(DWORD anStyle /*= 0*/, DWORD anStyleEx /*= 0*/)
{
	RECT rcPos = {0,0,grcAttachGuiClientOrig.right-grcAttachGuiClientOrig.left,grcAttachGuiClientOrig.bottom-grcAttachGuiClientOrig.top};
	if (ghConEmuWndBack)
	{
		user->getClientRect(ghConEmuWndBack, &rcPos);
		_ASSERTEX(ghAttachGuiClient!=NULL);
		DWORD nStyle = (anStyle || anStyleEx) ? anStyle : (DWORD)user->getWindowLongPtrW(ghAttachGuiClient, GWL_STYLE);
		DWORD nStyleEx = (anStyle || anStyleEx) ? anStyleEx : (DWORD)user->getWindowLongPtrW(ghAttachGuiClient, GWL_EXSTYLE);
		CorrectGuiChildRect(nStyle, nStyleEx, rcPos);
	}
	return rcPos;
}

bool OnSetGuiClientWindowPos(HWND hWnd, HWND hWndInsertAfter, int &X, int &Y, int &cx, int &cy, UINT uFlags)
{
	bool lbChanged = false;
	// GUI ������������ ��������� �������������� ��������� ������ ConEmu
	if (ghAttachGuiClient && hWnd == ghAttachGuiClient)
	{
		// -- ���-�� GetParent ���������� NULL (��� cifirica �� ������� ����)
		//if (user->getParent(hWnd) == ghConEmuWndBack)
		{
			RECT rcGui = AttachGuiClientPos();
			X = rcGui.left;
			Y = rcGui.top;
			cx = rcGui.right - X;
			cy = rcGui.bottom - Y;
			lbChanged = true;
		}
	}
	return lbChanged;
}

void SetGuiExternMode(BOOL abUseExternMode, LPRECT prcOldPos /*= NULL*/)
{
	if (gbGuiClientExternMode != abUseExternMode)
	{
		gbGuiClientExternMode = abUseExternMode;
		
		if (!abUseExternMode)
		{
			ReplaceGuiAppWindow(FALSE);
		}
		else
		{
			RECT rcGui;
			if (prcOldPos && (prcOldPos->right > prcOldPos->left && prcOldPos->bottom > prcOldPos->top))
				rcGui = *prcOldPos;
			else
				GetWindowRect(ghAttachGuiClient, &rcGui);

			if (gbAttachGuiClientStyleOk)
			{
				user->setWindowLongPtrW(ghAttachGuiClient, GWL_STYLE, gnAttachGuiClientStyle & ~WS_VISIBLE);
				user->setWindowLongPtrW(ghAttachGuiClient, GWL_EXSTYLE, gnAttachGuiClientStyleEx);
			}
			user->setParent(ghAttachGuiClient, NULL);
			
			TODO("������� ������ ������?");
			user->setWindowPos(ghAttachGuiClient, ghConEmuWnd, rcGui.left,rcGui.top, rcGui.right-rcGui.left, rcGui.bottom-rcGui.top,
				SWP_DRAWFRAME | SWP_SHOWWINDOW | SWP_ASYNCWINDOWPOS | SWP_NOACTIVATE);
		}
	}
}
