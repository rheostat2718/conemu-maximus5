
/*
Copyright (c) 2009-2010 Maximus5
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


#include "Header.h"
#include "../common/common.hpp"
#include "../common/WinObjects.h"
#include "ConEmu.h"
#include "ConEmuChild.h"
#include "Options.h"
#include "TabBar.h"
#include "VirtualConsole.h"

#if defined(__GNUC__)
#define EXT_GNUC_LOG
#endif

#define DEBUGSTRDRAW(s) //DEBUGSTR(s)
#define DEBUGSTRTABS(s) //DEBUGSTR(s)
#define DEBUGSTRLANG(s) //DEBUGSTR(s)

CConEmuChild::CConEmuChild()
{
	mn_MsgTabChanged = RegisterWindowMessage(CONEMUTABCHANGED);
	mn_MsgPostFullPaint = RegisterWindowMessage(L"CConEmuChild::PostFullPaint");
	mb_PostFullPaint = FALSE;
	mn_LastPostRedrawTick = 0;
	mb_IsPendingRedraw = FALSE;
	mb_RedrawPosted = FALSE;
	memset(&Caret, 0, sizeof(Caret));
	mb_DisableRedraw = FALSE;
}

CConEmuChild::~CConEmuChild()
{
}

HWND CConEmuChild::Create()
{
	// ��� ������ - �� �� �����, ��� � � �������� ����
	DWORD style = /*WS_VISIBLE |*/ WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	//RECT rc = gConEmu.DCClientRect();
	RECT rcMain; GetClientRect(ghWnd, &rcMain);
	RECT rc = gConEmu.CalcRect(CER_DC, rcMain, CER_MAINCLIENT);
	ghWndDC = CreateWindow(szClassName, 0, style, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, ghWnd, NULL, (HINSTANCE)g_hInstance, NULL);
	if (!ghWndDC) {
		ghWndDC = (HWND)-1; // ����� �������� �� �������
		MBoxA(_T("Can't create DC window!"));
		return NULL; //
	}
	//SetClassLong(ghWndDC, GCL_HBRBACKGROUND, (LONG)gConEmu.m_Back->mh_BackBrush);
	SetWindowPos(ghWndDC, HWND_TOP, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);
	//gConEmu.dcWindowLast = rc; //TODO!!!
	
	return ghWndDC;
}

LRESULT CConEmuChild::ChildWndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

	if (messg == WM_SYSCHAR)
		return TRUE;

    switch (messg)
    {
	case WM_SETFOCUS:
		SetFocus(ghWnd); // ����� ������ ���� � ������� ����!
		return 0;

    case WM_ERASEBKGND:
		result = 0;
		break;
		
    case WM_PAINT:
		result = gConEmu.m_Child->OnPaint();
		break;

    case WM_SIZE:
		result = gConEmu.m_Child->OnSize(wParam, lParam);
        break;

    case WM_CREATE:
        break;

    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_MOUSEWHEEL:
    case WM_ACTIVATE:
    case WM_ACTIVATEAPP:
    //case WM_MOUSEACTIVATE:
    case WM_KILLFOCUS:
    //case WM_SETFOCUS:
    case WM_MOUSEMOVE:
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
    case WM_VSCROLL:
        // ��� ��������� � ��������
		{
			POINT pt = {LOWORD(lParam),HIWORD(lParam)};
			MapWindowPoints(hWnd, ghWnd, &pt, 1);
			lParam = MAKELONG(pt.x,pt.y);
			result = gConEmu.WndProc(hWnd, messg, wParam, lParam);
		}
        return result;

	case WM_IME_NOTIFY:
		break;
    case WM_INPUTLANGCHANGE:
    case WM_INPUTLANGCHANGEREQUEST:
	{
		#ifdef _DEBUG
		if (IsDebuggerPresent()) {
			WCHAR szMsg[128];
			wsprintf(szMsg, L"InChild %s(CP:%i, HKL:0x%08X)\n",
				(messg == WM_INPUTLANGCHANGE) ? L"WM_INPUTLANGCHANGE" : L"WM_INPUTLANGCHANGEREQUEST",
				wParam, lParam);
			DEBUGSTRLANG(szMsg);

		}
		#endif
		result = DefWindowProc(hWnd, messg, wParam, lParam);
	} break;

#ifdef _DEBUG
		case WM_WINDOWPOSCHANGING:
			{
			WINDOWPOS* pwp = (WINDOWPOS*)lParam;
			result = DefWindowProc(hWnd, messg, wParam, lParam);
			}
			return result;
		case WM_WINDOWPOSCHANGED:
			{
			WINDOWPOS* pwp = (WINDOWPOS*)lParam;
			result = DefWindowProc(hWnd, messg, wParam, lParam);
			}
			return result;
#endif

    default:
		// ��������� �������� �� ConEmuPlugin
		if (messg == gConEmu.m_Child->mn_MsgTabChanged) {
			if (gSet.isTabs) {
				//���������� ����, �� ����� ����������
				#ifdef MSGLOGGER
					WCHAR szDbg[128]; swprintf(szDbg, L"Tabs:Notified(%i)\n", wParam);
					DEBUGSTRTABS(szDbg);
				#endif
				TODO("����� ������ �� ������ OnTimer ������� �������� mn_TopProcessID")
				// ����� �� ����� ������� PID ���� ��� ����� ���� �� ��������...
				//gConEmu.OnTimer(0,0); �� ����������. ������ ������� �� �������, ��-�� ����� ������ ��������� ���� ��� � ������� 0
				WARNING("gConEmu.mp_TabBar->Retrieve() ������ ��� �� ������ ������");
				_ASSERT(FALSE);
				gConEmu.mp_TabBar->Retrieve();
			}
		} else if (messg == gConEmu.m_Child->mn_MsgPostFullPaint) {
			gConEmu.m_Child->Redraw();		
		} else if (messg) {
			result = DefWindowProc(hWnd, messg, wParam, lParam);
		}
    }
    return result;
}

