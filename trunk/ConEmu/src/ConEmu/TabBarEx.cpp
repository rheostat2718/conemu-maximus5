
/*
Copyright (c) 2012 Maximus5
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

#pragma message("TabBarEx.cpp...")

#ifndef HIDE_USE_EXCEPTION_INFO
#define HIDE_USE_EXCEPTION_INFO
#endif

#ifndef SHOWDEBUGSTR
#define SHOWDEBUGSTR
#endif

#define DEBUGSTRTABS(s) //DEBUGSTR(s)

#include <windows.h>
#include <commctrl.h>
#include "header.h"
#include "TabBarEx.h"
#include "Options.h"
#include "ConEmu.h"
#include "VirtualConsole.h"
#include "TrayIcon.h"
#include "VConChild.h"
#include "VConGroup.h"
#include "Status.h"
#include "Menu.h"

// ���� �� ����� � ����� - ���
#ifdef USE_CONEMU_TOOLBAR
#include "ToolBarClass.cpp"
#endif


#define MIN_TAB_WIDTH 30
#define POST_UPDATE_TIMEOUT 2000
#define TAB_CANT_BE_ACTIVATED L"This tab can't be activated now!"

#define TOOL_TAB_SPACING 5   // ���������� ����� ������ � ����� toolbar-��
#define TAB_BUTTON_SPACING 10 // ���������� ����� ����� toolbar-�� (��� ������, ���� ��� �� �����) � �������� ���������

#define FAILED_TABBAR_TIMERID 101
#define FAILED_TABBAR_TIMEOUT 3000

#ifndef DEBUGSTRTABS
	#define DEBUGSTRTABS(s)
#endif


#ifdef _DEBUG
	//#define DEBUG_NO_DRAW_TABS
	#undef DEBUG_NO_DRAW_TABS
	//#define DRAW_CAPTION_MARKERS // ��� �������
	#undef DRAW_CAPTION_MARKERS
	#define DIRECT_CAP_PAINT
	//#undef DIRECT_CAP_PAINT
	//#define DEBUG_SHOW_RECT
	#undef DEBUG_SHOW_RECT
#else
	#undef DEBUG_NO_DRAW_TABS
	#undef DRAW_CAPTION_MARKERS
	#undef DIRECT_CAP_PAINT
	#undef DEBUG_SHOW_RECT
#endif



extern HICON hClassIconSm;

//TODO: �������� �� "ConEmu XXXXXX"
#define DefaultTitle gpConEmu->GetDefaultTitle();
#define SafeTitle(t) (((t)!=NULL) ? (t) : DefaultTitle )



TabBarClass::TabBarClass()
{
	mb_Active = true;
	mb_UpdateModified = false;
	mb_ToolbarFit = false;
	mn_ActiveTab = 0;
	mn_HoverTab = -1;
	mn_TabWidth = MIN_TAB_WIDTH; // �������������, ����������� ������������ ����� ����������� � RepositionTabs
	mn_EdgeWidth = 4;
	ZeroStruct(mrc_Caption);
	ZeroStruct(mrc_Tabs);
	ZeroStruct(mrc_TabsClient);
	ZeroStruct(mrc_Toolbar);
	mh_Theme = NULL;
	pPixels = NULL;
	mn_InUpdate = 0;

	ZeroStruct(m_Colors);
	ZeroStruct(mh_Pens);
	ZeroStruct(mh_Brushes);

	//m_Colors[clrText] = RGB(0,0,0);
	//m_Colors[clrDisabledText] = RGB(96,96,96);
	//m_Colors[clrDisabledTextShadow] = RGB(255,255,255);
	//m_Colors[clrBorder] = RGB(107,165,189);
	//m_Colors[clrInactiveBorder] = RGB(148,148,165);

	//m_Colors[clrEdgeLeft] = RGB(255,255,247);
	//m_Colors[clrEdgeRight] = RGB(255,255,247);
	//m_Colors[clrInactiveEdgeLeft] = RGB(239,239,247);
	//m_Colors[clrInactiveEdgeRight] = RGB(140,173,206);

	//m_Colors[clrBackActive] = RGB(255,255,247);
	//m_Colors[clrBackActiveBottom] = RGB(214,231,247);
	//m_Colors[clrBackHover] = RGB(239,239,247);
	//m_Colors[clrBackHoverBottom] = RGB(132,206,247);
	//m_Colors[clrBackInactive] = RGB(239,239,247);
	//m_Colors[clrBackInactiveBottom] = RGB(156,181,214);
	//m_Colors[clrBackDisabled] = RGB(243,243,243);
	//m_Colors[clrBackDisabledBottom] = RGB(189,189,189);

	mb_PostUpdateCalled = mb_PostUpdateRequested = FALSE;
	mn_PostUpdateTick = 0;
	mb_InKeySwitching = FALSE;
	
	mh_Balloon = NULL;
	ZeroStruct(tiBalloon);
	ms_TabErrText[0] = 0;
	mh_TabIcons = 0;

	// *** ������� ��������� ������� ***
	InitToolbar();
}

void TabBarClass::InitToolbar()
{
	// *** ������� ��������� ������� ***
	LPARAM lParam = (LPARAM)(TabBarClass*)this;
	//mn_ToolbarBmp = IDB_MAIN_TOOLBAR;
	SIZE sz = {14,14};
	//m_ToolbarBmpSize = sz;

	mp_Toolbar = new CToolBarClass;
	mp_Toolbar->Lock();

	RECT rcBtn = {0,0,sz.cx-1,sz.cy-1};

	// Bitmap indexes defined in header.h: enum ToolbarMainBitmapIdx

	mp_Toolbar->CreatePane(TID_CREATE_CON, 1020, true, LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_MAIN_TOOLBAR)), lParam, OnToolbarCommand, OnToolbarMenu, OnToolBarDraw);
	rcBtn.left = BID_NEWCON_IDX*sz.cx; rcBtn.right = rcBtn.left + sz.cx - 1;
	mp_Toolbar->AddTool(TID_CREATE_CON, TID_CREATE_CON, rcBtn, TIS_DROPDOWN, L"Create new console");

	mp_Toolbar->CreatePane(TID_ACTIVE_NUMBER, 1010, true, LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_MAIN_TOOLBAR)), lParam, OnToolbarCommand, OnToolbarMenu, OnToolBarDraw);
	rcBtn.left = 0; rcBtn.right = rcBtn.left + sz.cx - 1;
	mp_Toolbar->AddTool(TID_ACTIVE_NUMBER, TID_ACTIVE_NUMBER, rcBtn, TIS_DROPDOWN|/*TIS_DRAWCALLBACK|*/TIS_TIPCALLBACK, NULL);
	
	mp_Toolbar->CreatePane(TID_ALTERNATIVE, 1000, true, LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_MAIN_TOOLBAR)), lParam, OnToolbarCommand, OnToolbarMenu, OnToolBarDraw);
	rcBtn.left = BID_ALTERNATIVE_IDX*sz.cx; rcBtn.right = rcBtn.left + sz.cx - 1;
	mp_Toolbar->AddTool(TID_ALTERNATIVE, TID_ALTERNATIVE, rcBtn, TIS_TIPCALLBACK, NULL);
	//mp_Toolbar->CheckTool(mn_ToolPaneOptions, mn_ToolCmdBuffer, false/*isAlternative()*/);

	{
	COLORMAP colorMap = {0xC0C0C0,GetSysColor(COLOR_BTNFACE)};
	HBITMAP hBmp = CreateMappedBitmap(g_hInstance, IDB_SCROLL, 0, &colorMap, 1);
	mp_Toolbar->CreatePane(TID_SCROLL, 1000, true, hBmp, lParam, OnToolbarCommand, OnToolbarMenu, OnToolBarDraw);
	rcBtn.left = 0; rcBtn.right = rcBtn.left + sz.cx - 1;
	mp_Toolbar->AddTool(TID_SCROLL, TID_SCROLL, rcBtn, TIS_TIPCALLBACK, NULL);
	//mp_Toolbar->CheckTool(mn_ToolPaneOptions, mn_ToolCmdBuffer, false/*isBufferHeight()*/);
	}


	mp_Toolbar->CreatePane(TID_APPCLOSE, 3000, true, LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_MAIN_TOOLBAR)), lParam, OnToolbarCommand, OnToolbarMenu, OnToolBarDraw);
	rcBtn.left = BID_MINIMIZE_IDX*sz.cx; rcBtn.right = rcBtn.left + sz.cx - 1;
	mp_Toolbar->AddTool(TID_APPCLOSE, TID_MINIMIZE, rcBtn, TIS_TIPCALLBACK, NULL);
	rcBtn.left = BID_MAXIMIZE_IDX*sz.cx; rcBtn.right = rcBtn.left + sz.cx - 1;
	mp_Toolbar->AddTool(TID_APPCLOSE, TID_MAXIMIZE, rcBtn, TIS_TIPCALLBACK, NULL);
	rcBtn.left = BID_APPCLOSE_IDX*sz.cx; rcBtn.right = rcBtn.left + sz.cx - 1;
	mp_Toolbar->AddTool(TID_APPCLOSE, TID_APPCLOSE, rcBtn, TIS_TIPCALLBACK, NULL);


	mp_Toolbar->Commit();
}

TabBarClass::~TabBarClass()
{
	// ���������� ��� ����
	m_Tabs.ReleaseTabs(FALSE);
	m_TabStack.ReleaseTabs(FALSE);

	SafeDelete(mp_Toolbar);
}

//bool TabBarClass::OnMenuSelected(HMENU hMenu, WORD nID, WORD nFlags)
//{
//	return false;
//}

void TabBarClass::Retrieve()
{
}

void TabBarClass::Reset()
{
}

//bool TabBarClass::IsTabsActive()
//{
//	//TODO
//	return true;
//}

//bool TabBarClass::IsTabsShown()
//{
//	//TODO
//	return true;
//}

RECT TabBarClass::GetMargins()
{
	_ASSERTE(this);
	RECT rcNewMargins = {0,0};

	if (IsTabsActive() || (gpSet->isTabs == 1))
	{
		int _tabHeight = GetTabbarHeight();

		if (_tabHeight)
		{
			if (gpSet->nTabsLocation == 1)
				rcNewMargins = MakeRect(0,0,0,_tabHeight);
			else
				rcNewMargins = MakeRect(0,_tabHeight,0,0);
		}
	}

	return rcNewMargins;
}

//void TabBarClass::Activate(BOOL abPreSyncConsole/*=FALSE*/)
//{
//}

//HWND TabBarClass::CreateToolbar()
//{
//}

//HWND TabBarClass::CreateTabbar(bool abDummyCreate /*= false*/)
//{
//	return NULL;
//}

//HWND TabBarClass::GetTabbar()
//{
//	_ASSERTE(FALSE && "TabBarClass::GetTabbar obsolete");
//	return NULL;
//}

int TabBarClass::GetTabbarHeight()
{
	return gpSet->nTabFontHeight + 9;
}

void TabBarClass::CreateRebar()
{
    if (!mh_TabIcons)
	{
  //  	mh_TabIcons = ImageList_LoadImage(g_hInstance, MAKEINTRESOURCE(IDB_SHIELD), 14, 1, RGB(128,0,0), IMAGE_BITMAP, LR_CREATEDIBSECTION);
		////mn_AdminIcon = (gpConEmu->m_osv.dwMajorVersion >= 6) ? 0 : 1;
    }
    
    //if (!mh_TabTip || !IsWindow(mh_TabTip))
	//{
    //    mh_TabTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
    //                          WS_POPUP | TTS_ALWAYSTIP /*| TTS_BALLOON*/ | TTS_NOPREFIX,
    //                          CW_USEDEFAULT, CW_USEDEFAULT,
    //                          CW_USEDEFAULT, CW_USEDEFAULT,
    //                          ghWnd, NULL, 
    //                          g_hInstance, NULL);
    //    SetWindowPos(mh_TabTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
    //}
    
	if (!mh_Balloon || !IsWindow(mh_Balloon))
	{
		mh_Balloon = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
			WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX,
			CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT,
			ghWnd, NULL, 
			g_hInstance, NULL);
		SetWindowPos(mh_Balloon, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
		// Set up tool information.
		// In this case, the "tool" is the entire parent window.
		tiBalloon.cbSize = 44; // ��� sizeof(TOOLINFO);
		tiBalloon.uFlags = TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE;
		tiBalloon.hwnd = ghWnd;
		tiBalloon.hinst = g_hInstance;
		//static wchar_t szAsterisk[] = L"*"; // eliminate GCC warning
		//tiBalloon.lpszText = szAsterisk;
		tiBalloon.uId = (UINT_PTR)TTF_IDISHWND;
		//GetClientRect (mh_Tabbar, &tiBalloon.rect);
		//// Associate the ToolTip with the tool window.
		//SendMessage(mh_Balloon, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &tiBalloon);
		// Allow multiline
		SendMessage(mh_Balloon, TTM_SETMAXTIPWIDTH, 0, (LPARAM)300);
	}
}

//void TabBarClass::Deactivate(BOOL abPreSyncConsole/*=FALSE*/)
//{
//}

void TabBarClass::Invalidate()
{
	TODO("�������� Frame/Caption?");
	// -- gpConEmu->RedrawFrame();
	InvalidateRect(ghWnd, NULL, TRUE);
}

void TabBarClass::RePaint()
{
}

bool TabBarClass::GetRebarClientRect(RECT* rc)
{
	if (!IsTabsShown())
		return false;

	RECT client = gpConEmu->GetGuiClientRect();
	int  height = GetTabbarHeight();

	int nNewY;
	if (gpSet->nTabsLocation == 1)
	{
		int nStatusHeight = gpSet->isStatusBarShow ? gpSet->StatusBarHeight() : 0;
		nNewY = client.bottom - nStatusHeight - height;
	}
	else
	{
		nNewY = client.top;
	}

	*rc = MakeRect(client.left, nNewY, client.right, nNewY + height);

	return true;
}

//BOOL TabBarClass::NeedPostUpdate()
//{
//	return FALSE;
//}

void TabBarClass::UpdatePosition()
{
}

void TabBarClass::UpdateTabFont()
{
}

void TabBarClass::Reposition()
{
}

void TabBarClass::OnConsoleActivated(int nConNumber) //0-based
{
}

void TabBarClass::UpdateToolConsoles(bool abForcePos/*=false*/)
{
}

void TabBarClass::OnCaptionHidden()
{
}

void TabBarClass::OnWindowStateChanged()
{
}

void TabBarClass::OnBufferHeight(BOOL abBufferHeight)
{
}

void TabBarClass::OnAlternative(BOOL abAlternative)
{
}

//LRESULT TabBarClass::OnNotify(LPNMHDR nmhdr)
//{
//	return 0;
//}

void TabBarClass::OnChooseTabPopup()
{
}

//void TabBarClass::OnNewConPopupMenu(POINT* ptWhere /*= NULL*/, DWORD nFlags /*= 0*/)
//{
//}
//
//void TabBarClass::OnNewConPopupMenuRClick(HMENU hMenu, UINT nItemPos)
//{
//}

void TabBarClass::OnCommand(WPARAM wParam, LPARAM lParam)
{
}

bool TabBarClass::OnMouse(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT &lResult)
{
	POINT ptCurClient = {(int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)};
	if ((uMsg == WM_MOUSEWHEEL) || (uMsg == 0x020E/*WM_MOUSEHWHEEL*/))
		MapWindowPoints(NULL, ghWnd, &ptCurClient, 1);

	if (!PtInRect(&mrc_TabsClient, ptCurClient))
	{
		if (mn_HoverTab >= 0)
		{
			HoverTab(-1);
			#if defined(USE_CONEMU_TOOLBAR)
			// � ������ ������ ���������, ���� ����
			mp_Toolbar->UnHover();
			#endif
		}

		return false;
	}

	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEMOVE:
		//if (wParam == HT_CONEMUTAB || wParam == HTCAPTION)
		{
			if (mb_ToolbarFit && PtInRect(&mrc_Toolbar, ptCurClient))
			{
				if (mn_HoverTab >= 0)
				{
					HoverTab(-1);
				}
				#if defined(USE_CONEMU_TOOLBAR)
				if (mp_Toolbar->MouseEvent(hWnd, uMsg, wParam, lParam, ptCurClient, lResult))
				{
					break;
				}
				#endif
				break;
			}
			else
			{
				#if defined(USE_CONEMU_TOOLBAR)
				// � ������ ������ ���������, ���� ����
				mp_Toolbar->UnHover();
				#endif
			}

			DWORD nTabFlags = 0;
			int nTab = TabFromCursor(ptCurClient, &nTabFlags);
			if (nTab >= 0)
			{
				//TODO: WM_MBUTTONDOWN
				if ((uMsg == WM_LBUTTONDOWN) || (uMsg == WM_RBUTTONUP))
				{
					if ((nTabFlags & fwt_Disabled))
					{
						ShowTabError(TAB_CANT_BE_ACTIVATED, nTab);
					}
					else
					{
						if (OnTabSelected(nTab))
						{
							CVConGuard VCon;
							if ((uMsg == WM_RBUTTONUP) && (gpConEmu->GetActiveVCon(&VCon) >= 0))
							{
								POINT ptCur; GetCursorPos(&ptCur);
								gpConEmu->mp_Menu->ShowPopupMenu(VCon.VCon(), ptCur);
							}
						}
					}
				}
				else if (uMsg == WM_MOUSEMOVE)
				{
					// check hover - mn_HoverTab
					int nNewHover = (mn_ActiveTab == nTab) ? -1 : nTab;
					if (nNewHover != mn_HoverTab)
					{
						HoverTab(nNewHover);
					}
				}
				else if (uMsg == WM_MBUTTONUP)
				{
					// ������� ���?
				}
				break;
			}
			else if (mn_HoverTab >= 0)
			{
				HoverTab(-1);
			}
		}
		break;

	case WM_MOUSEHOVER:
		// �������� ������, ���� ����� ��� �������
		break;

	case WM_MOUSELEAVE:
		if (mn_HoverTab >= 0)
		{
			HoverTab(-1);
			#if defined(USE_CONEMU_TOOLBAR)
			// � ������ ������ ���������, ���� ����
			mp_Toolbar->UnHover();
			#endif
		}
		break;

	}

	return true; // ������� ��� ����� �������, ������ �� ������������!
}

// ������������ �����
//void TabBarClass::Switch(BOOL abForward, BOOL abAltStyle/*=FALSE*/)
//{
//}
//
//void TabBarClass::SwitchNext(BOOL abAltStyle/*=FALSE*/)
//{
//}
//
//void TabBarClass::SwitchPrev(BOOL abAltStyle/*=FALSE*/)
//{
//}
//
//BOOL TabBarClass::IsInSwitch()
//{
//	return FALSE;
//}
//
//void TabBarClass::SwitchCommit()
//{
//}
//
//void TabBarClass::SwitchRollback()
//{
//}

BOOL TabBarClass::OnKeyboard(UINT messg, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}

void TabBarClass::SetRedraw(BOOL abEnableRedraw)
{
}

int  TabBarClass::ActiveTabByName(int anType, LPCWSTR asName, CVirtualConsole** ppVCon)
{
	return 0;
}

void TabBarClass::GetActiveTabRect(RECT* rcTab)
{
}



/* ************************** */
/*  Owner-draw tab paintings
/* ************************** */

