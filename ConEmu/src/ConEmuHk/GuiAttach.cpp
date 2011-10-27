
#include <windows.h>
#include "UserImp.h"
#include "GuiAttach.h"
#include "../common/common.hpp"
#include "../common/ConEmuCheck.h"

extern HWND    ghConEmuWnd;   // Root! ConEmu window
extern HWND    ghConEmuWndDC; // ConEmu DC window
extern DWORD   gnHookMainThreadId;

RECT    grcConEmuClient = {}; // ��� ������ ������ ����
BOOL    gbAttachGuiClient = FALSE; // ��� ������ ������ ����
BOOL gbGuiClientAttached; // ��� ������ ������ ���� (������� ������������)
HWND ghAttachGuiClient = NULL; // ����� ShowWindow �����������
DWORD gnAttachGuiClientFlags = 0;
BOOL gbForceShowGuiClient = FALSE; // --
//HMENU ghAttachGuiClientMenu = NULL;
RECT grcAttachGuiClientPos = {};
#ifdef _DEBUG
HHOOK ghGuiClientRetHook = NULL;
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
							if (user->getParent(p->hwnd) == ghConEmuWndDC)
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
			OutputDebugString(szDbg);
	}

	return user->callNextHookEx(ghGuiClientRetHook, nCode, wParam, lParam);
}
#endif


