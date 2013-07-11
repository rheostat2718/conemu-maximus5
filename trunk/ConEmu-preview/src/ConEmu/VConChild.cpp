
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
#include "Header.h"
#include "../common/common.hpp"
#include "../common/WinObjects.h"
#include "ConEmu.h"
#include "VConChild.h"
#include "Options.h"
#include "TabBar.h"
#include "VirtualConsole.h"
#include "RealConsole.h"
#include "VConGroup.h"
#include "GestureEngine.h"

#if defined(__GNUC__)
#define EXT_GNUC_LOG
#endif

#define DEBUGSTRDRAW(s) //DEBUGSTR(s)
#define DEBUGSTRTABS(s) //DEBUGSTR(s)
#define DEBUGSTRLANG(s) //DEBUGSTR(s)

//#define SCROLLHIDE_TIMER_ID 1726
#define TIMER_SCROLL_SHOW         3201
#define TIMER_SCROLL_SHOW_DELAY   gpSet->nScrollBarAppearDelay /*1000*/
#define TIMER_SCROLL_SHOW_DELAY2  (gpSet->nScrollBarAppearDelay/2) /*500*/
#define TIMER_SCROLL_HIDE         3202
#define TIMER_SCROLL_HIDE_DELAY   gpSet->nScrollBarDisappearDelay /*1000*/
#define TIMER_SCROLL_CHECK        3203
#define TIMER_SCROLL_CHECK_DELAY  250
//#define TIMER_SCROLL_CHECK_DELAY2 1000
#define LOCK_DC_RECT_TIMEOUT      2000
#define TIMER_AUTOCOPY_DELAY      (GetDoubleClickTime()*10/9)
#define TIMER_AUTOCOPY            3204

CConEmuChild::CConEmuChild()
{
	mn_MsgTabChanged = RegisterWindowMessage(CONEMUTABCHANGED);
	mn_MsgPostFullPaint = RegisterWindowMessage(L"CConEmuChild::PostFullPaint");
	mn_MsgSavePaneSnapshoot = RegisterWindowMessage(L"CConEmuChild::SavePaneSnapshoot");
	mn_MsgDetachPosted = RegisterWindowMessage(L"CConEmuChild::Detach");
	mn_MsgRestoreChildFocus = RegisterWindowMessage(CONEMUMSG_RESTORECHILDFOCUS);
#ifdef _DEBUG
	mn_MsgCreateDbgDlg = RegisterWindowMessage(L"CConEmuChild::MsgCreateDbgDlg");
	hDlgTest = NULL;
#endif
	mb_PostFullPaint = FALSE;
	mn_LastPostRedrawTick = 0;
	mb_IsPendingRedraw = FALSE;
	mb_RedrawPosted = FALSE;
	ZeroStruct(Caret);
	mb_DisableRedraw = FALSE;
	mh_WndDC = NULL;
	mh_WndBack = NULL;
	mh_LastGuiChild = NULL;
	mb_ScrollVisible = FALSE; mb_Scroll2Visible = FALSE; /*mb_ScrollTimerSet = FALSE;*/ mb_ScrollAutoPopup = FALSE;
	mb_VTracking = FALSE;

	ZeroStruct(m_si);
	m_si.cbSize = sizeof(m_si);
	m_si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE /*| SIF_DISABLENOSCROLL*/;
	mb_ScrollDisabled = FALSE;
	m_LastAlwaysShowScrollbar = gpSet->isAlwaysShowScrollbar;
	mb_ScrollRgnWasSet = false;
	
	ZeroStruct(m_LockDc);
}

CConEmuChild::~CConEmuChild()
{
	if (mh_WndDC)
	{
		DestroyWindow(mh_WndDC);
		mh_WndDC = NULL;
	}
	if (mh_WndBack)
	{
		DestroyWindow(mh_WndBack);
		mh_WndBack = NULL;
	}
}

HWND CConEmuChild::CreateView()
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return NULL;
	}
	if (mh_WndDC || mh_WndBack)
	{
		_ASSERTE(mh_WndDC == NULL && mh_WndBack == NULL);
		return mh_WndDC;
	}

	if (!gpConEmu->isMainThread())
	{
		// ���� ������ ����������� � ������� ����!
		HWND hCreate = gpConEmu->PostCreateView(this); UNREFERENCED_PARAMETER(hCreate);
		_ASSERTE(hCreate && (hCreate == mh_WndDC));
		return mh_WndDC;
	}

	CVirtualConsole* pVCon = (CVirtualConsole*)this;
	_ASSERTE(pVCon!=NULL);
	//-- ��� ������� ������ ���������, guard �� �����
	//CVConGuard guard(pVCon);

	TODO("�������� ghWnd �� ghWndWork");
	HWND hParent = ghWnd;
	// ��� ������ - �� �� �����, ��� � � �������� ����
	DWORD style = /*WS_VISIBLE |*/ WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	DWORD styleEx = 0;
	
	RECT rcBack = gpConEmu->CalcRect(CER_BACK, pVCon);
	RECT rc = gpConEmu->CalcRect(CER_DC, rcBack, CER_BACK, pVCon);

	mh_WndBack = CreateWindowEx(styleEx, gsClassNameBack, L"BackWND", style,
		rcBack.left, rcBack.top, rcBack.right - rcBack.left, rcBack.bottom - rcBack.top, hParent, NULL, (HINSTANCE)g_hInstance, pVCon);

	mh_WndDC = CreateWindowEx(styleEx, gsClassName, L"DrawWND", style,
		rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hParent, NULL, (HINSTANCE)g_hInstance, pVCon);

	if (!mh_WndDC || !mh_WndBack)
	{
		DisplayLastError(L"Can't create DC window!");
		return NULL; //
	}

	SetWindowPos(mh_WndDC, HWND_TOP, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);

	// ���������� ���������� ����� � ������������ ����
	SetConEmuEnvVar(mh_WndDC);

	return mh_WndDC;
}

HWND CConEmuChild::GetView()
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return NULL;
	}
	return mh_WndDC;
}

HWND CConEmuChild::GetBack()
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return NULL;
	}
	return mh_WndBack;
}

BOOL CConEmuChild::ShowView(int nShowCmd)
{
	if (!this || !mh_WndDC)
		return FALSE;

	BOOL bRc = FALSE;
	DWORD nTID = 0, nPID = 0;
	wchar_t sInfo[200];

	// ������ ���� ������� � ������� ����!
	nTID = GetWindowThreadProcessId(mh_WndDC, &nPID);

	#ifdef _DEBUG
	DWORD nMainThreadID = GetWindowThreadProcessId(ghWnd, &nPID);
	_ASSERTE(nTID==nMainThreadID);
	#endif

	// ���� ��� "GUI" ����� - ����� ��������� ���������� ��-�� ��������� ����
	CVirtualConsole* pVCon = (CVirtualConsole*)this;
	_ASSERTE(pVCon!=NULL);
	CVConGuard guard(pVCon);

	HWND hChildGUI = pVCon->GuiWnd();
	BOOL bGuiVisible = (hChildGUI && nShowCmd) ? pVCon->RCon()->isGuiVisible() : FALSE;


	if (gpSetCls->isAdvLogging)
	{
		if (hChildGUI != NULL)
			_wsprintf(sInfo, SKIPLEN(countof(sInfo)) L"ShowView: Back=x%08X, DC=x%08X, ChildGUI=x%08X, ShowCMD=%u, ChildVisible=%u",
				(DWORD)mh_WndBack, (DWORD)mh_WndDC, (DWORD)hChildGUI, nShowCmd, bGuiVisible);
		else
			_wsprintf(sInfo, SKIPLEN(countof(sInfo)) L"ShowView: Back=x%08X, DC=x%08X, ShowCMD=%u",
				(DWORD)mh_WndBack, (DWORD)mh_WndDC, nShowCmd);
		gpConEmu->LogString(sInfo);
	}


	if ((GetCurrentThreadId() != nTID) || (hChildGUI != NULL))
	{
		bRc = ShowWindowAsync(mh_WndBack, nShowCmd);
		bRc = ShowWindowAsync(mh_WndDC, bGuiVisible ? SW_HIDE : nShowCmd);
	}
	else
	{
		bRc = ShowWindow(mh_WndBack, nShowCmd);
		bRc = ShowWindow(mh_WndDC, nShowCmd);
		if (nShowCmd)
		{
			SetWindowPos(mh_WndDC, HWND_TOP, 0, 0, 0,0, SWP_NOSIZE|SWP_NOMOVE);
			SetWindowPos(mh_WndBack, mh_WndDC, 0, 0, 0,0, SWP_NOSIZE|SWP_NOMOVE);
		}
	}

	return bRc;
}

