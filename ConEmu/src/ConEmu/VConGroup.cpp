
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


#include "Header.h"
#include "../common/common.hpp"
#include "../common/WinObjects.h"
#include "ConEmu.h"
#include "VConGroup.h"
#include "VConChild.h"
#include "Options.h"
#include "TabBar.h"
#include "VirtualConsole.h"
#include "RealConsole.h"
#include "Update.h"
#include "Status.h"

#define DEBUGSTRDRAW(s) //DEBUGSTR(s)
#define DEBUGSTRTABS(s) //DEBUGSTR(s)
#define DEBUGSTRLANG(s) //DEBUGSTR(s)
#define DEBUGSTRERR(s) DEBUGSTR(s)

static CVirtualConsole* gp_VCon[MAX_CONSOLE_COUNT] = {};

static CVirtualConsole* gp_VActive = NULL;
static bool gb_CreatingActive = false, gb_SkipSyncSize = false;
static UINT gn_CreateGroupStartVConIdx = 0;

static CRITICAL_SECTION gcs_VGroups;
static CVConGroup* gp_VGroups[MAX_CONSOLE_COUNT*2] = {}; // �� ������ ��������� ����������� +Parent

//CVirtualConsole* CVConGroup::mp_GrpVCon[MAX_CONSOLE_COUNT] = {};

static COORD g_LastConSize = {0,0}; // console size after last resize (in columns and lines)


void CVConGroup::Initialize()
{
	InitializeCriticalSection(&gcs_VGroups);
}

void CVConGroup::Deinitialize()
{
	DeleteCriticalSection(&gcs_VGroups);
}


// ���������� ��� �������� ������ ����, ��� ��������
CVConGroup* CVConGroup::CreateVConGroup()
{
	_ASSERTE(gpConEmu->isMainThread()); // �� ��������� ����� � ��������?
	CVConGroup* pGroup = new CVConGroup(NULL);
	return pGroup;
}

CVConGroup* CVConGroup::SplitVConGroup(RConStartArgs::SplitType aSplitType /*eSplitHorz/eSplitVert*/, UINT anPercent10 /*= 500*/)
{
	if (!this || !(aSplitType == RConStartArgs::eSplitHorz || aSplitType == RConStartArgs::eSplitVert))
	{
		_ASSERTE(this);
		return NULL;
	}

	if (mp_Item == NULL)
	{
		_ASSERTE(mp_Item && "VCon was not associated");
		return NULL;
	}
	
	// ��������� ����� ������ ��, ��� ��� �� ������� ("������")
	if (m_SplitType != RConStartArgs::eSplitNone)
	{
		MBoxAssert(m_SplitType == RConStartArgs::eSplitNone && "Can't split this pane");
		return CreateVConGroup();
	}
	
	// ������� ��� ������ ������ ��� ��������
	_ASSERTE(mp_Grp1==NULL && mp_Grp2==NULL);
	mp_Grp1 = new CVConGroup(this);
	mp_Grp2 = new CVConGroup(this);
	if (!mp_Grp1 || !mp_Grp2)
	{
		_ASSERTE(mp_Grp1 && mp_Grp2);
		SafeDelete(mp_Grp1);
		SafeDelete(mp_Grp2);
		return NULL;
	}

	// ��������� ���������
	m_SplitType = aSplitType; // eSplitNone/eSplitHorz/eSplitVert
	mn_SplitPercent10 = max(1,min(anPercent10,999)); // (0.1% - 99.9%)*10
	mrc_Splitter = MakeRect(0,0);

	// ��������� � mp_Grp1 ������� VCon, mp_Grp2 - ����� ����� ������ (����) �������
	mp_Grp1->mp_Item = mp_Item;
	mp_Item->mp_Group = mp_Grp1;
	mp_Item = NULL; // ������������ �� VCon, ��� ���������� ������ ���������� mp_Grp1

	SetResizeFlags();

	return mp_Grp2;
}


CVirtualConsole* CVConGroup::CreateVCon(RConStartArgs *args, CVirtualConsole*& ppVConI)
{
	_ASSERTE(ppVConI == NULL);
	if (!args)
	{
		_ASSERTE(args!=NULL);
		return NULL;
	}

	_ASSERTE(gpConEmu->isMainThread()); // �� ��������� ����� � ��������?

	if (args->pszSpecialCmd)
	{
		args->ProcessNewConArg();
	}

	if (args->bForceUserDialog)
	{
		_ASSERTE(args->aRecreate!=cra_RecreateTab);
		args->aRecreate = cra_CreateTab;

		int nRc = gpConEmu->RecreateDlg(args);
		if (nRc != IDC_START)
			return NULL;

		// ����� ������� ����� ���������� ��������� ������
	}

	CVConGroup* pGroup = NULL;
	if (gp_VActive && args->eSplit)
	{
		CVConGuard VCon;
		if (((args->nSplitPane && GetVCon(gn_CreateGroupStartVConIdx+args->nSplitPane-1, &VCon))
				|| (GetActiveVCon(&VCon) >= 0))
			&& VCon->mp_Group)
		{
			pGroup = ((CVConGroup*)VCon->mp_Group)->SplitVConGroup(args->eSplit, args->nSplitValue);
		}
		
		_ASSERTE((pGroup!=NULL) && "No active VCon?");
	}
	// Check
	if (!pGroup)
	{
		pGroup = CreateVConGroup();

		if (!pGroup)
			return NULL;
	}

	CVirtualConsole* pCon = new CVirtualConsole();
	ppVConI = pCon;
	pGroup->mp_Item = pCon;
	pCon->mp_Group = pGroup;
	pCon->Constructor(args);

	if (!pCon->mp_RCon->PreCreate(args))
	{
		delete pCon;
		return NULL;
	}

	pGroup->GetRootGroup()->InvalidateAll();

	return pCon;
}


CVConGroup::CVConGroup(CVConGroup *apParent)
{
	mp_Item = NULL/*apVCon*/;     // �������, � ������� �������� ���� "Pane"
	//apVCon->mp_Group = this;
	m_SplitType = RConStartArgs::eSplitNone;
	mn_SplitPercent10 = 500; // Default - �������
	mrc_Splitter = MakeRect(0,0);
	mp_Grp1 = mp_Grp2 = NULL; // ������ �� "��������" ������
	mp_Parent = apParent; // ������ �� "������������" ������

	EnterCriticalSection(&gcs_VGroups);
	bool bAdded = false;
	for (size_t i = 0; i < countof(gp_VGroups); i++)
	{
		if (gp_VGroups[i] == NULL)
		{
			gp_VGroups[i] = this;
			bAdded = true;
			break;
		}
	}
	_ASSERTE(bAdded && "gp_VGroups overflow");
	LeaveCriticalSection(&gcs_VGroups);
}

CVConGroup::~CVConGroup()
{
	_ASSERTE(gpConEmu->isMainThread()); // �� ��������� ����� � ��������?

	// �� ������ ���� �������� �������
	_ASSERTE(mp_Grp1==NULL && mp_Grp2==NULL);
	//SafeDelete(mp_Grp1);
	//SafeDelete(mp_Grp2);

	EnterCriticalSection(&gcs_VGroups);

	if (mp_Parent)
	{
		// ���� ���� �������� - �� ����� ���������� "AnotherPane" � ��������,
		// ����� �� ���� �������� � ������ �������.
		CVConGroup* p = GetAnotherGroup();
		if (!p)
		{
			_ASSERTE(p);
		}
		else
		{
			p->MoveToParent(mp_Parent);
			delete p;
		}
	}

	bool bRemoved = false;
	for (size_t i = 0; i < countof(gp_VGroups); i++)
	{
		if (gp_VGroups[i] == this)
		{
			gp_VGroups[i] = NULL;
			_ASSERTE(!bRemoved);
			bRemoved = true;
		}
	}
	_ASSERTE(bRemoved && "Was not pushed in gp_VGroups?");

	LeaveCriticalSection(&gcs_VGroups);
}

void CVConGroup::OnVConDestroyed(CVirtualConsole* apVCon)
{
	if (apVCon && apVCon->mp_Group)
	{
		CVConGroup* p = (CVConGroup*)apVCon->mp_Group;
		apVCon->mp_Group = NULL;
		delete p;
	}
}

CVConGroup* CVConGroup::GetRootGroup()
{
	if (!this)
	{
		_ASSERTE(FALSE);
		return NULL;
	}

	CVConGroup* p = this;
	while (p->mp_Parent)
	{
		p = p->mp_Parent;
	}

	_ASSERTE(p && (p->mp_Parent == NULL));
	return p;
}

CVConGroup* CVConGroup::GetRootOfVCon(CVirtualConsole* apVCon)
{
	CVConGuard VCon(apVCon);

	if (!apVCon || !apVCon->mp_Group)
	{
		_ASSERTE(apVCon && apVCon->mp_Group);
		return NULL;
	}

	CVConGroup* p = ((CVConGroup*)apVCon->mp_Group)->GetRootGroup();
	return p;
}

CVConGroup* CVConGroup::GetAnotherGroup()
{
	if (!this)
	{
		_ASSERTE(FALSE);
		return NULL;
	}

	if (!mp_Parent)
	{
		return NULL;
	}

	CVConGroup* p = (mp_Parent->mp_Grp1 == this) ? mp_Parent->mp_Grp2 : mp_Parent->mp_Grp1;
	_ASSERTE(p && p != this && p->mp_Parent == mp_Parent);
	return p;
}

void CVConGroup::SetResizeFlags()
{
	if (!this)
	{
		_ASSERTE(this);
		return;
	}

	CVConGroup* p = GetRootGroup();
	if (p)
	{
		p->mb_ResizeFlag = true;
	}
}

void CVConGroup::MoveToParent(CVConGroup* apParent)
{
	// �� ���� ���� ������ ���.
	_ASSERTE(apParent && apParent == mp_Parent);

	// �� ������ ���� � VCon � ��������� �� ������
	_ASSERTE((mp_Item!=NULL) != (mp_Grp1!=NULL || mp_Grp2!=NULL));

	apParent->SetResizeFlags();
	
	apParent->mp_Item = mp_Item;
	apParent->m_SplitType = m_SplitType; // eSplitNone/eSplitHorz/eSplitVert
	apParent->mn_SplitPercent10 = mn_SplitPercent10; // (0.1% - 99.9%)*10
	apParent->mrc_Splitter = mrc_Splitter;

	// ������ �� "��������" ������
	apParent->mp_Grp1 = mp_Grp1;
	apParent->mp_Grp2 = mp_Grp2;
	// ������ �� "������������" ������
	if (mp_Grp1)
	{
		mp_Grp1->mp_Parent = apParent;
		mp_Grp1 = NULL;
	}
	if (mp_Grp2)
	{
		mp_Grp2->mp_Parent = apParent;
		mp_Grp2 = NULL;
	}

	// VCon
	if (mp_Item)
	{
		_ASSERTE(mp_Grp1==NULL && mp_Grp2==NULL);
		mp_Item->mp_Group = apParent;
		mp_Item = NULL; // ������������ �� VCon, ��� ���������� ������ ���������� mp_Grp1
	}

	mp_Parent = NULL;
}

void CVConGroup::GetAllTextSize(SIZE& sz, bool abMinimal /*= false*/)
{
	if (!this)
	{
		_ASSERTE(this);
		sz.cx = MIN_CON_WIDTH;
		sz.cy = MIN_CON_HEIGHT;
		return;
	}

	sz.cx = sz.cy = 0;

	_ASSERTE((m_SplitType==RConStartArgs::eSplitNone) == (mp_Grp1==NULL && mp_Grp2==NULL && mp_Item!=NULL));
	if (m_SplitType==RConStartArgs::eSplitNone)
	{
		CVConGuard VCon(mp_Item);
		if (!abMinimal && mp_Item && mp_Item->RCon())
		{
			sz.cx = mp_Item->RCon()->TextWidth();
			sz.cy = mp_Item->RCon()->TextHeight();
		}
		else
		{
			_ASSERTE(mp_Item!=NULL);
			sz.cx = MIN_CON_WIDTH;
			sz.cy = MIN_CON_HEIGHT;
		}
	}
	else
	{
		CVConGuard VCon1(mp_Grp1->mp_Item);
		CVConGuard VCon2(mp_Grp2->mp_Item);
		SIZE sz1 = {MIN_CON_WIDTH,MIN_CON_HEIGHT}, sz2 = {MIN_CON_WIDTH,MIN_CON_HEIGHT};
		
		if (mp_Grp1)
		{
			mp_Grp1->GetAllTextSize(sz1, abMinimal);
		}
		else
		{
			_ASSERTE(mp_Grp1!=NULL);
		}

		sz = sz1;

		if (mp_Grp2 /*&& (m_SplitType == RConStartArgs::eSplitHorz)*/)
		{
			mp_Grp2->GetAllTextSize(sz2, abMinimal);
		}
		else
		{
			_ASSERTE(mp_Grp2!=NULL);
		}

		// Add second pane
		if (m_SplitType == RConStartArgs::eSplitHorz)
			sz.cx += sz2.cx;
		else if (m_SplitType == RConStartArgs::eSplitVert)
			sz.cy += sz2.cy;
		else
		{
			_ASSERTE((m_SplitType == RConStartArgs::eSplitHorz) || (m_SplitType == RConStartArgs::eSplitVert));
		}
	}

	return;
}

//uint CVConGroup::AllTextHeight()
//{
//	if (!this)
//	{
//		_ASSERTE(this);
//		return 0;
//	}
//	uint nSize = 0;
//	_ASSERTE((m_SplitType==RConStartArgs::eSplitNone) == (mp_Grp1==NULL && mp_Grp2==NULL && mp_Item!=NULL));
//	if (m_SplitType==RConStartArgs::eSplitNone)
//	{
//		CVConGuard VCon(mp_Item);
//		if (mp_Item && mp_Item->RCon())
//		{
//			nSize = mp_Item->RCon()->TextHeight();
//		}
//		else
//		{
//			_ASSERTE(mp_Item!=NULL);
//		}
//	}
//	else
//	{
//		CVConGuard VCon1(mp_Grp1->mp_Item);
//		CVConGuard VCon2(mp_Grp2->mp_Item);
//	
//		if (mp_Grp1)
//		{
//			nSize += mp_Grp1->AllTextHeight();
//		}
//		else
//		{
//			_ASSERTE(mp_Grp1!=NULL);
//		}
//		
//		if (mp_Grp2 && (m_SplitType == RConStartArgs::eSplitVert))
//		{
//			nSize += mp_Grp2->AllTextHeight();
//		}
//		else
//		{
//			_ASSERTE(mp_Grp2!=NULL);
//		}
//	}
//	return nSize;
//}

void CVConGroup::LogString(LPCSTR asText, BOOL abShowTime /*= FALSE*/)
{
	if (gpSetCls->isAdvLogging && gp_VActive)
		gp_VActive->RCon()->LogString(asText, abShowTime);
}

void CVConGroup::LogString(LPCWSTR asText, BOOL abShowTime /*= FALSE*/)
{
	if (gpSetCls->isAdvLogging && gp_VActive)
		gp_VActive->RCon()->LogString(asText, abShowTime);
}

