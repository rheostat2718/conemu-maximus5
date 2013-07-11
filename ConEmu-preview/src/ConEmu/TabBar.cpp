
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

#define SHOWDEBUGSTR

#ifdef _DEBUG
#include "TabBarEx.h"
#endif

#if defined(CONEMU_TABBAR_EX)
#include "TabBarEx.cpp"
#else


#define DEBUGSTRTABS(s) //DEBUGSTR(s)

#include <windows.h>
#include <commctrl.h>
#include "header.h"
#include "TabBar.h"
#include "Options.h"
#include "ConEmu.h"
#include "VirtualConsole.h"
#include "TrayIcon.h"
#include "VConChild.h"
#include "VConGroup.h"
#include "Status.h"
#include "Menu.h"

WARNING("!!! ��������� far, ������� edit, ������� � ������, ������� ������ edit, ESC, �� ���� ������� �� �������");
// ����� ����, ���� ���� ��� ���� ������� - �������� ������ ������ ������� ��������� ���������� �������
WARNING("�� �������� ���� ��� ������������ �� ������ �������");

TODO("��� WinXP ����� ���������� ������ WS_EX_COMPOSITED");

WARNING("isTabFrame �� ��������");

//TabBarClass TabBar;
//const int TAB_FONT_HEIGTH = 16;
//wchar_t TAB_FONT_FACE[] = L"Tahoma";
WNDPROC TabBarClass::_defaultTabProc = NULL;
WNDPROC TabBarClass::_defaultToolProc = NULL;
WNDPROC TabBarClass::_defaultReBarProc = NULL;
typedef BOOL (WINAPI* FAppThemed)();

#ifndef TBN_GETINFOTIP
#define TBN_GETINFOTIP TBN_GETINFOTIPW
#endif

#ifndef RB_SETWINDOWTHEME
#define CCM_SETWINDOWTHEME      (CCM_FIRST + 0xb)
#define RB_SETWINDOWTHEME       CCM_SETWINDOWTHEME
#endif

WARNING("TB_GETIDEALSIZE - awailable on XP only, use insted TB_GETMAXSIZE");
#ifndef TB_GETIDEALSIZE
#define TB_GETIDEALSIZE         (WM_USER + 99)
#endif

//enum ToolbarMainBitmapIdx
//{
//	BID_FIST_CON = 0,
//	BID_LAST_CON = (MAX_CONSOLE_COUNT-1),
//	BID_NEWCON_IDX,
//	BID_ALTERNATIVE_IDX,
//	BID_MINIMIZE_IDX,
//	BID_MAXIMIZE_IDX,
//	BID_RESTORE_IDX,
//	BID_APPCLOSE_IDX,
//	BID_DUMMYBTN_IDX,
//	BID_TOOLBAR_LAST_IDX,
//};

#define POST_UPDATE_TIMEOUT 2000

#define FAILED_TABBAR_TIMERID 101
#define FAILED_TABBAR_TIMEOUT 3000

#define ACTIVATE_TAB_CRITICAL 300

//typedef long (WINAPI* ThemeFunction_t)();

TabBarClass::TabBarClass()
{
	_active = false;
	_visible = false;
	_tabHeight = 0;
	mb_ForceRecalcHeight = false;
	mb_DisableRedraw = FALSE;
	//memset(&m_Margins, 0, sizeof(m_Margins));
	//m_Margins = gpSet->rcTabMargins; // !! �������� ��������
	_titleShouldChange = false;
	_prevTab = -1;
	mb_ChangeAllowed = FALSE;
	//mb_Enabled = TRUE;
	mh_Toolbar = NULL; mh_Tabbar = NULL; mh_Rebar = NULL; mh_TabIcons = NULL; mn_LastToolbarWidth = 0;
	mn_AdminIcon = 0;
	mb_PostUpdateCalled = FALSE;
	mb_PostUpdateRequested = FALSE;
	mn_PostUpdateTick = 0;
	//mn_MsgUpdateTabs = RegisterWindowMessage(CONEMUMSG_UPDATETABS);
	memset(&m_Tab4Tip, 0, sizeof(m_Tab4Tip));
	mb_InKeySwitching = FALSE;
	ms_TmpTabText[0] = 0;
	mn_CurSelTab = 0;
	mn_ThemeHeightDiff = 0;
	mn_InUpdate = 0;
	//mb_ThemingEnabled = FALSE;
	mh_TabTip = mh_Balloon = NULL; ms_TabErrText[0] = 0; memset(&tiBalloon,0,sizeof(tiBalloon));
	//mb_InNewConPopup = mb_InNewConRPopup = false;
	//mn_FirstTaskID = mn_LastTaskID = 0;
	mh_TabFont = NULL;
}

TabBarClass::~TabBarClass()
{
	if (mh_TabFont)
	{
		DeleteObject(mh_TabFont);
		mh_TabFont = NULL;
	}
}

//void TabBarClass::CheckTheming()
//{
//	static bool bChecked = false;
//	if (bChecked) return;
//	bChecked = true;
//
//	if (gOSVer.dwMajorVersion >= 6 || (gOSVer.dwMajorVersion == 5 && gOSVer.dwMinorVersion >= 1)) {
//		ThemeFunction_t fIsAppThemed = NULL;
//		ThemeFunction_t fIsThemeActive = NULL;
//		HMODULE hUxTheme = GetModuleHandle ( L"UxTheme.dll" );
//		if (hUxTheme)
//		{
//			fIsAppThemed = (ThemeFunction_t)GetProcAddress(hUxTheme, "IsAppThemed");
//			fIsThemeActive = (ThemeFunction_t)GetProcAddress(hUxTheme, "IsThemeActive");
//			if (fIsAppThemed && fIsThemeActive)
//			{
//				long llThemed = fIsAppThemed();
//				long llActive = fIsThemeActive();
//				if (llThemed && llActive)
//					mb_ThemingEnabled = TRUE;
//			}
//		}
//	}
//}

//void TabBarClass::Enable(BOOL abEnabled)
//{
//    if (mh_Tabbar && mb_Enabled!=abEnabled)
//    {
//        //EnableWindow(mh_Tabbar, abEnabled);
//        mb_Enabled = abEnabled;
//    }
//}

void TabBarClass::RePaint()
{
	if (!mh_Rebar)
		return;

	RECT client, self;
	client = gpConEmu->GetGuiClientRect();
	GetWindowRect(mh_Rebar, &self);
	MapWindowPoints(NULL, ghWnd, (LPPOINT)&self, 2);

	int nNewY;
	if (gpSet->nTabsLocation == 1)
	{
		int nStatusHeight = gpSet->isStatusBarShow ? gpSet->StatusBarHeight() : 0;
		nNewY = client.bottom-nStatusHeight-(self.bottom-self.top);
	}
	else
	{
		nNewY = 0;
	}

	if ((client.right != (self.right - self.left))
		|| (nNewY != self.top))
	{
		Reposition();

		#if 0
		MoveWindow(mh_Rebar, 0, nNewY, client.right, self.bottom-self.top, 1);

		if (gpSet->nTabsLocation == 1)
			m_Margins = MakeRect(0,0,0,_tabHeight);
		else
			m_Margins = MakeRect(0,_tabHeight,0,0);
		gpSet->UpdateMargins(m_Margins);
		#endif
	}

	UpdateWindow(mh_Rebar);
}

//void TabBarClass::Refresh(BOOL abFarActive)
//{
//    Enable(abFarActive);
//}

void TabBarClass::Reset()
{
	if (!_active)
	{
		return;
	}

	/*ConEmuTab tab; memset(&tab, 0, sizeof(tab));
	tab.Pos=0;
	tab.Current=1;
	tab.Type = 1;*/
	//gpConEmu->mp_TabBar->Update(&tab, 1);
	Update();
}

void TabBarClass::Retrieve()
{
	if (gpSet->isTabs == 0)
		return; // ���� ����� ��� ������ - � ������ ������ �� �����

	if (!CVConGroup::isFar())
	{
		Reset();
		return;
	}

	TODO("Retrieve() ����� ����� ��������� � RCon?");
	//CConEmuPipe pipe;
	//if (pipe.Init(_T("TabBarClass::Retrieve"), TRUE))
	//{
	//  DWORD cbWritten=0;
	//  if (pipe.Execute(CMD_REQTABS))
	//  {
	//      gpConEmu->DebugStep(_T("Tabs: Checking for plugin (1 sec)"));
	//      // �������� ��������, �������� ��� ������ �����
	//      cbWritten = WaitForSingleObject(pipe.hEventAlive, CONEMUALIVETIMEOUT);
	//      if (cbWritten!=WAIT_OBJECT_0) {
	//          TCHAR szErr[MAX_PATH];
	//          _wsprintf(szErr, countof(szErr), _T("ConEmu plugin is not active!\r\nProcessID=%i"), pipe.nPID);
	//          MBoxA(szErr);
	//      } else {
	//          gpConEmu->DebugStep(_T("Tabs: Waiting for result (10 sec)"));
	//          cbWritten = WaitForSingleObject(pipe.hEventReady, CONEMUREADYTIMEOUT);
	//          if (cbWritten!=WAIT_OBJECT_0) {
	//              TCHAR szErr[MAX_PATH];
	//              _wsprintf(szErr, countof(szErr), _T("Command waiting time exceeds!\r\nConEmu plugin is locked?\r\nProcessID=%i"), pipe.nPID);
	//              MBoxA(szErr);
	//          } else {
	//              gpConEmu->DebugStep(_T("Tabs: Recieving data"));
	//              DWORD cbBytesRead=0;
	//              int nTabCount=0;
	//              pipe.Read(&nTabCount, sizeof(nTabCount), &cbBytesRead);
	//              if (nTabCount<=0) {
	//                  gpConEmu->DebugStep(_T("Tabs: data empty"));
	//                  this->Reset();
	//              } else {
	//                  COPYDATASTRUCT cds = {0};
	//
	//                  cds.dwData = nTabCount;
	//                  cds.lpData = pipe.GetPtr(); // �����
	//                  gpConEmu->OnCopyData(&cds);
	//                  gpConEmu->DebugStep(NULL);
	//              }
	//          }
	//      }
	//  }
	//}
}

int TabBarClass::GetTabIcon(bool bAdmin)
{
	int iIconIdx = (bAdmin && gpSet->bAdminShield) ? mn_AdminIcon : -1;
	return iIconIdx;
}

// ��������� ��������, ��� ������ (��� �������������) ��������� ������������
void TabBarClass::AddTab(LPCWSTR text, int i, bool bAdmin)
{
	if (mh_Tabbar)
	{
		TCITEM tie;
		// ������ ��������� ������. ��� ����� ���������� ��� ����
		tie.mask = TCIF_TEXT | (mh_TabIcons ? TCIF_IMAGE : 0);
		tie.iImage = -1;
		tie.pszText = (LPWSTR)text ;
		tie.iImage = GetTabIcon(bAdmin); // ���� ������ ������ ���� - ��� ����� �� �����
		int nCurCount = GetItemCount();

		if (i>=nCurCount)
		{
			TabCtrl_InsertItem(mh_Tabbar, i, &tie);
		}
		else
		{
			// ��������, ��������� �� �����
			if (!wcscmp(GetTabText(i), text))
				tie.mask &= ~TCIF_TEXT;

			// ���������� �� ������
			if (tie.mask & TCIF_IMAGE)
			{
				TCITEM told;
				told.mask = TCIF_IMAGE;
				TabCtrl_GetItem(mh_Tabbar, i, &told);

				if (told.iImage == tie.iImage)
					tie.mask &= ~TCIF_IMAGE;
			}

			// "������" ������ ���� �� ������� ��������
			if (tie.mask)
				TabCtrl_SetItem(mh_Tabbar, i, &tie);
		}
	}
}

void TabBarClass::SelectTab(int i)
{
	mb_ChangeAllowed = TRUE;

	if (i != GetCurSel())    // ������ ���������, ������ ���� ��� ������� ��������
	{
		mn_CurSelTab = i;

		if (mh_Tabbar)
			TabCtrl_SetCurSel(mh_Tabbar, i);
	}

	mb_ChangeAllowed = FALSE;
}

int TabBarClass::GetCurSel()
{
	if (mh_Tabbar)
	{
		// ���� ���� ������� ���� - �������
		mn_CurSelTab = TabCtrl_GetCurSel(mh_Tabbar);
	}

	int nCurSel = mn_CurSelTab;
	return nCurSel;
}

int TabBarClass::GetItemCount()
{
	int nCurCount = 0;

	if (mh_Tabbar)
		nCurCount = TabCtrl_GetItemCount(mh_Tabbar);
	else
		nCurCount = m_Tab2VCon.size();

	return nCurCount;
}

void TabBarClass::DeleteItem(int I)
{
	if (!mh_Tabbar) return;

	TabCtrl_DeleteItem(mh_Tabbar, I);
}


/*char TabBarClass::FarTabShortcut(int tabIndex)
{
    return tabIndex < 10 ? '0' + tabIndex : 'A' + tabIndex - 10;
}*/

BOOL TabBarClass::NeedPostUpdate()
{
	return (mb_PostUpdateCalled || mb_PostUpdateRequested);
}

void TabBarClass::RequestPostUpdate()
{
	if (mb_PostUpdateCalled)
	{
		DWORD nDelta = GetTickCount() - mn_PostUpdateTick;

		// ����� ��� ����������, ��� ������ �������, � Post �� ��������
		if (nDelta <= POST_UPDATE_TIMEOUT)
			return; // ���
	}

	if (mn_InUpdate > 0)
	{
		mb_PostUpdateRequested = TRUE;
		DEBUGSTRTABS(L"   PostRequesting TabBarClass::Update\n");
	}
	else
	{
		mb_PostUpdateCalled = TRUE;
		DEBUGSTRTABS(L"   Posting TabBarClass::Update\n");
		//PostMessage(ghWnd, mn_MsgUpdateTabs, 0, 0);
		gpConEmu->RequestPostUpdateTabs();
		mn_PostUpdateTick = GetTickCount();
	}
}

BOOL TabBarClass::GetVConFromTab(int nTabIdx, CVirtualConsole** rpVCon, DWORD* rpWndIndex)
{
	BOOL lbRc = FALSE;
	CVirtualConsole *pVCon = NULL;
	DWORD wndIndex = 0;

	if ((nTabIdx >= 0) && (nTabIdx < m_Tab2VCon.size()))
	{
		pVCon = m_Tab2VCon[nTabIdx].pVCon;
		wndIndex = m_Tab2VCon[nTabIdx].nFarWindowId;

		if (!gpConEmu->isValid(pVCon))
		{
			RequestPostUpdate();
			//if (!mb_PostUpdateCalled)
			//{
			//    mb_PostUpdateCalled = TRUE;
			//    PostMessage(ghWnd, mn_Msg UpdateTabs, 0, 0);
			//}
		}
		else
		{
			lbRc = TRUE;
		}
	}

	if (rpVCon) *rpVCon = lbRc ? pVCon : NULL;

	if (rpWndIndex) *rpWndIndex = lbRc ? wndIndex : 0;

	return lbRc;
}

CVirtualConsole* TabBarClass::FarSendChangeTab(int tabIndex)
{
	CVirtualConsole *pVCon = NULL;
	DWORD wndIndex = 0;
	BOOL  bNeedActivate = FALSE, bChangeOk = FALSE;
	ShowTabError(NULL);

	if (!GetVConFromTab(tabIndex, &pVCon, &wndIndex))
	{
		if (mb_InKeySwitching) Update();  // �������� �������� ��������� ���

		return NULL;
	}

	if (!gpConEmu->isActive(pVCon, false))
		bNeedActivate = TRUE;

	DWORD nCallStart = TimeGetTime(), nCallEnd = 0;

	bChangeOk = pVCon->RCon()->ActivateFarWindow(wndIndex);

	if (!bChangeOk)
	{
		// ������� ������ � ������� - �� ������ ������������
		ShowTabError(L"This tab can't be activated now!", tabIndex);
	}
	else
	{
		nCallEnd = TimeGetTime();
		_ASSERTE((nCallEnd - nCallStart) < ACTIVATE_TAB_CRITICAL);
	}

	// ����� ������ �� �������� - ���������� �������
	// ������ ����� ����� ���� (�������� ��� ��������� - �������)
	TODO("�� �����������. ����� ������ �� ������� ���������� � ��������� � �������� ������ ���������� ������������ �������");

	if (bNeedActivate)
	{
		if (!gpConEmu->Activate(pVCon))
		{
			if (mb_InKeySwitching) Update();  // �������� �������� ��������� ���

			TODO("� ������� ��� �� ������, ���� ������������ �� �������?");
			return NULL;
		}
	}

	if (!bChangeOk)
	{
		pVCon = NULL;

		if (mb_InKeySwitching) Update();  // �������� �������� ��������� ���
	}

	UNREFERENCED_PARAMETER(nCallStart);
	UNREFERENCED_PARAMETER(nCallEnd);

	return pVCon;
}