LRESULT CConEmuChild::ChildWndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	// Logger
	MSG msgStr = {hWnd, messg, wParam, lParam};
	ConEmuMsgLogger::Log(msgStr, ConEmuMsgLogger::msgCanvas);

	if (gpSetCls->isAdvLogging >= 4)
	{
		gpConEmu->LogMessage(hWnd, messg, wParam, lParam);
	}

	CVirtualConsole* pVCon = NULL;
	if (messg == WM_CREATE || messg == WM_NCCREATE)
	{
		LPCREATESTRUCT lp = (LPCREATESTRUCT)lParam;
		pVCon = (CVirtualConsole*)lp->lpCreateParams;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pVCon);

		pVCon->m_TAutoCopy.Init(hWnd, TIMER_AUTOCOPY, TIMER_AUTOCOPY_DELAY);

		pVCon->m_TScrollShow.Init(hWnd, TIMER_SCROLL_SHOW, TIMER_SCROLL_SHOW_DELAY);
		pVCon->m_TScrollHide.Init(hWnd, TIMER_SCROLL_HIDE, TIMER_SCROLL_HIDE_DELAY);
		#ifndef SKIP_HIDE_TIMER
		pVCon->m_TScrollCheck.Init(hWnd, TIMER_SCROLL_CHECK, TIMER_SCROLL_CHECK_DELAY);
		#endif

	}
	else
	{
		pVCon = (CVirtualConsole*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	}

	CVConGuard guard(pVCon);


	if (messg == WM_SYSCHAR)
	{
		_ASSERTE(FALSE); // �� ����, ������ ��� ���� �� ������
		// ����� �� ������
		result = TRUE;
		goto wrap;
	}

	if (!pVCon)
	{
		_ASSERTE(pVCon!=NULL);
		result = DefWindowProc(hWnd, messg, wParam, lParam);
		goto wrap;
	}

	switch (messg)
	{
		case WM_SHOWWINDOW:
			{
				#ifdef _DEBUG
				HWND hGui = pVCon->GuiWnd();
				if (hGui)
				{
					_ASSERTE((wParam==0) && "Show DC while GuiWnd exists");
				}
				#endif
				result = DefWindowProc(hWnd, messg, wParam, lParam);
				break;
			}
		case WM_SETFOCUS:
			// ���� � ������� �������� "GUI" ���� (GUI �����), �� ����� ����� ������ ����.
			{
				// ����� ������ ���� � ������� ����! �� ����������� ������ ������ � GUI ������.
				pVCon->setFocus();
			}
			return 0;
		case WM_ERASEBKGND:
			result = 0;
			break;
		case WM_PAINT:
			result = pVCon->OnPaint();
			break;
		case WM_PRINTCLIENT:
			if (wParam && (lParam & PRF_CLIENT))
			{
				pVCon->PrintClient((HDC)wParam, false, NULL);
			}
			break;
		case WM_SIZE:
			#ifdef _DEBUG
			{
				RECT rc; GetClientRect(hWnd, &rc);
				short cx = LOWORD(lParam);
				rc.left = rc.left;
			}
			#endif
			result = pVCon->OnSize(wParam, lParam);
			break;
		case WM_MOVE:
			result = pVCon->OnMove(wParam, lParam);
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
				switch (messg)
				{
					case WM_VSCROLL:
						switch (LOWORD(wParam))
						{
						case SB_THUMBTRACK:
						case SB_THUMBPOSITION:
							pVCon->mb_VTracking = TRUE;
							break;
						case SB_ENDSCROLL:
							pVCon->mb_VTracking = FALSE;
							break;
						}
						pVCon->RCon()->OnSetScrollPos(wParam);
						break;

					case WM_LBUTTONUP:
						pVCon->mb_VTracking = FALSE;
						break;
				}

				TODO("��������� ghWndWork");
				HWND hParent = ghWnd;
				_ASSERTE(GetParent(hWnd)==ghWnd);

				if (messg >= WM_MOUSEFIRST && messg <= WM_MOUSELAST)
				{
					POINT pt = {LOWORD(lParam),HIWORD(lParam)};
					MapWindowPoints(hWnd, hParent, &pt, 1);
					lParam = MAKELONG(pt.x,pt.y);
				}

				result = gpConEmu->WndProc(hParent, messg, wParam, lParam);
			}
			break;
		case WM_IME_NOTIFY:
			break;
		case WM_INPUTLANGCHANGE:
		case WM_INPUTLANGCHANGEREQUEST:
			{
				#ifdef _DEBUG
				if (IsDebuggerPresent())
				{
					WCHAR szMsg[128];
					_wsprintf(szMsg, SKIPLEN(countof(szMsg)) L"InChild %s(CP:%i, HKL:0x%08X)\n",
							  (messg == WM_INPUTLANGCHANGE) ? L"WM_INPUTLANGCHANGE" : L"WM_INPUTLANGCHANGEREQUEST",
							  (DWORD)wParam, (DWORD)lParam);
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
			break;
#endif
		case WM_SETCURSOR:
			{
				gpConEmu->WndProc(hWnd, messg, wParam, lParam);

				//if (!result)
				//	result = DefWindowProc(hWnd, messg, wParam, lParam);
			}
			// If an application processes this message, it should return TRUE to halt further processing or FALSE to continue.
			break;
			
		case WM_SYSCOMMAND:
			// -- ������ �����������, ������
			result = DefWindowProc(hWnd, messg, wParam, lParam);
			//if (wParam >= SC_SIZE && wParam <= SC_CONTEXTHELP/*0xF180*/)
			//{
			//	// ��������� ��������/������������/� �.�. ���� ������� - ���������
			//	_ASSERTE(!(wParam >= SC_SIZE && wParam <= SC_CONTEXTHELP));
			//}
			//else
			//{
			//	// �� ����, ���� ������ ��������� ������ �� ������
			//	_ASSERTE(FALSE);
			//}
			break;

		case WM_TIMER:
			{
				switch(wParam)
				{
				#ifndef SKIP_HIDE_TIMER // �� ����� ������� �� ������� - ������ �� �������� �����
				case TIMER_SCROLL_CHECK:

					if (pVCon->mb_Scroll2Visible)
					{
						if (!pVCon->CheckMouseOverScroll())
						{
							pVCon->HideScroll(FALSE/*abImmediate*/);
						}
					}

					break;
				#endif

				case TIMER_SCROLL_SHOW:

					if (pVCon->CheckMouseOverScroll() || pVCon->CheckScrollAutoPopup())
						pVCon->ShowScroll(TRUE/*abImmediate*/);
					else
						pVCon->mb_Scroll2Visible = FALSE;

					if (pVCon->m_TScrollShow.IsStarted())
						pVCon->m_TScrollShow.Stop();

					break;

				case TIMER_SCROLL_HIDE:

					if (!pVCon->CheckMouseOverScroll())
						pVCon->HideScroll(TRUE/*abImmediate*/);
					else
						pVCon->mb_Scroll2Visible = TRUE;

					if (pVCon->m_TScrollHide.IsStarted())
						pVCon->m_TScrollHide.Stop();

					break;

				case TIMER_AUTOCOPY:
					pVCon->SetAutoCopyTimer(false);
					if (!isPressed(VK_LBUTTON))
					{
						pVCon->RCon()->AutoCopyTimer();
					}
					break;
				}
				break;
			} // case WM_TIMER:

		case WM_GESTURENOTIFY:
		case WM_GESTURE:
			{
				gpConEmu->ProcessGestureMessage(hWnd, messg, wParam, lParam, result);
				break;
			} // case WM_GESTURE, WM_GESTURENOTIFY

		default:

			// ��������� �������� �� ConEmuPlugin
			if (messg == pVCon->mn_MsgTabChanged)
			{
				if (gpSet->isTabs)
				{
					//���������� ����, �� ����� ����������
#ifdef MSGLOGGER
					WCHAR szDbg[128]; _wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"Tabs:Notified(%i)\n", (DWORD)wParam);
					DEBUGSTRTABS(szDbg);
#endif
					TODO("����� ������ �� ������ OnTimer ������� �������� mn_TopProcessID")
					// ����� �� ����� ������� PID ���� ��� ����� ���� �� ��������...
					//gpConEmu->OnTimer(0,0); �� ����������. ������ ������� �� �������, ��-�� ����� ������ ��������� ���� ��� � ������� 0
					WARNING("gpConEmu->mp_TabBar->Retrieve() ������ ��� �� ������ ������");
					_ASSERTE(FALSE);
					gpConEmu->mp_TabBar->Retrieve();
				}
			}
			else if (messg == pVCon->mn_MsgPostFullPaint)
			{
				pVCon->Redraw();
			}
			else if (messg == pVCon->mn_MsgSavePaneSnapshoot)
			{
				pVCon->SavePaneSnapshoot();
			}
			else if (messg == pVCon->mn_MsgDetachPosted)
			{
				pVCon->RCon()->Detach(true, (lParam == 1));
			}
			#ifdef _DEBUG
			else if (messg == pVCon->mn_MsgCreateDbgDlg)
			{
				pVCon->CreateDbgDlg();
			}
			#endif
			else if (messg)
			{
				result = DefWindowProc(hWnd, messg, wParam, lParam);
			}
	}

wrap:
	return result;
}