void CVConGroup::LogInput(UINT uMsg, WPARAM wParam, LPARAM lParam, LPCWSTR pszTranslatedChars /*= NULL*/)
{
	if (gpSetCls->isAdvLogging && gp_VActive)
		gp_VActive->RCon()->LogInput(uMsg, wParam, lParam, pszTranslatedChars);
}

void CVConGroup::StopSignalAll()
{
	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		if (gp_VCon[i])
		{
			gp_VCon[i]->RCon()->StopSignal();
		}
	}
}

void CVConGroup::DestroyAllVCon()
{
	for (size_t i = countof(gp_VCon); i--;)
	{
		if (gp_VCon[i])
		{
			CVirtualConsole* p = gp_VCon[i];
			gp_VCon[i] = NULL;
			p->Release();
		}
	}
}

void CVConGroup::OnDestroyConEmu()
{
	// ����� ���������, ����� ���� ��������� ��� ������ ������ (InsideIntegration)
	for (size_t i = countof(gp_VCon); i--;)
	{
		if (gp_VCon[i] && gp_VCon[i]->RCon())
		{
			if (!gp_VCon[i]->RCon()->isDetached())
			{
				gp_VCon[i]->RCon()->Detach(true, true);
			}
		}
	}
}

void CVConGroup::OnAlwaysShowScrollbar(bool abSync /*= true*/)
{
	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		if (gp_VCon[i])
			gp_VCon[i]->OnAlwaysShowScrollbar(abSync);
	}
}

void CVConGroup::RepositionVCon(RECT rcNewCon, bool bVisible)
{
	if (!this)
	{
		_ASSERTE(this);
		return;
	}

	bool lbPosChanged = false;
	RECT rcCurCon = {};

	CVConGuard VConI(mp_Item);
	if (m_SplitType == RConStartArgs::eSplitNone)
	{
		_ASSERTE(mp_Grp1==NULL && mp_Grp2==NULL);

		//RECT rcScroll = gpConEmu->CalcMargins(CEM_SCROLL);
		//gpConEmu->AddMargins(rcNewCon, rcScroll);
		
		HWND hWndDC = mp_Item ? VConI->GetView() : NULL;
		if (hWndDC)
		{
			GetWindowRect(hWndDC, &rcCurCon);
			_ASSERTE(GetParent(hWndDC) == ghWnd && ghWnd);
			MapWindowPoints(NULL, ghWnd, (LPPOINT)&rcCurCon, 2);

			if (bVisible)
			{
				lbPosChanged = memcmp(&rcCurCon, &rcNewCon, sizeof(RECT))!=0;
			}
			else
			{
				// ��� ��� ���������� ������ X/Y
				lbPosChanged = memcmp(&rcCurCon, &rcNewCon, sizeof(POINT))!=0;
			}

			// �������/��������
			if (lbPosChanged)
			{
				VConI->SetVConSizePos(rcNewCon, bVisible);
				//if (bVisible)
				//{
				//	// �������/�������� ������ DC
				//	MoveWindow(hWndDC, rcNewCon.left, rcNewCon.top, rcNewCon.right - rcNewCon.left, rcNewCon.bottom - rcNewCon.top, 1);
				//	VConI->Invalidate();
				//}
				//else
				//{
				//	// ������� ������ DC
				//	SetWindowPos(hWndDC, NULL, rcNewCon.left, rcNewCon.top, 0,0, SWP_NOSIZE|SWP_NOZORDER);
				//}
			}
		}

		if (VConI->RCon()->GuiWnd())
		{
			VConI->RCon()->SyncGui2Window(NULL);
		}
	}
	else if (mp_Grp1 && mp_Grp2)
	{
		RECT rcCon1, rcCon2, rcSplitter;
		CalcSplitRect(rcNewCon, rcCon1, rcCon2, rcSplitter);

		mrc_Splitter = rcSplitter;
		mp_Grp1->RepositionVCon(rcCon1, bVisible);
		mp_Grp2->RepositionVCon(rcCon2, bVisible);
	}
	else
	{
		_ASSERTE(mp_Grp1 && mp_Grp2);
	}
}

// ��������� � ����������� DC (pixels)
void CVConGroup::CalcSplitRect(RECT rcNewCon, RECT& rcCon1, RECT& rcCon2, RECT& rcSplitter)
{
	rcCon1 = rcNewCon;
	rcCon2 = rcNewCon;
	rcSplitter = MakeRect(0,0);

	if (!this)
	{
		_ASSERTE(this);
		return;
	}

	// ����������� �������
	CVConGuard VCon1(mp_Grp1 ? mp_Grp1->mp_Item : NULL);
	CVConGuard VCon2(mp_Grp2 ? mp_Grp2->mp_Item : NULL);

	if ((m_SplitType == RConStartArgs::eSplitNone) || !mp_Grp1 || !mp_Grp2)
	{
		_ASSERTE(mp_Grp1==NULL && mp_Grp2==NULL);
		_ASSERTE((m_SplitType != RConStartArgs::eSplitNone) && "Need no split");
		return;
	}

	UINT nSplit = max(1,min(mn_SplitPercent10,999));
	//UINT nPadSizeX = 0, nPadSizeY = 0;
	if (m_SplitType == RConStartArgs::eSplitHorz)
	{
		UINT nWidth = rcNewCon.right - rcNewCon.left;
		UINT nPadX = gpSet->nSplitWidth;
		if (nWidth >= nPadX)
			nWidth -= nPadX;
		else
			nPadX = 0;
		RECT rcScroll = gpConEmu->CalcMargins(CEM_SCROLL);
		_ASSERTE(rcScroll.left==0);
		if (rcScroll.right)
		{
			_ASSERTE(gpSet->isAlwaysShowScrollbar==1); // ���� ������ �������� ������ ��� ���������� ��������� �������
			if (nWidth > (UINT)(rcScroll.right * 2))
				nWidth -= rcScroll.right * 2;
			else
				rcScroll.right = 0;
		}
		UINT nScreenWidth = (nWidth * nSplit / 1000);
		LONG nCellWidth = gpSetCls->FontWidth();
		LONG nCon2Width;
		if (nCellWidth > 0)
		{
			UINT nTotalCellCountX = nWidth / nCellWidth;
			UINT nCellCountX = (nScreenWidth + (nCellWidth/2)) / nCellWidth;
			if ((nTotalCellCountX >= 2) && (nCellCountX >= nTotalCellCountX))
			{
				_ASSERTE(FALSE && "Too small rect?");
				nCellCountX = nTotalCellCountX - 1;
			}
			nScreenWidth = nCellCountX * nCellWidth;
			nCon2Width = (nTotalCellCountX - nCellCountX) * nCellWidth;
			_ASSERTE(nCon2Width > 0);
		}
		else
		{
			_ASSERTE(nCellWidth > 0);
			nCon2Width = rcNewCon.right - (rcCon1.right+nPadX+rcScroll.right);
			_ASSERTE(nCon2Width > 0);
		}

		rcCon1 = MakeRect(rcNewCon.left, rcNewCon.top,
			max(rcNewCon.left + nScreenWidth,rcNewCon.right - nCon2Width - nPadX), rcNewCon.bottom);
		rcCon2 = MakeRect(rcNewCon.right - nCon2Width, rcNewCon.top, rcNewCon.right, rcNewCon.bottom);
		rcSplitter = MakeRect(rcCon1.right+1, rcCon1.top, rcCon2.left, rcCon2.bottom);
	}
	else
	{
		UINT nHeight = rcNewCon.bottom - rcNewCon.top;
		UINT nPadY = gpSet->nSplitHeight;
		if (nHeight >= nPadY)
			nHeight -= nPadY;
		else
			nPadY = 0;
		RECT rcScroll = gpConEmu->CalcMargins(CEM_SCROLL);
		_ASSERTE(rcScroll.top==0);
		if (rcScroll.bottom)
		{
			_ASSERTE(gpSet->isAlwaysShowScrollbar==1); // ���� ������ �������� ������ ��� ���������� ��������� �������
			if (nHeight > (UINT)(rcScroll.bottom * 2))
				nHeight -= rcScroll.bottom * 2;
			else
				rcScroll.bottom = 0;
		}
		UINT nScreenHeight = (nHeight * nSplit / 1000);
		LONG nCellHeight = gpSetCls->FontHeight();
		LONG nCon2Height;
		if (nCellHeight > 0)
		{
			UINT nTotalCellCountY = nHeight / nCellHeight;
			UINT nCellCountY = (nScreenHeight + (nCellHeight/2)) / nCellHeight;
			if ((nTotalCellCountY >= 2) && (nCellCountY >= nTotalCellCountY))
			{
				_ASSERTE(FALSE && "Too small rect?");
				nCellCountY = nTotalCellCountY - 1;
			}
			nScreenHeight = nCellCountY * nCellHeight;
			nCon2Height = (nTotalCellCountY - nCellCountY) * nCellHeight;
			_ASSERTE(nCon2Height > 0);
		}
		else
		{
			_ASSERTE(nCellHeight > 0);
			nCon2Height = rcNewCon.bottom - (rcCon1.bottom+nPadY+rcScroll.bottom);
		}

		rcCon1 = MakeRect(rcNewCon.left, rcNewCon.top,
			rcNewCon.right, max(rcNewCon.top + nScreenHeight,rcNewCon.bottom - nCon2Height - nPadY));
		rcCon2 = MakeRect(rcCon1.left, rcNewCon.bottom - nCon2Height, rcNewCon.right, rcNewCon.bottom);
		rcSplitter = MakeRect(rcCon1.left, rcCon1.bottom, rcCon2.right, rcCon2.top);
	}
}

void CVConGroup::CalcSplitRootRect(RECT rcAll, RECT& rcCon, CVConGroup* pTarget /*= NULL*/)
{
	if (!this)
	{
		_ASSERTE(this);
		rcCon = rcAll;
		return;
	}

	if (!mp_Parent && !pTarget)
	{
		rcCon = rcAll;
		return;
	}

	RECT rc = rcAll;
	if (mp_Parent)
	{
		_ASSERTE(pTarget != this);
		mp_Parent->CalcSplitRootRect(rcAll, rc, this);
	}

	if (m_SplitType == RConStartArgs::eSplitNone)
	{
		_ASSERTE(mp_Grp1==NULL && mp_Grp2==NULL);
		rcCon = rc;
	}
	else
	{
		RECT rc1, rc2, rcSplitter;
		CalcSplitRect(rc, rc1, rc2, rcSplitter);

		_ASSERTE(pTarget == mp_Grp1 || pTarget == mp_Grp2);
		rcCon = (pTarget == mp_Grp2) ? rc2 : rc1;
	}
}

void CVConGroup::ShowAllVCon(int nShowCmd)
{
	if (!this)
	{
		_ASSERTE(this);
		return;
	}

	CVConGuard VConI(mp_Item);

	if (m_SplitType == RConStartArgs::eSplitNone)
	{
		_ASSERTE(mp_Grp1==NULL && mp_Grp2==NULL);
		if (VConI.VCon())
		{
			VConI->ShowView(nShowCmd);
		}
	}
	else if (mp_Grp1 && mp_Grp2)
	{
		mp_Grp1->ShowAllVCon(nShowCmd);
		mp_Grp2->ShowAllVCon(nShowCmd);
	}
	else
	{
		_ASSERTE(mp_Grp1 && mp_Grp2);
	}
}

void CVConGroup::MoveAllVCon(CVirtualConsole* pVConCurrent, RECT rcNewCon)
{
	CVConGroup* pGroup = GetRootOfVCon(pVConCurrent);
	if (!pGroup)
	{
		_ASSERTE(pGroup);
		return;
	}

	//bool lbPosChanged = false;
	//RECT rcCurCon = {};
	CVConGroup* pRoots[MAX_CONSOLE_COUNT+1] = {pGroup};
	CVConGuard VCon(pVConCurrent);

	WARNING("DoubleView � �� ������: ����������. ������ ������ ���� � ConEmuChild/VConGroup!");

	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		CVConGuard VConI(gp_VCon[i]);
		if (!VConI.VCon())
			break;
		if (VConI.VCon() == pVConCurrent)
			continue;
		// ������������ "�� �������", � �� "�� ��������"
		bool bProcessed = false;
		CVConGroup* pCurGroup = GetRootOfVCon(VConI.VCon());
		size_t j = 0;
		while (pRoots[j])
		{
			if (pRoots[j] == pCurGroup)
			{
				bProcessed = true; break;
			}
			j++;
		}
		if (bProcessed)
			continue; // ���
		pRoots[j] = pCurGroup; // ��������, ����������

		// ������� ������ DC
		pCurGroup->RepositionVCon(rcNewCon, false);

		//// �������
		//HWND hWndDC = VConI->GetView();
		//if (hWndDC)
		//{
		//	WARNING("DoubleView � �� ������: ����������. ������ ������ ���� � ConEmuChild!");
		//	GetWindowRect(hWndDC, &rcCurCon);
		//	MapWindowPoints(NULL, ghWnd, (LPPOINT)&rcCurCon, 2);
		//	// ��� ��� ���������� ������ X/Y
		//	lbPosChanged = memcmp(&rcCurCon, &rcNewCon, sizeof(POINT))!=0;

		//	if (lbPosChanged)
		//	{
		//		// ������� ������ DC
		//		SetWindowPos(hWndDC, NULL, rcNewCon.left, rcNewCon.top, 0,0, SWP_NOSIZE|SWP_NOZORDER);
		//	}
		//}
	}

	// �������/�������� ������ DC
	pGroup->RepositionVCon(rcNewCon, true);

	//HWND hWndDC = pVConCurrent ? pVConCurrent->GetView() : NULL;
	//if (hWndDC)
	//{
	//	WARNING("DoubleView � �� ������: ����������. ������ ������ ���� � ConEmuChild!");
	//	GetWindowRect(hWndDC, &rcCurCon);
	//	MapWindowPoints(NULL, ghWnd, (LPPOINT)&rcCurCon, 2);
	//	lbPosChanged = memcmp(&rcCurCon, &rcNewCon, sizeof(RECT))!=0;

	//	if (lbPosChanged)
	//	{
	//		// �������/�������� ������ DC
	//		MoveWindow(hWndDC, rcNewCon.left, rcNewCon.top, rcNewCon.right - rcNewCon.left, rcNewCon.bottom - rcNewCon.top, 1);
	//		pVConCurrent->Invalidate();
	//	}
	//}
}

bool CVConGroup::isValid(CRealConsole* apRCon)
{
	if (!apRCon)
		return false;

	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		if (gp_VCon[i] && apRCon == gp_VCon[i]->RCon())
			return true;
	}

	return false;
}

bool CVConGroup::isValid(CVirtualConsole* apVCon)
{
	if (!apVCon)
		return false;

	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		if (apVCon == gp_VCon[i])
			return true;
	}

	return false;
}

bool CVConGroup::isVConExists(int nIdx)
{
	if (nIdx < 0 || nIdx >= (int)countof(gp_VCon))
		return false;
	return (gp_VCon[nIdx] != NULL);
}