void TabBarClass::PreparePalette()
{
	switch (gpSet->nTabStyle)
	{
	case ts_Win8:
		{
			m_Colors[clrText] = RGB(0,0,0);
			m_Colors[clrDisabledText] = RGB(96,96,96);
			m_Colors[clrDisabledTextShadow] = RGB(255,255,255);
			m_Colors[clrBorder] = RGB(172,172,172);
			m_Colors[clrHoverBorder] = RGB(126,180,234);
			m_Colors[clrInactiveBorder] = RGB(172,172,172);

			// Not used in this theme
			m_Colors[clrEdgeLeft] = RGB(0,0,0);
			m_Colors[clrEdgeRight] = RGB(0,0,0);
			m_Colors[clrInactiveEdgeLeft] = RGB(0,0,0);
			m_Colors[clrInactiveEdgeRight] = RGB(0,0,0);

			m_Colors[clrBackActive] = RGB(255,255,255);
			m_Colors[clrBackActiveBottom] = RGB(255,255,255);
			m_Colors[clrBackHover] = RGB(236,244,252);
			m_Colors[clrBackHoverBottom] = RGB(221,237,252);
			m_Colors[clrBackInactive] = RGB(240,240,240);
			m_Colors[clrBackInactiveBottom] = RGB(230,230,230);
			m_Colors[clrBackDisabled] = RGB(240,240,240);
			m_Colors[clrBackDisabledBottom] = RGB(230,230,230);
		}
		break;
	default:
		{
			m_Colors[clrText] = RGB(0,0,0);
			m_Colors[clrDisabledText] = RGB(96,96,96);
			m_Colors[clrDisabledTextShadow] = RGB(255,255,255);
			m_Colors[clrBorder] = RGB(107,165,189);
			m_Colors[clrHoverBorder] = RGB(107,165,189);
			m_Colors[clrInactiveBorder] = RGB(148,148,165);

			m_Colors[clrEdgeLeft] = RGB(255,255,247);
			m_Colors[clrEdgeRight] = RGB(255,255,247);
			m_Colors[clrInactiveEdgeLeft] = RGB(239,239,247);
			m_Colors[clrInactiveEdgeRight] = RGB(140,173,206);

			m_Colors[clrBackActive] = RGB(255,255,247);
			m_Colors[clrBackActiveBottom] = RGB(214,231,247);
			m_Colors[clrBackHover] = RGB(239,239,247);
			m_Colors[clrBackHoverBottom] = RGB(132,206,247);
			m_Colors[clrBackInactive] = RGB(239,239,247);
			m_Colors[clrBackInactiveBottom] = RGB(156,181,214);
			m_Colors[clrBackDisabled] = RGB(243,243,243);
			m_Colors[clrBackDisabledBottom] = RGB(189,189,189);
		}
	}
}

void TabBarClass::CreateStockObjects(HDC hdc, const RECT &rcTabs)
{
	if (m_TabDrawStyle == fdt_Themed)
	{
		mh_Theme = gpConEmu->OpenThemeData(NULL, L"WINDOW"); 
	}

	PreparePalette();

	for (int i = 0; i <= clrColorIndexMax; i++)
	{
		mh_Pens[i] = CreatePen(PS_SOLID, 1, m_Colors[i]);
		mh_Brushes[i] = CreateSolidBrush(m_Colors[i]);
	}

	mh_OldBrush = (HBRUSH)SelectObject(hdc, mh_Brushes[0]);
	mh_OldPen = (HPEN)SelectObject(hdc, mh_Pens[0]);

	mh_Font = CreateTabFont(rcTabs.bottom-rcTabs.top);
	mh_OldFont = (HFONT)SelectObject(hdc, mh_Font);

	SetTextColor(hdc, m_Colors[clrText]);
	SetBkMode(hdc, TRANSPARENT);

	// ��������� ������ ������������� ������
	GetTextExtentPoint32(hdc, L"Ay", 2, &m_TabFontSize);
	switch (gpSet->nTabStyle)
	{
	case ts_Win8:
		if (gpSet->nTabsLocation == 0)
			mn_TextShift = 1+((rcTabs.bottom-rcTabs.top+1) - m_TabFontSize.cy)/2;
		else
			mn_TextShift = 3+((rcTabs.bottom-rcTabs.top+5) - m_TabFontSize.cy)/2;
		break;
	default:
		mn_TextShift = ((rcTabs.bottom-rcTabs.top+1) - m_TabFontSize.cy)/2;
	}
	if (mn_TextShift < 1) mn_TextShift = 1;
}

void TabBarClass::DeleteStockObjects(HDC hdc)
{
	SelectObject(hdc, mh_OldPen);
	SelectObject(hdc, mh_OldBrush);
	SelectObject(hdc, mh_OldFont);
	for (int i = 0; i <= clrColorIndexMax; i++)
	{
		if (mh_Pens[i]) { DeleteObject(mh_Pens[i]); mh_Pens[i] = NULL; }
		if (mh_Brushes[i]) { DeleteObject(mh_Brushes[i]); mh_Brushes[i] = NULL; }
	}
	if (mh_Font) { DeleteObject(mh_Font); mh_Font = NULL; }

	if (mh_Theme)
	{
		gpConEmu->CloseThemeData(mh_Theme);
		mh_Theme = NULL;
	}
}

void TabBarClass::RepositionTabs(const RECT &rcCaption, const RECT &rcTabs)
{
	//TODO: ���� ����� ��� ������� ����� - �� ����������� ������ ����?

	switch (gpSet->nTabStyle)
	{
	case ts_Win8:
		mn_EdgeWidth = (rcTabs.bottom - rcTabs.top) / 4;
		break;
	default:
		mn_EdgeWidth = (rcTabs.bottom - rcTabs.top) * 2 / 3;
	}

	int nTabCount = m_Tabs.GetCount();
	int nAllWidth = rcTabs.right - rcTabs.left - TAB_BUTTON_SPACING - mn_EdgeWidth;
	int nToolbarWidth = 0;
	bool bToolbarFit = false;

	#if defined(USE_CONEMU_TOOLBAR)
	if (gpSet->isMultiShowButtons)
	{
		int nMaxToolbarWidth = nAllWidth - nTabCount*MIN_TAB_WIDTH - TOOL_TAB_SPACING;
		if (nMaxToolbarWidth > 0)
		{
			nToolbarWidth = mp_Toolbar->GetWidth(nMaxToolbarWidth);
			if (nToolbarWidth > 0)
			{
				bToolbarFit = true;
				mrc_Toolbar = rcTabs;
				//mrc_Toolbar.right -= TAB_BUTTON_SPACING; -- ������ �� ������������ ������ � ���������, ���� ��� �������� ������
				mrc_Toolbar.left = mrc_Toolbar.right - nToolbarWidth;
				mrc_Toolbar.top = rcTabs.bottom - mp_Toolbar->GetHeight();
				if (gpConEmu->DrawType() == fdt_Aero)
				{
					if (mrc_Toolbar.top < (rcCaption.top+2))
						mrc_Toolbar.top = (rcCaption.top+2); // 2pix �� ���������/��������� ������ ������
				}
				else
				{
					if (mrc_Toolbar.top < (rcCaption.top+2))
						mrc_Toolbar.top = (rcCaption.top+2); // 2pix �� ���������/��������� ������ ������
				}
				nAllWidth -= (nToolbarWidth + TOOL_TAB_SPACING);
			}
		}
	}
	#endif
	mb_ToolbarFit = bToolbarFit;

	if (nTabCount > 0)
	{
		mn_TabWidth = (nAllWidth / nTabCount);
		if (mn_TabWidth < MIN_TAB_WIDTH)
			mn_TabWidth = MIN_TAB_WIDTH;
		else if ((DWORD)mn_TabWidth > gpSet->nTabWidthMax)
			mn_TabWidth = gpSet->nTabWidthMax;
	}
	else
	{
		mn_TabWidth = min(nAllWidth, (int)gpSet->nTabWidthMax);
	}
	
	RECT rcTab;
	switch (gpSet->nTabStyle)
	{
	case ts_Win8:
		rcTab = MakeRect(rcTabs.left+2, rcTabs.top, rcTabs.left+mn_TabWidth+1, rcTabs.bottom);
		break;
	default:
		rcTab = MakeRect(rcTabs.left, rcTabs.top, rcTabs.left+mn_TabWidth+(mn_EdgeWidth / 3), rcTabs.bottom);
	}

	int nRectWidth = rcTab.right - rcTab.left;
	int nTabShift = mn_TabWidth;
	// Try to distribute non-used space between tabs (+1px per tab)
	int nAddShift = nAllWidth - mn_TabWidth*nTabCount;

	for (int i = 0; i < nTabCount && (rcTab.left+MIN_TAB_WIDTH-1) < rcTabs.right; i++)
	{
		int nCur1 = 0; //((i > 0) && (i == mn_ActiveTab)) ? 1 : 0;
		int nCur2 = 0; //((i == mn_ActiveTab) && ((i+1) < nTabCount)) ? 1 : 0;
		RECT rc = {rcTab.left-nCur1, rcTab.top, rcTab.right+((nAddShift>0)?1:0)+nCur2, rcTab.bottom};
		m_Tabs.SetTabDrawRect(i, rc);
		//OffsetRect(&rcTab, nTabShift+((nAddShift>0)?1:0), 0);
		rcTab.left = rc.right;
		rcTab.right = rcTab.left + nRectWidth;
		nAddShift--;
	}
}

void TabBarClass::PaintTabs(HDC hdc, const RECT &rcCaption, const RECT &rcTabs)
{
	//if ((rcTabs.right - rcTabs.left) < MIN_TAB_WIDTH)
	//	return;

	m_TabDrawStyle = gpConEmu->DrawType();

	MSectionLock CS;
	m_Tabs.LockTabs(&CS);
	
	RepositionTabs(rcCaption, rcTabs);
	
	mrc_Caption = rcCaption;
	mrc_Tabs = rcTabs;
	WARNING("������ ���� ��������� � ���������� �������, � �� � ���������!");
	mrc_TabsClient = rcTabs;

	PaintTabs_Common(hdc, rcCaption, rcTabs);

	CS.Unlock();

	#ifdef DRAW_CAPTION_MARKERS
	{
	SetBkMode(hdc, TRANSPARENT);
	HPEN hGR = CreatePen(PS_DOT,1,RGB(0,255,0));
	HPEN hOldP = (HPEN)SelectObject(hdc, hGR);
	MoveToEx(hdc, rcCaption.left, rcCaption.top, 0);
	LineTo(hdc, rcCaption.left, rcCaption.bottom+1);
	MoveToEx(hdc, rcCaption.right, rcCaption.top, 0);
	LineTo(hdc, rcCaption.right, rcCaption.bottom+1);
	HPEN hTP = CreatePen(PS_DOT,1,RGB(0,255,255));
	SelectObject(hdc, hGR);
	MoveToEx(hdc, rcTabs.left, rcTabs.top, 0);
	LineTo(hdc, rcTabs.left, rcTabs.bottom+1);
	MoveToEx(hdc, rcTabs.right, rcTabs.top, 0);
	LineTo(hdc, rcTabs.right, rcTabs.bottom+1);
	SelectObject(hdc, hOldP);
	DeleteObject(hGR); DeleteObject(hTP);
	}
	#endif

	#ifdef DEBUG_SHOW_RECT
	{
	HPEN hGR = CreatePen(PS_DOT,1,RGB(0,255,0));
	HPEN hOldP = (HPEN)SelectObject(hdc, hGR);
	POINT pt[] = {{rcTabs.left, rcTabs.top}, {rcTabs.right, rcTabs.top}, {rcTabs.right, rcTabs.bottom}, {rcTabs.left, rcTabs.bottom}, {rcTabs.left, rcTabs.top}};
	Polyline(hdc, pt, countof(pt));
	SelectObject(hdc, hOldP);
	DeleteObject(hGR);
	}
	#endif
}

HFONT TabBarClass::CreateTabFont(int anTabHeight)
{
	//HFONT hFont = CreateFont((anTabHeight*5/6), 0, 0, 0, 400, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 
	//	CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, FF_MODERN, L"Verdana");
	HFONT hFont = CreateFont(min(gpSet->nTabFontHeight,anTabHeight), 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, gpSet->nTabFontCharSet, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, gpSet->sTabFontFace);
	return hFont;
}

void TabBarClass::PaintFlush()
{
#ifdef _DEBUG
	GdiFlush();
#endif
}

// ������ ���
void TabBarClass::PaintCaption_Plain(HDC hdc, const RECT &rcCaption, const RECT &rcTabs)
{
	TODO("��������� ����� TabBar");

	bool lbSysColor = (gpSet->isStatusBarFlags & csf_SystemColors) == csf_SystemColors;
	bool lbFade = lbSysColor ? false : gpSet->isFadeInactive && !gpConEmu->isMeForeground(true);

	COLORREF crBack = lbSysColor ? GetSysColor(COLOR_3DFACE) : lbFade ? gpSet->GetFadeColor(gpSet->nStatusBarBack) : gpSet->nStatusBarBack;

	HBRUSH hBr = CreateSolidBrush(crBack);
	FillRect(hdc, &rcCaption, hBr);
	DeleteObject(hBr);
}

void TabBarClass::PaintCaption_2k(HDC hdc, const RECT &rcCaption, const RECT &rcTabs)
{
	//// ��� ����� �������� �� Aero
	//memset(pPixels, 0xFF, bi.biWidth*bi.biHeight*sizeof(*pPixels));


	BOOL lbActive = gpConEmu->mb_NcActive;
	BOOL lbGradient = TRUE;
	SystemParametersInfo(SPI_GETGRADIENTCAPTIONS, 0, &lbGradient, 0);
	
	// -- �� ��������
	//RECT rc = rcCaption;
	//DrawCaption(ghWnd, hdc, &rc, (lbActive?DC_ACTIVE:0)|DC_BUTTONS|(lbGradient?DC_GRADIENT:0));
	
	DWORD clrLeft = GetSysColor(lbActive ? COLOR_ACTIVECAPTION : COLOR_INACTIVECAPTION);
	DWORD clrRight = clrLeft;
	if (lbGradient)
	{
		clrRight = lbGradient ? GetSysColor(lbActive ? COLOR_GRADIENTACTIVECAPTION : COLOR_GRADIENTINACTIVECAPTION) : clrLeft;
		
		// �������� ���� ������ �� "�������" ������� ���������. 
		TRIVERTEX vtx[2] = {
			{rcTabs.left, rcCaption.top, GetRValue(clrLeft)<<8, GetGValue(clrLeft)<<8, GetBValue(clrLeft)<<8, 0},
			{rcTabs.right+1, rcCaption.bottom, GetRValue(clrRight)<<8, GetGValue(clrRight)<<8, GetBValue(clrRight)<<8, 0}
		};
		GRADIENT_RECT gRect = {0,1};
		
		//TODO: � ����� � �������?!
		if (!GdiGradientFill(hdc, vtx, 2, &gRect, 1, GRADIENT_FILL_RECT_H))
		{
			lbGradient = FALSE; // �������, ��� �������� ���������
			clrRight = clrLeft;
		}
		//PaintFlush();
	}
	
	if (!lbGradient)
	{
		HBRUSH br = (HBRUSH)GetSysColorBrush(lbActive ? COLOR_ACTIVECAPTION : COLOR_INACTIVECAPTION);
		//TODO: � ����� � �������?!
		RECT rc = {rcTabs.left, rcCaption.top, rcTabs.right, rcCaption.bottom};
		FillRect(hdc, &rc, br);
		//PaintFlush();
	}
	
	// ������
	{
		HBRUSH br = (HBRUSH)GetSysColorBrush(lbActive ? COLOR_ACTIVECAPTION : COLOR_INACTIVECAPTION);
		RECT rc = {rcCaption.left, rcCaption.top, rcTabs.left, rcTabs.bottom};
		FillRect(hdc, &rc, br);
		
		POINT IconShift;
		gpConEmu->GetIconShift(IconShift);
		PaintCaption_Icon(hdc, rcCaption.left+IconShift.x, rcCaption.top+IconShift.y);
	}
	
	// ������
	{
		HBRUSH br = (HBRUSH)GetSysColorBrush(lbActive 
			? (lbGradient?COLOR_GRADIENTACTIVECAPTION:COLOR_ACTIVECAPTION)
			: (lbGradient?COLOR_GRADIENTINACTIVECAPTION:COLOR_INACTIVECAPTION));
		RECT rc = {rcTabs.right, rcCaption.top, rcCaption.right+1, rcTabs.bottom+1};
		FillRect(hdc, &rc, br);
		
		int nBtnWidth = GetSystemMetrics(SM_CXSIZE);
		int nBtnHeight = GetSystemMetrics(SM_CYSIZE);
		RECT rcBtn = {0, rcCaption.top+2, 0, rcCaption.top+nBtnHeight-2};
		rcBtn.right = rcCaption.right - 1;
		rcBtn.left = rcBtn.right - nBtnWidth + 2;
		DrawFrameControl(hdc, &rcBtn, DFC_CAPTION, DFCS_CAPTIONCLOSE);
		rcBtn.right -= nBtnWidth; rcBtn.left -= nBtnWidth;
		DrawFrameControl(hdc, &rcBtn, DFC_CAPTION, (gpConEmu->isZoomed()?DFCS_CAPTIONRESTORE:DFCS_CAPTIONMAX));
		rcBtn.right -= (nBtnWidth - 2); rcBtn.left -= (nBtnWidth-2);
		DrawFrameControl(hdc, &rcBtn, DFC_CAPTION, DFCS_CAPTIONMIN);
	}
}