LRESULT CConEmuChild::OnPaint()
{
	LRESULT result = 0;
	BOOL lbSkipDraw = FALSE;
    //if (gbInPaint)
	//    break;

	_ASSERT(FALSE);

	//2009-09-28 ����� ��� (autotabs)
	if (mb_DisableRedraw)
		return 0;
	
	mb_PostFullPaint = FALSE;

	if (gSet.isAdvLogging>1) 
		gConEmu.ActiveCon()->RCon()->LogString("CConEmuChild::OnPaint");

	gSet.Performance(tPerfBlt, FALSE);

    if (gConEmu.isPictureView())
    {
		// ���� PictureView ���������� �� �� ��� ���� - ���������� ������� ����� �������!
		RECT rcPic, rcClient, rcCommon;

		GetWindowRect(gConEmu.hPictureView, &rcPic);
		GetClientRect(ghWnd, &rcClient); // ��� ����� ������ ������ �� ��� ��������.
		MapWindowPoints(ghWnd, NULL, (LPPOINT)&rcClient, 2);
		#ifdef _DEBUG
		BOOL lbIntersect =
		#endif
		IntersectRect(&rcCommon, &rcClient, &rcPic);
		
		// ������ �� ��������� ������������� PictureView
		MapWindowPoints(NULL, ghWndDC, (LPPOINT)&rcPic, 2);
		ValidateRect(ghWndDC, &rcPic);

		GetClientRect(gConEmu.hPictureView, &rcPic);
		GetClientRect(ghWndDC, &rcClient);
		
		if (rcPic.right>=rcClient.right) {
			lbSkipDraw = TRUE;
	        result = DefWindowProc(ghWndDC, WM_PAINT, 0, 0);
        }
	}
	if (!lbSkipDraw)
	{
		//gConEmu.PaintCon();
	}

	Validate();

	gSet.Performance(tPerfBlt, TRUE);

	// ���� ������� ���� �������� - �������� ��������� ���������� � ��������
	gConEmu.UpdateSizes();

    return result;
}

LRESULT CConEmuChild::OnSize(WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	#ifdef _DEBUG
    BOOL lbIsPicView = FALSE;

	RECT rcNewClient; GetClientRect(ghWndDC,&rcNewClient);
	#endif

    // ����� ��� � �� �����. �� ��� Ansi �� ��� Unicode ������ �������
    // ��� ����� � ConEmu �������� ������ �� ����� ��������� ������ PictureView 
    
    //if (gConEmu.isPictureView())
    //{
    //    if (gConEmu.hPictureView) {
    //        lbIsPicView = TRUE;
    //        gConEmu.isPiewUpdate = true;
    //        RECT rcClient; GetClientRect(ghWndDC, &rcClient);
    //        //TODO: � ���� PictureView ����� � � QuickView ��������������...
    //        MoveWindow(gConEmu.hPictureView, 0,0,rcClient.right,rcClient.bottom, 1);
    //        //INVALIDATE(); //InvalidateRect(hWnd, NULL, FALSE);
	//		Invalidate();
    //        //SetFocus(hPictureView); -- ��� ����� �� ������ ������� ����� �������� ������...
    //    }
    //}

	return result;
}

void CConEmuChild::CheckPostRedraw()
{
	// ���� ��� "����������" Redraw, �� 
	if (mb_IsPendingRedraw && mn_LastPostRedrawTick && ((GetTickCount() - mn_LastPostRedrawTick) >= CON_REDRAW_TIMOUT)) {
		mb_IsPendingRedraw = FALSE;
		_ASSERTE(gConEmu.isMainThread());

		Redraw();
	}
}