LRESULT TabBarClass::TabHitTest(bool abForce /*= false*/)
{
	if (gpSet->isTabs && (abForce || gpSet->isCaptionHidden()))
	{
		if (gpConEmu->mp_TabBar->IsTabsShown())
		{
			TCHITTESTINFO tch = {{0,0}};
			HWND hTabBar = gpConEmu->mp_TabBar->mh_Tabbar;
			RECT rcWnd; GetWindowRect(hTabBar, &rcWnd);
			GetCursorPos(&tch.pt); // Screen coordinates

			if (PtInRect(&rcWnd, tch.pt))
			{
				tch.pt.x -= rcWnd.left; tch.pt.y -= rcWnd.top;
				LRESULT nTest = SendMessage(hTabBar, TCM_HITTEST, 0, (LPARAM)&tch);

				if (nTest == -1)
					return HTCAPTION;
			}
		}
	}

	return HTNOWHERE;
}

LRESULT CALLBACK TabBarClass::ReBarProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_WINDOWPOSCHANGING:
		{
			LPWINDOWPOS pos = (LPWINDOWPOS)lParam;
			if (gpConEmu->mp_TabBar->_tabHeight)
			{
				pos->cy = gpConEmu->mp_TabBar->_tabHeight;
				return 0;
			}
			break;
		}
		case WM_WINDOWPOSCHANGED:
		{
			#ifdef _DEBUG
			LPWINDOWPOS pos = (LPWINDOWPOS)lParam;
			#endif
			break;
		}
		case WM_SETFOCUS:
		{
			gpConEmu->setFocus();
			return 0;
		}
		case WM_SETCURSOR:

			if (gpSet->isTabs && !gpSet->isQuakeStyle
				&& (gpConEmu->GetWindowMode() == wmNormal)
				&& gpSet->isCaptionHidden())
			{
				if (TabHitTest() == HTCAPTION)
				{
					SetCursor(/*gpSet->isQuakeStyle ? gpConEmu->mh_CursorArrow :*/ gpConEmu->mh_CursorMove);
					return TRUE;
				}
			}

			break;
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_LBUTTONDBLCLK:
		/*case WM_RBUTTONDOWN:*/ case WM_RBUTTONUP: //case WM_RBUTTONDBLCLK:

			if (((uMsg == WM_RBUTTONUP)
					|| ((uMsg == WM_LBUTTONDBLCLK) && gpSet->nTabBarDblClickAction)
					|| gpSet->isCaptionHidden())
				&& gpSet->isTabs)
			{
				if (TabHitTest(true)==HTCAPTION)
				{
					POINT ptScr; GetCursorPos(&ptScr);
					lParam = MAKELONG(ptScr.x,ptScr.y);
					LRESULT lRc = 0;

					if (uMsg == WM_LBUTTONDBLCLK)
					{
						if ((gpSet->nTabBarDblClickAction == 2)
							|| ((gpSet->nTabBarDblClickAction == 1) && gpSet->isCaptionHidden()))
						{
							// ����� ���� �������� �� ���������� � �������
							gpConEmu->mouse.state |= MOUSE_SIZING_DBLCKL;
							// ������ AltF9
							gpConEmu->OnAltF9(TRUE);
						}
						else if ((gpSet->nTabBarDblClickAction == 3)
							|| ((gpSet->nTabBarDblClickAction == 1) && !gpSet->isCaptionHidden()))
						{
							gpConEmu->RecreateAction(cra_CreateTab/*FALSE*/, gpSet->isMultiNewConfirm || isPressed(VK_SHIFT));
						}
					}
					else if (uMsg == WM_RBUTTONUP)
					{
						gpConEmu->mp_Menu->ShowSysmenu(ptScr.x, ptScr.y/*-32000*/);
					}
					else if (gpConEmu->WindowMode == wmNormal)
					{
						lRc = gpConEmu->WndProc(ghWnd, uMsg-(WM_MOUSEMOVE-WM_NCMOUSEMOVE), HTCAPTION, lParam);
					}

					return lRc;
				}
			}

			break;

		case WM_MBUTTONUP:
			//if (uMsg == WM_MBUTTONUP)
			{
				gpConEmu->RecreateAction(cra_CreateTab/*FALSE*/, gpSet->isMultiNewConfirm || isPressed(VK_SHIFT));
			}
			break;
	}

	return CallWindowProc(_defaultReBarProc, hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK TabBarClass::TabProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_WINDOWPOSCHANGING:
		{
			if (gpConEmu->mp_TabBar->mh_Rebar)
			{
				LPWINDOWPOS pos = (LPWINDOWPOS)lParam;

				//if (gpConEmu->mp_TabBar->mb_ThemingEnabled) {
				if (gpSetCls->CheckTheming())
				{
					pos->y = 2; // ����� � Win7 �� ��������� � {0x0} � ����� ����� ���������� ������
					pos->cy = gpConEmu->mp_TabBar->_tabHeight -3; // �� ������ ������
				}
				else
				{
					pos->y = 1;
					pos->cy = gpConEmu->mp_TabBar->_tabHeight + 1;
				}

				return 0;
			}

			break;
		}
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		case WM_LBUTTONDBLCLK:
		{
			gpConEmu->mp_TabBar->OnMouse(uMsg, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		}
		case WM_SETFOCUS:
		{
			gpConEmu->setFocus();
			return 0;
		}
		case WM_TIMER:

			if (wParam == FAILED_TABBAR_TIMERID)
			{
				KillTimer(hwnd, wParam);
				SendMessage(gpConEmu->mp_TabBar->mh_Balloon, TTM_TRACKACTIVATE, FALSE, (LPARAM)&gpConEmu->mp_TabBar->tiBalloon);
				SendMessage(gpConEmu->mp_TabBar->mh_TabTip, TTM_ACTIVATE, TRUE, 0);
			};
	}

	return CallWindowProc(_defaultTabProc, hwnd, uMsg, wParam, lParam);
}

// Window procedure for Toolbar (Multiconsole & BufferHeight)
LRESULT CALLBACK TabBarClass::ToolProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_WINDOWPOSCHANGING:
		{
			LPWINDOWPOS pos = (LPWINDOWPOS)lParam;
			pos->y = (gpConEmu->mp_TabBar->mn_ThemeHeightDiff == 0) ? 2 : 1;

			if (gpSet->isHideCaptionAlways() && gpSet->nToolbarAddSpace > 0)
			{
				SIZE sz;

				if (CallWindowProc(_defaultToolProc, hwnd, TB_GETIDEALSIZE, 0, (LPARAM)&sz))
				{
					pos->cx = sz.cx + gpSet->nToolbarAddSpace;
					RECT rcParent; GetClientRect(GetParent(hwnd), &rcParent);
					pos->x = rcParent.right - pos->cx;
				}
			}

			return 0;
		}
		case TB_GETMAXSIZE:
		case TB_GETIDEALSIZE:
		{
			SIZE *psz = (SIZE*)lParam;

			if (!lParam) return 0;

			if (CallWindowProc(_defaultToolProc, hwnd, uMsg, wParam, lParam))
			{
				psz->cx += (gpSet->isHideCaptionAlways() ? gpSet->nToolbarAddSpace : 0);
				return 1;
			}
		} break;
		case WM_RBUTTONUP:
		{
			POINT pt = {(int)(short)LOWORD(lParam),(int)(short)HIWORD(lParam)};
			int nIdx = SendMessage(hwnd, TB_HITTEST, 0, (LPARAM)&pt);

			// If the return value is zero or a positive value, it is
			// the zero-based index of the nonseparator item in which the point lies.
			//if (nIdx >= 0 && nIdx < MAX_CONSOLE_COUNT)
			_ASSERTE(TID_ACTIVE_NUMBER==1);
			if (nIdx == (TID_ACTIVE_NUMBER-1))
			{
				CVConGuard VCon;
				CVirtualConsole* pVCon = (gpConEmu->GetActiveVCon(&VCon) >= 0) ? VCon.VCon() : NULL;

				if (!gpConEmu->isActive(pVCon, false))
				{
					if (!gpConEmu->ConActivate(nIdx))
					{
						if (!gpConEmu->isActive(pVCon, false))
						{
							return 0;
						}
					}
				}

				//ClientToScreen(hwnd, &pt);
				RECT rcBtnRect = {0};
				SendMessage(hwnd, TB_GETRECT, TID_ACTIVE_NUMBER, (LPARAM)&rcBtnRect);
				MapWindowPoints(hwnd, NULL, (LPPOINT)&rcBtnRect, 2);
				POINT pt = {rcBtnRect.right,rcBtnRect.bottom};

				gpConEmu->mp_Menu->ShowPopupMenu(pVCon, pt, TPM_RIGHTALIGN|TPM_TOPALIGN);
			}
			else
			{
				LRESULT nTestIdxMin = SendMessage(hwnd, TB_COMMANDTOINDEX, TID_MINIMIZE, 0);
				LRESULT nTestIdxClose = SendMessage(hwnd, TB_COMMANDTOINDEX, TID_APPCLOSE, 0);
				if ((nIdx == nTestIdxMin) || (nIdx == nTestIdxClose))
				{
					Icon.HideWindowToTray();
					return 0;
				}

				LRESULT nTestIdx = SendMessage(hwnd, TB_COMMANDTOINDEX, TID_CREATE_CON, 0);
				if (nIdx == nTestIdx)
				{
					gpConEmu->RecreateAction(cra_CreateTab/*FALSE*/, TRUE);
				}
			}

			return 0;
		}
	}

	return CallWindowProc(_defaultToolProc, hwnd, uMsg, wParam, lParam);
}


bool TabBarClass::IsTabsActive()
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return false;
	}
	return _active;
}

bool TabBarClass::IsTabsShown()
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return false;
	}
	return _active && _visible/*IsWindowVisible(mh_Tabbar)*/;
}

void TabBarClass::Activate(BOOL abPreSyncConsole/*=FALSE*/)
{
	if (!mh_Rebar)
	{
		CreateRebar();
		OnCaptionHidden();
	}

	_active = true;
	if (abPreSyncConsole && (gpConEmu->WindowMode == wmNormal))
	{
		RECT rcIdeal = gpConEmu->GetIdealRect();
		CVConGroup::SyncConsoleToWindow(&rcIdeal, TRUE);
	}
	UpdatePosition();
}

void TabBarClass::Deactivate(BOOL abPreSyncConsole/*=FALSE*/)
{
	if (!_active)
		return;

	_active = false;
	if (abPreSyncConsole && !(gpConEmu->isZoomed() || gpConEmu->isFullScreen()))
	{
		RECT rcIdeal = gpConEmu->GetIdealRect();
		CVConGroup::SyncConsoleToWindow(&rcIdeal, true);
	}
	UpdatePosition();
}

void TabBarClass::Update(BOOL abPosted/*=FALSE*/)
{
	#ifdef _DEBUG
	if (this != gpConEmu->mp_TabBar)
	{
		_ASSERTE(this == gpConEmu->mp_TabBar);
	}
	#endif

	MCHKHEAP
	/*if (!_active)
	{
	    return;
	}*/ // ������ - ������! �.�. ���� ��������� ��������������

	if (mb_DisableRedraw)
		return;

	if (!gpConEmu->isMainThread())
	{
		RequestPostUpdate();
		return;
	}

	gpConEmu->mp_Status->UpdateStatusBar();

	mb_PostUpdateCalled = FALSE;

	#ifdef _DEBUG
	_ASSERTE(mn_InUpdate >= 0);
	if (mn_InUpdate > 0)
	{
		_ASSERTE(mn_InUpdate == 0);
	}
	#endif

	mn_InUpdate ++;
	ConEmuTab tab = {0};
	MCHKHEAP
	int V, I, tabIdx = 0, nCurTab = -1, rFrom, rFound;
	BOOL bShowFarWindows = gpSet->bShowFarWindows;
	CVirtualConsole* pVCon = NULL;
	VConTabs vct = {NULL};
	// ����������� ������ ������ � �������� ����, ��� ��� CriticalSection �� �����
	m_Tab2VCon.clear();
	_ASSERTE(m_Tab2VCon.size()==0);

	#ifdef _DEBUG
	if (this != gpConEmu->mp_TabBar)
	{
		_ASSERTE(this == gpConEmu->mp_TabBar);
	}
	#endif

	TODO("��������� gpSet->bHideInactiveConsoleTabs ��� ����� �����");
	MCHKHEAP

	if (!gpConEmu->mp_TabBar->IsTabsActive() && gpSet->isTabs)
	{
		int nTabs = 0;

		for (V = 0; V < MAX_CONSOLE_COUNT && nTabs < 2; V++)
		{
			_ASSERTE(m_Tab2VCon.size()==0);

			if (!(pVCon = gpConEmu->GetVCon(V)))
				continue;

			if (gpSet->bHideInactiveConsoleTabs)
			{
				if (!gpConEmu->isActive(pVCon))
					continue;
			}

			_ASSERTE(m_Tab2VCon.size()==0);
			nTabs += pVCon->RCon()->GetTabCount(TRUE);
			_ASSERTE(m_Tab2VCon.size()==0);
		}

		if (nTabs > 1)
		{
			_ASSERTE(m_Tab2VCon.size()==0);
			Activate();
			_ASSERTE(m_Tab2VCon.size()==0);
		}
	}
	else if (gpConEmu->mp_TabBar->IsTabsActive() && gpSet->isTabs==2)
	{
		int nTabs = 0;

		for (V = 0; V < MAX_CONSOLE_COUNT && nTabs < 2; V++)
		{
			_ASSERTE(m_Tab2VCon.size()==0);

			if (!(pVCon = gpConEmu->GetVCon(V)))
				continue;

			if (gpSet->bHideInactiveConsoleTabs)
			{
				if (!gpConEmu->isActive(pVCon))
					continue;
			}

			_ASSERTE(m_Tab2VCon.size()==0);
			nTabs += pVCon->RCon()->GetTabCount(TRUE);
			_ASSERTE(m_Tab2VCon.size()==0);
		}

		if (nTabs <= 1)
		{
			_ASSERTE(m_Tab2VCon.size()==0);
			Deactivate();
			_ASSERTE(m_Tab2VCon.size()==0);
		}
	}

	#ifdef _DEBUG
	if (this != gpConEmu->mp_TabBar)
	{
		_ASSERTE(this == gpConEmu->mp_TabBar);
	}
	#endif

	MCHKHEAP
	_ASSERTE(m_Tab2VCon.size()==0);

	MMap<CVConGroup*,CVirtualConsole*> Groups; Groups.Init(MAX_CONSOLE_COUNT, true);

	for (V = 0; V < MAX_CONSOLE_COUNT; V++)
	{
		//if (!(pVCon = gpConEmu->GetVCon(V))) continue;
		CVConGuard guard;
		if (!CVConGroup::GetVCon(V, &guard))
			continue;
		pVCon = guard.VCon();

		BOOL lbActive = gpConEmu->isActive(pVCon, false);

		if (gpSet->bHideInactiveConsoleTabs)
		{
			if (!lbActive) continue;
		}

		if (gpSet->isOneTabPerGroup)
		{
			CVConGroup *pGr;
			CVConGuard VGrActive;
			if (CVConGroup::isGroup(pVCon, &pGr, &VGrActive))
			{
				CVirtualConsole* pGrVCon;

				if (Groups.Get(pGr, &pGrVCon))
					continue; // ��� ������ ��� ����

				pGrVCon = VGrActive.VCon();
				Groups.Set(pGr, pGrVCon);

				// � ���������� ��� ����� �� "��������" �������, � �� �� ������ � ������
				if (pVCon != pGrVCon)
				{
					guard = pGrVCon;
					pVCon = pGrVCon;
				}

				if (!lbActive)
				{
					lbActive = gpConEmu->isActive(pVCon, true);
				}
			}
		}

		CRealConsole *pRCon = pVCon->RCon();
		if (!pRCon)
		{
			_ASSERTE(pRCon!=NULL);
			continue;
		}

		// (Panels=1, Viewer=2, Editor=3) |(Elevated=0x100) |(NotElevated=0x200) |(Modal=0x400)
		bool bAllWindows = (bShowFarWindows && !(pRCon->GetActiveTabType() & fwt_Modal));
		rFrom = bAllWindows ? 0 : pRCon->GetActiveTab();
		rFound = 0;
		
		for (I = rFrom; bAllWindows || !rFound; I++)
		{
			#ifdef _DEBUG
				if (!I && !V)
				{
					_ASSERTE(m_Tab2VCon.size()==0);
				}
				if (this != gpConEmu->mp_TabBar)
				{
					_ASSERTE(this == gpConEmu->mp_TabBar);
				}
				MCHKHEAP;
			#endif
			

			// bShowFarWindows ���������, ����� �� ����������� � �������������� ������������� ����
			if (gpSet->bHideDisabledTabs && bShowFarWindows)
			{
				if (!pRCon->CanActivateFarWindow(I))
					continue;
			}

			if (!pRCon->GetTab(I, &tab))
				break;

			
			#ifdef _DEBUG
				if (this != gpConEmu->mp_TabBar)
				{
					_ASSERTE(this == gpConEmu->mp_TabBar);
				}
				MCHKHEAP;
			#endif
			
			
			PrepareTab(&tab, pVCon);
			vct.pVCon = pVCon;
			vct.nFarWindowId = I;

			
			#ifdef _DEBUG
				if (!I && !V)
				{
					_ASSERTE(m_Tab2VCon.size()==0);
				}
			#endif


			AddTab2VCon(vct);
			// ��������� ��������, ��� ������ (��� �������������) ��������� ������������
			AddTab(tab.Name, tabIdx, (tab.Type & fwt_Elevated)==fwt_Elevated);

			if (lbActive && tab.Current)
				nCurTab = tabIdx;

			rFound++;
			tabIdx++;

			
			#ifdef _DEBUG
				if (this != gpConEmu->mp_TabBar)
				{
					_ASSERTE(this == gpConEmu->mp_TabBar);
				}
			#endif
		}
	}

	MCHKHEAP

	if (tabIdx == 0)  // ���� �� "Console" �������
	{
		ZeroStruct(tab);
		PrepareTab(&tab, NULL);
		wcscpy_c(tab.Name, gpConEmu->GetDefaultTitle());
		vct.pVCon = NULL;
		vct.nFarWindowId = 0;
		AddTab2VCon(vct); //2009-06-14. �� ����!
		// ��������� ��������, ��� ������ (��� �������������) ��������� ������������
		AddTab(tab.Name, tabIdx, (tab.Type & fwt_Elevated)==fwt_Elevated);
		nCurTab = tabIdx;
		tabIdx++;
	}

	// Update ��������� ���������
	if ((nCurTab >= 0) && (nCurTab < m_Tab2VCon.size()))
		AddStack(m_Tab2VCon[nCurTab]);
	else
		CheckStack(); // ����� ������ �������� ����

	// ������� ������ �������� (���������)
	int nCurCount = GetItemCount();
	
	#ifdef _DEBUG
	wchar_t szDbg[128];
	_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"TabBarClass::Update.  ItemCount=%i, PrevItemCount=%i\n", tabIdx, nCurCount);
	DEBUGSTRTABS(szDbg);
	#endif

	for (I = tabIdx; I < nCurCount; I++)
	{
		#ifdef _DEBUG
		_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"   Deleting tab=%i\n", I+1);
		DEBUGSTRTABS(szDbg);
		#endif

		DeleteItem(tabIdx);
	}

	MCHKHEAP

	if (mb_InKeySwitching)
	{
		if (mn_CurSelTab >= nCurCount)  // ���� ��������� ��� ����� �� �������
			mb_InKeySwitching = FALSE;
	}

	if (!mb_InKeySwitching && nCurTab != -1)
	{
		SelectTab(nCurTab);
	}

	UpdateToolConsoles();

	//if (gpSet->isTabsInCaption)
	//{
	//	SendMessage(ghWnd, WM_NCPAINT, 0, 0);
	//}

	mn_InUpdate --;

	if (mb_PostUpdateRequested)
	{
		mb_PostUpdateCalled = FALSE;
		mb_PostUpdateRequested = FALSE;
		RequestPostUpdate();
	}

	MCHKHEAP
}