void TabBarClass::PaintCaption_XP(HDC hdc, const RECT &rcCaption, const RECT &rcTabs)
{
	BOOL lbActive = gpConEmu->mb_NcActive;
	
	RECT wr; GetWindowRect(ghWnd, &wr);
	RECT cr = {0,0,wr.right-wr.left,rcTabs.bottom+1};
	HRESULT hr;
	BOOL lbZoomed = gpConEmu->isZoomed();
	
#ifdef _DEBUG
	#define TMT_OFFSET	3401
	#define TMT_NORMALSIZE	3409
	POINT pt1;
	HRESULT hr1 = gpConEmu->GetThemePosition(mh_Theme, 1/*WP_CAPTION*/, gpConEmu->mb_NcActive ? 1/*CS_ACTIVE*/ : 2/*CS_INACTIVE*/,
		TMT_OFFSET, &pt1);
	POINT pt2;
	HRESULT hr2 = gpConEmu->GetThemePosition(mh_Theme, 1/*WP_CAPTION*/, gpConEmu->mb_NcActive ? 1/*CS_ACTIVE*/ : 2/*CS_INACTIVE*/,
		TMT_NORMALSIZE, &pt2);
	SIZE sz3;
	HRESULT hr3 = gpConEmu->GetThemePartSize(mh_Theme, hdc, 1/*WP_CAPTION*/, gpConEmu->mb_NcActive ? 1/*CS_ACTIVE*/ : 2/*CS_INACTIVE*/,
		NULL, 2, &sz3);
	int i1 = gpConEmu->GetThemeSysSize(mh_Theme, SM_CYSIZE); // Specifies the height of a caption.
	int i1_ = GetSystemMetrics(SM_CYSIZE);
	int i2 = gpConEmu->GetThemeSysSize(mh_Theme, 92/*SM_CXPADDEDBORDER*/); // Specifies the amount of border padding for captioned windows.
	int i2_ = GetSystemMetrics(92/*SM_CXPADDEDBORDER*/);
	//int i3 = gpConEmu->GetThemeSysSize(mh_Theme, SM_CYPADDEDBORDER); // Specifies the amount of border padding for captioned windows.
	int i4 = gpConEmu->GetThemeSysSize(mh_Theme, SM_CXBORDER); // Specifies the width of a border.
	int i5 = gpConEmu->GetThemeSysSize(mh_Theme, SM_CYBORDER); // Specifies the ??? of a border.
	int i6 = gpConEmu->GetThemeSysSize(mh_Theme, SM_CXSIZEFRAME); // Specifies the width of a border.
	int i6_ = GetSystemMetrics(SM_CXSIZEFRAME);
	int i7 = gpConEmu->GetThemeSysSize(mh_Theme, SM_CYSIZEFRAME); // Specifies the ??? of a border.
	int i7_ = GetSystemMetrics(SM_CYSIZEFRAME);

	static bool bShown = true;
	if (!bShown)
	{
		bShown = true;
		wchar_t szInfo[1024];
		wsprintf(szInfo, L"Index          Themed/System\n"
			L"SM_CYSIZE   %i/%i\n"
			L"SM_CXPADDEDBORDER   %i/%i\n"
			L"SM_CXBORDER/SM_CYBORDER   %i/%i\n"
			L"SM_CXSIZEFRAME   %i/%i\n"
			L"SM_CYSIZEFRAME   %i/%i\n"
			,
			i1,i1_, i2,i2_, i4,i5, i6,i6_, i7,i7_);
		MessageBox(NULL, szInfo, L"Test", MB_ICONINFORMATION);
	}
#endif


	int nFrameWidth = gpConEmu->GetFrameWidth();
	int nFrameHeight = gpConEmu->GetFrameHeight();
	int nDefaultCaptionFrame = nFrameHeight + gpConEmu->GetWinCaptionHeight();
	if ((cr.bottom - cr.top) > nDefaultCaptionFrame)
	{
		HDC hdcPaint = CreateCompatibleDC(hdc);
		HBITMAP hbmpPaint = CreateCompatibleBitmap(hdc, wr.right-wr.left+1, cr.bottom);
		HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcPaint, hbmpPaint);
		RECT rcPaint = {0, 0, wr.right-wr.left, nDefaultCaptionFrame};

		hr = gpConEmu->DrawThemeBackground(mh_Theme, hdcPaint,
					lbZoomed ? 5/*WP_MAXCAPTION*/ : 1/*WP_CAPTION*/, gpConEmu->mb_NcActive ? 1/*CS_ACTIVE*/ : 2/*CS_INACTIVE*/,
					&rcPaint, &rcPaint);
					
		StretchBlt(hdc, cr.left, cr.top, cr.right-cr.left, cr.bottom-cr.top,
			hdcPaint, 0/*nFrameWidth*/, nFrameHeight-1, rcPaint.right/*-2*nFrameWidth*/, rcPaint.bottom-nFrameHeight, SRCCOPY);
		
		SelectObject(hdcPaint, hbmpOld);
		DeleteObject(hbmpPaint);
		DeleteDC(hdcPaint);
	}
	else
	{
		if (lbZoomed)
			cr.top += 4;

		hr = gpConEmu->DrawThemeBackground(mh_Theme, hdc,
					lbZoomed ? 5/*WP_MAXCAPTION*/ : 1/*WP_CAPTION*/, gpConEmu->mb_NcActive ? 1/*CS_ACTIVE*/ : 2/*CS_INACTIVE*/,
					&cr, &cr);
	}
	
	if (FAILED(hr))
	{
		HBRUSH br = (HBRUSH)GetSysColorBrush(lbActive ? COLOR_ACTIVECAPTION : COLOR_INACTIVECAPTION);
		FillRect(hdc, &rcCaption, br);
	}

	// ������
	{
		//HBRUSH br = (HBRUSH)GetSysColorBrush(lbActive ? COLOR_ACTIVECAPTION : COLOR_INACTIVECAPTION);
		//RECT rc = {rcCaption.left, rcCaption.top, rcTabs.left, rcTabs.bottom};
		//FillRect(hdc, &rc, br);
		
		POINT IconShift;
		gpConEmu->GetIconShift(IconShift);
		PaintCaption_Icon(hdc, rcCaption.left+IconShift.x, rcCaption.top+IconShift.y);
	}
	
	// ������
	{
		RECT rc = {}; // = {rcCaption.right-50, rcCaption.top, rcCaption.right, rcCaption.bottom};
		hr = gpConEmu->GetThemeBackgroundContentRect(mh_Theme, hdc, 18/*WP_CLOSEBUTTON*/, 1/*CBS_NORMAL*/, &wr, &rc);
		RECT mrg = {};
		RECT tmpRc = wr;
		hr = gpConEmu->GetThemeMargins(mh_Theme, hdc, 18/*WP_CLOSEBUTTON*/, 1/*CBS_NORMAL*/, 205/*TMT_MARGINS*/, &tmpRc, &mrg);
		SIZE sz = {};
		hr = gpConEmu->GetThemePartSize(mh_Theme, hdc, 18/*WP_CLOSEBUTTON*/, 1/*CBS_NORMAL*/, &tmpRc, 2/*TS_DRAW*/, &sz);
		
		
		rc.right = rcCaption.right - 1;
		rc.left = rc.right - GetSystemMetrics(SM_CXSIZE) + 4;
		rc.top = rcCaption.top + 2;
		rc.bottom = rc.top + GetSystemMetrics(SM_CYSIZE) - 4;
		hr = gpConEmu->DrawThemeBackground(mh_Theme, hdc,
					18/*WP_CLOSEBUTTON*/, gpConEmu->mb_NcActive?1/*CBS_NORMAL*/:5/*Inactive*/,
					&rc, NULL);
		OffsetRect(&rc, - GetSystemMetrics(SM_CXSIZE)+2, 0);
		hr = gpConEmu->DrawThemeBackground(mh_Theme, hdc,
					lbZoomed?21/*WP_RESTOREBUTTON*/:17/*WP_MAXBUTTON*/, gpConEmu->mb_NcActive?1/*CBS_NORMAL*/:5/*Inactive*/,
					&rc, NULL);
		OffsetRect(&rc, - GetSystemMetrics(SM_CXSIZE)+2, 0);
		hr = gpConEmu->DrawThemeBackground(mh_Theme, hdc,
					15/*WP_MINBUTTON*/, gpConEmu->mb_NcActive?1/*CBS_NORMAL*/:5/*Inactive*/,
					&rc, NULL);
	}
}

void TabBarClass::PaintCaption_Aero(HDC hdc, const RECT &rcCaption, const RECT &rcTabs)
{
	nColorMagic = 0x1ABCDEF;
	// bi.biHeight ��� Aero ������ ��� rcCaption.bottom!
	int nMax = bi.biWidth*bi.biHeight;
	for (int i = 0; i < nMax; i++)
	{
		pPixels[i] = nColorMagic;
	}

	POINT IconShift;
	gpConEmu->GetIconShift(IconShift);
	PaintCaption_Icon(hdc, rcCaption.left+IconShift.x, rcCaption.top+IconShift.y);
}

void TabBarClass::PaintCaption_Win8(HDC hdc, const RECT &rcCaption, const RECT &rcTabs)
{
	if (gpSet->isTabsInCaption)
	{
		return;
	}

	BOOL lbActive = gpConEmu->mb_NcActive;
	BOOL lbGradient = TRUE;
	SystemParametersInfo(SPI_GETGRADIENTCAPTIONS, 0, &lbGradient, 0);
	
	// -- �� ��������
	//RECT rc = rcCaption;
	//DrawCaption(ghWnd, hdc, &rc, (lbActive?DC_ACTIVE:0)|DC_BUTTONS|(lbGradient?DC_GRADIENT:0));
	
	HBRUSH br = (HBRUSH)GetSysColorBrush(lbActive ? COLOR_ACTIVECAPTION : COLOR_INACTIVECAPTION);
	//TODO: � ����� � �������?!
	RECT rc = {rcCaption.left, rcCaption.top, rcTabs.right, rcCaption.bottom};
	FillRect(hdc, &rc, br);
	
	// ������
	{
		HBRUSH br = (HBRUSH)GetSysColorBrush(lbActive ? COLOR_ACTIVECAPTION : COLOR_INACTIVECAPTION);
		RECT rc = {rcCaption.left, rcCaption.top, rcTabs.left, rcTabs.bottom};
		FillRect(hdc, &rc, br);
		
		POINT IconShift;
		gpConEmu->GetIconShift(IconShift);
		PaintCaption_Icon(hdc, rcCaption.left+IconShift.x, rcCaption.top+IconShift.y);
	}
	
	// ������
	{
		HBRUSH br = (HBRUSH)GetSysColorBrush(lbActive 
			? (lbGradient?COLOR_GRADIENTACTIVECAPTION:COLOR_ACTIVECAPTION)
			: (lbGradient?COLOR_GRADIENTINACTIVECAPTION:COLOR_INACTIVECAPTION));
		RECT rc = {rcTabs.right, rcCaption.top, rcCaption.right+1, rcTabs.bottom+1};
		FillRect(hdc, &rc, br);
		
		int nBtnWidth = GetSystemMetrics(SM_CXSIZE);
		int nBtnHeight = GetSystemMetrics(SM_CYSIZE);
		RECT rcBtn = {0, rcCaption.top+2, 0, rcCaption.top+nBtnHeight-2};
		rcBtn.right = rcCaption.right - 1;
		rcBtn.left = rcBtn.right - nBtnWidth + 2;
		DrawFrameControl(hdc, &rcBtn, DFC_CAPTION, DFCS_CAPTIONCLOSE);
		rcBtn.right -= nBtnWidth; rcBtn.left -= nBtnWidth;
		DrawFrameControl(hdc, &rcBtn, DFC_CAPTION, (gpConEmu->isZoomed()?DFCS_CAPTIONRESTORE:DFCS_CAPTIONMAX));
		rcBtn.right -= (nBtnWidth - 2); rcBtn.left -= (nBtnWidth-2);
		DrawFrameControl(hdc, &rcBtn, DFC_CAPTION, DFCS_CAPTIONMIN);
	}
}

void TabBarClass::PaintCaption_Icon(HDC hdc, int X, int Y)
{
	_ASSERTE(gpSet->isCaptionHidden()); // ������ ���������� ������ ��� ������� ���������!

	int nW = GetSystemMetrics(SM_CXSMICON);
	int nH = GetSystemMetrics(SM_CYSMICON);

	if (hClassIconSm && (mrc_Tabs.right > (X + nW)))
	{
		//TODO: �������� �� DrawThemeIcon
		DrawIconEx(hdc, X,Y, hClassIconSm, nW,nH, 0, 0, DI_IMAGE);
	}
}

//void TabBarClass::PaintTabs_2k(HDC hdc, const RECT &rcCaption, const RECT &rcTabs)
//{
//	HDC hdcPaint = CreateCompatibleDC(hdc);
//	HBITMAP hMemBmp = CreateCompatibleBitmap(hdc, rcCaption.right, rcCaption.bottom);
//	HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcPaint, hMemBmp);
//
//	CreateStockObjects(hdcPaint, rcTabs);
//
//	PaintCaption_2k(hdcPaint, rcCaption, rcTabs);
//	
//	
//	for (int i = mn_TabCount-1; i >= 0; i--)
//	{
//		if ((m_Tabs[i].rcTab.left+MIN_TAB_WIDTH-1) >= rcTabs.right)
//		{
//			//TODO: ���� ���� �� ������ - ���������� ���������?
//			continue; // ���� ��� �� �����
//		}
//		
//		if (i != mn_ActiveTab)
//		{
//			_ASSERTE((m_Tabs[i].nFlags & etfActive)==0);
//			PaintTab_2k(hdcPaint, m_Tabs[i].rcTab, m_Tabs+i);
//		}
//	}
//	
//	if (mn_ActiveTab >= 0 && mn_ActiveTab < mn_TabCount)
//	{
//		PaintTab_2k(hdcPaint, pActiveTab->rcTab, pActiveTab);
//	}
//	
//
//	DeleteStockObjects(hdcPaint);
//
//
//	// Blitting result!
//	Bit Blt(hdc, rcTabs.left, rcTabs.top, rcTabs.right-rcTabs.left, rcTabs.bottom-rcTabs.top, hdcPaint, rcTabs.left, rcTabs.top, SRCCOPY);
//
//	// Free memory DC	
//	SelectObject(hdcPaint, hOldBmp);
//	DeleteObject(hMemBmp);
//	DeleteDC(hdcPaint);
//}

//void TabBarClass::PaintTabs_XP(HDC hdc, const RECT &rcCaption, const RECT &rcTabs)
//{
//	mh_Theme = gpConEmu->OpenThemeData(NULL, L"WINDOW"); 
//	HDC hdcPaint = CreateCompatibleDC(hdc);
//	HBITMAP hMemBmp = CreateCompatibleBitmap(hdc, rcCaption.right, rcCaption.bottom);
//	HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcPaint, hMemBmp);
//
//	CreateStockObjects(hdcPaint, rcTabs);
//
//	PaintCaption_XP(hdc, rcCaption, rcTabs)
//
//	for (int i = mn_TabCount-1; i >= 0; i--)
//	{
//		if ((m_Tabs[i].rcTab.left+MIN_TAB_WIDTH-1) >= rcTabs.right)
//		{
//			//TODO: ���� ���� �� ������ - ���������� ���������?
//			continue; // ���� ��� �� �����
//		}
//		
//		if ((m_Tabs[i].nFlags & etfActive) && !pActiveTab)
//		{
//			pActiveTab = m_Tabs+i;
//		}
//		else
//		{
//			_ASSERTE((m_Tabs[i].nFlags & etfActive)==0);
//			PaintTab_XP(hdcPaint, m_Tabs[i].rcTab, m_Tabs+i);
//		}
//	}
//	
//	if (pActiveTab)
//	{
//		PaintTab_XP(hdcPaint, pActiveTab->rcTab, pActiveTab);
//	}
//	
//	DeleteStockObjects(hdcPaint);
//	
//	// Blitting result!
//	Bit Blt(hdc, rcTabs.left, rcTabs.top, rcTabs.right-rcTabs.left, rcTabs.bottom-rcTabs.top, hdcPaint, rcTabs.left, rcTabs.top, SRCCOPY);
//
//	// Free memory DC	
//	SelectObject(hdcPaint, hOldBmp);
//	DeleteObject(hMemBmp);
//	DeleteDC(hdcPaint);
//}

//void TabBarClass::PaintTabs_Aero(HDC hdc, const RECT &rcCaption, const RECT &rcTabs)
//{
//	memset(&bi, 0, sizeof(bi));
//	bi.biSize = sizeof(BITMAPINFOHEADER);
//	bi.biWidth = rcCaption.right;
//#ifdef _DEBUG
//	int nBottom = gpConEmu->GetDwmClientRectTopOffset();
//	_ASSERTE(rcCaption.bottom >= nBottom);
//	//if (rcCaption.bottom > nBottom) nBottom = rcCaption.bottom;
//#endif
//	bi.biHeight = rcCaption.bottom;
//	bi.biPlanes = 1;
//	bi.biBitCount = 32;
//	pPixels = NULL;
//	HDC hdcPaint = CreateCompatibleDC(hdc);
//	HBITMAP hbmp = CreateDIBSection(hdcPaint, (BITMAPINFO*)&bi, DIB_RGB_COLORS, (void**)&pPixels, NULL, 0);
//	HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcPaint, hbmp);
//
//	CreateStockObjects(hdcPaint, rcTabs);
//
//	PaintCaption_Aero(hdcPaint, rcCaption, rcTabs);
//
//	for (int i = mn_TabCount-1; i >= 0; i--)
//	{
//		if ((m_Tabs[i].rcTab.left+MIN_TAB_WIDTH-1) >= rcTabs.right)
//		{
//			//TODO: ���� ���� �� ������ - ���������� ���������?
//			continue; // ���� ��� �� �����
//		}
//		
//		if ((m_Tabs[i].nFlags & etfActive) && !pActiveTab)
//		{
//			pActiveTab = m_Tabs+i;
//		}
//		else
//		{
//			_ASSERTE((m_Tabs[i].nFlags & etfActive)==0);
//			PaintTab_XP(hdcPaint, m_Tabs[i].rcTab, m_Tabs+i);
//		}
//	}
//	
//	if (pActiveTab)
//	{
//		// Aero!
//		PaintTab_XP(hdcPaint, pActiveTab->rcTab, pActiveTab);
//	}
//	
//
//	DeleteStockObjects(hdcPaint);
//
//
//	
//	for (int i = 0; i < nMax; i++)
//	{
//		if (pPixels[i] == nColorMagic)
//			pPixels[i] = 0;
//		else
//			pPixels[i] |= 0xFF000000;
//	}
//
//
//	// ������� ����� ������ ���� ������� ����� ����, ��� ��������� ����� Aero ����� "�������"
//	Bit Blt(hdc, 0, 0, bi.biWidth, bi.biHeight, hdcPaint, 0, 0, SRCCOPY);
//
//	SelectObject(hdcPaint, hOldBmp);
//	DeleteObject(hbmp);
//	DeleteDC(hdcPaint);
//}

void TabBarClass::PaintTabs_Common(HDC hdc, const RECT &rcCaption, const RECT &rcTabs)
{
	memset(&bi, 0, sizeof(bi));
	bi.biSize = sizeof(BITMAPINFOHEADER);
	RECT wr; GetWindowRect(ghWnd, &wr);
	bi.biWidth = (wr.right-wr.left);
	// bi.biHeight ��� Aero ������ ��� rcCaption.bottom!
	int nBottom = rcCaption.bottom; //(m_TabDrawStyle == fdt_Aero) ? gpConEmu->GetDwmClientRectTopOffset() : rcCaption.bottom;
	if (rcCaption.bottom > nBottom) nBottom = rcCaption.bottom;
	bi.biHeight = nBottom;
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	pPixels = NULL;
	HDC hdcPaint = CreateCompatibleDC(hdc);
	HBITMAP hbmp = CreateDIBSection(hdcPaint, (BITMAPINFO*)&bi, DIB_RGB_COLORS, (void**)&pPixels, NULL, 0);
	HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcPaint, hbmp);

	CreateStockObjects(hdcPaint, rcTabs);

	bool bCaptionHidden = gpSet->isCaptionHidden();

	if (!gpSet->isTabsInCaption)
	{
		if (!bCaptionHidden)
		{
			PaintCaption_Plain(hdcPaint, rcCaption, rcTabs);
		}
		else
		{
			switch (m_TabDrawStyle)
			{
			case fdt_Win8:
				PaintCaption_Win8(hdcPaint, rcCaption, rcTabs);
				break;
			case fdt_Aero:
				PaintCaption_Aero(hdcPaint, rcCaption, rcTabs);
				break;
			case fdt_Themed:
				PaintCaption_XP(hdcPaint, rcCaption, rcTabs);
				break;
			default:
				PaintCaption_2k(hdcPaint, rcCaption, rcTabs);
			}
		}
	}

	RECT rcActive = {}; //TODO: ����� �� ������� ��������, �.�. ���������� ������� �� ����������� ����?
	RECT rcHover = {}; //TODO: ����� �� ������� ��������, �.�. ���������� ������� �� ����������� ����?