void CConEmuChild::PostRestoreChildFocus()
{
	if (!gpConEmu->CanSetChildFocus())
	{
		_ASSERTE(FALSE && "Must not get here?");
		return;
	}

	PostMessage(mh_WndBack, mn_MsgRestoreChildFocus, 0, 0);
}

LRESULT CConEmuChild::BackWndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	// Logger
	MSG msgStr = {hWnd, messg, wParam, lParam};
	ConEmuMsgLogger::Log(msgStr, ConEmuMsgLogger::msgBack);

	if (gpSetCls->isAdvLogging >= 4)
	{
		gpConEmu->LogMessage(hWnd, messg, wParam, lParam);
	}

	CVConGuard guard;
	if (messg == WM_CREATE || messg == WM_NCCREATE)
	{
		LPCREATESTRUCT lp = (LPCREATESTRUCT)lParam;
		guard = (CVirtualConsole*)lp->lpCreateParams;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)guard.VCon());
	}
	else
	{
		guard = (CVirtualConsole*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	}

	CVirtualConsole* pVCon = guard.VCon();

	if (messg == WM_SYSCHAR)
	{
		_ASSERTE(FALSE); // �� ����, ������ ��� ���� �� ������
		// ����� �� ������
		result = TRUE;
		goto wrap;
	}

	if (!pVCon)
	{
		_ASSERTE(pVCon!=NULL);
		result = DefWindowProc(hWnd, messg, wParam, lParam);
		goto wrap;
	}

	switch (messg)
	{
	case WM_SHOWWINDOW:
			if (wParam)
			{
				HWND hView = pVCon->GetView();
				SetWindowPos(hView, HWND_TOP, 0, 0, 0,0, SWP_NOSIZE|SWP_NOMOVE);
				SetWindowPos(hWnd, hView, 0, 0, 0,0, SWP_NOSIZE|SWP_NOMOVE);
			}
			break; // DefaultProc
		case WM_SETFOCUS:
			// ���� � ������� �������� "GUI" ���� (GUI �����), �� ����� ����� ������ ����.
			{
				// ����� ������ ���� � ������� ����! �� ����������� ������ ������ � GUI ������.
				pVCon->setFocus();
			}
			return 0;
		case WM_ERASEBKGND:
			result = 0;
			break;
		case WM_PAINT:
			{
				PAINTSTRUCT ps = {};
				BeginPaint(hWnd, &ps);

				int nAppId = -1;
				int nColorIdx = RELEASEDEBUGTEST(0/*Black*/,1/*Blue*/);
				CRealConsole* pRCon = pVCon->RCon();
				if (pRCon)
				{
					nAppId = pRCon->GetActiveAppSettingsId();
					nColorIdx = pRCon->GetDefaultBackColorIdx();
				}

				HBRUSH hBrush = CreateSolidBrush(gpSet->GetColors(nAppId, !gpConEmu->isMeForeground())[nColorIdx]);
				if (hBrush)
				{
					FillRect(ps.hdc, &ps.rcPaint, hBrush);

					DeleteObject(hBrush);
				}

				EndPaint(hWnd, &ps);
			} // WM_PAINT
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
				switch (messg)
				{
					case WM_VSCROLL:
						switch (LOWORD(wParam))
						{
						case SB_THUMBTRACK:
						case SB_THUMBPOSITION:
							pVCon->mb_VTracking = TRUE;
							break;
						case SB_ENDSCROLL:
							pVCon->mb_VTracking = FALSE;
							break;
						}
						pVCon->RCon()->OnSetScrollPos(wParam);
						break;

					case WM_LBUTTONUP:
						pVCon->mb_VTracking = FALSE;
						break;
				}

				TODO("��������� ghWndWork");
				HWND hParent = ghWnd;
				_ASSERTE(GetParent(hWnd)==ghWnd);

				if (messg >= WM_MOUSEFIRST && messg <= WM_MOUSELAST)
				{
					POINT pt = {LOWORD(lParam),HIWORD(lParam)};
					MapWindowPoints(hWnd, hParent, &pt, 1);
					lParam = MAKELONG(pt.x,pt.y);
				}

				result = gpConEmu->WndProc(hParent, messg, wParam, lParam);
			}
			break;
		case WM_IME_NOTIFY:
			break;
		case WM_INPUTLANGCHANGE:
		case WM_INPUTLANGCHANGEREQUEST:
			{
				#ifdef _DEBUG
				if (IsDebuggerPresent())
				{
					WCHAR szMsg[128];
					_wsprintf(szMsg, SKIPLEN(countof(szMsg)) L"InChild %s(CP:%i, HKL:0x%08X)\n",
							  (messg == WM_INPUTLANGCHANGE) ? L"WM_INPUTLANGCHANGE" : L"WM_INPUTLANGCHANGEREQUEST",
							  (DWORD)wParam, (DWORD)lParam);
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
			break;
#endif
		case WM_SETCURSOR:
			{
				gpConEmu->WndProc(hWnd, messg, wParam, lParam);

				//if (!result)
				//	result = DefWindowProc(hWnd, messg, wParam, lParam);
			}
			// If an application processes this message, it should return TRUE to halt further processing or FALSE to continue.
			break;
			
		case WM_SYSCOMMAND:
			// -- ������ �����������, ������
			result = DefWindowProc(hWnd, messg, wParam, lParam);
			//if (wParam >= SC_SIZE && wParam <= SC_CONTEXTHELP/*0xF180*/)
			//{
			//	// ��������� ��������/������������/� �.�. ���� ������� - ���������
			//	_ASSERTE(!(wParam >= SC_SIZE && wParam <= SC_CONTEXTHELP));
			//}
			//else
			//{
			//	// �� ����, ���� ������ ��������� ������ �� ������
			//	_ASSERTE(FALSE);
			//}
			break;

		case WM_GESTURENOTIFY:
		case WM_GESTURE:
			{
				gpConEmu->ProcessGestureMessage(hWnd, messg, wParam, lParam, result);
				break;
			} // case WM_GESTURE, WM_GESTURENOTIFY

		default:

			if (pVCon && (messg == pVCon->mn_MsgRestoreChildFocus))
			{
				if (!gpConEmu->CanSetChildFocus())
				{
					// ���� �� ������ ��������� ��������� ����
					//_ASSERTE(FALSE && "Must not get here?");
				}
				else
				{
					CRealConsole* pRCon = pVCon->RCon();
					if (gpConEmu->isActive(pVCon, false))
					{
						pRCon->GuiWndFocusRestore();
					}

					if (pRCon->GuiWnd())
					{
						pRCon->StoreGuiChildRect(NULL);
					}
				}
			}
			else
			{
				result = DefWindowProc(hWnd, messg, wParam, lParam);
			}
	}

wrap:
	return result;
}

#ifdef _DEBUG
void CConEmuChild::CreateDbgDlg()
{
	return;

	if (!hDlgTest)
	{
		if (!gpConEmu->isMainThread())
		{
			PostMessage(mh_WndDC, mn_MsgCreateDbgDlg, 0, (LPARAM)this);
		}
		else
		{
			hDlgTest = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_SPG_MAIN), mh_WndDC, DbgChildDlgProc);
			if (hDlgTest)
			{
				SetWindowPos(hDlgTest, NULL, 100,100, 0,0, SWP_NOSIZE|SWP_NOZORDER);
				ShowWindow(hDlgTest, SW_SHOW);
			}
		}
	}
}