bool CVConGroup::isVConHWND(HWND hChild, CVConGuard* rpVCon /*= NULL*/)
{
	CVConGuard VCon;

	if (hChild)
	{
		for (size_t i = 0; i < countof(gp_VCon); i++)
		{
			if (gp_VCon[i] && (gp_VCon[i]->GetView() == hChild))
			{
				VCon = gp_VCon[i];
				break;
			}
		}
	}

	if (rpVCon)
		*rpVCon = VCon.VCon();

	bool bFound = (VCon.VCon() != NULL);
	return bFound;
}

bool CVConGroup::isEditor()
{
	if (!gp_VActive) return false;

	return gp_VActive->RCon()->isEditor();
}

bool CVConGroup::isViewer()
{
	if (!gp_VActive) return false;

	return gp_VActive->RCon()->isViewer();
}

bool CVConGroup::isFar(bool abPluginRequired/*=false*/)
{
	if (!gp_VActive) return false;

	return gp_VActive->RCon()->isFar(abPluginRequired);
}

// ���� �� ��� ���-��?
bool CVConGroup::isFarExist(CEFarWindowType anWindowType/*=fwt_Any*/, LPWSTR asName/*=NULL*/, CVConGuard* rpVCon/*=NULL*/)
{
	bool bFound = false, bLocked = false;
	CVConGuard VCon;

	if (rpVCon)
		*rpVCon = NULL;

	for (INT_PTR i = -1; !bFound && (i < (INT_PTR)countof(gp_VCon)); i++)
	{
		if (i == -1)
			VCon = gp_VActive;
		else
			VCon = gp_VCon[i];

		if (VCon.VCon())
		{
			// ��� ���?
			CRealConsole* pRCon = VCon->RCon();
			if (pRCon && pRCon->isFar(anWindowType & fwt_PluginRequired))
			{
				// ���� ���-�� ����������?
				if (!(anWindowType & (fwt_TypeMask|fwt_Elevated|fwt_NonElevated|fwt_Modal|fwt_NonModal|fwt_ActivateFound)) && !(asName && *asName))
				{
					bFound = true;
					break;
				}

				if (!(anWindowType & (fwt_TypeMask|fwt_ActivateFound)) && !(asName && *asName))
				{
					CEFarWindowType t = pRCon->GetActiveTabType();

					// ���� Far Elevated?
					if ((anWindowType & fwt_Elevated) && !(t & fwt_Elevated))
						continue;
					// � ���� ��������������� ���� fwt_Elevated
					// fwt_NonElevated ������������ ������ ��� �������� ������
					if ((anWindowType & fwt_NonElevated) && (t & fwt_Elevated))
						continue;

					// ��������� ����?
					WARNING("����� ��� ��������� <�����������> ��������������� ��������, ��� ����, ��� ��� ���-����!");
					if ((anWindowType & fwt_Modal) && !(t & fwt_Modal))
						continue;
					// � ���� ��������������� ���� fwt_Modal
					// fwt_NonModal ������������ ������ ��� �������� ������
					if ((anWindowType & fwt_NonModal) && (t & fwt_Modal))
						continue;

					bFound = true;
					break;
				}
				else
				{
					// ����� ���.�������� ���� ����
					ConEmuTab tab;
					LPCWSTR pszNameOnly = asName ? PointToName(asName) : NULL;
					if (pszNameOnly)
					{
						// ���������� ��� �������� (� PointToName), ��� � ������ �����
						// ��� ����� ���� ��������� ��� �������� �� ������/�����������
						LPCWSTR pszSlash = wcsrchr(pszNameOnly, L'/');
						if (pszSlash)
							pszNameOnly = pszSlash+1;
					}

					for (int j = 0; !bFound; j++)
					{
						if (!pRCon->GetTab(j, &tab))
							break;

						if ((tab.Type & fwt_TypeMask) != (anWindowType & fwt_TypeMask))
							continue;

						// ���� Far Elevated?
						if ((anWindowType & fwt_Elevated) && !(tab.Type & fwt_Elevated))
							continue;
						// � ���� ��������������� ���� fwt_Elevated
						// fwt_NonElevated ������������ ������ ��� �������� ������
						if ((anWindowType & fwt_NonElevated) && (tab.Type & fwt_Elevated))
							continue;

						// ��������� ����?
						WARNING("����� ��� ��������� <�����������> ��������������� ��������, ��� ����, ��� ��� ���-����!");
						if ((anWindowType & fwt_Modal) && !(tab.Type & fwt_Modal))
							continue;
						// � ���� ��������������� ���� fwt_Modal
						// fwt_NonModal ������������ ������ ��� �������� ������
						if ((anWindowType & fwt_NonModal) && (tab.Type & fwt_Modal))
							continue;

						// ���� ���� ���������� ��������/������
						if (asName && *asName)
						{
							if (lstrcmpi(tab.Name, asName) == 0)
							{
								bFound = true;
							}
							else if ((pszNameOnly != asName) && (lstrcmpi(PointToName(tab.Name), pszNameOnly) == 0))
							{
								bFound = true;
							}
						}
						else
						{
							bFound = true;
						}


						if (bFound)
						{
							if (anWindowType & fwt_ActivateFound)
							{
								if (pRCon->ActivateFarWindow(j))
								{
									gpConEmu->Activate(VCon.VCon());
									bLocked = false;
								}
								else
								{
									bLocked = true;
								}
							}

							break;
						}
					}
				}
			}
		}
	}

	// �����?
	if (bFound)
	{
		if (rpVCon)
		{
			*rpVCon = VCon.VCon();
			if (bLocked)
				bFound = false;
		}
	}

	return bFound;
}

// ���������� ������ (0-based) �������� �������
int CVConGroup::GetActiveVCon(CVConGuard* pVCon /*= NULL*/, int* pAllCount /*= NULL*/)
{
	int nCount = 0, nFound = -1;

	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		if (gp_VCon[i])
		{
			nCount++;

			if (gp_VCon[i] == gp_VActive)
			{
				if (pVCon)
					*pVCon = gp_VCon[i];
				nFound = i;
			}
		}
	}

	_ASSERTE((gp_VActive!=NULL) == (nFound>=0));

	if (pAllCount)
		*pAllCount = nCount;

	return nFound;
}

// ���������� ������ (0-based) �������, ��� -1, ���� ����� ���
int CVConGroup::GetVConIndex(CVirtualConsole* apVCon)
{
	if (!apVCon)
		return -1;
	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		if (gp_VCon[i] == apVCon)
			return i;
	}
	return -1;
}

bool CVConGroup::GetProgressInfo(short* pnProgress, BOOL* pbActiveHasProgress, BOOL* pbWasError)
{
	short nProgress = -1;
	short nUpdateProgress = gpUpd ? gpUpd->GetUpdateProgress() : -1;
	short n;
	BOOL bActiveHasProgress = FALSE;
	BOOL bWasError = FALSE;

	if (gp_VActive)
	{
		BOOL lbNotFromTitle = FALSE;
		if (!isValid(gp_VActive))
		{
			_ASSERTE(isValid(gp_VActive));
		}
		else if ((nProgress = gp_VActive->RCon()->GetProgress(&bWasError, &lbNotFromTitle)) >= 0)
		{
			//mn_Progress = max(nProgress, nUpdateProgress);
			bActiveHasProgress = TRUE;
			_ASSERTE(lbNotFromTitle==FALSE); // CRealConsole ������ �������� ��������� � GetTitle ���
			//bNeedAddToTitle = lbNotFromTitle;
		}
	}

	if (!bActiveHasProgress && nUpdateProgress >= 0)
	{
		nProgress = max(nProgress, nUpdateProgress);
	}

	// ��� ���������� ��������� ������� ������ �� ���� ��������� ��������
	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		if (gp_VCon[i])
		{
			BOOL bCurError = FALSE;
			n = gp_VCon[i]->RCon()->GetProgress(&bCurError);

			if (bCurError)
				bWasError = TRUE;

			if (!bActiveHasProgress && n > nProgress)
				nProgress = n;
		}
	}

	if (pnProgress)
		*pnProgress = nProgress;
	if (pbActiveHasProgress)
		*pbActiveHasProgress = bActiveHasProgress;
	if (pbWasError)
		*pbWasError = bWasError;

	return (nProgress >= 0);
}

void CVConGroup::OnDosAppStartStop(HWND hwnd, StartStopType sst, DWORD idChild)
{
	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		if (!gp_VCon[i]) continue;

		// ����������� ����� "-new_console" ��������� ����� CECMD_ATTACH2GUI, � �� ����� WinEvent
		// 111211 - "-new_console" ������ ���������� � GUI � ����������� � ���
		if (gp_VCon[i]->RCon()->isDetached() || !gp_VCon[i]->RCon()->isServerCreated())
			continue;

		HWND hRConWnd = gp_VCon[i]->RCon()->ConWnd();
		if (hRConWnd == hwnd)
		{
			//StartStopType sst = (anEvent == EVENT_CONSOLE_START_APPLICATION) ? sst_App16Start : sst_App16Stop;
			gp_VCon[i]->RCon()->OnDosAppStartStop(sst, idChild);
			break;
		}
	}
}

void CVConGroup::InvalidateAll()
{
	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		CVConGuard VCon(gp_VCon[i]);
		if (VCon.VCon() && isVisible(VCon.VCon()))
			VCon.VCon()->Invalidate();
	}
}

void CVConGroup::UpdateWindowChild(CVirtualConsole* apVCon)
{
	if (apVCon)
	{
		if (apVCon->isVisible())
			UpdateWindow(apVCon->GetView());
	}
	else
	{
		for (size_t i = 0; i < countof(gp_VCon); i++)
		{
			if (gp_VCon[i] && gp_VCon[i]->isVisible())
				UpdateWindow(gp_VCon[i]->GetView());
		}
	}
}

void CVConGroup::RePaint()
{
	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		if (gp_VCon[i] && gp_VCon[i]->isVisible())
		{
			CVirtualConsole* pVCon = gp_VCon[i];
			CVConGuard guard(pVCon);
			HWND hView = pVCon->GetView();
			if (hView)
			{
				HDC hDc = GetDC(hView);
				RECT rcClient = pVCon->GetDcClientRect();
				pVCon->Paint(hDc, rcClient);
				ReleaseDC(ghWnd, hDc);
			}
		}
	}
}

void CVConGroup::Update(bool isForce /*= false*/)
{
	if (isForce)
	{
		for (size_t i = 0; i < countof(gp_VCon); i++)
		{
			if (gp_VCon[i])
				gp_VCon[i]->OnFontChanged();
		}
	}

	CVirtualConsole::ClearPartBrushes();

	if (gp_VActive)
	{
		gp_VActive->Update(isForce);
		//InvalidateAll();
	}
}

bool CVConGroup::isActive(CVirtualConsole* apVCon, bool abAllowGroup /*= true*/)
{
	if (!apVCon)
		return false;

	if (apVCon == gp_VActive)
		return true;

	if (abAllowGroup)
	{
		TODO("DoubleView: ����� ����� ����������� ����� - ����� �������� ������ �� ���� ��������");
	}

	return false;
}

bool CVConGroup::isVisible(CVirtualConsole* apVCon)
{
	if (!apVCon)
		return false;

	if (apVCon == gp_VActive)
		return true;

	CVConGroup* pActiveRoot = GetRootOfVCon(gp_VActive);
	CVConGroup* pRoot = GetRootOfVCon(apVCon);
	if (pRoot && pActiveRoot && (pRoot == pActiveRoot))
		return true;

	return false;
}

bool CVConGroup::isConSelectMode()
{
	//TODO: �� �������, ���-�� ����������� ����������?
	//return gb_ConsoleSelectMode;
	if (gp_VActive)
		return gp_VActive->RCon()->isConSelectMode();

	return false;
}

bool CVConGroup::isFilePanel(bool abPluginAllowed/*=false*/)
{
	if (!gp_VActive) return false;

	bool lbIsPanels = gp_VActive->RCon()->isFilePanel(abPluginAllowed);
	return lbIsPanels;
}

bool CVConGroup::isNtvdm(BOOL abCheckAllConsoles/*=FALSE*/)
{
	if (gp_VActive && gp_VActive->RCon())
	{
		if (gp_VActive->RCon()->isNtvdm())
			return true;
	}

	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		if (gp_VCon[i] && gp_VCon[i] != gp_VActive && gp_VCon[i]->RCon())
		{
			if (gp_VCon[i]->RCon()->isNtvdm())
				return true;
		}
	}

	return false;
}

bool CVConGroup::isOurConsoleWindow(HWND hCon)
{
	if (!hCon)
		return false;
	
	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		if (gp_VCon[i] && gp_VCon[i]->RCon())
		{
			if (gp_VCon[i]->RCon()->ConWnd() == hCon)
				return true;
		}
	}
	
	return false;
}

bool CVConGroup::isChildWindow()
{
	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		if (gp_VCon[i] && isVisible(gp_VCon[i]))
		{
			if (gp_VCon[i]->RCon())
			{
				if (gp_VCon[i]->RCon()->GuiWnd() || gp_VCon[i]->RCon()->isPictureView())
					return true;
			}
		}
	}

	return false;
}

// ��������� ���� ��� PictureView ������ ����� ������������� �������������, ��� ���
// ������������ �� ���� ��� ����������� "���������" - ������
bool CVConGroup::isPictureView()
{
	bool lbRc = false;

	if (gpConEmu->hPictureView && (!IsWindow(gpConEmu->hPictureView) || !isFar()))
	{
		InvalidateAll();
		gpConEmu->hPictureView = NULL;
	}

	bool lbPrevPicView = (gpConEmu->hPictureView != NULL);

	for (size_t i = 0; !lbRc && i < countof(gp_VCon); i++)
	{
		CVirtualConsole* pVCon = gp_VCon[i];
		if (!pVCon || !isVisible(pVCon) || !pVCon->RCon())
			continue;

		gpConEmu->hPictureView = pVCon->RCon()->isPictureView();

		lbRc = gpConEmu->hPictureView!=NULL;

		// ���� �������� Help (F1) - ������ PictureView ��������
		if (gpConEmu->hPictureView && !IsWindowVisible(gpConEmu->hPictureView))
		{
			lbRc = false;
			gpConEmu->hPictureView = NULL;
		}
	}

	if (gpConEmu->bPicViewSlideShow && !gpConEmu->hPictureView)
	{
		gpConEmu->bPicViewSlideShow=false;
	}

	if (lbRc && !lbPrevPicView)
	{
		GetWindowRect(ghWnd, &gpConEmu->mrc_WndPosOnPicView);
	}
	else if (!lbRc)
	{
		memset(&gpConEmu->mrc_WndPosOnPicView, 0, sizeof(gpConEmu->mrc_WndPosOnPicView));
	}

	return lbRc;
}