#ifndef DEBUG_NO_DRAW_TABS

	CTab Tab;
	int nTabCount = m_Tabs.GetCount();
	
	for (int i = nTabCount-1; i >= 0; i--)
	{
		if (!m_Tabs.GetTabByIndex(i, /*OUT*/ Tab))
		{
			_ASSERTE(FALSE); // ��� �����?
			continue;
		}
		if ((Tab->DrawInfo.rcTab.left+MIN_TAB_WIDTH-1) >= rcTabs.right)
		{
			//TODO: ���� ���� �� ������ - ���������� ���������?
			continue; // ���� ��� �� �����
		}
		
		if (i != mn_ActiveTab)
		{
			//TODO: �������������� ���������? ��� �� �����?
			Tab->DrawInfo.Display.Set(Tab->Name.Ptr());
			PaintTab_Common(hdcPaint, Tab->DrawInfo.rcTab, &Tab->DrawInfo, Tab->Flags(), FALSE/*bCurrent*/, (i == mn_HoverTab && i != mn_ActiveTab)/*bHover*/);
			if (i == mn_HoverTab)
				rcHover = Tab->DrawInfo.rcTab;
		}
	}
	
	if (mn_ActiveTab >= 0 && mn_ActiveTab < nTabCount)
	{
		if (m_Tabs.GetTabByIndex(mn_ActiveTab, /*OUT*/ Tab))
		{
			if ((Tab->DrawInfo.rcTab.left+MIN_TAB_WIDTH-1) < rcTabs.right)
			{
				//TODO: �������������� ���������? ��� �� �����?
				Tab->DrawInfo.Display.Set(Tab->Name.Ptr());
				PaintTab_Common(hdcPaint, Tab->DrawInfo.rcTab, &Tab->DrawInfo, Tab->Flags(), TRUE/*bCurrent*/, FALSE/*bHover*/);
				rcActive = Tab->DrawInfo.rcTab;
			}
		}
	}
	
#endif

	DeleteStockObjects(hdcPaint);

	#if defined(USE_CONEMU_TOOLBAR)
	if (mb_ToolbarFit)
	{
		mp_Toolbar->Paint(hdcPaint, mrc_Toolbar);
	}
	#endif

	// ��� Aero - ����� �������� ������������ ���������
	//TODO: ��� ���������� Aero � ���� �������� "�����" (���-�� �� �����-������)
	if (m_TabDrawStyle == fdt_Aero || gpConEmu->IsDwmAllowed())
	{
		#ifdef _DEBUG
		int nMax = bi.biWidth*bi.biHeight;
		#endif
		int i = 0;
		DWORD nMaskActive   = 0xFF000000; //gpConEmu->mb_NcActive ? 0xFF000000 : 0xA0000000;
		DWORD nMaskInactive = 0xFF000000; //0xA0000000; //gpConEmu->mb_NcActive ? 0xA0000000 : 0x80000000;
		DWORD nToolMask     = 0xA0000000;
		int nX1 = rcActive.left, nX2 = rcActive.right;
		int nX3 = rcHover.left,  nX4 = rcHover.right;
		for (int y = 0; y < bi.biHeight; y++)
		{
			for (int x = 0; x < bi.biWidth; x++, i++)
			{
				if (pPixels[i] == nColorMagic)
					pPixels[i] = 0;
				else if (mb_ToolbarFit && x >= mrc_Toolbar.left)
					pPixels[i] |= nToolMask;
				else
					pPixels[i] |= nMaskActive;
				//else if ((x > nX1 && x <= nX2) || (x > nX3 && x <= nX4))
				//	pPixels[i] |= nMaskActive;
				//else
				//	pPixels[i] |= nMaskInactive;
			}
		}
	}

	// Blitting result!
	if (m_TabDrawStyle == fdt_Aero)
	{
		// ������� ����� ������ ���� ������� ����� ����, ��� ��������� ����� Aero ����� "�������"
		BitBlt(hdc, 0, 0, bi.biWidth, bi.biHeight, hdcPaint, 0, 0, SRCCOPY);
	}
	else if (m_TabDrawStyle == fdt_Themed)
	{
		BitBlt(
			hdc, rcCaption.left, rcCaption.top, rcCaption.right-rcCaption.left+1, rcTabs.bottom-rcCaption.top+1,
			hdcPaint, rcCaption.left, rcCaption.top, SRCCOPY);
		////TODO: ��������� ������/������ �����������/.../��������
		//BitBlt(
		//	hdc, rcCaption.left, rcCaption.top, rcTabs.right-rcCaption.left+1, rcTabs.bottom-rcCaption.top+1,
		//	hdcPaint, rcCaption.left, rcCaption.top, SRCCOPY);
		//// ���������� ������� ��� �������� Min/Max/Close
		//int nTop = rcCaption.top + gpConEmu->GetWinCaptionHeight();
		//if (nTop <= rcCaption.bottom)
		//{
		//	BitBlt(
		//		hdc, rcTabs.right, nTop, rcCaption.right-rcTabs.right+1, rcCaption.bottom-nTop+1,
		//		hdcPaint, rcTabs.right, nTop, SRCCOPY);
		//}
	}
	else // Win2k
	{
		BitBlt(
			hdc, rcCaption.left, rcCaption.top, rcCaption.right-rcCaption.left+1, rcTabs.bottom-rcCaption.top+1,
			hdcPaint, rcCaption.left, rcCaption.top, SRCCOPY);
	}


	SelectObject(hdcPaint, hOldBmp);
	DeleteObject(hbmp);
	DeleteDC(hdcPaint);
}

void TabBarClass::PaintTab_Common(HDC hdcPaint, RECT rcTab, struct TabDrawInfo* pTab, UINT anFlags, BOOL bCurrent, BOOL bHover)
{
	switch (gpSet->nTabStyle)
	{
	case ts_Win8:
		PaintTab_Win8(hdcPaint, rcTab, pTab, anFlags, bCurrent, bHover);
		break;
	default:
		PaintTab_VS2008(hdcPaint, rcTab, pTab, anFlags, bCurrent, bHover);
	}
}

void TabBarClass::PaintTab_VS2008(HDC hdcPaint, RECT rcTab, struct TabDrawInfo* pTab, UINT anFlags, BOOL bCurrent, BOOL bHover)
{
	int nColor1, nColor2, nColorL, nColorR, nColorO;

	int H3 = (rcTab.bottom - rcTab.top) / 3;
	int E3 = max(1,(mn_EdgeWidth/3));

	int T = bCurrent ? 0 : 1;

	// ������ � 2k ��������
	//POINT ptOuter[] = {
	//	{rcTab.left,rcTab.bottom}, // 0
	//	{rcTab.left+E3+1,rcTab.bottom-H3+T}, // 1
	//	{rcTab.left+2*E3,rcTab.top+H3+T}, // 2
	//	{rcTab.left+mn_EdgeWidth-1,rcTab.top+1+T}, // 3
	//	{rcTab.left+mn_EdgeWidth+2,rcTab.top+T}, // 4
	//	{rcTab.right-5,rcTab.top+T}, // 5
	//	{rcTab.right-2,rcTab.top+2+T}, // 6
	//	{rcTab.right,rcTab.top+5+T}, // 7
	//	{rcTab.right,rcTab.bottom}, // 8
	//	{rcTab.right+1,rcTab.bottom}, // 9
	//};
	//POINT ptInner[] = {
	//	{ptOuter[0].x+1,ptOuter[0].y},
	//	{ptOuter[1].x+1,ptOuter[1].y},
	//	{ptOuter[2].x+1,ptOuter[2].y},
	//	{ptOuter[3].x+1,ptOuter[3].y},
	//	{ptOuter[4].x+1,ptOuter[4].y},
	//	{ptOuter[5].x-1,ptOuter[5].y},
	//	{ptOuter[6].x-1,ptOuter[6].y},
	//	{ptOuter[7].x-1,ptOuter[7].y},
	//	{ptOuter[8].x-1,ptOuter[8].y},
	//};

	POINT ptOuter[] = {
		{rcTab.left,rcTab.bottom}, // 0
		{rcTab.left+mn_EdgeWidth-4,rcTab.top+3+T}, // 1
		{rcTab.left+mn_EdgeWidth-2,rcTab.top+1+T}, // 2
		{rcTab.left+mn_EdgeWidth+1,rcTab.top+T}, // 3
		{rcTab.right-5,rcTab.top+T}, // 4
		{rcTab.right-2,rcTab.top+2+T}, // 5
		{rcTab.right,rcTab.top+5+T}, // 6
		{rcTab.right,rcTab.bottom}, // 7
		{rcTab.right+1,rcTab.bottom}, // 8
	};

	POINT ptInner[] = {
		{ptOuter[0].x+1,ptOuter[0].y},
		{ptOuter[1].x+1,ptOuter[1].y},
		{ptOuter[2].x+1,ptOuter[2].y},
		{ptOuter[3].x+1,ptOuter[3].y},
		{ptOuter[4].x-1,ptOuter[4].y},
		{ptOuter[5].x-1,ptOuter[5].y},
		{ptOuter[6].x-1,ptOuter[6].y},
		{ptOuter[7].x-1,ptOuter[7].y+1},
	};

	if ((anFlags & fwt_Disabled))
	{
		nColorO = clrInactiveBorder;
		nColor1 = clrBackDisabled;
		nColor2 = clrBackDisabledBottom;
		nColorL = clrInactiveEdgeLeft;
		nColorR = clrInactiveEdgeRight;
	}
	else if (bCurrent)
	{
		nColorO = clrBorder;
		nColor1 = clrBackActive;
		nColor2 = clrBackActiveBottom;
		nColorL = clrEdgeLeft;
		nColorR = clrEdgeRight;
	}
	else if (bHover)
	{
		nColorO = clrHoverBorder;
		nColor1 = clrBackHover;
		nColor2 = clrBackHoverBottom;
		nColorL = clrEdgeLeft;
		nColorR = clrEdgeRight;
	}
	else 
	{
		nColorO = clrInactiveBorder;
		nColor1 = clrBackInactive;
		nColor2 = clrBackInactiveBottom;
		nColorL = clrInactiveEdgeLeft;
		nColorR = clrInactiveEdgeRight;
	}

	// ��� ����
	//SelectObject(hdcPaint, mh_Brushes[nColor2]);
	//SelectObject(hdcPaint, mh_Pens[nColor2]);
	//SetPolyFillMode(hdcPaint, WINDING);
	//Polygon(hdcPaint, ptOuter, countof(ptOuter)-1);
	TRIVERTEX vertex[] =
	{
		{rcTab.left+mn_EdgeWidth-2,
		 rcTab.top+1+T,
		 GetRValue(m_Colors[nColor1])<<8,
		 GetGValue(m_Colors[nColor1])<<8,
		 GetBValue(m_Colors[nColor1])<<8,
		 0x0000},
		{rcTab.right-2,
		 rcTab.top+1+T,
		 GetRValue(m_Colors[nColor1])<<8,
		 GetGValue(m_Colors[nColor1])<<8,
		 GetBValue(m_Colors[nColor1])<<8,
		 0x0000},
        {rcTab.left+1,
         rcTab.bottom+1,
         GetRValue(m_Colors[nColor2])<<8,
         GetGValue(m_Colors[nColor2])<<8,
         GetBValue(m_Colors[nColor2])<<8,
         0x0000},
        {rcTab.right-1,
         rcTab.bottom+1,
         GetRValue(m_Colors[nColor2])<<8,
         GetGValue(m_Colors[nColor2])<<8,
         GetBValue(m_Colors[nColor2])<<8,
         0x0000}
	};

	// Create a GRADIENT_TRIANGLE structure that
	// references the TRIVERTEX vertices.
	GRADIENT_TRIANGLE triangles[] = {{0,2,3},{0,1,3}};

	// Draw a shaded triangle.
	GdiGradientFill(hdcPaint, vertex, countof(vertex), triangles, countof(triangles), GRADIENT_FILL_TRIANGLE);

	// ��������� ������, ��� �������� � HitTest
	if (pTab->rgnTab) DeleteObject(pTab->rgnTab); //TODO: ����� �� �� ������������� ������, ���� �� �������
	pTab->rgnTab = CreatePolygonRgn(ptInner, countof(ptInner), WINDING);

	// ���������� �����
	RECT rcText = {rcTab.left+mn_EdgeWidth, rcTab.top+mn_TextShift, rcTab.right-mn_EdgeWidth, rcTab.bottom};
	PaintTab_Text(hdcPaint, rcText, pTab, anFlags, bCurrent, bHover);
	
	// �����
	SelectObject(hdcPaint, mh_Pens[nColorL]);
	Polyline(hdcPaint, ptInner, 4);
	SelectObject(hdcPaint, mh_Pens[nColorR]);
	_ASSERTE(countof(ptInner) == 8);
	Polyline(hdcPaint, ptInner+4, 4);

	// ���������
	SelectObject(hdcPaint, mh_Pens[nColorO]);
	Polyline(hdcPaint, ptOuter, countof(ptOuter));


	if (!bCurrent)
	{
		SelectObject(hdcPaint, mh_Pens[clrBorder]);
		int nY = 0; //(m_TabDrawStyle == fdt_Aero) ? 0 : 1;
		MoveToEx(hdcPaint, rcTab.left, rcTab.bottom-nY, 0);
		LineTo(hdcPaint, rcTab.right, rcTab.bottom-nY);
	}
}

void TabBarClass::PaintTab_Win8(HDC hdcPaint, RECT rcTab, struct TabDrawInfo* pTab, UINT anFlags, BOOL bCurrent, BOOL bHover)
{
	int nColor1, nColor2, /*nColorL, nColorR,*/ nColorO;
	if ((anFlags & fwt_Disabled))
	{
		nColorO = clrInactiveBorder;
		nColor1 = clrBackDisabled;
		nColor2 = clrBackDisabledBottom;
		//nColorL = clrInactiveEdgeLeft;
		//nColorR = clrInactiveEdgeRight;
	}
	else if (bCurrent)
	{
		nColorO = clrBorder;
		nColor1 = clrBackActive;
		nColor2 = clrBackActiveBottom;
		//nColorL = clrEdgeLeft;
		//nColorR = clrEdgeRight;
	}
	else if (bHover)
	{
		nColorO = clrHoverBorder;
		nColor1 = clrBackHover;
		nColor2 = clrBackHoverBottom;
		//nColorL = clrEdgeLeft;
		//nColorR = clrEdgeRight;
	}
	else 
	{
		nColorO = clrInactiveBorder;
		nColor1 = clrBackInactive;
		nColor2 = clrBackInactiveBottom;
		//nColorL = clrInactiveEdgeLeft;
		//nColorR = clrInactiveEdgeRight;
	}

	bool bUpDown = (gpSet->nTabsLocation == 1);
	//int T = bCurrent ? 2 : 2;
	int T = bUpDown ? 2 : 2;

	int  nY1 = bUpDown ? rcTab.top : rcTab.bottom;
	int  nY2 = bUpDown ? (rcTab.bottom-T) : (rcTab.top+T);
	POINT ptOuter[] = {
		{rcTab.left,nY1},    // 0
		{rcTab.left,nY2},    // 1
		{rcTab.right,nY2},   // 2
		{rcTab.right,nY1},   // 3
		{rcTab.right+1,nY1}, // 4
	};

	// Used for "click region" (upside/down - does not matter)
	POINT ptInner[] = {
		{rcTab.left,rcTab.bottom},    // 0
		{rcTab.left,rcTab.top},       // 1
		{rcTab.right,rcTab.top},      // 2
		{rcTab.right,rcTab.bottom},   // 3
		{rcTab.right+1,rcTab.bottom}, // 4
	};


	// ��� ����
	nY1 = bUpDown ? (rcTab.bottom-T) : (rcTab.top+1+T);
	nY2 = bUpDown ? (rcTab.top+1) : (rcTab.bottom+1);
	TRIVERTEX vertex[] =
	{
		{rcTab.left+1,
		 nY1,
		 GetRValue(m_Colors[nColor1])<<8,
		 GetGValue(m_Colors[nColor1])<<8,
		 GetBValue(m_Colors[nColor1])<<8,
		 0x0000},
		{rcTab.right/*-1*/,
		 nY1,
		 GetRValue(m_Colors[nColor1])<<8,
		 GetGValue(m_Colors[nColor1])<<8,
		 GetBValue(m_Colors[nColor1])<<8,
		 0x0000},
        {rcTab.left+1,
         nY2,
         GetRValue(m_Colors[nColor2])<<8,
         GetGValue(m_Colors[nColor2])<<8,
         GetBValue(m_Colors[nColor2])<<8,
         0x0000},
        {rcTab.right/*-1*/,
         nY2,
         GetRValue(m_Colors[nColor2])<<8,
         GetGValue(m_Colors[nColor2])<<8,
         GetBValue(m_Colors[nColor2])<<8,
         0x0000}
	};

	RECT rcText = {rcTab.left+mn_EdgeWidth, rcTab.top+mn_TextShift, rcTab.right-mn_EdgeWidth, rcTab.bottom};


	// Go

	// Paint gradient on TAB
	GRADIENT_TRIANGLE triangles[] = {{0,2,3},{0,1,3}};
	GdiGradientFill(hdcPaint, vertex, countof(vertex), triangles, countof(triangles), GRADIENT_FILL_TRIANGLE);

	// ��������� ������, ��� �������� � HitTest
	if (pTab->rgnTab) DeleteObject(pTab->rgnTab); //TODO: ����� �� �� ������������� ������, ���� �� �������
	pTab->rgnTab = CreatePolygonRgn(ptInner, countof(ptInner), WINDING);

	// ���������� �����
	PaintTab_Text(hdcPaint, rcText, pTab, anFlags, bCurrent, bHover);
	
	////�����
	////SelectObject(hdcPaint, mh_Pens[nColorL]);
	////Polyline(hdcPaint, ptInner, 4);
	////SelectObject(hdcPaint, mh_Pens[nColorR]);
	////_ASSERTE(countof(ptInner) == 8);
	////Polyline(hdcPaint, ptInner+4, 4);

	// ���������
	SelectObject(hdcPaint, mh_Pens[nColorO]);
	Polyline(hdcPaint, ptOuter, countof(ptOuter));

	//if (!bCurrent)
	//{
	//	SelectObject(hdcPaint, mh_Pens[clrBorder]);
	//	int nY = 0; //(m_TabDrawStyle == fdt_Aero) ? 0 : 1;
	//	MoveToEx(hdcPaint, rcTab.left, rcTab.bottom-nY, 0);
	//	LineTo(hdcPaint, rcTab.right, rcTab.bottom-nY);
	//}
}

