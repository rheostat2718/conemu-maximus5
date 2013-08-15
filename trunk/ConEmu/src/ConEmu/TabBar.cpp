
/*
Copyright (c) 2009-2013 Maximus5
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

#define DEBUGSTRTABS(s) //DEBUGSTR(s)

#include <windows.h>
#include <commctrl.h>
#include "header.h"
#include "TabBar.h"
#include "TabCtrlBase.h"
#include "TabCtrlWin.h"
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

//typedef long (WINAPI* ThemeFunction_t)();

CTabBarClass::CTabBarClass()
{
	_active = false;
	_visible = false;
	_tabHeight = 0;
	mn_CurSelTab = 0;
	mb_ForceRecalcHeight = false;
	mb_DisableRedraw = FALSE;
	//memset(&m_Margins, 0, sizeof(m_Margins));
	//m_Margins = gpSet->rcTabMargins; // !! �������� ��������
	//_titleShouldChange = false;
	//mb_Enabled = TRUE;
	mb_PostUpdateCalled = FALSE;
	mb_PostUpdateRequested = FALSE;
	mn_PostUpdateTick = 0;
	//mn_MsgUpdateTabs = RegisterWindowMessage(CONEMUMSG_UPDATETABS);
	memset(&m_Tab4Tip, 0, sizeof(m_Tab4Tip));
	mb_InKeySwitching = FALSE;
	ms_TmpTabText[0] = 0;
	mn_InUpdate = 0;

	mp_Rebar = new CTabPanelWin(this);
}

CTabBarClass::~CTabBarClass()
{
	SafeDelete(mp_Rebar);

	// ���������� ��� ����
	m_Tabs.ReleaseTabs(false);
	m_TabStack.ReleaseTabs(false);
}

void CTabBarClass::RePaint()
{
	mp_Rebar->RePaintInt();
}

//void CTabBarClass::Refresh(BOOL abFarActive)
//{
//    Enable(abFarActive);
//}

void CTabBarClass::Reset()
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

void CTabBarClass::Retrieve()
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
	//if (pipe.Init(_T("CTabBarClass::Retrieve"), TRUE))
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

void CTabBarClass::SelectTab(int i)
{
	mn_CurSelTab = mp_Rebar->SelectTabInt(i); // ������ ���������, ������ ���� ��� ������� ��������
}

int CTabBarClass::GetCurSel()
{
	mn_CurSelTab = mp_Rebar->GetCurSelInt();
	return mn_CurSelTab;
}

int CTabBarClass::GetItemCount()
{
	int nCurCount = 0;

	TODO("����� ������ �� ����� ���������� � mp_Rebar, ���� ������ ��������� �����");
	if (mp_Rebar->IsTabbarCreated())
		nCurCount = mp_Rebar->GetItemCountInt();
	else
		nCurCount = m_Tab2VCon.size();

	return nCurCount;
}

int CTabBarClass::CountActiveTabs(int nMax = 0)
{
	int  nTabs = 0;
	bool bHideInactiveConsoleTabs = gpSet->bHideInactiveConsoleTabs;

	for (int V = 0; V < MAX_CONSOLE_COUNT; V++)
	{
		_ASSERTE(m_Tab2VCon.size()==0);

		CVConGuard guard;
		if (!CVConGroup::GetVCon(V, &guard))
			continue;
		pVCon = guard.VCon();

		if (bHideInactiveConsoleTabs)
		{
			if (!gpConEmu->isActive(pVCon))
				continue;
		}

		_ASSERTE(m_Tab2VCon.size()==0);

		nTabs += pVCon->RCon()->GetTabCount(TRUE);
		
		_ASSERTE(m_Tab2VCon.size()==0);

		if ((nMax > 0) && (nTabs >= nMax))
			break;
	}

	return nTabs;
}

void CTabBarClass::DeleteItem(int I)
{
	if (mp_Rebar->IsTabbarCreated())
	{
		mp_Rebar->DeleteItemInt(I);
	}
}


/*char CTabBarClass::FarTabShortcut(int tabIndex)
{
    return tabIndex < 10 ? '0' + tabIndex : 'A' + tabIndex - 10;
}*/