// nIdx - 0 based
bool CVConGroup::GetVCon(int nIdx, CVConGuard* pVCon /*= NULL*/)
{
	bool bFound = false;

	if (nIdx < 0 || nIdx >= (int)countof(gp_VCon) || gp_VCon[nIdx] == NULL)
	{
		_ASSERTE(nIdx>=0 && nIdx<(int)countof(gp_VCon));
		if (pVCon)
			*pVCon = NULL;
	}
	else
	{
		if (pVCon)
			*pVCon = gp_VCon[nIdx];
		bFound = true;
	}

	return bFound;
}

bool CVConGroup::GetVConFromPoint(POINT ptScreen, CVConGuard* pVCon /*= NULL*/)
{
	bool bFound = false;
	CVConGuard VCon;

	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		VCon = gp_VCon[i];

		if (VCon.VCon() != NULL && VCon->isVisible())
		{
			
			HWND hView = VCon->GetView();
			if (hView)
			{
				RECT rcView; GetWindowRect(hView, &rcView);

				if (PtInRect(&rcView, ptScreen))
				{
					if (pVCon)
						*pVCon = VCon.VCon();
					bFound = true;
					break;
				}
			}
			else
			{
				_ASSERTE(FALSE && "(hView = VCon->GetView()) != NULL");
			}
		}
	}

	return bFound;
}

//bool CVConGroup::ResizeConsoles(const RECT &rFrom, enum ConEmuRect tFrom)
//{
//	_ASSERTE(FALSE && "TODO");
//	return false;
//}
//
//bool CVConGroup::ResizeViews(bool bResizeRCon/*=true*/, WPARAM wParam/*=0*/, WORD newClientWidth/*=(WORD)-1*/, WORD newClientHeight/*=(WORD)-1*/)
//{
//	_ASSERTE(FALSE && "TODO");
//	return false;
//}

bool CVConGroup::OnCloseQuery()
{
	int nEditors = 0, nProgress = 0, i, nConsoles = 0;

	for (i = ((int)countof(gp_VCon)-1); i >= 0; i--)
	{
		CRealConsole* pRCon = NULL;
		//ConEmuTab tab = {0};

		if (gp_VCon[i] && (pRCon = gp_VCon[i]->RCon())!=NULL)
		{
			nConsoles++;

			// ��������� (�����������, ��������, � �.�.)
			if (pRCon->GetProgress(NULL) != -1)
				nProgress ++;

			// ������������� ���������
			int n = pRCon->GetModifiedEditors();

			if (n)
				nEditors += n;
		}
	}

	if (nProgress || nEditors || (gpSet->isCloseConsoleConfirm && (nConsoles > 1)))
	{
		wchar_t szText[255], *pszText;
		//wcscpy_c(szText, L"Close confirmation.\r\n\r\n");
		_wsprintf(szText, SKIPLEN(countof(szText)) L"About to close %u console%s.\r\n\r\n", nConsoles, (nConsoles>1)?L"s":L"");
		pszText = szText+_tcslen(szText);

		if (nProgress) { _wsprintf(pszText, SKIPLEN(countof(szText)-(pszText-szText)) L"Incomplete operations: %i\r\n", nProgress); pszText += _tcslen(pszText); }

		if (nEditors) { _wsprintf(pszText, SKIPLEN(countof(szText)-(pszText-szText)) L"Unsaved editor windows: %i\r\n", nEditors); pszText += _tcslen(pszText); }

		lstrcpy(pszText, L"\r\nProceed with close ConEmu?");
		int nBtn = MessageBoxW(ghWnd, szText, gpConEmu->GetDefaultTitle(), MB_OKCANCEL|MB_ICONEXCLAMATION);

		if (nBtn != IDOK)
		{
			gpConEmu->mb_CloseGuiConfirmed = false;
			return false; // �� ���������
		}

		gpConEmu->mb_CloseGuiConfirmed = true;
	}

	#ifdef _DEBUG
	if (gbInMyAssertTrap)
		return false;
	#endif

	// ����� ��� ��������� ������ ��������
	gpConEmu->OnRConStartedSuccess(NULL);
	return true; // �����
}

// true - found
bool CVConGroup::OnFlashWindow(DWORD nFlags, DWORD nCount, HWND hCon)
{
	if (!hCon) return false;

	bool lbFlashSimple = false;

	// �������. ��������� ������� ���������� ��������
	if (gpSet->isDisableFarFlashing && gp_VActive->RCon()->GetFarPID(FALSE))
	{
		if (gpSet->isDisableFarFlashing == 1)
			return false;
		else
			lbFlashSimple = true;
	}
	else if (gpSet->isDisableAllFlashing)
	{
		if (gpSet->isDisableAllFlashing == 1)
			return false;
		else
			lbFlashSimple = true;
	}

	bool lbFound = false;

	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		if (!gp_VCon[i]) continue;

		if (gp_VCon[i]->RCon()->ConWnd() == hCon)
		{
			lbFound = true;

			FLASHWINFO fl = {sizeof(FLASHWINFO)};

			if (gpConEmu->isMeForeground())
			{
				if (gp_VCon[i] != gp_VActive)    // ������ ��� ���������� �������
				{
					fl.dwFlags = FLASHW_STOP; fl.hwnd = ghWnd;
					FlashWindowEx(&fl); // ����� ������� �� �������������
					fl.uCount = 3; fl.dwFlags = lbFlashSimple ? FLASHW_ALL : FLASHW_TRAY; fl.hwnd = ghWnd;
					FlashWindowEx(&fl);
				}
			}
			else
			{
				if (lbFlashSimple)
				{
					fl.uCount = 3; fl.dwFlags = FLASHW_TRAY;
				}
				else
				{
					fl.dwFlags = FLASHW_ALL|FLASHW_TIMERNOFG;
				}

				fl.hwnd = ghWnd;
				FlashWindowEx(&fl); // �������� � GUI
			}

			//fl.dwFlags = FLASHW_STOP; fl.hwnd = hCon; -- �� ���������, �.�. ��� �������
			//FlashWindowEx(&fl);
			break;
		}
	}

	return lbFound;
}

void CVConGroup::OnUpdateGuiInfoMapping(ConEmuGuiMapping* apGuiInfo)
{
	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		if (gp_VCon[i] && gp_VCon[i]->RCon())
		{
			gp_VCon[i]->RCon()->UpdateGuiInfoMapping(apGuiInfo);
		}
	}
}

void CVConGroup::OnPanelViewSettingsChanged()
{
	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		if (gp_VCon[i])
		{
			gp_VCon[i]->OnPanelViewSettingsChanged();
		}
	}
}

void CVConGroup::OnTaskbarSettingsChanged()
{
	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		if (gp_VCon[i])
			gp_VCon[i]->OnTaskbarSettingsChanged();
	}
}

// true - ���� nPID ������� � ����� �� ��������
bool CVConGroup::isConsolePID(DWORD nPID)
{
	bool lbPidFound = false;

	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		if (gp_VCon[i] == NULL || gp_VCon[i]->RCon() == NULL)
			continue;

		DWORD dwFarPID    = gp_VCon[i]->RCon()->GetFarPID();
		DWORD dwActivePID = gp_VCon[i]->RCon()->GetActivePID();

		if (dwFarPID == nPID || dwActivePID == nPID)
		{
			lbPidFound = true;
			break;
		}
	}

	return lbPidFound;
}

bool CVConGroup::OnScClose()
{
	bool lbAllowed = false;
	int nConCount = 0, nDetachedCount = 0;
		
	for (int i = (int)(countof(gp_VCon)-1); i >= 0; i--)
	{
		if (gp_VCon[i] && gp_VCon[i]->RCon())
		{
			if (gp_VCon[i]->RCon()->isDetached())
			{
				nDetachedCount ++;
				continue;
			}

			nConCount ++;

			if (gp_VCon[i]->RCon()->ConWnd())
			{
				gp_VCon[i]->RCon()->CloseConsole(false, false);
			}
		}
	}

	if (nConCount == 0)
	{
		if (nDetachedCount > 0)
		{
			if (MessageBox(ghWnd, L"ConEmu is waiting for console attach.\nIt was started in 'Detached' mode.\nDo You want to cancel waiting?",
			              gpConEmu->GetDefaultTitle(), MB_YESNO|MB_ICONQUESTION) != IDYES)
        	{
        		lbAllowed = false;
			}
		}

		lbAllowed = true;
	}

	return lbAllowed;
}

void CVConGroup::OnUpdateScrollInfo()
{
	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		CVConGuard VCon(gp_VCon[i]);
		if (!VCon.VCon())
			continue;
		if (!isActive(VCon.VCon()))
			continue;

		if (VCon->RCon())
			VCon->RCon()->UpdateScrollInfo();
	}
}

void CVConGroup::OnUpdateFarSettings()
{
	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		CVConGuard VCon(gp_VCon[i]);
		if (!VCon.VCon())
			continue;

		CRealConsole* pRCon = VCon->RCon();

		if (pRCon)
			pRCon->UpdateFarSettings();

		//DWORD dwFarPID = pRCon->GetFarPID();
		//if (!dwFarPID) continue;
		//pRCon->EnableComSpec(dwFarPID, gpSet->AutoBufferHeight);
	}
}

void CVConGroup::OnUpdateTextColorSettings(BOOL ChangeTextAttr /*= TRUE*/, BOOL ChangePopupAttr /*= TRUE*/)
{
	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		CVConGuard VCon(gp_VCon[i]);
		if (!VCon.VCon())
			continue;

		CRealConsole* pRCon = VCon->RCon();

		if (pRCon)
		{
			pRCon->UpdateTextColorSettings(ChangeTextAttr, ChangePopupAttr);
		}
	}
}

void CVConGroup::OnVConClosed(CVirtualConsole* apVCon)
{
	ShutdownGuiStep(L"OnVConClosed");

	bool bDbg1 = false, bDbg2 = false, bDbg3 = false, bDbg4 = false;
	int iDbg1 = -100, iDbg2 = -100, iDbg3 = -100;

	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		if (gp_VCon[i] == apVCon)
		{
			iDbg1 = i;

			// ������� ����� �������� ��������, ����� � ����������� �������
			// ����� ���� ��������� ������� � ������ ��������� ������ �������
			// ����� ������� ������������ ������ ������� ����������� �������
			//gpConEmu->mp_TabBar->Update(TRUE); -- � � �� ������ �� ������ ������������, �.�. RCon ������ FALSE

			// ��� ���������� ������ ������������ ���������� ������� (���� ������� �������)
			if (gpSet->isTabRecent && apVCon == gp_VActive)
			{
				if (gpConEmu->GetVCon(1))
				{
					bDbg1 = true;
					gpConEmu->mp_TabBar->SwitchRollback();
					gpConEmu->mp_TabBar->SwitchNext();
					gpConEmu->mp_TabBar->SwitchCommit();
				}
			}

			// ������ ����� �������� ���������� �������
			gp_VCon[i] = NULL;
			WARNING("������-�� ��� ����� �� � CriticalSection �������. ��������� �������� ����� ������������ ���������");

			if (gp_VActive == apVCon)
			{
				bDbg2 = true;

				for (int j=(i-1); j>=0; j--)
				{
					if (gp_VCon[j])
					{
						iDbg2 = j;
						ConActivate(j);
						break;
					}
				}

				if (gp_VActive == apVCon)
				{
					bDbg3 = true;

					for (size_t j = (i+1); j < countof(gp_VCon); j++)
					{
						if (gp_VCon[j])
						{
							iDbg3 = j;
							ConActivate(j);
							break;
						}
					}
				}
			}

			for (size_t j = (i+1); j < countof(gp_VCon); j++)
			{
				gp_VCon[j-1] = gp_VCon[j];
			}

			gp_VCon[countof(gp_VCon)-1] = NULL;

			if (gp_VActive == apVCon)
			{
				bDbg4 = true;
				gp_VActive = NULL;
			}

			apVCon->Release();
			break;
		}
	}

	if (gp_VActive == apVCon)
	{
		// ���� ������ �������� �� ������, �� �� ������ ������, ���������� gp_VActive
		_ASSERTE(gp_VActive == NULL && gp_VCon[0] == NULL);
		gp_VActive = NULL;
	}

	if (gp_VActive)
	{
		ShowActiveGroup(gp_VActive);
	}
}

void CVConGroup::OnUpdateProcessDisplay(HWND hInfo)
{
	if (!hInfo)
		return;

	SendDlgItemMessage(hInfo, lbProcesses, LB_RESETCONTENT, 0, 0);

	wchar_t temp[MAX_PATH];

	for (size_t j = 0; j < countof(gp_VCon); j++)
	{
		if (gp_VCon[j] == NULL) continue;

		ConProcess* pPrc = NULL;
		int nCount = gp_VCon[j]->RCon()->GetProcesses(&pPrc);

		if (pPrc && (nCount > 0))
		{
			for (int i=0; i<nCount; i++)
			{
				if (gp_VCon[j] == gp_VActive)
					_tcscpy(temp, _T("(*) "));
				else
					temp[0] = 0;

				swprintf(temp+_tcslen(temp), _T("[%i.%i] %s - PID:%i"),
						 j+1, i, pPrc[i].Name, pPrc[i].ProcessID);
				if (hInfo)
					SendDlgItemMessage(hInfo, lbProcesses, LB_ADDSTRING, 0, (LPARAM)temp);
			}

			SafeFree(pPrc);
		}
	}
}

// ���������� HWND ���� ���������
HWND CVConGroup::DoSrvCreated(DWORD nServerPID, HWND hWndCon, DWORD& t1, DWORD& t2, DWORD& t3, int& iFound, HWND& hWndBack)
{
	HWND hWndDC = NULL;

	//gpConEmu->WinEventProc(NULL, EVENT_CONSOLE_START_APPLICATION, hWndCon, (LONG)nServerPID, 0, 0, 0);
	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		CVirtualConsole* pVCon = gp_VCon[i];
		CVConGuard guard(pVCon);
		CRealConsole* pRCon;
		if (pVCon && ((pRCon = pVCon->RCon()) != NULL) && pRCon->isServerCreated())
		{
			if (pRCon->GetServerPID() == nServerPID)
			{
				iFound = i;
				t1 = timeGetTime();
				
				pRCon->OnServerStarted(hWndCon, nServerPID);
				
				t2 = timeGetTime();
				
				hWndDC = pVCon->GetView();
				hWndBack = pVCon->GetBack();

				t3 = timeGetTime();
				break;
			}
		}
	}

	return hWndDC;
}

bool CVConGroup::Activate(CVirtualConsole* apVCon)
{
	if (!isValid(apVCon))
		return false;

	bool lbRc = false;

	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		if (gp_VCon[i] == apVCon)
		{
			ConActivate(i);
			lbRc = (gp_VActive == apVCon);
			break;
		}
	}

	return lbRc;
}