void TabBarClass::PaintTab_Text(HDC hdcPaint, RECT rcText, struct TabDrawInfo* pTab, UINT anFlags, BOOL bCurrent, BOOL bHover)
{
	if ((anFlags & fwt_Disabled))
	{
		SetTextColor(hdcPaint, m_Colors[clrDisabledTextShadow]);
		OffsetRect(&rcText, 1, 1);
		DrawText(hdcPaint, pTab->Display.Ptr(), -1, &rcText, DT_LEFT|DT_NOPREFIX|DT_TOP|DT_WORD_ELLIPSIS);
		OffsetRect(&rcText, -1, -1);
	}
	SetTextColor(hdcPaint, m_Colors[(anFlags & fwt_Disabled) ? clrDisabledText : clrText]);
	DrawText(hdcPaint, pTab->Display.Ptr(), -1, &rcText, DT_LEFT|DT_NOPREFIX|DT_TOP|DT_WORD_ELLIPSIS);
}

//void TabBarClass::PaintTab_2k(HDC hdcPaint, RECT rcTab, struct TabInfo* pTab, BOOL bCurrent, BOOL bHover)
//{
//	if (rcTab.right > rcTab.right)
//	{
//		// ���� ���� �������� �� ������� ��������� - ���������� ������ �����
//		rcTab.right = rcTab.right;
//	}
//
//	//DrawThemeEdge(hTheme, hdcPaint, 
//
//	POINT ptInner[] = {
//		{rcTab.left+1,rcTab.bottom},
//		{rcTab.left+mn_EdgeWidth+1,rcTab.top},
//		{rcTab.right-mn_EdgeWidth-1,rcTab.top},
//		{rcTab.right-1,rcTab.bottom}
//	};
//	POINT ptEdge[] = {
//		{rcTab.left,rcTab.bottom},
//		{rcTab.left+mn_EdgeWidth,rcTab.top},
//		{rcTab.right-mn_EdgeWidth,rcTab.top},
//		{rcTab.right,rcTab.bottom},
//		{rcTab.right,rcTab.bottom+1}
//	};
//
//	SelectObject(hdcPaint, mh_TabEdge);
//	if ((pTab->nFlags & etfActive))
//		SelectObject(hdcPaint, mh_TabBkActive);
//	else if ((pTab->nFlags & fwt_Disabled))
//		SelectObject(hdcPaint, mh_TabBkDisabled);
//	else
//		SelectObject(hdcPaint, mh_TabBkInactive);
//	Polygon(hdcPaint, ptInner, countof(ptInner));
//
//	SelectObject(hdcPaint, mh_TabBorder);
//	Polyline(hdcPaint, ptEdge, countof(ptEdge));
//
//	RECT rcText = {rcTab.left+mn_EdgeWidth, rcTab.top+mn_TextShift, rcTab.right-mn_EdgeWidth, rcTab.bottom};
//
//	DrawText(hdcPaint, pTab->sTitle, -1, &rcText, DT_LEFT|DT_NOPREFIX|DT_TOP|DT_WORD_ELLIPSIS);
//}
//
//void TabBarClass::PaintTab_XP(HDC hdcPaint, RECT rcTab, struct TabInfo* pTab, BOOL bCurrent, BOOL bHover)
//{
//	if (rcTab.right > rcTab.right)
//	{
//		// ���� ���� �������� �� ������� ��������� - ���������� ������ �����
//		rcTab.right = rcTab.right;
//	}
//
//	//DrawThemeEdge(hTheme, hdcPaint, 
//
//	POINT ptEdge[] = {
//		{rcTab.left,rcTab.bottom},
//		{rcTab.left+mn_EdgeWidth,rcTab.top},
//		{rcTab.right-mn_EdgeWidth,rcTab.top},
//		{rcTab.right,rcTab.bottom}
//	};
//
//	SelectObject(hdcPaint, mh_TabBorder);
//	if ((pTab->nFlags & etfActive))
//		SelectObject(hdcPaint, mh_TabBkActive);
//	else if ((pTab->nFlags & fwt_Disabled))
//		SelectObject(hdcPaint, mh_TabBkDisabled);
//	else
//		SelectObject(hdcPaint, mh_TabBkInactive);
//	Polygon(hdcPaint, ptEdge, countof(ptEdge));
//
//	RECT rcText = {rcTab.left+mn_EdgeWidth, rcTab.top+mn_TextShift, rcTab.right-mn_EdgeWidth, rcTab.bottom};
//
//	DrawText(hdcPaint, pTab->sTitle, -1, &rcText, DT_LEFT|DT_NOPREFIX|DT_TOP|DT_WORD_ELLIPSIS);
//}
//
//void TabBarClass::PaintTab_Aero(HDC hdcPaint, RECT rcTab, struct TabInfo* pTab, BOOL bCurrent, BOOL bHover)
//{
//	if (rcTab.right > rcTab.right)
//	{
//		// ���� ���� �������� �� ������� ��������� - ���������� ������ �����
//		rcTab.right = rcTab.right;
//	}
//	
//	//DrawThemeEdge(mh_Theme, hdcPaint, 
//	
//	POINT ptInner[] = {
//		{rcTab.left+1,rcTab.bottom},
//		{rcTab.left+mn_EdgeWidth,rcTab.top},
//		{rcTab.right-mn_EdgeWidth,rcTab.top},
//		{rcTab.right,rcTab.bottom}
//	};
//	POINT ptEdge[] = {
//		{rcTab.left,rcTab.bottom},
//		{rcTab.left+mn_EdgeWidth,rcTab.top},
//		{rcTab.right-mn_EdgeWidth,rcTab.top},
//		{rcTab.right,rcTab.bottom}
//	};
//	
//	SelectObject(hdcPaint, mh_TabEdge);
//	if ((pTab->nFlags & etfActive))
//		SelectObject(hdcPaint, mh_TabBkActive);
//	else if ((pTab->nFlags & fwt_Disabled))
//		SelectObject(hdcPaint, mh_TabBkDisabled);
//	else
//		SelectObject(hdcPaint, mh_TabBkInactive);
//	Polygon(hdcPaint, ptInner, countof(ptInner));
//	
//	SelectObject(hdcPaint, mh_TabBorder);
//	Polyline(hdcPaint, ptEdge, countof(ptEdge));
//	
//	RECT rcText = {rcTab.left+mn_EdgeWidth, rcTab.top+mn_TextShift, rcTab.right-mn_EdgeWidth, rcTab.bottom};
//	
//	//DrawText(hdcPaint, pTab->sTitle, -1, &rcText, DT_LEFT|DT_NOPREFIX|DT_TOP|DT_WORD_ELLIPSIS);
//	
//	DTTOPTS DttOpts = {sizeof(DTTOPTS)};
//	DttOpts.dwFlags = DTT_COMPOSITED|DTT_TEXTCOLOR|DTT_BORDERCOLOR;
//	//if (pTab->nFlags & etfActive) DttOpts.dwFlags = DTT_GLOWSIZE;
//	DttOpts.crText  = mclr_TabText;
//	DttOpts.crBorder = 255;
//	DttOpts.iBorderSize = 1;
//    //DttOpts.iGlowSize = 12; // Default value
//    
//    //// CompositedWindow::Window is declared in AeroStyle.xml
//    //HTHEME hThemeWindow = OpenThemeData(NULL, L"CompositedWindow::Window");
//    //if (hThemeWindow != NULL)
//    //{
//    //    GetThemeInt(hThemeWindow, 0, 0, TMT_TEXTGLOWSIZE, &DttOpts.iGlowSize);
//    //    CloseThemeData(hThemeWindow);
//    //}
//    
//	gpConEmu->DrawThemeTextEx(mh_Theme, hdcPaint, 0, 0, pTab->sTitle, -1, 0, &rcText, &DttOpts);
//}

//void TabBarClass::BeginTabs(void)
//{
//	EnterCriticalSection(&mcs_Tabs);
//	mn_EditTabCount = 0;
//}
//
//void TabBarClass::CommitTabs(void)
//{
//	// ������ ���� ��� ������� ���� ���!
//	if (mn_EditTabCount < 1)
//	{
//		_ASSERTE(mn_EditTabCount == 0);
//		AddTab(DefaultTitle, 0);
//		mn_EditTabCount = 1;
//	}
//	if (mn_TabCount != mn_EditTabCount)
//	{
//		// ���� ����� ����� ������ - ���������� ��������� �� ����������� ����
//    	for (int i = (mn_EditTabCount+1); i < mn_TabCount; i++)
//		{
//			m_Tabs[i].Display.Release();
//			m_Tabs[i].Full.Release();
//			m_Tabs[i].nFlags = 0;
//			m_Tabs[i].rcTab.left = m_Tabs[i].rcTab.top = m_Tabs[i].rcTab.right = m_Tabs[i].rcTab.bottom = 0;
//			if (m_Tabs[i].rgnTab) { DeleteObject(m_Tabs[i].rgnTab); m_Tabs[i].rgnTab = NULL; }
//			m_Tabs[i].Clipped = false;
//		}
//
//		mb_UpdateModified = true;
//	}
//	mn_TabCount = mn_EditTabCount;
//	if (mn_ActiveTab >= mn_TabCount)
//	{
//		mb_UpdateModified = true;
//		mn_ActiveTab = (mn_TabCount - 1);
//	}
//	else if (mn_ActiveTab < 1)
//	{
//		mb_UpdateModified = true;
//		mn_ActiveTab = 0;
//	}
//	if (mn_HoverTab >= mn_TabCount)
//	{
//		mb_UpdateModified = true;
//		mn_HoverTab = -1;
//	}
//
//	// ���������, ��� ������������ ����
//	for (int i = 0; i < mn_TabCount; i++)
//		m_TabStack.CheckOrCreate((i == mn_ActiveTab), m_Tabs[i].pVCon, m_Tabs[i].Full.Ptr(), m_Tabs[i].nFlags & 0xFF);
//
//	LeaveCriticalSection(&mcs_Tabs);
//
//	if (mb_UpdateModified)
//	{
//		mb_UpdateModified = false;
//		gpConEmu->RedrawTabPanel();
//	}
//}
//
//void TabBarClass::AddTab(LPCWSTR asTitle, DWORD anFlags, CVirtualConsole* pVCon, UINT wndIndex)
//{
//	//if (mn_EditTabCount >= mn_MaxTabCount)
//	//{
//	//	// ��������� ���������� ���������-������������ �����
//	//	return;
//	//}
//
//	// ��������� �� ���������� �� ����?
//	if (lstrcmpW(m_Tabs[mn_EditTabCount].Full.Ptr(), SafeTitle(asTitle))
//		|| (m_Tabs[mn_EditTabCount].nFlags = anFlags)
//		|| (m_Tabs[mn_EditTabCount].pVCon != pVCon)
//		|| (m_Tabs[mn_EditTabCount].nFarWindowId != wndIndex))
//	{
//		mb_UpdateModified = true;
//	}
//	//lstrcpynW(m_Tabs[mn_EditTabCount].sTitle, SafeTitle(asTitle), countof(m_Tabs->sTitle));
//	m_Tabs[mn_EditTabCount].Full.Set(asTitle);
//	//TODO: �������������� ������
//	m_Tabs[mn_EditTabCount].Display.Set(asTitle);
//	m_Tabs[mn_EditTabCount].nFlags = anFlags;
//	m_Tabs[mn_EditTabCount].pVCon = pVCon;
//	m_Tabs[mn_EditTabCount].nFarWindowId = wndIndex;
//
//	mn_EditTabCount++;
//}

int TabBarClass::TabFromCursor(POINT point, DWORD *pnFlags /*= NULL*/)
{
	if (!PtInRect(&mrc_Tabs, point))
		return -1;
		
	int nTab = -1;
	int nTabCount = m_Tabs.GetCount();
	CTab Tab;
	for (int i = 0; i < nTabCount; i++)
	{
		if (m_Tabs.GetTabByIndex(i, Tab) && PtInRect(&Tab->DrawInfo.rcTab, point))
		{
			if (Tab->DrawInfo.rgnTab && !PtInRegion(Tab->DrawInfo.rgnTab, point.x, point.y))
				continue; // ������ ������ � ����, �� ����������� � ������� �������

			nTab = i;
			if (pnFlags)
				*pnFlags = Tab->Flags();
			break;
		}
	}
	
	return nTab;
}

int TabBarClass::TabBtnFromCursor(POINT point, DWORD *pnFlags /*= NULL*/)
{
	if (mb_ToolbarFit && PtInRect(&mrc_Toolbar, point))
		return 1; // ��� �� - �� ����������
	return -1;
}

// ������ ����������� ������ � ������� ����!
void TabBarClass::Update(BOOL abPosted/*=FALSE*/)
{
    if (!gpConEmu->isMainThread())
	{
		_ASSERTE(abPosted==FALSE);
		RequestPostUpdate();
        return;
    }

	mb_PostUpdateCalled = FALSE;

	//TODO: �������� mn_ActiveTab & mn_HoverTab=-1
	mn_InUpdate++;

	////TODO: ��������� ������� �� VCon/RCon
	//BeginTabs();
	//AddTab(L"{F:\\VCProject\\ConEmu\\src\\ConEmu\\ConEmuTh\\Modules\\gdip} - Far", etfActive);
	//AddTab(L"{C:\\Windows\\System32} - Far", etfAdmin);
	//AddTab(L"[TabBar.cpp] *", etfAdmin|etfActive);
	//AddTab(L"{7%} Copying - Far", etfUser);
	//AddTab(L"[changelog]", etfUser);
	///*AddTab(L"1.[Tab&Bar.h]", fwt_Disabled);
	//AddTab(L"Operation has been interrupted - Far", etfNonRespond);*/
	//CommitTabs();

	int nActiveTab = -1;
	CVirtualConsole* pVCon = NULL;
	CRealConsole* pRCon = NULL;
	HANDLE hUpdate = m_Tabs.UpdateBegin();
	BOOL lbVConActive = FALSE;
	int nIndex = 0;
	for (int i = 0; (pVCon = gpConEmu->GetVCon(i))!=NULL; i++)
	{
		if (lbVConActive = gpConEmu->isActive(pVCon))
		{
			nActiveTab = nIndex;
		}

		//if (!(pRCon = pVCon->RCon()))
		//{
		//	//m_Tabs.UpdateFarWindow(hUpdate, pVCon, DefaultTitle, etfPanels, 0, 0, 0, etfActive);
		//	continue;
		//}

		
		CTab Tab;
		for (int j = 0; pVCon->GetTab(j, &Tab); j++)
		{
			m_Tabs.UpdateAppend(hUpdate, Tab, FALSE/*abMoveFirst*/);
			if (lbVConActive && (j == pVCon->GetActiveTab()))
			{
				nActiveTab = nIndex;
			}
			nIndex++;
		}
	}
	if (nActiveTab >= 0)
		mn_ActiveTab = nActiveTab;
	else if (mn_ActiveTab >= nIndex)
		mn_ActiveTab = max(0,(nIndex-1));
	m_Tabs.UpdateEnd(hUpdate, FALSE/*abForceReleaseTail*/);

	mn_InUpdate--;
	
	if (mb_PostUpdateRequested)
	{
		mb_PostUpdateCalled = FALSE;
		mb_PostUpdateRequested = FALSE;
		RequestPostUpdate();
	}

	TODO("���������, � ����� �� ��������� ����?");
	Invalidate();
}

int TabBarClass::GetCurSel()
{
	if (!this) return -1;
	if (mb_InKeySwitching && mn_HoverTab >= 0 && mn_HoverTab < m_Tabs.GetCount())
		return mn_HoverTab;
	return mn_ActiveTab;
}

int TabBarClass::GetItemCount()
{
	if (!this) return 0;
	return m_Tabs.GetCount();
}

void TabBarClass::SelectTab(int anTab)
{
	if (!this)
		return;

	if (anTab < 0 || anTab >= m_Tabs.GetCount())
	{
		mn_HoverTab = -1;
	}
	else
	{
		mn_ActiveTab = anTab;
		if (mn_HoverTab == mn_ActiveTab)
			mn_HoverTab = -1;
	}

	Invalidate();
}

void TabBarClass::HoverTab(int anTab)
{
	int nNewHover = -1;
	if (anTab >= 0 && anTab < m_Tabs.GetCount() && anTab != mn_ActiveTab)
		nNewHover = anTab;

	if (mn_HoverTab != nNewHover)
	{
		mn_HoverTab = anTab;
		Invalidate();
	}
}

int TabBarClass::GetHoverTab()
{
	return mn_HoverTab;
}

// � ���������� �������� ������������ (������ ��� ����������)
// ����� ������� ���������� ���
bool TabBarClass::OnTabSelected(int anNewTab)
{
	if (anNewTab < 0 || anNewTab >= m_Tabs.GetCount())
	{
		_ASSERTE(anNewTab>=0 && anNewTab<m_Tabs.GetCount());
		return false;
	}
	TabInfo ti;
	if (!m_Tabs.GetTabInfoByIndex(anNewTab, /*OUT*/ ti))
	{
		_ASSERTE(FALSE);
		return false;
	}

	if (!CanSelectTab(anNewTab, ti))
	{
		ShowTabError(TAB_CANT_BE_ACTIVATED, anNewTab);
		return false;
	}

	BOOL bChangeOk = FALSE; // ������� �� ������� �������� ���� � ���� (������/��������/������)
	BOOL bNeedActivate = FALSE; // ����� �� ������ �������� VConsole	
	
	if (!gpConEmu->isActive(ti.pVCon))
		bNeedActivate = TRUE;

	bChangeOk = ti.pVCon->RCon()->ActivateFarWindow(ti.nFarWindowID);

	if (!bChangeOk)
	{
		// ������� ������ � ������� - �� ������ ������������
		ShowTabError(L"This tab can't be activated now!", anNewTab);
	}

	// ����� ������ �� �������� - ���������� �������
	// ������ ����� ����� ���� (�������� ��� ��������� - �������)
	TODO("�� �����������. ����� ������ �� ������� ���������� � ��������� � �������� ������ ���������� ������������ �������");

	if (bNeedActivate)
	{
		if (!gpConEmu->Activate(ti.pVCon))
		{
			if (mb_InKeySwitching) Update();  // �������� �������� ��������� ���

			TODO("� ������� ��� �� ������, ���� ������������ �� �������?");
			return false;
		}
	}

	if (!bChangeOk)
	{
		if (mb_InKeySwitching) Update();  // �������� �������� ��������� ���
		return false;
	}
	
	////TODO: -> FarSendChangeTab ( anNewTab );
	//SelectTab(anNewTab);
	//return true; // ������� ����� ������������� ����� ���������, ������ false
	
	Update();  // �������� �������� ��������� ���
	return true;
}

// ������� ����� ��������� �������
bool TabBarClass::CanSelectTab(int anNewTab, const TabInfo& ti)
{
	return ((ti.Flags & fwt_Disabled) == 0);
}