void CConEmuChild::Redraw()
{
	if (mb_DisableRedraw) {
		DEBUGSTRDRAW(L" +++ RedrawWindow on DC window will be ignored!\n");
		return;
	}

	if (!gConEmu.isMainThread()) {
		if (mb_RedrawPosted ||
			(mn_LastPostRedrawTick && ((GetTickCount() - mn_LastPostRedrawTick) < CON_REDRAW_TIMOUT)))
		{
			mb_IsPendingRedraw = TRUE;
			return; // ��������� ������� ������ ����������
		} else {
			mn_LastPostRedrawTick = GetTickCount();
			mb_IsPendingRedraw = FALSE;
		}
		mb_RedrawPosted = TRUE; // ����� �� ���� ������������� �������
		PostMessage(ghWndDC, mn_MsgPostFullPaint, 0, 0);
		return;
	}

	DEBUGSTRDRAW(L" +++ RedrawWindow on DC window called\n");

	RECT rcClient; GetClientRect(ghWndDC, &rcClient);
	MapWindowPoints(ghWndDC, ghWnd, (LPPOINT)&rcClient, 2);
	InvalidateRect(ghWnd, &rcClient, FALSE);
	// ��-�� ����� - ��������� ������� �����������
	//gConEmu.OnPaint(0,0);

	//#ifdef _DEBUG
	//BOOL lbRc =
	//#endif
	//RedrawWindow(ghWnd, NULL, NULL,
	//	RDW_INTERNALPAINT|RDW_NOERASE|RDW_UPDATENOW);

	mb_RedrawPosted = FALSE; // ����� ������ ���� ����� ������� ��� ����
}

void CConEmuChild::SetRedraw(BOOL abRedrawEnabled)
{
	mb_DisableRedraw = !abRedrawEnabled;
}

void CConEmuChild::Invalidate()
{
	if (mb_DisableRedraw)
		return; // �����, ��� ��������� ���������� �����

	// ����� ����� invalidate'��� � GAPS !!!
	// ����� ��� ������� "conemu.exe /max" - gaps �������� ��������, ��� �������� ������������� ��������
	
	//2009-06-22 ����� ������� ������������. ������ ���� �� ������ ������ �������� - ����������� ����������
	// ��� ��� ���� ����� ��� ���� ��������...
	//if (mb_Invalidated) {
	//	DEBUGSTRDRAW(L" ### Warning! Invalidate on DC window will be duplicated\n");
	////	return;
	//}
	if (ghWndDC) {
		DEBUGSTRDRAW(L" +++ Invalidate on DC window called\n");

		//RECT rcClient; GetClientRect(ghWndDC, &rcClient);
		//MapWindowPoints(ghWndDC, ghWnd, (LPPOINT)&rcClient, 2);
		//InvalidateRect(ghWnd, &rcClient, FALSE);

		RECT rcMainClient; GetClientRect(ghWnd, &rcMainClient);
		RECT rcClient = gConEmu.CalcRect(CER_BACK, rcMainClient, CER_MAINCLIENT);
		InvalidateRect(ghWnd, &rcClient, FALSE);
	}
}

void CConEmuChild::Validate()
{
	//mb_Invalidated = FALSE;
	//DEBUGSTRDRAW(L" +++ Validate on DC window called\n");
	//if (ghWndDC) ValidateRect(ghWnd, NULL);
}








WARNING("!!! �� ����� �������������� ���������� ���������� AutoScroll � TRUE, � ��� ���������� �������� - ������� ������ ��������!");
TODO("� ������, ��������� ����� ���������� ����� pipe");

//#define SCROLLHIDE_TIMER_ID 1726
#define TIMER_SCROLL_SHOW         3201
#define TIMER_SCROLL_SHOW_DELAY   1000
#define TIMER_SCROLL_SHOW_DELAY2  500
#define TIMER_SCROLL_HIDE         3202
#define TIMER_SCROLL_HIDE_DELAY   1000
#define TIMER_SCROLL_CHECK        3203
#define TIMER_SCROLL_CHECK_DELAY  250
#define TIMER_SCROLL_CHECK_DELAY2 1000

CConEmuBack::CConEmuBack()
{
	mh_WndBack = NULL;
	mh_WndScroll = NULL;
	mh_BackBrush = NULL;
	mn_LastColor = -1;
	mn_ScrollWidth = 0;
	mb_ScrollVisible = FALSE; mb_Scroll2Visible = FALSE; /*mb_ScrollTimerSet = FALSE;*/ mb_ScrollAutoPopup = FALSE;
	//m_TScrollShow, m_TScrollHide, m_TScrollCheck
	memset(&mrc_LastClient, 0, sizeof(mrc_LastClient));
	mb_LastTabVisible = false; mb_LastAlwaysScroll = false;
	mb_VTracking = false;
#ifdef _DEBUG
	mn_ColorIdx = 1; // Blue
#else
	mn_ColorIdx = 0; // Black
#endif
	//mh_UxTheme = NULL; mh_ThemeData = NULL; mfn_OpenThemeData = NULL; mfn_CloseThemeData = NULL;
}