INT_PTR CConEmuChild::DbgChildDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return 0;
}
#endif

LRESULT CConEmuChild::OnPaint()
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return 0;
	}
	LRESULT result = 0;
	BOOL lbSkipDraw = FALSE;
	//if (gbInPaint)
	//    break;
	//_ASSERTE(FALSE);

	//2009-09-28 ����� ��� (autotabs)
	if (mb_DisableRedraw)
		return 0;

	CVirtualConsole* pVCon = (CVirtualConsole*)this;
	_ASSERTE(pVCon!=NULL);
	CVConGuard guard(pVCon);

	mb_PostFullPaint = FALSE;

	if (gpSetCls->isAdvLogging>2)
		pVCon->RCon()->LogString("CConEmuChild::OnPaint", TRUE);

	gpSetCls->Performance(tPerfBlt, FALSE);

	if (gpConEmu->isPictureView())
	{
		// ���� PictureView ���������� �� �� ��� ���� - ���������� ������� ����� �������!
		RECT rcPic, rcVRect, rcCommon;
		GetWindowRect(gpConEmu->hPictureView, &rcPic);
		GetWindowRect(mh_WndDC, &rcVRect); // ��� ����� ������ ������ �� ��� ��������.
		//MapWindowPoints(mh_WndDC, NULL, (LPPOINT)&rcClient, 2);

		BOOL lbIntersect = IntersectRect(&rcCommon, &rcVRect, &rcPic);
		UNREFERENCED_PARAMETER(lbIntersect);

		// ������ �� ��������� ������������� PictureView
		MapWindowPoints(NULL, mh_WndDC, (LPPOINT)&rcPic, 2);
		ValidateRect(mh_WndDC, &rcPic);

		MapWindowPoints(NULL, mh_WndDC, (LPPOINT)&rcVRect, 2);

		//Get ClientRect(gpConEmu->hPictureView, &rcPic);
		//Get ClientRect(mh_WndDC, &rcClient);

		// ���� PicView �������� ��� (�����? 95%) ������� ����
		//if (rcPic.right>=rcClient.right)
		if ((rcPic.right * rcPic.bottom) >= (rcVRect.right * rcVRect.bottom * 95 / 100))
		{
			//_ASSERTE(FALSE);
			lbSkipDraw = TRUE;

			pVCon->CheckTransparent();

			// ���� "������ ������ ���� ����"?
			result = DefWindowProc(mh_WndDC, WM_PAINT, 0, 0);
		}
	}

	if (!lbSkipDraw)
	{
		if (mh_LastGuiChild)
		{
			if (!IsWindow(mh_LastGuiChild))
			{
				mh_LastGuiChild = NULL;
				Invalidate();
			}
		}
		else
		{
			mh_LastGuiChild = pVCon->RCon() ? pVCon->RCon()->GuiWnd() : NULL;
		}

		bool bRightClickingPaint = gpConEmu->isRightClickingPaint() && gpConEmu->isActive(pVCon);
		if (bRightClickingPaint)
		{
			// ������ ������ � "���������"
			gpConEmu->RightClickingPaint((HDC)INVALID_HANDLE_VALUE, pVCon);
		}

		PAINTSTRUCT ps;
		HDC hDc = BeginPaint(mh_WndDC, &ps);
		UNREFERENCED_PARAMETER(hDc);

		//RECT rcClient = pVCon->GetDcClientRect();
		pVCon->PaintVCon(ps.hdc);

		if (bRightClickingPaint)
		{
			// �������� ��������, ��� ������� ������, ���� ������ ���������
			gpConEmu->RightClickingPaint(pVCon->GetIntDC()/*ps.hdc*/, pVCon);
		}

		EndPaint(mh_WndDC, &ps);
	}

	//Validate();
	gpSetCls->Performance(tPerfBlt, TRUE);
	// ���� ������� ���� �������� - �������� ��������� ���������� � ��������
	gpConEmu->UpdateSizes();
	return result;
}