bool TabBarClass::ProcessNcTabMouseEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT &lResult)
{
	_ASSERTE(FALSE && "There is no tabs in 'Caption'");

	switch (uMsg)
	{
	//TODO: � ���� ���� � ���������� �������? ������ �� ����������?
	case WM_NCLBUTTONDOWN:
	case WM_NCLBUTTONUP:
	case WM_NCLBUTTONDBLCLK:
	case WM_NCRBUTTONDOWN:
	case WM_NCRBUTTONUP:
	case WM_NCMBUTTONDOWN:
	case WM_NCMBUTTONUP:
	case WM_NCMOUSEMOVE:
		if (wParam == HT_CONEMUTAB || wParam == HTCAPTION)
		{
			//TODO: ���� �� ��������������� (������������ ��������� � ������� ���������)
			
			POINT point;
			RECT wr; GetWindowRect(hWnd, &wr);
			point.x = LOWORD(lParam) - wr.left - ((gpConEmu->DrawType() == fdt_Aero) ? gpConEmu->GetFrameWidth() : 0);
			point.y = HIWORD(lParam) - wr.top;
			
			if (mb_ToolbarFit && PtInRect(&mrc_Toolbar, point))
			{
				if (mn_HoverTab >= 0)
				{
					HoverTab(-1);
				}
				#if defined(USE_CONEMU_TOOLBAR)
				if (mp_Toolbar->MouseEvent(hWnd, uMsg, wParam, lParam, point, lResult))
				{
					return true;
				}
				#endif
				return false;
			}
			else
			{
				#if defined(USE_CONEMU_TOOLBAR)
				// � ������ ������ ���������, ���� ����
				mp_Toolbar->UnHover();
				#endif
			}

			DWORD nTabFlags = 0;
			int nTab = TabFromCursor(point, &nTabFlags);
			if (nTab >= 0)
			{
				//TODO: WM_NCMBUTTONDOWN
				if ((uMsg == WM_NCLBUTTONDOWN) || (uMsg == WM_NCRBUTTONUP))
				{
					if ((nTabFlags & fwt_Disabled))
					{
						ShowTabError(TAB_CANT_BE_ACTIVATED, nTab);
					}
					else
					{
						if (OnTabSelected(nTab))
						{
							CVConGuard VCon;
							if ((uMsg == WM_NCRBUTTONUP) && (gpConEmu->GetActiveVCon(&VCon) >= 0))
							{
								POINT ptCur; GetCursorPos(&ptCur);
								gpConEmu->mp_Menu->ShowPopupMenu(VCon.VCon(), ptCur);
							}
						}
					}
				}
				else if (uMsg == WM_NCMOUSEMOVE)
				{
					// check hover - mn_HoverTab
					int nNewHover = (mn_ActiveTab == nTab) ? -1 : nTab;
					if (nNewHover != mn_HoverTab)
					{
						HoverTab(nNewHover);
					}
				}
				else if (uMsg == WM_NCMBUTTONUP)
				{
					// ������� ���?
				}
				return true; // ���������� ����
			}
			else if (mn_HoverTab >= 0)
			{
				HoverTab(-1);
			}
		}
		return false; // ���������� ���������
	case WM_NCMOUSEHOVER:
		// �������� ������, ���� ����� ��� �������
		return false; // ���������� ���������
	case WM_NCMOUSELEAVE:
		if (mn_HoverTab >= 0)
		{
			HoverTab(-1);
			#if defined(USE_CONEMU_TOOLBAR)
			// � ������ ������ ���������, ���� ����
			mp_Toolbar->UnHover();
			#endif
		}
		return false; // ���������� ���������
	}
	return false; // ���������� ���������
}

bool TabBarClass::ProcessTabKeyboardEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT &lResult)
{
    BOOL lbAltPressed = isPressed(VK_MENU);
    if (lbAltPressed)
    {
    	_ASSERTE(lbAltPressed == FALSE);
    	return false;
	}

    if (uMsg == WM_KEYDOWN && wParam == VK_TAB)
    {
    	bool bForward = !isPressed(VK_SHIFT);
    	Switch(bForward);
    }
	else if (mb_InKeySwitching && uMsg == WM_KEYDOWN
        && (wParam == VK_UP || wParam == VK_DOWN || wParam == VK_LEFT || wParam == VK_RIGHT))
    {
    	bool bRecent = gpSet->isTabRecent;
		gpSet->isTabRecent = false;
    	BOOL bForward = (wParam == VK_RIGHT || wParam == VK_DOWN);
    	Switch(bForward);
    	gpSet->isTabRecent = bRecent;
    	return true;
    }
        
    return TRUE;
}

// ���� �� ����� ���������� ������ ����� (���������� ������ �� RealCon)
// ��������� ������ (��� �������� � ��������?) �� ����� �������� ������,
// �� �������������� �� ������������ ����� ����������
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
		gpConEmu->RequestPostUpdateTabs();
		mn_PostUpdateTick = GetTickCount();
	}
}

//BOOL TabBarClass::GetVConFromTab(int nTabIdx, CVirtualConsole** rpVCon, DWORD* rpWndIndex)
//{
//    BOOL lbRc = FALSE;
//    CVirtualConsole *pVCon = NULL;
//    DWORD wndIndex = 0;
//
//    if (nTabIdx >= 0 && nTabIdx < mn_TabCount)
//	{
//        pVCon = m_Tab2VCon[nTabIdx].VCon.pVCon;
//        wndIndex = m_Tab2VCon[nTabIdx].VCon.wndIndex;
//
//        if (!gpConEmu->isValid(pVCon))
//		{
//			RequestPostUpdate();
//        }
//		else
//		{
//            lbRc = TRUE;
//        }
//    }
//
//    if (rpVCon) *rpVCon = lbRc ? pVCon : NULL;
//    if (rpWndIndex) *rpWndIndex = lbRc ? wndIndex : 0;
//
//    return lbRc;
//}

//CVirtualConsole* TabBarClass::FarSendChangeTab(int tabIndex)
//{
//    CVirtualConsole *pVCon = NULL;
//    DWORD wndIndex = 0;
//    BOOL  bNeedActivate = FALSE, bChangeOk = FALSE;
//
//	ShowTabError(NULL);
//
//    if (!GetVConFromTab(tabIndex, &pVCon, &wndIndex)) {
//        if (mb_InKeySwitching) Update(); // �������� �������� ��������� ���
//        return NULL;
//    }
//    
//    if (!gpConEmu->isActive(pVCon))
//        bNeedActivate = TRUE;
//        
//
//    bChangeOk = pVCon->RCon()->ActivateFarWindow(wndIndex);
//	if (!bChangeOk)
//	{
//		// ������� ������ � ������� - �� ������ ������������
//		ShowTabError(TAB_CANT_BE_ACTIVATED, tabIndex);
//	}
//    
//    // ����� ������ �� �������� - ���������� ������� 
//    // ������ ����� ����� ���� (�������� ��� ��������� - �������)
//	TODO("�� �����������. ����� ������ �� ������� ���������� � ��������� � �������� ������ ���������� ������������ �������");
//    if (bNeedActivate) {
//        if (!gpConEmu->Activate(pVCon)) {
//            if (mb_InKeySwitching) Update(); // �������� �������� ��������� ���
//            
//            TODO("� ������� ��� �� ������, ���� ������������ �� �������?");
//            return NULL;
//        }
//    }
//    
//    if (!bChangeOk) {
//        pVCon = NULL;
//        if (mb_InKeySwitching) Update(); // �������� �������� ��������� ���
//    }
//
//    return pVCon;
//}

//LRESULT TabBarClass::TabHitTest()
//{
//	if ((gpSet->isHideCaptionAlways() || gpSet->isFullScreen || (gpConEmu->isZoomed() && gpSet->isHideCaption))
//		&& gpSet->isTabs)
//	{
//		if (gpConEmu->mp_TabBar->IsShown()) {
//			TCHITTESTINFO tch = {{0,0}};
//			HWND hTabBar = gpConEmu->mp_TabBar->mh_Tabbar;
//			RECT rcWnd; GetWindowRect(hTabBar, &rcWnd);
//			GetCursorPos(&tch.pt); // Screen coordinates
//			if (PtInRect(&rcWnd, tch.pt)) {
//				tch.pt.x -= rcWnd.left; tch.pt.y -= rcWnd.top;
//				LRESULT nTest = SendMessage(hTabBar, TCM_HITTEST, 0, (LPARAM)&tch);
//				if (nTest == -1)
//					return HTCAPTION;
//			}
//		}
//	}
//	return HTNOWHERE;
//}

//LRESULT CALLBACK TabBarClass::ReBarProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//	switch (uMsg)
//	{
//	case WM_WINDOWPOSCHANGING:
//		{
//			if (gpConEmu->mp_TabBar->_tabHeight) {
//				LPWINDOWPOS pos = (LPWINDOWPOS)lParam;
//				pos->cy = gpConEmu->mp_TabBar->_tabHeight;
//				return 0;
//			}
//		}
//	case WM_SETFOCUS:
//		{
//			SetFocus(ghWnd);
//			return 0;
//		}
//	case WM_SETCURSOR:
//		if (gpSet->isHideCaptionAlways() && gpSet->isTabs && !gpSet->isFullScreen && !gpConEmu->isZoomed()) {
//			if (TabHitTest()==HTCAPTION) {
//				SetCursor(gpConEmu->mh_CursorMove);
//				return TRUE;
//			}
//		}
//		break;
//
//	case WM_MOUSEMOVE:
//	case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_LBUTTONDBLCLK:
//	/*case WM_RBUTTONDOWN:*/ case WM_RBUTTONUP: //case WM_RBUTTONDBLCLK:
//		if ((gpSet->isHideCaptionAlways() || gpSet->isFullScreen || (gpConEmu->isZoomed() && gpSet->isHideCaption))
//			&& gpSet->isTabs)
//		{
//			if (TabHitTest()==HTCAPTION) {
//				POINT ptScr; GetCursorPos(&ptScr);
//				lParam = MAKELONG(ptScr.x,ptScr.y);
//				LRESULT lRc = 0;
//				if (uMsg == WM_LBUTTONDBLCLK) {
//					// ����� ���� �������� �� ���������� � �������
//					gpConEmu->mouse.state |= MOUSE_SIZING_DBLCKL;
//					// ������ AltF9
//					//gpConEmu->SetWindowMode((gpConEmu->isZoomed()||(gpSet->isFullScreen&&gpConEmu->isWndNotFSMaximized)) ? rNormal : rMaximized);
//					gpConEmu->OnAltF9(TRUE);
//				} else if (uMsg == WM_RBUTTONUP) {
//					gpConEmu->ShowSysmenu(ghWnd, ptScr.x, ptScr.y/*-32000*/);
//				} else if (!gpSet->isFullScreen && !gpConEmu->isZoomed()) {
//					lRc = gpConEmu->WndProc(ghWnd, uMsg-(WM_MOUSEMOVE-WM_NCMOUSEMOVE), HTCAPTION, lParam);
//				}
//				return lRc;
//			}
//		}
//		break;
//	}
//	return CallWindowProc(_defaultReBarProc, hwnd, uMsg, wParam, lParam);
//}

//LRESULT CALLBACK TabBarClass::TabProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//    switch (uMsg)
//    {
//    case WM_WINDOWPOSCHANGING:
//        {
//        	if (gpConEmu->mp_TabBar->mh_Rebar) {
//	        	LPWINDOWPOS pos = (LPWINDOWPOS)lParam;
//				//if (gpConEmu->mp_TabBar->mb_ThemingEnabled) {
//				if (gpSet->CheckTheming()) {
//		            pos->y = 2; // ����� � Win7 �� ��������� � {0x0} � ����� ����� ���������� ������
//					pos->cy = gpConEmu->mp_TabBar->_tabHeight -3; // �� ������ ������
//				} else {
//					pos->y = 1;
//					pos->cy = gpConEmu->mp_TabBar->_tabHeight + 1;
//				}
//	            return 0;
//            }
//            break;
//        }
//    case WM_MBUTTONUP:
//    case WM_RBUTTONUP:
//        {
//            gpConEmu->mp_TabBar->OnMouse(uMsg, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
//            return 0;
//        }
//    case WM_SETFOCUS:
//        {
//            SetFocus(ghWnd);
//            return 0;
//        }
//	case WM_TIMER:
//		if (wParam == FAILED_TABBAR_TIMERID) {
//			KillTimer(hwnd, wParam);
//			SendMessage(gpConEmu->mp_TabBar->mh_Balloon, TTM_TRACKACTIVATE, FALSE, (LPARAM)&gpConEmu->mp_TabBar->tiBalloon);
//			SendMessage(gpConEmu->mp_TabBar->mh_TabTip, TTM_ACTIVATE, TRUE, 0);
//		};
//    }
//    return CallWindowProc(_defaultTabProc, hwnd, uMsg, wParam, lParam);
//}
//
//// Window procedure for Toolbar (Multiconsole & BufferHeight)
//LRESULT CALLBACK TabBarClass::ToolProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//    switch (uMsg)
//    {
//    case WM_WINDOWPOSCHANGING:
//        {
//        	LPWINDOWPOS pos = (LPWINDOWPOS)lParam;
//            pos->y = (gpConEmu->mp_TabBar->mn_ThemeHeightDiff == 0) ? 2 : 1;
//            if (gpSet->isHideCaptionAlways() && gpSet->nToolbarAddSpace > 0)
//            {
//            	SIZE sz;
//            	if (CallWindowProc(_defaultToolProc, hwnd, TB_GETIDEALSIZE, 0, (LPARAM)&sz))
//            	{
//            		pos->cx = sz.cx + gpSet->nToolbarAddSpace;
//            		RECT rcParent; GetClientRect(GetParent(hwnd), &rcParent);
//            		pos->x = rcParent.right - pos->cx;
//        		}
//            }
//            return 0;
//        }
//	case TB_GETMAXSIZE:
//	case TB_GETIDEALSIZE:
//		{
//			SIZE *psz = (SIZE*)lParam;
//			if (!lParam) return 0;
//			if (CallWindowProc(_defaultToolProc, hwnd, uMsg, wParam, lParam))
//			{
//				psz->cx += (gpSet->isHideCaptionAlways() ? gpSet->nToolbarAddSpace : 0);
//				return 1;
//			}
//		} break;
//	case WM_RBUTTONUP:
//		{
//			POINT pt = {LOWORD(lParam),HIWORD(lParam)};
//			int nIdx = SendMessage(hwnd, TB_HITTEST, 0, (LPARAM)&pt);
//			// If the return value is zero or a positive value, it is 
//			// the zero-based index of the nonseparator item in which the point lies.
//			if (nIdx >= 0 && nIdx < MAX_CONSOLE_COUNT)
//			{
//				// ���� ������ �� ������������� - ��� ����� ������ ����� �������
//				CVirtualConsole* pVCon = gpConEmu->GetVCon(nIdx);
//				if (!gpConEmu->isActive(pVCon))
//				{
//					if (!gpConEmu->ConActivate(nIdx))
//					{
//						if (!gpConEmu->isActive(pVCon))
//						{
//							return 0;
//						}
//					}
//				}
//				ClientToScreen(hwnd, &pt);
//				pVCon->ShowPopupMenu(pt);
//			}
//			return 0;
//		}
//    }
//    return CallWindowProc(_defaultToolProc, hwnd, uMsg, wParam, lParam);
//}

bool TabBarClass::IsTabsActive()
{
    return mb_Active && gpSet->isTabs;
}

bool TabBarClass::IsTabsShown()
{
    //return _active && IsWindowVisible(mh_Tabbar);
    return IsTabsActive() && gpSet->isTabs;
}

void TabBarClass::Activate(BOOL abPreSyncConsole/*=FALSE*/)
{
#if 1
    mb_Active = true;

	gpConEmu->ReSize(TRUE);
#else
	if (!mh_Rebar)
	{
		CreateRebar();
		OnCaptionHidden();
	}

	_active = true;
	if (abPreSyncConsole && (gpConEmu->WindowMode == wmNormal))
	{
		RECT rcIdeal = gpConEmu->GetIdealRect();
		gpConEmu->SyncConsoleToWindow(&rcIdeal, TRUE);
	}
	UpdatePosition();
#endif
}

void TabBarClass::Deactivate(BOOL abPreSyncConsole/*=FALSE*/)
{
#if 1
    if (!mb_Active)
        return;

	mb_Active = false;
    gpConEmu->ReSize(TRUE);
#else
	if (!_active)
		return;

	_active = false;
	if (abPreSyncConsole && !(gpConEmu->isZoomed() || gpConEmu->isFullScreen()))
	{
		RECT rcIdeal = gpConEmu->GetIdealRect();
		gpConEmu->SyncConsoleToWindow(&rcIdeal, true);
	}
	UpdatePosition();
#endif
}