void CVConGroup::MoveActiveTab(CVirtualConsole* apVCon, bool bLeftward)
{
	if (!apVCon)
		apVCon = gp_VActive;

	bool lbChanged = false;

	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		if (gp_VCon[i] == apVCon)
		{
			if (bLeftward)
			{
				if (i > 0)
				{
					CVirtualConsole* p = gp_VCon[i-1];
					gp_VCon[i-1] = gp_VCon[i];
					gp_VCon[i] = p;
					apVCon->RCon()->OnActivate(i-1, i);
					lbChanged = true;
				}
			}
			else
			{
				if ((i < (countof(gp_VCon))) && gp_VCon[i+1])
				{
					CVirtualConsole* p = gp_VCon[i+1];
					gp_VCon[i+1] = gp_VCon[i];
					gp_VCon[i] = p;
					apVCon->RCon()->OnActivate(i+1, i);
					lbChanged = true;
				}
			}
			break;
		}
	}

	UNREFERENCED_PARAMETER(lbChanged);
}

// 0 - based
int CVConGroup::ActiveConNum()
{
	int nActive = -1;

	if (gp_VActive)
	{
		for (size_t i = 0; i < countof(gp_VCon); i++)
		{
			if (gp_VCon[i] == gp_VActive)
			{
				nActive = i; break;
			}
		}
	}

	return nActive;
}

int CVConGroup::GetConCount()
{
	int nCount = 0;

	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		if (gp_VCon[i])
			nCount++;
		else
			break;
	}

	return nCount;
}

BOOL CVConGroup::AttachRequested(HWND ahConWnd, const CESERVER_REQ_STARTSTOP* pStartStop, CESERVER_REQ_STARTSTOPRET* pRet)
{
	CVirtualConsole* pVCon = NULL;
	CRealConsole* pRCon = NULL;
	_ASSERTE(pStartStop->dwPID!=0);

	// ����� ���� �����-�� VCon ���� ������?
	if (!pVCon)
	{
		for (size_t i = 0; i < countof(gp_VCon); i++)
		{
			if (gp_VCon[i] && (pRCon = gp_VCon[i]->RCon()) != NULL)
			{
				if (pRCon->GetServerPID() == pStartStop->dwPID)
				{
					//_ASSERTE(pRCon->GetServerPID() != pStartStop.dwPID);
					pVCon = gp_VCon[i];
					break;
				}
			}
		}
	}

	if (!pVCon)
	{
		for (size_t i = 0; i < countof(gp_VCon); i++)
		{
			if (gp_VCon[i] && (pRCon = gp_VCon[i]->RCon()) != NULL)
			{
				if (pRCon->isDetached())
				{
					pVCon = gp_VCon[i];
					break;
				}
			}
		}
	}

	// ���� �� ����� - ���������, ����� �� �������� ����� �������?
	if (!pVCon)
	{
		RConStartArgs* pArgs = new RConStartArgs;
		pArgs->bDetached = TRUE;
		pArgs->bBackgroundTab = pStartStop->bRunInBackgroundTab;

		// �.�. ��� �������� �� ���������� ������ - ����� � �������
		pVCon = (CVirtualConsole*)SendMessage(ghWnd, gpConEmu->mn_MsgCreateCon, gpConEmu->mn_MsgCreateCon, (LPARAM)pArgs);
		if (pVCon && !isValid(pVCon))
		{
			_ASSERTE(isValid(pVCon));
			pVCon = NULL;
		}
		//if ((pVCon = CreateCon(&args)) == NULL)
		//	return FALSE;
	}

	// �������� ��������� �������
	if (!pVCon->RCon()->AttachConemuC(ahConWnd, pStartStop->dwPID, pStartStop, pRet))
		return FALSE;

	// OK
	return TRUE;
}

CRealConsole* CVConGroup::AttachRequestedGui(LPCWSTR asAppFileName, DWORD anAppPID)
{
	CRealConsole* pRCon;

	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		if (gp_VCon[i] && (pRCon = gp_VCon[i]->RCon()) != NULL)
		{
			if (pRCon->GuiAppAttachAllowed(asAppFileName, anAppPID))
				return pRCon;
		}
	}
	
	return NULL;
}

// ������� ����� ���������� ��������� �� ���� ��������
DWORD CVConGroup::CheckProcesses()
{
	DWORD dwAllCount = 0;

	//mn_ActiveStatus &= ~CES_PROGRAMS;
	for (size_t j = 0; j < countof(gp_VCon); j++)
	{
		if (gp_VCon[j])
		{
			int nCount = gp_VCon[j]->RCon()->GetProcesses(NULL);
			if (nCount)
				dwAllCount += nCount;
		}
	}

	//if (gp_VActive) {
	//    mn_ActiveStatus |= gp_VActive->RCon()->GetProgramStatus();
	//}
	gpConEmu->m_ProcCount = dwAllCount;
	return dwAllCount;
}

bool CVConGroup::ConActivateNext(BOOL abNext)
{
	int nActive = ActiveConNum(), i, j, n1, n2, n3;

	for (j = 0; j <= 1; j++)
	{
		if (abNext)
		{
			if (j == 0)
			{
				n1 = nActive+1; n2 = countof(gp_VCon); n3 = 1;
			}
			else
			{
				n1 = 0; n2 = nActive; n3 = 1;
			}

			if (n1>=n2) continue;
		}
		else
		{
			if (j == 0)
			{
				n1 = nActive-1; n2 = -1; n3 = -1;
			}
			else
			{
				n1 = countof(gp_VCon)-1; n2 = nActive; n3 = -1;
			}

			if (n1<=n2) continue;
		}

		for (i = n1; i != n2 && i >= 0 && i < (int)countof(gp_VCon); i+=n3)
		{
			if (gp_VCon[i])
			{
				return ConActivate(i);
			}
		}
	}

	return false;
}

void CVConGroup::ShowActiveGroup(CVirtualConsole* pOldActive)
{
	CVConGuard VCon(gp_VActive);
	if (!gp_VActive)
		return;

	CVConGroup* pActiveGroup = GetRootOfVCon(gp_VActive);
	if (pActiveGroup && pActiveGroup->mb_ResizeFlag)
	{
		SyncConsoleToWindow();
		// �����������, ����� ���������� ������������
		RECT mainClient = gpConEmu->CalcRect(CER_MAINCLIENT, gp_VActive);
		CVConGroup::ReSizePanes(mainClient);
	}

	// Showing...
	CVConGroup* pOldGroup = (pOldActive && (pOldActive != gp_VActive)) ? GetRootOfVCon(pOldActive) : NULL;

	// ������ ����� �������� ��������
	pActiveGroup->ShowAllVCon(SW_SHOW);
	// � �������� ����������������
	if (pOldGroup && (pOldGroup != pActiveGroup))
		pOldGroup->ShowAllVCon(SW_HIDE);

	InvalidateAll();
}

// nCon - zero-based index of console
bool CVConGroup::ConActivate(int nCon)
{
	FLASHWINFO fl = {sizeof(FLASHWINFO)}; fl.dwFlags = FLASHW_STOP; fl.hwnd = ghWnd;
	FlashWindowEx(&fl); // ��� ������������ ��������� ������ ��������...

	if (nCon >= 0 && nCon < (int)countof(gp_VCon))
	{
		CVirtualConsole* pVCon = gp_VCon[nCon];

		if (pVCon == NULL)
		{
			if (gpSet->isMultiAutoCreate)
			{
				// ������� ����� default-�������
				gpConEmu->RecreateAction(cra_CreateTab/*FALSE*/, FALSE, FALSE);
				return true; // ������� ����� �������
			}

			return false; // ������� � ���� ������� �� ���� �������!
		}

		if (pVCon == gp_VActive)
		{
			// �������� �����
			int nTabCount;
			CRealConsole *pRCon;

			// ��� ���������������� ������� "Win+<Number>" - ������� ���� �������� �������
			if (gpSet->isMultiIterate
			        && ((pRCon = gp_VActive->RCon()) != NULL)
			        && ((nTabCount = pRCon->GetTabCount())>1))
			{
				int nActive = pRCon->GetActiveTab()+1;

				if (nActive >= nTabCount)
					nActive = 0;

				if (pRCon->CanActivateFarWindow(nActive))
					pRCon->ActivateFarWindow(nActive);
			}

			return true; // ���
		}

		bool lbSizeOK = true;
		int nOldConNum = ActiveConNum();

		CVirtualConsole* pOldActive = gp_VActive;

		// �������� PictureView, ��� ��� ����...
		if (gp_VActive && gp_VActive->RCon())
		{
			gp_VActive->RCon()->OnDeactivate(nCon);
		}

		// ����� ������������� �� ����� ������� - �������� �� �������
		if (gp_VActive)
		{
			//int nOldConWidth = gp_VActive->RCon()->TextWidth();
			//int nOldConHeight = gp_VActive->RCon()->TextHeight();
			int nOldConWidth = pVCon->RCon()->TextWidth();
			int nOldConHeight = pVCon->RCon()->TextHeight();
			//int nNewConWidth = pVCon->RCon()->TextWidth();
			//int nNewConHeight = pVCon->RCon()->TextHeight();
			//RECT rcMainClient = gpConEmu->CalcRect(CER_MAINCLIENT, pVCon);
			RECT rcNewCon = gpConEmu->CalcRect(CER_CONSOLE_CUR, pVCon);
			int nNewConWidth = rcNewCon.right;
			int nNewConHeight = rcNewCon.bottom;

			if (nOldConWidth != nNewConWidth || nOldConHeight != nNewConHeight)
			{
				lbSizeOK = pVCon->RCon()->SetConsoleSize(nNewConWidth,nNewConHeight);
			}
		}

		gp_VActive = pVCon;
		pVCon->RCon()->OnActivate(nCon, nOldConNum);

		if (!lbSizeOK)
			SyncWindowToConsole();

		ShowActiveGroup(pOldActive);
	}

	return false;
}

void CVConGroup::OnCreateGroupBegin()
{
	UINT nFreeCell = 0;
	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		if (gp_VCon[i] == NULL)
		{
			nFreeCell = i;
			break;
		}
	}
	gn_CreateGroupStartVConIdx = nFreeCell;
}

void CVConGroup::OnCreateGroupEnd()
{
	gn_CreateGroupStartVConIdx = 0;
}

CVirtualConsole* CVConGroup::CreateCon(RConStartArgs *args, bool abAllowScripts /*= false*/, bool abForceCurConsole /*= false*/)
{
	_ASSERTE(args!=NULL);
	if (!gpConEmu->isMainThread())
	{
		// �������� VCon � ������� ������� �� �����������, �.�. ����� ��������� HWND
		MBoxAssert(gpConEmu->isMainThread());
		return NULL;
	}

	CVirtualConsole* pVCon = NULL;

	if (args->pszSpecialCmd)
		args->ProcessNewConArg(abForceCurConsole);

	if (gpConEmu->m_InsideIntegration && gpConEmu->mb_InsideIntegrationShift)
	{
		gpConEmu->mb_InsideIntegrationShift = false;
		args->bRunAsAdministrator = true;
	}

	//wchar_t* pszScript = NULL; //, szScript[MAX_PATH];

	if (args->pszSpecialCmd
		&& (*args->pszSpecialCmd == CmdFilePrefix
			|| *args->pszSpecialCmd == DropLnkPrefix
			|| *args->pszSpecialCmd == TaskBracketLeft))
	{
		if (!abAllowScripts)
		{
			DisplayLastError(L"Console script are not supported here!", -1);
			return NULL;
		}

		// � �������� "�������" ������ "�������� ����" ��� "������ ������" �������������� ������� ���������� ��������
		wchar_t* pszDataW = gpConEmu->LoadConsoleBatch(args->pszSpecialCmd, &args->pszStartupDir);
		if (!pszDataW)
			return NULL;

		// GO
		pVCon = gpConEmu->CreateConGroup(pszDataW, args->bRunAsAdministrator, args->pszStartupDir);

		SafeFree(pszDataW);
		return pVCon;
	}

	// ���� �� ����� ConEmu �������� ������ ����� ��� ���������

	// Ok, ������ ������� ��������� ������ � gp_VCon � �����������
	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		if (gp_VCon[i] && gp_VCon[i]->RCon() && gp_VCon[i]->RCon()->isDetached())
		{
			// isDetached() means, that ConEmu.exe was started with "/detached" flag
			// so, it is safe to close "dummy" console, that was created on GUI startup
			gp_VCon[i]->RCon()->CloseConsole(false, false);
		}

		if (!gp_VCon[i])
		{
			CVirtualConsole* pOldActive = gp_VActive;
			gb_CreatingActive = true;
			pVCon = CVConGroup::CreateVCon(args, gp_VCon[i]);
			gb_CreatingActive = false;

			BOOL lbInBackground = args->bBackgroundTab && (pOldActive != NULL) && !args->eSplit;

			if (pVCon)
			{
				if (!lbInBackground && pOldActive && pOldActive->RCon())
				{
					pOldActive->RCon()->OnDeactivate(i);
				}

				_ASSERTE(gp_VCon[i] == pVCon);
				gp_VCon[i] = pVCon;
				
				if (!lbInBackground)
				{
					gp_VActive = pVCon;
				}
				else
				{
					_ASSERTE(gp_VActive==pOldActive);
				}
				
				pVCon->InitGhost();

				if (!lbInBackground)
				{
					pVCon->RCon()->OnActivate(i, ActiveConNum());

					//mn_ActiveCon = i;
					//Update(true);

					ShowActiveGroup(pOldActive);
					//TODO("DoubleView: �������� �� ����������?");
					//// ������ ����� �������� ��������
					//gp_VActive->ShowView(SW_SHOW);
					////ShowWindow(gp_VActive->GetView(), SW_SHOW);
					//// � �������� ����������������
					//if (pOldActive && (pOldActive != gp_VActive) && !pOldActive->isVisible())
					//	pOldActive->ShowView(SW_HIDE);
					//	//ShowWindow(pOldActive->GetView(), SW_HIDE);
				}
			}

			break;
		}
	}

	return pVCon;
}

HRGN CVConGroup::GetExclusionRgn(bool abTestOnly/*=false*/)
{
	HRGN hExclusion = NULL;
	int iComb = 0;

	// DoubleView: ���� ������ ��������� �������� - ����� ���������� �������

	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		CVConGuard VCon(gp_VCon[i]);
		if (VCon.VCon() && VCon->isVisible())
		{
			HRGN hVCon = VCon->GetExclusionRgn(abTestOnly);

			if (abTestOnly && hVCon)
			{

				_ASSERTE(hVCon == (HRGN)1);
				return (HRGN)1;
			}

			if (hVCon)
			{
				if (!hExclusion)
				{
					hExclusion = hVCon; // ������ (��� ������������)
				}
				else
				{
					iComb = CombineRgn(hExclusion, hExclusion, hVCon, RGN_OR);
					_ASSERTE(iComb != ERROR);
					DeleteObject(hVCon);
				}
			}
		}
	}

	return hExclusion;
}