void TabBarClass::AddTab2VCon(VConTabs& vct)
{
	//#ifdef _DEBUG
	//std::vector<VConTabs>::iterator i = m_Tab2VCon.begin();
	//while (i != m_Tab2VCon.end())
	//{
	//	_ASSERTE(i->pVCon!=vct.pVCon || i->nFarWindowId!=vct.nFarWindowId);
	//	++i;
	//}
	//#endif

	m_Tab2VCon.push_back(vct);
}

RECT TabBarClass::GetMargins()
{
	_ASSERTE(this);
	RECT rcNewMargins = {0,0};

	if (_active || (gpSet->isTabs == 1))
	{
		if (!_tabHeight)
		{
			// ��������� ������
			GetTabbarHeight();
		}

		if (_tabHeight /*&& IsTabsShown()*/)
		{
			_ASSERTE(_tabHeight!=0 && "Height must be evaluated already!");
			
			if (gpSet->nTabsLocation == 1)
				rcNewMargins = MakeRect(0,0,0,_tabHeight);
			else
				rcNewMargins = MakeRect(0,_tabHeight,0,0);

			//if (memcmp(&rcNewMargins, &m_Margins, sizeof(m_Margins)) != 0)
			//{
			//	m_Margins = rcNewMargins;
			//	gpSet->UpdateMargins(m_Margins);
			//}
		}
	}
	//return m_Margins;

	return rcNewMargins;
}

void TabBarClass::UpdatePosition()
{
	if (!mh_Rebar)
		return;

	if (gpConEmu->isIconic())
		return; // ����� ������ �������� ����� ������������!

	RECT client;
	client = gpConEmu->GetGuiClientRect(); // ��� ���������� ������ ����
	DEBUGSTRTABS(_active ? L"TabBarClass::UpdatePosition(activate)\n" : L"TabBarClass::UpdatePosition(DEactivate)\n");

	#ifdef _DEBUG
	DWORD_PTR dwStyle = GetWindowLongPtr(ghWnd, GWL_STYLE);
	#endif

	if (_active)
	{
		_visible = true;

		if (mh_Rebar)
		{
			if (!IsWindowVisible(mh_Rebar))
				apiShowWindow(mh_Rebar, SW_SHOW);

			//MoveWindow(mh_Rebar, 0, 0, client.right, _tabHeight, 1);
		}
		else
		{
			if (!IsWindowVisible(mh_Tabbar))
				apiShowWindow(mh_Tabbar, SW_SHOW);

			//if (gpSet->isTabFrame)
			//	MoveWindow(mh_Tabbar, 0, 0, client.right, client.bottom, 1);
			//else
			MoveWindow(mh_Tabbar, 0, 0, client.right, _tabHeight, 1);
		}

		#ifdef _DEBUG
		dwStyle = GetWindowLongPtr(ghWnd, GWL_STYLE);
		#endif

		if (!gpConEmu->InCreateWindow())
		{
			//gpConEmu->Sync ConsoleToWindow(); -- 2009.07.04 Sync ������ ���� �������� � ����� ReSize
			gpConEmu->ReSize(TRUE);
		}
	}
	else
	{
		_visible = false;

		//gpConEmu->Sync ConsoleToWindow(); -- 2009.07.04 Sync ������ ���� �������� � ����� ReSize
		gpConEmu->ReSize(TRUE);

		// _active ��� ��������, ������� ������� �������� ����� � �����
		if (mh_Rebar)
		{
			if (IsWindowVisible(mh_Rebar))
				apiShowWindow(mh_Rebar, SW_HIDE);
		}
		else
		{
			if (IsWindowVisible(mh_Tabbar))
				apiShowWindow(mh_Tabbar, SW_HIDE);
		}
	}
}

void TabBarClass::Reposition()
{
	if (!_active)
	{
		return;
	}

	RECT client, self;
	client = gpConEmu->GetGuiClientRect();
	GetWindowRect(mh_Tabbar, &self);

	if (mh_Rebar)
	{
		SIZE sz = {0,0};
		int nBarIndex = -1;
		BOOL lbNeedShow = FALSE, lbWideEnough = FALSE;

		if (mh_Toolbar)
		{
			nBarIndex = SendMessage(mh_Rebar, RB_IDTOINDEX, 2, 0);

			if (gpSet->isMultiShowButtons)
			{
				SendMessage(mh_Toolbar, TB_GETMAXSIZE, 0, (LPARAM)&sz);
				sz.cx += (gpSet->isHideCaptionAlways() ? gpSet->nToolbarAddSpace : 0);
				lbWideEnough = (sz.cx + 150) <= client.right;

				if (!lbWideEnough)
				{
					if (IsWindowVisible(mh_Toolbar))
						SendMessage(mh_Rebar, RB_SHOWBAND, nBarIndex, 0);
				}
				else
				{
					if (!IsWindowVisible(mh_Toolbar))
						lbNeedShow = TRUE;
				}
			}
			else
			{
				SendMessage(mh_Rebar, RB_SHOWBAND, nBarIndex, 0);
			}
		}

		if (gpSet->nTabsLocation == 1)
		{
			int nStatusHeight = gpSet->isStatusBarShow ? gpSet->StatusBarHeight() : 0;
			MoveWindow(mh_Rebar, 0, client.bottom-nStatusHeight-_tabHeight, client.right, _tabHeight, 1);
		}
		else
		{
			MoveWindow(mh_Rebar, 0, 0, client.right, _tabHeight, 1);
		}

		// �� ����� ���� �������. ��������� ���� ��������. ����� �� OwnerDraw ����������.
		#if 0
		DWORD nCurStyle = GetWindowLong(mh_Tabbar, GWL_STYLE);
		DWORD nNeedStyle = (gpSet->nTabsLocation == 1) ? (nCurStyle | TCS_BOTTOM) : (nCurStyle & ~TCS_BOTTOM);
		if (nNeedStyle != nCurStyle)
		{
			SetWindowLong(mh_Tabbar, GWL_STYLE, nNeedStyle);

			//_ASSERTE(!gpSet->isTabFrame);
			if (gpSet->nTabsLocation == 1)
			{
				RECT rcTab = client;
				GetWindowRect(mh_Tabbar, &rcTab);
				SetWindowPos(mh_Tabbar, NULL, 0, client.bottom - (rcTab.bottom-rcTab.top), 0,0, SWP_NOSIZE);
			}
			else
			{
				SetWindowPos(mh_Tabbar, NULL, 0,0, 0,0, SWP_NOSIZE);
			}
		}
		#endif

		//if (gpSet->nTabsLocation == 1)
		//	m_Margins = MakeRect(0,0,0,_tabHeight);
		//else
		//	m_Margins = MakeRect(0,_tabHeight,0,0);
		//gpSet->UpdateMargins(m_Margins);


		if (lbWideEnough && nBarIndex != 1)
		{
			SendMessage(mh_Rebar, RB_MOVEBAND, nBarIndex, 1);
			nBarIndex = SendMessage(mh_Rebar, RB_IDTOINDEX, 2, 0);
			_ASSERTE(nBarIndex == 1);
		}

		if (lbNeedShow)
		{
			SendMessage(mh_Rebar, RB_SHOWBAND, nBarIndex, 1);
		}
	}
	//else if (gpSet->isTabFrame)
	//{
	//	MoveWindow(mh_Tabbar, 0, 0, client.right, client.bottom, 1);
	//}
	else
	{
		MoveWindow(mh_Tabbar, 0, 0, client.right, _tabHeight, 1);
	}

	UpdateToolbarPos();
}

void TabBarClass::UpdateToolbarPos()
{
	if (mh_Toolbar && gpSet->isMultiShowButtons)
	{
		SIZE sz;
		SendMessage(mh_Toolbar, TB_GETMAXSIZE, 0, (LPARAM)&sz);
		// � Win2k ����� ����� ���� ���� ���������� ������� (�����������)
		if ((gOSVer.dwMajorVersion == 5) && (gOSVer.dwMinorVersion == 0) && !gbIsWine)
			sz.cx += 26;

		if (mh_Rebar)
		{
			if (sz.cx != mn_LastToolbarWidth)
			{
				REBARBANDINFO rbBand= {80}; // �� ���������� size, �.�. �������� "�����" ������ �� ����� � � XP ������������
				rbBand.fMask  = RBBIM_SIZE | RBBIM_CHILDSIZE;
				// Set values unique to the band with the toolbar.
				rbBand.cx = rbBand.cxMinChild = rbBand.cxIdeal = mn_LastToolbarWidth = sz.cx;
				rbBand.cyMinChild = sz.cy;
				// Add the band that has the toolbar.
				SendMessage(mh_Rebar, RB_SETBANDINFO, 1, (LPARAM)&rbBand);
			}
		}
		else
		{
			RECT rcClient;
			GetWindowRect(mh_Tabbar, &rcClient);
			MapWindowPoints(NULL, ghWnd, (LPPOINT)&rcClient, 2);
		}
	}
}

LRESULT TabBarClass::OnNotify(LPNMHDR nmhdr)
{
	if (!this)
		return false;

	if (!_active)
	{
		return false;
	}

	//gpConEmu->setFocus(); // 02.04.2009 Maks - ?
	if (nmhdr->code == TCN_SELCHANGING)
	{
		//if (mb_ChangeAllowed) {
		//  return FALSE;
		//}
		_prevTab = GetCurSel();
		// Returns TRUE to prevent the selection from changing, or FALSE to allow the selection to change.
		return FALSE; // ���������
	}

	if (nmhdr->code == TCN_SELCHANGE)
	{
		int lnNewTab = GetCurSel();
		//_tcscpy(_lastTitle, gpConEmu->Title);

		if (_prevTab>=0)
		{
			SelectTab(_prevTab);
			_prevTab = -1;
		}

		if (mb_ChangeAllowed)
		{
			return FALSE;
		}

		FarSendChangeTab(lnNewTab);
		// start waiting for title to change
		_titleShouldChange = true;
		return true;
	}

	if (nmhdr->code == TBN_GETINFOTIP /*&& nmhdr->hwndFrom == mh_Toolbar*/)
	{
		if (!gpSet->isMultiShowButtons)
			return 0;

		LPNMTBGETINFOTIP pDisp = (LPNMTBGETINFOTIP)nmhdr;

		//if (pDisp->iItem>=1 && pDisp->iItem<=MAX_CONSOLE_COUNT)
		if (pDisp->iItem == TID_ACTIVE_NUMBER)
		{
			if (!pDisp->pszText || !pDisp->cchTextMax)
				return false;

			CVConGuard VCon;
			CVirtualConsole* pVCon = (gpConEmu->GetActiveVCon(&VCon) >= 0) ? VCon.VCon() : NULL;
			LPCWSTR pszTitle = pVCon ? pVCon->RCon()->GetTitle() : NULL;

			if (pszTitle)
			{
				lstrcpyn(pDisp->pszText, pszTitle, pDisp->cchTextMax);
			}
			else
			{
				pDisp->pszText[0] = 0;
			}
		}
		else if (pDisp->iItem == TID_CREATE_CON)
		{
			lstrcpyn(pDisp->pszText, _T("Create new console"), pDisp->cchTextMax);
		}
		else if (pDisp->iItem == TID_ALTERNATIVE)
		{
			BOOL lbPressed = (SendMessage(mh_Toolbar, TB_GETSTATE, pDisp->iItem, 0) & TBSTATE_CHECKED) == TBSTATE_CHECKED;
			lstrcpyn(pDisp->pszText,
			         lbPressed ? L"Alternative mode is ON (console freezed)" : L"Alternative mode is off",
			         pDisp->cchTextMax);
		}
		else if (pDisp->iItem == TID_SCROLL)
		{
			BOOL lbPressed = (SendMessage(mh_Toolbar, TB_GETSTATE, pDisp->iItem, 0) & TBSTATE_CHECKED) == TBSTATE_CHECKED;
			lstrcpyn(pDisp->pszText,
			         lbPressed ? L"BufferHeight mode is ON (scrolling enabled)" : L"BufferHeight mode is off",
			         pDisp->cchTextMax);
		}
		else if (pDisp->iItem == TID_MINIMIZE)
		{
			lstrcpyn(pDisp->pszText, _T("Minimize window"), pDisp->cchTextMax);
		}
		else if (pDisp->iItem == TID_MAXIMIZE)
		{
			lstrcpyn(pDisp->pszText, _T("Maximize window"), pDisp->cchTextMax);
		}
		else if (pDisp->iItem == TID_APPCLOSE)
		{
			lstrcpyn(pDisp->pszText, _T("Close ALL consoles"), pDisp->cchTextMax);
		}
		else if (pDisp->iItem == TID_COPYING)
		{
			lstrcpyn(pDisp->pszText, _T("Show copying queue"), pDisp->cchTextMax);
		}

		return true;
	}

	if (nmhdr->code == TBN_DROPDOWN
	        && (mh_Toolbar && (nmhdr->hwndFrom == mh_Toolbar)))
	{
		LPNMTOOLBAR pBtn = (LPNMTOOLBAR)nmhdr;
		switch (pBtn->iItem)
		{
		case TID_ACTIVE_NUMBER:
			OnChooseTabPopup();
			break;
		case TID_CREATE_CON:
			gpConEmu->mp_Menu->OnNewConPopupMenu();
			break;
		}
		return TBDDRET_DEFAULT;
	}

	if (nmhdr->code == TTN_GETDISPINFO
	        && (mh_Tabbar && (nmhdr->hwndFrom == mh_Tabbar || nmhdr->hwndFrom == mh_TabTip)))
	{
		LPNMTTDISPINFO pDisp = (LPNMTTDISPINFO)nmhdr;
		CVirtualConsole *pVCon = NULL;
		DWORD wndIndex = 0;
		TCHITTESTINFO htInfo;
		pDisp->hinst = NULL;
		pDisp->szText[0] = 0;
		pDisp->lpszText = NULL;
		GetCursorPos(&htInfo.pt);
		MapWindowPoints(NULL, mh_Tabbar, &htInfo.pt, 1);
		int iPage = TabCtrl_HitTest(mh_Tabbar, &htInfo);

		if (iPage >= 0)
		{
			// ���� � ���� ��� "�" - ��� �� �����
			if (!wcschr(GetTabText(iPage), L'\x2026' /*"�"*/))
				return 0;

			if (!GetVConFromTab(iPage, &pVCon, &wndIndex))
				return 0;

			if (!pVCon->RCon()->GetTab(wndIndex, &m_Tab4Tip))
				return 0;

			pDisp->lpszText = m_Tab4Tip.Name;
		}

		return true;
	}

	return false;
}