CConEmuBack::~CConEmuBack()
{
}

HWND CConEmuBack::Create()
{
	mn_LastColor = gSet.GetColors()[mn_ColorIdx];
	mh_BackBrush = CreateSolidBrush(mn_LastColor);

	DWORD dwLastError = 0;
	WNDCLASS wc = {CS_OWNDC/*|CS_SAVEBITS*/, CConEmuBack::BackWndProc, 0, 0, 
			g_hInstance, NULL, LoadCursor(NULL, IDC_ARROW), 
			mh_BackBrush, 
			NULL, szClassNameBack};
	if (!RegisterClass(&wc))
	{
		dwLastError = GetLastError();
		mh_WndBack = (HWND)-1; // ����� �������� �� �������
		mh_WndScroll = (HWND)-1;
		MBoxA(_T("Can't register background window class!"));
		return NULL;
	}
	// Scroller
	wc.lpfnWndProc = CConEmuBack::ScrollWndProc;
	wc.lpszClassName = szClassNameScroll;
	if (!RegisterClass(&wc))
	{
		dwLastError = GetLastError();
		mh_WndBack = (HWND)-1; // ����� �������� �� �������
		mh_WndScroll = (HWND)-1;
		MBoxA(_T("Can't register scroller window class!"));
		return NULL;
	}

	DWORD style = /*WS_VISIBLE |*/ WS_CHILD | WS_CLIPSIBLINGS ;
	//RECT rc = gConEmu.ConsoleOffsetRect();
	RECT rcClient; GetClientRect(ghWnd, &rcClient);
	RECT rc = gConEmu.CalcRect(CER_BACK, rcClient, CER_MAINCLIENT);

	mh_WndBack = CreateWindow(szClassNameBack, NULL, style, 
		rc.left, rc.top,
		rcClient.right - rc.right - rc.left,
		rcClient.bottom - rc.bottom - rc.top,
		ghWnd, NULL, (HINSTANCE)g_hInstance, NULL);
	if (!mh_WndBack)
	{
		dwLastError = GetLastError();
		mh_WndBack = (HWND)-1; // ����� �������� �� �������
		MBoxA(_T("Can't create background window!"));
		return NULL; //
	}

	// ���������
	//style = SBS_RIGHTALIGN/*|WS_VISIBLE*/|SBS_VERT|WS_CHILD|WS_CLIPSIBLINGS;
	//mh_WndScroll = CreateWindowEx(0/*|WS_EX_LAYERED*/ /*WS_EX_TRANSPARENT*/, L"SCROLLBAR", NULL, style,
	//	rc.left, rc.top,
	//	rcClient.right - rc.right - rc.left,
	//	rcClient.bottom - rc.bottom - rc.top,
	//	ghWnd, NULL, (HINSTANCE)g_hInstance, NULL);
	style = WS_VSCROLL|WS_CHILD|WS_CLIPSIBLINGS;
	mn_ScrollWidth = GetSystemMetrics(SM_CXVSCROLL);
	mh_WndScroll = CreateWindowEx(0/*|WS_EX_LAYERED*/ /*WS_EX_TRANSPARENT*/, szClassNameScroll, NULL, style,
		rc.left - mn_ScrollWidth, rc.top,
		mn_ScrollWidth, rc.bottom - rc.top,
		ghWnd, NULL, (HINSTANCE)g_hInstance, NULL);
	if (!mh_WndScroll)
	{
		dwLastError = GetLastError();
		mh_WndScroll = (HWND)-1; // ����� �������� �� �������
		MBoxA(_T("Can't create scrollbar window!"));
		return NULL; //
	}
	//GetWindowRect(mh_WndScroll, &rcClient);
	//mn_ScrollWidth = rcClient.right - rcClient.left;
	TODO("alpha-blended. ������ ��� WS_CHILD ��� �� ��������...");
	//BOOL lbRcLayered = SetLayeredWindowAttributes ( mh_WndScroll, 0, 100, LWA_ALPHA );
	//if (!lbRcLayered)
	//	dwLastError = GetLastError();

	
	//// ����� �������� ������ ����� �������� �������� ����, ����� IsAppThemed ����� ���������� FALSE
    //BOOL bAppThemed = FALSE, bThemeActive = FALSE;
    //FAppThemed pfnThemed = NULL;
    //mh_UxTheme = LoadLibrary ( L"UxTheme.dll" );
    //if (mh_UxTheme) {
    //	pfnThemed = (FAppThemed)GetProcAddress( mh_UxTheme, "IsAppThemed" );
    //	if (pfnThemed) bAppThemed = pfnThemed();
    //	pfnThemed = (FAppThemed)GetProcAddress( mh_UxTheme, "IsThemeActive" );
    //	if (pfnThemed) bThemeActive = pfnThemed();
    //}
    //if (!bAppThemed || !bThemeActive) {
    //	FreeLibrary(mh_UxTheme); mh_UxTheme = NULL;
	//} else {
    //	mfn_OpenThemeData = (FOpenThemeData)GetProcAddress( mh_UxTheme, "OpenThemeData" );
    //	mfn_OpenThemeDataEx = (FOpenThemeData)GetProcAddress( mh_UxTheme, "OpenThemeDataEx" );
    //	mfn_CloseThemeData = (FCloseThemeData)GetProcAddress( mh_UxTheme, "CloseThemeData" );
    //	
    //	if (mfn_OpenThemeDataEx)
    //		mh_ThemeData = mfn_OpenThemeDataEx ( mh_WndScroll, L"Scrollbar", OTD_NONCLIENT );
	//}

	return mh_WndBack;
}