RECT CVConGroup::CalcRect(enum ConEmuRect tWhat, RECT rFrom, enum ConEmuRect tFrom, CVirtualConsole* pVCon, enum ConEmuMargins tTabAction/*=CEM_TAB*/)
{
	RECT rc = rFrom;
	RECT rcShift = MakeRect(0,0);

	CVConGuard VCon(pVCon);
	if (!pVCon && (GetActiveVCon(&VCon) >= 0))
	{
		pVCon = VCon.VCon();
	}

	CVConGroup* pGroup = pVCon ? ((CVConGroup*)pVCon->mp_Group) : NULL;

	// ������ rc ������ ��������������� CER_MAINCLIENT/CER_BACK
	RECT rcAddShift = MakeRect(0,0);

	if ((tWhat == CER_DC) && (tFrom != CER_CONSOLE_CUR))
	{
		_ASSERTE(pVCon!=NULL);
		WARNING("warning: DoubleView - ��� ����� ����������");
		RECT rcCalcBack = rFrom;
		// ����� ��������� �������������� ������
		if (tFrom != CER_BACK)
		{
			_ASSERTE(tFrom==CER_BACK);
			rcCalcBack = gpConEmu->CalcRect(CER_BACK, pVCon);
		}
		//--// ��������� �� ������ ���� ������ �����������
		//#ifdef MSGLOGGER
		//_ASSERTE((rcCalcDC.right - rcCalcDC.left)>=(prDC->right - prDC->left));
		//_ASSERTE((rcCalcDC.bottom - rcCalcDC.top)>=(prDC->bottom - prDC->top));
		//#endif
		RECT rcCalcCon = rFrom;
		if (tFrom != CER_CONSOLE_CUR)
		{
			rcCalcCon = CalcRect(CER_CONSOLE_CUR, rcCalcBack, CER_BACK, pVCon);
		}
		// ��������� DC (������)
		_ASSERTE(rcCalcCon.left==0 && rcCalcCon.top==0);
		RECT rcCalcDC = MakeRect(0,0,rcCalcCon.right*gpSetCls->FontWidth(), rcCalcCon.bottom*gpSetCls->FontHeight());

		RECT rcScroll = gpConEmu->CalcMargins(CEM_SCROLL);
		gpConEmu->AddMargins(rcCalcBack, rcScroll, FALSE);

		int nDeltaX = (rcCalcBack.right - rcCalcBack.left) - (rcCalcDC.right - rcCalcDC.left);
		int nDeltaY = (rcCalcBack.bottom - rcCalcBack.top) - (rcCalcDC.bottom - rcCalcDC.top);

		// ������ ������
		if (gpSet->isTryToCenter && (gpConEmu->isZoomed() || gpConEmu->isFullScreen() || gpSet->isQuakeStyle))
		{
			// ������� ���.������. �����
			if (nDeltaX > 0)
			{
				rcAddShift.left = nDeltaX >> 1;
				rcAddShift.right = nDeltaX - rcAddShift.left;
			}
		}
		else
		{
			if (nDeltaX > 0)
				rcAddShift.right = nDeltaX;
		}

		if (gpSet->isTryToCenter && (gpConEmu->isZoomed() || gpConEmu->isFullScreen()))
		{
			if (nDeltaY > 0)
			{
				rcAddShift.top = nDeltaY >> 1;
				rcAddShift.bottom = nDeltaY - rcAddShift.top;
			}
		}
		else
		{
			if (nDeltaY > 0)
				rcAddShift.bottom = nDeltaY;
		}
	}
	
	switch (tWhat)
	{
		case CER_BACK: // switch (tWhat)
		{
			TODO("DoubleView");
			rcShift = gpConEmu->CalcMargins(tTabAction|CEM_CLIENT_MARGINS);
			CConEmuMain::AddMargins(rc, rcShift);
		} break;
		case CER_SCROLL: // switch (tWhat)
		{
			rcShift = gpConEmu->CalcMargins(tTabAction);
			CConEmuMain::AddMargins(rc, rcShift);
			rc.left = rc.right - GetSystemMetrics(SM_CXVSCROLL);
			return rc; // ����� ����� ��� ����� ��������� �� DC (rcAddShift)
		} break;
		case CER_DC: // switch (tWhat)
		case CER_CONSOLE_ALL: // switch (tWhat)
		case CER_CONSOLE_CUR: // switch (tWhat)
		case CER_CONSOLE_NTVDMOFF: // switch (tWhat)
		{
			_ASSERTE(tWhat!=CER_DC || (tFrom==CER_BACK || tFrom==CER_CONSOLE_CUR)); // CER_DC ������ ��������� �� CER_BACK

			if (tFrom == CER_MAINCLIENT)
			{
				// ������ ������ �������� (�����)
				rcShift = gpConEmu->CalcMargins(tTabAction|CEM_CLIENT_MARGINS);
				CConEmuMain::AddMargins(rc, rcShift);
			}
			else if (tFrom == CER_BACK || tFrom == CER_WORKSPACE)
			{
				// -- ��������� ������ ��������� ����� ���������
				//rcShift = gpConEmu->CalcMargins(CEM_SCROLL);
				//CConEmuMain::AddMargins(rc, rcShift);
			}
			else
			{
				// ������ �������� - �� �����������
				_ASSERTE(tFrom == CER_MAINCLIENT);
			}

			RECT rcAll = rc;
			if (pGroup && (tWhat != CER_CONSOLE_ALL)
				&& (tFrom == CER_MAINCLIENT || tFrom == CER_WORKSPACE))
			{
				pGroup->CalcSplitRootRect(rcAll, rc);
			}

			if (tFrom == CER_MAINCLIENT || tFrom == CER_BACK || tFrom == CER_WORKSPACE)
			{
				rcShift = gpConEmu->CalcMargins(CEM_SCROLL);
				CConEmuMain::AddMargins(rc, rcShift);
			}

			//// ��� ����������� ������� �� ������ ����������...
			//         if (gpSetCls->FontWidth()==0 || gpSetCls->FontHeight()==0)
			//             pVCon->InitDC(false, true); // ���������������� ������ ������ �� ���������
			//rc.right ++;
			//int nShift = (gpSetCls->FontWidth() - 1) / 2; if (nShift < 1) nShift = 1;
			//rc.right += nShift;
			// ���� ���� �������
			//if (rcShift.top || rcShift.bottom || )
			//nShift = (gpSetCls->FontWidth() - 1) / 2; if (nShift < 1) nShift = 1;

			if (tWhat != CER_CONSOLE_NTVDMOFF && pVCon && pVCon->RCon() && pVCon->RCon()->isNtvdm())
			{
				// NTVDM ������������� ������ ��������� ������... � 25/28/43/50 �����
				// ����� ���������� ������� ������ (�� ���� ���� �� ������� 16bit
				// ���� 27 �����, �� ������ ����� ����� ����������� ������ � 28 �����)
				RECT rc1 = MakeRect(pVCon->TextWidth*gpSetCls->FontWidth(), pVCon->TextHeight*gpSetCls->FontHeight());

				//gpSet->ntvdmHeight /* pVCon->TextHeight */ * gpSetCls->FontHeight());
				if (rc1.bottom > (rc.bottom - rc.top))
					rc1.bottom = (rc.bottom - rc.top); // ���� ������ ����� �� ������� - ������� ����� :(

				int nS = rc.right - rc.left - rc1.right;

				if (nS>=0)
				{
					rcShift.left = nS / 2;
					rcShift.right = nS - rcShift.left;
				}
				else
				{
					rcShift.left = 0;
					rcShift.right = -nS;
				}

				nS = rc.bottom - rc.top - rc1.bottom;

				if (nS>=0)
				{
					rcShift.top = nS / 2;
					rcShift.bottom = nS - rcShift.top;
				}
				else
				{
					rcShift.top = 0;
					rcShift.bottom = -nS;
				}

				CConEmuMain::AddMargins(rc, rcShift);
			}

			// ���� ����� ������ ������� � �������� ����� ����� � �������
			if (tWhat == CER_CONSOLE_ALL || tWhat == CER_CONSOLE_CUR || tWhat == CER_CONSOLE_NTVDMOFF)
			{
				//2009-07-09 - ClientToConsole ������������ ������, �.�. ����� ���
				//  ����������� ������ ����� ���������� ������ Ideal, � ������ - ������
				//120822 - ���� "(rc.right - rc.left + 1)"
				int nW = (rc.right - rc.left) / gpSetCls->FontWidth();
				int nH = (rc.bottom - rc.top) / gpSetCls->FontHeight();
				rc.left = 0; rc.top = 0; rc.right = nW; rc.bottom = nH;

				//2010-01-19
				if (gpSet->isFontAutoSize)
				{
					if (gpConEmu->wndWidth && rc.right > (LONG)gpConEmu->wndWidth)
						rc.right = gpConEmu->wndWidth;

					if (gpConEmu->wndHeight && rc.bottom > (LONG)gpConEmu->wndHeight)
						rc.bottom = gpConEmu->wndHeight;
				}

				#ifdef _DEBUG
				_ASSERTE(rc.bottom>=MIN_CON_HEIGHT);
				#endif

				// ��������, ��� � RealConsole ��������� ������� �����, ������� �������� ��������� ����� �������
				if (pVCon)
				{
					CRealConsole* pRCon = pVCon->RCon();

					if (pRCon)
					{
						COORD crMaxConSize = {0,0};

						// ���� ���� ������� ��������� ������� - �� �������� �������������
						// � ����� ��������� ������� ������, �.�. ��� �������� ����� ������
						// ������� ������ (�������� ����������� ���������� ������)
						if (pRCon->GetMaxConSize(&crMaxConSize))
						{
							if (rc.right > crMaxConSize.X)
								rc.right = crMaxConSize.X;

							if (rc.bottom > crMaxConSize.Y)
								rc.bottom = crMaxConSize.Y;
						}
					}
				}
			}
			else
			{
				// �������������, �������������
				CConEmuMain::AddMargins(rc, rcAddShift);
			}
		}
		break;

	default:
		_ASSERTE(FALSE && "No supported value");
		break;
	}

	return rc;
}

void CVConGroup::CalcSplitConSize(COORD size, COORD& sz1, COORD& sz2)
{
	sz1 = size; sz2 = size;

	RECT rcCon = MakeRect(size.X, size.Y);
	RECT rcPixels = gpConEmu->CalcRect(CER_DC, rcCon, CER_CONSOLE_CUR, gp_VActive);
	// � �������� ����� ��������� ����������� � ���������
	_ASSERTE(gpSet->isAlwaysShowScrollbar!=1); // ��������
	if (m_SplitType == RConStartArgs::eSplitHorz)
	{
		if (gpSet->nSplitWidth && (rcPixels.right > (LONG)gpSet->nSplitWidth))
			rcPixels.right -= gpSet->nSplitWidth;
	}
	else
	{
		if (gpSet->nSplitHeight && (rcPixels.bottom > (LONG)gpSet->nSplitHeight))
			rcPixels.bottom -= gpSet->nSplitHeight;
	}

	RECT rc1 = rcPixels, rc2 = rcPixels, rcSplitter;
	CalcSplitRect(rcPixels, rc1, rc2, rcSplitter);

	RECT rcCon1 = CVConGroup::CalcRect(CER_CONSOLE_CUR, rc1, CER_BACK, gp_VActive);
	RECT rcCon2 = CVConGroup::CalcRect(CER_CONSOLE_CUR, rc2, CER_BACK, gp_VActive);

	//int nSplit = max(1,min(mn_SplitPercent10,999));

	//RECT rcScroll = gpConEmu->CalcMargins(CEM_SCROLL);

	if (m_SplitType == RConStartArgs::eSplitHorz)
	{
		//if (size.X >= gpSet->nSplitWidth)
		//	size.X -= gpSet->nSplitWidth;

		//_ASSERTE(rcScroll.left==0);
		//if (size.X > (UINT)(rcScroll.right * 2))
		//{
		//	_ASSERTE(gpSet->isAlwaysShowScrollbar==1); // ���� ������ �������� ������ ��� ���������� ��������� �������
		//	size.X -= rcScroll.right * 2;
		//}

		//sz1.X = max(((size.X+1) * nSplit / 1000),MIN_CON_WIDTH);
		//sz2.X = max((size.X - sz1.X),MIN_CON_WIDTH);
		sz1.X = max(rcCon1.right,MIN_CON_WIDTH);
		sz2.X = max(rcCon2.right,MIN_CON_WIDTH);
	}
	else
	{
		//sz1.Y = max(((size.Y+1) * nSplit / 1000),MIN_CON_HEIGHT);
		//sz2.Y = max((size.Y - sz1.Y),MIN_CON_HEIGHT);
		sz1.Y = max(rcCon1.bottom,MIN_CON_HEIGHT);
		sz2.Y = max(rcCon2.bottom,MIN_CON_HEIGHT);
	}
}

void CVConGroup::SetConsoleSizes(const COORD& size, bool abSync)
{
	CVConGuard VCon(mp_Item);

	// �����������. ����� ��������� ������ �������. � ������������ ��������� ������� ���� ������.
#if 0
	// ��� �� ������ ���������... ntvdm.exe �� ����������� ����� ������ �� 16��� ����������
	if (isNtvdm())
	{
		//if (size.X == 80 && size.Y>25 && lastSize1.X != size.X && size.Y == lastSize1.Y) {
		TODO("Ntvdm ������-�� �� ������ ������������� ������ ������� � 25/28/50 ��������...")
		//}
		return; // ������ ��������� �������� ������� ��� 16��� ����������
	}
#endif

	//#ifdef MSGLOGGER
	//	char szDbg[100]; wsprintfA(szDbg, "SetAllConsoleWindowsSize({%i,%i},%i)\n", size.X, size.Y, updateInfo);
	//	DEBUGLOGFILE(szDbg);
	//#endif
	//	//g_LastConSize = size;

	if (isPictureView())
	{
		_ASSERTE(FALSE && "isPictureView() must distinct by panes/consoles");
		gpConEmu->isPiewUpdate = true;
		return;
	}




	// ����������� �������
	CVConGuard VCon1(mp_Grp1 ? mp_Grp1->mp_Item : NULL);
	CVConGuard VCon2(mp_Grp2 ? mp_Grp2->mp_Item : NULL);

	if ((m_SplitType == RConStartArgs::eSplitNone) || !mp_Grp1 || !mp_Grp2)
	{
		_ASSERTE(mp_Grp1==NULL && mp_Grp2==NULL);


		RECT rcCon = MakeRect(size.X,size.Y);
		if (VCon.VCon() && VCon->RCon())
		{
			CRealConsole* pRCon = VCon->RCon();
			COORD CurSize = {(SHORT)pRCon->TextWidth(), (SHORT)pRCon->TextHeight()};
			if ((CurSize.X != size.X) || (CurSize.Y != size.Y))
			{
				if (!VCon->RCon()->SetConsoleSize(size.X,size.Y, 0/*don't change*/, abSync ? CECMD_SETSIZESYNC : CECMD_SETSIZENOSYNC))
					rcCon = MakeRect(VCon->TextWidth,VCon->TextHeight);
			}
		}


		return;
	}


	// Do Split

	COORD sz1 = size, sz2 = size;

	CalcSplitConSize(size, sz1, sz2);

	mp_Grp1->SetConsoleSizes(sz1, abSync);
	mp_Grp2->SetConsoleSizes(sz2, abSync);
}