LPCWSTR TabBarClass::GetTabText(int nTabIdx)
{
	TCITEM item = {TCIF_TEXT};
	item.pszText = ms_TmpTabText; item.cchTextMax = sizeof(ms_TmpTabText)/sizeof(ms_TmpTabText[0]);
	ms_TmpTabText[0] = 0;

	if (!TabCtrl_GetItem(mh_Tabbar, nTabIdx, &item))
		return L"";

	return ms_TmpTabText;
}

void TabBarClass::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (!this)
		return;

	if (mh_Toolbar != (HWND)lParam)
		return;

	if (!gpSet->isMultiShowButtons)
	{
		_ASSERTE(gpSet->isMultiShowButtons);
		return;
	}

	if (wParam == TID_ACTIVE_NUMBER)
	{
		//gpConEmu->ConActivate(wParam-1);
		OnChooseTabPopup();
	}
	else if (wParam == TID_CREATE_CON)
	{
		if (gpConEmu->IsGesturesEnabled())
			gpConEmu->mp_Menu->OnNewConPopupMenu();
		else
			gpConEmu->RecreateAction(gpSet->GetDefaultCreateAction(), gpSet->isMultiNewConfirm || isPressed(VK_SHIFT));
	}
	else if (wParam == TID_ALTERNATIVE)
	{
		CVConGuard VCon;
		CVirtualConsole* pVCon = (gpConEmu->GetActiveVCon(&VCon) >= 0) ? VCon.VCon() : NULL;
		SendMessage(mh_Toolbar, TB_CHECKBUTTON, TID_ALTERNATIVE, pVCon ? pVCon->RCon()->isAlternative() : false);
		gpConEmu->AskChangeAlternative();
	}
	else if (wParam == TID_SCROLL)
	{
		CVConGuard VCon;
		CVirtualConsole* pVCon = (gpConEmu->GetActiveVCon(&VCon) >= 0) ? VCon.VCon() : NULL;
		SendMessage(mh_Toolbar, TB_CHECKBUTTON, TID_SCROLL, pVCon ? pVCon->RCon()->isBufferHeight() : false);
		gpConEmu->AskChangeBufferHeight();
	}
	else if (wParam == TID_MINIMIZE)
	{
		PostMessage(ghWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
	}
	else if (wParam == TID_MAXIMIZE)
	{
		// ����� ���� �������� �� ���������� � �������
		gpConEmu->mouse.state |= MOUSE_SIZING_DBLCKL;
		// ������ AltF9
		gpConEmu->OnAltF9(TRUE);
	}
	else if (wParam == TID_APPCLOSE)
	{
		gpConEmu->PostScClose();
	}
	else if (wParam == TID_COPYING)
	{
		gpConEmu->OnCopyingState();
	}
}

void TabBarClass::OnMouse(int message, int x, int y)
{
	if (!this)
		return;

	if (!_active)
	{
		return;
	}

	if ((message == WM_MBUTTONUP)
		|| (message == WM_RBUTTONUP)
		|| ((message == WM_LBUTTONDBLCLK) && gpSet->nTabBtnDblClickAction))
	{
		TCHITTESTINFO htInfo;
		htInfo.pt.x = x;
		htInfo.pt.y = y;
		int iPage = TabCtrl_HitTest(mh_Tabbar, &htInfo);

		if (iPage != -1)
		{
			CVirtualConsole* pVCon = NULL;
			// ��� ���� ����� �������� ����������, ������� �� �����, ����� ������� �������� �� �����
			// � �� ����� ����� �����, �� ����� ��������� ����...
			POINT ptCur = {0,0}; GetCursorPos(&ptCur);
			pVCon = FarSendChangeTab(iPage);

			if (pVCon)
			{
				CVConGuard guard(pVCon);
				BOOL lbCtrlPressed = isPressed(VK_CONTROL);

				if (message == WM_LBUTTONDBLCLK)
				{
					switch (gpSet->nTabBtnDblClickAction)
					{
					case 1:
						// ����� ���� �������� �� ���������� � �������
						gpConEmu->mouse.state |= MOUSE_SIZING_DBLCKL;
						// ������ AltF9
						gpConEmu->OnAltF9(TRUE);
						break;
					case 2:
						guard->RCon()->CloseTab();
						break;
					case 3:
						gpConEmu->mp_Menu->ExecPopupMenuCmd(guard.VCon(), IDM_RESTART);
						break;
					case 4:
						gpConEmu->mp_Menu->ExecPopupMenuCmd(guard.VCon(), IDM_DUPLICATE);
						break;
					}
				}
				else if (message == WM_RBUTTONUP && !lbCtrlPressed)
				{
					gpConEmu->mp_Menu->ShowPopupMenu(guard.VCon(), ptCur);
				}
				else
				{
					guard->RCon()->CloseTab();
					//if (pVCon->RCon()->GetFarPID())
					//{
					//	pVCon->RCon()->PostMacro(gpSet->sTabCloseMacro ? gpSet->sTabCloseMacro : L"F10");
					//}
					//else
					//{
					//	// ���� ������� CMD, PowerShell, � �.�.
					//	pVCon->RCon()->CloseTab();
					//	//// �������� ������ ������������ �������, ��� ���� ������ Terminate & Recreate
					//	//gpConEmu->Recreate(TRUE, TRUE);
					//}
				}

				// ������ � ������������� � ������
				gpConEmu->isValid(pVCon);
			}
		}
	}
}

void TabBarClass::Invalidate()
{
	if (gpConEmu->mp_TabBar->IsTabsActive())
		InvalidateRect(mh_Rebar, NULL, TRUE);
}

void TabBarClass::OnCaptionHidden()
{
	if (!this) return;

	if (mh_Toolbar)
	{
		BOOL lbHide = !gpSet->isCaptionHidden();

		OnWindowStateChanged();
		SendMessage(mh_Toolbar, TB_HIDEBUTTON, TID_MINIMIZE_SEP, lbHide);
		SendMessage(mh_Toolbar, TB_HIDEBUTTON, TID_MINIMIZE, lbHide);
		SendMessage(mh_Toolbar, TB_HIDEBUTTON, TID_MAXIMIZE, lbHide);
		SendMessage(mh_Toolbar, TB_HIDEBUTTON, TID_APPCLOSE, lbHide);
		SendMessage(mh_Toolbar, TB_AUTOSIZE, 0, 0);

		//if (abUpdatePos)
		{
			UpdateToolbarPos();
		}
	}
}

void TabBarClass::OnWindowStateChanged()
{
	if (!this) return;

	if (mh_Toolbar)
	{
		TBBUTTONINFO tbi = {sizeof(TBBUTTONINFO), TBIF_IMAGE};
		tbi.iImage = (gpConEmu->GetWindowMode() != wmNormal) ? BID_MAXIMIZE_IDX : BID_RESTORE_IDX;
		SendMessage(mh_Toolbar, TB_SETBUTTONINFO, TID_MAXIMIZE, (LPARAM)&tbi);
		//OnCaptionHidden();
	}
}

// �������� ������� ������ �������� �� �������
void TabBarClass::UpdateToolConsoles(bool abForcePos/*=false*/)
{
	if (!mh_Toolbar) return;

	//bool bPresent[MAX_CONSOLE_COUNT] = {};
	//MCHKHEAP
	//for (int i = 1; i <= MAX_CONSOLE_COUNT; i++)
	//{
	//	bPresent[i-1] = (gpConEmu->GetVConTitle(i-1) != NULL);
	//}

	//bool bRedraw = abForcePos;
	//if (abForcePos)
	//	SendMessage(mh_Toolbar, WM_SETREDRAW, FALSE, 0);

	int nNewActiveIdx = gpConEmu->ActiveConNum(); // 0-based

	OnConsoleActivated(nNewActiveIdx);

	//TBBUTTONINFO tbi = {sizeof(TBBUTTONINFO), TBIF_IMAGE};
	//SendMessage(mh_Toolbar, TB_GETBUTTONINFO, TID_ACTIVE_NUMBER, (LPARAM)&tbi);
	//if (tbi.iImage != nNewActiveIdx)
	//{
	//	if ((nNewActiveIdx >= BID_FIST_CON) && (nNewActiveIdx <= BID_LAST_CON))
	//	{
	//		tbi.iImage = nNewActiveIdx;
	//	}
	//	else
	//	{
	//		tbi.iImage = BID_DUMMYBTN_IDX;
	//	}
	//	//if (!bRedraw)
	//	//{
	//	//	bRedraw = true;
	//	//	SendMessage(mh_Toolbar, WM_SETREDRAW, FALSE, 0);
	//	//}
	//	SendMessage(mh_Toolbar, TB_SETBUTTONINFO, TID_ACTIVE_NUMBER, (LPARAM)&tbi);
	//}

	//for (int i = 1; i <= MAX_CONSOLE_COUNT; i++)
	//{
	//	bool bShown = (SendMessage(mh_Toolbar, TB_ISBUTTONHIDDEN, i, 0) == 0);
	//	if (bShown != bPresent[i-1])
	//	{
	//		if (!bRedraw)
	//		{
	//			bRedraw = true;
	//			SendMessage(mh_Toolbar, WM_SETREDRAW, FALSE, 0);
	//		}
	//		SendMessage(mh_Toolbar, TB_HIDEBUTTON, i, !bPresent[i-1]);
	//	}
	//}

	//if (bRedraw)
	//{
	//	UpdateToolbarPos();
	//	SendMessage(mh_Toolbar, WM_SETREDRAW, TRUE, 0);
	//}
}

// nConNumber - 0-based
void TabBarClass::OnConsoleActivated(int nConNumber)
{
	if (!mh_Toolbar) return;

	TBBUTTONINFO tbi = {sizeof(TBBUTTONINFO), TBIF_IMAGE};
	SendMessage(mh_Toolbar, TB_GETBUTTONINFO, TID_ACTIVE_NUMBER, (LPARAM)&tbi);

	if (tbi.iImage != nConNumber)
	{
		bool bNeedShow = false, bWasHidden = false;

		if ((nConNumber >= BID_FIST_CON) && (nConNumber <= BID_LAST_CON))
		{
			bNeedShow = (tbi.iImage == BID_DUMMYBTN_IDX);
			tbi.iImage = nConNumber;
		}
		else
		{
			if (tbi.iImage != BID_DUMMYBTN_IDX)
			{
				SendMessage(mh_Toolbar, TB_HIDEBUTTON, TID_ACTIVE_NUMBER, TRUE);
				bWasHidden = true;
			}
			tbi.iImage = BID_DUMMYBTN_IDX;
		}

		//if (!bRedraw)
		//{
		//	bRedraw = true;
		//	SendMessage(mh_Toolbar, WM_SETREDRAW, FALSE, 0);
		//}

		SendMessage(mh_Toolbar, TB_SETBUTTONINFO, TID_ACTIVE_NUMBER, (LPARAM)&tbi);

		if (bNeedShow)
		{
			SendMessage(mh_Toolbar, TB_HIDEBUTTON, TID_ACTIVE_NUMBER, FALSE);
		}

		if (bNeedShow || bWasHidden)
		{
			SendMessage(mh_Toolbar, TB_AUTOSIZE, 0, 0);
			UpdateToolbarPos();
		}
	}

	//UpdateToolConsoles(true);
	////nConNumber = gpConEmu->ActiveConNum()+1; -- ���� ������ ��� ���������� �����!
	//if (nConNumber>=1 && nConNumber<=MAX_CONSOLE_COUNT)
	//{
	//	SendMessage(mh_Toolbar, TB_CHECKBUTTON, nConNumber, 1);
	//}
	//else
	//{
	//	for (int i = 1; i <= MAX_CONSOLE_COUNT; i++)
	//		SendMessage(mh_Toolbar, TB_CHECKBUTTON, i, 0);
	//}
}

void TabBarClass::OnBufferHeight(BOOL abBufferHeight)
{
	if (!mh_Toolbar) return;

	SendMessage(mh_Toolbar, TB_CHECKBUTTON, TID_SCROLL, abBufferHeight);
}

void TabBarClass::OnAlternative(BOOL abAlternative)
{
	if (!mh_Toolbar) return;

	SendMessage(mh_Toolbar, TB_CHECKBUTTON, TID_ALTERNATIVE, abAlternative);
}

void TabBarClass::OnShowButtonsChanged()
{
	if (!mh_Rebar)
		return;
	
	//Reposition();

	//bool bChanged = false;

	if (gpSet->isMultiShowButtons && !mh_Toolbar)
	{
		if (CreateToolbar() != NULL)
		{
			REBARBANDINFO rbBand = {80}; // �� ���������� size, �.�. �������� "�����" ������ �� ����� � � XP ������������

			rbBand.fMask  = RBBIM_SIZE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_ID | RBBIM_STYLE | RBBIM_COLORS;
			rbBand.fStyle = RBBS_CHILDEDGE | RBBS_FIXEDSIZE | RBBS_VARIABLEHEIGHT;
			rbBand.clrBack = GetSysColor(COLOR_BTNFACE);
			rbBand.clrFore = GetSysColor(COLOR_BTNTEXT);

			SIZE sz = {0,0};
			SendMessage(mh_Toolbar, TB_GETMAXSIZE, 0, (LPARAM)&sz);

			// Set values unique to the band with the toolbar.
			rbBand.wID        = 2;
			rbBand.hwndChild  = mh_Toolbar;
			rbBand.cx = rbBand.cxMinChild = rbBand.cxIdeal = mn_LastToolbarWidth = sz.cx;
			rbBand.cyChild = rbBand.cyMinChild = rbBand.cyMaxChild = sz.cy + mn_ThemeHeightDiff;

			// Add the band that has the toolbar.
			if (!SendMessage(mh_Rebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand))
			{
				DisplayLastError(_T("Can't initialize rebar (toolbar)"));
			}
		}
	}
	//else
	//{
	//	if (mh_Toolbar)
	//	{
	//		DestroyWindow(mh_Toolbar);
	//		mh_Toolbar = NULL;
	//		bChanged = true;
	//	}
	//}

	//if (bChanged)
	//{

	Reposition();

	//}
}