//void TabBarClass::Update(BOOL abPosted/*=FALSE*/)
//{
//	#ifdef _DEBUG
//	if (this != gpConEmu->mp_TabBar) {
//		_ASSERTE(this == gpConEmu->mp_TabBar);
//	}
//	#endif
//
//	MCHKHEAP 
//    /*if (!_active)
//    {
//        return;
//    }*/ // ������ - ������! �.�. ���� ��������� ��������������
//
//	if (mb_DisableRedraw)
//		return;
//    
//    if (!gpConEmu->isMainThread()) {
//		RequestPostUpdate();
//        return;
//    }
//    
//    mb_PostUpdateCalled = FALSE;
//
//	#ifdef _DEBUG
//	_ASSERTE(mn_InUpdate >= 0);
//	if (mn_InUpdate > 0) {
//		_ASSERTE(mn_InUpdate == 0);
//	}
//	#endif
//	mn_InUpdate ++;
//
//    ConEmuTab tab = {0};
//    
//	MCHKHEAP
//    int V, I, tabIdx = 0, nCurTab = -1;
//	CVirtualConsole* pVCon = NULL;
//    VConTabs vct = {NULL};
//
//    // ����������� ������ ������ � �������� ����, ��� ��� CriticalSection �� �����
//    m_Tab2VCon.clear();
//	_ASSERTE(m_Tab2VCon.size()==0);
//
//	#ifdef _DEBUG
//	if (this != gpConEmu->mp_TabBar) {
//		_ASSERTE(this == gpConEmu->mp_TabBar);
//	}
//	#endif
//
//	TODO("��������� gpSet->bHideInactiveConsoleTabs");
//
//	MCHKHEAP
//	if (!gpConEmu->mp_TabBar->IsActive() && gpSet->isTabs)
//	{
//		int nTabs = 0;
//		for (V = 0; V < MAX_CONSOLE_COUNT && nTabs < 2; V++)
//		{
//			_ASSERTE(m_Tab2VCon.size()==0);
//			if (!(pVCon = gpConEmu->GetVCon(V))) continue;
//			if (gpSet->bHideInactiveConsoleTabs) {
//				if (!gpConEmu->isActive(pVCon)) continue;
//			}
//			_ASSERTE(m_Tab2VCon.size()==0);
//			nTabs += pVCon->RCon()->GetTabCount();
//			_ASSERTE(m_Tab2VCon.size()==0);
//		}
//		if (nTabs > 1)
//		{
//			_ASSERTE(m_Tab2VCon.size()==0);
//			Activate();
//			_ASSERTE(m_Tab2VCon.size()==0);
//		}
//	}
//	else if (gpConEmu->mp_TabBar->IsActive() && gpSet->isTabs==2)
//	{
//		int nTabs = 0;
//		for (V = 0; V < MAX_CONSOLE_COUNT && nTabs < 2; V++)
//		{
//			_ASSERTE(m_Tab2VCon.size()==0);
//			if (!(pVCon = gpConEmu->GetVCon(V))) continue;
//			if (gpSet->bHideInactiveConsoleTabs)
//			{
//				if (!gpConEmu->isActive(pVCon)) continue;
//			}
//			_ASSERTE(m_Tab2VCon.size()==0);
//			nTabs += pVCon->RCon()->GetTabCount();
//			_ASSERTE(m_Tab2VCon.size()==0);
//		}
//		if (nTabs <= 1)
//		{
//			_ASSERTE(m_Tab2VCon.size()==0);
//			Deactivate();
//			_ASSERTE(m_Tab2VCon.size()==0);
//		}
//	}
//
//	#ifdef _DEBUG
//	if (this != gpConEmu->mp_TabBar)
//	{
//		_ASSERTE(this == gpConEmu->mp_TabBar);
//	}
//	#endif
//
//	MCHKHEAP
//	_ASSERTE(m_Tab2VCon.size()==0);
//
//    for (V = 0; V < MAX_CONSOLE_COUNT; V++)
//	{
//        if (!(pVCon = gpConEmu->GetVCon(V))) continue;
//		BOOL lbActive = gpConEmu->isActive(pVCon);
//
//		if (gpSet->bHideInactiveConsoleTabs)
//		{
//			if (!lbActive) continue;
//		}
//
//		CRealConsole *pRCon = pVCon->RCon();
//        
//        for (I = 0; TRUE; I++)
//		{
//			#ifdef _DEBUG
//			if (!I && !V)
//			{
//				_ASSERTE(m_Tab2VCon.size()==0);
//			}
//
//			if (this != gpConEmu->mp_TabBar)
//			{
//				_ASSERTE(this == gpConEmu->mp_TabBar);
//			}
//			MCHKHEAP;
//			#endif
//
//			if (!pRCon->GetTab(I, &tab))
//				break;
//
//			#ifdef _DEBUG
//			if (this != gpConEmu->mp_TabBar)
//			{
//				_ASSERTE(this == gpConEmu->mp_TabBar);
//			}
//			MCHKHEAP;
//			#endif
//
//            PrepareTab(&tab);
//            
//            vct.pVCon = pVCon;
//            vct.nFarWindowId = I;
//
//			#ifdef _DEBUG
//			if (!I && !V)
//			{
//				_ASSERTE(m_Tab2VCon.size()==0);
//			}
//			#endif
//
//            AddTab2VCon(vct);
//            // ��������� ��������, ��� ������ (��� �������������) ��������� ������������
//            AddTab(tab.Name, tabIdx, (tab.Type & 0x100)==0x100);
//            
//            if (lbActive && tab.Current)
//                nCurTab = tabIdx;
//            
//            tabIdx++;
//
//			#ifdef _DEBUG
//			if (this != gpConEmu->mp_TabBar)
//			{
//				_ASSERTE(this == gpConEmu->mp_TabBar);
//			}
//			#endif
//        }
//    }
//	MCHKHEAP
//    if (tabIdx == 0) // ���� �� "Console" �������
//    {
//        PrepareTab(&tab);
//        
//        vct.pVCon = NULL;
//        vct.nFarWindowId = 0;
//		AddTab2VCon(vct); //2009-06-14. �� ����!
//
//        // ��������� ��������, ��� ������ (��� �������������) ��������� ������������
//        AddTab(tab.Name, tabIdx, (tab.Type & 0x100)==0x100);
//        nCurTab = tabIdx;
//        tabIdx++;
//    }
//
//	// Update ��������� ���������
//	if (nCurTab >= 0 && (UINT)nCurTab < m_Tab2VCon.size())
//		AddStack(m_Tab2VCon[nCurTab]);
//	else
//		CheckStack(); // ����� ������ �������� ����
//
//    // ������� ������ �������� (���������)
//    int nCurCount = GetItemCount();
//	#ifdef _DEBUG
//	wchar_t szDbg[128];
//	wsprintf(szDbg, L"TabBarClass::Update.  ItemCount=%i, PrevItemCount=%i\n", tabIdx, nCurCount);
//	DEBUGSTRTABS(szDbg);
//	#endif
//	for (I = tabIdx; I < nCurCount; I++)
//	{
//		#ifdef _DEBUG
//		wsprintf(szDbg, L"   Deleting tab=%i\n", I+1);
//		DEBUGSTRTABS(szDbg);
//		#endif
//        DeleteItem(tabIdx);
//	}
//
//	MCHKHEAP
//    if (mb_InKeySwitching)
//	{
//	    if (mn_CurSelTab >= nCurCount) // ���� ��������� ��� ����� �� �������
//		    mb_InKeySwitching = FALSE;
//    }
//        
//    if (!mb_InKeySwitching && nCurTab != -1)
//	{
//        SelectTab(nCurTab);
//    }
//
//	if (gpSet->isTabsInCaption)
//	{
//		SendMessage(ghWnd, WM_NCPAINT, 0, 0);
//	}
//
//	mn_InUpdate --;
//	if (mb_PostUpdateRequested)
//	{
//		mb_PostUpdateCalled = FALSE;
//		mb_PostUpdateRequested = FALSE;
//		RequestPostUpdate();
//	}
//	MCHKHEAP
//}
//
//void TabBarClass::AddTab2VCon(VConTabs& vct)
//{
//	#ifdef _DEBUG
//	std::vector<VConTabs>::iterator i = m_Tab2VCon.begin();
//	while (i != m_Tab2VCon.end())
//	{
//		_ASSERTE(i->pVCon!=vct.pVCon || i->nFarWindowId!=vct.nFarWindowId);
//		i++;
//	}
//	#endif
//	m_Tab2VCon.push_back(vct);
//}
//
//RECT TabBarClass::GetMargins()
//{
//    return m_Margins;
//}
//
//void TabBarClass::UpdatePosition()
//{
//	if (gpConEmu->isIconic())
//		return; // ����� ������ �������� ����� ������������!
//
//    RECT client;
//    GetClientRect(ghWnd, &client); // ��� ���������� ������ ����
//
//	DEBUGSTRTABS(_active ? L"TabBarClass::UpdatePosition(activate)\n" : L"TabBarClass::UpdatePosition(DEactivate)\n");
//    
//	#ifdef _DEBUG
//	DWORD_PTR dwStyle = GetWindowLongPtr(ghWnd, GWL_STYLE);
//	#endif
//
//    if (_active) {
//        if (mh_Rebar) {
//            if (!IsWindowVisible(mh_Rebar))
//                apiShowWindow(mh_Rebar, SW_SHOW);
//            //MoveWindow(mh_Rebar, 0, 0, client.right, _tabHeight, 1);
//        } else {
//            if (!IsWindowVisible(mh_Tabbar))
//                apiShowWindow(mh_Tabbar, SW_SHOW);
//            if (gpSet->isTabFrame)
//                MoveWindow(mh_Tabbar, 0, 0, client.right, client.bottom, 1);
//            else
//                MoveWindow(mh_Tabbar, 0, 0, client.right, _tabHeight, 1);
//
//        }
//
//		#ifdef _DEBUG
//		dwStyle = GetWindowLongPtr(ghWnd, GWL_STYLE);
//		#endif
//
//		//gpConEmu->SyncConsoleToWindow(); -- 2009.07.04 Sync ������ ���� �������� � ����� ReSize
//		gpConEmu->ReSize(TRUE);
//    } else {
//		//gpConEmu->SyncConsoleToWindow(); -- 2009.07.04 Sync ������ ���� �������� � ����� ReSize
//		gpConEmu->ReSize(TRUE);
//		// _active ��� ��������, ������� ������� �������� ����� � �����
//        if (mh_Rebar) {
//            if (IsWindowVisible(mh_Rebar))
//                apiShowWindow(mh_Rebar, SW_HIDE);
//        } else {
//            if (IsWindowVisible(mh_Tabbar))
//                apiShowWindow(mh_Tabbar, SW_HIDE);
//        }
//    }
//}
//
//void TabBarClass::UpdateWidth()
//{
//    if (!_active)
//    {
//        return;
//    }
//    RECT client, self;
//    GetClientRect(ghWnd, &client);
//    GetWindowRect(mh_Tabbar, &self);
//    if (mh_Rebar) {
//		SIZE sz = {0,0};
//		int nBarIndex = -1;
//		BOOL lbNeedShow = FALSE, lbWideEnough = FALSE;
//		if (mh_Toolbar) {
//			nBarIndex = SendMessage(mh_Rebar, RB_IDTOINDEX, 2, 0);
//			SendMessage(mh_Toolbar, TB_GETMAXSIZE, 0, (LPARAM)&sz);
//			sz.cx += (gpSet->isHideCaptionAlways() ? gpSet->nToolbarAddSpace : 0);
//			lbWideEnough = (sz.cx + 150) <= client.right;
//			if (!lbWideEnough) {
//				if (IsWindowVisible(mh_Toolbar))
//					SendMessage(mh_Rebar, RB_SHOWBAND, nBarIndex, 0);
//			} else {
//				if (!IsWindowVisible(mh_Toolbar))
//					lbNeedShow = TRUE;
//			}
//		}
//        MoveWindow(mh_Rebar, 0, 0, client.right, _tabHeight, 1);
//		if (lbWideEnough && nBarIndex != 1) {
//			SendMessage(mh_Rebar, RB_MOVEBAND, nBarIndex, 1);
//			nBarIndex = SendMessage(mh_Rebar, RB_IDTOINDEX, 2, 0);
//			_ASSERTE(nBarIndex == 1);
//		}
//		if (lbNeedShow) {
//			SendMessage(mh_Rebar, RB_SHOWBAND, nBarIndex, 1);
//		}
//    } else
//    if (gpSet->isTabFrame) {
//        MoveWindow(mh_Tabbar, 0, 0, client.right, client.bottom, 1);
//    } else {
//        MoveWindow(mh_Tabbar, 0, 0, client.right, _tabHeight, 1);
//    }
//
//    UpdateToolbarPos();
//}
//
//void TabBarClass::UpdateToolbarPos()
//{
//    if (mh_Toolbar) {
//        SIZE sz; 
//        SendMessage(mh_Toolbar, TB_GETMAXSIZE, 0, (LPARAM)&sz);
//        if (mh_Rebar) {
//            if (sz.cx != mn_LastToolbarWidth)
//            {
//                REBARBANDINFO rbBand={80}; // �� ���������� size, �.�. �������� "�����" ������ �� ����� � � XP ������������
//                rbBand.fMask  = RBBIM_SIZE | RBBIM_CHILDSIZE;
//                // Set values unique to the band with the toolbar.
//                rbBand.cx = rbBand.cxMinChild = rbBand.cxIdeal = mn_LastToolbarWidth = sz.cx;
//                rbBand.cyMinChild = sz.cy;
//
//                // Add the band that has the toolbar.
//                SendMessage(mh_Rebar, RB_SETBANDINFO, 1, (LPARAM)&rbBand);
//            }
//        } else {
//            RECT rcClient;
//            GetWindowRect(mh_Tabbar, &rcClient);
//            MapWindowPoints(NULL, ghWnd, (LPPOINT)&rcClient, 2);
//        }
//    }
//}

//bool TabBarClass::OnNotify(LPNMHDR nmhdr)
//{
//	if (!this)
//		return false;
//    if (!_active)
//    {
//        return false;
//    }
//
//    //SetFocus(ghWnd); // 02.04.2009 Maks - ?
//    if (nmhdr->code == TCN_SELCHANGING)
//    {
//        //if (mb_ChangeAllowed) {
//        //  return FALSE;
//        //}
//        _prevTab = GetCurSel(); 
//        return FALSE; // ���������
//    }
//
//    if (nmhdr->code == TCN_SELCHANGE)
//    {
//        int lnNewTab = GetCurSel();
//        //_tcscpy(_lastTitle, gpConEmu->Title);
//        
//        if (_prevTab>=0) {
//            SelectTab(_prevTab);
//            _prevTab = -1;
//        }
//        
//        if (mb_ChangeAllowed) {
//            return FALSE;
//        }
//        
//        OnTabSelected(lnNewTab);
//        // start waiting for title to change
//        _titleShouldChange = true;
//        return true;
//    }
//
//    if (nmhdr->code == TBN_GETINFOTIP /*&& nmhdr->hwndFrom == mh_Toolbar*/)
//    {
//        if (!gpSet->isMulti)
//            return 0;
//        LPNMTBGETINFOTIP pDisp = (LPNMTBGETINFOTIP)nmhdr;
//        if (pDisp->iItem>=1 && pDisp->iItem<=MAX_CONSOLE_COUNT) {
//            if (!pDisp->pszText || !pDisp->cchTextMax) return false;
//            LPCWSTR pszTitle = gpConEmu->GetVCon(pDisp->iItem-1)->RCon()->GetTitle();
//            if (pszTitle) {
//                lstrcpyn(pDisp->pszText, pszTitle, pDisp->cchTextMax);
//            } else {
//                pDisp->pszText[0] = 0;
//            }
//        } else
//        if (pDisp->iItem == TID_CREATE_CON) {
//            lstrcpyn(pDisp->pszText, _T("Create new console"), pDisp->cchTextMax);
//        } else
//        if (pDisp->iItem == TID_BUFFERHEIGHT) {
//	        BOOL lbPressed = (SendMessage(mh_Toolbar, TB_GETSTATE, pDisp->iItem, 0) & TBSTATE_CHECKED) == TBSTATE_CHECKED;
//            lstrcpyn(pDisp->pszText, 
//	            lbPressed ? L"BufferHeight mode is ON" : L"BufferHeight mode is off",
//	            pDisp->cchTextMax);
//        } else
//        if (pDisp->iItem == TID_MINIMIZE) {
//        	lstrcpyn(pDisp->pszText, _T("Minimize window"), pDisp->cchTextMax);
//        } else 
//        if (pDisp->iItem == TID_MAXIMIZE) {
//			lstrcpyn(pDisp->pszText, _T("Maximize window"), pDisp->cchTextMax);
//        } else
//        if (pDisp->iItem == TID_APPCLOSE) {
//        	lstrcpyn(pDisp->pszText, _T("Close ALL consoles"), pDisp->cchTextMax);
//        }
//        return true;
//    }
//
//    if (nmhdr->code == TTN_GETDISPINFO 
//		&& (mh_Tabbar && (nmhdr->hwndFrom == mh_Tabbar || nmhdr->hwndFrom == mh_TabTip)))
//    {
//        LPNMTTDISPINFO pDisp = (LPNMTTDISPINFO)nmhdr;
//        CVirtualConsole *pVCon = NULL;
//        DWORD wndIndex = 0;
//        TCHITTESTINFO htInfo;
//        
//        pDisp->hinst = NULL;
//        pDisp->szText[0] = 0;
//        pDisp->lpszText = NULL;
//        
//        GetCursorPos(&htInfo.pt);
//        MapWindowPoints(NULL, mh_Tabbar, &htInfo.pt, 1);
//        
//        int iPage = TabCtrl_HitTest(mh_Tabbar, &htInfo);
//        
//        if (iPage >= 0) {
//            // ���� � ���� ��� "�" - ��� �� �����
//            if (!wcschr(GetTabText(iPage), L'\x2026' /*"�"*/))
//                return 0;
//        
//            if (!GetVConFromTab(iPage, &pVCon, &wndIndex))
//                return 0;
//            if (!pVCon->RCon()->GetTab(wndIndex, &m_Tab4Tip))
//                return 0;
//        
//            pDisp->lpszText = m_Tab4Tip.Name;
//        }
//        
//        return true;
//    }
//
//    return false;
//}

//LPCWSTR TabBarClass::GetTabText(int nTabIdx)
//{
//    TCITEM item = {TCIF_TEXT};
//    item.pszText = ms_TmpTabText; item.cchTextMax = sizeof(ms_TmpTabText)/sizeof(ms_TmpTabText[0]);
//    ms_TmpTabText[0] = 0;
//    
//    if (!TabCtrl_GetItem(mh_Tabbar, nTabIdx, &item))
//        return L"";
//        
//    return ms_TmpTabText;
//}

//void TabBarClass::OnCommand(WPARAM wParam, LPARAM lParam)
//{
//	if (!this)
//		return;
//    if (mh_Toolbar != (HWND)lParam)
//        return;
//
//	if (!gpSet->isMulti)
//        return;
//
//    if (wParam>=1 && wParam<=MAX_CONSOLE_COUNT) {
//        gpConEmu->ConActivate(wParam-1);
//    } else if (wParam == TID_CREATE_CON) {
//        gpConEmu->Recreate ( FALSE, gpSet->isMultiNewConfirm );
//    } else if (wParam == TID_BUFFERHEIGHT) {
//		SendMessage(mh_Toolbar, TB_CHECKBUTTON, TID_BUFFERHEIGHT, gpConEmu->ActiveCon()->RCon()->isBufferHeight());
//		gpConEmu->AskChangeBufferHeight();
//    } else
//    if (wParam == TID_MINIMIZE) {
//    	PostMessage(ghWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
//    } else 
//    if (wParam == TID_MAXIMIZE) {
//		// ����� ���� �������� �� ���������� � �������
//		gpConEmu->mouse.state |= MOUSE_SIZING_DBLCKL;
//		// ������ AltF9
//		gpConEmu->OnAltF9(TRUE);
//    } else
//    if (wParam == TID_APPCLOSE) {
//    	PostMessage(ghWnd, WM_SYSCOMMAND, SC_CLOSE, 0);
//    }
//}

//void TabBarClass::OnMouse(int message, int x, int y)
//{
//	if (!this)
//		return;
//    if (!_active)
//    {
//        return;
//    }
//
//    if (message == WM_MBUTTONUP || message == WM_RBUTTONUP)
//    {
//        TCHITTESTINFO htInfo;
//        htInfo.pt.x = x;
//        htInfo.pt.y = y;
//        int iPage = TabCtrl_HitTest(mh_Tabbar, &htInfo);
//        if (iPage != -1)
//        {
//            CVirtualConsole* pVCon = NULL;
//            // ��� ���� ����� �������� ����������, ������� �� �����, ����� ������� �������� �� �����
//            // � �� ����� ����� �����, �� ����� ��������� ����...
//            POINT ptCur = {0,0}; GetCursorPos(&ptCur);
//
//			pVCon = OnTabSelected(iPage);
//
//			if (pVCon) {
//				BOOL lbCtrlPressed = isPressed(VK_CONTROL);
//				if (message == WM_RBUTTONUP && !lbCtrlPressed) {
//					pVCon->ShowPopupMenu(ptCur);
//				} else {
//					if (pVCon->RCon()->GetFarPID()) {
//						pVCon->RCon()->PostMacro(gpSet->sTabCloseMacro ? gpSet->sTabCloseMacro : L"F10");
//					} else {
//						// ���� ������� CMD, PowerShell, � �.�. - �������� ������ ������������ �������
//						// ��� ���� ������ Terminate & Recreate
//						gpConEmu->Recreate ( TRUE, TRUE );
//					}
//				}
//			}
//        }
//    }
//}

//void TabBarClass::Invalidate()
//{
//    if (gpConEmu->mp_TabBar->IsActive())
//        InvalidateRect(mh_Rebar, NULL, TRUE);
//}

//void TabBarClass::OnCaptionHidden()
//{
//	if (!this) return;
//	if (mh_Toolbar)
//	{
//		BOOL lbHide = !(gpSet->isHideCaptionAlways() 
//			|| gpSet->isFullScreen 
//			|| (gpConEmu->isZoomed() && gpSet->isHideCaption));
//		SendMessage(mh_Toolbar, TB_HIDEBUTTON, TID_MINIMIZE_SEP, lbHide);
//		SendMessage(mh_Toolbar, TB_HIDEBUTTON, TID_MINIMIZE, lbHide);
//		SendMessage(mh_Toolbar, TB_HIDEBUTTON, TID_MAXIMIZE, lbHide);
//		SendMessage(mh_Toolbar, TB_HIDEBUTTON, TID_APPCLOSE, lbHide);
//		
//		SendMessage(mh_Toolbar, TB_AUTOSIZE, 0, 0);
//		
//		UpdateToolbarPos();
//	}
//}