// size in columns and lines
// ����� ����� �������� ������ GUI � "�����" ������ �������� � ��������
// �.�. ���� �� ����������� ���� 2 �������, � ������ ������ 80x25
// �� ��� ��� ������� ������ ����� 40x25 � GUI ������������� ��� 80x25
// � ��������, ��� ������� ����� ���� �� � � CConEmu ��������, �� ��� �������� ���� ����� �����
void CVConGroup::SetAllConsoleWindowsSize(const COORD& size, /*bool updateInfo,*/ bool bSetRedraw /*= false*/, bool bResizeConEmuWnd /*= false*/)
{
	CVConGuard VCon(gp_VActive);
	CVConGroup* pRoot = GetRootOfVCon(VCon.VCon());

	// �����������. ����� ��������� ������ �������. � ������������ ��������� ������� ���� ������.
#if 0
	// ��� �� ������ ���������... ntvdm.exe �� ����������� ����� ������ �� 16��� ����������
	if (isNtvdm())
	{
		//if (size.X == 80 && size.Y>25 && lastSize1.X != size.X && size.Y == lastSize1.Y) {
		TODO("Ntvdm ������-�� �� ������ ������������� ������ ������� � 25/28/50 ��������...")
		//}
		return; // ������ ��������� �������� ������� ��� 16��� ����������
	}
#endif

#ifdef MSGLOGGER
	char szDbg[100]; wsprintfA(szDbg, "SetAllConsoleWindowsSize({%i,%i},%i)\n", size.X, size.Y, bSetRedraw);
	DEBUGLOGFILE(szDbg);
#endif
	g_LastConSize = size;

	if (!pRoot)
	{
		_ASSERTE(pRoot && "Must be defined already!");
		return;
	}

	if (isPictureView())
	{
		gpConEmu->isPiewUpdate = true;
		return;
	}

	RECT rcCon = MakeRect(size.X,size.Y);

	if (bSetRedraw /*&& gp_VActive*/)
	{
		SetRedraw(FALSE);
	}


	// Go (size real consoles)
	pRoot->SetConsoleSizes(size, bSetRedraw/*as Sync*/);


	if (bSetRedraw /*&& gp_VActive*/)
	{
		SetRedraw(TRUE);
		Redraw();
	}


	//// update size info
	//// !!! ��� ����� ������ �������
	//WARNING("updateInfo �����");
	///*if (updateInfo && !mb_isFullScreen && !isZoomed() && !isIconic())
	//{
	//    gpSet->UpdateSize(size.X, size.Y);
	//}*/
	//RECT rcCon = MakeRect(size.X,size.Y);

	//if (apVCon)
	//{
	//	if (!apVCon->RCon()->SetConsoleSize(size.X,size.Y))
	//		rcCon = MakeRect(apVCon->TextWidth,apVCon->TextHeight);
	//}

	// ��� ������ �� ������� "Settings..." � ������� ������ "Apply" (Size & Pos)
	if (bResizeConEmuWnd)
	{
		/* ������� ��������������� �������� ������� */
		RECT rcWnd = gpConEmu->CalcRect(CER_MAIN, rcCon, CER_CONSOLE_ALL, NULL);
		RECT wndR; GetWindowRect(ghWnd, &wndR); // ������� XY
		MOVEWINDOW(ghWnd, wndR.left, wndR.top, rcWnd.right, rcWnd.bottom, 1);
	}
}

void CVConGroup::SyncAllConsoles2Window(RECT rcWnd, enum ConEmuRect tFrom /*= CER_MAIN*/, bool bSetRedraw /*= false*/)
{
	if (!isVConExists(0))
		return;
	CVConGuard VCon(gp_VActive);
	RECT rcAllCon = gpConEmu->CalcRect(CER_CONSOLE_ALL, rcWnd, tFrom, VCon.VCon());
	COORD crNewAllSize = {rcAllCon.right,rcAllCon.bottom};
	SetAllConsoleWindowsSize(crNewAllSize, bSetRedraw);
}

void CVConGroup::LockSyncConsoleToWindow(bool abLockSync)
{
	gb_SkipSyncSize = abLockSync;
}

// �������� ������ ������� �� ������� ���� (��������)
void CVConGroup::SyncConsoleToWindow(LPRECT prcNewWnd/*=NULL*/)
{
	if (gb_SkipSyncSize || isNtvdm())
		return;

	CVConGuard VCon(gp_VActive);
	if (!VCon.VCon())
		return;

	_ASSERTE(gpConEmu->mn_InResize <= 1);

	#ifdef _DEBUG
	if (gpConEmu->change2WindowMode!=(DWORD)-1)
	{
		_ASSERTE(gpConEmu->change2WindowMode==(DWORD)-1);
	}
	#endif

	//gp_VActive->RCon()->SyncConsole2Window();

	RECT rcWnd;
	if (prcNewWnd)
		rcWnd = gpConEmu->CalcRect(CER_MAINCLIENT, *prcNewWnd, CER_MAIN, VCon.VCon());
	else
		rcWnd = gpConEmu->CalcRect(CER_MAINCLIENT, VCon.VCon());

	SyncAllConsoles2Window(rcWnd, CER_MAINCLIENT);
}

// ���������� ������ ��������� ���� �� �������� ������� gp_VActive
void CVConGroup::SyncWindowToConsole()
{
	TODO("warning: �������� ���������, ����� ��� ��������� ������� � ������� �� ����� ���� �������� � ConEmu");

#if 0
	_ASSERTE(FALSE && "May be this function must be eliminated!");
	// �������� ����� ����� ������ �� ��������� GUI �� ������� �������.
	// ���� ������� ������ - ���������, ���� ������ - �������������.

	DEBUGLOGFILE("SyncWindowToConsole\n");

	if (gb_SkipSyncSize || !gp_VActive)
		return;

#ifdef _DEBUG
	_ASSERTE(GetCurrentThreadId() == gpConEmu->mn_MainThreadId);
	if (gp_VActive->TextWidth == 80)
	{
		int nDbg = gp_VActive->TextWidth;
	}
#endif

	CVConGuard VCon(gp_VActive);
	CRealConsole* pRCon = gp_VActive->RCon();

	if (pRCon && (gp_VActive->TextWidth != pRCon->TextWidth() || gp_VActive->TextHeight != pRCon->TextHeight()))
	{
		_ASSERTE(FALSE);
		gp_VActive->Update();
	}

	RECT rcDC = gp_VActive->GetRect();
	/*MakeRect(gp_VActive->Width, gp_VActive->Height);
	if (gp_VActive->Width == 0 || gp_VActive->Height == 0) {
	rcDC = MakeRect(gp_VActive->winSize.X, gp_VActive->winSize.Y);
	}*/
	//_ASSERTE(rcDC.right>250 && rcDC.bottom>200);
	RECT rcWnd = CalcRect(CER_MAIN, rcDC, CER_DC, gp_VActive); // ������� ����
	//GetCWShift(ghWnd, &cwShift);
	RECT wndR; GetWindowRect(ghWnd, &wndR); // ������� XY

	if (gpSetCls->isAdvLogging)
	{
		char szInfo[128]; wsprintfA(szInfo, "SyncWindowToConsole(Cols=%i, Rows=%i)", gp_VActive->TextWidth, gp_VActive->TextHeight);
		CVConGroup::LogString(szInfo, TRUE);
	}

	gpSetCls->UpdateSize(gp_VActive->TextWidth, gp_VActive->TextHeight);
	MOVEWINDOW(ghWnd, wndR.left, wndR.top, rcWnd.right, rcWnd.bottom, 1);
#endif
}

// ��� ����� ������� �������, ��������������� ����, ��� ���� �� ����
// ������ ���� �������� �������, ��� Split-screen
uint CVConGroup::TextWidth()
{
	uint nWidth = gpSet->_wndWidth;
	if (!gp_VActive)
	{
		_ASSERTE(FALSE && "No active VCon");
	}
	else
	{
		CVConGuard VCon(gp_VActive);
		CVConGroup* p = NULL;
		if (gp_VActive && gp_VActive->mp_Group)
		{
			p = ((CVConGroup*)gp_VActive->mp_Group)->GetRootGroup();
		}

		if (p != NULL)
		{
			SIZE sz; p->GetAllTextSize(sz);
        	nWidth = sz.cx; // p->AllTextWidth();
		}
		else
		{
			_ASSERTE(p && "CVConGroup MUST BE DEFINED!");

			if (gp_VActive->RCon())
			{
				// ��� ������� ����� ���� ��������� - gp_VActive ��� �� �������������
				// ��� ��� � TextWidth/TextHeight �� ���������
				//-- gpSetCls->UpdateSize(gp_VActive->TextWidth, gp_VActive->TextHeight);
				nWidth = gp_VActive->RCon()->TextWidth();
			}
		}
	}
	return nWidth;
}
uint CVConGroup::TextHeight()
{
	uint nHeight = gpSet->_wndHeight;
	if (!gp_VActive)
	{
		_ASSERTE(FALSE && "No active VCon");
	}
	else
	{
		CVConGuard VCon(gp_VActive);
		CVConGroup* p = NULL;
		if (gp_VActive && gp_VActive->mp_Group)
		{
			p = ((CVConGroup*)gp_VActive->mp_Group)->GetRootGroup();
		}

		if (p != NULL)
		{
			SIZE sz; p->GetAllTextSize(sz);
        	nHeight = sz.cy; // p->AllTextHeight();
		}
		else
		{
			_ASSERTE(p && "CVConGroup MUST BE DEFINED!");

			if (gp_VActive->RCon())
			{
				// ��� ������� ����� ���� ��������� - gp_VActive ��� �� �������������
				// ��� ��� � TextWidth/TextHeight �� ���������
				//-- gpSetCls->UpdateSize(gp_VActive->TextWidth, gp_VActive->TextHeight);
				nHeight = gp_VActive->RCon()->TextHeight();
			}
		}
	}
	return nHeight;
}

RECT CVConGroup::AllTextRect(bool abMinimal /*= false*/)
{
	RECT rcText = MakeRect(MIN_CON_WIDTH,MIN_CON_HEIGHT);

	if (!gp_VActive)
	{
		_ASSERTE(FALSE && "No active VCon");
	}
	else
	{
		CVConGuard VCon(gp_VActive);
		CVConGroup* p = NULL;
		if (gp_VActive && gp_VActive->mp_Group)
		{
			p = ((CVConGroup*)gp_VActive->mp_Group)->GetRootGroup();
		}

		if (p != NULL)
		{
			SIZE sz = {MIN_CON_WIDTH,MIN_CON_HEIGHT};
			p->GetAllTextSize(sz, abMinimal);
			rcText.right = sz.cx;
			rcText.bottom = sz.cy;
		}
		else
		{
			_ASSERTE(p && "CVConGroup MUST BE DEFINED!");

			if (gp_VActive->RCon())
			{
				// ��� ������� ����� ���� ��������� - gp_VActive ��� �� �������������
				// ��� ��� � TextWidth/TextHeight �� ���������
				//-- gpSetCls->UpdateSize(gp_VActive->TextWidth, gp_VActive->TextHeight);
				int nWidth = gp_VActive->RCon()->TextWidth();
				int nHeight = gp_VActive->RCon()->TextHeight();
				if (nWidth <= 0 || nHeight <= 0)
				{
					_ASSERTE(FALSE && "gp_VActive->RCon()->TextWidth()>=0 && gp_VActive->RCon()->TextHeight()>=0");
				}
				else
				{
					rcText.right = nWidth;
					rcText.bottom = nHeight;
				}
			}
		}
	}

	return rcText;
}

// WindowMode: rNormal, rMaximized, rFullScreen
// rcWnd: ������ ghWnd
// Returns: true - ���� �������, ����� ����������
bool CVConGroup::PreReSize(uint WindowMode, RECT rcWnd, enum ConEmuRect tFrom /*= CER_MAIN*/, bool bSetRedraw /*= false*/)
{
	bool lbRc = true;
	CVConGuard VCon(gp_VActive);

	if (gp_VActive)
	{
		gpConEmu->AutoSizeFont(rcWnd, tFrom);
	}

	if (tFrom == CER_MAIN)
	{
		rcWnd = gpConEmu->CalcRect(CER_MAINCLIENT, rcWnd, CER_MAIN);
		tFrom = CER_MAINCLIENT;
	}

	RECT rcCon = CalcRect(CER_CONSOLE_ALL, rcWnd, tFrom, gp_VActive);

	if (!rcCon.right || !rcCon.bottom) { rcCon.right = gpConEmu->wndWidth; rcCon.bottom = gpConEmu->wndHeight; }

	COORD size = {rcCon.right, rcCon.bottom};
	if (isVConExists(0))
	{
		SetAllConsoleWindowsSize(size, bSetRedraw);
	}

	//if (bSetRedraw /*&& gp_VActive*/)
	//{
	//	SetRedraw(FALSE);
	//}

	//TODO("DoubleView");
	//if (gp_VActive && !gp_VActive->RCon()->SetConsoleSize(rcCon.right, rcCon.bottom))
	//{
	//	LogString("!!!SetConsoleSize FAILED!!!");

	//	lbRc = false;
	//}

	//if (bSetRedraw /*&& gp_VActive*/)
	//{
	//	SetRedraw(TRUE);
	//	Redraw();
	//}

	return lbRc;
}

void CVConGroup::SetRedraw(bool abRedrawEnabled)
{
	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		CVConGuard VCon(gp_VCon[i]);
		if (VCon.VCon() && VCon->isVisible())
			VCon->SetRedraw(abRedrawEnabled);
	}
}

void CVConGroup::Redraw()
{
	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		CVConGuard VCon(gp_VCon[i]);
		if (VCon.VCon() && VCon->isVisible())
			VCon->Redraw();
	}
}

void CVConGroup::InvalidateGaps()
{
	int iRc = SIMPLEREGION;

	RECT rc = {};
	GetClientRect(ghWnd, &rc);
	HRGN h = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);

	#if 0
	TODO("DoubleView");
	if ((iRc != NULLREGION) && mp_TabBar->GetRebarClientRect(&rc))
	{
		HRGN h2 = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);
		iRc = CombineRgn(h, h, h2, RGN_DIFF);
		DeleteObject(h2);

		if (iRc == NULLREGION)
			goto wrap;
	}

	if ((iRc != NULLREGION) && mp_Status->GetStatusBarClientRect(&rc))
	{
		HRGN h2 = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);
		CombineRgn(h, h, h2, RGN_DIFF);
		DeleteObject(h2);

		if (iRc == NULLREGION)
			goto wrap;
	}
	#endif

	// ������ - VConsole (��� �������!)
	if (iRc != NULLREGION)
	{
		TODO("DoubleView");
		TODO("�������� �� Background, ����� �����");
		HWND hView = gp_VActive ? gp_VActive->GetView() : NULL;
		if (hView && GetWindowRect(hView, &rc))
		{
			MapWindowPoints(NULL, ghWnd, (LPPOINT)&rc, 2);
			HRGN h2 = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);
			iRc = CombineRgn(h, h, h2, RGN_DIFF);
			DeleteObject(h2);

			if (iRc == NULLREGION)
				goto wrap;
		}
	}

	InvalidateRgn(ghWnd, h, FALSE);