LRESULT CALLBACK CConEmuBack::BackWndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

	if (messg == WM_SYSCHAR)
		return TRUE;

	switch (messg)
	{
		case WM_CREATE:
			gConEmu.m_Back->mh_WndBack = hWnd;
			break;
		case WM_DESTROY:
			//if (gConEmu.m_Back->mh_ThemeData && gConEmu.m_Back->mfn_CloseThemeData) {
			//	gConEmu.m_Back->mfn_CloseThemeData ( gConEmu.m_Back->mh_ThemeData );
			//	gConEmu.m_Back->mh_ThemeData = NULL;
			//}
			//if (gConEmu.m_Back->mh_UxTheme) {
			//	FreeLibrary(gConEmu.m_Back->mh_UxTheme);
			//	gConEmu.m_Back->mh_UxTheme = NULL;
			//}
			DeleteObject(gConEmu.m_Back->mh_BackBrush);
			break;
		case WM_SETFOCUS:
			SetFocus(ghWnd); // ����� ������ ���� � ������� ����!
			return 0;
	    case WM_VSCROLL:
	        //POSTMESSAGE(ghConWnd, messg, wParam, lParam, FALSE);
			// -- �� ������ ���������� ������
			_ASSERTE(messg!=WM_VSCROLL);
	        break;
		case WM_PAINT:
			{
				PAINTSTRUCT ps; memset(&ps, 0, sizeof(ps));
				HDC hDc = BeginPaint(hWnd, &ps);
				#ifndef SKIP_ALL_FILLRECT
				if (!IsRectEmpty(&ps.rcPaint))
					FillRect(hDc, &ps.rcPaint, gConEmu.m_Back->mh_BackBrush);
				#endif
				EndPaint(hWnd, &ps);
			}
			return 0;
#ifdef _DEBUG
		case WM_SIZE:
			{
			UINT nW = LOWORD(lParam), nH = HIWORD(lParam);
			result = DefWindowProc(hWnd, messg, wParam, lParam);
			}
			return result;
		case WM_WINDOWPOSCHANGING:
			{
			WINDOWPOS* pwp = (WINDOWPOS*)lParam;
			result = DefWindowProc(hWnd, messg, wParam, lParam);
			}
			return result;
		case WM_WINDOWPOSCHANGED:
			{
			WINDOWPOS* pwp = (WINDOWPOS*)lParam;
			result = DefWindowProc(hWnd, messg, wParam, lParam);
			}
			return result;
#endif
	}

    result = DefWindowProc(hWnd, messg, wParam, lParam);

	return result;
}