LRESULT CConEmuChild::OnSize(WPARAM wParam, LPARAM lParam)
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return 0;
	}

	LRESULT result = 0;

	#ifdef _DEBUG
	BOOL lbIsPicView = FALSE;
	RECT rcNewClient; ::GetClientRect(mh_WndDC, &rcNewClient);
	RECT rcNewWnd;    ::GetWindowRect(mh_WndDC, &rcNewWnd);
	#endif

	if (gpSetCls->isAdvLogging)
	{
		char szInfo[128];
		_wsprintfA(szInfo, SKIPLEN(countof(szInfo)) "VCon(0x%08X).OnSize(%ux%u)", (DWORD)mh_WndDC, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
		gpConEmu->LogString(szInfo);
	}

	// ����� ��� � �� �����. �� ��� Ansi �� ��� Unicode ������ �������
	// ��� ����� � ConEmu �������� ������ �� ����� ��������� ������ PictureView
	//if (gpConEmu->isPictureView())
	//{
	//    if (gpConEmu->hPictureView) {
	//        lbIsPicView = TRUE;
	//        gpConEmu->isPiewUpdate = true;
	//        RECT rcClient; Get ClientRect('ghWnd DC', &rcClient);
	//        //TODO: � ���� PictureView ����� � � QuickView ��������������...
	//        MoveWindow(gpConEmu->hPictureView, 0,0,rcClient.right,rcClient.bottom, 1);
	//        //INVALIDATE(); //InvalidateRect(hWnd, NULL, FALSE);
	//		Invalidate();
	//        //SetFocus(hPictureView); -- ��� ����� �� ������ ������� ����� �������� ������...
	//    }
	//}
	return result;
}

LRESULT CConEmuChild::OnMove(WPARAM wParam, LPARAM lParam)
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return 0;
	}

	LRESULT result = 0;

	#ifdef _DEBUG
	RECT rcNewClient; ::GetClientRect(mh_WndDC, &rcNewClient);
	RECT rcNewWnd;    ::GetWindowRect(mh_WndDC, &rcNewWnd);
	#endif

	if (gpSetCls->isAdvLogging)
	{
		char szInfo[128];
		_wsprintfA(szInfo, SKIPLEN(countof(szInfo)) "VCon(0x%08X).OnMove(%ux%u)", (DWORD)mh_WndDC, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
		gpConEmu->LogString(szInfo);
	}

	return result;
}

void CConEmuChild::CheckPostRedraw()
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return;
	}
	// ���� ��� "����������" Redraw, ��
	if (mb_IsPendingRedraw && mn_LastPostRedrawTick && ((GetTickCount() - mn_LastPostRedrawTick) >= CON_REDRAW_TIMOUT))
	{
		mb_IsPendingRedraw = FALSE;
		_ASSERTE(gpConEmu->isMainThread());
		Redraw();
	}
}

void CConEmuChild::Redraw()
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return;
	}
	if (mb_DisableRedraw)
	{
		DEBUGSTRDRAW(L" +++ RedrawWindow on DC window will be ignored!\n");
		return;
	}

	if (!gpConEmu->isMainThread())
	{
		if (mb_RedrawPosted ||
		        (mn_LastPostRedrawTick && ((GetTickCount() - mn_LastPostRedrawTick) < CON_REDRAW_TIMOUT)))
		{
			mb_IsPendingRedraw = TRUE;
			return; // ��������� ������� ������ ����������
		}
		else
		{
			mn_LastPostRedrawTick = GetTickCount();
			mb_IsPendingRedraw = FALSE;
		}

		mb_RedrawPosted = TRUE; // ����� �� ���� ������������� �������
		PostMessage(mh_WndDC, mn_MsgPostFullPaint, 0, 0);
		return;
	}

	DEBUGSTRDRAW(L" +++ RedrawWindow on DC window called\n");
	//RECT rcClient; Get ClientRect(mh_WndDC, &rcClient);
	//MapWindowPoints(mh_WndDC, ghWnd, (LPPOINT)&rcClient, 2);
	InvalidateRect(mh_WndDC, NULL, FALSE);
	// ��-�� ����� - ��������� ������� �����������
	//gpConEmu->OnPaint(0,0);
	//#ifdef _DEBUG
	//BOOL lbRc =
	//#endif
	//RedrawWindow(ghWnd, NULL, NULL,
	//	RDW_INTERNALPAINT|RDW_NOERASE|RDW_UPDATENOW);
	mb_RedrawPosted = FALSE; // ����� ������ ���� ����� ������� ��� ����
}

// ���������� �� VConGroup::RepositionVCon
void CConEmuChild::SetVConSizePos(RECT arcBack, bool abReSize /*= true*/)
{
	CVirtualConsole* pVCon = (CVirtualConsole*)this;
	RECT rcBack = arcBack;
	TODO("��������������");
	//RECT rcCon = gpConEmu->CalcRect(CER_CONSOLE_CUR, arcBack, CER_BACK, pVCon);
	//RECT rcTmp = gpConEmu->CalcRect(CER_DC, rcCon, CER_CONSOLE_CUR, pVCon);
	RECT rcDC = gpConEmu->CalcRect(CER_DC, arcBack, CER_BACK, pVCon/*, &rcTmp*/);

	if (abReSize)
	{
		_ASSERTE((rcBack.right > rcBack.left) && (rcBack.bottom > rcBack.top));
		_ASSERTE((rcDC.right > rcDC.left) && (rcDC.bottom > rcDC.top));
		// �������/�������� ������ DC
		DEBUGTEST(RECT rc1; GetClientRect(mh_WndDC, &rc1););
		SetWindowPos(mh_WndDC, HWND_TOP, rcDC.left, rcDC.top, rcDC.right - rcDC.left, rcDC.bottom - rcDC.top, 0);
		DEBUGTEST(RECT rc2; GetClientRect(mh_WndDC, &rc2););
		SetWindowPos(mh_WndBack, mh_WndDC, rcBack.left, rcBack.top, rcBack.right - rcBack.left, rcBack.bottom - rcBack.top, 0);
		Invalidate();
	}
	else
	{
		// ������� ������ DC
		SetWindowPos(mh_WndDC, HWND_TOP, rcDC.left, rcDC.top, 0,0, SWP_NOSIZE);
		SetWindowPos(mh_WndBack, mh_WndDC, rcBack.left, rcBack.top, 0,0, SWP_NOSIZE);
	}

	// �������� ������ �������, ���� �� ����
	UpdateScrollRgn(true);
}

void CConEmuChild::SetRedraw(BOOL abRedrawEnabled)
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return;
	}
	mb_DisableRedraw = !abRedrawEnabled;
}

void CConEmuChild::Invalidate()
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return;
	}
	if (mb_DisableRedraw)
		return; // �����, ��� ��������� ���������� �����

	CVirtualConsole* pVCon = (CVirtualConsole*)this;

	//if (gpConEmu->isActive(pVCon))
	//	gpConEmu->mp_Status->UpdateStatusBar();


	// ����� ����� invalidate'��� � GAPS !!!
	// ����� ��� ������� "conemu.exe /max" - gaps �������� ��������, ��� �������� ������������� ��������

	//2009-06-22 ����� ������� ������������. ������ ���� �� ������ ������ �������� - ����������� ����������
	// ��� ��� ���� ����� ��� ���� ��������...
	//if (mb_Invalidated) {
	//	DEBUGSTRDRAW(L" ### Warning! Invalidate on DC window will be duplicated\n");
	////	return;
	//}
	if (mh_WndDC)
	{
		DEBUGSTRDRAW(L" +++ Invalidate on DC window called\n");
		
		if (!m_LockDc.bLocked)
		{
			InvalidateRect(mh_WndDC, NULL, FALSE);
		}
		else
		{
			RECT rcClient; GetClientRect(mh_WndDC, &rcClient);
			HRGN hRgn = CreateRectRgn(0, 0, rcClient.right, rcClient.bottom);
			HRGN hLock = CreateRectRgn(m_LockDc.rcScreen.left, m_LockDc.rcScreen.top, m_LockDc.rcScreen.right, m_LockDc.rcScreen.bottom);
			int iRc = CombineRgn(hRgn, hRgn, hLock, RGN_DIFF);
			
			if (iRc == ERROR)
				InvalidateRect(mh_WndDC, NULL, FALSE);
			else if (iRc != NULLREGION)
				InvalidateRgn(mh_WndDC, hRgn, FALSE);
			
			DeleteObject(hLock);
			DeleteObject(hRgn);
		}
	}
	else
	{
		_ASSERTE(mh_WndDC != NULL);
	}

	if (mh_WndBack)
	{
		InvalidateRect(mh_WndBack, NULL, FALSE);
	}
	else
	{
		_ASSERTE(mh_WndBack!=NULL);
	}

	UNREFERENCED_PARAMETER(pVCon);
}