wrap:
	DeleteObject(h);
}

void CVConGroup::PaintGaps(HDC hDC)
{
	bool lbReleaseDC = false;

	if (hDC == NULL)
	{
		hDC = GetDC(ghWnd); // ������� ����!
		lbReleaseDC = true;
	}

	HBRUSH hBrush = NULL;


	bool lbFade = gpSet->isFadeInactive && !gpConEmu->isMeForeground(true);


	////RECT rcClient = GetGuiClientRect(); // ���������� ����� �������� ����
	//RECT rcClient = gpConEmu->CalcRect(CER_WORKSPACE);

	_ASSERTE(ghWndWork!=NULL);
	RECT rcClient = {};
	GetClientRect(ghWndWork, &rcClient);

	//HWND hView = gp_VActive ? gp_VActive->GetView() : NULL;

	//if (!hView || !IsWindowVisible(hView))
	if (!isVConExists(0))
	{
		int nColorIdx = RELEASEDEBUGTEST(0/*Black*/,1/*Blue*/);
		HBRUSH hBrush = CreateSolidBrush(gpSet->GetColors(-1, lbFade)[nColorIdx]);

		FillRect(hDC, &rcClient, hBrush);
	}
	else
	{
		COLORREF crBack = lbFade ? gpSet->GetFadeColor(gpSet->nStatusBarBack) : gpSet->nStatusBarBack;
		COLORREF crText = lbFade ? gpSet->GetFadeColor(gpSet->nStatusBarLight) : gpSet->nStatusBarLight;
		COLORREF crDash = lbFade ? gpSet->GetFadeColor(gpSet->nStatusBarDark) : gpSet->nStatusBarDark;

		hBrush = CreateSolidBrush(crBack);

		TODO("DoubleView: �������� ��������� �������� ����������");

		FillRect(hDC, &rcClient, hBrush);

		//int iRc = SIMPLEREGION;

		//HRGN h = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);

		//TODO("DoubleView");
		//if ((iRc != NULLREGION) && gpConEmu->mp_TabBar->GetRebarClientRect(&rc))
		//{
		//	HRGN h2 = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);
		//	iRc = CombineRgn(h, h, h2, RGN_DIFF);
		//	DeleteObject(h2);
		//}

		//if ((iRc != NULLREGION) && gpConEmu->mp_Status->GetStatusBarClientRect(&rc))
		//{
		//	HRGN h2 = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);
		//	CombineRgn(h, h, h2, RGN_DIFF);
		//	DeleteObject(h2);
		//}

		//// ������ - VConsole (��� �������!)
		//if (iRc != NULLREGION)
		//{
		//	for (size_t i = 0; i < countof(gp_VCon); i++)
		//	{
		//		CVConGuard VCon(gp_VCon[i]);
		//		if (VCon.VCon() && VCon->isVisible())
		//		{
		//			HWND hView = VCon.VCon() ? VCon->GetView() : NULL;
		//			if (hView && GetWindowRect(hView, &rc))
		//			{
		//				MapWindowPoints(NULL, ghWnd, (LPPOINT)&rc, 2);
		//				HRGN h2 = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);
		//				iRc = CombineRgn(h, h, h2, RGN_DIFF);
		//				DeleteObject(h2);
		//				if (iRc == NULLREGION)
		//					break;
		//			}
		//		}
		//	}
		//}

		//if (iRc != NULLREGION)
		//	FillRgn(hDC, h, hBrush);

		//DeleteObject(h);

		////RECT rcMargins = gpConEmu->CalcMargins(CEM_TAB); // �������� �������, ������� ������� �����
		////AddMargins(rcClient, rcMargins, FALSE);
		////// �� ������ ��� /max - ghWnd DC ��� �� ������� ���� ���������
		//////RECT offsetRect; Get ClientRect(ghWnd DC, &offsetRect);
		////RECT rcWndClient; Get ClientRect(ghWnd, &rcWndClient);
		////RECT rcCalcCon = gpConEmu->CalcRect(CER_BACK, rcWndClient, CER_MAINCLIENT);
		////RECT rcCon = gpConEmu->CalcRect(CER_CONSOLE, rcCalcCon, CER_BACK);
		//// -- �������� �� ��������� - �� ��������� ������������� � Maximized
		////RECT offsetRect = gpConEmu->CalcRect(CER_BACK, rcCon, CER_CONSOLE);
		///*
		//RECT rcClient = {0};
		//if (ghWnd DC) {
		//	Get ClientRect(ghWnd DC, &rcClient);
		//	MapWindowPoints(ghWnd DC, ghWnd, (LPPOINT)&rcClient, 2);
		//}
		//*/
		//RECT dcSize = CalcRect(CER_DC, rcClient, CER_MAINCLIENT);
		//RECT client = CalcRect(CER_DC, rcClient, CER_MAINCLIENT, NULL, &dcSize);
		//WARNING("������� � CalcRect");
		//RECT offsetRect; memset(&offsetRect,0,sizeof(offsetRect));

		//if (gp_VActive && gp_VActive->Width && gp_VActive->Height)
		//{
		//	if ((gpSet->isTryToCenter && (isZoomed() || mb_isFullScreen || gpSet->isQuakeStyle))
		//			|| isNtvdm())
		//	{
		//		offsetRect.left = (client.right+client.left-(int)gp_VActive->Width)/2;
		//		offsetRect.top = (client.bottom+client.top-(int)gp_VActive->Height)/2;
		//	}

		//	if (offsetRect.left<client.left) offsetRect.left=client.left;

		//	if (offsetRect.top<client.top) offsetRect.top=client.top;

		//	offsetRect.right = offsetRect.left + gp_VActive->Width;
		//	offsetRect.bottom = offsetRect.top + gp_VActive->Height;

		//	if (offsetRect.right>client.right) offsetRect.right=client.right;

		//	if (offsetRect.bottom>client.bottom) offsetRect.bottom=client.bottom;
		//}
		//else
		//{
		//	offsetRect = client;
		//}

		//// paint gaps between console and window client area with first color
		//RECT rect;
		////TODO:!!!
		//// top
		//rect = rcClient;
		//rect.bottom = offsetRect.top;

		//if (!IsRectEmpty(&rect))
		//	FillRect(hDC, &rect, hBrush);

		//#ifdef _DEBUG
		////GdiFlush();
		//#endif
		//// right
		//rect.left = offsetRect.right;
		//rect.bottom = rcClient.bottom;

		//if (!IsRectEmpty(&rect))
		//	FillRect(hDC, &rect, hBrush);

		//#ifdef _DEBUG
		////GdiFlush();
		//#endif
		//// left
		//rect.left = 0;
		//rect.right = offsetRect.left;
		//rect.bottom = rcClient.bottom;

		//if (!IsRectEmpty(&rect))
		//	FillRect(hDC, &rect, hBrush);

		//#ifdef _DEBUG
		////GdiFlush();
		//#endif
		//// bottom
		//rect.left = 0;
		//rect.right = rcClient.right;
		//rect.top = offsetRect.bottom;
		//rect.bottom = rcClient.bottom;

		//if (!IsRectEmpty(&rect))
		//	FillRect(hDC, &rect, hBrush);

		//#ifdef _DEBUG
		////GdiFlush();
		//#endif

	}

	if (hBrush)
		DeleteObject(hBrush);

	if (lbReleaseDC)
		ReleaseDC(ghWnd, hDC);
}

DWORD CVConGroup::GetFarPID(BOOL abPluginRequired/*=FALSE*/)
{
	DWORD dwPID = 0;

	if (gp_VActive && gp_VActive->RCon())
		dwPID = gp_VActive->RCon()->GetFarPID(abPluginRequired);

	return dwPID;
}

// ����� ��� �������� ������ ������� �� ������ ����� ����� ���� ���-�� ����������
void CVConGroup::OnVConCreated(CVirtualConsole* apVCon, const RConStartArgs *args)
{
	if (!gp_VActive || (gb_CreatingActive && !args->bBackgroundTab))
	{
		gp_VActive = apVCon;

		HWND hWndDC = gp_VActive->GetView();
		if (hWndDC != NULL)
		{
			_ASSERTE(hWndDC==NULL && "Called from constructor, NULL expected");
			// ������ ����� �������� ��������� �������
			apiShowWindow(gp_VActive->GetView(), SW_SHOW);
		}
	}
}

void CVConGroup::OnGuiFocused(BOOL abFocus, BOOL abForceChild /*= FALSE*/)
{
	for (size_t i = 0; i < countof(gp_VCon); i++)
	{
		CVConGuard VCon(gp_VCon[i]);
		if (VCon.VCon() && VCon->isVisible())
			VCon->RCon()->OnGuiFocused(abFocus, abForceChild);
	}
}

void CVConGroup::OnConsoleResize(bool abSizingToDo)
{
	//DEBUGSTRERR(L"�� ��������. ConEmu �� ������ ��������� ��� ����� ������� �� �������\n");
	DEBUGSTRERR(L"CVConGroup::OnConsoleResize must NOT!!! be called while CONSOLE size is changed (from console)\n");

	//MSetter lInConsoleResize(&mb_InConsoleResize);
	// ����������� ������ � ���� ����, ����� ����� ���������
	_ASSERTE(gpConEmu->isMainThread() && !gpConEmu->isIconic());

	//COORD c = ConsoleSizeFromWindow();
	RECT client = gpConEmu->GetGuiClientRect();

	// ��������, ����� �� ��������� isIconic
	if (client.bottom > 10)
	{
		CVConGuard VCon(gp_VActive);
		gpConEmu->AutoSizeFont(client, CER_MAINCLIENT);
		RECT c = CalcRect(CER_CONSOLE_CUR, client, CER_MAINCLIENT, gp_VActive);
		// ����� �� ���������� ������� ������ ��� - �������� ����������� �� �������� ������
		// ��� ���������� ������ ����� ����
		BOOL lbSizeChanged = FALSE;
		int nCurConWidth = (int)gp_VActive->RCon()->TextWidth();
		int nCurConHeight = (int)gp_VActive->RCon()->TextHeight();

		if (gp_VActive)
		{
			lbSizeChanged = (c.right != nCurConWidth || c.bottom != nCurConHeight);
		}

		if (gpSetCls->isAdvLogging)
		{
			char szInfo[160]; wsprintfA(szInfo, "OnConsoleResize: lbSizeChanged=%i, client={{%i,%i},{%i,%i}}, CalcCon={%i,%i}, CurCon={%i,%i}",
			                            lbSizeChanged, client.left, client.top, client.right, client.bottom,
			                            c.right, c.bottom, nCurConWidth, nCurConHeight);
			CVConGroup::LogString(szInfo, TRUE);
		}

		if (!gpConEmu->isSizing() &&
		        (abSizingToDo /*����� ��������� ������� ������*/ ||
		         gpConEmu->gbPostUpdateWindowSize /*����� ���������/������� �����*/ ||
		         lbSizeChanged /*��� ������ � ����������� ������� �� ��������� � ���������*/))
		{
			gpConEmu->gbPostUpdateWindowSize = false;

			if (isNtvdm())
			{
				gpConEmu->SyncNtvdm();
			}
			else
			{
				if (!gpConEmu->mb_isFullScreen && !gpConEmu->isZoomed() && !abSizingToDo)
					SyncWindowToConsole();
				else
					SyncConsoleToWindow();

				gpConEmu->OnSize(true, 0, client.right, client.bottom);
			}

			//_ASSERTE(gp_VActive!=NULL);
			if (gp_VActive)
			{
				g_LastConSize = MakeCoord(gp_VActive->TextWidth,gp_VActive->TextHeight);
			}

			// ��������� "���������" ������ ����, ��������� �������������
			if (abSizingToDo)
				gpConEmu->UpdateIdealRect();

			//if (lbSizingToDo && !mb_isFullScreen && !isZoomed() && !isIconic()) {
			//	GetWindowRect(ghWnd, &mrc_Ideal);
			//}
		}
		else if (gp_VActive
		        && (g_LastConSize.X != (int)gp_VActive->TextWidth
		            || g_LastConSize.Y != (int)gp_VActive->TextHeight))
		{
			// �� ����, ���� �� �������� ������ ��� 16-��� ����������
			if (isNtvdm())
				gpConEmu->SyncNtvdm();

			g_LastConSize = MakeCoord(gp_VActive->TextWidth,gp_VActive->TextHeight);
		}
	}
}

// ���������� �� CConEmuMain::OnSize
void CVConGroup::ReSizePanes(RECT mainClient)
{
	//RECT mainClient = MakeRect(newClientWidth,newClientHeight);
	if (!gp_VActive)
	{
		// ��� ���� ������
		_ASSERTE(gp_VActive);
		return;
	}

	CVConGuard VCon(gp_VActive);
	CVirtualConsole* pVCon = VCon.VCon();

	RECT rcNewCon = {};

	WARNING("warning: Need to be corrected for release / DoubleView");
#if 0
	PRAGMA_ERROR("Need to be corrected");
	TODO("DoubleView");
	RECT dcSize = CalcRect(CER_DC, mainClient, CER_MAINCLIENT, pVCon);
	RECT client = CalcRect(CER_DC, mainClient, CER_MAINCLIENT, pVCon, &dcSize);
	WARNING("������� � CalcRect");
	

	TODO("��� DoubleView - �����������");
	if (gpSet->isAlwaysShowScrollbar == 1)
		client.right += GetSystemMetrics(SM_CXVSCROLL);

	if (pVCon && pVCon->Width && pVCon->Height)
	{
		if (pVCon->GuiWnd() && pVCon->RCon()->isGuiOverCon())
		{
			// ���� �������� � ������ "GUI �� �������" - ������ ��� ��������� �������
			rcNewCon = dcSize;
		}
		else
		{
			// ����� - "����������" ������� �������� �������� �������������� (�� ���������)
			if ((gpSet->isTryToCenter && (gpConEmu->isZoomed() || gpConEmu->mb_isFullScreen || gpSet->isQuakeStyle))
					|| isNtvdm())
			{
				rcNewCon.left = (client.right + client.left - (int)pVCon->Width)/2;
				if (!gpSet->isQuakeStyle)
					rcNewCon.top = (client.bottom + client.top - (int)pVCon->Height)/2;
			}

			if (rcNewCon.left<client.left) rcNewCon.left=client.left;

			if (rcNewCon.top<client.top) rcNewCon.top=client.top;

			rcNewCon.right = rcNewCon.left + pVCon->Width + ((gpSet->isAlwaysShowScrollbar == 1) ? GetSystemMetrics(SM_CXVSCROLL) : 0);
			rcNewCon.bottom = rcNewCon.top + pVCon->Height;

			if (rcNewCon.right>client.right) rcNewCon.right=client.right;

			if (rcNewCon.bottom>client.bottom) rcNewCon.bottom=client.bottom;
		}
	}
	else
	{
		rcNewCon = client;
	}
#endif

	rcNewCon = gpConEmu->CalcRect(CER_WORKSPACE, mainClient, CER_MAINCLIENT, pVCon);

	CVConGroup::MoveAllVCon(pVCon, rcNewCon);
}
