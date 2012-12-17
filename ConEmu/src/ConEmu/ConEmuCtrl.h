
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

struct ConEmuHotKey;

// ��������� ���������� ����� ������������ "�� ����������" �� ��������� ������ � �����������
extern const ConEmuHotKey* ConEmuSkipHotKey; // = ((ConEmuHotKey*)INVALID_HANDLE_VALUE)

// ������� �������������� �������
extern const ConEmuHotKey* gpCurrentHotKey;

class CConEmuCtrl
{
public:
	CConEmuCtrl();
	virtual ~CConEmuCtrl();

	bool ProcessHotKeyMsg(UINT messg, WPARAM wParam, LPARAM lParam, const wchar_t *pszChars, CRealConsole* pRCon);
	const ConEmuHotKey* ProcessHotKey(DWORD VkState, bool bKeyDown, const wchar_t *pszChars, CRealConsole* pRCon);
	DWORD VkModFromVk(DWORD Vk);

	void UpdateControlKeyState();
	DWORD GetControlKeyState(LPARAM lParam);

	void FixSingleModifier(DWORD Vk, CRealConsole* pRCon);

	static void StatusCommand(ConEmuStatusCommand nStatusCmd, int IntParm = 0, LPCWSTR StrParm = NULL, CRealConsole* pRCon = NULL); // csc_ShowHide, csc_SetStatusText

	static void TabCommand(ConEmuTabCommand nTabCmd); // ctc_ShowHide, ctc_SwitchNext, � �.�.
	static size_t GetOpenedTabs(CESERVER_REQ_GETALLTABS::TabInfo*& pTabs);

	static void ChooseTabFromMenu(BOOL abFirstTabOnly, POINT pt, DWORD Align /*= TPM_CENTERALIGN|TPM_VCENTERALIGN*/);

	static void DoFindText(int nDirection, CRealConsole* pRCon = NULL);
	static void DoEndFindText(CRealConsole* pRCon = NULL);

	void SkipOneAppsRelease(bool abSkip);

	static void MakeScreenshot(bool abFullscreen = false);

protected:
	BOOL mb_InWinTabSwitch;
	BOOL mb_InCtrlTabSwitch;

	UINT mn_DoubleKeyConsoleNum; // ���������� VK
	void ResetDoubleKeyConsoleNum(CRealConsole* pRCon = NULL);

private:
	DWORD dwControlKeyState;
	bool bWin, bApps;
	bool bCaps, bNum, bScroll;
	bool bLAlt, bRAlt;
	bool bLCtrl, bRCtrl;
	bool bLShift, bRShift;
	DWORD mn_LastSingleModifier, mn_SingleModifierFixKey, mn_SingleModifierFixState;
	BOOL mb_LastSingleModifier;

	UINT m_SkippedMsg; WPARAM m_SkippedMsgWParam; LPARAM m_SkippedMsgLParam;

	static bool mb_SkipOneAppsRelease;
	static HHOOK mh_SkipOneAppsRelease;
	static LRESULT CALLBACK SkipOneAppsReleaseHook(int code, WPARAM wParam, LPARAM lParam);

public:
	// true-����������, false-���������� � �������

	// User (Keys)
	static bool WINAPI key_MinimizeRestore(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_MinimizeRestoreByEsc(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_GlobalRestore(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_MultiNew(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_MultiNewShift(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_MultiNewPopupMenu(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_MultiNewWindow(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_MultiNewAttach(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_MultiNext(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_MultiNextShift(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_MultiRecreate(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_AlternativeBuffer(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_MultiBuffer(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_MultiClose(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	//static bool WINAPI key_CloseTab(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_TerminateProcess(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	//static bool WINAPI key_CloseConEmu(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_DuplicateRoot(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_DuplicateRootAs(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_MultiCmd(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_CTSVkBlockStart(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_CTSVkTextStart(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_Screenshot(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_ScreenshotFull(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_ShowStatusBar(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_ShowTabBar(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_ShowCaption(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_AlwaysOnTop(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	// System (predefined, fixed)
	static bool WINAPI key_About(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_Hotkeys(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_Settings(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_SystemMenu(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_TabMenu(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_AltF9(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_ShowRealConsole(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_AltEnter(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_FullScreen(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_ForcedFullScreen(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_BufferScrollUp(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_BufferScrollDown(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_BufferScrollPgUp(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_BufferScrollPgDn(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_BufferScroll(bool TestOnly, BYTE vk, CRealConsole* pRCon);
	static bool WINAPI key_CtrlTab(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_CtrlShiftTab(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_CtrlTab_Prev(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_CtrlTab_Next(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	// PicView2 slideshow
	static bool WINAPI key_PicViewSlideshow(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	// GuiMacro
	static bool WINAPI key_GuiMacro(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	// Tabs list
	static bool WINAPI key_ShowTabsList(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_RenameTab(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_MoveTabLeft(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_MoveTabRight(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	// Clipboard
	static bool WINAPI key_PasteText(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_PasteTextAllApp(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_PasteFirstLine(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_PasteFirstLineAllApp(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	// Text editing
	static bool WINAPI key_DeleteWordToLeft(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	// Find text
	static bool WINAPI key_FindTextDlg(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);

public:
	// ��� ��� ���� - ���� ��������� � "HostKey"
	static bool WINAPI key_WinWidthDec(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_WinWidthInc(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_WinHeightDec(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_WinHeightInc(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	static bool WINAPI key_WinSize(BYTE vk);
	static bool WINAPI key_WinDragStart(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
	// Console activate by number
	static bool WINAPI key_ConsoleNum(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon);
};