//void CConEmuChild::Validate()
//{
//	//mb_Invalidated = FALSE;
//	//DEBUGSTRDRAW(L" +++ Validate on DC window called\n");
//	//if ('ghWnd DC') ValidateRect(ghWnd, NULL);
//}

void CConEmuChild::OnAlwaysShowScrollbar(bool abSync /*= true*/)
{
	if (m_LastAlwaysShowScrollbar != gpSet->isAlwaysShowScrollbar)
	{
		CVirtualConsole* pVCon = (CVirtualConsole*)this;
		CVConGuard guard(pVCon);

		if (gpSet->isAlwaysShowScrollbar == 1)
			ShowScroll(TRUE);
		else if (!m_si.nMax)
			HideScroll(TRUE);
		else
			TrackMouse();

		if (abSync && pVCon->isVisible())
		{
			if (gpConEmu->WindowMode != wmNormal)
				pVCon->RCon()->SyncConsole2Window();
			else
				CVConGroup::SyncWindowToConsole(); // -- ������� ������, ������������
		}

		m_LastAlwaysShowScrollbar = gpSet->isAlwaysShowScrollbar;
	}
}

// ������ ������� TRUE, ���� ������� ���� �� ����� ���������� � �������
BOOL CConEmuChild::TrackMouse()
{
	_ASSERTE(this);
	BOOL lbCapture = FALSE; // �� ��������� - ���� �� �������������
	
	CVirtualConsole* pVCon = (CVirtualConsole*)this;
	CVConGuard guard(pVCon);

	#ifdef _DEBUG
	CRealConsole* pRCon = pVCon->RCon();
	BOOL lbBufferMode = pRCon->isBufferHeight() && !pRCon->GuiWnd();
	#endif

	BOOL lbOverVScroll = CheckMouseOverScroll();

	if (gpSet->isAlwaysShowScrollbar == 1)
	{
		// ���� ������ ��������� ������������ ������ - �� ��� � �� ��������
		if (!mb_ScrollVisible)
			ShowScroll(TRUE);
	}
	else if (lbOverVScroll)
	{
		if (!mb_Scroll2Visible)
		{
			mb_Scroll2Visible = TRUE;
			ShowScroll(FALSE/*abImmediate*/); // ���� gpSet->isAlwaysShowScrollbar==1 - ���� ����������
		}
		#ifndef SKIP_HIDE_TIMER
		else if (mb_ScrollVisible && (gpSet->isAlwaysShowScrollbar != 1) && !m_TScrollCheck.IsStarted())
		{
			m_TScrollCheck.Start();
		}
		#endif
	}
	else if (mb_Scroll2Visible || mb_ScrollVisible)
	{
		_ASSERTE(gpSet->isAlwaysShowScrollbar != 1);
		mb_Scroll2Visible = FALSE;
		// ��������� ������ �������
		HideScroll(FALSE/*abImmediate*/);
	}

	lbCapture = (lbOverVScroll && mb_ScrollVisible);
	return lbCapture;
}

BOOL CConEmuChild::CheckMouseOverScroll(bool abCheckVisible /*= false*/)
{
	if (abCheckVisible)
	{
		if (gpSet->isAlwaysShowScrollbar == 0)
		{
			return FALSE; // �� ������������ ������
		}
		else if ((gpSet->isAlwaysShowScrollbar != 1) // 1 -- ���������� ������
			&& !mb_ScrollVisible)
		{
			return FALSE; // �� ������������ ������
		}
	}

	BOOL lbOverVScroll = FALSE;

	CVirtualConsole* pVCon = (CVirtualConsole*)this;
	CVConGuard guard(pVCon);

	// ����� �� � ��������? ��� � this?
	CVConGuard VCon;
	CRealConsole* pRCon = (gpConEmu->GetActiveVCon(&VCon) >= 0) ? VCon->RCon() : NULL;

	if (pRCon)
	{
		BOOL lbBufferMode = pRCon->isBufferHeight() && !pRCon->GuiWnd();

		if (lbBufferMode)
		{
			// ���� ��������� ������ ������ � ���������
			if (mb_VTracking && !isPressed(VK_LBUTTON))
			{
				// ������� ������
				mb_VTracking = FALSE;
			}

			// ����� ������ �� ��������, ����� �� ����� ������
			if (mb_VTracking)
			{
				lbOverVScroll = TRUE;
			}
			else // ������ ��������, ���� ���� � ��� ����������� - �������� ���
			{
				POINT ptCur; RECT rcScroll, rcClient;
				GetCursorPos(&ptCur);
				GetWindowRect(mh_WndBack, &rcClient);
				//GetWindowRect(mh_WndScroll, &rcScroll);
				rcScroll = rcClient;
				rcScroll.left = rcScroll.right - GetSystemMetrics(SM_CXVSCROLL);

				if (PtInRect(&rcScroll, ptCur))
				{
					// ���� ��������� ��� ����� - �� ����� � ������� �� �������! ��� ��� ���������!
					if (mb_ScrollVisible)
						lbOverVScroll = TRUE;
					// ���� �� ��������� - �� ��������� ������ ��������� � ������� ���� ����
					//if (!gpSet->isSelectionModifierPressed())
					else if (!(isPressed(VK_SHIFT) || isPressed(VK_CONTROL) || isPressed(VK_MENU) || isPressed(VK_LBUTTON)))
						lbOverVScroll = TRUE;
				}
			}
		}
	}

	return lbOverVScroll;
}

BOOL CConEmuChild::CheckScrollAutoPopup()
{
	return mb_ScrollAutoPopup;
}

bool CConEmuChild::InScroll()
{
	if (mb_VTracking)
	{
		if (!isPressed(VK_LBUTTON))
			mb_VTracking = FALSE;
	}

	return (mb_VTracking != FALSE);
}