bool CTabBarClass::NeedPostUpdate()
{
	return (mb_PostUpdateCalled || mb_PostUpdateRequested);
}

void CTabBarClass::RequestPostUpdate()
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
		DEBUGSTRTABS(L"   PostRequesting CTabBarClass::Update\n");
	}
	else
	{
		mb_PostUpdateCalled = TRUE;
		DEBUGSTRTABS(L"   Posting CTabBarClass::Update\n");
		//PostMessage(ghWnd, mn_MsgUpdateTabs, 0, 0);
		gpConEmu->RequestPostUpdateTabs();
		mn_PostUpdateTick = GetTickCount();
	}
}

BOOL CTabBarClass::GetVConFromTab(int nTabIdx, CVirtualConsole** rpVCon, DWORD* rpWndIndex)
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

bool CTabBarClass::IsTabsActive()
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return false;
	}
	return _active;
}

bool CTabBarClass::IsTabsShown()
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return false;
	}

	// ���� "IsWindowVisible(mh_Tabbar)", ��� �����������, �.�. ��� ��������� ��������� ghWnd

	// ���� ������������ ���������� �������!
	return _active && mp_Rebar->IsTabbarCreated() && _visible;
}

void CTabBarClass::Activate(BOOL abPreSyncConsole/*=FALSE*/)
{
	if (!mp_Rebar->IsCreated())
	{
		// �������
		mp_Rebar->CreateRebar();
		// ������ ������ ����������
		_tabHeight = mp_Rebar->QueryTabbarHeight();
		// �����������
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

void CTabBarClass::Deactivate(BOOL abPreSyncConsole/*=FALSE*/)
{
	if (!_active)
		return;

	_active = false;
	if (abPreSyncConsole && !(gpConEmu->isZoomed() || gpConEmu->isFullScreen()))
	{
		RECT rcIdeal = gpConEmu->GetIdealRect();
		CVConGroup::SyncConsoleToWindow(&rcIdeal, true);
	}
	gpConEmu->OnTabbarActivated(false);
	UpdatePosition();
}

void CTabBarClass::Update(BOOL abPosted/*=FALSE*/)
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
	{
		_ASSERTE(mb_DisableRedraw); // ����?
		return;
	}

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

	//ConEmuTab tab = {0};
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


	// Check if we need to AutoSHOW or AutoHIDE tab bar
	if (!IsTabsActive() && gpSet->isTabs)
	{
		int nTabs = CountActiveTabs(2);
		if (nTabs > 1)
		{
			_ASSERTE(m_Tab2VCon.size()==0);
			Activate();
			_ASSERTE(m_Tab2VCon.size()==0);
		}
	}
	else if (IsTabsActive() && gpSet->isTabs==2)
	{
		int nTabs = CountActiveTabs(2);
		if (nTabs <= 1)
		{
			_ASSERTE(m_Tab2VCon.size()==0);
			Deactivate();
			_ASSERTE(m_Tab2VCon.size()==0);
		}
	}


    // Validation?
	#ifdef _DEBUG
	if (this != gpConEmu->mp_TabBar)
	{
		_ASSERTE(this == gpConEmu->mp_TabBar);
	}
	#endif

	MCHKHEAP
	_ASSERTE(m_Tab2VCon.size()==0);



	


	/* ********************* */
	/*          Go           */
	/* ********************* */
	{
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
				mp_Rebar->AddTabInt(tab.Name, tabIdx, (tab.Type & fwt_Elevated)==fwt_Elevated);

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

		Groups.Release();
	}

	MCHKHEAP

	// Must be at least one tab!
	if (tabIdx == 0)
	{
		ZeroStruct(tab);
		PrepareTab(&tab, NULL);
		wcscpy_c(tab.Name, gpConEmu->GetDefaultTitle());
		vct.pVCon = NULL;
		vct.nFarWindowId = 0;
		AddTab2VCon(vct); //2009-06-14. �� ����!
		// ��������� ��������, ��� ������ (��� �������������) ��������� ������������
		mp_Rebar->AddTabInt(tab.Name, tabIdx, (tab.Type & fwt_Elevated)==fwt_Elevated);
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
	_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"CTabBarClass::Update.  ItemCount=%i, PrevItemCount=%i\n", tabIdx, nCurCount);
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

void CTabBarClass::AddTab2VCon(VConTabs& vct)
{
	m_Tab2VCon.push_back(vct);
}

RECT CTabBarClass::GetMargins()
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

void CTabBarClass::UpdatePosition()
{
	if (!mp_Rebar->IsCreated())
		return;

	if (gpConEmu->isIconic())
		return; // ����� ������ �������� ����� ������������!

	DEBUGSTRTABS(_active ? L"CTabBarClass::UpdatePosition(activate)\n" : L"CTabBarClass::UpdatePosition(DEactivate)\n");

	#ifdef _DEBUG
	DWORD_PTR dwStyle = GetWindowLongPtr(ghWnd, GWL_STYLE);
	#endif

	if (_active)
	{
		_visible = true;
		mp_Rebar->ShowBar(true);

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
		mp_Rebar->ShowBar(false);
	}
}

void CTabBarClass::Reposition()
{
	if (!_active)
	{
		return;
	}

	mp_Rebar->RepositionInt();
}

LPCWSTR CTabBarClass::GetTabText(int nTabIdx)
{
	if (!mp_Rebar->GetTabText(nTabIdx, ms_TmpTabText, countof(ms_TmpTabText)))
		return L"";
	return ms_TmpTabText;
}

LRESULT CTabBarClass::OnNotify(LPNMHDR nmhdr)
{
	if (!this)
		return false;

	if (!_active)
	{
		return false;
	}

	LRESULT lResult = 0;

	if (mp_Rebar->OnNotifyInt(nmhdr, lResult))
		return lResult;


	if (nmhdr->code == TBN_GETINFOTIP && mp_Rebar->IsToolbarNotify(nmhdr))
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
			bool lbChecked = mp_Rebar->GetToolBtnChecked(TID_ALTERNATIVE);
			lstrcpyn(pDisp->pszText,
			         lbChecked ? L"Alternative mode is ON (console freezed)" : L"Alternative mode is off",
			         pDisp->cchTextMax);
		}
		else if (pDisp->iItem == TID_SCROLL)
		{
			bool lbChecked = mp_Rebar->GetToolBtnChecked(TID_SCROLL);
			lstrcpyn(pDisp->pszText,
			         lbChecked ? L"BufferHeight mode is ON (scrolling enabled)" : L"BufferHeight mode is off",
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

	if (nmhdr->code == TBN_DROPDOWN && mp_Rebar->IsToolbarNotify(nmhdr))
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

	if (nmhdr->code == TTN_GETDISPINFO && mp_Rebar->IsTabbarNotify(nmhdr))
	{
		LPNMTTDISPINFO pDisp = (LPNMTTDISPINFO)nmhdr;
		CVirtualConsole *pVCon = NULL;
		DWORD wndIndex = 0;
		pDisp->hinst = NULL;
		pDisp->szText[0] = 0;
		pDisp->lpszText = NULL;
		POINT ptScr = {}; GetCursorPos(&ptScr);
		int iPage = mp_Rebar->GetTabFromPoint(ptScr);

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

void CTabBarClass::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (!this)
		return;

	if (!mp_Rebar->IsToolbarCommand(wParam, lParam))
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
		// ������� �� ������ _�������_ ��������� ������
		mp_Rebar->SetToolBtnChecked(TID_ALTERNATIVE, pVCon ? pVCon->RCon()->isAlternative() : false);
		// � ���������� Action
		gpConEmu->AskChangeAlternative();
	}
	else if (wParam == TID_SCROLL)
	{
		CVConGuard VCon;
		CVirtualConsole* pVCon = (gpConEmu->GetActiveVCon(&VCon) >= 0) ? VCon.VCon() : NULL;
		// ������� �� ������ _�������_ ��������� ������
		mp_Rebar->SetToolBtnChecked(TID_SCROLL, pVCon ? pVCon->RCon()->isBufferHeight() : false);
		// � ���������� Action
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

void CTabBarClass::OnMouse(int message, int x, int y)
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
		TCHITTESTINFO htInfo = {{x,y}};
		int iPage = mp_Rebar->GetTabFromPoint(htInfo.pt, false);

		if (iPage >= 0)
		{
			CVirtualConsole* pVCon = NULL;
			// ��� ���� ����� �������� ����������, ������� �� �����, ����� ������� �������� �� �����
			// � �� ����� ����� �����, �� ����� ��������� ����...
			POINT ptCur = {0,0}; GetCursorPos(&ptCur);
			pVCon = mp_Rebar->FarSendChangeTab(iPage);

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

void CTabBarClass::Invalidate()
{
	if (gpConEmu->mp_TabBar->IsTabsActive())
		mp_Rebar->InvalidateBar();
}

void CTabBarClass::OnCaptionHidden()
{
	if (!this) return;

	mp_Rebar->OnCaptionHiddenChanged(gpSet->isCaptionHidden());
}

void CTabBarClass::OnWindowStateChanged()
{
	if (!this) return;

	mp_Rebar->OnWindowStateChanged(gpConEmu->GetWindowMode());
}

// �������� ������� ������ �������� �� �������
void CTabBarClass::UpdateToolConsoles(bool abForcePos/*=false*/)
{
	if (!mp_Rebar->IsToolbarCreated())
		return;

	int nNewActiveIdx = gpConEmu->ActiveConNum(); // 0-based

	OnConsoleActivated(nNewActiveIdx);
}

// nConNumber - 0-based
void CTabBarClass::OnConsoleActivated(int nConNumber)
{
	if (!mp_Rebar->IsToolbarCreated())
		return;

	mp_Rebar->OnConsoleActivatedInt(nConNumber);
}

void CTabBarClass::OnBufferHeight(BOOL abBufferHeight)
{
	if (!mp_Rebar->IsToolbarCreated())
		return;

	mp_Rebar->SetToolBtnChecked(TID_SCROLL, (abBufferHeight!=FALSE));
}

void CTabBarClass::OnAlternative(BOOL abAlternative)
{
	if (!mp_Rebar->IsToolbarCreated())
		return;

	mp_Rebar->SetToolBtnChecked(TID_ALTERNATIVE, (abAlternative!=FALSE));
}

void CTabBarClass::OnShowButtonsChanged()
{
	if (gpSet->isMultiShowButtons)
	{
		mp_Rebar->ShowToolbar(true);
	}

	Reposition();
}

int CTabBarClass::GetTabbarHeight()
{
	if (!this) return 0;

	_ASSERTE(gpSet->isTabs!=0);

	if (mb_ForceRecalcHeight || (_tabHeight == 0))
	{
		// ����� ����������� ������ ����
		_tabHeight = mp_Rebar->QueryTabbarHeight();
	}

	return _tabHeight;
}


void CTabBarClass::PrepareTab(ConEmuTab* pTab, CVirtualConsole *apVCon)
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

int CTabBarClass::GetIndexByTab(VConTabs tab)
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

	return -1;
}

int CTabBarClass::GetNextTab(BOOL abForward, BOOL abAltStyle/*=FALSE*/)
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
			int iter = m_TabStack.size()-1;

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

void CTabBarClass::SwitchNext(BOOL abAltStyle/*=FALSE*/)
{
	Switch(TRUE, abAltStyle);
}

void CTabBarClass::SwitchPrev(BOOL abAltStyle/*=FALSE*/)
{
	Switch(FALSE, abAltStyle);
}

void CTabBarClass::Switch(BOOL abForward, BOOL abAltStyle/*=FALSE*/)
{
	int nNewSel = GetNextTab(abForward, abAltStyle);

	if (nNewSel != -1)
	{
		// mh_Tabbar ����� ���� � ������, �� �������� �������������!
		if (gpSet->isTabLazy && mp_Rebar->IsTabbarCreated() && gpSet->isTabs)
		{
			mb_InKeySwitching = TRUE;
			// ���� Ctrl �� ������� - ������ ������������ ���, � �� ����������� �������
			SelectTab(nNewSel);
		}
		else
		{
			mp_Rebar->FarSendChangeTab(nNewSel);
			mb_InKeySwitching = FALSE;
		}
	}
}

BOOL CTabBarClass::IsInSwitch()
{
	return mb_InKeySwitching;
}

void CTabBarClass::SwitchCommit()
{
	if (!mb_InKeySwitching) return;

	int nCurSel = GetCurSel();
	mb_InKeySwitching = FALSE;
	CVirtualConsole* pVCon = mp_Rebar->FarSendChangeTab(nCurSel);
	UNREFERENCED_PARAMETER(pVCon);
}

void CTabBarClass::SwitchRollback()
{
	if (mb_InKeySwitching)
	{
		mb_InKeySwitching = FALSE;
		Update();
	}
}

// ����� �� ����� ������, � ������� �����
void CTabBarClass::CheckStack()
{
	_ASSERTE(gpConEmu->isMainThread());
	int i, j;

	BOOL lbExist = FALSE;
	j = 0;

	while (j < m_TabStack.size())
	{
		lbExist = FALSE;

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
void CTabBarClass::AddStack(VConTabs tab)
{
	_ASSERTE(gpConEmu->isMainThread());
	BOOL lbExist = FALSE;

	if (!m_TabStack.empty())
	{
		int iter = 0;

		while (iter < m_TabStack.size())
		{
			if (m_TabStack[iter] == tab)
			{
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

BOOL CTabBarClass::CanActivateTab(int nTabIdx)
{
	CVirtualConsole *pVCon = NULL;
	DWORD wndIndex = 0;

	if (!GetVConFromTab(nTabIdx, &pVCon, &wndIndex))
		return FALSE;

	if (!pVCon->RCon()->CanActivateFarWindow(wndIndex))
		return FALSE;

	return TRUE;
}

BOOL CTabBarClass::OnKeyboard(UINT messg, WPARAM wParam, LPARAM lParam)
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

void CTabBarClass::SetRedraw(BOOL abEnableRedraw)
{
	mb_DisableRedraw = !abEnableRedraw;
}

void CTabBarClass::ShowTabError(LPCTSTR asInfo, int tabIndex)
{
	mp_Rebar->ShowTabErrorInt(asInfo, tabIndex);
}

void CTabBarClass::OnChooseTabPopup()
{
	RECT rcBtnRect = {0};
	mp_Rebar->GetToolBtnRect(TID_ACTIVE_NUMBER, &rcBtnRect);
	MapWindowPoints(ghWnd, NULL, (LPPOINT)&rcBtnRect, 2);
	POINT pt = {rcBtnRect.right,rcBtnRect.bottom};

	gpConEmu->ChooseTabFromMenu(FALSE, pt, TPM_RIGHTALIGN|TPM_TOPALIGN);
}

int CTabBarClass::ActiveTabByName(int anType, LPCWSTR asName, CVirtualConsole** ppVCon)
{
	int nTab = -1;
	CVirtualConsole *pVCon = NULL;
	if (ppVCon)
		*ppVCon = NULL;
	
	TODO("CTabBarClass::ActiveTabByName - ����� ��� �� �����");

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
			pVCon = mp_Rebar->FarSendChangeTab(nTab);
			if (!pVCon)
				nTab = -2;
		}
	}
	
	if (ppVCon)
		*ppVCon = pVCon;
	
	return nTab;
}

void CTabBarClass::UpdateTabFont()
{
	mp_Rebar->UpdateTabFontInt();
}

// ������������� � ���������� ����������� ghWnd!
bool CTabBarClass::GetRebarClientRect(RECT* rc)
{
	return mp_Rebar->GetRebarClientRect(rc);
}

bool CTabBarClass::GetActiveTabRect(RECT* rcTab)
{
	bool bSet = false;

	if (!IsTabsShown())
	{
		_ASSERTE(IsTabsShown());
		memset(rcTab, 0, sizeof(*rcTab));
	}
	else
	{
		bSet = mp_Rebar->GetTabRect(-1/*Active*/, rcTab);
	}

	return bSet;
}

bool CTabBarClass::Toolbar_GetBtnRect(int nCmd, RECT* rcBtnRect)
{
	if (!IsTabsShown())
	{
		return false;
	}
	return mp_Rebar->GetToolBtnRect(nCmd, rcBtnRect);
}

LRESULT CTabBarClass::OnTimer(WPARAM wParam)
{
	return mp_Rebar->OnTimerInt(wParam);
}