bool CheckCanCreateWindow(LPCSTR lpClassNameA, LPCWSTR lpClassNameW, DWORD& dwStyle, DWORD& dwExStyle, HWND& hWndParent, BOOL& bAttachGui, BOOL& bStyleHidden)
{
	bAttachGui = FALSE;
	
	// "!dwStyle" ������� ��� shell32.dll!CExecuteApplication::_CreateHiddenDDEWindow()
	_ASSERTE(hWndParent==NULL || ghConEmuWnd == NULL || hWndParent!=ghConEmuWnd || !dwStyle);
	
	if (gbAttachGuiClient && ghConEmuWndDC && (GetCurrentThreadId() == gnHookMainThreadId))
	{
		bool lbCanAttach = ((dwStyle & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW)
						|| ((dwStyle & (WS_POPUP|WS_THICKFRAME)) == (WS_POPUP|WS_THICKFRAME));
		if (dwStyle & (DS_MODALFRAME|WS_CHILDWINDOW))
			lbCanAttach = false;
		else if ((dwStyle & WS_POPUP) && !(dwStyle & WS_THICKFRAME))
			lbCanAttach = false;
		else if (dwExStyle & (WS_EX_TOOLWINDOW|WS_EX_TOPMOST|WS_EX_DLGMODALFRAME|WS_EX_MDICHILD))
			lbCanAttach = false;

		if (lbCanAttach)
		{
			// ������������ ���� - ConEmu DC
			// -- hWndParent = ghConEmuWndDC; // ���� �� ��� ������� �����, ���� �� �������� WS_CHILD?

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
		}
		return true;
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
		|| (lpClassNameA && lstrcmpA(lpClassNameA, "CicMarshalWndClass") == 0))
	{
		return true;
	}

	#ifdef _DEBUG
	// � ������� ��� ����������� ���������, ������� �������� ���� � �������
	// ���� �������� � "���������" ���������� - ��������, ����� ���������,
	// ������������ DDE ����� �������.
	_ASSERTE(dwStyle == 0 && FALSE);
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

	RECT rcGui = grcAttachGuiClientPos;

	// DotNet: ���� �� �������� WS_CHILD - �� �������� toolStrip & menuStrip
	// Native: ���� �������� WS_CHILD - �������� ������� ����
	if (gnAttachGuiClientFlags & agaf_DotNet)
	{
		DWORD_PTR dwStyle = user->getWindowLongPtrW(ghAttachGuiClient, GWL_STYLE);
		if (!(dwStyle & WS_CHILD))
			user->setWindowLongPtrW(ghAttachGuiClient, GWL_STYLE, dwStyle|WS_CHILD);
	}

	HWND hCurParent = user->getParent(ghAttachGuiClient);
	if (hCurParent != ghConEmuWndDC)
	{
		user->setParent(ghAttachGuiClient, ghConEmuWndDC);
	}
	
	if (user->setWindowPos(ghAttachGuiClient, HWND_TOP, rcGui.left,rcGui.top, rcGui.right-rcGui.left, rcGui.bottom-rcGui.top,
		SWP_DRAWFRAME | /*SWP_FRAMECHANGED |*/ (abStyleHidden ? SWP_SHOWWINDOW : 0)))
	{
		if (abStyleHidden && IsWindowVisible(ghAttachGuiClient))
			abStyleHidden = FALSE;
	}
	
	// !!! OnSetForegroundWindow �� �������� - �� ������� Cmd.
	user->setForegroundWindow(ghConEmuWnd);

	user->postMessageW(ghAttachGuiClient, WM_NCPAINT, 0, 0);
}

void OnGuiWindowAttached(HWND hWindow, HMENU hMenu, LPCSTR asClassA, LPCWSTR asClassW, DWORD anStyle, DWORD anStyleEx, BOOL abStyleHidden)
{
	ghAttachGuiClient = hWindow;
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

	user->allowSetForegroundWindow(ASFW_ANY);

	DWORD nCurStyle = (DWORD)user->getWindowLongPtrW(hWindow, GWL_STYLE);
	DWORD nCurStyleEx = (DWORD)user->getWindowLongPtrW(hWindow, GWL_EXSTYLE);

	DWORD nSize = sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_ATTACHGUIAPP);
	CESERVER_REQ *pIn = ExecuteNewCmd(CECMD_ATTACHGUIAPP, nSize);

	gnAttachGuiClientFlags = agaf_Success;
	// � ����������� .Net - ���������� �������� ��� � WS_CHILD,
	// ����� � ��� "�� ����������" ������� � ����
	if (GetModuleHandle(L"mscoree.dll") != NULL)
		gnAttachGuiClientFlags |= agaf_DotNet;
	// ���� � ���� ��� ���� - �������� � ��� ��� � WS_CHILD
	// ��� �� ��������� ������� � ���������� � �.�.
	//if (user->getMenu(hWindow) == NULL)
	//	gnAttachGuiClientFlags |= agaf_DotNet;
	pIn->AttachGuiApp.nFlags = gnAttachGuiClientFlags;
	pIn->AttachGuiApp.nPID = GetCurrentProcessId();
	pIn->AttachGuiApp.hWindow = hWindow;
	pIn->AttachGuiApp.nStyle = nCurStyle; // ����� ����� ���������� ����� �������� ����,
	pIn->AttachGuiApp.nStyleEx = nCurStyleEx; // ������� ������� ����������
	user->getWindowRect(hWindow, &pIn->AttachGuiApp.rcWindow);
	GetModuleFileName(NULL, pIn->AttachGuiApp.sAppFileName, countof(pIn->AttachGuiApp.sAppFileName));

	wchar_t szGuiPipeName[128];
	msprintf(szGuiPipeName, countof(szGuiPipeName), CEGUIPIPENAME, L".", (DWORD)ghConEmuWnd);

	CESERVER_REQ* pOut = ExecuteCmd(szGuiPipeName, pIn, 1000, NULL);

	ExecuteFreeResult(pIn);

	// abStyleHidden == TRUE, ���� ���� ��� �������� ������� ���� WS_VISIBLE (�.�. �� ���������� ����� ShowWindow)

	if (pOut)
	{
		if (pOut->hdr.cbSize > sizeof(CESERVER_REQ_HDR))
		{
			_ASSERTE((pOut->AttachGuiApp.nFlags & agaf_Success) == agaf_Success);

            HWND hFocus = user->getFocus();
            DWORD nFocusPID = 0;
            if (hFocus)
            {
                user->getWindowThreadProcessId(hFocus, &nFocusPID);
                if (nFocusPID != GetCurrentProcessId())
                {                                                    
                    _ASSERTE(hFocus==NULL || (nFocusPID==GetCurrentProcessId()));
                    hFocus = NULL;
                }
            }
            
            grcAttachGuiClientPos = pOut->AttachGuiApp.rcWindow;
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
			//		SetParent(hWindow, ghConEmuWndDC);
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
	if (gbForceShowGuiClient && (ghAttachGuiClient == hWnd))
	{
		//if (nCmdShow == SW_HIDE)
		//	nCmdShow = SW_SHOWNORMAL;

		HWND hCurParent = user->getParent(hWnd);
		if (hCurParent != ghConEmuWndDC)
		{
			DWORD nCurStyle = (DWORD)user->getWindowLongPtrW(hWnd, GWL_STYLE);
			DWORD nCurStyleEx = (DWORD)user->getWindowLongPtrW(hWnd, GWL_EXSTYLE);

			DWORD nSize = sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_ATTACHGUIAPP);
			CESERVER_REQ *pIn = ExecuteNewCmd(CECMD_ATTACHGUIAPP, nSize);

			pIn->AttachGuiApp.nFlags = gnAttachGuiClientFlags;
			pIn->AttachGuiApp.nPID = GetCurrentProcessId();
			pIn->AttachGuiApp.hWindow = hWnd;
			pIn->AttachGuiApp.nStyle = nCurStyle; // ����� ����� ���������� ����� �������� ����,
			pIn->AttachGuiApp.nStyleEx = nCurStyleEx; // ������� ������� ����������
			user->getWindowRect(hWnd, &pIn->AttachGuiApp.rcWindow);
			GetModuleFileName(NULL, pIn->AttachGuiApp.sAppFileName, countof(pIn->AttachGuiApp.sAppFileName));

			wchar_t szGuiPipeName[128];
			msprintf(szGuiPipeName, countof(szGuiPipeName), CEGUIPIPENAME, L".", (DWORD)ghConEmuWnd);

			CESERVER_REQ* pOut = ExecuteCmd(szGuiPipeName, pIn, 1000, NULL);
			ExecuteFreeResult(pIn);
			if (pOut)
			{
				if (pOut->hdr.cbSize > sizeof(CESERVER_REQ_HDR) && (pOut->AttachGuiApp.nFlags & agaf_Success))
					grcAttachGuiClientPos = pOut->AttachGuiApp.rcWindow;
				ExecuteFreeResult(pOut);
			}

			//OnSetParent(hWnd, ghConEmuWndDC);
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
	//SetFocus(ghConEmuWnd);
	RECT rcGui = grcAttachGuiClientPos;
	//user->setWindowPos(hWnd, HWND_TOP, rcGui.left,rcGui.top, rcGui.right-rcGui.left, rcGui.bottom-rcGui.top,
	//	SWP_FRAMECHANGED|SWP_SHOWWINDOW);
	user->getClientRect(hWnd, &rcGui);
	user->sendMessageW(hWnd, WM_SIZE, SIZE_RESTORED, MAKELONG((rcGui.right-rcGui.left),(rcGui.bottom-rcGui.top)));
}

bool OnSetGuiClientWindowPos(HWND hWnd, HWND hWndInsertAfter, int &X, int &Y, int &cx, int &cy, UINT uFlags)
{
	bool lbChanged = false;
	// GUI ������������ ��������� �������������� ��������� ������ ConEmu
	if (ghAttachGuiClient && hWnd == ghAttachGuiClient && grcAttachGuiClientPos.right)
	{
		// -- ���-�� GetParent ���������� NULL (��� cifirica �� ������� ����)
		//if (user->getParent(hWnd) == ghConEmuWndDC)
		{
			X = grcAttachGuiClientPos.left;
			Y = grcAttachGuiClientPos.top;
			cx = grcAttachGuiClientPos.right - X;
			cy = grcAttachGuiClientPos.bottom - Y;
			lbChanged = true;
		}
	}
	return lbChanged;
}