void CConEmuChild::SetScroll(BOOL abEnabled, int anTop, int anVisible, int anHeight)
{
	//int nCurPos = 0;
	//BOOL lbScrollRc = FALSE;
	//SCROLLINFO si;
	//ZeroMemory(&si, sizeof(si));
	m_si.cbSize = sizeof(m_si);
	m_si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE; // | SIF_TRACKPOS;
	m_si.nMin = 0;

	if (!abEnabled)
	{
		m_si.nPos = 0;
		m_si.nPage = 0;
		m_si.nMax = 0; //(gpSet->isAlwaysShowScrollbar == 1) ? 1 : 0;
		//m_si.fMask |= SIF_DISABLENOSCROLL;
	}
	else
	{
		m_si.nPos = anTop;
		m_si.nPage = anVisible - 1;
		m_si.nMax = anHeight;
	}

	//// ���� ����� "BufferHeight" ������� - �������� �� ����������� ���� ������� ��������� ������ ���������
	//if (con.bBufferHeight) {
	//    lbScrollRc = GetScrollInfo(hConWnd, SB_VERT, &si);
	//} else {
	//    // ���������� ��������� ���, ����� ������ �� ������������ (��� �� 0)
	//}
	//TODO("����� ��� ������������� '�������' ������ ���������");
	//nCurPos = SetScrollInfo(mh_WndDC/*mh_WndScroll*/, SB_VERT, &m_si, true);

	if (!abEnabled)
	{
		mb_ScrollAutoPopup = FALSE;

		if (gpSet->isAlwaysShowScrollbar == 1)
		{
			if (!mb_ScrollVisible)
			{
				ShowScroll(TRUE);
			}
			else
			{
				MySetScrollInfo(TRUE, FALSE);
				//SetScrollInfo(mh_WndDC/*mh_WndScroll*/, SB_VERT, &m_si, TRUE);
				//if (!mb_ScrollDisabled)
				//{
				//	EnableScrollBar(mh_WndDC/*mh_WndScroll*/, SB_VERT, ESB_DISABLE_BOTH);
				//	mb_ScrollDisabled = TRUE;
				//}
			}
		}
		else
		{
			HideScroll(TRUE/*�����!*/);
		}
	}
	else
	{
		//if (!IsWindowEnabled(mh_WndScroll))
		//{
		//	//EnableWindow(mh_WndScroll, TRUE);
		//	EnableScrollBar(mh_WndDC/*mh_WndScroll*/, SB_VERT, ESB_ENABLE_BOTH);
		//}

		// �������� ���������, ���� �������� ����� ���������� � ����������
		if ((m_si.nPos > 0) && (m_si.nPos < (m_si.nMax - (int)m_si.nPage - 1)) && gpSet->isAlwaysShowScrollbar)
		{
			mb_ScrollAutoPopup = (gpSet->isAlwaysShowScrollbar == 2);

			if (!mb_Scroll2Visible)
			{
				ShowScroll((gpSet->isAlwaysShowScrollbar == 1));
			}
		}
		else
		{
			mb_ScrollAutoPopup = FALSE;
		}

		if (mb_ScrollVisible)
		{
			MySetScrollInfo(TRUE, TRUE);
			//SetScrollInfo(mh_WndDC/*mh_WndScroll*/, SB_VERT, &m_si, TRUE);
			//if (mb_ScrollDisabled)
			//{
			//	EnableScrollBar(mh_WndDC/*mh_WndScroll*/, SB_VERT, ESB_ENABLE_BOTH);
			//	mb_ScrollDisabled = FALSE;
			//}
		}
	}
}

void CConEmuChild::MySetScrollInfo(BOOL abSetEnabled, BOOL abEnableValue)
{
	SCROLLINFO si = m_si;

	if (/*!mb_ScrollVisible &&*/ !m_si.nMax && (gpSet->isAlwaysShowScrollbar == 1))
	{
		ShowScrollBar(mh_WndBack, SB_VERT, TRUE);
		// ��������� ������ ������������! �������� ������!
		si.nPage = 1;
		si.nMax = 100;
	}

	si.fMask |= SIF_PAGE|SIF_POS|SIF_RANGE/*|SIF_DISABLENOSCROLL*/;

	SetScrollInfo(mh_WndBack, SB_VERT, &si, TRUE);

	if (abSetEnabled)
	{
		if (abEnableValue)
		{
			EnableScrollBar(mh_WndBack/*mh_WndScroll*/, SB_VERT, ESB_ENABLE_BOTH);
			mb_ScrollDisabled = FALSE;
		}
		else
		{
			EnableScrollBar(mh_WndBack/*mh_WndScroll*/, SB_VERT, ESB_DISABLE_BOTH);
			mb_ScrollDisabled = TRUE;
		}
	}
}

void CConEmuChild::UpdateScrollRgn(bool abForce /*= false*/)
{
	bool bNeedRgn = (mb_ScrollVisible && (gpSet->isAlwaysShowScrollbar == 2));

	if ((bNeedRgn == mb_ScrollRgnWasSet) && !abForce)
		return;

	HRGN hRgn = NULL;
	if (mb_ScrollVisible && (gpSet->isAlwaysShowScrollbar == 2))
	{
		RECT rcDc = {}; GetClientRect(mh_WndDC, &rcDc);
		RECT rcScroll = {}; GetWindowRect(mh_WndBack, &rcScroll);
		MapWindowPoints(NULL, mh_WndDC, (LPPOINT)&rcScroll, 2);
		rcScroll.left = rcScroll.right - GetSystemMetrics(SM_CXVSCROLL);
		TODO("Horizontal scrolling");
		hRgn = CreateRectRgn(rcDc.left, rcDc.top, rcDc.right, rcDc.bottom);
		HRGN hScrlRgn = CreateRectRgn(rcScroll.left, rcScroll.top, rcScroll.right, rcScroll.bottom);
		int iRc = CombineRgn(hRgn, hRgn, hScrlRgn, RGN_DIFF);
		DeleteObject(hScrlRgn);
		UNREFERENCED_PARAMETER(iRc);
	}

	SetWindowRgn(mh_WndDC, hRgn, TRUE);

	mb_ScrollRgnWasSet = (hRgn != NULL);

	if (hRgn)
	{
		InvalidateRect(mh_WndBack, NULL, FALSE);
	}
}

void CConEmuChild::ShowScroll(BOOL abImmediate)
{
	bool bTShow = false, bTCheck = false;

	if (abImmediate || (gpSet->isAlwaysShowScrollbar == 1))
	{
		if (!mb_ScrollVisible && !m_si.nMax)
		{
			// ��������� ������ ������������! �������� ������!
			//m_si.nMax = (gpSet->isAlwaysShowScrollbar == 1) ? 1 : 0;
			SCROLLINFO si = {sizeof(si), SIF_PAGE|SIF_POS|SIF_RANGE/*|SIF_DISABLENOSCROLL*/, 0, 100, 1};
			SetScrollInfo(mh_WndBack, SB_VERT, &si, TRUE);
		}

		mb_ScrollVisible = TRUE; mb_Scroll2Visible = TRUE;

		#ifdef _DEBUG
		SCROLLINFO si = {sizeof(si), SIF_PAGE|SIF_POS|SIF_RANGE};
		GetScrollInfo(mh_WndBack, SB_VERT, &si);
		#endif

		//int nCurPos = -1;
		if (m_si.nMax || (gpSet->isAlwaysShowScrollbar == 1))
		{
			MySetScrollInfo(FALSE, FALSE);
			//nCurPos = SetScrollInfo(mh_WndDC/*mh_WndScroll*/, SB_VERT, &m_si, TRUE); UNREFERENCED_PARAMETER(nCurPos);
		}

		if (mb_ScrollDisabled && m_si.nMax > 1)
		{
			EnableScrollBar(mh_WndBack/*mh_WndScroll*/, SB_VERT, ESB_ENABLE_BOTH);
			mb_ScrollDisabled = FALSE;
		}
		else if (!mb_ScrollDisabled && !m_si.nMax)
		{
			EnableScrollBar(mh_WndBack/*mh_WndScroll*/, SB_VERT, ESB_DISABLE_BOTH);
			mb_ScrollDisabled = TRUE;
		}

		ShowScrollBar(mh_WndBack, SB_VERT, TRUE);

		#ifdef _DEBUG
		GetScrollInfo(mh_WndBack, SB_VERT, &si);
		#endif

		TODO("Scroll: Horizontal");

		#ifdef _DEBUG
		DWORD dwStyle = GetWindowLong(mh_WndBack, GWL_STYLE);
		//if (!(dwStyle & WS_VSCROLL))
		//	SetWindowLong(mh_WndDC, GWL_STYLE, dwStyle | WS_VSCROLL);
		#endif

		//if (!IsWindowVisible(mh_WndScroll))
		//	apiShowWindow(mh_WndScroll, SW_SHOWNOACTIVATE);

		//SetWindowPos(mh_WndScroll, HWND_TOP, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW);

		if ((gpSet->isAlwaysShowScrollbar != 1))
		{
			bTCheck = true;
		}
	}
	else
	{
		mb_Scroll2Visible = TRUE;
		bTShow = true;
	}

	UpdateScrollRgn();

	#ifndef SKIP_HIDE_TIMER
	if (bTCheck)
	{
		m_TScrollCheck.Start(mb_ScrollAutoPopup ? TIMER_SCROLL_CHECK_DELAY2 : TIMER_SCROLL_CHECK_DELAY);
	}
	else if (m_TScrollCheck.IsStarted())
	{
		m_TScrollCheck.Stop();
	}
	#endif

	//
	if (bTShow)
		m_TScrollShow.Start(mb_ScrollAutoPopup ? TIMER_SCROLL_SHOW_DELAY2 : TIMER_SCROLL_SHOW_DELAY);
	else if (m_TScrollShow.IsStarted())
		m_TScrollShow.Stop();

	//
	if (m_TScrollHide.IsStarted())
		m_TScrollHide.Stop();
}