LRESULT CALLBACK CConEmuBack::ScrollWndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	if (messg == WM_SYSCHAR)
		return TRUE;

	switch (messg)
	{
		case WM_CREATE:
			gConEmu.m_Back->mh_WndScroll = hWnd;
			gConEmu.m_Back->m_TScrollShow.Init(hWnd, TIMER_SCROLL_SHOW, TIMER_SCROLL_SHOW_DELAY);
			gConEmu.m_Back->m_TScrollHide.Init(hWnd, TIMER_SCROLL_HIDE, TIMER_SCROLL_HIDE_DELAY);
			gConEmu.m_Back->m_TScrollCheck.Init(hWnd, TIMER_SCROLL_CHECK, TIMER_SCROLL_CHECK_DELAY);
			break;
		case WM_VSCROLL:
			//POSTMESSAGE(ghConWnd, messg, wParam, lParam, FALSE);
			if (LOWORD(wParam) == SB_THUMBTRACK)
				gConEmu.m_Back->mb_VTracking = true;
			gConEmu.ActiveCon()->RCon()->OnSetScrollPos(wParam);
			break;
		case WM_SETFOCUS:
			SetFocus(ghWnd); // ����� ������ ���� � ������� ����!
			return 0;
		case WM_TIMER:
			switch (wParam)
			{
				case TIMER_SCROLL_CHECK:
					if (gConEmu.m_Back->mb_Scroll2Visible)
					{
						if (!gConEmu.m_Back->CheckMouseOverScroll())
							gConEmu.m_Back->HideScroll(FALSE/*abImmediate*/);
					}
					break;
				case TIMER_SCROLL_SHOW:
					if (gConEmu.m_Back->CheckMouseOverScroll() || gConEmu.m_Back->CheckScrollAutoPopup())
						gConEmu.m_Back->ShowScroll(TRUE/*abImmediate*/);
					else
						gConEmu.m_Back->mb_Scroll2Visible = FALSE;
					if (gConEmu.m_Back->m_TScrollShow.IsStarted())
						gConEmu.m_Back->m_TScrollShow.Stop();
					break;
				case TIMER_SCROLL_HIDE:
					if (!gConEmu.m_Back->CheckMouseOverScroll())
						gConEmu.m_Back->HideScroll(TRUE/*abImmediate*/);
					else
						gConEmu.m_Back->mb_Scroll2Visible = TRUE;
					if (gConEmu.m_Back->m_TScrollHide.IsStarted())
						gConEmu.m_Back->m_TScrollHide.Stop();
					break;
			}
			break;
	}

	result = DefWindowProc(hWnd, messg, wParam, lParam);

	return result;
}

void CConEmuBack::Resize()
{
	#if defined(EXT_GNUC_LOG)
	CVirtualConsole* pVCon = gConEmu.ActiveCon();
	#endif
	
	if (!mh_WndBack || !IsWindow(mh_WndBack))
	{
    	#if defined(EXT_GNUC_LOG)
    	if (gSet.isAdvLogging>1) 
        	pVCon->RCon()->LogString("  --  CConEmuBack::Resize() - exiting, mh_WndBack failed");
        #endif
		return;
	}

	//RECT rc = gConEmu.ConsoleOffsetRect();
	RECT rcClient; GetClientRect(ghWnd, &rcClient);

	bool bTabsShown = gConEmu.mp_TabBar->IsShown();
	if (mb_LastTabVisible == bTabsShown && mb_LastAlwaysScroll == (gSet.isAlwaysShowScrollbar == 1))
	{
		if (memcmp(&rcClient, &mrc_LastClient, sizeof(RECT))==0)
		{
        	#if defined(EXT_GNUC_LOG)
        	char szDbg[255];
        	wsprintfA(szDbg, "  --  CConEmuBack::Resize() - exiting, (%i,%i,%i,%i)==(%i,%i,%i,%i)",
        		rcClient.left, rcClient.top, rcClient.right, rcClient.bottom,
        		mrc_LastClient.left, mrc_LastClient.top, mrc_LastClient.right, mrc_LastClient.bottom);
        	if (gSet.isAdvLogging>1) 
            	pVCon->RCon()->LogString(szDbg);
            #endif
			return; // ������ �� ��������
		}
	}
	memmove(&mrc_LastClient, &rcClient, sizeof(RECT)); // ����� ��������
	mb_LastTabVisible = bTabsShown;
	mb_LastAlwaysScroll = (gSet.isAlwaysShowScrollbar == 1);

	//RECT rcScroll; GetWindowRect(mh_WndScroll, &rcScroll);

	RECT rc = gConEmu.CalcRect(CER_BACK, rcClient, CER_MAINCLIENT);

	#ifdef _DEBUG
	RECT rcTest;
	GetClientRect(mh_WndBack, &rcTest);
	#endif

	#if defined(EXT_GNUC_LOG)
	char szDbg[255]; wsprintfA(szDbg, "  --  CConEmuBack::Resize() - X=%i, Y=%i, W=%i, H=%i", rc.left, rc.top, 	rc.right - rc.left,	rc.bottom - rc.top );
	if (gSet.isAdvLogging>1) 
    	pVCon->RCon()->LogString(szDbg);
    #endif
    
	// ��� ������� ���� ���������. ��� ������ ���������������
	// ������� ����������� �������. �� ��� ���� ������������� �������!
	WARNING("DoubleView");
	MoveWindow(mh_WndBack, 
		rc.left, rc.top,
		rc.right - rc.left,
		rc.bottom - rc.top,
		1);

	rc = gConEmu.CalcRect(CER_SCROLL, rcClient, CER_MAINCLIENT);
#ifdef _DEBUG
	if (rc.bottom != rcClient.bottom || rc.right != rcClient.right)
	{
		_ASSERTE(rc.bottom == rcClient.bottom && rc.right == rcClient.right);
	}
#endif
	MoveWindow(mh_WndScroll,
		rc.left, rc.top,
		rc.right - rc.left,
		rc.bottom - rc.top,
		1);
	//MoveWindow(mh_WndScroll, 
	//	rc.right - (rcScroll.right-rcScroll.left),
	//	rc.top,
	//	rcScroll.right-rcScroll.left,
	//	rc.bottom - rc.top,
	//	1);

	#ifdef _DEBUG
	GetClientRect(mh_WndBack, &rcTest);
	#endif
}