HWND TabBarClass::CreateToolbar()
{
	gpSetCls->CheckTheming();

	if (!mh_Rebar || !gpSet->isMultiShowButtons)
		return NULL; // ��� ����� - ��� � �������

	if (mh_Toolbar)
		return mh_Toolbar; // ��� �������

	mh_Toolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
	                            WS_CHILD|WS_VISIBLE|
	                            TBSTYLE_FLAT|CCS_NOPARENTALIGN|CCS_NORESIZE|CCS_NODIVIDER|
	                            TBSTYLE_TOOLTIPS|TBSTYLE_TRANSPARENT,
	                            0, 0, 0, 0, mh_Rebar,
	                            NULL, NULL, NULL);
	_defaultToolProc = (WNDPROC)SetWindowLongPtr(mh_Toolbar, GWLP_WNDPROC, (LONG_PTR)ToolProc);
	DWORD lExStyle = ((DWORD)SendMessage(mh_Toolbar, TB_GETEXTENDEDSTYLE, 0, 0)) | TBSTYLE_EX_DRAWDDARROWS;
	SendMessage(mh_Toolbar, TB_SETEXTENDEDSTYLE, 0, lExStyle);
	SendMessage(mh_Toolbar, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);
	SendMessage(mh_Toolbar, TB_SETBITMAPSIZE, 0, MAKELONG(14,14));

	TBADDBITMAP bmp = {g_hInstance,IDB_MAIN_TOOLBAR};
	int nFirst = SendMessage(mh_Toolbar, TB_ADDBITMAP, BID_TOOLBAR_LAST_IDX, (LPARAM)&bmp);
	_ASSERTE(BID_TOOLBAR_LAST_IDX==37);
	
	//DWORD nLoadErr = 0;
	if (gnOsVer >= 0x600)
	{
		bmp.hInst = g_hInstance;
		bmp.nID = IDB_COPY24;
	}
	else
	{
		bmp.hInst = NULL;
		COLORMAP colorMap = {RGB(255,0,0),GetSysColor(COLOR_BTNFACE)};
		bmp.nID = (UINT_PTR)CreateMappedBitmap(g_hInstance, IDB_COPY4, 0, &colorMap, 1);
		//bmp.nID = (UINT_PTR)LoadImage(g_hInstance, MAKEINTRESOURCE(IDB_COPY24), IMAGE_BITMAP, 0,0, LR_LOADTRANSPARENT|LR_LOADMAP3DCOLORS);
		//nLoadErr = GetLastError();
	}

	int nCopyBmp = SendMessage(mh_Toolbar, TB_ADDBITMAP, 1, (LPARAM)&bmp);
	// ������ 37 ����������
	_ASSERTE(nCopyBmp == BID_TOOLBAR_LAST_IDX);
	if (nCopyBmp < BID_TOOLBAR_LAST_IDX)
		nCopyBmp = BID_TOOLBAR_LAST_IDX;

	{
		bmp.hInst = NULL;
		COLORMAP colorMap = {0xC0C0C0,GetSysColor(COLOR_BTNFACE)};
		bmp.nID = (UINT_PTR)CreateMappedBitmap(g_hInstance, IDB_SCROLL, 0, &colorMap, 1);
	}
	int nScrollBmp = SendMessage(mh_Toolbar, TB_ADDBITMAP, 1, (LPARAM)&bmp);
	// ������ 38 ����������
	_ASSERTE(nScrollBmp == (BID_TOOLBAR_LAST_IDX+1));
	if (nScrollBmp < (BID_TOOLBAR_LAST_IDX+1))
		nScrollBmp = BID_TOOLBAR_LAST_IDX+1;
		

	//buttons
	TBBUTTON btn = {0, 0, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP};
	TBBUTTON sep = {0, TID_MINIMIZE_SEP+1, TBSTATE_ENABLED, TBSTYLE_SEP};
	int nActiveCon = gpConEmu->ActiveConNum()+1;

	// New console
	btn.iBitmap = nFirst + BID_NEWCON_IDX;
	btn.idCommand = TID_CREATE_CON;
	btn.fsStyle = BTNS_DROPDOWN;
	btn.fsState = TBSTATE_ENABLED;
	SendMessage(mh_Toolbar, TB_ADDBUTTONS, 1, (LPARAM)&btn);

	// Console numbers
	btn.iBitmap = ((nActiveCon >= 0) ? (nFirst + BID_FIST_CON) : BID_DUMMYBTN_IDX);
	btn.idCommand = TID_ACTIVE_NUMBER;
	btn.fsStyle = BTNS_DROPDOWN;
	btn.fsState = TBSTATE_ENABLED;
	SendMessage(mh_Toolbar, TB_ADDBUTTONS, 1, (LPARAM)&btn);

	btn.fsStyle = BTNS_BUTTON;
	//SendMessage(mh_Toolbar, TB_ADDBUTTONS, 1, (LPARAM)&sep); sep.idCommand++;
#if 0 //defined(_DEBUG)
	// Show copying state
	btn.iBitmap = nCopyBmp; btn.idCommand = TID_COPYING;
	SendMessage(mh_Toolbar, TB_ADDBUTTONS, 1, (LPARAM)&btn);
	SendMessage(mh_Toolbar, TB_ADDBUTTONS, 1, (LPARAM)&sep); sep.idCommand++;
#endif
	// Buffer height mode
	btn.iBitmap = nFirst + BID_ALTERNATIVE_IDX; btn.idCommand = TID_ALTERNATIVE; btn.fsState = TBSTATE_ENABLED;
	SendMessage(mh_Toolbar, TB_ADDBUTTONS, 1, (LPARAM)&btn);
	// Show copying state
	btn.iBitmap = nScrollBmp; btn.idCommand = TID_SCROLL;
	SendMessage(mh_Toolbar, TB_ADDBUTTONS, 1, (LPARAM)&btn);
	// Separator before min/max/close
	sep.fsState |= TBSTATE_HIDDEN; sep.idCommand = TID_MINIMIZE_SEP;
	SendMessage(mh_Toolbar, TB_ADDBUTTONS, 1, (LPARAM)&sep);
	// Min,Max,Close
	btn.iBitmap = nFirst + BID_MINIMIZE_IDX; btn.idCommand = TID_MINIMIZE; btn.fsState = TBSTATE_ENABLED|TBSTATE_HIDDEN;
	SendMessage(mh_Toolbar, TB_ADDBUTTONS, 1, (LPARAM)&btn);
	btn.iBitmap = nFirst + ((gpConEmu->GetWindowMode() != wmNormal) ? BID_MAXIMIZE_IDX : BID_RESTORE_IDX); btn.idCommand = TID_MAXIMIZE;
	SendMessage(mh_Toolbar, TB_ADDBUTTONS, 1, (LPARAM)&btn);
	btn.iBitmap = nFirst + BID_APPCLOSE_IDX; btn.idCommand = TID_APPCLOSE;
	SendMessage(mh_Toolbar, TB_ADDBUTTONS, 1, (LPARAM)&btn);
	SendMessage(mh_Toolbar, TB_AUTOSIZE, 0, 0);
#ifdef _DEBUG
	SIZE sz;
	SendMessage(mh_Toolbar, TB_GETMAXSIZE, 0, (LPARAM)&sz);
#endif
	return mh_Toolbar;
}

HWND TabBarClass::GetTabbar()
{
	if (!this) return NULL;

	return mh_Tabbar;
}

int TabBarClass::GetTabbarHeight()
{
	if (!this) return 0;

	_ASSERTE(gpSet->isTabs!=0);

	if (mb_ForceRecalcHeight || (_tabHeight == 0))
	{
		// ����� ����������� ������ ����

		HWND hTabs = mh_Tabbar;
		//bool bDummyCreate = (hTabs == NULL);
		//
		//if (bDummyCreate)
		//{
		//	hTabs = CreateTabbar(true);
		//}

		if (hTabs)
		{
			// ��� ���������� �������� ���������� �������. �.�. ������ - �� 0. ��������� �� �����
			RECT rcClient = MakeRect(600, 400);
			//rcClient = gpConEmu->GetGuiClientRect();
			TabCtrl_AdjustRect(hTabs, FALSE, &rcClient);
			_tabHeight = rcClient.top - mn_ThemeHeightDiff;
		}
		else
		{
			// �� ����� ��������� TabBar. ��� ����� ��� ���� ConEmu ������ ���������� ��������
			//_ASSERTE((hTabs!=NULL) && "Creating of a dummy tab control failed");
			_tabHeight = gpSet->nTabFontHeight + 9;
		}

		//if (bDummyCreate && hTabs)
		//{
		//	DestroyWindow(hTabs);
		//	mh_Tabbar = NULL;
		//}
	}

	return _tabHeight;
}

HWND TabBarClass::CreateTabbar(bool abDummyCreate /*= false*/)
{
	gpSetCls->CheckTheming();

	if (!mh_Rebar && !abDummyCreate)
		return NULL; // ���� ��������� ������ ��� Band � ReBar

	if (mh_Tabbar)
		return mh_Tabbar; // ��� �������

	if (!abDummyCreate && !mh_TabIcons)
	{
		mh_TabIcons = ImageList_LoadImage(g_hInstance, MAKEINTRESOURCE(IDB_SHIELD), 14, 1, RGB(128,0,0), IMAGE_BITMAP, LR_CREATEDIBSECTION);
		mn_AdminIcon = (gOSVer.dwMajorVersion >= 6) ? 0 : 1;
	}

	// ����� �������� ������ ����� �������� �������� ����, ����� IsAppThemed ����� ���������� FALSE
	BOOL bAppThemed = FALSE, bThemeActive = FALSE;
	FAppThemed pfnThemed = NULL;
	HMODULE hUxTheme = LoadLibrary(L"UxTheme.dll");

	if (hUxTheme)
	{
		pfnThemed = (FAppThemed)GetProcAddress(hUxTheme, "IsAppThemed");

		if (pfnThemed) bAppThemed = pfnThemed();

		pfnThemed = (FAppThemed)GetProcAddress(hUxTheme, "IsThemeActive");

		if (pfnThemed) bThemeActive = pfnThemed();

		FreeLibrary(hUxTheme); hUxTheme = NULL;
	}

	if (!bAppThemed || !bThemeActive)
		mn_ThemeHeightDiff = 2;

	/*mh_TabbarP = CreateWindow(_T("VirtualConsoleClassBar"), _T(""),
	        WS_VISIBLE|WS_CHILD, 0,0,340,22, ghWnd, 0, 0, 0);
	if (!mh_TabbarP) return NULL;*/
	RECT rcClient = {-32000, -32000, -32000+300, -32000+100};
	if (ghWnd)
	{
		rcClient = gpConEmu->GetGuiClientRect();
	}
	else
	{
		_ASSERTE(abDummyCreate);
	}
	
	DWORD nPlacement = TCS_SINGLELINE|(abDummyCreate ? 0 : (WS_VISIBLE | WS_CHILD))/*|TCS_BUTTONS*//*|TCS_TOOLTIPS*/;
			//|((gpSet->nTabsLocation == 1) ? TCS_BOTTOM : 0);

	mh_Tabbar = CreateWindow(WC_TABCONTROL, NULL, nPlacement | WS_CLIPSIBLINGS | TCS_FOCUSNEVER, 0, 0,
	                         rcClient.right, 0, mh_Rebar, NULL, g_hInstance, NULL);

	if (mh_Tabbar == NULL)
	{
		return NULL;
	}

	if (!abDummyCreate)
	{
		//#if !defined(__GNUC__)
		//#pragma warning (disable : 4312)
		//#endif

		// ����
		_defaultTabProc = (WNDPROC)SetWindowLongPtr(mh_Tabbar, GWLP_WNDPROC, (LONG_PTR)TabProc);
		SendMessage(mh_Tabbar, TCM_SETIMAGELIST, 0, (LPARAM)mh_TabIcons);

		if (!mh_TabTip || !IsWindow(mh_TabTip))
		{
			mh_TabTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
									   WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
									   CW_USEDEFAULT, CW_USEDEFAULT,
									   CW_USEDEFAULT, CW_USEDEFAULT,
									   mh_Tabbar, NULL,
									   g_hInstance, NULL);
			SetWindowPos(mh_TabTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
			TabCtrl_SetToolTips(mh_Tabbar, mh_TabTip);
		}
	}

	UpdateTabFont();
	//HFONT hFont = CreateFont(gpSet->nTabFontHeight, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, gpSet->nTabFontCharSet, OUT_DEFAULT_PRECIS,
	//                         CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, gpSet->sTabFontFace);
	//SendMessage(mh_Tabbar, WM_SETFONT, WPARAM(hFont), TRUE);

	if (!abDummyCreate && (!mh_Balloon || !IsWindow(mh_Balloon)))
	{
		mh_Balloon = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
		                            gpSetCls->BalloonStyle() | WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
		                            CW_USEDEFAULT, CW_USEDEFAULT,
		                            CW_USEDEFAULT, CW_USEDEFAULT,
		                            mh_Tabbar, NULL,
		                            g_hInstance, NULL);
		SetWindowPos(mh_Balloon, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
		// Set up tool information.
		// In this case, the "tool" is the entire parent window.
		tiBalloon.cbSize = 44; // ��� sizeof(TOOLINFO);
		tiBalloon.uFlags = TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE;
		tiBalloon.hwnd = mh_Tabbar;
		tiBalloon.hinst = g_hInstance;
		static wchar_t szAsterisk[] = L"*"; // eliminate GCC warning
		tiBalloon.lpszText = szAsterisk;
		tiBalloon.uId = (UINT_PTR)TTF_IDISHWND;
		GetClientRect(mh_Tabbar, &tiBalloon.rect);
		// Associate the ToolTip with the tool window.
		SendMessage(mh_Balloon, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO) &tiBalloon);
		// Allow multiline
		SendMessage(mh_Balloon, TTM_SETMAXTIPWIDTH, 0, (LPARAM)300);
	}

	// ��������� ��������, ��� ������ (��� �������������) ��������� ������������
	//AddTab(gpConEmu->isFar() ? gpSet->szTabPanels : gpSet->pszTabConsole, 0);
	AddTab(gpConEmu->GetLastTitle(), 0, gpConEmu->mb_IsUacAdmin);
	// ��� ���������� �������� ���������� �������. �.�. ������ - �� 0. ��������� �� �����
	rcClient = MakeRect(600, 400);
	//rcClient = gpConEmu->GetGuiClientRect();
	TabCtrl_AdjustRect(mh_Tabbar, FALSE, &rcClient);
	_tabHeight = rcClient.top - mn_ThemeHeightDiff;
	return mh_Tabbar;
}