void CConEmuChild::HideScroll(BOOL abImmediate)
{
	bool bTHide = false;
	mb_ScrollAutoPopup = FALSE;

	if (gpSet->isAlwaysShowScrollbar == 1)
	{
		// ��������� ������ ������������! �������� ������!
		SCROLLINFO si = {sizeof(si), SIF_PAGE|SIF_POS|SIF_RANGE/*|SIF_DISABLENOSCROLL*/, 0, 100, 1};
		SetScrollInfo(mh_WndBack, SB_VERT, &si, TRUE);
		if (!mb_ScrollDisabled)
		{
			EnableScrollBar(mh_WndBack/*mh_WndScroll*/, SB_VERT, ESB_DISABLE_BOTH);
			mb_ScrollDisabled = TRUE;
		}
		Invalidate();
	}
	else if (abImmediate)
	{
		mb_ScrollVisible = FALSE;
		mb_Scroll2Visible = FALSE;

		SCROLLINFO si = {sizeof(si), SIF_PAGE|SIF_POS|SIF_RANGE};
		int nCurPos = SetScrollInfo(mh_WndBack, SB_VERT, &si, TRUE);  UNREFERENCED_PARAMETER(nCurPos);

		TODO("Scroll: Horizontal");
		#ifdef _DEBUG
		DWORD dwStyle = GetWindowLong(mh_WndBack, GWL_STYLE);
		//if (dwStyle & WS_VSCROLL)
		//	SetWindowLong(mh_WndBack, GWL_STYLE, dwStyle & ~WS_VSCROLL);
		#endif

		Invalidate();

		//if (IsWindowVisible(mh_WndScroll))
		//	apiShowWindow(mh_WndScroll, SW_HIDE);
		//RECT rcScroll; GetWindowRect(mh_WndScroll, &rcScroll);
		//// ������� ���������, ����������� ����� �������� ����
		//MapWindowPoints(NULL, ghWnd, (LPPOINT)&rcScroll, 2);
		//InvalidateRect(ghWnd, &rcScroll, FALSE);
	}
	else
	{
		mb_Scroll2Visible = FALSE;
		bTHide = true;
		//m_TScrollHide.Start(TIMER_SCROLL_HIDE_DELAY);
	}

	UpdateScrollRgn();

	#ifndef SKIP_HIDE_TIMER
	if (m_TScrollCheck.IsStarted())
		m_TScrollCheck.Stop();
	#endif

	//
	if (m_TScrollShow.IsStarted())
		m_TScrollShow.Stop();

	//
	if (bTHide)
		m_TScrollHide.Start(TIMER_SCROLL_HIDE_DELAY);
	else if (m_TScrollHide.IsStarted())
		m_TScrollHide.Stop();
}

// 0 - ���������� ���
// 1 - �������������, ������������ ���������� ������� ��������� (������� ���������� �������)
// 2 - �������������, ��� ����� ���� � ���� ������������� - ������� ����������
int CConEmuChild::IsDcLocked(RECT* CurrentConLockedRect)
{
	if (!m_LockDc.bLocked)
		return 0;
	
	if (CurrentConLockedRect)
		*CurrentConLockedRect = m_LockDc.rcCon;
	_ASSERTE(!(m_LockDc.rcCon.left < 0 || m_LockDc.rcCon.top < 0 || m_LockDc.rcCon.right < 0 || m_LockDc.rcCon.bottom < 0));

	DWORD nDelta = GetTickCount() - m_LockDc.nLockTick;

	return (nDelta <= LOCK_DC_RECT_TIMEOUT) ? 1 : 2;
}

void CConEmuChild::LockDcRect(bool bLock, RECT* Rect)
{
	if (!bLock)
	{
		if (m_LockDc.bLocked)
		{
			m_LockDc.bLocked = FALSE;
			Invalidate();
		}
	}
	else
	{
		if (!Rect)
		{
			_ASSERTE(Rect!=NULL);
			return;
		}

		CVirtualConsole* pVCon = (CVirtualConsole*)this;
		CVConGuard guard(pVCon);

		TODO("������ �� ����� ��������� � CompatibleBitmap ��, ��� ����������. ��� ����� ������ � MainThread!!");
		
		m_LockDc.rcScreen = *Rect;
		COORD cr = pVCon->ClientToConsole(m_LockDc.rcScreen.left+1, m_LockDc.rcScreen.top+1, true);
		m_LockDc.rcCon.left = cr.X; m_LockDc.rcCon.top = cr.Y;
		cr = pVCon->ClientToConsole(m_LockDc.rcScreen.right-1, m_LockDc.rcScreen.bottom-1, true);
		m_LockDc.rcCon.right = cr.X; m_LockDc.rcCon.bottom = cr.Y;

		if (m_LockDc.rcCon.left < 0 || m_LockDc.rcCon.top < 0 || m_LockDc.rcCon.right < 0 || m_LockDc.rcCon.bottom < 0)
		{
			_ASSERTE(!(m_LockDc.rcCon.left < 0 || m_LockDc.rcCon.top < 0 || m_LockDc.rcCon.right < 0 || m_LockDc.rcCon.bottom < 0));
			LockDcRect(false);
			return;
		}
		
		m_LockDc.nLockTick = GetTickCount();
		m_LockDc.bLocked = TRUE;
		ValidateRect(mh_WndDC, &m_LockDc.rcScreen);
	}
}

void CConEmuChild::SetAutoCopyTimer(bool bEnabled)
{
	if (bEnabled)
	{
		m_TAutoCopy.Stop();
		m_TAutoCopy.Start(TIMER_AUTOCOPY_DELAY);
	}
	else
	{
		m_TAutoCopy.Stop();
	}
}

void CConEmuChild::PostDetach(bool bSendCloseConsole /*= false*/)
{
	PostMessage(mh_WndDC, mn_MsgDetachPosted, 0, (LPARAM)bSendCloseConsole);
}