void CConEmuBack::Refresh()
{
	COLORREF* pcr = gSet.GetColors(gConEmu.isMeForeground());
	if (mn_LastColor == pcr[mn_ColorIdx])
		return;

	mn_LastColor = pcr[mn_ColorIdx];
	HBRUSH hNewBrush = CreateSolidBrush(mn_LastColor);

	SetClassLongPtr(mh_WndBack, GCLP_HBRBACKGROUND, (LONG)hNewBrush);
	DeleteObject(mh_BackBrush);
	mh_BackBrush = hNewBrush;

	//RECT rc; GetClientRect(mh_Wnd, &rc);
	//InvalidateRect(mh_Wnd, &rc, TRUE);
	Invalidate();
}

void CConEmuBack::RePaint()
{
	WARNING("mh_WndBack invisible, ������� ��������� ������ �� �����");
	if (mh_WndBack && mh_WndBack!=(HWND)-1)
	{
		Refresh();
		UpdateWindow(mh_WndBack);
		//if (mh_WndScroll) UpdateWindow(mh_WndScroll);
	}
}

void CConEmuBack::Invalidate()
{
	if (this && mh_WndBack) {
		InvalidateRect(mh_WndBack, NULL, FALSE);
		InvalidateRect(mh_WndScroll, NULL, FALSE);
	}
}

// ������ ������� TRUE, ���� ������� ���� �� ����� ���������� � �������
BOOL CConEmuBack::TrackMouse()
{
	BOOL lbCapture = FALSE; // �� ��������� - ���� �� �������������
	BOOL lbHided = FALSE;
	BOOL lbBufferMode = gConEmu.ActiveCon()->RCon()->isBufferHeight();
	
	BOOL lbOverVScroll = CheckMouseOverScroll();
	
	if (lbOverVScroll || (gSet.isAlwaysShowScrollbar == 1))
	{
		if (!mb_Scroll2Visible)
		{
			mb_Scroll2Visible = TRUE;
			ShowScroll(FALSE/*abImmediate*/); // ���� gSet.isAlwaysShowScrollbar==1 - ���� ����������
		}
		else if (mb_ScrollVisible && (gSet.isAlwaysShowScrollbar != 1) && !m_TScrollCheck.IsStarted())
		{
			m_TScrollCheck.Start();
		}
	}
	else if (mb_Scroll2Visible)
	{
		_ASSERTE(gSet.isAlwaysShowScrollbar != 1);
		mb_Scroll2Visible = FALSE;
		HideScroll(FALSE/*abImmediate*/); // ���� gSet.isAlwaysShowScrollbar==1 - ���� ����������
	}
	
	lbCapture = (lbOverVScroll && mb_ScrollVisible);
	
	return lbCapture;
}

BOOL CConEmuBack::CheckMouseOverScroll()
{
	BOOL lbOverVScroll = FALSE;
	BOOL lbBufferMode = gConEmu.ActiveCon()->RCon()->isBufferHeight();
	
	if (lbBufferMode)
	{
		// ���� ��������� ������ ������ � ���������
		if (mb_VTracking && !isPressed(VK_LBUTTON))
		{	// ������� ������
			mb_VTracking = false;
		}
		
		// ����� ������ �� ��������, ����� �� ����� ������
		if (mb_VTracking)
		{
			lbOverVScroll = TRUE;
		}
		else // ������ ��������, ���� ���� � ��� ����������� - �������� ���
		{
			POINT ptCur; RECT rcScroll;
			GetCursorPos(&ptCur);
			GetWindowRect(mh_WndScroll, &rcScroll);
			if (PtInRect(&rcScroll, ptCur))
			{
				// ���� �� ��������� - �� ��������� ������ ��������� � ������� ���� ����
				//if (!gSet.isSelectionModifierPressed())
				if (!(isPressed(VK_SHIFT) || isPressed(VK_CONTROL) || isPressed(VK_MENU) || isPressed(VK_LBUTTON)))
					lbOverVScroll = TRUE;
			}
		}
	}

	return lbOverVScroll;
}

BOOL CConEmuBack::CheckScrollAutoPopup()
{
	return mb_ScrollAutoPopup;
}