void TabBarClass::CreateRebar()
{
	RECT rcWnd = {-32000, -32000, -32000+300, -32000+100};
	if (ghWnd)
	{
		rcWnd = gpConEmu->GetGuiClientRect();
	}
	else
	{
		_ASSERTE(ghWnd!=NULL); // ����� ReBar ��� ����� �� ���������.
	}

	gpSetCls->CheckTheming();

	if (NULL == (mh_Rebar = CreateWindowEx(WS_EX_TOOLWINDOW, REBARCLASSNAME, NULL,
								(ghWnd ? (WS_VISIBLE | WS_CHILD) : 0)
								|WS_CLIPSIBLINGS|WS_CLIPCHILDREN
	                            |/*CCS_NORESIZE|*/CCS_NOPARENTALIGN
	                            |RBS_FIXEDORDER|RBS_AUTOSIZE|/*RBS_VARHEIGHT|*/CCS_NODIVIDER,
	                            0,0,rcWnd.right,16, ghWnd, NULL, g_hInstance, NULL)))
		return;

#if !defined(__GNUC__)
#pragma warning (disable : 4312)
#endif
	// ����
	_defaultReBarProc = (WNDPROC)SetWindowLongPtr(mh_Rebar, GWLP_WNDPROC, (LONG_PTR)ReBarProc);
	REBARINFO     rbi= {sizeof(REBARINFO)};
	REBARBANDINFO rbBand = {80}; // �� ���������� size, �.�. �������� "�����" ������ �� ����� � � XP ������������

	if (!SendMessage(mh_Rebar, RB_SETBARINFO, 0, (LPARAM)&rbi))
	{
		DisplayLastError(_T("Can't initialize rebar!"));
		DestroyWindow(mh_Rebar);
		mh_Rebar = NULL;
		return;
	}

	rbBand.fMask  = RBBIM_SIZE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_ID | RBBIM_STYLE | RBBIM_COLORS;
	rbBand.fStyle = RBBS_CHILDEDGE | RBBS_FIXEDSIZE | RBBS_VARIABLEHEIGHT;
	rbBand.clrBack = GetSysColor(COLOR_BTNFACE);
	rbBand.clrFore = GetSysColor(COLOR_BTNTEXT);
	SendMessage(mh_Rebar, RB_SETBKCOLOR, 0, GetSysColor(COLOR_BTNFACE));
	SendMessage(mh_Rebar, RB_SETWINDOWTHEME, 0, (LPARAM)L" ");
	CreateTabbar();
	CreateToolbar();
	SIZE sz = {0,0};

	if (mh_Toolbar)
	{
		SendMessage(mh_Toolbar, TB_GETMAXSIZE, 0, (LPARAM)&sz);
	}
	else
	{
		RECT rcClient = gpConEmu->GetGuiClientRect();
		TabCtrl_AdjustRect(mh_Tabbar, FALSE, &rcClient);
		sz.cy = rcClient.top - 3 - mn_ThemeHeightDiff;
	}

	if (mh_Tabbar)
	{
		// Set values unique to the band with the toolbar.
		rbBand.wID          = 1;
		rbBand.hwndChild  = mh_Tabbar;
		rbBand.cxMinChild = 100;
		rbBand.cx = rbBand.cxIdeal = rcWnd.right - sz.cx - 80;
		rbBand.cyChild = rbBand.cyMinChild = rbBand.cyMaxChild = _tabHeight; // sz.cy;

		if (!SendMessage(mh_Rebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand))
		{
			DisplayLastError(_T("Can't initialize rebar (tabbar)"));
		}
	}

	if (mh_Toolbar)
	{
		// Set values unique to the band with the toolbar.
		rbBand.wID        = 2;
		rbBand.hwndChild  = mh_Toolbar;
		rbBand.cx = rbBand.cxMinChild = rbBand.cxIdeal = mn_LastToolbarWidth = sz.cx;
		rbBand.cyChild = rbBand.cyMinChild = rbBand.cyMaxChild = sz.cy + mn_ThemeHeightDiff;

		// Add the band that has the toolbar.
		if (!SendMessage(mh_Rebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand))
		{
			DisplayLastError(_T("Can't initialize rebar (toolbar)"));
		}

		//if (mn_ThemeHeightDiff) {
		//	POINT pt = {0,0};
		//	MapWindowPoints(mh_Toolbar, mh_Rebar, &pt, 1);
		//	pt.y = 0;
		//	SetWindowPos(mh_Toolbar, NULL, pt.x, pt.y, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
		//}
	}

#ifdef _DEBUG
	RECT rc;
	GetWindowRect(mh_Rebar, &rc);
#endif
	//GetWindowRect(mh_Rebar, &rc);
	//_tabHeight = rc.bottom - rc.top;
	//if (gpSet->nTabsLocation == 1)
	//	m_Margins = MakeRect(0,0,0,_tabHeight);
	//else
	//	m_Margins = MakeRect(0,_tabHeight,0,0);
	//gpSet->UpdateMargins(m_Margins);
	//_hwndTab = mh_Rebar; // ����...
}

void TabBarClass::PrepareTab(ConEmuTab* pTab, CVirtualConsole *apVCon)
{
#ifdef _DEBUG

	if (this != gpConEmu->mp_TabBar)
	{
		_ASSERTE(this == gpConEmu->mp_TabBar);
	}

#endif
	MCHKHEAP
	// get file name
	TCHAR dummy[MAX_PATH*2];
	TCHAR fileName[MAX_PATH+4]; fileName[0] = 0;
	TCHAR szFormat[32];
	TCHAR szEllip[MAX_PATH+1];
	//wchar_t /**tFileName=NULL,*/ *pszNo=NULL, *pszTitle=NULL;
	int nSplit = 0;
	int nMaxLen = 0; //gpSet->nTabLenMax - _tcslen(szFormat) + 2/* %s */;
	int origLength = 0; //_tcslen(tFileName);

	CRealConsole* pRCon = apVCon ? apVCon->RCon() : NULL;
	bool bIsFar = pRCon ? pRCon->isFar() : false;


	if (pTab->Name[0]==0 || (pTab->Type & 0xFF) == 1/*WTYPE_PANELS*/)
	{
		//_tcscpy(szFormat, _T("%s"));
		lstrcpyn(szFormat, bIsFar ? gpSet->szTabPanels : gpSet->szTabConsole, countof(szFormat));
		nMaxLen = gpSet->nTabLenMax - _tcslen(szFormat) + 2/* %s */;

		if (apVCon && gpConEmu->mb_IsUacAdmin)
			pTab->Type |= fwt_Elevated;

		if (pTab->Name[0] == 0)
		{
			// ��� ����� ��������� ��� ������������� GUI ��� �������� �������
			#if 0
			_ASSERTE(!bIsFar);
			
			int nTabCount = GetItemCount();

			if (nTabCount>0 && gpConEmu->ActiveCon()!=NULL)
			{
				//_ASSERTE(pTab->Name[0] != 0);
				nTabCount = nTabCount;
			}
			#endif

			//100930 - ������. GetLastTitle() ������ ������� �������, � pTab ����� ���� �� ����� �������!
			// -- _tcscpy(pTab->Name, gpConEmu->GetLastTitle()); //isFar() ? gpSet->szTabPanels : gpSet->pszTabConsole);
			_tcscpy(pTab->Name, gpConEmu->GetDefaultTitle());
		}

		lstrcpyn(fileName, pTab->Name, countof(fileName));
		if (gpSet->szTabSkipWords[0])
		{
			LPCWSTR pszWord = gpSet->szTabSkipWords;
			while (pszWord && *pszWord)
			{
				LPCWSTR pszNext = wcschr(pszWord, L'|');
				if (!pszNext) pszNext = pszWord + _tcslen(pszWord);
				
				int nLen = (int)(pszNext - pszWord);
				if (nLen > 0)
				{
					lstrcpyn(dummy, pszWord, min((int)countof(dummy),(nLen+1)));
					wchar_t* pszFound;
					while ((pszFound = wcsstr(fileName, dummy)) != NULL)
					{
						size_t nLeft = _tcslen(pszFound);
						if (nLeft <= (size_t)nLen)
						{
							*pszFound = 0;
							break;
						}
						else
						{
							wmemmove(pszFound, pszFound+(size_t)nLen, nLeft - nLen + 1);
						}
					}
				}

				if (!*pszNext)
					break;
				pszWord = pszNext + 1;
			}
		}
		origLength = _tcslen(fileName);
		//if (origLength>6) {
		//    // ����� � ��������� ���� ���-�� ����� "{C:\Program Fil...- Far"
		//    //                              ������ "{C:\Program F...} - Far"
		//	����� ���������� �������� � ��������� ���� - ��� ��� ������� �� ����� � ����� ������... ��� ��� ���� ������ - '...' ������ ������� � �����
		//    if (lstrcmp(tFileName + origLength - 6, L" - Far") == 0)
		//        nSplit = nMaxLen - 6;
		//}
	}
	else
	{
		LPTSTR tFileName = NULL;
		if (GetFullPathName(pTab->Name, countof(dummy), dummy, &tFileName) && tFileName && *tFileName)
			lstrcpyn(fileName, tFileName, countof(fileName));
		else
			lstrcpyn(fileName, pTab->Name, countof(fileName));

		if ((pTab->Type & 0xFF) == 3/*WTYPE_EDITOR*/)
		{
			if (pTab->Modified)
				lstrcpyn(szFormat, gpSet->szTabEditorModified, countof(szFormat));
			else
				lstrcpyn(szFormat, gpSet->szTabEditor, countof(szFormat));
		}
		else if ((pTab->Type & 0xFF) == 2/*WTYPE_VIEWER*/)
		{
			lstrcpyn(szFormat, gpSet->szTabViewer, countof(szFormat));
		}
		else
		{
			_ASSERTE(FALSE && "Must be processed in previous branch");
			lstrcpyn(szFormat, bIsFar ? gpSet->szTabPanels : gpSet->szTabConsole, countof(szFormat));
		}
	}

	// restrict length
	if (!nMaxLen)
		nMaxLen = gpSet->nTabLenMax - _tcslen(szFormat) + 2/* %s */;

	if (!origLength)
		origLength = _tcslen(fileName);
	if (nMaxLen<15) nMaxLen=15; else if (nMaxLen>=MAX_PATH) nMaxLen=MAX_PATH-1;

	if (origLength > nMaxLen)
	{
		/*_tcsnset(fileName, _T('\0'), MAX_PATH);
		_tcsncat(fileName, tFileName, 10);
		_tcsncat(fileName, _T("..."), 3);
		_tcsncat(fileName, tFileName + origLength - 10, 10);*/
		//if (!nSplit)
		//    nSplit = nMaxLen*2/3;
		//// 2009-09-20 ���� � ��������� ��� ���������� (����������� �����)
		//const wchar_t* pszAdmin = gpSet->szAdminTitleSuffix;
		//const wchar_t* pszFrom = tFileName + origLength - (nMaxLen - nSplit);
		//if (!wcschr(pszFrom, L'.') && (*pszAdmin && !wcsstr(tFileName, pszAdmin)))
		//{
		//	// �� ��������� ������� � �����, � �� ��������
		//	nSplit = nMaxLen;
		//}
		// "{C:\Program Files} - Far 2.1283 Administrator x64"
		// ����� ���������� �������� � ��������� ���� - ��� ��� ������� �� ����� � ����� ������... ��� ��� ���� ������ - '...' ������ ������� � �����
		nSplit = nMaxLen;
		_tcsncpy(szEllip, fileName, nSplit); szEllip[nSplit]=0;
		szEllip[nSplit] = L'\x2026' /*"�"*/;
		szEllip[nSplit+1] = 0;
		//_tcscat(szEllip, L"\x2026" /*"�"*/);
		//_tcscat(szEllip, tFileName + origLength - (nMaxLen - nSplit));
		//tFileName = szEllip;
		lstrcpyn(fileName, szEllip, countof(fileName));
	}

	// szFormat ����������� ��� Panel/Viewer(*)/Editor(*)
	// ������: "%i-[%s] *"
	////pszNo = wcsstr(szFormat, L"%i");
	////pszTitle = wcsstr(szFormat, L"%s");
	////if (pszNo == NULL)
	////	_wsprintf(fileName, SKIPLEN(countof(fileName)) szFormat, tFileName);
	////else if (pszNo < pszTitle || pszTitle == NULL)
	////	_wsprintf(fileName, SKIPLEN(countof(fileName)) szFormat, pTab->Pos, tFileName);
	////else
	////	_wsprintf(fileName, SKIPLEN(countof(fileName)) szFormat, tFileName, pTab->Pos);
	//wcscpy(pTab->Name, fileName);
	const TCHAR* pszFmt = szFormat;
	TCHAR* pszDst = pTab->Name;
	TCHAR* pszEnd = pTab->Name + countof(pTab->Name) - 1; // � ����� ��� ����� ��������������� ����� ��� '\0'
	
	if (!pszFmt || !*pszFmt)
	{
		pszFmt = _T("%s");
	}
	*pszDst = 0;

	bool bRenamedTab = false;
	if (pTab->Type & fwt_Renamed)
	{
		if (wcsstr(pszFmt, L"%s") == NULL)
		{
			if (wcsstr(pszFmt, L"%n") != NULL)
				bRenamedTab = true;
			else
				pszFmt = _T("%s");
		}
	}
	
	TCHAR szTmp[64];
	
	while (*pszFmt && pszDst < pszEnd)
	{
		if (*pszFmt == _T('%'))
		{
			pszFmt++;
			LPCTSTR pszText = NULL;
			switch (*pszFmt)
			{
				case _T('s'): case _T('S'):
					pszText = fileName;
					break;
				case _T('i'): case _T('I'):
					_wsprintf(szTmp, SKIPLEN(countof(szTmp)) _T("%i"), pTab->Pos);
					pszText = szTmp;
					break;
				case _T('p'): case _T('P'):
					if (!apVCon || !apVCon->RCon())
					{
						wcscpy_c(szTmp, _T("?"));
					}
					else
					{
						_wsprintf(szTmp, SKIPLEN(countof(szTmp)) _T("%u"), apVCon->RCon()->GetActivePID());
					}
					pszText = szTmp;
					break;
				case _T('c'): case _T('C'):
					{
						int iCon = gpConEmu->isVConValid(apVCon);
						if (iCon > 0)
							_wsprintf(szTmp, SKIPLEN(countof(szTmp)) _T("%u"), iCon);
						else
							wcscpy_c(szTmp, _T("?"));
						pszText = szTmp;
					}
					break;
				case _T('n'): case _T('N'):
					{
						pszText = bRenamedTab ? fileName : pRCon ? pRCon->GetActiveProcessName() : NULL;
						wcscpy_c(szTmp, (pszText && *pszText) ? pszText : L"?");
						pszText = szTmp;
					}
					break;
				case _T('%'):
					pszText = L"%";
					break;
				case 0:
					pszFmt--;
					break;
			}
			pszFmt++;
			if (pszText)
			{
				while (*pszText && pszDst < pszEnd)
				{
					*(pszDst++) = *(pszText++);
				}
			}
		}
		else
		{
			*(pszDst++) = *(pszFmt++);
		}
	}
	*pszDst = 0;
	
	MCHKHEAP
}



// ������������ �����

int TabBarClass::GetIndexByTab(VConTabs tab)
{
	//int nIdx = -1;
	int nCount = m_Tab2VCon.size();

	for (int i = 0; i < nCount; i++)
	{
		if (m_Tab2VCon[i] == tab)
		{
			return i;
		}
	}

	//std::vector<VConTabs>::iterator iter = m_Tab2VCon.begin();
	//while (iter != m_Tab2VCon.end())
	//{
	//	nIdx ++;
	//	if (*iter == tab)
	//		return nIdx;
	//	++iter;
	//}

	return -1;
}

int TabBarClass::GetNextTab(BOOL abForward, BOOL abAltStyle/*=FALSE*/)
{
	BOOL lbRecentMode = (gpSet->isTabs != 0) &&
	                    (((abAltStyle == FALSE) ? gpSet->isTabRecent : !gpSet->isTabRecent));
	int nCurSel = GetCurSel();
	int nCurCount = GetItemCount();
	VConTabs cur = {NULL};

	#ifdef _DEBUG
	if (nCurCount != m_Tab2VCon.size())
	{
		_ASSERTE(nCurCount == m_Tab2VCon.size());
	}
	#endif

	if (nCurCount < 1)
		return 0; // ���� ������ � �� ������ ����

	if (lbRecentMode && (nCurSel >= 0) && (nCurSel < m_Tab2VCon.size()))
	{
		cur = m_Tab2VCon[nCurSel];
	}

	int i, nNewSel = -1;
	TODO("�������� ����������� ������������� �'�� RecentScreens");

	if (abForward)
	{
		if (lbRecentMode)
		{
			//std::vector<VConTabs>::iterator iter = m_TabStack.begin();
			//while (iter != m_TabStack.end())
			int iter = 0;

			while (iter < m_TabStack.size())
			{
				VConTabs Item = m_TabStack[iter]; // *iter;

				// ����� � ����� ���������� ���
				if (Item == cur)
				{
					int iCur = iter;

					// ���������� ��������� ���, ������� �� ����� ������������
					do
					{
						++iter;

                        // ���� ����� �� ����� (������ ������� ��������� ���) ������� ������
						if (iter >= m_TabStack.size())
						{
							iter = 0;
						}

						// ���������� ������ � m_Tab2VCon
						i = GetIndexByTab(m_TabStack[iter]);

						if (CanActivateTab(i))
						{
							return i;
						}
					}
					while (iter != iCur);

					break;
				}

				++iter;
			}
		} // ���� �� ������ � ����� Recent - ���� ������� �����

		for (i = nCurSel+1; nNewSel == -1 && i < nCurCount; i++)
		{
			if (CanActivateTab(i))
				nNewSel = i;
		}

		for (i = 0; nNewSel == -1 && i < nCurSel; i++)
		{
			if (CanActivateTab(i))
				nNewSel = i;
		}
	}
	else
	{
		if (lbRecentMode)
		{
			//std::vector<VConTabs>::reverse_iterator iter = m_TabStack.rbegin();
			int iter = m_TabStack.size()-1;

			//while (iter != m_TabStack.rend())
			while (iter >= 0)
			{
				VConTabs Item = m_TabStack[iter]; // *iter;

				// ����� � ����� ���������� ���
				if (Item == cur)
				{
					int iCur = iter;

					// ���������� ��������� ���, ������� �� ����� ������������
					do
					{
						iter--;

                        // ���� ����� �� ����� (������ ������� ��������� ���) ������� ������
						if (iter < 0)
						{
							iter = m_TabStack.size()-1;
						}

						// ���������� ������ � m_Tab2VCon
						i = GetIndexByTab(m_TabStack[iter]);

						if (CanActivateTab(i))
						{
							return i;
						}
					}
					while (iter != iCur);

					break;
				}

				iter--;
			}
		} // ���� �� ������ � ����� Recent - ���� ������� �����

		for (i = nCurSel-1; nNewSel == -1 && i >= 0; i++)
		{
			if (CanActivateTab(i))
				nNewSel = i;
		}

		for (i = nCurCount-1; nNewSel == -1 && i > nCurSel; i++)
		{
			if (CanActivateTab(i))
				nNewSel = i;
		}
	}

	return nNewSel;
}