//void TabBarClass::OnWindowStateChanged()
//{
//	if (!this) return;
//	if (mh_Toolbar)
//	{
//		TBBUTTONINFO tbi = {sizeof(TBBUTTONINFO), TBIF_IMAGE};
//		tbi.iImage = (gpSet->isFullScreen || gpConEmu->isZoomed()) ? BID_MAXIMIZE_IDX : BID_RESTORE_IDX;
//		SendMessage(mh_Toolbar, TB_SETBUTTONINFO, TID_MAXIMIZE, (LPARAM)&tbi);
//
//		OnCaptionHidden();
//	}
//}

//// nConNumber - 1based
//void TabBarClass::OnConsoleActivated(int nConNumber/*, BOOL bAlternative*/)
//{
//    if (!mh_Toolbar) return;
//
//    BOOL bPresent[MAX_CONSOLE_COUNT]; memset(bPresent, 0, sizeof(bPresent));
//    MCHKHEAP
//    for (int i=1; i<=MAX_CONSOLE_COUNT; i++)
//	{
//        bPresent[i-1] = gpConEmu->GetVConTitle(i-1) != NULL;
//    }
//
//    SendMessage(mh_Toolbar, WM_SETREDRAW, 0, 0);
//    for (int i=1; i<=MAX_CONSOLE_COUNT; i++)
//	{
//        SendMessage(mh_Toolbar, TB_HIDEBUTTON, i, !bPresent[i-1]);
//    }
//
//    UpdateToolbarPos();
//    SendMessage(mh_Toolbar, WM_SETREDRAW, 1, 0);
//
//    //nConNumber = gpConEmu->ActiveConNum()+1; -- ���� ������ ��� ���������� �����!
//    
//    if (nConNumber>=1 && nConNumber<=MAX_CONSOLE_COUNT)
//	{
//        SendMessage(mh_Toolbar, TB_CHECKBUTTON, nConNumber, 1);
//    }
//	else
//	{
//        for (int i=1; i<=MAX_CONSOLE_COUNT; i++)
//            SendMessage(mh_Toolbar, TB_CHECKBUTTON, i, 0);
//    }
//}

//void TabBarClass::OnBufferHeight(BOOL abBufferHeight)
//{
//	//if (!mh_Toolbar) return;
//	//
//    //SendMessage(mh_Toolbar, TB_CHECKBUTTON, TID_BUFFERHEIGHT, abBufferHeight);
//}

//void TabBarClass::PrepareTab(ConEmuTab* pTab)
//{
//	#ifdef _DEBUG
//	if (this != gpConEmu->mp_TabBar)
//	{
//		_ASSERTE(this == gpConEmu->mp_TabBar);
//	}
//	#endif
//
//	MCHKHEAP
//    // get file name
//    TCHAR dummy[MAX_PATH*2];
//    TCHAR fileName[MAX_PATH+4];
//    TCHAR szFormat[32];
//    TCHAR szEllip[MAX_PATH+1];
//    wchar_t *tFileName=NULL, *pszNo=NULL, *pszTitle=NULL; //--Maximus
//    int nSplit = 0;
//    int nMaxLen = 0; //gpSet->nTabLenMax - _tcslen(szFormat) + 2/* %s */;
//    int origLength = 0; //_tcslen(tFileName);
//    if (pTab->Name[0]==0 || (pTab->Type & 0xFF) == 1/*WTYPE_PANELS*/)
//	{
//	    //_tcscpy(szFormat, _T("%s"));
//	    _tcscpy(szFormat, gpSet->szTabConsole);
//	    nMaxLen = gpSet->nTabLenMax - _tcslen(szFormat) + 2/* %s */;
//	    
//        if (pTab->Name[0] == 0)
//		{
//            #ifdef _DEBUG
//            // ��� ������ ��������� ������ ��� ������������� GUI
//            int nTabCount = GetItemCount();
//            if (nTabCount>0 && gpConEmu->ActiveCon()!=NULL)
//			{
//                //_ASSERTE(pTab->Name[0] != 0);
//                nTabCount = nTabCount;
//            }
//            #endif
//			//100930 - ������. GetLastTitle() ������ ������� �������, � pTab ����� ���� �� ����� �������!
//            // -- _tcscpy(pTab->Name, gpConEmu->GetLastTitle()); //isFar() ? gpSet->szTabPanels : gpSet->pszTabConsole);
//			_tcscpy(pTab->Name, _T("ConEmu"));
//        }
//        tFileName = pTab->Name;
//        origLength = _tcslen(tFileName);
//        //if (origLength>6) {
//	    //    // ����� � ��������� ���� ���-�� ����� "{C:\Program Fil...- Far"
//	    //    //                              ������ "{C:\Program F...} - Far"
//		//	����� ���������� �������� � ��������� ���� - ��� ��� ������� �� ����� � ����� ������... ��� ��� ���� ������ - '...' ������ ������� � �����
//	    //    if (lstrcmp(tFileName + origLength - 6, L" - Far") == 0)
//		//        nSplit = nMaxLen - 6;
//	    //}
//	        
//    }
//	else
//	{
//        GetFullPathName(pTab->Name, MAX_PATH*2, dummy, &tFileName);
//        if (!tFileName)
//            tFileName = pTab->Name;
//
//        if ((pTab->Type & 0xFF) == 3/*WTYPE_EDITOR*/)
//		{
//            if (pTab->Modified)
//                _tcscpy(szFormat, gpSet->szTabEditorModified);
//            else
//                _tcscpy(szFormat, gpSet->szTabEditor);
//        } 
//        else if ((pTab->Type & 0xFF) == 2/*WTYPE_VIEWER*/)
//            _tcscpy(szFormat, gpSet->szTabViewer);
//    }
//    // restrict length
//    if (!nMaxLen)
//	    nMaxLen = gpSet->nTabLenMax - _tcslen(szFormat) + 2/* %s */;
//	if (!origLength)
//	    origLength = _tcslen(tFileName);
//    if (nMaxLen<15) nMaxLen=15; else
//        if (nMaxLen>=MAX_PATH) nMaxLen=MAX_PATH-1;
//    if (origLength > nMaxLen)
//    {
//        /*_tcsnset(fileName, _T('\0'), MAX_PATH);
//        _tcsncat(fileName, tFileName, 10);
//        _tcsncat(fileName, _T("..."), 3);
//        _tcsncat(fileName, tFileName + origLength - 10, 10);*/
//        //if (!nSplit)
//	    //    nSplit = nMaxLen*2/3;
//		//// 2009-09-20 ���� � ��������� ��� ���������� (����������� �����)
//		//const wchar_t* pszAdmin = gpSet->szAdminTitleSuffix;
//		//const wchar_t* pszFrom = tFileName + origLength - (nMaxLen - nSplit);
//		//if (!wcschr(pszFrom, L'.') && (*pszAdmin && !wcsstr(tFileName, pszAdmin)))
//		//{
//		//	// �� ��������� ������� � �����, � �� ��������
//		//	nSplit = nMaxLen;
//		//}
//		
//		// "{C:\Program Files} - Far 2.1283 Administrator x64"
//		// ����� ���������� �������� � ��������� ���� - ��� ��� ������� �� ����� � ����� ������... ��� ��� ���� ������ - '...' ������ ������� � �����
//		nSplit = nMaxLen;
//        
//        _tcsncpy(szEllip, tFileName, nSplit); szEllip[nSplit]=0;
//        szEllip[nSplit] = L'\x2026' /*"�"*/;
//        szEllip[nSplit+1] = 0;
//        //_tcscat(szEllip, L"\x2026" /*"�"*/);
//        //_tcscat(szEllip, tFileName + origLength - (nMaxLen - nSplit));
//        
//        tFileName = szEllip;
//    }
//    // szFormat ����������� ��� Panel/Viewer(*)/Editor(*)
//    // ������: "%i-[%s] *"
//    pszNo = wcsstr(szFormat, L"%i");
//    pszTitle = wcsstr(szFormat, L"%s");
//    if (pszNo == NULL)
//        wsprintf(fileName, szFormat, tFileName);
//    else if (pszNo < pszTitle || pszTitle == NULL)
//        wsprintf(fileName, szFormat, pTab->Pos, tFileName);
//    else
//        wsprintf(fileName, szFormat, tFileName, pTab->Pos);
//
//    wcscpy(pTab->Name, fileName);
//	MCHKHEAP
//}



// ������������ �����

//int TabBarClass::GetIndexByTab(VConTabs tab)
//{
//	int nIdx = -1;
//	std::vector<VConTabs>::iterator iter = m_Tab2VCon.begin();
//	while (iter != m_Tab2VCon.end())
//	{
//		nIdx ++;
//		if (*iter == tab)
//			return nIdx;
//		iter ++;
//	}
//	return -1;
//}

int TabBarClass::GetNextTab(BOOL abForward, BOOL abAltStyle/*=FALSE*/)
{
	_ASSERTE(FALSE); // ����������
	return 0;
//#if 0
//    BOOL lbRecentMode = /*(gpSet->isTabs != 0) &&*/ gpSet->isTabRecent;
//	// GetCurSel ������ ������� HoverTab, ���� ������������ ��� ���� ������ (������������ Ctrl)
//    int nCurSel = GetCurSel();
//    int nCurCount = GetItemCount();
//	VConTabs cur = {NULL};
//    
//	//#ifdef _DEBUG
//	//if (nCurCount != m_Tab2VCon.size())
//	//{
//	//	_ASSERTE(nCurCount == m_Tab2VCon.size());
//	//}
//	//#endif
//    if (nCurCount < 1)
//    	return 0; // ���� ������ � �� ������ ����
//    
//    if (lbRecentMode && nCurSel >= 0 && (UINT)nCurSel < m_Tab2VCon.size())
//        cur = m_Tab2VCon[nCurSel];
//    
//    
//    int i, nNewSel = -1;
//
//    if (abForward)
//	{
//    	if (lbRecentMode)
//		{
//        	std::vector<VConTabs>::iterator iter = m_TabStack.begin();
//        	while (iter != m_TabStack.end())
//			{
//				VConTabs Item = *iter;
//        		// ����� � ����� ���������� ���
//        		if (Item == cur)
//				{
//        			// ���������� ��������� ���, ������� �� ����� ������������
//        			do
//					{
//	        			iter ++; // ���� ����� �� ����� (������ ������� ��������� ���) ������� ������
//    	    			if (iter == m_TabStack.end()) iter = m_TabStack.begin();
//    	    			// ���������� ������ � m_Tab2VCon
//    	    			i = GetIndexByTab ( *iter );
//    	    			if (CanActivateTab(i))
//						{
//    	    				return i;
//        				}
//        			} while (*iter != cur);
//        			break;
//        		}
//				iter ++;
//        	}
//    	} // ���� �� ������ � ����� Recent - ���� ������� �����
//    	
//    
//        for (i = nCurSel+1; nNewSel == -1 && i < nCurCount; i++)
//            if (CanActivateTab(i)) nNewSel = i;
//        
//        for (i = 0; nNewSel == -1 && i < nCurSel; i++)
//            if (CanActivateTab(i)) nNewSel = i;
//
//    }
//	else
//	{
//    
//    	if (lbRecentMode)
//		{
//        	std::vector<VConTabs>::reverse_iterator iter = m_TabStack.rbegin();
//        	while (iter != m_TabStack.rend())
//			{
//				VConTabs Item = *iter;
//        		// ����� � ����� ���������� ���
//        		if (Item == cur)
//				{
//        			// ���������� ��������� ���, ������� �� ����� ������������
//        			do
//					{
//	        			iter ++; // ���� ����� �� ����� (������ ������� ��������� ���) ������� ������
//    	    			if (iter == m_TabStack.rend()) iter = m_TabStack.rbegin();
//    	    			// ���������� ������ � m_Tab2VCon
//    	    			i = GetIndexByTab ( *iter );
//    	    			if (CanActivateTab(i))
//						{
//    	    				return i;
//        				}
//        			} while (*iter != cur);
//        			break;
//        		}
//				iter++;
//        	}
//    	} // ���� �� ������ � ����� Recent - ���� ������� �����
//    
//        for (i = nCurSel-1; nNewSel == -1 && i >= 0; i++)
//            if (CanActivateTab(i)) nNewSel = i;
//        
//        for (i = nCurCount-1; nNewSel == -1 && i > nCurSel; i++)
//            if (CanActivateTab(i)) nNewSel = i;
//
//    }
//
//    return nNewSel;
//#endif
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
    int nNewSel = GetNextTab ( abForward, abAltStyle );
    
    if (nNewSel != -1)
	{
		// mh_Tabbar ����� ���� � ������, �� �������� �������������!
        if (gpSet->isTabLazy && /*mh_Tabbar &&*/ gpSet->isTabs)
		{
            mb_InKeySwitching = TRUE;
            // ���� Ctrl �� ������� - ������ ������������ ���, � �� ����������� �������
            HoverTab ( nNewSel );
        }
		else
		{
            OnTabSelected ( nNewSel );
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
    OnTabSelected(nCurSel);
}

void TabBarClass::SwitchRollback()
{
	if (mb_InKeySwitching)
	{
		mb_InKeySwitching = FALSE;
		Update();
	}
}

//// ����� �� ����� ������, � ������� �����
//void TabBarClass::CheckStack()
//{
//	_ASSERTE(gpConEmu->isMainThread());
//
//	std::vector<VConTabs>::iterator i, j;
//	BOOL lbExist = FALSE;
//
//	j = m_TabStack.begin();
//	while (j != m_TabStack.end())
//	{
//		lbExist = FALSE;
//		for (i = m_Tab2VCon.begin(); i != m_Tab2VCon.end(); i++)
//		{
//			if (*i == *j)
//			{
//				lbExist = TRUE; break;
//			}
//		}
//		if (lbExist)
//			j++;
//		else
//			j = m_TabStack.erase(j);
//	}
//
//	for (i = m_Tab2VCon.begin(); i != m_Tab2VCon.end(); i++)
//	{
//		lbExist = FALSE;
//		for (j = m_TabStack.begin(); j != m_TabStack.end(); j++)
//		{
//			if (*i == *j)
//			{
//				lbExist = TRUE; break;
//			}
//		}
//		if (!lbExist)
//			m_TabStack.push_back(*i);
//	}
//}
//
//// ����� �� ����� ������������� � �������� tab �� ���� �����
//void TabBarClass::AddStack(VConTabs tab)
//{
//	_ASSERTE(gpConEmu->isMainThread());
//	
//	BOOL lbExist = FALSE;
//	if (!m_TabStack.empty())
//	{
//		//VConTabs tmp;
//		std::vector<VConTabs>::iterator iter = m_TabStack.begin();
//		while (iter != m_TabStack.end())
//		{
//			if (*iter == tab)
//			{
//				if (iter == m_TabStack.begin())
//				{
//					lbExist = TRUE;
//				}
//				else
//				{
//					m_TabStack.erase(iter);
//				}
//				break;
//			}
//			iter ++;
//		}
//	}
//	if (!lbExist) // ��������� ������ �����
//		m_TabStack.insert(m_TabStack.begin(), tab);
//
//	CheckStack();
//}
//
//BOOL TabBarClass::CanActivateTab(int nTabIdx)
//{
//    CVirtualConsole *pVCon = NULL;
//    DWORD wndIndex = 0;
//
//    if (!GetVConFromTab(nTabIdx, &pVCon, &wndIndex))
//        return FALSE;
//
//    if (!pVCon->RCon()->CanActivateFarWindow(wndIndex))
//        return FALSE;
//        
//    return TRUE;
//}

//void TabBarClass::SetRedraw(BOOL abEnableRedraw)
//{
//	mb_DisableRedraw = !abEnableRedraw;
//}

void TabBarClass::ShowTabError(LPCTSTR asInfo, int tabIndex)
{
	if (!asInfo)
		ms_TabErrText[0] = 0;
	else if (asInfo != ms_TabErrText)
		lstrcpyn(ms_TabErrText, asInfo, ARRAYSIZE(ms_TabErrText));

	tiBalloon.lpszText = ms_TabErrText;
	if (ms_TabErrText[0])
	{
		//SendMessage(mh_TabTip, TTM_ACTIVATE, FALSE, 0);
		SendMessage(mh_Balloon, TTM_UPDATETIPTEXT, 0, (LPARAM)&tiBalloon);
		//RECT rcControl; GetWindowRect(GetDlgItem(hMain, tFontFace), &rcControl);
		RECT rcTab = {0}; TabGetItemRect(tabIndex, &rcTab);
		MapWindowPoints(ghWnd, NULL, (LPPOINT)&rcTab, 2);
		//int ptx = 0; //rcControl.right - 10;
		//int pty = 0; //(rcControl.top + rcControl.bottom) / 2;
		SendMessage(mh_Balloon, TTM_TRACKPOSITION, 0, MAKELONG(rcTab.left,rcTab.bottom));
		SendMessage(mh_Balloon, TTM_TRACKACTIVATE, TRUE, (LPARAM)&tiBalloon);
		//SetTimer(mh_Tabbar, FAILED_TABBAR_TIMERID, FAILED_TABBAR_TIMEOUT, 0);
	} else {
		SendMessage(mh_Balloon, TTM_TRACKACTIVATE, FALSE, (LPARAM)&tiBalloon);
		//SendMessage(mh_TabTip, TTM_ACTIVATE, TRUE, 0);
	}
}

bool TabBarClass::TabGetItemRect(int tabIndex, LPRECT rcTab)
{
	if (!this)
		return false;
	CTab Tab;
	if (!m_Tabs.GetTabByIndex(tabIndex, Tab))
		return false;
	if (rcTab)
		*rcTab = Tab->DrawInfo.rcTab;
	return true;
}

#ifdef FarSendChangeTab
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

	if (!gpConEmu->isActive(pVCon))
		bNeedActivate = TRUE;

	bChangeOk = pVCon->RCon()->ActivateFarWindow(wndIndex);

	if (!bChangeOk)
	{
		// ������� ������ � ������� - �� ������ ������������
		ShowTabError(L"This tab can't be activated now!", tabIndex);
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

	return pVCon;
}
#endif // #ifdef FarSendChangeTab

bool TabBarClass::Toolbar_GetBtnRect(int nCmd, RECT* rcBtnRect)
{
	_ASSERTE(FALSE); // TODO:
	return false;
}

void TabBarClass::OnShowButtonsChanged()
{
}

void TabBarClass::OnToolbarCommand(LPARAM lParam, int anPaneID, int anCmd, bool abArrow, POINT ptWhere)
{
	TabBarClass* p = (TabBarClass*)lParam;
}

void TabBarClass::OnToolbarMenu(LPARAM lParam, int anPaneID, int anCmd, POINT ptWhere)
{
	TabBarClass* p = (TabBarClass*)lParam;
}

void TabBarClass::OnToolBarDraw(LPARAM lParam, HDC hdc, const RECT& rc, int nPane, int nCmd, DWORD nFlags)
{
	TabBarClass* p = (TabBarClass*)lParam;
}
