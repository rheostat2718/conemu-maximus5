
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


#pragma once

#include "MenuIds.h"

class CConEmuMenu
{
public:
	CConEmuMenu();
	~CConEmuMenu();

public:
	void OnNewConPopupMenu(POINT* ptWhere = NULL, DWORD nFlags = 0);

	LRESULT OnInitMenuPopup(HWND hWnd, HMENU hMenu, LPARAM lParam);
	bool OnMenuSelected(HMENU hMenu, WORD nID, WORD nFlags);
	void OnMenuRClick(HMENU hMenu, UINT nItemPos);

	POINT CalcTabMenuPos(CVirtualConsole* apVCon);
	HMENU GetSysMenu(BOOL abInitial = FALSE);

	void ShowSysmenu(int x=-32000, int y=-32000, bool bAlignUp = false);
	void OnNcIconLClick();

	HMENU CreateDebugMenuPopup();
	HMENU CreateEditMenuPopup(CVirtualConsole* apVCon, HMENU ahExist = NULL);
	HMENU CreateHelpMenuPopup();
	HMENU CreateVConListPopupMenu(HMENU ahExist, BOOL abFirstTabOnly);
	HMENU CreateVConPopupMenu(CVirtualConsole* apVCon, HMENU ahExist, BOOL abAddNew, HMENU& hTerminate);

	int trackPopupMenu(TrackMenuPlace place, HMENU hMenu, UINT uFlags, int x, int y, HWND hWnd, RECT *prcRect = NULL);


	void ShowPopupMenu(CVirtualConsole* apVCon, POINT ptCur, DWORD Align = TPM_LEFTALIGN);
	void ExecPopupMenuCmd(CVirtualConsole* apVCon, int nCmd);

	LRESULT OnSysCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);

	// Returns previous value
	bool SetPassSysCommand(bool abPass = true);
	bool GetPassSysCommand();
	bool SetInScMinimize(bool abInScMinimize);
	bool GetInScMinimize();
	bool SetRestoreFromMinimized(bool abInRestore);
	bool GetRestoreFromMinimized();
	TrackMenuPlace SetTrackMenuPlace(TrackMenuPlace tmpPlace);
	TrackMenuPlace GetTrackMenuPlace();

private:
	bool OnMenuSelected_NewCon(HMENU hMenu, WORD nID, WORD nFlags);
	void OnNewConPopupMenuRClick(HMENU hMenu, UINT nItemPos);

	void UpdateSysMenu(HMENU hSysMenu);

	void ShowMenuHint(HMENU hMenu, WORD nID, WORD nFlags);

	// � ������ "������ ���� �� keybar � FarManager" - ��� ���������
	// ������ (�������� Alt+Shift+F9) "������" � ������� Alt+Shift
	// ����� Far ����������� ������ ������ (������� ��������� ��� Alt+Shift+...)
	void ShowKeyBarHint(HMENU hMenu, WORD nID, WORD nFlags);

	LPCWSTR MenuAccel(int DescrID, LPCWSTR asText);

private:
	bool  mb_InNewConPopup, mb_InNewConRPopup;
	//int   mn_FirstTaskID, mn_LastTaskID; // MenuItemID for Tasks, when mb_InNewConPopup==true
	DWORD mn_SysMenuOpenTick, mn_SysMenuCloseTick;
	bool  mb_PassSysCommand;
	bool  mb_InScMinimize;
	bool  mb_InRestoreFromMinimized;
	
	TrackMenuPlace mn_TrackMenuPlace;

	//struct CmdHistory
	//{
	//	int nCmd;
	//	LPCWSTR pszCmd;
	//	wchar_t szShort[32];
	//} m_CmdPopupMenu[MAX_CMD_HISTORY+1]; // ��������� ��� ���� ������ ������� ����� �������

	struct CmdTaskPopupItem
	{
		enum CmdTaskPopupItemType { eNone, eTaskPopup, eTaskAll, eTaskCmd, eMore, eCmd, eNewDlg, eSetupTasks, eClearHistory } ItemType;
		int nCmd;
		//int iFromGrpNo;
		union
		{
			const void/*Settings::CommandTasks*/* pGrp;
			LPCWSTR pszCmd;
		};
		wchar_t szShort[32];
		HMENU hPopup;
		wchar_t* pszTaskBuf;
		BOOL bPopupInitialized;

		void Reset(CmdTaskPopupItemType newItemType, int newCmdId, LPCWSTR asName = NULL);
		void SetShortName(LPCWSTR asName);
		static void SetMenuName(wchar_t* pszDisplay, INT_PTR cchDisplayMax, LPCWSTR asName, bool bTrailingPeriod);
	};
	MArray<CmdTaskPopupItem> m_CmdTaskPopup;
	int mn_CmdLastID;
	CmdTaskPopupItem* mp_CmdRClickForce;

	// ��� �� CConEmuMain
	HMENU mh_SysDebugPopup, mh_SysEditPopup, mh_ActiveVConPopup, mh_TerminateVConPopup, mh_VConListPopup, mh_HelpPopup; // Popup's ��� SystemMenu
	HMENU mh_InsideSysMenu;
	// � ��� �� VirtualConsole
	HMENU mh_PopupMenu, mh_TerminatePopup, mh_VConDebugPopup, mh_VConEditPopup;
	// Array
	size_t mn_MenusCount; HMENU** mph_Menus;
};