void TabBarClass::SwitchNext(BOOL abAltStyle/*=FALSE*/)
{
	Switch(TRUE, abAltStyle);
}

void TabBarClass::SwitchPrev(BOOL abAltStyle/*=FALSE*/)
{
	Switch(FALSE, abAltStyle);
}

void TabBarClass::Switch(BOOL abForward, BOOL abAltStyle/*=FALSE*/)
{
	int nNewSel = GetNextTab(abForward, abAltStyle);

	if (nNewSel != -1)
	{
		// mh_Tabbar ����� ���� � ������, �� �������� �������������!
		if (gpSet->isTabLazy && mh_Tabbar && gpSet->isTabs)
		{
			mb_InKeySwitching = TRUE;
			// ���� Ctrl �� ������� - ������ ������������ ���, � �� ����������� �������
			SelectTab(nNewSel);
		}
		else
		{
			FarSendChangeTab(nNewSel);
			mb_InKeySwitching = FALSE;
		}
	}
}

BOOL TabBarClass::IsInSwitch()
{
	return mb_InKeySwitching;
}

void TabBarClass::SwitchCommit()
{
	if (!mb_InKeySwitching) return;

	int nCurSel = GetCurSel();
	mb_InKeySwitching = FALSE;
	CVirtualConsole* pVCon = FarSendChangeTab(nCurSel);
	UNREFERENCED_PARAMETER(pVCon);
}

void TabBarClass::SwitchRollback()
{
	if (mb_InKeySwitching)
	{
		mb_InKeySwitching = FALSE;
		Update();
	}
}

// ����� �� ����� ������, � ������� �����
void TabBarClass::CheckStack()
{
	_ASSERTE(gpConEmu->isMainThread());
	//std::vector<VConTabs>::iterator i, j;
	int i, j;

	BOOL lbExist = FALSE;
	//j = m_TabStack.begin();
	j = 0;

	//while (j != m_TabStack.end())
	while (j < m_TabStack.size())
	{
		lbExist = FALSE;

		//for (i = m_Tab2VCon.begin(); i != m_Tab2VCon.end(); ++i)
		for (i = 0; i < m_Tab2VCon.size(); ++i)
		{
			//if (*i == *j)
			if (m_Tab2VCon[i] == m_TabStack[j])
			{
				lbExist = TRUE; break;
			}
		}

		if (lbExist)
		{
			++j;
		}
		else
		{
			m_TabStack.erase(j);
		}
	}

	//for (i = m_Tab2VCon.begin(); i != m_Tab2VCon.end(); ++i)
	for (i = 0; i < m_Tab2VCon.size(); ++i)
	{
		lbExist = FALSE;

		//for (j = m_TabStack.begin(); j != m_TabStack.end(); ++j)
		for (j = 0; j < m_TabStack.size(); ++j)
		{
			//if (*i == *j)
			if (m_Tab2VCon[i] == m_TabStack[j])
			{
				lbExist = TRUE; break;
			}
		}

		if (!lbExist)
		{
			//m_TabStack.push_back(*i);
			m_TabStack.push_back(m_Tab2VCon[i]);
		}
	}
}

// ����� �� ����� ������������� � �������� tab �� ���� �����
void TabBarClass::AddStack(VConTabs tab)
{
	_ASSERTE(gpConEmu->isMainThread());
	BOOL lbExist = FALSE;

	if (!m_TabStack.empty())
	{
		//VConTabs tmp;
		//std::vector<VConTabs>::iterator iter = m_TabStack.begin();
		int iter = 0;

		//while (iter != m_TabStack.end())
		while (iter < m_TabStack.size())
		{
			if (m_TabStack[iter] == tab)
			{
				//if (iter == m_TabStack.begin())
				if (iter == 0)
				{
					lbExist = TRUE;
				}
				else
				{
					m_TabStack.erase(iter);
				}

				break;
			}

			++iter;
		}
	}

	// ��������� ������ �����
	if (!lbExist)
	{
		//m_TabStack.insert(m_TabStack.begin(), tab);
		m_TabStack.insert(0, tab);
	}

	CheckStack();
}

BOOL TabBarClass::CanActivateTab(int nTabIdx)
{
	CVirtualConsole *pVCon = NULL;
	DWORD wndIndex = 0;

	if (!GetVConFromTab(nTabIdx, &pVCon, &wndIndex))
		return FALSE;

	if (!pVCon->RCon()->CanActivateFarWindow(wndIndex))
		return FALSE;

	return TRUE;
}

BOOL TabBarClass::OnKeyboard(UINT messg, WPARAM wParam, LPARAM lParam)
{
	//if (!IsShown()) return FALSE; -- ������. ���� ������ ���� � ������
	BOOL lbAltPressed = isPressed(VK_MENU);

	if (messg == WM_KEYDOWN && wParam == VK_TAB)
	{
		if (!isPressed(VK_SHIFT))
			SwitchNext(lbAltPressed);
		else
			SwitchPrev(lbAltPressed);

		return TRUE;
	}
	else if (mb_InKeySwitching && messg == WM_KEYDOWN && !lbAltPressed
	        && (wParam == VK_UP || wParam == VK_DOWN || wParam == VK_LEFT || wParam == VK_RIGHT))
	{
		bool bRecent = gpSet->isTabRecent;
		gpSet->isTabRecent = false;
		BOOL bForward = (wParam == VK_RIGHT || wParam == VK_DOWN);
		Switch(bForward);
		gpSet->isTabRecent = bRecent;

		return TRUE;
	}

	return FALSE;
}

void TabBarClass::SetRedraw(BOOL abEnableRedraw)
{
	mb_DisableRedraw = !abEnableRedraw;
}

void TabBarClass::ShowTabError(LPCTSTR asInfo, int tabIndex)
{
	if (!asInfo)
		ms_TabErrText[0] = 0;
	else if (asInfo != ms_TabErrText)
		lstrcpyn(ms_TabErrText, asInfo, countof(ms_TabErrText));

	tiBalloon.lpszText = ms_TabErrText;

	if (ms_TabErrText[0])
	{
		SendMessage(mh_TabTip, TTM_ACTIVATE, FALSE, 0);
		SendMessage(mh_Balloon, TTM_UPDATETIPTEXT, 0, (LPARAM)&tiBalloon);
		//RECT rcControl; GetWindowRect(GetDlgItem(hMain, tFontFace), &rcControl);
		RECT rcTab = {0}; TabCtrl_GetItemRect(mh_Tabbar, tabIndex, &rcTab);
		MapWindowPoints(mh_Tabbar, NULL, (LPPOINT)&rcTab, 2);
		//int ptx = 0; //rcControl.right - 10;
		//int pty = 0; //(rcControl.top + rcControl.bottom) / 2;
		SendMessage(mh_Balloon, TTM_TRACKPOSITION, 0, MAKELONG((rcTab.left+rcTab.right)>>1,rcTab.bottom));
		SendMessage(mh_Balloon, TTM_TRACKACTIVATE, TRUE, (LPARAM)&tiBalloon);
		SetTimer(mh_Tabbar, FAILED_TABBAR_TIMERID, FAILED_TABBAR_TIMEOUT, 0);
	}
	else
	{
		SendMessage(mh_Balloon, TTM_TRACKACTIVATE, FALSE, (LPARAM)&tiBalloon);
		SendMessage(mh_TabTip, TTM_ACTIVATE, TRUE, 0);
	}
}

//void TabBarClass::PaintHeader(HDC hdc, RECT rcPaint)
//{
//	int nCount = 0, V, R;
//	CVirtualConsole *pVCon = NULL;
//	CRealConsole *pRCon = NULL;
//
//	for(V = 0; V < MAX_CONSOLE_COUNT; V++)
//	{
//		if (!(pVCon = gpConEmu->GetVCon(V))) continue;
//
//		//BOOL lbActive = gpConEmu->isActive(pVCon);
//		//if (gpSet->bHideInactiveConsoleTabs)
//		//{
//		//	if (!lbActive) continue;
//		//}
//		if (!(pRCon = pVCon->RCon())) continue;
//
//		nCount += pRCon->GetTabCount();
//	}
//
//	if (nCount < 1) nCount = 1;
//
//	int nWidth = ((rcPaint.right - rcPaint.left) / nCount);
//	HBRUSH hWhite = (HBRUSH)GetStockObject(WHITE_BRUSH);
//	FillRect(hdc, &rcPaint, hWhite);
//	HBRUSH hSilver = (HBRUSH)GetStockObject(GRAY_BRUSH);
//	HBRUSH hOldBr = (HBRUSH)SelectObject(hdc, hSilver);
//	HPEN hBlack = CreatePen(PS_SOLID, 1, RGB(80,80,80));
//	HPEN hOldPen = (HPEN)SelectObject(hdc, hBlack);
//	ConEmuTab tab = {0};
//	RECT rcTab = {rcPaint.left, rcPaint.top, rcPaint.left+nWidth-1, rcPaint.bottom};
//	std::vector<VConTabs>::iterator iter = m_Tab2VCon.begin();
//	int i = 0;
//
//	for(V = 0; V < MAX_CONSOLE_COUNT && i < nCount; V++)
//	{
//		if (!(pVCon = gpConEmu->GetVCon(V))) continue;
//
//		//BOOL lbActive = gpConEmu->isActive(pVCon);
//		//if (gpSet->bHideInactiveConsoleTabs)
//		//{
//		//	if (!lbActive) continue;
//		//}
//		if (!(pRCon = pVCon->RCon())) continue;
//
//		int nRCount = pRCon->GetTabCount();
//
//		for(R = 0; R < nRCount; R++)
//		{
//			if (!pRCon->GetTab(R, &tab))
//				continue;
//
//			i++;
//
//			if (i > nCount)
//			{
//				_ASSERTE(i <= nCount);
//				break;
//			}
//
//			Rectangle(hdc, rcTab.left, rcTab.top, rcTab.right, rcTab.bottom);
//			DrawText(hdc, tab.Name, lstrlenW(tab.Name), &rcTab, DT_LEFT|DT_NOPREFIX|DT_VCENTER|DT_WORD_ELLIPSIS);
//			// Next
//			++iter; rcTab.left += nWidth; rcTab.right += nWidth;
//		}
//	}
//
//	SelectObject(hdc, hOldBr);
//	SelectObject(hdc, hOldPen);
//}

void TabBarClass::OnChooseTabPopup()
{
	RECT rcBtnRect = {0};
	SendMessage(mh_Toolbar, TB_GETRECT, TID_ACTIVE_NUMBER, (LPARAM)&rcBtnRect);
	MapWindowPoints(mh_Toolbar, NULL, (LPPOINT)&rcBtnRect, 2);
	POINT pt = {rcBtnRect.right,rcBtnRect.bottom};

	gpConEmu->ChooseTabFromMenu(FALSE, pt, TPM_RIGHTALIGN|TPM_TOPALIGN);
}