void CConEmuBack::SetScroll(BOOL abEnabled, int anTop, int anVisible, int anHeight)
{
    int nCurPos = 0;
    //BOOL lbScrollRc = FALSE;
    SCROLLINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cbSize = sizeof(si);
    si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE; // | SIF_TRACKPOS;
	si.nMin = 0;
	if (!abEnabled)
	{
		si.nPos = 0;
		si.nPage = 1;
		si.nMax = 1;
	}
	else
	{
		si.nPos = anTop;
		si.nPage = anVisible - 1;
		si.nMax = anHeight;
	}

	//// ���� ����� "BufferHeight" ������� - �������� �� ����������� ���� ������� ��������� ������ ���������
	//if (con.bBufferHeight) {
	//    lbScrollRc = GetScrollInfo(hConWnd, SB_VERT, &si);
	//} else {
	//    // ���������� ��������� ���, ����� ������ �� ������������ (��� �� 0)
	//}

    //TODO("����� ��� ������������� '�������' ������ ���������");
    nCurPos = SetScrollInfo(mh_WndScroll, SB_VERT, &si, true);

	if (!abEnabled)
	{
		mb_ScrollAutoPopup = FALSE;
		if ((gSet.isAlwaysShowScrollbar == 1) && IsWindowEnabled(mh_WndScroll))
		{
			EnableScrollBar(mh_WndScroll, SB_VERT, ESB_DISABLE_BOTH);
			EnableWindow(mh_WndScroll, FALSE);
		}
		HideScroll((gSet.isAlwaysShowScrollbar != 1));
	}
	else
	{
		if (!IsWindowEnabled(mh_WndScroll))
		{
			EnableWindow(mh_WndScroll, TRUE);
			EnableScrollBar(mh_WndScroll, SB_VERT, ESB_ENABLE_BOTH);
		}
		// �������� ���������, ���� �������� ����� ���������� � ����������
		if ((si.nPos > 0) && (si.nPos < (si.nMax - (int)si.nPage - 1)) && gSet.isAlwaysShowScrollbar)
		{
			mb_ScrollAutoPopup = (gSet.isAlwaysShowScrollbar == 2);
			if (!mb_Scroll2Visible)
				ShowScroll((gSet.isAlwaysShowScrollbar == 1));
		}
		else
		{
			mb_ScrollAutoPopup = FALSE;
		}
	}
}

void CConEmuBack::ShowScroll(BOOL abImmediate)
{
	bool bTShow = false, bTCheck = false;
	
	if (abImmediate || (gSet.isAlwaysShowScrollbar == 1))
	{
		mb_ScrollVisible = TRUE; mb_Scroll2Visible = TRUE;
		if (!IsWindowVisible(mh_WndScroll))
			apiShowWindow(mh_WndScroll, SW_SHOWNOACTIVATE);
		SetWindowPos(mh_WndScroll, HWND_TOP, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW);
		
		if ((gSet.isAlwaysShowScrollbar != 1))
		{
			bTCheck = true;
		}
	}
	else
	{
		mb_Scroll2Visible = TRUE;
		bTShow = true;
	}

	if (bTCheck)
		m_TScrollCheck.Start(mb_ScrollAutoPopup ? TIMER_SCROLL_CHECK_DELAY2 : TIMER_SCROLL_CHECK_DELAY);
	else if (m_TScrollCheck.IsStarted())
		m_TScrollCheck.Stop();
	//
	if (bTShow)
		m_TScrollShow.Start(mb_ScrollAutoPopup ? TIMER_SCROLL_SHOW_DELAY2 : TIMER_SCROLL_SHOW_DELAY);
	else if (m_TScrollShow.IsStarted())
		m_TScrollShow.Stop();
	//
	if (m_TScrollHide.IsStarted())
		m_TScrollHide.Stop();
}

void CConEmuBack::HideScroll(BOOL abImmediate)
{
	bool bTHide = false;

	mb_ScrollAutoPopup = FALSE;
	
	if (gSet.isAlwaysShowScrollbar == 1)
	{
		// ��������� ������ ������������! �������� ������!
	}
	else if (abImmediate)
	{
		mb_ScrollVisible = FALSE;
		mb_Scroll2Visible = FALSE;
		if (IsWindowVisible(mh_WndScroll))
			apiShowWindow(mh_WndScroll, SW_HIDE);

		RECT rcScroll; GetWindowRect(mh_WndScroll, &rcScroll);
		// ������� ���������, ����������� ����� �������� ����
		MapWindowPoints(NULL, ghWnd, (LPPOINT)&rcScroll, 2);
		InvalidateRect(ghWnd, &rcScroll, FALSE);
	}
	else
	{
		mb_Scroll2Visible = FALSE;
		bTHide = true;
		//m_TScrollHide.Start();
	}

	if (m_TScrollCheck.IsStarted())
		m_TScrollCheck.Stop();
	//
	if (m_TScrollShow.IsStarted())
		m_TScrollShow.Stop();
	//
	if (bTHide)
		m_TScrollHide.Start();
	else if (m_TScrollHide.IsStarted())
		m_TScrollHide.Stop();		
}