//void TabBarClass::OnNewConPopupMenu(POINT* ptWhere /*= NULL*/, DWORD nFlags /*= 0*/)
//{
//	HMENU hPopup = CreatePopupMenu();
//	LPCWSTR pszCurCmd = NULL;
//	bool lbReverse = (nFlags & TPM_BOTTOMALIGN) == TPM_BOTTOMALIGN;
//
//	CVConGuard VCon;
//
//	if ((gpConEmu->GetActiveVCon(&VCon) >= 0) && VCon->RCon())
//		pszCurCmd = VCon->RCon()->GetCmd();
//
//	LPCWSTR pszHistory = gpSet->psCmdHistory;
//	int nFirstID = 0, nLastID = 0, nFirstGroupID = 0, nLastGroupID = 0;
//	int nCreateID = 0, nSetupID = 0, nResetID = 0;
//	bool bWasHistory = false;
//
//	memset(m_CmdPopupMenu, 0, sizeof(m_CmdPopupMenu));
//
//	//// �������� ������ ������
//	//gpSet->LoadCmdTasks(NULL);
//
//	int nGroup = 0;
//	const Settings::CommandTasks* pGrp = NULL;
//	while ((nGroup < MAX_CMD_GROUP_SHOW) && (pGrp = gpSet->CmdTaskGet(nGroup)))
//	{
//		m_CmdPopupMenu[nLastID].nCmd = nLastID+1;
//		m_CmdPopupMenu[nLastID].pszCmd = NULL; // pGrp->pszCommands; - don't show hint, there is SubMenu on RClick
//
//		if (nGroup < 9)
//			_wsprintf(m_CmdPopupMenu[nLastID].szShort, SKIPLEN(countof(m_CmdPopupMenu[nLastID].szShort)) L"&%i: ", nGroup+1);
//		else if (nGroup == 9)
//			wcscpy_c(m_CmdPopupMenu[nLastID].szShort, L"1&0: ");
//		else
//			m_CmdPopupMenu[nLastID].szShort[0] = 0;
//
//		int nCurLen = _tcslen(m_CmdPopupMenu[nLastID].szShort);
//
//		int nMaxShort = countof(m_CmdPopupMenu[nLastID].szShort)-2; // wchar_t szShort[32];
//		int nLen = _tcslen(pGrp->pszName);
//		if ((nLen+nCurLen) < nMaxShort)
//		{
//			lstrcpyn(m_CmdPopupMenu[nLastID].szShort+nCurLen, pGrp->pszName, countof(m_CmdPopupMenu[nLastID].szShort)-nCurLen);
//			nLen = lstrlen(m_CmdPopupMenu[nLastID].szShort);
//		}
//		else
//		{
//			lstrcpyn(m_CmdPopupMenu[nLastID].szShort+nCurLen, pGrp->pszName, countof(m_CmdPopupMenu[nLastID].szShort)-1-nCurLen);
//			m_CmdPopupMenu[nLastID].szShort[nMaxShort-2] = /*�*/L'\x2026';
//			nLen = nMaxShort-1;
//		}
//		m_CmdPopupMenu[nLastID].szShort[nLen] = L'\t';
//		m_CmdPopupMenu[nLastID].szShort[nLen+1] = 0xBB /* RightArrow/Quotes */;
//		m_CmdPopupMenu[nLastID].szShort[nLen+2] = 0;
//
//		InsertMenu(hPopup, lbReverse ? 0 : -1, MF_BYPOSITION | MF_STRING | MF_ENABLED, m_CmdPopupMenu[nLastID].nCmd, m_CmdPopupMenu[nLastID].szShort);
//		nLastID++; nGroup++;
//		nLastGroupID = nLastID;
//		if (!nFirstGroupID)
//			nFirstGroupID = nLastID;
//	}
//
//	nSetupID = ++nLastID;
//	InsertMenu(hPopup, lbReverse ? 0 : -1, MF_BYPOSITION | MF_STRING | MF_ENABLED, nSetupID, L"Setup tasks...");
//
//	nFirstID = nLastID+1;
//
//	if (pszHistory && *pszHistory)
//	{
//		bool bSeparator = false;
//		int nCount = 0;
//		while (*pszHistory && (nCount < MAX_CMD_HISTORY_SHOW))
//		{
//			// ������� - ����� ������
//			if (!pszCurCmd || lstrcmp(pszCurCmd, pszHistory))
//			{
//				m_CmdPopupMenu[nLastID].nCmd = nLastID+1;
//				m_CmdPopupMenu[nLastID].pszCmd = pszHistory;
//				int nLen = _tcslen(pszHistory);
//				int nMaxShort = countof(m_CmdPopupMenu[nLastID].szShort);
//				if (nLen >= nMaxShort)
//				{
//					m_CmdPopupMenu[nLastID].szShort[0] = /*�*/L'\x2026';
//					_wcscpyn_c(m_CmdPopupMenu[nLastID].szShort+1, nMaxShort-1, pszHistory+nLen-nMaxShort+2, nMaxShort-1);
//					m_CmdPopupMenu[nLastID].szShort[nMaxShort-1] = 0;
//				}
//				else
//				{
//					_wcscpyn_c(m_CmdPopupMenu[nLastID].szShort, nMaxShort, pszHistory, nMaxShort);
//				}
//
//				if (!bSeparator)
//				{
//					bSeparator = true;
//					InsertMenu(hPopup, lbReverse ? 0 : -1, MF_BYPOSITION | MF_SEPARATOR, -1, NULL);
//				}
//
//				InsertMenu(hPopup, lbReverse ? 0 : -1, MF_BYPOSITION | MF_STRING | MF_ENABLED, m_CmdPopupMenu[nLastID].nCmd, m_CmdPopupMenu[nLastID].szShort);
//				nLastID++; nCount++; bWasHistory = true;
//			}
//
//			pszHistory += _tcslen(pszHistory)+1;
//		}
//	}
//
//	if (pszCurCmd && *pszCurCmd)
//	{
//		m_CmdPopupMenu[nLastID].nCmd = nLastID+1;
//		m_CmdPopupMenu[nLastID].pszCmd = pszCurCmd;
//		int nLen = _tcslen(pszCurCmd);
//		int nMaxShort = countof(m_CmdPopupMenu[nLastID].szShort);
//		if (nLen >= nMaxShort)
//		{
//			m_CmdPopupMenu[nLastID].szShort[0] = /*�*/L'\x2026';
//			_wcscpyn_c(m_CmdPopupMenu[nLastID].szShort+1, nMaxShort-1, pszCurCmd+nLen-nMaxShort+2, nMaxShort-1);
//			m_CmdPopupMenu[nLastID].szShort[nMaxShort-1] = 0;
//		}
//		else
//		{
//			_wcscpyn_c(m_CmdPopupMenu[nLastID].szShort, nMaxShort, pszCurCmd, nMaxShort);
//		}
//		InsertMenu(hPopup, lbReverse ? -1 : 0, MF_BYPOSITION|MF_SEPARATOR, 0, 0);
//		InsertMenu(hPopup, lbReverse ? -1 : 0, MF_BYPOSITION|MF_ENABLED|MF_STRING, m_CmdPopupMenu[nLastID].nCmd, m_CmdPopupMenu[nLastID].szShort);
//		nLastID++;
//		nCreateID = ++nLastID;
//		InsertMenu(hPopup, lbReverse ? -1 : 0, MF_BYPOSITION|MF_ENABLED|MF_STRING, nCreateID, L"New console dialog...");
//	}
//
//	if (bWasHistory)
//	{
//		nResetID = ++nLastID;
//		InsertMenu(hPopup, lbReverse ? 0 : -1, MF_BYPOSITION | MF_STRING | MF_ENABLED, nResetID, L"Clear history...");
//	}
//
//	RECT rcBtnRect = {0};
//	LPRECT lpExcl = NULL;
//	DWORD nAlign = TPM_RIGHTALIGN|TPM_TOPALIGN;
//
//	if (ptWhere)
//	{
//		rcBtnRect.right = ptWhere->x;
//		rcBtnRect.bottom = ptWhere->y;
//		if (nFlags)
//			nAlign = nFlags;
//	}
//	else if (IsTabsShown())
//	{
//		SendMessage(mh_Toolbar, TB_GETRECT, TID_CREATE_CON, (LPARAM)&rcBtnRect);
//		MapWindowPoints(mh_Toolbar, NULL, (LPPOINT)&rcBtnRect, 2);
//		lpExcl = &rcBtnRect;
//	}
//	else
//	{
//		GetClientRect(ghWnd, &rcBtnRect);
//		MapWindowPoints(ghWnd, NULL, (LPPOINT)&rcBtnRect, 2);
//		rcBtnRect.left = rcBtnRect.right;
//		rcBtnRect.bottom = rcBtnRect.top;
//	}
//
//	mn_FirstTaskID = nFirstGroupID; mn_LastTaskID = nLastGroupID;
//
//	mb_InNewConRPopup = false; // JIC
//
//	mb_InNewConPopup = true;
//	int nId = gpConEmu->trackPopupMenu(tmp_Cmd, hPopup, nAlign|TPM_RETURNCMD/*|TPM_NONOTIFY*/,
//	                         rcBtnRect.right,rcBtnRect.bottom, ghWnd, lpExcl);
//	mb_InNewConPopup = mb_InNewConRPopup = false;
//	//gpConEmu->mp_Tip->HideTip();
//	
//	if (nCreateID && (nId == nCreateID))
//	{
//		gpConEmu->RecreateAction(gpSet->GetDefaultCreateAction(), TRUE);
//	}
//	else if (nSetupID && (nId == nSetupID))
//	{
//		CSettings::Dialog(IDD_SPG_CMDTASKS);
//	}
//	else if (nResetID && (nId == nResetID))
//	{
//		gpSetCls->ResetCmdHistory();
//	}
//	else if (nId >= 1 && nId <= nLastID)
//	{
//		RConStartArgs con;
//		if (nLastGroupID > 0 && nId <= nLastGroupID)
//		{
//			const Settings::CommandTasks* pGrp = gpSet->CmdTaskGet(nId-nFirstGroupID);
//			if (pGrp)
//			{
//				con.pszSpecialCmd = lstrdup(pGrp->pszName);
//				_ASSERTE(con.pszSpecialCmd && *con.pszSpecialCmd==TaskBracketLeft && con.pszSpecialCmd[lstrlen(con.pszSpecialCmd)-1]==TaskBracketRight);
//			}
//			else
//			{
//				MBoxAssert(pGrp!=NULL);
//				goto wrap;
//			}
//		}
//		else if (nId >= nFirstID)
//		{
//			con.pszSpecialCmd = lstrdup(m_CmdPopupMenu[nId-1].pszCmd);
//		}
//
//		if (isPressed(VK_SHIFT))
//		{
//			int nRc = gpConEmu->RecreateDlg(&con);
//
//			if (nRc != IDC_START)
//				return;
//
//			CVConGroup::Redraw();
//		}
//		else
//		{
//			gpSet->HistoryAdd(con.pszSpecialCmd);
//		}
//
//		//����������, ������
//		if (gpSet->isMulti)
//			gpConEmu->CreateCon(&con, true);
//		else
//			gpConEmu->CreateWnd(&con);
//	}
//
//wrap:
//	DestroyMenu(hPopup);
//}
//
//void TabBarClass::OnNewConPopupMenuRClick(HMENU hMenu, UINT nItemPos)
//{
//	if (!mb_InNewConPopup || mb_InNewConRPopup)
//		return;
//
//	MENUITEMINFO mi = {sizeof(mi)};
//	mi.fMask = MIIM_ID;
//	if (!GetMenuItemInfo(hMenu, nItemPos, TRUE, &mi))
//		return;
//
//	wchar_t szClass[128] = {};
//	POINT ptCur = {}; GetCursorPos(&ptCur);
//	HWND hMenuWnd = WindowFromPoint(ptCur);
//	GetClassName(hMenuWnd, szClass, countof(szClass));
//	if (lstrcmp(szClass, L"#32768") != 0)
//		hMenuWnd = NULL;
//
//	UINT nId = mi.wID;
//
//	if (mn_LastTaskID < 0 || nId > (UINT)mn_LastTaskID)
//		return;
//
//	const Settings::CommandTasks* pGrp = gpSet->CmdTaskGet(nId-mn_FirstTaskID);
//	if (!pGrp || !pGrp->pszCommands || !*pGrp->pszCommands)
//		return;
//
//	// �������
//	HMENU hPopup = CreatePopupMenu();
//	wchar_t *pszDataW = lstrdup(pGrp->pszCommands);
//	wchar_t *pszLine = pszDataW;
//	wchar_t *pszNewLine = wcschr(pszLine, L'\n');
//	const UINT nStartID = 1000;
//	UINT nLastID = 0;
//	LPCWSTR pszLines[MAX_CONSOLE_COUNT];
//
//	while (*pszLine && (nLastID < MAX_CONSOLE_COUNT))
//	{
//		if (pszNewLine)
//		{
//			*pszNewLine = 0;
//			if ((pszNewLine > pszDataW) && (*(pszNewLine-1) == L'\r'))
//				*(pszNewLine-1) = 0;
//		}
//
//		while (*pszLine == L' ') pszLine++;
//
//		if (*pszLine)
//		{
//			pszLines[nLastID] = pszLine;
//			InsertMenu(hPopup, -1, MF_BYPOSITION | MF_STRING | MF_ENABLED, (++nLastID) + nStartID, pszLine);
//		}
//
//		if (!pszNewLine) break;
//
//		pszLine = pszNewLine+1;
//
//		if (!*pszLine) break;
//
//		while ((*pszLine == L'\r') || (*pszLine == L'\n'))
//			pszLine++; // ���������� ��� �������� �����
//
//		pszNewLine = wcschr(pszLine, L'\n');
//	}
//
//	int nRetID = -1;
//	
//	if (nLastID > 0)
//	{
//		RECT rcMenuItem = {};
//		GetMenuItemRect(ghWnd, hMenu, nItemPos, &rcMenuItem);
//
//		mb_InNewConRPopup = true;
//		nRetID = gpConEmu->trackPopupMenu(tmp_CmdPopup, hPopup, TPM_RETURNCMD|TPM_NONOTIFY|TPM_RECURSE,
//	                         rcMenuItem.right,rcMenuItem.top, ghWnd, &rcMenuItem);
//		mb_InNewConRPopup = false;
//	}
//
//	DestroyMenu(hPopup);
//
//	if (nRetID >= (nStartID+1) && nRetID <= (int)(nStartID+nLastID))
//	{
//		// Need to close parentmenu
//		if (hMenuWnd)
//		{
//			PostMessage(hMenuWnd, WM_CLOSE, 0, 0);
//		}
//
//		bool bRunAs = false;
//		LPCWSTR pszCmd = gpConEmu->ParseScriptLineOptions(pszLines[nRetID-nStartID-1], &bRunAs, NULL);
//
//		// Well, start selected line from Task
//		RConStartArgs con;
//		con.pszSpecialCmd = lstrdup(pszCmd);
//		if (!con.pszSpecialCmd)
//		{
//			_ASSERTE(con.pszSpecialCmd!=NULL);
//		}
//		else
//		{
//			con.bRunAsAdministrator = bRunAs; // May be set in style ">*powershell"
//
//			// May be directory was set in task properties?
//			pGrp->ParseGuiArgs(&con.pszStartupDir, NULL);
//
//			con.ProcessNewConArg();
//			
//			if (isPressed(VK_SHIFT))
//			{
//				int nRc = gpConEmu->RecreateDlg(&con);
//
//				if (nRc != IDC_START)
//					return;
//
//				CVConGroup::Redraw();
//			}
//			else
//			{
//				gpSet->HistoryAdd(con.pszSpecialCmd);
//			}
//
//			//����������, ������
//			if (gpSet->isMulti)
//				gpConEmu->CreateCon(&con, true);
//			else
//				gpConEmu->CreateWnd(&con);
//		}
//	}
//
//	SafeFree(pszDataW);
//}
//
//bool TabBarClass::OnMenuSelected(HMENU hMenu, WORD nID, WORD nFlags)
//{
//	if (mb_InNewConPopup)
//	{
//		if (nID >= 1 && nID <= countof(m_CmdPopupMenu) && m_CmdPopupMenu[nID-1].pszCmd)
//		{
//			LPCWSTR pszCmd = m_CmdPopupMenu[nID-1].pszCmd;
//			if (m_CmdPopupMenu[nID-1].szShort[0] && pszCmd && lstrcmp(pszCmd, m_CmdPopupMenu[nID-1].szShort))
//			{
//				POINT pt; GetCursorPos(&pt);
//				RECT rcMenuItem = {};
//				BOOL lbMenuItemPos = FALSE;
//				if (nFlags & MF_POPUP)
//				{
//					lbMenuItemPos = GetMenuItemRect(ghWnd, hMenu, nID, &rcMenuItem);
//				}
//				else
//				{
//					for (int i = 0; i < 100; i++)
//					{
//						if (GetMenuItemID(hMenu, i) == nID)
//						{
//							lbMenuItemPos = GetMenuItemRect(ghWnd, hMenu, i, &rcMenuItem);
//							break;
//						}
//					}
//				}
//				if (lbMenuItemPos)
//				{
//					pt.x = rcMenuItem.left + (rcMenuItem.bottom - rcMenuItem.top)*2; //(rcMenuItem.left + rcMenuItem.right) >> 1;
//					pt.y = rcMenuItem.bottom;
//				}
//				
//				gpConEmu->mp_Tip->ShowTip(ghWnd, ghWnd, pszCmd, TRUE, pt, g_hInstance);
//			}
//			else
//			{
//				gpConEmu->mp_Tip->HideTip();
//			}
//		}
//		else
//		{
//			gpConEmu->mp_Tip->HideTip();
//		}
//		return true;
//	}
//	return false;
//}

int TabBarClass::ActiveTabByName(int anType, LPCWSTR asName, CVirtualConsole** ppVCon)
{
	int nTab = -1;
	CVirtualConsole *pVCon = NULL;
	if (ppVCon)
		*ppVCon = NULL;
	
	TODO("TabBarClass::ActiveTabByName - ����� ��� �� �����");

	INT_PTR V, I;
	int tabIdx = 0;
	ConEmuTab tab = {0};
	for (V = 0; V < MAX_CONSOLE_COUNT && nTab == -1; V++)
	{
		if (!(pVCon = gpConEmu->GetVCon(V))) continue;

		#ifdef _DEBUG
		BOOL lbActive = gpConEmu->isActive(pVCon, false);
		#endif

		//111120 - ��� ����� ����������. ���� �������� ������ � ������ ������� - ���������� �� �����
		//if (gpSet->bHideInactiveConsoleTabs)
		//{
		//	if (!lbActive) continue;
		//}

		CRealConsole *pRCon = pVCon->RCon();

		for (I = 0; TRUE; I++)
		{
			if (!pRCon->GetTab(I, &tab))
				break;
			if (tab.Type == (anType & fwt_TypeMask))
			{
				LPCWSTR pszName = PointToName(tab.Name);
				if ((pszName && (lstrcmpi(pszName, asName) == 0)) || (lstrcmpi(tab.Name, asName) == 0))
				{
					nTab = tabIdx;
					break;
				}
			}

			tabIdx++;
		}
	}

	
	if (nTab >= 0)
	{
		if (!CanActivateTab(nTab))
		{
			nTab = -2;
		}
		else
		{
			pVCon = FarSendChangeTab(nTab);
			if (!pVCon)
				nTab = -2;
		}
	}
	
	if (ppVCon)
		*ppVCon = pVCon;
	
	return nTab;
}

void TabBarClass::UpdateTabFont()
{
	if (!mh_Tabbar)
		return;

	HFONT hFont = CreateFont(gpSet->nTabFontHeight, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, gpSet->nTabFontCharSet, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, gpSet->sTabFontFace);
	
	SendMessage(mh_Tabbar, WM_SETFONT, WPARAM(hFont), TRUE);

	if (mh_TabFont)
		DeleteObject(mh_TabFont);
	mh_TabFont = hFont;
}

// ������������� � ���������� ����������� ghWnd!
bool TabBarClass::GetRebarClientRect(RECT* rc)
{
	if (!IsTabsShown())
		return false;

	HWND hWnd = mh_Rebar ? mh_Rebar : mh_Tabbar;
	GetWindowRect(hWnd, rc);
	MapWindowPoints(NULL, ghWnd, (LPPOINT)rc, 2);

	return true;
}

void TabBarClass::GetActiveTabRect(RECT* rcTab)
{
	if (!IsTabsShown())
	{
		_ASSERTE(IsTabsShown());
		memset(rcTab, 0, sizeof(*rcTab));
	}
	else
	{
		TODO("TabBar: ���������� ����� '����� �����'");
		int iSel = TabCtrl_GetCurSel(mh_Tabbar);
		TabCtrl_GetItemRect(mh_Tabbar, iSel, rcTab);
		MapWindowPoints(mh_Tabbar, NULL, (LPPOINT)rcTab, 2);
		RECT rcBar = {}; GetWindowRect(mh_Rebar, &rcBar);
		rcTab->bottom = min(rcTab->bottom, rcBar.bottom);
		rcTab->left = max(rcTab->left-2, rcBar.left);
	}
}

bool TabBarClass::Toolbar_GetBtnRect(int nCmd, RECT* rcBtnRect)
{
	if (!IsTabsShown())
	{
		return false;
	}
	if (!mh_Toolbar)
	{
		_ASSERTE(mh_Toolbar);
		return false;
	}
	
	SendMessage(mh_Toolbar, TB_GETRECT, nCmd/*�������� TID_CREATE_CON*/, (LPARAM)rcBtnRect);
	MapWindowPoints(mh_Toolbar, ghWnd, (LPPOINT)rcBtnRect, 2);
	return true;
}

#endif // #if !defined(CONEMU_TABBAR_EX)
